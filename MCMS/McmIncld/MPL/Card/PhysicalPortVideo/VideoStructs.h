#ifndef   __VIDEO_STRUCTS_H__
#define   __VIDEO_STRUCTS_H__

//#ifndef __TEST_FOR_CHANGE_LAYOUT_IMPROVEMENT__
//#define __TEST_FOR_CHANGE_LAYOUT_IMPROVEMENT__
//#define TEST_NUM_OF_BYTES 600
//#endif


#include "PhysicalResource.h"



//////////////////////////////
//   CONSTANTS DEFINITIONS
//////////////////////////////
#define MAX_NUMBER_OF_CELLS_IN_LAYOUT   	16
#define MAX_SITE_NAME_SIZE					96 // Shelly: was 64 // 32 characters decoded in UCS-2BE (16 bit), no null terminating if the name is full 32 characters
#define MAX_TEXT_LINES_IN_MESSAGE			8
#define MAX_NUM_OF_BOXES_IN_TEXT_LAYOUT 	8
#define MAX_MESSAGE_OVERLAY_STRING_LENGTH	150	 // 150 English letters = 50 Chinese letters

// Call Generator SoftMCU video parameters
typedef struct
{
	APIU8 bIsCallGenerator;
	APIU8 reserved[3];
} TCGVideoParams;

/*************************************************************************/
/* The nMBPS value is in units of 500 macro blocks per second.
   The nFS value is in units of 256 luma macro blocks.*/
typedef struct
{
     APIS32 nMBPS;              	//for content H264_L3_1_DEFAULT_MBPS/2 = 108000/2=54000 ==>54000/3600=15fps
     APIS32 nFS ;					//for content H264_L3_1_DEFAULT_FS=3600 (HD720)
     APIS32 nStaticMB;				//indicate static MB negotiated with EP or decided at MCMS according to flag. 0 - not activate, X - the number of static MB used in the call
     APIS32 nResolutionWidth;    	//For COP feature
     APIS32 nResolutionHeight;   	//For COP feature
     APIS32 nResolutionFrameRate;	//For COP feature
     APIS32 nProfile ;  		 	// 1 \u2013 baseline, 2 \u2013 High
     APIU32 unPacketPayloadFormat;  // EPacketPayloadFormat values
     APIU32 unMaxDPB;


     APIU32 nFrThreshold;

} H264_VIDEO_PARAM_S;

/*************************************************************************/
typedef struct
{
	APIS32   nQcifFrameRate;   // for Legacy use (E_VIDEO_30_FPS) -1 indicate NA (use E_VIDEO_FPS_DUMMY to make it irrelevant)
	APIS32   nCifFrameRate;    // for Legacy use (E_VIDEO_30_FPS)
	APIS32   n4CifFrameRate;   // for Legacy use (E_VIDEO_15_FPS)
	APIS32   nVGAFrameRate;    // for Legacy use (E_VIDEO_15_FPS) for normal and H261 use -1 E_VIDEO_FPS_DUMMY
	APIS32   nSVGAFrameRate;   // for Legacy use (E_VIDEO_10_FPS) for normal and H261 use -1 E_VIDEO_FPS_DUMMY
	APIS32   nXGAFrameRate;    // for Legacy use (E_VIDEO_7_5_FPS)for normal and H261 use -1 E_VIDEO_FPS_DUMMY
	APIUBOOL b263HighBbIntra;  // Default should be FALSE !! H263_HIGH_BIT_BUDGET_INTRA when set to True (1) the H263 intra that sent will be bigger.(the intra will be SOFTER)
	APIUBOOL bIs263Plus;       // Default should be FALSE

} H263_H261_VIDEO_PARAM_S;

/*************************************************************************/
typedef struct
{
    APIS32 nResolutionWidth;
    APIS32 nResolutionHeight;
    APIS32 nFrameRate;

} VP8_VIDEO_PARAM_S;

/*************************************************************************/
/*	The tH263_H261VideoParams is not relevant in case of H264 video protocol and will
 *  be set with dummy values: E_VIDEO_FPS_DUMMY and E_VIDEO_RES_DUMMY
 * 	The tH264Params is not relevant in case of H263 or H261 video protocol and will
 * 	be set with dummy values: DUMMY.*/
