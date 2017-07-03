// McuState.cpp: implementation of the CMcuState class.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "McuState.h"
#include "StatusesGeneral.h"
#include "McuMngrProcess.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "SysConfig.h"
#include "SystemFunctions.h"


//#include "OsFileIF.h" // for GetNumberOfCoreDumps()
extern char* ProductTypeToString(APIU32 productType);
extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);

// colinzuo: we can modify ApiCom to add a new mode for meridian if needed, but for now I don't think we need
#ifndef eSystemCardsMode_meridian
#define eSystemCardsMode_meridian NUM_OF_SYSTEM_CARDS_MODES
#endif

// ============================================================================
// =============  converting eLicensingValidationState to string  =============
// ==  used an external function in CMcuMngrProcess::AddExtraStringsToMap()  ==
// ============================================================================
char* LicensingValidationStateToString(eLicensingValidationState validationState)
{
	switch (validationState)
	{
		case eLicensingValidationSucceeded:
		{
			return "success";
		}
	
		case eLicensingValidationFailed:
		{
			return "failure";
		}
	
		case eLicensingValidationUnknown:
		{
			return "unknown";
		}
	
		default:
		{
			return NULL;
		}
	}
	
	return NULL;
}
// ============================================================================

// ------------------------------------------------------------
CMcuState::CMcuState ()
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();

	m_numOfActiveAlarms		= 0;
	m_mplSerialNumber[0]	= '\0';
	m_validationState		= eLicensingValidationUnknown;
	m_numOfCoreDumps		= 0;//GetActualNumberOfCoreDumps();
	m_mcuStateProductType	= m_pProcess->GetProductType();
	m_mcuStateSystemCardsMode   = m_pProcess->GetRmxSystemCardsModeDefault();

	// for now, mediaRecordingState is boolean: 0 (no recording is done now) / 1 (recording(s) is done now)
	m_mediaRecordingState = 0;
	m_collectingInfoState = 0;
	m_backupState   	  = eBackup_Idle;
	m_restoreState  	  = eRestore_Success;
    m_IsSshOn = FALSE;
    m_primaryLicenseServer[0] = '\0';
    m_primaryLicenseServerPort = 3333;
}


// ------------------------------------------------------------
CMcuState::~CMcuState ()
{
}


// ------------------------------------------------------------
void  CMcuState::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n\n"
		<< "McuState::Dump\n"
		<< "--------------\n";

//	CPObject::Dump(msg);

    eMcuState mcuState = GetMcuState();
    msg << "Mcu State: " << GetMcuStateName(mcuState);
	
	msg << "MPL Serial Number: "       << m_mplSerialNumber   << "\n"
	    << "Is Validation Succeeded: " << m_validationState   << "\n"
	    << "Number of Active Alarms: " << m_numOfActiveAlarms << "\n"
	    << "Number of core dumps: "    << m_numOfCoreDumps    << "\n"
	    << "Product Type; "            << ::ProductTypeToString(m_mcuStateProductType) << "\n"
	    << "System Card Mode; "            <<   ::GetSystemCardsModeStr( m_mcuStateSystemCardsMode )<< "\n"
        << "Is Ssh On : "              << (TRUE == m_IsSshOn ? "True" : "False");
    
	msg << "\nMedia Recording now: " << (0 != m_mediaRecordingState ? "yes" : "no");
  
	msg << "\nCollecting Info now: " << (0 != m_collectingInfoState ? "yes" : "no");
	
	msg << "\nBackup in progress: " << (eBackup_InProgress != m_backupState ? "yes" : "no");
	
	msg << "\nRestore in progress: " << (eRestore_InProgress != m_restoreState ? "yes" : "no");

	msg << "\nPrimary License Server IP: " << m_primaryLicenseServer << "\n"
		<< "\nPrimary License Server Port: " << m_primaryLicenseServerPort
		<< "\nExpiration date : " << m_expirationDate;
}


