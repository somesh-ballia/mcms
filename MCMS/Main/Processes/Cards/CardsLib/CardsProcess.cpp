// CardsProcess.cpp

#include "CardsProcess.h"

#include <netinet/in.h>

#include "StatusesGeneral.h"
#include "StringsLen.h"
#include "StringsMaps.h"
#include "HwMonitoring.h"
#include "TraceStream.h"
#include "SystemFunctions.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "Segment.h"
#include "McmsDaemonApi.h"
#include "ApiStatuses.h"
#include "SNMPStructs.h"
#include "ManagerApi.h"
#include "MfaTask.h"
#include "MplMcmsProtocol.h"
#include "MplMcmsProtocolTracer.h"
#include "EthernetSettingsMonitoring.h"
#include "SlotsNumberingConversionTableWrapper.h"
#include "OpcodesMcmsCardMngrICE.h"

extern char* CardTypeToString(APIU32 cardType);
extern char* CardUnitLoadedStatusToString(APIU32 unitLoadedStatus);
extern char* CardMediaIpConfigStatusToString(APIU32 mediaIpConfigStatus);
extern char* CardStateToString(APIU32 cardState);
extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);
extern char* ProductTypeToString(APIU32 productType);

extern void  CardsManagerEntryPoint(void* appParam);

CProcessBase* CreateNewProcess()
{
	return new CCardsProcess;
}

TaskEntryPoint CCardsProcess::GetManagerEntryPoint()
{
	return CardsManagerEntryPoint;
}

CCardsProcess::CCardsProcess() : m_bSNMPEnabled(false)
{
	int i=0, j=0;
	for (i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for(j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			m_pMfaTasksMbxs_List[i][j] = NULL;
			m_MfaTasksState_List[i][j] = eMfaTaskOk;
		}
	}

	eProductType curProductType = GetProductType();

	m_pLicensingStruct = new CARDS_LICENSING_S;
	m_pLicensingStruct->authenticationStruct.productType      = (DWORD)curProductType;
	m_pLicensingStruct->authenticationStruct.switchBoardId    = 0;
	m_pLicensingStruct->authenticationStruct.switchSubBoardId = 1;
	m_pLicensingStruct->authenticationStruct.rmxSystemCardsMode = (DWORD)( GetRmxSystemCardsModeDefault() );
	m_pLicensingStruct->authenticationStruct.chassisVersion.ver_major = 1;
	m_pLicensingStruct->authenticationStruct.chassisVersion.ver_minor = 0;
	m_pLicensingStruct->authenticationStruct.chassisVersion.ver_release = 0;
	m_pLicensingStruct->authenticationStruct.chassisVersion.ver_internal = 1;
	m_pLicensingStruct->federal = NO;

	InitMediaBoardsIds(curProductType);

	m_pAuthenticationStruct = new MCMS_AUTHENTICATION_S;
	m_pAuthenticationStruct->productType      = (DWORD)curProductType;
	m_pAuthenticationStruct->switchBoardId		= 0;
	m_pAuthenticationStruct->switchSubBoardId	= 1;
	m_pAuthenticationStruct->rmxSystemCardsMode	= (DWORD)( GetRmxSystemCardsModeDefault() );
	m_pAuthenticationStruct->chassisVersion.ver_major = 1;
	m_pAuthenticationStruct->chassisVersion.ver_minor = 0;
	m_pAuthenticationStruct->chassisVersion.ver_release = 0;
	m_pAuthenticationStruct->chassisVersion.ver_internal = 1;
	m_isAuthenticationSucceeded		= NO;
	m_numOfBoards					= 0;
	m_numOfBoardsExpected			= 0;
	m_isToTreatRtmIsdnKeepAlive		= YES; // 14/01/07 - according to Emb request // 26.01.09 - Emb confirmed
	
	m_pIpMediaParamsVector     = new CMediaIpParamsVector;
	m_pCardsMonitoringDB       = new CCommCardDB;
	m_pIvrAddMusicSourceVector = new CIvrAddMusicSourceVector;

	m_pRtmIsdnSpanMapsList  = new RTM_ISDN_SPAN_MAPS_LIST_S;
	memset(m_pRtmIsdnSpanMapsList, 0, sizeof(RTM_ISDN_SPAN_MAPS_LIST_S));

	m_rtmIsdnParamsIdx = 0;
	for (i=0; i<MAX_ISDN_SERVICES_IN_LIST; i++)
	{
		m_pRtmIsdnParamsList[i] = NULL;
	}

	for (i=0; i<NumOfCardsStartupCondTypes; i++)
	{
		m_StartupEndCondArray[i] = FALSE;
	}
/*------ TEMP -------------------------------------------------------------------*/
	m_StartupEndCondArray[eCardsIvr] = TRUE;
/*--------------------------------------------------------------------------------*/

	m_systemCardsModeCur					= GetRmxSystemCardsModeDefault();
    m_systemCardsModeFromFile				= GetRmxSystemCardsModeDefault();

    TRACEINTO << "\nCCardsProcess::CCardsProcess - m_systemCardsModeFromFile " << m_systemCardsModeFromFile;

    m_NGBSystemCardsMode                    = eNGBSystemCardsMode_illegal;
    m_isCardsModeEventAlertAlreadyProduced	= false; // since it may be produced from various tasks of the process
    m_isIvrMusicAddSrcReqReceived			= false;

	m_cpuBoardId	= 0;
	m_cpuSubBoardId	= 0;
    m_pEthernetSettingsStructsList = new CEthernetSettingsStructWrappersList;
    
    m_pSlotsNumConversionTable = new CSlotsNumberingConversionTableWrapper;
    m_pIceResponseList = NULL;

	for (i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		m_CardsHotSwapStatus[i] = NO;
	}

	memset(m_CSInternalExtMsgsStatus, 0, sizeof(CS_EXT_INT_MSG_CARD_PARAMS_S)* (MAX_NUM_OF_IP_SERVICES+1));
	// VNGR-21504
	m_firstSsmKeepAliveReceived = FALSE;
	memset(&m_lastSsmKeepAliveStruct, 0, sizeof(SWITCH_SM_KEEP_ALIVE_S));
	m_last_total_software_progress = 0;
	m_last_total_ipmc_progress = 0;
	m_numberofIpmcIndToCount = 0;
	m_numberOfBoardInstalling = 0;
	m_numberOfBoardNeedsIpmc = 0;

}

//////////////////////////////////////////////////////////////////////
CCardsProcess::~CCardsProcess()
{
	POBJDELETE(m_pLicensingStruct);

	POBJDELETE(m_pIpMediaParamsVector);
	POBJDELETE(m_pIvrAddMusicSourceVector);
	POBJDELETE(m_pCardsMonitoringDB);

	POBJDELETE(m_pRtmIsdnSpanMapsList);

	for (int i=0; i<MAX_ISDN_SERVICES_IN_LIST; i++)
	{
		POBJDELETE(m_pRtmIsdnParamsList[i]);
	}

	POBJDELETE(m_pEthernetSettingsStructsList);
	POBJDELETE(m_pSlotsNumConversionTable);
	POBJDELETE(m_pIceResponseList);
	POBJDELETE(m_pAuthenticationStruct);

// Note: m_pMfaTasksMbxs_List should not be deleted here,
//       since its items are already deleted in CTaskApp::~CTaskApp
}

//////////////////////////////////////////////////////////////////////
void CCardsProcess::AddToMfaTasksList(COsQueue* mbx, DWORD boardId, DWORD subBoardId)
{
	if ( (MAX_NUM_OF_BOARDS <= boardId) || (MAX_NUM_OF_SUBBOARDS <= subBoardId) )
	{
		char buff[TEN_LINE_BUFFER_LEN];
		sprintf(buff, "CCardsProcess::AddToMfaTasksList - Illegal: boardId: %d, subBoardId: %d", boardId, subBoardId);
		PASSERTMSG(1, buff);
	}

	m_pMfaTasksMbxs_List[boardId][subBoardId] = mbx;
	m_MfaTasksState_List[boardId][subBoardId] = eMfaTaskOk;

	TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::AddToMfaTasksList: " << m_pMfaTasksMbxs_List[boardId][subBoardId]
	                       << "\nBoardId " << boardId << ", subBoardId " << subBoardId;
}

void CCardsProcess::RemoveFromMfaTasksList(DWORD boardId, DWORD subBoardId)
{
	if ( (MAX_NUM_OF_BOARDS <= boardId) || (MAX_NUM_OF_SUBBOARDS <= subBoardId) )
	{
		char buff[TEN_LINE_BUFFER_LEN];
		sprintf(buff, "CCardsProcess::RemoveFromMfaTasksList - Illegal: boardId: %d, subBoardId: %d", boardId, subBoardId);
		PASSERTMSG(1, buff);
	}

	TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::RemoveFromMfaTasksList: " << m_pMfaTasksMbxs_List[boardId][subBoardId]
	                       << "\n(boardId " << boardId << ", subBoardId " << subBoardId << ")";
    TurnMfaTaskToZombie(m_pMfaTasksMbxs_List[boardId][subBoardId]);
	m_pMfaTasksMbxs_List[boardId][subBoardId] = NULL;
	m_MfaTasksState_List[boardId][subBoardId] = eMfaTaskOk;

}

void CCardsProcess::TurnMfaTaskToZombie(COsQueue* mbx)
{
	if (NULL == mbx)
	{
		TRACESTR(eLevelInfoNormal)<< "\nCCardsProcess::TurnMfaTaskToZombie: no mbx";
		return;
	}

	BOOL isFound = NO;

	int i=0, j=0;
	for (i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			if ( (mbx != NULL) && (mbx == m_pMfaTasksMbxs_List[i][j]) )
			{
				isFound=YES;
				m_MfaTasksState_List[i][j] = eMfaTaskZombie;

				TRACESTR(eLevelInfoNormal)<< "\nCCardsProcess::TurnMfaTaskToZombie"
				                      << "\nMbx for boardId " << i << ", subBoard " << j
				                      << " becomes Zombie";
			} // end record found
		} // end loop (j)
	} // end loop (i)

	if (NO == isFound)
	{
		TRACESTR(eLevelInfoNormal)<< "\nCCardsProcess::TurnMfaTaskToZombie: no mbx found";
	}
}

void CCardsProcess::RemoveFromMfaTasksList(COsQueue* mbx)
{
	if (NULL == mbx)
	{
		TRACESTR(eLevelInfoNormal)<< "\nCCardsProcess::RemoveFromMfaTasksList: no mbx";
		return;
	}

	BOOL isFound = NO;

	int i=0, j=0;
	for (i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			if ( (mbx != NULL) && (mbx == m_pMfaTasksMbxs_List[i][j]) )
			{
				isFound=YES;
				m_pMfaTasksMbxs_List[i][j] = NULL;
				m_MfaTasksState_List[i][j] = eMfaTaskOk;

 				TRACESTR(eLevelInfoNormal)<< "\nCCardsProcess::RemoveFromMfaTasksList"
				                      << "\nMbx for boardId " << i << ", subBoard " << j
				                      << " is deleted from MfaTasksMbxs_List";
			} // end record found
		} // end loop (j)

		if (YES == isFound)
			break;
	} // end loop (i)

	if (NO == isFound)
	{
		TRACESTR(eLevelInfoNormal)<< "\nCCardsProcess::RemoveFromMfaTasksList: no mbx found";
	}
}

COsQueue* CCardsProcess::GetMfaMbx(DWORD boardId, DWORD subBoardId)
{
	PASSERTSTREAM_AND_RETURN_VALUE(
	    (MAX_NUM_OF_BOARDS <= boardId) || (MAX_NUM_OF_SUBBOARDS <= subBoardId),
	    "Illegal boardId " << boardId << ", subBoardId " << subBoardId,
	    NULL);

	return m_pMfaTasksMbxs_List[boardId][subBoardId];
}

