#if !defined(_CONFPARTYH323DEFINES_H__)
#define _CONFPARTYH323DEFINES_H__

//#include "IpCommonDefinitions.h"
//#include "IpCommonTypes.h"


#include "IpAddressDefinitions.h"

#include "NonStandard.h"
#include "Capabilities.h"

typedef struct{
	int					capTypeCode;
	int					capLength;
}capBufferBaseApi522;
/*
typedef struct{
	UINT8				capTypeCode;
	UINT8				sipPayloadType;
	UINT16				capLength;
	unsigned char		dataCap[1];
}capBuffer;
*/
typedef struct{
	int					capTypeCode;
	int					capLength;
	unsigned char		dataCap[1];
}capBufferApi522;
/*
typedef struct {
#ifdef __MCMS_ANNEX_C__	
	unsigned long				bUseAAL5;
	unsigned long				maxNTUSize;
	unsigned long				trafficType;
	INT8				transportType;
	INT8				filler1[3];
#endif
	cap_fd_set			altMatrix;   
	INT8				numberOfCaps;
	INT8				numberOfAlts;
	INT8				numberOfSim;
	INT8				filler2;
	capBuffer			caps;
}ctCapabilitiesStruct;

typedef struct {
	cap_fd_setApi522	altMatrix;   
	long				numberOfCaps;
	long				numberOfAlts;
	long				numberOfSim;
	capBuffer			caps;
}ctCapabilitiesStructApi522;

typedef struct {
#ifdef __MCMS_ANNEX_C__
	unsigned long				bUseAAL5;
	unsigned long				maxNTUSize;
	unsigned long				trafficType;
	INT8				transportType;
	INT8				filler1[3];
#endif
	cap_fd_set			altMatrix;   
	INT8				numberOfCaps;
	INT8				numberOfAlts;
	INT8				numberOfSim;
	INT8				filler2;
}ctCapabilitiesBasicStruct;

typedef struct {
	cap_fd_setApi522	altMatrix;   
	long				numberOfCaps;
	long				numberOfAlts;
	long				numberOfSim;
}ctCapabilitiesBasicStructApi522;

//---------------------------------

typedef enum {
  cmCapEmpty=0,
  cmCapAudio=1,
  cmCapVideo=2,
  cmCapData=3,
  cmCapNonStandard=4,
  cmCapUserInput=5,
  cmCapConference=6,
  cmCapH235=7,
  cmCapMaxPendingReplacementFor=8,
  cmCapGeneric=9,
  cmCapMultiplexedStream=10,
  cmCapAudioTelephonyEvent=11,
  cmCapAudioTone=12
} cmCapDataType;

typedef struct {
  UINT8	direction;
  UINT8 type;			//cmCapDataType
  UINT8 roleLabel;
  UINT8	capTypeCode;
} ctCapStruct;
*/
typedef struct {
  char *name;
  long capabilityId;   // capabilityTableEntryNumber 
  int capabilityHandle; // capability item message tree (video/audio/data/nonStandard) 
  unsigned long direction;
  unsigned long type;
  unsigned long roleLabel;
  int	 nameEnum;	
} cmCapStructApi522;

