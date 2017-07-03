// IPMCInterfaceManager.cpp: implementation of the CIPMCInterfaceManager class.
//
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <libgen.h>

#include "IPMCInterfaceManager.h"
#include "IPMCInterfaceOpcodes.h"
#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "IPMC.h"
#include "IpmcInt.h"
#include "serial.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include <sys/signal.h>
#include "OsFileIF.h"
#include "IncludePaths.h"
#include "FaultsDefines.h"
#include "ObjString.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesRanges.h"
#include "TerminalCommand.h"
#include "SysConfigKeys.h"
#include <upgdefs.h>

#include <upgrade_img.h>
#undef SLEEP
#include <upgrade_io.h>

#define DIAGNOSTICS_WATCHDOG_TIME_INTERVAL 20

#define SYSTEM_IPMC_RESET_IND_FILE          "States/ResetIPMCUpdate"

#define IPMC_FAIL_CHIP_READ     1
#define IPMC_FAIL_BUILD_READ    2
#define IPMC_NEED_UPGRADE       3
#define IPMC_NO_NEED_UPGRADE    4

/* upgrade block size */
int blk_size = 20;

static const char * GetReadIPMCErrorStr(int error);

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CIPMCInterfaceManager)
    ONEVENT(XML_REQUEST    			, IDLE , CIPMCInterfaceManager::HandlePostRequest )
    ONEVENT(IPMC_INITIALIZE_LEDS	, IDLE , CIPMCInterfaceManager::OnInitializeLeds)
    ONEVENT(IPMC_CHANGE_LED_STATE	, IDLE , CIPMCInterfaceManager::OnChangeLedState)
    ONEVENT(IPMC_SET_WATCHDOG 		, IDLE , CIPMCInterfaceManager::OnSetWatchdog)
    ONEVENT(IPMC_TRIGGER_WATCHDOG 	, IDLE , CIPMCInterfaceManager::OnTriggerWatchdog)
    ONEVENT(IPMC_TURN_OFF_WATCHDOG 	, IDLE , CIPMCInterfaceManager::OnTurnOffWatchdog)
    ONEVENT(IPMC_SET_POST_COMMAND 	, IDLE , CIPMCInterfaceManager::OnSetPostCommand)
    ONEVENT(IPMC_CPU_TEMPERATURE 	, IDLE , CIPMCInterfaceManager::OnSetCPUTemperature)
    ONEVENT(IPMC_HDD_TEMPERATURE 	, IDLE , CIPMCInterfaceManager::OnSetHardDriveTemperature)
    ONEVENT(IPMC_UPGRADE_VERSION 	, IDLE , CIPMCInterfaceManager::OnUpgradeVersion)
    ONEVENT(CHECK_IPMC_VERSION_TOUT	, IDLE , CIPMCInterfaceManager::OnCheckIpmcVersionTout)
    ONEVENT(CHANGE_LED_STATE_TIMER	, IDLE , CIPMCInterfaceManager::OnChangeLedStateTimeout)

PEND_MESSAGE_MAP(CIPMCInterfaceManager,CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CIPMCInterfaceManager)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CIPMCInterfaceManager::HandleOperLogin)
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CIPMCInterfaceManager)
  ONCOMMAND("get_version",		CIPMCInterfaceManager::HandleGetVersion,"get the ipmc software version")
  ONCOMMAND("upgrade_version",	CIPMCInterfaceManager::HandleUpgradeVersion,"burn the ipmc software version")
  ONCOMMAND("command",	CIPMCInterfaceManager::HandleIpmcCommand,"send direct IPMC command")
END_TERMINAL_COMMANDS

