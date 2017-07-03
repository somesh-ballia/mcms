#ifndef H263_H
#define H263_H

#define NUMBER_OF_BEST_SETS			  2
#define FORBIDDEN_IMAGE_FORMAT_H263   5
#define FORBIDDEN_MPI_H263			  15
#define H263_CUSTOM_FORMAT            4

#define STANDARD_FORMAT_ADDITIONAL_CAP	  0
#define CUSTOM_FORMAT_CAP_EQUAL_BOUNDS	  2
#define CUSTOM_FORMAT_CAP_TWO_DISTINCT_BOUNDS	  3

enum {VideoFormat,MPI,UMV,AMP,AC,PB,HRD_B,BPPmaxKB,MinPictureHeight,MinPictureWidth,
       MaxPictureHeight,MaxPictureWidth,ClockDivisor,ClockConversionCode,CustomMPIIndicator,
	   Annex_P,Annex_N,Annex_F_J,Annex_L,InterlaceMode,Annex_T, /*MUST be last*/ LengthOfH263Buffer};

////////////////////////////////////////////////////
/////             H263 Capabilities				////
////////////////////////////////////////////////////

enum{ H263_QCIF_SQCIF,H263_CIF,H263_CIF_4,//Standard formats must be first.
      H263_CIF_16,VGA,NTSC,SVGA,XGA,NTSC_60_FIELDS,PAL_50_FIELDS,
	  /*MUST be last*/ NUMBER_OF_H263_FORMATS};


static char*  NumberOfFormat[NUMBER_OF_H263_FORMATS] = {
"QCIF/SQCIF",		     // 0
"CIF",					 // 1
"4CIF",					 // 2
"16CIF",	             // 3
"VGA",                   // 4
"NTSC", 		         // 5
"SVGA",                  // 6
"XGA", 		             // 7
"NTSC (60 Fields)",      // 8
"PAL (50 Fields)" 		 // 9
};

// Interlace mode
#define INTERLACED_NONE   0
#define INTERLACED_AUTO   0xFFFF // added for vsw auto highest common phase 2 (04/04)

#define MPI_1             0
#define MPI_2             1
#define MPI_3             2
#define MPI_4             3
#define MPI_5             4
#define MPI_6             5
#define MPI_10            6
#define MPI_15            7
#define MPI_30            8

#define HRD_B_Default     0
#define HRD_B_x_1_25	  1
#define HRD_B_x_1_5  	  2
#define HRD_B_x_1_75 	  3
#define HRD_B_x_1_2 	  4
#define HRD_B_x_2_5 	  5
#define HRD_B_x_3 		  6
#define HRD_B_x_4 		  7
#define HRD_B_x_8 		  8
#define HRD_B_x_16 	      9
#define HRD_B_x_32		  10
#define HRD_B_x_64		  11
#define HRD_B_x_128		  12
#define HRD_B_x_256		  13


#define BPPmax_Default	  0
#define BPPmax_x_1_25	  1
#define BPPmax_x_1_5	  2
#define BPPmax_x_1_75	  3
#define BPPmax_x_1_2      4
#define BPPmax_x_2_5	  5
#define BPPmax_x_3	      6
#define BPPmax_x_4	      7
#define BPPmax_x_8		  8
#define BPPmax_x_16		  9
#define BPPmax_x_32		  10
#define BPPmax_x_64		  11
#define BPPmax_x_128	  12
#define BPPmax_x_256	  13


//values for pro-motion, which are used also for IP:
#define CLOCK_CONVERSION_CODE_50_FILEDS_ISDN 0
#define CLOCK_DEVISOR_50_FILEDS 36

#define CLOCK_CONVERSION_CODE_60_FILEDS_ISDN 1
#define CLOCK_DEVISOR_60_FILEDS 30

#define CUSTOM_MPI_INDICATOR_ISDN MPI_1

////////////////////////////////////////////////////
/////             H263 Capabilities				////
////////////////////////////////////////////////////

static char*  h263_cap_MPI[16] = {
"30 fps",                // 0
"15 fps",				// 1
"10 fps",				// 2
"7.5 fps",				// 3
"6 fps",				// 4
"5 fps",				// 5
"3 fps",				// 6
"2 fps",				// 7
"1 fps",				// 8
"(R)",					// 9
"(R)",					// 10
"(R)",					// 11
"(R)",					// 12
"(R)",					// 13
"(R)",					// 14
"FORBIDDEN",		// 15
};

static char*  h263_cap_Format[6] = {
"QCIF/SQCIF",		     // 0
"CIF",					 // 1
"4CIF",					 // 2
"16CIF",	             // 3
"H263 CUSTOM FORMAT",    // 4
"FORBIDDEN"  		     // 5
};

static char*  h263_cap_Optional[8] = {
"0",				// 0
"CPM",              // 1
"UMV",				// 2
"AMP",				// 3
"AC",				// 4
"PB",				// 5
"Specify HRD-B",    // 6
"Specify BPPmaxKB", // 7
};