typedef cmCapStructApi522	capStructHeader;
/*

// MCMS use.
typedef struct{
	ctCapStruct		header;
}BaseCapStruct;

typedef struct{
	capStructHeader			header;
}BaseCapStructApi522;


// --------------------------------------------------------------------- 
// ----------------------- Audio caps structures ----------------------- 
// --------------------------------------------------------------------- 


//Audio capabilities algorithems :
//--------------------------------
//	nonStandard, g711Alaw64k, g711Alaw56k, g711Ulaw64k,	g711Ulaw56k,
//	g722-64k, g722-56k,	g722-48k, g7231, g728, g729, g729AnnexA,
//	is11172AudioCapability, is13818AudioCapability, g729wAnnexB,
//	g729AnnexAwAnnexB, g7231AnnexCCapability.

//Video capabilities algorithems :
//--------------------------------
//	nonStandard, h261VideoMode,	h262VideoMode, h263VideoMode, is11172VideoMode,
//  GenericVideo (H26L).
	
//Data capabilities algorithems :
//--------------------------------
//	nonStandard, t120,	dsm_cc, userData, t434, h224, h222DataPartitioning,
//	t30fax, t84, nlpid, dsvdControl.

*/
typedef struct{
	ctCapStruct			header;
	long				value;   
}simpleAudioCapStruct;
/*
typedef struct{
	capStructHeader		header;
	int					value;   
}simpleAudioCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g711Alaw64k
}g711Alaw64kCapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g711Alaw64kCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g711Alaw56k
}g711Alaw56kCapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g711Alaw56kCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g711Ulaw64k
}g711Ulaw64kCapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g711Ulaw64kCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g711Ulaw56k
}g711Ulaw56kCapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g711Ulaw56kCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g722-64k
}g722_64kCapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g722_64kCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g722-56k
}g722_56kCapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g722_56kCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g722-48k
}g722_48kCapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g722_48kCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g728
}g728CapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g728CapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g729
}g729CapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g729CapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g729AnnexA
}g729AnnexACapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g729AnnexACapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g729wAnnexB
}g729wAnnexBCapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g729wAnnexBCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   // path = audioData.g729AnnexAwAnnexB
}g729AnnexAwAnnexBCapStruct;

typedef struct{
	capStructHeader		header;
	int					value;   
}g729AnnexAwAnnexBCapStructApi522;

typedef struct{
	ctCapStruct			header;
	INT16				maxAl_sduAudioFrames;	// INTEGER (1..256)
    INT16				capBoolMask;

#	define				g7231_silenceSuppression		0x8000

	// path = audioData.g7231.(element name)
}g7231CapStruct;

typedef struct{
	capStructHeader		header;
	int					maxAl_sduAudioFrames;
	BOOL				silenceSuppression;
}g7231CapStructApi522;

typedef struct{
	ctCapStruct			header;
	INT16				bitRate;				// (1..448) -- units kbit/s

    INT16				capBoolMask;

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
	capStructHeader		header;
	BOOL				audioLayer1;
	BOOL				audioLayer2;
	BOOL				audioLayer3;
	BOOL				audioSampling32k;
	BOOL				audioSampling44k1;
	BOOL				audioSampling48k;
	BOOL				singleChannel;
	BOOL				twoChannels;
	int					bitRate;
}IS11172AudioCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				bitRate;				// INTEGER (1..1130) -- units kbit/s

    long				capBoolMask;

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
	capStructHeader		header;
	BOOL				audioLayer1;
	BOOL				audioLayer2;
	BOOL				audioLayer3;
	BOOL				audioSampling16k;
	BOOL				audioSampling22k05;
	BOOL				audioSampling24k;
	BOOL				audioSampling32k;
	BOOL				audioSampling44k1;
	BOOL				audioSampling48k;
	BOOL				singleChannel;
	BOOL				twoChannels;
	BOOL				threeChannels2_1;  
	BOOL				threeChannels3_0;
	BOOL				fourChannels2_0_2_0;
	BOOL				fourChannels2_2;
	BOOL				fourChannels3_1;
	BOOL				fiveChannels3_0_2_0;
	BOOL				fiveChannels3_2;
	BOOL				lowFrequencyEnhancement;
	BOOL				multilingual;
	long				bitRate;
}IS13818CapStructApi522;
typedef struct{
	ctCapStruct			header;
	INT8				maxAl_sduAudioFrames;	// INTEGER (1..256)	
	INT8				highRateMode0;			// INTEGER (27..78) -- units octets
	INT8				highRateMode1;			// INTEGER (27..78) -- units octets
	INT8				lowRateMode0;			// INTEGER (23..66) -- units octets
	INT8				lowRateMode1;			// INTEGER (23..66) -- units octets
	INT8				sidMode0;				// INTEGER (6..17)  -- units octets
	INT8				sidMode1;				// INTEGER (6..17)  -- units octets

    INT8				capBoolMask;
#	define				G7231Annex_silenceSuppression	0x80
	
	//path=audioData.G7231AnnexCCapability.g723AnnexCAudioMode.(element name)
}G7231AnnexCapStruct;

typedef struct{
	capStructHeader		header;
	int					maxAl_sduAudioFrames;	
	BOOL				silenceSuppression;		
	int					highRateMode0;			
	int					highRateMode1;			
	int					lowRateMode0;			
	int					lowRateMode1;			
	int					sidMode0;				
	int					sidMode1;					
}G7231AnnexCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   
}g7221_32kCapStruct;

typedef struct{
	capStructHeader		header;
	long				value;   
}g7221_32kCapStructApi522;

typedef struct{
	ctCapStruct			header;
	long				value;   
}g7221_24kCapStruct;

typedef struct{
	capStructHeader		header;
	long				value;   
}g7221_24kCapStructApi522;

//Only for new--------------------
typedef struct{
	ctCapStruct			header;
	long				value;   
}siren14_48kCapStruct;

typedef struct{
	ctCapStruct			header;
	long				value;   
}siren14_32kCapStruct;

typedef struct{
	ctCapStruct			header;
	long				value;   
}siren14_24kCapStruct;
//--------------------------------

// ---------------------------------------------------------------------
// ----------------------- Video caps structures -----------------------
// ---------------------------------------------------------------------

typedef struct{
	ctCapStruct			header;

	INT16				maxBitRate;			// INTEGER (1..19200),		-- units of 100 bit/s
	INT8				qcifMPI;			// INTEGER (1..4) OPTIONAL, -- units 1/29.97 Hz
	INT8				cifMPI;				// INTEGER (1..4) OPTIONAL, -- units 1/29.97 Hz

    long				capBoolMask;
#	define				h261_temporalSpatialTradeOffCapability	0x80000000
#	define				h261_stillImageTransmission				0x40000000

	// path = videoData.h261VideoCapability.(element name)
}h261CapStruct;

typedef struct{
	capStructHeader		header;

	int					qcifMPI;
	int					cifMPI;
	BOOL				temporalSpatialTradeOffCapability;
	int					maxBitRate;
	BOOL				stillImageTransmission;
}h261CapStructApi522;

typedef struct{
	ctCapStruct			header;

	long				videoBitRate;		// INTEGER (0.. 1073741823) -- units 400 bit/s
	long				vbvBufferSize;		// INTEGER (0.. 262143)		-- units 16 384 bits
	long				LuminanceSampleRate;// INTEGER (0..4294967295)  -- units samples/s
	INT16				samplesPerLine;		// INTEGER (0..16383)		-- units samples/line
	INT16				linesPerFrame;		// INTEGER (0..16383)		-- units lines/frame
	INT8				framesPerSecond;	// INTEGER (0..15)			-- frame_rate_code
	INT8				filler;

    INT16				capBoolMask;

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
	capStructHeader		header;

	BOOL				profileAndLevel_SPatML;
	BOOL				profileAndLevel_MPatLL;
	BOOL				profileAndLevel_MPatML;
	BOOL				profileAndLevel_MPatH_14;
	BOOL				profileAndLevel_MPatHL;
	BOOL				profileAndLevel_SNRatLL;
	BOOL				profileAndLevel_SNRatML;
	BOOL				profileAndLevel_SpatialatH_14;
	BOOL				profileAndLevel_HPatML;
	BOOL				profileAndLevel_HPatH_14;
	BOOL				profileAndLevel_HPatHL;
	int					videoBitRate;
	int					vbvBufferSize;
	int					samplesPerLine;
	int					linesPerFrame;
	int					framesPerSecond;
	int					LuminanceSampleRate;
}h262CapStructApi522;
*/
typedef struct{
	BOOL	separateVideoBackChannel;
	BOOL	videoBadMBsCap;
}annexesCommonSt;
/*
typedef struct{
	ctCapStruct			header;

	long				maxBitRate;			// INTEGER (1..192400)		-- units 100 bit/s
	long				hrd_B;				// INTEGER (0..524287)		-- units 128 bits
	INT16				bppMaxKb;			// INTEGER (0..65535)		-- units 1024 bits
	INT16				slowSqcifMPI;		// INTEGER (1..3600)		-- units seconds/frame
	INT16				slowQcifMPI;		// INTEGER (1..3600)		-- units seconds/frame
	INT16				slowCifMPI;			// INTEGER (1..3600)		-- units seconds/frame
	INT16				slowCif4MPI;		// INTEGER (1..3600)		-- units seconds/frame
	INT16				slowCif16MPI;		// INTEGER (1..3600)		-- units seconds/frame
	INT8				sqcifMPI;			// INTEGER (1..32)			-- units 1/29.97 Hz
	INT8				qcifMPI;			// INTEGER (1..32)			-- units 1/29.97 Hz
	INT8				cifMPI;				// INTEGER (1..32)			-- units 1/29.97 Hz
	INT8				cif4MPI;			// INTEGER (1..32)			-- units 1/29.97 Hz
	INT8				cif16MPI;			// INTEGER (1..32)			-- units 1/29.97 Hz	
	INT8				filler;

    INT16				capBoolMask;

#	define				h263_unrestrictedVector					0x8000
#	define				h263_arithmeticCoding					0x4000
#	define				h263_advancedPrediction					0x2000
#	define				h263_pbFrames							0x1000
#	define				h263_temporalSpatialTradeOffCapability	0x0800
#	define				h263_errorCompensation					0x0400

#	define				h263Options_separateVideoBackChannel	0x0200
#	define				h263Options_videoBadMBsCap				0x0100

	annexes_fd_set		annexesMask;
//	annexesCommonSt		annexesCommon;

}h263CapStructBase;
*/
typedef struct{
	capStructHeader		header;

	int					sqcifMPI;
	int					qcifMPI;
	int					cifMPI;
	int					cif4MPI;
	int					cif16MPI;
	int					maxBitRate;
	BOOL				unrestrictedVector;
	BOOL				arithmeticCoding;	
	BOOL				advancedPrediction;
	BOOL				pbFrames;	
	BOOL				temporalSpatialTradeOffCapability;	
	int					hrd_B;
	int					bppMaxKb;
	int					slowSqcifMPI;
	int					slowQcifMPI;
	int					slowCifMPI;
	int					slowCif4MPI;
	int					slowCif16MPI;
	BOOL				errorCompensation;
	annexes_fd_set		annexesMask;
	annexesCommonSt		annexesCommon;
} h263CapStructBaseApi522;
/*
typedef struct{
	ctCapStruct			header;

	long				maxBitRate;
	long				hrd_B;
	INT16				bppMaxKb;
	INT16				slowSqcifMPI;
	INT16				slowQcifMPI;
	INT16				slowCifMPI;
	INT16				slowCif4MPI;
	INT16				slowCif16MPI;
	INT8				sqcifMPI;
	INT8				qcifMPI;
	INT8				cifMPI;
	INT8				cif4MPI;
	INT8				cif16MPI;
	INT8				filler;

    INT16				capBoolMask;

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
//	annexesCommonSt		annexesCommon;
	char				annexesPtr[1]; //sequece of h263OptionsStruct;
}h263CapStruct;
*/
typedef struct{
	capStructHeader		header;

	int					sqcifMPI;
	int					qcifMPI;
	int					cifMPI;
	int					cif4MPI;
	int					cif16MPI;
	int					maxBitRate;
	BOOL				unrestrictedVector;
	BOOL				arithmeticCoding;	
	BOOL				advancedPrediction;
	BOOL				pbFrames;	
	BOOL				temporalSpatialTradeOffCapability;	
	int					hrd_B;
	int					bppMaxKb;
	int					slowSqcifMPI;
	int					slowQcifMPI;
	int					slowCifMPI;
	int					slowCif4MPI;
	int					slowCif16MPI;
	BOOL				errorCompensation;
	annexes_fd_set		annexesMask;
	annexesCommonSt		annexesCommon;
	char				annexesPtr[1]; //sequece of h263OptionsStruct;
}h263CapStructApi522;
/*
//  CUSTOM PICTURE FORMAT
// =======================

typedef struct{
	UINT8				sqcifAdditionalPictureMemory;	// INTEGER (1..256) -- units frame
	UINT8				qcifAdditionalPictureMemory;	// INTEGER (1..256) -- units frame
	UINT8				cifAdditionalPictureMemory;		// INTEGER (1..256) -- units frame
	UINT8				cif4AdditionalPictureMemory;	// INTEGER (1..256) -- units frame
	UINT8				cif16AdditionalPictureMemory;	// INTEGER (1..256) -- units frame
	UINT8				bigCpfAdditionalPictureMemory;	// INTEGER (1..256) -- units frame

} additionalPictureMemoryStruct; 
*/
typedef struct{
	long				sqcifAdditionalPictureMemory;	
	long				qcifAdditionalPictureMemory;	
	long				cifAdditionalPictureMemory;		
	long				cif4AdditionalPictureMemory;	
	long				cif16AdditionalPictureMemory;	
	long				bigCpfAdditionalPictureMemory;	
} additionalPictureMemoryStructApi522; 
/*
typedef struct{															
	UINT16        		customMPI;						// INTEGER (1..2048)
	UINT16        		clockConversionCode;			// INTEGER (1000..1001)
	UINT8	       		clockDivisor;					// INTEGER (1..127)
}customPCFSt;
*/
typedef struct{
	long        		clockConversionCode;
	long       		clockDivisor;
	long        		customMPI;
}customPCFStApi522;

