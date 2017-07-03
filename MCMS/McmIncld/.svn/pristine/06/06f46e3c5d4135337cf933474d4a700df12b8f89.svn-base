
// Capabilities.h
// Yossi Maimon
// Mikhail Karasik
// Ran Stone

#ifndef __CAPABILITIES_H__
#define __CAPABILITIES_H__

// Includes
//----------------------------------------------------

#include "DataTypes.h"
#include "CapH263Annexes.h"
#include "IpCsSizeDefinitions.h"
#include "NonStandard.h"
#include "CsHeader.h"
#include "IpCsEncryptionDefinitions.h"
#include "SrtpStructsAndDefinitions.h"
#include <ICEApiDefinitions.h>
#include "VideoStructs.h"

// Defines
//----------------------------------------------------


#define rfc_2429			2429

#define OID_H241AnnexA		"itu-t(0) recommendation(0) h(8) 241 \
							specificVideoCodecCapabilities(0) h264(0) \
							iPpacketization(0)"

#define OID_264				"itu-t(0) recommendation(0) h(8) 241\
							specificVideoCodecCapabilities(0) h264(0) generic-capabilities(1)"

#define OID_H239			"itu-t (0) recommendation (0) h (8) 239\
							generic-capabilities (1) h239ControlCapability (1)"

#define	OID_H239_EXT		"itu-t (0) recommendation (0) h (8) 239\
							generic-capabilities (1) h239ExtendedVideoCapability (2)"

#define OID_H224			"itu-t recommendation h 224 1 0"

#define OID_DYNAMIC_PT		"itu-t (0) recommendation (0) h (8) 245\
							generic-capabilities(1) control (3) dynamic-rtp-payload-replacement(1)"

#define oidBuffSize			128

#define CAP_FD_SETSIZE		0x100
#define CAP_NFDBITS			0x20        // bits per mask

#define SVC_SCALE_TEMPORAL  4
#define SVC_SCALE_SPATIAL   2
#define SVC_SCALE_SNR       1
#define SVC_SCALE_NONE      0

//H224 FECC channel is used for both FECC and MRC-SCP, if MRC-SCP mask is turned on, then we need
#define	FeccMask     0x01
#define MrcScpMask   0x02

#define MAX_NUM_STREAMS_PER_SET     10
#define MAX_NUM_RECV_STREAM_GROUPS  5
typedef struct
{
    APIU32          streamSsrcId;       // the stream SSRC
    APIU32          frameWidth;
    APIU32          frameHeight;
    APIU32          maxFrameRate;
    APIU32          requstedStreamSsrcId;  // specific stream that this SSRC want to receive
} STREAM_S;

typedef struct
{
    APIU32     streamGroupId;
    APIU8      numberOfStreams;
    STREAM_S   streams[MAX_NUM_STREAMS_PER_SET];
} STREAM_GROUP_S;

typedef struct cap_fd_set {
	long fds_bits[FD_SET_SIZE];

} cap_fd_set;

// Macros for Select fd_sets
//----------------------------------------------------

// Sets bit 'n' in the bitmask.
#define CAP_FD_SET(n, p)    ((p)->fds_bits[(n)/CAP_NFDBITS] |= \
							(((unsigned long) 0x80000000) >> ((n) % CAP_NFDBITS)))

// Clears bit 'n' in the bitmask.
#define CAP_FD_CLR(n, p)    ((p)->fds_bits[(n)/CAP_NFDBITS] &= \
							~(((unsigned long) 0x80000000) >> ((n) % CAP_NFDBITS)))

// Checks to see if bit 'n' is set.
#define CAP_FD_ISSET(n, p)  ((p)->fds_bits[(n)/CAP_NFDBITS] & \
							(((unsigned long) 0x80000000) >> ((n) % CAP_NFDBITS)))

// Zeroes out the cap_fd_set.
#define CAP_FD_ZERO(p)   {                        \
                        (p)->fds_bits[0] = 0; \
                        (p)->fds_bits[1] = 0; \
                        (p)->fds_bits[2] = 0; \
                        (p)->fds_bits[3] = 0; \
                        (p)->fds_bits[4] = 0; \
                        (p)->fds_bits[5] = 0; \
                        (p)->fds_bits[6] = 0; \
                        (p)->fds_bits[7] = 0; \
                     }


//----------------------------------------------------

#define	 MAX_ALT_CAPS		15
#define	 CAP_ARRAY_SIZE		90
#define	 MAX_CAP_SIM		3