static char*  h263_cap_HRD_B_Size[16] = {
"HRD-B_Default",    // 0
"HRD-B x 1.25",		// 1
"HRD-B x 1.5",		// 2
"HRD-B x 1.75",		// 3
"HRD-B x 1.2",		// 4
"HRD-B x 2.5",		// 5
"HRD-B x 3",		// 6
"HRD-B x 4",		// 7
"HRD-B x 8",		// 8
"HRD-B x 16",		// 9
"HRD-B x 32",		// 10
"HRD-B x 64",		// 11
"HRD-B x 128",		// 12
"HRD-B x 256",		// 13
"(R)",				// 14
"(R)",			    // 15
};


static char*  h263_cap_BPPmaxKB[16] = {
"BPPmax_Default",   // 0
"BPPmaxKB x 1.25",	// 1
"BPPmaxKB x 1.5",	// 2
"BPPmaxKB x 1.75",	// 3
"BPPmaxKB x 1.2",	// 4
"BPPmaxKB x 2.5",	// 5
"BPPmaxKB x 3",		// 6
"BPPmaxKB x 4",		// 7
"BPPmaxKB x 8",		// 8
"BPPmaxKB x 16",	// 9
"BPPmaxKB x 32",	// 10
"BPPmaxKB x 64",	// 11
"BPPmaxKB x 128",	// 12
"BPPmaxKB x 256",	// 13
"(R)",				// 14
"(R)",			    // 15
};

static char*  h263_FormatIndicator[4] = {
"STANDARD FORMAT ADDITIONAL CAP",       // 0
"FORBIDDEN",                            // 1
"CUSTOM FORMAT CAP EQUAL BOUNDS",	    // 2
"CUSTOM FORMAT CAP TWO DISTINCT BOUNDS"	// 3
};

static char*  FormatIndicator[4] = {
"Standard format ",       // 0
"FORBIDDEN",              // 1
"Custom format ",	      // 2
"Custom format "	      // 3
};

static char*  h263_OptionsIndicator[8] = {
"Options are signaled separately",                   // 0
"Inherit options from immediately larger format",    // 1                       
"Level 1 supported",	                             // 2
"Level 1,2 supported",	                             // 3
"Level 1,2,3 supported",                             // 4
"No additional H.263 options are supported",         // 5
"Reserved",                                          // 6
"Reserved for future profile extension"              // 7
};

static char*  h263_OptionsIndicator_short[8] = {
"Annexes: ",                   // 0
"Inherit Annexes from larger format",    // 1                       
"Annexes Level 1 supported",	         // 2
"Annexes Level 1,2 supported",	         // 3
"Annexes Level 1,2,3 supported",         // 4
"No Annexes are supported",              // 5
"Reserved",                              // 6
"Reserved for future profile extension"  // 7
};

static char*  h263_VideoBackChannel[8] = {
"Ack Message Only",            // 0
"Nack Message Only",           // 1                       
"Ack Or Nack Message Only",	   // 2
"Ack And Nack Message Only",   // 3
"None",                        // 4
"Reserved",                    // 5
"Reserved",                    // 6
"Forbidden"                    // 7
};

static char*  h263_SliceType[4] = {
"Slices In Order - Non Rect",       // 0
"Slices In Order - Rect",           // 1                       
"Slices No Order - Non Rect",	    // 2
"Slices No Order - Rect",           // 3
};

static char*  h263_MaxBandWidthOfLayer[16] = {
"64 kbps",    // 0
"128 kbps",	  // 1
"192 kbps",	  // 2
"256 kbps",	  // 3
"320 kbps",	  // 4
"384 kbps",	  // 5
"768 kbps",	  // 6
"1158 kbps",  // 7
"1536 kbps",  // 8
"16 kbps",	  // 9
"32 kbps",    // 10
"48 kbps",	  // 11
"Reserved",	  // 12
"Reserved",	  // 13
"Reserved",	  // 14
"Reserved",	  // 15
};

static char*  h263_MaxBandWidthOfEnhancementLayer[16] = {
"64 kbps",                 // 0
"128 kbps",	               // 1
"192 kbps",	               // 2
"Undefined",	           // 3
"320 kbps",	               // 4
"384 kbps",	               // 5
"768 kbps",	               // 6
"1158 kbps",               // 7
"1536 kbps",               // 8
"Undefined",               // 9
"1/4 of previous layer",   // 10
"Same as previous layer",  // 11
"1/2 of previous layer",   // 12 
"2/3 of previous layer",   // 13	  
"Forbidden",	           // 14
"Forbidden"	               // 15
};

static char*  h263_DynamicWarping[4] = {
"No Dynamic Warping Supported",         // 0
"Dynamic Warping Half Pel",             // 1
"Dynamic Warping Sixteenth Pel",	    // 2
"FORBIDDEN"                          	// 3
};

static char*  h263_EnhancedReferencePicSelect[4] = {
"Inherit second additional options from immediately larger format", // 0
"Not capable of using enhancedReferencePicSelect", // 1
"Capable of using enhancedReferencePicSelect without sub-picture removal", // 2
"Capable of using enhancedReferencePicSelect with sub-picture removal" // 3                          	// 3
};

static char*  h263_EnhancedReferencePicSelectShort[4] = {
"Inherit second capabilities from larger format", // 0
"Not capable of using enhancedReferencePicSelect", // 1
"Capable of using enhancedReferencePicSelect without sub-picture removal", // 2
"Capable of using enhancedReferencePicSelect with sub-picture removal" // 3                          	// 3
};

#endif
