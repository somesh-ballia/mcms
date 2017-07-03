// AcServiceIndFormats
// Ami Noy

// Include files:
//---------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpCsDnsApi.h"
#include "IpCommonDefinitions.h"


// External variables
//-------------------
extern genXmlFormat maintHeaderFormat[];
extern genXmlFormat versionsFormat[];
extern genXmlFormat commonParamsFormat[];
extern genXmlFormat xmlDynamicHeaderFormat[];
extern genXmlFormat xmlDynamicPropertiesFormat[];

//---------------------  cs new service init indication format ---------------------------
genXmlFormat srCsNewServiceInitIndFormat[] = {
	{rootElemType,	tagVarType,	"CS_NEW_SERVICE_INIT_IND",	2,	0,0,0,0},
		{parentElemType, structVarType,			NULL,	1, (int) &maintHeaderFormat,0,0,0},
		{parentElemType, dynamicDataVarType,	NULL,	1, (int) &commonParamsFormat,0,0,0}
};

//---------------------  cs new common param indication format ---------------------------
genXmlFormat srCsCommonParamIndFormat[] = {
    {rootElemType,  tagVarType, "CS_COMMON_PARAM_IND",  2,  0,0,0,0},
        {parentElemType, structVarType,         NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, dynamicDataVarType,    NULL,    1, (int) &commonParamsFormat,0,0,0}
};

//---------------------  cs end service init indication format ---------------------------
genXmlFormat srCsEndServiceInitIndFormat[] = {
    {rootElemType,  tagVarType, "CS_END_SERVICE_INIT_IND",  2,  0,0,0,0},
        {parentElemType, structVarType,         NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, dynamicDataVarType,   NULL,    1, (int) &commonParamsFormat,0,0,0}
};

//---------------------  cs delete service indication format ---------------------------
genXmlFormat srCsDelServiceIndFormat[] = {
    {rootElemType,  tagVarType, "CS_DEL_SERVICE_IND",  2,  0,0,0,0},
        {parentElemType, structVarType,         NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, dynamicDataVarType,    NULL,   1, (int) &commonParamsFormat,0,0,0}
};

//---------------------  cs ping indication format---------- ---------------------------
genXmlFormat csPingIndFormat[] = {
	{parentElemType, tagVarType, "csPingIndFormat", 1, 0,0,0,0},
		{childElemType,		longVarType,	"pingStatus",			0, 0,0,0,0},
};

genXmlFormat srCsPingIndFormat[] = {
    {rootElemType,  tagVarType, "CS_PING_IND",  2,  0,0,0,0},
        {parentElemType, structVarType,         NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, structVarType, NULL, 1, (int) &csPingIndFormat,0,0,0}
};

//---------------------  cs dns resolve indication format ---------------------------
genXmlFormat ipAddressElemFormat[] = {
	{parentElemType,	tagVarType,	"ipAddressElem",	2,	0,0,0,0},
		{childElemType,		longVarType, 	"ip_type" , 		0, 0, 0, 0, 0},
		{siblingElemType,	stringVarType,	"ip_address",		0, MaxIpAddressStringSize,0,0,0},
};

genXmlFormat ipAddressListFormat[] = {
	{parentElemType,	tagVarType,		"ipAddressList",	2,	0,0,0,0},
		{parentElemType,	structVarType, NULL,		1,(int)	&xmlDynamicHeaderFormat,0, 0, 0},
		{parentElemType,	structVarType, NULL,		1,(int)	&ipAddressElemFormat,   0, 0, 0}
};

dynamicTableStruct dnsDynamicTbl[] = {
	{typeDnsResolve,	(int) &ipAddressListFormat}
};

genXmlFormat dnsCsResolveIndFormat[] = {
	{parentElemType,	tagVarType,		"dnsCsResolveInd",	7,	0,0,0,0},
		{childElemType,		longVarType, 	"status" , 			0, 0, 0, 0, 0},
		{siblingElemType,	longVarType,	"transaction_id",	0, 0, 0, 0, 0},
		{siblingElemType,	stringVarType,	"host_name",		0, DnsQueryNameSize, 0, 0, 0},
		{siblingElemType,	stringVarType,	"err_reason",		0, DnsErrDescSize, 0, 0, 0},
		{siblingElemType,	longVarType,	"nAddresses",		0, 0, 0, 0, 0},
		{parentElemType,	structVarType,	 NULL, 				1, (int)&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType,	"ipAddrList",		NumOfElements(dnsDynamicTbl), (int) &dnsDynamicTbl, 0, 0, 0},
};

genXmlFormat srDnsCsResolveIndFormat[] = {
    {rootElemType,  tagVarType, "DNS_CS_RESOLVE_IND",  2,  0,0,0,0},
        {parentElemType, structVarType,    NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, structVarType,    NULL,   1, (int) &dnsCsResolveIndFormat,0,0,0}
};

//---------------------  cs dns service indication format ---------------------------
genXmlFormat dnsCsServiceIndFormat[] = {
	{parentElemType,	tagVarType,		"csDnsServiceInd",	12,	0,0,0,0},
		{childElemType,		longVarType, 	"status" , 			0, 0, 0, 0, 0},
		{siblingElemType,	longVarType,	"transaction_id",	0, 0, 0, 0, 0},
		{siblingElemType,	stringVarType,	"domain_name",		0, DnsQueryNameSize, 0, 0, 0},
		{siblingElemType,	stringVarType,	"host_name",		0, DnsQueryNameSize, 0, 0, 0},
		{siblingElemType,	stringVarType,	"err_reason",		0, DnsErrDescSize,   0, 0, 0},
		{siblingElemType,	longVarType,	"ttl",				0, 0, 0, 0, 0},
		{siblingElemType,	shortVarType,	"port",				0, 0, 0, 0, 0},
		{siblingElemType,	shortVarType,	"priority",			0, 0, 0, 0, 0},
		{siblingElemType,	longVarType,	"weight",			0, 0, 0, 0, 0},
		{siblingElemType,	longVarType,	"nAddresses",		0, 0, 0, 0, 0},
		{parentElemType,	structVarType,	 NULL, 				1, (int)&xmlDynamicPropertiesFormat,				0, 0, 0},
		{parentElemType,	dynamicVarType,	"ipAddrList",		NumOfElements(dnsDynamicTbl), (int) &dnsDynamicTbl, 0, 0, 0},
//		{siblingElemType,	stringVarType,	"srv",				0, DnsSrvDetailsSize, 0, 0, 0},
};

genXmlFormat srDnsCsServiceIndFormat[] = {
    {rootElemType,  tagVarType, "DNS_CS_SERVICE_IND",  2,  0,0,0,0},
        {parentElemType, structVarType,    NULL,   1, (int) &maintHeaderFormat,0,0,0},
        {parentElemType, structVarType,    NULL,   1, (int) &dnsCsServiceIndFormat,0,0,0}
};

//EOF