// TIP
#define  MAX_AACLD_MIME_TYPE	32
#define  MAX_AACLD_MODE		8
#define  MAX_AACLD_CONFIG	8

typedef struct{
	xmlDynamicHeader		xmlHeader;
	APIU8					capTypeCode;
	APIU8					sipPayloadType;
	APIU16					capLength;
	xmlDynamicProperties	xmlDynamicProps;
}capBufferBase;

typedef struct{
	xmlDynamicHeader		xmlHeader;
	APIU8					capTypeCode;
	APIU8					sipPayloadType;
	APIU16					capLength;
	xmlDynamicProperties	xmlDynamicProps;
	unsigned char			dataCap[1];
}capBuffer;

typedef struct {
	cap_fd_set				altMatrix;
	APIU8					numberOfCaps;
	APIU8					numberOfAlts;
	APIU8					numberOfSim;
	APIU8					filler2;
 	xmlDynamicProperties	xmlDynamicProps;
}ctCapabilitiesBasicStruct;

typedef struct capFromMcms{
	cap_fd_set				altMatrix;
	APIU8					numberOfCaps;
	APIU8					numberOfAlts;
	APIU8					numberOfSim;
	APIU8					filler2;
	xmlDynamicProperties	xmlDynamicProps;
	capBuffer				caps;
}ctCapabilitiesStruct;

//---------------------------------

#define MaxPathLength	512
#define CapNameLength	64

typedef struct {
  xmlDynamicHeader	xmlHeader;
  APIU8				direction;
  APIU8				type;
  APIU8				roleLabel;
  APIU8				capTypeCode;
} ctCapStruct;

// MCMS use.
typedef struct{
	ctCapStruct		header;
}BaseCapStruct;

//LYNC2013_FEC_RED:
typedef BaseCapStruct fecCapStruct;

//Audio capabilities algorithems :
//--------------------------------
//	nonStandard, g711Alaw64k, g711Alaw56k, g711Ulaw64k,	g711Ulaw56k,
//	g722-64k, g722-56k,	g722-48k, g7231, g728, g729, g729AnnexA,
//	is11172AudioCapability, is13818AudioCapability, g729wAnnexB,
//	g729AnnexAwAnnexB, g7231AnnexCCapability, ACC-LD (TIP)

//Video capabilities algorithems :
//--------------------------------
//	nonStandard, h261VideoMode,	h262VideoMode, h263VideoMode, is11172VideoMode,
//  GenericVideo (H26L).

//Data capabilities algorithems :
//--------------------------------
//	nonStandard, t120,	dsm_cc, userData, t434, h224, h222DataPartitioning,
//	t30fax, t84, nlpid, dsvdControl.


typedef struct{
	ctCapStruct			header;
	APIS32				rtcpFeedbackMask;
	APIS32				maxValue;
	APIS32				minValue; // must be the last
}audioCapStructBase;

typedef audioCapStructBase redCapStruct;

typedef audioCapStructBase g711Alaw64kCapStructtypedef;
typedef audioCapStructBase g711Alaw64kCapStruct;
typedef audioCapStructBase g711Alaw56kCapStruct;
typedef audioCapStructBase g711Ulaw64kCapStruct;
typedef audioCapStructBase g711Ulaw56kCapStruct;
typedef audioCapStructBase g722_64kCapStruct;
typedef audioCapStructBase g722_56kCapStruct;
typedef audioCapStructBase g722_48kCapStruct;
typedef audioCapStructBase g722Stereo_128kCapStruct;
typedef audioCapStructBase g728CapStruct;
typedef audioCapStructBase g729CapStruct;
typedef audioCapStructBase g729AnnexACapStruct;
typedef audioCapStructBase g729wAnnexBCapStruct;
typedef audioCapStructBase g729AnnexAwAnnexBCapStruct;
typedef audioCapStructBase g7221_32kCapStruct;
typedef audioCapStructBase g7221_24kCapStruct;
typedef audioCapStructBase g7221_16kCapStruct;
typedef audioCapStructBase siren7_16kCapStruct;
typedef audioCapStructBase siren14_48kCapStruct;
typedef audioCapStructBase siren14_32kCapStruct;
typedef audioCapStructBase siren14_24kCapStruct;
typedef audioCapStructBase g7221C_48kCapStruct;
typedef audioCapStructBase g7221C_32kCapStruct;
typedef audioCapStructBase g7221C_24kCapStruct;
typedef audioCapStructBase g719_32kCapStruct;
typedef audioCapStructBase g719_48kCapStruct;
typedef audioCapStructBase g719_64kCapStruct;
typedef audioCapStructBase g719_96kCapStruct;
typedef audioCapStructBase g719_128kCapStruct;
typedef audioCapStructBase siren22_32kCapStruct;
typedef audioCapStructBase siren22_48kCapStruct;
typedef audioCapStructBase siren22_64kCapStruct;
typedef audioCapStructBase siren14Stereo_96kCapStruct;
typedef audioCapStructBase siren14Stereo_64kCapStruct;
typedef audioCapStructBase siren14Stereo_56kCapStruct;
typedef audioCapStructBase siren14Stereo_48kCapStruct;
typedef audioCapStructBase g719Stereo_64kCapStruct;
typedef audioCapStructBase g719Stereo_96kCapStruct;
typedef audioCapStructBase g719Stereo_128kCapStruct;
typedef audioCapStructBase siren22Stereo_64kCapStruct;
typedef audioCapStructBase siren22Stereo_96kCapStruct;
typedef audioCapStructBase siren22Stereo_128kCapStruct;
typedef audioCapStructBase iLBC_13kCapStruct;
typedef audioCapStructBase iLBC_15kCapStruct;
typedef audioCapStructBase rfc2833DtmfCapStruct;



