// McuMngrProcess.cpp: implementation of the CMcuMngrProcess class.
//
//////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <string>

#include "McuMngrProcess.h"
#include "McuMngrDefines.h"
#include "SysConfigEma.h"
#include "StringsMaps.h"
#include "IpService.h"
#include "IPServiceDynamicList.h"
#include "McuState.h"
#include "FaultsContainer.h"
#include "SystemTime.h"
#include "Licensing.h"
#include "ApiStatuses.h"
#include "SetMcuRestore.h"
#include "TraceStream.h"
#include "SnmpManagerApi.h"
#include "EthernetSettingsConfig.h"
#include "OsFileIF.h"
#include "SystemInterface.h"

#include "PrettyTable.h"

using namespace std;

extern void  McuMngrManagerEntryPoint(void* appParam);
extern char* LicensingValidationStateToString(eLicensingValidationState validationState);



//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CMcuMngrProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CMcuMngrProcess::GetManagerEntryPoint()
{

	return McuMngrManagerEntryPoint;
}

//////////////////////////////////////////////////////////////////////
CMcuMngrProcess::CMcuMngrProcess()
  :m_installPhaseList(READ_ONLY)
{
	m_XMLWraper			= new CIPServiceFullList;
	m_pIpInterfaces_asIpServicesList	= new CIPServiceList(m_XMLWraper);
	m_pMngmntIpParams_asIpService		= new CIPService;
	m_pShelfMngrIpInterface_asIpSpan	= new CIPSpan;
	m_pActiveAlarmsList					= new CFaultList;
	m_pMcuStateObject					= new CMcuState;
	m_pMcuTime							= new CSystemTime();
	m_pLicense                          = new CLicensing;
	m_pEthernetSettingsConfigList		= new CEthernetSettingsConfigList;
	m_pPrecedenceSettings				= new CPrecedenceSettings();
	m_pSysInterfaceList                 = new CSystemInterfaceList();

	// because EMA does not send an ObjToken(Yuri R)
	m_pActiveAlarmsList->IncreaseUpdateCounter();

//	m_pMcuStateObject->SetMcuState(eMcuState_Invalid);
	m_pMcuStateObject->ClearSerialNumber();
	m_pMcuStateObject->SetValidationState(eLicensingValidationUnknown);
	m_pMcuStateObject->SetNumOfActiveAlarms( m_pActiveAlarmsList->GetSize() );

	const char * flexeraLicense = getenv("FLEXERA_LICENSE");

	if ((NULL != flexeraLicense )  && (strcmp(flexeraLicense ,"YES") == 0))
		m_isFlexeraSimulationMode=TRUE;
	else
		m_isFlexeraSimulationMode=FALSE;


	for (int i=0;i<MAX_NUM_OF_FEATURES;i++)
	{
		//m_flexeraCapabilitiesList.featuresArray[i].capabilityStr = "RPCS";
		//m_flexeraCapabilitiesList.featuresArray[i].version = "";
		m_flexeraCapabilitiesList.featuresArray[i].Counted = 0;
		m_flexeraCapabilitiesList.featuresArray[i].IsEnabled = false;
		m_flexeraCapabilitiesList.featuresArray[i].IsChanged = false;
		m_flexeraCapabilitiesList.featuresArray[i].LicenseFeature = RPCS;
	}

}

//////////////////////////////////////////////////////////////////////
CMcuMngrProcess::~CMcuMngrProcess()
{
	POBJDELETE(m_XMLWraper);
	POBJDELETE(m_pIpInterfaces_asIpServicesList);
	POBJDELETE(m_pMngmntIpParams_asIpService);
	POBJDELETE(m_pShelfMngrIpInterface_asIpSpan);
	POBJDELETE(m_pActiveAlarmsList);
	POBJDELETE(m_pMcuStateObject);
	POBJDELETE(m_pMcuTime);
	POBJDELETE(m_pLicense);
	POBJDELETE(m_pEthernetSettingsConfigList);
	POBJDELETE(m_pPrecedenceSettings);
	POBJDELETE(m_pSysInterfaceList);
}


