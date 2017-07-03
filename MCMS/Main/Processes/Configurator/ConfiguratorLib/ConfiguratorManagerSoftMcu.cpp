// ConfiguratorManager.cpp

#include "ConfiguratorManagerSoftMcu.h"

#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <shadow.h>
#include <streambuf>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>

#include "ConfigManagerOpcodes.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "Segment.h"
#include "SystemFunctions.h"
#include "TraceStream.h"
#include "ConfigManagerApi.h"
#include "ApiStatuses.h"
#include "OpcodesMcmsCommon.h"
#include "TraceStream.h"
#include "OsFileIF.h"
#include "IfConfig.h"
#include "InternalProcessStatuses.h"
#include "TerminalCommand.h"
#include "EthernetSettingsUtils.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "StringsLen.h"
#include "WhiteList.h"



extern bool IsJitcAndNetSeparation();

PBEGIN_MESSAGE_MAP(CConfiguratorManagerSoftMcu)
    ONEVENT(CONFIGURATOR_RESTART_SSHD                  ,IDLE , CConfiguratorManagerSoftMcu::RestartSSH )
    ONEVENT(CONFIGURATOR_KILL_SSHD                     ,IDLE , CConfiguratorManagerSoftMcu::KillSsh )

    ONEVENT(CONFIGURATOR_NTP_SERVICE            ,IDLE , CConfiguratorManagerSoftMcu::HandleNtpService )
    ONEVENT(CONFIGURATOR_CONFIG_NTP_SERVERS     ,IDLE , CConfiguratorManagerSoftMcu::ConfigNtpServers )
    
    ONEVENT(CONFIGURATOR_ENABLE_DHCP_IPV6     ,IDLE , CConfiguratorManagerSoftMcu::EnableDHCPIPv6 )
    ONEVENT(CONFIGURATOR_DISABLE_DHCP_IPV6    ,IDLE , CConfiguratorManagerSoftMcu::DisableDHCPIPv6 )

    ONEVENT(CONFIGURATOR_GET_UDP_OCCUPIED_PORTS    ,IDLE , CConfiguratorManagerSoftMcu::GetUdpOccupiedPorts )
	
PEND_MESSAGE_MAP(CConfiguratorManagerSoftMcu,CConfiguratorManager);

BEGIN_SET_TRANSACTION_FACTORY(CConfiguratorManagerSoftMcu)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CConfiguratorManagerSoftMcu)
	ONCOMMAND("ssh", 	CConfiguratorManagerSoftMcu::SSHTerminalRequest,    "ssh")
END_TERMINAL_COMMANDS

extern void ConfiguratorMonitorEntryPoint(void* appParam);

///////////////////////////////////////////////////////////////////////////////////////////////
CConfiguratorManagerSoftMcu::CConfiguratorManagerSoftMcu()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////
CConfiguratorManagerSoftMcu::~CConfiguratorManagerSoftMcu()
{

}


