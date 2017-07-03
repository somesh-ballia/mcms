/*
 * SipProxyGlobals.h
 *
 *  Created on: Mar 5, 2012
 *      Author: mvolovik
 */

#ifndef SIPPROXYGLOBALS_H_
#define SIPPROXYGLOBALS_H_
#include "Segment.h"


//////////////////////////////////////////////////////////////////////
/*               Functions                */
//////////////////////////////////////////////////////////////////////
extern void SendReqToConfParty(OPCODE opcode, DWORD connId, DWORD partyId, CSegment *pSeg);


#endif /* SIPPROXYGLOBALS_H_ */
