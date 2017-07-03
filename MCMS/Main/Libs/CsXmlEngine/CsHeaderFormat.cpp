// CsHeaderFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"

// Macros
//--------

#undef	SRC
#define SRC	"CapFormat"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------

//	External Routines:
//--------------------

// Forward Declarations:
//----------------------


// Routines:
//----------
//------------- message description header format ----------------------------
//typedef struct
//{
//	APIU16			dynamicType;			// Enc, G711, H263 Etc.
//	APIU16			dynamicLength;			// Dynamic part length.
//} xmlDynamicHeader;

genXmlFormat xmlDynamicHeaderFormat[] = {
	{parentElemType,	tagVarType,	"xmlDynamicHeader",	2,	0,0,0,0},
		{childElemType,		shortVarType,	"dynamicType",0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"dynamicLength",0,	0,0,0,0},
};
//------------------------------------------------------------------------

//typedef struct
//{
//	APIU16			numberOfDynamicParts;
//	APIU16			sizeOfAllDynamicParts;
//}xmlDynamicProperties;

genXmlFormat xmlDynamicPropertiesFormat[] = {
	{parentElemType,	tagVarType,	"xmlDynamicProperties",	2,	0,0,0,0},
		{childElemType,		shortVarType,	"numberOfDynamicParts",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"sizeOfAllDynamicParts",0,	0,0,0,0},
};
//------------------------------------------------------------------------
//typedef struct {/
//
//	int		unionType;
//	int		unionSize;
//
//} xmlUnionPropertiesSt;

genXmlFormat xmlUnionPropertiesFormat[] = {
	{parentElemType,	tagVarType,	"xmlUnionProperties",	2,	0,0,0,0},
		{childElemType,		longVarType,	"unionType",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"unionSize",	0,	0,0,0,0},
};

