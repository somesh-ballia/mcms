// GkIfMsgsPhFormat.cpp
// Koren 

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"


// Macros
//--------

#undef	SRC
#define SRC	"GkIfMsgPhFormat"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------

//	External Routines:
//--------------------
extern genXmlFormat commonHeaderFormat[];

extern genXmlFormat gkReqRasGRQFormat[];
extern genXmlFormat gkReqRasRRQFormat[];
extern genXmlFormat gkReqRasURQFormat[];
extern genXmlFormat gkReqURQResponseFormat[];
extern genXmlFormat gkReqDRQResponseFormat[];
extern genXmlFormat gkReqRasARQFormat[];
extern genXmlFormat gkReqRasBRQFormat[];
extern genXmlFormat gkReqRasIRRFormat[];
extern genXmlFormat gkReqRasDRQFormat[];
extern genXmlFormat gkReqLRQResponseFormat[];
extern genXmlFormat gkReqBRQResponseFormat[];
extern genXmlFormat gkReqRasRAIFormat[];

extern genXmlFormat gkIndRasGRQFormat[];
extern genXmlFormat gkIndRasRRQFormat[];
extern genXmlFormat gkIndRasURQFormat[];
extern genXmlFormat gkIndRasARQFormat[];
extern genXmlFormat gkIndRasDRQFormat[];
extern genXmlFormat gkIndRasBRQFormat[];
extern genXmlFormat gkIndRasTimeoutFormat[];
extern genXmlFormat gkIndURQFromGkFormat[];
extern genXmlFormat gkIndDRQFromGkFormat[];
extern genXmlFormat gkIndBRQFromGkFormat[];
extern genXmlFormat gkIndLRQFromGkFormat[];
extern genXmlFormat gkIndGKIRQFormat[];
extern genXmlFormat gkIndRasFailFormat[];

//-----------------------------------------------------------------


// Forward Declarations:
//----------------------


// Routines:
//----------

//-------------------------------------------------------------
// H323_CS_RAS_GRQ_IND
genXmlFormat gkDsIndRasGRQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_GRQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndRasGRQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_RRQ_IND
genXmlFormat gkDsIndRasRRQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_RRQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndRasRRQFormat,0,0,0}
};


//-------------------------------------------------------------
// H323_CS_RAS_URQ_IND
genXmlFormat gkDsIndRasURQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_URQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndRasURQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_ARQ_IND
genXmlFormat gkDsIndRasARQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_ARQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndRasARQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_DRQ_IND
genXmlFormat gkDsIndRasDRQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_DRQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndRasDRQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_BRQ_IND
genXmlFormat gkDsIndRasBRQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_BRQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndRasBRQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_FAIL_IND
genXmlFormat gkDsIndRasFailFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_FAIL_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndRasFailFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_TIMEOUT_IND
genXmlFormat gkDsIndRasTimeoutFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_TIMEOUT_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndRasTimeoutFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_GKURQ_IND
genXmlFormat gkDsIndURQFromGkFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_GKURQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndURQFromGkFormat,0,0,0}
};


//-------------------------------------------------------------
// H323_CS_RAS_GKDRQ_IND
genXmlFormat gkDsIndDRQFromGkFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_GKDRQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndDRQFromGkFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_GKBRQ_IND
genXmlFormat gkDsIndBRQFromGkFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_GKBRQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndBRQFromGkFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_GKLRQ_IND
genXmlFormat gkDsIndLRQFromGkFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_GKLRQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndLRQFromGkFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_GKIRQ_IND
genXmlFormat gkDsIndGKIRQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_GKIRQ_IND",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkIndGKIRQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_RAC_IND
genXmlFormat gkDsIndRACFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_RAC_IND",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};


// Requests::
//-------------------------------------------------------------
// H323_CS_RAS_GRQ_REQ
genXmlFormat gkDsReqRasGRQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_GRQ_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqRasGRQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_RRQ_REQ
genXmlFormat gkDsReqRasRRQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_RRQ_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqRasRRQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_URQ_REQ
genXmlFormat gkDsReqRasURQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_URQ_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqRasURQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_URQ_RESPONSE_REQ
genXmlFormat gkDsReqURQResponseFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_URQ_RESPONSE_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqURQResponseFormat,0,0,0}
};
//-------------------------------------------------------------
// H323_CS_RAS_DRQ_RESPONSE_REQ
genXmlFormat gkDsReqDRQResponseFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_DRQ_RESPONSE_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqDRQResponseFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_ARQ_REQ
genXmlFormat gkDsReqRasARQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_ARQ_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqRasARQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_BRQ_REQ
genXmlFormat gkDsReqRasBRQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_BRQ_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqRasBRQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_IRR_REQ
// H323_CS_RAS_IRR_RESPONSE_REQ
genXmlFormat gkDsReqRasIRRFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_IRR_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqRasIRRFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_DRQ_REQ
genXmlFormat gkDsReqRasDRQFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_DRQ_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqRasDRQFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_LRQ_RESPONSE_REQ
genXmlFormat gkDsReqLRQResponseFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_LRQ_RESPONSE_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqLRQResponseFormat,0,0,0}
};

//-------------------------------------------------------------
// H323_CS_RAS_BRQ_RESPONSE_REQ
genXmlFormat gkDsReqBRQResponseFormat[] = {
	 {rootElemType,	tagVarType,	"H323_CS_RAS_LRQ_RESPONSE_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqBRQResponseFormat,0,0,0}
};

//-------------------------------------------------------------
//H323_CS_RAS_REMOVE_GK_CALL_REQ
genXmlFormat gkDsReqRemoveCallFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_RAS_REMOVE_GK_CALL_REQ",	1,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
};

//-------------------------------------------------------------
//H323_CS_RAS_RAI_REQ
genXmlFormat gkDsReqRAIFormat[] = {
	{rootElemType,	tagVarType,	"H323_CS_RAS_RAI_REQ",	2,	0,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &commonHeaderFormat,0,0,0},
		{parentElemType,	structVarType,  NULL,   1,	(int) &gkReqRasRAIFormat,0,0,0},
};


//EOF
