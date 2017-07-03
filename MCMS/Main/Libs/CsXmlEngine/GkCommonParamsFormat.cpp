// GkCommonParamsFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpCsSizeDefinitions.h"
#include "IpChannelParams.h"

// Macros
//--------

#undef	SRC
#define SRC	"GkCommonParamsFormat"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------


//	External Routines:
//--------------------
extern genXmlFormat ctNonStandardIdentifierStFormat[];
extern genXmlFormat xmlDynamicHeaderFormat[];
extern genXmlFormat mcXmlTransportAddressFormat[];
extern genXmlFormat xmlDynamicPropertiesFormat[];
extern genXmlFormat xmlUnionPropertiesFormat[];
// Forward Declarations:
//----------------------

//typedef struct
//{
//    ctNonStandardIdentifierSt	info;		//8
//    int							productLen;	//4
//    int							versionLen;	//4
//    char						productID[64];//32
//    char						versionID[32];//32
//} mcuVendor;					// total = 80

genXmlFormat mcuVendorFormat[] = {
	{parentElemType,	tagVarType,	"mcuVendor",	5,	0,0,0,0},
		{parentElemType,	structVarType,  "info",   			1,	(int) &ctNonStandardIdentifierStFormat,0,0,0},
		{childElemType,		longVarType,	"productLen", 		0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"versionLen",		0,	0,0,0,0},
		{siblingElemType,	stringVarType,	"productID",		0,	64,0,0,0},
		{siblingElemType,	stringVarType,	"versionID",		0,	32,0,0,0},
};
//-----------------------------------------------------------------------------------------------
//typedef struct {
//	xmlDynamicHeader		xmlHeader;	
//	BOOL					bNeedToRegister;
//	mcXmlTransportAddress	rasAddress;		
//	int						priority;
//	APIU32					gkIdentLength;
//	char					gkIdent[MaxIdentifierSize];
//} alternateGkSt;

genXmlFormat alternateGkStFormat[] = {
	{parentElemType,	tagVarType,	"alternateGkSt",	6,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		longVarType,	"bNeedToRegister", 	0,	0,0,0,0},
		{parentElemType,	structVarType,  "rasAddress",   	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{siblingElemType,	longVarType,	"priority",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"length",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"gkIdent",	0,	MaxIdentifierSize,0,0,0},
};
//-----------------------------------------------------------------------------------------------
dynamicTableStruct altGkTbl[] = {
	{tblAltGk,	(int)&alternateGkStFormat},
};

//typedef struct {
//	APIU32					bIsReject;
//	int			    		numOfAltGks; //like altGksListStBase
//	xmlDynamicProperties    xmlDynamicProps;	
//	char					altGkSt[1];  //list of alternateGkSt
//} altGksListSt;

genXmlFormat altGksListStFormat[] = {
	{parentElemType,	tagVarType,	"altGksListSt",	4,	0,0,0,0},
		{childElemType,		longVarType,	"bIsReject", 	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"numOfAltGks",	0,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType,	"altGkSt",		NumOfElements(altGkTbl),	(int) &altGkTbl,0,0,0},	//Dynamic!!!!!!
};

//--------------------------------------

//typedef struct  {
//	APIU32						bIsReject;
//	APIU32						rejectReason;	
//	BOOL				    	bAltGkPermanent;
//	altGksListSt				altGkList;   	
//} rejectInfoSt;

genXmlFormat rejectInfoStFormat[] = {
	{parentElemType,	tagVarType,	"rejectInfoSt",	4,	0,0,0,0},
		{childElemType,		longVarType,	"bIsReject", 		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"rejectReason",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bAltGkPermanent",	0,	0,0,0,0},		
		{parentElemType,	structVarType,	"altGkList",		1,(int)	&altGksListStFormat,0,0,0},
};

//---------------------------------------

int rejOrConfTbl[] = {
	(int)&rejectInfoStFormat,
	(int)&altGksListStFormat,
};


genXmlFormat mcRejectOrConfirmChoiceFormat[] = {
	{parentElemType,	tagVarType,		"mcRejectOrConfirmChoice",	1,	0,0,0,0},
		{parentElemType,	unionVarType,		NULL,				1,(int)&rejOrConfTbl, 2, SizeOfRejOrConfUnion, 0},			
};

genXmlFormat mcXmlRejectOrConfirmChoiceFormat[] = {
	{parentElemType,	tagVarType,		"mcXmlRejectOrConfirmChoiceFormat",	2,	0,0,0,0},
		{parentElemType,	structVarType,		NULL,				1,(int)	&xmlUnionPropertiesFormat,0,0,0},	
		{parentElemType,	structVarType,		NULL,				1,(int)&mcRejectOrConfirmChoiceFormat,0,0,0},		
};




