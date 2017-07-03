// CFailoverSyncTask.cpp: implementation of the CFailoverSyncTask class.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "FailoverSyncTask.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"
#include "psosxml.h"
#include "XmlMiniParser.h"
#include "SyncedElementOngoingConfs.h"
#include "SyncedElementIpService.h"
#include "SyncedElementMngmntService.h"
#include "SyncedElementBasicReservations.h"
#include "SyncedElementConfProfiles.h"
#include "SyncedElementMeetingRooms.h"
#include "SyncedElementReservations.h"
#include "SyncedElementRecordingLinks.h"
#include "SyncedElementIvrServices.h"

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CFailoverSyncTask)
	ONEVENT(FAILOVER_SOCKET_RCV_IND,	        ANYCASE,	CFailoverSyncTask::OnSocketRcvInd)
	ONEVENT(SYNC_IP_SERVICE_TIMER,				ANYCASE,	CFailoverSyncTask::OnTimerSyncIpServiceTimeout)
	ONEVENT(SYNC_MNGMNT_SERVICE_TIMER,			ANYCASE,	CFailoverSyncTask::OnTimerSyncMngmntServiceTimeout)
	ONEVENT(SYNC_IVR_SERVICES_TIMER,	        ANYCASE,	CFailoverSyncTask::OnTimerSyncIvrServicesTimeout)
	ONEVENT(SYNC_CONFERENCES_PROFILES_TIMER,	ANYCASE,	CFailoverSyncTask::OnTimerSyncConferenceProfilesTimeout)
	ONEVENT(SYNC_MEETING_ROOMS_TIMER,	        ANYCASE,	CFailoverSyncTask::OnTimerSyncMeetingRoomsTimeout)
	ONEVENT(SYNC_ONGOING_CONFERENCES_TIMER,		ANYCASE,	CFailoverSyncTask::OnTimerSyncOngoingConferencesTimeout)
	ONEVENT(SYNC_RESERVATIONS_TIMER,	        ANYCASE,	CFailoverSyncTask::OnTimerSyncReservationsTimeout)
	ONEVENT(SYNC_RECORDING_LINKS_TIMER,	        ANYCASE,	CFailoverSyncTask::OnTimerSyncRecordingLinksTimeout)

PEND_MESSAGE_MAP(CFailoverSyncTask,CStateMachine);


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
extern "C" void FailoverSyncEntryPoint(void* appParam)
{
	CFailoverSyncTask* pTaskApp = new CFailoverSyncTask;
	pTaskApp->Create(*(CSegment*)appParam);
}


//////////////////////////////////////////////////////////////////////////////
CFailoverSyncTask::CFailoverSyncTask() // constructor
{
	PTRACE(eLevelInfoNormal,"CFailoverSyncTask - CFailoverSyncTask");

	InitSyncedElementsArray();
	InitSyncTimersNamesArray();
	InitSyncTimersTimeoutsArray();
	InitIsSyncAlreadyDoneArray();
	
	m_isWholeSyncCompletedOnce = false;
}

