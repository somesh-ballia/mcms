// CapH263AnnexesFormat.cpp
// Anat Elia

// Includes files
//----------------
#include "XmlDefs.h"
#include "XmlFormat.h"
#include "IpChannelParams.h"

// Macros
//--------

#undef	SRC
#define SRC	"H263AnnxFormat"

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
// Forward Declarations:
//----------------------


// Routines:
//----------

//typedef struct annexes_fd_set {
//	long fds_bits[1];
//
//} annexes_fd_set;

genXmlFormat annexes_fd_setFormat[] = {
	{parentElemType,	tagVarType,	"annexes_fd_set",	1,	0,0,0,0},
		{childElemType,		longListVarType,	"fds_bits",		1,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	UINT8				sqcifAdditionalPictureMemory;	// INTEGER (1..256) -- units frame
//	UINT8				qcifAdditionalPictureMemory;	// INTEGER (1..256) -- units frame
//	UINT8				cifAdditionalPictureMemory;		// INTEGER (1..256) -- units frame
//	UINT8				cif4AdditionalPictureMemory;	// INTEGER (1..256) -- units frame
//	UINT8				cif16AdditionalPictureMemory;	// INTEGER (1..256) -- units frame
//	UINT8				bigCpfAdditionalPictureMemory;	// INTEGER (1..256) -- units frame
//
//} additionalPictureMemoryStruct; 

genXmlFormat additionalPictureMemoryStructFormat[] = {
	{parentElemType,	tagVarType,	"additionalPictureMemoryStruct",	6,	0,0,0,0},
		{childElemType,		charVarType,	"sqcifAdditionalPictureMemory",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"qcifAdditionalPictureMemory",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"cifAdditionalPictureMemory",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"cif4AdditionalPictureMemory",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"cif16AdditionalPictureMemory",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"bigCpfAdditionalPictureMemory",	0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	UINT16        		customMPI;						// INTEGER (1..2048)
//	UINT16        		clockConversionCode;			// INTEGER (1000..1001)
//	UINT8	       		clockDivisor;					// INTEGER (1..127)
//}customPCFSt;

genXmlFormat customPCFStFormat[] = {
	{parentElemType,	tagVarType,	"customPCFSt",	3,	0,0,0,0},
		{childElemType,		shortVarType,	"customMPI",			0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"clockConversionCode",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"clockDivisor",			0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT16				maxCustomPictureWidth;			// INTEGER (1..2048) -- units 4 pixels
//	UINT16				maxCustomPictureHeight;			// INTEGER (1..2048) -- units 4 pixels
//	UINT16				minCustomPictureWidth;			// INTEGER (1..2048) -- units 4 pixels
//	UINT16				minCustomPictureHeight;			// INTEGER (1..2048) -- units 4 pixels
//
//	//mPIStruct
//		//customPCFSt
//		UINT16        	customMPI;						// INTEGER (1..2048)
//		UINT16        	clockConversionCode;			// INTEGER (1000..1001)
//		UINT8	       	clockDivisor;					// INTEGER (1..127)	
//		//------------------------------------------
//	UINT8				standardMPI;				// INTEGER (1..31)
//	//---------------------------------------
//	//pixelAspectInformationStruct
//	UINT8				pixelAspectCode[1]; //  INTEGER(1..14) - According ASN1, the array can be bigger.
//	//-----------------------------------------
//	UINT8				filler;
//
//} customPicFormatSt;

genXmlFormat customPicFormatStFormat[] = {
	{parentElemType,	tagVarType,	"customPicFormatSt",	11,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		shortVarType,	"maxCustomPictureWidth",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"maxCustomPictureHeight",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"minCustomPictureWidth",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"minCustomPictureHeight",	0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"customMPI",				0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"clockConversionCode",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"clockDivisor",				0,	0,0,0,0},
		{siblingElemType,	charVarType,	"standardMPI",				0,	0,0,0,0},
		{siblingElemType,	charArrayVarType,	"pixelAspectCode",			0,	1,0,0,0},
		{siblingElemType,	charVarType,	"filler",					0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//		INT8	clockConversionCode;					// INTEGER (1000..1001)
//		INT8	clockDivisor;							// INTEGER (1..127)
//		INT16	sqcifMPI;								// INTEGER (1..2048) OPTIONAL
//		INT16	qcifMPI;								// INTEGER (1..2048) OPTIONAL
//		INT16	cifMPI;									// INTEGER (1..2048) OPTIONAL
//		INT16	cif4MPI;								// INTEGER (1..2048) OPTIONAL
//		INT16	cif16MPI;								// INTEGER (1..2048) OPTIONAL
//}customPicClockFrequencySt;

genXmlFormat customPicClockFrequencyStFormat[] = {
	{parentElemType,	tagVarType,	"customPicClockFrequencySt",		7,	0,0,0,0},
		{childElemType,		charVarType,	"clockConversionCode",	0,	0,0,0,0},
		{siblingElemType,	charVarType,	"clockDivisor",			0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"sqcifMPI",				0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"qcifMPI",				0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"cifMPI",				0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"cif4MPI",				0,	0,0,0,0},
		{siblingElemType,	shortVarType,	"cif16MPI",				0,	0,0,0,0},
};
//--------------------------------------------------------------------------
dynamicTableStruct customPicDynamicTbl[] = {
	{typeCustomPic,	(int)&customPicFormatStFormat},
};
//--------------------------------------------------------------------------

//typedef struct{
//	customPicClockFrequencySt	customPictureClockFrequency;
//	UINT32						numberOfCustomPic;
//	xmlDynamicProperties		xmlDynamicProps;
//	char						customPicPtr[1]; //sequece of customPicFormatSt;
//}customPic_St;

genXmlFormat customPic_StFormat[] = {
	{parentElemType,	tagVarType,	"customPic_St",		5,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",						1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{parentElemType,	structVarType,	"customPictureClockFrequency",		1,(int)	&customPicClockFrequencyStFormat,0,0,0},
		{childElemType,		longVarType,	"numberOfCustomPic",				0,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlDynamicProps",					1,(int)	&xmlDynamicPropertiesFormat,0,0,0},
		{parentElemType,	dynamicVarType,	"customPicPtr",						NumOfElements(customPicDynamicTbl),	(int) &customPicDynamicTbl,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	additionalPictureMemoryStruct	additionalPictureMemory;
//	UINT8			videoBackChannelSend;				// has value one from videoBackChannelSendEnum
//	UINT8			mpuHorizMBs;						// INTEGER (1..128)
//	UINT8			mpuVertMBs;							// INTEGER (1..72)
//	UINT8			annexBoolMask;
//#	define			refPic_videoMux						0x80
//
//} refPictureSelectionStruct;

genXmlFormat refPictureSelectionStructFormat[] = {
	{parentElemType,	tagVarType,	"refPictureSelectionStruct",		5,	0,0,0,0},
		{parentElemType,	structVarType,	"additionalPictureMemory",	1,(int)	&additionalPictureMemoryStructFormat,0,0,0},
		{childElemType,		charVarType,	"videoBackChannelSend",		0,	0,0,0,0},
		{siblingElemType,	charVarType,	"mpuHorizMBs",				0,	0,0,0,0},
		{siblingElemType,	charVarType,	"mpuVertMBs",				0,	0,0,0,0},
		{siblingElemType,	charVarType,	"annexBoolMask",			0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT8	dummy;
//	// H263Options is not used.
//}annexBEFGHO_St;

genXmlFormat annexBEFGHO_StFormat[] = {
	{parentElemType,	tagVarType,	"annexBEFGHO_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"dummy",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT8	annexBoolMask;								
//#	define	annexD_unlimitedMotionVectors				0x80
//}annexD_St;												

genXmlFormat annexD_StFormat[] = {
	{parentElemType,	tagVarType,	"annexD_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------
														
//typedef struct{	
//	xmlDynamicHeader	xmlHeader;										
//	UINT8	annexBoolMask;								
//#	define	annexI_advancedIntraCodingMode				0x80
//}annexI_St;												

genXmlFormat annexI_StFormat[] = {
	{parentElemType,	tagVarType,	"annexI_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------
														
//typedef struct{
//	xmlDynamicHeader	xmlHeader;											
//	UINT8	annexBoolMask;								
//#	define	annexJ_deblockingFilterMode					0x80
//}annexJ_St;

genXmlFormat annexJ_StFormat[] = {
	{parentElemType,	tagVarType,	"annexJ_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT8	annexBoolMask;
//#	define	annexK_slicesInOrder_NonRect				0x80
//#	define	annexK_slicesInOrder_Rect					0x40
//#	define	annexK_slicesNoOrder_NonRect				0x20
//#	define	annexK_slicesNoOrder_Rect					0x10
//}annexK_St;												

genXmlFormat annexK_StFormat[] = {
	{parentElemType,	tagVarType,	"annexK_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------
														
//typedef struct{
//	xmlDynamicHeader	xmlHeader;											
//	UINT8	annexBoolMask;								
//#	define	annexL_fullPictureFreeze					0x80
//#	define	annexL_partialPictureFreezeAndRelease		0x40
//#	define	annexL_resizingPartPicFreezeAndRelease		0x20
//#	define	annexL_fullPictureSnapshot					0x10
//#	define	annexL_partialPictureSnapshot				0x08
//#	define	annexL_videoSegmentTagging					0x04
//#	define	annexL_progressiveRefinement				0x02
//	//transparencyParameters	TransparencyParameters OPTIONAL,
//}annexL_St;

genXmlFormat annexL_StFormat[] = {
	{parentElemType,	tagVarType,	"annexL_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT8	annexBoolMask;
//#	define	annexM_improvedPBFramesMode					0x80
//}annexM_St;

genXmlFormat annexM_StFormat[] = {
	{parentElemType,	tagVarType,	"annexM_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	refPictureSelectionStruct	refPictureSelection; // Byte*10
//	UINT16						filler;
//}annexN_St;

genXmlFormat annexN_StFormat[] = {
	{parentElemType,	tagVarType,	"annexN_St",	3,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{parentElemType,	structVarType,	"refPictureSelection",	1,(int)	&refPictureSelectionStructFormat,0,0,0},
		{childElemType,		shortVarType,	"filler",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT8	annexBoolMask;
//#	define	annexP_dynamicPictureResizingByFour			0x80
//#	define	annexP_dynamicPictureResizingSixteenthPel	0x40
//#	define	annexP_dynamicWarpingHalfPel				0x20
//#	define	annexP_dynamicWarpingSixteenthPel			0x10
//}annexP_St;

genXmlFormat annexP_StFormat[] = {
	{parentElemType,	tagVarType,	"annexP_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT8	annexBoolMask;
//#	define	annexQ_reducedResolutionUpdate				0x80
//}annexQ_St;

genXmlFormat annexQ_StFormat[] = {
	{parentElemType,	tagVarType,	"annexQ_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT8	annexBoolMask;
//#	define	annexR_independentSegmentDecoding			0x80
//}annexR_St;

genXmlFormat annexR_StFormat[] = {
	{parentElemType,	tagVarType,	"annexR_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT8	annexBoolMask;
//#	define	annexS_alternateInterVLCMode				0x80
//}annexS_St;

genXmlFormat annexS_StFormat[] = {
	{parentElemType,	tagVarType,	"annexS_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT8	annexBoolMask;
//#	define	annexT_modifiedQuantizationMode				0x80
//}annexT_St;

genXmlFormat annexT_StFormat[] = {
	{parentElemType,	tagVarType,	"annexT_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	refPictureSelectionStruct	refPictureSelection; // Byte*10
//  APIU16						filler;
//}annexU_St;

genXmlFormat annexU_StFormat[] = {
	{parentElemType,	tagVarType,	"annexU_St",	3,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",			1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{parentElemType,	structVarType,	"refPictureSelection",	1,(int)	&refPictureSelectionStructFormat,0,0,0},
		{childElemType,		shortVarType,	"filler",				0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	UINT8	dummy;
//	//H263Version3Option		h263Version3Option;	
//}annexV_St;

genXmlFormat annexV_StFormat[] = {
	{parentElemType,	tagVarType,	"annexV_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------

//typedef struct{
//	xmlDynamicHeader	xmlHeader;
//	//H263Version3Option
//	UINT8	annexBoolMask;
//#	define	annexW_dataPartitionedSlices				0x80
//#	define	annexW_fixedPointIDCT0						0x40
//#	define	annexW_interlacedFields						0x20
//#	define	annexW_currentPictureHeaderRepetition		0x10
//#	define	annexW_previousPictureHeaderRepetition		0x08
//#	define	annexW_nextPictureHeaderRepetition			0x04
//#	define	annexW_pictureNumber						0x02
//#	define	annexW_spareReferencePictures				0x01
//}annexW_St;

genXmlFormat annexW_StFormat[] = {
	{parentElemType,	tagVarType,	"annexW_St",	2,	0,0,0,0},
		{parentElemType,	structVarType,	"xmlHeader",		1,(int)	&xmlDynamicHeaderFormat,0,0,0},
		{childElemType,		charVarType,	"annexBoolMask",		0,	0,0,0,0},
};
//--------------------------------------------------------------------------