//////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::AddExtraStringsToMap()
{
	CProcessBase::AddExtraStringsToMap();

	for(eCfgParamType t1 = eCfgParamUser ; t1 < NumOfCfgTypes ; t1 = (eCfgParamType)(t1 + 1))
	{
		CStringsMaps::AddItem(CFG_TYPE_ENUM, t1, GetCfgTypeName(t1));
	}

	for(eMcuRestoreType t2 = eMcuRestoreStandard ; t2 < NumOfMcuRestoreTypes ; t2 = (eMcuRestoreType)(t2 + 1))
	{
		CStringsMaps::AddItem(MCU_RESTORE_TYPE_ENUM, t2, GetMcuRestoreName(t2));
	}

	CStringsMaps::AddItem(INSTALL_PHASE_TYPE_ENUM, eInstallPhaseType_swLoading,   "InstallPhaseType_swLoading");
	CStringsMaps::AddItem(INSTALL_PHASE_TYPE_ENUM, eInstallPhaseType_ipmcBurning, "InstallPhaseType_ipmcBurning");
	CStringsMaps::AddItem(INSTALL_PHASE_TYPE_ENUM, eInstallPhaseType_completed,   "InstallPhaseType_completed");

	CStringsMaps::AddItem(INSTALL_PHASE_STATUS_ENUM, eStatusNotStarted,   "not_started");
	CStringsMaps::AddItem(INSTALL_PHASE_STATUS_ENUM, eStatusInProgress,   "in_progress");
	CStringsMaps::AddItem(INSTALL_PHASE_STATUS_ENUM, eStatusSuccess,      "success");
	CStringsMaps::AddItem(INSTALL_PHASE_STATUS_ENUM, sStatusFailure,      "failure");


	CStringsMaps::AddItem(MCU_STATE_ENUM, eMcuState_Invalid,  GetMcuStateName(eMcuState_Invalid));
	CStringsMaps::AddItem(MCU_STATE_ENUM, eMcuState_Normal,   GetMcuStateName(eMcuState_Normal));
	CStringsMaps::AddItem(MCU_STATE_ENUM, eMcuState_Startup,  GetMcuStateName(eMcuState_Startup));
	CStringsMaps::AddItem(MCU_STATE_ENUM, eMcuState_Major,    GetMcuStateName(eMcuState_Major));
	CStringsMaps::AddItem(MCU_STATE_ENUM, eMcuState_Minor,    GetMcuStateName(eMcuState_Minor));
	CStringsMaps::AddItem(MCU_STATE_ENUM, eMcuState_Critical, GetMcuStateName(eMcuState_Critical));

	CStringsMaps::AddItem(LICENSING_VALIDATION_ENUM, eLicensingValidationSucceeded, ::LicensingValidationStateToString(eLicensingValidationSucceeded));
	CStringsMaps::AddItem(LICENSING_VALIDATION_ENUM, eLicensingValidationFailed,    ::LicensingValidationStateToString(eLicensingValidationFailed));
	CStringsMaps::AddItem(LICENSING_VALIDATION_ENUM, eLicensingValidationUnknown,   ::LicensingValidationStateToString(eLicensingValidationUnknown));

	CStringsMaps::AddItem(LICENSING_CONNECTION_STATUS_ENUM, eLicensingConnectionNotAttempt, ::GetLicensingConnectionStatusStr(eLicensingConnectionNotAttempt));
	CStringsMaps::AddItem(LICENSING_CONNECTION_STATUS_ENUM, eLicensingConnectionConnecting,    ::GetLicensingConnectionStatusStr(eLicensingConnectionConnecting));
	CStringsMaps::AddItem(LICENSING_CONNECTION_STATUS_ENUM, eLicensingConnectionSuccess,   ::GetLicensingConnectionStatusStr(eLicensingConnectionSuccess));
	CStringsMaps::AddItem(LICENSING_CONNECTION_STATUS_ENUM, eLicensingConnectionFail,   ::GetLicensingConnectionStatusStr(eLicensingConnectionFail));
	CStringsMaps::AddItem(LICENSING_CONNECTION_STATUS_ENUM, eLicensingConnectionUnknown,   ::GetLicensingConnectionStatusStr(eLicensingConnectionUnknown));


	CStringsMaps::AddItem(LICENSING_STATUS_ENUM, eLicensingStatusValid, ::GetLicensingStatusStr(eLicensingStatusValid));
	CStringsMaps::AddItem(LICENSING_STATUS_ENUM, eLicensingStatusInvalid,    ::GetLicensingStatusStr(eLicensingStatusInvalid));
	CStringsMaps::AddItem(LICENSING_STATUS_ENUM, eLicensingStatusRestartRequired,   ::GetLicensingStatusStr(eLicensingStatusRestartRequired));
	CStringsMaps::AddItem(LICENSING_STATUS_ENUM, eLicensingStatusUnknown,   ::GetLicensingStatusStr(eLicensingStatusUnknown));



	CStringsMaps::AddItem(NTP_SERVER_STATUS_ENUM, eNtpServerOk,   "ok");
	CStringsMaps::AddItem(NTP_SERVER_STATUS_ENUM, eNtpServerFail, "fail");

	CStringsMaps::AddItem(NTP_SERVER_STATUS_ENUM, eNtpServerConnecting, "connecting");
	CStringsMaps::AddItem(NTP_SERVER_STATUS_ENUM, eNtpServerNotConfigured, "NotConfigured");


	CStringsMaps::AddMinMaxItem(_0_TO_MAC_ADDRESS_CONFIG_LENGTH, 0, MAC_ADDRESS_CONFIG_LEN - 1);

	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_Illegal,				GetEthPortTypeStr(eEthPortType_Illegal));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_Management1,			GetEthPortTypeStr(eEthPortType_Management1));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_Management2,			GetEthPortTypeStr(eEthPortType_Management2));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_ManagementShelfMngr,	GetEthPortTypeStr(eEthPortType_ManagementShelfMngr));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_Signaling1,				GetEthPortTypeStr(eEthPortType_Signaling1));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_Signaling2,				GetEthPortTypeStr(eEthPortType_Signaling2));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_Media,					GetEthPortTypeStr(eEthPortType_Media));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_Modem,					GetEthPortTypeStr(eEthPortType_Modem));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_Media_Signaling_Managment,					GetEthPortTypeStr(eEthPortType_Media_Signaling_Managment));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_Media_Signaling,					        GetEthPortTypeStr(eEthPortType_Media_Signaling));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_ManagementShelfMngr_Managment_Signaling_media,					    GetEthPortTypeStr(eEthPortType_ManagementShelfMngr_Managment_Signaling_media));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_ManagementShelfMngr_Managment_Signaling,					    GetEthPortTypeStr(eEthPortType_ManagementShelfMngr_Managment_Signaling));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_ManagementShelfMngr_Managment,					    GetEthPortTypeStr(eEthPortType_ManagementShelfMngr_Managment));
	CStringsMaps::AddItem(ETHERNET_PORT_TYPE_ENUM, eEthPortType_ManagementShelfMngr_Signaling,					    GetEthPortTypeStr(eEthPortType_ManagementShelfMngr_Signaling));

	CStringsMaps::AddItem(AUTHENTICATION_PROTOCOL_802_1X_ENUM, eEth802_1xAuthenticationType_Off,				Get802_1xAuthenticationTypeStr(eEth802_1xAuthenticationType_Off));
	CStringsMaps::AddItem(AUTHENTICATION_PROTOCOL_802_1X_ENUM, eEth802_1xAuthenticationType_PEAPv0_MSCHAPV2,			Get802_1xAuthenticationTypeStr(eEth802_1xAuthenticationType_PEAPv0_MSCHAPV2));
	CStringsMaps::AddItem(AUTHENTICATION_PROTOCOL_802_1X_ENUM, eEth802_1xAuthenticationType_EAP_TLS,			Get802_1xAuthenticationTypeStr(eEth802_1xAuthenticationType_EAP_TLS));
	CStringsMaps::AddItem(AUTHENTICATION_PROTOCOL_802_1X_ENUM, eEth802_1xAuthenticationType_EAP_MD5,	Get802_1xAuthenticationTypeStr(eEth802_1xAuthenticationType_EAP_MD5));

}

