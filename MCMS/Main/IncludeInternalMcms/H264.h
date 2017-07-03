#ifndef H264_H
#define H264_H

#define MAX_NUMBER_OF_BYTES_IN_H264_CAP_SET  20
#define DEFAULT_H264_UPPER_LIMIT_LINE_RATE   384
#define MAX_H264_LEVEL_SUPPORTED_IN_VSW      H264_Level_2
#define MAX_FS_SUPPORTED_IN_VSW				 3072	// xga
#define H264_CP_TR_DEC_DEFAULT_LEVEL         H264_Level_1    //H264_Level_1 To be able to support the ASYMETRIC mode QCIF in the decoder
#define H264_CP_TR_DEC_QCIF_LEVEL            H264_Level_1
#define H264_CP_TR_DEC_CIF_LEVEL             H264_Level_1_2
#define H264_COP_ENC_DEFAULT_LEVEL           H264_Level_1_2 //to take from version.log???

#define H264_HD720_LEVEL      				 H264_Level_3_1
#define H264_HD720_FS					     3840       // H264_L3_1_DEFAULT_FS normalized to 256 Macro Blocks (MBs)
#define H264_HD720_15_MBPS					 54000		//We need for the threshold for the HD72030 asymmetric mode
#define H264_HD720_30_MBPS					 H264_L3_1_DEFAULT_MBPS
#define H264_HD720_50_MBPS                   180000		//We need for the threshold for the HD720 60 asymmetric mode
#define H264_HD720_60_MBPS                   216000
#define H264_HD720_5_MBPS                    22000
#define H264_XGA_5_MBPS                      15500
#define H264_XGA_FS                          3072
#define H264_XGA_FS_AS_DEVISION    		       12

#define H264_HD1080_LEVEL      				 H264_Level_4
#define H264_HD1080_FS                       8160
#define H264_HD1080_30_MBPS                  244800

#define H264_HD1080_30_MBPS_TOLERACE         246000  //This still counts as HD1080 30 and not HD1080 60

#define H264_HD1080_60_MBPS                  489600
#define H264_HD1080_60_VSW_MBPS              489600

#define H264_CIF_FS                          512




#define MAX_BR_IN_H264_CP                    H264_L2_DEFAULT_BR
#define H264_CIF_30_MBPS					 H264_L2_DEFAULT_MBPS

#define H264_CIF_25_MBPS					 10000
#define H264_SD_8_MBPS					     12960
#define H264_SD_15_MBPS					     20500 //round up of H264_L2_2_DEFAULT_MBPS
#define H264_WCIF60_MBPS                     39600
#define H264_SD_30_MBPS					     40000
#define H264_864_480_30_MBPS			     48600
#define H264_SD_60_MBPS					     81000
#define H264_VGA_15_MBPS					 18000 // 1200*15
#define H264_VGA_FS					         1200
#define MSSVC_SD_FS					         900
#define SD_15_FS                             1792
//#define H264_HD1080_FS                       8160

#define H264_W4CIF_FS						 2304
#define H264_W4CIF_30_MBPS                   69500 //round up
#define H264_W4CIF_30_MBPS_AS_DEVISION       139
#define H264_WCIF60_FS_AS_DEVISION           9

#define H264_QCIF_FRAME_RATE_30_MBPS            2970   //1485*2  //the value to get 30fps in QCIF level (level 1)

#define H264_RESOLUTION_4CIF      H264_L2_1_DEFAULT_FS
#define H264_RESOLUTION_CIF       H264_L1_1_DEFAULT_FS
#define H264_RESOLUTION_QCIF      H264_L1_DEFAULT_FS
//#define H264_RESOLUTION_HD720	  3840       // H264_L3_1_DEFAULT_FS normalized to 256 Macro Blocks (MBs)

#define H264_4CIF_50_MBPS     79500  // 79200 round up to 500 factor
#define H264_W4CIF_15_MBPS                   35000 //round up
#define H264_W4CIF_50_MBPS                   115500 //round up
#define H264_HD720_25_MBPS		     90000
#define H264_HD720_15_MBPS		     54000
#define H264_HD1080_15_MBPS                  122500
#define H264_HD1080_25_MBPS                  204000
#define H264_HD1080_50_MBPS                  408000