extern void IPMCInterfaceMonitorEntryPoint(void* appParam);
//extern int LoadIpmcSoftware(int argc, char **argv) ;


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void IPMCInterfaceManagerEntryPoint(void* appParam)
{
	CIPMCInterfaceManager * pIPMCInterfaceManager = new CIPMCInterfaceManager;
	pIPMCInterfaceManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CIPMCInterfaceManager::GetMonitorEntryPoint()
{
	return IPMCInterfaceMonitorEntryPoint;
}


//////////////////////////////////////////////////////////////////////
void PrintScreen(const char *message, int secondsToSleep)
{
    cerr << message << endl;
    SystemSleep(secondsToSleep);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CIPMCInterfaceManager::CIPMCInterfaceManager()
{

  m_disable_IPMC_usage = FALSE;
  for (int i=0 ; i<LAST_LED_COLOR;i++)
	  m_arrayLastConfigLedsState[i] = eNoState;

  for (int i=0 ; i<LAST_LED_COLOR;i++)
	  m_arrayLastReqLedsState[i] = eNoState;
}

//////////////////////////////////////////////////////////////////////
CIPMCInterfaceManager::~CIPMCInterfaceManager()
{

}

//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::ManagerPostInitActionsPoint()
{
    if (!IsTarget())
    {
        return;
    }

    CheckIPMCVersion();

    SetPostCommand(0);

    OnInitializeLeds();
}

//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::CheckIPMCVersion(bool bByRequest, bool bForceBurn)
{
    COstrStream msg;

	int exitCode = 0;
	DWORD chip = 0;
	DWORD image = 0;

	WORD isUpgradeNeeded = IsUpgradeNeeded(chip,image, msg, bByRequest);
	if (IPMC_NEED_UPGRADE == isUpgradeNeeded || bForceBurn)
	{
		// Next IPMC startup it will reset the entire system.
		// it must be done through IPMC
		//This is done only if IPMC is burned while system is in startup, and not while system upgrades normally.
		if(IsBurningWhileStartup(bByRequest))
		{
			cerr << "IPMCProcess: Old upgarde flow - sleeping 4 minutes to enable switch to access all needed files." << endl;
			SystemSleep(4*60*SECOND);	// Let embedded take all their files before starting to burn ipmc - 4 minutes wait
			cerr << "IPMCProcess: Old upgarde flow - finished sleeping 4 minutes, starting ipmc burn cycle." << endl;
			MarkNextSystemReset();
		}

		PrintScreen(msg.str().c_str(), 5);

		// this line is last
		// the IPMC card will reset without any warning.
		UpgradeIPMCVersion(bByRequest, bForceBurn);

		exitCode = 1;
	}
	else
	{
		//Send progress data to Installer process
		CSegment*  pParam = new CSegment;
		const COsQueue* pInstallerMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessInstaller, eManager);
		pInstallerMbx->Send(pParam, IPMCIF_TO_INSTALLER_NO_UPGRADE_NEEDED);

		// this happens after updating of IPMC version (MarkNextSystemReset)
		// there should be reset through IPMC
		if(IsFileExists(SYSTEM_IPMC_RESET_IND_FILE))
		{
			DeleteFile(SYSTEM_IPMC_RESET_IND_FILE);
			// this line is last
			ResetThroughIPMC();
			return;
		}
	}

	TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::CheckIPMCVersion: IsUpgradeNeeded returned: " << msg.str();

	// this happens before MCMS startup.
	if(IsUpgradeIPMCMode() && !bByRequest)
	{
		cerr << "IPMCProcess: EXITING WITH CODE " << exitCode << endl;
		TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::CheckIPMCVersion: " << "IPMCProcess: EXITING WITH CODE " << exitCode;
		exit(exitCode);
	}

    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    sysConfig->GetBOOLDataByKey("DISABLE_IPMC_USAGE", m_disable_IPMC_usage);
    // this happens in normal startup.
	isUpgradeNeeded = IsUpgradeNeeded(chip,image, msg, bByRequest);
	if(IPMC_NO_NEED_UPGRADE != isUpgradeNeeded)
	{
		StartTimer(CHECK_IPMC_VERSION_TOUT, 15 * SECOND);
	}
}

//////////////////////////////////////////////////////////////////////
bool CIPMCInterfaceManager::IsBurningWhileStartup(bool bByRequest)
{
	bool result = FALSE;

	//During system startup
	if(IsUpgradeIPMCMode() || !bByRequest)
	{
		result = TRUE;
	}

	return result;
}

//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnCheckIpmcVersionTout(CSegment *pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::OnCheckIpmcVersionTout: " << m_count_failures;
	COstrStream msg;

	DWORD chip = 0;
	DWORD image = 0;

	WORD isUpgradeNeeded = IsUpgradeNeeded(chip,image, msg, FALSE);
	if(IPMC_NO_NEED_UPGRADE != isUpgradeNeeded)
	{
		if(m_count_failures < 20)
		{
			++m_count_failures;
			StartTimer(CHECK_IPMC_VERSION_TOUT, 15 * SECOND);
		}

		else
		{
			AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
					CPU_IPCM_SOFTWARE_IS_NOT_UPDATED,
					MAJOR_ERROR_LEVEL,
					msg.str(),
					true,
					true);
		}
	}
}



//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::MarkNextSystemReset()
{
    BOOL res = CreateFile(SYSTEM_IPMC_RESET_IND_FILE);
    CLargeString message;
    message << "IPMC " << (res ? "OK " : "FAILED ")
            << "create IPMC reset Ind file : " << SYSTEM_IPMC_RESET_IND_FILE  << "\n";
    PrintScreen(message.GetString(), 5);
}

//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::ResetThroughIPMC()
{
    PrintScreen("Enter to IPMC reset scenario", 5);

    CSegment *pSeg = new CSegment;
    //IPMC - can not be set to a value greater than 25 seconds!
    DWORD timeInterval = 25;
    *pSeg << timeInterval;
    OnSetWatchdog(pSeg);

    OnTriggerWatchdog();
    PrintScreen("Trigger HW WD", 5);

    // just wait for IPMC reset
    SystemSleep(NEVER);
}

//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnTriggerWatchdog()
{
    int res =0;
    //Only When The Interface Will Be in The Version //
    if (IsTarget())
        res = ResetWatchDogFunc();
    TRACESTR(res ? eLevelError:eLevelInfoNormal) <<
            "\nCIPMCInterfaceManager::OnTriggerWatchDog";

}
//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnInitializeLeds()
{
    int res =0 ;
    //Only When The Interface Will Be in The Version //
    if (IsTarget())
        res = IpmcGetFruLedProperties();

    TRACESTR(res ? eLevelError:eLevelInfoNormal) <<
            "\nCIPMCInterfaceManager::OnInitializeLeds";


}
////////////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::SetLastConfigLedColorState(const eLedColor  color,const eLedState  state)
{
	m_arrayLastConfigLedsState[color] = state;
}
////////////////////////////////////////////////////////////////////////////
eLedState CIPMCInterfaceManager::GetLastConfigLedColorState(const eLedColor  color) const
{
	return m_arrayLastConfigLedsState[color] ;
}






////////////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::SetLastReqLedColorState(const eLedColor  color,const eLedState  state)
{
	m_arrayLastReqLedsState[color] = state;
}
////////////////////////////////////////////////////////////////////////////
eLedState CIPMCInterfaceManager::GetLastReqLedColorState(const eLedColor  color) const
{
	return m_arrayLastReqLedsState[color] ;
}



