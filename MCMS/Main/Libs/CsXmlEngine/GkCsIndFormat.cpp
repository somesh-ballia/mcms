// GkCsIndFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpCsSizeDefinitions.h"
// Macros
//--------

#undef	SRC
#define SRC	"GkCsIndFormat"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------

//	External Routines:
//--------------------
extern genXmlFormat mcXmlTransportAddressFormat[]; 
extern genXmlFormat h460FsStFormat[];
//extern genXmlFormat rejectOrConfirmChoiceFormat[];
extern genXmlFormat	mcXmlRejectOrConfirmChoiceFormat[];
extern genXmlFormat rejectInfoStFormat[];
extern genXmlFormat altGksListStFormat[];
extern genXmlFormat h460AvayaFeVndrIndStFormat[];
extern genXmlFormat h460AvayaMaxNonAudioBitRateIndStFormat[];
extern genXmlFormat h460AvayaFeDscpIndFormat[];
// Forward Declarations:
//----------------------

//typedef struct {
//    APIU32					gkIdentLength;
//    char						gatekeeperIdent[MaxIdentifierSize];	
//    APIS32   				    nRequredAuthMethod   ;// H.235 Gk Auth.Method Required from GK 
//    mcXmlTransportAddress		rasAddress;		// GK transport address
//    h460FsSt					fs;
//    mcRejectOrConfirmChoice   rejectOrConfirmCh;
//} gkIndRasGRQ;