////////////////////////////////////////////////////
/////             H264 Capabilities				////
////////////////////////////////////////////////////
/* H264 PROFILES */

#define    H264_Profile_BaseLine  64
#define    H264_Profile_Main      32
#define    H264_Profile_Extended  16
#define    H264_Profile_High      8
#define    H264_Profile_None      0 // For use in case that profile isn't relevant.

/* SVC PROFILES */
#define    SVC_Profile_BaseLine   16384
#define    SVC_Profile_High       8192

/* H264 LEVELS */

#define    H264_Level_1       15
#define    H264_Level_1_1     22
#define    H264_Level_1_2     29
#define    H264_Level_1_3     36
#define    H264_Level_2       43
#define    H264_Level_2_1     50
#define    H264_Level_2_2     57
#define    H264_Level_3       64
#define    H264_Level_3_1     71
#define    H264_Level_3_2     78
#define    H264_Level_4       85
#define    H264_Level_4_1     92
#define    H264_Level_4_2     99
#define    H264_Level_5       106
#define    H264_Level_5_1     113


//Customize parameters
//first 4 according to the H264 Standard
#define CUSTOM_MAX_CPB_CODE        		2
#define CUSTOM_MAX_MBPS_CODE       		3
#define CUSTOM_MAX_FS_CODE         		4
#define CUSTOM_MAX_DPB_CODE        		5
#define	CUSTOM_MAX_BR_CODE         		6
#define CUSTOM_MAX_STATIC_MBPS	   		7
#define CUSTOM_MAX_RCMD_NAL_UNIT_SIZE	8
#define CUSTOM_MAX_NAL_UNIT_SIZE 	    9
#define CUSTOM_SAR_CODE			   	    10
#define ADDITIONAL_MODES_SUPPORETD		11
#define ADDITIONAL_DISPLAY_CAPABILITIES 12


#define CUSTOM_MAX_MBPS_FACTOR    500
#define CUSTOM_MAX_FS_FACTOR      256
#define CUSTOM_MAX_DPB_FACTOR   32768
#define CUSTOM_MAX_BR_FACTOR    25000  // ?? standard page 15 30000 NAL HRD
//#define CUSTOM_MAX_CPB_FACTOR   25000 //need to be divided in the level default BR*1000


#define H264_MBPS_UNT_FACTOR    1
#define H264_FS_UNT_FACTOR      1
#define H264_DPB_UNT_FACTOR  1024
#define H264_BR_UNT_FACTOR   1000
#define H264_CPB_UNT_FACTOR  1000


#define H264_L1_DEFAULT_MBPS  1485     // Macro Blocks per Sec (MB/s)
#define H264_L1_DEFAULT_FS    99       // Macro Blocks (MBs)
#define H264_L1_DEFAULT_DPB   152064   // 148.5*1024Bytes
#define H264_L1_DEFAULT_BR    64000    // 64*1000 bits/s
#define H264_L1_DEFAULT_CPB   175000   // 175*1000 bits/s

#define H264_L1_1_DEFAULT_MBPS  3000     // Macro Blocks per Sec (MB/s)
#define H264_L1_1_DEFAULT_FS    396      // Macro Blocks (MBs)
#define H264_L1_1_DEFAULT_DPB   345600   // 337.5*1024Bytes
#define H264_L1_1_DEFAULT_BR    192000   // 192*1000 bits/s
#define H264_L1_1_DEFAULT_CPB   500000   // 500*1000 bits/s

#define H264_L1_2_DEFAULT_MBPS  6000      // Macro Blocks per Sec (MB/s)
#define H264_L1_2_DEFAULT_FS    396       // Macro Blocks (MBs)
#define H264_L1_2_DEFAULT_DPB   912384    // 891*1024Bytes
#define H264_L1_2_DEFAULT_BR    384000    // 384*1000 bits/s
#define H264_L1_2_DEFAULT_CPB   1000000   // 1000*1000 bits/s