//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnChangeLedState(CSegment *pParam)
{
	DWORD  Color=0;
	DWORD  State=0;
	int res=0;

	*pParam >> Color;
	*pParam >> State;


	SetLastReqLedColorState((eLedColor)Color,(eLedState)State);
	TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::OnChangeLedState- timer CHANGE_LED_STATE_TIMER is on"
	<<" we do not set ipmc. the last color req is "<<Color << " \nThe last state is " << State;



	if ( IsValidTimer(CHANGE_LED_STATE_TIMER))
		return;
	else

		// we do the setting after timer expired
		// if (IsTarget())
		//	  res = LedInterface(Color , State);

		StartTimer(CHANGE_LED_STATE_TIMER, SECOND);
}

//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnChangeLedStateTimeout()
{
	//TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::OnChangeLedStateTimeout - delete timer";

	for (int i=0 ; i< LAST_LED_COLOR ;i++)
	{
		eLedState lastColorStateReq =  GetLastReqLedColorState((eLedColor)i);
		TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::OnChangeLedStateTimeout - " << "color "
		<< i << " state "<< lastColorStateReq;

		if  (lastColorStateReq == eNoState)
			continue;

		if ( lastColorStateReq != GetLastConfigLedColorState((eLedColor)i))

		{

			int res=0;

			//eLedState lastStateReq =  GetLastReqLedColorState((eLedColor)i);

			if (IsTarget())
				res = LedInterface((DWORD)i ,(DWORD) lastColorStateReq);
			TRACESTR(res? eLevelError:eLevelInfoNormal) <<
			"\nCIPMCInterfaceManager::OnChangeLedStateTimeout -  from: " <<
			i << " and State: " << lastColorStateReq;

			SetLastConfigLedColorState((eLedColor)i,lastColorStateReq) ;

		}
	}

	DeleteTimer(CHANGE_LED_STATE_TIMER);

}
//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnSetWatchdog(CSegment *pParam)
{
    DWORD Time_Interval=0;
    int res = eIpmcSerialOpenFail;
    *pParam >> Time_Interval;
    BOOL writeToFile, resStat;
    resStat = CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_WRITE_LOG_TO_FILE, writeToFile);
    PASSERTSTREAM_AND_RETURN(!resStat, "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_ENABLE_WRITE_LOG_TO_FILE);
    if ( (IsTarget() ) && ( m_disable_IPMC_usage == FALSE) )
    {
    	while(eIpmcSuccess != res)
    	{
    		SystemSleep(100);

    		res = SetWatchDogFunc(Time_Interval*10);
		if(writeToFile)
    		{
    			std::ostringstream cmd;
    			cmd <<"IPMCInterfaceManager::OnSetWatchdog to" << Time_Interval <<" Seconds, res=" << res;
    			PrintErrorToLocalFile(cmd);
    		}
    		TRACESTR(res? eLevelError:eLevelInfoNormal) <<
    				"\nCIPMCInterfaceManager::OnSetWatchdog to  " << Time_Interval <<" Seconds, res=" << res;
    	}
    }

}
//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnTurnOffWatchdog()
{
    int res =0 ;
    //Only When The Interface Will Be in The Version //
    if (IsTarget())
        res = TurnOffWatchDogFunc();
    TRACESTR(res? eLevelError:eLevelInfoNormal) <<
        "\nCIPMCInterfaceManager::OnTurnOffWatchdog";
}
//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnSetCPUTemperature(CSegment *pParam)
{
    DWORD CPUTemperature = 0;
    int res = 0;
    *pParam >> CPUTemperature;
    //INT8 cReqBuf[100];
    //sprintf(cReqBuf,"%s %02x]\n","[48 00 0F",CPUTemperature) ;
    //cerr << cReqBuf;
    if (IsTarget())
        res = SetIPMCCPUTemperature(CPUTemperature);

    //TODO - remove the print when temperature reading is stable.
    TRACESTR(res? eLevelError:eLevelInfoNormal) <<
        "\nCIPMCInterfaceManager::OnSetCPUTemperature " << CPUTemperature;
}
//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnSetHardDriveTemperature(CSegment *pParam)
{
    DWORD HardDriveTemperatue= 0;
    int res = 0 ;
    *pParam >> HardDriveTemperatue;

    //INT8 cReqBuf[100];
    //sprintf(cReqBuf,"%s %02x]\n","[48 00 0F",HardDriveTemperatue) ;
    //cerr << cReqBuf;

    if (IsTarget())
        res = SetIPMCHardDriveTemperature(HardDriveTemperatue);

    TRACESTR(res? eLevelError:eLevelInfoNormal) <<
        "\nCIPMCInterfaceManager::OnSetHardDriveTemperature " << HardDriveTemperatue;
}
//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnSetPostCommand(CSegment *pParam)
{
    DWORD command = 0;
    int res = 0 ;
    *pParam >> command;
    if (IsTarget())
        res = SetPostCommand(command);

    TRACESTR(res? eLevelError:eLevelInfoNormal) <<
        "\nCIPMCInterfaceManager::OnSetPostCommand";
}

/////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::OnUpgradeVersion(CSegment *pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::OnUpgradeVersion";
	WORD force = FALSE;

	if (pParam)
		*pParam >> force;

	//Force burning ipmc
	CheckIPMCVersion(TRUE, force);
}

