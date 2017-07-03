
// CapH263Annexes.h
// Yossi Maimon / Mikhail Karasik

#include "CsHeader.h"
#include "IpChannelParams.h"
#define H263_Annexes_Number	22
#define H263_Custom_Number	6 //Number of customPictureFormat. SIZE (1..16) 


//  CUSTOM PICTURE FORMAT
// =======================

// NOTE: The APIU8 is not big enought to support 1 - 256.
// Currently this is not a problem as we support noly 1 AdditionalPictureMemory.
typedef struct{
	APIU8				sqcifAdditionalPictureMemory;	// INTEGER (1..256) -- units frame
	APIU8				qcifAdditionalPictureMemory;	// INTEGER (1..256) -- units frame
	APIU8				cifAdditionalPictureMemory;		// INTEGER (1..256) -- units frame
	APIU8				cif4AdditionalPictureMemory;	// INTEGER (1..256) -- units frame
	APIU8				cif16AdditionalPictureMemory;	// INTEGER (1..256) -- units frame
	APIU8				bigCpfAdditionalPictureMemory;	// INTEGER (1..256) -- units frame

} additionalPictureMemoryStruct; 

typedef struct{
	APIU16        		customMPI;						// INTEGER (1..2048)
	APIU16        		clockConversionCode;			// INTEGER (1000..1001)
	APIU8	       		clockDivisor;					// INTEGER (1..127)
}customPCFSt;

/*
Alinment of customPicFormatSt - 4 longs
----------------------------------------
APIU16	maxCustomPictureWidth	
APIU16	maxCustomPictureHeight
APIU16	minCustomPictureWidth	
APIU16	minCustomPictureHeight
APIU16	customMPI		
APIU16  clockConversionCode
APIU8	clockDivisor			
APIU8	standardMPI
APIU8	pixelAspectCode[1]
APIU8	filler
*/

typedef struct{
	xmlDynamicHeader	xmlHeader;

	APIU16				maxCustomPictureWidth;			// INTEGER (1..2048) -- units 4 pixels
	APIU16				maxCustomPictureHeight;			// INTEGER (1..2048) -- units 4 pixels
	APIU16				minCustomPictureWidth;			// INTEGER (1..2048) -- units 4 pixels
	APIU16				minCustomPictureHeight;			// INTEGER (1..2048) -- units 4 pixels

	//mPIStruct
		//customPCFSt
		APIU16        	customMPI;						// INTEGER (1..2048)
		APIU16        	clockConversionCode;			// INTEGER (1000..1001)
		APIU8	       	clockDivisor;					// INTEGER (1..127)	
		//------------------------------------------
	APIS8				standardMPI;				// INTEGER (1..31)
	//---------------------------------------
	//pixelAspectInformationStruct
	APIU8				pixelAspectCode[1]; //  INTEGER(1..14) - According ASN1, the array can be bigger.
	//-----------------------------------------
	APIU8				filler;

} customPicFormatSt;

typedef struct{
		APIS8	clockConversionCode;					// INTEGER (1000..1001)
		APIS8	clockDivisor;							// INTEGER (1..127)
		APIS16	sqcifMPI;								// INTEGER (1..2048) OPTIONAL
		APIS16	qcifMPI;								// INTEGER (1..2048) OPTIONAL
		APIS16	cifMPI;									// INTEGER (1..2048) OPTIONAL
		APIS16	cif4MPI;								// INTEGER (1..2048) OPTIONAL
		APIS16	cif16MPI;								// INTEGER (1..2048) OPTIONAL
}customPicClockFrequencySt;

typedef struct{
	xmlDynamicHeader			xmlHeader;
	customPicClockFrequencySt	customPictureClockFrequency;
	APIU32						numberOfCustomPic;
	xmlDynamicProperties		xmlDynamicProps;
}customPic_StBase;

typedef struct{
	xmlDynamicHeader			xmlHeader;
	customPicClockFrequencySt	customPictureClockFrequency;
	APIU32						numberOfCustomPic;
	xmlDynamicProperties		xmlDynamicProps;
	char						customPicPtr[1]; //sequece of customPicFormatSt;
}customPic_St;


