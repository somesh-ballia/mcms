// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpSizeDefinitions.h"

// Macros
//--------

#undef	SRC
#define SRC	"H323ComPrmsFormat"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------

//	External Routines:
//--------------------
extern genXmlFormat xmlUnionPropertiesFormat[];
// Forward Declarations:
//----------------------

//typedef struct
//{
//    APIU32		ip; /* IPv4 address */
//} ipAddressV4;

genXmlFormat ipAddressV4Format[] = {
	{parentElemType,	tagVarType,		"ipAddressV4",	1,	0,0,0,0},
		{childElemType,		longVarType,		"ip",				0,	0,0,0,0},
};

//typedef struct
//{
//    APIU8		ip[16]; /* IPv6 address */
//    APIU32		scopeId;
//} ipAddressV6;

genXmlFormat ipAddressV6Format[] = {
	{parentElemType,	tagVarType,		"ipAddressV6",	2,	0,0,0,0},
		{childElemType,		charArrayVarType, "ip" , 0, 16, 0, 0, 0},
		{siblingElemType,	longVarType, "scopeId", 0, 0, 0, 0, 0},
};

int ipAddrTbl[] = {
	(int)&ipAddressV4Format,
	(int)&ipAddressV6Format,
};

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


//typedef struct
//{
//    union	ipAddress addr;	
//    APIU32	ipVersion; // enIpVersion
//    APIU32	port;
//    APIU32	distribution; // enDistribution - Distribution type of this address. for H.245 addresses
//	  APIU32	transportType;	// enTransportType - TCP,UDP,TLS
//} mcTransportAddress;
		

genXmlFormat mcTransportAddressFormat[] = {
	{parentElemType,	tagVarType,		"mcTransportAddress",	5,	0,0,0,0},
		{parentElemType,	unionVarType,		NULL,				1,(int)&ipAddrTbl, 2, SizeOfIpAddressUnion, 0},		
		{childElemType,		longVarType,		"ipVersion",		0,	0,0,0,0},
		{siblingElemType,	longVarType,		"port",				0,	0,0,0,0},		
		{siblingElemType,	longVarType,		"distribution",		0,	0,0,0,0},
		{siblingElemType,	longVarType,		"transportType",	0,	0,0,0,0},
};

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//typedef struct
//{
//    xmlUnionPropertiesSt  unionProps;
//    mcTransportAddress	  transAddr;
//} mcXmlTransportAddress;


genXmlFormat mcXmlTransportAddressFormat[] = {
	{parentElemType,	tagVarType,		"mcXmlTransportAddress",	2,	0,0,0,0},
		{parentElemType,	structVarType,		NULL,				1,(int)	&xmlUnionPropertiesFormat,0,0,0},	
		{parentElemType,	structVarType,		NULL,				1,(int)&mcTransportAddressFormat,0,0,0},		
};

//add for CG_SoftMCU
//typedef struct
//{
//	APIU8  bIsCallGenerator;
//	APIU8  eEndpointModel;
//	APIU16 reserved;
//} mcCallGeneratorParams;

genXmlFormat mcXmlCallGeneratorParamsFormat[] = {
	{parentElemType,	tagVarType,		"mcXmlCallGeneratorParams",	3,	0,0,0,0},	
		{childElemType,	charVarType,		"bIsCallGenerator",		0,	0,0,0,0},
		{siblingElemType,	charVarType,		"eEndpointModel",			0,	0,0,0,0},		
		{siblingElemType,	shortVarType,		"reserved",				0,	0,0,0,0},
		
};