// ------------------------------------------------------------
void CMcuState::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement *pMcuStateNode,*pFlexeraLicensingNode;

	pMcuStateNode = pFatherNode->AddChildNode("MCU_STATE");

	if (pMcuStateNode)
	{
		// Mcu state
        eMcuState mcuState = GetMcuState();
		pMcuStateNode->AddChildNode("ID", (WORD)mcuState);
		pMcuStateNode->AddChildNode("DESCRIPTION",(WORD)mcuState,MCU_STATE_ENUM); // as in Mgc's code

		// MPL's serial number
		pMcuStateNode->AddChildNode("MPL_SERIAL_NUMBER",m_mplSerialNumber);
		
		if (m_mcuStateProductType == eProductTypeEdgeAxis &&  m_pProcess->IsFlexeraLicenseInSysFlag() == true)
		{
			CXMLDOMElement* pFlexeraLicensingNode = pMcuStateNode->AddChildNode("LICENSING_SERVER_CONFIGURATION");
			pFlexeraLicensingNode->AddChildNode("PRIMARY_LICENSE_SERVER"     ,m_primaryLicenseServer);
			pFlexeraLicensingNode->AddChildNode("PRIMARY_LICENSE_SERVER_PORT",m_primaryLicenseServerPort);
			pFlexeraLicensingNode->AddChildNode("LICENSE_EXPIRATION_DATE"    ,m_expirationDate);
		}

		// Is validation succeeded
		pMcuStateNode->AddChildNode("LICENSING_VALIDATION_STATE",m_validationState,LICENSING_VALIDATION_ENUM);

		// num of faultElements in ActiveAlarms list
		pMcuStateNode->AddChildNode("NUMBER_OF_ACTIVE_ALARMS", m_numOfActiveAlarms);

		// num of core dumps
		pMcuStateNode->AddChildNode("NUMBER_OF_CORE_DUMPS", m_numOfCoreDumps);

		// is mediaRecording is done now
		pMcuStateNode->AddChildNode("MEDIA_RECORDING", m_mediaRecordingState);

		// is collectingInfo is done now
		pMcuStateNode->AddChildNode("COLLECTING_INFO", m_collectingInfoState);

		// product type
		pMcuStateNode->AddChildNode("PRODUCT_TYPE",(WORD)m_mcuStateProductType,PRODUCT_TYPE_ENUM);

		// system card mode
		if ((m_mcuStateSystemCardsMode == eSystemCardsMode_mpmrx) && (eProductTypeNinja == m_mcuStateProductType))
		{
			pMcuStateNode->AddChildNode("SYSTEM_CARDS_MODE",eSystemCardsMode_meridian, SYSTEM_CARDS_MODE_ENUM);
		}
		else
		{
			pMcuStateNode->AddChildNode("SYSTEM_CARDS_MODE",m_mcuStateSystemCardsMode, SYSTEM_CARDS_MODE_ENUM);
		}
        // is ssh on
		pMcuStateNode->AddChildNode("SSH", m_IsSshOn, _BOOL);

		// for startup progress bar
		int startupDurationTotalInSeconds		= m_pProcess->GetMaxTimeForStartup() / SECOND;
		int startupDurationReminingInSeconds	= m_remainingTimeForStartup / SECOND;

		CXMLDOMElement* pStartupDurationNode = pMcuStateNode->AddChildNode("SYSTEM_STARTUP_DURATION");
		pStartupDurationNode->AddChildNode("SYSTEM_STARTUP_DURATION_TOTAL_SECONDS", startupDurationTotalInSeconds);
		pStartupDurationNode->AddChildNode("SYSTEM_STARTUP_DURATION_REMAINING_SECONDS", startupDurationReminingInSeconds);
			
		// is backup in progress now
		pMcuStateNode->AddChildNode("BACKUP_STATE", m_backupState, BACKUP_TYPE_ENUM);
		
		// is restore in progress now
		pMcuStateNode->AddChildNode("RESTORE_STATE", m_restoreState, RESTORE_TYPE_ENUM);	

		// is installer during installation
		m_pProcess->m_installPhaseList.SerializeXml(pMcuStateNode);
	}
}


// ------------------------------------------------------------
int	 CMcuState::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
    PASSERTMSG(TRUE, "CMcuState::DeSerializeXml - this function should not be called");
    return 0;


    
//     int *deathPtr = (int*)0xdeadbeaf;
//     *deathPtr = 42;
    
    
// 	STATUS nStatus = STATUS_OK;
// 	WORD tmp=0;

