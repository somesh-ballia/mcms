
/*===================================================================================================================*/
/*            Copyright     �    2005  Polycom Israel, Ltd. All rights reserved                                      */
/*-------------------------------------------------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary   information of                              */
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form                    */
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without                          */
/* prior written authorization from Polycom Israel Ltd.                                                              */
/*-------------------------------------------------------------------------------------------------------------------*/
/* FILE:            general.h                                                                                          */
/* PROJECT:         GIDEON                                                                                           */
/* PROGRAMMER:      Gadit Aviram                                              .                                      */
/* FUNCTION LIST:                                                                                                    */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Who             |      Date           |         Description                                                       */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Gadit Aviram    |      22/03/2005     |                                                                           */
/*===================================================================================================================*/

#ifndef  GENERAL_H 
#define  GENERAL_H

extern int NUM_OF_BOARDS_LOOP;
#define NUMBER_OF_LIGHT_PORTS_IN_DSP	20
#define NUMBER_OF_ART_PORTS_IN_DSP		10
#define NUMBER_OF_VIDEO_PORTS_IN_DSP	2
#define NUMBER_OF_PORTS_IN_RTM			360
#define NUMBER_OF_SPANS_IN_RTM			12
#define NUMBER_OF_PORTS_IN_SPAN			30


#define NUMBER_OF_VOIP_CONFERENCES_IN_BOARD		520
#define NUMBER_OF_VSW_CONFERENCES_IN_BOARD		260
#define NUMBER_OF_CP_CONFERENCES_IN_BOARD		40

#define MAX_NUM_OF_BOARDS_IN_SYSTEM				100	
#define MAX_NUM_OF_BOARDS_IN_BOX				10	

//Shnaps needs to update these definitions

#define NUM_OF_SUB_BOARDS								2	
#define NUM_OF_BOARDS									4

	
#define NUM_OF_RTMS_IN_BOARD							1
#define NUM_OF_PQ3										2	
#define BOARD_LINK_DSP_NUM								0		// the DSP num which is located at link 4 - PCI switch
#define MAX_NUM_OF_DSPS_CONTROLLED						26


#define NUM_OF_DSPS_IN_IP_MEDIA_MEMORY_BLOCK			4
#define NUM_OF_DSPS_IN_IVR_MEMORY_BLOCK					5

//Shnaps needs to update these definitions
#define CM_UNIT_ID						0	
#define SECOND_PQ3_UNIT_ID				99	


#define BOX_NUM							1		//need to be extracted from the IPMC
#define BOARD_NUM						1		//need to be extracted from the IPMC

#define MAIN_SUB_BOARD					1	
#define SECONDARY_SUB_BOARD				2	

#define AC_BOARD_NUM					1		//need to be extracted from the IPMC
#define AUDIO_CONTROLLER_DSP_NUM		11		// the DSP where the AC is located


#define FREE		0
#define OCCUPIED	1


//#define ENTRY_QUEUE	0xffffffff
#define STATUS_UNKNOWN_YET			-1

#endif