/** Opus uses only one hex code for all the
 *  bit rates as well as the mono/stereo.
 */
typedef struct{
	ctCapStruct			header;
	APIS32				rtcpFeedbackMask;
	APIS32				maxValue;
	APIS32				minValue;

    APIU32				maxAverageBitrate;
    APIU32				maxPtime;
    APIU32				minPtime;
    APIU32				cbr;
    APIU32				useInbandFec;
    APIU32				useDtx;

}opus_CapStruct;

typedef opus_CapStruct opusStereo_CapStruct;


/**
 * When SirenLPR_Stereo is supported then Mono is also implicitly supported.
 * */
#	define				sirenLPRMono		0x0001
#	define				sirenLPRStereo		0x0002

/** SirenLPR uses only one hex code for all the
 *  bit rates as well as the mono/stereo.
 */
typedef struct{
	ctCapStruct			header;
	APIS32				rtcpFeedbackMask;
	APIS32				maxValue;
	APIS32				minValue;
    APIU32				sirenLPRMask;

}sirenLPR_CapStruct;

/** SirenLPRScalable uses only one hex code for all the
 *  bit rates as well as the mono/stereo.
 */
typedef struct{
    ctCapStruct         header;
    APIS32				rtcpFeedbackMask;
    APIS32              maxValue;
    APIS32              minValue;
    APIU32              sirenLPRMask;

    APIU32              sampleRate;
    APIU32              mixDepth;
    STREAM_GROUP_S      recvStreamsGroup;
    STREAM_GROUP_S      sendStreamsGroup;
}sirenLPR_Scalable_CapStruct;

typedef sirenLPR_CapStruct sirenLPR_32kCapStruct;
typedef sirenLPR_CapStruct sirenLPR_48kCapStruct;
typedef sirenLPR_CapStruct sirenLPR_64kCapStruct;
typedef sirenLPR_CapStruct sirenLPRStereo_64kCapStruct;
typedef sirenLPR_CapStruct sirenLPRStereo_96kCapStruct;
typedef sirenLPR_CapStruct sirenLPRStereo_128kCapStruct;

typedef struct{
	ctCapStruct			header;
	APIS32				rtcpFeedbackMask;
	APIS32				maxValue;
	APIS32				minValue; // must be the last
    APIU32				capBoolMask;

#	define				g7221C_Mask_Rate48K		0x80000000
#	define				g7221C_Mask_Rate32K		0x40000000
#	define				g7221C_Mask_Rate24K		0x20000000

}g7221C_CapStruct;

typedef struct{
	ctCapStruct			header;
	APIS16				maxAl_sduAudioFrames;	// INTEGER (1..256)
    APIS16				capBoolMask;
	APIS32				minAl_sduAudioFrames;	// INTEGER (1..256) // must be the last

#	define				g7231_silenceSuppression		0x8000

	// path = audioData.g7231.(element name)
}g7231CapStruct;

