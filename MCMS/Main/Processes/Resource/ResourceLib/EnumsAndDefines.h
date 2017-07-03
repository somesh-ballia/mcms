#ifndef ENUMSANDDEFINES_
  #define ENUMSANDDEFINES_

#include "AllocateStructs.h"

#define SMALL_ERROR                                             0.01
#define FLOATING_POINT_ERROR                                    0.00001
#define MAX_CONN_IDS                                            10000                                       // temp
#define MAX_MNTR_CONF_IDS                                       0x7FFFFFFF                                  // 10000 //temp
#define MAX_PARTY_SSRC_IDS                                      0x400                                       // SSRC ID = 10 bit
#define MAX_RSRC_PARTY_IDS                                      min(4096, RSRC_ALLOCATOR_MAX_RSRC_PARTY_ID) // updated from 1024 amos
#define MAX_ROOM_IDS                                            min(4096, RSRC_ALLOCATOR_MAX_RSRC_PARTY_ID) // TIP Cisco

// AmosCapacity constants
#define MAX_RSRC_CONF_IDS                                       min(831, RSRC_ALLOCATOR_MAX_CONNECTION_ID)  // min(416,RSRC_ALLOCATOR_MAX_CONNECTION_ID)

#define MAX_CHANNEL_IDS                                         360

#define MAX_PORTS                                               48
#define BOARDS_NUM                                              4

#define VSW_28_UNITS_NUM                                        7
#define VSW_56_UNITS_NUM                                        14

#define MEDIA_SUB_BOARD_NUM                                     1
#define RTM_SUB_BOARD_NUM                                       2

#define MAX_ART_CHANNELS_PER_ART                                150
#define NUM_ART_CHANNELS_FOR_SOFT_MCU_PARTY                     0

#define NO_AC_ID                                                0xFF
#define NO_DISPLAY_BOARD_ID                                     0xFFFF

#define ALL_BOARD_IDS                                           0xFF

#define MAX_PARTIES_FOR_RAM_HALF_SIZE                           400

#define TEMPORARY_BONDING_NUMBER_LENGTH                         7

#define ART_PROMILLES_BARAK                                     62.5
#define VID_ENC_CIF_PROMILLES_BARAK                             125
#define VID_DEC_CIF_PROMILLES_BARAK                             125
#define VID_TOTAL_CIF_PROMILLES_BARAK                           (VID_ENC_CIF_PROMILLES_BARAK + VID_DEC_CIF_PROMILLES_BARAK)

// tbd zoe - breeze
#define ART_LIGHT_PROMILLES_BREEZE                              25

// Maximum number of parties per system (can be video AVC/SVC or Audio only)
#define MAX_PARTIES_PER_SYSTEM_RMX1500                          360  // 1 MPMx card = 360 audio only
#define MAX_PARTIES_PER_SYSTEM_RMX2000                          720  // 2 MPMx cards X 360 audio only = 720 (MPM-Rx audio only capacity is lower)
#define MAX_PARTIES_PER_SYSTEM_RMX4000                          1440 // 4 MPMx cards X 360 audio only = 1440
#define MAX_PARTIES_PER_SYSTEM_NINJA                            300  // Maximum of 300 SVC/Audio only ports
#define MAX_PARTIES_PER_SYSTEM_SOFT_MCU                         600  // Maximum of 600 Audio only ports
#define MAX_PARTIES_PER_SYSTEM_SOFT_MFW                         2000 // Maximum of 2000 SVC only ports

//OLGA - SoftMCU
#define MAX_NUMBER_AVC_PARTICIPANTS_SOFT_MCU                    60   //In mix max AVC SD parties is 60
#define MAX_NUMBER_AVC_PARTICIPANTS_NINJA                       100
#define MAX_NUMBER_AVC_PARTICIPANTS_SOFT_CG                     120
#define MAX_NUMBER_VOICE_PARTICIPANTS_NINJA                     100

#define MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU                 30
#define MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU_8               5
#define MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU_12              8
#define MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU_16              10
#define MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU_24              15

#define MAX_NUMBER_HD_AVC_PARTIES_CALL_GENERATOR_SOFT_MCU       120

/*
 * 12 Dec 2012, Rafi Fellert, JIRA issue #46
 * softMCUMfw must support 600 SVC participants
 */
#define MAX_NUMBER_SVC_PARTICIPANTS_SOFT_MFW_HIGH               1000
#define MAX_NUMBER_SVC_PARTICIPANTS_SOFT_MFW_LOW                200
#define MAX_NUMBER_SVC_PARTICIPANTS_SOFT_DEMO                   50
#define MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW_8                   4
#define MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW_16                  14
#define MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW                     67

#define MAX_NUMBER_PARTICIPANTS_ICE_SOFT_MFW                    1000

#define ART_PROMILLES_SOFT_MCU                                  5
#define VID_ENC_CIF_PROMILLES_SOFT_MPMX                         3.333
#define VID_DEC_CIF_PROMILLES_SOFT_MPMX                         3.333
#define VID_ENC_SD_PROMILLES_SOFT_MPMX                          3.333
#define VID_DEC_SD_PROMILLES_SOFT_MPMX                          3.333
#define VID_ENC_HD_PROMILLES_SOFT_MPMX                          5
#define VID_DEC_HD_PROMILLES_SOFT_MPMX                          5
#define VID_TOTAL_HD720_PROMILLES_SOFT_MPMX                     (VID_ENC_HD_PROMILLES_SOFT_MPMX + VID_DEC_HD_PROMILLES_SOFT_MPMX)
#define VID_ENC_HD1080_30FS_PROMILLES_SOFT_MPMX                 16.66
#define VID_DEC_HD1080_30FS_PROMILLES_SOFT_MPMX                 16.66
#define VID_ENC_HD1080_60FS_PROMILLES_SOFT_MPMX                 33.32
#define VID_DEC_HD1080_60FS_PROMILLES_SOFT_MPMX                 33.32

