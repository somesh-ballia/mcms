//+========================================================================+
//                       H323UTIL.H                                        |
//             Copyright 2003 Polycom Israel Ltd.                          |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Israel Ltd. and is protected by law.             |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Israel Ltd.                    |
//-------------------------------------------------------------------------|
// FILE:       H323UTIL.H                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Yael                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 05/03/03   | Definitions for H323 mcms uses             |
//+========================================================================+

#ifndef _H323UTIL
#define _H323UTIL

	 
#include "PObject.h"
#include "IPUtils.h"

#include "RvCommonDefs.h"  //for cmCapDirection and cmCapDataType 


//#ifdef __H323_SIM__
//#define _MCMS_H323_SIM_    // - for MCMS simulation
//#undef _MCMS_H323_SIM_		// for old H323 simulation,
						// undef - just in case , that this can be defined anywhere
//#endif//__H323_SIM__


/* IP UTILITIES: */
#define SmallPrintLen      56
#define MediumPrintLen     104
#define LargePrintLen      256
#define VeryLargePrintLen  512


/* H323 UTILITIES: */
#define PARTYCNTL_CHANGEVIDEO_TIME 26*SECOND

#define SmallPrintLen      56
#define MediumPrintLen     104
#define LargePrintLen      256
#define VeryLargePrintLen  512

/* The ARQTIMEOUT definition can't be changed, since it is equal to the definitions in
H323/ras/cmras.h : cmRASTransaction*/
#define ARQTIMEOUT   4 //the other timeouts are defined in gkmngr.cpp

#define ALT_GK_PROCESS  10

typedef enum {
	eNotNeeded,
	eChangeIncoming, 
	eReopenOut,
	eChangeOutgoing,
	eChangeInAndReopenOut,
	eReopenIn, 
	eReopenInAndOut,
    eChangeInAndOut,
	eFlowControlIn,
	eFlowControlOut,
	eFlowControlInAndOut,
	eFlowControlOutAndChangeIn,
	eFlowControlOutAndReopenIn,
	eFlowControlInAndReopenOut, //new in RMX - upgrade from secondary, where is channel is flowd control to zero.
	eCanNotChange,	
	eChangeContentOut,
	eChangeContentRate,	
	eChangeContentIn,
	eChangeContentInAndOut,
	eFallBackFromIceToSip,
	eFallBackFromTipToSip,
	eConfRequestMoveToMixed,
	eLastChangeModeState // for enum checking
} eChangeModeState; 

typedef enum {
	ePnOff,
	ePnDirect,
	ePnRouted
} PathNavigatorStates;


typedef enum {
	Idle,
	ZeroIp,
	GetPort,
	GetPortDisconnecting,
	GkARQ,
	GkARJ,
	McmsNotAcceptAcfParams, //small bandwidth in ACF or CRV=0 in ACF
	GkConnecting,
	GkConnected,
	GkDRQ,
	GkDrqReject,
	DRQfromGK,
	GkDRQafterARQ,
	ReleasePort, 
	CallThroughGk,
	ControlPortsDeficiency,
	RouteCallToGK 
} ConnectionStatus; //Add because of different reaction to disconnecting with the gatekeeper




typedef enum {
	TokenIdle, //When party sends incoming channel response to H323Cntl 
	TokenAquireAck, //When party receives Aquire Ack from content control, until it sends incoming channel response to H323Cntl
	TokenAquireAckWaitForNewTSS, //When party receives new TSS after it has received Aquire Ack from content control 
		//and before sending incoming channel response to H323Cntl (happens in changing speaker)
	ChangeSpeaker, //For the previous speaker: Between closing the duo in and opening the duo out
	TokenAquireReject, //When party receives Aquire Nack from content control, until it closes the incoming channel
	TokenAquireRejectWaitForNewTSS, //When party receives new TSS after it has received Aquire Nack from content control 
		//and before closing the incoming channel
	TokenAquireWhileChangeMode
}contentState;

extern const char* g_changeModeStateStrings[];
inline const char* GetChangeModeStateStr(eChangeModeState state){return (state < eLastChangeModeState) ? g_changeModeStateStrings[state] : "Unknown State";}


/* FUNCTIONS: */

//Highest Common:
BYTE GetDirectionByChangeModeState(eChangeModeState changeModeState, cmCapDirection &direction);
BYTE  IsChangeModeOfIncomingState(eChangeModeState changeModeState);

std::string GetHexNum(const char* str);
std::string GetGuidFormat(const char* str);

#endif //_H323UTIL
