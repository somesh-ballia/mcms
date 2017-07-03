#ifndef GKGLOBALS_H_
#define GKGLOBALS_H_

#include "Segment.h"

#define MIN_RRQ_INTERVAL		     30 

//////////////////////////////////////////////////////////////////////
/*							Definitions								*/
//////////////////////////////////////////////////////////////////////
#define MIN_RRQ_INTERVAL		     30 

// Definitions of Polycom Israel (Accord) manufacturer Identification
#define Israel_t35CountryCode		88
#define Israel_t35Extension			0
#define Accord_manufacturerCode		172

typedef enum
{
	eInvalidCallState = 0,
	//ARQ:
	eArqReq,
	eArqHold, //for alt GK
	eArqInd,
	//DRQ:
	eDrqReq,
	eDrqHold,//for alt GK
	eDrqInd,
	eGkDrqInd,
	eGkDrqRes,
	eDrqAfterArq,
	eDrqAfterArqHold,//for alt GK
	eSendDrqAfterArq,
	eDrqReqFromGkManager,  // ErrorHandling -  When Gk Manager inits the DRQ, and not the party or the GK
	eDrqReqFromGkAfterViolentClose,  // ErrorHandling -  When Gk Manager inits the DRQ, and not the party or the GK
	//BRQ:
	eBrqHold, //for alt GK
}eCallStatus;


//////////////////////////////////////////////////////////////////////
/*							 Functions 								*/
//////////////////////////////////////////////////////////////////////
extern void SendReqToCSMngr(CSegment *pSeg, OPCODE opcode);
extern void SendReqToConfParty(OPCODE opcode, DWORD connId, DWORD partyId, CSegment *pSeg = NULL);


#endif /*GKGLOBALS_H_*/