#define ART_PROMILLES_SOFT_MFW                                  2
#define VIDEO_PROMILLES_SOFT_MFW                                1


#define MAX_NUMBER_PARTICIPANTS_NINJA                           100
#define MAX_NUMBER_SVC_PARTICIPANTS_NINJA                       300

#define ART_PROMILLES_SOFT_NINJA                                10
#define VIDEO_PROMILLES_SOFT_NINJA                              10

#define VID_ENC_CIF_SD_HD720_PROMILLES_NINJA                    5
#define VID_DEC_CIF_SD_HD720_PROMILLES_NINJA                    5

//Ninja mix mode, need to support 50 AVC + 100 SVC,  full capacity 1000 = 50 * ( 5 + 5 + 2.5 +2.5) + 100 * 2.5
#define VID_AVC_ENC_CIF_SD_MIX_MODE_PROMILLES_NINJA             2.5   //In AVC+SVC mix mode, the AVC->SVC encoder needed promilles
#define VID_SVC_DEC_SD_MIX_MODE_PROMILLES_NINJA                 2.5   //In AVC+SVC mix mode, the SVC->AVC decoder needed promilles
#define ART_PROMILLES_SOFT_CG                                   4.166 // maximum number of video ports supported in SoftMCU Call Generator - 120
#define VID_ENC_CIF_SD_PROMILLES_SOFT_CG                        4.166
#define VID_DEC_CIF_SD_PROMILLES_SOFT_CG                        4.166
#define VID_TOTAL_CIF_SD_PROMILLES_CG                           (VID_ENC_CIF_SD_PROMILLES_SOFT_CG + VID_DEC_CIF_SD_PROMILLES_SOFT_CG)
#define VID_ENC_HD720_30FS_PROMILLES_SOFT_CG                    4.166
#define VID_DEC_HD720_30FS_PROMILLES_SOFT_CG                    4.166
#define VID_TOTAL_HD720_30FS_PROMILLES_SOFT_CG                  (VID_ENC_HD720_30FS_PROMILLES_SOFT_CG + VID_DEC_HD720_30FS_PROMILLES_SOFT_CG)
#define VID_ENC_HD720_60FS_PROMILLES_SOFT_CG                    4.166
#define VID_DEC_HD720_60FS_PROMILLES_SOFT_CG                    4.166
#define VID_ENC_HD1080_30FS_PROMILLES_SOFT_CG                   4.166
#define VID_DEC_HD1080_30FS_PROMILLES_SOFT_CG                   4.166
#define VID_ENC_HD1080_60FS_PROMILLES_SOFT_CG                   4.166
#define VID_DEC_HD1080_60FS_PROMILLES_SOFT_CG                   4.166

// decission 21/03/2011 - open 9 ports per art in 4
#define ART_PROMILLES_BREEZE                                    111
#define VID_ENC_CIF_PROMILLES_BREEZE                            125
#define VID_DEC_CIF_PROMILLES_BREEZE                            125
#define VID_TOTAL_CIF_PROMILLES_BREEZE                          (VID_ENC_CIF_PROMILLES_BREEZE + VID_DEC_CIF_PROMILLES_BREEZE)
#define VID_ENC_H261_H263_PROMILLES_BREEZE                      250
#define VID_DEC_H261_H263_PROMILLES_BREEZE                      250
#define VID_ENC_SD30_PROMILLES_BREEZE                           250
#define VID_DEC_SD30_PROMILLES_BREEZE                           250
#define VID_ENC_CIF_SD30_PROMILLES_MODE3_BREEZE                 333 // For V8.2 ICE capacity issue, reduce the CIF/SD ports count to 45 per MPMx card when system cfg flag is ON - LIMIT_CIF_SD_PORTS_PER_MPMX_CARD
#define VID_DEC_CIF_SD30_PROMILLES_MODE3_BREEZE                 333 // For V8.2 ICE capacity issue, reduce the CIF/SD ports count to 45 per MPMx card when system cfg flag is ON - LIMIT_CIF_SD_PORTS_PER_MPMX_CARD
#define VID_DEC_H263_4CIF_PROMILLES_BREEZE                      500
#define VID_ENC_H263_4CIF_PROMILLES_BREEZE                      500
#define VID_TOTAL_SD30_PROMILLES_BREEZE                         (VID_ENC_SD30_PROMILLES_BREEZE + VID_DEC_SD30_PROMILLES_BREEZE)
#define VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE           500
#define VID_DEC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE           500
#define VID_TOTAL_HD720_30FS_PROMILLES_BREEZE                   (VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE + VID_DEC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE)
#define VID_ENC_HD720_60FS_PROMILLES_BREEZE                     1000
#define VID_DEC_HD720_60FS_PROMILLES_BREEZE                     1000
#define VID_ENC_HD1080_30FS_PROMILLES_BREEZE                    1000
#define VID_DEC_HD1080_30FS_PROMILLES_BREEZE                    1000
#define VID_COP_ENC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE       1000
#define VID_COP_HD1080_DEC_PROMILLES_BREEZE                     1000
#define VID_COP_CIF_ENC_PROMILLES_BREEZE                        250  // 125
#define VID_COP_CIF_DEC_PROMILLES_BREEZE                        125
#define VID_COP_CIF_DEC_PROMILLES_COP_BREEZE                    250  // Breeze-COP
#define VID_COP_CIF_DEC_PROMILLES_BREEZE_TEMP                   250
#define VID_COP_PCM_ENC_PROMILLES_BREEZE                        250  // Breeze-COP 500
#define VID_COP_VSW_ENC_PROMILLES_BREEZE                        125
#define VID_COP_VSW_DEC_PROMILLES_BREEZE                        125

