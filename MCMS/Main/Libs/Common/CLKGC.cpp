/*
 * CLastKnownGoodConfig.cpp
 *
 *  Created on: Jun 27, 2013
 *      Author: stanny
 */
#include "TraceStream.h"
#include "CLKGC.h"
#include "SystemFunctions.h"
#include "OsFileIF.h"

CLastKnownGoodConfig::CLastKnownGoodConfig(RESTART_CALLBACK_FUNC pfunc,eProductType curProductType) {

	m_pfunc =pfunc;
	m_ifType = 	eManagmentNetwork;
	m_iptype = eIpType_Both;
	m_needSudo = (eProductTypeGesher==curProductType) || (eProductTypeNinja==curProductType);

}

CLastKnownGoodConfig::~CLastKnownGoodConfig() {
	//
}

/// Main flow function
// Receive configuration status
WORD CLastKnownGoodConfig::network_configuration_status(WORD status,eConfigInterfaceType ifType,eIpType ipType)
{
	m_ifType = 	ifType;
	m_iptype = ipType;
  //check if we are after restart request (due to previous failure)
  if(IsFileExists(LKGC_FILE_RESTART_IND))
  {
	  OnRestartAfterFailure(status);
	  return status;
  }
  BOOL isMngntInterfaceUp =IsMngmntInterfaceIsUp();
  //check status
  switch(status)
  {
     case STATUS_CONFIG_FAILURE:
    	 	 	 if(!isMngntInterfaceUp)
    	 	 			OnStatusFailure();
    	 	 	 else
    	 	 	 {
    	 	 		 FTRACESTR(eLevelWarn)  << "Conflicts between internal status to mngnt interface this could indicate a bug \n" <<
    	 	 				 "Status is Failure ,But mngmnt Interfaces are up";
    	 	 		 OnStatusPartialSuccess();
    	 	 		 status = STATUS_CONFIG_PARTIAL_SUCCESS;
    	 	 	 }
    	 	 break;
     case STATUS_CONFIG_SUCCESS:
    	 	 	 if(!isMngntInterfaceUp)
    	 	 	 {
    	 	 		FTRACESTR(eLevelWarn)  << "Conflicts between internal status to mngnt interface this could indicate a bug \n"
    	 	 							   << "Status is Success ,But mngmnt Interfaces are down"; ;
    	 	 		 OnStatusFailure();
    	 	 		status =  STATUS_CONFIG_FAILURE;
    	 	 	 }
    	     break;
     case STATUS_CONFIG_PARTIAL_SUCCESS:
    	 	 	 	 	 if(isMngntInterfaceUp)
    	 	 	 	 		OnStatusPartialSuccess();
    	     	 	 	 else
    	     	 	 	 {
    	     	 	 		FTRACESTR(eLevelWarn)  << "Conflicts between internal status to mngnt interface this could indicate a bug \n"
    	     	 	 							   << "Status is Partial Success ,But mngmnt Interfaces are down";
    	     	 	 		 OnStatusFailure();
    	     	 	 		status = STATUS_CONFIG_FAILURE;
    	     	 	 	 }
    	 	 break;
     default:
    	 FTRACESTR(eLevelWarn)  << "Received Invalid parameter" ;
  }
  // if interfaces are up check if Apache process is up
  if(STATUS_CONFIG_FAILURE != status)
  {
	  if(!IsApacheProcessIsUp())
	  {
		  FTRACESTR(eLevelError)  << "Apache Process is Down -> go to failure flow ";
		  status = STATUS_CONFIG_FAILURE;
		  OnStatusFailure();
	  }
  }
  if(STATUS_CONFIG_SUCCESS == status)
		 OnStatusSuccess();
  return status;
}
BOOL  CLastKnownGoodConfig::IsApacheProcessIsUp()
{
	std::string cmd,ans;
	// a trick so grep does not report it self
    cmd = "ps -ef|grep http[d]";
    STATUS stat = SystemPipedCommand(cmd.c_str(), ans);
    if (STATUS_OK == stat)
    {
    	if(ans.size() >0)
    		return TRUE;
    	else
    		return FALSE;
    }
    return TRUE;
}
/* explaination on the trick.
 * question 'why does grep emac[s] work', here is the real answer:
The regular expression (the s inside the square brackets) matches a character set which contains a single character (the character s in this case) and the literal string "emac".
Grep does not self-match the original regular expression string ('grep emac[s]'), because that string also includes the square brackets.
i.e. the word 'emacs' does not appear in the original command given: 'grep emac[s]'
 * */