//////////////////////////////////////////////////////////////////////
void CCardsProcess::SetMfaTaskState(DWORD boardId, DWORD subBoardId, eMfaTaskStateType mfaTaskState)
{
	if ( (MAX_NUM_OF_BOARDS <= boardId) || (MAX_NUM_OF_SUBBOARDS <= subBoardId) )
	{
		char buff[TEN_LINE_BUFFER_LEN];
		sprintf(buff, "CCardsProcess::SetMfaTaskState - Illegal: boardId: %d, subBoardId: %d", boardId, subBoardId);
		PASSERTMSG(1, buff);
	}

	m_MfaTasksState_List[boardId][subBoardId] = mfaTaskState;
}

//////////////////////////////////////////////////////////////////////
eMfaTaskStateType CCardsProcess::GetMfaTaskState(DWORD boardId, DWORD subBoardId)
{
	if ( (MAX_NUM_OF_BOARDS <= boardId) || (MAX_NUM_OF_SUBBOARDS <= subBoardId) )
	{
		char buff[TEN_LINE_BUFFER_LEN];
		sprintf(buff, "CCardsProcess::GetMfaTaskState - Illegal: boardId: %d, subBoardId: %d", boardId, subBoardId);
		PASSERTMSG(1, buff);
	}

	return m_MfaTasksState_List[boardId][subBoardId];
}

CMediaIpParamsVector* CCardsProcess::GetMediaIpParamsVector()const
{
	return m_pIpMediaParamsVector;
}

//////////////////////////////////////////////////////////////////////
CIvrAddMusicSourceVector* CCardsProcess::GetIvrAddMusicSourceVector()const
{
	return m_pIvrAddMusicSourceVector;
}

/////////////////////////////////////////////////////////////////////////
CCommCardDB* CCardsProcess::GetCardsMonitoringDB()const
{
	return m_pCardsMonitoringDB;
}

//////////////////////////////////////////////////////////////////////
void CCardsProcess::AddExtraStringsToMap()
{
	CStringsMaps::AddItem(CARD_STATE_ENUM, eNormal,       ::CardStateToString(eNormal));
	CStringsMaps::AddItem(CARD_STATE_ENUM, eMajorError,   ::CardStateToString(eMajorError));
	CStringsMaps::AddItem(CARD_STATE_ENUM, eMinorError,   ::CardStateToString(eMinorError));
	CStringsMaps::AddItem(CARD_STATE_ENUM, eSimulation,   ::CardStateToString(eSimulation));
	CStringsMaps::AddItem(CARD_STATE_ENUM, eCardStartup,  ::CardStateToString(eCardStartup));
	CStringsMaps::AddItem(CARD_STATE_ENUM, eNoConnection, ::CardStateToString(eNoConnection));
	CStringsMaps::AddItem(CARD_STATE_ENUM, eDisabled,     ::CardStateToString(eDisabled));

	CStringsMaps::AddItem(UNIT_STATUS_ENUM, eOk,          ::CardUnitLoadedStatusToString(eOk));
	CStringsMaps::AddItem(UNIT_STATUS_ENUM, eUnitStartup, ::CardUnitLoadedStatusToString(eUnitStartup));
	CStringsMaps::AddItem(UNIT_STATUS_ENUM, eUnknown,     ::CardUnitLoadedStatusToString(eUnknown));
	CStringsMaps::AddItem(UNIT_STATUS_ENUM, eFatal,       ::CardUnitLoadedStatusToString(eFatal));
	CStringsMaps::AddItem(UNIT_STATUS_ENUM, eReady,       ::CardUnitLoadedStatusToString(eReady));
	CStringsMaps::AddItem(UNIT_STATUS_ENUM, eNotExist,    ::CardUnitLoadedStatusToString(eNotExist));
	CStringsMaps::AddItem(UNIT_STATUS_ENUM, eNotLoaded,	  ::CardUnitLoadedStatusToString(eNotLoaded));

	CStringsMaps::AddItem(MEDIA_IP_CONFIG_STATUS_ENUM, eMediaIpConfig_Ok,           ::CardMediaIpConfigStatusToString(eMediaIpConfig_Ok));
	CStringsMaps::AddItem(MEDIA_IP_CONFIG_STATUS_ENUM, eMediaIpConfig_NotSupported, ::CardMediaIpConfigStatusToString(eMediaIpConfig_NotSupported));
	CStringsMaps::AddItem(MEDIA_IP_CONFIG_STATUS_ENUM, eMediaIpConfig_NotExist,     ::CardMediaIpConfigStatusToString(eMediaIpConfig_NotExist));
	CStringsMaps::AddItem(MEDIA_IP_CONFIG_STATUS_ENUM, eMediaIpConfig_IpFail,       ::CardMediaIpConfigStatusToString(eMediaIpConfig_IpFail));
	CStringsMaps::AddItem(MEDIA_IP_CONFIG_STATUS_ENUM, eMediaIpConfig_IpDuplicate,  ::CardMediaIpConfigStatusToString(eMediaIpConfig_IpDuplicate));
	CStringsMaps::AddItem(MEDIA_IP_CONFIG_STATUS_ENUM, eMediaIpConfig_DhcpFail,     ::CardMediaIpConfigStatusToString(eMediaIpConfig_DhcpFail));
	CStringsMaps::AddItem(MEDIA_IP_CONFIG_STATUS_ENUM, eMediaIpConfig_VLanFail,     ::CardMediaIpConfigStatusToString(eMediaIpConfig_VLanFail));
}

//////////////////////////////////////////////////////////////////////
void CCardsProcess::PrintIvrAddMusicSourceDataToTrace(SIVRAddMusicSource *ivrAddMusicSource, eLogLevel traceLevel)
{
	CLargeString *pStr = new CLargeString;

	*pStr << "\nCCardsProcess::PrintIvrAddMusicSourceDataToTrace\n"
		  <<   "================================================\n\n"
		  << "Source Id: "             << ivrAddMusicSource->sourceID << "\n"
		  << "Number of Media Files: " << ivrAddMusicSource->mediaFilesNum << "\n";


	if ( MAX_MUSIC_SOURCE_FILES < ivrAddMusicSource->mediaFilesNum)
	{
		*pStr << "Wrong Parameter!! - Illegal Number of Media Files" << "\n";
	}

	else
	{
		DWORD i=0;
		for (i=0; i < ivrAddMusicSource->mediaFilesNum; i++)
		{
			*pStr << "Media file " << i << ": "
				  << ivrAddMusicSource->mediaFiles[i] << "\n";
		}
	}


	TRACESTR(traceLevel) << pStr->GetString();

	POBJDELETE(pStr);
}
/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::PrintFolderMountDataToTrace(CM_FOLDER_MOUNT_S pFolderMount)
{
	CMedString str = "\nCCardsProcess::PrintFolderMountDataToTrace\n";
	str <<             "==========================================\n";

	char ServerIpAddressStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(pFolderMount.ulServerIpAddrss, ServerIpAddressStr);
	
	str << "Status: ";
	if (STATUS_OK == pFolderMount.ulMounted)
	{
		str << "Ok";
	}
	else
	{
		str << "Fail";
	}
	
	str << "; Return value: "   << pFolderMount.ulReason
	    << "\nServer address: " << ServerIpAddressStr
	    << "; Folder path: "    << (char*)pFolderMount.path;
	
	TRACESTR(eLevelInfoNormal) << str.GetString();
}
////////////////////////////////////////////////////////////////////////////
EArtFips140SimulationErrCode  CCardsProcess::TranslateSysConfigDataToEnumForArt(std::string & data)
{
	EArtFips140SimulationErrCode eSimValue = E_ART_FIPS140_SIMULATION_ERR_CODE_INACTIVE;

	if(data == "FAIL_DETERMINISTIC_TEST")
		eSimValue = E_ART_FIPS140_SIMULATION_ERR_CODE_FAIL_DETERMINISTIC_TEST;

	return eSimValue;
}

void CCardsProcess::SetAuthenticationStruct(const MCMS_AUTHENTICATION_S authentStruct)
{
	memcpy( &m_pLicensingStruct->authenticationStruct, &authentStruct, sizeof(MCMS_AUTHENTICATION_S) );
}

MCMS_AUTHENTICATION_S* CCardsProcess::GetAuthenticationStruct()
{
	return &m_pLicensingStruct->authenticationStruct;
}
////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetLicensingStruct(CARDS_LICENSING_S* LicensingStruct)
{
	m_pLicensingStruct = m_pLicensingStruct;
}
////////////////////////////////////////////////////////////////////////////
CARDS_LICENSING_S*  CCardsProcess::GetLicensingStruct()
{
	return m_pLicensingStruct;
}
////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetIsAuthenticationSucceeded(const BOOL isSucceeded)
{
	m_isAuthenticationSucceeded = isSucceeded;
}

////////////////////////////////////////////////////////////////////////////
BOOL CCardsProcess::GetIsAuthenticationSucceeded() const
{
	return m_isAuthenticationSucceeded;
}

////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetNumOfBoards()
{
	switch (m_pLicensingStruct->authenticationStruct.productType)
	{
		case eProductTypeRMX2000:
		case eProductTypeRMX4000:
		case eProductTypeCallGenerator:
		case eProductTypeRMX1500:
		case eProductTypeSoftMCU:
		case eProductTypeGesher:
		case eProductTypeNinja:
		case eProductTypeSoftMCUMfw:
		case eProductTypeEdgeAxis:
		{
			m_numOfBoards = MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS;
			break;
		}
		
		default:
			m_numOfBoards = 0;
	}
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardsProcess::GetNumOfBoards() const
{
	return m_numOfBoards;
}

////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetNumOfBoardsExpected(const int numOfBoards)
{
	m_numOfBoardsExpected = numOfBoards;
}

////////////////////////////////////////////////////////////////////////////
int CCardsProcess::GetNumOfBoardsExpected() const
{
	return m_numOfBoardsExpected;
}

////////////////////////////////////////////////////////////////////////////
void CCardsProcess::IncreaseNumOfBoardsExpected()
{
	m_numOfBoardsExpected++;
	
	if (MAX_NUM_OF_BOARDS < m_numOfBoardsExpected)
	{
		PASSERT(m_numOfBoardsExpected);
		m_numOfBoardsExpected = MAX_NUM_OF_BOARDS;
	}
}

////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetIsToTreatRtmIsdnKeepAlive(const BOOL isToTreat)
{
	m_isToTreatRtmIsdnKeepAlive = isToTreat;
}

////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::SetSystemCardsModeCur(const string &theModeStr)
{
	if (SYSTEM_CARDS_MODE_MPMX_VAL == theModeStr)
		m_systemCardsModeCur = eSystemCardsMode_breeze;

	else if (SYSTEM_CARDS_MODE_MPMRX_VAL == theModeStr)
		m_systemCardsModeCur = eSystemCardsMode_mpmrx;
	else
	{
		SetSystemCardsModeCur(SYSTEM_CARDS_MODE_DEFAULT_VAL);
		TRACEINTO << "CCardsProcess::SetSystemCardsModeCur wrong cards mode string: " << theModeStr; 	
		return false;
	}

	TRACEINTO << "CCardsProcess::SetSystemCardsModeCur  cards mode string: " << theModeStr;
	return true;
}

////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetSystemCardsModeCur(const eSystemCardsMode theMode)
{
    TRACEINTO << "CCardsProcess::SetSystemCardsModeCur  new cards mode string: " << theMode;

	m_systemCardsModeCur = theMode;
}

////////////////////////////////////////////////////////////////////////////
eSystemCardsMode CCardsProcess::GetSystemCardsModeCur() const
{
	return m_systemCardsModeCur;
}

////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetSystemCardsModeFromFile(const string &theModeStr)
{

    TRACEINTO << "\nCCardsProcess::SetSystemCardsModeFromFile - theModeStr " << theModeStr;
	if (SYSTEM_CARDS_MODE_MPMX_VAL == theModeStr)
	    m_systemCardsModeFromFile = eSystemCardsMode_breeze;

	else if (SYSTEM_CARDS_MODE_MPMRX_VAL == theModeStr)
	    m_systemCardsModeFromFile = eSystemCardsMode_mpmrx;

	else
	    m_systemCardsModeFromFile = eSystemCardsMode_illegal;

	   TRACEINTO << "\nCCardsProcess::SetSystemCardsModeFromFile - m_systemCardsModeFromFile " << m_systemCardsModeFromFile;

}

////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetNGBSystemCardsMode(const eNGBSystemCardsMode theMode)
{
    m_NGBSystemCardsMode = theMode;
}

////////////////////////////////////////////////////////////////////////////
eNGBSystemCardsMode CCardsProcess::GetNGBSystemCardsMode() const
{
    return m_NGBSystemCardsMode;
}

////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetSystemCardsModeFromFile(const eSystemCardsMode theMode)
{
	m_systemCardsModeFromFile = theMode;
	 TRACEINTO << "\nCCardsProcess::SetSystemCardsModeFromFile - m_systemCardsModeFromFile " << m_systemCardsModeFromFile;

}

////////////////////////////////////////////////////////////////////////////
eSystemCardsMode CCardsProcess::GetSystemCardsModeFromFile() const
{
    TRACEINTO << "\nCCardsProcess::GetSystemCardsModeFromFile - m_systemCardsModeFromFile " << m_systemCardsModeFromFile;

	return m_systemCardsModeFromFile;
}

////////////////////////////////////////////////////////////////////////////
BOOL CCardsProcess::GetIsToTreatRtmIsdnKeepAlive() const
{
	return m_isToTreatRtmIsdnKeepAlive;
}


////////////////////////////////////////////////////////////////////////////
BOOL CCardsProcess::GetIsDebugMode() const
{
	BOOL isDebugMode = FALSE;
	GetSysConfig()->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, isDebugMode);
	return isDebugMode;
}

