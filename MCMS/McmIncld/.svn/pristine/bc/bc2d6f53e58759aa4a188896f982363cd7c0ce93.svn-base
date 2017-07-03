/********************************************************************************
*                        P O L Y C O M    I s r a e l
*********************************************************************************
*
*   Title:           H.460.1 Generic Extensible Framework:
*                    Mcms - Stack-Controller interface.
*
*   Filename:        $Workfile: $
*   SubSystem:       IP card / Stack Controller
*   Project:         APOLLO 
*   Authors:         Avi Miller
*   Latest update:   $Modtime: $
*   Created:         7 September, 2004
*
*********************************************************************************
*  Description:
*      
*********************************************************************************
*  $Header: $  
*                                                               
*  Change history:
*  ---------------
*  $History: $
*
********************************************************************************/
#ifndef  __H460_1_H__
#define  __H460_1_H__

/*=======   Compliation Switches   =======================================*/

/*=======   Macro definitions   ==========================================*/

/*=======   Exported const, type and variable definitions   ==============*/
#include "DataTypes.h"
#include "IpCsSizeDefinitions.h"
#include "CsHeader.h"

/* Negotiation */
/* ----------- */

/* Avaya specific: */
#define H460_C_AvfSubFsId	0x01

typedef enum
{
	H460_K_FsId_None = 0,
	H460_K_FsId_Avaya,
	/* Future FeatureSet IDs go here */
	H460_K_FsId_Max = H460_K_FsId_Avaya
} h460_E_FsId;

typedef struct
{
	APIU32	fsId;	//from enum h460_E_FsId
	APIU32	subFs;	//for Avaya, use: H460_C_AvfSubFsId   
} h460FsNegSt;

typedef struct
{
	h460FsNegSt			needFs;		/* Needed */
	h460FsNegSt			desireFs;   /* Desired */
	h460FsNegSt			supportFs;  /* Supported */	
}h460FsSt;
/* Far-End VendorId Request - Indication */
/* ------------------------------------- */

/* Request */
typedef struct 
{
	APIU32	fsId;	//from enum h460_E_FsId; xxx_None: Don't send far-end-Resuset.
} h460AvayaFeVndrReqSt;

/* Indication */
//#define H460_C_ProdIdMaxSize    40
//#define H460_C_VerIdMaxSize     40
//#define H460_C_EntrpNumMaxSize  24

// Parameter identifier::standard = 3
typedef struct
{
	APIU32	     fsId; //from enum h460_E_FsId; xxx_None: far-end-Indication info wasn't received.
	APIU8	     countryCode;
	APIU8        t35Extension;
	APIU8        bSipCM;								/* indicate sip behind ACM */
	APIU8        filler1;
	APIU16       manfctrCode;
	APIU16       filler2;
	char         productId[H460_C_ProdIdMaxSize];	    /* C string; Null if absent */
	char         versionId[H460_C_VerIdMaxSize];	    /* C string; Null if absent */
	char         enterpriseNum[H460_C_EntrpNumMaxSize]; /* C String; Null if absent */
} h460AvayaFeVndrIndSt;

// MaxNonAudioBitRate Indication
// Parameter identifier::standard::7
// Received on :ACF, BRQ & Facility
typedef struct
{
	APIU32	     fsId; 		// from enum h460_E_FsId;
	APIU32		 bitRate;
} h460AvayaFeMaxNonAudioBitRateInd ;

// DSCP Indication
// Parameter identifier::standard::8
// Received on : first RCF.
typedef struct
{
	APIU32	     fsId; 		// from enum h460_E_FsId; 
	APIU16		 audioDscp; // lower 2 bytes
	APIU16		 videoDscp; // higher 2 bytes
} h460AvayaFeDscpInd ;


//-------------------------------------------------------------------------

// RAI conference profile 
typedef struct
{
	char 				profileE164ID[MaxAliasLength];  // Mandatory - This should never be omitted! 
	APIU16  			minimumPorts; 					// if zero is sent - omit this value
	APIU16				partyCallRate;					// if zero is sent - omit this value
	APIU16				numOfPortsAvailable;			// if 0xffff - omit this value
	APIU16				maxNumOfPortsCapacity;			// if zero is sent - omit this value
	APIU16				numOfConferencesAvailable;		// if 0xffff - omit this value
	APIU16				maxNumofConferencesCapacity;	// if zero is sent - omit this value
	APIU8				videoBitRateType;				// Mandatory - This should never be omitted! zero is a legitimate value
} h460ConferenceProfile ;

typedef struct
{
	xmlDynamicHeader 		xmlHeader;
	h460ConferenceProfile	h460ConfProfile;
	
} h460ConferenceProfileExt;

/*=======   Exported function prototypes   ===============================*/

#endif /* __H460_1_H__ */
