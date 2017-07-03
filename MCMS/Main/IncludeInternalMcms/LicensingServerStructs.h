//+========================================================================+
//                  LicensingServerStructs.h							   |
//					Copyright Polycom							           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       LicensingServerStructs.h                                    |
// SUBSYSTEM:  Libs/Common		                                           |
// PROGRAMMER: Rachel Cohen.	                                           |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |   The file includes structures interface between     |
//						McuMngr process and Licensing process.             |
//						                          |
//+========================================================================+


#ifndef _LicensingServerStructs_H_
#define _LicensingServerStructs_H_

#include <string>


#include "McuMngrStructs.h"
#include "DefinesGeneral.h"
#include "StructTm.h"




/******************************************************************************************/
/*							  Interface between McuMngr process and Licensing process. 	  */
/******************************************************************************************/

#define LICENSE_SERVER_HOST_MAX_LEN 256

//////////////////////////////////////////////////////////////////////////
/* opcode: LICENSE_SERVER_PARAMS_IND  */
typedef struct
{
	char               primaryLicenseServerHost[LICENSE_SERVER_HOST_MAX_LEN];
	DWORD              primaryLicenseServerPort;
	char               mcuHostId[MPL_SERIAL_NUM_LEN];
	VERSION_S          mcuVersion;
	DWORD              maxMcuCapacity;
}LICENSE_SERVER_DATA_S ;


typedef enum {
	E_FLEXERA_LICENSE_VALID,
	E_FLEXERA_LICENSE_INVALID,
	E_FLEXERA_LICENSE_TIME_EXPIRED,
	E_FLEXERA_LICENSE_WINDBACK_DETECTED,
	E_FLEXERA_LICENSE_INSUFFICIENT_RESOURCES
} E_FLEXERA_LICENSE_VALIDATION_STATUS;


typedef enum {
	RPCS,
	RPP_PKG,
	AVC_CIF_PLUS,
	TIP,
	RPCS_MAX_PORTS,
	MEDIA_ENCRYPTION,
	RPCS_TELEPRESENCE,
	RPCS_MULTIPLE_SERVICES,
	RPCS_SVC,
	RPCS_AVAYA,
	RPCS_IBM,
	MAX_NUM_OF_FEATURES
} E_FLEXERA_LICENSE_FEATURES;


static const char *licensingFeaturesStr[] =
{
	"RPCS",
	"RPP_PKG",
	"AVC_CIF_PLUS",
	"TIP",
	"RPCS_MAX_PORTS",
	"MEDIA_ENCRYPTION",
	"RPCS_TELEPRESENCE",
	"RPCS_MULTIPLE_SERVICES",
	"RPCS_SVC",
	"RPCS_AVAYA",
	"RPCS_IBM"

};

static const char *GetLicensingFeaturesStr(E_FLEXERA_LICENSE_FEATURES type)
{
	const char *name = (0 <= type && type < MAX_NUM_OF_FEATURES
						?
								licensingFeaturesStr[type] : "Invalid feature idx");
	return name;
}










typedef struct
{
	E_FLEXERA_LICENSE_FEATURES                  LicenseFeature;
	BOOL                                        IsEnabled;
	E_FLEXERA_LICENSE_VALIDATION_STATUS         LicenseStatus;
	BOOL                                        IsChanged;
	DWORD                                       Counted;
	CStructTm                                   expirationDate;
	//std::string                                      version;

} FLEXERA_COMPONENT_DATA_S;



 typedef struct
 {
	 FLEXERA_COMPONENT_DATA_S featuresArray[MAX_NUM_OF_FEATURES];

 } FLEXERA_DATA_S;



 //************************************ EE-560 ************************************************

 enum eLicensingConnectionStatus
 {
 	eLicensingConnectionNotAttempt = 0,
 	eLicensingConnectionConnecting,
 	eLicensingConnectionSuccess,
 	eLicensingConnectionFail,
 	eLicensingConnectionUnknown,
 	NUM_OF_CONNECTION_STATUS
 };




 static const char *licensingConnectionStatusStr[] =
 {
 	"connection_not_attempted",
 	"connecting",
 	"connect_success",
 	"connect_failure",
 	"unknown"

 };

 static const char *GetLicensingConnectionStatusStr(eLicensingConnectionStatus type)
 {
 	const char *name = (0 <= type && type < NUM_OF_CONNECTION_STATUS
 						?
 								licensingConnectionStatusStr[type] : "Invalid Connection Status");
 	return name;
 }


 enum eLicensingStatus
 {
 	eLicensingStatusValid = 0,
 	eLicensingStatusInvalid,
 	eLicensingStatusRestartRequired,
 	eLicensingStatusUnknown,
 	NUM_OF_LICENSING_STATUS
 };

 static const char *licensingStatusStr[] =
 {
 	"valid",
 	"invalid",
 	"valid_system_restart_required",
 	"unknown"

 };

 static const char *GetLicensingStatusStr(eLicensingStatus type)
 {
 	const char *name = (0 <= type && type < NUM_OF_LICENSING_STATUS
 						?
 								licensingStatusStr[type] : "Invalid Licensing Status");
 	return name;
 }



 enum eLicensingConnectionTime
  {
  	eLicensingConnectionLastAttemptTime = 0,
  	eLicensingConnectionSuccessTime,
  	NUM_OF_CONNECTION_TIME
  };




  static const char *licensingConnectionTimeStr[] =
  {
  	"connection_last_attempted_time",
  	"connection_success_time",
  	"unknown"

  };

  static const char *GetLicensingConnectionTimeStr(eLicensingConnectionTime type)
  {
  	const char *name = (0 <= type && type < NUM_OF_CONNECTION_TIME
  						?
  								licensingConnectionTimeStr[type] : "Invalid Connection time mode");
  	return name;
  }

 //************************************************************************************



#endif /*_LicensingServerStructs_H_*/