/*
typedef struct{
	UINT16				maxCustomPictureWidth;			// INTEGER (1..2048) -- units 4 pixels
	UINT16				maxCustomPictureHeight;			// INTEGER (1..2048) -- units 4 pixels
	UINT16				minCustomPictureWidth;			// INTEGER (1..2048) -- units 4 pixels
	UINT16				minCustomPictureHeight;			// INTEGER (1..2048) -- units 4 pixels

	//mPIStruct
		//customPCFSt
		UINT16        	customMPI;						// INTEGER (1..2048)
		UINT16        	clockConversionCode;			// INTEGER (1000..1001)
		UINT8	       	clockDivisor;					// INTEGER (1..127)	
		//------------------------------------------
	UINT8				standardMPI;				// INTEGER (1..31)
	//---------------------------------------
	//pixelAspectInformationStruct
	UINT8				pixelAspectCode[1]; //  INTEGER(1..14) - According ASN1, the array can be bigger.
	//-----------------------------------------
	UINT8				filler;

}customPicFormatSt;
*/
typedef struct{
	long				maxCustomPictureWidth;			
	long				maxCustomPictureHeight;			
	long				minCustomPictureWidth;			
	long				minCustomPictureHeight;			
	//pixelAspectInformation CHOICE
	struct mPIStruct{
		long					standardMPI;	
		customPCFStApi522		customPCF;
	}mPI;
	
	struct pixelAspectInformationStruct{
		//BOOL		anyPixelAspectRatio;
		unsigned long		pixelAspectCode[1]; // According ASN1, the array can be bigger.
		//struct	TBD extendPAR;
		
	}pixelAspectInformation;
	
} customPicFormatStApi522;
/*
typedef struct{
		INT8	clockConversionCode;					// INTEGER (1000..1001)
		INT8	clockDivisor;							// INTEGER (1..127)
		INT16	sqcifMPI;								// INTEGER (1..2048) OPTIONAL
		INT16	qcifMPI;								// INTEGER (1..2048) OPTIONAL
		INT16	cifMPI;									// INTEGER (1..2048) OPTIONAL
		INT16	cif4MPI;								// INTEGER (1..2048) OPTIONAL
		INT16	cif16MPI;								// INTEGER (1..2048) OPTIONAL
}customPicClockFrequencySt;
*/
typedef struct{
	long	clockConversionCode;
	long	clockDivisor;
	long	sqcifMPI;
	long	qcifMPI;
	long	cifMPI;
	long	cif4MPI;
	long	cif16MPI;
}customPicClockFrequencyStApi522;
/*
typedef struct{
	customPicClockFrequencySt	customPictureClockFrequency;
	unsigned long						numberOfCustomPic;
}customPic_StBase;
*/
typedef struct{
	customPicClockFrequencyStApi522	customPictureClockFrequency; // 7 longs
	unsigned long							numberOfCustomPic;
}customPic_StBaseApi522;
/*
typedef struct{
	customPicClockFrequencySt	customPictureClockFrequency;
	unsigned long						numberOfCustomPic;
	char						customPicPtr[1]; //sequece of customPic_St;
}customPic_St;
*/
typedef struct{
	customPicClockFrequencyStApi522	customPictureClockFrequency; // 7 longs
	unsigned long							numberOfCustomPic;
	char							customPicPtr[1]; //sequece of customPic_St;
}customPic_StApi522;
/*
//  ANNEXES
// =========

typedef enum{///!!!!!!!!!!!!!!!!!
	VBnone,
	VBackMessageOnly,
	VBnackMessageOnly,
	VBackOrNackMessageOnly,
	VBackAndNackMessage,	

} videoBackChannelSendEnum;

typedef struct{
	additionalPictureMemoryStruct	additionalPictureMemory;
	UINT8							videoBackChannelSend;				// has value one from videoBackChannelSendEnum
	UINT8							mpuHorizMBs;						// INTEGER (1..128)
	UINT8							mpuVertMBs;							// INTEGER (1..72)
	UINT8							annexBoolMask;
#	define							refPic_videoMux						0x80

} refPictureSelectionStruct;
*/
typedef struct{
	additionalPictureMemoryStructApi522	additionalPictureMemory;
	BOOL								videoMux;
	unsigned long								videoBackChannelSend;	// has value one from videoBackChannelSendEnum
	long								mpuHorizMBs;
	long								mpuVertMBs;
	
} refPictureSelectionStructApi522;
/*
typedef struct{
	UINT8	dummy;
	// H263Options is not used.
}annexBEFGHO_St;
*/
typedef struct{
	unsigned long	dummy;
	// H263Options is not used.
}annexBEFGHO_StApi522;
/*
typedef struct{
	UINT8	annexBoolMask;								
#	define	annexD_unlimitedMotionVectors				0x80
}annexD_St;												
*/
typedef struct{
	BOOL	unlimitedMotionVectors;	
}annexD_StApi522;
/*														
typedef struct{											
	UINT8	annexBoolMask;								
#	define	annexI_advancedIntraCodingMode				0x80
}annexI_St;												
*/	
typedef struct{
	BOOL	advancedIntraCodingMode;
}annexI_StApi522;
/*													
typedef struct{											
	UINT8	annexBoolMask;								
#	define	annexJ_deblockingFilterMode					0x80
}annexJ_St;
*/
typedef struct{
	BOOL	deblockingFilterMode;
}annexJ_StApi522;
/*
typedef struct{
	UINT8	annexBoolMask;
#	define	annexK_slicesInOrder_NonRect				0x80
#	define	annexK_slicesInOrder_Rect					0x40
#	define	annexK_slicesNoOrder_NonRect				0x20
#	define	annexK_slicesNoOrder_Rect					0x10
}annexK_St;												
*/	
typedef struct{
	BOOL	slicesInOrder_NonRect;
	BOOL	slicesInOrder_Rect;
	BOOL	slicesNoOrder_NonRect;
	BOOL	slicesNoOrder_Rect;
}annexK_StApi522;
/*													
typedef struct{											
	UINT8	annexBoolMask;								
#	define	annexL_fullPictureFreeze					0x80
#	define	annexL_partialPictureFreezeAndRelease		0x40
#	define	annexL_resizingPartPicFreezeAndRelease		0x20
#	define	annexL_fullPictureSnapshot					0x10
#	define	annexL_partialPictureSnapshot				0x08
#	define	annexL_videoSegmentTagging					0x04
#	define	annexL_progressiveRefinement				0x02
	//transparencyParameters	TransparencyParameters OPTIONAL,
}annexL_St;
*/
typedef struct{
	BOOL	fullPictureFreeze;
	BOOL	partialPictureFreezeAndRelease;
	BOOL	resizingPartPicFreezeAndRelease;
	BOOL	fullPictureSnapshot;
	BOOL	partialPictureSnapshot;
	BOOL	videoSegmentTagging;
	BOOL	progressiveRefinement;
	//transparencyParameters	TransparencyParameters OPTIONAL,
}annexL_StApi522;
/*
typedef struct{
	UINT8	annexBoolMask;
#	define	annexM_improvedPBFramesMode					0x80
}annexM_St;
*/
typedef struct{
	BOOL	improvedPBFramesMode;
}annexM_StApi522;
/*
typedef struct{
	refPictureSelectionStruct	refPictureSelection; // Byte*10
	UINT16						filler;
}annexN_St;
*/
typedef struct{
	refPictureSelectionStructApi522	refPictureSelection; // long*10
}annexN_StApi522;
/*
typedef struct{
	UINT8	annexBoolMask;
#	define	annexP_dynamicPictureResizingByFour			0x80
#	define	annexP_dynamicPictureResizingSixteenthPel	0x40
#	define	annexP_dynamicWarpingHalfPel				0x20
#	define	annexP_dynamicWarpingSixteenthPel			0x10
}annexP_St;
*/
typedef struct{
	BOOL	dynamicPictureResizingByFour;
	BOOL	dynamicPictureResizingSixteenthPel;
	BOOL	dynamicWarpingHalfPel;
	BOOL	dynamicWarpingSixteenthPel;
}annexP_StApi522;
/*
typedef struct{
	UINT8	annexBoolMask;
#	define	annexQ_reducedResolutionUpdate				0x80
}annexQ_St;
*/
typedef struct{
	BOOL	reducedResolutionUpdate; 
}annexQ_StApi522;
/*
typedef struct{
	UINT8	annexBoolMask;
#	define	annexR_independentSegmentDecoding			0x80
}annexR_St;
*/
typedef struct{
	BOOL	independentSegmentDecoding;
}annexR_StApi522;
/*
typedef struct{
	UINT8	annexBoolMask;
#	define	annexS_alternateInterVLCMode				0x80
}annexS_St;
*/
typedef struct{
	BOOL	alternateInterVLCMode;
}annexS_StApi522;
/*
typedef struct{
	UINT8	annexBoolMask;
#	define	annexT_modifiedQuantizationMode				0x80
}annexT_St;
*/
typedef struct{
	BOOL	modifiedQuantizationMode;
}annexT_StApi522;
/*
typedef struct{
	refPictureSelectionStruct	refPictureSelection; // Byte*10
}annexU_St;
*/
typedef struct{
	refPictureSelectionStructApi522	refPictureSelection; // long*8
}annexU_StApi522;
/*
typedef struct{
	UINT8	dummy;
	//H263Version3Option		h263Version3Option;	
}annexV_St;
*/
typedef struct{
	unsigned long	dummy;
	//H263Version3Option		h263Version3Option;	
}annexV_StApi522;
/*
typedef struct{
	//H263Version3Option
	UINT8	annexBoolMask;
#	define	annexW_dataPartitionedSlices				0x80
#	define	annexW_fixedPointIDCT0						0x40
#	define	annexW_interlacedFields						0x20
#	define	annexW_currentPictureHeaderRepetition		0x10
#	define	annexW_previousPictureHeaderRepetition		0x08
#	define	annexW_nextPictureHeaderRepetition			0x04
#	define	annexW_pictureNumber						0x02
#	define	annexW_spareReferencePictures				0x01
}annexW_St;
*/
typedef struct{
	unsigned long	dummy;
	//H263Version3Option		h263Version3Option;	
}annexW_StApi522;
/*
typedef union {

	//Note: annexes struct's have UINT8 elements only, thus they all need same endian treatment.
	annexBEFGHO_St		annexB;		
	annexD_St			annexD;		
	annexBEFGHO_St		annexE;		
	annexBEFGHO_St		annexF;		
	annexBEFGHO_St		annexG;		
	annexBEFGHO_St		annexH;		
	annexI_St			annexI;		
	annexJ_St			annexJ;		
	annexK_St			annexK;	
	annexL_St			annexL;		
	annexM_St			annexM;		
	annexN_St			annexN;	// Size is 10 Byte (biggest) + 2 = 12.
	annexBEFGHO_St		annexO;
	annexP_St			annexP;
	annexQ_St			annexQ;		
	annexR_St			annexR;		
	annexS_St			annexS;		
	annexT_St			annexT;		
	annexU_St			annexU;	
	annexV_St			annexV;	
	annexW_St			annexW;	

} h263OptionsStruct;
*/
typedef union {
	//Note: annexes struct's have UINT elements only, thus they all need same endian treatment.
	annexBEFGHO_StApi522	annexB;		
	annexD_StApi522			annexD;		
	annexBEFGHO_StApi522	annexE;		
	annexBEFGHO_StApi522	annexF;		
	annexBEFGHO_StApi522	annexG;		
	annexBEFGHO_StApi522	annexH;		
	annexI_StApi522			annexI;		
	annexJ_StApi522			annexJ;		
	annexK_StApi522			annexK;	
	annexL_StApi522			annexL;		
	annexM_StApi522			annexM;		
	annexN_StApi522			annexN;	// Size is 10 longs(biggest).
	annexBEFGHO_StApi522	annexO;
	annexP_StApi522			annexP;
	annexQ_StApi522			annexQ;		
	annexR_StApi522			annexR;		
	annexS_StApi522			annexS;		
	annexT_StApi522			annexT;		
	annexU_StApi522			annexU;	
	annexV_StApi522			annexV;	
	annexW_StApi522			annexW;	
} h263OptionsStructApi522;