// MPM-Rx ports promilles per accelerator
#define ART_PROMILLES_AUDIO_OR_SVC_ONLY_MPMRX                   50   // 300 SVC, Audio Only support: support 300 SVC ports (SVN only), 20 ports per unit
#define ART_PROMILLES_UP_TO_1MB_MPMRX                           71
#define ART_PROMILLES_ABOVE_1MB_UP_TO_4MB_MPMRX                 142

#define ART_PROMILLES_ABOVE_4MB_MPMRX                           250
#define ART_CAPACITY_FOR_MIX_MODE_TRANSLATORS_MPMRX             1024 // The maximum for AVC ART Mix-Mode (AVC+SVC conf) translators can be video out channel of SD, therefore we use 1MB as maximum artCapacity.
#define VID_ENC_CIF_PROMILLES_MPMRX                             125
#define VID_DEC_CIF_PROMILLES_MPMRX                             125
#define VID_TOTAL_CIF_PROMILLES_MPMRX                           (VID_ENC_CIF_PROMILLES_MPMRX + VID_DEC_CIF_PROMILLES_MPMRX)
#define VID_ENC_SD30_PROMILLES_MPMRX                            125
#define VID_DEC_SD30_PROMILLES_MPMRX                            125
#define VID_ENC_VSW_PROMILLES_MPMRX                             250
#define VID_DEC_VSW_PROMILLES_MPMRX                             250
#define VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_MPMRX            250
#define VID_DEC_HD720_30FS_SYMMETRIC_PROMILLES_MPMRX            250
#define VID_TOTAL_HD720_30FS_PROMILLES_MPMRX                    (VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_MPMRX + VID_DEC_HD720_30FS_SYMMETRIC_PROMILLES_MPMRX)
#define VID_ENC_HD720_60FS_PROMILLES_MPMRX                      500
#define VID_DEC_HD720_60FS_PROMILLES_MPMRX                      500
#define VID_ENC_HD1080_30FS_PROMILLES_MPMRX                     500
#define VID_DEC_HD1080_30FS_PROMILLES_MPMRX                     500
#define VID_ENC_HD1080_60FS_SYMMETRIC_PROMILLES_MPMRX           1000
#define VID_DEC_HD1080_60FS_SYMMETRIC_PROMILLES_MPMRX           1000
#define VID_ENC_H261_PROMILLES_MPMRX                            500
#define VID_DEC_H261_PROMILLES_MPMRX                            500

// There is a limitation of maximum number of Encoders per DaVinci chip (Embedded limitation).
// The number of decoders per chip is not limited and any combination according to capacity will be OK.
// Below are the defines describe each port weight, out of 12 total encoders per DaVinci chip.
#define VID_ENC_CIF_WEIGHT_MPMX                                 3   // currently only relevant for RMX1500Q
#define VID_ENC_CIF_WEIGHT_MODE2_MPMX                           3.6 // 3.6=12*0.3 because CIF consumes 30% in the regular mode
#define VID_ENC_SD30_WEIGHT_MPMX                                6   // currently only relevant for RMX1500Q
#define VID_ENC_SD30_WEIGHT_MODE2_MPMX                          4.8 // 4.8=12*0.4 because SD consumes 40% in the regular mode
#define VID_ENC_H263_4CIF_WEIGHT_MPMX                           12
#define VID_ENC_HD720_30FS_WEIGHT_MPMX                          12
#define VID_ENC_HD720_60FS_WEIGHT_MPMX                          12
#define VID_ENC_HD1080_30FS_WEIGHT_MPMX                         12
#define VID_ENC_HD1080_60FS_WEIGHT_MPMX                         12

// bandwidth
#define VID_ENC_CIF_BW_IN_BREEZE                                36
#define VID_ENC_CIF_BW_OUT_BREEZE                               0
#define VID_DEC_CIF_BW_IN_BREEZE                                0
#define VID_DEC_CIF_BW_OUT_BREEZE                               11

#define VID_ENC_SD30_BW_IN_BREEZE                               52
#define VID_ENC_SD30_BW_OUT_BREEZE                              0
#define VID_DEC_SD30_BW_IN_BREEZE                               0
#define VID_DEC_SD30_BW_OUT_BREEZE                              64
#define VID_DEC_360P_BW_OUT_BREEZE                              40

#define VID_ENC_HD720_30FS_SYMMETRIC_BW_IN_BREEZE               117
#define VID_ENC_HD720_30FS_SYMMETRIC_BW_OUT_BREEZE              0
#define VID_DEC_HD720_30FS_SYMMETRIC_BW_IN_BREEZE               0
#define VID_DEC_HD720_30FS_SYMMETRIC_BW_OUT_BREEZE              107
#define VID_ENC_HD720_60FS_SYMMETRIC_BW_IN_BREEZE               234
#define VID_ENC_HD720_60FS_SYMMETRIC_BW_OUT_BREEZE              0
#define VID_DEC_HD720_60FS_SYMMETRIC_BW_IN_BREEZE               0
#define VID_DEC_HD720_60FS_SYMMETRIC_BW_OUT_BREEZE              214