typedef struct{
	ctCapStruct			header;
	APIS16				bitRate;				// (1..448) -- units kbit/s

        APIS16				capBoolMask;

#	define				IS11172Audio_audioLayer1		0x8000
#	define				IS11172Audio_audioLayer2		0x4000
#	define				IS11172Audio_audioLayer3		0x2000
#	define				IS11172Audio_audioSampling32k	0x1000
#	define				IS11172Audio_audioSampling44k1	0x0800
#	define				IS11172Audio_audioSampling48k	0x0400
#	define				IS11172Audio_singleChannel		0x0200
#	define				IS11172Audio_twoChannels		0x0100

	// path = audioData.IS11172AudioCapability.(element name)
}IS11172AudioCapStruct;

typedef struct{
	ctCapStruct			header;
	APIS32				bitRate;				// INTEGER (1..1130) -- units kbit/s

        APIS32				capBoolMask;

#	define				IS13818_audioLayer1				0x80000000
#	define				IS13818_audioLayer2				0x40000000
#	define				IS13818_audioLayer3				0x20000000
#	define				IS13818_audioSampling16k		0x10000000
#	define				IS13818_audioSampling22k05		0x08000000
#	define				IS13818_audioSampling24k		0x04000000
#	define				IS13818_audioSampling32k		0x02000000
#	define				IS13818_audioSampling44k1		0x01000000
#	define				IS13818_audioSampling48k		0x00800000
#	define				IS13818_singleChannel			0x00400000
#	define				IS13818_twoChannels				0x00200000
#	define				IS13818_threeChannels2_1		0x00100000
#	define				IS13818_threeChannels3_0		0x00080000
#	define				IS13818_fourChannels2_0_2_0		0x00040000
#	define				IS13818_fourChannels2_2			0x00020000
#	define				IS13818_fourChannels3_1			0x00010000
#	define				IS13818_fiveChannels3_0_2_0		0x00008000
#	define				IS13818_fiveChannels3_2			0x00004000
#	define				IS13818_lowFrequencyEnhancement	0x00002000
#	define				IS13818_multilingual			0x00001000

	// To create path, the '_' must convert to '-'
	// path = audioData.IS13818AudioCapability.(element name)
}IS13818CapStruct;

typedef struct{
	ctCapStruct			header;
	APIS8				maxAl_sduAudioFrames;	// INTEGER (1..256)
	APIS8				highRateMode0;			// INTEGER (27..78) -- units octets
	APIS8				highRateMode1;			// INTEGER (27..78) -- units octets
	APIS8				lowRateMode0;			// INTEGER (23..66) -- units octets
	APIS8				lowRateMode1;			// INTEGER (23..66) -- units octets
	APIS8				sidMode0;				// INTEGER (6..17)  -- units octets
	APIS8				sidMode1;				// INTEGER (6..17)  -- units octets

        APIS8				capBoolMask;
#	define				G7231Annex_silenceSuppression	0x80

	//path=audioData.G7231AnnexCCapability.g723AnnexCAudioMode.(element name)
}G7231AnnexCapStruct;

// TIP --------------------------------------------------------------------------
typedef struct {
	ctCapStruct			header;
	APIS32				rtcpFeedbackMask;
	APIS32				maxValue;
	APIS32				minValue;

	APIS8				mimeType[MAX_AACLD_MIME_TYPE];	// mpeg4-generic
	APIU32				sampleRate;			// 48000
	APIU16				profileLevelId;			//=16;
	APIU16				streamType;			//=5;
	APIS8				mode[MAX_AACLD_MODE];		//=AAC-hbr;
	APIS8				config[MAX_AACLD_CONFIG];	//=11B0 or B98C00;
	APIU16				sizeLength;			//=13;
	APIU16				indexLength;			//=3;
	APIU16				indexDeltaLength;		//=3;
	APIU32				constantDuration;		//=480;

	APIU32 				maxBitRate;
} AAC_LDCapStruct;

// Video :
//--------


/* Definitions for RTCP feedback mask, additional masks can be added in the future */
#define	RTCP_MASK_FIR						0x0001
#define RTCP_MASK_TMMBR						0x0002
#define RTCP_MASK_PLI						0x0004
#define RTCP_MASK_IS_NOT_STANDARD_ENCODE	0x0008
//For Lync2013
#define	RTCP_MASK_MS_SRC					0x0010	// Microsoft video source request
#define RTCP_MASK_MS_XPLI					0x0020	// Microsoft X-PLI Picture Loss Indicator
#define RTCP_MASK_MS_DSH					0x0030	// Microsoft Dominant Speaker History Notification

