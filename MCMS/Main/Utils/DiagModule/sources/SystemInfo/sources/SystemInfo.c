/*===============================================================================================================*/
/*            Copyright        2008  Polycom Israel, Ltd. All rights reserved                            	     */
/*---------------------------------------------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary   information of                          */
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form                */
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without                      */
/* prior written authorization from Polycom Israel Ltd.                                                          */
/*---------------------------------------------------------------------------------------------------------------*/
/* Header:    SystemInfo.h                                                         								     */
/*                                                                                                               */
/* Purpose:   Handle System Info Database      		                                 */
/*                                                                                                               */
/* Author:    Yuval Tepper                                                                                       */
/*                                                                                                               */
/* Revision:  1.0                      Modtime: 19-Aug-2008                                                      */
/*****************************************************************************************************************/

//Included Files
#define _GNU_SOURCE // for warning of implicit declaration of function ‘getline’ on OS 5.8
#include <stdio.h>
#include "SystemInfo.h"
//#include "e2prom.h"
#include "Print.h"
#include "timers.h"

extern UINT32 VerifyUINT32ParamInBoundaries(UINT32 Value,UINT32 Low_bound, UINT32 High_bound, UINT8* ParamName, UINT32 bIsNeedToPrint);

//Parameters declarations

static UINT32 isDataBaseInitSucceeded = 0;

//Structs Declarations

static ST_SYSTEM_INFO_DATABASE ST_SystemInfoDatabase;
static ST_SYSTEM_INFO_DATABASE *pST_SystemInfoDatabase;

//Enum declaration


//Functions Declaration
eChassisType GetProductType();

/*****************************************************************************/
/* Name:      SystemInfoGetParam                          		             */
/*                                                                           */
/* Purpose:   Get a Parameter from system info database                      */
/*                                                                           */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                       */
/*                                                                           */
/* Params:    eParameterName - Name of the Parameter to get   				 */
/*            pReturnValue -  pointer to the parameter requested value 		 */
/*            sourceName   -  The Process which called the function  	     */
/*                                                                           */
/*****************************************************************************/

UINT32 SystemInfoGetParam(EN_SYSTEM_INFO_DATABASE_PARAMETER eParameterName, void *pReturnValue ,UINT8 *sourceName, BOOL bIsNeedToPrint)
{
	UINT32 ReturnValue;

	ReturnValue = 0;

	//If SystemInfoDataBase was not inited - Return fail
	if(isDataBaseInitSucceeded == FALSE)
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoGetParam) SystemInfoDataBase was not inited or not inited succesfuly - couldn't complete request");
		return -1;
	}

	if(bIsNeedToPrint)
	{
		//Declare the Caller and the requested parameter
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(SystemInfoGetParam) - %s Asked For the following parameter : %d (EN_SystemInfoDataBaseParameter)", sourceName,eParameterName);
	}

	//Switch on Parameter Name and assign value.
	switch (eParameterName)
		{
		case ePLATFORM_TYPE_REQ:
			{
				//Verify Parameter Boundaries
				ReturnValue	 = VerifyUINT32ParamInBoundaries(pST_SystemInfoDatabase->PlatformType, 	DEFAULTLOWBOUNDARY , NUM_OF_PLATFORM_TYPES , "ePLATFORM_TYPE"   , bIsNeedToPrint);
				if(ReturnValue!=0)
				{
					MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoGetParam) - %s Asked For Parameter number : %d (EN_SystemInfoDataBaseParameter).Parameter is out of its boundaries.", sourceName,eParameterName);
				}
				else
				{
					//Assign the parameter value to the pointer
					*((UINT32*)pReturnValue) = pST_SystemInfoDatabase->PlatformType;
					MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(SystemInfoGetParam) - %s Asked For Parameter number : %d (EN_SystemInfoDataBaseParameter), got value: %d.", sourceName,eParameterName, pST_SystemInfoDatabase->PlatformType);
				}
				break;
			}
		/*
		case eSWITCH_SLOT_ID_NUM:
			{
				//Verify Parameter Boundaries
				ReturnValue	 = VerifyUINT32ParamInBoundaries(pST_SystemInfoDatabase->SwitchSlotID, 	DEFAULTLOWBOUNDARY , eSWITCH_SLOT_ID_NUM_MAX , "EN_SWITCH_SLOT_ID_NUM"   , bIsNeedToPrint ,NULL, NULL);
				if(ReturnValue!=0)
				{
					MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoGetParam) - %s Asked For Parameter number : %d (EN_SystemInfoDataBaseParameter).Parameter is out of its boundaries.", sourceName,eParameterName);
				}
				else
				{
					//Assign the parameter value to the pointer
					*((UINT32*)pReturnValue) = pST_SystemInfoDatabase->SwitchSlotID;
					MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(SystemInfoGetParam) - %s Asked For Parameter number : %d (EN_SystemInfoDataBaseParameter), got value: %d.", sourceName,eParameterName, pST_SystemInfoDatabase->SwitchSlotID);
				}
				break;
			}*/
			//Add more Parameters here
		default:
			{
				//Declare Unknown Parameter request
				MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoGetParam) - %s Asked For an unknow Parameter number : %d (EN_SystemInfoDataBaseParameter)", sourceName,eParameterName);
				ReturnValue = -1;
			}
		}

	return ReturnValue;
}