#define VID_ENC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE              207
#define VID_ENC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE             0
#define VID_DEC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE              0
#define VID_DEC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE             229

#define VID_COP_HD1080_DEC_BW_IN_BREEZE                         0
#define VID_COP_HD1080_DEC_BW_OUT_BREEZE                        229 // Breeze-COP 40
#define VID_COP_4CIF_ENC_BW_IN_BREEZE                           52
#define VID_COP_4CIF_ENC_BW_OUT_BREEZE                          0
#define VID_COP_CIF_ENC_BW_IN_BREEZE                            36
#define VID_COP_CIF_ENC_BW_OUT_BREEZE                           0
#define VID_COP_PCM_ENC_BW_IN_BREEZE                            207
#define VID_COP_PCM_ENC_BW_OUT_BREEZE                           0

#define VID_COP_VSW_ENC_BW_IN_BREEZE                            40
#define VID_COP_VSW_ENC_BW_OUT_BREEZE                           0
#define VID_COP_VSW_DEC_BW_IN_BREEZE                            0
#define VID_COP_VSW_DEC_BW_OUT_BREEZE                           40

// FPGA BW Limitation for 24 enc/dec support
#define TOTAL_POSTSCALER_BW_IN                                  400000000
#define TOTAL_POSTSCALER_BW_OUT                                 400000000
#define VID_HD_1080_60_BW_MPMRX_NINJA_ENC_DEC                   186624000
#define VID_HD_1080_30_BW_MPMRX_NINJA_ENC_DEC                   93312000
#define VID_HD_720_60_BW_MPMRX_NINJA_ENC_DEC                    82944000
#define VID_HD_720_30_BW_MPMRX_NINJA_ENC_DEC                    41472000
#define VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC                    26542080 // WSD=18662400, W4CIF=26542080
#define VID_CIF_BW_MPMRX_NINJA_ENC_DEC                          4561920

#define MAX_NUM_VIDEO_PARTICIPANTS_WITH_ART_ON_CARD             100

#define MAX_NUM_COP_CONF_ON_CARD                                4

#define NUM_T1_PORTS                                            23
#define NUM_E1_PORTS                                            30

#define PORTS_CONFIGURATION_STEP                                1      // Ports Configuration possible values "jump" in 1's steps
#define PORTS_CONFIGURATION_STEP_BREEZE                         1      // was 3
#define PORTS_CONFIGURATION_STEP_RMX_1500Q                      1

#define AUDIO_FACTOR                                            1      // 1 video (HD720) = 1 audio
#define AUDIO_FACTOR_BREEZE                                     12     // 1 video (HD720) = 12 audio
#define AUDIO_FACTOR_RMX_1500Q                                  12.857 // 1 video (HD720) = 12.857 audio
#define AUDIO_FACTOR_SOFTMCU                                    6      // 1 video (HD720) = 6 audio
#define AUDIO_FACTOR_SOFTMCU_MFW                                10     // 1 video (HD720) = 10 audio
#define AUDIO_FACTOR_SOFTMCU_NINJA                              2      // 1 video (HD720) = 2 audio

#define MAX_NUM_VIDEO_PREVIEW_PER_BOARD                         4
#define MAX_NUM_PCM_MENU_PER_BOARD_MPMX                         6

const int RECONFIGURE_UNITS_TIMER             = 1234;
const int RECOVERY_UNITS_TIMER                = 1235;
const int GET_PORTS_INFO_TIMER                = 1236;
const int RETRIEVE_INFO_TIMER                 = 1237;
const int RETRIEVE_OCCUPIED_UDP_PORTS_TIMER   = 1238;
const int SECONDS_FOR_RECONFIGURE_UNITS_TIMER = 20;             // VNGFE-4697 - Instead of 10 seconds
const int SECONDS_FOR_RECOVERY_UNITS_TIMER    = 40;             // VNGR-22871 - Instead of 20 seconds
const int GET_PORTS_INFO_TIMEOUT              = 3 * SECOND;
const int RETRIEVE_INFO_TIMEOUT               = 10 * SECOND;

#define PORT_WEIGHT_AUDIO_MPMRX                                 0.0833
#define PORT_WEIGHT_AUDIO_MPMRX_HW_LIMIT                        0.3333
#define PORT_WEIGHT_AUDIO_MPMRX_S_HW_LIMIT                      0.1
#define PORT_WEIGHT_CIF_MPMRX                                   0.5
#define PORT_WEIGHT_SD30_MPMRX                                  0.5
#define PORT_WEIGHT_HD720_MPMRX                                 1
#define PORT_WEIGHT_HD1080_MPMRX                                2
#define PORT_WEIGHT_HD1080_60_MPMRX                             4
#define PORT_WEIGHT_SVC_MPMRX                                   0.3333
#define PORT_WEIGHT_SVC_HD1080_MPMRX                            0.6666  //150 1080p SVC for MPMRx-D
#define PORT_WEIGHT_SVC_HD1080_MPMRX_S_HW_LIMIT                 0.3333  //90 1080p SVC for MPMRx-S

