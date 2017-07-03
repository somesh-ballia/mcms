// AcServiceReqFormats.c
// Ami Noy

// Include files:
//---------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpCsSizeDefinitions.h"


// External variables
//-------------------
extern genXmlFormat maintHeaderFormat[];
extern genXmlFormat versionsFormat[];
extern genXmlFormat commonParamsFormat[];
extern genXmlFormat terminalCommandFormat[];

//---------------------  cs new service init request format ---------------------------
genXmlFormat srCsNewServiceInitReqFormat[] = {
	{rootElemType,	tagVarType,	"CS_NEW_SERVICE_INIT_REQ",	2,	0,0,0,0},
		{parentElemType, structVarType,			NULL,	1, (int) &maintHeaderFormat,0,0,0},
		{parentElemType, dynamicDataVarType,	NULL,	1, (int) &commonParamsFormat,0,0,0}
};

//---------------------  cs new common param request format ---------------------------
genXmlFormat srCsCommonParamReqFormat[] = {
    {rootElemType,  tagVarType, "CS_COMMON_PARAM_REQ",  2,  0,0,0,0},
        {parentElemType, structVarType,         NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, dynamicDataVarType,   NULL,   1, (int) &commonParamsFormat,0,0,0}
};

//---------------------  cs end service init request format ---------------------------
genXmlFormat srCsEndServiceInitReqFormat[] = {
    {rootElemType,  tagVarType, "CS_END_SERVICE_INIT_REQ",  2,  0,0,0,0},
        {parentElemType, structVarType,         NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, dynamicDataVarType,   NULL,   1, (int) &commonParamsFormat,0,0,0}
};

//---------------------  cs delete service request format ---------------------------
genXmlFormat srCsDelServiceReqFormat[] = {
    {rootElemType,  tagVarType, "CS_DEL_SERVICE_REQ",  2,  0,0,0,0},
        {parentElemType, structVarType,         NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, dynamicDataVarType,   NULL,   1, (int) &commonParamsFormat,0,0,0}
};

//---------------------  cs delete service request format ---------------------------
genXmlFormat srCsTerminalCommandReqFormat[] = {
    {rootElemType,  tagVarType, "CS_TERMINAL_COMMAND_REQ",  2,  0,0,0,0},
        {parentElemType, structVarType,         NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, dynamicDataVarType,   NULL,   1, (int) &terminalCommandFormat,0,0,0}
};

//---------------------  cs ping request format---------------------------------------
genXmlFormat csPingReqFormat[] = {
	{parentElemType, tagVarType, "csPingReqFormat", 2, 0,0,0,0},
		{childElemType,		longVarType,	"ipType",			0, 0,0,0,0},
        {siblingElemType,	stringVarType,	"destination", 0, MaxPingStrLen,0,0,0},
    
};

genXmlFormat srCsPingReqFormat[] = {
    {rootElemType,  tagVarType, "CS_PING_REQ",  2,  0,0,0,0},
        {parentElemType, structVarType,         NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, structVarType, NULL, 1, (int) &csPingReqFormat,0,0,0}
};


//---------------------  cs dns query request format ---------------------------
genXmlFormat dnsCsResolveReqFormat[] = {
	{parentElemType,	tagVarType,	"dnsCsResolveReq",	2,	0,0,0,0},
		{childElemType,		longVarType,	"transaction_id",	0,	0,0,0,0},
		{siblingElemType,	stringVarType,	"host_name",		0,	DnsQueryNameSize,0,0,0},
};

genXmlFormat srDnsCsResolveReqFormat[] = {
    {rootElemType,  tagVarType, "DNS_CS_RESOLVE_REQ",  2,  0,0,0,0},
        {parentElemType, structVarType,	NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, structVarType, NULL,   1, (int) &dnsCsResolveReqFormat,0,0,0}
};

//---------------------  cs dns service request format ---------------------------
genXmlFormat dnsCsServiceReqFormat[] = {
	{parentElemType,	tagVarType,	"dnsCsServiceReq",	3,	0,0,0,0},
		{childElemType,		longVarType,	"transaction_id",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"transport_type",	0,	0,0,0,0},
		{siblingElemType,	stringVarType,	"domain_name",		0,	DnsQueryNameSize,0,0,0},
};

genXmlFormat srDnsCsServiceReqFormat[] = {
    {rootElemType,  tagVarType, "DNS_CS_SERVICE_REQ",  2,  0,0,0,0},
        {parentElemType, structVarType,	NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, structVarType, NULL,   1, (int) &dnsCsServiceReqFormat,0,0,0}
};