typedef struct
{
  APIS32                  nVideoConfType;			// EVideoConfType ename
  APIS32		   		  nVideoDecoderType;  		// E_VIDEO_DECODER_NORMAL,E_VIDEO_DECODER_CONTENT
  APIS32			      nBitRate;
  APIS32			      nProtocol;				// EVideoProtocol ename
  H263_H261_VIDEO_PARAM_S tH263_H261VideoParams;
  H264_VIDEO_PARAM_S	  tH264VideoParams;
  VP8_VIDEO_PARAM_S	  	  tVP8VideoParams;			// VP8 parameters
  APIS32                  nSampleAspectRatio;
  APIUBOOL			      bInterSync;
  APIU32                  nParsingMode; 			// EParsingMode: E_PARSING_MODE_CP, E_PARSING_MODE_PSEUDO_VSW
  APIU32                  nDecoderFrameRateLimit; 	// PCI bug patch (to be removed in V3.x)
  APIU32				  nDecoderResolutionRatio; 	// Legacy RESOLUTION_RATIO_16 if danny fail to HD ==>RESOLUTION_RATIO_4
  APIS32		          nBackgroundImageId; 		//The background id is needed for video optimization - in order to fit to FPGA scaler output resolutions more accurately
  APIUBOOL			      bIsVideoClarityEnabled;
  APIUBOOL                bIsAutoBrightnessEnabled;
  APIUBOOL				  bIsTipMode;
  TCGVideoParams		  tCallGeneratorParams;		// Call Generator SoftMCU video parameters

} DECODER_PARAM_S;


/*************************************************************************/
typedef struct
{
          APIS32 nDecoderDetectedModeWidth;
          APIS32 nDecoderDetectedModeHeight;
          APIS32 nDecoderDetectedSampleAspectRatioWidth;
          APIS32 nDecoderDetectedSampleAspectRatioHeight;
} DECODER_DETECTED_MODE_PARAM_S;
/*************************************************************************/
typedef struct
{
	APIS32		nHorizontalCroppingPercentage;		// Values: from -1 to 100 (-1 means "no cropping")
	APIS32		nVerticalCroppingPercentage;		// Values: from -1 to 100 (-1 means "no cropping")

} CROPPING_PARAM_S;
/*************************************************************************/

#define MAX_NUMBER_OF_TEMPORAL_LAYERS 3

typedef struct
{
	APIS32 nResolutionFrameRate;
	APIS32 nBitRate;
} SVC_TEMPORAL_LAYER_S;

typedef struct
{
	APIU32 unSsrcID;    // Ssrc for T0 – Encoder will use Ssrc+1 and +2 for T1,T2
	APIU32 unPrID;       // PrId for T0 – Encoder will use PrId+1 and +2 for T1,T2
	APIS32 nResolutionWidth;
	APIS32 nResolutionHeight;
	APIU32 unNumberOfTemporalLayers;
	APIS32 nProfile ;                       // 1 \u2013 baseline, 2 \u2013 High
	APIU32 unPacketPayloadFormat;  			// EPacketPayloadFormat values ?
	SVC_TEMPORAL_LAYER_S atSvcTemporalLayer[MAX_NUMBER_OF_TEMPORAL_LAYERS];
} H264_SVC_VIDEO_PARAM_S;


/*	The tH263_H261VideoParams is not relevant in case of H264 video protocol and will
 *  be set with dummy values: E_VIDEO_FPS_DUMMY and E_VIDEO_RES_DUMMY
 * 	The tH264Params is not relevant in case of H263 or H261 video protocol and will
 * 	be set with dummy values: DUMMY.*/
