// AcStartupIndFormat.c
// Ami Noy

// Include files:
//---------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpCsSizeDefinitions.h"
//#include "MplMcmsStructs.h"
//#include "CsStructs.h"

// External variables
//-------------------
extern genXmlFormat maintHeaderFormat[];
extern genXmlFormat versionsFormat[];



//---------------------  cs new indication format ---------------------------

genXmlFormat csNewIndFormat[] = {
	{parentElemType, tagVarType, "csNewInd", 2, 0,0,0,0},
		{childElemType, longVarType,	"id",	0, 0,0,0,0},
		{parentElemType, structVarType,	NULL,	1, (int) &versionsFormat,0,0,0}
};

genXmlFormat suCsNewIndFormat[] = {
	{rootElemType,	tagVarType,	"CS_NEW_IND",	2,	0,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &csNewIndFormat,0,0,0},
};

//---------------------  cs config indication format ---------------------------

genXmlFormat csConfigParamIndFormat[] = {
	{parentElemType, tagVarType, "csConfigParamInd", 4, 0,0,0,0},
		{childElemType,		stringVarType,	"str_section_name",	0, STR_LEN,0,0,0},
		{siblingElemType,	stringVarType,	"str_key_name",		0, STR_LEN,0,0,0},
		{siblingElemType,	longVarType,	"result",			0, 0,0,0,0},
		{siblingElemType,	stringVarType,	"str_error_reason",	0, STR_LEN,0,0,0}
};

genXmlFormat suCsConfigParamIndFormat[] = {
	{rootElemType,	tagVarType,	"CS_CONFIG_PARAM_IND",	2,	0,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &csConfigParamIndFormat,0,0,0}
};

//---------------------  cs end config indication format ---------------------------

genXmlFormat csEndConfigParamIndFormat[] = {
	{parentElemType, tagVarType, "csEndConfigParamInd", 2, 0,0,0,0},
		{childElemType,		longVarType,	"result",			0, 0,0,0,0},
		{siblingElemType,	stringVarType,	"str_error_reason",	0, STR_LEN,0,0,0}
};

genXmlFormat suCsEndConfigParamIndFormat[] = {
	{rootElemType,	tagVarType,	"CS_END_CONFIG_PARAM_IND",	2,	0,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &csEndConfigParamIndFormat,0,0,0}
};

//---------------------  cs end startup indication format ---------------------------

genXmlFormat csEndStartupIndFormat[] = {
	{parentElemType, tagVarType, "csEndStartupInd", 2, 0,0,0,0},
		{childElemType,		longVarType,	"result",			0, 0,0,0,0},
		{siblingElemType,	stringVarType,	"str_error_reason",	0, STR_LEN,0,0,0}
};

genXmlFormat suCsEndStartupIndFormat[] = {
	{rootElemType,	tagVarType,	"CS_END_CS_STARTUP_IND",	2,	0,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0},
		{parentElemType, structVarType, NULL, 1, (int) &csEndStartupIndFormat,0,0,0}
};

//---------------------  cs reconnect indication format ---------------------------
genXmlFormat suCsReconnectIndFormat[] = {
    {rootElemType,  tagVarType, "CS_RECONNECT_IND",   1,  0,0,0,0},
        {parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0},
};

//---------------------  cs keep alive indication format ---------------------------
genXmlFormat compKeepAliveStIndFormat[] = {
	{parentElemType, tagVarType, "compKeepAliveSt", 5, 0,0,0,0},
		{childElemType,     longVarType,	"bActive",	0, 0,0,0,0},
        {siblingElemType,   longVarType,	"id",   	0, 0,0,0,0},
        {siblingElemType,   longVarType,    "type",		0, 0,0,0,0},
        {siblingElemType,   longVarType,    "status",	0, 0,0,0,0},
        {siblingElemType,   longVarType,    "reason",	0, 0,0,0,0}	
};

genXmlFormat csKeepAliveStFormat[] = {
	{parentElemType, tagVarType, "csKeepAliveInd", 1, 0,0,0,0},
		{parentElemType,     structVarType,	"componentTbl",	NumOfComponents, (int) &compKeepAliveStIndFormat,0,0,0},
};

genXmlFormat suCsKeepAliveIndFormat[] = {
    {rootElemType,  tagVarType, "CS_KEEP_ALIVE_IND",   2,  0,0,0,0},
        {parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, structVarType, NULL, 1, (int) &csKeepAliveStFormat,0,0,0}
};

//---------------------  cs component status indication struct ---------------------------
genXmlFormat csCompStatusStFormat[] = {
	{parentElemType, tagVarType, "csCompStatusInd", 6, 0,0,0,0},
        {siblingElemType,   longVarType,    "type",		0, 0,0,0,0},
        {siblingElemType,   longVarType,    "status",	0, 0,0,0,0},
        {siblingElemType,   longVarType,    "reason",	0, 0,0,0,0},
        {siblingElemType,   longVarType,    "unitId",	0, 0,0,0,0},        
        {siblingElemType,	stringVarType,	"unitName",	0, MAX_UNIT_NAME_SIZE,0,0,0},
        {siblingElemType,	stringVarType,	"errorStr",	0, MaxRecoveryErrorStrSize,0,0,0}        
};

genXmlFormat suCsCompStatusIndFormat[] = {
    {rootElemType,  tagVarType, "CS_COMP_STATUS_IND",   2,  0,0,0,0},
        {parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, structVarType, NULL, 1, (int) &csCompStatusStFormat,0,0,0}
};

////---------------------  cs active alarm indications struct -------------------------------
//
genXmlFormat suUserMessageStFormat[] = {
	{parentElemType, tagVarType, "alarmMessage", 7, 0,0,0,0},
        {siblingElemType,   longVarType,    "messageCode",	0, 0,0,0,0},
        {siblingElemType,   longVarType,    "location",		0, 0,0,0,0},
        {siblingElemType,   longVarType,    "operation",	0, 0,0,0,0},
        {siblingElemType,   longVarType,    "autoRemoval",	0, 0,0,0,0},
        {siblingElemType,   longVarType,    "process_type",	0, 0,0,0,0}, 
        {siblingElemType,   longVarType,    "future_use1",	0, 0,0,0,0},
        {siblingElemType,   longVarType,    "future_use2",	0, 0,0,0,0}, 
            
};
//
genXmlFormat suCSActiveAlarmIndFormat[] = {
		{rootElemType,  tagVarType, "CS_USER_MSG_IND",   2,  0,0,0,0},
			{parentElemType, structVarType, NULL, 1, (int) &maintHeaderFormat,0,0,0},
		    {parentElemType, structVarType, NULL, 1, (int) &suUserMessageStFormat,0,0,0}
};