typedef struct{
	ctCapStruct			header;

	APIS32				maxBitRate;			// INTEGER (1..19200),		-- units of 100 bit/s
	APIS8				qcifMPI;			// INTEGER (1..4) OPTIONAL, -- units 1/29.97 Hz
	APIS8				cifMPI;				// INTEGER (1..4) OPTIONAL, -- units 1/29.97 Hz
	APIS8				filler[2];
    APIS32				capBoolMask;
    APIS32				rtcpFeedbackMask;
#	define				h261_temporalSpatialTradeOffCapability	0x80000000
#	define				h261_stillImageTransmission				0x40000000

	// path = videoData.h261VideoCapability.(element name)
}h261CapStruct;

typedef struct{
	ctCapStruct			header;

	APIS32				videoBitRate;		// INTEGER (0.. 1073741823) -- units 400 bit/s
	APIS32				vbvBufferSize;		// INTEGER (0.. 262143)		-- units 16 384 bits
	APIS32				LuminanceSampleRate;// INTEGER (0..4294967295)  -- units samples/s
	APIS16				samplesPerLine;		// INTEGER (0..16383)		-- units samples/line
	APIS16				linesPerFrame;		// INTEGER (0..16383)		-- units lines/frame
	APIS8				framesPerSecond;	// INTEGER (0..15)			-- frame_rate_code
	APIS8				filler1;

    APIS16				capBoolMask;
    APIS32				rtcpFeedbackMask;

#	define				h262_profileAndLevel_SPatML			0x8000
#	define				h262_profileAndLevel_MPatLL			0x4000
#	define				h262_profileAndLevel_MPatML			0x2000
#	define				h262_profileAndLevel_MPatH_14		0x1000
#	define				h262_profileAndLevel_MPatHL			0x0800
#	define				h262_profileAndLevel_SNRatLL		0x0400
#	define				h262_profileAndLevel_SNRatML		0x0200
#	define				h262_profileAndLevel_SpatialatH_14	0x0100
#	define				h262_profileAndLevel_HPatML			0x0080
#	define				h262_profileAndLevel_HPatH_14		0x0040
#	define				h262_profileAndLevel_HPatHL			0x0020

	// path = videoData.h262VideoCapability.(element name)
}h262CapStruct;

typedef struct{
	ctCapStruct			header;

	APIS32				maxBitRate;			// INTEGER (1..192400)		-- units 100 bit/s
	APIS32				hrd_B;				// INTEGER (0..524287)		-- units 128 bits
	APIS16				bppMaxKb;			// INTEGER (0..65535)		-- units 1024 bits
	APIS16				slowSqcifMPI;		// INTEGER (1..3600)		-- units seconds/frame
	APIS16				slowQcifMPI;		// INTEGER (1..3600)		-- units seconds/frame
	APIS16				slowCifMPI;			// INTEGER (1..3600)		-- units seconds/frame
	APIS16				slowCif4MPI;		// INTEGER (1..3600)		-- units seconds/frame
	APIS16				slowCif16MPI;		// INTEGER (1..3600)		-- units seconds/frame
	APIS8				sqcifMPI;			// INTEGER (1..32)			-- units 1/29.97 Hz
	APIS8				qcifMPI;			// INTEGER (1..32)			-- units 1/29.97 Hz
	APIS8				cifMPI;				// INTEGER (1..32)			-- units 1/29.97 Hz
	APIS8				cif4MPI;			// INTEGER (1..32)			-- units 1/29.97 Hz
	APIS8				cif16MPI;			// INTEGER (1..32)			-- units 1/29.97 Hz
	APIS8				filler;

    APIS16				capBoolMask;

#	define				h263_unrestrictedVector					0x8000		//Annex D
#	define				h263_arithmeticCoding					0x4000		//Annex E
#	define				h263_advancedPrediction					0x2000		//Annex F
#	define				h263_pbFrames							0x1000		//Annex G
#	define				h263_temporalSpatialTradeOffCapability	0x0800
#	define				h263_errorCompensation					0x0400		//Annex N

#	define				h263Options_separateVideoBackChannel	0x0200
#	define				h263Options_videoBadMBsCap				0x0100		//with Annex W

	annexes_fd_set		annexesMask;
	xmlDynamicProperties	xmlDynamicProps;
	APIS32				rtcpFeedbackMask;

}h263CapStructBase;