//  ANNEXES
// =========

typedef enum{
	VBnone,
	VBackMessageOnly,
	VBnackMessageOnly,
	VBackOrNackMessageOnly,
	VBackAndNackMessage,	

} videoBackChannelSendEnum;

typedef struct{
	additionalPictureMemoryStruct	additionalPictureMemory;
	APIU8			videoBackChannelSend;				// has value one from videoBackChannelSendEnum
	APIU8			mpuHorizMBs;						// INTEGER (1..128)
	APIU8			mpuVertMBs;							// INTEGER (1..72)
	APIU8			annexBoolMask;
#	define			refPic_videoMux						0x80

} refPictureSelectionStruct;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	APIU8	dummy;
	// H263Options is not used.
}annexBEFGHO_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;								
#	define	annexD_unlimitedMotionVectors				0x80
}annexD_St;												
														
typedef struct{		
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;								
#	define	annexI_advancedIntraCodingMode				0x80
}annexI_St;												
														
typedef struct{	
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;								
#	define	annexJ_deblockingFilterMode					0x80
}annexJ_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;
#	define	annexK_slicesInOrder_NonRect				0x80
#	define	annexK_slicesInOrder_Rect					0x40
#	define	annexK_slicesNoOrder_NonRect				0x20
#	define	annexK_slicesNoOrder_Rect					0x10
}annexK_St;												
														
typedef struct{	
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;								
#	define	annexL_fullPictureFreeze					0x80
#	define	annexL_partialPictureFreezeAndRelease		0x40
#	define	annexL_resizingPartPicFreezeAndRelease		0x20
#	define	annexL_fullPictureSnapshot					0x10
#	define	annexL_partialPictureSnapshot				0x08
#	define	annexL_videoSegmentTagging					0x04
#	define	annexL_progressiveRefinement				0x02
	//transparencyParameters	TransparencyParameters OPTIONAL,
}annexL_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;
#	define	annexM_improvedPBFramesMode					0x80
}annexM_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	refPictureSelectionStruct	refPictureSelection; // Byte*10
	APIU16						filler;
}annexN_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;
#	define	annexP_dynamicPictureResizingByFour			0x80
#	define	annexP_dynamicPictureResizingSixteenthPel	0x40
#	define	annexP_dynamicWarpingHalfPel				0x20
#	define	annexP_dynamicWarpingSixteenthPel			0x10
}annexP_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;
#	define	annexQ_reducedResolutionUpdate				0x80
}annexQ_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;
#	define	annexR_independentSegmentDecoding			0x80
}annexR_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;
#	define	annexS_alternateInterVLCMode				0x80
}annexS_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	APIU8	annexBoolMask;
#	define	annexT_modifiedQuantizationMode				0x80
}annexT_St;

typedef struct{
	xmlDynamicHeader			xmlHeader;
	refPictureSelectionStruct	refPictureSelection; // Byte*10
	APIU16						filler;
}annexU_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	APIU8	dummy;
	//H263Version3Option		h263Version3Option;	
}annexV_St;

typedef struct{
	xmlDynamicHeader	xmlHeader;
	//H263Version3Option
	APIU8	annexBoolMask;
#	define	annexW_dataPartitionedSlices				0x80
#	define	annexW_fixedPointIDCT0						0x40
#	define	annexW_interlacedFields						0x20
#	define	annexW_currentPictureHeaderRepetition		0x10
#	define	annexW_previousPictureHeaderRepetition		0x08
#	define	annexW_nextPictureHeaderRepetition			0x04
#	define	annexW_pictureNumber						0x02
#	define	annexW_spareReferencePictures				0x01
}annexW_St;

typedef union {

	//Note: annexes struct's have APIU8 elements only, thus they all need same endian treatment.
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

/*
AcEndian.c
	for (i=0;i<sizeof(annexesSpecificParms);i++)
		1. covert long(32bit).
		2. skip long(32bit) size.
*/
