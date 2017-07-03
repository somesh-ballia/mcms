#ifndef __SIP_COMMON__
#define __SIP_COMMON__

#include "IpCommon.h"
#include "OpcodesMcmsCommon.h"


//contain common declarations for MCMS SIP logic
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif


#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

//SIP Carmel party control states
const long sPARTY_IDLE									= 0;
const long sPARTY_RSRC_ALLOCATING						= 3;
const long sPARTY_MPL_CREATING							= 4;


//SIP OLD party control states
const WORD sPARTY_SETUP				= 1;    // party was asked to establish call
const WORD sPARTY_ICONNECT			= 2;    // party connected via the initial channel
const WORD sPARTY_CHANGEVIDEO		= 4;
const WORD sPARTY_CHANGECONTENT		= 5;
const WORD sPARTY_CHANGEOTHER		= 6;
const WORD sPARTY_CHANGEALL			= 8;		//change all the media in parallel and not in sequence
const WORD sPARTY_UPDATEVIDEO		= 9;		//change all the media in parallel and not in sequence


//from IPParty
//const WORD IP_FRACTION_LOST_TIMER = 101;
//const WORD INITIAL = 0;  // channel type


#endif