// Ports promilles per accelerator for traffic shaping support (MPMX and MPM-Rx)
#define ART_PROMILLES_UP_TO_1MB_MPMRX_TRAFFIC_SHAPING           90
#define ART_PROMILLES_ABOVE_1MB_UP_TO_4MB_MPMRX_TRAFFIC_SHAPING 250

#define ART_PROMILLES_CIF_MPMX_TRAFFIC_SHAPING                  142         // weight of 1/7
#define ART_PROMILLES_ABOVE_512MB_MPMX_TRAFFIC_SHAPING          200         // weight of 1/5
#define ART_PROMILLES_BREEZE_TRAFFIC_SHAPING                    111         // weight of 1/9

// Port weight for MPM-Rx, traffic shaping (MPM-X and MPM-Rx)
#define PORT_WEIGHT_CIF_MPMRX_HW_LIMIT_TRAFFIC_SHAPING          0.6666      // 150
#define PORT_WEIGHT_SD30_MPMRX_HW_LIMIT_TRAFFIC_SHAPING         0.6666      // 150
#define PORT_WEIGHT_CIF_MPM_X_TRAFFIC_SHAPING                   0.4285      // 70
#define PORT_WEIGHT_SD30_MPM_X_TRAFFIC_SHAPING                  0.6         // 50

#define PORT_WEIGHT_CIF_MIXED_MPMRX_HW_LIMIT                    1
#define PORT_WEIGHT_SD30_MIXED_MPMRX_HW_LIMIT                   1

#define PORT_WEIGHT_CIF_MIXED_MPMRX_HW_HALF_LIMIT               0.75
#define PORT_WEIGHT_SD30_MIXED_MPMRX_HW_HALF_LIMIT              0.75

#define PORT_WEIGHT_HD720_MIXED_MPMRX_HW_LIMIT                  1.5
#define PORT_WEIGHT_HD1080_MIXED_MPMRX_HW_LIMIT                 2.5
#define PORT_WEIGHT_HD1080_60_MIXED_MPMRX_HW_LIMIT              4.5
#define PORT_WEIGHT_SVC_MIXED_MPMRX_HW_LIMIT                    0.5
#define PORT_WEIGHT_AUDIO_MIXED_MPMRX_HW_LIMIT                  0.6666
#define PORT_WEIGHT_AUDIO_MIXED_MPMRX_S_HW_LIMIT                0.1999 // Should be 0.2, but changed to 0.1999 due to floating point error

#define PORT_WEIGHT_SVC_MPM_X                                   0.3333f
#define PORT_WEIGHT_SVC_MPM_X_HW_LIMIT                          0.3333f
#define PORT_WEIGHT_AUDIO_MPM_X                                 0.0833
#define PORT_WEIGHT_CIF_MPM_X                                   0.3333
#define PORT_WEIGHT_SD30_MPM_X                                  0.5
#define PORT_WEIGHT_CIF_SD30_LIMITED_MPM_X                      0.6666 // For V8.2 ICE capacity issue, reduce the CIF/SD ports count to 45 per MPMx card when system cfg flag is ON - LIMIT_CIF_SD_PORTS_PER_MPMX_CARD
#define PORT_WEIGHT_HD720_MPM_X                                 1
#define PORT_WEIGHT_HD1080_MPM_X                                2
#define PORT_WEIGHT_HD1080_60_MPM_X                             3
#define PORT_WEIGHT_COP_MPM_X                                   0.0833

#define PORT_WEIGHT_SVC_MIXED_MPM_X_HW_LIMIT                    0.3333
#define PORT_WEIGHT_CIF_MIXED_MPM_X_HW_LIMIT                    0.75
#define PORT_WEIGHT_SD30_MIXED_MPM_X_HW_LIMIT                   0.75
#define PORT_WEIGHT_HD720_MIXED_MPM_X_HW_LIMIT                  1.5
#define PORT_WEIGHT_HD1080_MIXED_MPM_X_HW_LIMIT                 3
#define PORT_WEIGHT_HD1080_60_MIXED_MPM_X_HW_LIMIT              5

#define PORT_WEIGHT_AUDIO_SOFT_MPM_X_RPP                        0.05   //30/600
#define PORT_WEIGHT_AUDIO_SOFT_MPM_X_A_LA_CART                  0.0833 //30/360
#define PORT_WEIGHT_SVC_SOFT_MPM_X_RPP                          0.0666 //30/450
#define PORT_WEIGHT_SVC_SOFT_MPM_X_A_LA_CART                    0.3333 //30/90
#define PORT_WEIGHT_SVC_1080_SOFT_MPM_X_RPP                     0.1    //30/300
#define PORT_WEIGHT_SVC_1080_SOFT_MPM_X_A_LA_CART               0.3333 //30/90
#define PORT_WEIGHT_CIF_SOFT_MPM_X                              0.5    //30/60
#define PORT_WEIGHT_SD30_SOFT_MPM_X                             0.5    //30/60
#define PORT_WEIGHT_HD720_SOFT_MPM_X                            1
#define PORT_WEIGHT_HD1080_SOFT_MPM_X                           2      //30/15
#define PORT_WEIGHT_HD1080_P60_SOFT_MPM_X                       3      //30/10