BOOL CLastKnownGoodConfig::IsMngmntInterfaceIsUp()
{
	std::string cmd,ans;
	std::string base_cmd = "ifconfig |grep '''";
	std::string tail_cmd = " .*'''";
	const char* pStrinterfaceName  = GetLogicalInterfaceName(m_ifType,m_iptype);


	if (m_needSudo)
	    {
			cmd = std::string("sudo ") + base_cmd + std::string(pStrinterfaceName) +tail_cmd;
	    }
	    else
	    {
	    	cmd = base_cmd + std::string(pStrinterfaceName) +tail_cmd;
	    }

	STATUS stat = SystemPipedCommand(cmd.c_str(), ans);
	if (STATUS_OK == stat)
	{
		if(ans.size() >0)
			return TRUE;
		else
			return FALSE;
	}
	return TRUE;
}
/// called after mcms was restarted due to roll back on the network configuration files
void CLastKnownGoodConfig::OnRestartAfterFailure(WORD& status)
{
	if(STATUS_CONFIG_FAILURE < status)
	{
		FTRACESTR(eLevelInfoNormal) << "Network configuration status success or partial success after roll back to previous configuration. ";
		// also for partial success since it is better then failure
		OnStatusSuccess();
		DeleteFile(LKGC_FILE_RESTART_IND);
	}
	else
		FTRACESTR(eLevelInfoNormal) <<"Network configuration status failure roll back to previous configuration. ";

}



void CLastKnownGoodConfig::OnStatusSuccess()
{
	FTRACESTR(eLevelInfoNormal) <<"Network configuration status success - make backup files";
    CopyFile(SOURCE_FILENAME_NET_MANGEMENT,BACKUP_FILENAME_NET_MANGEMENT);
    CopyFile(SOURCE_FILENAME_SYSTEM_FLAGS,BACKUP_FILENAME_SYSTEM_FLAGS);

}
void CLastKnownGoodConfig::OnStatusPartialSuccess()
{
	FTRACESTR(eLevelInfoNormal) <<"Network configuration status partial success - do not do anything ";
}

void CLastKnownGoodConfig::OnStatusFailure()
{
	FTRACESTR(eLevelInfoNormal) <<"Network configuration status failure -start  LKGC failure flow";
	TICKS  timestamp = SystemGetTickCount();
	char BadFileNameMangment[ONE_LINE_BUFFER_LEN];
	char BadFileNameFlags[ONE_LINE_BUFFER_LEN];
	memset(BadFileNameMangment, 0, ONE_LINE_BUFFER_LEN);
	memset(BadFileNameFlags, 0, ONE_LINE_BUFFER_LEN);
    // make bad file name save for analysis/debug
	snprintf(BadFileNameMangment,sizeof(BadFileNameMangment)-1,(FORMAT_BAD_FILENAME_NET_MANGMENT).c_str(),timestamp.GetIntegerPartForTrace());
	sprintf(BadFileNameFlags,(FORMAT_BAD_FILENAME_SYSTEM_FLAGS).c_str(),timestamp.GetIntegerPartForTrace());

	CopyFile(SOURCE_FILENAME_NET_MANGEMENT,BadFileNameMangment);
	CopyFile(SOURCE_FILENAME_SYSTEM_FLAGS,BadFileNameFlags);

	// create startup failure indication so when we come back from restart we know that we came after LKGC on failure flow

	CopyFile(BACKUP_FILENAME_NET_MANGEMENT,SOURCE_FILENAME_NET_MANGEMENT);
	CopyFile(BACKUP_FILENAME_SYSTEM_FLAGS,SOURCE_FILENAME_SYSTEM_FLAGS);

	if(!CreateFile(LKGC_FILE_RESTART_IND))
	{
		FTRACESTR(eLevelInfoNormal) <<"LKGC failed to write LKGC_FILE_RESTART_IND file ";
		return;
	}
	FTRACESTR(eLevelInfoNormal) <<"LKGC failure flow done call restart function";
	//call restart function
	//m_pfunc();
}

//called for restore factory defaults;
void CLastKnownGoodConfig::CleanLastKnownGoodConfig()
{
	FTRACESTR(eLevelInfoNormal) <<"Cleanup - delete last known good configuration files";
	DeleteFile(LKGC_FILE_RESTART_IND);
	DeleteFile(BACKUP_FILENAME_NET_MANGEMENT);
	DeleteFile(BACKUP_FILENAME_SYSTEM_FLAGS);
}
