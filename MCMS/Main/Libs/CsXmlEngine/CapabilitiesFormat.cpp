// CapabilitiesFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpCsSizeDefinitions.h"
#include "IpChannelParams.h"
#include "SrtpStructsAndDefinitions.h"
#include  "Capabilities.h"
// Macros
//--------

#undef	SRC
#define SRC	"CapFormat"

//	Static variables
//-------------------

// Global variables
//------------------





// External variables
//------------------

//	External Routines:
//--------------------
extern genXmlFormat xmlDynamicHeaderFormat[];
extern genXmlFormat xmlDynamicPropertiesFormat[];
extern genXmlFormat annexes_fd_setFormat[];
extern genXmlFormat ctNonStandardParameterStFormat[];
extern genXmlFormat annexBEFGHO_StFormat[];
extern genXmlFormat annexD_StFormat[];
extern genXmlFormat annexBEFGHO_StFormat[];
extern genXmlFormat annexBEFGHO_StFormat[];
extern genXmlFormat annexBEFGHO_StFormat[];
extern genXmlFormat annexBEFGHO_StFormat[];
extern genXmlFormat annexI_StFormat[];
extern genXmlFormat annexJ_StFormat[];
extern genXmlFormat annexK_StFormat[];
extern genXmlFormat annexL_StFormat[];
extern genXmlFormat annexM_StFormat[];
extern genXmlFormat annexN_StFormat[];
extern genXmlFormat annexBEFGHO_StFormat[];
extern genXmlFormat annexP_StFormat[];
extern genXmlFormat annexQ_StFormat[];
extern genXmlFormat annexR_StFormat[];
extern genXmlFormat annexS_StFormat[];
extern genXmlFormat annexT_StFormat[];
extern genXmlFormat annexU_StFormat[];
extern genXmlFormat annexV_StFormat[];
extern genXmlFormat annexW_StFormat[];
extern genXmlFormat customPic_StFormat[];


// Forward Declarations:
//----------------------


// Routines:
//----------


//typedef struct {
//	xmlDynamicHeader		xmlHeader;
//  APIU8	direction;
//  APIU8 type;
//  APIU8 roleLabel;
//  APIU8	capTypeCode;
//} ctCapStruct;

