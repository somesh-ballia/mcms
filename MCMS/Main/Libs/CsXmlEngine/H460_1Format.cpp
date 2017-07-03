// H460_1Format.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpCsSizeDefinitions.h"
// Macros
//--------

#undef	SRC
#define SRC	"H460_1Format"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------
extern genXmlFormat xmlDynamicHeaderFormat[];

//	External Routines:
//--------------------

//typedef struct
//{
//	UINT32	fsId;	//from enum h460_E_FsId
//	UINT32	subFs;	//for Avaya, use: H460_C_AvfSubFsId   
//} h460FsNegSt;

genXmlFormat h460FsNegStFormat[] = {
	{parentElemType,	tagVarType,	"h460FsNegSt",	2,	0,0,0,0},
		{childElemType,		longVarType,	"fsId",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"subFs",		0,	0,0,0,0},

};
//-------------------------------------------------------------------------------

//typedef struct
//{
//	h460FsNegSt			needFs;		/* Needed */
//	h460FsNegSt			desireFs;   /* Desired */
//	h460FsNegSt			supportFs;  /* Supported */	
//}h460FsSt;

genXmlFormat h460FsStFormat[] = {
	{parentElemType,	tagVarType,	"h460FsSt",	3,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,			1,	(int) &h460FsNegStFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,			1,	(int) &h460FsNegStFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,			1,	(int) &h460FsNegStFormat,0,0,0},
};

//-------------------------------------------------------------------------------

//typedef struct 
//{
//	UINT32	fsId;	//from enum h460_E_FsId; xxx_None: Don't send far-end-Resuset.
//} h460AvayaFeVndrReqSt;

genXmlFormat h460AvayaFeVndrReqStFormat[] = {
	{parentElemType,	tagVarType,	"h460AvayaFeVndrReqSt",	1,	0,0,0,0},
		{childElemType,		longVarType,	"fsId",				0,	0,0,0,0},
};
//-------------------------------------------------------------------------------

//typedef struct
//{
//	UINT32	     fsId; //from enum h460_E_FsId; xxx_None: far-end-Indication info wasn't received.
//	UINT8	     countryCode;
//	UINT8        t35Extension;
//	UINT8        bSipCM;
//	UINT8        filler1;
//	UINT16       manfctrCode;
//	UINT16       filler2;
//	char         productId[H460_C_ProdIdMaxSize];	    /* C string; Null if absent */
//	char         versionId[H460_C_VerIdMaxSize];	    /* C string; Null if absent */
//	char         enterpriseNum[H460_C_EntrpNumMaxSize]; /* C String; Null if absent */
//} h460AvayaFeVndrIndSt;

genXmlFormat h460AvayaFeVndrIndStFormat[] = {
	{parentElemType,	tagVarType,	"h460AvayaFeVndrIndSt",	10,	0,0,0,0},
		{childElemType,		longVarType,	"fsId",				0,	0,0,0,0},
		{siblingElemType,	charVarType,	"countryCode",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"t35Extension",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"bSipCM",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"filler1",			0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"manfctrCode",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"filler2",			0,	0,0,0,0},
		{siblingElemType,	stringVarType,  "productId",		0,	H460_C_ProdIdMaxSize,0,0,0},
		{siblingElemType,	stringVarType,  "versionId",		0,	H460_C_VerIdMaxSize,0,0,0},
		{siblingElemType,	stringVarType,  "enterpriseNum",	0,	H460_C_EntrpNumMaxSize,0,0,0},
};
//-------------------------------------------------------------------------------
// MaxNonAudioBitRate Indication
// Parameter identifier::standard::7
// Received on :ACF, BRQ & Facility
//typedef struct
//{
//	APIU32	     fsId; 		// from enum h460_E_FsId;
//	APIU32		 bitRate;
//} h460AvayaFeMaxNonAudioBitRateInd ;

genXmlFormat h460AvayaMaxNonAudioBitRateIndStFormat[] = {
	{parentElemType,	tagVarType,	"h460AvayaFeMaxNonAudioBitRateInd",	2,	0,0,0,0},
		{childElemType,		longVarType, "fsId", 0,	0,0,0,0},
		{siblingElemType,	longVarType, "bitRate",	0, 0,0,0,0},
		
};
//-----------------------------------------------------------------------
// DSCP Indication
// Parameter identifier::standard::8
// Received on : first RCF.
//typedef struct
//{
//	APIU32	     fsId; 		// from enum h460_E_FsId; 
//	APIU16		 audioDscp; // lower 2 bytes
//	APIU16		 videoDscp; // higher 2 bytes
//} h460AvayaFeDscpInd ;

genXmlFormat h460AvayaFeDscpIndFormat[] = {
	{parentElemType,	tagVarType,	"h460AvayaFeDscpIndFormat",	3,	0,0,0,0},
		{childElemType,		longVarType, "fsId", 0,	0,0,0,0},		
		{siblingElemType,	shortVarType, "audioDscp",	0, 0,0,0,0},
		{siblingElemType,	shortVarType, "videoDscp",	0, 0,0,0,0},
};
//-----------------------------------------------------------------------
//// RAI conference profile 
//typedef struct
//{
//	char 				profileE164ID[MaxAliasLength];
//	APIU16  			minimumPorts;
//	APIU16				partyCallRate;
//	APIU16				numOfPortsAvailable;
//	APIU16				maxNumOfPortsCapacity;
//	APIU16				numOfConferencesAvailable;
//	APIU16				maxNumofConferencesCapacity;
//	APIU8				videoBitRateType;
//} h460ConferenceProfile ;

genXmlFormat h460ConferenceProfileIndFormat[] = {
	{parentElemType,	tagVarType,	"h460ConferenceProfileIndFormat",	8,	0,0,0,0},
		{childElemType,		stringVarType, "profileE164ID", 0,	MaxAliasLength,0,0,0},		
		{siblingElemType,	shortVarType, "minimumPorts"	, 0, 0,0,0,0},
		{siblingElemType,	shortVarType, "partyCallRate"	, 0, 0,0,0,0},
		{siblingElemType,	shortVarType, "numOfPortsAvailable"	, 0, 0,0,0,0},
		{siblingElemType,	shortVarType, "maxNumOfPortsCapacity"	, 0, 0,0,0,0},		
		{siblingElemType,	shortVarType, "numOfConferencesAvailable"	, 0, 0,0,0,0},
		{siblingElemType,	shortVarType, "maxNumofConferencesCapacity"	, 0, 0,0,0,0},		
		{siblingElemType,	charVarType,  "videoBitRateType"	, 0, 0,0,0,0},
};

//typedef struct
//{
//	xmlDynamicHeader 		xmlHeader;
//	h460ConferenceProfile	h460ConfProfile;
//	
//} h460ConferenceProfileExt;

genXmlFormat h460ConferenceProfileExtFormat[] = {
	{parentElemType,	tagVarType,	"h460ConferenceProfileExtFormat",	2,	0,0,0,0},
		{parentElemType,	structVarType, "xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},		
		{parentElemType,	structVarType, NULL, 1 ,(int) &h460ConferenceProfileIndFormat,0,0,0},		
};

