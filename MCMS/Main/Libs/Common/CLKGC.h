//************************************************************************
//   (c) Polycom
//
//  Name:   CLKGC.h
//
//  Description:
/*
 *  This file implements the last known good configuration feature on the management configuration file.
 *  the class will keep backup of files
 *   1. NetworkCfg_Management.xml
 *   2. SystemCfgUserTmp.xml
 *    for successful configuration.
 *   in case of failure which will block user to connect to the management  an attempt to reconfigure the management will occur
 *   by using last known good configuration this attempt will happen only once.
 *
 *	 Flags:
 *	 	CFG_DISABLED_LAST_GOOD_CONFIG/DISABLED_LAST_GOOD_CONFIG - YES/NO
 *   Documentation List :
 *   HLD-
 *   	1. http://isrportal07/sites/rd/carmel/Restricted%20Documents/SW%20Department/Management/V100/ReWrite/Startup%20New%20design.pptx
 *   TestList
 *      1. http://isrportal07/sites/rd/carmel/Restricted%20Documents/SW%20Department/Management/V100/ReWrite/Tests%20for%20Last%20known%20configuration.xls
 */
//
//  Revision History:
//
//  Date            Author           Functional/Interface Changes
//  -----------    -------------    ----------------------------------------
//  Jun 27, 2013    stanny			  created class
//************************************************************************
#ifndef CLastKnownGoodConfig_H_
#define CLastKnownGoodConfig_H_

//#include "Segment.h"
#include "Trace.h"
#include "ConfigManagerApi.h"
//#include "PObject.h"

typedef  void  (*RESTART_CALLBACK_FUNC)(); //callback pointer to a function that will do a restart on mcms

// CLastKnownGoodConfig status success,partial success and failure
#define  STATUS_CONFIG_PARTIAL_SUCCESS 2
#define  STATUS_CONFIG_SUCCESS 1
#define  STATUS_CONFIG_FAILURE 0
#define  STATUS_CONFIG_APACHE_FAILURE 3
// file created before request for restart callback function
#define  LKGC_FILE_RESTART_IND  "States/LKGC_RESTART_REQ.lkgc"

#define BACKUP_FILENAME_NET_MANGEMENT ((std::string)MCU_MCMS_DIR+"/Cfg/NetworkCfg_Management.xml.bup")
#define BACKUP_FILENAME_SYSTEM_FLAGS ((std::string)MCU_MCMS_DIR+"/Cfg/SystemCfgUserTmp.xml.bup")

#define SOURCE_FILENAME_NET_MANGEMENT ((std::string)MCU_MCMS_DIR+"/Cfg/NetworkCfg_Management.xml")
#define SOURCE_FILENAME_SYSTEM_FLAGS ((std::string)MCU_MCMS_DIR+"/Cfg/SystemCfgUserTmp.xml")

#define FORMAT_BAD_FILENAME_NET_MANGMENT ((std::string)MCU_MCMS_DIR+"/Cfg/bad_%d_NetworkCfg_Management.xml")
#define FORMAT_BAD_FILENAME_SYSTEM_FLAGS ((std::string)MCU_MCMS_DIR+"/Cfg/bad_%d_SystemCfgUserTmp.xml")
/*
  CLastKnownGoodConfig - class last known good configuration
   main class
*/
class CLastKnownGoodConfig
{
private:
	RESTART_CALLBACK_FUNC m_pfunc; //Call back function for calling the API to restart the MCMS
	eConfigInterfaceType m_ifType; //interface type
	eIpType				 m_iptype; //ip type ipv4/ipv6
	BOOL 				 m_needSudo;
protected:
	// event handlers
	void OnRestartAfterFailure(WORD& status);
	void OnStatusFailure();
	void OnStatusSuccess();
	void OnStatusPartialSuccess();
	BOOL IsMngmntInterfaceIsUp();
	BOOL IsApacheProcessIsUp();
public:
	CLastKnownGoodConfig(RESTART_CALLBACK_FUNC pfunc,eProductType curProductType);
	static void CleanLastKnownGoodConfig(); //called for restore factory defaults;
	virtual ~CLastKnownGoodConfig();
	//send internal status and interface and iptype so we can verify interfaces
	WORD network_configuration_status(WORD status,eConfigInterfaceType ifType,eIpType ipType);
};

#endif /* CLastKnownGoodConfig_H_ */
