// IpChannelParams.h
// Uri A.

#ifndef __IPCHANNELPARAMS_H__
#define __IPCHANNELPARAMS_H__

#define H263_Annexes_Number	22
#define H263_Custom_Number	6 //Number of customPictureFormat. SIZE (1..16)


typedef struct annexes_fd_set {
	long fds_bits[1];

} annexes_fd_set;


#ifndef RMX1000
typedef enum
{
	kUnknownFormat = -1,
	kQCif  = 0,
	kCif,
	k4Cif,
	k16Cif,
	kVGA,
	kNTSC,
	kSVGA,
	kXGA,
	kSIF, //kQNTSC,
	kQVGA,
	k2Cif,
	k2SIF,
	k4SIF,
	k525SD,
	k625SD,
	k720p,
	k1080p,
	kLastFormat
} EFormat;
#endif

typedef enum{

	typeAnnexB,
	typeAnnexD,
	typeAnnexE,
	typeAnnexF,
	typeAnnexG,
	typeAnnexH,
	typeAnnexI,
	typeAnnexJ,
	typeAnnexK,
	typeAnnexL,
	typeAnnexM,
	typeAnnexN,
	typeAnnexO,
	typeAnnexP,
	typeAnnexQ,
	typeAnnexR,
	typeAnnexS,
	typeAnnexT,
	typeAnnexU,
	typeAnnexV,
	typeAnnexW,

	/* typeAnnexI_NS must be last!!!! */
	typeAnnexI_NS, //for MCMS internal use
	typeCustomPic
}annexesListEn;

typedef enum{ 								//Capabilities.h
	tblEncToken,
	tblCapBuffer,
	tblAltGk,
	tblH460Conf,
	tblCustomFormat = typeAnnexW+1
}tblTypes;

/////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	eG711Alaw64kCapCode,
	eG711Alaw56kCapCode,
	eG711Ulaw64kCapCode,
	eG711Ulaw56kCapCode,
	eG722_64kCapCode,
	eG722_56kCapCode,
	eG722_48kCapCode,
	eG722Stereo_128kCapCode,
	eG728CapCode,
	eG729CapCode,
	eG729AnnexACapCode,
	eG729wAnnexBCapCode,
	eG729AnnexAwAnnexBCapCode,
	eG7231CapCode,
	eIS11172AudioCapCode,
	eIS13818CapCode,
	eG7231AnnexCapCode,
	eG7221_32kCapCode,
	eG7221_24kCapCode,
	eG7221_16kCapCode,
	eSiren14_48kCapCode,
	eSiren14_32kCapCode,
	eSiren14_24kCapCode,
	eG7221C_48kCapCode,
	eG7221C_32kCapCode,
	eG7221C_24kCapCode,
	eG7221C_CapCode,
	eSiren14Stereo_48kCapCode,
	eSiren14Stereo_56kCapCode,
	eSiren14Stereo_64kCapCode,
	eSiren14Stereo_96kCapCode,

	eG719_32kCapCode,
	eG719_48kCapCode,
	eG719_64kCapCode,
	eG719_96kCapCode,
	eG719_128kCapCode,
	eSiren22_32kCapCode,
	eSiren22_48kCapCode,
	eSiren22_64kCapCode,
	eG719Stereo_64kCapCode,
	eG719Stereo_96kCapCode,
	eG719Stereo_128kCapCode,
	eSiren22Stereo_64kCapCode,
	eSiren22Stereo_96kCapCode,
	eSiren22Stereo_128kCapCode,
	eSirenLPR_32kCapCode,
	eSirenLPR_48kCapCode,
	eSirenLPR_64kCapCode,
	eSirenLPRStereo_64kCapCode,
	eSirenLPRStereo_96kCapCode,
	eSirenLPRStereo_128kCapCode,
