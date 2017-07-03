// GkCsReqFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "GateKeeperCommonParams.h"
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpSizeDefinitions.h"

// Macros
//--------

#undef	SRC
#define SRC	"GkCsReqFormat"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------

//	External Routines:
//--------------------
extern genXmlFormat mcXmlTransportAddressFormat[];
extern genXmlFormat h460FsStFormat[];
extern genXmlFormat mcuVendorFormat[];
extern genXmlFormat ctNonStandardParameterStFormat[];
extern genXmlFormat h460AvayaFeVndrReqStFormat[];
extern genXmlFormat h460ConferenceProfileExtFormat[];
extern genXmlFormat xmlDynamicPropertiesFormat[];

// Forward Declarations:
//----------------------

//-------------------------------------------------------------
genXmlFormat aliasFormat[] = {
	{parentElemType,	tagVarType,	"aliasFormat",	2,	0,0,0,0},
		{childElemType,		longVarType,		"length",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,	"aliasesList",		0,	0,0,0,0},
};  


//-----------------------------------//
//typedef  struct  _GkH235AuthParam 
//{   
//    APIS32         eEncryptMethodConfiguredScale    ; // Default: eH235Method_Non   =  0
//    APIS32         eEncryptMethodRequired           ; // Default: eH235Method_Undef = -1
//    APIS32     	 nIsAuth                          ; // Default: FALSE/0
//    char   		 AuthUserName[H235MaxAuthUserName]; // Default: ‘\0’
//    char   		 AuthPassword[H235MaxAuthPwd]     ; // Default: ‘\0’ 
//}GkH235AuthParam;

genXmlFormat GkH235AuthParamFormat[] = {
    {parentElemType , tagVarType   , "GkH235AuthParam" , 5,0 ,0,0,0},
    {childElemType  , longVarType  , "eEncryptMethodConfiguredScale",0,0,0,0,0},
    {siblingElemType, longVarType  , "eEncryptMethodRequired",0,0,0,0,0},
    {siblingElemType, longVarType  , "nIsAuth" ,0,0,0,0,0},
    {siblingElemType, stringVarType, "AuthUserName" ,0,H235MaxAuthUserName,0,0,0},
    {siblingElemType, stringVarType, "AuthPassword" ,0,H235MaxAuthPwd ,0,0,0},
}; 

//typedef struct {
//    mcXmlTransportAddress		rasAddress;
//    mcXmlTransportAddress		gatekeeperAddress;
//    GkH235AuthParam	        H235AuthParams;                     //H.235 GK Authentication
//    APIU32					endpointType;
//    APIU32					bIsMulticast;
//    APIU32					bSupportAltGk; 
//    h460FsSt					fs;
//    APIU32					aliasesTypes[MaxNumberOfAliases];	// emun cmAliasType.
//    APIU32					aliasesLength;
//    char						aliasesList[1];
//} gkReqRasGRQ;