//////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::SetMngmntIpParams_asIpService(CIPService &theService)
{
	POBJDELETE(m_pMngmntIpParams_asIpService);
	m_pMngmntIpParams_asIpService = new CIPService(theService);
	UpdateIpInterfaceInList(*m_pMngmntIpParams_asIpService);
}

//////////////////////////////////////////////////////////////////////
CIPService* CMcuMngrProcess::GetMngmntIpParams_asIpService()
{
	return m_pMngmntIpParams_asIpService;
}

//////////////////////////////////////////////////////////////////////
string CMcuMngrProcess::GetCntrlIPv6Address()
{
	CIPSpan* pMngmnt = m_pMngmntIpParams_asIpService->GetFirstSpan();

	return pMngmnt->GetIPv6Address(0);
}

const APIU8* CMcuMngrProcess::GetMngmtIpV6ByteArray() const
{

	const CIPSpan* ipSpan = m_pMngmntIpParams_asIpService->GetFirstSpan();
    if(NULL == ipSpan)
    {
    	TRACEINTO << (eLevelError) << "MNGMNT span does not exist"; 
        return NULL;
    }
    
    return ipSpan->GetIPv6AddressByteArray(0);
}

const APIU8* CMcuMngrProcess::GetSheIfIpV6ByteArray() const
{

	const CIPSpan *ipSpan = m_pMngmntIpParams_asIpService->GetSpanByIdx(1);
    if(NULL == ipSpan)
    {
    	TRACEINTO << (eLevelError) << "Shelf MNGMNT span does not exist"; 
        return NULL;
    }
    return ipSpan->GetIPv6AddressByteArray(0);
}


//////////////////////////////////////////////////////////////////////
CIPServiceList* CMcuMngrProcess::GetIpInterfaces_asIpServicesList()
{
	return m_pIpInterfaces_asIpServicesList;
}