typedef struct{
	ctCapStruct			header;

	APIS32				maxBitRate;
	APIS32				hrd_B;
	APIS32				rtcpFeedbackMask;
	APIS16				bppMaxKb;
	APIS16				slowSqcifMPI;
	APIS16				slowQcifMPI;
	APIS16				slowCifMPI;
	APIS16				slowCif4MPI;
	APIS16				slowCif16MPI;
	APIS8				sqcifMPI;
	APIS8				qcifMPI;
	APIS8				cifMPI;
	APIS8				cif4MPI;
	APIS8				cif16MPI;
	APIS8				filler;

    APIS16				capBoolMask;

#	define				h263_unrestrictedVector					0x8000
#	define				h263_arithmeticCoding					0x4000
#	define				h263_advancedPrediction					0x2000
#	define				h263_pbFrames							0x1000
#	define				h263_temporalSpatialTradeOffCapability	0x0800
#	define				h263_errorCompensation					0x0400

#	define				h263Options_separateVideoBackChannel	0x0200
#	define				h263Options_videoBadMBsCap				0x0100

	//enhncementLayerInfo	enhncementLayerInfo OPTIONAL
	annexes_fd_set		annexesMask;
	xmlDynamicProperties	xmlDynamicProps;
	char				annexesPtr[1]; //sequece of h263OptionsStruct;
}h263CapStruct;

#define H264_standard     0x0001
#define H264_tipContent   0x0002

typedef struct{
	ctCapStruct			header;
	APIS32				customMaxMbpsValue;
	APIS32				customMaxFsValue;
	APIS32				customMaxDpbValue;
	APIS32				customMaxBrAndCpbValue;
	APIS32				maxStaticMbpsValue;
	APIS32				sampleAspectRatiosValue;

	APIS32				maxBitRate;     // INTEGER (1..19200), -- units of 100 bit/s
	APIS32				maxFR;
	APIS32              H264mode;
	APIS32				rtcpFeedbackMask;

	APIU16				profileValue;   // H264_Profile_BaseLine
	APIU8				levelValue;
	APIU8				packetizationMode;
	APIS8				filler;

}h264CapStruct;

typedef struct {
	h264CapStruct 		h264;

    // SVC fields
    VIDEO_OPERATION_POINT_SET_S operationPoints;
    STREAM_GROUP_S              recvStreamsGroup;
    STREAM_GROUP_S              sendStreamsGroup;
    APIU8                       scalableLayerId;    // needed for legacy SVC support; if MRC SVC, it will be ignored
    APIU8                       isLegacy;           // 0 – for MRC, 1 – for legacy

} svcCapStruct;

typedef struct {
	ctCapStruct			header;
	APIS8 				icePwd[IcePwdLen];
} icePwdCapStruct;

typedef struct {
	ctCapStruct			header;
	APIS8 				iceUfrag[IceUfragLen];
} iceUfragCapStruct;

typedef struct {
	ctCapStruct			header;
	APIS8 				candidate[512];
	APIS8 				candidateType; // eCandidateTypeV4 for "a=candidate" ; eCandidateTypeV6 for "a=x-candidate-ipv6"
	APIS8				filler1;
	APIS8				filler2;
	APIS8				filler3;
} iceCandidateCapStruct;

typedef struct {
	ctCapStruct			header;
	APIS8 				candidate[512];
} iceRemoteCandidateCapStruct;

typedef struct {
	ctCapStruct			header;
	APIU16				port;
} rtcpCapStruct;

typedef struct{
	ctCapStruct			header;
	APIS32				maxFR;
	APIS32				maxFS;
	APIS32              maxBitRate;
	APIS32				rtcpFeedbackMask;
	//additional parameters: resolution etc.
} vp8CapStruct;

#ifdef __BFCP_CS_CONNECTION_ENABLED__
typedef enum{
	bfcp_msg_status_ok 			= 0,
	bfcp_msg_status_connected   = 1,
	bfcp_msg_status_error 		= 2
} eBfcpMsgStatus;
#endif //__BFCP_CS_CONNECTION_ENABLED__

typedef enum{
	bfcp_flctrl_null = 0,
	bfcp_flctrl_c_s    = 1,
	bfcp_flctrl_c_only = 2,
	bfcp_flctrl_s_only = 3
} eBfcpFloorCtrl;

typedef enum{
	bfcp_setup_null = 0,
	bfcp_setup_actpass = 1,
	bfcp_setup_active = 2,
	bfcp_setup_passive = 3,
	bfcp_setup_holdconn = 4
} eBfcpSetup;