////////////////////////////////////////////////////////////////////////////////



/*****************************************************************************/
/* Name:      SystemInfoSetParam                          		             */
/*                                                                           */
/* Purpose:   Set Value for a Parameter from system info database            */
/*                                                                           */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                            */
/*                                                                           */
/* Params:    eParameterName - Name of the Parameter to set   				 */
/*            pValueToSet   -  pointer to the value to set				     */
/*            sourceName    -  The Process which called the function  	         */
/*                                                                           */
/*****************************************************************************/

UINT32 SystemInfoSetParam(EN_SYSTEM_INFO_DATABASE_PARAMETER eParameterName, void *pValueToSet ,UINT8 *sourceName, BOOL bIsNeedToPrint)
{
	UINT32 ReturnValue;

	ReturnValue = 0;

	if(bIsNeedToPrint)
	{
		//Declare the Caller, the requested parameter, and the set value
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoSetParam) - %s Requested to set %d (EN_SystemInfoDataBaseParameter)", sourceName,eParameterName);
	}

	//Switch on Parameter Name and assign value.
	switch (eParameterName)
		{
		case ePLATFORM_TYPE_REQ:
			{
				//Verify Parameter Boundaries
				ReturnValue	= VerifyUINT32ParamInBoundaries(*((UINT32*)pValueToSet), DEFAULTLOWBOUNDARY , NUM_OF_PLATFORM_TYPES , "ePLATFORM_TYPE"   , bIsNeedToPrint);
				if(ReturnValue!=0)
				{
					MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoSetParam) - %s Requested to set Parameter number : %d (EN_SystemInfoDataBaseParameter) to an illegal value: %d", sourceName,eParameterName, *((UINT32*)pValueToSet));
				}
				else
				{
					//Set the parameter value
					pST_SystemInfoDatabase->PlatformType = *((UINT32*)pValueToSet);
					MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoSetParam) - %s Requested to set Parameter number : %d (EN_SystemInfoDataBaseParameter) to value: %d, set succeeded", sourceName,eParameterName, *((UINT32*)pValueToSet));
				}
				break;
			}
		/*
		case eSWITCH_SLOT_ID_NUM:
			{
				//Verify Parameter Boundaries
				ReturnValue	= VerifyUINT32ParamInBoundaries(*((UINT32*)pValueToSet), DEFAULTLOWBOUNDARY , eSWITCH_SLOT_ID_NUM_MAX , "eSWITCH_SLOT_ID_NUM"   , bIsNeedToPrint ,NULL, NULL);
				if(ReturnValue!=0)
				{
					MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoSetParam) - %s Requested to set Parameter number : %d (EN_SystemInfoDataBaseParameter) to an illegal value: %d", sourceName,eParameterName, *((UINT32*)pValueToSet));
				}
				else
				{
					//Set the parameter value
					pST_SystemInfoDatabase->SwitchSlotID = *((UINT32*)pValueToSet);
					MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoSetParam) - %s Requested to set Parameter number : %d (EN_SystemInfoDataBaseParameter) to value: %d, set succeeded", sourceName,eParameterName, *((UINT32*)pValueToSet));
				}
				break;
			}
			*/
			//Add more Parameters here
		default:
			{
				//Declare Unknown Parameter request
				MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoSetParam) - %s Requested to set an unknow Parameter number : %d (EN_SystemInfoDataBaseParameter)", sourceName,eParameterName);
				ReturnValue = -1;
			}

		}
	return ReturnValue;
}


////////////////////////////////////////////////////////////////////////////////




/********************************************************************************/
/* Name:      SystemInfoDatabaseInit                           		            */
/*                                                                              */
/* Purpose:   Set Value for all system info database parameters (during startup)*/
/*                                                                              */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                               */
/*                                                                              */
/* Params:	  sourceName    -  The Process which called the function            */
/*                                                                              */
/********************************************************************************/