#define PORT_WEIGHT_AUDIO_MIXED_SOFT_MPM_X                      0.1    //30/300
#define PORT_WEIGHT_SVC_MIXED_SOFT_MPM_X_RPP                    0.1    //30/300
#define PORT_WEIGHT_SVC_MIXED_SOFT_MPM_X_A_LA_CART              0.3333 //30/90
#define PORT_WEIGHT_CIF_MIXED_SOFT_MPM_X                        0.5    //30/60
#define PORT_WEIGHT_SD30_MIXED_SOFT_MPM_X                       0.5    //30/60
#define PORT_WEIGHT_HD720_MIXED_SOFT_MPM_X                      1      //30/30
#define PORT_WEIGHT_HD1080_MIXED_SOFT_MPM_X                     2      //30/15
#define PORT_WEIGHT_HD1080_P60_MIXED_SOFT_MPM_X                 3      //30/10

#define PORT_WEIGHT_AUDIO_SOFT_MFW                              0.5    // 2000 SAC ports (when license is 100 HD720 ports)
//#define PORT_WEIGHT_SVC_SOFT_MFW                              1  // 1000 SVC ports (when license is 100 HD720 ports)
#define PORT_WEIGHT_CIF_SOFT_MFW                                0.2
#define PORT_WEIGHT_SD30_SOFT_MFW                               0.4
#define PORT_WEIGHT_HD720_SOFT_MFW                              1
#define PORT_WEIGHT_HD1080_SOFT_MFW                             2

#define PORT_WEIGHT_AUDIO_MIXED_SOFT_MFW                        0.0335 //0.1675 // port_weight = license/num_ports, i.e. 67/400=0.1675 (according to SRS)
#define PORT_WEIGHT_CIF_MIXED_SOFT_MFW                          0.067  //0.2392 // port_weight = license/num_ports, i.e. 67/280
#define PORT_WEIGHT_SD30_MIXED_SOFT_MFW                         0.134  //0.3045 // port_weight = license/num_ports, i.e. 67/220
#define PORT_WEIGHT_HD720_MIXED_SOFT_MFW                        0.335  //0.5153 // port_weight = license/num_ports, i.e. 67/130
#define PORT_WEIGHT_HD1080_MIXED_SOFT_MFW                       0.67   //0.8375 // port_weight = license/num_ports, i.e. 67/80

#define PORT_WEIGHT_SVC_AUDIO_SOFT_MFW                          0.0335 // port_weight = license/num_ports, i.e. 67/2000
#define PORT_WEIGHT_SVC_CIF_SOFT_MFW                            0.067  // port_weight = license/num_ports, i.e. 67/1000
#define PORT_WEIGHT_SVC_SD30_SOFT_MFW                           0.134  // port_weight = license/num_ports, i.e. 67/500
#define PORT_WEIGHT_SVC_HD720_SOFT_MFW                          0.335  // port_weight = license/num_ports, i.e. 67/200
#define PORT_WEIGHT_SVC_HD1080_SOFT_MFW                         0.67   // port_weight = license/num_ports, i.e. 67/100

#define PORT_WEIGHT_SVC_AUDIO_MIXED_SOFT_MFW                    0.0335 //0.1675
#define PORT_WEIGHT_SVC_CIF_MIXED_SOFT_MFW                      0.067  //0.2392
#define PORT_WEIGHT_SVC_SD30_MIXED_SOFT_MFW                     0.134  //0.3045
#define PORT_WEIGHT_SVC_HD720_MIXED_SOFT_MFW                    0.335  //0.5153
#define PORT_WEIGHT_SVC_HD1080_MIXED_SOFT_MFW                   0.67   //0.8375

/////////////////////////
//            Ninja Section
////////////////////////

// Part 1: Non Mix AVC
#define PORT_WEIGHT_AUDIO_SOFT_NINJA                            0.0833
#define PORT_WEIGHT_AUDIO_SOFT_NINJA_HW_LIMIT_1_CARD            0.1166 // 300 audio only, max license=35
#define PORT_WEIGHT_AUDIO_SOFT_NINJA_HW_LIMIT_3_CARDS           0.3333 // 300 audio only, max license=100

#define PORT_WEIGHT_CIF_SD_SOFT_NINJA                           0.5
#define PORT_WEIGHT_HD720P30_SOFT_NINJA                         1      // 100
#define PORT_WEIGHT_HD1080P30_SOFT_NINJA                        2
#define PORT_WEIGHT_HD1080P60_SOFT_NINJA                        4

// Part 1: Non Mix AVC

// Part 2: Non Mix SVC
#define PORT_WEIGHT_SVC_SOFT_NINJA                              0.3333
#define PORT_WEIGHT_SVC_HD1080_SOFT_NINJA_HW_LIMIT_1_CARD       0.3333    //150 1080p SVC, max license= 35
#define PORT_WEIGHT_SVC_HD1080_SOFT_NINJA_HW_LIMIT_3_CARDS      0.6666    //150 1080p SVC, max license= 100

// Part 3: Mix AVC
#define PORT_WEIGHT_AUDIO_MIXED_SOFT_NINJA                      0.0833
#define PORT_WEIGHT_AUDIO_MIXED_SOFT_NINJA_1_CARD               0.175  // 200 audio only, max license=35
#define PORT_WEIGHT_AUDIO_MIXED_SOFT_NINJA_3_CARDS              0.5    // 200 audio only, max license=100

#define PORT_WEIGHT_CIF_MIXED_SOFT_NINJA                        0.75
#define PORT_WEIGHT_CIF_MIXED_SOFT_NINJA_1_CARD                 0.75
#define PORT_WEIGHT_CIF_MIXED_SOFT_NINJA_3_CARDS                1      // 100 CIF, max license=100

