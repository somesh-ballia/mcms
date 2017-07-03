// DisconnectCause.h
// Maimon Yossi

#ifndef __DISCONNECTCAUSE_H__
#define __DISCONNECTCAUSE_H__

#include "DataTypes.h"
// Call/Channel Disconnect Modes:
// ------------------------------

#define channelCloseProcessRejectedByRemote	-1
#define channelOpenRequestRejectedByCT	-2

typedef enum
{
	callCloseReasonRemoteBusy = 1,		
	callCloseReasonNormal,				
	callCloseReasonRemoteReject,		
	callCloseReasonRemoteUnreachable,	
	callCloseReasonUnknownReason,		
	callCloseReasonClosedByMcms,		

	callStateModesNeedMaintenance	// Might cause by radVision stack changes.

} callDisconnectedStateModes;
// Based on cmCallStateMode_e Radvision stack.

typedef enum
{
    channelCloseReasonModeOn = 1,		
    channelCloseReasonModeOff,			//part of call close process,
    channelCloseReasonClosedByMcms,			
    channelCloseReasonClosedByRemote,		
    channelCloseReasonMasterSlaveConflict,	
    channelCloseReasonDuplex,				
    channelCloseReasonUnknownReason,		
    channelCloseReasonReopenReason,			
    channelCloseReasonReservationFailure,
	
	channelCloseNoResponseIsNeeded,

	channelStateModesNeedMaintenance,	// Might cause by radVision stack changes.
	channelClosedButWaitForT120Board,	// only for t120 channels

} channelDisconnectedStateModes;
// Based on cmChannelStateMode_e Radvision stack.

typedef enum {
//  ERESOURCES = -2
	rjNoReject,						// Default reason.
	rjAllocatePortFailed	=	1,  //:rjGetFreeIndexFailed
	rjAllocateEntryFailed,
	rjGetPartitionFailed,			// pChannelParams GetDynamicPartitionBuffer failed.
	rjChannelNewFailed		=	10,	
	rjExtraChannel,
	rjInvalidProtocolType,
	rjUnexpectedChannel,			// Not in use.
	rjNoChannelType,					
	rjUnkownPaylodType,					
	rjCallAlreadyDisconnected,			
	rjChanAlreadyDisconnected,
	rjOLCRejectByRemote,			// Receive channelIdle on channelNew instead of channelConnected.
	rjTMBadIndication,				// TimerTask received ctOpenChannelInd or ctStreamOnInd and can't handle with it

} rejectReasonChannel;

// Call Status.
//-------------
typedef enum
{
	ACCEPT,       //0
	DROP,         //1
	CALLFORWARD,  //2
} callStatus;

typedef enum
{
	duplicateMessage,
	missingMessage,
	rejectOutChannel,
}stopAllProcessorsReason;

#endif // __DISCONNECTCAUSE_H__