//TIP
	eAAC_LDCapCode,

	eSirenLPR_Scalable_32kCapCode,
	eSirenLPR_Scalable_48kCapCode,
	eSirenLPR_Scalable_64kCapCode,
	eSirenLPRStereo_Scalable_64kCapCode,
	eSirenLPRStereo_Scalable_96kCapCode,
	eSirenLPRStereo_Scalable_128kCapCode,

	eiLBC_13kCapCode,
	eiLBC_15kCapCode,

 	eOpus_CapCode,
   	eOpusStereo_CapCode,	
	eRfc2833DtmfCapCode,
	eH261CapCode,
	eH262CapCode,
	eH263CapCode,
	eH264CapCode,
	eH26LCapCode,
	eRtvCapCode,
	eIS11172VideoCapCode,
	eVP8CapCode,
	
	eGenericVideoCapCode,

	eSvcCapCode,

	eT120DataCapCode,
	eAnnexQCapCode,
	eRvFeccCapCode,
	eNonStandardCapCode,
	eGenericCapCode,
	ePeopleContentCapCode,
	eRoleLabelCapCode,
	eH239ControlCapCode,
	eChairControlCapCode,
	eEncryptionCapCode,
	eDBC2CapCode,
	eLPRCapCode,

	eDynamicPTRCapCode,

	eIcePwdCapCode,
	eIceUfragCapCode,
	eIceCandidateCapCode,
	eIceRemoteCandidateCapCode,
	eRtcpCapCode,
	eSdesCapCode,

	eBFCPCapCode,

	eMCCFCapCode,

	eDtlsCapCode,
	eMsSvcCapCode,

   eFECCapCode,                // 0x5C
   eREDCapCode,                // 0x5D
   
   eSiren7_16kCapCode,         // 0x5E

	eUnknownAlgorithemCapCode // 0x5F Must be the last one.

} CapEnum;


// If you change this Enum - Let MFA CM people know
typedef enum
{
	kEmptyChnlType		= 0,
	kIpAudioChnlType	= 1,
	kIpVideoChnlType	= 2,
	kIpFeccChnlType		= 3,
	kIpContentChnlType	= 4,
	kPstnAudioChnlType	= 5,
	kRtmChnlType		= 6,
	kIsdnMuxChnlType	= 7,
	kBfcpChnlType		= 8,
	kSvcAvcChnlType		= 9,
	kAvcVSWChnType		= 10,
	kAvcToSacChnlType   = 11,
	kAvcToSvcChnlType	= 12,
	kUnknownChnlType	= 13,
    kChanneltypeMAX
}kChanneltype;

typedef enum
{
	MemortAllocationFailure		= 0x0,
	SendToFastQueueFailed,
	BchUnSynchronized,
	PacketReleaseFailure,
	RealTimeProblems,
	VideoJitterBufferOverFlow,
	AudioJitterBufferOverFlow,
	FailedToAddNewPacketToRtpQueue,
	Starvation,
	PacketNotReadyFailure,
	VideoIsOnlyFillFrames,
	NoIntraInVideoStream,
	ParserOutOfSync,
	noConnWithRemoteInd,
	Audio1FramePerPacket,	// G7231 / G7239
	Audio10FramesPerPacket, // G711
	lastBadSpontanIndReason
}BadSpontanIndReason;

typedef enum
{
	H261previewStreamPT = 31,
	H263previewStreamPT = 34,
	H263PLUSpreviewStreamPT = 35,
	H264previewStreamPT = 37
} previewStreamPayloadType;

#ifndef __CS__
// this part of the file should not be compiled by the CS code only declared in it for documentation
typedef enum {
  cmCapReceive=1,
  cmCapTransmit=2,
  cmCapReceiveAndTransmit=3,

  cmCapDirectionMAX
} cmCapDirection;


#endif //__CS_USE__

// Channels
#define MaxAudioDirectedChannelsPerCall	1
#define MaxVideoDirectedChannelsPerCall	2	// people + content
#define MaxFeccDirectedChannelsPerCall	1

#define MaxDirectedChannelsPerCall		(MaxAudioDirectedChannelsPerCall + MaxVideoDirectedChannelsPerCall +  MaxFeccDirectedChannelsPerCall)
#define MaxChannelsPerCall				(2 * MaxDirectedChannelsPerCall)


#endif // __IPCHANNELPARAMS_H__