genXmlFormat gkIndRasGRQFormat[] = {
    {parentElemType,	tagVarType,	"gkIndRasGRQ",	6,	0,0,0,0},
    {childElemType,		longVarType,  	"gkIdentLength",		0,	0,0,0,0},
    {siblingElemType,	charArrayVarType,"gatekeeperIdent",		0,	MaxIdentifierSize,0,0,0},
    //--------- H235 GK Authentication params ---------------------------------------------------//
    {siblingElemType,	longVarType,  	"nRequredAuthMethod",	0,	0,0,0,0},
    //-------------------------------------------------------------------------------------------//
    {parentElemType,	structVarType,  "gatekeeperAddress",   	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
    {parentElemType,	structVarType,  "fs",					1,	(int) &h460FsStFormat,0,0,0},				
    {parentElemType,	structVarType,  "rejectOrConfirmCh",	1,	(int) &mcXmlRejectOrConfirmChoiceFormat,0,0,0},	
};

//typedef struct {
//	APIU32					timeToLive;
//	APIU32					gkIdentLength;
//	char					gatekeeperIdent[MaxIdentifierSize];
//	APIU32					epIdentLength;
//	char					endpointIdent[MaxIdentifierSize];	//changed receive upon RCF
//	h460FsSt				fs;
//	rejectOrConfirmChoice   rejectOrConfirmCh; //Alternate GK list
//
//} gkIndRasRRQ; 

genXmlFormat gkIndRasRRQFormat[] = {
	{parentElemType,	tagVarType,	"gkIndRasRRQ",	8,	0,0,0,0},
		{childElemType,		longVarType,  	"timeToLive",			0,	0,0,0,0},	
		{siblingElemType,	longVarType,  	"gkIdentLength",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"gatekeeperIdent",		0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	longVarType,  	"epIdentLength",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"endpointIdent",		0,	MaxIdentifierSize,0,0,0},
		{parentElemType,	structVarType,  "fs",					1,	(int) &h460FsStFormat,0,0,0},				
		{parentElemType,	structVarType,   "fsAvayaFeDscpInd",	1,	(int) &h460AvayaFeDscpIndFormat,0,0,0},
		{parentElemType,	structVarType,  "rejectOrConfirmCh",	1,	(int) &mcXmlRejectOrConfirmChoiceFormat,0,0,0},						
};
//-------------------------------------------------------------
//typedef struct {
//	rejectInfoSt			rejectInfo;							// in case of URJ 
//
//} gkIndRasURQ;

genXmlFormat gkIndRasURQFormat[] = {
	{parentElemType,	tagVarType,	"gkIndRasURQ",	1,	0,0,0,0},
		{parentElemType,	structVarType,  "rejectInfoSt",	1,	(int) &rejectInfoStFormat,0,0,0},						
};
//-------------------------------------------------------------

//typedef struct {
//	mcXmlTransportAddress	destCallSignalAddress;//according to the standart, the dest in the ACF should always (dialIn / dialOut) be the remote				
//	int						crv;	
//	cmRASCallType			callType;							
//	cmRASCallModelType		callModel;	
//	int						bandwidth;
//	int						irrFrequency;
//	APIU32					destExtraCallInfoTypes[MaxNumberOfAliases];
//	h460AvayaFeVndrIndSt    avfFeVndIdInd;
//	char					destExtraCallInfo[MaxAddressListSize];		//canMapAlias
//	char					destInfo[MaxAddressListSize];				//canMapAlias
//	char					remoteExtensionAddress[MaxAliasLength];		//canMapAlias
//	char					conferenceId[MaxConferenceIdSize];
//	char					callId[Size16];
//	rejectInfoSt			rejectInfo;
//} gkIndRasARQ;

genXmlFormat gkIndRasARQFormat[] = {
	{parentElemType,	tagVarType,	"gkIndRasARQ",	17,	0,0,0,0},
		{parentElemType,	structVarType,  "destCallSignalAddress",   	1,	(int) &mcXmlTransportAddressFormat,0,0,0},	
		{childElemType,		longVarType,  	"crv",			0,	0,0,0,0},	
		{siblingElemType,	longVarType,  	"callType",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"callModel",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bandwidth",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"irrFrequency",		0,	0,0,0,0},
		{siblingElemType,	longListVarType,"destExtraCallInfoTypes",		MaxNumberOfAliases,	0,0,0,0},
		{parentElemType,	structVarType,  "avfFeVndIdInd",	1,	(int) &h460AvayaFeVndrIndStFormat,0,0,0},							
		{parentElemType,	structVarType,  "avfFeMaxNonAudioBitRateInd", 1, (int) &h460AvayaMaxNonAudioBitRateIndStFormat,0,0,0},							
		{siblingElemType,	stringVarType,	"destExtraCallInfo",0,	MaxAddressListSize,0,0,0},
		{siblingElemType,	stringVarType,	"destInfo",			0,	MaxAddressListSize,0,0,0},
		{siblingElemType,	stringVarType,	"remoteExtensionAddress",		0,	MaxAliasLength,0,0,0},
		{siblingElemType,	charArrayVarType,"conferenceId",		0,	MaxConferenceIdSize,0,0,0},
		{siblingElemType,	charArrayVarType,"callId",			0,	Size16,0,0,0},
		{siblingElemType,	longVarType,  	"numOfGkRoutedAddres",	0,	0,0,0,0},
		{parentElemType,	structVarType,  "gkRouteAddress",	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "rejectInfoSt",		1,	(int) &rejectInfoStFormat,0,0,0},						
};
//-------------------------------------------------------------
//typedef struct {
//	rejectInfoSt			rejectInfo;							// in case of DRJ
//
//} gkIndRasDRQ;

genXmlFormat gkIndRasDRQFormat[] = {
	{parentElemType,	tagVarType,	"gkIndRasDRQ",	1,	0,0,0,0},
		{parentElemType,	structVarType,  "rejectInfoSt",	1,	(int) &rejectInfoStFormat,0,0,0},						
};
//-------------------------------------------------------------

//typedef struct {
//	int						bandwidth;
//	rejectInfoSt			rejectInfo;							// in case of DRJ
//
//} gkIndRasBRQ;

genXmlFormat gkIndRasBRQFormat[] = {
	{parentElemType,	tagVarType,	"gkIndRasBRQ",	2,	0,0,0,0},
		{childElemType,		longVarType,  	"bandwidth",	0,	0,0,0,0},		
		{parentElemType,	structVarType,  "rejectInfoSt",	1,	(int) &rejectInfoStFormat,0,0,0},						
};
//-------------------------------------------------------------

//typedef struct {
//	char					message[MaxErrorMessageSize];
//	APIU32					FailIndicationOpcode;
//
//} gkIndRasFail;

genXmlFormat gkIndRasFailFormat[] = {
	{parentElemType,	tagVarType,	"gkIndRasFail",	2,	0,0,0,0},
		{childElemType,		stringVarType,	"message",				0,	MaxErrorMessageSize,0,0,0},	
		{siblingElemType,	longVarType,  	"FailIndicationOpcode",	0,	0,0,0,0},
};
//-------------------------------------------------------------
//typedef struct {
//	cmRASTransaction		transaction;
//	mcXmlTransportAddress   gkAddress;	
//
//} gkIndRasTimeout;

genXmlFormat gkIndRasTimeoutFormat[] = {
	{parentElemType,	tagVarType,	"gkIndRasTimeout",	2,	0,	0,0,0},
		{childElemType,		longVarType,	"transaction",	0,	0,0,0,0},
		{parentElemType,	structVarType,  "gkAddress",   		1,	(int) &mcXmlTransportAddressFormat,0,0,0},	
};
//-------------------------------------------------------------
//typedef struct {
//	int			            hsRas;
//    APIU32					unRegisterReason; 
//	altGksListSt			altGkList;  // no need candidate to delete - Ask guy 
//
//} gkIndURQFromGk;

genXmlFormat gkIndURQFromGkFormat[] = {
	{parentElemType,	tagVarType,	"gkIndURQFromGk",	3,	0,0,0,0},
		{childElemType,		longVarType,	"hsRas",			0,	0,0,0,0},	
		{siblingElemType,	longVarType,  	"unRegisterReason",	0,	0,0,0,0},
		{parentElemType,	structVarType,  "altGkList",	1,	(int) &altGksListStFormat,0,0,0},								
};
//-------------------------------------------------------------
//typedef struct {
//	cmRASReason				disengageReason; //GK if not transfer this param to mcms ask guy
//	int			            hsRas;
//
//} gkIndDRQFromGk;

genXmlFormat gkIndDRQFromGkFormat[] = {
	{parentElemType,	tagVarType,	"gkIndDRQFromGk",	2,	0,0,0,0},
		{childElemType,		longVarType,	"disengageReason",	0,	0,0,0,0},	
		{siblingElemType,	longVarType,  	"hsRas",			0,	0,0,0,0},
};
//-------------------------------------------------------------

//typedef struct {
//	int						hsRas;
//	int						bandwidth;
//}gkIndBRQFromGk;

genXmlFormat gkIndBRQFromGkFormat[] = {
	{parentElemType,	tagVarType,	"gkIndBRQFromGk",	3,	0,0,0,0},
		{childElemType,		longVarType,	"hsRas",	0,	0,0,0,0},	
		{siblingElemType,	longVarType,  	"bandwidth",0,	0,0,0,0},
		{parentElemType,	structVarType,  "avfFeMaxNonAudioBitRateInd", 1, (int) &h460AvayaMaxNonAudioBitRateIndStFormat,0,0,0},
};
//-------------------------------------------------------------

//typedef struct {				
//	char					destinationInfo[MaxAddressListSize];	// aliases for dest.
//	char					sourceInfo[MaxAddressListSize];			// aliases for dest.
//	char					callId[Size16];							//Needed ??
//	int			            hsRas;
//
//}gkIndLRQFromGk;

genXmlFormat gkIndLRQFromGkFormat[] = {
	{parentElemType,	tagVarType,	"gkIndLRQFromGk",	5,	0,0,0,0},
		{parentElemType,	structVarType,   "gatekeeperAddress",1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{childElemType,		stringVarType,	 "destinationInfo",	0,	MaxAddressListSize,0,0,0},	
		{siblingElemType,	stringVarType,	 "sourceInfo",		0,	MaxAddressListSize,0,0,0},	
		{siblingElemType,	charArrayVarType,"callId",			0,	Size16,0,0,0},			
		{siblingElemType,	longVarType,  	 "hsRas",			0,	0,0,0,0},
};
//-------------------------------------------------------------
typedef struct {
	int						hsRas;
} gkIndGKIRQ;

genXmlFormat gkIndGKIRQFormat[] = {
	{parentElemType,	tagVarType,	"gkIndGKIRQ",	1,	0,0,0,0},
		{childElemType,		longVarType,	"hsRas",	0,	0,0,0,0},	
};
//-------------------------------------------------------------