genXmlFormat gkReqRasGRQFormat[] = {
    {parentElemType,	tagVarType,	"gkReqRasGRQ",	11,	0,0,0,0},
    {parentElemType,	structVarType,  "rasAddress",   	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
    {parentElemType,	structVarType,  "gatekeeperAddress",1,	(int) &mcXmlTransportAddressFormat,0,0,0},
    //--------- GK Identifier --------------------------------------------------//
    {siblingElemType,	longVarType,  	"gkIdentLength",	0,	0,0,0,0},
    {siblingElemType,	charArrayVarType,"gatekeeperIdent",	0,	MaxIdentifierSize,0,0,0},
    //--------------------------------------------------------------------------//
    //--------- H235 GK Authentication params ---------------------------------------------------//
    {parentElemType,    structVarType,  "H235AuthParams",   1,  (int) &GkH235AuthParamFormat,0,0,0},
    //-------------------------------------------------------------------------------------------//
    {childElemType,		longVarType,  	"endpointType", 0,	0,0,0,0},
    {siblingElemType,	longVarType,  	"bIsMulticast",	0,	0,0,0,0},
    {siblingElemType,	longVarType,	"bSupportAltGk",0,	0,0,0,0},
    {parentElemType,	structVarType,  "fs",			1,	(int) &h460FsStFormat,0,0,0},
    {siblingElemType,	longListVarType,"aliasesTypes",	MaxNumberOfAliases,	0,0,0,0},
    {parentElemType,	dynamicDataVarType,	"alias",	1, (int) &aliasFormat,0,0,0}, 
};

//-------------------------------------------------------------
// H323_CS_RAS_RRQ_REQ
//typedef struct {
//	mcXmlTransportAddress		gatekeeperAddress;								// used when there is no gk_id.
//	mcXmlTransportAddress		callSignalingAddress[MaxNumberOfEndpointIp];	// endpoint address.
//	mcXmlTransportAddress		rasAddress;										// endpoint address for GK reply.
//	mcuVendor					endpointVendor;
//    GkH235AuthParam	        H235AuthParams;                                 //H.235 GK Authentication
//	char						gwPrefix[PHONE_NUMBER_DIGITS_LEN + 1];			//32 //for CISCO GK
//    APIU32						gkIdentLength;
//	char						gatekeeperIdent[MaxIdentifierSize];
//    APIU32						epIdentLength;
//	char						endpointIdent[MaxIdentifierSize];
//    ctNonStandardParameterSt 	mcuDetails;
//	h460FsSt					fs;
//	APIU32						dicoveryComplete;								// TRUE ==> discovery + registration
//	APIU32						bIsMulticast;
//	APIU32						bIsKeepAlive;									// equal 0 if it is the first RRQ.
//	APIU32						bSupportAltGk;
//	APIU32						timeToLive;
//	APIU32						aliasesTypes[MaxNumberOfAliases];				// emun cmAliasType.
//	char						prefix[PHONE_NUMBER_DIGITS_LEN + 1];
//	APIU32						aliasesLength;
//	char						aliasesList[1];
//} gkReqRasRRQ;

genXmlFormat gkReqRasRRQFormat[] = {
	{parentElemType,	tagVarType,	"gkReqRasRRQ",	20,	0,0,0,0},
		{parentElemType,	structVarType,  "gatekeeperAddress",   	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "callSignalingAddress",MaxNumberOfEndpointIp,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "rasAddress",		1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "endpointVendor",	1,	(int) &mcuVendorFormat,0,0,0},
        //--------- H235 GK Authentication params ---------------------------------------------------//
        {parentElemType,    structVarType,  "H235AuthParams",   1,  (int) &GkH235AuthParamFormat,0,0,0},
        //-------------------------------------------------------------------------------------------//
		{childElemType,		stringVarType,  "gwPrefix", 		0,	PHONE_NUMBER_DIGITS_LEN + 1,0,0,0},
		{siblingElemType,	longVarType,  	"gkIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"gatekeeperIdent",	0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	longVarType,  	"epIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"endpointIdent",	0,	MaxIdentifierSize,0,0,0},
		{parentElemType,	structVarType,  "mcuDetails",		1,	(int) &ctNonStandardParameterStFormat,0,0,0},
		{parentElemType,	structVarType,  "fs",				1,	(int) &h460FsStFormat,0,0,0},
		{siblingElemType,	longVarType,  	"dicoveryComplete",	0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bIsMulticast",	0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bIsKeepAlive",	0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bSupportAltGk",	0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"timeToLive",	0,	0,0,0,0},
		{siblingElemType,	longListVarType,"aliasesTypes",	MaxNumberOfAliases,	0,0,0,0},
		{siblingElemType,	stringVarType,  "prefix", 			0,	PHONE_NUMBER_DIGITS_LEN + 1,0,0,0},
		{parentElemType,	dynamicDataVarType,	"alias",	1, (int) &aliasFormat,0,0,0},
};
//-------------------------------------------------------------

// H323_CS_RAS_URQ_REQ
//typedef struct {
//	mcXmlTransportAddress	gatekeeperAddress;					// used when there is no gk_id.
//	mcXmlTransportAddress	callSignalingAddress[MaxNumberOfEndpointIp];	// endpoint address.
//  GkH235AuthParam	        H235AuthParams;                             //H.235 GK Authentication
//	APIU32					aliasesTypes[MaxNumberOfAliases];	// emun cmAliasType.
//	APIU32					gkIdentLength;
//	char					gatekeeperIdent[MaxIdentifierSize];
//    APIU32					epIdentLength;
//	char					endpointIdent[MaxIdentifierSize];
//	APIU32					aliasesLength;
//	char					sAliasesList[1];
//} gkReqRasURQ;

genXmlFormat gkReqRasURQFormat[] = {
	{parentElemType,	tagVarType,	"gkReqRasURQ",	9,	0,0,0,0},
		{parentElemType,	structVarType,  "gatekeeperAddress",   	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "callSignalingAddress",MaxNumberOfEndpointIp,	(int) &mcXmlTransportAddressFormat,0,0,0},
        //--------- H235 GK Authentication params ---------------------------------------------------//
        {parentElemType,    structVarType,  "H235AuthParams",   1,  (int) &GkH235AuthParamFormat,0,0,0},
        //-------------------------------------------------------------------------------------------//
		{childElemType,		longListVarType,"aliasesTypes", 	MaxNumberOfAliases,	0,0,0,0},
		{siblingElemType,	longVarType,  	"gkIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"gatekeeperIdent",	0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	longVarType,  	"epIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"endpointIdent",	0,	MaxIdentifierSize,0,0,0},
		{parentElemType,	dynamicDataVarType,	"alias",		1, (int) &aliasFormat,0,0,0},
};
//-------------------------------------------------------------
// H323_CS_RAS_URQ_RESPONSE_REQ
//typedef struct {
//	int						hsRas;
//} gkReqURQResponse;

genXmlFormat gkReqURQResponseFormat[] = {
	{parentElemType,	tagVarType,	"gkReqURQResponse",	1,	0,0,0,0},
		{childElemType,		longVarType, "hsRas", 0,	0,0,0,0},
};
//-------------------------------------------------------------

// H323_CS_RAS_DRQ_RESPONSE_REQ

typedef struct {
	int						hsRas;
} gkReqDRQResponse;

genXmlFormat gkReqDRQResponseFormat[] = {
	{parentElemType,	tagVarType,	"gkReqDRQResponse",	1,	0,0,0,0},
		{childElemType,		longVarType, "hsRas", 0,	0,0,0,0},
};
//-------------------------------------------------------------
genXmlFormat infoFormat[] = {
	{parentElemType,	tagVarType,	"infoFormat",	2,	0,0,0,0},
		{childElemType,		longVarType,		"length",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,	"info",		0,	0,0,0,0},
};

//typedef struct {
//	mcXmlTransportAddress	gatekeeperAddress;					// used when there is no gk_id.
//	mcXmlTransportAddress	destCallSignalAddress;				// used at dest for call signaling.
//	mcXmlTransportAddress	srcCallSignalAddress;				// used at src for call signaling.
//  GkH235AuthParam	        H235AuthParams;                             //H.235 GK Authentication
//	char 					cid[MaxConferenceIdSize];			//Only for Arq requests
//	char 					callId[Size16];						//Only for Arq requests
//	APIU32					gkIdentLength;
//	char					gatekeeperIdent[MaxIdentifierSize];
//  APIU32					epIdentLength;
//	char					endpointIdent[MaxIdentifierSize];
//  cmRASCallType			callType;							// p2p,one2N,N2N act.
//	cmRASCallModelType		callModel;							// direct or routed.
//	int						bandwidth;
//	APIU32					bIsDialIn;		//bIsAnswerCall
//	APIU32					bCanMapAlias;
//	h460AvayaFeVndrReqSt    avfFeVndIdReq;
//	APIU32					destInfoTypes[MaxNumberOfAliases];	// emun cmAliasType.
//	APIU32					srcInfoTypes[MaxNumberOfAliases];	// emun cmAliasType.
//	APIU32					srcInfoLength;
//	APIU32					destInfoLength;
//	APIU32					totalDynLen;
//	char					srcAndDestInfo[1];
//} gkReqRasARQ;

genXmlFormat gkReqRasARQFormat[] = {
	{parentElemType,	tagVarType,	"gkReqRasARQ",	21,	0,0,0,0},
		{parentElemType,	structVarType,  "gatekeeperAddress",   	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "destCallSignalAddress",1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "srcCallSignalAddress",	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
        //--------- H235 GK Authentication params ---------------------------------------------------//
        {parentElemType,    structVarType,  "H235AuthParams",   1,  (int) &GkH235AuthParamFormat,0,0,0},
        //-------------------------------------------------------------------------------------------//
		{childElemType,		charArrayVarType,"cid",					0,	MaxConferenceIdSize,0,0,0},
		{siblingElemType,	charArrayVarType,"callId",				0,	Size16,0,0,0},
		{siblingElemType,	longVarType,  	"gkIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"gatekeeperIdent",	0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	longVarType,  	"epIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"endpointIdent",	0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	longVarType,  	"callType",			0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"callModel",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bandwidth",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bIsDialIn",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bCanMapAlias",		0,	0,0,0,0},
		{parentElemType,	structVarType,  "avfFeVndIdReq",	1,	(int) &h460AvayaFeVndrReqStFormat,0,0,0},
		{siblingElemType,	longListVarType,"destInfoTypes",	MaxNumberOfAliases,	0,0,0,0},
		{siblingElemType,	longListVarType,"srcInfoTypes",		MaxNumberOfAliases,	0,0,0,0},
		{siblingElemType,	longVarType,  	"srcInfoLength",	0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"destInfoLength",	0,	0,0,0,0},
		{parentElemType,	dynamicDataVarType,	"srcAndDestInfo",	1, (int) &infoFormat,0,0,0},
};
//-------------------------------------------------------------
// H323_CS_RAS_BRQ_REQ
//typedef struct {
//	mcXmlTransportAddress	gatekeeperAddress;
//  GkH235AuthParam	        H235AuthParams;                     // H.235 GK Authentication 
//  APIU32					gkIdentLength;
//	char					gatekeeperIdent[MaxIdentifierSize];
//  APIU32					epIdentLength;
//	char					endpointIdent[MaxIdentifierSize];
// 	APIU32					callType;							// p2p,one2N,N2N act.
//	int						bandwidth;
//	APIU32					bIsDialIn;	//bIsAnswerCall
//} gkReqRasBRQ;

genXmlFormat gkReqRasBRQFormat[] = {
	{parentElemType,	tagVarType,	"gkReqRasBRQ",	9,	0,0,0,0},
		{parentElemType,	structVarType,  "gatekeeperAddress",1,	(int) &mcXmlTransportAddressFormat,0,0,0},
        //--------- H235 GK Authentication params ---------------------------------------------------//
        {parentElemType,    structVarType,  "H235AuthParams",   1,  (int) &GkH235AuthParamFormat,0,0,0},
        //-------------------------------------------------------------------------------------------//
		{childElemType,		longVarType,  	"gkIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"gatekeeperIdent",	0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	longVarType,  	"epIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"endpointIdent",	0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	longVarType,  	"callType",			0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bandwidth",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bIsDialIn",		0,	0,0,0,0},
};

//-S- HOMOLOGATION 3.0 RAS_TE_STA_02 -----------------------//
//-------------------------------------------------------------
// H323_CS_RAS_IRR_REQ
// H323_CS_RAS_IRR_RESPONSE_REQ

//typedef struct {
//    mcXmlTransportAddress	gatekeeperAddress;					// used when there is no gk_id.
//    mcXmlTransportAddress	rasAddress;			                //Local ras address need to take it from Mcms now - CardRasAddress
//    mcXmlTransportAddress	srcCallSignalAddress;	
//    mcXmlTransportAddress	destCallSignalAddress;	
//    GkH235AuthParam	    H235AuthParams;                     //H.235 GK Authentication
//    int 					bandwidth;
//    int						hsRas;
//    APIU32					epIdentLength;
//    char					endpointIdent[MaxIdentifierSize];
//    char 					callId[Size16];	
//    char 					conferenceId[MaxConferenceIdSize];  // HOMOLOGATION 3.0
//    cmRASCallType			callType;							// Use In GKIF As Constant cmCallTypeP2P p2p,one2N,N2N act.
//    cmRASCallModelType		callModel;							// direct or routed.
//    APIU32					bIsAnswer;							// is call originator.
//    APIU32					bIsNeedResponse;
//    APIU32					srcInfoTypes[MaxNumberOfAliases];	// emun cmAliasType.
//    APIU32					srcInfoLength;
//    char					srcInfo[1];
//} gkReqRasIRR;//common for IRR and IRRResponse

genXmlFormat gkReqRasIRRFormat[] = {
	{parentElemType,	tagVarType,	"gkReqRasIRR",	19,	0,0,0,0},
		{parentElemType,	structVarType,  "gatekeeperAddress",   	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "rasAddress",			1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "srcCallSignalAddress",	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "destCallSignalAddress",1,	(int) &mcXmlTransportAddressFormat,0,0,0},
        //--------- H235 GK Authentication params ---------------------------------------------------//
        {parentElemType,    structVarType,  "H235AuthParams",   1,  (int) &GkH235AuthParamFormat,0,0,0},
        //-------------------------------------------------------------------------------------------//
		{childElemType,		longVarType,  	"bandwidth",			0,	0,0,0,0},
        //---HOMOLOGATION----------------------------------------------------------------------------//
        {siblingElemType,	longVarType,  	"n931Crv",			0,	0,0,0,0},     
        //-------------------------------------------------------------------------------------------//  
		{siblingElemType,	longVarType,  	"hsRas",			0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"epIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"endpointIdent",	0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	charArrayVarType,"callId",			0,	Size16,0,0,0},
		//-S- HOMOLOGATION 3.0 RAS_TE_STA_02 -----------------------//
		{siblingElemType,	charArrayVarType,"conferenceId" ,	0,	MaxConferenceIdSize,0,0,0}, 
		//-E- HOMOLOGATION 3.0 RAS_TE_STA_02 -----------------------//
		{siblingElemType,	longVarType,  	"callType",			0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"callModel",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bIsDialIn",		0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bIsNeedResponse",	0,	0,0,0,0},
		{siblingElemType,	longListVarType,"srcInfoTypes",		MaxNumberOfAliases,	0,0,0,0},
        {siblingElemType,	longVarType    ,"srcInfoLength",	0,	0,0,0,0},
		{parentElemType,	dynamicDataVarType,	"srcInfo",		1, (int) &infoFormat,0,0,0},
};
//-E- HOMOLOGATION 3.0 RAS_TE_STA_02 -----------------------//
//-------------------------------------------------------------
// H323_CS_RAS_DRQ_REQ
//typedef struct {
//	mcXmlTransportAddress	gatekeeperAddress;					// used when there is no gk_id.
//  GkH235AuthParam	        H235AuthParams;                     // H.235 GK Authentication 
//	APIU32					gkIdentLength;
//	char					gatekeeperIdent[MaxIdentifierSize];
//  APIU32					epIdentLength;
//	char					endpointIdent[MaxIdentifierSize];
//	cmRASReason				disengageReason;
//	APIU32					bIsDialIn;	//bIsAnswerCall
//} gkReqRasDRQ;

genXmlFormat gkReqRasDRQFormat[] = {
	{parentElemType,	tagVarType,	"gkReqRasDRQ",	8,	0,0,0,0},
		{parentElemType,	structVarType,   "gatekeeperAddress",1,	(int) &mcXmlTransportAddressFormat,0,0,0},
        //--------- H235 GK Authentication params ---------------------------------------------------//
        {parentElemType,    structVarType,  "H235AuthParams",   1,  (int) &GkH235AuthParamFormat,0,0,0},
        //-------------------------------------------------------------------------------------------//
		{childElemType,		longVarType,  	 "gkIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"gatekeeperIdent",	0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	longVarType,  	 "epIdentLength",	0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"endpointIdent",	0,	MaxIdentifierSize,0,0,0},
		{siblingElemType,	longVarType,  	 "disengageReason",			0,	0,0,0,0},
		{siblingElemType,	longVarType,  	 "bIsDialIn",		0,	0,0,0,0},

};
//-------------------------------------------------------------

// H323_CS_RAS_LRQ_RESPONSE_REQ
//typedef struct {
//	int						hsRas;
//	mcXmlTransportAddress	callSignalingAddress[MaxNumberOfEndpointIp];   // of the card (destination)
//	mcXmlTransportAddress	rasAddress;			                           //Local ras address need to take it from Mcms now
//} gkReqLRQResponse;

genXmlFormat gkReqLRQResponseFormat[] = {
	{parentElemType,	tagVarType,	"gkReqLRQResponse",	3,	0,0,0,0},
		{childElemType,		longVarType,  	"hsRas",	0,	0,0,0,0},
		{parentElemType,	structVarType,  "callSignalingAddress",MaxNumberOfEndpointIp,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{parentElemType,	structVarType,  "rasAddress",   		1,	(int) &mcXmlTransportAddressFormat,0,0,0},

};
//-------------------------------------------------------------

// H323_CS_RAS_BRQ_RESPONSE_REQ
//typedef struct {
//	int						hsRas;
//	int 					bandwidth;
//} gkReqBRQResponse;

genXmlFormat gkReqBRQResponseFormat[] = {
	{parentElemType,	tagVarType,	"gkReqBRQResponse",	2,	0,0,0,0},
		{childElemType,		longVarType,  	"hsRas",	0,	0,0,0,0},
		{siblingElemType,	longVarType,  	"bandwidth",		0,	0,0,0,0},

};
//-------------------------------------------------------------

dynamicTableStruct h460ConferenceProfTbl[] = {
	{tblH460Conf,	(int)&h460ConferenceProfileExtFormat},
};

//-------------------------------------------------------------

// H323_CS_RAS_RAI_REQ
//typedef struct {
//
//		mcXmlTransportAddress		gatekeeperAddress;
//  	APIU32    					bAlmostOutOfResources; 	// boolean
//
//  	APIU32    					maximumAudioCapacity;   // Max Rai CallCapacity
//  	APIU32    					maximumVideoCapacity;
//
//  	APIU32    					currentAudioCapacity;	// Current Rai CallCapacity
//  	APIU32    					currentVideoCapacity;
//
//
// 	APIU16	  					numOfSupportedProfiles;	// integer
//	xmlDynamicProperties		xmlDynamicProps;
// 	char						profilesArray [1];
//} gkReqRasRAI;


genXmlFormat gkReqRasRAIFormat[] = {
	{parentElemType,	tagVarType,	"gkReqRasRai",	9,	0,0,0,0},
		{parentElemType,	structVarType, "gatekeeperAddress",   	1,	(int) &mcXmlTransportAddressFormat,0,0,0},
		{childElemType,		longVarType,   "bAlmostOutOfResources",	0, 0,0,0,0},
		{siblingElemType, 	longVarType,   "maximumAudioCapacity",	0, 0,0,0,0},
	  	{siblingElemType, 	longVarType,   "maximumVideoCapacity",	0, 0,0,0,0},
  		{siblingElemType, 	longVarType,   "currentAudioCapacity",	0, 0,0,0,0},
  		{siblingElemType, 	longVarType,   "currentVideoCapacity",	0, 0,0,0,0},
		{siblingElemType,	shortVarType,  "numOfSupportedProfiles",0, 0,0,0,0},
		{parentElemType,	structVarType,  NULL, 					1, (int)&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType, "profilesArray",		NumOfElements(h460ConferenceProfTbl),	(int) &h460ConferenceProfTbl,0,0,0},
};