/////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceManager::HandleUpgradeVersion(CTerminalCommand & command,std::ostream& answer)
{
	const string &bulkSize = command.GetToken(eCmdParam1);

	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd upgrade_version IPMCInterface [bulkSize]\n";
		return STATUS_FAIL;
	}

	blk_size = atoi(bulkSize.c_str());

	CSegment *pParam = new CSegment;
	*pParam << (WORD)TRUE;

	OnUpgradeVersion(pParam);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////
const char * GetReadIPMCErrorStr(int error)
{
#define IPMC_ERR_NAME(err) #err

    const char *name = "uknown";
    switch(error)
    {
        case eIpmcSerialOpenFail:
            name = IPMC_ERR_NAME(eIpmcSerialOpenFail);
            break;

        case eIpmcSerialUpdateIRQFail:
            name = IPMC_ERR_NAME(eIpmcSerialOpenFail);
            break;

        case eIpmcSerialSetParamFail:
            name = IPMC_ERR_NAME(eIpmcSerialOpenFail);
            break;

        case eIpmcOpenInterfaceFail:
            name = IPMC_ERR_NAME(eIpmcSerialOpenFail);
            break;

        case eIpmcWriteIpmcFail:
            name = IPMC_ERR_NAME(eIpmcSerialOpenFail);
            break;

        case eIpmcRcvHexResponseFail:
            name = IPMC_ERR_NAME(eIpmcRcvHexResponseFail);
            break;


        case eIpmcGetHWSlotIdFail:
            name = IPMC_ERR_NAME(eIpmcGetHWSlotIdFail);
            break;

        case eIpmcReadEepromFail:
            name = IPMC_ERR_NAME(eIpmcReadEepromFail);
            break;

        case eIpmcGetFruLedPropertiesFail:
            name = IPMC_ERR_NAME(eIpmcGetFruLedPropertiesFail);
            break;

        case eIpmcGetLedColorCapabilitiesFuncFail:
            name = IPMC_ERR_NAME(eIpmcGetLedColorCapabilitiesFuncFail);
            break;

        case eIpmcReadIPMCVersionFromChipFail:
            name = IPMC_ERR_NAME(eIpmcReadIPMCVersionFromChipFail);
            break;

        case eIpmcSuccess:
            name = IPMC_ERR_NAME(eIpmcGetLedColorCapabilitiesFuncFail);
            break;
    };
    return name;
}


//////////////////////////////////////////////////////////////////////
int CIPMCInterfaceManager::IsUpgradeNeeded(DWORD &outVersionOnChip,
                                            DWORD &outVersionOnBuild,
                                            COstrStream & msg,
                                            bool bByRequest)
{
    if(!IsTarget())
    {
      return IPMC_NEED_UPGRADE;
    }

    // 1. Get version from chip
    char chipVersion[7] = "";
    int res = ReadIPMCVersionFromChip(chipVersion);
    chipVersion[6] = '\0';
    if(0 != res)
    {
        const char *failedReadMsg = "FAILED READING IPMC version from CHIP";
        msg << failedReadMsg << ", err = "<< res << ":" << GetReadIPMCErrorStr(res);
        cerr << "IPMCProcess: " << failedReadMsg << ", err = "<< res << ":" << GetReadIPMCErrorStr(res) << endl;

        return IPMC_FAIL_CHIP_READ;
    }
    DWORD ipmcChipVersion = TranslateChipVersionStringToNumber(chipVersion);


    // 2. Get version from build(image)
    DWORD ipmcBuildVersion = 0;
    res = GetIPMCVersionFromBuild(ipmcBuildVersion, bByRequest);
    if(0 != res)
    {
        const char *failedReadMsg = "FAILED READING IPMC version from Build\n";
        msg << failedReadMsg;
        cerr << "IPMCProcess: " << failedReadMsg << endl;

        return IPMC_FAIL_BUILD_READ;
    }

    cerr << "IPMCProcess: Version from CHIP:" << chipVersion << " - " << ipmcChipVersion << endl;
    cerr << "IPMCProcess: IPMC from build:" << ipmcBuildVersion << endl;

    outVersionOnChip = ipmcChipVersion;
    outVersionOnBuild = ipmcBuildVersion;


    // 3. compare versions Chip vs. Build
    msg << "\rVersion from CHIP : " << outVersionOnChip
        << "\rIPMC from build : " << outVersionOnBuild;

    int retStatus = IPMC_NO_NEED_UPGRADE;
    if(outVersionOnChip != outVersionOnBuild)
    {
        msg << "\r -> IPMC upgrade needed";
        retStatus = IPMC_NEED_UPGRADE;
    }

    TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::IsUpgradeNeeded " << msg.str();

    return retStatus;
}

//////////////////////////////////////////////////////////////////////
DWORD CIPMCInterfaceManager::TranslateChipVersionStringToNumber(char *Version_String)
{
    strncpy(&Version_String[1],&Version_String[2], 1);
    strncpy(&Version_String[2], &Version_String[4] ,1);
    strncpy(&Version_String[3], "\0", 1);
    return(atoi(Version_String));
}

