// XmlFormat.h
// Ami Noy

#ifndef _XMLFORMAT_H_
#define _XMLFORMAT_H_

// Includes files
//----------------

// Macros
//--------
// Use it only in the scope of the file, where the table is defined.
// !!! Don't use it in a file where the table is extern.
#define NumOfElements(a) (sizeof(a) / sizeof(dynamicTableStruct))

#define XmlFormatSize sizeof(genXmlFormat)

// num of dynamic elements in table
#define NumOfDynamicCaps MAX_CAP_TYPES

typedef enum {

	noElemType,
	rootElemType,
	parentElemType,
	childElemType,
	siblingElemType,

} elementsTypes;

typedef enum {
	
	tagVarType,
	dynamicVarType,
	structVarType,
	unionVarType,
	shortListVarType,
	longListVarType,
	charVarType,
	stringVarType,
//	dynamicStringVarType,
	shortVarType,
	longVarType,
	dynamicDataVarType,
	charArrayVarType,
	binVarType

} variablesTypes;

typedef struct {

	int     xmlType;
	int     varType;
	char               *name;
	unsigned long      nElements;
	unsigned long      val;
	int		minVal;
	int		maxVal;
	int		defaultVal;

} genXmlFormat;

//typedef struct {
//
//	unsigned short	numOfDynamics;
//	unsigned short	sizeOfDynamics;
//
//} xmlDynamicProperties;

//typedef struct {
//
//	unsigned short	dynamicType;
//	unsigned short	dynamicLength;
//
//} xmlDynamicElementProperties;

typedef struct {

	int type;
	int format; 

} dynamicTableStruct;

typedef struct {

    int 			nElements;
    genXmlFormat 	*pGenFormat;

} genXmlElements;

typedef struct {

	unsigned long		length;
	char	paramBuffer[1];

} xmlDynamicStringSt;


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



#endif // _XMLFORMAT_H_
