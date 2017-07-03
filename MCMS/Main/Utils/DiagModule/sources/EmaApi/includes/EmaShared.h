/*============================================================================*/
/*            Copyright ?? 2006 Polycom Israel,Ltd. All rights reserved        */
/*----------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary        */
/* information of Polycom Israel, Ltd. and is protected by law.               */
/* It may not be copied or distributed in any form or medium, disclosed  to   */
/* third parties, reverse engineered or used in any manner without prior      */
/* written authorization from Polycom Israel Ltd.                             */
/*----------------------------------------------------------------------------*/
/* FILE:     	EmaShared.h                                                   */
/* PROJECT:  	Switch Card - Ema API Module								  */
/* PROGRAMMER:  Eyal Ben-Sasson												  */
/* DESCRIPTION: Shared Information Between EMA ,Switch CM, Switch Ipmi,    	  */
/* 				Card Diagnostics & Lan Statistics Modules.		              */
/*----------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                            */
/*----------------------------------------------------------------------------*/
/*         |                 |                                       		  */
/*============================================================================*/

#ifndef EMASHARED_H_
#define EMASHARED_H_

#define MAX_PARAMS_IN_STRING	1000
#define MAX_STRING_PARAM    	24
#define MAX_DESC_STR_SIZE 		100

#define LISTEN_MFA_RESET_PORT		9020
#define LISTEN_MFA_DIAG_PORT		9040
#define LISTEN_EMA_SIM_PORT			9050
#define LISTEN_IPMI_PORT   	    	9060
#define LISTEN_LAN_STAT_PORT   		9080
#define LISTEN_EMA_PORT   			9090
#define LISTEN_SWITCH_DIAG_PORT    	9095
#define LISTEN_CPU_DIAG_PORT    	3333
#define MAX_DSP_CARD_SLOT_NUM		3
#define MAX_DSPUNIT_ON_CARD_NUM		6
#define MAX_DSPUNIT_ON_ISDN_CARD_NUM		1
#define ISDN_DSP_LOCATION		18

typedef enum
{
	eEmaStatOk,
	eEmaStatFail,
}eEmaStatus;


enum EVarType
{
  e_unsignedChar,
  e_signedChar,
  e_unsignedShort,
  e_signedShort,
  e_unsignedLong,
  e_signedLong,
  e_string
};

enum ESlotIds_Ninja
{
      CNTL_SLOT_ID = 4
    , RISER_SLOT_ID = 5
    , DSP_CARD_SLOT_ID_0=6
    , DSP_CARD_SLOT_ID_1=7
    , DSP_CARD_SLOT_ID_2=8
    , ISDN_CARD_SLOT_ID = 15
    , FANS_SLOT_ID = 21
    , PWRS_SLOT_ID = 22
    , LAN_SLOT_ID_START = 31
    , IPMI_SLOT_ID_RESET = -1
    , IPMI_SLOT_ID_SHUTDOWN = -2
};


enum ESlotIds_2000
{
	e_slotIdAll_2000=0,
	e_slotIdMfa1_2000=1,
	e_slotIdMfa2_2000=2,
	e_slotIdCpu1_2000=3,
	e_slotIdCpu2_2000=4,
	e_slotIdSwitch_2000=5,
	e_slotIdBackplane_2000=20,
	e_slotIdFan_2000=21,
	e_slotIdPSupply_2000=22,
    e_slotIdMax_2000=23
};

//[Dotan.H 11/01/2010 Support Yona RMX]
enum ESlotIds_1500
{
	e_slotIdAll_1500       = 0 ,
	    e_slotIdMPMY_1500      = 1 ,
	//e_slotIdMfa1_1500      = 1 ,
	//e_slotIdMfa2_1500      = 2 ,
    //e_slotIdMfa3_1500      = 3 ,

    //e_slotIdShoval_1500    = 5 ,
	//e_slotIdCpu2_1500      = 6 ,
    //e_slotIdLogo_1500	   = 7 ,
	e_slotIdCpu1_1500      = 8 ,
    e_slotIdPSupply_1500  = 9 ,
    //e_slotIdPSupply2_1500  = 10 ,
    //e_slotIdPSupply3_1500  = 11 ,
    e_slotIdFan_1500       = 12 ,
    //e_slotIdRtmIsdn1_1500  = 13 ,
    //e_slotIdRtmIsdn2_1500  = 14 ,
    //e_slotIdRtmIsdn3_1500  = 15 ,
    e_slotIdRtmIsdn_1500   = 16 ,
    e_slotIdSwitch_1500    = 17 ,
    //e_slotIdPolystar_1500  = 18 ,
	e_slotIdBackplane_1500 = 20 , // temp /*RC*/
	//e_slotIdPSupply_1500   = 22 , // temp /*RC*/
    //e_slotIdMax_1500       = 23 // temp /*RC*/
};