#define PORT_WEIGHT_SD_MIXED_SOFT_NINJA                         0.75
#define PORT_WEIGHT_SD_MIXED_SOFT_NINJA_1_CARD                  0.75
#define PORT_WEIGHT_SD_MIXED_SOFT_NINJA_3_CARDS                 1      // 100 SD, max license=100

#define PORT_WEIGHT_HD720P30_MIXED_SOFT_NINJA                   1.5
#define PORT_WEIGHT_HD1080P30_MIXED_SOFT_NINJA                  2.5
#define PORT_WEIGHT_HD1080P60_MIXED_SOFT_NINJA                  4.5

// Part 4: Mix SVC
#define PORT_WEIGHT_SVC_MIXED_SOFT_NINJA                        0.3333
#define PORT_WEIGHT_SVC_MIXED_SOFT_NINJA_1_CARD                 0.388  // 90 720P SVC, max license=35
#define PORT_WEIGHT_SVC_MIXED_SOFT_NINJA_3_CARDS                0.5    // 200 720P SVC, max license=100

#define PORT_WEIGHT_SVC_HD1080_MIXED_SOFT_NINJA_1_CARD          0.388  // 90 720P SVC, max license=35
#define PORT_WEIGHT_SVC_HD1080_MIXED_SOFT_NINJA_3_CARDS         0.666  // 150 1080p SVC, max license=100

#define PORT_WEIGHT_SVC_1500Q                                   0.28f
#define PORT_WEIGHT_SVC_1500Q_HW_LIMIT                          0.28f
#define PORT_WEIGHT_AUDIO_1500Q                                 0.077
#define PORT_WEIGHT_CIF_1500Q                                   0.28
#define PORT_WEIGHT_SD30_1500Q                                  0.5
#define PORT_WEIGHT_HD720_1500Q                                 1
#define PORT_WEIGHT_HD1080_1500Q                                2.3333
#define PORT_WEIGHT_COP_1500Q                                   0.077

#define PORT_WEIGHT_AUDIO_SOFT_CG                               1
#define PORT_WEIGHT_CIF_SOFT_CG                                 1
#define PORT_WEIGHT_SD30_SOFT_CG                                1
#define PORT_WEIGHT_HD720_SOFT_CG                               1
#define PORT_WEIGHT_HD1080_SOFT_CG                              1
#define PORT_WEIGHT_HD1080_60_SOFT_CG                           1

// License factors
#define LICENSE_FACTOR_SLAVE_IN_CIF_SD                          0.6666
#define LICENSE_FACTOR_SLAVE_IN_CIF_SD_SOFT                     0.5
#define LICENSE_FACTOR_SLAVE_IN_CIF_MPMX                        1
#define LICENSE_FACTOR_SLAVE_IN_HD                              0.5
#define LICENSE_FACTOR_SLAVE_OUT_HD                             0.5
#define LICENSE_FACTOR_SLAVE_OUT_HD_MPMX                        1
#define LICENSE_FACTOR_SLAVE_OUT_CIF_SD_MPMX                    1
#define LICENSE_FACTOR_SLAVE_OUT_CIF_SD_SOFT                    0.5
#define LICENSE_FACTOR_SLAVE_OUT_CIF_SD_MPMRX_NINJA             0.6666

#define PARTS_PER_MILLION                                       1000000
#define MIN_PPM_FLOAT_VALUE                                     0.001

#define NUM_OF_UNIT_PER_BOARD_NINJA                             6

enum eUnitType
{
	eUnitType_Generic,
	eUnitType_Rtm,
	eUnitType_Art,
	eUnitType_Art_Control,
	eUnitType_Video,
	eUnitType_Max
};