typedef struct
{
	APIS32                        nVideoConfType;			// EVideoConfType ename
	APIS32 					      nVideoEncoderType;		// EVideoEncoderType ename
	APIS32					      nBitRate;
	APIS32					      nProtocol;				// EVideoProtocol ename
	H263_H261_VIDEO_PARAM_S		  tH263_H261VideoParams;
	H264_VIDEO_PARAM_S			  tH264VideoParams;
	VP8_VIDEO_PARAM_S	  	  	  tVP8VideoParams;			// VP8 parameters
	APIS32                        nSampleAspectRatio;
	DECODER_DETECTED_MODE_PARAM_S tDecoderDetectedMode;
	APIS32						  nResolutionTableType;		// EVideoResolutionTableType ename
	APIU32    					  nParsingMode; 			// EParsingMode: E_PARSING_MODE_CP, E_PARSING_MODE_PSEUDO_VSW
	APIS32					      nMTUSize;					// Max packet size
	APIS32                        nTelePresenceMode;		// ETelePresenceMode ename
	APIS32					      nVideoQualityType;		// EVideoQualityType ename
	APIUBOOL			          bIsVideoClarityEnabled;
	APIU32                        nFpsMode;					// EFpsMode ename
	CROPPING_PARAM_S              tCroppingParams;
    APIUBOOL                      bEnableMbRefresh;
    APIU32						  nEncoderResolutionRatio;
    APIU32                        nMaxSingleTransferFrameSize;
	APIUBOOL					  bIsTipMode;
	APIUBOOL                      bIsLinkEncoder;
    APIUBOOL					  bUseIntermediateSDResolution;
    APIUBOOL                      bRtvEnableBFrames;
    APIU8                         nFontType;               // FontTypesEnum
	APIU32 						  nFrThreshold;

    H264_SVC_VIDEO_PARAM_S		  tH264SvcVideoParams;
    H264_SVC_VIDEO_PARAM_S		  tMsSvcPacsiParams[3]; 	// Pacsi parameters will be held here

	APIUBOOL					  bFollowSpeaker;

    TCGVideoParams				  tCallGeneratorParams;		// Call Generator SoftMCU video parameters
} ENCODER_PARAM_S;

enum FontTypesEnum
{
	ftDefault,
	ftHeiti,
	ftSongti,
	ftKaiti,
	ftWeibei,
};

/*************************************************************************/

typedef struct
{
	APIU32								pucBufferAddressPretoPostScaler;	// Breeze VC - Address of image before post scaler.
	APIS32								nNumberOfBuffersPretoPostScaler;	// Breeze VC - Number of buffers of image before post scaler.
}IMAGE_PARAM_PRE_TO_POST_SCALER_S;

/*************************************************************************/

typedef struct 
{
	APIU8	 	ucRight;   	//boolean
	APIU8 		ucLeft;   	//boolean
	APIU8 		ucUp;   		//boolean
	APIU8 		ucDown;  	//boolean
} BORDER_PARAM_S;



typedef struct
{
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S   tDecoderPhysicalId;					// Includes connection Id and Physical Id
	APIU32								nDecoderPartyId;
	APIU32								nArtPartyId;  						//For the COP feature use when the Art and Decoder dont have the same party ID
	APIS32								nDecoderSizeInLayout;
	APIS32								nDecoderResolutionRatio;
	DECODER_DETECTED_MODE_PARAM_S       nDecoderDetectedMode;
	APIS32								nDecoder_xX_CoordinatorInLayout;	// For Stereo currently invalid
	APIU32								pucInitialMemoryBufferAddress;
	APIS32								nNumberOfBuffers;
	APIS8								siteName[MAX_SITE_NAME_SIZE];
	IMAGE_PARAM_PRE_TO_POST_SCALER_S	tImageParamsPreToPostScaler;
	APIS32								nThressoldCroppingOnImage;

} IMAGE_PARAM_S;

/*************************************************************************/

typedef struct
{
	APIS32		nColor;
	APIS32		nThickness;
	APIS32		nTexture;

} VISUAL_ATTRIBUTES_S;

/*************************************************************************/

typedef struct
{
	VISUAL_ATTRIBUTES_S tVisualAttributes;
	BORDER_PARAM_S		 tBorderEdges [MAX_NUMBER_OF_CELLS_IN_LAYOUT];

} BORDERS_PARAM_S;

/*************************************************************************/

typedef struct
{
	APIS32		nSpeakerImageID; 					//image within the layout
	VISUAL_ATTRIBUTES_S tVisualAttributes;

} SPEAKER_PARAM_S;
/*************************************************************************/

typedef struct
{
	APIS32		nColor;
	APIS32		nImageId;

} BACKGROUND_PARAM_S;

/*************************************************************************/
typedef struct
{
	DECODER_DETECTED_MODE_PARAM_S	 tDecoderDetectedMode;
	APIU32  						 nStatus;
	APIU32  				 		 unSsrcID;
	APIU32  				 		 unPrID;
	
} DECODER_SYNC_IND_S;

/*************************************************************************/