#define H264_L1_3_DEFAULT_MBPS  11880     // Macro Blocks per Sec (MB/s)
#define H264_L1_3_DEFAULT_FS    396       // Macro Blocks (MBs)
#define H264_L1_3_DEFAULT_DPB   912384    // 891*1024Bytes
#define H264_L1_3_DEFAULT_BR    768000    // 768*1000 bits/s
#define H264_L1_3_DEFAULT_CPB   2000000   // 2000*1000 bits/s

#define H264_L2_DEFAULT_MBPS    11880     // Macro Blocks per Sec (MB/s)
#define H264_L2_DEFAULT_FS      396       // Macro Blocks (MBs)
#define H264_L2_DEFAULT_DPB     912384    // 891*1024Bytes
#define H264_L2_DEFAULT_BR      2000000   // 2000*1000 bits/s
#define H264_L2_DEFAULT_CPB     2000000   // 2000*1000 bits/s

#define H264_L2_1_DEFAULT_MBPS  19800     // Macro Blocks per Sec (MB/s)
#define H264_L2_1_DEFAULT_FS    792       // Macro Blocks (MBs)
#define H264_L2_1_DEFAULT_DPB   1824768   // 1782*1024Bytes
#define H264_L2_1_DEFAULT_BR    4000000   // 4000*1000 bits/s
#define H264_L2_1_DEFAULT_CPB   4000000   // 4000*1000 bits/s

#define H264_L2_2_DEFAULT_MBPS  20250     // Macro Blocks per Sec (MB/s)
#define H264_L2_2_DEFAULT_FS    1620      // Macro Blocks (MBs)
#define H264_L2_2_DEFAULT_DPB   3110400   // 3037.5*1024Bytes
#define H264_L2_2_DEFAULT_BR    4000000   // 4000*1000 bits/s
#define H264_L2_2_DEFAULT_CPB   4000000   // 4000*1000 bits/s

#define H264_L3_DEFAULT_MBPS    40500     // Macro Blocks per Sec (MB/s)
#define H264_L3_DEFAULT_FS      1620      // Macro Blocks (MBs)
#define H264_L3_DEFAULT_DPB     3110400   // 3037.5*1024Bytes
#define H264_L3_DEFAULT_BR      10000000  // 10000*1000 bits/s
#define H264_L3_DEFAULT_CPB     10000000  // 10000*1000 bits/s

#define H264_L3_1_DEFAULT_MBPS  108000     // Macro Blocks per Sec (MB/s)
#define H264_L3_1_DEFAULT_FS    3600       // Macro Blocks (MBs)
#define H264_L3_1_DEFAULT_DPB   6912000   // 6750*1024Bytes
#define H264_L3_1_DEFAULT_BR    14000000   // 14000*1000 bits/s
#define H264_L3_1_DEFAULT_CPB   14000000   // 14000*1000 bits/s

#define H264_L3_2_DEFAULT_MBPS  216000     // Macro Blocks per Sec (MB/s)
#define H264_L3_2_DEFAULT_FS    5120      // Macro Blocks (MBs)
#define H264_L3_2_DEFAULT_DPB   7864320   // 7680*1024Bytes
#define H264_L3_2_DEFAULT_BR    20000000   // 20000*1000 bits/s
#define H264_L3_2_DEFAULT_CPB   20000000   // 20000*1000 bits/s

#define H264_L4_DEFAULT_MBPS    245760     // Macro Blocks per Sec (MB/s)
#define H264_L4_DEFAULT_FS      8192      // Macro Blocks (MBs)
#define H264_L4_DEFAULT_DPB     12582912   // 12288*1024Bytes
#define H264_L4_DEFAULT_BR      20000000  // 20000*1000 bits/s
#define H264_L4_DEFAULT_CPB     20000000  // 20000*1000 bits/s

#define H264_L4_1_DEFAULT_MBPS  245760     // Macro Blocks per Sec (MB/s)
#define H264_L4_1_DEFAULT_FS    8192       // Macro Blocks (MBs)
#define H264_L4_1_DEFAULT_DPB   12582912   // 12288*1024Bytes
#define H264_L4_1_DEFAULT_BR    50000000   // 50000*1000 bits/s
#define H264_L4_1_DEFAULT_CPB   50000000   // 50000*1000 bits/s