genXmlFormat ctCapStructFormat[] = {
	{parentElemType,	tagVarType,	"ctCapStruct",	5,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",	1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"direction",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"type",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"roleLabel",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"capTypeCode",	1,	0,0,0,0},
};
//-----------------------------------------------------------------------

// MCMS use.
//typedef struct{
//	ctCapStruct		header;
//}BaseCapStruct;
//
genXmlFormat BaseCapStructFormat[] = {
	{parentElemType,	tagVarType,	"BaseCapStruct",	1,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
};
//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS32				rtcpFeedbackMask;
//	APIS32				maxValue;  
//	APIS32				minValue; // must be the last
//}audioCapStructBase;

genXmlFormat audioCapStructBaseFormat[] = {
	{parentElemType,	tagVarType,	"audioCapStructBase",	4,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"rtcpFeedbackMask",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"minValue",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS32				rtcpFeedbackMask;
//	APIS32				maxValue;
//	APIS32				minValue;
//	APIU32				sirenLPRMask;
//}sirenLPR_CapStruct;

genXmlFormat SirenLPRCapStructFormat[] = {
	{parentElemType,	tagVarType,	"sirenLPR_CapStruct",	5,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
        {childElemType,		longVarType,	"rtcpFeedbackMask",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"minValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sirenLPRMask",	0,	0,0,0,0},
};

//-----------------------------------------------------------------------;


//typedef struct{
//	ctCapStruct			header;
    //    APIS32				rtcpFeedbackMask;
//	APIS32				maxValue;
//	APIS32				minValue; // must be the last
    //    APIU32				capBoolMask;

//#	define				g7221C_Mask_Rate48K		0x80000000
//#	define				g7221C_Mask_Rate32K		0x40000000
//#	define				g7221C_Mask_Rate24K		0x20000000

//}g7221C_CapStruct;

genXmlFormat G7221CCapStructFormat[] = {
	    {parentElemType,	tagVarType,	"g7221C_CapStruct",	5,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
        {childElemType,		longVarType,	"rtcpFeedbackMask",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"minValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"capBoolMask",	0,	0,0,0,0},
};
//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS16				maxAl_sduAudioFrames;	// INTEGER (1..256)
//  APIS16				capBoolMask;
//	APIS32				minAl_sduAudioFrames;	// INTEGER (1..256) // must be the last
//
//#	define				g7231_silenceSuppression		0x8000
//
//	// path = audioData.g7231.(element name)
//}g7231CapStruct;

genXmlFormat g7231CapStructFormat[] = {
	{parentElemType,	tagVarType,	"g7231CapStruct",	4,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		shortVarType,	"maxAl_sduAudioFrames",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"capBoolMask",				0,	0,0,0,0},
		{siblingElemType,	longVarType,	"minAl_sduAudioFrames",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS16				bitRate;				// (1..448) -- units kbit/s
//  APIS16				capBoolMask;
//
//#	define				IS11172Audio_audioLayer1		0x8000
//#	define				IS11172Audio_audioLayer2		0x4000
//#	define				IS11172Audio_audioLayer3		0x2000
//#	define				IS11172Audio_audioSampling32k	0x1000
//#	define				IS11172Audio_audioSampling44k1	0x0800
//#	define				IS11172Audio_audioSampling48k	0x0400
//#	define				IS11172Audio_singleChannel		0x0200
//#	define				IS11172Audio_twoChannels		0x0100
//
//	// path = audioData.IS11172AudioCapability.(element name)
//}IS11172AudioCapStruct;

genXmlFormat IS11172AudioCapStructFormat[] = {
	{parentElemType,	tagVarType,	"IS11172AudioCapStruct",	3,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		shortVarType,	"bitRate",			0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"capBoolMask",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------;


//typedef struct{
//	ctCapStruct			header;
//	APIS32				bitRate;				// INTEGER (1..1130) -- units kbit/s
//  APIS32				capBoolMask;
//
//#	define				IS13818_audioLayer1				0x80000000
//#	define				IS13818_audioLayer2				0x40000000
//#	define				IS13818_audioLayer3				0x20000000
//#	define				IS13818_audioSampling16k		0x10000000
//#	define				IS13818_audioSampling22k05		0x08000000
//#	define				IS13818_audioSampling24k		0x04000000
//#	define				IS13818_audioSampling32k		0x02000000
//#	define				IS13818_audioSampling44k1		0x01000000
//#	define				IS13818_audioSampling48k		0x00800000
//#	define				IS13818_singleChannel			0x00400000
//#	define				IS13818_twoChannels				0x00200000
//#	define				IS13818_threeChannels2_1		0x00100000
//#	define				IS13818_threeChannels3_0		0x00080000
//#	define				IS13818_fourChannels2_0_2_0		0x00040000
//#	define				IS13818_fourChannels2_2			0x00020000
//#	define				IS13818_fourChannels3_1			0x00010000
//#	define				IS13818_fiveChannels3_0_2_0		0x00008000
//#	define				IS13818_fiveChannels3_2			0x00004000
//#	define				IS13818_lowFrequencyEnhancement	0x00002000
//#	define				IS13818_multilingual			0x00001000
//
//	// To create path, the '_' must convert to '-'
//	// path = audioData.IS13818AudioCapability.(element name)
//}IS13818CapStruct;

genXmlFormat IS13818CapStructFormat[] = {
	{parentElemType,	tagVarType,	"IS13818CapStruct",	3,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"bitRate",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"capBoolMask",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS8				maxAl_sduAudioFrames;	// INTEGER (1..256)
//	APIS8				highRateMode0;			// INTEGER (27..78) -- units octets
//	APIS8				highRateMode1;			// INTEGER (27..78) -- units octets
//	APIS8				lowRateMode0;			// INTEGER (23..66) -- units octets
//	APIS8				lowRateMode1;			// INTEGER (23..66) -- units octets
//	APIS8				sidMode0;				// INTEGER (6..17)  -- units octets
//	APIS8				sidMode1;				// INTEGER (6..17)  -- units octets
//	APIS8				capBoolMask;
//#	define				G7231Annex_silenceSuppression	0x80
//
//	//path=audioData.G7231AnnexCCapability.g723AnnexCAudioMode.(element name)
//}G7231AnnexCapStruct;

genXmlFormat G7231AnnexCapStructFormat[] = {
	{parentElemType,	tagVarType,	"G7231AnnexCapStruct",	9,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		charVarType,	"maxAl_sduAudioFrames",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"highRateMode0",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"highRateMode1",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"lowRateMode0",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"lowRateMode1",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"sidMode0",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"sidMode1",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"capBoolMask",		0,	0,0,0,0},
};

// TIP --------------------------------------------------------------------------
//typedef struct {
//	ctCapStruct			header;
//	APIS32				rtcpFeedbackMask;
//	APIS32				maxValue;
//	APIS32				minValue;
//
//	APIS8				mimeType[MAX_AACLD_MIME_TYPE];	// mpeg4-generic
//	APIU32				sampleRate;			// 48000
//	APIU16				profileLevelId;			//=16;
//	APIU16				streamType;			//=5;
//	APIS8				mode[MAX_AACLD_MODE];		//=AAC-hbr;
//	APIS8				config[MAX_AACLD_CONFIG];	//=11B0 or B98C00;
//	APIU16				sizeLength;			//=13;
//	APIU16				indexLength;			//=3;
//	APIU16				indexDeltaLength;		//=3;
//	APIU32				constantDuration;		//=480;
//
//	APIU32 				maxBitRate;
//} AAC_LDCapStruct;
genXmlFormat AAC_LDCapStructFormat[] = {
	{parentElemType,	tagVarType,	"AAC_LDCapStruct",	15,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"rtcpFeedbackMask",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"minValue",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"mimeType",	0,	MAX_AACLD_MIME_TYPE,0,0,0},
		{siblingElemType,	longVarType,	"sampleRate",		0,	0,0,0,0},		
		{siblingElemType,	shortVarType,	"profileLevelId",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"streamType",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"mode",	0,	MAX_AACLD_MODE,0,0,0},		
		{siblingElemType,	charVarType,	"config",	0,	MAX_AACLD_CONFIG,0,0,0},
		{siblingElemType,	shortVarType,	"sizeLength",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"indexLength",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"indexDeltaLength",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"constantDuration",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxBitRate",		0,	0,0,0,0}

};

//-----------------------------------------------------------------------;

// Video :
//--------

//typedef struct{
//	ctCapStruct			header;
//	APIS32				maxBitRate;			// INTEGER (1..19200),		-- units of 100 bit/s
//	APIS8				qcifMPI;			// INTEGER (1..4) OPTIONAL, -- units 1/29.97 Hz
//	APIS8				cifMPI;				// INTEGER (1..4) OPTIONAL, -- units 1/29.97 Hz
//	APIS8				filler[2];
//   APIS32				capBoolMask;
//#	define				h261_temporalSpatialTradeOffCapability	0x80000000
//#	define				h261_stillImageTransmission				0x40000000
//
//	// path = videoData.h261VideoCapability.(element name)
//}h261CapStruct;

genXmlFormat h261CapStructFormat[] = {
	{parentElemType,	tagVarType,	"h261CapStruct",	7,	0,0,0,0},
		{parentElemType,	structVarType,		NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,		"maxBitRate",		0,0,0,0,0},
		{siblingElemType,	charVarType,		"qcifMPI",			0,0,0,0,0},
		{siblingElemType,	charVarType,		"cifMPI",			0,0,0,0,0},
		{siblingElemType,	charArrayVarType,	"filler",			0,2,0,0,0},
		{siblingElemType,	longVarType,		"capBoolMask",		0,0,0,0,0},
		{siblingElemType,	longVarType,		"rtcpFeedbackMask",	0,0,0,0,0},
};
//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS32				videoBitRate;		// INTEGER (0.. 1073741823) -- units 400 bit/s
//	APIS32				vbvBufferSize;		// INTEGER (0.. 262143)		-- units 16 384 bits
//	APIS32				LuminanceSampleRate;// INTEGER (0..4294967295)  -- units samples/s
//	APIS16				samplesPerLine;		// INTEGER (0..16383)		-- units samples/line
//	APIS16				linesPerFrame;		// INTEGER (0..16383)		-- units lines/frame
//	APIS8				framesPerSecond;	// INTEGER (0..15)			-- frame_rate_code
//	APIS8				filler;
//  APIS16				capBoolMask;
//	APIS16				filler;
//
//#	define				h262_profileAndLevel_SPatML			0x8000
//#	define				h262_profileAndLevel_MPatLL			0x4000
//#	define				h262_profileAndLevel_MPatML			0x2000
//#	define				h262_profileAndLevel_MPatH_14		0x1000
//#	define				h262_profileAndLevel_MPatHL			0x0800
//#	define				h262_profileAndLevel_SNRatLL		0x0400
//#	define				h262_profileAndLevel_SNRatML		0x0200
//#	define				h262_profileAndLevel_SpatialatH_14	0x0100
//#	define				h262_profileAndLevel_HPatML			0x0080
//#	define				h262_profileAndLevel_HPatH_14		0x0040
//#	define				h262_profileAndLevel_HPatHL			0x0020
//
//	// path = videoData.h262VideoCapability.(element name)
//}h262CapStruct;

genXmlFormat h262CapStructFormat[] = {
	{parentElemType,	tagVarType,	"h262CapStruct",	10,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,					1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"videoBitRate",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"vbvBufferSize",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"LuminanceSampleRate",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"samplesPerLine",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"linesPerFrame",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"framesPerSecond",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"filler",				0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"capBoolMask",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"rtcpFeedbackMask",		0,	0,0,0,0},

};
//-----------------------------------------------------------------------;
dynamicTableStruct annexDynamicTbl[] = {
	{typeAnnexB,	(int)&annexBEFGHO_StFormat},
	{typeAnnexD,	(int)&annexD_StFormat},
	{typeAnnexE,	(int)&annexBEFGHO_StFormat},
	{typeAnnexF,	(int)&annexBEFGHO_StFormat},
	{typeAnnexG,	(int)&annexBEFGHO_StFormat},
	{typeAnnexH,	(int)&annexBEFGHO_StFormat},
	{typeAnnexI,	(int)&annexI_StFormat},
	{typeAnnexJ,	(int)&annexJ_StFormat},
	{typeAnnexK,	(int)&annexK_StFormat},
	{typeAnnexL,	(int)&annexL_StFormat},
	{typeAnnexM,	(int)&annexM_StFormat},
	{typeAnnexN,	(int)&annexN_StFormat},
	{typeAnnexO,	(int)&annexBEFGHO_StFormat},
	{typeAnnexP,	(int)&annexP_StFormat},
	{typeAnnexQ,	(int)&annexQ_StFormat},
	{typeAnnexR,	(int)&annexR_StFormat},
	{typeAnnexS,	(int)&annexS_StFormat},
	{typeAnnexT,	(int)&annexT_StFormat},
	{typeAnnexU,	(int)&annexU_StFormat},
	{typeAnnexV,	(int)&annexV_StFormat},
	{typeAnnexW,	(int)&annexW_StFormat},
	{tblCustomFormat,(int)&customPic_StFormat},
};
//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS32				maxBitRate;
//	APIS32				hrd_B;
//	APIS16				bppMaxKb;
//	APIS16				slowSqcifMPI;
//	APIS16				slowQcifMPI;
//	APIS16				slowCifMPI;
//	APIS16				slowCif4MPI;
//	APIS16				slowCif16MPI;
//	APIS8				sqcifMPI;
//	APIS8				qcifMPI;
//	APIS8				cifMPI;
//	APIS8				cif4MPI;
//	APIS8				cif16MPI;
//	APIS8				filler;
//    APIS16				capBoolMask;
//
//#	define				h263_unrestrictedVector					0x8000
//#	define				h263_arithmeticCoding					0x4000
//#	define				h263_advancedPrediction					0x2000
//#	define				h263_pbFrames							0x1000
//#	define				h263_temporalSpatialTradeOffCapability	0x0800
//#	define				h263_errorCompensation					0x0400
//
//#	define				h263Options_separateVideoBackChannel	0x0200
//#	define				h263Options_videoBadMBsCap				0x0100
//
//	//enhncementLayerInfo	enhncementLayerInfo OPTIONAL
//	annexes_fd_set		annexesMask;
//	xmlDynamicProperties	xmlDynamicProps;
//	char				annexesPtr[1]; //sequece of h263OptionsStruct;
//}h263CapStruct;

genXmlFormat h263CapStructFormat[] = {
	{parentElemType,	tagVarType,	"h263CapStruct",	20,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"maxBitRate",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"hrd_B",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"rtcpFeedbackMask",	0,0,0,0,0},
		{siblingElemType,	shortVarType,	"bppMaxKb",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"slowSqcifMPI",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"slowQcifMPI",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"slowCifMPI",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"slowCif4MPI",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"slowCif16MPI",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"sqcifMPI",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"qcifMPI",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"cifMPI",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"cif4MPI",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"cif16MPI",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"filler",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"capBoolMask",	0,	0,0,0,0},
		{parentElemType,	structVarType,	"annexesMask",	1,(int)	&annexes_fd_setFormat,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType,	"annexesPtr",	NumOfElements(annexDynamicTbl),	(int) &annexDynamicTbl,0,0,0},	//Dynamic!!!!!!
//		{parentElemType,	structVarType,	"customPtr",	1,	(int) &customPic_StFormat},	//Dynamic!!!!!!
};
//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS32				customMaxMbpsValue; 
//	APIS32				customMaxFsValue; 
//	APIS32				customMaxDpbValue; 
//	APIS32				customMaxBrAndCpbValue;
//	APIS32				maxStaticMbpsValue;
//	APIS32				sampleAspectRatiosValue;
//
//	APIS32				maxBitRate;     // INTEGER (1..19200), -- units of 100 bit/s
//	APIS32				maxFR;
//	APIS32              		H264mode;
//	APIS32				rtcpFeedbackMask;
//	APIU16				profileValue;   // H264_Profile_BaseLine
//	APIU8				levelValue; 
//	APIU8				packetizationMode;
//	APIS16				filler;
//
//}h264CapStruct;

genXmlFormat h264CapStructFormat[] = {
	{parentElemType,	tagVarType,	"h264CapStruct",	15,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"customMaxMbpsValue",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"customMaxFsValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"customMaxDpbValue",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"customMaxBrAndCpbValue",0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxStaticMbpsValue",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sampleAspectRatiosValue",0,0,0,0,0},
		{siblingElemType,	longVarType,	"maxBitRate",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxFR",				0,	0,0,0,0},
		{siblingElemType,	longVarType,	"H264mode",				0,	0,0,0,0},
		{siblingElemType,	longVarType,	"rtcpFeedbackMask",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"profileValue",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"levelValue",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"packetizationMode", 	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"filler",				0,	0,0,0,0},
};


//===========================================================
//Sync Api
//typedef struct
//{
//	APIU32      	layerId;
//	APIU8 	        Tid; 		//Temporal id
//	APIU8       	Did; 		//Dependency id
//	APIU8       	Qid; 		//Quality id
//	APIU8       	Pid; 		//Priority id
//	APIU16   		profile;	//for sanity check??
//	APIU8   		level;	//for sanity check??
//	APIU32      	frameWidth;
//	APIU32      	frameHeight;//for sanity check??
//	APIU32   		frameRate;	//for sanity check??
//	APIU32       	maxBitRate;	//for sanity check??
//} VIDEO_OPERATION_POINT_S;
genXmlFormat videoOperationPointSFormat[] = {
	{parentElemType,	tagVarType,	"VIDEO_OPERATION_POINT_S",	11,	0,0,0,0},
		{childElemType,	longVarType,	"layerId",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"Tid",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"Did",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"Qid",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"Pid",			0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"profile",			0,	0,0,0,0},
		{siblingElemType,	charVarType,	"level",			0,	0,0,0,0},		
		{siblingElemType,	longVarType,	"frameWidth",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"frameHeight",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"frameRate",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxBitRate",0,	0,0,0,0},

};
////////////////////////////////////////////////////////////////////////////////////////////
//typedef struct
//{
//	APIU32      					operationPointSetId;
//	APIU8       					numberOfOperationPoints;
//	VIDEO_OPERATION_POINT_S  		tVideoOperationPoints[MAX_NUM_OPERATION_POINTS_PER_SET];
//}VIDEO_OPERATION_POINT_SET_S;
genXmlFormat videoOperationPointSetSFormat[] = {
	{parentElemType,	tagVarType,	"VIDEO_OPERATION_POINT_SET_S",	3,	0,0,0,0},
		{childElemType,	longVarType,	"operationPointSetId",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"numberOfOperationPoints",			0,	0,0,0,0},
    		{parentElemType,  structVarType,  "tVideoOperationPoints",  MAX_NUM_OPERATION_POINTS_PER_SET,  (int)&videoOperationPointSFormat, 0, 0, 0},
		

};


//===========================================================
//typedef struct{
//                ctCapStruct        header;
//                APIS32             width;
//                APIS32             height;
//                APIS32             aspectRatio;
//                APIS32             maxFrameRate;
//
//                APIS32             maxBitRate;     // INTEGER (1..19200), -- units of 100 bit/s
//				  APIS32             minBitRate;
//                APIS32             rtcpFeedbackMask;
//				  APIS32             maxPixelsNum;//width*height
//                APIU8              packetizationMode;
//                APIS8               filler;
//}msSvcCapStruct;

genXmlFormat msSvcCapStructFormat[] = {
	{parentElemType,	tagVarType,	"msSvcCapStruct",	11,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"width",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"height",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"aspectRatio",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxFrameRate",0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxBitRate",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"minBitRate",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"rtcpFeedbackMask",0,0,0,0,0},
		{siblingElemType,	longVarType,	"maxPixelsNum",0,0,0,0,0},
		{siblingElemType,   charVarType,    "packetizationMode",    0,  0,0,0,0},
		{siblingElemType,	charVarType,	"filler",				0,	0,0,0,0},
};
//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS32				genericCodeType; // genericCodeEnum
//	APIS32				maxBitRate;
//	char				data[CT_GenricVideo_Data_Len];
//}genericVideoCapStruct;

genXmlFormat genericVideoCapStructFormat[] = {
	{parentElemType,	tagVarType,	"genericVideoCapStruct",	4,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"genericCodeType",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxBitRate",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,"data",			0,	CT_GenricVideo_Data_Len,0,0,0},
};

//typedef struct{
//	ctCapStruct			header;
//
//	APIU32				versionID;
//	APIU32				minProtectionPeriod;
//	APIU32				maxProtectionPeriod;
//	APIU32				maxRecoverySet;
//	APIU32				maxRecoveryPackets;
//	APIU32				maxPacketSize;
//}lprCapStruct;

genXmlFormat lprCapStructFormat[] = {
	{parentElemType,	tagVarType,	"lprCapStruct",	7,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"versionID",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"minProtectionPeriod",	0,0,0,0,0},
		{siblingElemType,	longVarType,	"maxProtectionPeriod",	0,0,0,0,0},
		{siblingElemType,	longVarType,	"maxRecoverySet",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxRecoveryPackets",0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxPacketSize",	0,	0,0,0,0},
};

////////////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
genXmlFormat fecCapStructFormat[] = {
		{parentElemType,	tagVarType,	"fecCapStruct",	1,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,		1,(int)	&ctCapStructFormat,0,0,0},
};
genXmlFormat redCapStructFormat[] = {
		{parentElemType,	tagVarType,	"redCapStruct",	1,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,		1,(int)	&ctCapStructFormat,0,0,0},
};
////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//
//	APIS32				videoBitRate;			// INTEGER(0..1073741823) -- units 400 bit/s
//	APIS32				luminanceSampleRate;	// INTEGER(0..4294967295) -- units samples/s
//	APIS32				vbvBufferSize;			// INTEGER(0..262143)	  -- units 16 384 bits
//	APIS16				samplesPerLine;			// INTEGER(0..16383)	  -- units samples/line
//	APIS16				linesPerFrame;			// INTEGER(0..16383)	  -- units samples/line
//	APIS8				pictureRate;			// INTEGER(0..15)
//	APIS8				filler[2];
//
//    APIS8				capBoolMask;
//
//#	define				IS11172Video_constrainedBitstream		0x80
//
//	// path = videoData.IS11172VideoCapability.(element name)
//}IS11172VideoCapStruct;
//
genXmlFormat IS11172VideoCapStructFormat[] = {
	{parentElemType,	tagVarType,	"IS11172VideoCapStruct",	10,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"videoBitRate",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"luminanceSampleRate",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"vbvBufferSize",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"samplesPerLine",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"linesPerFrame",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"pictureRate",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,	"filler",			0,	2,0,0,0},
		{siblingElemType,	charVarType,	"capBoolMask",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"rtcpFeedbackMask",		0,0,0,0,0},
};
//-----------------------------------------------------------------------;

// NonStandard :
//-------------

//typedef struct{
//	ctCapStruct					header;
//	ctNonStandardParameterSt	nonStandardData;
//
//	//path= {videoData/audioData/data}.NonStandardParameter...(element name)
//}ctNonStandardCapStruct;

genXmlFormat ctNonStandardCapStructFormat[] = {
	{parentElemType,	tagVarType,	"ctNonStandardCapStruct",	2,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctNonStandardParameterStFormat,0,0,0},
};
//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS32				value;
//}ChairControlCapStruct;

genXmlFormat ChairControlCapStructFormat[] = {
	{parentElemType,	tagVarType,	"ChairControlCapStruct",	2,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"value",		0,	0,0,0,0},
};
//-----------------------------------------------------------------------;

// Data :
//-------

//typedef struct{
//	ctCapStruct			header;
//	INT32				maxBitRate;
//	APIS8               		mask;	//currently used for H224 Fecc and MrcScp Mask
//}dataCapStructBase;

genXmlFormat dataCapStructBaseFormat[] = {
	{parentElemType,	tagVarType,	"dataCapStructBase",	3,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"maxBitRate",		0,	0,0,0,0},
        {siblingElemType,	charVarType,	"mask"      ,		0,	0,0,0,0},
};
//-----------------------------------------------------------------------;

//  Encryption Capability.
// ------------------------

//typedef struct{
//	ctCapStruct		header;
//	APIU16			type;  //EencryMediaType
//	APIU16			entry; //the entry to be encrypted.
//}encryptionCapStruct;

genXmlFormat encryptionCapStructFormat[] = {
	{parentElemType,	tagVarType,	"encryptionCapStruct",	3,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,		1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		shortVarType,	"type",		0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"entry",	0,	0,0,0,0},
};
//-----------------------------------------------------------------------;

//  Encryption Token.
// ------------------

genXmlFormat halfKeyFormat[] = {
	{parentElemType,	tagVarType,	"halfKeyFormat",	2,	0,0,0,0},
		{childElemType,		longVarType,		"length",		0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,	"halfKey",		0,	0,0,0,0},
};
//typedef struct{
//	xmlDynamicHeader		xmlHeader;		// For XML API.
//	APIU32					modSize;		// BIT STRING
//	APIU32					generator;		// BIT STRING
//	APIU16					tokenOID;		// EenHalfKeyType
//	APIU16					filler;
//	APIU32					hkLen;			// The len of the halfKey in BYTE
//	APIU8					halfKey[1];
//}encryptionToken;

genXmlFormat encryptionTokenFormat[] = {
	{parentElemType,	tagVarType,	"encryptionToken",	6,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		longVarType,	"modSize",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"generator",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"tokenOID",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"filler",	0,	0,0,0,0},
		{parentElemType,	dynamicDataVarType,	"halfKey",	1, (int) &halfKeyFormat,0,0,0}, //dynamic

};
//-----------------------------------------------------------------------;

dynamicTableStruct encryDynamicTbl[] = {
	{tblEncToken,	(int)&encryptionTokenFormat},
};

//-----------------------------------------------------------------------;

//typedef struct{
//
//	APIU16					numberOfTokens;
//	APIU16					dynamicTokensLen;
//	xmlDynamicProperties    xmlDynamicProps;
//	APIU8					token[1];
//
//}encTokensHeaderStruct;

genXmlFormat encTokensHeaderStructFormat[] = {
	{parentElemType,	tagVarType,	"encTokensHeaderStruct",	4,	0,0,0,0},
		{childElemType,		shortVarType,	"numberOfTokens",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"dynamicTokensLen",	0,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlDynamicProps",	1,(int)	&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType,	"token",			NumOfElements(encryDynamicTbl),	(int) &encryDynamicTbl,0,0,0},	//Dynamic!!!!!!
};
//-----------------------------------------------------------------------;
//N.A. DEBUG VP8
/*
typedef struct{
	ctCapStruct			header;
	APIS32				maxFR;
	APIS32				maxFS;
	APIS32             	maxBitRate;

	//additional parameters: resolution etc.
} vp8CapStruct;
*/

genXmlFormat vp8CapStructFormat[] = {
	{parentElemType,	tagVarType,	"vp8CapStruct",			3,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"maxFR",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxFS",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxBitRate",		0,	0,0,0,0},
};


//-----------------------------------------------------------------------;

//typedef struct{
//	ctCapStruct			header;
//	APIS32				rtcpFeedbackMask;
//	APIS32				maxValue;
//	APIS32				minValue;
//	APIU32				maxAverageBitrate;
//  APIU32				maxPtime;
//  APIU32				minPtime;
//  APIU32				cbr;
//  APIU32				useInbandFec;
//  APIU32				useDtx;
//}opus_CapStruct;

genXmlFormat opusCapStructFormat[] = {
	{parentElemType,	tagVarType,	"opus_CapStruct",		10,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"rtcpFeedbackMask",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"minValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxAverageBitrate",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxPtime",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"minPtime",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"cbr",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"useInbandFec",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"useDtx",		0,	0,0,0,0},
};

//-----------------------------------------------------------------------;

dynamicTableStruct capDynamicTbl[] = {
	{eG711Alaw64kCapCode,         (int)&audioCapStructBaseFormat},
	{eG711Alaw56kCapCode,         (int)&audioCapStructBaseFormat},
	{eG711Ulaw64kCapCode,         (int)&audioCapStructBaseFormat},
	{eG711Ulaw56kCapCode,         (int)&audioCapStructBaseFormat},
	{eG722_64kCapCode,            (int)&audioCapStructBaseFormat},
	{eG722_56kCapCode,            (int)&audioCapStructBaseFormat},
	{eG722_48kCapCode,            (int)&audioCapStructBaseFormat},
	{eG722Stereo_128kCapCode,     (int)&audioCapStructBaseFormat},
	{eG728CapCode,                (int)&audioCapStructBaseFormat},
	{eG729CapCode,                (int)&audioCapStructBaseFormat},
	{eG729AnnexACapCode,          (int)&audioCapStructBaseFormat},
	{eG729wAnnexBCapCode,         (int)&audioCapStructBaseFormat},
	{eG729AnnexAwAnnexBCapCode,   (int)&audioCapStructBaseFormat},
	{eG7231CapCode,               (int)&g7231CapStructFormat},
	{eIS11172AudioCapCode,        (int)&IS11172AudioCapStructFormat},
	{eIS13818CapCode,             (int)&IS13818CapStructFormat},
	{eG7231AnnexCapCode,          (int)&G7231AnnexCapStructFormat},
	{eG7221_32kCapCode,           (int)&audioCapStructBaseFormat},
	{eG7221_24kCapCode,           (int)&audioCapStructBaseFormat},
	{eG7221_16kCapCode,           (int)&audioCapStructBaseFormat},
	{eSiren14_48kCapCode,         (int)&audioCapStructBaseFormat},
	{eSiren14_32kCapCode,         (int)&audioCapStructBaseFormat},
	{eSiren14_24kCapCode,         (int)&audioCapStructBaseFormat},
	{eG7221C_48kCapCode,          (int)&audioCapStructBaseFormat},
	{eG7221C_32kCapCode,          (int)&audioCapStructBaseFormat},
	{eG7221C_24kCapCode,          (int)&audioCapStructBaseFormat},
	{eG7221C_CapCode,             (int)&G7221CCapStructFormat},
	{eSiren14Stereo_48kCapCode,   (int)&audioCapStructBaseFormat},
	{eSiren14Stereo_56kCapCode,   (int)&audioCapStructBaseFormat},
	{eSiren14Stereo_64kCapCode,   (int)&audioCapStructBaseFormat},
	{eSiren14Stereo_96kCapCode,   (int)&audioCapStructBaseFormat},
	{eG719_32kCapCode,            (int)&audioCapStructBaseFormat},
	{eG719_48kCapCode,            (int)&audioCapStructBaseFormat},
	{eG719_64kCapCode,            (int)&audioCapStructBaseFormat},
	{eG719_96kCapCode,            (int)&audioCapStructBaseFormat},
	{eG719_128kCapCode,           (int)&audioCapStructBaseFormat},
	{eSiren22_32kCapCode,         (int)&audioCapStructBaseFormat},
	{eSiren22_48kCapCode,         (int)&audioCapStructBaseFormat},
	{eSiren22_64kCapCode,         (int)&audioCapStructBaseFormat},
	{eG719Stereo_64kCapCode,      (int)&audioCapStructBaseFormat},
	{eG719Stereo_96kCapCode,      (int)&audioCapStructBaseFormat},
	{eG719Stereo_128kCapCode,     (int)&audioCapStructBaseFormat},
	{eSiren22Stereo_64kCapCode,   (int)&audioCapStructBaseFormat},
	{eSiren22Stereo_96kCapCode,   (int)&audioCapStructBaseFormat},
	{eSiren22Stereo_128kCapCode,  (int)&audioCapStructBaseFormat},
	{eSirenLPR_32kCapCode,        (int)&SirenLPRCapStructFormat},
	{eSirenLPR_48kCapCode,        (int)&SirenLPRCapStructFormat},
	{eSirenLPR_64kCapCode,        (int)&SirenLPRCapStructFormat},
	{eSirenLPRStereo_64kCapCode,  (int)&SirenLPRCapStructFormat},
	{eSirenLPRStereo_96kCapCode,  (int)&SirenLPRCapStructFormat},
	{eSirenLPRStereo_128kCapCode, (int)&SirenLPRCapStructFormat},
	{eiLBC_13kCapCode,            (int)&audioCapStructBaseFormat},
	{eiLBC_15kCapCode,            (int)&audioCapStructBaseFormat},
	{eOpus_CapCode,			(int)&opusCapStructFormat},
	{eOpusStereo_CapCode,	(int)&opusCapStructFormat},
	{eRfc2833DtmfCapCode,         (int)&audioCapStructBaseFormat},
	{eH261CapCode,                (int)&h261CapStructFormat},
	{eH262CapCode,                (int)&h262CapStructFormat},
	{eH263CapCode,                (int)&h263CapStructFormat},
	{eH264CapCode,                (int)&h264CapStructFormat},
	{eMsSvcCapCode,               (int)&msSvcCapStructFormat},
	{eH26LCapCode,                (int)&genericVideoCapStructFormat},
	{eIS11172VideoCapCode,        (int)&IS11172VideoCapStructFormat},
	{eVP8CapCode,			(int)&vp8CapStructFormat}, //N.A. DEBUG VP8
	{eGenericVideoCapCode,        (int)&genericVideoCapStructFormat},
	{eT120DataCapCode,            (int)&dataCapStructBaseFormat},
	{eAnnexQCapCode,              (int)&dataCapStructBaseFormat},
	{eRvFeccCapCode,              (int)&dataCapStructBaseFormat},
	{eNonStandardCapCode,         (int)&ctNonStandardCapStructFormat},
	{eGenericCapCode,             (int)&audioCapStructBaseFormat},
	{ePeopleContentCapCode,       (int)&audioCapStructBaseFormat},
	{eRoleLabelCapCode,           (int)&audioCapStructBaseFormat},
	{eH239ControlCapCode,         (int)&audioCapStructBaseFormat},
	{eChairControlCapCode,        (int)&ChairControlCapStructFormat},
	{eEncryptionCapCode,          (int)&encryptionCapStructFormat},
	{eDBC2CapCode,                (int)&genericVideoCapStructFormat},
	{eLPRCapCode,                 (int)&lprCapStructFormat},
	{eDynamicPTRCapCode,          (int)&audioCapStructBaseFormat},
	{eREDCapCode,           (int)&redCapStructFormat},     //LYNC2013_FEC_RED
	{eFECCapCode,           (int)&fecCapStructFormat},     //LYNC2013_FEC_RED
	{eSiren7_16kCapCode,          (int)&audioCapStructBaseFormat},
	{eUnknownAlgorithemCapCode,   (int)&ctCapStructFormat},
	};

//-----------------------------------------------------------------------;

//typedef struct cap_fd_set {
//	long fds_bits[FD_SET_SIZE];
//
//} cap_fd_set;

genXmlFormat cap_fd_setFormat[] = {
	{parentElemType,	tagVarType,	"cap_fd_set",	1,	0,0,0,0},
		{childElemType,		longListVarType,	"fds_bits",		FD_SET_SIZE,	0,0,0,0},
};
//-----------------------------------------------------------------------
//typedef struct{
//	xmlDynamicHeader		xmlHeader;
//	APIU8					capTypeCode;
//	APIU8					sipPayloadType;
//	UINT16					capLength;
//	xmlDynamicProperties	xmlDynamicProps;
//	unsigned char			dataCap[1];
//}capBuffer;

genXmlFormat capBufferFormat[] = {
	{parentElemType,	tagVarType,	"capBuffer",	6,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"capTypeCode",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"sipPayloadType",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"capLength",		0,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType,	"dataCap",			NumOfDynamicCaps,	(int) &capDynamicTbl,0,0,0},	//Dynamic!!!!!!
};

//-----------------------------------------------------------------------
dynamicTableStruct capBufferDynamicTbl[] = {
	{tblCapBuffer,	(int)&capBufferFormat},
};


//typedef struct capFromMcms{
//	cap_fd_set				altMatrix;
//	APIU8					numberOfCaps;
//	APIU8					numberOfAlts;
//	APIU8					numberOfSim;
//	APIU8					filler2;
//	xmlDynamicProperties	xmlDynamicProps;
//	capBuffer				caps;
//}ctCapabilitiesStruct;

genXmlFormat ctCapabilitiesStructFormat[] = {
	{parentElemType,	tagVarType,	"ctCapabilitiesStruct",	7,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&cap_fd_setFormat,0,0,0},
		{childElemType,		charVarType,	"numberOfCaps",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"numberOfAlts",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"numberOfSim",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"filler2",			0,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType,	"caps",				NumOfElements(capBufferDynamicTbl),	(int) &capBufferDynamicTbl,0,0,0},
};

//SDES
//-----------------------------------------------------------------------
//typedef struct{
//	char				keySalt[MAX_BASE64_KEY_SALT_LEN];	// mandatory param
//	BOOL				bIsLifeTimeInUse;
//	APIU32				lifetime;							// optional param
//	BOOL				bIsMkiInUse;
//	APIU8 				mkiValue;							// optional param
//	BOOL				bIsMkiValueLenInUse;
//	APIU8 				mkiValueLen;						// optional param
//
//}sdesKeyInfoStruct;

genXmlFormat sdesKeyInfoStructFormat[] = {
	{parentElemType,	tagVarType,	"sdesKeyInfoStruct",	7,	0,0,0,0},
		{childElemType,	charArrayVarType,	"keySalt",					0,	MAX_BASE64_KEY_SALT_LEN,0,0,0},
		{siblingElemType,	longVarType,	"bIsLifeTimeInUse",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"lifetime",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsMkiInUse",				0,	0,0,0,0},
		{siblingElemType,	longVarType,	"mkiValue",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsMkiValueLenInUse",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"mkiValueLen",				0,	0,0,0,0},
};

//-----------------------------------------------------------------------
//typedef struct{
//	APIU16					keyMethod;
//	sdesKeyInfoStruct		keyInfo;
//
//} sdesKeyParamsStruct;

genXmlFormat sdesKeyParamsStructFormat[] = {
	{parentElemType,	tagVarType,	"sdesKeyParamsStruct",	2,	0,0,0,0},
		{childElemType,	longVarType,	"keyMethod",	0,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,	1,(int)	&sdesKeyInfoStructFormat,0,0,0},
};

//-----------------------------------------------------------------------
//typedef struct {
//	BOOL								sdesUnencryptedSrtp;		// default is 0-encrypted
//	BOOL								sdesUnencryptedSrtcp;		// default is 0-encrypted
//	BOOL								sdesUnauthenticatedSrtp;	// default is 0-authenticated
//	BOOL								bIsKdrInUse;
//	APIU8								sdesKdr;
//	BOOL								bIsWshInUse;
//	APIU16								sdesWsh;	//SRTP window size-parameter to protect against replay attacks
//	BOOL								bIsFecOrderInUse;
//	APIU16								sdesFecOrder;
//	BOOL								bIsFecKeyInUse;
//
//} sdesSessionParamsStruct;
genXmlFormat sdesSessionParamsStructFormat[] = {
	{parentElemType,	tagVarType,	"sdesSessionParamsStruct",	10,	0,0,0,0},
		{childElemType,		longVarType,	"sdesUnencryptedSrtp",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sdesUnencryptedSrtcp",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sdesUnauthenticatedSrtp",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsKdrInUse",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sdesKdr",						0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsWshInUse",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sdesWsh",						0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsFecOrderInUse",				0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sdesFecOrder",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"bIsFecKeyInUse",				0,	0,0,0,0},
};

//-----------------------------------------------------------------------
//typedef struct {
//	xmlDynamicHeader	xmlHeader;
//	sdesKeyParamsStruct	elem;
//} xmlSdesKeyParamsStruct;
genXmlFormat sdesKeyParamsListFormat[] = {
	{parentElemType,	tagVarType,		"keyParamList",	2,	0,0,0,0},
		{parentElemType,	structVarType, NULL,		1,(int)	&xmlDynamicHeaderFormat,0, 0, 0},
		{parentElemType,	structVarType, NULL,		1,(int)	&sdesKeyParamsStructFormat,   0, 0, 0}
};

dynamicTableStruct sdesKeyParamsDynamicTbl[] = {
	{1,	(int) &sdesKeyParamsListFormat}
};
//-----------------------------------------------------------------------
//typedef struct{
//	ctCapStruct				header;
//
//	APIU32					tag; 				// mandatory param
//	APIU16					cryptoSuite; 		// mandatory param
//	sdesSessionParamsStruct	sessionParams;		// optional param
//
//	int						numKeyParams;
//	xmlDynamicProperties	xmlDynamicProps;
//	char					keyParamsList[1];
//
//}sdesCapStruct;
genXmlFormat sdesCapStructFormat[] = {
	{parentElemType,	tagVarType,	"sdesCapStruct",	7,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"tag",					0,	0,0,0,0},
		{siblingElemType,	longVarType,	"cryptoSuite",			0,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&sdesSessionParamsStructFormat,0,0,0},
		{siblingElemType,	longVarType,	"numKeyParams",		0, 0, 0, 0, 0},
		{parentElemType,	structVarType,	 NULL, 				1, (int)&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType,	"keyParamsList",		NumOfElements(sdesKeyParamsDynamicTbl), (int) &sdesKeyParamsDynamicTbl, 0, 0, 0},
};



//Sync API
//-----------------------------------------------------------------------
//typedef struct {
//	ctCapStruct			header;
//	APIS8 				icePwd[IcePwdLen];
//} icePwdCapStruct;
genXmlFormat icePwdCapStructFormat[] = {
	{parentElemType,	tagVarType,	"icePwdCapStruct",	2,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		charVarType,	"icePwd",		0,	IcePwdLen,0,0,0},		
		
};

//-----------------------------------------------------------------------
//typedef struct {
//	ctCapStruct			header;
//	APIS8 				iceUfrag[IceUfragLen];
//} iceUfragCapStruct;
genXmlFormat iceUfragCapStructFormat[] = {
	{parentElemType,	tagVarType,	"iceUfragCapStruct",	2,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		charVarType,	"iceUfrag",		0,	IceUfragLen,0,0,0},		
		
};

//-----------------------------------------------------------------------
////typedef struct {
////	ctCapStruct			header;
////	APIS8 				candidate[512];
////	APIS8               candidateType;
////	APIS8               filler1;
////	APIS8               filler2;
////	APIS8               filler3;
////} iceCandidateCapStruct;
//
genXmlFormat iceCandidateCapStructFormat[] = {
	{parentElemType,	tagVarType,	"iceCandidateCapStruct",	6,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		charVarType,	"candidate",		0,	512,0,0,0},
		{childElemType,		charVarType,	"candidateType",		0,	1,0,0,0},
		{childElemType,		charVarType,	"filler1",		0,	1,0,0,0},
		{childElemType,		charVarType,	"filler2",		0,	1,0,0,0},
		{childElemType,		charVarType,	"filler3",		0,	1,0,0,0},
		
};

//-----------------------------------------------------------------------
//typedef struct {
//	ctCapStruct			header;
//	APIS8 			mandidate[512];
//} iceRemoteCandidateCapStruct;
genXmlFormat iceRemoteCandidateCapStructFormat[] = {
	{parentElemType,	tagVarType,	"iceRemoteCandidateCapStruct",	2,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		charVarType,	"candidate",		0,	512,0,0,0},		
		
};

//-----------------------------------------------------------------------
//typedef struct {
//	ctCapStruct			header;
//	APIU16				port;
//} rtcpCapStruct;
genXmlFormat rtcpCapStructFormat[] = {
	{parentElemType,	tagVarType,	"rtcpCapStruct",	2,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,		1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		shortVarType,	"port",		0,	0,0,0,0},

};

//-----------------------------------------------------------------------
//typedef struct{
//	APIS8				floorid[32];
//	APIS8				m_stream_0[32];
//	APIS8				m_stream_1[32];
//	APIS8				m_stream_2[32];
//	APIS8				m_stream_3[32];
//}bfcpFlooridStruct;
genXmlFormat bfcpFlooridStructFormat[] = {
	{parentElemType,	tagVarType,	"bfcpFlooridStruct",	5,	0,0,0,0},
		{childElemType,		charVarType,	"floorid",		0,	32,0,0,0},
		{siblingElemType,	charVarType,	"m_stream_0",		0,	32,0,0,0},
		{siblingElemType,	charVarType,	"m_stream_1",		0,	32,0,0,0},
		{siblingElemType,	charVarType,	"m_stream_2",		0,	32,0,0,0},
		{siblingElemType,	charVarType,	"m_stream_3",		0,	32,0,0,0},
};

//-----------------------------------------------------------------------
//typedef struct{
//	ctCapStruct			header;
//	APIU8				floorctrl;
//	APIS8				confid[32];
//	APIS8				userid[32];
//	bfcpFlooridStruct	floorid_0;
//	bfcpFlooridStruct	floorid_1;
//	bfcpFlooridStruct	floorid_2;
//	bfcpFlooridStruct	floorid_3;
//	APIU8				setup;
//	APIU8				connection;
//	APIU8				xbfcp_info_enabled;
//	APIU16				xbfcp_info_time;
//	APIU8				mstreamType;
//	enTransportType			transType;
//}bfcpCapStruct;
genXmlFormat bfcpCapStructFormat[] = {
	{parentElemType,	tagVarType,	"bfcpCapStruct",	14,	0,0,0,0},
		{parentElemType,		structVarType,	NULL,			1,(int)	&ctCapStructFormat,0,0,0},
		{siblingElemType,		charVarType,	"floorctrl",		0,	0,0,0,0},
		{siblingElemType,		charVarType,	"confid",		0,	32,0,0,0},
		{siblingElemType,		charVarType,	"userid",		0,	32,0,0,0},
		{parentElemType,	structVarType,	"floorid_0",				1,(int)	&bfcpFlooridStructFormat,0,0,0},
		{parentElemType,	structVarType,	"floorid_1",				1,(int)	&bfcpFlooridStructFormat,0,0,0},
		{parentElemType,	structVarType,	"floorid_2",				1,(int)	&bfcpFlooridStructFormat,0,0,0},
		{parentElemType,	structVarType,	"floorid_3",				1,(int)	&bfcpFlooridStructFormat,0,0,0},
		{siblingElemType,	charVarType,	"setup",		0, 0, 0, 0, 0},
		{siblingElemType,	charVarType,	"connection",		0, 0, 0, 0, 0},
		{siblingElemType,	charVarType,	"xbfcp_info_enabled",		0, 0, 0, 0, 0},
		{siblingElemType,	shortVarType,	"xbfcp_info_time",		0, 0, 0, 0, 0},
		{siblingElemType,	charVarType,	"mstreamType",		0, 0, 0, 0, 0},
		{siblingElemType,	longVarType,	"transType",		0, 0, 0, 0, 0},
};

//-----------------------------------------------------------------------
//typedef struct {
//	APIU32 	capabilityID;	// Unique random integer among the listed capability ID
//	APIU32 	widthVF;		// Width of video frame
//	APIU32 	heightVF;		// Height of video frame
//	APIU32 	fps;			// Frames-per-second
//	APIU32	maxBitrateInBps;// For future use : maximum-bitrate-in bits-per-second
//
//}rtvCapItemS;
genXmlFormat rtvCapItemSFormat[] = {
	{parentElemType,	tagVarType,	"rtvCapItemS",	5,	0,0,0,0},
		{childElemType,		longVarType,	"capabilityID",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"widthVF",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"heightVF",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"fps",			0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxBitrateInBps",	0,	0,0,0,0},
};

//typedef struct{
//	ctCapStruct		header;
//	APIU32			numOfItems;
//	rtvCapItemS 	rtvCapItem[NumOfRtvItems];
//	APIS32			rtcpFeedbackMask;
//
//}rtvCapStruct;
genXmlFormat rtvCapStructFormat[] = {
	{parentElemType,	tagVarType,	"rtvCapStruct",	4,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,		1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"numOfItems",		0,	0,0,0,0},
		{parentElemType,	structVarType,	"rtvCapItem",			NumOfRtvItems,	(int) &rtvCapItemSFormat,0,0,0},
		{siblingElemType,	longVarType,	"rtcpFeedbackMask",		0,	0,0,0,0},

};

//-----------------------------------------------------------------------
//typedef struct{
//	ctCapStruct				header;
//
//	APIU32					setupMode; //setupModeEnum
//	APIU32					connectionType; //connectionTypeEnum
//	APIU8               	cfw_id[128];
//} mccfCapStruct;
genXmlFormat mccfCapStructFormat[] = {
	{parentElemType,	tagVarType,	"mccfCapStruct",	4,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,		1,(int)	&ctCapStructFormat,0,0,0},
		{childElemType,		longVarType,	"setupMode",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"connectionType",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"cfw_id",		0,	128,0,0,0},

};

//-----------------------------------------------------------------------
//typedef struct
//{
//    APIU32          streamSsrcId;       // the stream SSRC
//    APIU32          frameWidth;
//    APIU32          frameHeight;
//    APIU32          maxFrameRate;
//    APIU32          requstedStreamSsrcId;  // specific stream that this SSRC want to receive
//} STREAM_S;
genXmlFormat stream_sFormat[] = {
	{parentElemType,	tagVarType,	"STREAM_S",	5,	0,0,0,0},
		{childElemType,		longVarType,	"streamSsrcId",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"frameWidth",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"frameHeight",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxFrameRate",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"requstedStreamSsrcId",		0,	0,0,0,0},
};

//-----------------------------------------------------------------------
//typedef struct
//{
//    APIU32     streamGroupId;
//    APIU8      numberOfStreams;
//    STREAM_S   streams[MAX_NUM_STREAMS_PER_SET];
//} STREAM_GROUP_S;
genXmlFormat stream_group_sFormat[] = {
	{parentElemType,	tagVarType,	"STREAM_GROUP_S",	3,	0,0,0,0},
		{childElemType,		longVarType,	"streamGroupId",		0,	0,0,0,0},
		{siblingElemType,	charVarType,		"numberOfStreams",		0,	0,0,0,0},
		{parentElemType,	structVarType,		"streams",			MAX_NUM_STREAMS_PER_SET,	(int) &stream_sFormat,0,0,0},
};

//-----------------------------------------------------------------------;
//typedef struct{
//    ctCapStruct         header;
//    APIS32		  rtcpFeedbackMask;
//    APIS32              maxValue;
//    APIS32              minValue;
//    APIU32              sirenLPRMask;
//
//    APIU32              sampleRate;
//    APIU32              mixDepth;
//    STREAM_GROUP_S      recvStreamsGroup;
//    STREAM_GROUP_S      sendStreamsGroup;
//
//}sirenLPR_Scalable_CapStruct;
genXmlFormat sirenLPR_Scalable_CapStructFormat[] = {
	{parentElemType,	tagVarType,	"sirenLPR_Scalable_CapStruct",	9,	0,0,0,0},
		{parentElemType,	structVarType,	NULL,				1,(int)	&ctCapStructFormat,0,0,0},
        	{childElemType,		longVarType,	"rtcpFeedbackMask",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"maxValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"minValue",		0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sirenLPRMask",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"sampleRate",	0,	0,0,0,0},
		{siblingElemType,	longVarType,	"mixDepth",	0,	0,0,0,0},
		{parentElemType,	structVarType,	"recvStreamsGroup",				1,(int)	&stream_group_sFormat,0,0,0},
		{parentElemType,	structVarType,	"sendStreamsGroup",				1,(int)	&stream_group_sFormat,0,0,0},

};

//-----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////
//typedef struct {
//    h264CapStruct 		h264;
//    // SVC fields
//    VIDEO_OPERATION_POINT_SET_S operationPoints;
//    STREAM_GROUP_S              recvStreamsGroup;
//    STREAM_GROUP_S              sendStreamsGroup;
//    APIU8                       scalableLayerId;    // needed for legacy SVC support; if MRC SVC, it will be ignored
//    APIU8                       isLegacy;           // 0 � for MRC, 1 � for legacy
//
//} svcCapStruct;
genXmlFormat svcCapStructFormat[] = {
	{parentElemType,	tagVarType,	"svcCapStruct",	6,	0,0,0,0},
		{parentElemType,  structVarType,  "264",  1,  (int)&h264CapStructFormat, 0, 0, 0},
		{parentElemType,  structVarType,  "operationPoints",  1,  (int)&videoOperationPointSetSFormat, 0, 0, 0},
		{parentElemType,  structVarType,  "recvStreamsGroup",  1,  (int)&stream_group_sFormat, 0, 0, 0},
		{parentElemType,  structVarType,  "scalableLayerId",  1,  (int)&stream_group_sFormat, 0, 0, 0},
		{siblingElemType,	charVarType,	"scalableLayerId",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"isLegacy",			0,	0,0,0,0},

};


//end of sync