typedef enum{
	bfcp_connection_null = 0,
	bfcp_connection_new = 1,
	bfcp_connection_existing = 2
} eBfcpConnection;

typedef enum {
	bfcp_m_stream_None,
	bfcp_m_stream,
	bfcp_mstrm,
} eBfcpMStreamType;

typedef struct{
	APIS8				floorid[32];
	APIS8				m_stream_0[32];
	APIS8				m_stream_1[32];
	APIS8				m_stream_2[32];
	APIS8				m_stream_3[32];
}bfcpFlooridStruct;

typedef struct{
	ctCapStruct			header;
	APIU8				floorctrl;
	APIS8				confid[32];
	APIS8				userid[32];
	bfcpFlooridStruct	floorid_0;
	bfcpFlooridStruct	floorid_1;
	bfcpFlooridStruct	floorid_2;
	bfcpFlooridStruct	floorid_3;
	APIU8				setup;
	APIU8				connection;
	APIU8				xbfcp_info_enabled;
	APIU16				xbfcp_info_time;
	APIU8				mstreamType;
	enTransportType			transType;
}bfcpCapStruct;


//  RTV parameters :
// ==============================

//Width
//176 for QCIF
//352 for CIF
//640 for VGA
//1280 for HD

//High
//144 for QCIF
//288 for CIF
//480 for VGA
//720 for HD
#define NumOfRtvItems	7

typedef struct {
	APIU32 	capabilityID;	// Unique random integer among the listed capability ID
	APIU32 	widthVF;		// Width of video frame
	APIU32 	heightVF;		// Height of video frame
	APIU32 	fps;			// Frames-per-second
	APIU32	maxBitrateInBps;// For future use : maximum-bitrate-in bits-per-second

}rtvCapItemS;

typedef struct{
	ctCapStruct		header;
	APIU32			numOfItems;
	rtvCapItemS 	rtvCapItem[NumOfRtvItems];
	APIS32			rtcpFeedbackMask;

}rtvCapStruct;


//  DBC2 capability parameters :
// ==============================

//  |8|7  |6            |5            |4                    |3|2|1     |
//	| |Res|motionVectors|canInterleave|requiresEncapsulation|maxOverlap|
//  |0|0  |0            |0            |1                    |0|1|0     |

#define DBC2_motionVectors_Cap			0x20
#define DBC2_CanInterleave_Cap			0x10
#define DBC2_requiresEncapsulation_Cap	0x08
#define DBC2_maxOverlap_Cap				0x07

//  DBC2 On command options:
// ==========================

//  |8|7|6|5            |4              |3|2|1  |
//	|Res  |motionVectors|NoEncapsulation|Overlap|
//  |0|0|0|x            |1              |x|x|x  |

#define DBC2_MotionVectors_Options		0x10
#define DBC2_NoEncapsulation_Options	0x08
#define DBC2_Overlap_Options			0x07


typedef enum{
	eH26LCode,
	eDropField,
	eDBC2Code,
	eUnknownGenericCode,		// last one.
} genericCodeEnum;

typedef struct{
	ctCapStruct			header;
	APIS32				genericCodeType; // genericCodeEnum
	APIS32				maxBitRate;
	char				data[CT_GenricVideo_Data_Len];
}genericVideoCapStruct;

typedef struct{
	ctCapStruct			header;

	APIU32				versionID;
	APIU32				minProtectionPeriod;
	APIU32				maxProtectionPeriod;
	APIU32				maxRecoverySet;
	APIU32				maxRecoveryPackets;
	APIU32				maxPacketSize;

}lprCapStruct;

typedef struct{
	ctCapStruct			header;

	APIS32				videoBitRate;			// INTEGER(0..1073741823) -- units 400 bit/s
	APIS32				luminanceSampleRate;	// INTEGER(0..4294967295) -- units samples/s
	APIS32				vbvBufferSize;			// INTEGER(0..262143)	  -- units 16 384 bits
	APIS16				samplesPerLine;			// INTEGER(0..16383)	  -- units samples/line
	APIS16				linesPerFrame;			// INTEGER(0..16383)	  -- units samples/line
	APIS8				pictureRate;			// INTEGER(0..15)
	APIS8				filler[2];

    APIS8				capBoolMask;
    APIS32				rtcpFeedbackMask;

#	define				IS11172Video_constrainedBitstream		0x80

	// path = videoData.IS11172VideoCapability.(element name)
}IS11172VideoCapStruct;

