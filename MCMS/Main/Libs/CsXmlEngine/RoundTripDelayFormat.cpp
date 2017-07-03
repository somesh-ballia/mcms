// RoundTripDelayFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"


// Macros
//--------

#undef	SRC
#define SRC	"RoundTripDlyFormat"

//	Static variables
//-------------------

// Global variables
//------------------

// External variables
//------------------

//	External Routines:
//--------------------

// H323_CS_SIG_ROUND_TRIP_DELAY_REQ
//typedef struct  {
//	UINT32		delay;
//	
//} mcReqRoundTripDelay;


genXmlFormat mcReqRoundTripDelayFormat[] = {
	{parentElemType,	tagVarType,	"mcReqRoundTripDelay",	1,	0,0,0,0},
		{childElemType,		longVarType,	"delay",	0,	0,0,0,0},
};