typedef struct
{
	APIS32				nTextColor;				// Font color
	APIS32				nFontSize;				// Values 9-32

	APIS32				nBackgroundColor;		// Background stripe color
	APIS32				nTransparency;			// 0-100 (0-Opaque, 100-Transparent)
	APIS32				nShadowWidth;			// Pixel width for shadowing (0-No shadow, 4-Maximum width)
	APIS32				nStripHeight;			// Height for strip (defaults: HD/SD=32, CIF/SIF=16)
	APIS32 				nSiteNamesVerPosition; 	// Vertical position in cell 0-100
	APIS32 				nSiteNamesHorPosition; 	// Horizontal position in cell 0-100	
	APIS32 				nSiteNamesLocation; // See ESiteNamesLocation for MPMP	
} SITENAMES_PARAM_S;

/*************************************************************************/

typedef struct
{
	APIS32				nEffectDurationMsec;	// Defines the duration for effect (in Miliseconds).
} FADE_IN_FADE_OUT_PARAM_S;

/*************************************************************************/
typedef struct
{
	BORDERS_PARAM_S           tBorder;
	SPEAKER_PARAM_S           tSpeaker;
	BACKGROUND_PARAM_S        tBackground;
	SITENAMES_PARAM_S         tSitenames;
	FADE_IN_FADE_OUT_PARAM_S  tFadeInFadeOut;

} CHANGE_LAYOUT_ATTRIBUTES_S;

/*************************************************************************/

typedef enum
{
	eLocationTop = 0,
	eLocationBottom,
	eLocationLeft,
	eLocationRight,
	eLocationTopLeft,
	eLocationTopRight,
	eLocationBottomLeft,
	eLocationBottomRight,
	// if needed, insert additional locations above this line
	MAX_NUM_LOCATION // it MUST be always the LAST item in this enumeration
} iconLocationEnum;

typedef enum
{
	eIconNetworkQuality = 0,
//	eIconEncryptedConf,
	eIconAudioPartiesCount,
//	eIconVideoPartiesCount,
//	eIconLocalPartyAppearsInLayout,
//	eIconMutedParty,
	eIconRecording,
	// if needed, insert additional icon types above this line
	MAX_NUM_TYPES_OF_ICON // it MUST be always the LAST item in this enumeration
} iconTypeEnum;

typedef struct
{
	APIU8 bActive         : 1; // true = active
	APIU8 nLocation       : 3; // taken from 'iconLocationEnum'
	APIU8 nIndexIntoStrip : 4; // 0-based index of icon position in the strip of icons
	APIU8 nIconData       : 8;  // data needed for a specific indication
} ICON_ATTR_S;

typedef struct
{
	ICON_ATTR_S    atLayoutIconParams[MAX_NUM_TYPES_OF_ICON];                              // [7]  Conference/Party indications
	ICON_ATTR_S    atCellIconParams[MAX_NUMBER_OF_CELLS_IN_LAYOUT][MAX_NUM_TYPES_OF_ICON]; // [16][7] per Cell indications
} ICONS_DISPLAY_S;


typedef struct
{
	APIS32                              nLayoutType;              // ELayoutType ename
	APIS32                              nEncoderResolutionRatio;
	DECODER_DETECTED_MODE_PARAM_S       nDecoderDetectedMode;     // the detected (resolution) width of the EP decoder
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S   tAudioEncoderPhysicalId;  // For stereo currently invalid
	IMAGE_PARAM_S                       atImageParam[MAX_NUMBER_OF_CELLS_IN_LAYOUT];
	CHANGE_LAYOUT_ATTRIBUTES_S          tChangeLayoutAttributes;
	ICONS_DISPLAY_S                     tIconsDisplay;
#ifdef __TEST_FOR_CHANGE_LAYOUT_IMPROVEMENT__
	APIS8		paddingForTesting[TEST_NUM_OF_BYTES];
#endif
} CHANGE_LAYOUT_S;






///CHANGE LAYOUT IMPROVEMENT///

//Change Layout Attributes
/*************************************************************************/
typedef struct
{
	APIS32		nColor;
	APIS32		nThickness;

} MCMS_CM_VISUAL_ATTRIBUTES_S;

/*************************************************************************/

typedef struct
{
	APIU8	 	ucRight;   	//boolean
	APIU8 		ucLeft;   	//boolean
	APIU8 		ucUp;   		//boolean
	APIU8 		ucDown;  	//boolean
}MCMS_CM_BORDER_PARAM_S;

typedef struct
{
	MCMS_CM_VISUAL_ATTRIBUTES_S tVisualAttributes;
	MCMS_CM_BORDER_PARAM_S	tBorderEdges [MAX_NUMBER_OF_CELLS_IN_LAYOUT];

} MCMS_CM_BORDERS_PARAM_S;