// Generic :
//----------
typedef audioCapStructBase genericCapStruct;

// Control :
//----------
typedef audioCapStructBase PeopleAndContentCapStruct;
typedef audioCapStructBase h239ControlCapStruct;
typedef audioCapStructBase DynamicPTRCapStruct;
typedef audioCapStructBase RoleLabelCapStruct;


// NonStandard :
//-------------

typedef struct{
	ctCapStruct					header;
	ctNonStandardParameterSt	nonStandardData;

	//path= {videoData/audioData/data}.NonStandardParameter...(element name)
}ctNonStandardCapStruct;

typedef struct{
	ctCapStruct			header;
	APIS32				value;
}ChairControlCapStruct;

// Data :
//-------

typedef struct{
	ctCapStruct			header;
	APIS32				maxBitRate;
	APIS8               mask;	//currently used for H224 Fecc and MrcScp Mask
}dataCapStructBase;

typedef dataCapStructBase t120DataCapStruct;
typedef dataCapStructBase annexQDataCapStruct;
typedef dataCapStructBase rvFeccDataCapStruct;


//  Encryption Capability.
// ------------------------

typedef struct{
	ctCapStruct		header;
	APIU16			type;  //EencryMediaType
	APIU16			entry; //the entry to be encrypted.
}encryptionCapStruct;


//  Encryption Token.
// ------------------

typedef struct{
	xmlDynamicHeader		xmlHeader;
	APIU32					modSize;		// BIT STRING
	APIU32					generator;		// BIT STRING
	APIU16					tokenOID;		// EenHalfKeyType
	APIU16					filler;
	APIU32					hkLen;			// The len of the halfKey in BYTE
}encryptionTokenBase;

typedef struct{
	xmlDynamicHeader		xmlHeader;		// For XML API.
	APIU32					modSize;		// BIT STRING
	APIU32					generator;		// BIT STRING
	APIU16					tokenOID;		// EenHalfKeyType
	APIU16					filler;
	APIU32					hkLen;			// The len of the halfKey in BYTE
	APIU8					halfKey[1];
}encryptionToken;

typedef struct{

	APIU16					numberOfTokens;
	APIU16					dynamicTokensLen;
	xmlDynamicProperties    xmlDynamicProps;

}encTokensHeaderBasicStruct;

typedef struct{

	APIU16					numberOfTokens;
	APIU16					dynamicTokensLen;
	xmlDynamicProperties    xmlDynamicProps;
	APIU8					token[1];

}encTokensHeaderStruct;

// SRTP section

typedef struct {
	xmlDynamicHeader	xmlHeader;
	sdesKeyParamsStruct	elem;
} xmlSdesKeyParamsStruct;

typedef struct{
	ctCapStruct				header;

	APIU32					tag; 				// mandatory param
	APIU32					cryptoSuite; 		// mandatory param
	sdesSessionParamsStruct	sessionParams;		// optional param

	APIU32					numKeyParams;
	xmlDynamicProperties	xmlDynamicProps;

}sdesCapStructBase;

typedef struct{
	ctCapStruct				header;

	APIU32					tag; 				// mandatory param
	APIU32					cryptoSuite; 		// mandatory param
	sdesSessionParamsStruct	sessionParams;		// optional param

	APIU32					numKeyParams;
	xmlDynamicProperties	xmlDynamicProps;
	char					keyParamsList[1];
}sdesCapStruct;

typedef enum{
	eActive	= 1,
	ePassive,
	eActpass,
	eHoldconn,
} setupModeEnum;

typedef enum{
	eNewConn	= 1,
	eExistingConn,
} connectionTypeEnum;

typedef struct{
	ctCapStruct				header;

//	mcXmlTransportAddress	mccfIpAddress; /* AS IP + port */
	APIU32					setupMode; //setupModeEnum
	APIU32					connectionType; //connectionTypeEnum
	APIU8               	cfw_id[128];
} mccfCapStruct;

typedef struct{
                ctCapStruct        header;
                APIS32             width;
                APIS32             height;
                APIS32             aspectRatio;
                APIS32             maxFrameRate;
                
                APIS32             maxBitRate;     // INTEGER (1..19200), -- units of 100 bit/s
                APIS32             minBitRate;
                                                                
                APIS32             rtcpFeedbackMask;
                APIS32             maxPixelsNum;//width*height

                APIU8              packetizationMode; 
                APIS8               filler;
}msSvcCapStruct;

#endif //__CAPABILITIES_H__


