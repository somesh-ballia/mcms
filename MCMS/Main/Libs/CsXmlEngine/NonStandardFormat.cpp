// NonStandardFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpCsSizeDefinitions.h"

// Macros
//--------

#undef	SRC
#define SRC	"NonStndFormat"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------

//	External Routines:
//--------------------

//typedef struct {
//// Set one of the following choises 
//// 1. object id 
//  char		object[NonStandard_Object_Size];				// Don't chenge object size 128.
//  UINT32	objectLength; // in bytes. <=0 if not set		//4
//// 2. h.221 id 
//  UINT8		t35CountryCode;									//1
//  UINT8		t35Extension;									//1
//  UINT16	manufacturerCode;								//2
//} nonStandardIdentifierSt;									//total = 136

genXmlFormat nonStandardIdentifierStFormat[] = {
	{parentElemType,	tagVarType,	"nonStandardIdentifierSt",	5,	0,0,0,0},
		{childElemType,		charArrayVarType,	"object",			0,	NonStandard_Object_Size,0,0,0},
		{siblingElemType,	longVarType,	"objectLength",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"t35CountryCode",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"t35Extension",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"manufacturerCode",	0,	0,0,0,0},
};
//---------------------------------------------------------------------------------------------
//typedef struct
//{
//    nonStandardIdentifierSt		info;
//	char						data[NonStandard_Data_Size];
//} nonStandardParameterSt;

genXmlFormat nonStandardParameterStFormat[] = {
	{parentElemType,	tagVarType,	"nonStandardParameterSt",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,		1,	(int) &nonStandardIdentifierStFormat,0,0,0},
		{childElemType,		charArrayVarType,	"data",		0,	NonStandard_Data_Size,0,0,0},
};
//---------------------------------------------------------------------------------------------

//typedef struct {
//// 1. object id 
////  char		object[128]; 	
//  UINT32	objectLength;									//4
//// 2. h.221 id 
//  UINT8		t35CountryCode;									//1
//  UINT8		t35Extension;									//1
//  UINT16	manufacturerCode;								//2
//} ctNonStandardIdentifierSt;								//total = 8

genXmlFormat ctNonStandardIdentifierStFormat[] = {
	{parentElemType,	tagVarType,	"ctNonStandardIdentifierSt",	4,	0,0,0,0},
		{childElemType,		longVarType,	"objectLength",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"t35CountryCode",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"t35Extension",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"manufacturerCode",	0,	0,0,0,0},
};
//---------------------------------------------------------------------------------------------

//typedef struct
//{
//    ctNonStandardIdentifierSt	info;
//	char						data[CT_NonStandard_Data_Size];
//} ctNonStandardParameterSt;

genXmlFormat ctNonStandardParameterStFormat[] = {
	{parentElemType,	tagVarType,	"ctNonStandardParameterSt",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,		1,	(int) &ctNonStandardIdentifierStFormat,0,0,0},
		{childElemType,		charArrayVarType,	"data",		0,	CT_NonStandard_Data_Size,0,0,0},
};
//---------------------------------------------------------------------------------------------

//typedef struct	{
//	ctNonStandardParameterSt	nonStandardData;
//} mcNonStandardSt;
//
genXmlFormat mcNonStandardStFormat[] = {
	{parentElemType,	tagVarType,	"mcNonStandardSt",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,		1,	(int) &ctNonStandardParameterStFormat,0,0,0},
};
//---------------------------------------------------------------------------------------------