//////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceManager::GetIPMCVersionFromBuild(DWORD &versionNumber, bool bByRequest)
{
    versionNumber = 0;
    char buf[255] = {0};
    char *base = NULL;
    int size = 0;

    //During system startup
	if(IsUpgradeIPMCMode() || !bByRequest)
	{
		TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::GetIPMCVersionFromBuild link=CpuIpmc";
		size = readlink("CpuIpmc", buf, 255);
	}
	//else, During system upgrade (link created by Scripts/ExposeNewEmbeddedVersion.sh
	else
	{
		std::string fname = MCU_TMP_DIR+"/upgrade_CpuIpmc";
		TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::GetIPMCVersionFromBuild link="<< fname.c_str();
		size = readlink(fname.c_str(), buf, 255);
	}

    if (size == -1)
    {
        return STATUS_FAIL;
    }

    base = basename(buf);

    TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::GetIPMCVersionFromBuild base=" << base;

    while (*base != 0)
    {
        if (*base >= '0' && *base <= '9')
        {
            versionNumber *= 10;
            versionNumber += *base - '0';
        }

        base++;
    }
    if (versionNumber == 0)
        return STATUS_FAIL;
    else
        return STATUS_OK;

}
//////////////////////////////////////////////////////////////////////
BOOL CIPMCInterfaceManager::IsUpgradeIPMCMode()
{
    std::string Answer;
    SystemPipedCommand("pidof McmsDaemon",Answer);
    bool result = FALSE;
 //   if(Answer=="\n")
    if(Answer=="")
    	result = TRUE;
    TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::IsUpgradeIPMCMode Answer=" << Answer << ", result=" << result;
    return result;
}
//////////////////////////////////////////////////////////////////////
void CIPMCInterfaceManager::UpgradeIPMCVersion(bool bByRequest, bool bForceBurn)
{
	if (IsTarget())
	{


		int res =0;
		//During system startup
		if(IsUpgradeIPMCMode() || !bByRequest || bForceBurn)
		{
			std::string cpuIpmcPath = MCU_MCMS_DIR+"/CpuIpmc";
			char *startupUpgradeCmd[5] = {"upgradefw","-s -v","ttyS0",(char *)(cpuIpmcPath.c_str()),NULL};
			res = LoadIpmcSoftware(4,(char **)startupUpgradeCmd);
		}
		//else, During system upgrade
		else
		{
			std::string upgradeCpuIpmc= MCU_TMP_DIR+"/upgrade_CpuIpmc";
			char *upgradeCmd[5] = {"upgradefw","-s -v","ttyS0",(char *)(upgradeCpuIpmc.c_str()),NULL};
			res = LoadIpmcSoftware(4,(char **)upgradeCmd);
		}

		if (0==res)
		{
			cerr << "IPMC UPGRADE Succeeded" << endl;
		}
		else
		{
			cerr << "IPMC Upgrade Failed"<<endl;
		}
	}
	else
	{
		for (int i=0; i<=100; i++)
		{
			//Send progress data to Installer process
			CSegment*  pParam = new CSegment;
			*pParam << (DWORD)i;
			const COsQueue* pInstallerMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessInstaller, eManager);
			pInstallerMbx->Send(pParam, IPMCIF_TO_INSTALLER_UPGRADE_PROGRESS);
			SystemSleep(25);
		}
		// simulate ipmc burning indication
	}
}