inline const char* to_string(eUnitType val)
{
	static const char* enumNamesUnitType[] =
	{
		"eUnitType_Generic",
		"eUnitType_Rtm",
		"eUnitType_Art",
		"eUnitType_Art_Control",
		"eUnitType_Video"
	};
	return (eUnitType_Generic <= val && val < eUnitType_Max) ? enumNamesUnitType[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eUnitType val) { return os << to_string(val); }

enum eRTMSpanType
{
  SPAN_GENERIC = 0,
  TYPE_SPAN_T1 = 1,
  TYPE_SPAN_E1 =2
};

enum ePortType
{
	PORT_GENERIC   = 0,
	PORT_ART       = 1,
	PORT_ART_LIGHT = 2,
	PORT_VIDEO     = 3,
	NUM_OF_PORT_TYPES
};

inline const char* to_string(ePortType val)
{
	static const char* enumNamesPortType[] =
	{
		"PORT_GENERIC",
		"PORT_ART",
		"PORT_ART_LIGHT",
		"PORT_VIDEO",
		"INVALID"
	};
	return (PORT_GENERIC <= val && val < NUM_OF_PORT_TYPES) ? enumNamesPortType[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, ePortType val) { return os << to_string(val); }

// startup resources enabled, mfa complete, ivr check
enum eCardStartup
{
  MFA_COMPLETE = 0,
  COND_NUM     = 1,
};

enum eStartupCondType
{
  eIpService = 0,
  eRsrcEnough,
  eMeetingInd,

  NumOfStartupCondTypes
};


////////////////////////////////////////////////////////////////////////////
//                        eConfModeTypes
////////////////////////////////////////////////////////////////////////////
enum eConfModeTypes
{
	eNonMix,
	eMix,
	NUM_OF_CONF_MODE_TYPES
};

inline const char* to_string(eConfModeTypes val)
{
	static const char* enumNamesConfModeTypes[] =
	{
		"eNonMix",
		"eMix"
	};
	return (eNonMix <= val && val < NUM_OF_CONF_MODE_TYPES) ? enumNamesConfModeTypes[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eConfModeTypes val) { return os << to_string(val); }


////////////////////////////////////////////////////////////////////////////
//                        ePartyType
////////////////////////////////////////////////////////////////////////////
enum ePartyType
{
	ePartyType_Avc,
	ePartyType_Svc,
	ePartyType_Max
};

inline const char* to_string(ePartyType val)
{
	static const char* enumNamesPartyType[] =
	{
		"ePartyType_Avc",
		"ePartyType_Svc"
	};
	return (ePartyType_Avc <= val && val < ePartyType_Max) ? enumNamesPartyType[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, ePartyType val) { return os << to_string(val); }


////////////////////////////////////////////////////////////////////////////
//                        ePartyResourceTypes
////////////////////////////////////////////////////////////////////////////
enum ePartyResourceTypes
{
	e_Audio,
	e_Cif,
	e_SD30,
	e_HD720,
	e_HD1080p30,
	e_HD1080p60,
	NUM_OF_PARTY_RESOURCE_TYPES
};

inline const char* to_string(ePartyResourceTypes val)
{
	static const char* enumNamesPartyResourceTypes[] =
	{
		"eAudio",
		"eCif",
		"eSD30",
		"eHD720",
		"eHD1080p30",
		"eHD1080p60"
	};
	return (e_Audio <= val && val < NUM_OF_PARTY_RESOURCE_TYPES) ? enumNamesPartyResourceTypes[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, ePartyResourceTypes val) { return os << to_string(val); }


////////////////////////////////////////////////////////////////////////////
//                        eResourceAllocationTypes
////////////////////////////////////////////////////////////////////////////
enum eResourceAllocationTypes
{
	eNoMode,
	eFixedBreezeMode, // Breeze fixed
	eAutoBreezeMode,  // Breeze flexible
	eFixedMpmRxMode,  // MPM-Rx fixed
	eAutoMpmRxMode,   // MPM-Rx flexible (auto)
	eAutoMixedMode,   // MPM-Rx + MPMx flexible
	NUM_OF_RESOURCE_ALLOCATION_TYPES
};

inline const char* to_string(eResourceAllocationTypes val)
{
	static const char* enumNamesResourceAllocationTypes[] =
	{
		"eNoMode",
		"eFixedMPMxMode (MPMx fixed)",
		"eAutoMPMxMode (MPMx flexible)",
		"eFixedMpmRxMode (MPM-Rx fixed)",
		"eAutoMpmRxMode (MPM-Rx flexible)",
		"eAutoMixedMode (MPM-Rx + MPMx flexible)"
	};
	return (eNoMode <= val && val < NUM_OF_RESOURCE_ALLOCATION_TYPES) ? enumNamesResourceAllocationTypes[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eResourceAllocationTypes val) { return os << to_string(val); }


////////////////////////////////////////////////////////////////////////////
//                        eRereserveType
////////////////////////////////////////////////////////////////////////////
enum eRereserveType
{
	eRereserveAll,
	eRereserveFaulty,
	eRereserveNotFaulty,
	eRereserve_Max
};

inline const char* to_string(eRereserveType val)
{
	static const char* enumNamesRereserveType[] =
	{
		"eRereserveAll",
		"eRereserveFaulty",
		"eRereserveNotFaulty",
	};
	return (eRereserveAll <= val && val < eRereserve_Max) ? enumNamesRereserveType[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eRereserveType val) { return os << to_string(val); }


////////////////////////////////////////////////////////////////////////////
//                        ePhoneAllocationTypes
////////////////////////////////////////////////////////////////////////////
enum ePhoneAllocationTypes
{
	ePhoneAlloc = 0,
	ePhoneCheck,
	ePhoneCheckAlloc,
	ePhoneAllocation_Max
};

inline const char* to_string(ePhoneAllocationTypes val)
{
	static const char* enumNamesPhoneAllocationTypes[] =
	{
		"ePhoneAlloc",
		"ePhoneCheck",
		"ePhoneCheckAlloc",
	};
	return (ePhoneAlloc <= val && val < ePhoneAllocation_Max) ? enumNamesPhoneAllocationTypes[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, ePhoneAllocationTypes val) { return os << to_string(val); }

enum eRPMode
{
	eRPRegularMode = 0,
	eRPMasterMode,
	eRPSlaveMode,
	eRPMode_Max
};

inline const char* to_string(eRPMode val)
{
	static const char* enumNamesRPMode[] =
	{
		"eRPRegularMode",
		"eRPMasterMode",
		"eRPSlaveMode",
	};
	return (eRPRegularMode <= val && val < eRPMode_Max) ? enumNamesRPMode[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eRPMode val) { return os << to_string(val); }

inline const char* to_string(eLogicalResourceTypes val)
{
	return ::LogicalResourceTypeToString(val);
}

inline std::ostream& operator <<(std::ostream& os, eLogicalResourceTypes val) { return os << to_string(val); }



typedef WORD BoxID;
typedef WORD BoardID;
typedef WORD SubBoardID;
typedef WORD DisplayBoardID;
typedef WORD UnitID;
typedef WORD PortID;
typedef WORD AcceleratorID;
typedef WORD SpanID;
typedef WORD ChannelID;
typedef WORD ServiceID;
typedef WORD SubServiceID;

#endif /*ENUMSANDDEFINES_*/
