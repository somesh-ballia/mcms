// SipSdpAndHdrsFormat.cpp
// Avner Lami

// Pre-definitions:
// ----------------
#define SRC	"SipSdpAndHdrsFormat"

// Includes files
//----------------
#include <XmlFormat.h>
#include <XmlDefs.h>
#include "SharedDefines.h"

// External variables:
// -------------------
extern genXmlFormat mcXmlTransportAddressFormat;
//extern genXmlFormat ctCapabilitiesStructFormat;

//===========================================================
genXmlFormat headersListFormat[] = {
    {parentElemType,    tagVarType, "headersList",  2,  0, 0, 0, 0},
        {childElemType,     longVarType,		"length",       0,  0, 0, 0, 0},
        {siblingElemType,   charArrayVarType,	"headersBuffer",  0,  0, 0, 0, 0}
};

genXmlFormat  sipMessageHeadersFormat[] = {
	{parentElemType,	tagVarType,	"sipMessageHeaders",	2,	0, 0, 0, 0},
		{childElemType,		longVarType,		"numOfHeaders",	0,	0, 0, 0, 0},
		{parentElemType,	dynamicDataVarType,	NULL,			1,	(int)&headersListFormat, 0, 0, 0}
};

//typedef struct{
//	int					numOfHeaders;
//	int					headersListLength;
//	char				headersList[1]; //list of numOfHeaders * sipHeaderElement						
//} sipMessageHeaders;

//===========================================================
genXmlFormat sdpListFormat[] = {
	{parentElemType,	tagVarType,	"sdpAndHeadersList",	2,	0, 0, 0, 0},
		{childElemType,		longVarType,		"length",		0,	0, 0, 0, 0},
		{siblingElemType,	charArrayVarType,	"sdpBuffer",	0,	0, 0, 0, 0}
};

genXmlFormat sipSdpAndHeadersFormat[] = {
	{parentElemType,	tagVarType,			"sipSdpAndHeaders",	14,	0, 0, 0, 0},
		{childElemType,     stringVarType,  	"cCname",           0, (CNAME_STRING_MAX_LEN +1),0,0,0},
		{childElemType,		longVarType,		"callRate",				0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"msVideoRateTx",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"msVideoRateRx",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"msAudioRateTx",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"msAudioRateRx",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"sipMediaLinesOffset",	0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"sipMediaLinesLength",	0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"sipHeadersOffset",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"sipHeadersLength",		0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"sipIceOffset",			0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"sipIceLength",			0,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"rssConnectionType",		0,	0, 0, 0, 0},
		{parentElemType,	dynamicDataVarType,	NULL,					1,	(int)&sdpListFormat, 0, 0, 0}
};

//typedef struct sipSdpAndHeaders{
//	APIS8                   cCname[CNAME_STRING_MAX_LEN +1];
//	APIU32					callRate;
//	APIU8					sipMediaLinesOffset;
//	APIU8					sipMediaLinesLength;
//	APIU32					sipHeadersOffset;
//	APIU32					sipHeadersLength;
//	APIU32					sipIceOffset;
//	APIU32					sipIceLength;
//	APIU32					lenOfDynamicSection;
//	APIS8					capsAndHeaders[1];
//} sipSdpAndHeadersSt;

//===========================================================
genXmlFormat contentFormat[] = {
	{parentElemType,	tagVarType,	"contentAndHeadersList",	2,	0, 0, 0, 0},
		{childElemType,		longVarType,		"length",			0,	0, 0, 0, 0},
		{siblingElemType,	charArrayVarType,	"contentBuffer",	0,	0, 0, 0, 0}
};

genXmlFormat sipContentAndHeadersFormat[] = {
	{parentElemType,	tagVarType,			"sipContentAndHeaders",	2,	0, 0, 0, 0},
		{siblingElemType,	longVarType,		"sipHeadersOffset",		0,	0, 0, 0, 0},
		{parentElemType,	dynamicDataVarType,	NULL,					1,	(int)&contentFormat, 0, 0, 0}
};

//typedef struct sipContentAndHeaders{
//	APIU32					sipHeadersOffset;
//	APIU32					lenOfDynamicSection;
//	APIS8					contentAndHeaders[1];
//}sipContentAndHeadersSt;

//-S- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//
genXmlFormat MessDesrFormat_Part[] = {
	{parentElemType,	tagVarType ,  "PartMessageDescriptor",	7,	0, 0, 0, 0},
	{childElemType,		longVarType     , "dwFullMessLen"        ,	0,	0, 0, 0, 0},
	{siblingElemType,	shortVarType    , "wPartLen"             ,	0,	0, 0, 0, 0},
	{siblingElemType,	shortVarType    , "wCsId"                ,	0,	0, 0, 0, 0},
	{siblingElemType,	shortVarType    , "wOpcMessSeq"          ,	0,	0, 0, 0, 0},
	{siblingElemType,	shortVarType    , "wPartSeq"             ,	0,	0, 0, 0, 0},
	{siblingElemType,	shortVarType    , "wNumberOfParts"       ,	0,	0, 0, 0, 0},
	{siblingElemType,	charArrayVarType, "aReserve"             ,	0,	2, 0, 0, 0}
};

genXmlFormat contentFormat_Part[] = {
	{parentElemType,	tagVarType      , "contentAndHeadersList",	3,	0, 0, 0, 0},
	{parentElemType,	structVarType   , "sMessagePartDescr"    ,	1,	(int)&MessDesrFormat_Part, 0, 0, 0},
	{childElemType,		longVarType     , "length"               ,	0,	0, 0, 0, 0},
	{siblingElemType,	charArrayVarType, "contentBuffer"        ,	0,	0, 0, 0, 0}
};

genXmlFormat sipContentAndHeadersFormat_Part[] = {
	{parentElemType,	tagVarType,			"sipContentAndHeaders_Part",	2,	0, 0, 0, 0},
	{siblingElemType,	longVarType,		"sipHeadersOffset",		0,	0, 0, 0, 0},
	{parentElemType,	dynamicDataVarType,	NULL,					1,	(int)&contentFormat_Part, 0, 0, 0}
};
//-E- BRIDGE-13277.[Add. 1] BIG_BUFF ----------------------//

//-------------- bfcp message format -----------------------------
genXmlFormat bfcpMessageFormat[] = {
    {parentElemType,    tagVarType, "bfcpMessage",  2,  0,0,0,0},
        {childElemType,     longVarType,            "length",       0,  0,0,0,0},
        {siblingElemType,   charArrayVarType,   	"buffer",  	0,  0,0,0,0},
};