UINT32 SystemInfoDatabaseInit(UINT8 *sourceName, BOOL bIsNeedToPrint, eChassisType chassisType)

{
	UINT32 ReturnValue;

	ReturnValue = 0;

	//Assign Database pointer
	pST_SystemInfoDatabase = &ST_SystemInfoDatabase;

	//Memset Satabase Parameters to -1
	memset(pST_SystemInfoDatabase, -1, sizeof(ST_SYSTEM_INFO_DATABASE));

	if(bIsNeedToPrint)
	{
		//Declare the Caller
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoDatabaseInit) - %s Requested to init the System Info Database", sourceName);
	}

	//Init all Parameters
	printf("yosi - before SystemInfoDatabaseInit !!!!!!!!!!!!!!!!!!!!! \n");
	EmbSleep(1);
	ReturnValue |= SystemInfoDatabaseInitPlatformType("SystemInfoDatabaseInit",bIsNeedToPrint, chassisType);
	//ReturnValue |= SystemInfoDatabaseInitSwitchSlotID("SystemInfoDatabaseInit",bIsNeedToPrint);

	//Add more parameters here

	if(!ReturnValue) //Success
	{
		isDataBaseInitSucceeded = TRUE;
	}
	else //Fail
	{
		isDataBaseInitSucceeded =FALSE;
	}

	return ReturnValue;
}

////////////////////////////////////////////////////////////////////////////////




/********************************************************************************/
/* Name:      SystemInfoDatabaseInitPlatformType   		                        */
/*                                                                              */
/* Purpose:   Init Value for Platform Type parameter  (e.g. amos)   			*/
/*                                                                              */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                          */
/*                                                                              */
/* Params:	  sourceName    -  The Process which called the function            */
/*                                                                              */
/********************************************************************************/

UINT32 SystemInfoDatabaseInitPlatformType(UINT8 *sourceName , BOOL bIsNeedToPrint , eChassisType chassisType)

{
	UINT32 ReturnValue;
	UINT32 PlatformType = eSoftMcu;

	ReturnValue = 0;

	if(bIsNeedToPrint)
	{
		//Declare the Caller
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoDatabaseInitPlatformType) - %s Requested to init PlatformType parameter in the system info database", sourceName);
	}

	//enable after GetChasisPlatformTypeInDword is implemented
	//PlatformType =  GetChasisPlatformTypeInDword(1);
	//[Dotan.H 07/01/2010 Add support for RMX1500]
	if(chassisType == eChassisType_Unknown)
	{
		chassisType = GetProductType();
	}
	
	switch(chassisType)
	{
		case eChassisType_RMX2000:
			PlatformType = eGideonLite;
			break;
		case eChassisType_RMX4000:
			PlatformType = eAmos;
			break;
		case eChassisType_RMX1500:
			PlatformType = eYona;
			break;
		case eChassisType_Gesher:
			PlatformType = eSoftMcu;
			break;
		case eChassisType_Ninja:
			PlatformType = eSoftMcu;
			break;
		default:
			MfaBoardPrint(CM_INITIATOR_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(%s %s %d): Unable to recognize chassis type : %d",__FILE__,__FUNCTION__,__LINE__, chassisType);
			break;
	}

	//Verify Parameters Boundaries (and (&) with return value to assure one error returns error)
	ReturnValue	|= VerifyUINT32ParamInBoundaries(PlatformType, DEFAULTLOWBOUNDARY , NUM_OF_PLATFORM_TYPES , "ePLATFORM_TYPE"   , bIsNeedToPrint);

	//Set Parameter On Database
	ReturnValue |= SystemInfoSetParam(ePLATFORM_TYPE_REQ, &PlatformType ,"SystemInfoDatabaseInit", bIsNeedToPrint);

	return ReturnValue;
}
////////////////////////////////////////////////////////////////////////////////