#define H264_L4_2_DEFAULT_MBPS  491520     // Macro Blocks per Sec (MB/s)
#define H264_L4_2_DEFAULT_FS    8192      // Macro Blocks (MBs)
#define H264_L4_2_DEFAULT_DPB   12582912   // 12288*1024Bytes
#define H264_L4_2_DEFAULT_BR    50000000   // 50000*1000 bits/s
#define H264_L4_2_DEFAULT_CPB   50000000   // 50000*1000 bits/s

#define H264_L5_DEFAULT_MBPS    589824     // Macro Blocks per Sec (MB/s)
#define H264_L5_DEFAULT_FS      22080      // Macro Blocks (MBs)
#define H264_L5_DEFAULT_DPB     42301440   // 41310*1024Bytes
#define H264_L5_DEFAULT_BR      135000000  // 135000*1000 bits/s
#define H264_L5_DEFAULT_CPB     135000000  // 135000*1000 bits/s

#define H264_L5_1_DEFAULT_MBPS  983040     // Macro Blocks per Sec (MB/s)
#define H264_L5_1_DEFAULT_FS    36864       // Macro Blocks (MBs)
#define H264_L5_1_DEFAULT_DPB   70778880   // 69120*1024Bytes
#define H264_L5_1_DEFAULT_BR    240000000   // 240000*1000 bits/s
#define H264_L5_1_DEFAULT_CPB   240000000   // 240000*1000 bits/s

#define H264_ALL_LEVEL_DEFAULT_SAR			13			// According to standard
#define H264_ALL_LEVEL_DEFAULT_STATIC_MBPS	-1			// Not supported yet

#define H264_HD1080_FS_AS_DEVISION	32
#define H264_HD720_FS_AS_DEVISION	15
#define H264_SD_FS_AS_DEVISION		7
#define H264_CIF_FS_AS_DEVISION		2

enum{ H264_PROFILE,H264_LEVEL,H264_CUSTOM_1,H264_CUSTOM_2,H264_CUSTOM_3,H264_CUSTOM_4
,/*MUST be last*/ NUMBER_OF_H264_FIELDS};

#define H264_COP_THRESHOLD_1080     1728
#define H264_COP_THRESHOLD_720_50   1232 //same as the HDX threshold according to Arik Y.
#define H264_COP_THRESHOLD_720      832
#define H264_COP_THRESHOLD_4CIF     256
#define H264_COP_THRESHOLD_CIF      64

#define H264_HP_COP_THRESHOLD_1080   1024
#define H264_HP_COP_THRESHOLD_720_50 832
#define H264_HP_COP_THRESHOLD_720    512
#define H264_HP_COP_THRESHOLD_4CIF   128
#define H264_HP_COP_THRESHOLD_CIF    64

#define COP_MAX_RATE_FOR_SD_RESOLUTION 20480
#define COP_MAX_RATE_FOR_CIF_RESOLUTION 7680


#define H264_SINGLE_NAL_PACKETIZATION_MODE 		0
#define H264_NON_INTERLEAVED_PACKETIZATION_MODE 1
#define H264_INTERLEAVED_PACKETIZATION_MODE 	2
#define H264_PACKETIZATION_MODE_UNSET 			9

/*
enum{ H264_PROFILE_BASELINE,H264_PROFILE_MAIN,H264_PROFILE_EXTENDED,
,MUST be last NUMBER_OF_H264_PROFILES};


static char*  h264_profile[NUMBER_OF_H264_PROFILES] = {
"Baseline Profile",  // 0
"Main Profile",	     // 1
"Extended Profile"   // 2
};


enum{ H264_LEVEL_1,H264_LEVEL_1_1,H264_LEVEL_1_2,H264_LEVEL_1_3,
	  H264_LEVEL_2,MUST be last NUMBER_OF_H264_LEVELS};


static char*  h264_level[NUMBER_OF_H264_LEVELS] = {
"Level 1",		   // 0
"Level 1.1",	   // 1
"Level 1.2",	   // 2
"Level 1.3",	   // 3
"Level 2"		   // 4
};


enum{ H264_PROFILE,H264_LEVEL,H264_CUSTOM_1,H264_CUSTOM_2,H264_CUSTOM_3,H264_CUSTOM_4
,MUST be last NUMBER_OF_H264_FIELDS};


*/
#endif
