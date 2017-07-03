// CommonFormats.c
// Ami Noy

#include "XmlDefs.h"
#include "XmlFormat.h"

genXmlFormat mcmsHeaderFormat[] = {
	{parentElemType,	tagVarType,	"COMMON_HEADER",	11,	0,0,0,0},
		{childElemType,		charVarType,	"protocol_version",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"option",				0,	0,0,0,0},
		{siblingElemType,	charVarType,	"src_id",				0,	0,0,0,0},
		{siblingElemType,	charVarType,	"dest_id",				0,	0,0,0,0},
		{siblingElemType,	longVarType,	"opcode",				0,	0,0,0,0},
        {siblingElemType,   longVarType,    "time_stamp",           0,  0,0,0,0},
        {siblingElemType,	longVarType,	"sequence_num",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"payload_len",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"payload_offset",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"next_header_type",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"next_header_offset",	0,	0,0,0,0}
};

//------------- message description header format ----------------------------

genXmlFormat msgDescHeaderFormat[] = {
	{parentElemType,	tagVarType,	"MESSAGE_DESCRIPTION_HEADER",	8,	0,0,0,0},
		{childElemType,		longVarType,	"request_id",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"entity_type",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"time_stamp",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"msg_ack_ind",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"future_use1",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"future_use2",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"next_header_type",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"next_header_size",	0,	0,0,0,0}
};

//------------- port description header format ----------------------------

genXmlFormat portDescHeaderFormat[] = {
	{parentElemType,	tagVarType,	"PORT_DESCRIPTION_HEADER",	8,	0,0,0,0},
		{childElemType,		longVarType,	"party_id",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"conf_id",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"connection_id",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"logical_resource_type_1",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"logical_resource_type_2",	0,	0,0,0,0},
//		{siblingElemType,	charVarType,	"future_use1",				0,	0,0,0,0},
//		{siblingElemType,	charVarType,	"future_use2",				0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"room_id",				    0,	0,0,0,0},
		{siblingElemType,	longVarType,	"next_header_type",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"next_header_size",			0,	0,0,0,0}
};

//------------- cs header and format ----------------------------

genXmlFormat csHeaderFormat[] = {
	{parentElemType,	tagVarType,	"CENTRAL_SIGNALING_HEADER",	11,	0,0,0,0},
		{childElemType,		shortVarType,	"cs_id",				0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"src_unit_id",			0,	0,0,0,0},
        {siblingElemType,   shortVarType,   "dst_unit_id",          0,  0,0,0,0},
        {siblingElemType,   shortVarType,   "reserved",             0,  0,0,0,0},
		{siblingElemType,	longVarType,	"call_index",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"service_id",      		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"channel_index",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"mc_channel_index",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"status",				0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"next_header_type",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"next_header_offset",	0,	0,0,0,0}
};

genXmlFormat maintHeaderFormat[] = {
	{parentElemType,	tagVarType,	"maintHeader",	3,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcmsHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &msgDescHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &csHeaderFormat,0,0,0}
};

//--------------------------------------------
//Call control headers and format.

genXmlFormat commonHeaderFormat[] = {
	{parentElemType,	tagVarType,	"cntlHeader",	4,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &mcmsHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &msgDescHeaderFormat,0,0,0},
		{parentElemType,    structVarType,  NULL,   1,  (int) &csHeaderFormat,0,0,0},
        {parentElemType,	structVarType,  NULL,   1,	(int) &portDescHeaderFormat,0,0,0}		
};

genXmlFormat versionsFormat[] = {
	{parentElemType,	tagVarType,	"VERSIONS",	4,	0,0,0,0},
		{childElemType,		longVarType,	"ver_major",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"ver_minor",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"ver_release",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"ver_internal",	0,	0,0,0,0}
};

//-------------- Common param format -----------------------------
genXmlFormat commonParamsFormat[] = {
    {parentElemType,    tagVarType, "commonParam",  2,  0,0,0,0},
        {childElemType,     longVarType,            "length",       0,  0,0,0,0},
        {siblingElemType,   charArrayVarType,   	"paramBuffer",  0,  0,0,0,0},
};

//-------------- Terminal command format -----------------------------
genXmlFormat terminalCommandFormat[] = {
    {parentElemType,    tagVarType, "terminalCommand",  2,  0,0,0,0},
        {childElemType,     longVarType,            "length",       0,  0,0,0,0},
        {siblingElemType,   charArrayVarType,       "paramBuffer",  0,  0,0,0,0},
};

/*genXmlFormat sipInfoDynamicFormat[] = {
	{parentElemType,	tagVarType,	"sipInfoDynamicFormat",	2,	0,0,0,0},
		{childElemType,		longVarType,		"length",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,	"valueList",	0,	0,0,0,0},
};*/