/*************************************************************************/

typedef struct
{
	APIS32		nSpeakerImageID; 					//image within the layout
	MCMS_CM_VISUAL_ATTRIBUTES_S tVisualAttributes;

} MCMS_CM_SPEAKER_PARAM_S;
/*************************************************************************/


/*************************************************************************/

typedef struct
{
	APIS32				nTextColor;				// Font color
	APIS8				nFontSize;				// Values 9-32
	APIS32				nBackgroundColor;		// Background stripe color
	APIS8				nTransparency;			// 0-100 (0-Opaque, 100-Transparent)
	APIS8				nShadowWidth;			// Pixel width for shadowing (0-No shadow, 4-Maximum width)
	APIS8				nStripHeight;			// Height for strip (defaults: HD/SD=32, CIF/SIF=16)
	APIS8 				nSiteNamesVerPosition; 	// Vertical position in cell 0-100
	APIS8 				nSiteNamesHorPosition; 	// Horizontal position in cell 0-100
	APIS32 				nSiteNamesLocation; // See ESiteNamesLocation for MPMP
} MCMS_CM_SITENAMES_PARAM_S;

/*************************************************************************/

typedef struct
{
	MCMS_CM_BORDERS_PARAM_S           tBorder;
	MCMS_CM_SPEAKER_PARAM_S           tSpeaker;
	BACKGROUND_PARAM_S        			tBackground;
	MCMS_CM_SITENAMES_PARAM_S         tSitenames;
	FADE_IN_FADE_OUT_PARAM_S  			tFadeInFadeOut;

} MCMS_CM_CHANGE_LAYOUT_ATTRIBUTES_S;

/*************************************************************************/

//Image params
/*************************************************************************/

typedef struct
{
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S   tDecoderPhysicalId;					// Includes connection Id and Physical Id
	APIU32								nArtPartyId;  						//For the COP feature use when the Art and Decoder dont have the same party ID
	APIS8								nDecoderSizeInLayout;
	APIS8								nDecoderResolutionRatio;
	DECODER_DETECTED_MODE_PARAM_S       nDecoderDetectedMode;
	APIS8								siteName[MAX_SITE_NAME_SIZE];
	APIS32								nThressoldCroppingOnImage;
} MCMS_CM_IMAGE_PARAM_S;

/*************************************************************************/
/*************************************************************************/


typedef struct
{
	APIS8                               	nLayoutType;              // ELayoutType ename
	APIS8                               	nEncoderResolutionRatio;
	DECODER_DETECTED_MODE_PARAM_S       	nDecoderDetectedMode;     // the detected (resolution) width of the EP decoder
	MCMS_CM_CHANGE_LAYOUT_ATTRIBUTES_S	tChangeLayoutAttributes;
	ICONS_DISPLAY_S                     	tIconsDisplay;
	MCMS_CM_IMAGE_PARAM_S                 *atImageParam;
} MCMS_CM_CHANGE_LAYOUT_S;



///************************///









/*************************************************************************/

typedef struct	// for IVR: PLC (Private-Layout-Control)
{
	APIU32	screenNumber;				// screen number to use
	APIU32	rsrv[2];					// reserve for future

} START_PLC_S;

/*************************************************************************/
/*typedef struct
{
	APIU32  status;
} ENCODER_SYNC_IND_S;
*/
/*************************************************************************/
/*************************************************************************/
/*
 *	TEXT ON SCREEN API
 */
/*************************************************************************/
/*************************************************************************/

typedef struct
{
	APIS8				acTextLine[MAX_MESSAGE_OVERLAY_STRING_LENGTH]; // do not send more than MAX_SITE_NAME_SIZE for MPMP
	APIS32				nTextColor;			// Font color
	APIS32				nBackgroundColor;	// Background stripe color
	APIS32				nTransparency;		// 0-100 (0-Opaque, 100-Transparent)
	APIS32				nShadowWidth;		// Pixel width for shadowing (0-No shadow, 4-Maximum width)
	APIS32				nAlignment;			// See ETextAlignment
	APIS32				nFontType;			// Bitwise OR with all possible options (TEXT_FONT_TYPE)
	APIS32              nFontSize;          // Value 9-32 (Value from ETextFontSize for MPMP)
} TEXT_LINE_PARAMS_S;