/*
typedef struct{
	ctCapStruct			header;
	long				customMaxMbpsValue; 
	long				customMaxFsValue; 
	long				customMaxDpbValue; 
	long				customMaxBrAndCpbValue; 

	INT16				maxBitRate;     // INTEGER (1..19200), -- units of 100 bit/s

	UINT8				profileValue;   // H264_Profile_BaseLine
	UINT8				levelValue; 
}h264CapStruct;


#define CT_GenricVideo_Data_Len		4

typedef enum{
	H26LCode,
	DropField,
	UnknownGenericCode,		// last one.
} genericCodeEnum;

// MCMS use.
static char* GenericCodeArray[] = { // Matching to "genericCodeEnum" order !	
	"H26LCode",
	"DropField",
	"UnknownGenericCode",
};

***********Api 522***********
// Generic (H26L) structure 
typedef enum{
	kNoPatameter = 0,		//0
	kCollapsing,			//1
	kNonCollapsing,			//2
	kNonCollapsingRaw,		//3
	ktransport				//4
} EParameterType;

typedef struct{ 
	unsigned long						t35CountryCode;			
	unsigned long						t35Extension;			
	unsigned long						manufacturerCode;
}h221NonStandSt;

typedef struct {
	h221NonStandSt	h221NonStand;
	char			data[Size2]; //[0] 9E
} h221NonStandParam;

typedef struct{
	h221NonStandParam			h221NoneStand;
	char						octetString[Mcms_NonStandard_Data_Size];
	long						parameterValue;
}collapsingParameter;

typedef struct{
	h221NonStandParam			h221NoneStand;
	char						octetString[Mcms_NonStandard_Data_Size];
	long						parameterValue;
}nonCollapsingParameter;
***************

typedef struct{
	ctCapStruct			header;
	long				genericCodeType; // genericCodeEnum
	long				maxBitRate;
	char				data[CT_GenricVideo_Data_Len];
}genericVideoCapStruct;

typedef struct{
	capStructHeader			header;

	h221NonStandParam		h221NoneStand;
	long					maxBitRate;

	EParameterType			ParameterType;
	union{
		collapsingParameter		pCollapsing;
		nonCollapsingParameter	pNonCollapsimg;
	}Parameter;
}genericVideoCapStructApi522;

typedef struct{
	ctCapStruct			header;

	long				videoBitRate;			// INTEGER(0..1073741823) -- units 400 bit/s
	long				luminanceSampleRate;	// INTEGER(0..4294967295) -- units samples/s
	long				vbvBufferSize;			// INTEGER(0..262143)	  -- units 16 384 bits
	INT16				samplesPerLine;			// INTEGER(0..16383)	  -- units samples/line
	INT16				linesPerFrame;			// INTEGER(0..16383)	  -- units samples/line
	INT8				pictureRate;			// INTEGER(0..15)
	INT8				filler[2];

    INT8				capBoolMask;

#	define				IS11172Video_constrainedBitstream		0x80

	// path = videoData.IS11172VideoCapability.(element name)
}IS11172VideoCapStruct;


// Generic :
//----------

typedef struct{
	ctCapStruct			header;
	long				value;   
}genericCapStruct;


// Control : 
//----------

typedef struct{
	ctCapStruct			header;
	long				value;   
}GenericControlBaseCapStruct;

typedef struct{
	ctCapStruct			header;
	long				value;   
}PeopleAndContentCapStruct;

typedef struct{
	ctCapStruct			header;
	long				value;   
}RoleLabelCapStruct;

// ---------------------------------------------------------------------- 
// -------------------- Non-Standard caps structures -------------------- 
// ---------------------------------------------------------------------- 

typedef struct {
// 1. object id 
//  char		object[128]; 	
	unsigned long	objectLength;									//4
// 2. h.221 id 
  UINT8		t35CountryCode;									//1
  UINT8		t35Extension;									//1
  UINT16	manufacturerCode;								//2
} ctNonStandardIdentifierSt;								//total = 8

typedef struct
{
    ctNonStandardIdentifierSt	info;
	char						data[CT_NonStandard_Data_Size];
} ctNonStandardParameterSt;

typedef struct{
	char						data[NonStandard_Data_Size];
	union{						
	   char						object[128];
	   h221NonStandSt			h221NonStandard;	
	}nonStandardIdentifier;	  
}nonStandardParameterStApi522; 

typedef struct{ 
	ctCapStruct					header;
	ctNonStandardParameterSt	nonStandardData;

	//path= {videoData/audioData/data}.NonStandardParameter...(element name)
}ctNonStandardCapStruct; 

typedef struct{
	capStructHeader					header;
	nonStandardParameterStApi522	nonStandard;

}nonStandardCapStructApi522; 


typedef struct{
	ctCapStruct			header;
	long				value;   
}ChairControlCapStruct;

// --------------------------------------------------------------------- 
// ----------------------- Data caps structures ------------------------ 
// --------------------------------------------------------------------- 

typedef struct{ 
		//nonStandardParameterSt	nonStandard;
	union{
		long					v14buffered;
		long					v42lapm;
		long					hdlcFrameTunnelling;
		long					h310SeparateVCStack;
		long					h310SingleVCStack;
		long					transparent;
		long					segmentationAndReassembly;
		long					hdlcFrameTunnelingwSAR;
		long					v120;
		long					separateLANStack;
		//union{ ...  }v76wCompression;
	}dataProtocolCapabilityChoice;

}dataProtocolCapabilitySt;

typedef struct{
	ctCapStruct					header;
	dataProtocolCapabilitySt	protocol;
	long						maxBitRate;
	
}t120DataCapStruct; // path = data.t120...(element name)

typedef struct{
	capStructHeader				header;
	dataProtocolCapabilitySt	protocol;
	long						maxBitRate;
	
}t120DataCapStructApi522;
 
typedef struct{
	ctCapStruct					header;
	dataProtocolCapabilitySt	protocol;
	
}h224DataCapStruct; // path = data.h224...(element name)

typedef struct{
	capStructHeader				header;
	dataProtocolCapabilitySt	protocol;
	
}h224DataCapStructApi522;
 

typedef struct{
	ctCapStruct			header;
	long				maxBitRate;
	
}dataCapStructBase;

//  Encryption Capability.
// ------------------------

typedef struct{
	ctCapStruct		header;
	UINT16			type;  //EencryMediaType
	UINT16			entry; //the entry to be encrypted.   
}encryptionCapStruct;

*/

typedef struct
{
   DWORD   ip_address;
   WORD    port;
}IpAddressPort;

#endif //_CONFPARTYH323DEFINES_H__