/////////////////////////////////////////////////////////////////////////////
CFailoverSyncTask::~CFailoverSyncTask() // destructor
{
	for (int i=0; i<eNUM_OF_SYNCED_ELEMENTS; i++)
	{
		POBJDELETE(m_pSyncedElements[i]);

		if ( IsValidTimer(m_syncTimersNames[i]) )
			DeleteTimer(m_syncTimersNames[i]);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::InitTask()
{
	PTRACE(eLevelInfoNormal,"CFailoverSyncTask - InitTask");

	/*
	The sync is done in the foolowing order:
	    1. IpService, MngmntService
	    2. (upon IpService completion) IVR
	    3. (upon IVR completion) Profiles
	    4. (upon Profiles completion) MeetingRooms
	    5. (upon MeetingRooms completion) OngoingConfs, Reservations, RecordingLinks
	*/
	SendSyncSpec(eSyncedElement_IpService);
	SendSyncSpec(eSyncedElement_MngmntService);
}

//////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::InitSyncedElementsArray()
{
	m_pSyncedElements[eSyncedElement_ConfProfiles]			= new CSyncedElementConfProfiles();
	m_pSyncedElements[eSyncedElement_MeetingRooms]			= new CSyncedElementMeetingRooms();
	m_pSyncedElements[eSyncedElement_OngoingConferences]	= new CSyncedElementOngoingConfs();
    m_pSyncedElements[eSyncedElement_Reservations]			= new CSyncedElementReservations();
	m_pSyncedElements[eSyncedElement_IvrServices]			= new CSyncedElementIvrServices();
	m_pSyncedElements[eSyncedElement_RecordingLinks]		= new CSyncedElementRecordingLinks();
	m_pSyncedElements[eSyncedElement_MngmntService]			= new CSyncedElementMngmntService();
	m_pSyncedElements[eSyncedElement_IpService]				= new CSyncedElementIpService();
}

//////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::InitSyncTimersNamesArray()
{
	m_syncTimersNames[eSyncedElement_ConfProfiles]			= SYNC_CONFERENCES_PROFILES_TIMER;
	m_syncTimersNames[eSyncedElement_MeetingRooms]			= SYNC_MEETING_ROOMS_TIMER;
	m_syncTimersNames[eSyncedElement_OngoingConferences]	= SYNC_ONGOING_CONFERENCES_TIMER;
	m_syncTimersNames[eSyncedElement_Reservations]			= SYNC_RESERVATIONS_TIMER;
	m_syncTimersNames[eSyncedElement_IvrServices]			= SYNC_IVR_SERVICES_TIMER;
	m_syncTimersNames[eSyncedElement_RecordingLinks]		= SYNC_RECORDING_LINKS_TIMER;
	m_syncTimersNames[eSyncedElement_MngmntService]			= SYNC_MNGMNT_SERVICE_TIMER;
	m_syncTimersNames[eSyncedElement_IpService]				= SYNC_IP_SERVICE_TIMER;
}

//////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::InitSyncTimersTimeoutsArray()
{
	m_syncTimersTimeouts[eSyncedElement_ConfProfiles]		= SYNC_ONGOING_CONFERNCES_TIMEOUT;
	m_syncTimersTimeouts[eSyncedElement_MeetingRooms]		= SYNC_ONGOING_CONFERNCES_TIMEOUT;
	m_syncTimersTimeouts[eSyncedElement_OngoingConferences]	= SYNC_ONGOING_CONFERNCES_TIMEOUT;
	m_syncTimersTimeouts[eSyncedElement_Reservations]		= SYNC_ONGOING_CONFERNCES_TIMEOUT;
	m_syncTimersTimeouts[eSyncedElement_IvrServices]		= SYNC_ONGOING_CONFERNCES_TIMEOUT;
	m_syncTimersTimeouts[eSyncedElement_RecordingLinks]		= SYNC_ONGOING_CONFERNCES_TIMEOUT;
	m_syncTimersTimeouts[eSyncedElement_MngmntService]		= SYNC_IP_SERVICE_TIMEOUT;
	m_syncTimersTimeouts[eSyncedElement_IpService]			= SYNC_IP_SERVICE_TIMEOUT;
}

//////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::InitIsSyncAlreadyDoneArray()
{
	m_isSyncAlreadyDone[eSyncedElement_ConfProfiles]		= false;
	m_isSyncAlreadyDone[eSyncedElement_MeetingRooms]		= false;
	m_isSyncAlreadyDone[eSyncedElement_OngoingConferences]	= false;
	m_isSyncAlreadyDone[eSyncedElement_Reservations]		= false;
	m_isSyncAlreadyDone[eSyncedElement_IvrServices]			= false;
	m_isSyncAlreadyDone[eSyncedElement_RecordingLinks]		= false;
	m_isSyncAlreadyDone[eSyncedElement_MngmntService]		= false;
	m_isSyncAlreadyDone[eSyncedElement_IpService]			= false;
}

//////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::SendSyncSpec(eSyncedElement theSyncElement)
{
	if ( (0 <= theSyncElement) && (eNUM_OF_SYNCED_ELEMENTS > theSyncElement) )
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::SendSpecSync - " << GetSyncedElementStr(theSyncElement) << " element sent basic GET trans";
		m_pSyncedElements[theSyncElement]->SendBasicGetTrans();
		StartTimer(m_syncTimersNames[theSyncElement], m_syncTimersTimeouts[theSyncElement]);
	}
	
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::SendSpecSync - illegal element: " << theSyncElement;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::OnSocketRcvInd(CSegment* pMsg)
{
	DWORD len;
	*pMsg >> len;

	ALLOCBUFFER(pXMLString, len+1);

	*pMsg >> pXMLString;
	pXMLString[len] = '\0';

	// ===== 1. RESPONSE_TRANS_CONF_LIST
	if ( strstr(pXMLString, "RESPONSE_TRANS_CONF_LIST") )
	{
		if (CXmlMiniParser::GetResponseStatus(pXMLString) == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_CONF_LIST with status ok";
			TreatConfListResponse(pXMLString);
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_CONF_LIST with status NOT ok!!!";
			//TODO: Complete code !!
		}
	} // end RESPONSE_TRANS_CONF_LIST


	// ===== 2. RESPONSE_TRANS_CONF
	else if ( strstr(pXMLString, "RESPONSE_TRANS_CONF") )
	{
		if (CXmlMiniParser::GetResponseStatus(pXMLString) == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_CONF with status ok";
			TreatConfResponse(pXMLString);
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_CONF with status NOT ok!!!";
			//TODO: Complete code !!
		}
	} // end RESPONSE_TRANS_CONF


	// ===== 3. RESPONSE_TRANS_RES_LIST
	else if ( strstr(pXMLString, "RESPONSE_TRANS_RES_LIST") )
	{
		if (CXmlMiniParser::GetResponseStatus(pXMLString) == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_RES_LIST with status ok";
			TreatResListResponse(pXMLString);
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_RES_LIST with status NOT ok!!!";
			//TODO: Complete code !!
		}
	} // end RESPONSE_TRANS_RES_LIST


	// ===== 4. RESPONSE_TRANS_RES
	else if ( strstr(pXMLString, "RESPONSE_TRANS_RES") )
	{
		if (CXmlMiniParser::GetResponseStatus(pXMLString) == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_RES with status ok";
			TreatResResponse(pXMLString);
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_RES with status NOT ok!!!";
			//TODO: Complete code !!
		}
	} // end RESPONSE_TRANS_RES_LIST


	// ===== 5. RESPONSE_TRANS_IP_SERVICE_LIST
	else if ( strstr(pXMLString, "RESPONSE_TRANS_IP_SERVICE_LIST") )
	{
		if (CXmlMiniParser::GetResponseStatus(pXMLString) == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_IP_SERVICE_LIST with status ok";
			TreatIpServiceListResponse(pXMLString);
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_IP_SERVICE_LIST with status NOT ok!!!";
			//TODO: Complete code !!
		}
	} // end RESPONSE_TRANS_IP_SERVICE_LIST


	// ===== 6. RESPONSE_TRANS_RECORDING_LINKS_LIST
	else if ( strstr(pXMLString, "RESPONSE_TRANS_RECORDING_LINKS_LIST") )
	{
		if (CXmlMiniParser::GetResponseStatus(pXMLString) == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_RECORDING_LINKS_LIST with status ok";
			TreatRecordingLinksResponse(pXMLString);
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_RECORDING_LINKS_LIST with status NOT ok!!!";
			//TODO: Complete code !!
		}
	} // end RESPONSE_TRANS_IP_SERVICE_LIST


	// ===== 7. RESPONSE_TRANS_AV_MSG_SERVICE_LIST
	else if ( strstr(pXMLString, "RESPONSE_TRANS_AV_MSG_SERVICE_LIST") )
	{
		if (CXmlMiniParser::GetResponseStatus(pXMLString) == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_AV_MSG_SERVICE_LIST with status ok";
			TreatIvrServicesResponse(pXMLString);
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - RESPONSE_TRANS_AV_MSG_SERVICE_LIST with status NOT ok!!!";
			//TODO: Complete code !!
		}
	} // end RESPONSE_TRANS_IP_SERVICE_LIST

	// ===== 8. other transaction
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnSocketRcvInd - other response";
	}
	
	DEALLOCBUFFER(pXMLString);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::TreatConfListResponse(const char* pXMLString)
{
    CXMLDOMDocument xmlDoc;
    xmlDoc.Parse((const char**)&pXMLString);
    CXMLDOMElement *pResponseNode = xmlDoc.GetRootElement();
    
    bool isChanged = IsChanged( pResponseNode, eSyncedElement_OngoingConferences,
    							"ACTION", "GET_FULL_CONF_CHANGE_LS", "FULL_CHANGE_LS");

    if (isChanged)
    {
    	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatConfListResponse - changed from previously";

    	m_pSyncedElements[eSyncedElement_OngoingConferences]->HandleChange(pResponseNode);
    } // end if isChanged
    
    else // no change from previously
    {
    	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatConfListResponse - no change from previously";
    }
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::TreatResListResponse(const char* pXMLString)
{
    CXMLDOMDocument xmlDoc;
    xmlDoc.Parse((const char**)&pXMLString);
    CXMLDOMElement *pResponseNode = xmlDoc.GetRootElement();

    eSyncedElement curElement;
    char* elementName = NULL;
    char* actionNodeName = NULL;

    CXMLDOMElement *pActionNode	= NULL;
    pResponseNode->getChildNodeByName(&pActionNode, "ACTION");
    if (pActionNode)
    {
    	bool bFound = FALSE;
    	CXMLDOMElement *pSpecActionNode	= NULL;
		pActionNode->getChildNodeByName(&pSpecActionNode, "GET_PROFILE_LIST");
		if (pSpecActionNode)
		{
			curElement = eSyncedElement_ConfProfiles;
			elementName = "PROFILE_SUMMARY_LS";
			actionNodeName = "GET_PROFILE_LIST";
			bFound = TRUE;
		}

		if (bFound == FALSE)
		{
			pActionNode->getChildNodeByName(&pSpecActionNode, "GET_MEETING_ROOM_LIST");
			if (pSpecActionNode)
			{
				curElement = eSyncedElement_MeetingRooms;
				elementName = "MEETING_ROOM_SUMMARY_LS";
				actionNodeName = "GET_MEETING_ROOM_LIST";
				bFound = TRUE;
			}
		}

		if (bFound == FALSE)
		{
			pActionNode->getChildNodeByName(&pSpecActionNode, "GET_RES_LIST");
			if (pSpecActionNode)
			{
				curElement = eSyncedElement_Reservations;
				elementName = "RES_SUMMARY_LS";
				actionNodeName = "GET_RES_LIST";
				bFound = TRUE;
			}
		}

		if (pSpecActionNode == NULL)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatResListResponse - Element not found";
			return;//??? start timer again???
		}

		bool isChanged = IsChanged(pResponseNode, curElement, "ACTION", actionNodeName, elementName);

		if (isChanged)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatResListResponse - changed from previously";

			m_pSyncedElements[curElement]->HandleChange(pResponseNode);
		} // end if isChanged

		else // no change from previously
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatResListResponse - no change from previously";
		}
    }
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::TreatConfResponse(const char* pXMLString)
{
   	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatConfResponse";

   	CXMLDOMDocument xmlDoc;
    xmlDoc.Parse((const char**)&pXMLString);
    CXMLDOMElement *pResponseNode = xmlDoc.GetRootElement();
    
    ((CSyncedComplexElement*)m_pSyncedElements[eSyncedElement_OngoingConferences])->HandleResponseTrans2(pResponseNode);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::TreatResResponse(const char* pXMLString)
{
   	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatResResponse";

   	CXMLDOMDocument xmlDoc;
    xmlDoc.Parse((const char**)&pXMLString);
    CXMLDOMElement *pResponseNode = xmlDoc.GetRootElement();

	eSyncedElement curElement;
	char* elementName = NULL;
	char* actionNodeName = NULL;

	CXMLDOMElement *pActionNode	= NULL;
	pResponseNode->getChildNodeByName(&pActionNode, "ACTION");
	if (pActionNode)
	{
		bool bFound = FALSE;
		CXMLDOMElement *pSpecActionNode	= NULL;
		pActionNode->getChildNodeByName(&pSpecActionNode, "GET_PROFILE");
		if (pSpecActionNode)
		{
			curElement = eSyncedElement_ConfProfiles;
			bFound = TRUE;
		}

		if (bFound == FALSE)
		{
			pActionNode->getChildNodeByName(&pSpecActionNode, "GET_MEETING_ROOM");
			if (pSpecActionNode)
			{
				curElement = eSyncedElement_MeetingRooms;
				bFound = TRUE;
			}
		}

		if (bFound == FALSE)
		{
			pActionNode->getChildNodeByName(&pSpecActionNode, "GET_RES");
			if (pSpecActionNode)
			{
				curElement = eSyncedElement_Reservations;
				bFound = TRUE;
			}
		}

		if (pSpecActionNode == NULL)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatResResponse - Element not found";
			return;//??? start timer again???
		}

		((CSyncedComplexElement*)m_pSyncedElements[curElement])->HandleResponseTrans2(pResponseNode);
	}
    m_pSyncedElements[eSyncedElement_OngoingConferences]->HandleResponseTransConf(pResponseNode);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::TreatRecordingLinksResponse(const char* pXMLString)
{
    CXMLDOMDocument xmlDoc;
    xmlDoc.Parse((const char**)&pXMLString);
    CXMLDOMElement *pResponseNode = xmlDoc.GetRootElement();

    bool isChanged = IsChanged(pResponseNode, eSyncedElement_RecordingLinks,
    							"ACTION", "GET", "RECORDING_LINKS_LIST");

    if (isChanged)
    {
    	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatRecordingLinksResponse - changed from previously";

    	m_pSyncedElements[eSyncedElement_RecordingLinks]->HandleChange(pResponseNode);
    } // end if isChanged

    else // no change from previously
    {
    	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatRecordingLinksResponse - no change from previously";
    }
}

////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::TreatIvrServicesResponse(const char* pXMLString)
{
    CXMLDOMDocument xmlDoc;
    xmlDoc.Parse((const char**)&pXMLString);
    CXMLDOMElement *pResponseNode = xmlDoc.GetRootElement();

    bool isChanged = IsChanged(pResponseNode, eSyncedElement_IvrServices,
    							"ACTION", "GET_IVR_LIST", "AV_SERVICE_LIST");

    if (isChanged)
    {
    	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatIvrServicesResponse - changed from previously";

    	m_pSyncedElements[eSyncedElement_IvrServices]->HandleChange(pResponseNode);
    } // end if isChanged

    else // no change from previously
    {
    	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatIvrServicesResponse - no change from previously";
    }
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::TreatIpServiceListResponse(const char* pXMLString)
{
   	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatIpServiceListResponse";

   	CXMLDOMDocument xmlDoc;
    xmlDoc.Parse((const char**)&pXMLString);
    CXMLDOMElement *pResponseNode = xmlDoc.GetRootElement();

	eSyncedElement curElement;
	char* elementName = NULL;
	char* actionNodeName = NULL;

	CXMLDOMElement *pActionNode	= NULL;
	pResponseNode->getChildNodeByName(&pActionNode, "ACTION");
	if (pActionNode)
	{
		CXMLDOMElement *pSpecActionNode	= NULL;
		pActionNode->getChildNodeByName(&pSpecActionNode, "GET");
		if (pSpecActionNode) // IpService element
		{
			TreatIpServiceChangeIfNeeded(pXMLString, eSyncedElement_IpService, "GET");
		}

		else
		{
			pActionNode->getChildNodeByName(&pSpecActionNode, "GET_MANAGEMENT_NETWORK_LIST");
			if (pSpecActionNode) // MngmntService element
			{
				TreatIpServiceChangeIfNeeded(pXMLString, eSyncedElement_MngmntService, "GET_MANAGEMENT_NETWORK_LIST");
			}
		}

		if (pSpecActionNode == NULL)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatIpServiceListResponse - Element not found";
			return;//??? start timer again???
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::TreatIpServiceChangeIfNeeded(const char* pXMLString, eSyncedElement curElement, char* specActionNodeName)
{
    CXMLDOMDocument xmlDoc;
    xmlDoc.Parse((const char**)&pXMLString);
    CXMLDOMElement *pResponseNode = xmlDoc.GetRootElement();
    
    bool isChanged = IsChanged( pResponseNode, curElement, "ACTION", specActionNodeName, "IP_SERVICE_LIST" );

    if (isChanged)
    {
    	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatIpServiceChangeIfNeeded - changed from previously"
    						   << "\ncurElement:         "	<< GetSyncedElementStr(curElement)
    						   << "\nspecActionNodeName: "	<< specActionNodeName;

    	m_pSyncedElements[curElement]->HandleChange(pResponseNode);
    } // end if isChanged
    
    else // no change from previously
    {
    	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatIpServiceChangeIfNeeded - no change from previously"
							   << "\ncurElement:         "	<< GetSyncedElementStr(curElement)
							   << "\nspecActionNodeName: "	<< specActionNodeName;
    }

}

/////////////////////////////////////////////////////////////////////////////
bool CFailoverSyncTask::IsChanged( CXMLDOMElement *pRootElem, eSyncedElement curElement,
									char* actionNodeName, char* specActionNodeName, char* elementName )
{
	bool isChanged = false;
	
	CXMLDOMElement *pActionNode		= NULL,
				   *pSpecActionNode	= NULL,
				   *pElemNode		= NULL;

	
	pRootElem->getChildNodeByName(&pActionNode, actionNodeName);
	if (pActionNode)
	{
		pActionNode->getChildNodeByName(&pSpecActionNode, specActionNodeName);
		if (pSpecActionNode)
		{
			pSpecActionNode->getChildNodeByName(&pElemNode, elementName);
			if (pElemNode)
			{
				char* isChangedStr;
				pElemNode->getChildNodeValueByName("CHANGED", &isChangedStr);
				
				if ( !strcmp("true", isChangedStr) )
				{
					isChanged = true;
					
					// update objToken
					int   objToken;
					char* objTokenStr;
					pElemNode->getChildNodeValueByName("OBJ_TOKEN", &objTokenStr);
					objToken = atoi(objTokenStr);

					m_pSyncedElements[curElement]->SetObjToken(objToken);
				}
			}
		} // end if pSpecActionNode
	} // end if pActionNode

	return isChanged;
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::OnTimerSyncIpServiceTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnTimerSyncIpServiceTimeout";
	
	if (false == m_isSyncAlreadyDone[eSyncedElement_IpService])
	{
		m_isSyncAlreadyDone[eSyncedElement_IpService] = true;
		if (false == m_isWholeSyncCompletedOnce)
		{
			TreatWholeSyncCompletedOnce();
		}

		SendSyncSpec(eSyncedElement_IvrServices); // IVR sync starts upon IpService sync completes once
	}
	
	SendSyncSpec(eSyncedElement_IpService);

	return;
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::OnTimerSyncMngmntServiceTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnTimerSyncMngmntServiceTimeout";
	
	m_isSyncAlreadyDone[eSyncedElement_MngmntService] = true;
	if (false == m_isWholeSyncCompletedOnce)
	{
		TreatWholeSyncCompletedOnce();
	}

	SendSyncSpec(eSyncedElement_MngmntService);

	return;
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::OnTimerSyncIvrServicesTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnTimerSyncIvrServicesTimeout";

	if (false == m_isSyncAlreadyDone[eSyncedElement_IvrServices])
	{
		m_isSyncAlreadyDone[eSyncedElement_IvrServices] = true;
		if (false == m_isWholeSyncCompletedOnce)
		{
			TreatWholeSyncCompletedOnce();
		}

		SendSyncSpec(eSyncedElement_ConfProfiles); // Profiles sync starts upon IVR sync completes once
	}

	SendSyncSpec(eSyncedElement_IvrServices);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::OnTimerSyncConferenceProfilesTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnTimerSyncConferenceProfilesTimeout";

	if (false == m_isSyncAlreadyDone[eSyncedElement_ConfProfiles])
	{
		m_isSyncAlreadyDone[eSyncedElement_ConfProfiles] = true;
		if (false == m_isWholeSyncCompletedOnce)
		{
			TreatWholeSyncCompletedOnce();
		}

		SendSyncSpec(eSyncedElement_MeetingRooms); // MRs sync starts upon Profiles sync completes once
	}

	SendSyncSpec(eSyncedElement_ConfProfiles);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::OnTimerSyncMeetingRoomsTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnTimerSyncMeetingRoomsTimeout";

	if (false == m_isSyncAlreadyDone[eSyncedElement_MeetingRooms])
	{
		m_isSyncAlreadyDone[eSyncedElement_MeetingRooms] = true;
		if (false == m_isWholeSyncCompletedOnce)
		{
			TreatWholeSyncCompletedOnce();
		}

		SendSyncSpec(eSyncedElement_OngoingConferences);
		SendSyncSpec(eSyncedElement_Reservations);
		SendSyncSpec(eSyncedElement_RecordingLinks);
	}

	SendSyncSpec(eSyncedElement_MeetingRooms);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::OnTimerSyncOngoingConferencesTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnTimerSyncOngoingConferencesTimeout";
	
	m_isSyncAlreadyDone[eSyncedElement_OngoingConferences] = true;
	if (false == m_isWholeSyncCompletedOnce)
	{
		TreatWholeSyncCompletedOnce();
	}

	SendSyncSpec(eSyncedElement_OngoingConferences);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::OnTimerSyncReservationsTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnTimerSyncReservationsTimeout";

	m_isSyncAlreadyDone[eSyncedElement_Reservations] = true;
	if (false == m_isWholeSyncCompletedOnce)
	{
		TreatWholeSyncCompletedOnce();
	}

	SendSyncSpec(eSyncedElement_Reservations);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::OnTimerSyncRecordingLinksTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::OnTimerSyncRecordingLinksTimeout";
	
	m_isSyncAlreadyDone[eSyncedElement_RecordingLinks] = true;
	if (false == m_isWholeSyncCompletedOnce)
	{
		TreatWholeSyncCompletedOnce();
	}

	SendSyncSpec(eSyncedElement_RecordingLinks);
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverSyncTask::TreatWholeSyncCompletedOnce()
{
	m_isWholeSyncCompletedOnce = IsWholeSyncCompletedOnce();
	
	TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::TreatWholeSyncCompletedOnce"
						   << "\nm_isWholeSyncCompletedOnce == " << (m_isWholeSyncCompletedOnce? "true" : "false");

	if (true == m_isWholeSyncCompletedOnce)
	{
	    CManagerApi api(eProcessFailover);
		api.SendOpcodeMsg(WHOLE_SYNC_COMPLETED_IND);
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CFailoverSyncTask::IsWholeSyncCompletedOnce()
{
	bool isSomethingMissing = false;

	for (int i=0; i<eNUM_OF_SYNCED_ELEMENTS; i++)
	{
		if ( false == m_isSyncAlreadyDone[i])
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailoverSyncTask::IsWholeSyncCompletedOnce"
								   << "\nStill missing sync of " << GetSyncedElementStr( (eSyncedElement)i );

			isSomethingMissing = true;
			break;
		}
	}
	
	return !isSomethingMissing;	// if isSomethingMissing==false, then the return value for 'IsWholeSyncCompleted' is true (and vice versa) 
}