enum ESlotIds_4000
{
	e_slotIdAll_4000       = 0 ,
	e_slotIdMfa1_4000      = 1 ,
	e_slotIdMfa2_4000      = 2 ,
    e_slotIdMfa3_4000      = 3 ,
    e_slotIdMfa4_4000      = 4 ,
    e_slotIdShoval_4000    = 5 ,
	e_slotIdCpu2_4000      = 6 ,
    e_slotIdLogo_4000	   = 7 ,
	e_slotIdCpu1_4000      = 8 ,
    e_slotIdPSupply1_4000  = 9 ,
    e_slotIdPSupply2_4000  = 10 ,
    e_slotIdPSupply3_4000  = 11 ,
    e_slotIdFan_4000       = 12 ,
    e_slotIdRtmIsdn1_4000  = 13 ,
    e_slotIdRtmIsdn2_4000  = 14 ,
    e_slotIdRtmIsdn3_4000  = 15 ,
    e_slotIdRtmIsdn4_4000  = 16 ,
    e_slotIdSwitch_4000    = 17 ,
    e_slotIdPolystar_4000  = 18 ,
	e_slotIdBackplane_4000 = 20 , // temp /*RC*/
	e_slotIdPSupply_4000   = 22 , // temp /*RC*/
    e_slotIdMax_4000       = 23 // temp /*RC*/
};


typedef struct _tSpecGnrlHdr
{
	UINT32 ulMsgOffset;
}TSpecGnrlHdr,*PTSpecGnrlHdr;


typedef struct SStructToStr
{
	UINT32 	varType;
	UINT32 	varCount;
	UINT32 	jumpOffst;
	INT8 	varString[32];
}TStructToStr,*PTStructToStr;


typedef struct _tEmaReqHeader
{
	UINT32 ulOpcode;
	UINT32 ulMsgID;
	UINT32 ulSlotID;
}TEmaReqHeader,*PTEmaReqHeader;

typedef struct _tEmaDebug
{
	UINT32 ulOpcode;
	UINT8 ulMsgID[80];
	
}TEmaDebug;

typedef struct _tEmaIndHeader
{
	UINT32 ulOpcode;
	UINT32 ulMsgID;
	UINT32 ulSlotID;
	UINT32 ulStatus;
	UINT8  acDesc[MAX_DESC_STR_SIZE];
//	UINT32 ulNumOfElements;
}TEmaIndHeader,*PTEmaIndHeader;

//PAVELK START:
#define	MAX_CTRL_DATA_SIZE	10
typedef struct 
{
		int socket;
		int	isActive;
		char isDataReady;
		char ctrlData[MAX_CTRL_DATA_SIZE + 1];
}boardConnectInfo;

#define	AMOS_BOARD_COUNT 4	
#define	SWITCH_COORDINATOR_PORT	35924
#define	SHOVAL_MAX_SINGLE_OUTPUT_MBPS	1100000//1120000//1000000//800000///1000000
#define	AMOS_BOARD_NUMBER_1_IP	"169.254.128.67"
#define	AMOS_BOARD_NUMBER_2_IP	"169.254.128.68"
#define	AMOS_BOARD_NUMBER_3_IP	"169.254.128.69"
#define	AMOS_BOARD_NUMBER_4_IP	"169.254.128.70"


enum	switchBarakProtocol
{
	eMcastSwitchAskBarakToPrepare,
	eMcastBarakAckReceiptOfRequest,
	eMcastBarakReadyForTest,
	eMcastSwitchAskBarakStartTest,
	eMcastBarakTestStarted,
	eMcastBarakSuccessTest,
	eMcastBarakFailTest,
	eMcastSwitchBarakEndTest,
	eMcastSwitchBarakKeepAlive,
	eMcastBarakKeepAliveAck,
	eMcastLastMessage,
	
	eLantestSwitchReqBarak,			//ask baraks to prepare
	eLantestBarakAckReceipt,		//baraks ack
	eLantestBarakReadyForTest,		// now waiting for baraks to be ready for test -> baraks are ready
	eLantestSwitchEndTest,			//tell barak, that you are finished
	
	eLantestBarakRequestedLantest,	//barak requested lan test
	eLantestSwitchAckBarakRequest,	//ack the receipt of message
	eLantestSwitchReadyForBarakTest,	//allow lan test - iperf is not busy at moment
	eLantestBarakFinishedLanTest,	//Barak is finished. Continue with normal operation
	
	eLantestLastMessage,
	
	
	eUnicastTestBarakRequest,		//barak --> Switch -request unicast 
	eUnicastTestBarakOkToStart,	     //  barak <-- Switch - Ok,start the unicast. Sent when switch is ready
	eUnicastTestBarakFinished,		//barak --> Switch - finished unicast test
		
};

enum	mCastTestBoardStatus
{
	eBoardPreTest,
	eBoardAckedTestReq,
	eBoardReadyForTest,
	eBoardRunningMcastTest,
	eBoardFailedMcastTest,
	eBoardSuccessMcastTest,
	
	//eBoardSw2ButtonRunAllTestsPressed = 0x9a,//special message.  taking usage of tcp connection in amos
};

//5 seconds. All immediate acks
#define	SWITCH_TIMEOUT_MS_FOR_ACK	5000
//30 minutes - time for barak board to finish what it is doing,and switch to multicast test
#define	SWITCH_TIMEOUT_MS_OK_TO_START	1800000

//The test should take no longer that 150seconds (60seconds is normal test)
#define	MULTICAST_TEST_TIME		150000
#endif /*EMASHARED_H_*/