typedef struct
{
	APIS32				nUpperLeftX;		// X coordinate of upper left corner (relative to HD720)
	APIS32				nUpperLeftY;		// Y coordinate of upper left corner (relative to HD720)
	APIS32				nWidth;				// Width in pixels (relative to HD720)
	APIS32				nHeight;			// Height in pixels (relative to HD720)
} TEXT_BOX_SIZE_LOCATION_S;

typedef struct
{
	APIS32				nTextBoxType;					// Value from ETextBoxType
	APIUBOOL            		bIsNewData;				// Boolean flag. 0 - no need to render,  1 - need to render
	APIS32						nNumberOfTextLines;		// Number of text lines. (Possible ranges 1-8)
	APIS32						nPresentationMode;		// See ETextBoxPresentationMode (Static, Scrolled up, Scrolled down)
	APIS32						nPosition;				// Values 0-100 (See ETextBoxPosition for MPMP)
	APIS32              nDisplaySpeed;					// Value from ETextDisplaySpeed
	APIS32              nMessageRepetitionCount;
	TEXT_BOX_SIZE_LOCATION_S	tTextBoxSizeLocation;	// In case nPosition is INVALID (-1) use these coordinates
} TEXT_BOX_PARAMS_S;

typedef struct
{
	TEXT_BOX_PARAMS_S	tTextBoxParams;									// Text Box Parameters
	TEXT_LINE_PARAMS_S	atTextLineParams[MAX_TEXT_LINES_IN_MESSAGE];	// Text Line parameters. Maximum array size is 400

} TEXT_BOX_DISPLAY_S;

typedef struct
{
	TEXT_BOX_SIZE_LOCATION_S	tTextBoxSizeLocation;			// Size and location of the background box
	APIS32						nNumOfBoxes;					// Number of boxes in the current layout (excluding the background box)
	APIS32						nBackgroundTransparency;		// 0-100 (0-Opaque, 100-Transparent)
	APIS32						nBackgroundColor;				// for the half transparent layer
	TEXT_BOX_DISPLAY_S			atTextBoxDisplay[MAX_NUM_OF_BOXES_IN_TEXT_LAYOUT];
} TEXT_BOX_LAYOUT_S;
/*************************************************************************/

typedef struct
{
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S        tDecoderPhysicalId;
	APIS32                                   nOnOffResolution0; // up to qcif
	APIS32                                   nOnOffResolution1; // up to cif
	APIS32                                   nOnOffResolution4; // up to SD
} UPDATE_DECODER_RESOLUTION_S;


#define MAX_VIDEO_PORTS_PER_UNIT			8

typedef enum
{
	ePortClose	= 0,
	ePortOpen
} portStatusEnum;

/*************************************************************************/
typedef struct
{
	MCMS_MPL_PHYSICAL_RESOURCE_INFO_S		tPortPhysicalId;
	DECODER_PARAM_S							tDecoderParam;
	ENCODER_PARAM_S							tEncoderParam;
	APIU32									nPortStatus; 		//values from portStatusEnum
} ALLOC_STATUS_PER_PORT_S;
/*************************************************************************/
typedef struct
{
	ALLOC_STATUS_PER_PORT_S							atPortsStatus[MAX_VIDEO_PORTS_PER_UNIT];
} ALLOC_STATUS_PER_UNIT_S;

/*************************************************************************/

// used by older MPM+ Recording icon message
typedef struct
{
	APIS32          nIconType;             //
} ICON_PARAMS_S;

/*************************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////
//
//			SVC
//
/////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_NUM_OPERATION_POINTS_PER_SET  50


typedef struct
{
	APIU32      	layerId;
	APIU8 	        Tid; 		//Temporal id
	APIU8       	Did; 		//Dependency id
	APIU8       	Qid; 		//Quality id
	APIU8       	Pid; 		//Priority id
	APIU16   		profile;	//for sanity check??
	APIU8   		level;	//for sanity check??
	APIU32      	frameWidth;
	APIU32      	frameHeight;//for sanity check??
	APIU32   		frameRate;	//for sanity check??
	APIU32       	maxBitRate;	//for sanity check??
} VIDEO_OPERATION_POINT_S;
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	APIU32      					operationPointSetId;
	APIU8       					numberOfOperationPoints;
	VIDEO_OPERATION_POINT_S  		tVideoOperationPoints[MAX_NUM_OPERATION_POINTS_PER_SET];
}VIDEO_OPERATION_POINT_SET_S;

#endif //__VIDEO_STRUCTS_H__

