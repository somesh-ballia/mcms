//+========================================================================+
//                       H323CapabilityInfo.h							   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323CapabilityInfo.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: GuyD                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |23/08/05     |                                                     |
//+========================================================================+
#ifndef __H323CapabilityInfo
#define __H323CapabilityInfo

// country code & manufacturer code for NonStandard cap
#define NS_T35COUNTRY_CODE_USA      0xB5
#define NS_T35EXTENSION_USA         0x00
#define NS_MANUFACTURER_POLYCOM     0x2331
#define NS_MANUFACTURER_PICTURETEL  0x0001
#define NS_CAP_ACCORD_SENDER        0x6C
#define NS_T35COUNTRY_CODE_NORWAY   0x82
#define NS_T35EXTENSION_NORWAY      0x01
#define NS_MANUFACTURER_NORWAY		0x100
#define NO_NS_T35COUNTRY			0x0
#define NO_NS_T35EXTENSION			0x0
#define NO_NS_MANUFACTURER			0x0

#define  NS_CAP_H323_H263_ANNEX_I	    0x40 //according to a mail from Dave Hein: "The code 0x40 is used to signal Annex I at all resolutions"
#define  NS_CAP_H323_H263_QCIF_ANNEX_I  0x40
#define  NS_CAP_H323_H263_CIF_ANNEX_I   0x41
#define  NS_CAP_H323_H263_4CIF_ANNEX_I  0x42
#define  NS_CAP_H323_H263_QCIF_ANNEX_T  0x44
#define  NS_CAP_H323_H263_CIF_ANNEX_T   0x45
#define  NS_CAP_H323_H263_4CIF_ANNEX_T  0x46
#define  NS_CAP_H323_HIGH_CAPACITY      0x54
#define  NS_CAP_H323_VISUAL_CONCERT_PC  0x5A
#define  NS_CAP_H323_VGA_800X600        0x61
#define  NS_CAP_H323_VGA_1024X768       0x63
#define  NS_CAP_H323_VGA_1280X1024      0x67
#define  NS_CAP_H323_VISUAL_CONCERT_FX  0x72
#define  NS_CAP_H323_VIDEO_STREAMS_2    0x73

// Polycom H26L
#define Polycom_H26L		  (BYTE)0x9E
#define Polycom_H26L_Profile  (BYTE)0x01
#define H26L_ONDATA_LOCATION		  0
#define H26L_PROFILE_ONDATA_LOCATION  0
#define H26L_MPI_ONDATA_LOCATION	  1
#define H26L_DUAL_MPI_ONDATA_LOCATION 2

typedef struct{
	int					capTypeCode;
	int					capLength;
}capBufferBaseApi522;

typedef struct{
	int					capTypeCode;
	int					capLength;
	unsigned char		dataCap[1];
}capBufferApi522;

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

typedef struct{
	ctCapStruct			header;
	long				value;   
}simpleAudioCapStruct;

static const char *g_roleTokenOpcodeStrings[] = 
{
	"RoleTokenFirstOpcodeReq",  //0 
	"RoleProviderIdentityReq",	//1
	"NoRoleProviderReq",		//2 
	"RoleTokenAcquireReq",		//3
	"RoleTokenWithdrawReq",		//4
	"RoleTokenReleaseReq",		//5
	"RoleTokenAcquireAckReq",	//6
	"RoleTokenAcquireNakReq",	//7
	"RoleTokenWithdrawAckReq",	//8
	"RoleTokenReleaseAckReq",	//9
	"RoleTokenLastOpcodeReq",	//10
		
	"RoleTokenFirstOpcodeInd",	//0x0B
	"RoleProviderIdentityInd",	//0x0C
	"NoRoleProviderInd",		//0x0D
	"RoleTokenAcquireInd",		//0x0E
	"RoleTokenWithdrawInd",		//0x0F
	"RoleTokenReleaseInd",		//0x10
	"RoleTokenAcquireAckInd",	//0x11
	"RoleTokenAcquireNakInd",	//0x12
	"RoleTokenWithdrawAckInd",	//0x13
	"RoleTokenReleaseAckInd",	//0x14
	"RoleTokenLastOpcodeInd",	//0x15
};

static const char* g_H239GeneicOpcodeStrings[] =
{	
	"StartH239TokenOpcodes", 
	"FlowControlReleaseRequest",  	 
	"FlowControlReleaseResponse",	 
	"PresentationTokenRequest",		 
	"PresentationTokenResponse", 	
	"PresentationTokenRelease",		 
	"PresentationTokenIndicateOwner"  
};


static const char* g_badSpontanIndReasonStrings[] = 
{
	"MemortAllocationFailure",		// 0x0
	"SendToFastQueueFailed",
	"BchUnSynchronized",
	"PacketReleaseFailure",
	"RealTimeProblems",
	"VideoJitterBufferOverFlow",
	"AudioJitterBufferOverFlow",
	"FailedToAddNewPacketToRtpQueue",
	"Starvation",
	"PacketNotReadyFailure",
	"VideoIsOnlyFillFrames",
	"NoIntraInVideoStream",
	"ParserOutOfSync",
	"noConnWithRemoteInd",	
	"Audio1FramePerPacketInG7231OrG7239",	// G7231 / G7239
	"Audio10FramesPerPacketInG711", // G711
};

#endif


