// AcStartupReqFormats.c
// Ami Noy

// Include files:
//---------------
#include "CsStructs.h"
#include "XmlFormat.h"


// External variables
//-------------------
extern genXmlFormat maintHeaderFormat[];
extern genXmlFormat versionsFormat[];

//---------------------  cs new request format ---------------------------

genXmlFormat csNewReqFormat[] = {
	{parentElemType, tagVarType, "csNewReq", 2, 0,0,0,0},
		{childElemType,		shortVarType,	"num_h323_ports",	0, 0,0,0,0},
		{siblingElemType,	shortVarType,	"num_sip_ports",	0, 0,0,0,0}
};

genXmlFormat suCsNewReqFormat[] = {
	{rootElemType,	tagVarType,	"CS_NEW_REQ",	2,	0,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &csNewReqFormat,0,0,0}
};

//---------------------  cs config request format ---------------------------

genXmlFormat csConfigParamReqFormat[] = {
	{parentElemType, tagVarType, "csConfigParamReq", 3, 0,0,0,0},
		{childElemType,		stringVarType,	"section",			0, STR_LEN,0,0,0},
		{siblingElemType,	stringVarType,	"key",				0, H323_CONFIG_KEY_SIZE,0,0,0},
		{siblingElemType,	stringVarType,	"data",				0, H323_CONFIG_DATA_SIZE,0,0,0}
};

genXmlFormat suCsConfigParamReqFormat[] = {
	{rootElemType,	tagVarType,	"CS_CONFIG_PARAM_REQ",	2,	0,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &csConfigParamReqFormat,0,0,0}
};

//---------------------  cs end config request format ---------------------------

genXmlFormat suCsEndConfigParamReqFormat[] = {
	{rootElemType,	tagVarType,	"CS_END_CONFIG_PARAM_REQ",	1,	0,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0}
};

//---------------------  cs lan config request format ---------------------------

genXmlFormat suCsLanConfigReqFormat[] = {
	{rootElemType,	tagVarType,	"CS_LAN_CFG_REQ",	1,	0,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0}
};

//---------------------  cs reconnect request format ---------------------------
genXmlFormat suCsReconnectReqFormat[] = {
    
    {rootElemType,  tagVarType, "CS_RECONNECT_REQ",   1,  0,0,0,0},
        {parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0}    
};

//---------------------  cs keep alive request format ---------------------------
genXmlFormat suCsKeepAliveReqFormat[] = {  
    {rootElemType,  tagVarType, "CS_KEEP_ALIVE_REQ",   1,  0,0,0,0},
        {parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0}    
};
