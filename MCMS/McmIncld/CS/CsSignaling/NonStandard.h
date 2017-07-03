//	NonStandard.h
//	Mikhail Karasik.
//	Ran Stone.

#ifndef __NONSTANDARD_H__
#define __NONSTANDARD_H__

//	Include:
//----------

#include "DataTypes.h"
#include "IpCommonDefinitions.h"


// Macro definitions:
//-------------------

//#define	NonStandard_Object_Size			128 // 128 byte by standard (needed in ASNExt).
//#define	NonStandard_Data_Size			128 // 128 byte by standard (needed in ASNExt).
//#define CT_NonStandard_Data_Size		48  // Used for nonStandard messages to/from mcms.
#define	Size2							 2  // Temp data length - should be changed when genric is implemented.

//enum:
//-----
typedef enum{
	nsRequest,
	nsResponse,
	nsCommand,
	nsIndication,
}nonStandardSend;
//	Structures:
//-------------

// =============================================================
// Standard H245 NonStandard structure.
// This Struct is a copy of cm.h cmNonStandardIdentifier
// =============================================================

typedef struct {
// Set one of the following choises 
// 1. object id 
  char		object[NonStandard_Object_Size];				// Don't chenge object size 128.
  APIU32	objectLength; // in bytes. <=0 if not set		//4
// 2. h.221 id 
  APIU8		t35CountryCode;									//1
  APIU8		t35Extension;									//1
  APIU16	manufacturerCode;								//2
} nonStandardIdentifierSt;									//total = 136

typedef struct
{
    nonStandardIdentifierSt		info;
	char						data[NonStandard_Data_Size];
} nonStandardParameterSt;

typedef struct	{
//	commonHeader				header;
	nonStandardParameterSt		nonStandardData;
} nonStandardSt;


// =============================================================
// CallTask implementation of the NonStandard structure for 
// internal MCMS <=> CallTask messages.
// =============================================================

//  Currently we do not support any nonStandard identifier objects.
//  We discharge the object peremeter in order to reduce capability
//	message size.


typedef struct {
// 1. object id 
//  char		object[128]; 	
	APIU32	objectLength;									//4
// 2. h.221 id 
  APIU8		t35CountryCode;									//1
  APIU8		t35Extension;									//1
  APIU16	manufacturerCode;								//2
} ctNonStandardIdentifierSt;								//total = 8

typedef struct
{
    ctNonStandardIdentifierSt	info;
	char						data[CT_NonStandard_Data_Size];
} ctNonStandardParameterSt;

typedef struct	{
	ctNonStandardParameterSt	nonStandardData;
} mcNonStandardSt;

#endif// __NONSTANDARD_H__
