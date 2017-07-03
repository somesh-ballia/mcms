/*
 * TipStructs.h
 *
 *  Created on: Feb 14, 2011
 *      Author: shmuell
 */

#ifndef TIPSTRUCTS_H_
#define TIPSTRUCTS_H_

#include "DataTypes.h"

// For Party/RA/ART
typedef enum
{
	eTipNone,
	eTipMasterCenter,
	eTipSlaveLeft,
	eTipSlaveRigth,
	eTipSlaveAux,
	eTipVideoAux,
	eTipLast
} ETipPartyTypeAndPosition;

typedef enum {
	eTipNegSuccess = 0,
	eTipNegError,
	eTipNegErrorHdxRemEp
} ETipNegStatus;

typedef enum {
	eTipAudioPosCenter,
	eTipAudioPosLeft,
	eTipAudioPosRight,
	eTipAudioPosAux,
	eTipAudioPosLegacy,
	eTipAudioPosLast
} ETipAudioPosition;

typedef enum {
	eTipVideoPosCenter,
	eTipVideoPosLeft,
	eTipVideoPosRight,
	eTipVideoPosAux5Fps,
	eTipVideoPosAux30Fps,
	eTipVideoPosLegacyCenter,
	eTipVideoPosLegacyLeft,
	eTipVideoPosLegacyRight,
	eTipVideoPosLast
} ETipVideoPosition;

typedef enum {
	eTipAux1FPS,
	eTipAux5FPS,
	eTipAux30FPS,
	eTipAuxNone
} ETipAuxFPS;

typedef enum {
	eRtpAvp,
	eRtpSavp,
	eRtpAvpf,
	eRtpSavpf
} ERtpProfile;

typedef enum {
	eTipDeviceMultipointFocus,
	eTipDeviceTranscoding,
	eTipDeviceNone
} ETipDevice;


// general systemwide properties

typedef struct {
	APIU8	rtpProfile;				// eRtpProfile enum
	APIU8	deviceOptions;			// ETipDevice enum
	APIU32	confId;					// conference id
} TipSystemWideOptionsSt;


// Mux control structures:

typedef struct  {
	APIU8	txPositions[eTipAudioPosLast];	// For each position from ETipAudioPosition enum - 1/0 (supported/unsupported)
	APIU8	rxPositions[eTipAudioPosLast];	// For each position from ETipAudioPosition enum - 1/0 (supported/unsupported)
}TipAudioMuxCntlSt;

typedef struct  {
	APIU8	txPositions[eTipVideoPosLast];	// For each position from ETipVideoPosition enum - 1/0 (supported/unsupported)
	APIU8	rxPositions[eTipVideoPosLast];	// For each position from ETipVideoPosition enum - 1/0 (supported/unsupported)
}TipVideoMuxCntlSt;


// Media Option structures:

typedef struct  {
	APIU32					IsEKT;				// 1/0 (supported/unsupported)
} TipGenericOptionsSt;

typedef struct  {
	TipGenericOptionsSt	genericOptions;
	APIU32				IsAudioDynamicOutput;	// 1/0 (supported/unsupported)
	APIU32				IsAudioActivityMetric;  // 1/0 (supported/unsupported)
	APIU32				G722Negotiation;		// 1/0 (supported/unsupported). Enum will be defined later if needed.
} TipAudioOptionsSt;

typedef struct  {
	TipGenericOptionsSt	genericOptions;
	APIU32				IsVideoRefreshFlag;			// 1/0 (supported/unsupported)
	APIU32				IsInbandParameterSets;		// 1/0 (supported/unsupported)
	APIU32				IsCABAC;					// 1/0 (supported/unsupported)
	APIU32				IsLTRP;						// 1/0 (supported/unsupported)
	APIU32				AuxVideoFPS;				// ETipAuxFPS enum
	APIU32				IsGDR;						// 1/0 (supported/unsupported)
	APIU32				IsHighProfile;				// 1/0 (supported/unsupported)
	APIU32				IsUnrestrictedMediaXGA5or1;	// 1/0 (supported/unsupported)
	APIU32				IsUnrestrictedMediaXGA30;	// 1/0 (supported/unsupported)
	APIU32				IsUnrestrictedMedia720p;	// 1/0 (supported/unsupported)
	APIU32				IsUnrestrictedMedia1080p;	// 1/0 (supported/unsupported)
} TipVideoOptionsSt;


// Tip negotiation structure:

typedef struct  {
	TipSystemWideOptionsSt systemwideOptions;

	TipAudioMuxCntlSt	audioMuxCntl;
	TipVideoMuxCntlSt	videoMuxCntl;

	TipAudioOptionsSt	audioTxOptions;
	TipAudioOptionsSt	audioRxOptions;

	TipVideoOptionsSt	videoTxOptions;
	TipVideoOptionsSt	videoRxOptions;
}TipNegotiationSt;


// TIP API structures:

typedef struct  {
	APIU32				unitId;
	APIU32				portId;
	APIU32              isPreferTip; //TIP call from Polycom EPs feature
	TipNegotiationSt	negotiationSt;
} mcTipStartNegotiationReq;

typedef struct  {
	APIU32				status; 		// negotiation status
	//this is for VIDEO only!!!
	APIU32				doVideoReInvite;
	TipNegotiationSt	negotiationSt;
} mcTipNegotiationResultInd;

typedef struct  {
	APIU32				unitId;
	APIU32				portId;
	APIU32 status;						// negotiation status
} mcTipEndNegotiationReq;


typedef struct  //descriptor of a each party , that is part of TIP (left/right/center/aux)
{
    UINT32  bIsActive;
    UINT32  unPartyId;
    UINT32  unDspNum;
    UINT32  unPortNum;
} TipPartyPortDescriptorSt;


typedef struct
{
    TipPartyPortDescriptorSt leftTipChannelDescr; // don't expect partyId to be filled in left/right/aux
    TipPartyPortDescriptorSt rightTipChannelDescr;
    TipPartyPortDescriptorSt auxTipChannelDescr;
    TipPartyPortDescriptorSt centerTipChannelPartyId; //the center is different, since it is already connected; expect it's partyId to be filled
} mcTipMsgInfoReq;

//////////////////////////////////////////////////////////////////////////////////////////
//     TIP Content structs
//////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
	AuxCntlRequest,
	AuxCntlRequestGranted,
	AuxCntlRequestDenied,
	AuxCntlRelease,
	AuxCntlReleaseAck,
	AuxCntlMsgUnknown
} EAuxCntlSubOpcode;

typedef enum {
	eTipAuxPositionNone,
	eTipAuxPosition1or5FPS,
	eTipAuxPosition30FPS,
	eTipAuxPositionLast
} ETipAuxPosition;

typedef struct  {
	APIU32 subOpcode; // EAuxCntlSubOpcode
	APIU8 auxPositions[eTipAuxPositionLast]; // For each position from ETipAuxPositions 1/0 (supported/unsupported)
} TipContentMsgSt;

typedef struct  {
	APIU32 unitId;
	APIU32 portId;
	TipContentMsgSt ContentMsgSt;
} mcTipContentMsgReq;

typedef struct  {
	TipContentMsgSt ContentMsgSt;
} mcTipContentMsgInd;

#endif /* TIPSTRUCTS_H_ */