//////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceManager::HandleIpmcCommand(CTerminalCommand &command,
		std::ostream& answer)
{
    DWORD IPMCBuildVersion = 0,IPMCChipVersion = 0;
    char ChipVersion[6] = "";
    char pResponse[2024]="\0";;

    int res = 0;
    if (IsTarget())
    {

    	const DWORD numOfParams = command.GetNumOfParams();
    	if(numOfParams != 1)
    	{
    		answer << "Error 1" << endl;
    		answer << "Usage: ca command IPMCInterface [5000XX]";
    		return STATUS_OK;
    	}

    	std::string data = command.GetToken(eCmdParam1);
    	if (data.length()<8)
    	{
       		answer << "Error 2" << endl;
        	answer << "Usage: ca command IPMCInterface [5000XX]";
        	return STATUS_OK;

    	}
    	else
    	{

			if (data.find("[5000")!=0)
			{
				answer << "Error 3" << endl;
				answer << "Usage: ca command IPMCInterface [5000XX]";
				return STATUS_OK;
			}
			if (data.find("]")!=(data.length()-1))
			{
				answer << "Error 4" << endl;
				answer << "Usage: ca command IPMCInterface [5000XX]";
				return STATUS_OK;
			}
    	}




     	answer << "\nCommand " << data.c_str() << " sent to IPMC controller";

     	TRACESTR(eLevelInfoNormal) << "\nCIPMCInterfaceManager::HandleIpmcCommand cmd=" << command.GetNumOfParams();
       	std::string formatedData=data+"\r";
     	res = IpmcSendCommand((char *)formatedData.c_str(),&(pResponse[0]));

        if (13==res)
        {
        	answer << "\nIPMC command " << data.c_str() << "- sent OK";
        	answer << "\nIPMC Response:\n "<< pResponse;
        	//res = GetIPMCVersionFromBuild(IPMCBuildVersion);
        }
        else
        {
        	answer << "\nIPMC command " << data.c_str() << " - failed to send.res= " << res;
        	return STATUS_FAIL;
        }

        /*if (0 == res)
        {
            IPMCChipVersion=TranslateChipVersionStringToNumber(ChipVersion);
        }
        answer << "version on chip: " << IPMCChipVersion << " version on build: " << IPMCBuildVersion;*/
    }
    else
    {
        //res = GetIPMCVersionFromBuild(IPMCBuildVersion);
    	answer << "IPMC command not available in simulation mode";
    }

    return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceManager::HandleGetVersion(CTerminalCommand &command,
		std::ostream& answer)
{
    DWORD IPMCBuildVersion = 0,IPMCChipVersion = 0;
    char ChipVersion[6] = "";
    int res = 0;
    if (IsTarget())
    {
        res = ReadIPMCVersionFromChip(ChipVersion);
        if (0==res)
        {
            res = GetIPMCVersionFromBuild(IPMCBuildVersion);
        }
        else
        {
            return STATUS_FAIL;
        }

        if (0 == res)
        {
            IPMCChipVersion=TranslateChipVersionStringToNumber(ChipVersion);
        }
        answer << "version on chip: " << IPMCChipVersion << " version on build: " << IPMCBuildVersion;
    }
    else
    {
        res = GetIPMCVersionFromBuild(IPMCBuildVersion);
    	answer << "IPMC version on build is: " << IPMCBuildVersion;
    }

    return STATUS_OK;
}



//**************************************************************************************************************************************

//    K E E P	F O L L O W I N G 	C O D E 	 A T	  T H E		E N D 	O F		F I L E ! ! !

//**************************************************************************************************************************************

// **************************************************************
// *															*
// * Following code was taken from COMMON/IpMc/upgradefw.cpp	*
// *															*
// **************************************************************

/* upgrade interface type */
static int ug_type = -1;

/* upgrade device/parameters */
static char *device_name = NULL;
static char *device_param = NULL;

/* upgrade image */
static char *fw_file_name = NULL;

/* quiet mode */
static int quiet = 0;

/* verbosity level */
static int verbosity = 0;

/* upgrade buffer */
static unsigned char *buf;

/* restore backup flags */
static int restore_backup = 0;
static int get_backup_version = 0;

/* upgrade image headers */
static ug_file_head fhead;
static ug_img_head head;

/* -------------------------------------------------------------- */

typedef struct _strtab_entry {
    int code;
    char *str;
} strtab_entry;

static strtab_entry ipmi_errors[] =
{
    { 0, "OK" },
    { -1, "Unknown error" },
    { ERR_TIMEOUT, "Target does not respond" },
    { ERR_INVALID_RESPONSE, "Communication is lost (invalid response)" },
    { ERR_SYNTAX, "Communication is lost (bad syntax)" },
    { IPMI_INVALID_COMMAND, "Upgrade is not supported" },
    { IPMI_DEVICE_IN_FIRMWARE_UPDATE_MODE, "Device is in update mode" },
    { IPMI_NODE_BUSY, "Device is busy" },
    { -1, NULL }
};

static char str_err_hex[16];

static char *strtab_lookup(strtab_entry *tab, int code)
{
    if (!tab) {
        return NULL;
    }

    while (tab->str) {
        if (code == tab->code) {
            return tab->str;
        }
        tab++;
    }
    return NULL;
}

static char *ipmi_error_str(int code)
{
    char *p;

    if (code & ERR_SYNTAX) {
        code = ERR_SYNTAX;
    }
    p = strtab_lookup(ipmi_errors, code);
    if (p) {
        return p;
    }
    sprintf(str_err_hex, "error %02X", code);
    return str_err_hex;
}

/* -------------------------------------------------------------- */

/*
    Open the Firmware Upgrade session
*/
#define SESSION_OPEN_RETRY	5
//////////////////////////////////////////////////////////////////////
int CIPMCInterfaceManager::open_session(int nFd)
{
    int err, t;

    if (verbosity) {
		fprintf(stderr, "Opening upgrade session ... ");
		fflush(stderr);
    }

    /* Open session - Firmware Upgrade Start */
    for (t = 0; t < SESSION_OPEN_RETRY; t++) {
        err = upgrade_start(nFd);
        if (!err || err == IPMI_DEVICE_IN_FIRMWARE_UPDATE_MODE ||
		err == IPMI_INVALID_COMMAND) {
            break;
        }
        SystemSleep(2000);
    }

    if (err) {
		if (verbosity) {
			fprintf(stderr, "failed\n");
		}
		fprintf(stderr, "Error: ");
		}
		if (err == IPMI_INVALID_COMMAND) {
			fprintf(stderr, "Device does not support firmware update\n");
		}
		if (err == ERR_INVALID_RESPONSE) {
			fprintf(stderr, "Invalid response\n");
		}
		if (err == IPMI_DEVICE_IN_FIRMWARE_UPDATE_MODE) {
			fprintf(stderr, "Upgrade session is already opened\n");
		}
		if (err) {
			fprintf(stderr, "%s\n", ipmi_error_str(err));
		} else {
		if (verbosity) {
			fprintf(stderr, "OK\n");
		}

    }

    return err;
}

/*
    Close the Firmware Upgrade session
*/
//////////////////////////////////////////////////////////////////////
int CIPMCInterfaceManager::close_session(int nFd)
{
    int err;

    /* Firmware Upgrade Complete */
    if (verbosity) {
    	fprintf(stderr, "Closing upgrade session ... ");
		fflush(stderr);
    }

    err = upgrade_complete(nFd);
    if (verbosity) {
		if (err) {
			fprintf(stderr, "failed\n");
		} else {
			fprintf(stderr, "OK\n");
		}
    }
    return err;
}

/*
    Print backup version
*/
//////////////////////////////////////////////////////////////////////
int CIPMCInterfaceManager::show_version(int nFd)
{
    unsigned char st[2];
    int err;

    if (verbosity) {
    	fprintf(stderr, "Reading backup image version ... ");
    	fflush(stderr);
    }

    /* Firmware Upgrade Complete */
    err = upgrade_get_backup_version(st,nFd);
    if (err) {
		if (verbosity) {
			fprintf(stderr, "failed\n");
		}
		fprintf(stderr, "Error: ");
    }
    if (err == IPMI_REQUESTED_DATA_NOT_PRESENT) {
    	fprintf(stderr, "No valid backup copy\n");
        return err;
    } else if (err == IPMI_INVALID_COMMAND) {
    	fprintf(stderr, "Enhanced mode is not supported\n");
        return err;
    } else if (err) {
    	fprintf(stderr, "%s\n", ipmi_error_str(err));
        return err;
    } else {
		if (verbosity) {
			fprintf(stderr, "OK\n");
		}
    }

    if (st[1] & 0x0F) {
    	fprintf(stderr, "Backup version: %d.%d.%d\n",
		st[0], st[1] >> 4, st[1] & 0x0F);
    } else {
    	fprintf(stderr, "Backup version: %d.%d\n", st[0], st[1] >> 4);
    }

    return 0;
}
/* -------------------------------------------------------------- */

/*
    Wait for backup/restore operaation
*/
#define BACKUP_TOUT 60
//////////////////////////////////////////////////////////////////////
int CIPMCInterfaceManager::wait_backup(int nFd)
{
    int t;
    int err;
    unsigned char st[2];

    t = BACKUP_TOUT;
    while (--t > 0) {
    	SystemSleep(1000);

        err = upgrade_status(st,nFd);
        if (err != 0) {
            return err;
        } else {
            if (st[0] == UPGRADE_STATUS_BUSY) {
                /* still busy */
            	fprintf(stderr, ".");
		fflush(stderr);
                continue;
            } else {
                break;
            }
        }
    }

    if (t <= 0) {
        return ERR_TIMEOUT;
    }
    return err;
}

/*
    Restore firmware from backup copy
*/
//////////////////////////////////////////////////////////////////////
int CIPMCInterfaceManager::do_restore(int nFd)
{
    int err;

    err = 0;

    fprintf(stderr, "Restoring firmware from backup ...");
    fflush(stderr);

    err = upgrade_restore(nFd);
    if (err) {
    	fprintf(stderr, " failed\n");
    	fprintf(stderr, "Error: ");
	if (err == ERR_TIMEOUT) {
		fprintf(stderr, "Timeout\n");
	} else if (err == IPMI_INVALID_COMMAND) {
		fprintf(stderr, "Enhanced mode is not supported\n");
	} else {
		fprintf(stderr, "%s\n", ipmi_error_str(err));
	}
        return err;
    }

    err = wait_backup(nFd);
    if (err) {
    	fprintf(stderr, " failed\n");
    	fprintf(stderr, "Error: ");
	if (err == ERR_TIMEOUT) {
		fprintf(stderr, "Timeout\n");
	} else {
		fprintf(stderr, "%s\n", ipmi_error_str(err));
	}
	return err;
    }

    fprintf(stderr, " OK\n");

    return err;
}

/*
    write firmware
*/
//////////////////////////////////////////////////////////////////////
int CIPMCInterfaceManager::write_image(unsigned char *data, int target, unsigned long addr,
										int size, int flag , int nFd)
{
    int i, percent, percent0;
    int err;
    unsigned long addr_start, addr_end;

    addr_start = addr;
    addr_end = addr_start + size;

    err = 0;

    /* Firmware Upgrade Prepare */
    if (flag & 1) {
    	fprintf(stderr, "Preparing target %i for programming ...", target);
		fflush(stderr);

		err = upgrade_prepare(target,nFd);
		if (err) {
			fprintf(stderr, " failed\n");
			fprintf(stderr, "Error: ");
			if (err == ERR_TIMEOUT) {
				fprintf(stderr, "Timeout\n");
			} else {
				fprintf(stderr, "%s\n", ipmi_error_str(err));
			}
			return err;
		}

		err = wait_backup(nFd);
		if (err) {
			fprintf(stderr, " failed\n");
			fprintf(stderr, "Error: ");
			if (err == ERR_TIMEOUT) {
				fprintf(stderr, "Timeout\n");
			} else {
				fprintf(stderr, "%s\n", ipmi_error_str(err));
			}
			return err;
		}

		fprintf(stderr, " OK\n");
    }

    i = 0; percent0 = -1;
    while (1)
    {
        if (size == 0) {
            percent = 0;
        } else if (i < size) {
            percent = (i * 100) / size;
        } else {
        	percent = 100;
        }

        if (percent != percent0) {
        	fprintf(stderr, "\rProgramming %d bytes to target %i at %06lX ... %3d%% ", size, target, addr_start, percent);

            char message[128] ="";
            sprintf(message, "\rIPMC upgrade: Programming %d bytes to target %i at %06lX ... %3d%% ", size, target, addr_start, percent);
            FTRACEINTO << message;

            //Send progress data to Installer process
            CSegment*  pParam = new CSegment;
	    *pParam << (DWORD)percent;
	    const COsQueue* pInstallerMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessInstaller, eManager);
	    pInstallerMbx->Send(pParam, IPMCIF_TO_INSTALLER_UPGRADE_PROGRESS);

            fflush(stderr);
            percent0 = percent;
        }
		if (addr >= addr_end) {
			break;
		}
        err = upgrade_write(target, addr, blk_size, data + i , nFd);
        if (err) {
            break;
        }
        addr += blk_size;
        i += blk_size;
        SystemSleep(0);
    }

    fprintf(stderr, "\n");

    if (err) {
    	fprintf(stderr, "Error: ");
		if (err == ERR_TIMEOUT) {
			fprintf(stderr, "Connection lost\n");
		} else if (err == IPMI_DEVICE_IN_FIRMWARE_UPDATE_MODE) {
			fprintf(stderr, "Session aborted\n");
		} else {
			fprintf(stderr, "%s\n", ipmi_error_str(err));
		}
    }

    return err;
}