// 	GET_VALIDATE_CHILD(pActionNode, "ID", &tmp, _0_TO_WORD);
// 	SetMcuState((eMcuState)tmp);
// 	GET_VALIDATE_CHILD(pActionNode,"DESCRIPTION",&tmp,MCU_STATE_ENUM); // as in Mgc's code
// 	GET_VALIDATE_CHILD(pActionNode,"MPL_SERIAL_NUMBER", m_mplSerialNumber, _0_TO_MPL_SERIAL_NUM_LENGTH);
// 	GET_VALIDATE_CHILD(pActionNode,"LICENSING_VALIDATION_STATE", (WORD*)m_validationState, LICENSING_VALIDATION_ENUM);
// 	GET_VALIDATE_CHILD(pActionNode,"NUMBER_OF_ACTIVE_ALARMS", &m_numOfActiveAlarms,     _0_TO_WORD);
// 	GET_VALIDATE_CHILD(pActionNode,"NUMBER_OF_CORE_DUMPS", &m_numOfCoreDumps,     _0_TO_DWORD);
// 	GET_VALIDATE_CHILD(pActionNode,"MEDIA_RECORDING", &m_mediaRecordingState,     _0_TO_DWORD);
// 	GET_VALIDATE_CHILD(pActionNode,"COLLECTING_INFO", &m_collectingInfoState,     _0_TO_DWORD);
//  GET_VALIDATE_CHILD(pActionNode,"BACKUP_STATE", &m_backupState,     BACKUP_TYPE_ENUM);
//  GET_VALIDATE_CHILD(pActionNode,"RESTORE_STATE", &m_restoreState,   RESTORE_TYPE_ENUM);    
// 	GET_VALIDATE_CHILD(pActionNode,"PRODUCT_TYPE", (WORD*)m_mcuStateProductType, PRODUCT_TYPE_ENUM);
//     GET_VALIDATE_CHILD(pActionNode,"SSH", &m_IsSshOn, _BOOL);

// 	return nStatus;
}


// ------------------------------------------------------------
CMcuState& CMcuState::operator = (const CMcuState &rOther)
{
    if(this == &rOther)
    {
        return *this;
    }
    
//	m_mcuState				= rOther.m_mcuState;
	strncpy(m_mplSerialNumber, rOther.m_mplSerialNumber, MPL_SERIAL_NUM_LEN );
	m_validationState		= rOther.m_validationState;
	m_numOfActiveAlarms		= rOther.m_numOfActiveAlarms;
	m_numOfCoreDumps		= rOther.m_numOfCoreDumps;
	m_mcuStateProductType	= rOther.m_mcuStateProductType;
	m_mcuStateSystemCardsMode	= rOther.m_mcuStateSystemCardsMode;
	m_mediaRecordingState	= rOther.m_mediaRecordingState;
	m_collectingInfoState	= rOther.m_collectingInfoState;
	m_backupState			= rOther.m_backupState;
	m_restoreState			= rOther.m_restoreState;
	m_primaryLicenseServer      = rOther.m_primaryLicenseServer;
	m_primaryLicenseServerPort	= rOther.m_primaryLicenseServerPort;
	m_expirationDate            = rOther.m_expirationDate;

	return *this;
}


// ------------------------------------------------------------
void CMcuState::SetNumOfActiveAlarms(WORD activeAlarmsNum)
{
	m_numOfActiveAlarms = activeAlarmsNum;
}


// ------------------------------------------------------------
WORD CMcuState::GetNumOfActiveAlarms()
{
	return m_numOfActiveAlarms;
}


// ------------------------------------------------------------
void CMcuState::ClearSerialNumber()
{
	memset( m_mplSerialNumber,
	        0,
	        MPL_SERIAL_NUM_LEN );
}


// ------------------------------------------------------------
void CMcuState::SetMplSerialNumber(char* serialNum)
{
	strncpy( m_mplSerialNumber,
	         serialNum,
	         sizeof(m_mplSerialNumber) - 1);
	m_mplSerialNumber[sizeof(m_mplSerialNumber) - 1] = '\0';
}


// ------------------------------------------------------------
char* CMcuState::GetMplSerialNumber()
{
	return m_mplSerialNumber;
}


// ------------------------------------------------------------
void CMcuState::SetMcuState(const string &caller, eMcuState mcuState)
{	
	CProcessBase::GetProcess()->SetSystemState(caller, mcuState);
}


// ------------------------------------------------------------
eMcuState CMcuState::GetMcuState()const
{
    eMcuState mcuState = CProcessBase::GetProcess()->GetSystemState();
	return mcuState;
}


// ------------------------------------------------------------
void CMcuState::SetValidationState(eLicensingValidationState validationState)
{
	m_validationState = validationState;
}


// ------------------------------------------------------------
eLicensingValidationState CMcuState::GetValidationState()
{
	return m_validationState;
}



// ------------------------------------------------------------
void CMcuState::IncreaseNumOfCoreDumps()
{
	BOOL is_core_dump_notification_enabled = FALSE;

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_COREDUMP_NOTIFICATIONS, is_core_dump_notification_enabled);  // the default value is "NO"

	if (is_core_dump_notification_enabled == TRUE)
		m_numOfCoreDumps++;

	if (10000 <= m_numOfCoreDumps) // just for protection
		m_numOfCoreDumps = 0;
}