////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetStartupCond(eCardsStartupCondType condType, BYTE value)
{
	if(condType < NumOfCardsStartupCondTypes)
	{
		m_StartupEndCondArray[condType] = value;
	}
	else
	{
		string buff = "Illegal index : ";
		buff += condType;
		PASSERTMSG(1, buff.c_str());
	}
}

////////////////////////////////////////////////////////////////////////////
/*
STATUS CCardsProcess::IsStartupOk()
{	
	STATUS status = (	TRUE == m_StartupEndCondArray[eCardsIpService] && 
						TRUE == m_StartupEndCondArray[eCardsIvr]
						?
						STATUS_OK : STATUS_FAIL);	
	return status;
}
*/

////////////////////////////////////////////////////////////////////////////
STATUS CCardsProcess::IsCondOk(eCardsStartupCondType condType)
{
	STATUS status = STATUS_FAIL;
	if(condType < NumOfCardsStartupCondTypes)
	{
		status = (TRUE == m_StartupEndCondArray[condType] ? STATUS_OK : status);
	}
	else
	{
		string buff = "Illegal index : ";
		buff += condType;
		PASSERTMSG(1, buff.c_str());
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
eCardState CCardsProcess::ConvertTaskStatusToCardState(eTaskStatus taskStatus, eTaskState taskState)
{
	eCardState cardState = eNormal;

// Added by Yuri R.
	if(eTaskStateIdle == taskState)
	{
		cardState = eMajorError;
	}
	else if(eTaskStateStartup == taskState)
	{
		cardState = eCardStartup;
	}
	else
	{
		switch(taskStatus)
		{
			case eTaskNormal:
					cardState = eNormal;
					break;
		
		//		case eTaskIdle:
			case eTaskMajor:
			case eTaskInvalid:
					cardState = eMajorError;
					break;
		
			case eTaskMinor:
					cardState = eMinorError;
					break;
		
		//		case eTaskStartUp:
		//				cardState = eCardStartup;
		//				break;
		
			default:
					cardState = eNormal;
		}
	}
	return cardState;
}


eCardState CCardsProcess::ConvertTaskStatusToCardState(eTaskStatus taskStatus, eTaskState taskState,bool isFault)
{
	eCardState cardState = eNormal;

// Added by Yuri R.
	if(eTaskStateIdle == taskState)
	{
		cardState = eMajorError;
	}
	else if(eTaskStateStartup == taskState)
	{
		cardState = eCardStartup;
	}
	else
	{
		switch(taskStatus)
		{
			case eTaskNormal:
					cardState = eNormal;
					break;

		//		case eTaskIdle:
			case eTaskMajor:
			case eTaskInvalid:
				  if (isFault)
					  cardState = eMajorError;
				  else
					  cardState = eNormal;
					break;

			case eTaskMinor:
				  if (isFault)
					cardState = eMinorError;
				  else
					  cardState = eNormal;
					break;

		//		case eTaskStartUp:
		//				cardState = eCardStartup;
		//				break;

			default:
					cardState = eNormal;
		}
	}
	return cardState;
}


//////////////////////////////////////////////////////////////////////
void CCardsProcess::SendResetReqToDaemon(CSmallString errStr)
{
    const char *desc = errStr.GetString();
    
	TRACEINTO << "\nCCardsProcess::SendResetReqToDaemon - " << desc; 

    CMcmsDaemonApi api;
    STATUS res = api.SendResetReq(desc);
}

//////////////////////////////////////////////////////////////////////
void CCardsProcess::ActiveAlarmsStructAsString( ACTIVE_ALARMS_SPECIFIC_CARD_S* pAaStruct,
                                                CLargeString &AaStructStr,
                                                DWORD boardId)
{
	AaStructStr << "\nActiveAlarms for card on boardId " << boardId << ":";
	
	if (YES == pAaStruct->isVoltageProblem)
	{
		AaStructStr << "\nVoltage: YES";
	}
	else
	{
		AaStructStr << "\nVoltage: NO";
	}
	
	if (YES == pAaStruct->isTemperatureMajorProblem)
	{
		AaStructStr << "\nTemperature Major: YES";
	}
	else
	{
		AaStructStr << "\nTemperature Major: NO";
	}

	if (YES == pAaStruct->isTemperatureCriticalProblem)
	{
		AaStructStr << "\nTemperature Critical: YES";
	}
	else
	{
		AaStructStr << "\nTemperature Critical: NO";
	}
    if (YES == pAaStruct->isPowerOffProblem)
	{
		AaStructStr << "\nPower Off: YES";
	}
	else
	{
		AaStructStr << "\nPower Off: NO";
	}
    if (YES == pAaStruct->isRtmIsdnMissingProblem)
    {
        AaStructStr << "\nRtm card missing: YES";
    }
    else
    {
    	AaStructStr << "\nRtm card missing: NO";
    }
	if (YES == pAaStruct->isFailureProblem)
	{
		AaStructStr << "\nFailure: YES";
	}
	else
	{
		AaStructStr << "\nFailure: NO";
	}

	if (YES == pAaStruct->isOtherProblem)
	{
		AaStructStr << "\nOther: YES";
	}
	else
	{
		AaStructStr << "\nOther: NO";
	}
}

//////////////////////////////////////////////////////////////////////
void CCardsProcess::PrintLastKeepAliveTimes( DWORD boardId,
                                             CStructTm lastKeepAliveReqTime,
                                             CStructTm lastKeepAliveIndTime )
{
	CSmallString headline;
	headline	<< "\nCCardsProcess::PrintLastKeepAliveTimes (boardId " << boardId << ") -";
	
	COstrStream lastReqTimeStr, lastIndTimeStr;
	lastKeepAliveReqTime.Serialize(lastReqTimeStr);
	lastKeepAliveIndTime.Serialize(lastIndTimeStr);

	string kaStr = headline.GetString();
	kaStr += "\nLast KeepAliveReq was sent at     ";
	kaStr += lastReqTimeStr.str();
	kaStr += "\nLast KeepAliveInd was received at ";
	kaStr += lastIndTimeStr.str();


// temp: not printing
	TRACESTR(eLevelInfoNormal) << kaStr.c_str();
}


//////////////////////////////////////////////////////////////////////
STATUS CCardsProcess::AddRtmIsdnParamsToList(const RTM_ISDN_PARAMS_MCMS_S &theStruct)
{
	STATUS retStat = STATUS_OK; 

	if (MAX_ISDN_SERVICES_IN_LIST <= m_rtmIsdnParamsIdx)
		return STATUS_NUMBER_OF_ISDN_SERVICES_EXCEEDED;
	
	if ( NOT_FIND != FindRtmIsdnService((char*)(theStruct.serviceName)) )
		return STATUS_SERVICE_PROVIDER_NAME_EXISTS;  

	m_pRtmIsdnParamsList[m_rtmIsdnParamsIdx] = new RTM_ISDN_PARAMS_MCMS_S;
	WORD structLen = sizeof(RTM_ISDN_PARAMS_MCMS_S);
	memcpy(m_pRtmIsdnParamsList[m_rtmIsdnParamsIdx], &theStruct, structLen);

	// print
	TRACEINTO << "\nCCardsProcess::AddRtmIsdnParamsToList"; 
	m_rtmIsdnCommonMethods.PrintRtmIsdnParamsMcmsStruct( *(m_pRtmIsdnParamsList[m_rtmIsdnParamsIdx]), "CCardsProcess::AddRtmIsdnParamsToList" );
	
	m_rtmIsdnParamsIdx++;
	
	return retStat;
}

//////////////////////////////////////////////////////////////////////
STATUS CCardsProcess::CancelRtmIsdnParams(const RTM_ISDN_SERVICE_NAME_S &theStruct)
{
	STATUS retStat = STATUS_OK; 

	// ===== 1. print
	TRACEINTO << "\nCCardsProcess::CancelRtmIsdnParams"; 
	m_rtmIsdnCommonMethods.PrintRtmIsdnServiceNameStruct(theStruct, "CCardsProcess::CancelRtmIsdnParams");


	// ===== 2. delete
	int ind = FindRtmIsdnService((char*)(theStruct.serviceName));
	if (NOT_FIND == ind)
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;  

	POBJDELETE(m_pRtmIsdnParamsList[ind]);
	
	int i=0, j=0;
	for (i=0; i<(int)m_rtmIsdnParamsIdx; i++)
	{
		if (m_pRtmIsdnParamsList[i]==NULL)
			break; 
	}    
	for (j=i; j<(int)m_rtmIsdnParamsIdx-1; j++)
	{
		m_pRtmIsdnParamsList[j] = m_pRtmIsdnParamsList[j+1] ;
	}

    m_pRtmIsdnParamsList[m_rtmIsdnParamsIdx - 1] = NULL;
	
	m_rtmIsdnParamsIdx--;
	if (0 > m_rtmIsdnParamsIdx)
		m_rtmIsdnParamsIdx=0;


	return retStat;
}

//////////////////////////////////////////////////////////////////////
STATUS CCardsProcess::DetachAllSpansInList(const char* serviceName)
{
	TRACEINTO << "\nCCardsProcess::DetachAllSpansInList";
	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansMapsListStruct(*m_pRtmIsdnSpanMapsList, "CCardsProcess::DetachAllSpansInList - before detaching");

	for (int i=0; i<MAX_ISDN_SPAN_MAPS_IN_LIST; i++)
	{
		RTM_ISDN_SPAN_MAP_S curSpan = m_pRtmIsdnSpanMapsList->spanMap[i];
		
		string servNameInSpan = (char*)(m_pRtmIsdnSpanMapsList->spanMap[i].serviceName);
		if ( servNameInSpan == serviceName )
		{
			memset( m_pRtmIsdnSpanMapsList->spanMap[i].serviceName, 0, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN );
		}
	}

	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansMapsListStruct(*m_pRtmIsdnSpanMapsList, "CCardsProcess::DetachAllSpansInList - after detaching");
	
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CCardsProcess::UpdateSpanMapsList(const RTM_ISDN_SPAN_MAPS_LIST_S &theStruct)
{
	STATUS retStat = STATUS_OK;

	POBJDELETE(m_pRtmIsdnSpanMapsList);

	m_pRtmIsdnSpanMapsList = new RTM_ISDN_SPAN_MAPS_LIST_S;
	WORD structLen = sizeof(RTM_ISDN_SPAN_MAPS_LIST_S);
	memcpy(m_pRtmIsdnSpanMapsList, &theStruct, structLen);

	// print
	TRACEINTO << "\nCCardsProcess::UpdateSpanMapsList";
	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansMapsListStruct(*m_pRtmIsdnSpanMapsList, "CCardsProcess::AddSpanMapsList");
	
	return retStat;
}

//////////////////////////////////////////////////////////////////////
STATUS CCardsProcess::UpdateSpecSpanMapInList(const RTM_ISDN_SPAN_MAP_S &theStruct)
{
	STATUS retStat = STATUS_SPAN_MAP_NOT_EXISTS;

	for (int i=0; i<MAX_ISDN_SPAN_MAPS_IN_LIST; i++)
	{
		RTM_ISDN_SPAN_MAP_S curSpan = m_pRtmIsdnSpanMapsList->spanMap[i];

		if ( (curSpan.boardId == theStruct.boardId) && (curSpan.spanId  == theStruct.spanId) )
		{
			memcpy( m_pRtmIsdnSpanMapsList->spanMap[i].serviceName,
			        theStruct.serviceName,
			        RTM_ISDN_SERVICE_PROVIDER_NAME_LEN );

			retStat = STATUS_OK;
		}
	}

	// print
	TRACEINTO << "\nCCardsProcess::UpdateSpecSpanMapInList "
	          << "\nBoardId: " << theStruct.boardId << ", spanId: " << theStruct.spanId << " - was updated";

	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansMapsListStruct(*m_pRtmIsdnSpanMapsList, "CCardsProcess::UpdateSpecSpanMapInList");
	
	return retStat;
}

//////////////////////////////////////////////////////////////////////
STATUS CCardsProcess::DetachSpecSpanMapInList(const SPAN_DISABLE_S &theStruct)
{
	STATUS retStat = STATUS_SPAN_MAP_NOT_EXISTS;

	for (int i=0; i<MAX_ISDN_SPAN_MAPS_IN_LIST; i++)
	{
		RTM_ISDN_SPAN_MAP_S curSpan = m_pRtmIsdnSpanMapsList->spanMap[i];

		if ( (curSpan.boardId == theStruct.boardId) && (curSpan.spanId  == theStruct.spanId) )
		{
			memset( m_pRtmIsdnSpanMapsList->spanMap[i].serviceName, 0, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN );
			retStat = STATUS_OK;
		}
	}

	// print
	TRACEINTO << "\nCCardsProcess::DetachSpecSpanMapInList "
	          << "\nBoardId: " << theStruct.boardId << ", spanId: " << theStruct.spanId << " - was updated";

	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansMapsListStruct(*m_pRtmIsdnSpanMapsList, "CCardsProcess::DetachSpecSpanMapInList");
	
	return retStat;
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::FindRtmIsdnService(const char* name)
{
	string sName = name;

	for (int i=0; i<(int)MAX_ISDN_SERVICES_IN_LIST; i++)
	{
		if (NULL != m_pRtmIsdnParamsList[i])
		{
			string sCurName = (char*)(m_pRtmIsdnParamsList[i]->serviceName);

			if (sName == sCurName)
				return i;
		} 
	}

	return NOT_FIND; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const RTM_ISDN_PARAMS_MCMS_S* CCardsProcess::GetRtmIsdnParamsStruct(int idx) const
{
	return m_pRtmIsdnParamsList[idx];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const RTM_ISDN_SPAN_MAPS_LIST_S* CCardsProcess::GetRtmIsdnSpanMapsList() const
{
	return m_pRtmIsdnSpanMapsList;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const RTM_ISDN_SPAN_MAP_S CCardsProcess::GetRtmIsdnSpanMap(int idx) const
{
	return m_pRtmIsdnSpanMapsList->spanMap[idx];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SendSnmpMfaInterface()
{
    SNMP_CARDS_INFO_S param;
    FillSnmpMfaParam(param);
    
    CSegment *pSeg = new CSegment;
    pSeg->Put((BYTE*)&param, sizeof(SNMP_CARDS_INFO_S));

    CManagerApi apiSnmp(eProcessSNMPProcess);
    apiSnmp.SendMsg(pSeg, SNMP_MFA_INTERFACE_IP_IND);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsRtmIpAddress(DWORD ipAddrrToTest)
{
    DWORD rtmMediaIp1 = SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_1),
    	  rtmMediaIp2 = SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_2),
    	  rtmMediaIp3 = SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_3),
    	  rtmMediaIp4 = SystemIpStringToDWORD(RTM_ISDN_MEDIA_IP_BOARD_4);

//    DWORD ipAddressLittleEndian = ntohl(ipAddrrToTest); // convert to LittleEndian (since the addresses are sent to Mcms in that format)
//    return ( (ipAddressLittleEndian == rtmMediaIp1) || (ipAddressLittleEndian == rtmMediaIp2));
    
    bool isRtmIpAddres = ( (ipAddrrToTest == rtmMediaIp1) ||
    					   (ipAddrrToTest == rtmMediaIp2) ||
    					   (ipAddrrToTest == rtmMediaIp3) ||
    					   (ipAddrrToTest == rtmMediaIp4) );
    	
    return isRtmIpAddres;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsProcess::FillSnmpMfaParam(SNMP_CARDS_INFO_S &outParam)
{
    DWORD mfaIndex = 0;
    
    LockSemaphore(m_TasksSemaphoreId);
    {
        for( std::vector<CTaskApp*>::iterator itr = m_pTasks->begin() ;
             itr != m_pTasks->end() ;
             itr++ )
        {
            CTaskApp *ptr = *itr;
            CMfaTask *pMfaTask = dynamic_cast<CMfaTask *>(ptr);
            if(NULL != pMfaTask)
            {
                CMediaIpConfigVector *pIpMediaConfigVector = pMfaTask->GetMediaIpConfigVector();
                if(NULL == pIpMediaConfigVector)
                {
                    PASSERTMSG(TRUE, "MFa task does not contain IpMediaConfigVector");
                    continue;
                }
                
                CMediaIpConfig *pMediaConfig = pIpMediaConfigVector->GetFirst();
                while(NULL != pMediaConfig)
                {
                    DWORD currentAddr = pMediaConfig->GetIpV4Address();
                    if(!IsRtmIpAddress(currentAddr))
                    {                    	
                        outParam.mfaLinks[mfaIndex].ipAddress = currentAddr;
                        outParam.mfaLinks[mfaIndex].boardId =
                                        pMfaTask->GetBoardId();
                        outParam.mfaLinks[mfaIndex].portId =  pMediaConfig->GetPqNumber();
                        mfaIndex++;
                        break;
                    }
                    
                    pMediaConfig = pIpMediaConfigVector->GetNext();
                }
            }
        }
    }    
    UnlockSemaphore(m_TasksSemaphoreId);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SendSnmpLinkStatus(DWORD boardId, WORD portId, BOOL isUp) const
{
//	TRACESTR(eLevelInfoNormal) << "\nSendSnmpLinkStatus " << boardId << " port " << portId << " isUp " << (int) isUp;
			
	LINK_STATUS_S linkStatus;
	linkStatus.boardId = boardId;
	linkStatus.portId = portId;
	linkStatus.isUp = isUp;
	
	
	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)&linkStatus, sizeof(LINK_STATUS_S));
	
	CManagerApi apiSnmp(eProcessSNMPProcess);
	apiSnmp.SendMsg(pSeg, SNMP_MFA_LINK_STATUS_IND);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCardsProcess::RemoveCard (DWORD boardId, DWORD subBoardId, eCardType cardType)
{
	if (eSwitch != cardType)
	{
	    SendCardRemoveIndToRsrcalloc(boardId,subBoardId);
	
	    DWORD mfaSubBoardId = FIRST_SUBBOARD_ID;
	   // if (2 == subBoardId)
	    if (mfaSubBoardId == subBoardId)
	                                                  // if it's MFA that was removed...
	        SendHotSwapCardRemovedToRtmTask(boardId);	//  ...then remove also its RTMs


	    //it should be checked only for MS
		for (int i=0; i<= MAX_NUM_OF_IP_SERVICES; i++)
		{
			if (m_CSInternalExtMsgsStatus[i].boardId == boardId )
			{
				//m_CSInternalExtMsgsStatus[i].ExtMsgArrived = false;
				//m_CSInternalExtMsgsStatus[i].IntMsgArrived = false;
				//m_CSInternalExtMsgsStatus[i].boardId = 0;
				//m_CSInternalExtMsgsStatus[i].state = 0;

				memset(&m_CSInternalExtMsgsStatus[i], 0, sizeof(CS_EXT_INT_MSG_CARD_PARAMS_S));
			}


		}


	}
	else
		/**********************************************************************************/
		/* 20.9.10 Rachel Cohen                                                           */
		/* In case switch resets itself and send MNGR_LOADED again, m_numOfBoardsExpected */
		/* reachs its max value and user see endless ASSERTs on it .                      */
		/**********************************************************************************/
		m_numOfBoardsExpected = 0;

	//VNGR-17552 17.10.10
	DecearseBoardInstallingIpmc(boardId);
    SendHotSwapCardRemovedToCardMngr(boardId, subBoardId, cardType);
    //SendSnmpMfaInterface();

}
//////////////////////////////////////////////////////////////////////
void CCardsProcess::SendCardRemoveIndToRsrcalloc(DWORD boardId,DWORD subBoardId)	
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::SendCardRemoveIndToRsrcalloc "
	                       << "(BoardId: " << boardId << ", SubBoardId: " << subBoardId << ")";

    CARD_REMOVED_IND_S* pRemParamStruct = new CARD_REMOVED_IND_S;

	pRemParamStruct->BoardID	= (WORD)boardId;
	pRemParamStruct->SubBoardID	= (WORD)subBoardId;

	// ===== fill the Segment 
	CSegment*  pRemParam = new CSegment;
	pRemParam->Put( (BYTE*)pRemParamStruct, sizeof(CARD_REMOVED_IND_S) );

	delete pRemParamStruct;

	
	// ===== 3. send
	const COsQueue* pResourceMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResource, eManager);

	//STATUS res = pResourceMbx->Send(pRemParam, CARD_REMOVED_IND, &GetRcvMbx());
	STATUS res = pResourceMbx->Send(pRemParam, CARD_REMOVED_IND);
}

////////////////////////////////////////////////////////////////////
void CCardsProcess::SendHotSwapCardRemovedToRtmTask(DWORD boardId)	
	{
		///  i loop 0 to max sub boards,DWORD m_subBoardId
		for (int i=0; i<MAX_NUM_OF_SUBBOARDS;i++)
		{
			COsQueue* mfaMbx = GetMfaMbx(boardId, i);
			if (mfaMbx)
			{
				CTaskApi api;
				api.CreateOnlyApi(*mfaMbx);
				api.SendOpcodeMsg(HOT_SWAP_RTM_REMOVED);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SendHotSwapCardRemovedToCardMngr(DWORD boardId, DWORD subBoardId, eCardType cardType)
{
	TRACESTR(eLevelInfoNormal) << "CCardsProcess::SendHotSwapRtmRemovedToCardMngr";

	// ===== 1. insert switchBoardId to a segment
    CSegment* pRetParam = new CSegment;
    *pRetParam <<  boardId;
    *pRetParam <<  subBoardId;
    *pRetParam <<  (DWORD)cardType;
    

	// ===== 2. send to CardsMngr
	const COsQueue* pCardsMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);

	STATUS res = pCardsMngrMbx->Send(pRetParam, CARD_REMOVED_IND);

}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsMpmCard(const eCardType cardType)
{
	bool isMfa = false;
	
	if ( (eMfa_26 == cardType) ||
		 (eMfa_13 == cardType) )
	{
		isMfa = true;
	}
	
	return isMfa;
}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsMpmPlusCard(const eCardType cardType)
{
	bool isBarak = false;
	
	if ( (eMpmPlus_20			== cardType) ||
		 (eMpmPlus_40			== cardType) ||
		 (eMpmPlus_80			== cardType) ||
		 (eMpmPlus_MezzanineA	== cardType) ||
		 (eMpmPlus_MezzanineB	== cardType) // ||(eBreeze	== cardType)
		 //tbd by someone in cards: see if to add a special function for breeze or if everywhere it's OK to treat breeze and barak the same
		 )
	{
		isBarak = true;
	}
	
	return isBarak;
}
/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsBreezeCard(const eCardType cardType)
{
    if((eMpmx_80 == cardType) || (eMpmx_40 == cardType) || (eMpmx_20 == cardType))
	    return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsMpmRXCard(const eCardType cardType)
{
    if((eMpmRx_Half == cardType) || (eMpmRx_Full == cardType) )
        return true;
    return false;
}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsSoftCard(const eCardType cardType) //OLGA - SoftMCU
{
    if((eMpmx_Soft_Half == cardType) || (eMpmx_Soft_Full == cardType))
        return true;
    return false;
}

//////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsMediaCard(const eCardType cardType)
{
	bool isMfa        = false,
         isBarak      = false,
	     isBreeze     = false,
		 isMpmxRx     = false,
	     isSoft       = false;

    isMfa   = IsMpmCard(cardType);
    isBarak = IsMpmPlusCard(cardType);
	isBreeze = IsBreezeCard(cardType);
	isMpmxRx = IsMpmRXCard(cardType);
	isSoft = IsSoftCard(cardType); //OLGA - SoftMCU

	if ( (true == isMfa) || (true == isBarak) || (true == isBreeze) || (true == isMpmxRx) || (true == isSoft) )
	{
		return true;
	}

	return false;
}


/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsRtmIsdnCard(const eCardType cardType)
{
	bool isRtmIsdn = false;

	if (eRtmIsdn == cardType)
	{
		isRtmIsdn = true;
	}

	return isRtmIsdn;
}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsRtmIsdnCardExistInDB()
{
    bool isExists = false;

	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			eCardType curCardType = m_pCardsMonitoringDB->GetCardType(i, j);
            bool curCardIsMpm  = IsRtmIsdnCard(curCardType);

            if (true == curCardIsMpm)
            {
                isExists = true;
                break;
            }
		}
	} // end loop over m_pCard

    return isExists;
}


/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsRtmIsdnCard(int boardId,int SubBoardId)
{
	bool curCardIsRtmIsdn = false;

	eCardType curCardType = m_pCardsMonitoringDB->GetCardType(boardId, SubBoardId);
	curCardIsRtmIsdn = IsRtmIsdnCard(curCardType);



	return curCardIsRtmIsdn;
}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsMpmCardExistInDB()
{
    bool isExists = false;
    
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			eCardType curCardType = m_pCardsMonitoringDB->GetCardType(i, j);
            bool curCardIsMpm  = IsMpmCard(curCardType);

            if (true == curCardIsMpm)
            {
                isExists = true;
                break;
            }
		}
	} // end loop over m_pCard

    return isExists;
}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsMpmPlusCardExistInDB()
{
    bool isExists = false;
    
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			eCardType curCardType = m_pCardsMonitoringDB->GetCardType(i, j);
            bool curCardIsMpmPlus  = IsMpmPlusCard(curCardType);

            if (true == curCardIsMpmPlus)
            {
                isExists = true;
                break;
            }
		}
	} // end loop over m_pCard

    return isExists;
}
/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsBreezeCardExistInDB()
{
    bool isExists = false;

	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			eCardType curCardType = m_pCardsMonitoringDB->GetCardType(i, j);
            bool curCardIsBreeze  = IsBreezeCard(curCardType);
            bool curCardIsSoft  = IsSoftCard(curCardType);

            if (true == curCardIsBreeze || curCardIsSoft ) //OLGA - SoftMCU
            {
                isExists = true;
                break;
            }
		}
	} // end loop over m_pCard

    return isExists;
}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsMPMRXCardExistInDB()
{
    bool isExists = false;

    for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
    {
        for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
        {
            eCardType curCardType = m_pCardsMonitoringDB->GetCardType(i, j);
            bool curCardIsMpmRx  = IsMpmRXCard(curCardType);

            if (true == curCardIsMpmRx)
            {
                isExists = true;
                break;
            }
        }
    } // end loop over m_pCard

    return isExists;
}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsNotMpmPlus80ExistInDB()
{
    bool isExists = false;

	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			eCardType curCardType = m_pCardsMonitoringDB->GetCardType(i, j);
			bool isMediaCard = IsMediaCard(curCardType);
            
			if ( (true == isMediaCard) && (eMpmPlus_80 != curCardType) )
            {
                isExists = true;
                break;
            }
		}
	} // end loop over m_pCard

    return isExists;
}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsAnyMediaCardExistsInDB()
{
    bool isExists = m_pCardsMonitoringDB->IsAnyMediaBoardExistsInDB();
    return isExists;
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::GetNumOfMpmBoards()
{
	bool isCurCardMfa = false;
	int numOfMFAs = 0;
    
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			eCardType curCardType = m_pCardsMonitoringDB->GetCardType(i, j);
			isCurCardMfa  = IsMpmCard(curCardType);

            if (true == isCurCardMfa)
            {
            	numOfMFAs++;
            }
		}
	} // end loop over m_pCard

    return numOfMFAs;
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::GetNumOfMpmPlusBoards()
{
	bool isCurCardBarak = false;
	int numOfBaraks = 0;
    
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			eCardType curCardType = m_pCardsMonitoringDB->GetCardType(i, j);
			isCurCardBarak  = IsMpmPlusCard(curCardType);

            if (true == isCurCardBarak)
            {
            	numOfBaraks++;
            }
		}
	} // end loop over m_pCard

    return numOfBaraks;
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::GetNumOfBreezeBoards()
{
	bool isCurCardBreeze = false;
	int numOfBreeze = 0;

	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
	{
		for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
		{
			eCardType curCardType = m_pCardsMonitoringDB->GetCardType(i, j);
			isCurCardBreeze  = IsBreezeCard(curCardType);

            if (true == isCurCardBreeze)
            {
            	numOfBreeze++;
            }
		}
	} // end loop over m_pCard

    return numOfBreeze;
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::GetNumOfMPMRXBoards()
{
    bool isCurCardMPMRX = false;
    int numOfMPMRX = 0;

    for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
    {
        for (int j=0; j<MAX_NUM_OF_SUBBOARDS; j++)
        {
            eCardType curCardType = m_pCardsMonitoringDB->GetCardType(i, j);
            isCurCardMPMRX  = IsMpmRXCard(curCardType);

            if (true == isCurCardMPMRX)
            {
                numOfMPMRX++;
            }
        }
    } // end loop over m_pCard

    return numOfMPMRX;
}

/////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsSingleMediaBoardOnSecondSlot()
{
	bool isSingleBoardOnSecondSlot	= false;
	
	int  numOfBoardsOnSystem = m_pCardsMonitoringDB->GetNumOfMediaBoards();
	bool isSecondSlotEmpty = m_pCardsMonitoringDB->IsSlotEmpty(2/*boardId*/, 1/*subBoardId*/);  // (all hard coded since this method is for a very specific purpose)
	
	if ( (1 == numOfBoardsOnSystem) && (false == isSecondSlotEmpty) )
	{
		isSingleBoardOnSecondSlot = true;
	}
	
	TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::IsSingleMediaBoardOnSecondSlot"
	                       << "\nnumOfBoardsOnSystem: " << numOfBoardsOnSystem
	                       << "\nisSecondSlotEmpty:   " << isSecondSlotEmpty;

    return isSingleBoardOnSecondSlot;
}

//////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsCardEnabled_ProductType(eCardType curCardType)
{
    bool isEnabled = true;

    // MPM  is not enabled in RMX4000
    bool curCardIsMpm		= IsMpmCard(curCardType);
    eProductType curType	= GetProductType();

    if ( ((eProductTypeRMX4000 == curType) || (eProductTypeRMX1500 == curType)) && (true == curCardIsMpm) )
    {
    	isEnabled = false;
    }

    string isMpmStr		= ( (true == curCardIsMpm)	? "yes" : "no" );
    string isEnabledStr	= ( (true == isEnabled)		? "yes" : "no" );
    TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::IsCardEnabled_ProductType"
                           << "\nProduct type:    " << ::ProductTypeToString(curType)
                           << "\nIs MPM  card:    " << isMpmStr
                           << "\nIs card enabled: " << isEnabledStr;

    return isEnabled;
}

//////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsCardEnabled_SystemCardsMode(eCardType curCardType)
{
    bool isEnabled = true;

    // MPM  is not enabled in MPM+ mode
    // MPM+ is not enabled in MPM  mode
    eSystemCardsMode curMode = GetSystemCardsModeCur();
    bool curCardIsMpm		= IsMpmCard(curCardType);
    bool curCardIsMpmPlus	= IsMpmPlusCard(curCardType);
	bool curCardIsBreeze    = IsBreezeCard(curCardType);
	bool curCardIsMpmRx      = IsMpmRXCard(curCardType);

    if ( ((eSystemCardsMode_mpm_plus == curMode) && (true == curCardIsMpm || true == curCardIsBreeze)) ||
    	 ((eSystemCardsMode_mpm		 == curMode) && (true == curCardIsMpmPlus || true == curCardIsBreeze)) ||
		 ((eSystemCardsMode_breeze 	 == curMode) && (true == curCardIsMpmPlus || true == curCardIsMpm)) ||
		 ((eSystemCardsMode_mpmrx    == curMode) && (true == curCardIsMpmPlus || true == curCardIsMpm)) )
    {
    	isEnabled = false;
    }

    string isMpmStr		= ( (true == curCardIsMpm)		? "yes" : "no" );
    string isMpmPlusStr	= ( (true == curCardIsMpmPlus)	? "yes" : "no" );
    string isBreezeStr	= ( (true == curCardIsBreeze)	? "yes" : "no" );
    string isMPMRXStr  = ( (true == curCardIsMpmRx)   ? "yes" : "no" );
    string isEnabledStr	= ( (true == isEnabled)			? "yes" : "no" );

    TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::IsCardEnabled_SystemCardsMode"
                           << "\nCurrent mode:    " << ::GetSystemCardsModeStr(curMode)
                           << "\nIs MPM  card:    " << isMpmStr
                           << "\nIs MPM+ card:    " << isMpmPlusStr
						   << "\nIs Breeze card: " << isBreezeStr
						   << "\nIs MPMRX card: " << isMPMRXStr
                           << "\nIs card enabled: " << isEnabledStr;

    return isEnabled;
}

//////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::IsCardEnabled_specRMX2000C(eCardType curCardType)
{
    bool isEnabled = true;

    // only MPM+80  is enabled in RMX2000C
    if (eMpmPlus_80 != curCardType)
    {
//    	isEnabled = false;
	    TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::IsCardEnabled_specRMX2000C"
	                           << "\nTemporarily enable all cards";
	}

    string isEnabledStr = ( (true == isEnabled) ? "yes" : "no" );
    TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::IsCardEnabled_specRMX2000C"
                           << "\nCard Type:    " << ::CardTypeToString(curCardType)
                           << "\nIs card enabled: " << isEnabledStr;

    return isEnabled;
}

DWORD CCardsProcess::CreateIdFromBoardIdMsgId(const int boardId, const int msgId, const int autoRemovalMode)
{
	// just to gnerate a unique number (as an Alert's identifier) from BoardId + msgId
	DWORD retId = (10000*boardId) + (100*msgId) + autoRemovalMode;
	return retId;
}

//////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetIsCardsModeEventAlertAlreadyProduced(const bool isProduced)
{
	m_isCardsModeEventAlertAlreadyProduced = isProduced;
}

//////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::GetIsCardsModeEventAlertAlreadyProduced() const
{
	return m_isCardsModeEventAlertAlreadyProduced;
}

//////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetIsIvrMusicAddSrcReqReceived(const bool isReceived)
{
	m_isIvrMusicAddSrcReqReceived = isReceived;
}

//////////////////////////////////////////////////////////////////////////////
bool CCardsProcess::GetIsIvrMusicAddSrcReqReceived() const
{
	return m_isIvrMusicAddSrcReqReceived;
}

/////////////////////////////////////////////////////////////////////////////
ePlatformType CCardsProcess::ConvertProductTypeToPlatformType(eProductType productType)
{
	ePlatformType platformType = eGideonLite;
	
 	switch(productType)
 	{
		case eProductTypeRMX2000:
 		case eProductTypeCallGenerator:
 		case eProductTypeSoftMCU:
 		case eProductTypeGesher:
 		case eProductTypeNinja:
 		case eProductTypeSoftMCUMfw:
 		case eProductTypeEdgeAxis:
 			platformType = eGideonLite;	// 'eGideonLite' is sent on RMX2000, therefore 'RMX 2000' should be returned
 			break;
				
 		case eProductTypeRMX4000:
 			platformType = eAmos;
 			break;

		case eProductTypeRMX1500:
 			platformType = eYona;
 			break;

 		default:
 			platformType = eGideonLite;
 	}
 	
 	return platformType;
}

/////////////////////////////////////////////////////////////////////////////
CEthernetSettingsStructWrappersList* CCardsProcess::GetEthernetSettingsStructsList()
{
	return m_pEthernetSettingsStructsList;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetCpuBoardIdSubBoardId(const int boardId, const int subBoardId)
{
	m_cpuBoardId	= boardId;
	m_cpuSubBoardId	= subBoardId;
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::GetCpuBoardId()
{
	return m_cpuBoardId;
}
bool CCardsProcess::IsCNRLPort(int portNum, eProductType curProductType)
{


	if ( ((curProductType == eProductTypeRMX4000) &&
			((portNum == ETH_SETTINGS_CPU_MNGMNT_1_PORT_ON_SWITCH_BOARD) ||
			 (portNum == ETH_SETTINGS_CPU_SGNLNG_1_PORT_ON_SWITCH_BOARD) ) )||

		((curProductType == eProductTypeRMX1500) &&
				((portNum == ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500) ||
				 ( portNum== ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500) ) ))

	     return true;

	return false;

}
/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::GetCpuSubBoardId()
{
	return m_cpuSubBoardId;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SendClearMaxCountersIndToSwitch(ETH_SETTINGS_STATE_S* pEthState)
{
	eEthSettingsState curState = (eEthSettingsState)(pEthState->configState);

	TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::SendClearMaxCountersIndToSwitch - params sent to CM:"
						   << "\nslotId:      " << (int)(pEthState->portParams.slotId)
						   << "\nportNum:     " << (int)(pEthState->portParams.portNum)
						   << "\nconfigState: " << (eEthSettingsState_ok == curState ? "ok" : "fail");

	BYTE switchBoardId    = GetAuthenticationStruct()->switchBoardId,
	     switchSubBoardId = GetAuthenticationStruct()->switchSubBoardId;

	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(ETHERNET_SETTINGS_CLEAR_MAX_COUNTERS_SWITCH_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, switchBoardId, switchSubBoardId);
	mplPrtcl->AddData(sizeof(ETH_SETTINGS_STATE_S), (char*)pEthState);

	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL_API");
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}

//////////////////////////////////////////////////////////////////////
void CCardsProcess::InitMediaBoardsIds(eProductType curProductType)
{
	if ( IsTarget() )
	{
		if (eProductTypeRMX4000 == curProductType)
		{
			m_1stMediaBoardId = FIXED_BOARD_ID_MEDIA_1;
			m_2ndMediaBoardId = FIXED_BOARD_ID_MEDIA_2;
			m_3rdMediaBoardId = FIXED_BOARD_ID_MEDIA_3;
			m_4thMediaBoardId = FIXED_BOARD_ID_MEDIA_4;
		}
		else
		{
			if (eProductTypeRMX1500 == curProductType)
			{
				  m_1stMediaBoardId = FIXED_BOARD_ID_MEDIA_1;
				  m_2ndMediaBoardId = 0; // irrelevant on RMX1500
				  m_3rdMediaBoardId = 0; // irrelevant on RMX1500
				  m_4thMediaBoardId = 0; // irrelevant on RMX1500
			}
		else // eProductTypeRMX2000, eProductTypeNPG2000, eProductTypeCallGenerator etc
		{
			m_1stMediaBoardId = FIXED_BOARD_ID_MEDIA_1;
			m_2ndMediaBoardId = FIXED_BOARD_ID_MEDIA_2;
			m_3rdMediaBoardId = 0; // irrelevant on RMX2000
			m_4thMediaBoardId = 0; // irrelevant on RMX2000
		}
	}
	}
	else
	{
		m_1stMediaBoardId = 1;
		m_2ndMediaBoardId = 2;
		m_3rdMediaBoardId = 3;
		m_4thMediaBoardId = 4;
	}
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::Get1stMediaBoardId()
{
	return m_1stMediaBoardId;
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::Get2ndMediaBoardId()
{
	return m_2ndMediaBoardId;
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::Get3rdMediaBoardId()
{
	return m_3rdMediaBoardId;
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::Get4thMediaBoardId()
{
	return m_4thMediaBoardId;
}


/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetSlotsNumberingConversionTable(const SLOTS_NUMBERING_CONVERSION_TABLE_S* pTable)
{
	m_pSlotsNumConversionTable->SetStruct(pTable);
}

/////////////////////////////////////////////////////////////////////////////
CSlotsNumberingConversionTableWrapper* CCardsProcess::GetSlotsNumberingConversionTable()
{
	return m_pSlotsNumConversionTable;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::ResetBoardInstalling()
{
  TRACESTR(eLevelInfoNormal) << "CCardsProcess::ResetBoardInstalling";
  m_numberOfBoardInstalling = 0;
  m_software_progress_map.clear();
  m_last_total_software_progress = -1;
}


/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::AddBoardInstalling()
{
  TRACESTR(eLevelInfoNormal) << "CCardsProcess::AddBoardInstalling";
  m_numberOfBoardInstalling++;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::DecearseBoardInstalling()
{

	m_numberOfBoardInstalling--;
	TRACESTR(eLevelInfoNormal) << "CCardsProcess::DecearseBoardInstalling " << m_numberOfBoardInstalling;
	if (m_numberOfBoardInstalling < 0)
	{
		PASSERT(m_numberOfBoardInstalling);
	}
	if (m_numberOfBoardInstalling == 0)
	{
		CManagerApi api(eProcessCards);
		api.SendOpcodeMsg(LAST_CARD_INSTALLATION_FINISHED);
		TRACESTR(eLevelInfoNormal) << "CCardsProcess::DecearseBoardInstalling sending LAST_CARD_INSTALLATION_FINISHED to CardsManager";
	}
}


/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::ResetBoardInstallingIpmc()
{
  TRACESTR(eLevelInfoNormal) << "CCardsProcess::ResetBoardInstallingIpmc";
  m_numberOfBoardNeedsIpmc = 0;
  m_boards_requires_ipmc_upgrade.clear();
  m_ipmc_progress_map.clear();
  m_last_total_ipmc_progress = 0;
  m_numberofIpmcIndToCount = 0;
}


/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::IncBoardIpmcIndCounter(int boardId)
{
	m_numberofIpmcIndToCount++;
	TRACESTR(eLevelInfoNormal) << "CCardsProcess::IncBoardIpmcIndCounter boardId=" << boardId
								<< " m_numberofIpmcIndToCount=" << m_numberofIpmcIndToCount;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::DecBoardIpmcIndCounter(int boardId)
{
	m_numberofIpmcIndToCount--;
	TRACESTR(eLevelInfoNormal) << "CCardsProcess::DecBoardIpmcIndCounter boardId=" << boardId
							<< " m_numberofIpmcIndToCount=" << m_numberofIpmcIndToCount;
}

/////////////////////////////////////////////////////////////////////////////
int CCardsProcess::GetBoardIpmcIndCounter()
{
	return m_numberofIpmcIndToCount;
}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::AddBoardInstallingIpmc(int board_id)
{
  TRACESTR(eLevelInfoNormal) << "CCardsProcess::AddBoardInstallingIpmc";
  if (TestIfCardNeedIpmcUpgrade(board_id))
  {
	  // do nothing aready reported
  }
  else
  {
	  m_numberOfBoardNeedsIpmc++;
	  m_boards_requires_ipmc_upgrade.insert(board_id);
  }
  // m_boards_requires_ipmc_upgrade.push_back(m_board_id
}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::UpdateSoftwareProgress(int board,int value)
{
	if (m_software_progress_map.count(board) != 0)
	{
		// the value must to monotonic increasing
		PASSERT(m_software_progress_map[board] > value);
	}
	m_software_progress_map[board] = value;

	int new_total = 0;
	for (std::map< int,int >::iterator itr = m_software_progress_map.begin();
	itr != m_software_progress_map.end();
	itr++)
	{
		new_total += itr->second;
	}

	TRACESTR(eLevelInfoNormal) << "CCardsProcess::UpdateSoftwareProgress, new_total=" << new_total
							<< " m_last_total_software_progress=" << m_last_total_software_progress
							<< " m_software_progress_map.size()=" << m_software_progress_map.size();


	if (new_total > m_last_total_software_progress)
	{
		// inform installer
		CManagerApi api(eProcessInstaller);
		CSegment*  pMsg = new CSegment;
		*pMsg << (DWORD) (new_total / m_software_progress_map.size());
		api.SendMsg(pMsg, INSTALLER_SOFTWARE_PROGRESS_FROM_CARDS);
		m_last_total_software_progress = new_total;
	}

}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::UpdateIpmcProgress(int board,int value)
{

	if (m_ipmc_progress_map.count(board) != 0)
	{
		// the value must to monotonic increasing
		PASSERT(m_ipmc_progress_map[board] > value);
	}
	m_ipmc_progress_map[board] = value;

	int new_total = 0;
	for (std::map< int,int >::iterator itr = m_ipmc_progress_map.begin();
	itr != m_ipmc_progress_map.end();
	itr++)
	{
		new_total += itr->second;
	}

	if(new_total > 0)
		new_total = new_total / ((int )(m_ipmc_progress_map.size()));

	TRACESTR(eLevelInfoNormal) << "CCardsProcess::UpdateIpmcProgress, new_total=" << new_total
								<< " m_last_total_ipmc_progress=" << m_last_total_ipmc_progress
								<< " m_ipmc_progress_map.size()=" << m_ipmc_progress_map.size();


	if (new_total > m_last_total_ipmc_progress)
	{
		// inform installer
		CManagerApi api(eProcessInstaller);
		CSegment*  pMsg = new CSegment;
		*pMsg << (DWORD) new_total;
		api.SendMsg(pMsg, INSTALLER_IPMC_PROGRESS_FROM_CARDS);
		m_last_total_ipmc_progress = new_total;
	}

}


/////////////////////////////////////////////////////////////////////////////
BOOL CCardsProcess::TestIfCardNeedIpmcUpgrade(int board_id)
{
	return (m_boards_requires_ipmc_upgrade.find(board_id) != m_boards_requires_ipmc_upgrade.end());

}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::DecearseBoardInstallingIpmc(int board_id)
{

	if (m_boards_requires_ipmc_upgrade.find(board_id) == m_boards_requires_ipmc_upgrade.end())
	{
		TRACESTR(eLevelInfoNormal) << "CCardsProcess::DecearseBoardInstallingIIpmc board not found " <<board_id
		<< " m_numberOfBoardNeedsIpmc "<<m_numberOfBoardNeedsIpmc;


	}
	else
	{
		m_numberOfBoardNeedsIpmc--;
		m_boards_requires_ipmc_upgrade.erase(m_boards_requires_ipmc_upgrade.find(board_id));



		TRACESTR(eLevelInfoNormal) << "CCardsProcess::DecearseBoardInstallingIIpmc " << m_numberOfBoardNeedsIpmc;
		if (m_numberOfBoardInstalling < 0)
		{
			PASSERT(m_numberOfBoardNeedsIpmc);
		}
		if (m_numberOfBoardNeedsIpmc == 0)
		{
			CManagerApi api(eProcessInstaller);
			api.SendOpcodeMsg(INSTALLER_LAST_CARD_IPMC_FINISHED);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
CCardsIceResponseList* CCardsProcess::GetCardsIceResponseList()
{
	return m_pIceResponseList;
}
/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::InitIceResponseList()
{
	if (m_pIceResponseList == NULL)
		m_pIceResponseList = new CCardsIceResponseList;		//for the first time working\setting the list	
}
/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::PrintIceResponseListDB()
{
	string strToPrint = "\nCCardsProcess::PrintIceResponseListDB";

	char tmpBuf[TEN_LINE_BUFFER_LEN];
    for (int i=0; i<m_pIceResponseList->GetNumOfCards(); i++)
    {
    	memset(tmpBuf, 0, TEN_LINE_BUFFER_LEN);
    	
    	CCardIceResponse* pCardResponse = m_pIceResponseList->GetCardResponse(i);
    	if(pCardResponse == NULL) 
    	{
    		TRACESTR(eLevelInfoNormal) << "CCardsProcess::PrintIceResponseListDB - pCardResponse is NULL for i = " << i;
    		continue;
    	}
    	if (pCardResponse->GetResponseReceived()==true)
    	{
    		ICE_INIT_IND_S response = pCardResponse->GetIceResponse();
	    	sprintf( tmpBuf, 
	    			 "\npos = %d BoardId = %d, SubBoardId = %d, \nresponse received = req_id = %d,STATUS = %d, \n \
	    			 STUN_Pass_status = %d, STUN_udp_status = %d, STUN_tcp_status = %d, Relay_udp_status = %d, Relay_tcp_status = %d, fw_type = %d",
	    			 i,
	    			 pCardResponse->GetBoardId(),
	    			 pCardResponse->GetSubBoardId(),
	    			 response.req_id,
	    			 response.status,
	    			 response.STUN_Pass_status,
	    			 response.STUN_udp_status,
	    			 response.STUN_tcp_status,
	    			 response.Relay_udp_status,
	    			 response.Relay_tcp_status,
	    			 response.fw_type);
    	}
    	else
    	{
	    	sprintf( tmpBuf, 
	    			 "\npos = %d, BoardId = %d, SubBoardId = %d - still waiting for the response from the CM",
	    			 i,
	    			 pCardResponse->GetBoardId(),
	    			 pCardResponse->GetSubBoardId());
    	}
    	
    	strToPrint += tmpBuf;
    }
    
	TRACESTR(eLevelInfoNormal) << strToPrint;    
}
/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::UpdateIceResponseList(DWORD req_id,DWORD Status, DWORD board_Id, DWORD subBoard_id,
										  WORD STUN_Pass_status, WORD STUN_udp_status, WORD STUN_tcp_status, 
										  WORD Relay_udp_status, WORD Relay_tcp_status, WORD fw_type)
{
	if (m_pIceResponseList == NULL)
		TRACESTR(eLevelInfoNormal) << "CCardsProcess::UpdateIceResponseList - m_pIceResponseList = NULL!!!";
	else
	{		
		if (m_pIceResponseList->GetReqId() == req_id)
		{
			for (int i=0; i<m_pIceResponseList->GetNumOfCards(); i++)
			{
				CCardIceResponse* pCardResponse = m_pIceResponseList->GetCardResponse(i);
				if (pCardResponse && pCardResponse->GetBoardId()==board_Id && pCardResponse->GetSubBoardId() == subBoard_id)
				{
					TRACESTR(eLevelInfoNormal) << "CCardsProcess::UpdateIceResponseList - right card was found";
					pCardResponse->SetIceResponse(req_id,Status,STUN_Pass_status, STUN_udp_status, STUN_tcp_status, Relay_udp_status, Relay_tcp_status, fw_type);
					pCardResponse->SetResponseReceived(true);
					break;
				}
			}
			
			PrintIceResponseListDB();	//TBD - to be removed after integration
		}
		else
			TRACESTR(eLevelInfoNormal) << "CCardsProcess::UpdateIceResponseList - bad request id";
	}
}


////_M_S_
/////////////////////////////////////////////////////////////////////////////
DWORD CCardsProcess::GetCardServiceId(DWORD board_id, DWORD subBoardId)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::GetCardServiceId";

	CMediaIpParameters* mediaIpParamsFromVector = NULL;
	CMediaIpParamsVector * mediaIpParamsVector = GetMediaIpParamsVector();
	if(mediaIpParamsVector == NULL)
	{
		TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::GetCardServiceId  - GetMediaIpParamsVector() == NULL!!";
		return -1;
	}

	int nNumOfMediaParams = mediaIpParamsVector->Size();
	if(nNumOfMediaParams <=0)
	{
		TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::GetCardServiceId  - nNumOfMediaParams <=0!!";
		return -1;
	}

	for(int i = 0; i < nNumOfMediaParams; i++)
	{
		mediaIpParamsFromVector = mediaIpParamsVector->At(i);
		if (mediaIpParamsFromVector == NULL)
			TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::GetCardServiceId - nothing!!!";
		else
		{
			if(mediaIpParamsFromVector->GetBoardId() == board_id && mediaIpParamsFromVector->GetSubBoardId() == subBoardId )
				return mediaIpParamsFromVector->GetServiceId();
		}

	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CCardsProcess::IsAllICEResponseReceived(DWORD req_id)
{
	if (m_pIceResponseList && m_pIceResponseList->GetReqId() == req_id)
	{
		for (int i=0; i<m_pIceResponseList->GetNumOfCards(); i++)
		{
			CCardIceResponse* pCardResponse = m_pIceResponseList->GetCardResponse(i);
			if(pCardResponse == NULL)
			{
				TRACESTR(eLevelInfoNormal) << "CCardsProcess::IsAllICEResponseReceived - pCardResponse is NULL";
				return FALSE;
			}

			if (!pCardResponse->GetResponseReceived())
				return FALSE;
		}
	}
	return TRUE;
}

//_M_S_
/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SetCardIPAddressesInICEInitReq(const DWORD boardId, const DWORD subBoardId, ICE_SERVER_TYPES_S* pIceServerTypesStruct)
{
	DBGPASSERT_AND_RETURN(!pIceServerTypesStruct);

	TRACEINTO << "boardId:" << boardId << ", subBoardId:" << subBoardId;

	DWORD nNumOfMediaParams = (m_pIpMediaParamsVector) ? m_pIpMediaParamsVector->Size() : 0;
	if(nNumOfMediaParams <=0)
	{
		return;
	}

	TRACEINTO << "DBG nNumOfMediaParams:" << nNumOfMediaParams;

	char resIpAddressStr[IceAddrInitStr];
	DWORD resIpAddressLen = 0;
	char tmpIP[64] = {'\0'};
	BOOL isIPv6AddressAlreadyTaken = FALSE;

	memset(resIpAddressStr, '\0', sizeof(resIpAddressStr));

	CMediaIpParameters* mediaIpParamsFromVector = NULL;
	for(DWORD i = 0; i < nNumOfMediaParams && (resIpAddressLen < IceAddrInitStr - 1); i++)
	{
		//TRACEINTO << "DBG -------------------------------media param [" << i << "]:";
		mediaIpParamsFromVector = m_pIpMediaParamsVector->At(i);
		if (mediaIpParamsFromVector == NULL)
			TRACEINTO << "mediaIpParamsFromVector == NULL, do nothing";
		else
		{
			TRACEINTO << "boardId:" << mediaIpParamsFromVector->GetBoardId()
					  << " , subBoardId:" << mediaIpParamsFromVector->GetSubBoardId()
					  << " , index:" << i;

			if(mediaIpParamsFromVector->GetBoardId() == boardId && mediaIpParamsFromVector->GetSubBoardId() == subBoardId && (resIpAddressLen < IceAddrInitStr - 1))
			{
				COsQueue* queue = GetMfaMbx(boardId, subBoardId);
				if (queue)
				{
					TRACEINTO << "DBG Send ICE init to specific board"<< ", resIpAddressLen:" << resIpAddressLen;

					for(DWORD interfaceNdx = 0; interfaceNdx < MAX_NUM_OF_PQS ; interfaceNdx ++)
					{
						DWORD ipType = mediaIpParamsFromVector->GetIpParams().interfacesList[interfaceNdx].ipType;
						TRACEINTO << "DBG iptype: " << ipType;//<< ", resIpAddressLen:" << resIpAddressLen;

						if( eIpType_IpV4 == ipType || eIpType_Both == ipType)
						{
							if(mediaIpParamsFromVector->GetIpParams().interfacesList[interfaceNdx].iPv4.iPv4Address != 0)
							{
								memset(tmpIP, '\0', sizeof(tmpIP));
								SystemDWORDToIpString(mediaIpParamsFromVector->GetIpParams().interfacesList[interfaceNdx].iPv4.iPv4Address, tmpIP);
								tmpIP[sizeof(tmpIP) -1 ] = '\0';

								if( resIpAddressLen + strlen(tmpIP) + 1/*;*/ < IceAddrInitStr)
								{
									strncat(resIpAddressStr, tmpIP, sizeof(resIpAddressStr) - strlen(resIpAddressStr) - 1);
									strncat(resIpAddressStr, ";", sizeof(resIpAddressStr) - strlen(resIpAddressStr) - 1);
								}
								resIpAddressLen = min( resIpAddressLen + (DWORD)strlen(tmpIP) + 1, (DWORD)IceAddrInitStr-1);
								TRACEINTO << "DBG concat ipv4 tmpIP:" << tmpIP << ", resIpAddressStr:" << resIpAddressStr<< ", resIpAddressLen:" << resIpAddressLen;
							}
						}

						if(eIpType_IpV6 == ipType || eIpType_Both == ipType)
						{
							// commented as :  we'll take the first ipv6 for each card.  assuming, if there is global ipv6 it will be the first one
							//for(DWORD ipv6Ndx = 0; ipv6Ndx < NUM_OF_IPV6_ADDRESSES && resIpAddressLen < IceAddrInitStr -1; ipv6Ndx++)
							{
								if( strcmp("::/64",(char*)mediaIpParamsFromVector->GetIpParams().interfacesList[interfaceNdx].iPv6s[0].iPv6Address)
									&& strcmp("",(char*)mediaIpParamsFromVector->GetIpParams().interfacesList[interfaceNdx].iPv6s[0].iPv6Address )
									&& ! isIPv6AddressAlreadyTaken)
								{
									memset(tmpIP, '\0', sizeof(tmpIP));
									strncpy(tmpIP, (char*)(mediaIpParamsFromVector->GetIpParams().interfacesList[interfaceNdx].iPv6s[0].iPv6Address), sizeof(tmpIP)-1);
									char * pch = strstr(tmpIP, "/");
									if(pch)	{
										*pch = '\0';
									}

									if( resIpAddressLen + strlen(tmpIP) + 1/*;*/ < IceAddrInitStr)
									{
										strncat(resIpAddressStr, tmpIP, sizeof(resIpAddressStr) - strlen(resIpAddressStr) - 1);
										strncat(resIpAddressStr, ";", sizeof(resIpAddressStr) - strlen(resIpAddressStr) - 1);
									}
									resIpAddressLen = min( resIpAddressLen + (DWORD)strlen(tmpIP) + 1, (DWORD)IceAddrInitStr -1);
									isIPv6AddressAlreadyTaken = TRUE;
									TRACEINTO << "DBG concat ipv6 tmpIP:" << tmpIP << ", resIpAddressStr:" << resIpAddressStr << ", resIpAddressLen:" << resIpAddressLen;
								}
							}
						}

						if(resIpAddressLen >= IceAddrInitStr - 1)
						{
							PASSERTMSG(1,"no more ip addresses can be added");
						}


					}

				}
			} // if : mediaIpParamsFromVector->GetBoardId() == boardId
		} // if : mediaIpParamsFromVector != NULL
	} // for : nNumOfMediaParams

	memset(pIceServerTypesStruct->ipAddrsStr, '\0', IceAddrInitStr);
	strncpy(pIceServerTypesStruct->ipAddrsStr, resIpAddressStr, IceAddrInitStr - 1 );
}

/////////////////////////////////////////////////////////////////////////////
void CCardsProcess::SendIceInitToSpecificMpmCM(const DWORD boardId, const DWORD subBoardId, ICE_SERVER_TYPES_S* pIceServerTypesStruct)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsProcess::SendIceInitToSpecificMpmCM - struct sent to mpm CM boardId = "<< boardId<< " subBoardId = "<<subBoardId;
	
    CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	mplPrtcl->AddCommonHeader(ICE_INIT_REQ);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, boardId, subBoardId);

	BOOL bTypeStructAdded = TRUE; //BRIDGE-1358
	ICE_SERVER_TYPES_S* iceServerTypesSt = NULL;
	if(pIceServerTypesStruct)
	{
		iceServerTypesSt = pIceServerTypesStruct;
		//mplPrtcl->AddData(sizeof(ICE_SERVER_TYPES_S), (char*)pIceServerTypesStruct);
	} else if(m_pIceResponseList->GetIceServerTypes())
	{
		iceServerTypesSt = m_pIceResponseList->GetIceServerTypes();
		//mplPrtcl->AddData(sizeof(ICE_SERVER_TYPES_S), (char*)m_pIceResponseList->GetIceServerTypes());
	} else
	{
		bTypeStructAdded = FALSE;
        PASSERTMSG(TRUE, "CCardsProcess::SendIceInitToSpecificMpmCM IceServerTypesStruct is NULL!!");
	}

	if(bTypeStructAdded)
	{
		SetCardIPAddressesInICEInitReq(boardId, subBoardId, iceServerTypesSt);
		TRACEINTO << "IP addresses: " << iceServerTypesSt->ipAddrsStr;
		mplPrtcl->AddData(sizeof(ICE_SERVER_TYPES_S), (char*)iceServerTypesSt);

		CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("CARDS_SENDS_TO_MPL");
		mplPrtcl->SendMsgToMplApiCommandDispatcher();
		m_pIceResponseList->AddCardIceResponse(boardId, subBoardId);
	}
	
	POBJDELETE(mplPrtcl);
}



//class CCardIceResponse
CCardIceResponse::CCardIceResponse()
{
	m_ResponseReceived = false;
	m_boardId = 0;
	m_subBoardId = 0;
}
/////////////////////////////////////////////////////////////////////////////
CCardIceResponse::CCardIceResponse(DWORD board_id, DWORD subBoardId)
{
	m_ResponseReceived = false;
	m_boardId = board_id;
	m_subBoardId = subBoardId;
	
	ResetIceResponseDetails();

}
/////////////////////////////////////////////////////////////////////////////
void CCardIceResponse::SetIceResponse(DWORD req_id,DWORD Status,WORD STUN_Pass_status, WORD STUN_udp_status, WORD STUN_tcp_status,
									  WORD Relay_udp_status, WORD Relay_tcp_status, WORD fw_type)
{
	m_IceResponse.req_id = req_id;
	m_IceResponse.status = Status;
	m_IceResponse.STUN_Pass_status = STUN_Pass_status;
	m_IceResponse.STUN_udp_status = STUN_udp_status;
	m_IceResponse.STUN_tcp_status = STUN_tcp_status;
	m_IceResponse.Relay_udp_status = Relay_udp_status;
	m_IceResponse.Relay_tcp_status = Relay_tcp_status;
	m_IceResponse.fw_type = fw_type;
}
/////////////////////////////////////////////////////////////////////////////
void CCardIceResponse::ResetIceResponseDetails()
{
	m_IceResponse.req_id = 0;
	m_IceResponse.status = 0;
	m_IceResponse.STUN_Pass_status = eIceServerUnavailble;
	m_IceResponse.STUN_udp_status = eIceServerUnavailble;
	m_IceResponse.STUN_tcp_status = eIceServerUnavailble;
	m_IceResponse.Relay_udp_status = eIceServerUnavailble;
	m_IceResponse.Relay_tcp_status = eIceServerUnavailble;
	m_IceResponse.fw_type = eFwTypeUnknown;	
}
/////////////////////////////////////////////////////////////////////////////
void CCardIceResponse::SetResponseReceived(BOOL bResponseReceived)
{
	m_ResponseReceived = bResponseReceived;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CCardIceResponse::GetBoardId()
{
	return m_boardId;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CCardIceResponse::GetSubBoardId()
{
	return m_subBoardId;
}
/////////////////////////////////////////////////////////////////////////////
bool CCardIceResponse::GetResponseReceived()
{
	return m_ResponseReceived;
}
/////////////////////////////////////////////////////////////////////////////
ICE_INIT_IND_S CCardIceResponse::GetIceResponse()
{
	return m_IceResponse;
}
/////////////////////////////////////////////////////////////////////////////



// class CCardsIceResponseList
CCardsIceResponseList::CCardsIceResponseList()
{
	m_NumOfCards=0;
	m_ReqId = 0;

	//_M_S_
	//m_pIceServerTypesStruct = NULL;
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
		m_pIceServerTypesStruct[i] = NULL;
}
/////////////////////////////////////////////////////////////////////////////
CCardsIceResponseList::~CCardsIceResponseList()
{
	for (int i=0; i<m_NumOfCards; i++)
		POBJDELETE(m_ResponseList[i]);

	//_M_S_
	//POBJDELETE(m_pIceServerTypesStruct);
	for (int i=0; i<MAX_NUM_OF_BOARDS; i++)
		if(m_pIceServerTypesStruct[i])
			POBJDELETE(m_pIceServerTypesStruct[i]);
}
/////////////////////////////////////////////////////////////////////////////
void CCardsIceResponseList::SetReqId(DWORD id)
{
	m_ReqId = id;
}
/////////////////////////////////////////////////////////////////////////////
void CCardsIceResponseList::SetIceServerTypes(ICE_SERVER_TYPES_S* pIceServerTypesStruct, int nService_id)
{
	//_M_S_
	//m_pIceServerTypesStruct = pIceServerTypesStruct;
	if(m_pIceServerTypesStruct[nService_id] != NULL) //preventing leaks
	  POBJDELETE(m_pIceServerTypesStruct[nService_id]);

	m_pIceServerTypesStruct[nService_id] = pIceServerTypesStruct;
}
/////////////////////////////////////////////////////////////////////////////
WORD CCardsIceResponseList::GetNumOfCards()
{
	return m_NumOfCards;
}
/////////////////////////////////////////////////////////////////////////////
CCardIceResponse* CCardsIceResponseList::GetCardResponse(WORD pos)
{
	if(pos < MAX_NUM_OF_BOARDS)
	{
		return m_ResponseList[pos];
	}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CCardsIceResponseList::GetReqId()
{
	return m_ReqId;
}
/////////////////////////////////////////////////////////////////////////////
void CCardsIceResponseList::AddCardIceResponse(DWORD board_id, DWORD sub_board_id)
{
	TRACESTR(eLevelInfoNormal) << "\nCCardsIceResponseList::AddCardIceResponse";

	if(m_NumOfCards >= MAX_NUM_OF_BOARDS)
	{
		PASSERT_AND_RETURN(1);
	}

	for (int i=0; i<m_NumOfCards; i++)
	{
		//if this card already exist in thelist, and we are waiting for his response
		if (m_ResponseList[i]->GetBoardId()==board_id && m_ResponseList[i]->GetSubBoardId()==sub_board_id)
		{
			TRACESTR(eLevelInfoNormal) << "\nCCardsIceResponseList::AddCardIceResponse - send again ice request to board_id: "<< board_id<<" sub_board_id = "<<sub_board_id;
			m_ResponseList[i]->SetResponseReceived(false);
			m_ResponseList[i]->ResetIceResponseDetails();
			return;
		}
	}
	m_ResponseList[m_NumOfCards] = new CCardIceResponse(board_id, sub_board_id);
	m_NumOfCards++;
}

////////////////////////////////////////////////////////////////////////////
BOOL CCardsProcess::GetCardHotSwapStatus(int boardId) const
{
	BOOL cardHotSwap = FALSE;
	if(boardId < MAX_NUM_OF_BOARDS)
	{
		cardHotSwap = m_CardsHotSwapStatus[boardId];
	}
	return cardHotSwap;
}
////////////////////////////////////////////////////////////////////////////

void CCardsProcess::SetCardHotSwapStatus(int boardId,const BOOL hotSwapStatus)
{

	m_CardsHotSwapStatus[boardId]= hotSwapStatus;
}

// VNGR-21504
void CCardsProcess::SaveLastSsmKeepAliveStruct(SWITCH_SM_KEEP_ALIVE_S &lastSsmKeepAliveStruct)
{
	if (!m_firstSsmKeepAliveReceived)
	{
		m_firstSsmKeepAliveReceived = TRUE;
	}

	m_lastSsmKeepAliveStruct = lastSsmKeepAliveStruct;
}

int CCardsProcess::GetSubBoardStatus(DWORD boardId, DWORD subBoardId, DWORD &status)
{
	int i;

	if (!m_firstSsmKeepAliveReceived)
	{
		return -1;
	}

	SM_COMPONENT_STATUS_S *smComponentStatusPtr = (SM_COMPONENT_STATUS_S *)&m_lastSsmKeepAliveStruct;

	for (i = 0; i < MAX_NUM_OF_SLOTS; i++, smComponentStatusPtr++)
	{
		if (smComponentStatusPtr->unSlotId == boardId && 
			smComponentStatusPtr->unSubBoardId == subBoardId)
		{
			status = smComponentStatusPtr->unStatus;

			return 0;
		}
	}

	return -2;
}

////////////////////////////////////////////////////////////////////////////
 eCSExtIntMsgState CCardsProcess::GetCSMsgState(int serviceId) const
{
	eCSExtIntMsgState   state= eIlegalState;
	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
	{
		state = m_CSInternalExtMsgsStatus[serviceId].state;
	}
	return state;
}

////////////////////////////////////////////////////////////////////////////

void CCardsProcess::SetCSMsgState(int serviceId,const eCSExtIntMsgState state)
{
	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
	  m_CSInternalExtMsgsStatus[serviceId].state = state;
}

////////////////////////////////////////////////////////////////////////////
 BOOL CCardsProcess::IsExtMsgArrived(int serviceId) const
{

	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
	{
		return  m_CSInternalExtMsgsStatus[serviceId].ExtMsgArrived;
	}

	return false;

}

////////////////////////////////////////////////////////////////////////////
 BOOL CCardsProcess::IsIntMsgArrived(int serviceId) const
{

	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
	{
		return  m_CSInternalExtMsgsStatus[serviceId].IntMsgArrived;
	}

	return false;

}

////////////////////////////////////////////////////////////////////////////

void CCardsProcess::SetExtMsgArrived(int serviceId,BOOL isArrive)
{
	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
	  m_CSInternalExtMsgsStatus[serviceId].ExtMsgArrived = isArrive;
}

////////////////////////////////////////////////////////////////////////////

void CCardsProcess::SetIntMsgArrived(int serviceId,BOOL isArrive)
{
	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
	  m_CSInternalExtMsgsStatus[serviceId].IntMsgArrived = isArrive;
}

////////////////////////////////////////////////////////////////////////////

void CCardsProcess::SetCSIpConfigBoardId(int serviceId,DWORD boardId)
{
	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
	  m_CSInternalExtMsgsStatus[serviceId].boardId = boardId;
}

////////////////////////////////////////////////////////////////////////////
 DWORD CCardsProcess::GetCSIpConfigBoardId(int serviceId) const
{
	DWORD   boardId= 0;
	if(serviceId <= MAX_NUM_OF_IP_SERVICES)
	{
		return m_CSInternalExtMsgsStatus[serviceId].boardId;
	}
	return boardId;
}

 eNGBSystemCardsMode CCardsProcess::ConvertSystemCardsModeToNGBCardsMode(eSystemCardsMode cardMode)
 {
	 switch(cardMode)
	 {

	 case eSystemCardsMode_breeze:
		 return eNGBSystemCardsMode_breeze_only;

	 default:
		 return eNGBSystemCardsMode_mpmrx_only; //MPMRX is the default value

	 }
 }







