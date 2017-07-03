// ConferenceMCMFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"

// Macros
//--------

#undef	SRC
#define SRC	"ConfMCMFormat"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------

//	External Routines:
//--------------------

//typedef struct{	
//	int					ConReqOpcode; // from ConferenceResponseEnum
//	UINT32				val1;
//	UINT32				val2;
//}genConferenceRequestStruct;

genXmlFormat genConferenceRequestStructFormat[] = {
	{parentElemType,	tagVarType,	"genConferenceRequestStruct",	3,	0,0,0,0},
		{childElemType,		longVarType,	"ConReqOpcode",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val1",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val2",			0,	0,0,0,0},
};
//-----------------------------------------------------------------------------

//typedef struct{	
//	int					ConResOpcode;	// from ConferenceResponseEnum
//	UINT32				val1;
//	UINT32				val2;
//	char				strID[128];
//}genConferenceResponseStruct;

genXmlFormat genConferenceResponseStructFormat[] = {
	{parentElemType,	tagVarType,	"genConferenceResponseStruct",	4,	0,0,0,0},
		{childElemType,		longVarType,	"ConResOpcode",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val1",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val2",			0,	0,0,0,0},
		{siblingElemType,	stringVarType,	"strID",		0,	128,0,0,0},
};
//-----------------------------------------------------------------------------

//typedef struct{
//	int					ConComOpcode;	//from ConferenceCommandEnum
//	UINT32				val1;
//	UINT32				val2;
//	char				conID[16];
//}genConferenceCommandStruct;

genXmlFormat genConferenceCommandStructFormat[] = {
	{parentElemType,	tagVarType,	"genConferenceCommandStruct",	4,	0,0,0,0},
		{childElemType,		longVarType,	"ConComOpcode",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val1",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val2",			0,	0,0,0,0},
		{siblingElemType,	stringVarType,	"conID",		0,	16,0,0,0},

};
//-----------------------------------------------------------------------------

//typedef struct{
//	int					ConIndOpcode;	//from ConferenceIndicationEnum
//	UINT32				val1;
//	UINT32				val2;
//}genConferenceIndicationStruct;

genXmlFormat genConferenceIndicationStructFormat[] = {
	{parentElemType,	tagVarType,	"genConferenceIndicationStruct",	3,	0,0,0,0},
		{childElemType,		longVarType,	"ConIndOpcode",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val1",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"val2",			0,	0,0,0,0},
};
//-----------------------------------------------------------------------------