// ------------------------------------------------------------
void CMcuState::SetNumOfCoreDumps(DWORD numOfCores)
{
	m_numOfCoreDumps = numOfCores;
}


// ------------------------------------------------------------
DWORD CMcuState::GetNumOfCoreDumps()
{
	return m_numOfCoreDumps;
}

DWORD CMcuState::GetActualNumberOfCoreDumps()
{
  DWORD numOfCores = 0;

  if (YES == IsTarget())
  {
    std::string answer;
    std::string coreDumpsNumCmd = "ls -1 ";
    coreDumpsNumCmd += CORE_DUMPS_FOLDER;
    coreDumpsNumCmd += " | wc -l"; // coreDumpsNumCmd = "ls -1 MCU_OUTPUT_DIR/core | wc -l"

    SystemPipedCommand(coreDumpsNumCmd.c_str(), answer);

    numOfCores = atoi(answer.c_str());
  }

  return numOfCores;
}

void CMcuState::SetMcuStateProductType(eProductType theType)
{
	m_mcuStateProductType = theType;
}

eProductType CMcuState::GetMcuStateProductType()
{
	return m_mcuStateProductType;
}

// ------------------------------------------------------------
void CMcuState::SetMcuStateSystemCardsMode(eSystemCardsMode theMode)
{

	m_mcuStateSystemCardsMode = theMode;

		// EMA should not show 'illegal'; instead, it should display the default
		if (eSystemCardsMode_illegal == m_mcuStateSystemCardsMode)
			m_mcuStateSystemCardsMode =  m_pProcess->GetRmxSystemCardsModeDefault();
}


// ------------------------------------------------------------
eSystemCardsMode CMcuState::GetMcuStateSystemCardsMode() const
{
	return m_mcuStateSystemCardsMode;
}


// ------------------------------------------------------------
void CMcuState::SetMediaRecordingState(DWORD mediaRecording)
{
	m_mediaRecordingState = mediaRecording;
}


// ------------------------------------------------------------
DWORD CMcuState::GetMediaRecordingState()
{
	return m_mediaRecordingState;
}

// ------------------------------------------------------------
void CMcuState::SetCollectingInfoState(DWORD collectingInfo)
{
	m_collectingInfoState = collectingInfo;
}


// ------------------------------------------------------------
DWORD CMcuState::GetCollectingInfoState()
{
	return m_collectingInfoState;
}
// ------------------------------------------------------------

void CMcuState::SetBackupState(BYTE progress)
{
	m_backupState = (eBackupProgressType)progress;
}

// ------------------------------------------------------------
eBackupProgressType CMcuState::GetBackupState() 
{ 
	return m_backupState;
}

// ------------------------------------------------------------
void CMcuState::SetRestoreState(BYTE progress)
{
	m_restoreState = (eRestoreProgressType)progress;
}

// ------------------------------------------------------------
eRestoreProgressType CMcuState::GetRestoreState() 
{ 
	return m_restoreState; 
}

// ------------------------------------------------------------
void CMcuState::SetIsSshOn(BOOL val)
{
    m_IsSshOn = val;
}

// ------------------------------------------------------------
BOOL CMcuState::GetIsSshOn()const
{
    return m_IsSshOn;
}

// ------------------------------------------------------------
void CMcuState::SetRemainingTimeForStartup(int val)
{
	m_remainingTimeForStartup = val;
}

// ------------------------------------------------------------
int CMcuState::GetRemainingTimeForStartup()const
{
	return m_remainingTimeForStartup;
}

// ------------------------------------------------------------
void CMcuState::ClearPrimaryLicenseServer()
{
	m_primaryLicenseServer = "";
	m_primaryLicenseServerPort = 0;
}

// ------------------------------------------------------------
void CMcuState::SetPrimaryLicenseServer(std::string primaryLicenseSrver)
{
	m_primaryLicenseServer = primaryLicenseSrver;

}


// ------------------------------------------------------------
std::string CMcuState::GetPrimaryLicenseServer()
{
	return m_primaryLicenseServer;
}

// ------------------------------------------------------------
void CMcuState::SetPrimaryLicenseServerPort(DWORD portNum)
{
	m_primaryLicenseServerPort = portNum;
}


// ------------------------------------------------------------
DWORD CMcuState::GetPrimaryLicenseServerPort()
{
	return m_primaryLicenseServerPort;
}
void CMcuState::SetExpirationDate(const CStructTm &other)
{
	m_expirationDate = other;
}

 CStructTm  CMcuState::GetExpirationDate(void)
{
  return m_expirationDate;
}