/* -------------------------------------------------------------- */

static char *usage = "Usage: upgradefw <options> upgrade_image\n"
        "Options:\n"
        "  -s device[:options]             upgrade over a serial line using\n"
        "                                  the SIPL-TM (Terminal Mode) protocol\n"
        "  -b device[:options]             upgrade over a serial line using\n"
        "                                  the SIPL-BM (Basic Mode) protocol\n"
        "  -n hostname[:port],targetaddr   upgrade over LAN/IPMB\n"
#ifdef _UPGRADE_IPMB
        "  -I device:targetaddr            upgrade over IPMB\n"
#endif
        "  -r                              restore from the backup copy\n"
        "  -R                              get backup version\n"
        "  -q                              quiet mode";

static int optindex = 0;

//////////////////////////////////////////////////////////////////////
int CIPMCInterfaceManager::LoadIpmcSoftware(int argc, char **argv)
{
    FILE *f;
    unsigned long addr, size;
    int c;
    int err = 0 , nFd = 0;
    char *p;

    puts("BMR firmware upgrade utility. Pigeon Point Systems (c) 2004-2005.");

    while (1) {
        if (++optindex >= argc) {
            break;
        }
        p = argv[optindex];
        if (p[0] == '-') {
            c = p[1];
        } else {
	    if (fw_file_name != NULL) {
		ug_type = -1;
		break;
	    }
            c = 1;
        }

        switch (c) {
        case 'q':
            quiet = 1;
            break;

        case 'v':
            verbosity = 1;
            break;

        case 'd':
            upgrade_debug = 1;
            break;

        case 'r':
            restore_backup = 1;
            break;

        case 'R':
            get_backup_version = 1;
            break;

        case 'I':
            p = argv[++optindex];
            device_name = p;
    	    p = strchr(device_name, ':');
    	    if (p) {
    		*p = 0;
    		device_param = p + 1;
    	    }
            ug_type = 1;
            break;

        case 'n':
            p = argv[++optindex];
            device_param = p;
    	    device_name = NULL;
            ug_type = 2;
            break;


	case 'b':
        case 's':
            p = argv[++optindex];
            device_name = p;
    	    p = strchr(device_name, ':');
    	    if (p) {
    		*p = 0;
    		device_param = p + 1;
    	    }
            ug_type = ('b' == c) ? 3 : 0;
            break;

        case 1:
            fw_file_name = p;
            break;
        }
    }

    if (ug_type < 0) {
        puts(usage);
        return 0;
    }

    if (!restore_backup && !get_backup_version && fw_file_name == NULL) {
        puts(usage);
        return 0;
    }

    setbuf(stdout, NULL);

    if (!quiet) {
        p = (char*)upgrade_if_name(ug_type);
        if(p)
        {
            fprintf(stderr, "Upgrade interface: %s", p);
        }
		if (device_name) {
			fprintf(stderr, ", device: %s", device_name);
		}
        if (device_param) {
        	fprintf(stderr, ", options: %s", device_param);
        }
        fprintf(stderr, "\n");
    }

    nFd = upgrade_open(ug_type, device_name, device_param);
    if (nFd == 0) {
    	fprintf(stderr, "Cannot open upgrade interface: error %d\n", err);
        return nFd;
    }

    err = open_session(nFd);

    if (err) {
        upgrade_close(nFd);
        return err;
    }

    if (get_backup_version) {
        show_version(nFd);
    }

    if (restore_backup) {
        err = do_restore(nFd);
        close_session(nFd);
        upgrade_close(nFd);
        return 0;
    }

    if (fw_file_name == NULL) {
    	close_session(nFd);
        upgrade_close(nFd);
	return 0;
    }

    if (!quiet) {
    	fprintf(stderr, "Firmware upgrade image: %s\n", fw_file_name);
    }

    f = fopen(fw_file_name, "rb");
    if (!f) {
        perror(fw_file_name);
        return 1;
    }

    c = fread(&fhead, sizeof(ug_file_head), 1, f);
    if (fhead.mfg[0] != ((IANA_MANUFACTURER_PPS >> 16) & 0xFF) ||
	    fhead.mfg[1] != ((IANA_MANUFACTURER_PPS >> 8) & 0xFF) ||
	    fhead.mfg[2] != ((IANA_MANUFACTURER_PPS) & 0xFF)) {
    	fprintf(stderr, "Invalid image: %s\n", fw_file_name);
        close_session(nFd);
        upgrade_close(nFd);
        return 1;
    }
    if (fhead.version != 0x10) {
    	fprintf(stderr, "Incorrect image format version (%02X)\n",
		fhead.version);
        close_session(nFd);
        upgrade_close(nFd);
        return 1;
    }

    err = 0;
    while (err == 0)
    {
        c = fread(&head, sizeof(ug_img_head), 1, f);
        if (c < 1) {
            break;
        }
        addr = get3(head.offset);
        size = get3(head.size);
        buf = (unsigned char*)malloc(size);
        if (!buf) {
            puts("Not enough memory");
            return 1;
        }
        fread(buf, 1, size, f);

        err = write_image(buf, head.target, addr, size, head.flags , nFd);

        if (buf) {
            free(buf);
            buf = NULL;
        }

    	if (err) {
    	    break;
        }
    	SystemSleep(0);
    }

    if (f) {
        fclose(f);
        f = NULL;
    }

    close_session(nFd);
    upgrade_close(nFd);

    return err;
}

// **************************************************************
// *															*
// * End of code that was taken from COMMON/IpMc/upgradefw.cpp	*
// *															*
// **************************************************************

// **************************************************************************************************************************************