///////////////////////////////////////////////////////////////////////////////////////////////
void CConfiguratorManagerSoftMcu::RestartSSH(CSegment *pSeg)
{
	STATUS stat = STATUS_FAIL;
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL isJITCMode = NO;

	sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, isJITCMode);

	COstrStream ssh_cmd;

	ssh_cmd << MCU_UTILS_DIR << "/Scripts/SSHTools.sh  ";

	if(!isJITCMode)
	{
		BYTE isPermanentNetwork=FALSE, isOn=FALSE;
		std::string ipV4;
		std::string ipV6;

		*pSeg >> isPermanentNetwork;
		*pSeg >> ipV4;
		*pSeg >> ipV6;
		*pSeg >> isOn;


		if (ipV4 != "")
		{
			ssh_cmd << " --listen " << ipV4;
		}


		if (ipV6 != "" && ipV6 != "::")
		{
			//ssh_cmd << "; echo ListenAddress " << ipV6;
		}

		if(isOn)
			ssh_cmd << " --service restart";
		else
			ssh_cmd << " --service stop";

		std::string answer;
			stat = SystemPipedCommand(ssh_cmd.str().c_str(), answer);

		if(STATUS_OK != stat)
		{
			stat = STATUS_FAIL;
		}

		// temp temp temp - for debugging
		TRACESTR(eLevelInfoNormal) << "CConfiguratorManagerSoftMcu::RestartSSH"
							   << "\nCmd:    " << ssh_cmd.str().c_str()
							   << "\nstat:   " << stat << " (" << CProcessBase::GetProcess()->GetStatusAsString(stat) << ")"
							   << "\nAnswer: " << answer;

	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "CConfiguratorManagerSoftMcu::RestartSSH : ssh is not allowed under JITC Mode!";
		ssh_cmd << " --service stop";
	}

    ResponedClientRequest(stat);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CConfiguratorManagerSoftMcu::KillSsh(CSegment* seg)
{
    std::string ans;
    std::string cmd = MCU_UTILS_DIR+"/Scripts/SSHTools.sh --service stop";
	
    STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManagerSoftMcu::KillSsh :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;

    ResponedClientRequest(stat);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CConfiguratorManagerSoftMcu::EnableDHCPIPv6(CSegment *pSeg)
{
	std::string ans;
    	std::string cmd = MCU_UTILS_DIR+"/Scripts/ManageIPv6.sh Enable";

	STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManagerSoftMcu::EnableIPv6 :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;

	ResponedClientRequest(stat);
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CConfiguratorManagerSoftMcu::DisableDHCPIPv6(CSegment *pSeg)
{
	std::string ans;
    	std::string cmd = MCU_UTILS_DIR+"/Scripts/ManageIPv6.sh Disable";

	STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

	TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManagerSoftMcu::DisableIPv6 :" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;

	ResponedClientRequest(stat);
}

///////////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfiguratorManagerSoftMcu::SSHTerminalRequest(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		return STATUS_FAIL;
	}

	const string &OnOff = command.GetToken(eCmdParam1);
	const string &ip = command.GetToken(eCmdParam2);

	TRACESTR(eLevelInfoNormal) << "CConfiguratorManagerSoftMcu::SSHTerminalRequest : " << OnOff << " , " << ip;

	STATUS stat = STATUS_OK;

	CSegment *pSeg = new CSegment;
	*pSeg << FALSE
			<< ip
			<< "";

	if("on" == OnOff)
		RestartSSH(pSeg);
	else
		KillSsh(pSeg);


	answer <<  stat;
	return stat;
}

void  CConfiguratorManagerSoftMcu::HandleNtpService(CSegment* seg)
{
    std::string ans, server_ip;
    std::stringstream cmd_buf;
    STATUS stat = STATUS_OK;

    enum ServiceCommand cmd;
 
    *seg  >>  (DWORD&)cmd;

    const string ntpCfgScript = MCU_UTILS_DIR+"/Scripts/NtpTools.sh";

    switch (cmd)
    {
       case eCmdStart:
           cmd_buf << ntpCfgScript << " --service " << "start";
           stat = SystemPipedCommand(cmd_buf.str().c_str(), ans);
           break;
       case eCmdStop:
           cmd_buf << ntpCfgScript << " --service " << "stop";
           stat = SystemPipedCommand(cmd_buf.str().c_str(), ans);
           break;
       default:
            ResponedClientRequest(STATUS_FAIL);
            return;
    }

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
        "CConfiguratorManagerSoftMcu::HandleNtpService :" << cmd_buf.str() << std::endl << "answer:" << ans << ", stat:" << stat;

    ResponedClientRequest(stat);
} 

void  CConfiguratorManagerSoftMcu::ConfigNtpServers(CSegment* seg)
{
    std::string ans,s1,s2,s3;
    std::stringstream cmd_buf;
    STATUS stat = STATUS_OK;

    *seg  >>  s1 >> s2 >> s3;

    const string ntpCfgScript = MCU_UTILS_DIR+"/Scripts/NtpTools.sh";

    if(s1.empty())
    {
       stat = STATUS_FAIL;
    }
    else
    {
        cmd_buf << ntpCfgScript << " --conf " << s1;

        if(!s2.empty())
            cmd_buf << "," << s2; 

        if(!s3.empty())
            cmd_buf << "," << s3; 

        stat = SystemPipedCommand(cmd_buf.str().c_str(), ans);
    }

    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
            "CConfiguratorManagerSoftMcu::ConfigNtpServers :" << cmd_buf.str() << std::endl << "answer:" << ans << ", stat:" << stat;

    ResponedClientRequest(stat);
}