/********************************************************************************/
/* Name:      SystemInfoDatabaseInitSwitchSlotID   		                        */
/*                                                                              */
/* Purpose:   Init Value for Switch SlotID Number  			        			*/
/*                                                                              */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                          */
/*                                                                              */
/* Params:	  sourceName    -  The Process which called the function            */
/*                                                                              */
/********************************************************************************/
/*
UINT32 SystemInfoDatabaseInitSwitchSlotID(UINT8 *sourceName, BOOL bIsNeedToPrint)

{
	UINT32 ReturnValue;
	UINT32 SwitchSlotID;
	UINT32 PlatformType;

	ReturnValue = 0;
	printf ("yosi - in function SystemInfoDatabaseInitSwitchSlotID \n");
	EmbSleep(1);
	if(bIsNeedToPrint)
	{
		//Declare the Caller
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoDatabaseInitSwitchSlotID) - %s Requested to init SwitchSlotID parameter in the system info database", sourceName);
	}

	//Get Platform Type (Switch SlotID depends on Platform type)
	printf ("yosi - before GetChasisPlatformTypeInDword \n");
	EmbSleep(1);
	//enable after GetChasisPlatformTypeInDword is implemented
	//PlatformType =  GetChasisPlatformTypeInDword(1);
	printf ("yosi - after GetChasisPlatformTypeInDword  PlatformType =%d \n",PlatformType);
	EmbSleep(1);
	//Verify Parameters Boundaries (and (|) with return value to assure one error returns error)
	ReturnValue	|= VerifyUINT32ParamInBoundaries(PlatformType, DEFAULTLOWBOUNDARY , NUM_OF_PLATFORM_TYPES , "ePLATFORM_TYPE"   , bIsNeedToPrint ,NULL, NULL);

	if(!(ReturnValue)) //If Platform type is valid
	{
		switch(PlatformType)
		{
			case eGideonLite:
				SwitchSlotID = eSWITCH_SLOT_ID_NUM_RMX2000;
				break;
			case eAmos:
				SwitchSlotID = eSWITCH_SLOT_ID_NUM_AMOS;
				break;
			case eYona:
				SwitchSlotID = eSWITCH_SLOT_ID_NUM_YONA;
				break;
			default:
				MfaBoardPrint(CM_INITIATOR_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(%s %s %d): Unable to recognize Platform type : %d",__FILE__,__FUNCTION__,__LINE__, PlatformType);
				break;
		}

		//Set Parameter On Database
		ReturnValue |= SystemInfoSetParam(eSWITCH_SLOT_ID_NUM, &SwitchSlotID ,"SystemInfoDatabaseInit", bIsNeedToPrint);

	}
	else
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SystemInfoDatabaseInitSwitchSlotID) - PlatformType not valid, can't determine Switch SlotID");
		ReturnValue = -1;
	}


	return ReturnValue;

}
*/
////////////////////////////////////////////////////////////////////////////////

/********************************************************************************/
/* Name:      StringToProductType   		                        */
/*                                                                              */
/* Purpose:   Init Value for Switch SlotID Number  			        			*/
/*                                                                              */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                          */
/*                                                                              */
/* Params:	  sourceName    -  The Process which called the function            */
/*                                                                              */
/********************************************************************************/
#define STRING_RMX2000        "RMX2000"
#define STRING_RMX4000        "RMX4000"
#define STRING_RMX1500        "RMX1500"
#define STRING_NPG2000        "NPG2000"
#define STRING_CALL_GENERATOR "CALL_GENERATOR"
#define STRING_SOFT_MCU       "SOFT_MCU"
#define STRING_GESHER         "GESHER"
#define STRING_NINJA          "NINJA"
#define STRING_SOFT_MCU_MFW   "SOFT_MCU_MFW"
#define STRING_UNKNWON        "UNKNOWN"

eChassisType StringToProductType(const char * pn_string)
{
    if (pn_string == NULL)
        return eChassisType_Unknown;

    if (strcmp(pn_string,STRING_RMX2000) == 0)
        return eChassisType_RMX2000;

    if (strcmp(pn_string,STRING_RMX4000) == 0)
        return eChassisType_RMX4000;

    if (strcmp(pn_string,STRING_RMX1500) == 0)
        return eChassisType_RMX1500;

    if (strcmp(pn_string,STRING_GESHER) == 0)
        return eChassisType_Gesher;

    if (strcmp(pn_string,STRING_NINJA) == 0)
        return eChassisType_Ninja;

    return eChassisType_Unknown;
}

eChassisType GetProductType()
{
    eChassisType prodType = eChassisType_Ninja;
    FILE *pProductTypeFile = fopen("/mcms/ProductType", "r" );
    if (pProductTypeFile)
    {
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        read = getline(&line, &len, pProductTypeFile );
        if (read != -1)
        {
            prodType = StringToProductType(line);
            if (prodType == eChassisType_Unknown) //read from file and prodtType not analyzed yet so put defaults
            {
            	    prodType = eChassisType_Ninja;
            }
        }
        if (line)
        {
            free(line);
        }

        fclose(pProductTypeFile);
    }
	
    return prodType;
}

