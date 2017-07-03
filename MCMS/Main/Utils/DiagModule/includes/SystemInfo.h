/*===============================================================================================================*/   
/*            Copyright        2008  Polycom Israel, Ltd. All rights reserved                            	     */    
/*---------------------------------------------------------------------------------------------------------------*/    
/* NOTE: This software contains valuable trade secrets and proprietary   information of                          */    
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form                */    
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without                      */    
/* prior written authorization from Polycom Israel Ltd.                                                          */    
/*---------------------------------------------------------------------------------------------------------------*/   
/* Header:    SystemInfo.h                                                         								 */ 
/*                                                                                                               */ 
/* Purpose:   Header of SystemInfo Module: System Info Database.                   			 					 */ 
/*                                                                                                               */ 
/* Author:    Yuval Tepper                                                                                       */ 
/*                                                                                                               */ 
/* Revision:  1.0                      Modtime: 19-Aug-2008                                                      */ 
/*****************************************************************************************************************/

#ifndef _SYSTEM_INFO_H_
#define _SYSTEM_INFO_H_

/***************************************************************************/
/* Common include files                                                    */
/***************************************************************************/
#include <string.h>
#include "stdio.h"
#include "stdlib.h" 
#include "string.h"
#include "stdarg.h"
#include "DiagDataTypes.h"
#include "SharedDefines.h"
#include "Print.h"
//#include "CardsStructs.h"
#include "McuMngrStructs.h"


/***************************************************************************/
/* Defines                                                   */
/***************************************************************************/

/***************************************************************************/
/* Enumerations  definition                                                   */
/***************************************************************************/


typedef enum
{
    eChassisType_Unknown = 0,
	eChassisType_RMX2000 = 1,
    eChassisType_RMX4000 = 2,
	eChassisType_RMX1500 = 3,
	eChassisType_Gesher = 4,
	eChassisType_Ninja = 5,
    eChassisType_Max

}	eChassisType; 


//Parameters Names Enum (New system Parameters should be added here)
	typedef enum
	{
	
	  ePLATFORM_TYPE_REQ = 0,
//	  eSWITCH_SLOT_ID_NUM = 1,
	  eFutureUse1,
	  eFutureUse2,
	  eMAX_SYSTEM_INFO_PARAMETER 
	
	} EN_SYSTEM_INFO_DATABASE_PARAMETER;


/***************************************************************************/
/* Structurs definitions                                            */
/***************************************************************************/

/*****System Info DataBase Struct (New system Parameters should be added here)******/ 

	typedef struct 
	{
		UINT32 PlatformType;
//		UINT32 SwitchSlotID;
		UINT32 FutureUse2;
		UINT32 FutureUse3;
	
	} ST_SYSTEM_INFO_DATABASE;

/**********************************************************************************/

/***************************************************************************/
/* 						Functions prototypes                               */
/***************************************************************************/

/*************** System Info DataBase Functions ********************/

	// Get Parameter from System Info Databas
	extern UINT32 SystemInfoGetParam(EN_SYSTEM_INFO_DATABASE_PARAMETER eParameterName, void *pReturnValue ,UINT8 *sourceName, BOOL bIsNeedToPrint);
	
	// Set System Info Database Parameter value
	extern UINT32 SystemInfoSetParam(EN_SYSTEM_INFO_DATABASE_PARAMETER eParameterName, void *pValueToSet  ,UINT8 *sourceName, BOOL bIsNeedToPrint);
	
	// Init Database (On Start-Up)
	extern UINT32 SystemInfoDatabaseInit(UINT8 *sourceName, BOOL bIsNeedToPrint , eChassisType chassisType);
	
	// Init One Parameters
	extern UINT32 SystemInfoDatabaseInitPlatformType(UINT8 *sourceName, BOOL bIsNeedToPrint , eChassisType chassisType);
//	extern UINT32 SystemInfoDatabaseInitSwitchSlotID(UINT8 *sourceName, BOOL bIsNeedToPrint);

	extern eChassisType GetProductType();
/********************************************************************/

#endif /* _SYSTEM_INFO_H_ */