//////////////////////////////////////////////////////////////////////
CIPSpan* CMcuMngrProcess::GetShelfMngrIpInterface_asIpSpan()
{
	return m_pShelfMngrIpInterface_asIpSpan;
}

//////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::SetShelfMngrIpInterface_asIpSpan(CIPSpan& other)
{
	POBJDELETE(m_pShelfMngrIpInterface_asIpSpan);
	m_pShelfMngrIpInterface_asIpSpan = new CIPSpan(other);
}

//////////////////////////////////////////////////////////////////////
BOOL CMcuMngrProcess::IsIpInterfaceExistsInList(const CIPService&  other)
{
	BOOL isExist = NO;
	if ( NULL != m_pIpInterfaces_asIpServicesList &&
         m_pIpInterfaces_asIpServicesList->FindService(other) != NOT_FIND )
    {
        isExist = YES;
    }
    return isExist;
}

//////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::AddIpInterfaceToList(const CIPService&  other)
{
	STATUS retStatus = m_pIpInterfaces_asIpServicesList->AddOnlyMem(other);

	return retStatus;
}

//////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::UpdateIpInterfaceInList(CIPService&  other)
{
	const char *description = "Unexpected status";
	STATUS retStatus = m_pIpInterfaces_asIpServicesList->UpdateOnlyMem(other);
	if(STATUS_OK == retStatus)
	{
		description = "Service updated";
	}
	else if(STATUS_H323_SERVICE_NAME_NOT_EXISTS == retStatus)
	{
		retStatus = m_pIpInterfaces_asIpServicesList->AddOnlyMem(other);
		description = (STATUS_OK == retStatus ? "Service Added" : "Failed to Add Service");
	}
	else
	{}

	TRACEINTO 	<< (eLevelInfoNormal) << "\nCMcuMngrProcess::UpdateIpInterfaceInList : \n"
				<< description << "\n"
				<< "Status : " << CProcessBase::GetProcess()->GetStatusAsString(retStatus);

//	other.WriteXmlFile(MANAGEMENT_NETWORK_CONFIG_PATH, "MngmntNetwork");

	return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
CMcuState* CMcuMngrProcess::GetMcuStateObject()
{
	return m_pMcuStateObject;
}

//////////////////////////////////////////////////////////////////////////////
// void CMcuMngrProcess::SetMcuStateObject(CMcuState* pMcuState)
// {
//	m_pMcuStateObject = pMcuState;
// 	CSnmpManagerApi api;
//     api.UpdateSNMPnotification();

//}


//////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::CreateDefaultMcuTimeFile()
{
	m_pMcuTime->SetIsNTP(NO);
	m_pMcuTime->WriteXmlFile(SYSTEM_TIME_PATH);
}


/////////////////////////////////////////////////////////////////////////////

const CSystemTime& CMcuMngrProcess::GetCurrentMcuTime() const
{
	return *m_pMcuTime;
}


/////////////////////////////////////////////////////////////////////////////
BOOL CMcuMngrProcess::GetMcuTime(CSystemTime &Time)
{
	Time=*m_pMcuTime;
	Time.UpdateCurrentTime();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::SetMcuTime(CSystemTime &Time)
{
	m_pMcuTime->SetParams(Time);//does not set the actual time
	m_pMcuTime->WriteXmlFile(SYSTEM_TIME_PATH);
}

//////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::SetMcuTimeNtpServerStatus(int index, eNtpServerStatus serverStatus, WORD numFailuresSinceConnecting)
{
	m_pMcuTime->SetNtpServerStatus(index, serverStatus);
	m_pMcuTime->SetNumFailuresSinceConnecting(index, numFailuresSinceConnecting);

}

//////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::LoadMcuTime()
{
	STATUS status = m_pMcuTime->ReadXmlFile(SYSTEM_TIME_PATH,
						eNoActiveAlarm,
						eRenameFile,
						FILE_IDENTIFIER_timeConfigFile);

	for (int i = 0; i < NTP_MAX_NUM_OF_SERVERS; i++)
	{
			m_pMcuTime->SetNtpServerStatus(i, eNtpServerNotConfigured);
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::SetLicensing(CLicensing* pLicensing)
{
	POBJDELETE(m_pLicense);
	m_pLicense = new CLicensing(*pLicensing);
}

/////////////////////////////////////////////////////////////////////////////
CLicensing* CMcuMngrProcess::GetLicensing()
{
	return m_pLicense;
}

/////////////////////////////////////////////////////////////////////////////
CFaultList* CMcuMngrProcess::GetActiveAlarmsList()
{
	return m_pActiveAlarmsList;
}

//////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::SetActiveAlarmsList(CFaultList* pActiveAlarmsList)
{
	m_pActiveAlarmsList = pActiveAlarmsList;
}

//////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::AddExtraStatusesStrings()
{
}

//////////////////////////////////////////////////////////////////////////////
DWORD CMcuMngrProcess::GetMngmntIp()const
{
    DWORD mngmntIp = 0;
    const CIPSpan *ipSpan = m_pMngmntIpParams_asIpService->GetFirstSpan();
    if(NULL != ipSpan)
    {
        mngmntIp = ipSpan->GetIPv4Address();
    }
    else
    {
        PASSERTMSG(TRUE, "MNGMNT span does not exist");
    }
    return mngmntIp;
}

//////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::GetMngmntIpV6(char *retIp)const
{
    const CIPSpan *ipSpan = m_pMngmntIpParams_asIpService->GetFirstSpan();
    if(NULL != ipSpan)
    {
        ipSpan->GetIPv6Address(0, retIp);
    }
    else
    {
        PASSERTMSG(TRUE, "MNGMNT span does not exist");
    }
}

//////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::GetMngmntIpAsString(string & outStrMngmntIp)const
{
    DWORD mngmntIp = GetMngmntIp();
    char buffer [32];
    SystemDWORDToIpString(mngmntIp, buffer);
    outStrMngmntIp = buffer;
}

//////////////////////////////////////////////////////////////////////////////
DWORD CMcuMngrProcess::GetShelfIp()const
{
    DWORD shelfIp = 0;

    if (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator)
    {
    	shelfIp = GetMngmntIp();
    }
   	else
    {
    	const CIPSpan *ipSpan = m_pMngmntIpParams_asIpService->GetSpanByIdx(1);
    	if(NULL == ipSpan)
    	{
    		PASSERTMSG(TRUE, "Shelf MNGMNT span does not exist");
    		return shelfIp;
    	}

    	shelfIp = ipSpan->GetIPv4Address();
    }

    return shelfIp;
}



/////////////////////////////////////////////////////////////////////////////
bool CMcuMngrProcess::IsFlexeraCapabilityEnabled(int i)
{
	return m_flexeraCapabilitiesList.featuresArray[i].IsEnabled;
}
/////////////////////////////////////////////////////////////////////////////
bool CMcuMngrProcess::IsFlexeraCapabilityChanged(int i)
{
	return m_flexeraCapabilitiesList.featuresArray[i].IsChanged;
}
/////////////////////////////////////////////////////////////////////////////
//void CMcuMngrProcess::GetFlexeraCapability(int i,char* name)
//{
//	strcpy(name, m_flexeraCapabilitiesList.featuresArray[i].capabilityStr.c_str());
//}

/////////////////////////////////////////////////////////////////////////////
DWORD CMcuMngrProcess::GetFlexeraCapabilityCounted(int i)
{
	return m_flexeraCapabilitiesList.featuresArray[i].Counted;
}

/////////////////////////////////////////////////////////////////////////////
E_FLEXERA_LICENSE_VALIDATION_STATUS CMcuMngrProcess::GetFlexeraCapabilityStatus(int i)
{
	return m_flexeraCapabilitiesList.featuresArray[i].LicenseStatus;
}
/////////////////////////////////////////////////////////////////////////////
CStructTm CMcuMngrProcess::GetFlexeraCapabilityExpDate(int i)
{
	return m_flexeraCapabilitiesList.featuresArray[i].expirationDate;
}


/////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::IncreaseFlexeraCapabilityExpDateMon(int i)
{
	 m_flexeraCapabilitiesList.featuresArray[i].expirationDate.m_mon++;
}

/////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::SetFlexeraData(FLEXERA_DATA_S * serverParams)
{
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SetFlexeraData Start";

	PrintFlexeraData();

	memcpy((BYTE*)&m_flexeraCapabilitiesList,(BYTE *)serverParams,sizeof(FLEXERA_DATA_S));

	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::SetFlexeraData -----------";

	PrintFlexeraData();
}



/////////////////////////////////////////////////////////////////////////////
CEthernetSettingsConfigList* CMcuMngrProcess::GetEthernetSettingsConfigList()
{
	return m_pEthernetSettingsConfigList;
}

//////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::SetEthernetSettingsConfig(CEthernetSettingsConfig &ethernetSettingsConfig)
{
	m_pEthernetSettingsConfigList->SetSpecEthernetSettingsConfig(ethernetSettingsConfig);
	m_pEthernetSettingsConfigList->WriteXmlFile(ETHERNET_SETTINGS_MULT_SERV_PATH.c_str());
}

//////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::LoadEthernetSettingsConfig()
{
	/************************************************************************************************/
	/* 16.1.11 Rachel Cohen adding ethernet settings for Multiple Service . adding RTM_LAN 1 port   */
	/* After the first upgrade to the MS version .The Multiple Service EthernetSetting xml file is  */
	/* being created. and the data from the EthernetSetting.xml is being copied to the new xml file */
	/* EthernetSettingMultipleSevice.xml     .                                                      */
	/************************************************************************************************/

	STATUS status=STATUS_OK;
	if ((! IsFileExists(ETHERNET_SETTINGS_MULT_SERV_PATH)) && (!IsFileExists(ETHERNET_SETTINGS_PATH)) )
	{
		TRACEINTO 	<< (eLevelInfoNormal) << "\nCMcuMngrProcess::LoadEthernetSettingsConfig : EthernetsettingMultService.xml file and EthernetSetting.xml file are NOT EXIST\n";
		return status;
	}

	if (! IsFileExists(ETHERNET_SETTINGS_MULT_SERV_PATH))
	{
		   TRACEINTO 	<< (eLevelInfoNormal) << "\nCMcuMngrProcess::LoadEthernetSettingsConfig :reading from  EthernetSetting.xml file \n";


		   CEthernetSettingsConfigList * EthernetSettingsConfigListTemp		= new CEthernetSettingsConfigList;

		   status = EthernetSettingsConfigListTemp->ReadXmlFile( ETHERNET_SETTINGS_PATH.c_str(),
					                                   eNoActiveAlarm,
					                                   eRenameFile);

          if (status==STATUS_OK)
          {
		   CEthernetSettingsConfig* pEthernetSettingsConfig;
		   for (int i=0; i<MAX_NUM_OF_LAN_PORTS; i++)
		   {
              pEthernetSettingsConfig = EthernetSettingsConfigListTemp->GetSpecEthernetSettingsConfig(i);
              m_pEthernetSettingsConfigList->SetSpecEthernetSettingsConfig(*pEthernetSettingsConfig);
		   }
		   POBJDELETE(EthernetSettingsConfigListTemp);
          }
          else
        	  PASSERTMSG(TRUE, "Can not read old ETHERNET_SETTINGS_PATH.................");



	}
	else
	{
		m_pEthernetSettingsConfigList->SetIsFromEma(false);
	      status = m_pEthernetSettingsConfigList->ReadXmlFile( ETHERNET_SETTINGS_MULT_SERV_PATH.c_str(),
																eNoActiveAlarm,
																eRenameFile);
	}
	return status;
}
//////////////////////////////////////////////////////////////////////////////
void CMcuMngrProcess::GetSheIfIpV6(char *retIp)const
{
	if(NULL == retIp)
		return;

	const CIPSpan *ipSpan = m_pMngmntIpParams_asIpService->GetSpanByIdx(1);
    if(NULL == ipSpan)
    {
        PASSERTMSG(TRUE, "Shelf MNGMNT span does not exist");
    }
    else
    {
    	ipSpan->GetIPv6Address(0, retIp);

    }
}

CPrecedenceSettings* CMcuMngrProcess::GetPrecedenceSettings()
{
	if (!m_pPrecedenceSettings)
	{
		TRACESTR(eLevelInfoNormal) << "CMcuMngrProcess::GetPrecedenceSettings - m_pPrecedenceSettings is empty!\n";
	}
	return m_pPrecedenceSettings;
}

void CMcuMngrProcess::SetPrecedenceSettings(CPrecedenceSettings* pPrecedenceSettings)
{
	*m_pPrecedenceSettings = *pPrecedenceSettings;

}
///////////////////////////////////////////////////////////////////////
bool CMcuMngrProcess::IsFailoverBlockTransaction_SlaveMode(string sAction)
{
	bool isBlocked = false;

	if ( ("UPDATE_MANAGEMENT_NETWORK" == sAction) )
	{
		isBlocked = true;
	}

	return isBlocked;
}

///////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::ValidateSwMcuFields(CIPService* pUpdatedIpService)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrProcess::ValidateSwMcuFields\n";

	STATUS status = ValidateIpTypeDetails(m_pMngmntIpParams_asIpService->GetIpType(), pUpdatedIpService->GetIpType());
	if (status!=STATUS_OK)
		return status;

	status = ValidateDnsDetails(m_pMngmntIpParams_asIpService, pUpdatedIpService);
	if (status!=STATUS_OK)
		return status;

	status = ValidateSubnetMaskDetails(m_pMngmntIpParams_asIpService->GetNetMask(), pUpdatedIpService->GetNetMask());
	if (status!=STATUS_OK)
		return status;

	status = ValidateLanPortsDetails(m_pMngmntIpParams_asIpService->GetPortSpeedVector(), pUpdatedIpService->GetPortSpeedVector());
	if (status!=STATUS_OK)
		return status;

	//check the cpu ip address
	CIPSpan* ip_span = m_pMngmntIpParams_asIpService->GetFirstSpan();
	CIPSpan* updated_ip_span = pUpdatedIpService->GetFirstSpan();

	status = ValidateHostname(ip_span->GetSpanHostName(), updated_ip_span->GetSpanHostName());
	if (status!=STATUS_OK)
		return status;

    //EdgeAxis, not allowed to change mng interface
    string strOldInterface = ip_span->GetInterface();
    string strNewInterface = updated_ip_span->GetInterface();
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

    if (curProductType == eProductTypeEdgeAxis && strOldInterface != strNewInterface)
    {
        status = STATUS_MANAGEMENT_INTERFACE_CANT_BE_CHANGED_IN_EDGE;
      	TRACESTR(eLevelInfoNormal) << "CMcuMngrProcess::ValidateSwMcuFields EdgeAxis not allowed to change Management intereface. old:" << strOldInterface<< " new:" << strNewInterface;  
        return status;
    }
    
	//check that the CPU ip addresses and interface configured exist in the system
	status = ValidateIpAddressAndInterface(ip_span->GetIPv4Address(), updated_ip_span->GetIPv4Address(),
			ip_span->GetInterface(), updated_ip_span->GetInterface());

	if (status!=STATUS_OK)
		return status;

	// check the SHM ip - it is not supported and should be changed
	ip_span = m_pMngmntIpParams_asIpService->GetNextSpan();
	updated_ip_span = pUpdatedIpService->GetNextSpan();

	if (ip_span !=NULL && updated_ip_span !=NULL)
	   status = ValidateSHMIpAddress(ip_span->GetIPv4Address(), updated_ip_span->GetIPv4Address());
	if (status!=STATUS_OK)
		return status;

	status = ValidateRouterDetails(m_pMngmntIpParams_asIpService, pUpdatedIpService);
	if (status!=STATUS_OK)
		return status;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////
std::string CMcuMngrProcess::GetMFWValidatedResult()
{
	return m_pSysInterfaceList->GetValidatedResult();
}

/////////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::ValidateMFWFields(CIPService* pUpdateIpService)
{
	TRACESTR(eLevelInfoNormal)<< "CMcuMngrProcess::ValidateMFWFields";
	STATUS status = ValidateDnsDetails(m_pMngmntIpParams_asIpService, pUpdateIpService);
	if (status!=STATUS_OK)
		return status;
	status = ValidateLanPortsDetails(m_pMngmntIpParams_asIpService->GetPortSpeedVector(), pUpdateIpService->GetPortSpeedVector());
	if (status!=STATUS_OK)
		return status;
	// 1. check NIC name be must be non-empty
	CIPSpan* pIpSpan = pUpdateIpService->GetFirstSpan();
	if ( NULL == pIpSpan )
	{
		TRACESTR(eLevelError)<< "CMcuMngrProcess::ValidateMFWFields : the first span of pUpdateIpService is NULL\n";
		return STATUS_SERVICE_NO_SPAN;
	}
	if ( pIpSpan->GetInterface().empty() )
	{
		TRACESTR(eLevelError)<< "CMcuMngrProcess::ValidateMFWFields : the interface must be assigned \n";
		return STATUS_MUST_ASSIGN_INTERFACE_IN_MFW;
	}
	//2. check for Span
	eIpType ipType = pUpdateIpService->GetIpType();
	return m_pSysInterfaceList->ValidateMFWSPANField(pIpSpan, ipType);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::ValidateHostname(const CSmallString& host_name, const CSmallString& updated_host_name)
{
	TRACESTR(eLevelInfoNormal) << "CMcuMngrProcess::ValidateHostname\n" << " host_name: "
			<< host_name.GetString() << " updated_host_name: " << updated_host_name.GetString();

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	if (host_name != updated_host_name && curProductType != eProductTypeEdgeAxis)
		return STATUS_HOST_NAME_CANT_BE_CHANGED_IN_SMCU;
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::ValidateDnsDetails(CIPService *pService, CIPService *pUpdatedService)
{
	//check DNS
	TRACESTR(eLevelInfoNormal) << "CMcuMngrProcess::ValidateDnsDetails";

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	if (*(pService->GetpDns())!=*(pUpdatedService->GetpDns()) && curProductType != eProductTypeEdgeAxis)
		return STATUS_DNS_CANT_BE_CHANGED_IN_SMCU;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::ValidateSubnetMaskDetails(DWORD netMask, DWORD updatedNetmask)
{
	//check Subnet mask
	if (netMask!=updatedNetmask)
		return STATUS_SUBNET_MASK_CANT_BE_CHANGED_IN_SMCU;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::ValidateRouterDetails(CIPService *pService, CIPService *pUpdatedService)
{
	//check default GW
	if (pService->GetDefaultGatewayIPv4()!=pUpdatedService->GetDefaultGatewayIPv4())
		return STATUS_ROUTER_CANT_BE_CHANGED_IN_SMCU;

	if (pService->GetRoutersNumber()!=pUpdatedService->GetRoutersNumber())
		return STATUS_ROUTER_CANT_BE_CHANGED_IN_SMCU;

	//check routers list
	CH323Router *router = pService->GetFirstRouter();
	CH323Router *pUpdatedRouter = pUpdatedService->GetFirstRouter();

	while(NULL != router && pUpdatedRouter != NULL)
	{
		if (*router != *pUpdatedRouter)
			return STATUS_ROUTER_CANT_BE_CHANGED_IN_SMCU;

		router = pService->GetNextRouter();
		pUpdatedRouter = pUpdatedService->GetNextRouter();
	}

	//they both suppose to be NULL when the while loop is over
	if (router!=NULL || pUpdatedRouter!=NULL)
		return STATUS_ROUTER_CANT_BE_CHANGED_IN_SMCU;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::ValidateIpTypeDetails(eIpType ip_type, eIpType updated_ip_type)
{
	if (ip_type != updated_ip_type && updated_ip_type != eIpType_IpV4 
		&& updated_ip_type != eIpType_IpV6 && updated_ip_type != eIpType_Both)
		return STATUS_FAIL;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::ValidateLanPortsDetails(CPortSpeed* port_speed_vector, CPortSpeed* updated_port_speed_vector)
{
	//check DNS
	if (*port_speed_vector!=*updated_port_speed_vector)
		return STATUS_PORT_SPEED_CANT_BE_CHANGED_IN_SMCU;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::ValidateSHMIpAddress(DWORD ipv4Address, DWORD UpdatedIpv4Address)
{
	if (ipv4Address!=UpdatedIpv4Address)
		return STATUS_SHM_IP_ADDRESS_CANT_BE_CHANGED_IN_SMCU;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CMcuMngrProcess::ValidateIpAddressAndInterface(DWORD ipv4Address, DWORD UpdatedIpv4Address, string interface, string UpdatedInterface)
{
	if (ipv4Address!=UpdatedIpv4Address || interface!=UpdatedInterface)
	{
		//the ip address and interface were removed
		if (UpdatedIpv4Address==0 && UpdatedInterface=="")
			return STATUS_OK;

		//read the interface ip address
		string ip_address;
		string cmd = "echo -n `/sbin/ifconfig | grep -A1 ";
		cmd += UpdatedInterface.c_str();
		cmd += " | grep 'inet addr' | cut -d':' -f2 | cut -d' ' -f1`";
		SystemPipedCommand(cmd.c_str(), ip_address);

		if (ip_address=="")	//if no interface defined on this system
			return STATUS_INTERFACE_IS_NOT_CONFIGURED_IN_THE_SYSTEM;

		DWORD dword_ipAddress = 0;
		dword_ipAddress = SystemIpStringToDWORD(ip_address.c_str());
		if (dword_ipAddress != UpdatedIpv4Address)
			return STATUS_IP_ADDRESS_IS_DIFFERENT_FROM_THE_ONE_CONFIGURED_IN_THE_SYSTEM;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMcuMngrProcess::IsFlexeraSimulationMode()
{
	return m_isFlexeraSimulationMode;
}

LicenseFeatureStatus CMcuMngrProcess::ConvertFeatureStatus(E_FLEXERA_LICENSE_VALIDATION_STATUS type)
 {
 	switch(type)
 	{
 	case E_FLEXERA_LICENSE_VALID: return eLicenseFeatureStatus_VALID;
 	case E_FLEXERA_LICENSE_TIME_EXPIRED: return eLicenseFeatureStatus_EXPIRED;
 	case E_FLEXERA_LICENSE_WINDBACK_DETECTED: return eLicenseFeatureStatus_WINDBACKDETECTED;

 	default: return eLicenseFeatureStatus_GENERALFEATUREERROR;
 	}
 }

void CMcuMngrProcess::PrintFlexeraData()
{
	CPrettyTable<int, int, int, int, int>
	tbl("Feature", "IsEnabled", "Status", "HasChanged", "Count" );



	for(int i=0; i < MAX_NUM_OF_FEATURES ; ++i)
	{


		tbl.Add(m_flexeraCapabilitiesList.featuresArray[i].LicenseFeature,
				m_flexeraCapabilitiesList.featuresArray[i].IsEnabled,
				m_flexeraCapabilitiesList.featuresArray[i].LicenseStatus,
				m_flexeraCapabilitiesList.featuresArray[i].IsChanged,
				m_flexeraCapabilitiesList.featuresArray[i].Counted);
	}
	PTRACE(eLevelInfoNormal, tbl.Get().c_str());

}



