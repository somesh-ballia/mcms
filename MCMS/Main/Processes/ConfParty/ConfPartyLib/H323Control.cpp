//+========================================================================+
//                            H323control.cpp                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323Control.cpp                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// GuyD| 18/7/05    | Carmel - First phase                           |
//+========================================================================+
#include <stdio.h>
#include <string.h>

#include  "H323Control.h"
//
#include <netinet/in.h>
#include <stdio.h>

#include "TraceStream.h"
#include "IpCsOpcodes.h"
#include "IpMfaOpcodes.h"
#include "IpMngrOpcodes.h"

#include  "Party.h"
#include  "PartyApi.h"

#include "Capabilities.h"
#include "H323Scm.h"
#include "CommModeInfo.h"
#include "CapClass.h"
#include "CapInfo.h"
#include "H264Util.h"
#include "ConfPartyOpcodes.h"
#include "H323Caps.h"
#include "ConfPartyRoutingTable.h"
#include "H323NetSetup.h"
#include "SystemFunctions.h"
#include "Trace.h"
#include "H323Party.h"
#include "DisconnectCause.h"
#include "StatusesGeneral.h"
#include "ConfPartyDefines.h"
#include "ConfPartyGlobals.h"
#include "IpRtpReq.h"
#include "IpRtpInd.h"
#include "SecondaryParameters.h"
#include "GkCsReq.h"
#include "GkCsInd.h"
#include "GKManagerOpcodes.h"
#include "IpServiceListManager.h"
#include "SysConfig.h"
#include "ConfPartyOpcodes.h"
#include "encrAuth.h"
#include "OpcodesMcmsCommon.h"
#include "SysConfigKeys.h"
#include "IpCommon.h"
#include "H323PartyIn.h"
#include "IpCmInd.h"
#include "HostCommonDefinitions.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "H264VideoMode.h"
#include "MsSvcMode.h"
#include "IpAddress.h"
#include "H263VideoMode.h"
#include "OpcodesMcmsVideo.h"
#include "ChannelParams.h"

#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/fips.h>
#include "GkTaskApi.h"
#include "ArtRequestStructs.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesRanges.h"
#include "CdrPersistHelper.h"

extern BOOL GetVendorDetection(); // for Call Generator - Vendor detection

// for debug only #define memset(p,v,l) {for (DWORD qwerty=0;qwerty<(DWORD)l;qwerty++)*(((BYTE*)p)+qwerty)=v;}

//#include "ipload.h"

#define ACTIVE_MC_MASTER_NUMBER 240
#define DEFAULT_MASTER_NUMBER 190
#define SLAVE_NUMBER_DEFAULT  160

using namespace std;

const WORD   SETUP          = 1;
const WORD   CHANGEMODE       = 2;
const WORD   CONNECT        = 3;
const WORD   DISCONNECTING      = 4;
const WORD   AUDCONNECTTOUT     = 5;
const WORD   IRR_TIMER        = 6;
const WORD   PARTYCONNECTING    = 7;
const WORD   H323_GETPORTFAILURE  = 8;
const WORD   MCMSOPENCHANNELS   = 9;
const WORD   FORWARDINGTIMER    = 10;
const WORD   MCMSOPENDATACHANNELS = 12;
const WORD   OTHERMEDIACONNECTED  = 13;
const WORD   REOPEN_CONTENT_IN_TIMER = 16;
const WORD   H323CHANGERATETOUT   = 17;
const WORD   H239_WAIT_FOR_UPDATE_RATE_TIMER = 18;
const WORD   WAIT_FOR_CHANNELS_ECS = 19;
const WORD   PARTYCSKEEPALIVEFIRSTTOUT = 20;
const WORD   PARTYCSKEEPALIVESECONDTOUT = 21;

//26.12.2006 Changes by VK. Stress Test
const WORD   STRESSTESTTOUT          = 22;
// VNGR-787
const WORD   CODIANVIDCHANTOUT       = 23;
#define    STRESS_TEST_TIME            30
#define    STRESS_TEST_TIME_REOPEN     5
static long  s_lStressTestTimeoutCounter = 0;
CapEnum    g_eAudioOpenedCapEnumValue;
#define    STRESS_TEST_SIMULATION      FALSE

const WORD   MinAliasNumber  = 1;

const BYTE   g_kCallWithAllChannels = 8;
const BYTE   g_kCallWithoutOneMedia = 6;
const BYTE   g_kCallWithoutTwoMedia = 4;
const BYTE   g_kCallWithout3Media   = 2;

const APIU8  AVAYA_SIP_CM_FLAG_ON   = 1;
const APIU8  AVAYA_SIP_CM_FLAG_OFF  = 0;

const UINT32   AVF_Entry_Queue_Transfer = 10;

const char *g_initiatorOfCloseStrings[] = {"NoInitiator", "McInitiator",
   "CtInitiator", "PmInitiator", "GkInitiator"};

const DWORD AUDIO_ONLY_SETUP_RATE = 64000;

#define  ShiftPartyAddress  3
#define  IPSize       15
#define  MinimumBandwidth 64
#define  H323_LOG_FILE      "7.256/mcu/h323/logfile.log"
#define  NO_PORT_LEFT   65534

#define  MaxGapRateForVcon    320
#define IP_LIMIT_ADDRESS_CHAR_LEN 256

// LPR
const WORD   LPRTOUT         = 24;
const WORD   LOCALLPRTOUT    = 25;
const DWORD  minPeopleRate	 = 640;


// TandbergEp
const WORD   CAPABILITIESTOUT    = 26;

#define      POLYCOM_BANDWITH_MIN    rate64K
#define      POLYCOM_BANDWITH_MAX    rate6144K

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

extern CIpServiceListManager* GetIpServiceListMngr();


#if !defined(FAR)
  #define FAR
#endif /* INT8 */

#if !defined(TYPE_Uint8)
	typedef unsigned char Uint8, *pUint8;
	#define TYPE_Uint8
#endif /* UINT8 */


extern "C"
{
    FAR int CreateCipherKey(const Uint8 *pucSharedSecret, Uint8 *pucCipherKey, Uint8 *pucEncryptedCipherKey);
    FAR int CreateCipherKeyOpenSSL(const Uint8 *pucSharedSecret, Uint8 *pucCipherKey, Uint8 *pucEncryptedCipherKey);
    FAR void DecryptCipherKey(const Uint8 *pucSharedSecret, Uint8 *pucCipherKey, Uint8 *pucDecryptedCipherKey);
    FAR void DecryptCipherKeyOpenSSL(const Uint8 *pucSharedSecret, Uint8 *pucCipherKey, Uint8 *pucDecryptedCipherKey);
}


const WORD US_t35CountryCode   = 181;
const WORD US_t35Extension     = 0;
const WORD Israel_t35CountryCode = 88;
const WORD Israel_t35Extension   = 0;
const WORD Sweden_t35CountryCode = 165;
const WORD Sweden_t35Extension   = 0;
const WORD Test_t35CountryCode   = 11;
const WORD Test_t35Extension     = 11;

// Definitions of PictureTel manufacturer Identification
// Used to add G.7221 to the capability
const WORD PictureTel_manufacturerCode  = 1;

// Definitions of Polycom manufacturer Identification
// Used to add/remove G.7221/P&C from the capability
const WORD Polycom_manufacturerCode = 9009;

#define ViaVideoVendorIdPrefix "ViaVideo"

// Definitions of Polycom Israel (Accord) manufacturer Identification
// Used to cascade, increase video rate in CP conferences, at the capability
const WORD Accord_manufacturerCode  = 172;

// Definitions of RadVision identification
const WORD  RadVision_manufacturerCode1 = 21;
const WORD  RadVision_manufacturerCode2 = 15;
const char *RadVisionVersionID      = "RADVision";
const char *RadVisionTestApp      = "Test application";
const char *RadVisionMcu        = "RADVision MCU";
const char *RadVisionViaIpMcu     = "RADVision ViaIp MCU";
const char *RadVisionViuProductId     = "RADVision. VIU-323";

//Cisco
const WORD  Cisco_manufacturerCode1 = 18;
const char *CiscoCallManagerID    = "CiscoCallManager";

// Definitions of Polycom VoIP manufacturer identification
// Used to remove G.7221 , G.722 and P+C from the capability set
const WORD  PolycomVoIP_manufacturerCode  = 11;
const char *PolycomVoIPProduct   = "Hmx";
const char *PolycomVoIPVersionID = "Circa";

// Definitions of Vcon identification
const WORD VconManufacturerCode = 1;
const char *VconProductId   = "Vcon:h323";

// Definitions of Tandberg 6000 and 7000(for DuoVideo):
const char *TandbergProductId = "Tandberg";
const char *Tandberg6000EVersionID = "30";
const char *Tandberg1000EVersionID = "23";

// Definitions of NetMeeting:
const char *MicrosoftId = "Microsoft";
const char *NetMeetingId = "NetMeeting";

// Definitions of Ericsson VIG:
const WORD  Ericsson_manufacturerCode  = 2;
const char *EricssonVIGProductId     = "CRA";
const WORD  EricssonSip_t35CountryCode    = 0;
const WORD  EricssonSip_manufacturerCode  = 0;

const char *SonyProductId = "SONY";

const WORD  Avistar_manufacturerCode  = 128;
const WORD  Avistar_t35CountryCode = 181;
const char *AvistarProductId = "Avistar";

const char *LifeSizeProductId = "LifeSize";
//-S- HOMOLOGATION 3.0 RAS_TE_STA_02 -----------------------//
#define GapFromRealIrrInterval   10
//-E- HOMOLOGATION 3.0 RAS_TE_STA_02 -----------------------//

//#define  CHANGERATE_TIME 10 * SECOND

#define PARTY_CHANGERATE_TIME CHANGERATE_TIME - 2*SECOND

// Definitions of DST
const WORD DST_manufacturerCode = 1;
const char* DstMcsProductId = "DST H.323 MCS";
const char* RMX1000ProductId= "Polycom RMX1000";

const WORD China_t35CountryCode = 38;
const WORD China_t35Extension  = 0;

// Definitions of Codian VCR product ID (VNGFE-787)
const char *CodianVcrProductId     = "Codian VCR";

// Polycom NGP
const char* PolycomNpgProductId= "POLYCOM NPG";


// Polycom 2000C
const char* RMX2000ProductId= "Polycom RMX1000_2000";

// Polycom QDX
const char* QDXProductId = "QDX";

const char* H323InvalidDisplayChars = "#,;:/\"<>\0";
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//                                               //
//            Test Of UserUser Method                        //
//                                               //
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////m_pLocalCapH323/////////////////////////////////////////////////////

// UserUser table and values limitation
const int UserUserTableSize   = 6;
const int MaxAddedStringSize    = 16;
const int FixOpcodeStringLength = 3;
const int MaxOpcodeValue    = 999;
const int MinOpcodeValue    = 0;

// UserUser opcodes
const int CascadeVSW    = 0; //for recognize version 4 and less VSW
const int IpOnly      = 1; //for vsw ip only
const int RRQasGW320    = 2;
const int VsMixed     = 3; //for vsw mixed
const int TransGw     = 4;
const int COP               = 5;


char *UserUserMessageTable[UserUserTableSize] = {
  "VSW_",       //for recognize version 4 and less VSW
  "IPONLY_",   //for vsw ip only or SWCP
  "GW_320",
  "VSMIXED_",  //for vsw mixed
  "TRANSGW",    // For transparent GW
  "COP",
};

char *UserUserOpcodeTable[UserUserTableSize] = {
  "000",
  "001",
  "002",
  "003",
  "004",
  "005"
};

const char *g_roleTokenOpcodeStrings[] =
{
  "RoleTokenFirstOpcodeReq",  //0
  "RoleProviderIdentityReq",  //1
  "NoRoleProviderReq",    //2
  "RoleTokenAcquireReq",    //3
  "RoleTokenWithdrawReq",   //4
  "RoleTokenReleaseReq",    //5
  "RoleTokenAcquireAckReq", //6
  "RoleTokenAcquireNakReq", //7
  "RoleTokenWithdrawAckReq",  //8
  "RoleTokenReleaseAckReq", //9
  "RoleTokenLastOpcodeReq", //10

  "RoleTokenFirstOpcodeInd",  //0x0B
  "RoleProviderIdentityInd",  //0x0C
  "NoRoleProviderInd",    //0x0D
  "RoleTokenAcquireInd",    //0x0E
  "RoleTokenWithdrawInd",   //0x0F
  "RoleTokenReleaseInd",    //0x10
  "RoleTokenAcquireAckInd", //0x11
  "RoleTokenAcquireNakInd", //0x12
  "RoleTokenWithdrawAckInd",  //0x13
  "RoleTokenReleaseAckInd", //0x14
  "RoleTokenLastOpcodeInd", //0x15
};

const char* g_H239GeneicOpcodeStrings[] =
{
  "StartH239TokenOpcodes",
  "FlowControlReleaseRequest",
  "FlowControlReleaseResponse",
  "PresentationTokenRequest",
  "PresentationTokenResponse",
  "PresentationTokenRelease",
  "PresentationTokenIndicateOwner"
};

const char* GetRoleTokenOpcodeStr(ERoleTokenOpcode o)
{
  if (o > kStartH239TokenOpcodes)
    return g_H239GeneicOpcodeStrings[o-kStartH239TokenOpcodes];
  else if((o > kRoleTokenFirstOpcodeReq && o < kRoleTokenLastOpcodeReq)|| (o > kRoleTokenFirstOpcodeInd && o < kRoleTokenLastOpcodeInd))
      return g_roleTokenOpcodeStrings[o];
  else
    return "Unknown opcode";
}


const char *g_feccTokenOpcodeStrings[] =
{
  "TokenRequest",
  "TokenAccept",
  "TokenReject",
  "TokenRelease",
  "TokenReleaseRequest",
  "TokenWithdrow",
};

const char* g_badSpontanIndReasonStrings[] =
{
  "MemortAllocationFailure",    // 0x0
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
  "Audio1FramePerPacketInG7231OrG7239", // G7231 / G7239
  "Audio10FramesPerPacketInG711", // G711
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Variables:     UserUserString - out value. the string to which we add a new opcode string.
//          opcode - the type of the added message.
//          AddedString - the string to be add, if needed.
// Description:   add an opcode string to the useruser string (a field in the h225 messages).
//          the opcode string includes the opcode number (a 3 characters number) and
//                a message representing the opcode (UserUserMessageTable[])
// Return value:  int the size of useruser string. 0 - in case of false or failure.
//          The function returns FALSE in the following cases:
//          1. opcode out of valid range.
//          2. there is not enough free place in the useruser string.
// Remarks:       As result of a successful operation (method returns no zero value),
//          opcode,opcode message and pAddedString will be concatenated to pUserUser.
//////////////////////////////////////////////////////////////////////////////////////////////
int SetUserUserFieldByOpcode(char *pUserUserString, int opcode, const char *pAddedString)
{
  int AddedStringSize;
  if((opcode >= UserUserTableSize) || (opcode < MinOpcodeValue))// the opcode value must be between 0 - 999.
    return 0;
  if(opcode==COP)
  {
    strcat(pUserUserString, UserUserMessageTable[opcode]);
    return strlen(pUserUserString);
  }

  AddedStringSize = strlen(UserUserOpcodeTable[opcode])  + strlen(UserUserMessageTable[opcode] + strlen(pAddedString) );
  // check if the added string will cause overlowing the max field size.
  // + 2 is for ";" and for null terminated string
  if( (strlen(pUserUserString) + AddedStringSize + 2) > MaxUserUserSize )
    return 0;

  strcat(pUserUserString, ";");
  strcat(pUserUserString, UserUserOpcodeTable[opcode]);
  strcat(pUserUserString, UserUserMessageTable[opcode]);
  strncat(pUserUserString, pAddedString, strlen(pAddedString));
  return strlen(pUserUserString);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Variables:     UserUserString - the string from which we take the opcode string.
//          opcode - the type of the get message.
//          FoundString - out value. the string that follows the opcode.
// Description:   Finds the additional string out of the useruser string.
//                The additional string follows the opcode string inside the useruser string
//                (an opcode is a field in the h225 messages).
// Return value:  True or False for success of failure in finding the string.
// Remarks:     The function returns FALSE in the following cases:
//          1. opcode out of valid range.
//          2. opcode to be found is not at the useruser field.
//////////////////////////////////////////////////////////////////////////////////////////////
/*
BYTE GetUserUserStringByOpcode(const char *UserUserString, int opcode, char *FoundString)
{
  char *pOpcodeString, *pTempUser, *pEndString;
  if((opcode >= UserUserTableSize) || (opcode < MinOpcodeValue))// the opcode value must be between 0 - 999.
    return FALSE;
  pOpcodeString = UserUserOpcodeTable[opcode];
  // set pTempUser to (one character before)the beginning of opcode string within UserUserString
  // if no opcode found, pTempUser is set to NULL
  pTempUser = strchr(UserUserString, ';');
  while(pTempUser) // opcode found
  {
    // if string opcode equals searched opcode, grab the following string and set 'FoundString' with it
    if( !strncmp(pTempUser + 1, pOpcodeString, strlen(pOpcodeString)))
    {
      // UserUserString structure:
      // "<UserUserFirstPart>;<opcode><opcode message><additional tring>[;]
      //       pTempUser-----^                 pEndString----^

      int opcodeFullLength = FixOpcodeStringLength + strlen(UserUserMessageTable[opcode]);

      pEndString = strchr(pTempUser + 1, ';');
      if(pEndString)
        strncpy( FoundString, pTempUser + 1 + opcodeFullLength, pEndString - (pTempUser + 1 + opcodeFullLength) );
      else
      {
        if ( strlen(UserUserString) > MaxUserUserSize )
          return FALSE;
        // since character ending string is not found, we need to count on null terminated string
        // which after previous check, is safe to be used
        else
          // strlen(pTempUser + 1 + opcodeFullLength)  is the length of the additional tring
          strncpy( FoundString, pTempUser + 1 + opcodeFullLength, strlen(pTempUser + 1 + opcodeFullLength) );
      }
      return TRUE;
    }
    // opcode doesn't equal. go to next opcode (if any)
    else
      pTempUser = strchr(pTempUser + 1, ';');
  }
  // no opcode found - return false
  return FALSE;
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CH323Cntl)
        // party events
  ONEVENT(H323_CS_SIG_VIDEO_UPDATE_PIC_REQ,   IDLE,   CH323Cntl::NullActionFunction)
  ONEVENT(H323_CS_SIG_VIDEO_UPDATE_PIC_REQ,   SETUP,    CH323Cntl::NullActionFunction)
  ONEVENT(H323_CS_SIG_VIDEO_UPDATE_PIC_REQ,   CONNECT,  CH323Cntl::OnPartyVideoUpdatePicReq)
  ONEVENT(H323_CS_SIG_VIDEO_UPDATE_PIC_REQ,   ANYCASE,  CH323Cntl::NullActionFunction)

  ONEVENT(H323_CS_SIG_CALL_NEW_RATE_IND,      SETUP,    CH323Cntl::OnH323CallNewRateInd)
  ONEVENT(H323_CS_SIG_CALL_CONNECTED_IND,     IDLE ,    CH323Cntl::OnH323CallConnectedInd)
  ONEVENT(H323_CS_SIG_CALL_CONNECTED_IND,     SETUP,    CH323Cntl::OnH323CallConnectedInd)
  ONEVENT(H323_CS_SIG_CALL_CNTL_CONNECTED_IND,    IDLE,   CH323Cntl::OnH323CallCntlInd)
  ONEVENT(H323_CS_SIG_CALL_CNTL_CONNECTED_IND,    SETUP,    CH323Cntl::OnH323CallCntlInd)
  ONEVENT(H323_CS_SIG_CALL_CNTL_CONNECTED_IND,    CONNECT,  CH323Cntl::OnH323CallCntlInd)
  ONEVENT(H323_CS_SIG_CAPABILITIES_IND,       CONNECT,  CH323Cntl::OnH323CapIndication)
  ONEVENT(H323_CS_SIG_CAPABILITIES_IND,       SETUP,    CH323Cntl::OnH323CapIndication)
  ONEVENT(H323_CS_FACILITY_IND,           SETUP,    CH323Cntl::OnH323FacilityIndSetup)
  ONEVENT(H323_CS_FACILITY_IND,           CONNECT,  CH323Cntl::OnH323FacilityIndConnect)
  ONEVENT(H323_CS_SIG_CAP_RESPONSE_IND,       SETUP,    CH323Cntl::OnH323CapResponseInd)
  ONEVENT(H323_CS_SIG_CAP_RESPONSE_IND,       CONNECT,  CH323Cntl::OnH323CapResponseInd)
/*  ONEVENT(H323_CONF_CONNECTED_IND,        IDLE,   CH323Cntl::OnH323ConfConnectedInd)
  ONEVENT(H323_CONF_CONNECTED_IND,          SETUP,    CH323Cntl::OnH323ConfConnectedInd) */
  ONEVENT(H323_CS_SIG_INCOMING_CHANNEL_IND,     CONNECT,  CH323Cntl::OnH323IncomingChnlInd)
  ONEVENT(H323_CS_SIG_INCOMING_CHANNEL_IND,     SETUP,    CH323Cntl::OnH323IncomingChnlInd)
  ONEVENT(H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND,  CONNECT,  CH323Cntl::OnH323IncomingChnlConnectedInd)
  ONEVENT(H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND,  SETUP,    CH323Cntl::OnH323IncomingChnlConnectedInd)
/*  ONEVENT(H323_INCOMING_MEDIA_IND,        CONNECT,  CH323Cntl::OnH323IncomingMediaInd)
  ONEVENT(H323_INCOMING_MEDIA_IND,          SETUP,    CH323Cntl::OnH323IncomingMediaInd)*/
  ONEVENT(IP_RTP_VIDEO_UPDATE_PIC_IND,        CONNECT,  CH323Cntl::OnH323RtpVideoUpdatePicInd)
  ONEVENT(IP_RTP_VIDEO_UPDATE_PIC_IND,        SETUP,    CH323Cntl::OnH323RtpVideoUpdatePicInd)
  ONEVENT(IP_CS_VIDEO_UPDATE_PIC_IND,       CONNECT,  CH323Cntl::OnH323VideoUpdatePicInd)
  ONEVENT(IP_CS_VIDEO_UPDATE_PIC_IND,       SETUP,    CH323Cntl::OnH323VideoUpdatePicInd)
  ONEVENT(IP_CM_RTCP_PACKET_LOSS_STATUS_IND,       CONNECT,  CH323Cntl::OnH323PacketLostStatusConnected)
  ONEVENT(IP_CM_RTCP_PACKET_LOSS_STATUS_IND,       ANYCASE,  CH323Cntl::OnH323PacketLostStatusImproperState)
  ONEVENT(H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND, CONNECT,  CH323Cntl::OnH323OutgoingChnlResponseInd)
  ONEVENT(H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND, SETUP,    CH323Cntl::OnH323OutgoingChnlResponseInd)
  ONEVENT(H323_CS_SIG_CALL_IDLE_IND,        CONNECT,  CH323Cntl::OnH323CallIdleInd)
  ONEVENT(H323_CS_SIG_CALL_IDLE_IND,        SETUP,    CH323Cntl::OnH323CallIdleInd)
  ONEVENT(H323_CS_SIG_CALL_IDLE_IND,        DISCONNECT, CH323Cntl::OnH323CallIdleInd)
  ONEVENT(H323_CS_SIG_FLOW_CONTROL_IND_IND,     CONNECT,  CH323Cntl::OnH323FlowControlIndInd)
  ONEVENT(H323_CS_SIG_FLOW_CONTROL_IND_IND,     SETUP,    CH323Cntl::OnH323FlowControlIndInd)
  ONEVENT(H323_CS_SIG_START_CHANNEL_CLOSE_IND,    CONNECT,  CH323Cntl::OnH323StartChannelCloseInd)
  ONEVENT(H323_CS_SIG_START_CHANNEL_CLOSE_IND,    SETUP,    CH323Cntl::OnH323StartChannelCloseInd)
  ONEVENT(H323_CS_SIG_CHANNEL_CLOSE_IND,      CONNECT,  CH323Cntl::OnH323ChannelCloseInd)
  ONEVENT(H323_CS_SIG_CHANNEL_CLOSE_IND,      SETUP,    CH323Cntl::OnH323ChannelCloseInd)
//  ONEVENT(H323_CHAN_TSTO_IND,       CONNECT,  CH323Cntl::OnH323ChannelTSTOInd)
//  ONEVENT(H323_CHAN_TSTO_IND,       SETUP,    CH323Cntl::OnH323ChannelTSTOInd)
//  ONEVENT(H323_CALL_USER_INPUT_IND,     CONNECT,  CH323Cntl::OnH323CallUserInd)
//  ONEVENT(H323_CALL_USER_INPUT_IND,     SETUP,    CH323Cntl::OnH323CallUserInd)
//  ONEVENT(H323_ROUND_TRIP_DELAY_IND,    CONNECT,  CH323Cntl::OnH323RoundTripDelayInd)
//  ONEVENT(H323_ROUND_TRIP_DELAY_IND,    SETUP,    CH323Cntl::OnH323RoundTripDelayInd)
  ONEVENT(H323_CS_SIG_CHAN_NEW_RATE_IND,      CONNECT,  CH323Cntl::OnH323ChannelNewRateInd)
  ONEVENT(H323_CS_SIG_CHAN_NEW_RATE_IND,      SETUP,    CH323Cntl::OnH323ChannelNewRateInd)
  ONEVENT(H323_CS_SIG_CHAN_MAX_SKEW_IND,      CONNECT,  CH323Cntl::OnH323ChannelMaxSkewInd)
  ONEVENT(H323_CS_SIG_CHAN_MAX_SKEW_IND,      SETUP,    CH323Cntl::OnH323ChannelMaxSkewInd)
  ONEVENT(H323_CS_SIG_GET_PORT_IND,         IDLE,   CH323Cntl::OnH323GetPortInd)
  ONEVENT(H323_CS_SIG_GET_PORT_IND,         SETUP,    CH323Cntl::OnH323GetPortInd)

  ONEVENT(H323_CS_CHANNEL_OFF_IND,          CONNECT,  	CH323Cntl::OnH323ChannelOffInd) //CS
  ONEVENT(H323_CS_CHANNEL_OFF_IND,          SETUP,    	CH323Cntl::OnH323ChannelOffInd)
  ONEVENT(H323_CS_CHANNEL_OFF_IND,          IDLE,    	CH323Cntl::NullActionFunction)

  ONEVENT(H323_CS_CHANNEL_ON_IND,         	CONNECT,  	CH323Cntl::OnH323ChannelOnInd)
  ONEVENT(H323_CS_CHANNEL_ON_IND,         	SETUP,    	CH323Cntl::OnH323ChannelOnInd)
  ONEVENT(H323_CS_CHANNEL_ON_IND,        	IDLE,    	CH323Cntl::NullActionFunction)

  ONEVENT(H323_CS_DTMF_INPUT_IND,         CONNECT,  CH323Cntl::OnH323DTMFInd)
  ONEVENT(H323_CS_DTMF_INPUT_IND,         SETUP,      CH323Cntl::OnH323DTMFInd)

  ONEVENT(IP_RTP_BAD_SPONTAN_IND,           IDLE,   CH323Cntl::OnH323RtpBadSpontaneuosInd)
  ONEVENT(IP_RTP_BAD_SPONTAN_IND,           ANYCASE,  CH323Cntl::OnH323RtpBadSpontaneuosInd)



  ONEVENT(H323_CS_SIG_CALL_BAD_SPONTAN_IND,     IDLE,   CH323Cntl::OnH323CsBadSpontaneuosInd)
  ONEVENT(H323_CS_SIG_CALL_BAD_SPONTAN_IND,     ANYCASE,  CH323Cntl::OnH323CsBadSpontaneuosInd)

  ONEVENT(IP_CM_PARTY_MONITORING_IND,     CONNECT,  CH323Cntl::OnH323PartyMonitoringInd)
  ONEVENT(IP_CM_PARTY_MONITORING_IND,     SETUP,    CH323Cntl::OnH323PartyMonitoringInd)
  ONEVENT(IP_CM_PARTY_MONITORING_IND,     IDLE,   CH323Cntl::OnH323PartyMonitoringInd)
  ONEVENT(IP_CM_PARTY_MONITORING_IND,     ANYCASE,  CH323Cntl::NullActionFunction)

  ONEVENT(IP_CM_RTCP_MSG_IND,     CONNECT,  CH323Cntl::OnCmPacketLossInd)
  ONEVENT(IP_CM_RTCP_MSG_IND,     ANYCASE,  CH323Cntl::NullActionFunction)

  ONEVENT(CM_SEND_CNAME_INFO_AS_STRING_IND,     DISCONNECT, CH323Cntl::NullActionFunction)
  ONEVENT(CM_SEND_CNAME_INFO_AS_STRING_IND,     ANYCASE,  CH323Cntl::RetriveCNAMEInfoIfNeeded)


  ONEVENT(IP_RTP_STREAM_STATUS_IND,         CONNECT,  CH323Cntl::OnH323StreamStatusInd)

//  ONEVENT(H323_CT_AUTHENTICATION_IND,   IDLE,   CH323Cntl::OnH323CtAuthenticationInd)
//  ONEVENT(H323_CT_AUTHENTICATION_IND,   SETUP,    CH323Cntl::OnH323CtAuthenticationInd)

//  ONEVENT(H323_NOTIFY_IND,          CONNECT,    (AFUNC)CH323Cntl::OnH323NotifyInd      ,"H323_NOTIFY_IND",  "CONNECT")

        // self timer events
  ONEVENT(AUDCONNECTTOUT,             SETUP,    CH323Cntl::OnAudioConnectTimeOutSetup)
  ONEVENT(AUDCONNECTTOUT,             CONNECT,  CH323Cntl::OnAudioConnectTimeOutConnect)
  ONEVENT(OTHERMEDIACONNECTED,            SETUP,    CH323Cntl::OnOtherMediaConnectTimeOutSetup)
  ONEVENT(OTHERMEDIACONNECTED,            CONNECT,  CH323Cntl::OnOtherMediaConnectTimeOutConnect)//highest common

  ONEVENT(PARTYCONNECTING,              IDLE,   CH323Cntl::OnPartyConnectingTimeout)
  ONEVENT(PARTYCONNECTING,              SETUP,    CH323Cntl::OnPartyConnectingTimeout)
  ONEVENT(IRR_TIMER,                SETUP,    CH323Cntl::OnIrrTimeout)
  ONEVENT(IRR_TIMER,                CONNECT,  CH323Cntl::OnIrrTimeout)

  ONEVENT(DRQ_TIMER,                  IDLE,     CH323Cntl::OnDrqTimer)
  ONEVENT(DRQ_TIMER,                  ANYCASE,  CH323Cntl::OnDrqTimer)

  ONEVENT(H323_GETPORTFAILURE,            IDLE,   CH323Cntl::OnH323GetPortFailed)
  ONEVENT(H323_GETPORTFAILURE,            SETUP,    CH323Cntl::OnH323GetPortFailed)

  ONEVENT(MCMSOPENCHANNELS,             SETUP,    CH323Cntl::OnTimerOpenChannelFromMcms)
  ONEVENT(MCMSOPENCHANNELS,             CONNECT,  CH323Cntl::OnTimerOpenChannelFromMcms)
  ONEVENT(MCMSOPENDATACHANNELS,           SETUP,    CH323Cntl::OnTimerOpenDataChannelFromMcms)
  ONEVENT(MCMSOPENDATACHANNELS,           CONNECT,  CH323Cntl::OnTimerOpenDataChannelFromMcms)

  ONEVENT(PARTYDISCONNECTTOUT,            IDLE,     CH323Cntl::OnTimerClear)
  ONEVENT(PARTYDISCONNECTTOUT,            SETUP,      CH323Cntl::OnTimerClear)
  ONEVENT(PARTYDISCONNECTTOUT,            CONNECT,  CH323Cntl::OnTimerClear)
  ONEVENT(PARTYDISCONNECTTOUT,            DISCONNECT, CH323Cntl::OnTimerClear)



  ONEVENT(H323CHANGERATETOUT,           CONNECT,  CH323Cntl::OnContentChangeModeTimeOut)
  ONEVENT(REOPEN_CONTENT_IN_TIMER,          CONNECT,  CH323Cntl::OnTimerRopenContentIn)
  ONEVENT(H239_WAIT_FOR_UPDATE_RATE_TIMER,      CONNECT,    CH323Cntl::OnTimerWaitForUpdateRate)

  ONEVENT(H323_CS_SIG_CALL_ROLE_TOKEN_IND,      SETUP,    CH323Cntl::OnH323RoleTokenInd)
  ONEVENT(H323_CS_SIG_CALL_ROLE_TOKEN_IND,      CONNECT,  CH323Cntl::OnH323RoleTokenInd)
  ONEVENT(H323_CS_SIG_CALL_ROLE_TOKEN_IND,      DISCONNECT, CH323Cntl::NullActionFunction)

  ONEVENT(IP_RTP_FECC_TOKEN_IND,          SETUP,    CH323Cntl::OnH323FeccTokenInd)
  ONEVENT(IP_RTP_FECC_TOKEN_IND,          CONNECT,  CH323Cntl::OnH323FeccTokenInd)
  ONEVENT(IP_RTP_FECC_TOKEN_IND,          DISCONNECT, CH323Cntl::OnH323FeccTokenInd)

  ONEVENT(IP_RTP_FECC_KEY_IND,            SETUP,    CH323Cntl::OnH323FeccKeyInd)
  ONEVENT(IP_RTP_FECC_KEY_IND,            CONNECT,  CH323Cntl::OnH323FeccKeyInd)
  ONEVENT(IP_RTP_FECC_KEY_IND,            DISCONNECT, CH323Cntl::OnH323FeccKeyInd)

  ONEVENT(FORWARDINGTIMER,              SETUP,    CH323Cntl::OnH323ForwardDisconnect)
  ONEVENT(WAIT_FOR_CHANNELS_ECS,          ANYCASE,  CH323Cntl::OnChannelsECSTimer)
                // ConferenceRequest event
  ONEVENT(H323_CS_CONFERENCE_REQ_IND,       SETUP,      CH323Cntl::OnH323ConferenceReqInd)
  ONEVENT(H323_CS_CONFERENCE_REQ_IND,       CONNECT,    CH323Cntl::OnH323ConferenceReqInd)
  ONEVENT(H323_CS_CONFERENCE_COM_IND,       SETUP,      CH323Cntl::OnH323ConferenceComInd)
  ONEVENT(H323_CS_CONFERENCE_COM_IND,       CONNECT,    CH323Cntl::OnH323ConferenceComInd)
  ONEVENT(H323_CS_CONFERENCE_RES_IND,       SETUP,      CH323Cntl::OnH323ConferenceResInd)
  ONEVENT(H323_CS_CONFERENCE_RES_IND,       CONNECT,    CH323Cntl::OnH323ConferenceResInd)
  ONEVENT(H323_CS_CONFERENCE_IND_IND,       SETUP,      CH323Cntl::OnH323ConferenceIndInd)
  ONEVENT(H323_CS_CONFERENCE_IND_IND,       CONNECT,    CH323Cntl::OnH323ConferenceIndInd)

                // generic NonStandard command event
  ONEVENT(H323_CS_NON_STANDARD_REQ_IND,       SETUP,      CH323Cntl::OnH323NonStandardReqInd)
  ONEVENT(H323_CS_NON_STANDARD_REQ_IND,       CONNECT,    CH323Cntl::OnH323NonStandardReqInd)
  ONEVENT(H323_CS_NON_STANDARD_COM_IND,       SETUP,      CH323Cntl::OnH323NonStandardComInd)
  ONEVENT(H323_CS_NON_STANDARD_COM_IND,       CONNECT,    CH323Cntl::OnH323NonStandardComInd)
  ONEVENT(H323_CS_NON_STANDARD_RES_IND,       SETUP,      CH323Cntl::OnH323NonStandardResInd)
  ONEVENT(H323_CS_NON_STANDARD_RES_IND,       CONNECT,    CH323Cntl::OnH323NonStandardResInd)
  ONEVENT(H323_CS_NON_STANDARD_IND_IND,       SETUP,      CH323Cntl::OnH323NonStandardIndInd)
  ONEVENT(H323_CS_NON_STANDARD_IND_IND,       CONNECT,    CH323Cntl::OnH323NonStandardIndInd)
    //DBC2
  ONEVENT(H323_CS_SIG_DBC2_COMMAND_CT_ON_IND,   SETUP,      CH323Cntl::OnH323DBC2CommandInd)
  ONEVENT(H323_CS_SIG_DBC2_COMMAND_CT_ON_IND,   CONNECT,    CH323Cntl::OnH323DBC2CommandInd)
/*  ONEVENT(H323_RTP_DBC2_COMMAND_ON_IND,       SETUP,      CH323Cntl::OnH323DBC2CommandInd)
  ONEVENT(H323_RTP_DBC2_COMMAND_ON_IND,       CONNECT,    CH323Cntl::OnH323DBC2CommandInd)
  ONEVENT(H323_CS_SIG_DBC2_COMMAND_CT_OFF_IND,      SETUP,      CH323Cntl::OnH323DBC2CommandInd)
  ONEVENT(H323_CS_SIG_DBC2_COMMAND_CT_OFF_IND,      CONNECT,    CH323Cntl::OnH323DBC2CommandInd)
  ONEVENT(H323_RTP_DBC2_COMMAND_OFF_IND,      SETUP,      CH323Cntl::OnH323DBC2CommandInd)
  ONEVENT(H323_RTP_DBC2_COMMAND_OFF_IND,      CONNECT,    CH323Cntl::OnH323DBC2CommandInd)
*/
  // PArty CS keep alive timers - Error handling
  ONEVENT(PARTYCSKEEPALIVEFIRSTTOUT,        ANYCASE,    CH323Cntl::OnPartyCsErrHandleKeepAliveFirstTout)
  ONEVENT(PARTYCSKEEPALIVESECONDTOUT,       ANYCASE,    CH323Cntl::OnPartyCsErrHandleKeepAliveSecondTout)
  ONEVENT(H323_CS_PARTY_KEEP_ALIVE_IND,       ANYCASE,    CH323Cntl::OnPartyCsErrHandleKeepAliveInd)


  ONEVENT(ACK_IND,                  SETUP,    CH323Cntl::OnMfaAck)
  ONEVENT(ACK_IND,                  CONNECT,  CH323Cntl::OnMfaAck)
  ONEVENT(ACK_IND,                  IDLE,     CH323Cntl::OnMfaAckDisconnectInternalArt)
  ONEVENT(ACK_IND,                  ANYCASE,  CH323Cntl::OnMfaAckDisconnectInternalArt)
  ONEVENT(MFARESPONSE_TOUT,               ANYCASE,  CH323Cntl::OnMfaReqToutAnycase)

/* GK */
  ONEVENT(H323_CS_RAS_ARQ_IND,          IDLE,   CH323Cntl::OnH323ARQInd)
  ONEVENT(H323_CS_RAS_ARQ_IND,          SETUP,    CH323Cntl::OnH323ARQInd)

  ONEVENT(H323_CS_RAS_FAIL_IND,           IDLE,     CH323Cntl::OnH323GKFailInd)
  ONEVENT(H323_CS_RAS_FAIL_IND,           ANYCASE,  CH323Cntl::OnH323GKFailInd)

  ONEVENT(H323_CS_RAS_GKDRQ_IND,            IDLE,     CH323Cntl::OnH323GkDRQInd)
  ONEVENT(H323_CS_RAS_GKDRQ_IND,            ANYCASE,  CH323Cntl::OnH323GkDRQInd)

  ONEVENT(H323_CS_RAS_GKBRQ_IND,            IDLE,   CH323Cntl::OnH323GkBRQInd)
  ONEVENT(H323_CS_RAS_GKBRQ_IND,            ANYCASE,  CH323Cntl::OnH323GkBRQInd)

  ONEVENT(H323_CS_RAS_BRQ_IND,          IDLE,     CH323Cntl::OnH323BRQInd)
  ONEVENT(H323_CS_RAS_BRQ_IND,          ANYCASE,  CH323Cntl::OnH323BRQInd)

  ONEVENT(H323_CS_RAS_GKIRQ_IND,            IDLE,     CH323Cntl::OnH323GkIRQInd)
  ONEVENT(H323_CS_RAS_GKIRQ_IND,            ANYCASE,  CH323Cntl::OnH323GkIRQInd)

  ONEVENT(DRQ_IND_OR_FAIL,            ANYCASE,  CH323Cntl::OnGkMangerSendDrqIndOrFail)

  ONEVENT(HOLD_GK_REQ,              IDLE,     CH323Cntl::OnGkManagerHoldGkReq)
  ONEVENT(HOLD_GK_REQ,              ANYCASE,  CH323Cntl::OnGkManagerHoldGkReq)

  ONEVENT(RESEND_GK_REQ,              IDLE,   CH323Cntl::OnGkManagerResendGkReq)
  ONEVENT(RESEND_GK_REQ,              ANYCASE,  CH323Cntl::OnGkManagerResendGkReq)

  ONEVENT(REMOVE_GK_CALL,             IDLE,   CH323Cntl::OnGkManagerRemoveCallReq)
  ONEVENT(REMOVE_GK_CALL,             ANYCASE,  CH323Cntl::OnGkManagerRemoveCallReq)

  ONEVENT(STOP_IRRTIMER,              IDLE,   CH323Cntl::OnGkManagerStopIrrTimer)
  ONEVENT(STOP_IRRTIMER,              ANYCASE,  CH323Cntl::OnGkManagerStopIrrTimer)

  ONEVENT(GK_MANAGER_PARTY_KEEP_ALIVE_REQ,    IDLE,   CH323Cntl::OnGkManagerKeepAliveReq)
  ONEVENT(GK_MANAGER_PARTY_KEEP_ALIVE_REQ,    ANYCASE,  CH323Cntl::OnGkManagerKeepAliveReq)
  //VNGR 2227
  ONEVENT(IP_RTP_DTMF_INPUT_IND,          ANYCASE,  CH323Cntl::OnRtpDtmfInputInd)
  //26.12.2006 Changes by VK. Stress Test
  ONEVENT(STRESSTESTTOUT,             ANYCASE,  CH323Cntl::OnStressTestTimeout)
  // VNGR-787
  ONEVENT(CODIANVIDCHANTOUT,            ANYCASE,  CH323Cntl::OnCodianVcrVidChannelTimeout)

  ONEVENT(IP_RTP_DIFF_PAYLOAD_TYPE_IND,     ANYCASE,  CH323Cntl::NullActionFunction)

  // LPR
  ONEVENT(IP_RTP_LPR_MODE_CHANGE_IND,       SETUP ,   CH323Cntl::NullActionFunction)
  ONEVENT(IP_RTP_LPR_MODE_CHANGE_IND,       DISCONNECT, CH323Cntl::NullActionFunction)
  ONEVENT(IP_RTP_LPR_MODE_CHANGE_IND,       ANYCASE,  CH323Cntl::OnRtpLprChangeModeInd)

  ONEVENT(H323_CS_SIG_LPR_MODE_CHANGE_RES_IND,  SETUP,    CH323Cntl::NullActionFunction)
  ONEVENT(H323_CS_SIG_LPR_MODE_CHANGE_RES_IND,  DISCONNECT, CH323Cntl::NullActionFunction)
  ONEVENT(H323_CS_SIG_LPR_MODE_CHANGE_RES_IND,  ANYCASE,  CH323Cntl::OnH323LprChangeModeAckInd)

  ONEVENT(H323_CS_SIG_LPR_MODE_CHANGE_IND,  SETUP,    CH323Cntl::OnH323LprChangeModeIndSetup)
  ONEVENT(H323_CS_SIG_LPR_MODE_CHANGE_IND,  DISCONNECT, CH323Cntl::OnH323LprChangeModeIndSetup)
  ONEVENT(H323_CS_SIG_LPR_MODE_CHANGE_IND,  ANYCASE,  CH323Cntl::OnH323LprChangeModeInd)

  ONEVENT(LPRTOUT,              SETUP,    CH323Cntl::NullActionFunction)
  ONEVENT(LPRTOUT,              DISCONNECT, CH323Cntl::NullActionFunction)
  ONEVENT(LPRTOUT,              ANYCASE,  CH323Cntl::OnH323LprTout)

  ONEVENT(LOCALLPRTOUT,             SETUP,    CH323Cntl::NullActionFunction)
  ONEVENT(LOCALLPRTOUT,             DISCONNECT, CH323Cntl::NullActionFunction)
  ONEVENT(LOCALLPRTOUT,             ANYCASE,  CH323Cntl::OnRtpLprTout)

  ONEVENT(CAPABILITIESTOUT,             SETUP,  CH323Cntl::OnCapabilitiesTout)

  ONEVENT(IP_RTP_ASK_ENCODER_FOR_INTRA_IND,   CONNECT,  CH323Cntl::OnH323RtpSelfVideoUpdatePicReq)
    ONEVENT(IP_RTP_ASK_ENCODER_FOR_INTRA_IND,   ANYCASE,  CH323Cntl::NullActionFunction)

  //Multiple links for ITP in cascaded conference feature  //added by Jason
  ONEVENT(H323_CS_SIG_NEW_ITP_SPEAKER_ACK_IND,  	DISCONNECT, 	CH323Cntl::NullActionFunction)
  ONEVENT(H323_CS_SIG_NEW_ITP_SPEAKER_ACK_IND,  	ANYCASE,  		CH323Cntl::OnH323NewITPSpeakerAckInd)
  ONEVENT(H323_CS_SIG_NEW_ITP_SPEAKER_IND,  		DISCONNECT, 	CH323Cntl::NullActionFunction)
  ONEVENT(H323_CS_SIG_NEW_ITP_SPEAKER_IND,  		ANYCASE,  		CH323Cntl::OnH323NewITPSpeakerInd)

  ONEVENT(CONF_PARTY_MRMP_RTCP_FIR_IND,             ANYCASE,        CH323Cntl::OnMrmpRtcpFirInd)

  ONEVENT(IP_CM_MEDIA_DISCONNECTED_IND,  CONNECT,		CH323Cntl::OnMediaDisconnectDetectionInd)
  ONEVENT(IP_CM_MEDIA_DISCONNECTED_IND,  CHANGEMODE,		CH323Cntl::OnMediaDisconnectDetectionInd)
  ONEVENT(IP_CM_MEDIA_DISCONNECTED_IND,  ANYCASE, 	CH323Cntl::OnMediaDisconnectDetectionIndInAnycase)
  ONEVENT(MEDIA_DISCONNECTION_RESUME_CELL_TIMER, ANYCASE, CH323Cntl::OnMediaResume)

  PEND_MESSAGE_MAP(CH323Cntl,CIpCntl);

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Decides about the outgoing channels to be opened.
//---------------------------------------------------------------------------------------------------
EResult CH323Cntl::BuildOpenChannelStruct(CCapSetInfo capInfo,ERoleLabel eRole,BYTE bMCMSOrigin,DWORD bitRate,channelSpecificParameters *pOutChannelParams, BYTE bIsNeedToImprove)
{
  EResult eResOfSet = kFailure;
  CBaseCap* pBaseCap = NULL;
  capBuffer* pCapBuffer = NULL;
  cmCapDataType eType = capInfo.GetCapType();

  cmCapDirection eDirection;
  if (bMCMSOrigin || (m_pTargetModeH323->GetConfType() == kCop && eRole == kRolePeople))
    eDirection = cmCapTransmit;
  else
    eDirection = cmCapReceive;

  int length = 0;
  if (m_pTargetModeH323->IsMediaOn(eType,eDirection,eRole))
  {
    length = m_pTargetModeH323->GetMediaLength(eType,eDirection,eRole);
    pCapBuffer = (capBuffer *)new BYTE[length + sizeof(capBufferBase)];
    m_pTargetModeH323->CopyMediaToCapBuffer(pCapBuffer,eType,eDirection,eRole);
    pBaseCap = CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE*)pCapBuffer->dataCap);
  }
  else
  {
      TRACESTR(eLevelInfoNormal) << " CH323Cntl::BuildOpenChannelStruct: Target mode is off!!! "
                 << GetDirectionStr(eDirection) << ",  Name - " << PARTYNAME;
    // PTRACE2(eLevelInfoNormal,"CH323Cntl::BuildOpenChannelStruct: Target mode is off!!! ",GetDirectionStr(eDirection));
    return eResOfSet;
  }


  if (capInfo.IsType(cmCapAudio))
  {
    int maxFramePerPacket = 0;
    int remoteMaxFramePerPacket = 0;
    int minFramePerPacket = 0;
    int remoteMinFramePerPacket = 0;
    CBaseAudioCap* pOutAudioCap = (CBaseAudioCap *)CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE*)pOutChannelParams);
    if (pOutAudioCap)
    {
      maxFramePerPacket   = capInfo.GetMaxFramePerPacket(); //local's
      remoteMaxFramePerPacket = m_pRmtCapH323->GetMaxAudioFramePerPacket(capInfo,TRUE);//get also for advanced search
      maxFramePerPacket = min(maxFramePerPacket,remoteMaxFramePerPacket);
      maxFramePerPacket = capInfo.GetFramePerPacketQuotient(maxFramePerPacket);

      minFramePerPacket   = m_pLocalCapH323->GetMinAudioFramePerPacket(capInfo);
      remoteMinFramePerPacket = m_pRmtCapH323->GetMinAudioFramePerPacket(capInfo);
      minFramePerPacket = max(minFramePerPacket,remoteMinFramePerPacket);
      minFramePerPacket = capInfo.GetFramePerPacketQuotient(minFramePerPacket);

      if (maxFramePerPacket != NA && minFramePerPacket != NA)
      {
        eResOfSet = kSuccess;
        eResOfSet &= pOutAudioCap->SetStruct(cmCapTransmit,maxFramePerPacket,minFramePerPacket);

        if (eResOfSet == kFailure)
          PTRACE(eLevelInfoNormal,"CH323Cntl::BuildOpenChannelStruct : Couldn't set audio struct!!");
      }
      else
        PTRACE(eLevelInfoNormal,"CH323Cntl::BuildOpenChannelStruct: frame per packet is zero or not supported!!!");
    }
    POBJDELETE(pOutAudioCap);
  }

  else if (capInfo.IsType(cmCapVideo))
  {
    CBaseVideoCap* pOutVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE*)pOutChannelParams);
    if (pOutVideoCap)
    {
      eResOfSet = kSuccess;
      eResOfSet &= pOutVideoCap->SetDefaults(cmCapTransmit,eRole);

      if (pBaseCap) //the outgoing channel is opened from the target mode
        eResOfSet &= pOutVideoCap->CopyQualities(*pBaseCap);

      eResOfSet &= pOutVideoCap->SetBitRate(bitRate);
      pOutVideoCap->SetAdditionalXmlInfo();

//      COstrStream msg;
//      pBaseCap->Dump(msg);
//      pOutVideoCap->Dump(msg);
//      PTRACE2(eLevelInfoNormal,"CH323Cntl::BuildOpenChannelStruct - channel params ", msg.str().c_str());

      if (eResOfSet == kFailure)
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::BuildOpenChannelStruct : Couldn't set video struct!!");
      }
    }
    POBJDELETE(pOutVideoCap);
  }

  else if (capInfo.IsType(cmCapData))
  {
    CBaseDataCap* pOutDataCap = (CBaseDataCap *)CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE*)pOutChannelParams);
    if (pOutDataCap)
    {
      eResOfSet = kSuccess;
      eResOfSet &= pOutDataCap->SetStruct(cmCapData, cmCapTransmit, kRolePeople);
      eResOfSet &= pOutDataCap->SetBitRate(bitRate);

      if (eResOfSet == kFailure)
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::BuildOpenChannelStruct : Couldn't set data struct!!");
      }
    }
    POBJDELETE(pOutDataCap);
  }
  else
      TRACESTR(eLevelInfoNormal) <<" CH323Cntl::BuildOpenChannelStruct: Unknown alg " << capInfo.GetH323CapName() << ",  Name - " << PARTYNAME;
    //PTRACE2(eLevelInfoNormal,"CH323Cntl::BuildOpenChannelStruct: Unknown alg ",capInfo.GetH323CapName());

  POBJDELETE(pBaseCap);
  PDELETEA(pCapBuffer);
  return eResOfSet;
}


/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
  DispatchEvent(opCode,pMsg);
}

//////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetOrign(BOOL isOrigin)
{
  m_pmcCall->SetIsOrigin(isOrigin);
}

/////////////////////////////////////////////////////////////////////////////
CH323Cntl::CH323Cntl(CTaskApp* pOwnerTask)    // constructor
  :CIpCntl(pOwnerTask)
{
  m_pLocalCapH323   = new CCapH323;
  m_pRmtCapH323   = new CCapH323;
  m_pTargetModeH323 = new CComModeH323;
  m_pCurrentModeH323  = new CComModeH323;

  m_pDHKeyManagement  = new CIpDHKeyManagement;

  m_pParty            = NULL;
  m_pTaskApi            = new CPartyApi;
  m_pH323NetSetup         = new CH323NetSetup;
  m_pmcCall             = new CCall;
  //m_pNewInChanSeg                 = NULL;
  for (int i = 0; i < NUM_OF_MEDIA_TYPES; i++)
    m_pNewInChanSeg[i] = NULL;

  m_combineOpcode         = 0;
  m_channelTblIndex       = 0;
  m_isAudioOutgoingChannelConnected = 0;
  m_isVideoOutgoingChannelConnected = 0;
  m_isDataOutgoingChannelConnected  = 0;
  m_isVideoContentOutgoingChannelConnected = 0;
  m_isIncomingAudioHasDisconnectedOnce = FALSE;
  m_isOutgoingAudioHasDisconnectedOnce = FALSE;
  m_isCallDropRequestWaiting      = 0;
  m_CallConnectionState       = Idle;

  m_encAlg              = kUnKnownMediaType;
  m_isH245Connected         = FALSE;
  m_isCallingThroughGk        = 0;
  m_isReceiveCallDropMessage        = 0;
  m_isReceiveCallIdleMessage      = 0;
  m_isCloseConfirm          = 0;
  m_isCallAnswerReject        = 0;
  m_bIsRemoveGeneric          = FALSE;
  m_oldMediaBytes                   = new DWORD[MaxChannelsPerCall+2];//+2 actually the +2 looks like a mistake. We use those array in the monitoring functions.
    m_oldFrames                       = new DWORD[MaxChannelsPerCall+2];// and those function has an adjustment between the GUI value of the channel (which is +2 because of 225 and 245) to the location in the array.
  for (int i=0;i<MaxChannelsPerCall+2;i++)
  {
    m_oldMediaBytes[i]=0;
    m_oldFrames[i]=0;
  }
  m_pQos = new CQoS;

  m_McmsOpenChannels     = 0;
  //m_OneOfTheMediaChannelWasConnected = 0;
    m_maxCallChannel = g_kCallWithoutTwoMedia;

  m_remoteIdent = Regular;

  m_bVideoInRejected     = FALSE;
  m_bVideoOutRejected    = FALSE;
  m_NumOfGetPortMsg = 0;
  m_isAudioConnected     = 0;
  m_CapabilityNegotiation  = kInitialCapNegotiation;

//  m_PNGkState        = ePnOff;
  m_gkRequestedBrqBw = 0;

//  m_pLoadMngrConnector = NULL;

  //EPC
  m_bIsContentSpeaker = FALSE;
  m_curConfContRate = 0;

  m_targetConfContRate = 0;
  m_curPeopleRate = 0;
//  m_curPeopleTdmRate = 0;

  m_bWaitForFlowCntlIndIndOnPeople   = FALSE;
  m_bWaitForFlowCntlIndIndOnContent  = FALSE;
  m_bNeedToAnswerToMasterOnPeople    = FALSE;
  m_bNeedToAnswerToMasterOnContent   = FALSE;
  m_bFirstFlowControlIndIndOnContent = TRUE;
  m_bLinkFirstFlowControlIndIndOnContent = TRUE;
  m_bIsStreamOffContentNeeded        = FALSE;

  m_bContentInClosedWhileChangeVidMode = FALSE;

  m_bIsOutContentChanReject = FALSE;
  m_bIsContentRejected    = FALSE;
  m_eContentInState = eNoChannel;

//  m_numOfPartyMonitoring    = 0;
  m_bIsDataInRejected     = FALSE;
  m_bIsDataOutRejected    = FALSE;

  m_bIsAvaya            = FALSE;
  m_onGatewayCall             = FALSE;

  m_isCallConnetIndArrived  = FALSE;
  m_remoteVendor.countryCode    = 0;
  m_remoteVendor.t35Extension   = 0;
  m_remoteVendor.manufactorCode = 0;
  m_remoteVendor.productId      = new char[H460_C_ProdIdMaxSize];
  m_remoteVendor.versionId      = new char[H460_C_VerIdMaxSize];
  memset(m_remoteVendor.productId, '\0', H460_C_ProdIdMaxSize);
  memset(m_remoteVendor.versionId, '\0', H460_C_VerIdMaxSize);
  m_remoteVendor.isAvayaSipCm   = AVAYA_SIP_CM_FLAG_OFF;
  m_remoteVendor.isCopMcu = FALSE;
  m_remoteInfo.remoteEndpointType = cmEndpointTypeTerminal;
  m_remoteInfo.h225RemoteVersion  = 0;
  m_remoteInfo.endPointNetwork  = NetworkH323;

  m_remoteCapIndNotHandle = FALSE;
  m_pDestUnitId = eBalancer;
  m_callIndex = 0;
  m_serviceId = 0;
  m_isDelayedIvr = NO;

  InitSpeakerParams();

  m_keepAliveTimerCouter = 0;
  m_isKeepAliveIndArrived = 0;
  ON(m_isAutoVidBitRate);

  m_isH239FlowCntlSent = 0;
  DWORD  structLen = sizeof(mcIndGetPort);
  memset(&m_getPortInd,0,structLen);
  //m_roundTripDelayLostCounter = 0;
    m_bDisguiseAsEPMode = FALSE;
    m_mcuNumFromMaster      = 0;
    m_terminalNumFromMaster = 0;

  // VNGR-787
  m_isCodianVcr = 0;
  m_isRealIncVidChanSentFromCodianVcr = 0;
   m_isLprModeOn = 0;
   m_LprModeTimeout = 0;

    //Flow Control constraint
    m_confPeopleFlowControlConstraint = 0;

    m_realLprRate = 0;
    m_isLprContentForceReductionTo64 = 0;

    // since the outgoing channel is close and re-open we must save LPR data and re-send it to the RTP.
	m_lprModeChangeData.lossProtection = 0;
	m_lprModeChangeData.mtbf = 0;
	m_lprModeChangeData.congestionCeiling = 0;
	m_lprModeChangeData.fill = 0;
	m_lprModeChangeData.modeTimeout = 0;

    m_LastContentRateFromMaster    = 0; // for the content coming from the master side (slave)
    m_lastContentRateFromMasterForThisToken = 0; // for the content coming from our side (slave)
    // V4.1c <--> V6 merge m_lastContentRateToSlave = 0;

    m_isStreaming = FALSE;
    m_pExchangeConfId = NULL;
    m_pCopVideoModes = NULL;
    m_pCopRemoteVideoModes = NULL;
    m_FixVideoRateAccordingToType = FALSE;

   if (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator)
   {
    PTRACE(eLevelInfoNormal,"CH323Cntl::constructor - Call Generator - act as reg ep!");
    m_bDisguiseAsEPMode = TRUE;
   }
    m_isCameraControl = 0;
    m_useRtcp = 1;

    m_ChannelsWithLprPayload  = NO;
    m_isAlreadySentMultipointToMGC = FALSE;
    m_RtcpCnameMask = 0x0001;//1 in the LSD indicates we are MCU and not EP
    m_IsNeedToExtractInfoFromRtcpCname = TRUE;

    //================================
    // Video quality related members
    //================================
	m_cmInboundPacketLossStatus		= ePacketLossNormal;;
	m_cmOutboundPacketLossStatus	= ePacketLossNormal;;
    m_adjInboundPacketLossStatus	= ePacketLossNormal;
    m_adjOutboundPacketLossStatus	= ePacketLossNormal;
    m_inboundLprActive 				= FALSE;
    m_outboundLprActive 			= FALSE;

    m_isContentOn = NO;
    m_isAudioMuted = NO;
    m_isVideoMuted = NO;

    //Multiple links for ITP in cascaded conference feature:
    m_linkType = eRegularParty;

  VALIDATEMESSAGEMAP;
}

/////////////////////////////////////////////////////////////////////////////
CH323Cntl::~CH323Cntl()     // destructor
{
//  DestroyTimer();
    DeleteAllTimers();

/*  if ((m_numOfPartyMonitoring > 0) && (CParty::numberOfPm[m_pRsrcDesc->m_boardId] > 0))
  {
    int res = CParty::numberOfPm[m_pRsrcDesc->m_boardId] - m_numOfPartyMonitoring;

    CParty::numberOfPm[m_pRsrcDesc->m_boardId] = (res > 0)? res : 0;
  }
*/
  POBJDELETE(m_pTaskApi);
  POBJDELETE(m_pH323NetSetup);
  //PDELETE (m_pNewInChanSeg);
  for (int i = 0; i < NUM_OF_MEDIA_TYPES; i++)
  {
    if (m_pNewInChanSeg[i] != NULL)
    {
      POBJDELETE(m_pNewInChanSeg[i]);
      m_pNewInChanSeg[i] = NULL;
    }
    }

  POBJDELETE(m_pLocalCapH323);
  POBJDELETE(m_pRmtCapH323);
  POBJDELETE(m_pTargetModeH323);
  POBJDELETE(m_pCurrentModeH323);
/*  POBJDELETE(m_pLoadMngrConnector);*/

  PDELETEA (m_oldMediaBytes);
  PDELETEA (m_oldFrames);

  POBJDELETE(m_pDHKeyManagement);

  POBJDELETE(m_pQos);
  PDELETEA(m_remoteVendor.productId);
  PDELETEA(m_remoteVendor.versionId);
  POBJDELETE(m_pmcCall);
  POBJDELETE(m_pCopVideoModes);
  POBJDELETE(m_pCopRemoteVideoModes);


  PDELETEA(m_pExchangeConfId);// Note: we can delete it after sending the information to RSS.
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void*  CH323Cntl::GetMessageMap()
{
  return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::Dump(std::ostream& msg) const
{

  msg << "\nCH323Cntl::Dump\n"
      << "-----------\n";

/*      << setw(20) << "this"             << (hex) << (DWORD)this                    << "\n"
      << setw(20) << "m_pParty"         << (hex) << (DWORD)m_pParty                << "\n"
      << setw(20) << "m_pRsrcDesc  "    << (hex) << (DWORD)m_pIpDesc             << "\n"

      << setw(20) << "m_pCurComMode  "     << (hex) << (DWORD)m_pCurrentModeH323   << "\n"
      << setw(20) << "m_pTargetComMode AC "  << (hex) << (DWORD)m_pTargetModeH323    << "\n"
      << setw(20) << "m_pRmtXmitComMode  " << (hex) << (DWORD)m_pRmtXmitComMode    << "\n"
      << setw(20) << "m_pCap  "            << (hex) << (DWORD)m_pCap               << "\n"
      << setw(20) << "party name"       << (hex) << PARTYNAME                      << "\n"
      << setw(20) << "m_state   "       << (hex) << STATEASSTRING                  << "\n";



  PTRACE(eLevelInfoNormal,msg.str());
//  CLOSEOSTRSTREAM;*/

}
/////////////////////////////////////////////////////////////////////////////

// reject dial in flow
void  CH323Cntl::CreateForReject(CH323Party* pParty, CH323NetSetup& pH323NetSetup, CCapH323& capH323,
            CComModeH323* pInitialModeH323, WORD isLocalCap,char *pSid,
            DWORD serviceId,BYTE isAutoVidBitRate, WORD room_id,eTypeOfLinkParty linkType)
{
	Create(pParty, pH323NetSetup, capH323, pInitialModeH323, isLocalCap,
			pSid, serviceId,isAutoVidBitRate, room_id, linkType);
}

/////////////////////////////////////////////////////////////////////////////
// dial out and dial in accept call flow
void  CH323Cntl::CreateForAccept(CH323Party* pParty, CH323NetSetup& pH323NetSetup, CCapH323& capH323,
            CComModeH323* pInitialModeH323, WORD isLocalCap,char *pSid,
            DWORD serviceId,BYTE isAutoVidBitRate, WORD room_id,eTypeOfLinkParty linkType)
{
	Create(pParty, pH323NetSetup, capH323, pInitialModeH323, isLocalCap,
			pSid, serviceId,isAutoVidBitRate, room_id, linkType);
	//add for CG_SoftMCU
	if (IsCallGeneratorConf())
	{
		PTRACE(eLevelInfoNormal,"CH323Cntl::Create - Call Generator - act as reg ep!");
		m_bDisguiseAsEPMode = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::Create(CH323Party* pParty, CH323NetSetup& pH323NetSetup, CCapH323& capH323,
            CComModeH323* pInitialModeH323, WORD isLocalCap,char *pSid,
            DWORD serviceId,BYTE isAutoVidBitRate, WORD room_id,eTypeOfLinkParty linkType)
{
	m_linkType		  = linkType;
	m_pParty		  = pParty;

	*m_pH323NetSetup  = pH323NetSetup;
	BYTE stackUnitId  = 1;
	m_pDHKeyManagement->SetGenerator(POLYCOM_DH_GENERATOR);

	if(isLocalCap)// case of dial out initiate the capability
	{
		m_maxCallChannel = g_kCallWithoutTwoMedia;
		*m_pLocalCapH323 = capH323;
		if(m_pLocalCapH323->OnType(cmCapData))
			m_maxCallChannel += 2;
		if(m_pLocalCapH323->IsH239())
			m_maxCallChannel += 2;
	}

	if(pInitialModeH323)
		UpdateTargetMode(pInitialModeH323);

	if( m_pTargetModeH323->GetIsLpr() == FALSE )
		m_pCurrentModeH323->SetIsLpr(FALSE);
	//if( m_encAlg == kUnKnownMediaType )
	//	m_pCurrentModeH323->SetIsEncrypted(Encryp_Off);

	m_pTaskApi->CreateOnlyApi(pParty->GetRcvMbx(),this);
	m_pTaskApi->SetLocalMbx(pParty->GetLocalQueue());

	if(pSid)
		memcpy(m_sId,pSid,2);

	m_serviceId = serviceId;
	m_isAutoVidBitRate = isAutoVidBitRate;

	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
	if (pServiceParams == NULL)
	{
		PASSERTMSG(GetConnectionId(), "CH323Cntl::Create - IP Service does not exist!!!");
		return;
	}
	CONF_IP_PARAMS_S* pConfIpParamsSt = pServiceParams->GetConfIpParamsStruct();
	if (pConfIpParamsSt == NULL)
	{
		PASSERTMSG(GetConnectionId(), "CH323Cntl::Create - IP Service does not have parameters!!!");
		return;
	}
	//m_bIsAvaya = (pConfIpParamsSt->isAvfOn == TRUE) ? TRUE : FALSE;
	//if (AVF_DEBUG_MODE == TRUE)
	//	m_bIsAvaya = TRUE;

	CCall* pCall = GetCallParams();

	if (pCall)
	{
		pCall->SetMediaDetectionTimer(GetMediaDetectionTimer(CFG_KEY_H323_DETECT_DISCONNECT_TIMER));
	}

	SetTipRoomId( room_id );
	SetITPRtcpMask();

	if (m_pQos && pServiceParams)
	{
		const QOS_S& qos = pServiceParams -> GetQOS();
		m_pQos -> AdjustToService(qos);
	}
}



/////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::PrintDHToTrace(int len, BYTE *pDhDetail) const
{
  CLargeString traceHh;
  traceHh.SetFormat("%x");
  traceHh<<" 0x";
  for(int i=0;i<len;i++)
    traceHh<<(BYTE)pDhDetail[i];

  //PTRACE2(eLevelInfoNormal,"CH323Cntl::PrintDHToTrace:", traceHh.GetString());
}

////////////////////////////////////////////////////////////////////////////
HeaderToGkManagerStruct CH323Cntl::SetHeaderToGkManagerStruct(APIS32 status)
{
  HeaderToGkManagerStruct headerToGkManager;

  //for PORT_DESCRIPTION_HEADER_S:
  headerToGkManager.connId    = m_pCsRsrcDesc->GetConnectionId();
  headerToGkManager.partyId   = m_pCsRsrcDesc->GetPartyRsrcId();
  headerToGkManager.confId   = m_pCsRsrcDesc->GetConfRsrcId();

  //for CENTRAL_SIGNALING_HEADER_S:
  headerToGkManager.csId      = m_serviceId;  //temp!!
  headerToGkManager.callIndex = m_pmcCall->GetCallIndex();
  headerToGkManager.serviceId = m_serviceId;
  headerToGkManager.status    = status;

  return headerToGkManager;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendReqToGkManager(CSegment *pSeg, OPCODE opcode)
{
    TRACESTR(eLevelInfoNormal) <<" CH323Cntl::SendReqToGkManager - opcode = " << (DWORD)opcode << ",  Name - " << PARTYNAME;
  CProcessBase* pProcess = CProcessBase::GetProcess();
  if (!pProcess)
  {
    PASSERTMSG(GetConnectionId(),"CH323Cntl::SendReqToGkManager - Process not valid");
    return;
  }

  /*const COsQueue* pCSManager = pProcess->GetOtherProcessQueue(eProcessGatekeeper, eManager);
  CTaskApi api;
  api.CreateOnlyApi(*pCSManager);
  STATUS res = api.SendMsg(pSeg ,opcode);
  if (res != STATUS_OK)
    FPASSERT(opcode);*/
  CGatekeeperTaskApi api(m_serviceId);
        STATUS status = api.SendMsg(pSeg, opcode);

}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetEncrAlgType(EenMediaType encAlg)
{
  m_encAlg = encAlg;
}

////////////////////////////////////////////////////////////////////////////
//Not in Avaya mode
BOOL  CH323Cntl::HandleEncryptionSession(encryptionToken *pEncToken,APIU32 numOfTokens)
{
  BOOL bIsSuccess = TRUE;

  if(m_pmcCall->GetIsOrigin()) //dial Out
  {
    if(numOfTokens > 0) //For now we support only for one token
    {
      EenHalfKeyType  rmtHalfKeyAlg = (EenHalfKeyType)(pEncToken->tokenOID);
      m_pDHKeyManagement->SetRmtHalfKeyAlg(rmtHalfKeyAlg);

      if(rmtHalfKeyAlg == (EenHalfKeyType)m_pDHKeyManagement->GetLocalHalfKeyAlg())
      {
        BYTE* halfKey= &pEncToken->halfKey[0];
        CDHKey *pRmtHalfKey = new CDHKey;
        pRmtHalfKey->SetArray(halfKey, pEncToken->hkLen);
        m_pDHKeyManagement->SetDHRmtSharedSecret(*pRmtHalfKey);
        ::CalculateSharedSecret(m_pDHKeyManagement);
        m_pDHKeyManagement->SetEncCallKey(m_pDHKeyManagement->GetDHResultSharedSecret()->GetArray(),
			(m_pDHKeyManagement->GetDHResultSharedSecret()->GetLength() - XMIT_RCV_KEY_LENGTH));

        POBJDELETE(pRmtHalfKey);
      }
      else
      {
        //In case the remote does not has the same switch key algorithm we can't continue with the connect,so
        //we should reject the call in this point.
        if (m_pTargetModeH323->GetIsDisconnectOnEncryptionFailure())
        {
        PTRACE(eLevelInfoNormal,"CH323Cntl::HandleEncryptionSession - Call close: no matching switch key algorithm");
        m_pTaskApi->H323PartyDisConnect(REMOTE_DEVICES_SELECTED_ENCRYPTION_ALGORITHM_DOES_NOT_MATCH_THE_LOCAL_SELECTED_ENCRYPTION_ALGORITHM);
        }
        else
          PTRACE(eLevelInfoNormal,"CH323Cntl::HandleEncryptionSession - Call will not be encrypted: no matching switch key algorithm");
        bIsSuccess = FALSE;
      }
    }
    else //the endpoint do not support encryption
    {
      if (m_pTargetModeH323->GetIsDisconnectOnEncryptionFailure())
      {
      PTRACE(eLevelInfoNormal,"CH323Cntl::HandleEncryptionSession - Call close: remote does not support encryption");
      m_pTaskApi->H323PartyDisConnect(REMOTE_DEVICE_DID_NOT_OPEN_THE_ENCRYPTION_SIGNALING_CHANNEL);
      }
      else
        PTRACE(eLevelInfoNormal,"CH323Cntl::HandleEncryptionSession - Call will not be encrypted: remote does not support encryption");
      bIsSuccess = FALSE;
    }
  }
  else //dial In
  {
    DWORD localHkeyAlg  = m_pDHKeyManagement->GetLocalHalfKeyAlg();
    DWORD rmtHkeyAlg  = m_pDHKeyManagement->GetRmtHalfKeyAlg();
    //Local is encrypted but the remote did not open encrypted call.
    if(!rmtHkeyAlg && localHkeyAlg)
    {
      if (m_pTargetModeH323->GetIsDisconnectOnEncryptionFailure())
      {
      PTRACE(eLevelInfoNormal,"CH323Cntl::HandleEncryptionSession - Call close: remote does not support encryption");
      m_pTaskApi->H323PartyDisConnect(REMOTE_DEVICE_DID_NOT_OPEN_THE_ENCRYPTION_SIGNALING_CHANNEL);
      }
      else
        PTRACE(eLevelInfoNormal,"CH323Cntl::HandleEncryptionSession - Call will not be encrypted: remote does not support encryption");
      bIsSuccess = FALSE;
    }
    else if(rmtHkeyAlg != localHkeyAlg)
    {
      if (m_pTargetModeH323->GetIsDisconnectOnEncryptionFailure())
      {
      PTRACE(eLevelInfoNormal,"CH323Cntl::HandleEncryptionSession - no matching switch key algorithm");
        m_pTaskApi->H323PartyDisConnect(REMOTE_DEVICE_DID_NOT_OPEN_THE_ENCRYPTION_SIGNALING_CHANNEL);
      }
      else
        PTRACE(eLevelInfoNormal,"CH323Cntl::HandleEncryptionSession - Call will not be encrypted: no matching switch key algorithm");
      bIsSuccess = FALSE;
    }
  }
  return bIsSuccess;
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::SetNonEncPartyInEncrConf()
{
    if (m_encAlg  == kUnKnownMediaType)
        return;

  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SetNonEncPartyInEncrConf - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  // This case means that we are in an encrypted conference + The EP doesn't have encryption
  // But we will allow it to connect non-encrypted to the conference due to SysConfig flag.
  // 1. Change the m_encAlg to non.
  m_encAlg  = kUnKnownMediaType;
  // 2. Update out target mode - Removing the encryption
  m_pTargetModeH323->CreateLocalComModeECS(kUnKnownMediaType,kHalfKeyUnKnownType);
  m_pTargetModeH323->SetIsEncrypted(Encryp_Off, FALSE);
  // 3. Create new local caps from the non encrypted target mode

        CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  PASSERT_AND_RETURN(NULL == pCommConf);

  eVideoQuality vidQuality = pCommConf->GetVideoQuality();
  BYTE isH263H621inLocalCAps = m_pLocalCapH323->IsFoundOrH263H261();
  BYTE is4cifEnabledinOriginalCapsTx = FALSE;
  BYTE is4cifEnabledinOriginalCapsRx = FALSE;
  if(isH263H621inLocalCAps && m_pLocalCapH323->Get4CifMpi() != -1 )
  {
      is4cifEnabledinOriginalCapsTx = TRUE;
      PTRACE(eLevelInfoNormal, "CH323Cntl::SetNonEncPartyInEncrConf - 4ciftr");
  }
  if( isH263H621inLocalCAps && m_pLocalCapH323->GetMpi(eH263CapCode,k4Cif) != ((APIU8)-1))
  {
      is4cifEnabledinOriginalCapsRx = TRUE;
      PTRACE(eLevelInfoNormal, "CH323Cntl::SetNonEncPartyInEncrConf - rec");
  }
  POBJDELETE(m_pLocalCapH323);
  m_pLocalCapH323 = new CCapH323;
  m_pLocalCapH323->SetEncryptionAlg(kUnKnownMediaType);

  // create with default caps and enable H263 4cif according to video parameters
  if(m_pParty->GetIsVoice())
    m_pLocalCapH323->CreateAudioOnlyCap(m_pParty->GetVideoRate(), m_pTargetModeH323, PARTYNAME);
  else
  {
    BYTE highestframerate = ((BYTE)eCopVideoFrameRate_None);
    if(m_pTargetModeH323->GetConfType() == kCop)
    {
      CVidModeH323* copHighetlevel= m_pCopVideoModes->GetVideoMode(0);
      WORD profile;
      BYTE level;
      long maxMBPS,maxFS,maxDPB,maxBR,maxSAR,maxStaticMbps,brandcpb;
      copHighetlevel->GetH264Scm(profile,level,maxMBPS,maxFS,maxDPB,maxSAR,brandcpb,maxStaticMbps);
      highestframerate = GetCopFrameRateAccordingtoMbpsAndFs(level,maxMBPS,maxFS);

    }
    m_pLocalCapH323->CreateWithDefaultVideoCaps(m_pParty->GetVideoRate(),m_pTargetModeH323,PARTYNAME, vidQuality,FALSE,0/*service id*/,((ECopVideoFrameRate)highestframerate));
  }
  if(!isH263H621inLocalCAps)
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::SetNonEncPartyInEncrConf - removeh263andh261");
    m_pLocalCapH323->RemovePeopleCapSet(eH263CapCode);
      m_pLocalCapH323->RemovePeopleCapSet(eH261CapCode);
  }
  if(!is4cifEnabledinOriginalCapsTx)
    {
    m_pLocalCapH323->Set4CifMpi ((APIS8)-1);

  }
    if(!is4cifEnabledinOriginalCapsRx)
    m_pLocalCapH323->SetH263FormatMpi(k4Cif, -1, kRolePeople);

  m_pLocalCapH323->BuildSortedCap();
  m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
}

//////////////////////////////////////////////////////////////////////////////shiraITP - 16 - CH323Cntl::OnPartyCallSetupReq
void  CH323Cntl::OnPartyCallSetupReq(char *tempGkPrefixAlias, APIU32 *pDestExtraCallInfoTypes,  char *pDestInfo, char *pDestExtraCallInfo, char *pRemoteExtensionAddress)
{
    CMedString msg;
    msg << "CH323Cntl::OnPartyCallSetupReq - Party Name is " << PARTYNAME << " and McmsConnId is = " << m_pCsRsrcDesc->GetConnectionId();
  PTRACE(eLevelInfoNormal,msg.GetString());
  EIpChannelType channelType=H225;

  if (m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallSetupReq bIsClosing process : ");
    return;
  }

  m_state = SETUP;
  APIU32 lengthStructure = sizeof(mcReqCallSetup);
  //In avaya environment the m_pDHKeyManagement is initialize but we do not need to use it here.
  if(!m_bIsAvaya)
    lengthStructure += GetEncrySectionLen(m_pDHKeyManagement);

  BYTE* tmp = new BYTE[lengthStructure]; AUTO_DELETE_ARRAY(tmp);
    mcReqCallSetup* pCallSetupReq = (mcReqCallSetup*)tmp;
    memset(pCallSetupReq, 0, lengthStructure);

  // set the source address according to the destination ip address type parameters ( we need TSAP and atlist one alias )
  // IpV6

    SetSrcSigAddressAccordingToDestAddress();
  memcpy(&(pCallSetupReq->srcIpAddress.transAddr),m_pH323NetSetup->GetTaSrcPartyAddr(), sizeof(mcTransportAddress));
  //set XML params
  pCallSetupReq->srcIpAddress.unionProps.unionType = (int)m_pH323NetSetup->GetIpVersion();
  pCallSetupReq->srcIpAddress.unionProps.unionSize = sizeof(ipAddressIf);
  pCallSetupReq->srcIpAddress.transAddr.port = m_getPortInd.srcCallSignalAddress.transAddr.port;

  int isPartyNumber = 0;
  isPartyNumber = SetSrcPartyAddress(*pCallSetupReq);

  //Init cid, crv, callId:
  memcpy(pCallSetupReq->conferenceId, m_pH323NetSetup->GetH323ConfIdAsGUID(), MaxConferenceIdSize);
  memcpy(pCallSetupReq->callId, m_pH323NetSetup->GetCallId(), Size16);
  pCallSetupReq->referenceValue = m_pmcCall->GetReferenceValueForEp();
// IpV6
  if(m_pParty)
    strncat(pCallSetupReq->srcPartyAliases, m_pParty->GetNumericConfId(), MaxConferenceIdSize);
  else
    PASSERT_AND_RETURN(1);

  // give the party also a partyNumber at the end of the SrcPartyAddress. this number is allocated in the H323Part
//  if(isPartyNumber == 0)
//  {
//    ALLOCBUFFER(partyNumStr, MaxConferenceIdSize);
//    DWORD uniqueNum = 0;
//    if (m_pH323NetSetup->GetIpVersion() == eIpVersion4)
//      uniqueNum = pCallSetupReq->srcIpAddress.transAddr.addr.v4.ip;
//    else
//    {
//      for (int i = 0 ; i < 16; i++)
//        uniqueNum += pCallSetupReq->srcIpAddress.transAddr.addr.v6.ip[i];
//    }

//    ::AllocateUniqueString(partyNumStr, uniqueNum);
//    strncat(pCallSetupReq->srcPartyAliases, partyNumStr, MaxConferenceIdSize);
//    DEALLOCBUFFER(partyNumStr);
//  }

  // set the destination parameters only if the call is through GK
  if (m_isCallingThroughGk && m_pmcCall->GetCanMapAlias())
  {// set the destination parameters according to ther gk's answer
    if (pDestExtraCallInfo != NULL)
    {
    strncpy(pCallSetupReq->destExtraCallInfo, pDestExtraCallInfo, MaxAddressListSize);
    pCallSetupReq->destExtraCallInfo[MaxAddressListSize-1] = '\0';
    }
    else
    {
      PASSERT(1);
      pCallSetupReq->destExtraCallInfo[0] = '\0';
    }
    pCallSetupReq->remoteExtensionAddress[0] = '\0';
    if(pRemoteExtensionAddress)
    	strncpy(pCallSetupReq->remoteExtensionAddress, pRemoteExtensionAddress, MaxAliasLength);
    pCallSetupReq->remoteExtensionAddress[MaxAliasLength-1] = '\0';

    for( int i=0; i<MaxNumberOfAliases; i++ )
    	if (pDestExtraCallInfoTypes)
    		pCallSetupReq->destExtraCallInfoTypes[i] = pDestExtraCallInfoTypes[i];
  }
  else
  {
    for( int i=0; i<MaxNumberOfAliases; i++ )
      pCallSetupReq->destExtraCallInfoTypes[i] = 0;
    pCallSetupReq->destExtraCallInfo[0] = '\0';
    pCallSetupReq->remoteExtensionAddress[0] = '\0';
  }

  memcpy(&(pCallSetupReq->destIpAddress.transAddr),m_pH323NetSetup->GetTaDestPartyAddr(), sizeof(mcTransportAddress)) ;
  pCallSetupReq->destIpAddress.unionProps.unionType = (int)m_pH323NetSetup->GetIpVersion();
  pCallSetupReq->destIpAddress.unionProps.unionSize = sizeof(ipAddressIf);

  SetDestPartyAddress(*pCallSetupReq, pDestInfo);


  if (m_isCallingThroughGk && m_pTargetModeH323->GetConfType() == kCp) //in CP we set the call according to allocated bandwidth by gk
    pCallSetupReq->maxRate = m_pmcCall->GetBandwidth() / 2; //gk returns bw for both sides
  else
    pCallSetupReq->maxRate = m_pH323NetSetup->GetMaxRate();

  m_pmcCall->SetRate(pCallSetupReq->maxRate);
  m_pmcCall->SetMaxRate(pCallSetupReq->maxRate); // set the rate of the CCall like Dial in.
  pCallSetupReq->minRate       = m_pH323NetSetup->GetMinRate();

    if (strstr(PARTYNAME,"LinkToMaster") || m_pParty->GetCascadeMode() == SLAVE || m_pParty->IsGateway())
        pCallSetupReq->localEndpointType = cmEndpointTypeGateway; //cmEndpointTypeMCU;// the type is MCU in any working case. ignoring the previous setting.
    else if ( m_pParty->IsCallGeneratorParty() )
      pCallSetupReq->localEndpointType = cmEndpointTypeTerminal;
    else
        pCallSetupReq->localEndpointType = cmEndpointTypeMCU;
  if (m_pmcCall->GetSrcTerminalParams().callSignalAddress.ipVersion == eIpVersion4)
    pCallSetupReq->haCall = (void*)(m_pmcCall->GetSrcTerminalParams().callSignalAddress.addr.v4.ip);
  else
    pCallSetupReq->haCall = (void*)(m_pmcCall->GetSrcTerminalParams().callSignalAddress.addr.v6.ip);

  strncpy(pCallSetupReq->callTransient.sDisplay, m_pH323NetSetup->GetLocalDisplayName(), MaxDisplaySize);
  pCallSetupReq->callTransient.sDisplay[MaxDisplaySize-1] = '\0';
  pCallSetupReq->callTransient.sDisplaySize = strlen(m_pH323NetSetup->GetLocalDisplayName());

  pCallSetupReq->type = m_pmcCall->GetCallType();

  // telling the card if it can be an Audio Only, EPC or a Duo call
  pCallSetupReq->conferenceType = confTypeUnspecified;
  if (m_pParty->GetIsVoice())
    pCallSetupReq->conferenceType |= confTypeAudioOnly;
  else if (m_pParty->IsH239Conf())
  {
    pCallSetupReq->conferenceType |= confTypeH239;
//    pCallSetupReq->conferenceType |= confTypePPCVersion1; // in carmel GL1 there is no P+C only H239
  }

  if(m_pLocalCapH323->IsFECC())
    pCallSetupReq->conferenceType |= confTypeFECC;

  int userUserSize = 0;
  pCallSetupReq->callTransient.userUser[0]  = '\0';

  if(m_pTargetModeH323->GetConfType()== kCop)
  {
    userUserSize = SetUserUserFieldByOpcode(pCallSetupReq->callTransient.userUser, COP, NULL);
  }
  else if (m_linkType != eRegularParty && m_pParty)
  {
      //Multiple links for ITP in cascaded conference feature: CH323Cntl::OnPartyCallSetupReq - set useruser field
      CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
      if (pCommConf)
      {
          CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());

          if (pConfParty)
          {
              char strForITPcascade[H243_NAME_LEN];
              CreateStrForITPcascade(strForITPcascade,m_linkType,m_pParty->GetName(),pConfParty->GetCascadedLinksNumber(),pConfParty->GetMainPartyNumber());
              userUserSize = SetUserUserFieldForMultipleLinksForITPcascadedConf(pCallSetupReq->callTransient.userUser,strForITPcascade);
          }
          else
          {
              PTRACE(eLevelError, "ITP_CASCADE: CH323Cntl::OnPartyCallSetupReq ERROR - pConfParty is NULL");
              PASSERTMSG(1,"ITP_CASCADE: CH323Cntl::OnPartyCallSetupReq ERROR - pConfParty is NULL");
          }
      }
      else
      {
          PTRACE(eLevelError, "ITP_CASCADE: CH323Cntl::OnPartyCallSetupReq ERROR - pCommConf is NULL");
          PASSERTMSG(1,"ITP_CASCADE: CH323Cntl::OnPartyCallSetupReq ERROR - pCommConf is NULL");
      }
  }

  //in VSW add to the useruser the conf typstee and its rate.
  if((m_pTargetModeH323->GetConfType() == kVideoSwitch || kVSW_Fixed == m_pTargetModeH323->GetConfType()))//VSW or SWCP
  {
    char stringVidRate[6];
    snprintf(stringVidRate,sizeof(stringVidRate), "%d", m_pParty->GetVideoRate());
    int opcode = ( (m_pParty->IsIpOnlyConf()) ? IpOnly : VsMixed ) ;
    userUserSize = SetUserUserFieldByOpcode(pCallSetupReq->callTransient.userUser, opcode, stringVidRate);
  }

  if (tempGkPrefixAlias)// add for calls through RRQ AS GW method.
    userUserSize = SetUserUserFieldByOpcode(pCallSetupReq->callTransient.userUser, RRQasGW320, tempGkPrefixAlias);

  pCallSetupReq->callTransient.userUserSize = userUserSize;

  //encryption section--------------------------

  if(!m_bIsAvaya)
    SetEncryptionParams(pCallSetupReq->encryTokens,m_pmcCall->GetIsOrigin(),m_pDHKeyManagement);
  else
  {
    SetEncryptionInStructToZero(pCallSetupReq->encryTokens);
    pCallSetupReq->avfFeVndIdReq.fsId = H460_K_FsId_Avaya;
  }
    if (strstr(PARTYNAME,"LinkToMaster") || m_pParty->GetCascadeMode() == SLAVE || m_pParty->IsGateway())
        pCallSetupReq->bIsCascade = SLAVE;
    else if (m_pParty->GetCascadeMode() == MASTER)
        pCallSetupReq->bIsCascade = MASTER;
    else
        pCallSetupReq->bIsCascade = NONE;

  // in MGC it was set to zero. NM can't support calls when its value is different than zero therefore we hardcoded the parameters to zero (VNGR-2181)
  pCallSetupReq->bIsActiveMc = 0;//m_pH323NetSetup->GetbIsActiveMc();

  pCallSetupReq->callGoal = cmCreate;
  pCallSetupReq->localTerminalType = GetMasterSlaveTerminalType();

  //add for CG_SoftMCU
  if (IsCallGeneratorConf())
  {
	pCallSetupReq->callGeneratorParams.bIsCallGenerator=1;
  	pCallSetupReq->callGeneratorParams.eEndpointModel=endpointModelHDX9000;
  }

  LogicalChannelUpdate((DWORD)channelType);
  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)(pCallSetupReq),lengthStructure);

  m_pCsInterface->SendMsgToCS(H323_CS_SIG_CALL_SETUP_REQ,pMsg,m_serviceId, (m_pH323NetSetup->IsItPrimaryNetwork()? m_serviceId : m_pH323NetSetup->GetSubServiceId()),m_pDestUnitId,m_pmcCall->GetCallIndex(),0,0,0);


  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  if (pCommConf)
  {
	  TRACEINTO << "OnPartyCallSetupReq  CCommConf not null = ";//
	  CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
	  if (pConfParty)
	  {
		  TRACEINTO << "OnPartyCallSetupReq GetGuidFormat(pCallSetupReq->callId = " <<GetGuidFormat(pCallSetupReq->callId);//

		  pConfParty->SetCorrelationId(GetGuidFormat(pCallSetupReq->callId));
		  PlcmCdrEventCallStartExtendedHelper cdrEventCallStartExtendedHelper;
		  cdrEventCallStartExtendedHelper.SetNewIsdnUndefinedParty_BasicAndContinue(*pConfParty, H323_INTERFACE_TYPE, *pCommConf);
		  pCommConf->SendCdrEvendToCdrManager((ApiBaseObjectPtr)&cdrEventCallStartExtendedHelper.GetCdrObject(), false, cdrEventCallStartExtendedHelper.GetCdrObject().m_partyDetails.m_id);

		  //send event to cdr only here because only here we get the correlationID
		  PlcmCdrEventConfUserDataUpdate ConfUserDataUpdateEvent;
		  ConfUserDataUpdateEvent.m_userDefinedInformation.m_contactInfoList.m_contactInfo = pConfParty->GetUserDefinedInfo(0);
		  ConfUserDataUpdateEvent.m_userDefinedInformation.m_contactInfoList.m_contactInfo2 = pConfParty->GetUserDefinedInfo(1);
		  ConfUserDataUpdateEvent.m_userDefinedInformation.m_contactInfoList.m_contactInfo3 = pConfParty->GetUserDefinedInfo(2);
		  ConfUserDataUpdateEvent.m_userDefinedInformation.m_contactInfoList.m_contactInfo4 = pConfParty->GetUserDefinedInfo(3);
		  ConfUserDataUpdateEvent.m_userDefinedInformation.m_vip = false;
		  pCommConf->SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfUserDataUpdateEvent, false, pConfParty->GetPartyId());
		  pCommConf->PartyCorrelationDataToCDR(pConfParty->GetName(), m_pParty->GetMonitorPartyId(), pConfParty->GetCorrelationId());
	  }
  }
  POBJDELETE(pMsg);
}
//////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323Cntl::CreateStrForITPcascade
void  CH323Cntl::CreateStrForITPcascade(char* pNewStr, eTypeOfLinkParty linkType, const char* partyName,DWORD cascadedLinksNumber, DWORD unrsrvMainLinkDialInNumber)
{
    if (linkType == eMainLinkParty)
        strcpy(pNewStr,"Main,");
    else
        strcpy(pNewStr,"Sub,");

    char* index  = (char*)strrchr(partyName,'_');
    char  numOfLinks [5];
    snprintf(numOfLinks,sizeof(numOfLinks),"%d",cascadedLinksNumber);
    char  mainLinkDialInNumber [5];
    snprintf(mainLinkDialInNumber,sizeof(mainLinkDialInNumber),"%d",unrsrvMainLinkDialInNumber);

    if (index && index+1)
    {
        strcat(pNewStr,index+1);

        strcat(pNewStr,",");

        strcat(pNewStr,numOfLinks);

        strcat(pNewStr,",");

        strcat(pNewStr,mainLinkDialInNumber);

        strcat(pNewStr,"\0");

        //we can write the same in one line - need to check..
        //sprintf(pNewStr, "%s,%s,%d,%d", linkType == eMainLinkParty?"Main":"Slave", index+1, cascadedLinksNumber ,unrsrvMainLinkDialInNumber);

        //PTRACE2(eLevelInfoNormal,"ITP_CASCADE: CH323Cntl::CreateStrForITPcascade pNewStr ",pNewStr);
    }
    else
    {
        PTRACE(eLevelError, "ITP_CASCADE: CH323Cntl::CreateStrForITPcascade ERROR - index or index+1 is NULL");
        PASSERTMSG(1,"ITP_CASCADE: CH323Cntl::CreateStrForITPcascade ERROR - index or index+1 is NULL");
    }

}
//////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323Cntl::SetUserUserFieldForMultipleLinksForITPcascadedConf
int CH323Cntl::SetUserUserFieldForMultipleLinksForITPcascadedConf(char *pUserUserString, char *pAddedString)
{
  int AddedStringSize;

  AddedStringSize = strlen(pAddedString);

  // check if the added string will cause overloding the max field size.
  // + 1 is for null ("\0") terminated string
  if( (strlen(pUserUserString) + AddedStringSize + 1) > MaxUserUserSize ) //in ApiCom (128)
    return 0;

  strcat(pUserUserString, pAddedString);
  return strlen(pUserUserString);
}

////////////////////////////////////////////////////////////////////////////
//Add for CallSetUpReq function:
// The call setup structure is filled with aliases and at the end we put party number.
int  CH323Cntl::SetSrcPartyAddress (mcReqCallSetup& pCallSetupReq)
{
  int aliasType , i = 0, isPartyNumber = 0;

  char* pRear = NULL;
  char* pInfo = m_pmcCall->GetSourceInfoAlias();
  if (pInfo[0] == '\0')
    return isPartyNumber;

  char  PartyNumber[ALIAS_NAME_LEN];

  memset(&PartyNumber[0],0,ALIAS_NAME_LEN);
  DWORD* SrcInfoAliasType = m_pmcCall->GetSrcInfoAliasType();
  while(pInfo)
  {
    pRear = strchr( pInfo, ',');

    aliasType = SrcInfoAliasType[i];

    if ( aliasType<7 || aliasType>14 )
      return 0;
    switch(aliasType)
    { // IpV6 - Temp
      case PARTY_H323_ALIAS_H323_ID_TYPE:
        strncat(pCallSetupReq.srcPartyAliases, "NAME:", 5);
        break;
      case PARTY_H323_ALIAS_E164_TYPE:
        strncat(pCallSetupReq.srcPartyAliases, "TEL:", 4);
        isPartyNumber++;
        break;
      case PARTY_H323_ALIAS_PARTY_NUMBER_TYPE:
        isPartyNumber++;
        break;
      case PARTY_H323_ALIAS_EMAIL_ID_TYPE:
        strncat(pCallSetupReq.srcPartyAliases, "EMAIL:", 6);
          break;
      case PARTY_H323_ALIAS_URL_ID_TYPE:
        strncat(pCallSetupReq.srcPartyAliases, "URL:", 4);
        break;
    }

    if(pRear)
      strncat(pCallSetupReq.srcPartyAliases, pInfo, pRear - pInfo);
    else
      strncat(pCallSetupReq.srcPartyAliases, pInfo, (ARRAYSIZE(pCallSetupReq.srcPartyAliases)-1)-strlen(pCallSetupReq.srcPartyAliases));

    strncat(pCallSetupReq.srcPartyAliases,",",2);

    if(isPartyNumber == 1)
    {
      if((aliasType == PARTY_H323_ALIAS_E164_TYPE) || (aliasType == PARTY_H323_ALIAS_PARTY_NUMBER_TYPE))
      {
        if(pRear)
          strncpy(PartyNumber, pInfo, min(pRear - pInfo, ALIAS_NAME_LEN));
        else
          strncpy(PartyNumber, pInfo, ALIAS_NAME_LEN);
        PartyNumber[ALIAS_NAME_LEN - 1] = '\0'; //to keep it a string
      }
    }

    i++;
    pInfo = pRear;
    if(pInfo)
      pInfo++;
  }

  if(isPartyNumber)
    strncat(pCallSetupReq.srcPartyAliases, PartyNumber, ALIAS_NAME_LEN);


  return isPartyNumber;
}
////////////////////////////////////////////////////////////////////////////////////
int  CH323Cntl::SetDstPartyAddressForAnswer (mcReqCallAnswer& pCallAnswer)
{
  int aliasType , i = 0, isPartyNumber = 0;

  char* pRear = NULL;
  char* pInfo = m_pmcCall->GetDestinationInfoAlias();
  if (pInfo[0] == '\0')
    return isPartyNumber;
  //PTRACE2(eLevelInfoNormal,"CH323Cntl::SetDstPartyAddressForAnswer - pInfo",pInfo);
  char  PartyNumber[ALIAS_NAME_LEN];

  memset(&PartyNumber[0],0,ALIAS_NAME_LEN);
  DWORD* SrcInfoAliasType = m_pmcCall->GetDestInfoAliasType();
  while(pInfo)
  {
    pRear = strchr( pInfo, ',');

    aliasType = SrcInfoAliasType[i];

    if ( aliasType<7 || aliasType>14 )
      return 0;
    switch(aliasType)
    { // IpV6 - Temp
      case PARTY_H323_ALIAS_H323_ID_TYPE:
        strncat(pCallAnswer.ConnectedAddressAliases, "NAME:", 5);
        break;
      case PARTY_H323_ALIAS_E164_TYPE:
        strncat(pCallAnswer.ConnectedAddressAliases, "TEL:", 4);
        isPartyNumber++;
        break;
      case PARTY_H323_ALIAS_PARTY_NUMBER_TYPE:
        isPartyNumber++;
        break;
      case PARTY_H323_ALIAS_EMAIL_ID_TYPE:
        strncat(pCallAnswer.ConnectedAddressAliases, "EMAIL:", 6);
          break;
      case PARTY_H323_ALIAS_URL_ID_TYPE:
        strncat(pCallAnswer.ConnectedAddressAliases, "URL:", 4);
        break;
    }

    if(pRear)
      strncat(pCallAnswer.ConnectedAddressAliases, pInfo, pRear - pInfo);
    else
      strncat(pCallAnswer.ConnectedAddressAliases, pInfo, (ARRAYSIZE(pCallAnswer.ConnectedAddressAliases)-1)-strlen(pCallAnswer.ConnectedAddressAliases));
    if(isPartyNumber == 1)
    {
      if((aliasType == PARTY_H323_ALIAS_E164_TYPE) || (aliasType == PARTY_H323_ALIAS_PARTY_NUMBER_TYPE))
      {
        if(pRear)
          strncpy(PartyNumber, pInfo, min(pRear - pInfo, ALIAS_NAME_LEN));
        else
          strncpy(PartyNumber, pInfo, ALIAS_NAME_LEN);
        PartyNumber[ALIAS_NAME_LEN - 1] = '\0'; //to keep it a string
      }
    }

    i++;
    pInfo = pRear;
    if(pInfo)
      pInfo++;
  }

  if(isPartyNumber)
    strncat(pCallAnswer.ConnectedAddressAliases, PartyNumber, ALIAS_NAME_LEN);


  return isPartyNumber;
}


/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::SetDestPartyAddress (mcReqCallSetup& pCallSetupReq, char *pDestInfo)
{
  ALLOCBUFFER(dstAddr, MaxAddressListSize); AUTO_DELETE_ARRAY(dstAddr);
  dstAddr[0] = '\0';

  BOOL flag = strlen(m_pH323NetSetup->GetH323PartyAlias());
  int aliasType = 0;
  if(flag)
  {
    aliasType = m_pH323NetSetup->GetH323PartyAliasType();
    if (aliasType<7 || aliasType>14)
      return;
  }



  if(flag) //the dest has aliases
  {
    if (aliasType == PARTY_H323_ALIAS_H323_ID_TYPE)
      strcat( dstAddr, "NAME:");
    else if (aliasType == PARTY_H323_ALIAS_E164_TYPE)
      strncat(dstAddr,"TEL:",6);
    else if (aliasType == PARTY_H323_ALIAS_EMAIL_ID_TYPE)
      strcat(dstAddr, "EMAIL:");
    else if (aliasType == PARTY_H323_ALIAS_URL_ID_TYPE)
      strcat(dstAddr, "URL:");

    //can map alias:
    if (m_pmcCall->GetCanMapAlias() && pDestInfo && (pDestInfo[0] != '\0'))
    {
        if(NULL != strchr(pDestInfo, ','))
            strncat(dstAddr, pDestInfo, strlen(pDestInfo) - 1); //without the ';' in the end
        else
            strncat(dstAddr, pDestInfo, strlen(pDestInfo));
    }
    else
        strncat(dstAddr, m_pH323NetSetup->GetH323PartyAlias(), MaxAddressListSize - strlen(dstAddr) - 1);
  }// IpV6 - Temp

  strncat(pCallSetupReq.destPartyAliases,dstAddr,MaxAddressListSize);

  DEALLOCBUFFER(dstAddr);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323CallDialToneInd(CSegment* pParam)
{
  UpdateCallString(pParam);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323CallProceedingInd(CSegment* pParam)
{
  UpdateCallString(pParam);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323CallRingBackInd(CSegment* pParam)
{
  UpdateCallString(pParam);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::UpdateCallString(CSegment* pParam)
{
  mcIndCallReport callReport ;
  memcpy(&callReport, (mcIndCallReport*)(pParam->GetPtr()), sizeof(mcIndCallReport));

  m_pmcCall->SetAnswerH245Address(callReport.h245IpAddress);
}

///////////////////////////////////////////////////////////////////////////shiraITP - 17 - CH323Cntl::OnH323CallConnectedInd
void  CH323Cntl::OnH323CallConnectedInd(CSegment* pParam)
{
    TRACESTR(eLevelInfoNormal) << " CH323Cntl::OnH323CallConnectedInd - Conn Id = " << m_pCsRsrcDesc->GetConnectionId() << ",  Name - " << PARTYNAME;
  ON(m_isCallConnetIndArrived);
  //  m_pLoadMngrConnector->CallConnectExitCriticalSection();
  ////////////parameters for party monitoring///////////////////////
  EIpChannelType channelType=H225;
  DWORD connectionStatus =1;
  DWORD actualRate=0xFFFFFFFF;
  mcTransportAddress partyAddr;
//  DWORD partyPort;
  mcTransportAddress mcuAddr;
//  DWORD mcuPort;
  memset(&partyAddr,0,sizeof(mcTransportAddress));
  memset(&mcuAddr,0,sizeof(mcTransportAddress));

  mcIndCallConnected *pCallConnectedInd = NULL;
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;// >> structSize;

  status = (APIS32)stat1;
  if(m_pmcCall->GetIsOrigin())// dial out
  {// in dial in we recive the useruser at the offering ind, in dial out at the call connected ind.
    if(!m_pmcCall->GetCallIndex())
    {
      m_callIndex = callIndex;
      m_pmcCall->SetCallIndex(m_callIndex);
    }
  }

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323CallConnectedInd - Call Index incorrect");
    return;
  }

  if (status != 0)
  {
    PASSERTMSG(status,"CH323Cntl::OnH323CallConnectedInd - status incorrect");
    return;
  }

  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323CallConnectedInd - srcUnitId incorrect");
    return;
  }

  DWORD nMsgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
  if( !nMsgLen )
    return;

  pCallConnectedInd = (mcIndCallConnected*)new BYTE[nMsgLen];
  BYTE* pMessage = new BYTE[nMsgLen];
  memset(pCallConnectedInd, 0, nMsgLen);
  memset(pMessage, 0, nMsgLen);
  pParam->Get(pMessage,nMsgLen);
  pParam->DecRead(nMsgLen);

  memcpy(pCallConnectedInd, pMessage, nMsgLen);
  PDELETEA(pMessage);

  //encryption section---------
  //Only in dail out we received the remote HK.
  if(!m_bIsAvaya)
  {
    if(m_encAlg != kUnKnownMediaType)
    {
      BOOL bIsSuccess = HandleEncryptionSession((encryptionToken*)(pCallConnectedInd->encryTokens.token),
                            pCallConnectedInd->encryTokens.numberOfTokens);
      if(!bIsSuccess)
      {
        if (m_pTargetModeH323->GetIsDisconnectOnEncryptionFailure())
        {
          PTRACE(eLevelError,"CH323Cntl::OnH323CallConnectedInd - encryption session failed");
		  delete[] pCallConnectedInd;
		  
          return;
        }
        else
        {
          m_pTaskApi->UpdateEncryptionCurrentStateInDB(NO);
          SetNonEncPartyInEncrConf();
        }
      }
      else
      {
        m_pTaskApi->UpdateEncryptionCurrentStateInDB(YES);
      }
    }
    else
    {
              PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323CallConnectedInd - no encryption for this call");
      m_pTaskApi->UpdateEncryptionCurrentStateInDB(NO);
    }
  }

  BYTE isCascadeToPreviousVer = FALSE;

  if(m_pmcCall->GetIsOrigin())
  {
      //pCallConnectedInd->userUser[pCallConnectedInd->userUserSize] = '\0';
      //PTRACE2(eLevelError, "ITP_CASCADE: CH323Cntl::OnH323CallConnectedInd useruser:",pCallConnectedInd->userUser); //shiraITP - TODO clean useruser.
      m_pmcCall->SetCallTransientUserUser(pCallConnectedInd->userUser);
  }

  bool isVendorPolycom = ChangeCapabilitySetAccordingToVendorId(pCallConnectedInd, isCascadeToPreviousVer);

  // for Call Generator - Vendor detection
  if ((CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator) &&
      ::GetVendorDetection() && !isVendorPolycom)
  {
      PASSERTMSG(callIndex,"CH323Cntl::OnH323CallConnectedInd - Call Generator - Not a Polycom manufacturer");
      delete[] pCallConnectedInd;
	  
      return;
  }

  FindSiteAndVisualNamePlusProductIdAndSendToConfLevel(pCallConnectedInd->sDisplay);// the function must be after we find the vendor ID!!!
  m_pmcCall->SetAnswerH245Address(pCallConnectedInd->h245remote);

  // IpV6 - Monitoring

  memcpy(&mcuAddr, &(pCallConnectedInd->h225local.transAddr), sizeof(mcTransportAddress));
  memcpy(&partyAddr, &(pCallConnectedInd->h225remote.transAddr), sizeof(mcTransportAddress));

  if(m_pmcCall->GetIsOrigin())//dial out
    m_pmcCall->SetDestinationEpType((cmRASEndpointType)pCallConnectedInd->remoteEndpointType);
  else
    m_pmcCall->SetSourceEpType((cmRASEndpointType)pCallConnectedInd->remoteEndpointType);

  m_pmcCall->SetRmtType(pCallConnectedInd->remoteEndpointType);

    if (m_remoteIdent != PolycomQDX && m_remoteIdent != TandbergEp)
        OnPartyCreateControl();
    else
  {
           PTRACE (eLevelInfoNormal, "CH323Cntl::OnH323CallConnectedInd - delayed caps until we receive Tandberg caps");
     //start timer to wait
     StartTimer(CAPABILITIESTOUT,5*SECOND);
  }

  SendRssRequest();

  CPrtMontrBaseParams *pPrtMonitrParams = new CPrtMontrBaseParams;
  // SetPartyMonitoringBaseParam is according to the operator CapEnum
  CCapSetInfo capInfo;
  SetPartyMonitorBaseParams(pPrtMonitrParams,channelType,actualRate,&partyAddr,&mcuAddr,(DWORD)capInfo.GetIpCapCode());

  DWORD vendorType = Regular;
  if((m_remoteIdent == EricssonVIG) || (m_remoteIdent == EricssonVigSip))
    vendorType = EricssonVIG;

  //Multiple links for ITP in cascaded conference feature: CH323Cntl::OnH323CallConnectedInd - 1.UpdateConfMainLinkIsConnected (to create sub links) 2.for dialOUT SetMainPartyNumber(example confname:s , partyname:linkToM_1)
  if(m_linkType == eMainLinkParty &&  m_pmcCall && m_pmcCall->GetIsOrigin() == TRUE && m_pParty)
  {
      //update m_mainPartyNumber field -> subs will have the same value in their field
      CCommConf*  pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

      if (pCommConf)
      {
          CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());

          if (pConfParty)
          {
              DWORD mainPartyNumber = 0;

              mainPartyNumber = (DWORD) atoi (pCallConnectedInd->userUser);
              pConfParty->SetMainPartyNumber(mainPartyNumber);

              PTRACE2INT(eLevelError, "ITP_CASCADE: CH323Cntl::OnH323CallConnectedInd mainPartyNumber:",mainPartyNumber);

              //UpdateConfMainLinkIsConnected - will create sub links
              m_pParty->UpdateConfMainLinkIsConnected();  //shiraITP - 18
          }
          else
          {
              PTRACE(eLevelError, "ITP_CASCADE: CH323Cntl::OnH323CallConnectedInd ERROR - pConfParty is NULL");
              PASSERTMSG(1,"ITP_CASCADE: CH323Cntl::OnH323CallConnectedInd ERROR - pConfParty is NULL");
          }

      }
      else
      {
          PTRACE(eLevelError, "ITP_CASCADE: CH323Cntl::OnH323CallConnectedInd ERROR - pCommConf is NULL");
          PASSERTMSG(1,"ITP_CASCADE: CH323Cntl::OnH323CallConnectedInd ERROR - pCommConf is NULL");
      }

  }

  LogicalChannelConnect(pPrtMonitrParams,(DWORD)channelType,(DWORD)vendorType);

  if(m_pmcCall)
  {
    if(m_pmcCall->GetIsOrigin() == 0)// || isEricsson))// dial in and Ericsson
      LogicalChannelUpdate((DWORD)channelType,vendorType);
  }
  else
    PASSERT(1);



  POBJDELETE(pPrtMonitrParams);
  PDELETEA(pCallConnectedInd);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::UpdatePresentationOutStream()
{
    TRACESTR(eLevelInfoNormal) << " CH323Cntl::UpdatePresentationOutStream - Conn Id = " << m_pCsRsrcDesc->GetConnectionId() << ",  Name - " << PARTYNAME;

  CChannel* pPresentationOutChannel = FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation);
  if (pPresentationOutChannel)
  {
		m_isContentOn = TRUE;
		PTRACE(eLevelInfoNormal,"CH323Cntl::UpdatePresentationOutStream - Presentation out channels was found");
    if (!pPresentationOutChannel->IsCsChannelStateDisconnecting() && pPresentationOutChannel->GetCsChannelState() != kDisconnectedState)
    {
      pPresentationOutChannel->SetStreamState(kStreamUpdate);
      BYTE isUpdate = 1;
      if (!Rtp_FillAndSendUpdatePortOpenRtpStruct(pPresentationOutChannel,isUpdate))
      {
        PASSERT(1);
        m_pTaskApi->EncryptionDisConnect(FIPS140_STATUS_FAILURE);
        return;
      }

    }
		else
		{
			   	PTRACE(eLevelInfoNormal,"CH323Cntl::UpdatePresentationOutStream - channel is in disconnecting state probably change mode- Set m_isContentOn = TRUE!!");
		}
  }

  else

    PTRACE2(eLevelError,"CH323Cntl::UpdatePresentationOutStream - Channel wasn't found, ",PARTYNAME);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::UpdateOutStream(ERoleLabel eRole)
{
    CSmallString msg;
    msg << "CH323Cntl::UpdateOutStream - Conn Id = " << m_pCsRsrcDesc->GetConnectionId() << " Role = " << eRole << ",  Name - " << PARTYNAME;
  PTRACE(eLevelInfoNormal,msg.GetString());

  CChannel* pOutChannel = FindChannelInList(cmCapVideo, TRUE, eRole );
  if (pOutChannel)
  {
    if (!pOutChannel->IsCsChannelStateDisconnecting())
    {
      pOutChannel->SetStreamState(kStreamUpdate);
      if(!Rtp_FillAndSendUpdatePortOpenRtpStruct(pOutChannel))
      {
        PASSERT(1);
        m_pTaskApi->EncryptionDisConnect(FIPS140_STATUS_FAILURE);
        return;
      }
    }
  }

  else
  {
    PTRACE2(eLevelError,"CH323Cntl::UpdateOutStream  - Channel wasn't found, ",PARTYNAME);
  }
}


////////////////////////////////////////////////////////////////////////////
// update the people and content, tdm rate and lan rate
void  CH323Cntl::SetNewVideoRateInH239Call(DWORD newContentRate)
{
    TRACESTR(eLevelError)<< " CH323Cntl::SetNewVideoRateInH239Call - new content rate = " << newContentRate << ",  Name - " << PARTYNAME;

  m_curConfContRate    = newContentRate;

//  DWORD TotalPeopleRate = m_pCurrentModeH323->GetTotalVideoRate() - m_curConfContRate;
  //PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SetNewVideoRateInH239Call :  Party total video-people rate - ",TotalPeopleRate);

  if (newContentRate != 0)
  {
    if (m_isLprModeOn)
    {
      m_curPeopleRate = m_pCurrentModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit) - m_curConfContRate;
      if (m_curPeopleRate < 640)
      {
        m_curPeopleRate = 640;
        m_isLprContentForceReductionTo64 = 1;
      }
    }
    else
    {
      m_curPeopleRate = m_pCurrentModeH323->GetTotalVideoRate() - m_curConfContRate;

    }
  }
  else
  {
    if (m_isLprModeOn)
    {
      m_curPeopleRate = m_realLprRate;
      if (m_pTargetModeH323->GetConfType() == kCp)
      m_pCurrentModeH323->SetVideoBitRate(m_realLprRate, cmCapTransmit, kRolePeople );
      if (m_isLprContentForceReductionTo64)
        m_isLprContentForceReductionTo64 = 0;

    }
    else
    {
      m_curPeopleRate = m_pCurrentModeH323->GetTotalVideoRate() - m_curConfContRate;
    }
  }

  m_pParty->UpdateContentRate(m_curConfContRate);
}


////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyContentRateChange(APIU32 newRate, BYTE bIsSpeaker)
{
  BYTE bIsFlowControlSent = FALSE;
  EStat status = statOK;

    CMedString str;
    str << "***CH323Cntl::OnPartyContentRateChange " << "new content rate " << newRate <<" speaker " << (BOOL)bIsSpeaker <<
          ", content out connected: " << (int)m_isVideoContentOutgoingChannelConnected << ",  Name - " << PARTYNAME << "\n";
    PTRACE (eLevelError, str.GetString());

  DWORD oldConfContRate = m_curConfContRate;
  SetNewVideoRateInH239Call(newRate);

  BYTE bIsH239Party = TRUE;
  CChannel* pPresentationOutChannel = FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation);
  if (!pPresentationOutChannel)
    bIsH239Party = FALSE;


  // We check those values only if the remote opened content channel.
  if (m_isVideoContentOutgoingChannelConnected)
  {
    // case that we flow control on content channel:
    // if speaker is changed or party is the speaker and the content rate is changed
    if ((m_bIsContentSpeaker != bIsSpeaker)||(bIsSpeaker && (newRate != oldConfContRate)))
    {
      DWORD flowContentRate = 0;// flow control to content in with zero
      if (bIsSpeaker) //only to the speaker - flow control to content in with m_curConfContRate
      {
        flowContentRate = m_curConfContRate;
        bIsFlowControlSent = SendFlowControlReq(cmCapVideo, FALSE, kRoleContentOrPresentation, flowContentRate);
        //The following line is commented out for JIRA GS-12818.
        //This flow control on people channel is not necessary and will cause GS issue JIRA GS-12818.
        //BYTE temp = SendFlowControlReq(cmCapVideo, FALSE, kRolePeople, m_curPeopleRate);

        if(bIsFlowControlSent)
        {
          ON(m_isH239FlowCntlSent);
          if (m_pTargetModeH323->GetConfType() == kCp)
          m_pCurrentModeH323->SetVideoBitRate(flowContentRate, cmCapReceive, kRoleContentOrPresentation);
        }
        else
        {
          //In case the channel in closing process or already closed we should update the conf as the change mode finish in
          //case we do not need to wait the people.
          PTRACE2(eLevelError,"CH323Cntl::OnPartyContentRateChange - flowControl did not sent - CONTENT : Name - ",PARTYNAME);
        }
      }
      else if (m_bIsContentSpeaker && !bIsSpeaker && newRate==0)
      {
        BYTE temp = SendFlowControlReq(cmCapVideo, FALSE, kRolePeople, m_curPeopleRate);
        m_isH239FlowCntlSent = 0;
      }
      m_bIsContentSpeaker = bIsSpeaker;
      if( (m_pTargetModeH323->GetConfType() == kVideoSwitch) || (m_pTargetModeH323->GetConfType() == kVSW_Fixed) )
      {
        BOOL bIsFlowControl = 0;
        std::string key = "VSW_CONTENT_VIDEO_RATE_REDUCTION";
        CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, bIsFlowControl);
        if( bIsFlowControl )
        {
            PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyContentRateChange - update party with rate - ", m_curPeopleRate);
            m_pTaskApi->UpdatePartyH323VideoBitRate(m_curPeopleRate, cmCapTransmit, kRolePeople);
        }
      }
    }
  }
  else
    status = statIllegal;

    if ((m_remoteIdent == PolycomRMX || m_remoteIdent == PolycomMGC) && !IsSlaveCascadeModeForH239() && !bIsFlowControlSent)
    {
        //Informative, but start timer and wait for indication from the remote anyway
        //I'm just informing the slave MCU my rate here
        PTRACE2(eLevelError,"CH323Cntl::OnPartyContentRateChange - Waiting for flow control response",PARTYNAME);
        if(m_remoteIdent == PolycomRMX)
          BYTE temp = SendFlowControlReq(cmCapVideo, TRUE, kRoleContentOrPresentation, m_curConfContRate);
        m_bWaitForFlowCntlIndIndOnContent = TRUE;
        StartTimer(H239_WAIT_FOR_UPDATE_RATE_TIMER, 2*SECOND);
    }
    else
        SendEndChangeContentToConfLevel(status);

//  PDELETEA(msgStr);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyContentSpeakerChange(BYTE bIsSpeaker, DWORD curConfContRate)
{
    TRACESTR(eLevelInfoNormal) << " CH323Cntl::OnPartyContentSpeakerChange - Conn Id = " << m_pCsRsrcDesc->GetConnectionId() << ",  Name - " << PARTYNAME;
  //In case of master conf change speaker we shouldn't send it to the SLAVE mcu - the SLAVE will send changeRate to it's
  //endpoint when it get "AcquireAck or WithdrawAck"
  if((m_pParty->GetCascadeMode() == MASTER) && (m_pmcCall->GetRmtType() == cmEndpointTypeMCU))
  {
    PTRACE2(eLevelInfoNormal,"CH323Cntl::OnPartyContentSpeakerChange - do not send MASTER to remote MCU : Name - ",PARTYNAME);
    SendEndChangeContentToConfLevel(); //act as if you received the answer (FlowControlIndInd)already.
    return;
  }
  if (bIsSpeaker && curConfContRate)
  {
    // In this case we will send flow cntl for the video+content channels of the new speaker.
    m_bIsContentSpeaker = bIsSpeaker;
//    SetNewVideoRateInH239Call(curConfContRate);
    if (m_isVideoContentOutgoingChannelConnected && !m_isH239FlowCntlSent)
    {
        BYTE bIsFlowControlSent = SendFlowControlReq(cmCapVideo, FALSE, kRoleContentOrPresentation, curConfContRate);
        BYTE temp = SendFlowControlReq(cmCapVideo, FALSE, kRolePeople, m_curPeopleRate);

        if(bIsFlowControlSent)
        {
          ON(m_isH239FlowCntlSent);
          if (m_pTargetModeH323->GetConfType() == kCp)
          m_pCurrentModeH323->SetVideoBitRate(curConfContRate, cmCapReceive, kRoleContentOrPresentation);

        }
    }
  }
  // Change only the speaker. The rate stays the same
  //  OnPartyContentRateChange(m_curConfContTdmRate,bIsSpeaker);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnContentChangeModeTimeOut(CSegment* pParam)
{
  DBGPASSERT(m_pmcCall->GetConnectionId());
  TRACESTR(eLevelInfoNormal) << " CH323Cntl::OnContentChangeModeTimeOut - Conn Id = " << m_pCsRsrcDesc->GetConnectionId() << ",  Name - " << PARTYNAME;
  SendEndChangeContentToConfLevel(statIllegal);
  if (m_bIsStreamOffContentNeeded)
  {
    OnConfStreamOffMediaChannel(cmCapVideo,cmCapReceive,kRoleContentOrPresentation);
    OnConfStreamOffMediaChannel(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation);
    m_bIsStreamOffContentNeeded = FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnTimerWaitForUpdateRate(CSegment* pParam)
{
    if (m_bWaitForFlowCntlIndIndOnContent)
    {
      TRACESTR(eLevelInfoNormal) << " CH323Cntl::OnTimerWaitForUpdateRate - Conn Id = " << m_pCsRsrcDesc->GetConnectionId() << ",  Name - " << PARTYNAME;
        if (!IsSlaveCascadeModeForH239()) //endpoint or slave link
        {
            SendEndChangeContentToConfLevel();
        }
        else
        {
            DWORD contentRate = 0;
            CChannel* pChannel = FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation);
            if (pChannel)
                contentRate = pChannel->GetRate();
            m_pTaskApi->NotifyConfOnMasterActionsRegardingContent(MASTER_START_CONTENT, contentRate);
        }
        m_bWaitForFlowCntlIndIndOnContent = FALSE;

    }


}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323FlowControlIndInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323FlowControlInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  mcIndFlowControlIndication pFlowControlIndication;
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323FlowControlIndInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323FlowControlIndInd - srcUnitId incorrect");
    return;
  }
  DWORD  structLen = sizeof(mcIndFlowControlIndication);
  memset(&pFlowControlIndication,0,structLen);
  pParam->Get((BYTE*)(&pFlowControlIndication),structLen);

  CChannel *pChannel = NULL;
  APIS8     arrayIndex = NA;
  int res = SetCurrentChannel(channelIndex,mcChannelIndex,&pChannel,&arrayIndex);

  if (res == 0) // success
  {

    if (pChannel->GetType() == cmCapVideo &&
            pChannel->GetRoleLabel() & kRoleContentOrPresentation &&
            m_bWaitForFlowCntlIndIndOnContent)
        {
            m_LastContentRateFromMaster = pFlowControlIndication.rate;
            if (m_pTargetModeH323->GetConfType() == kCp)
            m_pCurrentModeH323->SetVideoBitRate(pFlowControlIndication.rate, cmCapReceive,kRoleContentOrPresentation );
            if (IsValidTimer(H239_WAIT_FOR_UPDATE_RATE_TIMER))
                    DeleteTimer (H239_WAIT_FOR_UPDATE_RATE_TIMER);
            if (!IsSlaveCascadeModeForH239())
            {

                //Ack for a request made from master to slave (we are master, handling slave response)
                m_bWaitForFlowCntlIndIndOnContent = FALSE;
                if (pFlowControlIndication.rate <= m_curConfContRate)
                    SendEndChangeContentToConfLevel(statOK);
                else
                    SendEndChangeContentToConfLevel(statIllegal);
            }
            else if (pFlowControlIndication.rate)
            {

                //This is a requset from the master to the slave - send it to the conference
                //This should arrive only after presentation token indicate owner on our propriety implementation
                m_pTaskApi->NotifyConfOnMasterActionsRegardingContent(MASTER_START_CONTENT,
                                                                      pFlowControlIndication.rate);
                m_bWaitForFlowCntlIndIndOnContent = FALSE;
            }

        }
        else if (!m_bWaitForFlowCntlIndIndOnContent && IsSlaveCascadeModeForH239())
        {
            //in this case the master already has content active, but we will wait for the PresentationTokenIndicateOwner
            //before notifying the bridge
            m_LastContentRateFromMaster = pFlowControlIndication.rate;
            m_pCurrentModeH323->SetVideoBitRate(pFlowControlIndication.rate, cmCapReceive, kRoleContentOrPresentation);
        }
        else
            PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323FlowControlInd - Flow control not handled Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
        //m_bWaitForFlowCntlIndIndOnContent = FALSE;
    }

}


////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnPartyMediaProducerStatusReq(BYTE bIsActive,cmCapDataType eType) const
{
  // find content outgoing channel
  CChannel *pChannel = FindChannelInList(eType,TRUE,kRoleContent);
  if (pChannel)
  {
/*//currently we support only P+C
    if(pChannel->channelCloseInitiator == NoInitiator)
    {
      mcReqMediaProducerStatus *pReq = new mcReqMediaProducerStatus;

      pReq->channelID = pChannel->index;
      pReq->status    = bIsActive;

      CSegment* pMsg = new CSegment;
      pMsg->Put((BYTE*)(pReq),sizeof(mcReqMediaProducerStatus));
      m_pCsInterface->SendMsgToCS(H323_CS_SIG_MEDIA_PRODUCER_STATUS_REQ, pMsg, m_serviceId,
                  m_serviceId, m_pDestUnitId, m_callIndex, pChannel->GetCsIndex(), pChannel->GetIndex(), 0);
      POBJDELETE(pMsg);
      PDELETE(pReq);
    }
    else
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyMediaProducerStatusReq: content channel wasn't found");
*/

  }
  else
  { // find presentation outgoing channel
    pChannel = FindChannelInList(eType, TRUE, kRoleContentOrPresentation);
    if (pChannel)
    {
      if(pChannel->GetChannelCloseInitiator() == NoInitiator)
      {
        if (bIsActive)
        {//send channelActive
          mcReqChannelOn sChannelOnReq;
          sChannelOnReq.channelType   = pChannel->GetType();

          CSegment* pMsg = new CSegment;
          pMsg->Put((BYTE*)(&sChannelOnReq),sizeof(mcReqChannelOn));
          m_pCsInterface->SendMsgToCS(H323_CS_SIG_CHANNEL_ON_REQ, pMsg, m_serviceId,
                  m_serviceId, m_pDestUnitId, m_callIndex, pChannel->GetCsIndex(), pChannel->GetIndex(), 0);
          POBJDELETE(pMsg);
        }
        else
        {
          PTRACE(eLevelError,"CH323Cntl::OnPartyMediaProducerStatusReq: Presentation channel, status is inactive ");
          mcReqChannelOff sChannelOffReq;
          sChannelOffReq.channelType    = pChannel->GetType();
          CSegment* pMsg = new CSegment;
          pMsg->Put((BYTE*)(&sChannelOffReq),sizeof(mcReqChannelOff));
          m_pCsInterface->SendMsgToCS(H323_CS_SIG_CHANNEL_OFF_REQ, pMsg, m_serviceId,
                  m_serviceId, m_pDestUnitId, m_callIndex, pChannel->GetCsIndex(), pChannel->GetIndex(), 0);
          POBJDELETE(pMsg);
        }

      }
      else
        PTRACE(eLevelError,"CH323Cntl::OnPartyMediaProducerStatusReq: Presentation channel wasn't found");
    }
    else
      PTRACE(eLevelError,"CH323Cntl::OnPartyMediaProducerStatusReq: Outgoing content channel wasn't found");
  }
}


////////////////////////////////////////////////////////////////////////////
/*void  CH323Cntl::OnPartyRoleTokenReq(DWORD eRoleTokenOpcode,BYTE mcuNum,BYTE terminalNum,BYTE randomNum, BYTE isAck)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyRoleTokenReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  mcReqRoleTokenMessage *pReq = new mcReqRoleTokenMessage;
    // try to find the channel according to the message type - request = outgoing, response/ack=incoming
  BYTE isOutgoing = isContentRequest(eRoleTokenOpcode);
  CChannel* pChannel = FindChannelInList(cmCapVideo, isOutgoing, kRoleContentOrPresentation);

    //if we didn't find the channel, it might be the incoming which is not open, try the outgoing
  if (!pChannel)
    CChannel* pChannel = FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation);

  WORD SendVidFlowCntl = 0;

  //If Channel is open and not in disconnecting/closing state
  if ((pChannel) && (pChannel->GetChannelCloseInitiator() == NoInitiator))
  {
      pReq->randNumber = randomNum;
    pReq->subOpcode  = eRoleTokenOpcode;
    pReq->bIsAck     = isAck;
    if (pChannel->GetRoleLabel() == kRolePresentation)
    {
      switch (eRoleTokenOpcode)
      {
        case kPresentationTokenIndicateOwner:
          break;

        case kPresentationTokenRequest:
        {
                    if (IsSlaveCascadeModeForH239())
                        m_lastContentRateToSlave = 0;
          randomNum = 0;
          SendVidFlowCntl = 1;
          break;
        }

        case kPresentationTokenResponse:
          break;


        *** only in H239 */
/*        case kFlowControlReleaseResponse:
          break;
      }
    }
    else
      pReq->subOpcode = eRoleTokenOpcode;

    if((mcuNum == 0) && (terminalNum == 0))
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyRoleTokenReq: MCU and terminal number ZERO");
      //DBGPASSERT(pChannel->index);
    }
        if (IsSlaveCascadeModeForH239())
        {
            //Here we replace the internal numbers with the numbers we received from the master
            //Because the master is not aware of our internam MT numbers.
            pReq->mcuID         = m_mcuNumFromMaster;
            pReq->terminalID      = m_terminalNumFromMaster;
        }
        else
        {
            pReq->mcuID         = mcuNum;
            pReq->terminalID      = terminalNum;
        }

    pReq->contentProviderInfo = 0;//for now always zero (video)
    pReq->label               = LABEL_CONTENT;
    pReq->bitRate             = 0;
    CSegment* pMsg = new CSegment;
    pMsg->Put((BYTE*)(pReq),sizeof(mcReqRoleTokenMessage));
    m_pCsInterface->SendMsgToCS(H323_CS_SIG_ROLE_TOKEN_REQ,pMsg,m_serviceId,
                m_serviceId,m_pDestUnitId,m_callIndex,pChannel->GetCsIndex(),pChannel->GetIndex(),0);
    POBJDELETE(pMsg);
  }
  else
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyRoleTokenReq: Incoming content channel wasn't found");
    // This code was written for Tandberg EPs in order to allow token response in case the EP sent token
    // request and waiting for the tokenInd in order to open it's incoming channel. In this case we will forward the
    // request and respond and the EP will dynamically open the H239 channel toward us.
      pReq->randNumber = randomNum;
    pReq->subOpcode  = eRoleTokenOpcode;
    pReq->bIsAck     = 0;
    switch (eRoleTokenOpcode)
    {
      case kPresentationTokenIndicateOwner:
        break;

      case kPresentationTokenResponse:
        pReq->bIsAck    = 1;
        break;
        *** only in H239 */
/*      case kFlowControlReleaseResponse:
        break;
            case kPresentationTokenRequest:
            {
                randomNum = 0;
                SendVidFlowCntl = 1;
                break;
            }

      default:
        PDELETE(pReq);
        return;
    }
    if((mcuNum == 0) && (terminalNum == 0))
    {
      PTRACE(eLevelError,"CH323Cntl::OnPartyRoleTokenReq: MCU and terminal number ZERO (Else)");
      //DBGPASSERT(pChannel->index);
    }
        if (IsSlaveCascadeModeForH239())
        {
            //Here we replace the internal numbers with the numbers we received from the master
            //Because the master is not aware of our internam MT numbers.
            pReq->mcuID         = m_mcuNumFromMaster;
            pReq->terminalID      = m_terminalNumFromMaster;
        }
        else
        {
            pReq->mcuID         = mcuNum;
            pReq->terminalID      = terminalNum;
        }
    pReq->contentProviderInfo = 0;//for now always zero (video)
    pReq->label               = LABEL_CONTENT;
    pReq->bitRate             = 0;
    CSegment* pMsg = new CSegment;
    pMsg->Put((BYTE*)(pReq),sizeof(mcReqRoleTokenMessage));
    m_pCsInterface->SendMsgToCS(H323_CS_SIG_ROLE_TOKEN_REQ,pMsg,m_serviceId,
                m_serviceId,m_pDestUnitId,m_callIndex,0,0, 0);
    POBJDELETE(pMsg);
  }
  if (SendVidFlowCntl && !m_bIsContentRejected && !( m_pParty->IsCallGeneratorParty() ) )
  {
    // Means there is a speaker switch - We need to send video flow cntl to the old speaker with the full rate.
    SendFlowCntlInCaseStopingBeingTheSpeaker();
  }
  PDELETE(pReq);
}
*/
////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnPartyRoleTokenReq(DWORD eRoleTokenOpcode,BYTE mcuNum,BYTE terminalNum,BYTE randomNum)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyRoleTokenReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  mcReqRoleTokenMessage *pReq = new mcReqRoleTokenMessage;

  WORD SendVidFlowCntl = 0;

  //********** 1. build request
  if((mcuNum == 0) && (terminalNum == 0))
    PTRACE(eLevelError,"CH323Cntl::OnPartyRoleTokenReq: MCU and terminal number ZERO (Else)");

  if (IsSlaveCascadeModeForH239())
  {
    //Here we replace the internal numbers with the numbers we received from the master
    //Because the master is not aware of our internam MT numbers.
    pReq->mcuID         = m_mcuNumFromMaster;
    pReq->terminalID      = m_terminalNumFromMaster;
  }
  else
  {
      pReq->mcuID         = mcuNum;
      pReq->terminalID      = terminalNum;
  }
  pReq->randNumber = randomNum;
  pReq->contentProviderInfo = 0;//for now always zero (video)
  pReq->label               = LABEL_CONTENT;
  pReq->bitRate             = 0;
  pReq->subOpcode       = kUnknownRoleTokenOpcode;


  //********** 2. Translate opcode from API opcode to H239 or EPC opcode

  CChannel* pChannel = FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation);
  CChannel* pChannelin = FindChannelInList(cmCapVideo, FALSE, kRoleContentOrPresentation);
  CSegment* pMsg = new CSegment;
  BYTE isUseInChannel = FALSE;

  if ((pChannel) && (pChannel->GetChannelCloseInitiator() == NoInitiator))
  {
    if ( pChannel->GetRoleLabel() == kRolePresentation)
      TranslatePPCOpcodeToH239Opcode(eRoleTokenOpcode,&pReq);
    else
      TranslatePPCOpcodeToEPCOpcode(eRoleTokenOpcode,&pReq);

    //Send msg to CS
    if(pReq->subOpcode != (APIU32)kUnknownRoleTokenOpcode)
    {
      if(pReq->subOpcode == kPresentationTokenResponse && pChannelin && (pChannelin->GetChannelCloseInitiator() == NoInitiator)  )
      {
        //PTRACE(eLevelInfoNormal," CH323Cntl::OnPartyRoleTokenReq - USING INCOMING");
        isUseInChannel = TRUE;
      }
      if( pReq->subOpcode ==(APIU32) kPresentationTokenRequest)
      {
             if (IsSlaveCascadeModeForH239())
                    m_lastContentRateFromMasterForThisToken = 0;
        randomNum = 0;
      }
      pMsg->Put((BYTE*)(pReq),sizeof(mcReqRoleTokenMessage));
      if(!isUseInChannel)
      {
        m_pCsInterface->SendMsgToCS(H323_CS_SIG_ROLE_TOKEN_REQ,pMsg,m_serviceId,
          m_serviceId,m_pDestUnitId,m_callIndex,pChannel->GetCsIndex(),pChannel->GetIndex(),0);
      }
      else
      {
        PTRACE2INT(eLevelInfoNormal," CH323Cntl::OnPartyRoleTokenReq - sending on in channel",pChannelin->GetIndex());
        m_pCsInterface->SendMsgToCS(H323_CS_SIG_ROLE_TOKEN_REQ,pMsg,m_serviceId,
                  m_serviceId,m_pDestUnitId,m_callIndex,pChannelin->GetCsIndex(),pChannelin->GetIndex(),0);
      }
    }

  }
  else //If channel not open - in the middle of change protocol
  {
    if(m_pLocalCapH323->IsH239())
      TranslatePPCOpcodeToH239Opcode(eRoleTokenOpcode,&pReq);
    else if(m_pLocalCapH323->IsEPC())
        TranslatePPCOpcodeToEPCOpcode(eRoleTokenOpcode,&pReq);

    //Send msg to CS
    if(pReq->subOpcode != (APIU32)kUnknownRoleTokenOpcode)
    {
      pMsg->Put((BYTE*)(pReq),sizeof(mcReqRoleTokenMessage));

      m_pCsInterface->SendMsgToCS(H323_CS_SIG_ROLE_TOKEN_REQ,pMsg,m_serviceId,
          m_serviceId,m_pDestUnitId,m_callIndex,0,0, 0);
    }
  }

  if(pReq->subOpcode == kPresentationTokenRequest && !m_bIsContentRejected)
  {
    // Means there is a speaker switch - We need to send video flow cntl to the old speaker with the full rate.
    SendFlowCntlInCaseStopingBeingTheSpeaker();
  }

  POBJDELETE(pMsg);
  PDELETE(pReq);

}

////////////////////////////////////////////////////////////////////////////
BYTE  CH323Cntl::isContentRequest (ERoleTokenOpcode eRoleTokenOpcode) const
{
  //return TRUE if it's a request or FALSE if it's a response or ACK
  switch (eRoleTokenOpcode)
      {
        case kFlowControlReleaseRequest:
        case kPresentationTokenRequest:
        case kPresentationTokenIndicateOwner:
        case kPresentationTokenRelease:
          return TRUE;
        case kFlowControlReleaseResponse:
        case kPresentationTokenResponse:

          return FALSE;

		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
      }
  return FALSE;
}
////////////////////////////////////////////////////////////////////////////

void CH323Cntl::SendFlowCntlInCaseStopingBeingTheSpeaker()
{
//  DWORD newRate = 0;
  PTRACE(eLevelInfoNormal,"CH323Cntl::SendFlowCntlInCaseStopingBeingTheSpeaker: Sending flow cntl to old H239 speaker ");
//  SetNewVideoRateInH239Call(newRate);
  DWORD curPeopleRate = 0;
  if (m_isLprModeOn)
    curPeopleRate = m_pCurrentModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit);
  else
    curPeopleRate      = m_pParty->GetVideoRate();

  BYTE temp = SendFlowControlReq(cmCapVideo, FALSE, kRolePeople, curPeopleRate);
  m_bIsContentSpeaker = FALSE;
  m_isH239FlowCntlSent = 0;
}
////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323RoleTokenInd(CSegment *pParam)
{
  mcIndRoleToken *pRoleToken = new mcIndRoleToken;
  APIS8 arrayIndex;
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;
  DWORD PPCOpocde = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;
  memset(pRoleToken, 0, sizeof(mcIndRoleToken));
  pParam->Get((BYTE*)pRoleToken, sizeof(mcIndRoleToken));

  PTRACE2(eLevelInfoNormal, "CH323Cntl::OnH323RoleTokenInd: Role token request: ", GetRoleTokenOpcodeStr((ERoleTokenOpcode)pRoleToken->subOpcode));

  CChannel* pChannel = NULL;
  pChannel = FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation);

  if(pChannel)
  {
    if (pChannel->GetRoleLabel() == kRolePresentation)
      PPCOpocde = TranslateH239OpcodeToPPCOpcode(pRoleToken);

    else if (pChannel->GetRoleLabel() == kRoleContent)
      PPCOpocde = TranslateEPCOpcodeToPPCOpcode(pRoleToken);

    HandleContentOpcodes(PPCOpocde,pRoleToken);
  }
  else
	  TRACEINTO << " no channel found for presentation!!!";

  PDELETE(pRoleToken);
}
/////////////////////////////////////////////////////////////////////////////
void CH323Cntl::HandleContentOpcodes(DWORD Opcode ,mcIndRoleToken *pRoleToken)
{

//  DWORD Opcode = pRoleToken->subOpcode;
  BYTE  terminalID = pRoleToken->terminalID;
  BYTE  mcuID      = pRoleToken->mcuID;
  TRACEINTO << "partyId " << m_pParty->GetPartyRsrcID() << ", opcode " << Opcode << " is callgenerator " <<  (int)m_pParty->IsCallGeneratorParty();
  if( m_pParty->IsCallGeneratorParty() )
  {
      m_pTaskApi->SendTokenMessageToCallGenerator(pRoleToken->subOpcode,(BYTE)pRoleToken->bIsAck);
      PDELETE(pRoleToken);
      return;
  }
    if (IsSlaveCascadeModeForH239())
    {
        terminalID = m_pParty->GetMcuNum();
        mcuID      = m_pParty->GetTerminalNum();
    }

  BYTE  randomNum  = pRoleToken->randNumber;

  switch (Opcode)
  {
    case FLOW_CONTROL_RELEASE_REQ:
    {
      OnPartyRoleTokenReq(FLOW_CONTROL_RELEASE_NAK,mcuID,terminalID);
      break;
    }
    case PARTY_TOKEN_ACQUIRE:
    {
      // check if the rx channel is symmetric to the tx one - allow it.
      // if not - reject the request.
      CChannel* pChannelTx = FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation);
      CChannel* pChannelRx = FindChannelInList(cmCapVideo, FALSE, kRoleContentOrPresentation);
      if (pChannelTx != NULL && pChannelRx != NULL)
      {
        if (pChannelTx->GetCapNameEnum() != pChannelRx->GetCapNameEnum())
        {
          TRACEINTO << "Channels Different in type"
                << " RX " << pChannelRx->GetCapNameEnum()
                << " TX " << pChannelTx->GetCapNameEnum();

          OnPartyRoleTokenReq(PARTY_TOKEN_ACQUIRE_NAK,mcuID,terminalID);
        }
      }
//TODO Ofir: check if merge ok
      if(m_state == DISCONNECT)
        OnPartyRoleTokenReq(PARTY_TOKEN_ACQUIRE_NAK,mcuID,terminalID);
      else if(randomNum == 0 && !IsSlaveCascadeModeForH239() && pChannelTx && (pChannelTx->GetRoleLabel() == kRolePresentation && (m_remoteIdent != PolycomRMX) && (m_remoteIdent != PolycomMGC)))
      {
        m_pTaskApi->SendTokenMessageToConfLevel(PARTY_TOKEN_ACQUIRE,mcuID,terminalID,LABEL_CONTENT,randomNum);
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323RoleTokenInd: Symetry Breaking is Zero but we are not slave -  do not ignore!");
      }

      else
      {
    	  m_pTaskApi->SendTokenMessageToConfLevel(PARTY_TOKEN_ACQUIRE,mcuID,terminalID,LABEL_CONTENT,randomNum);
      }

      break;
    }
    case PARTY_TOKEN_WITHDRAW:
    {
            m_pTaskApi->SendTokenMessageToConfLevel(PARTY_TOKEN_WITHDRAW, mcuID,terminalID,LABEL_CONTENT,randomNum);
      break;
    }
    case ROLE_PROVIDER_IDENTITY:
    {
        CSegment* pConfParam = new CSegment;
        int size = pConfParam->GetLen();
        m_pTaskApi->SendTokenMessageToConfLevel(ROLE_PROVIDER_IDENTITY,mcuID,terminalID,LABEL_CONTENT,0,size,pConfParam);
        POBJDELETE (pConfParam);
                if (IsSlaveCascadeModeForH239() && !m_bIsContentSpeaker)
                {
                    if (m_remoteIdent == PolycomMGC)
                        m_LastContentRateFromMaster = m_pLocalCapH323->GetMaxContentBitRate();
                    if (m_LastContentRateFromMaster)
                    {
             PTRACE2INT (eLevelInfoNormal, "CH323Cntl::HandleContentOpcodes kPresentationTokenIndicateOwner - informing content bridge", m_LastContentRateFromMaster);
                        //We already know the content rate - notify the bridge
                        m_pTaskApi->NotifyConfOnMasterActionsRegardingContent(MASTER_START_CONTENT, m_LastContentRateFromMaster);
                    }

                    else if (m_remoteIdent != PolycomMGC)
                    {
                        PTRACE (eLevelInfoNormal, "CH323Cntl::OnH323RoleTokenInd kPresentationTokenIndicateOwner - waiting for rate indication from master");
                        //wait for flow control indication
                        m_bWaitForFlowCntlIndIndOnContent = TRUE;
                    }

                }
      break;
    }
    case PARTY_TOKEN_RELEASE:
    {
        m_pTaskApi->SendTokenMessageToConfLevel(PARTY_TOKEN_RELEASE,mcuID,terminalID,LABEL_CONTENT,randomNum);
        break;
    }
    case PARTY_TOKEN_ACQUIRE_ACK:
    case PARTY_TOKEN_ACQUIRE_NAK:
    {
      m_pTaskApi->SendTokenMessageToConfLevel(Opcode,mcuID,terminalID,LABEL_CONTENT);
      break;
    }
    case PARTY_TOKEN_WITHDRAW_ACK:
                {
                    m_pTaskApi->SendTokenMessageToConfLevel(PARTY_TOKEN_WITHDRAW_ACK,mcuID,terminalID,LABEL_CONTENT);
      break;
    }

    default:
    {
      PTRACE2INT(eLevelError,"CH323Cntl::HandleContentOpcodes : Unknown Sub opcode'",Opcode);
      break;
    }
  }

}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::RemoveGenericCapAndUpdateConfLevel()
{
  m_bIsRemoveGeneric = TRUE;
  WORD numCapRemoved = 0;

  numCapRemoved = m_pLocalCapH323->RemoveProtocolFromCapSet(_Siren14);
  if(numCapRemoved)
    m_pTaskApi->SendRemovedProtocolToConfLevel(1, _Siren14);

  numCapRemoved = m_pLocalCapH323->RemoveProtocolFromCapSet(_Siren7);
  if(numCapRemoved)
    m_pTaskApi->SendRemovedProtocolToConfLevel(1, _Siren7);

  numCapRemoved = m_pLocalCapH323->RemoveProtocolFromCapSet(_G7221);
  if(numCapRemoved)
    m_pTaskApi->SendRemovedProtocolToConfLevel(1, _G7221);

  numCapRemoved = m_pLocalCapH323->RemoveProtocolFromCapSet(_H264);
  if(numCapRemoved)
    m_pTaskApi->SendRemovedProtocolToConfLevel(1, _H264);

  numCapRemoved = m_pLocalCapH323->RemoveProtocolFromCapSet(_AnnexQ);
  if(numCapRemoved)
    m_pTaskApi->SendRemovedProtocolToConfLevel(numCapRemoved, (WORD)_AnnexQ);

  numCapRemoved = m_pLocalCapH323->RemoveProtocolFromCapSet(eDBC2CapCode);
  if(numCapRemoved)
    m_pTaskApi->SendRemovedProtocolToConfLevel(numCapRemoved, (WORD)eDBC2CapCode, TRUE);
}

////////////////////////////////////////////////////////////////////////////
// Store remote vendor identification received from Avaya in local member
void CH323Cntl::StoreRemoteVendorInfo(h460AvayaFeVndrIndSt& avfFeVndIdInd)
{
  PTRACE(eLevelInfoNormal,"CH323Cntl::StoreRemoteVendorInfo");
  // put the new information into m_remoteVendor
  m_remoteVendor.countryCode    = avfFeVndIdInd.countryCode;
  m_remoteVendor.t35Extension   = avfFeVndIdInd.t35Extension;
  m_remoteVendor.manufactorCode   = avfFeVndIdInd.manfctrCode;
  m_remoteVendor.isAvayaSipCm = (avfFeVndIdInd.bSipCM == AVAYA_SIP_CM_FLAG_ON) ? AVAYA_SIP_CM_FLAG_ON : AVAYA_SIP_CM_FLAG_OFF;

  if (avfFeVndIdInd.productId)
  {
    if ((m_remoteVendor.productId != NULL) && (*m_remoteVendor.productId != '\0'))
    {
      PDELETEA(m_remoteVendor.productId);
      m_remoteVendor.productId      = new char[H460_C_ProdIdMaxSize];
      memset(m_remoteVendor.productId, '\0', H460_C_ProdIdMaxSize);
      strncpy(m_remoteVendor.productId, avfFeVndIdInd.productId, H460_C_ProdIdMaxSize - 1);
      m_remoteVendor.productId[H460_C_ProdIdMaxSize - 1] = '\0';
    }
  }

  if (avfFeVndIdInd.versionId)
  {
    if ((m_remoteVendor.versionId != NULL) && (*m_remoteVendor.versionId != '\0'))
    {
      PDELETEA(m_remoteVendor.versionId);
      m_remoteVendor.versionId      = new char[H460_C_VerIdMaxSize];
      memset(m_remoteVendor.versionId, '\0', H460_C_VerIdMaxSize);
      strncpy(m_remoteVendor.versionId, avfFeVndIdInd.versionId, H460_C_VerIdMaxSize);
      m_remoteVendor.versionId[H460_C_VerIdMaxSize - 1] = '\0';
    }
  }

  if ((m_remoteVendor.productId != NULL) && !strncmp(m_remoteVendor.productId, "ACCORD MGC", strlen("ACCORD MGC")) )
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::StoreRemoteVendorInfo - POLYCOM MGC");
    m_remoteIdent = PolycomMGC;
  }
  else if ( ((m_remoteVendor.countryCode == Israel_t35CountryCode) && (m_remoteVendor.t35Extension == Israel_t35Extension) &&
           (m_remoteVendor.manufactorCode == Accord_manufacturerCode)) )

  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::StoreRemoteVendorInfo - POLYCOM RMX");
    m_remoteIdent = PolycomRMX;
  }
}

////////////////////////////////////////////////////////////////////////////
// Duplicate remote vendor identification into supplied variable
void CH323Cntl::DuplicateRemoteVendorInfo(RemoteVendorSt& stRemoteVendor)
{
  stRemoteVendor.countryCode    = m_remoteVendor.countryCode;
  stRemoteVendor.t35Extension   = m_remoteVendor.t35Extension;
  stRemoteVendor.manufactorCode = m_remoteVendor.manufactorCode;
  stRemoteVendor.isAvayaSipCm   = m_remoteVendor.isAvayaSipCm;
  stRemoteVendor.isCopMcu       = m_remoteVendor.isCopMcu;

  if ((m_remoteVendor.productId != NULL) && (*m_remoteVendor.productId != '\0'))
    strncpy(stRemoteVendor.productId, m_remoteVendor.productId, H460_C_ProdIdMaxSize);
  if ((m_remoteVendor.versionId != NULL) && (*m_remoteVendor.versionId != '\0'))
    strncpy(stRemoteVendor.versionId, m_remoteVendor.versionId, H460_C_VerIdMaxSize);
}

void CH323Cntl::DumpRemoteVendorSt(RemoteVendorSt& stRemoteVendor) // temp
{
  CSmallString Msg;
  Msg << "stRemoteVendor:\n--------------\ncountryCode: " << stRemoteVendor.countryCode;
  Msg << "\nt35Extension: " << stRemoteVendor.t35Extension;
  Msg << "\nmanufactorCode: " << stRemoteVendor.manufactorCode;
  Msg << "\nproductId:  " << stRemoteVendor.productId;
  Msg << "\nversionId:  " << stRemoteVendor.versionId;
  Msg << "\nisAvayaSipCm: " << stRemoteVendor.isAvayaSipCm;
  Msg << "\nisRemoteCOPmcu: " << stRemoteVendor.isCopMcu;
  PTRACE(eLevelInfoNormal, Msg.GetString());
}

////////////////////////////////////////////////////////////////////////////
// Store remote vendor information received in CALL_CONNECTED_IND and
//duplicate them into supplied variable
void CH323Cntl::FillRemoteVendorInfoAvaya(RemoteVendorSt& stRemoteVendor, mcIndCallConnected* pCallConnectedInd)
{
  PTRACE(eLevelInfoNormal,"CH323Cntl::FillRemoteVendorInfoAvaya");
  if (m_pmcCall->GetIsOrigin())// dial out
  {
    if (pCallConnectedInd->avfFeVndIdInd.fsId != H460_K_FsId_Avaya)
    {
      PTRACE2INT(eLevelError, "CH323Cntl::FillRemoteVendorInfoAvaya - Fs Id isn't Avaya. Connection Id = ", m_pmcCall->GetConnectionId());
      DBGPASSERT(pCallConnectedInd->avfFeVndIdInd.fsId);
    }
    StoreRemoteVendorInfo(pCallConnectedInd->avfFeVndIdInd);
  }
  // in case of dial out the info already in m_remoteVendor but in case of dial in it was there before
  DuplicateRemoteVendorInfo(stRemoteVendor);
}

////////////////////////////////////////////////////////////////////////////
// Check remote vendor information and update local variable if needed. Also
// supplies remote vendor info above.
void CH323Cntl::CheckAndUpdateRemoteVendor(RemoteVendorSt& stRemoteVendor, mcIndCallConnected* pCallConnectedInd)
{
  //AVAYA:
  if (m_bIsAvaya )
  {
    FillRemoteVendorInfoAvaya(stRemoteVendor, pCallConnectedInd);
    DumpRemoteVendorSt(stRemoteVendor);
    return;
  }
  // fill the structure for non Avaya case
  stRemoteVendor.countryCode    = pCallConnectedInd->remoteVendor.info.t35CountryCode;
  stRemoteVendor.t35Extension   = pCallConnectedInd->remoteVendor.info.t35Extension;
  stRemoteVendor.manufactorCode = pCallConnectedInd->remoteVendor.info.manufacturerCode;
  stRemoteVendor.isAvayaSipCm   = AVAYA_SIP_CM_FLAG_OFF;
  if ((pCallConnectedInd->remoteVendor.productID)[0] != '\0')
  {
    strncpy(stRemoteVendor.productId, pCallConnectedInd->remoteVendor.productID, H460_C_ProdIdMaxSize - 1);
    stRemoteVendor.productId[H460_C_ProdIdMaxSize - 1] = '\0';
  }
  if ((pCallConnectedInd->remoteVendor.versionID)[0] != '\0')
  {
    strncpy(stRemoteVendor.versionId, pCallConnectedInd->remoteVendor.versionID, H460_C_VerIdMaxSize - 1);
    stRemoteVendor.versionId[H460_C_VerIdMaxSize - 1] = '\0';
  }
}

////////////////////////////////////////////////////////////////////////////
// Change local capabilities according to remote vendor information
void  CH323Cntl::ChangeCapabilitySetAccordingToVendor(RemoteVendorSt& stRemoteVendor, BYTE& isCascadeToPreviousVer)
{
  BOOL   bAnyEpFlag     = TRUE;
  BOOL   bIdentify      = FALSE;          //for calls via Proxy - we identify it by the endpoint
  WORD   bIsH239      = m_pLocalCapH323->IsH239();
  WORD   bIsEPC       = m_pLocalCapH323->IsSupportPeopleAndContent();
  WORD   numCapRemoved  = 0;

  // remove protocols
  BYTE isRemoveGenericAudioCaps   = FALSE;
  BYTE isRemoveAnnexQ       = FALSE;
  BYTE isRemoveRvFecc       = FALSE;
  BYTE isRemoveH239       = FALSE;
  BYTE isRemoveGenericVideoCap  = FALSE;
  BYTE isRemoveG722       = FALSE;
  BYTE isRemoveH264       = FALSE;
  BYTE isRemoveOtherThenQCif    = FALSE;
  BYTE isRemoveDBC2       = FALSE;
  BYTE isRemoveG7221C       = FALSE;
  BYTE isRemoteMgcWithLowRateConf = FALSE;
  BYTE isRemoveLpr        = FALSE;
    BYTE isCascadeOrRecordingLink   = FALSE;
    BYTE isFixContentProtocol    = FALSE;
    BYTE isPolycomVoIpEp      = FALSE;
    BYTE isRemoveEPC        = FALSE;
    BYTE isRemoveG7231              = FALSE;
    BYTE isRemoveG719               = FALSE;
    BYTE isRemoveDPTR               = FALSE;
    BYTE isNeedToAddTranmitCaps     = FALSE;
    BYTE isRemoveHP                 = FALSE;

        BOOL bRemoveLPR = GetSystemCfgFlagInt<BOOL>("REMOVE_H323_LPR_CAP_TO_NON_POLYCOM_VENDOR");
        if(bRemoveLPR && stRemoteVendor.manufactorCode != Polycom_manufacturerCode && stRemoteVendor.manufactorCode != Accord_manufacturerCode )
        {
        	m_pTargetModeH323->SetIsLpr(FALSE);
        	isRemoveLpr = TRUE;
        }
        BOOL bRemoveextendedAudioCaps = GetSystemCfgFlagInt<BOOL>("REMOVE_H323_HIGH_QUALITY_AUDIO_CAP_TO_NON_POLYCOM_VENDOR");
        if(bRemoveextendedAudioCaps && stRemoteVendor.manufactorCode != Polycom_manufacturerCode && stRemoteVendor.manufactorCode != Accord_manufacturerCode )
        {
        	isRemoveGenericAudioCaps = TRUE;
        	isRemoveG7231 = TRUE;
        }
        BOOL bRemoveEPC = GetSystemCfgFlagInt<BOOL>("REMOVE_H323_EPC_CAP_TO_NON_POLYCOM_VENDOR");
        if(bRemoveEPC && stRemoteVendor.manufactorCode != Polycom_manufacturerCode && stRemoteVendor.manufactorCode != Accord_manufacturerCode )
        	isRemoveEPC = TRUE;

        BOOL bRemoveHighProfile = GetSystemCfgFlagInt<BOOL>("REMOVE_H323_HIGH_PROFILE_CAP_TO_NON_POLYCOM_VENDOR");
        if(bRemoveHighProfile && stRemoteVendor.manufactorCode != Polycom_manufacturerCode && stRemoteVendor.manufactorCode != Accord_manufacturerCode )
        {

        	if( m_pTargetModeH323->GetH264Profile(cmCapReceive) == H264_Profile_High)
        	{
        		m_pTargetModeH323->SetH264Profile(H264_Profile_BaseLine,cmCapReceive);
        		m_pTargetModeH323->SetH264Profile(H264_Profile_BaseLine,cmCapTransmit);
        		isRemoveHP = TRUE;
        	}
        }

  if (!m_bIsAvaya)
  {
    if ((m_remoteVendor.productId != NULL) && (*m_remoteVendor.productId != '\0'))
    {
      PDELETEA(m_remoteVendor.productId);
      m_remoteVendor.productId      = new char[H460_C_ProdIdMaxSize];
      memset(m_remoteVendor.productId, '\0', H460_C_ProdIdMaxSize);
    }
    strncpy(m_remoteVendor.productId, stRemoteVendor.productId, H460_C_ProdIdMaxSize);

    if ((m_remoteVendor.versionId != NULL) && (*m_remoteVendor.versionId != '\0'))
    {
      PDELETEA(m_remoteVendor.versionId);
      m_remoteVendor.versionId = new char[H460_C_VerIdMaxSize];
            memset(m_remoteVendor.versionId, '\0', H460_C_VerIdMaxSize);
    }
    strncpy(m_remoteVendor.versionId, stRemoteVendor.versionId,  H460_C_VerIdMaxSize);

  }
  // PictureTel (Supports generic audio caps) & Polycom Sabre EP
  if ((stRemoteVendor.countryCode == US_t35CountryCode) && (stRemoteVendor.t35Extension == US_t35Extension) &&
      ((stRemoteVendor.manufactorCode == PictureTel_manufacturerCode) ||
       (stRemoteVendor.manufactorCode == Polycom_manufacturerCode && m_remoteInfo.h225RemoteVersion >= 4)))
  {
        PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - PictureTel EP or other Polycom EPC EP");
    bAnyEpFlag = FALSE;             // turn the flag to false for skipping the any EP code
    bIdentify  = TRUE;
    if( strstr(stRemoteVendor.productId, "VVX") )
    {
      PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - vvx");
      m_remoteIdent = PolycomVVX;
    }
    }
  if(!strncmp(stRemoteVendor.productId,"iPower 9000",strlen("iPower 9000")))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - ipower ep remain with audio codecs of G711,G722,G729");
    isRemoveGenericAudioCaps = TRUE;
    isRemoveG7231 = TRUE;

  }
  if(strstr(stRemoteVendor.productId, "SONY PCS-101") )
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - sony");
    isRemoveEPC = TRUE;
    m_pLocalCapH323->SetH263FormatMpi(k4Cif, -1, kRolePeople);
    m_remoteIdent = SonyEp;
    DWORD contentRate = m_pTargetModeH323->GetContentBitRate(cmCapReceive);
      m_pTargetModeH323->SetContent (contentRate, cmCapReceiveAndTransmit, eH263CapCode);

  }
  if( strstr(stRemoteVendor.productId, "CnS CIP4500") )
	{
		isRemoveG7231 = TRUE;
		PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - CnS CIP4500 remove G7231");//Problem in Audio on ChinaUnicom - got it from Noa R.
	}if( strstr(stRemoteVendor.productId, "Aethra") )
	{
		isRemoveRvFecc = TRUE;
		isRemoveEPC =TRUE;

	}

  // VSX 8.5 & 8.5.1 problem - Need to remove the G7221C from our capability set (VNGR-4111)
  if((stRemoteVendor.countryCode    == US_t35CountryCode) &&
     (stRemoteVendor.t35Extension   == US_t35Extension)   &&  (stRemoteVendor.manufactorCode   == Polycom_manufacturerCode)
     && !strncmp(stRemoteVendor.productId,"VSX",strlen("VSX")))
  {
    if ( (!strncmp(stRemoteVendor.versionId,"Release 8.5.1 -",strlen("Release 8.5.1 -"))) ||
      (!strncmp(stRemoteVendor.versionId,"Release 8.5 -",strlen("Release 8.5 -"))))
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - VSX V8.5.1 Or V8.5");
      isRemoveG7221C = TRUE;
    }
    //isRemoveEPC = TRUE;
    //VNGR-21808
    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    std::string key = "MINIMUM_FRAME_RATE_TRESHOLD_FOR_SD";
    DWORD minFrameRateForSD;
    sysConfig->GetDWORDDataByKey(key, minFrameRateForSD);
    BYTE isReduceToCif = FALSE;
    if(!strncmp(stRemoteVendor.productId,"VSX 8000",strlen("VSX 8000")) && minFrameRateForSD > 12 && !strncmp(stRemoteVendor.versionId,"Release 9.0.5",strlen("Release 9.0.5")) )
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - VSX 8000 Release 9.0.5");
      isReduceToCif = TRUE;
    }
    if( (isReduceToCif || !strncmp(stRemoteVendor.productId,"VSX 7000",strlen("VSX 7000")) || !strncmp(stRemoteVendor.productId,"VSX 6000",strlen("VSX 6000")) ||
        !strncmp(stRemoteVendor.productId,"VSX 5000",strlen("VSX 5000")) ) &&
      m_pTargetModeH323->GetVideoPartyType(cmCapReceiveAndTransmit) > eCP_H264_upto_CIF_video_party_type )
    {
      APIS8 cif4 = m_pLocalCapH323->Get4CifMpi();
      SetH264ModeInLocalCapsAndScmAccordingToVideoPartyType(eCP_H264_upto_CIF_video_party_type,cif4);
      m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
    }

  }
    else if((stRemoteVendor.countryCode   == US_t35CountryCode) &&
            (stRemoteVendor.t35Extension    == US_t35Extension)   &&
            (stRemoteVendor.manufactorCode   == Polycom_manufacturerCode) &&
            !strncmp(stRemoteVendor.productId,QDXProductId,strlen(QDXProductId)))
    {
        //QDX
        PTRACE (eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - Polycom QDX");
        m_remoteIdent = PolycomQDX;
  }
    // Polycom MCU (Supports generic audio caps)
    // we check this in productID that equals to "ACCORD MGC" and not "AccordMGC" (like in version 2)
  else if(!strncmp(stRemoteVendor.productId, "ACCORD MGC", strlen("ACCORD MGC")))
  {
    m_remoteIdent = PolycomMGC;
    PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - MGC ");
       if(m_pH323NetSetup->GetMaxRate() < rate320K)
       {
         PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - MGC with low conf rate");
         isRemoteMgcWithLowRateConf = TRUE;
       }
       isCascadeOrRecordingLink = TRUE;
       isRemoveG719 = YES;
  }
  else if (((stRemoteVendor.countryCode == Israel_t35CountryCode) && (stRemoteVendor.t35Extension == Israel_t35Extension) &&
           (stRemoteVendor.manufactorCode == Accord_manufacturerCode)) ||
             (!strncmp(stRemoteVendor.productId, "RMX_C_1500", strlen("RMX_C_1500"))) ||
             (!strncmp(stRemoteVendor.productId, "RMX_C_2000", strlen("RMX_C_2000"))) ||
             (!strncmp(stRemoteVendor.productId, "RMX_C_4000", strlen("RMX_C_4000"))) ||
        !strncmp(stRemoteVendor.productId, "Polycom RMX 1500", strlen("Polycom RMX 1500")) ||
        !strncmp(stRemoteVendor.productId, "Polycom RMX 2000", strlen("Polycom RMX 2000")) ||
        !strncmp(stRemoteVendor.productId, "Polycom RMX 4000", strlen("Polycom RMX 4000")))
  {
    m_remoteIdent = PolycomRMX;
        isCascadeOrRecordingLink = TRUE;
        isRemoveG719 = TRUE;
    if (!( m_pParty->IsCallGeneratorParty()))
          isFixContentProtocol   = TRUE;  // for Call Generator to support H264 content
  }

    else if ((((stRemoteVendor.countryCode  == China_t35CountryCode)&&(stRemoteVendor.t35Extension  == China_t35Extension)&&(stRemoteVendor.manufactorCode == DST_manufacturerCode))
        ||!strncmp(stRemoteVendor.productId, DstMcsProductId ,strlen(DstMcsProductId)))
        ||((!strncmp(stRemoteVendor.productId, RMX1000ProductId ,strlen(RMX1000ProductId)))
                 &&(strncmp(stRemoteVendor.productId, RMX2000ProductId ,strlen(RMX2000ProductId)))) )
   {
        PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - DstH323Mcs");
    m_remoteIdent = DstH323Mcs;
    bAnyEpFlag = FALSE;
    bIdentify  = TRUE;
        isRemoveH239 = FALSE;
        if ( strstr(PARTYNAME,"LinkToMaster") || ( m_pParty->GetCascadeMode() == SLAVE ) ||
      ((m_pParty->IsCallGeneratorParty()) && ( m_bDisguiseAsEPMode == TRUE )) )
        {
            PTRACE2(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Disguising myself as endpoint  ", PARTYNAME);
            m_bDisguiseAsEPMode = TRUE;
            m_pParty->SetCascadeMode(SLAVE);
        }

    if( !strncmp(stRemoteVendor.productId, RMX1000ProductId ,strlen(RMX1000ProductId)) && m_pTargetModeH323->GetConfType() != kVSW_Fixed && m_pTargetModeH323->GetConfType() != kVideoSwitch)
    {
      m_pParty->UpdateVideoRate(m_pTargetModeH323->GetCallRate()*10);
      m_remoteVendor.isCopMcu = TRUE;
    }

      //Cascade no FECC between them two our MCUs or confs on the same MCU
      isRemoveAnnexQ = TRUE;
      isRemoveRvFecc = TRUE;
      isCascadeOrRecordingLink = TRUE;
      isFixContentProtocol   = TRUE;
      isRemoveG719 = TRUE;
    }
  // Transparent GW - In this case we will add the H239 capabilities
  char* userUserName = NULL;
  if  (m_pmcCall->GetIsOrigin())
      userUserName = m_pmcCall->GetCallTransientUserUser();
  else
    userUserName= m_pH323NetSetup->GetH323userUser();

  if(m_remoteIdent == PolycomMGC || m_remoteIdent == PolycomRMX)
  {
      PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - POLYCOM MCU");
        isRemoveH239 = FALSE;
        if (strstr(PARTYNAME,"LinkToMaster") || (m_pParty->GetCascadeMode() == SLAVE) ||
      ((m_pParty->IsCallGeneratorParty()) && ( m_bDisguiseAsEPMode == TRUE )) )
        {
            PTRACE2(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Disguising myself as endpoint  ", PARTYNAME);
            m_bDisguiseAsEPMode = TRUE;
            m_pParty->SetCascadeMode(SLAVE);
        }
    bAnyEpFlag = FALSE;
    bIdentify  = TRUE;
    //Cascade no FECC between them two our MCUs or confs on the same MCU
    isRemoveAnnexQ = TRUE;
    isRemoveRvFecc = TRUE;
        isCascadeOrRecordingLink = TRUE;
        isRemoveG719 = TRUE;
        if ( strstr(userUserName,"COP") )
        {
          PTRACE2(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - remote mcu is cop  ", PARTYNAME);
          m_remoteVendor.isCopMcu = TRUE;

        }

  }

  if ( !strncmp(userUserName, "TRANSGW", strlen("TRANSGW")))

  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - TRANSPARENT GW");
    m_remoteIdent = Regular; //AccordMCU;
    bAnyEpFlag = FALSE;
    bIdentify  = TRUE;
    isRemoveH239 = FALSE;
    isRemoveAnnexQ = FALSE;
    isRemoveRvFecc = FALSE;
    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
    CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
    pConfParty->SetIsTransparentGw(YES);
//    m_pParty->SetCascadeMode() = MASTER;
  }
  // Polycom NPG
  else if (!strcmp(stRemoteVendor.productId, PolycomNpgProductId))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Polycom NPG");
    m_remoteIdent = PolycomNPG;
    bAnyEpFlag = FALSE;
    bIdentify  = TRUE;
  }
  else if (!strcmp(stRemoteVendor.productId, CiscoCallManagerID))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Cisco Call manager");
    bIdentify  = TRUE;
    m_remoteIdent = CiscoGW;

  }
  // RadVision TestApp or ProLab (doesn't support generic audio caps)
  else if ((!strcmp(stRemoteVendor.productId, RadVisionTestApp) && !strcmp(stRemoteVendor.versionId, RadVisionVersionID)) ||
         (!strcmp(stRemoteVendor.productId, "ProLab") && !strcmp(stRemoteVendor.versionId, "RADVISION")))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - RadVision Test Application or ProLab");
    DWORD isRVTestApp = GetSystemCfgFlagInt<DWORD>(CFG_KEY_IP_RV_TEST_APPLICATION);
    if (isRVTestApp == YES)
      m_remoteIdent = RVTestapplication;
    bAnyEpFlag = FALSE;
    bIdentify  = TRUE;
  }
  // RadVision MCU (will be count as any EP at the end)
  else if (!strncmp(stRemoteVendor.productId, RadVisionMcu, strlen(RadVisionMcu)) || !strcmp(stRemoteVendor.productId, RadVisionViaIpMcu))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - RadVision MCU");
    m_remoteIdent = RvMCU;
    bIdentify  = TRUE;
        isCascadeOrRecordingLink = TRUE;
  }
  //Vcon endpoints:
  else if (!strcmp(stRemoteVendor.productId, VconProductId) && (stRemoteVendor.manufactorCode == VconManufacturerCode))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Vcon Endpoint");
    m_remoteIdent = VconEp;
    bIdentify  = TRUE;
  }
  // PolycomVoIP remove g722 and P+C from capability set
  // (doesn't support generic audio caps, even if the h225 version is 3)
  else if ((stRemoteVendor.countryCode == Test_t35CountryCode) && (stRemoteVendor.t35Extension == Test_t35Extension) &&
         (stRemoteVendor.manufactorCode == PolycomVoIP_manufacturerCode))
  {
    if (!strncmp(stRemoteVendor.productId, PolycomVoIPProduct,strlen(PolycomVoIPProduct)) &&
        !strncmp(stRemoteVendor.versionId, PolycomVoIPVersionID, strlen(PolycomVoIPVersionID)))
        {
      PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Polycom VoIP EP");
      bAnyEpFlag = FALSE;
      bIdentify  = TRUE;
      // remove the P&C and 263+ from all vendors stay only with 263.
      if(bIsEPC)
        isRemoveEPC = TRUE;

      if(bIsH239)
        isRemoveH239 = TRUE;
      //remove the G722 cause if not - the EP does not open audio channel.
      isRemoveG722       = TRUE;
      isRemoveGenericAudioCaps = TRUE;
      isRemoveH264       = TRUE;
      isRemoveAnnexQ       = TRUE;
      isRemoveDBC2       = TRUE;
            isRemoveLpr              = TRUE;
            m_pTargetModeH323->SetIsLpr(FALSE);
      isPolycomVoIpEp      = TRUE;
       }
  }
  //TandbergEp
  else if (!strncmp(stRemoteVendor.productId, TandbergProductId, strlen(TandbergProductId)))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Tandberg Ep");
    m_remoteIdent = TandbergEp;

    //check if need to change local caps for HD resolution - vngr-11380
    SetH264ModeInLocalCapsForTandEP(stRemoteVendor);

    bIdentify  = TRUE;
    // AN - 29.7.10 - VNGR 15919 CQS/RMX 7.0 MPM+/Tandberg MXP is connecting audio only in H323 motion CP conferences
    // When forcing video to H263 Tandberg open video with content capabilities (4cif with custom formats) that RMX doesn't support so we remove EPC
    // that confuse it from RMX capabilities
    isRemoveEPC = TRUE;

    if (!strncmp(stRemoteVendor.versionId, Tandberg6000EVersionID, strlen(Tandberg6000EVersionID)))
    {
      PTRACE2(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVersion - Tandberg 6000E: ", m_remoteVendor.versionId);
    }
    if (!strncmp(stRemoteVendor.versionId, Tandberg1000EVersionID, strlen(Tandberg1000EVersionID)))
    {
      PTRACE2(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVersion - Tandberg 1000: ", m_remoteVendor.versionId);
      isRemoveGenericAudioCaps = TRUE;
      isRemoveG7231 = TRUE;

    }


  }
    else if (strstr(stRemoteVendor.productId, LifeSizeProductId) && m_pTargetModeH323->GetConfType() != kCp)
    {
        PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - LifeSize Ep");
        m_remoteIdent = LifeSizeEp;
        isRemoveLpr = TRUE;
        bIdentify  = TRUE;
    }

    else if (strstr(stRemoteVendor.productId, LifeSizeProductId))
     {
           if((m_pTargetModeH323->GetConfType() != kVideoSwitch) && (m_pTargetModeH323->GetConfType() != kVSW_Fixed))
           {
             DWORD expectedAudioRate = m_pLocalCapH323->GetAudioDesiredRate();
               PTRACE2INT(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - LifeSize Ep - m_videoRate ", expectedAudioRate);
               m_remoteIdent = LifeSizeEp;
              //Life Size support only 48K Audio and the video rate must be derived from this.


              if (expectedAudioRate > 48)
             {
                 DWORD videoRate = m_pParty->GetVideoRate();
               PTRACE2INT(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - LifeSize Ep - videoRate ", videoRate);
                 videoRate += 160; //add 16K to video
                 m_pParty->UpdateVideoRate (videoRate);
               m_pTargetModeH323->SetVideoBitRate(videoRate,cmCapReceiveAndTransmit);
              }
             isRemoveLpr = TRUE;
              bIdentify  = TRUE;
       }
     }

    else if( strstr(stRemoteVendor.productId, "Huawei") )
      {
       	PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Huawei ");
       	isRemoveEPC = TRUE;
       	isRemoveGenericAudioCaps = TRUE;
       	CapEnum algorithm = (CapEnum)(m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit));
       	if(algorithm  == eH264CapCode)
       	{
       		PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Huawei h264");
       		APIU16 profile = 0;
       		APIU8 level=0;
       		long Targetmbps=0, Targetfs=0, dpb=0, brAndCpb=0, sar=0, staticMB=0;
       		m_pTargetModeH323->GetH264Scm(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
       		if (Targetfs == -1)
       	    {
       			    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Huawei Targetfs == -1");
       		        CH264Details thisH264Details = level;
       		        Targetfs = thisH264Details.GetDefaultFsAsDevision();
       	    }
       		m_pTargetModeH323->SetH264Profile(H264_Profile_BaseLine,cmCapReceive);
       		m_pTargetModeH323->SetH264Profile(H264_Profile_BaseLine,cmCapTransmit);
       		if(Targetfs == H264_HD720_FS_AS_DEVISION)
       		{
       			PTRACE2INT(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Targetfs ",Targetfs);
       			level = H264_Level_2_2;
       			Targetfs = H264_HD720_FS_AS_DEVISION;
       			Targetmbps = H264_L3_1_DEFAULT_MBPS /CUSTOM_MAX_MBPS_FACTOR;
       			m_pTargetModeH323->SetH264Scm(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
       			m_pTargetModeH323->SetH264Scm(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);
       			m_pTargetModeH323->SetH264Profile(H264_Profile_BaseLine,cmCapReceive);
       			m_pTargetModeH323->SetH264Profile(H264_Profile_BaseLine,cmCapTransmit);
       		}
       	}
       }
    else if ( strstr(stRemoteVendor.productId, "Codian") )
    {
    	PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor -Codian");
    	isRemoveEPC = TRUE;
    }
    //Avistar GW
  else if((stRemoteVendor.manufactorCode == Avistar_manufacturerCode) &&(m_remoteInfo.remoteEndpointType == cmEndpointTypeGateway)&& (strstr(stRemoteVendor.productId,AvistarProductId)))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Avistar GW");
    m_remoteIdent = AvistarGW;
    bIdentify  = TRUE;
  }
  //VIU
  else if (!strncmp(stRemoteVendor.productId, RadVisionViuProductId, strlen(RadVisionViuProductId)))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - VIU");
    m_remoteIdent = VIU;
    bIdentify  = TRUE;
  }
  //Ericsson VIG identifyier when calls arrives from 3G
  else if ((stRemoteVendor.countryCode == Sweden_t35CountryCode) && (stRemoteVendor.manufactorCode == Ericsson_manufacturerCode))
  {
    if (!strncmp(stRemoteVendor.productId, EricssonVIGProductId, strlen(EricssonVIGProductId)))
    {
      m_remoteIdent = EricssonVIG;
      bIdentify   = TRUE;
    }
    PTRACE2INT(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Ericsson VIG? ", bIdentify);
  }
  //EricssonSip VIG identifyier when calls arrives from 3G
  else if ((stRemoteVendor.countryCode == EricssonSip_t35CountryCode) && (stRemoteVendor.manufactorCode == EricssonSip_manufacturerCode) &&
       (GetSystemCfgFlagInt<DWORD>(CFG_KEY_IP_MOBILE_PHONE_RATE) != 0))
  {
    m_remoteIdent = EricssonVigSip;
    bIdentify   = TRUE;
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Ericsson VIG For SIP calls");
  }
  //NetMeeting
  else if (strstr(stRemoteVendor.productId, NetMeetingId) && strstr(stRemoteVendor.productId, MicrosoftId))
  {
    //since the we didn't find a way (yet) to pass the all right reserve to the linux editors we are building the
    // NM identifier string according to the ASCII values.
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Identify NM");
    char NetMeetingProductId[50];
    memcpy(NetMeetingProductId, MicrosoftId, strlen(MicrosoftId));
    NetMeetingProductId[strlen(MicrosoftId)] = 0xae;
    NetMeetingProductId[strlen(MicrosoftId)+1] = ' ';
    memcpy(NetMeetingProductId + strlen(MicrosoftId) + 2, NetMeetingId, strlen(NetMeetingId));
    NetMeetingProductId[strlen(MicrosoftId) + 2 + strlen(NetMeetingId)] = 0;
    if (!strncmp(stRemoteVendor.productId, NetMeetingProductId, strlen(NetMeetingProductId)))
    {
      PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - NetMeeting");
      WORD bIsAutoVideoMode = m_pTargetModeH323->IsAutoVideoResolution();
      DWORD mobileRate = GetSystemCfgFlagInt<DWORD>(CFG_KEY_IP_MOBILE_PHONE_RATE);
      if (bIsAutoVideoMode)
      {
        // for Mobile Phone (3G) the flag will always different than zero, and we need to adjust the local
        // capability because of NM bugs
        CapEnum h323CapCode = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople);
          //NetMeeting does not know 264 so if I will move all other protocols but 264 (in RMX its only H263) - later in the code
        //we will move 264 also and the result we will not declare on any viedo capabilities.
        if(m_pLocalCapH323->AreCapsSupportProtocol(eH263CapCode,cmCapVideo))
          isRemoveH264 = TRUE;
        else
          PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - NM - local caps has only 264.");
        if(mobileRate)
          isRemoveOtherThenQCif = TRUE;
      }
    }
    m_remoteIdent = NetMeeting;
    bIdentify   = TRUE;
  }
//  if (strstr(PARTYNAME,"LinkToMaster"))
//         {
//             PTRACE2(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Disguising myself as endpoint  ", PARTYNAME);
//             m_bDisguiseAsEPMode = TRUE;
//             m_remoteIdent = AccordMCU;
//             isRemoveH239 = FALSE;
//             m_pParty->SetCascadeMode(SLAVE);
//         }

	if (m_pParty->GetCascadeMode() == SLAVE || m_pParty->GetCascadeMode() == MASTER || m_remoteIdent == PolycomRMX)//BRIDGE-7231
			isFixContentProtocol = TRUE;
  if ( (m_remoteInfo.remoteEndpointType == cmEndpointTypeGateway) && (m_remoteInfo.endPointNetwork == NetworkH320) &&
      (m_remoteIdent != PolycomRMX) && (m_remoteIdent != PolycomMGC) )
  {
    m_remoteIdent = RvGWOrProxy;    //it might be actually RV MCU. We need to know it for the cascade to their MCU
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Proxy or GW");
    bIdentify = TRUE;
  }

  // VNGFE-1594
  BOOL bIsVideoInFromVcr = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ALLOW_IN_VIDEO_FROM_IPVCR);
  // VNGFE-787
  if ((!strncmp(stRemoteVendor.productId, CodianVcrProductId, strlen(CodianVcrProductId)))
    && (bIsVideoInFromVcr == NO))
  {
    m_isCodianVcr = 1;
  }
  // LPR - All cascade blocking
  if ((!strncmp(stRemoteVendor.productId, "ACCORD MGC", strlen("ACCORD MGC")) || m_remoteIdent == PolycomMGC) && m_remoteInfo.endPointNetwork != NetworkH320
      && ((m_pTargetModeH323->GetConfType() == kVideoSwitch) || (m_pTargetModeH323->GetConfType() == kVSW_Fixed)) && (m_pLocalCapH323->IsLPR() == TRUE))
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - Call close: ling to MGC not supported in VSW calls");
    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_MGC_CASCADED_IN_HD_VSW_LPR_ENABLED_CONF);
  }
/*  if ((!strncmp(stRemoteVendor.productId, "ACCORD MGC", strlen("ACCORD MGC")) || m_remoteIdent == PolycomMGC) && m_remoteInfo.endPointNetwork != NetworkH320
      && ((m_pTargetModeH323->GetConfType() == kVideoSwitch) || (m_pTargetModeH323->GetConfType() == kVSW_Fixed)))
  {
    if (!strncmp(userUserName, "TRANSGW", strlen("TRANSGW")) || !strncmp(userUserName, "VSW_", strlen("VSW_")) ||
        !strncmp(userUserName, "IPONLY_", strlen("IPONLY_")) || !strncmp(userUserName, "VSMIXED_", strlen("VSMIXED_"))  ||
        !strncmp(userUserName, "GW_320", strlen("GW_320")))
      isRemoveLpr = TRUE;
  }*/


    //old endpoints
  if (m_remoteInfo.h225RemoteVersion < 4)
    {
        PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Remove LPR");
      isRemoveLpr=TRUE;
      m_pTargetModeH323->SetIsLpr(FALSE);
        if( stRemoteVendor.manufactorCode != PictureTel_manufacturerCode )
      isRemoveH239 = TRUE;

        //remove the drop field:
        if ((m_remoteInfo.h225RemoteVersion < 3) && m_pLocalCapH323->IsDropFieldCap())
        {
            PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Remove GenericVideoCap");
            isRemoveGenericVideoCap = TRUE;
        }
        if (m_remoteInfo.h225RemoteVersion < 3)
          isRemoveDPTR = TRUE;
    }
  //penteview VNGR-22662
      if( strstr(stRemoteVendor.productId, "penteview") )
      {
      	PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - penteview ");
      	isRemoveEPC = TRUE;
      	isRemoveH239 = TRUE;
      }


  // Any EP
    if (bAnyEpFlag)
    {
        PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - any vendor EP");
        // remove the P&C and 263+ from all vendors stay only with 263.
    // versions 2 and down not support generic audio cap except if it's ViaVideo with version 2.
        if ((m_remoteInfo.h225RemoteVersion < 3) && strncmp(stRemoteVendor.productId, ViaVideoVendorIdPrefix, strlen(ViaVideoVendorIdPrefix)) != 0)
        {
        	PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - any vendor EP -low version");
        	isRemoveEPC        = TRUE;
        	isRemoveGenericAudioCaps = TRUE;
        	isRemoveH264       = TRUE;
        	isRemoveAnnexQ       = TRUE;
        	isRemoveDBC2       = TRUE;
        	isRemoveLpr              = TRUE;
        	m_pTargetModeH323->SetIsLpr(FALSE);
        }

    }

  if (bIsH239 && (m_remoteInfo.h225RemoteVersion < 4) && (bAnyEpFlag || (m_remoteIdent == CiscoGW) || isPolycomVoIpEp))
  {
        PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Remove H239 (and EPC)");
    isRemoveH239 = TRUE;
  }

  //remove the drop field:
  if ((m_remoteInfo.h225RemoteVersion < 3) && m_pLocalCapH323->IsDropFieldCap())
  {
        PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Remove Generic Video");
    isRemoveGenericVideoCap = TRUE;
    }

    //recording link
    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
    CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
    isCascadeOrRecordingLink |=  pConfParty->GetRecordingLinkParty();

    isNeedToAddTranmitCaps = IsNeedToSendTrasmitCap();

  if (isRemoveGenericAudioCaps || isRemoveH239 || isRemoveAnnexQ || isRemoveRvFecc || isRemoveGenericVideoCap || isRemoveG7221C ||
      isRemoveG722 || isRemoveH264 || isRemoveDBC2 || isRemoveOtherThenQCif || isRemoteMgcWithLowRateConf || isCascadeOrRecordingLink || isFixContentProtocol || isRemoveLpr ||isRemoveEPC || isRemoveG7231 || isRemoveG719 || isRemoveDPTR || isNeedToAddTranmitCaps || isRemoveHP)
  {
    //COstrStream msg;
    //m_pLocalCapH323->Dump(msg);
    //PTRACE2(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - old caps are", msg.str().c_str());
    DWORD videoRate = m_pParty->GetVideoRate();
    BYTE isH263H621inLocalCAps = m_pLocalCapH323->IsFoundOrH263H261();
    BYTE is4cifEnabledinOriginalCapsTx = FALSE;
    BYTE is4cifEnabledinOriginalCapsRx = FALSE;
    if(isH263H621inLocalCAps && m_pLocalCapH323->Get4CifMpi() != -1 )
    {
        is4cifEnabledinOriginalCapsTx = TRUE;
        PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - 4ciftr");
    }
       if( isH263H621inLocalCAps && m_pLocalCapH323->GetMpi(eH263CapCode,k4Cif) != ((APIU8)-1))
    {
        is4cifEnabledinOriginalCapsRx = TRUE;
        PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - 4cifrec");
    }

    POBJDELETE(m_pLocalCapH323);
    m_pLocalCapH323 = new CCapH323;
        PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - ReBuild caps");
        BYTE highestframerate = ((BYTE)eCopVideoFrameRate_None);
        if(m_pTargetModeH323->GetConfType() == kCop)
        {
          CVidModeH323* copHighetlevel= m_pCopVideoModes->GetVideoMode(0);
          WORD profile;
          BYTE level;
          long maxMBPS,maxFS,maxDPB,maxBR,maxSAR,maxStaticMbps,brandcpb;
          copHighetlevel->GetH264Scm(profile,level,maxMBPS,maxFS,maxDPB,maxSAR,brandcpb,maxStaticMbps);
          highestframerate = GetCopFrameRateAccordingtoMbpsAndFs(level,maxMBPS,maxFS);

        }
        if(isNeedToAddTranmitCaps)
        {
          m_pLocalCapH323->BuildCapsWithSpecialCaps(videoRate, m_pTargetModeH323, PARTYNAME, isRemoveGenericAudioCaps, isRemoveH239, isFixContentProtocol,
                          isRemoveAnnexQ, isRemoveRvFecc,isRemoveGenericVideoCap, isRemoveG722, isRemoveH264,
                          isRemoveDBC2, isRemoveOtherThenQCif, isRemoveG7221C,isRemoteMgcWithLowRateConf,isRemoveLpr, isCascadeOrRecordingLink,isRemoveEPC,isRemoveG7231,isRemoveG719,m_serviceId,((ECopVideoFrameRate)highestframerate),m_pCopVideoModes,isRemoveDPTR);
        }
        else
        {

          m_pLocalCapH323->BuildCapsWithSpecialCaps(videoRate, m_pTargetModeH323, PARTYNAME, isRemoveGenericAudioCaps, isRemoveH239, isFixContentProtocol,
                                    isRemoveAnnexQ, isRemoveRvFecc,isRemoveGenericVideoCap, isRemoveG722, isRemoveH264,
                            isRemoveDBC2, isRemoveOtherThenQCif, isRemoveG7221C,isRemoteMgcWithLowRateConf,isRemoveLpr,
                       isCascadeOrRecordingLink,isRemoveEPC,isRemoveG7231,isRemoveG719,m_serviceId,((ECopVideoFrameRate)highestframerate),NULL/*NO TRANSMIT CAPS*/,isRemoveDPTR);
  }
    if ((isRemoveGenericAudioCaps && m_isAutoVidBitRate) ||isRemoteMgcWithLowRateConf)
    {
      m_pLocalCapH323->UpdateCorrectVideoRateAfterRemovingGenericCap(m_pmcCall->GetRate());
    }

    if( !strncmp(stRemoteVendor.productId, RMX1000ProductId ,strlen(RMX1000ProductId)) && m_pTargetModeH323->GetConfType() != kVSW_Fixed && m_pTargetModeH323->GetConfType() != kVideoSwitch)
               m_pLocalCapH323->SetVideoBitRate((m_pTargetModeH323->GetCallRate()*10), kRolePeople, eUnknownAlgorithemCapCode);
    m_pLocalCapH323->BuildSortedCap();
    if(m_pTargetModeH323->GetConfType() == kCp || m_pTargetModeH323->GetConfType() == kCop)
    {
      BYTE protocol = pConfParty->GetVideoProtocol();
      m_pLocalCapH323->SetSingleVideoProtocolIfNeeded (protocol);
    }
    if(!isH263H621inLocalCAps)
    {
      PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - removeh263andh261");
      m_pLocalCapH323->RemovePeopleCapSet(eH263CapCode);
      m_pLocalCapH323->RemovePeopleCapSet(eH261CapCode);
    }
    if(!is4cifEnabledinOriginalCapsTx)
    {
      m_pLocalCapH323->Set4CifMpi ((APIS8)-1);

    }
    else
    {
      m_pLocalCapH323->Set4CifMpi ((APIS8)2);
    }
    if(!is4cifEnabledinOriginalCapsRx)
      m_pLocalCapH323->SetH263FormatMpi(k4Cif, -1, kRolePeople);

        m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
  }


  //If we need to force CIF rsrcs to party (according to flag).

   BYTE confBitRate = pCommConf->GetConfTransferRate();
   CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
  DWORD h323Rate = lCapInfo.TranslateReservationRateToIpRate(confBitRate);
  BYTE IsCIFForce = IsSetCIFRsrcForUser(stRemoteVendor.productId, h323Rate);
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - this is conf rate, ",h323Rate);
  if(IsCIFForce && (m_pTargetModeH323->GetConfType() == kCp ||  m_pTargetModeH323->GetConfType() == kCop))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Force CIF rsrc - ReBuild caps");
    eVideoPartyType videoPartyType = eCP_H264_upto_CIF_video_party_type;
    APIS8 cif4Mpi = -1;

    SetH264ModeInLocalCapsAndScmAccordingToVideoPartyType(videoPartyType,cif4Mpi);
    m_pLocalCapH323->Remove4CifFromH263VideoCap(); // currently it relevant to cop only. in cp we don't declare 4cif anyway.
    m_pLocalCapH323->SetH263FormatMpi(k4Cif, -1, kRolePeople);
    m_pLocalCapH323->Set4CifMpi ((APIS8)-1);
    COstrStream msg;
    m_pLocalCapH323->Dump(msg);
    PTRACE2(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - new caps ", msg.str().c_str());
    m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
  }
  DWORD callRate = m_pmcCall->GetRate()/1000;
//  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - CALL RATE , ",callRate);
  h323Rate = min(h323Rate, callRate);
  DWORD NewVideoRteForUser = GetNewRateForUser(stRemoteVendor.productId,h323Rate);
  if(NewVideoRteForUser && (m_pTargetModeH323->GetConfType() == kCp ||  m_pTargetModeH323->GetConfType() == kCop))
  {
    PTRACE2INT(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Force video rate of , ",NewVideoRteForUser);
    m_pLocalCapH323->SetVideoBitRate(NewVideoRteForUser,kRolePeople);
    COstrStream msg;
    m_pLocalCapH323->Dump(msg);
    PTRACE2(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - new caps ", msg.str().c_str());
    m_pTargetModeH323->SetVideoBitRate(NewVideoRteForUser,cmCapReceiveAndTransmit,kRolePeople);
    m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
    m_FixVideoRateAccordingToType = TRUE;
    m_pTargetModeH323->Dump("CH323Cntl::ChangeCapabilitySetAccordingToVendor new rate -  :", eLevelInfoNormal);
    m_pmcCall->SetBandwidth(( (NewVideoRteForUser + 640) *200) );
    m_pParty->SetVideoRate(NewVideoRteForUser);
    m_pTargetModeH323->SetTotalVideoRate(NewVideoRteForUser);

  }

  BYTE IsHd720Force = IsSetHD720RsrcForUser(stRemoteVendor.productId);
  BYTE isRemoteSupportHighProfie = FALSE;
  BOOL bEnableHighfProfile = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE);
  if(bEnableHighfProfile && strstr(stRemoteVendor.productId,"HDX") && ( strstr(stRemoteVendor.versionId,"2.6") || strstr(stRemoteVendor.versionId,"2.7")  ))
  {
    isRemoteSupportHighProfie = TRUE;
  }
  if(!isRemoteSupportHighProfie && IsHd720Force && m_pTargetModeH323->GetVideoPartyType(cmCapReceiveAndTransmit) > eCP_H264_upto_HD720_30FS_Symmetric_video_party_type && m_pTargetModeH323->GetConfType() == kCp )
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - setting to HD720 when remote do not support Highprofile ");
    APIS8 cif4 = m_pLocalCapH323->Get4CifMpi();
    SetH264ModeInLocalCapsAndScmAccordingToVideoPartyType(eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,cif4);
  }
  if(!IsCIFForce && !strncmp(stRemoteVendor.productId,"VSX 3000",strlen("VSX 3000"))  && m_pTargetModeH323->GetConfType() == kCp)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - VSX-3000 disable 4cif in rec because it send 4cif in very low fps ");
    m_pLocalCapH323->SetH263FormatMpi(k4Cif, -1, kRolePeople);
    APIS8 cif4 = m_pLocalCapH323->Get4CifMpi();
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendor - VSX-3000 disable 4cif in rec because it send 4cif in very low fps  -4cif is ",cif4);
    SetH264ModeInLocalCapsAndScmAccordingToVideoPartyType(eCP_H264_upto_CIF_video_party_type,cif4);
    m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
  }

  if (strstr(stRemoteVendor.productId, "Tandberg MXP"))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - Tandberg Ep -MXP");
    m_useRtcp = 0;
  }

  //fix for SABAN VNGFE-4085
  BOOL bIsForced25fpsOnHdx = GetSystemCfgFlagInt<BOOL>(CFG_KEY_H239_FORCE_CAPABILITIES);

  if (bIsForced25fpsOnHdx && strstr(stRemoteVendor.productId, "HDX 8000"))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendor - HDX 8000");
    APIU16 profile = 0;
    APIU8 level=0;
    long Targetmbps=0, Targetfs=0, dpb=0, brAndCpb=0, sar=0, staticMB=0;
    m_pTargetModeH323->GetH264Scm(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
    level=64, Targetmbps=180, Targetfs=15;
    m_pTargetModeH323->SetH264Scm(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, cmCapReceive);

    m_pLocalCapH323->SetLevelAndAdditionals(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, kRolePeople);
  }
}

////////////////////////////////////////////////////////////////////////////
//This function change the capability according to the type of Ep we work with.
//In cases of 263+ conferences not all the EP support it.
// This is a new function which used when FacilityInd received
void  CH323Cntl::ChangeCapabilitySetAccordingToVendorId()
{
  PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendorId - Without parameters from facility");
  RemoteVendorSt stRemoteVendor;
  stRemoteVendor.productId = new char[H460_C_ProdIdMaxSize];
  memset(stRemoteVendor.productId, '\0', H460_C_ProdIdMaxSize);
  stRemoteVendor.versionId = new char[H460_C_VerIdMaxSize];
  memset(stRemoteVendor.versionId, '\0', H460_C_VerIdMaxSize);

  DuplicateRemoteVendorInfo(stRemoteVendor);
  BYTE isCascadeToPreviousVer = FALSE;
  ChangeCapabilitySetAccordingToVendor(stRemoteVendor, isCascadeToPreviousVer);

  delete[] stRemoteVendor.productId;
  delete[] stRemoteVendor.versionId;
}

//This function change the capability according to the type of Ep we work with.
//In cases of 263+ conferences not all the EP support it.
// This is a new function which used when CallConnectedInd received
bool  CH323Cntl::ChangeCapabilitySetAccordingToVendorId(mcIndCallConnected* pCallConnectedInd, BYTE& isCascadeToPreviousVer)
{
  PTRACE(eLevelInfoNormal, "CH323Cntl::ChangeCapabilitySetAccordingToVendorId - With parameters from call connected");
  RemoteVendorSt stRemoteVendor;
  stRemoteVendor.productId = new char[H460_C_ProdIdMaxSize];
  memset(stRemoteVendor.productId, '\0', H460_C_ProdIdMaxSize);
  stRemoteVendor.versionId = new char[H460_C_VerIdMaxSize];
  memset(stRemoteVendor.versionId, '\0', H460_C_VerIdMaxSize);

  CheckAndUpdateRemoteVendor(stRemoteVendor, pCallConnectedInd);
  m_remoteInfo.remoteEndpointType = pCallConnectedInd->remoteEndpointType;
  m_remoteInfo.h225RemoteVersion  = pCallConnectedInd->h225RemoteVersion;
  m_remoteInfo.endPointNetwork  = pCallConnectedInd->endPointNetwork;

  ChangeCapabilitySetAccordingToVendor(stRemoteVendor, isCascadeToPreviousVer);

  delete[] stRemoteVendor.productId;
  delete[] stRemoteVendor.versionId;

  // for Call Generator - Vendor detection
  if (stRemoteVendor.manufactorCode != Polycom_manufacturerCode && stRemoteVendor.manufactorCode != Accord_manufacturerCode)
    return false;
  return true;
}

/* The OLD Function !!!
      //26.11.2006 Changes by VK. Avaya SIP CM
      if (pCallConnectedInd->avfFeVndIdInd.bSipCM == AVAYA_SIP_CM_FLAG_ON)
        m_remoteVendor.isAvayaSipCm = AVAYA_SIP_CM_FLAG_ON;
          (!strncmp(productID, "ACCORD MGC" ,strlen("ACCORD MGC")))) // added for call via proxy)
  //  if (m_pTargetModeH323->GetConfType() == kVideoSwitch) //VSW conference
  //    isCascadeToPreviousVer = ChangeCapsForSpecialVSWCascades(m_pmcCall->GetCallTransient().userUser);
  //  else
  //    ChangeCapsForSpecialCP2VSWCascades(m_pmcCall->GetCallTransient().userUser);
//  if ((bIdentify == FALSE) && (m_PNGkState == ePnRouted) && (::GetpSystemCfg()->IsPathNavigatorOldVersionInRoutedMode() == YES))
//  {
//    PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendorId - Routed PN");
//    if (m_pParty->IsH239Conf() && (h225RemoteVersion >= 4))
//    {
//      bAnyEpFlag = FALSE; //in order not to remove the EPC caps
//    }
//
//    if (::GetpSystemCfg()->IsRemoveG722AndGenericAudio() == YES)
//    {
//      PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendorId - Routed PN - remove audio");
//      //remove the G722 cause if not - the EP does not open audio channel.
//      numCapRemoved = m_pLocalCapH323->RemoveProtocolFromCapSet(_G722);
//      if(numCapRemoved)
//        m_pTaskApi->SendRemovedProtocolToConfLevel(numCapRemoved, _G722);
//
//      RemoveGenericCapAndUpdateConfLevel();
//    }
//  }
    if(bAnyEpFlag)
    {
        PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendorId - any vendor EP");

        // remove the P&C and 263+ from all vendors stay only with 263.

    // versions 2 and down not support generic audio cap except if it's ViaVideo with version 2.
    if ((h225RemoteVersion < 3) && strncmp(productID,ViaVideoVendorIdPrefix,strlen(ViaVideoVendorIdPrefix)) != 0)
    {
      isRemoveGenericAudioCaps = TRUE;
      isRemoveH264       = TRUE;
      isRemoveAnnexQ       = TRUE;
      isRemoveDBC2       = TRUE;
    }
    }

  if (bIsH239 && (h225RemoteVersion < 4))
  {
    isRemoveH239 = TRUE;
  }

  //remove the drop field:
  if ( (h225RemoteVersion < 3) && m_pLocalCapH323->IsDropFieldCap())
    isRemoveGenericVideoCap = TRUE;


  if(isRemoveGenericAudioCaps || isRemoveH239 || isRemoveAnnexQ || isRemoveRvFecc || isRemoveGenericVideoCap || isRemoveG722 || isRemoveH264 || isRemoveDBC2 ||
        isRemoveOtherThenQCif || isAddG7231 || isRemoveG7221C)
  {
    DWORD videoRate = m_pParty->GetVideoRate();
    POBJDELETE(m_pLocalCapH323);
    m_pLocalCapH323 = new CCapH323;
        PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendorId - ReBuild caps");

    m_pLocalCapH323->BuildCapsWithSpecialCaps(videoRate, m_pTargetModeH323, PARTYNAME,
                        isRemoveGenericAudioCaps, isRemoveH239, isRemoveAnnexQ, isRemoveRvFecc,
                        isRemoveGenericVideoCap, isRemoveG722,
                        isRemoveH264, isRemoveDBC2, isRemoveOtherThenQCif,isAddG7231,isRemoveG7221C);

    if (isRemoveGenericAudioCaps && m_isAutoVidBitRate)
    {
      DWORD newVideoRate = 0;
      newVideoRate = m_pLocalCapH323->UpdateCorrectVideoRateAfterRemovingGenericCap(m_pmcCall->GetRate());
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ChangeCapabilitySetAccordingToVendorId - newVideoRate= ",newVideoRate);
      if (newVideoRate == 0)
      {
        m_pTargetModeH323->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit);
        m_pTargetModeH323->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit,kRoleContentOrPresentation);
        m_pParty->UpdateVideoRate(newVideoRate);
        m_pLocalCapH323->RemoveSpecificDataTypeFromCaps(cmCapVideo);
        m_pLocalCapH323->RemoveSpecificDataTypeFromCaps(cmCapData);
      }
      else
        m_pParty->UpdateVideoRate(newVideoRate);

    }

    m_pLocalCapH323->BuildSortedCap();
    m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
  }
}
//End of OLD Function!!!
*/

///////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::RoundUpConfRate(int CallRate)
{
  DWORD roundedRate = 0;

  if (CallRate <= 640)
    roundedRate = 640;
  else if (CallRate <= 1280)
     roundedRate = 1280;
  else if (CallRate <= 1920)
     roundedRate = 1920;
  else if (CallRate <= 2560)
     roundedRate = 2560;
  else if (CallRate <= 3840)
     roundedRate = 3840;
  else if (CallRate <= 5120)
     roundedRate = 5120;
  else if (CallRate <= 7680)
     roundedRate = 7680;
  else if (CallRate <= 10240)
     roundedRate = 10240;
  else if (CallRate <= 14720)
     roundedRate = 14720;
  else if (CallRate <= 19200)
     roundedRate = 19200;

  return roundedRate;
}



/*BYTE CH323Cntl::ChangeCapsForSpecialVSWCascades(const char* userUser)
{
//  char pFoundString[MaxAddedStringSize];
  BYTE isCascadeToPreviousVer = FALSE;
  BYTE rval = 0;

  // CASCADE VSW_IP_ONLY - VSW_MIXED
  //code in order to connect the ip only not as secondary
  else if(m_pParty->IsIpOnlyConf()) //VSW ip only
  {
    rval = GetUserUserStringByOpcode(userUser, VsMixed, pFoundString);
    if(rval)
    {// we found the VSW Mixed opcode so get the new video rate from pFoundString
      char *pString = pFoundString + strlen(UserUserMessageTable[VsMixed]);
      int mixed_Rate = atoi(pString);//the video bit rate of the vsw mixed conference.

      if(mixed_Rate < m_pParty->GetVideoRate())
      {//therefore mixed_NumOfTs < ipOnly_NumOfTs and there is no problem to
        //decrease the ipOnly rate

        PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapsForSpecialVSWCascades - Cascade VSW IP ONLY - VSW MIXED");

        int isdnIpRate = m_pParty->GetVideoRate() - 16; //minus FAS+BUS

        //the video rate if we calculate it according to VSW mixed rate rules
        DWORD aud_bitrate       = 0;
        DWORD lsd_bitrate       = 0;
        DWORD hsd_bitrate       = 0;
        DWORD mlp_bitrate       = 0;
        DWORD hmlp_bitrate      = 0;
        DWORD content_rate      = 0;
        DWORD mixedRateOfIPConf = 0;
        // get from the SCM the basic rates
        m_pCurrentScm->GetMediaBitrate(aud_bitrate,mixedRateOfIPConf,lsd_bitrate,hsd_bitrate,mlp_bitrate,hmlp_bitrate,content_rate);
        mixedRateOfIPConf += content_rate;
        if(aud_bitrate == rate64K)// at the conference calculation the audio algorithm is 56k
          mixedRateOfIPConf = mixedRateOfIPConf + rate8K;

        // same calculation as in the video rate function calculations
        mixedRateOfIPConf = mixedRateOfIPConf / 800 * 800; // rounded down
        mixedRateOfIPConf = mixedRateOfIPConf *96 / 100; //remove BCH
        mixedRateOfIPConf = mixedRateOfIPConf / 800 * 800;// rounded down
        mixedRateOfIPConf = mixedRateOfIPConf / 100;// the rate is in 100 bit per second

        //to enable cascade between them the conf rate should be the same
        if ((isdnIpRate - mixed_Rate < 100) ||  //example:384conf: (ipOnly=3200-16=3184) - mixed=3128) = 56
          (mixedRateOfIPConf == mixed_Rate))
        {
          DWORD mixed_TdmRate = CalculateTdmRate(mixed_Rate);
          m_pTaskApi->SendFlowControlPartyToConflevel(mixed_Rate, mixed_TdmRate);
        }m_pCsRsrcDesc->GetConnectionId()
        else
          PTRACE(eLevelInfoNormal,"CH323Cntl::ChangeCapsForSpecialVSWCascades - remote rate at cascade is too low");
      }
    }
  }
  return isCascadeToPreviousVer;
}


///////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::ChangeCapsForSpecialCP2VSWCascades(const char* userUser)
{
  // cascade to Previous version has no effect because we used the allocated ability
  // of this party and since its CP there is no flow control effect on the conference.
  char pFoundString[MaxAddedStringSize];
  BYTE rval = FALSE;

  //Search for CascadeVSW, since it is sent only in version 4.6 and less
  DWORD VswType = IpOnly;
  //Search for IpOnly, since it is sent only in version 5 and upper
  // CASCADE CP - VSW_IpOnly or CP - SWCP
  rval = GetUserUserStringByOpcode(userUser, IpOnly, pFoundString);

  // CASCADE CP - VSW_MIXED
  if(rval == FALSE)//Search for VsMixed, since it is sent only in version 5 and upper
  {
    rval = GetUserUserStringByOpcode(userUser, VsMixed, pFoundString);
    VswType = VsMixed;
  }

  if(rval)
  {
    // if we are here it CP call trying to cascade to our MCU and the remote conference
    // is VSW or SWCP
    // if the Call rates is bigger than the video rate of the remote
    // and if the video rate of the remote is bigger than the local video rate
    // upgrade the local rate (at the party level and conference level)
    // return

    char *pString = pFoundString + strlen(UserUserMessageTable[VswType]);
    int  VswRate  = atoi(pString);//the video bit rate of the vsw conference.

    if((VswRate < (m_pmcCall->maxRate/100)) && (VswRate > m_pParty->GetVideoRate()))
    {
      DWORD vswTdmRate;
      vswTdmRate = CalculateTdmRate(VswRate);// VSW mixed rate

      m_pLocalCapH323->SetVideoBitRate(VswRate);
      m_pTaskApi->SendNewVideoRateToConflevel(VswRate, vswTdmRate);
    }
  }
  return 0;
}*/


////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::NoActionFunc()
{
  PTRACE2(eLevelInfoNormal,"CH323Cntl::NoActionFunc - ",PARTYNAME);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnPartyCallAnswerReq(int reason, BYTE isGkRouted)
{
  if (m_isCallAnswerReject)
  {
    // 21791
    PTRACE2INT(eLevelError, "CH323Cntl::OnPartyCallAnswerReq - We already rejected this call, we should not send call answer again! - %d", m_pCsRsrcDesc->GetConnectionId());
    DBGPASSERT(m_pCsRsrcDesc->GetConnectionId());
    return;
  }

  // Trace of the Answer request
  DWORD msgLen = LargePrintLen + strlen(PARTYNAME)+1;
  ALLOCBUFFER(msgStr,msgLen); AUTO_DELETE_ARRAY(msgStr);
  msgStr[0] = '\0';
  if(m_pmcCall)
	  sprintf(msgStr, "  Party Name is %s, McmsConnId is = %d, Status is = %d, ServiceId is %d, reason is %d",PARTYNAME,
							 m_pCsRsrcDesc->GetConnectionId(),
							 m_pmcCall->GetCallStatus(),
							 m_serviceId,
							 reason);
  else
  	PASSERT_AND_RETURN(1);

  PTRACE2(eLevelInfoNormal,"CH323Cntl::OnPartyCallAnswerReq \n ",msgStr);
  DEALLOCBUFFER(msgStr);


  APIU32 lengthStructure = sizeof(mcReqCallAnswer);

  //In avaya environment the m_pDHKeyManagement is initialize but we do not need to use it here.
  if(!m_bIsAvaya)
    lengthStructure += GetEncrySectionLen(m_pDHKeyManagement);

  m_callIndex = m_pH323NetSetup->GetCallIndex();
  m_pDestUnitId = m_pH323NetSetup->GetSrcUnitId();
  mcReqCallAnswer* pCallAnswerReq = (mcReqCallAnswer *)new BYTE[lengthStructure];
  memset(pCallAnswerReq, '\0', lengthStructure);

  strncpy(pCallAnswerReq->callTransient.sDisplay, m_pH323NetSetup->GetLocalDisplayName(), MaxDisplaySize);
  pCallAnswerReq->callTransient.sDisplay[MaxDisplaySize-1] = '\0';
  pCallAnswerReq->callTransient.sDisplaySize = strlen(pCallAnswerReq->callTransient.sDisplay);

  TRACEINTO << "m_pH323NetSetup->GetH323PartyAlias: " << m_pH323NetSetup->GetH323PartyAlias() << " , m_pH323NetSetup->GetEndpointType(): " << (DWORD)m_pH323NetSetup->GetEndpointType();

  //Bridge-12756 In call received from RMX Gateway: if checkbox "Register as Gatway" in GK configuration is true then EP is cmEndpointTypeGateway else cmEndpointTypeMCU
  if(strstr(m_pH323NetSetup->GetH323PartyAlias(),"GW_") && (cmEndpointTypeMCU == m_pH323NetSetup->GetEndpointType() || cmEndpointTypeGateway == m_pH323NetSetup->GetEndpointType()) ) //since the EP=GW setting isn't unique to GW (cascade is also set to GW ..)... need to check the party name that has GW_ in it
  {
                  m_pParty->SetIsCallFromGateway(true);
                  PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallAnswerReq Party is call from Gateway !!! \n ");
  }


  int userUserSize = 0;
  pCallAnswerReq->callTransient.userUser[0]  = '\0';
  //Multiple links for ITP in cascaded conference feature: CH323Cntl::OnPartyCallAnswerReq - for dialIN undefined (example confname:m -> partyname:m_(000)_1)
  CCommConf* pCommConf = NULL;
  if (m_pParty)
      pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  else
  {
	  PDELETEA (pCallAnswerReq);
      PASSERT_AND_RETURN(1);
  }

  if (pCommConf && m_pParty)
  {
      CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());

      if (pConfParty)
      {

          if(m_pParty->IsCallFromGateway())
          {
			  pConfParty->SetIsCallFromGW(1);
			  TRACEINTO << "setting pConfParty->SetIsCallFromGW(1)";
          }

    	  //Send Info to CDR for H323 dial in
    	  pCommConf->PartyCorrelationDataToCDR(pConfParty->GetName(), m_pParty->GetMonitorPartyId(), pConfParty->GetCorrelationId());

          m_linkType = pConfParty->GetPartyType();

          if (pConfParty->GetPartyType() != eSubLinkParty)
              pConfParty->SetMainPartyNumber(pCommConf->NextMainPartiesCounter());

          TRACESTR(eLevelError) << "ITP_CASCADE: CH323Cntl::OnPartyCallAnswerReq partyName:" << pConfParty->GetName()  << " MainPartyNumber:" << pConfParty->GetMainPartyNumber()
        					    << "correlationid= " << pConfParty->GetCorrelationId();

          if ( (pConfParty->GetPartyType() == eMainLinkParty)  &&  m_pmcCall && (m_pmcCall->GetIsOrigin() == FALSE) )
          {
              char strForITPcascade[H243_NAME_LEN];
              memset(strForITPcascade, '\0', H243_NAME_LEN);

              char  mainLinkDialInNumber [5];
              snprintf(mainLinkDialInNumber,sizeof(mainLinkDialInNumber),"%d",pConfParty->GetMainPartyNumber());

              strcat(strForITPcascade,mainLinkDialInNumber);
              strcat(strForITPcascade,"\0");

              userUserSize = SetUserUserFieldForMultipleLinksForITPcascadedConf(pCallAnswerReq->callTransient.userUser,strForITPcascade);
          }
      }
      else
      {
          PTRACE(eLevelInfoNormal, "ITP_CASCADE: CH323Cntl::OnPartyCallAnswerReq ERROR - pConfParty is NULL");
      }
  }
  else
  {
      PTRACE(eLevelInfoNormal, "ITP_CASCADE: CH323Cntl::OnPartyCallAnswerReq ERROR - pCommConf is NULL");
  }

  //in VSW add to the useruser the conf type and its rate.
  if((kVSW_Fixed == m_pTargetModeH323->GetConfType()))//VSW or SWCP
  {
    char stringVidRate[12];
//    itoa(m_pParty->GetVideoRate(), stringVidRate, 10);
    int opcode = -1;
    if(m_pParty)
    {
      sprintf(stringVidRate,"%d",m_pParty->GetVideoRate());
      opcode = ( (m_pParty->IsIpOnlyConf()) ? IpOnly : VsMixed ) ;
    }
    else
      PASSERT(1);

    userUserSize = SetUserUserFieldByOpcode(pCallAnswerReq->callTransient.userUser, opcode, stringVidRate);
  }

  if(m_pTargetModeH323->GetConfType()== kCop)
  {
      userUserSize = SetUserUserFieldByOpcode(pCallAnswerReq->callTransient.userUser, COP, NULL);
  }


  pCallAnswerReq->callTransient.userUserSize = userUserSize;

  m_getPortInd.srcCallSignalAddress.transAddr.port = m_pH323NetSetup->GetLocalH225Port();

  if(m_pmcCall && m_pmcCall->GetCallStatus() != 0)
  {
    if (reason >= 0)
      pCallAnswerReq->rejectCallReason = reason;
    else
      pCallAnswerReq->rejectCallReason = cmReasonTypeNoPermision;

    m_isCallAnswerReject = TRUE;  //we send call answer with -1 or -2 and the card will send callidle
  }
    memset(&(pCallAnswerReq->remoteAddress),0,sizeof(mcXmlTransportAddress));
  pCallAnswerReq->remoteAddress.unionProps.unionSize = sizeof(ipAddressIf);
  //in case of facility: the status is -2
  if( m_pmcCall && (m_pmcCall->GetCallStatus() == -2) &&
   ((reason == int(cmReasonTypeCallForwarded))||(reason == cmReasonTypeRouteCallToGatekeeper) ))
   {
    if(isGkRouted) // IpV6 - Temp
		{
			PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallAnswerReq.isGkRouted true");
			if(m_numOfGkRoutedAddres > 0)
			{
				PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallAnswerReq.m_numOfGkRoutedAddres bigger than null");
				if (m_gkRouteAddress.ipVersion == eIpVersion4)
				{
					pCallAnswerReq->remoteAddress.unionProps.unionType = eIpVersion4;
					pCallAnswerReq->remoteAddress.transAddr.addr.v4.ip	= m_gkRouteAddress.addr.v4.ip;
				}
				else
				{
					pCallAnswerReq->remoteAddress.unionProps.unionType = eIpVersion6;
					pCallAnswerReq->remoteAddress.transAddr.addr.v6.scopeId = m_gkRouteAddress.addr.v6.scopeId;
					APIU32 i = 0;
					for (i = 0; i < IPV6_ADDRESS_BYTES_LEN; i++)
					{
						pCallAnswerReq->remoteAddress.transAddr.addr.v6.ip[i] = m_gkRouteAddress.addr.v6.ip[i];
					}
				}
				pCallAnswerReq->remoteAddress.transAddr.ipVersion = m_gkRouteAddress.ipVersion;
			}
			else
			{
				PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCallAnswerReq. numOfGkRoutedAddres is zero. m_serviceId = ", m_serviceId);
				CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
				CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
				ipAddressStruct  * gkIp = pServiceParams->GetGKIp();
				if (gkIp->ipVersion == eIpVersion4)
				{
					PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallAnswerReq.IpV4 version found");
					pCallAnswerReq->remoteAddress.unionProps.unionType = eIpVersion4;
					pCallAnswerReq->remoteAddress.transAddr.addr.v4.ip  = gkIp->addr.v4.ip;

					/*  eyaln */
					char str[128];
					memset(str, '\0',128);
					::ipV4ToString(gkIp->addr.v4.ip, str);
					TRACESTR(eLevelInfoNormal) << "!@#eyaln: IP address of GK taken from service is: " << str ;
			}
			else if (gkIp->ipVersion == eIpVersion6)
			{ // Case IpV6
				PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallAnswerReq.IpV6 version found");
				pCallAnswerReq->remoteAddress.unionProps.unionType = eIpVersion6;
				pCallAnswerReq->remoteAddress.transAddr.addr.v6.scopeId = gkIp->addr.v6.scopeId;
				APIU32 i = 0;
				for (i = 0; i < IPV6_ADDRESS_BYTES_LEN; i++)
				{
					pCallAnswerReq->remoteAddress.transAddr.addr.v6.ip[i] = gkIp->addr.v6.ip[i];
				}
				}
				else
				{
					PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallAnswerReq.Fallback to IpV4. Incorrect version found");
					pCallAnswerReq->remoteAddress.unionProps.unionType = eIpVersion4;
					pCallAnswerReq->remoteAddress.transAddr.addr.v4.ip	= gkIp->addr.v4.ip;
				}
				pCallAnswerReq->remoteAddress.transAddr.ipVersion = gkIp->ipVersion;
			}

			pCallAnswerReq->remoteAddress.transAddr.port  = 1720;
			pCallAnswerReq->remoteAddress.transAddr.distribution = eDistributionUnicast;
			pCallAnswerReq->remoteAddress.transAddr.transportType = eTransportTypeTcp;
		}
	}

  memcpy(pCallAnswerReq->conferenceId, m_pH323NetSetup->GetH323ConfIdAsGUID(), MaxConferenceIdSize);
  // debuging for perpose of Proshare
  //  pCallAnswerReq->conferenceId[MaxConferenceIdSize - 1] = '\0'; the field is checked as an array
    if (strstr(PARTYNAME,"LinkToMaster") || m_pParty->GetCascadeMode() == SLAVE)
        pCallAnswerReq->localEndpointType = cmEndpointTypeGateway; //cmEndpointTypeMCU;// the type is MCU in any working case. ignoring the previous setting.
    else if ( m_pParty->IsCallGeneratorParty() )
      pCallAnswerReq->localEndpointType = cmEndpointTypeTerminal;
    else
      pCallAnswerReq->localEndpointType = cmEndpointTypeMCU;
    
  //pCallAnswerReq->localEndpointType     = cmEndpointTypeMCU;
  pCallAnswerReq->maxRate           = m_pmcCall->GetMaxRate();

  // telling the card that it could be an EPC call
  pCallAnswerReq->conferenceType = confTypeUnspecified;
  if (m_pParty->IsH239Conf())
  {
    pCallAnswerReq->conferenceType |= confTypeH239;
    pCallAnswerReq->conferenceType |= confTypePPCVersion1;
  }

  if(m_pLocalCapH323->IsFECC())
    pCallAnswerReq->conferenceType |= confTypeFECC;

  if ((m_pmcCall->GetSetupH245Address())->ipVersion == eIpVersion4)
  {
//    pCallAnswerReq->h245Address.transAddr.addr.v4.ip  = (m_pmcCall->GetSetupH245Address())->addr.v4.ip;

	  //std::string strCloudIp = GetSystemCfgFlagStr<std::string>(m_serviceId, CFG_KEY_CLOUD_IP);
	  std::string strCloudIp = GetCloudIp();

	  if (strCloudIp.length() > 0)
	  {
		  pCallAnswerReq->h245Address.transAddr.addr.v4.ip  = ::SystemIpStringToDWORD(strCloudIp.c_str());
	  }
	  else
	  {
		  pCallAnswerReq->h245Address.transAddr.addr.v4.ip  = (m_pmcCall->GetSetupH245Address())->addr.v4.ip;
	  }

//    APIU32 i = 0;
  }
  else
  {   // Case IpV6
    pCallAnswerReq->h245Address.transAddr.addr.v6.scopeId = (m_pmcCall->GetSetupH245Address())->addr.v6.scopeId;
    APIU32 i = 0;
    for (i =0 ; i < IPV6_ADDRESS_BYTES_LEN; i++)
       {
    	pCallAnswerReq->h245Address.transAddr.addr.v6.ip[i] = (m_pmcCall->GetSetupH245Address())->addr.v6.ip[i];
    }

  }
  pCallAnswerReq->h245Address.transAddr.port  = (m_pmcCall->GetSetupH245Address())->port;
  pCallAnswerReq->h245Address.transAddr.distribution = (m_pmcCall->GetSetupH245Address())->distribution;
  pCallAnswerReq->h245Address.transAddr.transportType = (m_pmcCall->GetSetupH245Address())->transportType;
  pCallAnswerReq->h245Address.transAddr.ipVersion = (m_pmcCall->GetSetupH245Address())->ipVersion;

  // fill the XML header of IP address
  pCallAnswerReq->h245Address.unionProps.unionType = (m_pmcCall->GetSetupH245Address())->ipVersion;
  pCallAnswerReq->h245Address.unionProps.unionSize = sizeof(ipAddressIf);

  //encryption section--------------------------
  if(!m_bIsAvaya)
    SetEncryptionParams(pCallAnswerReq->encryTokens,m_pmcCall->GetIsOrigin(),m_pDHKeyManagement);
  else
  {
    SetEncryptionInStructToZero(pCallAnswerReq->encryTokens);
    }

    if (strstr(PARTYNAME,"LinkToMaster") || m_pParty->GetCascadeMode() == SLAVE)
        pCallAnswerReq->bIsCascade = SLAVE;
    else if ( m_pParty->GetCascadeMode() == MASTER)
        pCallAnswerReq->bIsCascade = MASTER;
    else
        pCallAnswerReq->bIsCascade = NONE;

  if ( (reason == int(cmReasonTypeCallForwarded)) || (reason == int(cmReasonTypeRouteCallToGatekeeper)))
    StartTimer(FORWARDINGTIMER, 30 * SECOND);
  else if (reason != -1)
    StartTimer(PARTYDISCONNECTTOUT, 20*SECOND);

  //add for CG_SoftMCU
	if ((reason == -1) && IsCallGeneratorConf())// we set call generator values only if it is not rejected call.
	{
	  pCallAnswerReq->callGeneratorParams.bIsCallGenerator=1;
	  pCallAnswerReq->callGeneratorParams.eEndpointModel=endpointModelHDX9000;
	}

  int isPartyNumber = 0;
  isPartyNumber = SetDstPartyAddressForAnswer(*pCallAnswerReq);
  pCallAnswerReq->localTerminalType = GetMasterSlaveTerminalType();
  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)(pCallAnswerReq),lengthStructure);
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_CALL_ANSWER_REQ,pMsg,m_serviceId,
    m_serviceId,m_pDestUnitId,m_callIndex,0,0,m_pmcCall->GetCallStatus());
  POBJDELETE(pMsg);

  PDELETEA (pCallAnswerReq);
}

////////////////////////////////////////////////////////////////////////////
std::string CH323Cntl::GetCloudIp() const
{
	std::string strCloudIp="";

	if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		  FILE *pCloudIpFile = fopen((MCU_TMP_DIR+"/cloudIp").c_str(), "r" );
		  if (pCloudIpFile)
		  {
			  char * line = NULL;
	          size_t len = 0;
	          ssize_t read;
	          read = getline(&line, &len, pCloudIpFile );
	          if (read != -1)
	        	  strCloudIp = line;

	          TRACESTR(eLevelInfoNormal) << "\nCH323Cntl::GetCloudIp() - strCloudIp = "<<strCloudIp;

              if (line)
	              free(line);

              fclose(pCloudIpFile);
		  }
		  else
			  PTRACE (eLevelInfoNormal, "No cloud IP address");
	}

	return strCloudIp;

}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323CallNewRateInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323CallNewRateInd - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());
  mcIndCallNewRate callNewRate;
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if(m_pmcCall->GetIsOrigin())// dial out
  {// in dial in we recive the useruser at the offering ind, in dial out at the call connected ind.
    if(!m_pmcCall->GetCallIndex())
    {
      m_callIndex = callIndex;
      m_pmcCall->SetCallIndex(m_callIndex);
    }
  }

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323CallNewRateInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323CallNewRateInd - srcUnitId incorrect");
    return;
  }
  DWORD  structLen = sizeof(mcIndCallNewRate);
  memset(&callNewRate,0,structLen);
  pParam->Get((BYTE*)(&callNewRate),structLen);

//  m_pmcCall->SetRate(callNewRate.rate);
}

////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::ChangeCapsAndCreateControl(DWORD vidRate)
{
  m_pLocalCapH323->SetVideoBitRate(vidRate);
  OnPartyCreateControl();
}

////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnCapabilitiesTout(CSegment* pParam)
{
  OnPartyCreateControl(FALSE, TRUE);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyCreateControl(BYTE bIsReCap, BYTE capTout)
{
  if( m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnPartyCreateControl bIsClosing process - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }
    if (m_remoteIdent == PolycomQDX)
    {
        if (m_pRmtCapH323->IsCapableOfHD720())
        {

            PTRACE (eLevelInfoNormal, "CH323Cntl::OnPartyCreateControl - remote is QDX which support HD 720 - remove H263 4cif to force connect with H264");
            m_pLocalCapH323->SetH263FormatMpi (k4Cif, -1, kRolePeople);
        }
        else
            PTRACE (eLevelInfoNormal, "CH323Cntl::OnPartyCreateControl - remote is QDX which does not support HD 720");

    }

    else if (m_pLocalCapH323->IsCapableOfHD720() &&
        m_remoteIdent == TandbergEp &&
        !bIsReCap &&
        m_pTargetModeH323->GetConfType() == kCp)
  {
        BYTE isH263Preffered = FALSE;
        BYTE bCheckProfile = FALSE;
        if (m_pLocalCapH323->IsSupportH264HighProfile() == FALSE)
            bCheckProfile = TRUE;
        const BaseCapStruct* pRemoteBestStruct = m_pRmtCapH323->GetBestCapStruct(eH264CapCode, cmCapReceive, kUnknownFormat, kRolePeople, bCheckProfile);

        if (pRemoteBestStruct)
        {
            CBaseVideoCap* pRemoteBestCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(eH264CapCode,(BYTE *)pRemoteBestStruct);
            if (pRemoteBestCap)
            {
                COstrStream msg;
                msg << "CH323Cntl::OnPartyCreateControl Best remote H264 video cap \n";
                pRemoteBestCap->Dump(msg);
                PTRACE(eLevelInfoNormal, msg.str().c_str());
                isH263Preffered = checkIsh263preffered(pRemoteBestCap, TRUE);

                long fs = ((CH264VideoCap*)pRemoteBestCap)->GetFs();
                long mbps = ((CH264VideoCap*)pRemoteBestCap)->GetMbps();
                APIU8 level = ((CH264VideoCap*)pRemoteBestCap)->GetLevel();
                POBJDELETE(pRemoteBestCap);
                if (mbps == -1)
                {
                  CH264Details thisH264Details = level;
                  mbps = thisH264Details.GetDefaultMbpsAsProduct();
                }
                else
                  mbps *= CUSTOM_MAX_MBPS_FACTOR;
                PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCreateControl -fs rec : ",fs);
                if (fs == -1)
                {
                  CH264Details thisH264Details = level;
                  fs = thisH264Details.GetDefaultFsAsProduct();
                  if(0 == fs)
                    PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyCreateControl, level value is:", level);
                }
                else
                  fs *= CUSTOM_MAX_FS_FACTOR;


                int fps;
                PASSERTMSG(fs == 0, "FrameSize equals to 0 - this is unexpected");

                if(fs)
                  fps = (int)(mbps/fs);
                else
                  fps = (int)(mbps/1);

                CapEnum algorithm = (CapEnum)(m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit));
            //    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCreateControl -fs : ",fs);
            //    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCreateControl -mbps : ",mbps);
                PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCreateControl -fps : ",fps);
            //    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCreateControl -alg : ",algorithm);
                if(( fps <= 10 ) && algorithm == eH264CapCode)
                {
                  APIU16 profile = 0;
                  APIU8 level=0;
                  long Targetmbps=0, Targetfs=0, dpb=0, brAndCpb=0, sar=0, staticMB=0;
                  m_pTargetModeH323->GetH264Scm(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);

                  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
                  eVideoQuality scmVideoQuality = (pCommConf) ? pCommConf->GetVideoQuality() : eVideoQualityAuto;

                  if(Targetfs >= H264_HD720_FS_AS_DEVISION && scmVideoQuality == eVideoQualityMotion)
                  {

                    PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCreateControl -TB ep and fps is less than 10 -sending only SD -changing transmit only!");
                  //  Targetfs = H264_WCIF60_FS_AS_DEVISION;
                    //Targetmbps =H264_W4CIF_30_MBPS_AS_DEVISION;
                  //  m_pTargetModeH323->SetH264Scm(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);
                    APIS8 cif4 = m_pLocalCapH323->Get4CifMpi();
                    SetH264ModeInLocalCapsAndScmAccordingToVideoPartyType(eCP_H264_upto_SD30_video_party_type,cif4);
                    //m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323); -noa just for test

                  }

                }

            }
            else
                isH263Preffered = TRUE;
        }
        else
           isH263Preffered = TRUE;

        PTRACE2INT (eLevelInfoNormal, "CH323Cntl::OnPartyCreateControl. TB endpoint - h263 preffered according to Caps - ", isH263Preffered);
		std::string sProducts;
		CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey("FORCE_STATIC_MB_ENCODING", sProducts);

		if(sProducts.size() > 0&&(strcmp("NONE", sProducts.c_str()) == 0))
		{
			SetH264ModeInLocalCapsAndScmAccordingToVideoPartyType(eCP_H264_upto_SD30_video_party_type,m_pLocalCapH323->Get4CifMpi());
			PTRACE (eLevelInfoNormal, "CH323Cntl::OnPartyCreateControl. Static MB isn't set.  TB endpoint can't be 720p ");
		}

        m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
  }

  // LPR - Setting remote LPR caps in m_pMcCall
  if (m_pLocalCapH323->IsLPR() == TRUE)
  {
    CLprCap* pLprLocalCap = NULL;
    lprCapCallStruct lprLocalCapStruct;
    memset(&lprLocalCapStruct,0, sizeof(lprCapCallStruct));
    pLprLocalCap = m_pLocalCapH323->GetLprCapability(eLPRCapCode);

    if(pLprLocalCap)
    {
    lprLocalCapStruct.versionID = pLprLocalCap->GetLprVersionID();
    lprLocalCapStruct.minProtectionPeriod = pLprLocalCap->GetLprMinProtectionPeriod();
    lprLocalCapStruct.maxProtectionPeriod = pLprLocalCap->GetLprMaxProtectionPeriod();
    lprLocalCapStruct.maxRecoverySet = pLprLocalCap->GetLprMaxRecoverySet();
    lprLocalCapStruct.maxRecoveryPackets = pLprLocalCap->GetLprMaxRecoveryPackets();
    lprLocalCapStruct.maxPacketSize = pLprLocalCap->GetLprMaxPacketSize();

    m_pmcCall->SetLprCapStruct(&lprLocalCapStruct, 0);
    pLprLocalCap->FreeStruct();
    POBJDELETE(pLprLocalCap);
    }
    else
      PASSERTMSG(NULL == pLprLocalCap, "GetLprCapability return NULL");

  }
//  if (bIsReCap && (m_pCurrentModeH323->IsMediaOff(cmCapAudio,cmCapTransmit) ||
//                      m_pCurrentModeH323->IsMediaOff(cmCapAudio,cmCapReceive)  ||
//                      m_pRmtCapH323->GetNumOfAudioCap() == 0))
//          InitAudioCapsAndInformRemote(FALSE, FALSE); // Re build the Audio caps
  // For debug Dial In or Out

  /////for cop only in intial caps


  int i = 0;
  if(!m_pmcCall->GetIsOrigin())//CASE OF DIALIN
    i++;
  else//CASE OF DIALOUT
    i++;

  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCreateControl - Conn Id =", m_pCsRsrcDesc->GetConnectionId());
  mcReqCreateControl *pReqCreateCntl = new mcReqCreateControl;
  memset(pReqCreateCntl, 0, sizeof(mcReqCreateControl));

  if ((m_pmcCall->GetAnswerH245Address())->ipVersion == eIpVersion4)
    pReqCreateCntl->h245IpAddress.transAddr.addr.v4.ip  = (m_pmcCall->GetAnswerH245Address())->addr.v4.ip;
  else
  {   // Case IpV6
    pReqCreateCntl->h245IpAddress.transAddr.addr.v6.scopeId = (m_pmcCall->GetAnswerH245Address())->addr.v6.scopeId;
    APIU32 i = 0;
    for (i = 0; i < 16; i++)
      pReqCreateCntl->h245IpAddress.transAddr.addr.v6.ip[i] = (m_pmcCall->GetAnswerH245Address())->addr.v6.ip[i];

  }
  // set the XML fields
  pReqCreateCntl->h245IpAddress.unionProps.unionType = (m_pmcCall->GetAnswerH245Address())->ipVersion;
  pReqCreateCntl->h245IpAddress.unionProps.unionSize = sizeof(ipAddressIf);

  pReqCreateCntl->h245IpAddress.transAddr.port  = (m_pmcCall->GetAnswerH245Address())->port;
  pReqCreateCntl->h245IpAddress.transAddr.distribution = (m_pmcCall->GetAnswerH245Address())->distribution;
  pReqCreateCntl->h245IpAddress.transAddr.ipVersion = (m_pmcCall->GetAnswerH245Address())->ipVersion;
  pReqCreateCntl->h245IpAddress.transAddr.transportType = (m_pmcCall->GetAnswerH245Address())->transportType;
  // IpV6
  ipAddressIf controlIp;
  mcTransportAddress h245cntlAddr;
  memset(&h245cntlAddr,0,sizeof(mcTransportAddress));
  memset(&controlIp,0,sizeof(ipAddressIf));
  WORD  controlPort = (m_pmcCall->GetAnswerH245Address())->port;

  if ((m_pmcCall->GetAnswerH245Address())->ipVersion == eIpVersion4)
  {
    if (m_pmcCall->GetIsOrigin())
    {
      if ((m_pmcCall->GetSetupH245Address())->addr.v4.ip)
        controlIp.v4.ip = 0;
      else
        controlIp.v4.ip = (m_pmcCall->GetAnswerH245Address())->addr.v4.ip;
    }
    else
    {
      if ((m_pmcCall->GetSetupH245Address())->port)
        controlIp.v4.ip = (m_pmcCall->GetSetupH245Address())->addr.v4.ip;
      else
        controlIp.v4.ip = 0;
    }
  }
  else
  {
    // IpV6
    if (m_pmcCall->GetIsOrigin())
    {
      memcpy(&h245cntlAddr,m_pmcCall->GetSetupH245Address(),sizeof(mcTransportAddress));
      if (!::isApiTaNull(&h245cntlAddr))
        memset(&controlIp,0,sizeof(ipAddressIf));
      else
      {
        memcpy(&h245cntlAddr,m_pmcCall->GetAnswerH245Address(),sizeof(mcTransportAddress));
        memcpy(&controlIp,&h245cntlAddr.addr,sizeof(ipAddressIf));
      }
    }
    else
    {
      if ((m_pmcCall->GetSetupH245Address())->port)
      {
        memcpy(&h245cntlAddr,m_pmcCall->GetSetupH245Address(),sizeof(mcTransportAddress));
        memcpy(&controlIp,&h245cntlAddr.addr,sizeof(ipAddressIf));
      }
      else
      {
        memset (&controlIp,0,sizeof(ipAddressIf));
      }
    }
  }
    m_pmcCall->SetControlH245AddressPort(controlPort);

    if(!m_pmcCall->GetIsOrigin())//CASE OF DIALIN
    memset(&(pReqCreateCntl->h245IpAddress.transAddr.addr),0,sizeof(ipAddressIf));
    else//CASE OF DIALOUT
    memcpy(&(pReqCreateCntl->h245IpAddress.transAddr.addr),&controlIp,sizeof(ipAddressIf));

    pReqCreateCntl->h245IpAddress.transAddr.port  = controlPort;

  pReqCreateCntl->capabilitiesStructLength = m_pLocalCapH323->GetCapTotalLength();

  if(m_pParty->GetCascadeMode() == SLAVE || IsSlaveCascadeModeForH239() || m_pParty->IsGateway())
    pReqCreateCntl->masterSlaveTerminalType  = SLAVE_NUMBER_DEFAULT;
  else if (m_pParty->GetCascadeMode() == MASTER)
    pReqCreateCntl->masterSlaveTerminalType  = ACTIVE_MC_MASTER_NUMBER;
    else
        pReqCreateCntl->masterSlaveTerminalType  = DEFAULT_MASTER_NUMBER;

  APIS32 length = m_pLocalCapH323->GetCapTotalLength() + sizeof(mcReqCreateControl);

  mcReqCreateControl* ptr = (mcReqCreateControl*)new BYTE[length];
  if(ptr)
  {
  memset(ptr, 0, length);
  memcpy((mcReqCreateControl*)ptr,pReqCreateCntl,(sizeof(mcReqCreateControl)));
  }
  else
    PASSERTMSG(GetConnectionId(),"CH323Cntl::OnPartyCreateControl - ptr not valid");

  ptr->capabilities.numberOfCaps = m_pLocalCapH323->GetCapStructPtr()->numberOfCaps;
  ptr->capabilities.numberOfAlts = m_pLocalCapH323->GetCapStructPtr()->numberOfAlts;
  ptr->capabilities.numberOfSim  = m_pLocalCapH323->GetCapStructPtr()->numberOfSim;

  ptr->capabilities.xmlDynamicProps.numberOfDynamicParts  = ptr->capabilities.numberOfCaps;
  ptr->capabilities.xmlDynamicProps.sizeOfAllDynamicParts = ptr->capabilitiesStructLength;

  BYTE* dest_caps   = (BYTE*)(&((mcReqCreateControl*)ptr)->capabilities.caps);
  BYTE* source_caps = (BYTE*)(&m_pLocalCapH323->GetCapStructPtr()->caps);
  memcpy(dest_caps, source_caps, m_pLocalCapH323->GetCapTotalLength());
  memcpy(&((mcReqCreateControl*)ptr)->capabilities.altMatrix, &m_pLocalCapH323->GetCapStructPtr()->altMatrix, sizeof(cap_fd_set));
  PDELETE (pReqCreateCntl);

  DWORD opcode = bIsReCap ? H323_CS_SIG_RE_CAPABILITIES_REQ : H323_CS_SIG_CREATE_CNTL_REQ;
  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)(ptr), length);
  m_pCsInterface->SendMsgToCS(opcode, pMsg, m_serviceId, m_serviceId, m_pDestUnitId, m_callIndex, 0, 0, 0);

  POBJDELETE(pMsg);
  PDELETEA (ptr);

  if (bIsReCap == FALSE && m_pTargetModeH323->GetConfType() == kCop && IsNeedToSendTrasmitCap())
  {
    PTRACE2INT (eLevelInfoNormal, "CH323Cntl::OnPartyCreateControl.remove transmit caps ",m_pTargetModeH323->GetConfType());
    RemoveTransmitCaps();
    m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::HandleCapIndication(BYTE bPrevCapsAreFull,BYTE bPrevCapsHaveAudio, BYTE bAreAllChannelsConnected)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::HandleCapIndication - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    CloseUnSupportedChannels();

    if (m_pRmtCapH323->IsECS()) //We received ECS
    {
      OFF(m_isH245Connected);
      if (IsValidTimer(MCMSOPENCHANNELS))
          DeleteTimer(MCMSOPENCHANNELS);

        if (!(m_CapabilityNegotiation & kRemoteCapsRecieved)) // ECS can't come as first remote caps in a call
        {
            PTRACE2INT(eLevelInfoNormal,"CH323Cntl::HandleCapIndication -  remote sent empty capabilities as first TCS - m_CapabilityNegotiation = ", m_CapabilityNegotiation);
      DisconnectCallBecauseBadCaps();
      return;
        }
        else
        { BOOL bIsH460 = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_H460);
            if (IsValidTimer(AUDCONNECTTOUT))
                DeleteTimer(AUDCONNECTTOUT);

            m_pTaskApi->SendECSToPartyControl();
      m_CapabilityNegotiation = kInitialCapNegotiation;
        }
        m_pParty->CancelIVR();
        //PTRACE2INT(eLevelInfoNormal,"CH323Cntl::HandleCapIndication -VVX SEND ECS this means hold so we mute in video also",m_remoteIdent);
        if( m_remoteIdent == PolycomVVX && FindChannelInList(cmCapVideo, FALSE, kRolePeople) )
         {
               m_pTaskApi->MuteVideo(eOn);
               PTRACE(eLevelInfoNormal,"CH323Cntl::HandleCapIndication -VVX SEND ECS this means hold so we mute in video also");
          }
    }
    else
    {
      BYTE isTryOpenChannelFromMcms = TRUE;
        // Handle no Audio caps
        if (m_pRmtCapH323->GetNumOfAudioCap() == 0)
        {
            //InitAudioCapsAndInformRemote(TRUE, TRUE); // Don't inform the remote, just init the Audio
            m_pParty->CancelIVR();
        }

        // Open the Audio if it's closed and the new caps have Audio
        else if  (m_pCurrentModeH323->IsMediaOff(cmCapAudio, cmCapTransmit, kRolePeople))
        {
            PTRACE2(eLevelInfoNormal,"CH323Cntl::HandleCapIndication : Open outgoing Audio channel. Name - ",PARTYNAME);
            OpenOutgoingChannel(cmCapAudio);
            DWORD index = GetChannelIndexInList(true, cmCapAudio,TRUE, kRolePeople);
            if(index < m_maxCallChannel && m_state == SETUP)
            {
                PTRACE2INT(eLevelError,"CCH323Cntl::HandleCapIndication - audio channel is already open -this is recap in the middle of setup do not try to reopen - ",m_pmcCall->GetConnectionId());
                isTryOpenChannelFromMcms = FALSE;
            }
        }
        if (m_pParty->IsRemoteCapsEcs()) //We received TCS after ECS
            OnPartyCreateControl(FALSE);
        if ( IsValidTimer(CAPABILITIESTOUT) )
            DeleteTimer(CAPABILITIESTOUT);
        if( m_remoteIdent == TandbergEp && !(m_CapabilityNegotiation & kRemoteCapsRecieved)  &&  m_state == SETUP)
            OnPartyCreateControl(FALSE);
        if (m_state != SETUP)
        {
          //27.12.2006 Changes by VK. Stress Test
          if (STRESS_TEST_SIMULATION == TRUE)
          {
            PTRACE(eLevelInfoNormal,"Stress Test CH323Cntl::HandleCapIndication - Handle new remote capabilities.");
            if (!IsIncomingVideoExist())
            {
              PTRACE(eLevelInfoNormal,"Stress Test CH323Cntl::HandleCapIndication - The incoming video channel is not connected.");
              m_pTaskApi->SendReCapsToPartyControl(*m_pRmtCapH323);
            }
          }
          else
          {
          //end of changes for Stress Test
            if(m_remoteIdent == PolycomVVX && !m_bPrevCapsAreFull && FindChannelInList(cmCapVideo, FALSE, kRolePeople))
            {
                   m_pTaskApi->MuteVideo(eOff);
                   PTRACE(eLevelInfoNormal,"CH323Cntl::HandleCapIndication -VVX SEND CAPS AFTER ECS -meannig resume hold undo mute for in video");
            }
              m_pTaskApi->SendReCapsToPartyControl(*m_pRmtCapH323);
          }
        }
        m_CapabilityNegotiation |= kRemoteCapsRecieved;

        //COP HERE TO EXTRACT TX of remote caps - noa only in first tcs

        if( m_state == SETUP && m_pTargetModeH323->GetConfType() == kCop && IsNeedToCreateRemoteTxMode())
        {
          if(m_pCopRemoteVideoModes)
          {
            PTRACE(eLevelInfoNormal,"CH323Cntl::HandleCapIndication - creating remote tx not for the first time!!!");
            DBGPASSERT(110);

          }
          POBJDELETE(m_pCopRemoteVideoModes);
          m_pCopRemoteVideoModes = new CCopVideoTxModes();
		  if(m_pCopRemoteVideoModes)
		  {
          m_pRmtCapH323->TransferRemoteCapsToRemoteTxModeAndRemove(m_pCopRemoteVideoModes);
          if(( strstr(m_remoteVendor.productId, "RMX1000") || strstr(m_remoteVendor.productId, "RMX500")) && !strstr(m_remoteVendor.productId, "ACCORD MGC / Polycom RMX1000_2000") && m_pCopRemoteVideoModes )
            AdjustRMX1000levelsToRMX2000levels();
              m_pCopRemoteVideoModes->Dump("CH323Cntl::HandleCapIndication this is remote tx mode for cascade: ", eLevelInfoNormal);
		  }
		  else
		      PASSERTMSG(1, "CH323Cntl::HandleCapIndication - new CCopVideoTxModes failed");

        }
        if(m_onGatewayCall)
            m_pTaskApi->SendRemoteCapabilities( *m_pRmtCapH323 );  //it's a VS/SVS Gateway call pass the capabilities to the party
        if(isTryOpenChannelFromMcms)
          OpenChannelFromMcmsIfNeeded();
        m_pParty->ResetEcs();
    }
    if (m_remoteIdent == PolycomQDX)
        OnPartyCreateControl(FALSE);
    if(m_CapabilityNegotiation == kCompleteCapNegotiation)
    // kCompleteCapNegotiation means that remote capability arrives and
    //cap response indication arrives (remote cap can arrives more than once)
  {
    if(IsValidTimer(PARTYCONNECTING))
      DeleteTimer(PARTYCONNECTING);
    if (m_pRmtCapH323->GetNumOfAudioCap())
        {
            if (!IsValidTimer(AUDCONNECTTOUT))
                StartTimer(AUDCONNECTTOUT,30*SECOND);
        }
  }



}

////////////////////////////////////////////////////////////////////////////
//Check is there a need to open outgoing video channel first
void CH323Cntl::CheckOpenOutgoingVideoChannel()
{
  //check if remote contains video capabilities and Avaya and flag SIP CM is on
  if (m_pRmtCapH323->GetNumOfVideoCap() != 0 && m_bIsAvaya && m_remoteVendor.isAvayaSipCm == AVAYA_SIP_CM_FLAG_ON)
  {
    //check if target mode contains video
    if(m_pTargetModeH323->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople))
    {
            PTRACE (eLevelInfoNormal, "CH323Cntl::CheckOpenOutgoingVideoChannel - Avaya SIP_CM ON - Open Channel");

      CCapSetInfo capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit, kRolePeople);
      //in this function there is check if outgoing video channel exist
      OnPartyOutgoingChannelReq(capInfo.GetIpCapCode(), kRolePeople, FALSE, TRUE);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323CapIndication(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323CapIndication - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  mcIndCapabilities *ptr;
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;
  CConfParty* pConfParty = m_pParty? m_pParty -> GetConfPartyNonConst() : NULL;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

//  if (callIndex != m_callIndex)
//  {
//    PASSERTMSG(callIndex,"CH323Cntl::OnH323CapIndication - Call Index incorrect");
//    return;
//  }
//  if (srcUnitId != m_pDestUnitId)
//  {
//    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323CapIndication - srcUnitId incorrect");
//    return;
//  }

  m_bPrevCapsAreFull = !m_pRmtCapH323->IsECS();

  if (status != 0)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323CapIndication -  remote sent bad capabilities");
    m_pmcCall->SetIsClosingProcess(TRUE);
    DisconnectCallBecauseBadCaps();
    return;
  }
  else if (m_state != SETUP && m_bPrevCapsAreFull && pConfParty && pConfParty -> GetTokenRecapCollisionDetection() == etrcdTokenHandlingInProgress)
  {
	//===============================================================
	// Token is being granted/released at the moment, pending recap
	//===============================================================
	if(m_pParty)m_pParty -> PendRecapOnToken(pParam);
	return;
  }
  else if (m_state != SETUP && m_bPrevCapsAreFull && pConfParty)
  {
	  pConfParty -> SetTokenRecapCollisionDetection(etrcdRecapInProgress);
  }

  ptr = (mcIndCapabilities*)(pParam->GetPtr(1));
  int size = sizeof(mcIndCapabilitiesBase) + ptr->capabilitiesSize;

  mcIndCapabilities* pRemoteCapInd = (mcIndCapabilities*)new BYTE[size];
  memcpy(pRemoteCapInd, (mcIndCapabilities*)(pParam->GetPtr(1)), size);

  m_bPrevCapsHaveAudio = (m_pRmtCapH323->GetNumOfAudioCap() != 0); //audio shuffle

  BYTE bAreAllChannelsConnected = AreAllChannelsConnected();

  m_pRmtCapH323->SetEncryptionAlg(m_encAlg);

  m_pRmtCapH323->Create( &(pRemoteCapInd->capabilities),size);

  // LPR - Setting remote LPR caps in m_pMcCall
  if (m_pRmtCapH323->IsLPR() == TRUE)
  {
    CLprCap* pLprRmtCap = NULL;
    lprCapCallStruct lprRmtCapStruct;
    memset(&lprRmtCapStruct,0, sizeof(lprCapCallStruct));
    pLprRmtCap = m_pRmtCapH323->GetLprCapability(eLPRCapCode);

    if(pLprRmtCap)
    {

    lprRmtCapStruct.versionID = pLprRmtCap->GetLprVersionID();
    lprRmtCapStruct.minProtectionPeriod = pLprRmtCap->GetLprMinProtectionPeriod();
    lprRmtCapStruct.maxProtectionPeriod = pLprRmtCap->GetLprMaxProtectionPeriod();
    lprRmtCapStruct.maxRecoverySet = pLprRmtCap->GetLprMaxRecoverySet();
    lprRmtCapStruct.maxRecoveryPackets = pLprRmtCap->GetLprMaxRecoveryPackets();
    lprRmtCapStruct.maxPacketSize = pLprRmtCap->GetLprMaxPacketSize();

    m_pmcCall->SetLprCapStruct(&lprRmtCapStruct, 1);
    pLprRmtCap->FreeStruct();
    POBJDELETE(pLprRmtCap);
    }
    else
      PASSERTMSG(1, "GetLprCapability return NULL");

    if( m_pTargetModeH323->GetIsLpr())
    {
      if(!m_ChannelsWithLprPayload)
      {
        m_ChannelsWithLprPayload = TRUE;
        m_pTaskApi->UpdateChannelLprHeaderDB(YES);
        //update db
      }
    }

  }

  HandleCapIndication(m_bPrevCapsAreFull,m_bPrevCapsHaveAudio, bAreAllChannelsConnected);
  PDELETEA(pRemoteCapInd);
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_CAPABILITIES_RES_REQ, NULL, m_serviceId,m_serviceId,m_pDestUnitId,m_pmcCall->GetCallIndex(),0,0,0);

  return;
}
////////////////////////////////////////////////////////////////////////////

void CH323Cntl::CloseUnSupportedChannels ()
{
    CMedString str;
    str << "CH323Cntl::CloseUnSupportedChannels: ";
  BYTE isCloseMedia = FALSE;

    // Video Channel
    if (!m_pRmtCapH323->GetNumOfVideoCap()  &&
        (m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapTransmit ,kRolePeople) || FindChannelInList(cmCapVideo,TRUE)))
    {
        str << "Close Video, " << m_pmcCall->GetConnectionId() << " ";
        CloseOutgoingChannel(cmCapVideo);
        isCloseMedia = TRUE;
    }

    // Audio Channel
    if (!m_pRmtCapH323->GetNumOfAudioCap()  &&
        (m_pCurrentModeH323->IsMediaOn(cmCapAudio, cmCapTransmit ,kRolePeople) || FindChannelInList(cmCapAudio,TRUE)))
    {
        str << "Close Audio, " << m_pmcCall->GetConnectionId() << " ";
        CloseOutgoingChannel(cmCapAudio);
        isCloseMedia = TRUE;
        //OFF(m_OneOfTheMediaChannelWasConnected);
    }

    // Presentation Channel
     if (!(m_pRmtCapH323->IsH239() || m_pRmtCapH323->IsEPC())  &&
         (m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapTransmit ,kRoleContentOrPresentation) || FindChannelInList(cmCapVideo,TRUE, kRoleContentOrPresentation)))
    {
      str << "Close Presentation, " << m_pmcCall->GetConnectionId() << " ";
        CloseOutgoingChannel(cmCapVideo, kRoleContentOrPresentation);
        isCloseMedia = TRUE;
    }

    // Data Channel
     if (!m_pRmtCapH323->GetNumOfDataCap()  &&
         (m_pCurrentModeH323->IsMediaOn(cmCapData, cmCapTransmit ,kRolePeople) || FindChannelInList(cmCapData,TRUE)))
    {
      str << "Close Data, " << m_pmcCall->GetConnectionId() << " ";
        CloseOutgoingChannel(cmCapData);
        isCloseMedia = TRUE;
    }

     if(isCloseMedia)
     {
          m_pCurrentModeH323->Dump(str.GetString(), eLevelInfoNormal);
     }
}
////////////////////////////////////////////////////////////////////////////
void CH323Cntl::InitAudioCapsAndInformRemote(BYTE bIsRecap, BYTE informRemote)
{
     // Send recap with all the Audio to the remote so that it will be able to re-open in case it changes
    CSmallString str;
    str << "CH323Cntl::InitAudioCapsAndInformRemote Conn ID - " << m_pCsRsrcDesc->GetConnectionId();
    WORD audioRate = CAudModeH323::GetBitRateFromAudioMode(m_pParty->GetAudioRate());
    DWORD videoRate = m_pParty->GetVideoRate();
    DWORD confRate = (DWORD)(m_pmcCall->GetBandwidth() / 2);
    str << "audioRate=" << audioRate << " videoRate=" << videoRate << " confRate=" << confRate;
    PTRACE (eLevelInfoNormal, str.GetString());
    m_pLocalCapH323->ReArrangeAudioCaps(audioRate, confRate, videoRate);
    if (informRemote)
        OnPartyCreateControl(bIsRecap);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323FacilityIndSetup(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323FacilityIndSetup - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
}

////////////////////////////////////////////////////////////////////////////
// Action function for facility event received from CS
void  CH323Cntl::OnH323FacilityIndConnect(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323FacilityIndConnect - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  if (!m_bIsAvaya)
    return;
  //common parameters for all messages
  APIU32 callIndex    = 0;
  APIU32 channelIndex   = 0;
  APIU32 mcChannelIndex   = 0;
  APIU32 stat1      = 0;
  APIS32 status       = 0;    //historical value and not used
  APIU16 srcUnitId    = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;
  status = (APIS32)stat1;

  //make a local copy of received buffer
  mcIndFacility* ptr = (mcIndFacility*)(pParam->GetPtr(1));
  int size = sizeof(mcIndFacility);
  mcIndFacility* pFacilityInd = (mcIndFacility*)new BYTE[size];
  memcpy(pFacilityInd, ptr, size);

  if ((pFacilityInd->avfFeVndIdInd).fsId != H460_K_FsId_Avaya)
  {
    PTRACE2INT(eLevelError, "CH323Cntl::OnH323FacilityIndConnect - Fs Id isn't Avaya. Connection Id = ", m_pCsRsrcDesc->GetConnectionId());
    DBGPASSERT(pFacilityInd->avfFeVndIdInd.fsId);
  }
  StoreRemoteVendorInfo(pFacilityInd->avfFeVndIdInd);
  PDELETEA(pFacilityInd);
  // update local capabilities according to new remote vendor information
  //ChangeCapabilitySetAccordingToVendorId();
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::DisconnectCallBecauseBadCaps()
{
  m_pmcCall->SetIsClosingProcess(TRUE);
  m_pmcCall->SetCallCloseInitiator(McInitiator);
  if(IsValidTimer(PARTYCONNECTING))
    DeleteTimer(PARTYCONNECTING);
  if(IsValidTimer(AUDCONNECTTOUT))
    DeleteTimer(AUDCONNECTTOUT);
  if(IsValidTimer(CODIANVIDCHANTOUT))
    DeleteTimer(CODIANVIDCHANTOUT);

  m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_BAD_REMOTE_CAP);
  PASSERTMSG(m_pmcCall->GetConnectionId(),"CH323Cntl::DisconnectCallBecauseBadCaps" );
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323CapResponseInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323CapResponseInd - ", m_pmcCall->GetConnectionId());
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323CapResponseInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323CapResponseInd - srcUnitId incorrect");
    return;
  }

  if (status != 0)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323CapResponseInd - Caps not accepted by remote");
    m_pmcCall->SetIsClosingProcess(TRUE);
    m_pmcCall->SetCallCloseInitiator(McInitiator);
    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_CAPS_NOT_ACCPTED_BY_REMOTE);
      PASSERT(status);
    return;
  }
  // After remote get our local caps, we wait 40 seconds for video and
  // audio channels to be opened (incomming & outgoing).
  // If during this time audio channels are not opened the party
  // will be desconnected!!!
  // If during this time only audio channels are opened, party should be
  // 'SECONDARY'.
  // If audio and video chennels are opened. party will be connected (we send
  // 'CONNECT' message to the conference).

  m_CapabilityNegotiation |= kLocalCapsAcknowledged;

  if(m_CapabilityNegotiation == kCompleteCapNegotiation)
    // kCompleteCapNegotiation means that remote capability arrives and cap response indication arrives (remote cap can arrives mpre than once)
  {
    if(IsValidTimer(PARTYCONNECTING))
      DeleteTimer(PARTYCONNECTING);
    if (! m_pParty->IsPartyInChangeVideoMode()) //in highest common we have a timer in the party
      StartTimer(AUDCONNECTTOUT,30*SECOND);
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323ConfConnectedInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ConfConnectedInd - ",m_pmcCall->GetConnectionId());
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323CallCntlInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323CallCntlInd - ",m_pmcCall->GetConnectionId());

//  m_pLoadMngrConnector->H245ConnectExitCriticalSection();

  EIpChannelType channelType=H245;
  DWORD actualRate=0xFFFFFFFF;
  BOOL  bIsGoodParams = TRUE;
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;
  // IpV6 - Monitoring
  mcTransportAddress partyAddr;
  mcTransportAddress mcuAddr;

  memset(&partyAddr,0,sizeof(mcTransportAddress));
  memset(&mcuAddr,0,sizeof(mcTransportAddress));

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323CallCntlInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323CallCntlInd - srcUnitId incorrect");
    return;
  }

  ON(m_isH245Connected);

  mcIndCallControlConnected callCntlCncted;

  DWORD  structLen = sizeof(mcIndCallControlConnected);
  memset(&callCntlCncted,0,structLen);
  pParam->Get((BYTE*)(&callCntlCncted),structLen);

    m_pmcCall->SetMasterSlaveStatus((int)callCntlCncted.masterSlaveStatus);

  // IpV6 - Monitoring

  memcpy(&mcuAddr, &(callCntlCncted.localH245Address.transAddr), sizeof(mcTransportAddress));
  memcpy(&partyAddr, &(callCntlCncted.remoteH245Address.transAddr), sizeof(mcTransportAddress));


  //We should update the chair cntl only if we in the cascade mode (if the remote is \mcu we in cascade mode).
  if((m_pParty->GetCascadeMode() != TERMINAL) && (m_pParty->GetCascadeMode() != AUTO))
    bIsGoodParams = CheckCascadeParams();

  if(bIsGoodParams)
  {
    if (m_remoteIdent != PolycomMGC)
    SendRemoteNumbering();

    CPrtMontrBaseParams *pPrtMonitrParams = new CPrtMontrBaseParams;
    // SetPartyMonitoringBaseParam is according to the operator CapEnum
    CCapSetInfo capInfo;
    SetPartyMonitorBaseParams(pPrtMonitrParams,channelType,actualRate,&partyAddr,&mcuAddr,(DWORD)capInfo.GetIpCapCode());
    LogicalChannelConnect(pPrtMonitrParams,(DWORD)channelType);
    POBJDELETE(pPrtMonitrParams);
  }
  else
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323CallCntlInd -  reject call - master slave problem");
    m_pmcCall->SetIsClosingProcess(TRUE);
    m_pmcCall->SetCallCloseInitiator(McInitiator);
    if(IsValidTimer(PARTYCONNECTING))
      DeleteTimer(PARTYCONNECTING);
    if(IsValidTimer(AUDCONNECTTOUT))
      DeleteTimer(AUDCONNECTTOUT);
    if(IsValidTimer(CODIANVIDCHANTOUT))
      DeleteTimer(CODIANVIDCHANTOUT);
    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_MASTER_SLAVE_PROBLEM);
    PASSERT(GetConnectionId());
        return;
  }
  OpenChannelFromMcmsIfNeeded();
    CheckOpenOutgoingVideoChannel();

//26.12.2006 Changes by VK. Stress Test
  if (STRESS_TEST_SIMULATION == TRUE)
    StartStressTestTimerOnce();
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyOutgoingChannelReq(CapEnum h323CapCode,ERoleLabel eRole,BYTE isMCMSOpenChannel, BYTE bIsNeedToImprove, BYTE isNeedToIntersec,BYTE isopen4cif)
{
    if (m_pmcCall->GetIsClosingProcess() == TRUE)
    {
        PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - Closing Process",m_pmcCall->GetConnectionId());
        return;
    }

  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - ",m_pmcCall->GetConnectionId());

  int  totalSize = 0;
  CCapSetInfo capInfoTemp(h323CapCode);
  cmCapDataType eType = capInfoTemp.GetCapType();
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  PASSERT_AND_RETURN(NULL == pCommConf);
  CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());

  if( !m_pTargetModeH323->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation) && (eType == cmCapVideo) && (eRole & kRoleContentOrPresentation) )
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - Don't open outgoing content - not supported in target mode",m_pmcCall->GetConnectionId());
    return;

  }


  if(eType==cmCapData)
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq, <cmCapData> = ",eType);

  m_bVideoOutRejected = FALSE;
  // VNGFE-787
  // In case we are opening our outgoing channel in Codian case (No incoming channel)
  // We would like to open a correct channel - Meaning - Intersec the correct video and not our own
  if (isNeedToIntersec == TRUE && m_isCodianVcr && eType == cmCapVideo && eRole == kRolePeople)
  {
    CBaseVideoCap* pIntersectCap = NULL;
    pIntersectCap = m_pLocalCapH323->FindIntersectionBetweenTwoCaps(m_pRmtCapH323, h323CapCode, cmCapReceiveAndTransmit);
    if(pIntersectCap)
    {
    m_pTargetModeH323->SetMediaMode(pIntersectCap->GetCapCode(), pIntersectCap->SizeOf(), (BYTE*)pIntersectCap->GetStruct(), cmCapVideo, cmCapTransmit, kRolePeople, true);
        POBJDELETE (pIntersectCap);
  }
    else
      PASSERTMSG(NULL == pIntersectCap, "FindIntersectionBetweenTwoCaps return NULL!!!");
  }
  //If we need to open capability that the remote does not support I shouldn't continue with the opening.
  //For example: The remote cap has only 261 and the local has 261 and 263. the remote open 263, we can't open
  //263 either because the remote does not know how to receive such capability.

  BYTE bRes = TRUE;
  if (eType == cmCapVideo)
  {
    if ((m_pTargetModeH323->GetConfType() == kCop) && (eRole == kRolePeople) && m_pTargetModeH323->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople))
    {
      m_pTargetModeH323->SetCopTxLevel(INVALID_COP_LEVEL);
      CComModeH323* pScm = new CComModeH323(*m_pTargetModeH323);
      DWORD videoRate = pConfParty ? pConfParty->GetVideoRate() : 0;
            if (m_remoteIdent == PolycomQDX)
            {
                CChannel *pSameSessionChannel = FindChannelInList(cmCapVideo, FALSE, eRole);

                if (pSameSessionChannel)
                {
                    PTRACE (eLevelInfoNormal, "CH323Cntl::OnPartyOutgoingChannelReq - QDX - open the video out with the same protocol as video in");
                    //TBD add profile for high profile
                    BYTE otherDirectionProtocol = ConvertCapEnumToReservationProtocol(pSameSessionChannel->GetCapNameEnum());
                    m_pRmtCapH323->FindBestVidTxModeForCop(m_pCopVideoModes, pScm, otherDirectionProtocol, videoRate);
                }
                else
                    m_pRmtCapH323->FindBestVidTxModeForCop(m_pCopVideoModes, pScm, (pConfParty ? pConfParty->GetVideoProtocol():0), videoRate);
            }

            else
            {
              DWORD definedMaxRate = videoRate;
              if( !m_pmcCall->GetIsOrigin()  &&  m_pH323NetSetup->GetRemoteSetupRate() && m_pParty->GetCascadeMode() == NONE )
              {
                //PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - remote is HDX");

                WORD remoteSetupRate = (WORD)(m_pH323NetSetup->GetRemoteSetupRate() / 1000);

                DWORD AudioRateAccordingToSetupRate = (CalculateAudioRate(m_pH323NetSetup->GetRemoteSetupRate())) /1000;
          remoteSetupRate = remoteSetupRate - AudioRateAccordingToSetupRate;//from setup rate we calculate the new actual video of matching level
                PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - ,remoteSetupRate ",remoteSetupRate);
                if(definedMaxRate != 0xFFFFFFFF)
                  definedMaxRate = min(definedMaxRate,(DWORD)remoteSetupRate);
                else
                  definedMaxRate = remoteSetupRate;
              }
              if(m_pCopRemoteVideoModes && m_pCopVideoModes && IsNeedToOPenAccordingToRemoteTxModes())
                m_pRmtCapH323->FindBestVidTxModeForCopThatMatchesWithRemoteTxMode(m_pCopVideoModes,m_pCopRemoteVideoModes ,pScm,
                		(pConfParty ? pConfParty->GetVideoProtocol():0), definedMaxRate);
              else
                m_pRmtCapH323->FindBestVidTxModeForCop(m_pCopVideoModes, pScm, (pConfParty ? pConfParty->GetVideoProtocol():0), definedMaxRate);
            }
      bRes = (pScm->GetCopTxLevel() < NUMBER_OF_COP_LEVELS)? TRUE : FALSE;
      if (bRes)
      {
        m_pTargetModeH323->SetMediaMode(pScm->GetMediaMode(cmCapVideo, cmCapTransmit, kRolePeople), cmCapVideo, cmCapTransmit, kRolePeople, true);
        CChannel *pChannel = NULL;
          pChannel = m_pmcCall->FindChannelInList(cmCapAudio,FALSE);
        DWORD inaudiorate = 0;
        if(pChannel && !m_pmcCall->GetIsOrigin()  &&  m_pH323NetSetup->GetRemoteSetupRate()  && m_pParty->GetCascadeMode() == NONE/* &&  strstr(m_remoteVendor.productId,"HDX")*/ )
        {
               inaudiorate = (pChannel->GetRate())*10;
               DWORD levelvideoraterate = (pScm->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRolePeople) ) / 10;
               PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - level video rate is",levelvideoraterate);
               DWORD audioRateAccordingToLevel = CalculateAudioRateAccordingToVideoRateOfCopLevel(levelvideoraterate);
               audioRateAccordingToLevel = audioRateAccordingToLevel *10;
             //  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - remote is HDX,audio rate is",inaudiorate);
            //   PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - remote is HDX,audio according to level is",audioRateAccordingToLevel);
               if(inaudiorate > audioRateAccordingToLevel)
               {


                    DWORD newVideoBitRate = pScm->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRolePeople) - ( inaudiorate - audioRateAccordingToLevel );
                    PTRACE2INT(eLevelError,"CH323Cntl::OnPartyOutgoingChannelReq - actual rate exceeds call rate of level adjusting level using flow control, ",newVideoBitRate);
                    m_pTaskApi->UpdatePartyH323VideoBitRate(newVideoBitRate, cmCapTransmit, kRolePeople);

               }


        }
        m_pTargetModeH323->SetCopTxLevel(pScm->GetCopTxLevel());
        h323CapCode = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit, kRolePeople);
        m_pTargetModeH323->Dump("CH323Cntl::OnPartyOutgoingChannelReq after setting cop",eLevelInfoNormal);
      }
      POBJDELETE(pScm);
    }

    if (bRes)
    {
      const CMediaModeH323& rCurrentMode = m_pTargetModeH323->GetMediaMode(eType,cmCapTransmit,eRole);
      bRes = m_pRmtCapH323->IsContaining(rCurrentMode, kCapCode|kH264Profile,cmCapReceive, eRole);
      if (!bRes)
      {
        PTRACE2(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - Can't open with a protocol that isn't supported by remote capabilities  : Name - ",PARTYNAME);
    	TRACEINTO << "Check out channel error. media type - " << eType << ", role - " << eRole;
    	m_pTargetModeH323->Dump("CH323Cntl::OnPartyOutgoingChannelReq checking none cop conferences, m_pTargetModeH323->Dump:",eLevelInfoNormal);
    	m_pRmtCapH323->Dump("CH323Cntl::OnPartyOutgoingChannelReq checking none cop conferences, m_pRmtCapH323->Dump:",eLevelInfoNormal);
    	m_pLocalCapH323->Dump("CH323Cntl::OnPartyOutgoingChannelReq checking none cop conferences, m_pLocalCapH323->Dump:",eLevelInfoNormal);
      }
      /*
      else if ((eRole == kRolePeople) && m_pTargetModeH323->IsH264HighProfile(cmCapTransmit) && !m_pRmtCapH323->IsSupportH264HighProfile())
      {
        PTRACE2(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - remote doesn't support high profile : Name - ",PARTYNAME);
        bRes = FALSE;
      }
      */

    /*if (m_pLocalCapH323->IsCapableOfHD720() && m_remoteIdent == TandbergEp && m_pTargetModeH323->GetConfType() == kCp) remove Tandberg patch
    {
              eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
              BYTE isH263Preffered = FALSE;
              BYTE bIgnoreHighProfileCap = FALSE;
              if (m_pLocalCapH323->IsSupportH264HighProfile() == FALSE)
                      bIgnoreHighProfileCap = TRUE;
              const BaseCapStruct* pRemoteBestStruct = m_pRmtCapH323->GetBestCapStruct(eH264CapCode, cmCapReceive, kUnknownFormat, kRolePeople, bIgnoreHighProfileCap);

              if (pRemoteBestStruct)
              {
                CBaseVideoCap* pRemoteBestCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(eH264CapCode,(BYTE *)pRemoteBestStruct);
                if (pRemoteBestCap)
                {
                    COstrStream msg;
                    msg << "CH323Cntl::OnPartyOutgoingChannelReq Best remote H264 video cap \n";
                    pRemoteBestCap->Dump(msg);
                    PTRACE(eLevelInfoNormal, msg.str().c_str());
                    isH263Preffered = checkIsh263preffered(pRemoteBestCap, TRUE);

                    long fs = ((CH264VideoCap*)pRemoteBestCap)->GetFs();
                    long mbps = ((CH264VideoCap*)pRemoteBestCap)->GetMbps();
                    APIU8 level = ((CH264VideoCap*)pRemoteBestCap)->GetLevel();
                    if (mbps == -1)
                    {
                      CH264Details thisH264Details = level;
                      mbps = thisH264Details.GetDefaultMbpsAsProduct();
                    }
                    else
                      mbps *= CUSTOM_MAX_MBPS_FACTOR;
                    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq -fs rec : ",fs);
                    if (fs == -1)
                    {
                      CH264Details thisH264Details = level;
                      fs = thisH264Details.GetDefaultFsAsProduct();
                    }
                    else
                      fs *= CUSTOM_MAX_FS_FACTOR;

                    int fps = (int)(mbps/fs);
                    CapEnum algorithm = (CapEnum)(m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit));
                //    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCreateControl -fs : ",fs);
                //    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCreateControl -mbps : ",mbps);
                    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq -fps : ",fps);
                //    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCreateControl -alg : ",algorithm);
                      if(systemCardsBasedMode == eSystemCardsMode_breeze && ( fps <= 10 ) && algorithm == eH264CapCode )
                    {

                      APIU8 profile = 0, level=0;
                      long Targetmbps=0, Targetfs=0, dpb=0, brAndCpb=0, sar=0, staticMB=0;
                      m_pTargetModeH323->GetH264Scm(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);
                      if(Targetfs >= H264_HD720_FS_AS_DEVISION)
                      {

                        PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq -TB ep and fps is less than 10 -sending only SD -changing transmit only!");
                              Targetfs = H264_WCIF60_FS_AS_DEVISION;
                        Targetmbps =H264_W4CIF_30_MBPS_AS_DEVISION;
                              m_pTargetModeH323->SetH264Scm(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);
                        //BYTE cif4 = m_pLocalCapH323->Get4CifMpi();
                        //SetH264ModeInLocalCapsAndScmAccordingToVideoPartyType(eCP_H264_upto_SD30_video_party_type,cif4);
                        //m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323); //-noa just for test

                      }

                    }

                }
              }
      }*/

    }
  }
  else// for none video media. in advance audio codec, check also implicit declaration
    bRes = m_pRmtCapH323->OnCap(h323CapCode, TRUE);

  if(!bRes)
  {
    BYTE bCanNotOpen    = TRUE;
    BYTE bConnnectToConf  = TRUE;
    if (eType == cmCapAudio) // wait for timer in order to connect to conf
    {
                    if(m_pLocalCapH323->FindSecondBestCap(m_pRmtCapH323, h323CapCode,cmCapAudio))
                    {
                        bCanNotOpen     = FALSE;
                      bConnnectToConf = FALSE;
                      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq Find best cap!!",h323CapCode);
                    }
                    else
                      bConnnectToConf = FALSE;
    }

    if ((eType == cmCapVideo) && (eRole == kRolePeople)) //in this case, out can be opened with another protocol
    {
      if(m_pTargetModeH323->GetConfType() == kVideoSwitch
        || m_pTargetModeH323->GetConfType() == kVSW_Fixed
        || m_pTargetModeH323->GetConfType() == kCop
        || !m_pLocalCapH323->FindSecondBestCap(m_pRmtCapH323, h323CapCode))
      {
        CSecondaryParams secParams;
        m_bVideoOutRejected = TRUE;
        m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_RMT_DIFF_CAPCODE,secParams);
      }
      else
      {
        bCanNotOpen   = FALSE;
        bConnnectToConf = FALSE;
                isMCMSOpenChannel = FALSE;
      }
    }

    if (bConnnectToConf)
      ConnectPartyToConf();
    if(bCanNotOpen)
      return;
  }

  CCapSetInfo capInfo(h323CapCode);
  if( m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnPartyOutgoingChannelReq bIsClosing process - ",m_pmcCall->GetConnectionId());
    return;
  }

  // if the outgoing channel is already open don't open it again
  DWORD index = GetChannelIndexInList(true, eType,TRUE, eRole);
  if(index < m_maxCallChannel)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnPartyOutgoingChannelReq - channel is already open - ",m_pmcCall->GetConnectionId());
    return;
  }

  //Decide on the out rate:
  if (capInfo.GetCapType() == cmCapVideo)
  {
    if(eRole == kRolePeople)
    {
      BYTE bUpdateSuccess = UpdateVideoOutRates(h323CapCode);
      if (bUpdateSuccess == FALSE)
      {
        PTRACE2INT(eLevelError,"CH323Cntl::OnPartyOutgoingChannelReq - UpdateVideoOutRates failed - ",m_pmcCall->GetConnectionId());
        return;
      }
    }
  }

  DWORD rate = 0;

  if((capInfo.GetPayloadType() == _AnnexQ) || (capInfo.GetPayloadType() == _RvFecc))
      rate = m_pLocalCapH323->GetMaxDataRate();
  else if(capInfo.GetCapType() == cmCapAudio)
    rate = capInfo.GetBitRate() / _K_;
  else if (eRole & kRoleContentOrPresentation)
    rate = min(m_pLocalCapH323->GetMaxContRate(),GetRmtPossibleContentRate());
  else //kRolePeople
  {
    if (m_pTargetModeH323->GetConfType() == kCop)
      rate = m_pTargetModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit);
    else
      rate = m_pParty->GetVideoRate();
  }


  // Set of McChannel Params
  CChannel *pMcChannel = new CChannel;
  if(!pMcChannel)
    PASSERTMSG(m_pCsRsrcDesc->GetConnectionId(),"CH323Cntl::OnPartyOutgoingChannelReq - pMcChannel allocation failed");
  else
  {
    int rIndex;
    BOOL bIsOutgoingChannel = TRUE;
    BOOL bRval        = FALSE;
    WORD isLpr        = 0;
    if (m_pRmtCapH323->IsLPR() && m_pTargetModeH323->GetIsLpr() && eType==cmCapVideo)
      isLpr = 1;
    //ON(m_OneOfTheMediaChannelWasConnected);
    rIndex = InitMcChannelParams(bIsOutgoingChannel, pMcChannel, capInfo, eRole, rate, NULL,0,0, isLpr);
    CChannel *pSameSessionChannel = FindChannelInList(pMcChannel->GetType(), FALSE, (ERoleLabel)pMcChannel->GetRoleLabel());

    BYTE bIsSupportDBC2   = IsSupportingDBC2(pMcChannel);
    pMcChannel->SetIsDbc2(bIsSupportDBC2);

    if(rIndex < m_maxCallChannel)
    {
      // Set outgoing channel params
      mcReqOutgoingChannel* pOutChnlReq;

      BYTE bPrtInChangeModeOfOutChanProcess = FALSE;
      eChangeModeState partyStateMode = m_pParty->GetChangeModeState();
      if((partyStateMode == eReopenOut) || (partyStateMode == eFlowControlInAndReopenOut))
        bPrtInChangeModeOfOutChanProcess = TRUE;

      BYTE bIsNetMeeting = (m_remoteIdent == NetMeeting)? 1:0;
      BYTE bChooseBestRemoteCap = bIsNeedToImprove && !m_onGatewayCall && !bPrtInChangeModeOfOutChanProcess && !bIsNetMeeting;
      //1- In case of gw call, we already choose the desired mode from remote caps, and put
      //it in the local capabilities.
      //2- In case of cascade dial in, the best remote cap was already selected. But in case of dial out we do want to select it
      //3 - In case we in change mode state and the state is one of the follow we don't want to take the best remote
      //we should take the target transmit mode as we receive from the scm in the change mode process.
      //eReopenOut or eChangeInAndReopenOut or eReopGideonSimenInAndOut
      //4 - In case the remote is NetMeeting we need to open the intersection between what he opened and our choice,
      //we can't take the best remote cap because there is case the remote open QCIF and it's best is CIF and he does
      //not support asymmetric choice.
      if( (eType == cmCapVideo) && (eRole == kRolePeople) && bChooseBestRemoteCap)
      {
        BYTE bIsVideoSwitch   = (m_pTargetModeH323->GetConfType() == kVideoSwitch);
        BYTE bIsAutoVideoMode = m_pTargetModeH323->IsAutoVideoResolution();
        BYTE bIsVSWAutoResolution = (bIsVideoSwitch && bIsAutoVideoMode);
        if (bIsVSWAutoResolution)
        {
          BYTE bIgnoreHighProfileCap = TRUE; // in vsw we don't support high profile yet
          const BaseCapStruct* pRemoteBestStruct = m_pRmtCapH323->GetBestCapStruct(h323CapCode, cmCapReceive, kUnknownFormat, kRolePeople, bIgnoreHighProfileCap);
          if (pRemoteBestStruct)
          {
            CBaseVideoCap* pRemoteBestCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(h323CapCode,(BYTE *)pRemoteBestStruct);
            if (pRemoteBestCap)
            {
              int structSize = pRemoteBestCap->SizeOf();

              pRemoteBestCap->NullifySqcifMpi();
              m_pTargetModeH323->SetMediaMode(h323CapCode, structSize, (BYTE*)pRemoteBestCap->GetStruct(),
                              cmCapVideo, cmCapTransmit, kRolePeople, true);
              bIsNeedToImprove = FALSE;
              isMCMSOpenChannel = TRUE;
            }
            POBJDELETE(pRemoteBestCap);
          }
        }
      }

      WORD length = 0;
      //in case of cmCapVideo and kRolePeople and !isMCMSOpenChannel and !cop => we do intersect
      BYTE bTakeAsIs = isMCMSOpenChannel || (eType != cmCapVideo) || (eRole != kRolePeople) || (m_pTargetModeH323->GetConfType()== kCop);

      if (bTakeAsIs)//take from target transmit mode as is
      {
        if( (eType == cmCapAudio) || (eType == cmCapData) )
          length = sizeof(channelSpecificParameters);
        else
        {
          if (isMCMSOpenChannel == TRUE || (m_pTargetModeH323->GetConfType() == kCop && eRole == kRolePeople))
          {
            if (m_pTargetModeH323->IsMediaOn(eType, cmCapTransmit, eRole))
              length = m_pTargetModeH323->GetMediaLength(eType, cmCapTransmit, eRole);
            else
              length = m_pTargetModeH323->GetMediaLength(eType, cmCapReceive, eRole);
          }
          else
          {
            if (m_pTargetModeH323->IsMediaOn(eType, cmCapReceive, eRole))
              length = m_pTargetModeH323->GetMediaLength(eType, cmCapReceive, eRole);
            else
              length = m_pTargetModeH323->GetMediaLength(eType, cmCapTransmit, eRole);
          }

        }
      }

      else //do intersect
      {//video channel and initiator isn't mcms:
        CapEnum scmProtocol = eUnknownAlgorithemCapCode;
        CapEnum incomingProtocol = h323CapCode;
        if (m_pTargetModeH323->IsMediaOn(cmCapVideo, cmCapTransmit))
          scmProtocol= (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit);
                if(strstr(PARTYNAME, "##FORCE_MEDIA_VIDEO_H261"))
                {
                  m_pTargetModeH323->SetMediaMode((m_pTargetModeH323->GetMediaMode(cmCapVideo,cmCapReceive,kRolePeople)),cmCapVideo,cmCapTransmit,kRolePeople, true);
          length = m_pTargetModeH323->GetMediaLength(cmCapVideo, cmCapTransmit, kRolePeople);
          isMCMSOpenChannel = TRUE;
          capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit, kRolePeople);

                }
                else
                {

          //CapEnum incomingProtocol = eUnknownAlgorithemCapCode;
          //CChannel* pInChannel = FindChannelInList(cmCapVideo, FALSE);
          //if (pInChannel)
          //  incomingProtocol= pInChannel->GetCapNameEnum();

          if ( (scmProtocol != eUnknownAlgorithemCapCode) && (incomingProtocol != eUnknownAlgorithemCapCode))
          {
            CBaseVideoCap* pIntersect = NULL;
            BYTE bIsCP = m_pTargetModeH323->GetConfType() == kCp;

            if(bIsNetMeeting)
              pIntersect = FindIntersectForNetMeeting(incomingProtocol,scmProtocol);
            else
              pIntersect = FindIntersect(incomingProtocol,scmProtocol,isopen4cif);

            /*PTRACE(eLevelError, "CH323Cntl::OnPartyOutgoingChannelReq - After Intersect"); Remove Tandberg patch

                      if( pIntersect && (pIntersect->GetCapCode() == eH264CapCode) && (m_remoteIdent == TandbergEp) )
                      {
                        COstrStream msg;
                        eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
                        msg << "CH323Cntl::OnPartyOutgoingChannelReq Inside new intersect \n";
                        pIntersect->Dump(msg);
                        PTRACE(eLevelInfoNormal, msg.str().c_str());

                        long fs = ((CH264VideoCap*)pIntersect)->GetFs();
                        long mbps = ((CH264VideoCap*)pIntersect)->GetMbps();
                        APIU8 level = ((CH264VideoCap*)pIntersect)->GetLevel();
                        if (mbps == -1)
                        {
                          CH264Details thisH264Details = level;
                          mbps = thisH264Details.GetDefaultMbpsAsProduct();
                        }
                        else
                          mbps *= CUSTOM_MAX_MBPS_FACTOR;
                        PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq -fs rec : ",fs);
                        if (fs == -1)
                        {
                          CH264Details thisH264Details = level;
                          fs = thisH264Details.GetDefaultFsAsProduct();
                        }
                        else
                          fs *= CUSTOM_MAX_FS_FACTOR;

                        int fps = (int)(mbps/fs);
                        CapEnum algorithm = (CapEnum)(m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit));
                        PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq -fps : ",fps);
                        if(systemCardsBasedMode == eSystemCardsMode_breeze && ( fps <= 10 ) && algorithm == eH264CapCode )
                        {
                     //change intersect to sd
                     APIU8 profile = 0, level=0;
                     long Targetmbps=0, Targetfs=0, dpb=0, brAndCpb=0, sar=0, staticMB=0;
                     m_pTargetModeH323->GetH264Scm(profile, level, Targetmbps, Targetfs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);

                               PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq -Targetfs : ",Targetfs);
                               PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq -Targetmbps : ",Targetmbps);

                               ((CH264VideoCap*)pIntersect)->SetFs(Targetfs);
                               ((CH264VideoCap*)pIntersect)->SetMbps(Targetmbps);
                               ((CH264VideoCap*)pIntersect)->SetLevel(level);

                        }
                      }*/
            if (pIntersect)
            {
              m_pTargetModeH323->SetMediaMode(pIntersect->GetCapCode(), pIntersect->SizeOf(), (BYTE*)pIntersect->GetStruct(), cmCapVideo, cmCapTransmit, kRolePeople, true);
              length = m_pTargetModeH323->GetMediaLength(cmCapVideo, cmCapTransmit, kRolePeople);
              isMCMSOpenChannel = TRUE;
              capInfo = pIntersect->GetCapCode();
              pIntersect->FreeStruct();
		//FSN-613: Dynamic Content for SVC/Mix Conf
		if (eType == cmCapVideo && eRole == kRolePeople)
			rate = m_pTargetModeH323->GetVideoBitRate(cmCapTransmit, kRolePeople);
            }
            else
              PTRACE(eLevelError, "CH323Cntl::OnPartyOutgoingChannelReq - Intersect Failed");

            POBJDELETE(pIntersect);
          }

          else
          {
            if (incomingProtocol == eUnknownAlgorithemCapCode)//isMCMSOpenChannel should have been true, in order to insert to case of bTakeAsIs
              PTRACE(eLevelError, "CH323Cntl::OnPartyOutgoingChannelReq - Incoming protocol wasn't found. isMCMSOpenChannel should have been TRUE");
            else //scmProtocol == UnknownAlgorithemCapCode
              PTRACE(eLevelError, "CH323Cntl::OnPartyOutgoingChannelReq - Scm protocol wasn't found.");
          }
                }
      }
            UpdateMcChannelParams (pMcChannel, capInfo, rate);
      BYTE bSend = FALSE;
      DWORD fips140Status = STATUS_OK;
      if (length)
      {
        totalSize   = sizeof(mcReqOutgoingChannel) + length;
        pOutChnlReq = (mcReqOutgoingChannel *)new BYTE[totalSize];

        if(!pOutChnlReq)
          PASSERTMSG(m_pCsRsrcDesc->GetConnectionId(),"CH323Cntl::OnPartyOutgoingChannelReq - pOutChnlReq allocation failed");
        else
        {
          memset(pOutChnlReq, 0, totalSize);
//          pOutChnlReq->bIsDBC2        = bIsSupportDBC2;
          pOutChnlReq->sizeOfChannelParams  = length;
                    AllocateDynamicPayloadType(pMcChannel, capInfo);

          // if (pSameSessionChannel && CiscoGW == m_remoteIdent)
//          {
//            PTRACE2INT (eLevelInfoNormal, "CH323Cntl::OnPartyOutgoingChannelReq - cisco GW incoming channel pt: ",pSameSessionChannel->GetPayloadType());
//            pMcChannel->SetPayloadType(pSameSessionChannel->GetPayloadType());
//            pOutChnlReq->payloadType    = pSameSessionChannel->GetPayloadType();
//          }
//          else
//                         pOutChnlReq->payloadType     = capInfo.GetPayloadType() ;
          pOutChnlReq->channelType      = capInfo.GetCapType() ;
          pOutChnlReq->capTypeCode            = (CapEnum)capInfo;
          pOutChnlReq->channelIndex     = pMcChannel->GetCsIndex();
          pOutChnlReq->channelDirection   = pMcChannel->IsOutgoingDirection();

          if(  !pMcChannel->GetDynamicPayloadType()  && eRole == kRolePeople && eType == cmCapVideo )
          {
            pOutChnlReq->payloadType      = capInfo.GetPayloadType() ;
            PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - changing to static payload!");

          }
          // IpV6
          enIpVersion eIpAddrMatch = CheckForMatchBetweenPartyAndUdp(m_pH323NetSetup->GetIpVersion(),m_UdpAddressesParams.IpType);

          pOutChnlReq->SourceRtpAddressIp.transAddr.ipVersion = eIpAddrMatch;
          if (pOutChnlReq->SourceRtpAddressIp.transAddr.ipVersion == eIpVersion4)
          {
            pOutChnlReq->SourceRtpAddressIp.transAddr.addr.v4.ip = m_UdpAddressesParams.IpV4Addr.ip;


          }
          else
          {// Case IpV6
            // --- UDP: array of addresses ---
            BYTE  place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(m_pH323NetSetup->GetTaDestPartyAddr(), m_UdpAddressesParams.IpV6AddrArray);
            pOutChnlReq->SourceRtpAddressIp.transAddr.addr.v6.scopeId = m_UdpAddressesParams.IpV6AddrArray[place].scopeId;
            APIU32 i = 0;
            for (i = 0; i < 16; i++)
              pOutChnlReq->SourceRtpAddressIp.transAddr.addr.v6.ip[i] = m_UdpAddressesParams.IpV6AddrArray[place].ip[i];

          }
          pOutChnlReq->SourceRtpAddressIp.transAddr.distribution = eDistributionUnicast;
          pOutChnlReq->SourceRtpAddressIp.transAddr.transportType = eTransportTypeUdp;
          switch(pOutChnlReq->channelType)
          {
            case cmCapAudio:
            {
              pOutChnlReq->SourceRtpAddressIp.transAddr.port = m_UdpAddressesParams.AudioChannelPort;
              break;
            }
            case cmCapData:
            {
              pOutChnlReq->SourceRtpAddressIp.transAddr.port = m_UdpAddressesParams.FeccChannelPort;
              break;
            }
            case cmCapVideo:
            {
              if (eRole == kRolePeople)
                pOutChnlReq->SourceRtpAddressIp.transAddr.port = m_UdpAddressesParams.VideoChannelPort;
              else
                pOutChnlReq->SourceRtpAddressIp.transAddr.port = m_UdpAddressesParams.ContentChannelPort;
              break;
            }
            default:
            {
              PASSERTMSG(pOutChnlReq->channelType,"CH323Cntl::OnPartyOutgoingChannelReq - Channel Type");
              break;
            }
          }
          // fill the XML IP Address fields
          pOutChnlReq->SourceRtpAddressIp.unionProps.unionType = (DWORD)eIpAddrMatch;
          pOutChnlReq->SourceRtpAddressIp.unionProps.unionSize = sizeof(ipAddressIf);

          memset(&(pOutChnlReq->channelSpecificParams), 0, pOutChnlReq->sizeOfChannelParams);
          EResult bRes = kSuccess;

          bRes = BuildOpenChannelStruct(capInfo,eRole,isMCMSOpenChannel,pMcChannel->GetRate(),
                          (channelSpecificParameters *)&(pOutChnlReq->channelSpecificParams), bIsNeedToImprove);
          if (bRes)
          {
            BYTE* channelSpecificParams = (BYTE*)(pOutChnlReq->channelSpecificParams);
            AllocateAndSetChannelParams(pOutChnlReq->sizeOfChannelParams, (char*)channelSpecificParams, pMcChannel); //set channels params in pMcChannel
            AllocateDynamicPayloadType(pMcChannel, capInfo);
            SetIsH263PlusForOutgoingChannel(pMcChannel);
            m_pTargetModeH323->SetMediaMode(capInfo, length, (BYTE *)&(pOutChnlReq->channelSpecificParams), eType, cmCapTransmit, eRole, true);
          }

          pOutChnlReq->dynamicPayloadType = pMcChannel->GetDynamicPayloadType();
          if (bRes)
          {
            strncpy(pOutChnlReq->channelName, capInfo.GetH323CapName() , ChannelNameSize);
            pOutChnlReq->channelName[ChannelNameSize - 1] = '\0';

            pOutChnlReq->bIsEncrypted     = pMcChannel->GetIsEncrypted();
            pOutChnlReq->encryptionAlgorithm  = pMcChannel->GetEncryptionType();
            pOutChnlReq->xmlDynamicProps.numberOfDynamicParts  = 1;
            pOutChnlReq->xmlDynamicProps.sizeOfAllDynamicParts = pOutChnlReq->sizeOfChannelParams;

            if (m_pmcCall->GetMasterSlaveStatus() == cmMSMaster)
            {
              if(pMcChannel->GetIsEncrypted())
              { // In this case we will create the session and encrypted session key with
                // Alg functions using the shared secret(Master key)
                APIU8 sessionKey[sizeOf128Key];
                APIU8 encSessionKey[sizeOf128Key];
                memset(sessionKey,'0',sizeOf128Key);
                memset(encSessionKey,'0',sizeOf128Key);

                PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - master key= ");
                PrintDHToTrace(sizeOf128Key,(BYTE *)m_pDHKeyManagement->GetEncrCallKey()->GetArray());

                BOOL isOpenSSLFunc = NO;
                CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_OPENSSL_ENC_FUNC, isOpenSSLFunc);
                if (isOpenSSLFunc != NO)
                  fips140Status = CreateCipherKeyOpenSSL(m_pDHKeyManagement->GetEncrCallKey()->GetArray(), sessionKey, encSessionKey);
                else
                  fips140Status = CreateCipherKey(m_pDHKeyManagement->GetEncrCallKey()->GetArray(),sessionKey,encSessionKey);
                if(fips140Status != STATUS_OK)
                {
                  CMedString errorString;
                  char errStr[128] = {0};

                  errorString << "CH323Cntl::OnPartyOutgoingChannelReq: FIPS140 Test Failure - Disconnect the call!\n";

                  ERR_error_string_n(ERR_get_error(),errStr,128);

                  ERR_load_crypto_strings();
                  ERR_load_FIPS_strings();

                  errorString << errStr;

                  PTRACE(eLevelInfoNormal,errorString.GetString());

                }

                // ===== 1. get the unSimulationErrCode from SysConfig
                std::string eSimValue;
                CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
                pSysConfig->GetDataByKey(CFG_KEY_FIPS140_SIMULATE_CONFPARTY_PROCESS_ERROR, eSimValue);
                eConfPartyFipsSimulationMode fips140SimulationConfPartyError = eInactiveSimulation;
                fips140SimulationConfPartyError = ::TranslateSysConfigDataToEnumForConfParty(eSimValue);

                if(fips140SimulationConfPartyError == eFailPartyCipherFipsTest)
                {
                  fips140Status = STATUS_FAIL;
                  PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq: simulate TEST_FAILED - Disconnect the call!");
                }


                PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - session key= ");
                PrintDHToTrace(sizeOf128Key,(BYTE *)sessionKey);


                pMcChannel->SetH235SessionKey(sessionKey);
                pMcChannel->SetH235EncryptedSessionKey(encSessionKey);
                memcpy(&(pOutChnlReq->EncryptedSession235Key),encSessionKey , sizeOf128Key);
                PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - Encrypted session key = ");
                PrintDHToTrace(sizeOf128Key,(BYTE *)pOutChnlReq->EncryptedSession235Key);
                if(!m_bIsAvaya)
                {
                  PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyOutgoingChannelReq - Full key (taking the LSB) = ");
                  PrintDHToTrace(HALF_KEY_SIZE,(BYTE *)m_pDHKeyManagement->GetDHResultSharedSecret()->GetArray());
                }
              }
              else
                memset(&(pOutChnlReq->EncryptedSession235Key),'0',sizeOf128Key);
            }
            else
              memset(&(pOutChnlReq->EncryptedSession235Key),'0',sizeOf128Key);

            if (fips140Status == STATUS_OK)
            {
            // LPR
            pOutChnlReq->bIsLPR = isLpr;

            CSegment* pMsg = new CSegment;
            pMsg->Put((BYTE*)(pOutChnlReq),totalSize);
            m_pCsInterface->SendMsgToCS(H323_CS_SIG_OUTGOING_CHNL_REQ,pMsg,m_serviceId,
                          m_serviceId,m_pDestUnitId,m_callIndex,pMcChannel->GetCsIndex(),pMcChannel->GetIndex(),0);
            pMcChannel->SetCsChannelState(kBeforeConnectedInd);
            POBJDELETE(pMsg);

            bSend = TRUE;
          }
        }
        }
        PDELETEA(pOutChnlReq);
      }
      if (!bSend)
        m_pmcCall->RemoveChannel(rIndex);
      if (fips140Status != STATUS_OK)
        m_pTaskApi->EncryptionDisConnect(FIPS140_STATUS_FAILURE);

    }
    else
      POBJDELETE(pMcChannel);
  }
}

///////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::AllocateDynamicPayloadType(CChannel* pMcChannel, CCapSetInfo capInfo)
{
  channelSpecificParameters* pChannelParams = (channelSpecificParameters*)pMcChannel->GetChannelParams();
  DWORD payloadType = pMcChannel->GetPayloadType();
  BYTE bUseDynamicPayloadType = FALSE;
  BYTE isContentH263L = FALSE;
  if (pMcChannel->GetIsEncrypted())
  {
    bUseDynamicPayloadType = TRUE;
    if (payloadType ==  _H263)
    {
      CapEnum capEnum = pMcChannel->GetCapNameEnum();
      CH263VideoCap *pH263VideoCap = (CH263VideoCap*)CBaseCap::AllocNewCap(capEnum,(BYTE*)pChannelParams);
      if(pH263VideoCap)
      {
      BYTE isWithoutAnnexF = TRUE;
      isContentH263L = pH263VideoCap->IsH263Plus(isWithoutAnnexF);// annex F is H263
      if (pH263VideoCap->GetRole() == kRolePresentation)
        isContentH263L ++;
	  /*find by valgrind. If pChannelParams is NULL, AllocNewCap will allocate the structure*/
	  if (pChannelParams == NULL)
	  	pH263VideoCap->FreeStruct();
      POBJDELETE(pH263VideoCap);
      }
      else
        PASSERTMSG(NULL == pH263VideoCap, "AllocNewCap return NULL!!!");
    }

  }
  else
  {
    switch (payloadType)
    {
      case _Siren7:
      case _Siren14:
      case _G7221:
      case _H264:
      case _AnnexQ:
      case _RvFecc:
      case _Siren14S:
      case _Siren22S:
      case _Siren22:
      case _SirenLPR:
      case _G719:
      case _G719S:
        bUseDynamicPayloadType = TRUE;
        break;
      case _H263:
      {
        CapEnum capEnum = pMcChannel->GetCapNameEnum();
        CH263VideoCap *pH263VideoCap = (CH263VideoCap*)CBaseCap::AllocNewCap(capEnum,(BYTE*)pChannelParams);
        if(pH263VideoCap)
        {
        BYTE isWithoutAnnexF = TRUE;
        isContentH263L = pH263VideoCap->IsH263Plus(isWithoutAnnexF);// annex F is H263
        if (pH263VideoCap->GetRole() == kRolePresentation )
          isContentH263L ++;
        if (isContentH263L)
        {
          bUseDynamicPayloadType = TRUE;
        }
		/*find by valgrind. If pChannelParams is NULL, AllocNewCap will allocate the structure*/
		if (pChannelParams == NULL)
		  pH263VideoCap->FreeStruct();

        POBJDELETE(pH263VideoCap);
        }
        else
          PASSERTMSG(NULL == pH263VideoCap, "AllocNewCap return NULL!!!");

        break;
      }
      default:
        break;
    }
  }

  if (bUseDynamicPayloadType)
  {
    APIU8 dynamicPayload = 0;
    CChannel *pSameSessionChannel = FindChannelInList(pMcChannel->GetType(),
      !pMcChannel->IsOutgoingDirection(), (ERoleLabel)pMcChannel->GetRoleLabel());
    if (pSameSessionChannel && (CiscoGW == m_remoteIdent || LifeSizeEp == m_remoteIdent))
    {
      PTRACE2INT (eLevelInfoNormal, "CH323Cntl::AllocateDynamicPayloadType - cisco GW incoming channel dynamic pt: ",pSameSessionChannel->GetDynamicPayloadType());
      dynamicPayload = pSameSessionChannel->GetDynamicPayloadType();

    }
    else
      dynamicPayload = capInfo.GetDynamicPayloadType(isContentH263L);
    pMcChannel->SetDynamicPayloadType(dynamicPayload);
  }
  else
    pMcChannel->SetDynamicPayloadType(0);
}

///////////////////////////////////////////////////////////////////////////////////
//Only for outgoing channel we can assume that the channel is h263+ only in case it has annexes or customs.
void CH323Cntl::SetIsH263PlusForOutgoingChannel(CChannel* pMcChannel)
{
  CapEnum capEnum = pMcChannel->GetCapNameEnum();
  APIU8 bIsH263Plus = 0;
  if (capEnum == eH263CapCode)
  {
    channelSpecificParameters* pChannelParams = (channelSpecificParameters*)pMcChannel->GetChannelParams();
    CH263VideoCap *pH263VideoCap = (CH263VideoCap*)CBaseCap::AllocNewCap(eH263CapCode, (BYTE*)pChannelParams);
    if(pH263VideoCap)
    {
    bIsH263Plus = pH263VideoCap->IsH263Plus();
    POBJDELETE(pH263VideoCap);
  }
    else
      PASSERTMSG(NULL == pH263VideoCap, "AllocNewCap return NULL!!!");
  }

  pMcChannel->SetIsH263Plus(bIsH263Plus);
}

///////////////////////////////////////////////////////////////////////////////////
CBaseVideoCap* CH323Cntl::FindIntersect(CapEnum incomingProtocol,CapEnum scmProtocol,BYTE isopen4cif)
{
  PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersect - General trace");
  CBaseVideoCap* pIntersect = NULL;

  BYTE bIsCp  = (m_pTargetModeH323->GetConfType() == kCp);

  //******************* 1. First we try to open HD asymmetric modes*******************//
  if (bIsCp && (scmProtocol == eH264CapCode) && m_pLocalCapH323->OnCap(eH264CapCode))
  {
    //HDCP
    pIntersect = FindIntersectForHDCPAsymmetricModes(); //Incase of HD asymmetric modes in H264 CP call find the intersect
  }

    //************2. try to find intersect between SCM and Rmtcaps ******************//
  if (pIntersect == NULL)
    //if (scmProtocol == incomingProtocol)  //intersect scm and remote caps
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersect - Try intersect between scm and Rmtcaps");
      m_pTargetModeH323->Dump("CH323Cntl::FindIntersect -  Try intersect between scm and Rmtcaps this is scm:", eLevelInfoNormal);
      pIntersect = m_pRmtCapH323->FindIntersectionBetweenCapsAndVideoScm(m_pTargetModeH323);
      //if(pIntersect)
        //pIntersect->Dump("CH323Cntl::FindIntersect - Try intersect between scm and Rmtcaps -this is intersect");

    }

  //****************** 3. We try to open H263 4CIF ************************//

  // if selected video protocol is H263 or even if not h263 but couldn't find intersectection and incoming protocol is h263,
  //  check if transimitting 4Cif is supporetd.
  // if answer is yes, set transmit channel to H263 4cif.
  if (pIntersect == NULL && bIsCp && (scmProtocol == eH263CapCode || incomingProtocol == eH263CapCode))
  {// open outgoing channel with 4CIF capability although the local caps has CIF only capability.

    PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersect - Try H2634CIF #1");

    if(IsH2634CifSupported()&& isopen4cif)
      pIntersect = SetScmWith4Cif();
  }
    else if ( bIsCp && pIntersect && eH263CapCode == pIntersect->GetCapCode())
    {
        PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersect - Try H2634CIF #2");

    if(IsH2634CifSupported() && isopen4cif)
        {
      pIntersect->FreeStruct();
            POBJDELETE(pIntersect);
      pIntersect = SetScmWith4Cif();
        }

    }


  if (pIntersect == NULL && scmProtocol == incomingProtocol)  //intersect scm and remote caps
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersect - None HD attempt");
  }
  //then we will try to  open H2634CIF instead of H264
  if(isopen4cif && IsH2634CifSupported() && pIntersect )
    if(checkIsh263preffered(pIntersect,bIsCp))
    {
      pIntersect->FreeStruct();
      POBJDELETE(pIntersect);
      pIntersect = SetScmWith4Cif();
    }

  // TODO: debug mode for incoming H261 outgoing H263
  // if the name is to force H263 out although the incoming is H263, take remote H263 people.
  if(pIntersect == NULL && strstr(PARTYNAME, "##FORCE_MEDIA_VIDEO_H261_OUT_H263"))
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersect - Force outgoing H263");
        CComModeH323* pScm = new CComModeH323;
        if(pScm)
        {
			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
			if(pCommConf)
			{
				*pScm = *m_pTargetModeH323;
				pScm->SetHighestH263ScmForCP(cmCapReceiveAndTransmit, pCommConf->GetVideoQuality());
				pIntersect = m_pRmtCapH323->FindIntersectionBetweenCapsAndVideoScm(pScm);
			}
			else
			  PASSERT(NULL == pCommConf);

			POBJDELETE(pScm);
        }

  }

  //*********4. try to find intersect between SCM and Rmtcaps************//
  if (pIntersect == NULL)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersect - local and remote");
    pIntersect = m_pLocalCapH323->FindIntersectionBetweenTwoCaps(m_pRmtCapH323, incomingProtocol, cmCapReceiveAndTransmit);
    PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersect - Try intersect between two caps");
    if(pIntersect)
    {

      //then we will try to  open H2634CIF instead of H264
      if(isopen4cif && IsH2634CifSupported()&& checkIsh263preffered(pIntersect,bIsCp))
      {
        pIntersect->FreeStruct();
        POBJDELETE(pIntersect);
        pIntersect = SetScmWith4Cif();
      }
    }
    else
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersect - No Intersect found");
      PASSERT(1);
      POBJDELETE(pIntersect);
      return NULL;
    }
  }

  return pIntersect;
}
///////////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::checkIsh263preffered(CBaseVideoCap* pIntersect,BYTE bIsCp)
{
    BYTE retVal = FALSE;
    if (CPObject::IsValidPObjectPtr(pIntersect))
    {
        if(IsH2634CifSupported())
  {
            CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
            if(pCommConf)
            retVal = (pIntersect->checkIsh263preffered(bIsCp, pCommConf->GetVideoQuality()));
            else
                PASSERT_AND_RETURN_VALUE(1, retVal);
        }
        else
            retVal = FALSE;
    }
    else
       DBGPASSERT(m_pmcCall->GetConnectionId());

    return retVal;
}

///////////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsH2634CifSupported()
{
  BYTE res = FALSE;
  BYTE isInChannelAllow4cif = TRUE;
  CChannel* invideochannel = m_pmcCall->FindChannelInList(cmCapVideo,0/*rec channel*/,kRolePeople);
  if(invideochannel && invideochannel->GetIsRejectChannel() == FALSE)
  {
    eVideoPartyType inpartytype =m_pTargetModeH323->GetVideoPartyType(cmCapReceive);
    //PTRACE2INT(eLevelInfoNormal,"CH323Cntl::IsH2634CifSupported - DO NOT ALLOW OUT4CIF  because  in is HD ",(eVideoPartyType)inpartytype);
    if(inpartytype >=eCP_H264_upto_HD720_30FS_Symmetric_video_party_type)
    {

      PTRACE(eLevelInfoNormal,"CH323Cntl::IsH2634CifSupported -   because 4cif in channel is HD party type");
      //isInChannelAllow4cif  = FALSE;

    }



  }
  else
    PTRACE(eLevelInfoNormal,"CH323Cntl::IsH2634CifSupported - no in channel");

    int remote4CifMpi = m_pRmtCapH323->Get4CifMpi();
    int local4CifMpi  = m_pLocalCapH323->Get4CifMpi();
    BYTE isLocalSupportH263 = m_pLocalCapH323->IsH263CapFound();
    if (isLocalSupportH263 &&isInChannelAllow4cif && remote4CifMpi != -1 && local4CifMpi != -1 && remote4CifMpi <= 2 && local4CifMpi <= 2)
      res = TRUE;
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::IsH2634CifSupported - res,",res);
  return res;

}
///////////////////////////////////////////////////////////////////////////////////
CBaseVideoCap* CH323Cntl::SetScmWith4Cif()
{
  CBaseVideoCap* pIntersect = NULL;

      CComModeH323* pScmWith4Cif = new CComModeH323; AUTO_DELETE(pScmWith4Cif);
      CBaseVideoCap* pLocalBestCap;
      BYTE bCheckProfile = TRUE; // not relevant for h263
      const BaseCapStruct* pLocalBestStruct = m_pLocalCapH323->GetBestCapStruct(eH263CapCode, cmCapReceive, kUnknownFormat, kRolePeople, bCheckProfile);
      if (pLocalBestStruct)
      {
        // build CComModeH323 base on local h263 cap
        pLocalBestCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(eH263CapCode,(BYTE *)pLocalBestStruct);
        if (pLocalBestCap)
        {
      int local4CifMpi = m_pLocalCapH323->Get4CifMpi();

          pScmWith4Cif->SetMediaMode(pLocalBestCap, cmCapVideo, cmCapTransmit, kRolePeople, true);
          // set video transmit direction to transmit (since it was taken from local caps, which its direction is "received")
          pScmWith4Cif->SetDirection(cmCapVideo,cmCapTransmit, kRolePeople);

          pScmWith4Cif->SetFormatMpi(k4Cif, local4CifMpi , cmCapTransmit);  // set 4cif transmit values (highest mpi will be chosen)

          pScmWith4Cif->Dump("CH323Cntl::FindIntersect - 4Cif CommMode:", eLevelInfoNormal);

          pIntersect = m_pRmtCapH323->FindIntersectionBetweenCapsAndVideoScm(pScmWith4Cif);
          POBJDELETE(pScmWith4Cif);
        }
      }
      else
      {
        PASSERT(1);
        POBJDELETE(pScmWith4Cif);
        return NULL;
      }
      POBJDELETE(pLocalBestCap);

  return pIntersect;


}
///////////////////////////////////////////////////////////////////////////////////
CBaseVideoCap* CH323Cntl::FindIntersectForHDCPAsymmetricModes()
{
  PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersectForHDCPAsymmetricModes");
  CBaseVideoCap* pIntersect = NULL;

  if(!m_pRmtCapH323->IsH264CapFound() )
    return NULL;

  //VNGR-9489 In case of HD1080/HD720 60fps asymmetric modes, we must consider the actual rate that the remote supports
  DWORD tmpCallRate = GetCallRateAllowedByRemote();
  DWORD callRateWithDefencePrecentage = (DWORD)((m_pTargetModeH323->GetCallRate()) * 0.9);
  BYTE bNeedToReCheckDecisionMatrix = NO;
  H264VideoModeDetails h264VidModeDetails;
  Eh264VideoModeType eH264VideoModeType = eLasth264VideoMode;
  if(tmpCallRate<callRateWithDefencePrecentage)
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::FindIntersectForHDCPAsymmetricModes, Recheck the decision matrix the rate is lower than 90% of the expected call rate this is call rate",tmpCallRate);
    bNeedToReCheckDecisionMatrix = YES;
    PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersectForHDCPAsymmetricModes, Recheck the decision matrix the rate is lower than 90% of the expected call rate");
    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
    if(pCommConf)
    {
    eVideoQuality vidQuality = pCommConf->GetVideoQuality();
    GetH264VideoParams(h264VidModeDetails, tmpCallRate*1000, vidQuality);
    eH264VideoModeType = h264VidModeDetails.videoModeType;
    }
    else
      PASSERT(NULL == pCommConf);


  }

  ////////////////////////////HD1080At60/////////////////////////////////////////////////
  // Currently we support HD1080At60 only in MPMx based system and only in asymmetric mode
  BYTE bRemoteSupportHD1080At60 = m_pRmtCapH323->IsCapableOfHD1080At60();
  BYTE bLocalSupportHD1080At60  = m_pTargetModeH323->IsHd1080At60Enabled();

  if (bRemoteSupportHD1080At60 && bLocalSupportHD1080At60)
  {
		if(bNeedToReCheckDecisionMatrix && eH264VideoModeType!=eHD1080At60Asymmetric)
		{
		  PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersectForHDCPAsymmetricModes asymmetric HD1080 at 60 fpsnot found because of remote low rate");
		  return NULL;
		}
		PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersectForHDCPAsymmetricModes - Found HD1080 60 fps");
		CComModeH323 scmWithHD1080At60;
		scmWithHD1080At60 = *m_pTargetModeH323;
		APIU8 prof = scmWithHD1080At60.GetH264Profile(cmCapTransmit);
		scmWithHD1080At60.SetScmToCpHD1080At60(cmCapReceiveAndTransmit);
		if(!m_pRmtCapH323->IsSupportH264HighProfile())
			scmWithHD1080At60.SetH264Profile(H264_Profile_BaseLine,cmCapTransmit);
		else
			scmWithHD1080At60.SetH264Profile(prof,cmCapTransmit);

		pIntersect = m_pRmtCapH323->FindIntersectionBetweenCapsAndVideoScm(&scmWithHD1080At60);
  }
  ////////////////////////////HD1080/////////////////////////////////////////////////
  // Currently we support HD1080 only in MPM+ based system and only in asymmetric mode
  BYTE bRemoteSupportHD1080 = m_pRmtCapH323->IsCapableOfHD1080();
  BYTE bLocalSupportHD1080  = m_pTargetModeH323->IsHd1080Enabled();

  if (bRemoteSupportHD1080 && bLocalSupportHD1080)
  {
		if(bNeedToReCheckDecisionMatrix && eH264VideoModeType!=eHD1080Asymmetric && eH264VideoModeType!=eHD1080At60Asymmetric)
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersectForHDCPAsymmetricModes asymmetric HD1080 not found because of remote low rate");
      return NULL;
    }
    PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersectForHDCPAsymmetricModes - Found HD1080");
    CComModeH323* pScmWithHD1080 = new CComModeH323;
    *pScmWithHD1080 = *m_pTargetModeH323;
    pScmWithHD1080->SetScmToHdCp(eHD1080Res, cmCapReceiveAndTransmit);
    pIntersect = m_pRmtCapH323->FindIntersectionBetweenCapsAndVideoScm(pScmWithHD1080);
    POBJDELETE(pScmWithHD1080);
  }
  ////////////////////////////HD720 60fps/////////////////////////////////////////////////
  // Currently we support HD720 60fps only in MPM+ based system and only in asymmetric mode
  BYTE bRemoteSupportHD720At60 = m_pRmtCapH323->IsCapableOfHD720At60();
  BYTE bLocalSupportHD720At60  = m_pTargetModeH323->IsHd720At60Enabled();

  if (bRemoteSupportHD720At60 && bLocalSupportHD720At60)
  {
    if(bNeedToReCheckDecisionMatrix && eH264VideoModeType!=eHD720At60Asymmetric)
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersectForHDCPAsymmetricModes asymmetric HD720 at 60 fps not found because of remote low rate");
      return NULL;
    }
    PTRACE(eLevelInfoNormal,"CH323Cntl::FindIntersectForHDCPAsymmetricModes - Found HD720At60");
    CComModeH323* pScmWithHD720At60 = new CComModeH323;
    *pScmWithHD720At60 = *m_pTargetModeH323;
    pScmWithHD720At60->SetScmToCpHD720At60(cmCapReceiveAndTransmit);
    pIntersect = m_pRmtCapH323->FindIntersectionBetweenCapsAndVideoScm(pScmWithHD720At60);
    POBJDELETE(pScmWithHD720At60);
  }

  return pIntersect;
}

///////////////////////////////////////////////////////////////////////////////////
//In VSW only
CBaseVideoCap* CH323Cntl::FindIntersectForNetMeeting(CapEnum incomingProtocol,CapEnum scmProtocol)
{
  CBaseVideoCap* pIntersect = NULL;

  BYTE bCheckRate   = FALSE;
  BYTE bIsCP        = m_pTargetModeH323->GetConfType() == kCp; //in cp the scm isn't must! If it is quad + 4cif than the scm contains only 4cif, but we want also cif, so we can't take from the scm.
  BYTE bTakeFromScm = !bIsCP || (bIsCP && m_pParty->IsPartyInChangeVideoMode()); //if it is change cp mode, so the scm is ok, and we can take from scm
  if ((scmProtocol == incomingProtocol) && bTakeFromScm)
  {
    CBaseVideoCap* pRcvScmCap = (CBaseVideoCap*)m_pTargetModeH323->GetMediaAsCapClass(cmCapVideo, cmCapReceive);
    CBaseVideoCap* pTrsScmCap = (CBaseVideoCap*)m_pTargetModeH323->GetMediaAsCapClass(cmCapVideo, cmCapTransmit);
    if(!pRcvScmCap)
    {
    	if(pTrsScmCap)				//Fix Memory Leak
			POBJDELETE(pTrsScmCap);

    	PASSERT_AND_RETURN_VALUE(NULL == pRcvScmCap, pIntersect);

    }
    if(!pTrsScmCap) //Fix Memory Leak
    {
    	POBJDELETE(pRcvScmCap);
    	PASSERT_AND_RETURN_VALUE(NULL == pTrsScmCap, pIntersect);
    }

    BYTE bIntersectRate = !bCheckRate;
    pIntersect = pTrsScmCap->CreateIntersectBetweenTwoVidCaps(pRcvScmCap, cmCapTransmit, bIntersectRate, FALSE);
    POBJDELETE(pRcvScmCap);
    POBJDELETE(pTrsScmCap);
  }

  if(pIntersect == NULL)
  {
    CBaseVideoCap* pRcvScmCap = (CBaseVideoCap*)m_pTargetModeH323->GetMediaAsCapClass(cmCapVideo, cmCapReceive);
    pIntersect = m_pLocalCapH323->FindIntersectionBetweenTwoCaps(pRcvScmCap, incomingProtocol,cmCapReceiveAndTransmit, kRolePeople, bCheckRate);
    PTRACE(eLevelInfoNormal, "CH323Cntl::FindIntersectForNetMeeting - Intersect from receive");
    POBJDELETE(pRcvScmCap);
  }

  return pIntersect;
}

///////////////////////////////////////////////////////////////////////////////////
//When decide on video rate in CP, the totsl BW should be taken into consideration.
DWORD CH323Cntl::ChangeVideoRateInCp(DWORD oldVideoRate)
{
  PTRACE2(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp. Name - ",PARTYNAME);
  if(m_FixVideoRateAccordingToType)
    return oldVideoRate;

  DWORD confRate = m_pmcCall->GetRate();
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp. confRate- ",confRate);
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp. oldVideoRate- ",oldVideoRate);
  DWORD totalVidRateScm = m_pTargetModeH323->GetTotalVideoRate();



  if (m_isCallingThroughGk == FALSE)
  {
    CChannel* pOutAudio = FindChannelInList(cmCapAudio, TRUE);
    if (pOutAudio)
    {
      DWORD audRate = pOutAudio->GetRate();
    //  DWORD confRate = m_pmcCall->GetRate();

  if(0 < totalVidRateScm)
  {
	  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp. audRate- ",audRate);
  	if(confRate/100 > (totalVidRateScm + (audRate*10)))
	    confRate = totalVidRateScm*100 + (audRate*1000);
	   m_pmcCall->SetRate(confRate);
  }

      DWORD newVidRate = ((confRate/100) - (audRate*10));
      if (newVidRate <= (rate64K / 100))
      {
        m_pTargetModeH323->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
        if(FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation))
        {
          PTRACE2(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp -new video rate was less than 64k downgrade to secondary content. Name - ",PARTYNAME);
          CloseOutgoingChannel(cmCapVideo, kRoleContentOrPresentation);
        }
      }

	  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp. newVidRate- ", newVidRate);
      return newVidRate;
    }
    else
    {
	  if(0 < totalVidRateScm && oldVideoRate > totalVidRateScm)
		  oldVideoRate = totalVidRateScm;
      return oldVideoRate;

  }
  }

  DWORD newVideoRate = (0 < totalVidRateScm ) ? min( oldVideoRate , totalVidRateScm) : oldVideoRate ; // eFeatureLineRate_6M

  PTRACE2(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp 1. Name - ",PARTYNAME);
  DWORD expectedAudioRate = m_pLocalCapH323->GetAudioDesiredRate();
  DWORD currentAudioRate  = 0;
  CChannel* pInAudio = FindChannelInList(cmCapAudio, FALSE);
  if (pInAudio)
    currentAudioRate = pInAudio->GetRate();

  if (currentAudioRate > expectedAudioRate)//remote opened audio with higher rate than expected
  {//we should reduce the outgoing video rate, if with the current video rate, the conf rate is exceeded

    DWORD newTotalRate = 0;
    BYTE  bIsBrqNeeded = CalculateNewBandwidth(newTotalRate);
    if (bIsBrqNeeded)
    {
      DWORD currentTotalRate =( m_pmcCall->GetBandwidth());
      if (newTotalRate > currentTotalRate) //means that the BRQ is on higher rate than the ARQ.
      {
        DWORD gapToRemove = newTotalRate - currentTotalRate;
        gapToRemove /= 100;
        if (gapToRemove < oldVideoRate)
          newVideoRate -= gapToRemove;
        else
          PTRACE2INT(eLevelError,"ChangeVideoRateInCp - gapToRemove is too high - ", gapToRemove);
      }
    }
  }
  else
  {
    CChannel* pOutAudio = FindChannelInList(cmCapAudio, TRUE);
    if (pOutAudio)
    {
      DWORD currentTotalRate =( m_pmcCall->GetBandwidth()/2);
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp. currentTotalRate- ",currentTotalRate);

      DWORD audRate = pOutAudio->GetRate();

      //Incase GK rate is higher than conf rate, will take the minimum rate
      DWORD MinRate = min(currentTotalRate,confRate);
        PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp. MinRate- ",MinRate);

      newVideoRate = ((MinRate/100) - (audRate*10));

      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp. audRate- ",audRate);
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp. newVideoRate- ",newVideoRate);
    }
  }
  // VNGR-4159
  DWORD totalRate = m_pmcCall->GetBandwidth();
  DWORD outRate = totalRate/2000;
  if ( currentAudioRate >= outRate && totalRate <= 128000)
  {
    newVideoRate = 0;
  }
  //vngr-7840
  if (newVideoRate <= (rate64K / 100))
  {
    PTRACE2(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp -out video channel less than 64k. Name - ",PARTYNAME);
    m_pTargetModeH323->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

    if(FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation))
    {
      PTRACE2(eLevelInfoNormal,"CH323Cntl::ChangeVideoRateInCp -new video rate was less than 64k downgrade to secondary content-with gk. Name - ",PARTYNAME);
      CloseOutgoingChannel(cmCapVideo, kRoleContentOrPresentation);
    }
  }


  return newVideoRate;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::RejectChannel(CChannel* pChannel, APIS32 status,APIU32 channelIndex, APIU32 csChannelIndex,BYTE arrayIndex)
{
  APIS32      rejectReason  = status;
  CCapSetInfo   capInfo     = pChannel->GetCapNameEnum();
  ERoleLabel    eRole     = (ERoleLabel)pChannel->GetRoleLabel();
  cmCapDataType type      = pChannel->GetType();

  BYTE isNeedToCloseUdp = 0;

  TRACEINTO << " RejectChannel\n " << "  Party Name "   << PARTYNAME
   << ", McmsConnId = " << m_pCsRsrcDesc->GetConnectionId()
   << ", mcms ChannelIndex = " << channelIndex
   << ", ChannelIndex = " << csChannelIndex
   << ", CapCode = "    << capInfo.GetH323CapName()
   << ", RejectReason = " << GetRejectReasonAsString(rejectReason);

  pChannel->SetIsRejectChannel(TRUE);
  if (pChannel->IsOutgoingDirection() != TRUE)
  {
    if (pChannel->GetCmUdpChannelState() != kNotSendOpenYet && pChannel->GetCmUdpChannelState() != kRecieveCloseAck &&
       pChannel->GetCmUdpChannelState() != kSendClose)
    {
      ON(isNeedToCloseUdp);
      pChannel->SetCsChannelState(kNoNeedToDisconnect);
      if (pChannel->GetRoleLabel() == kRolePeople)
      {
          CloseInternalChannels(pChannel->GetType());
      }
      Cm_FillAndSendCloseUdpPortStruct(pChannel);
    }
  }

  if(rejectReason == rjCallAlreadyDisconnected)
  {
    if(m_isReceiveCallDropMessage)  //after try to open channel we send call drop - the channel already close
      PTRACE(eLevelError,"RejectChannel: callDrop by MCMS");
    else
      PTRACE(eLevelError,"RejectChannel: Call close by remote");
  }
  else if(rejectReason == rjChanAlreadyDisconnected) //could be only in incoming channel
  {
    PTRACE(eLevelError,"RejectChannel: channel closed by remote");
    StartCloseChannel(pChannel, TRUE);
  }
  else if(rejectReason != rjOLCRejectByRemote)
  {
    if (capInfo.IsType(cmCapAudio))
      m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_NO_PORT_LEFT_FOR_AUDIO);
    else if (capInfo.IsType(cmCapVideo))
    {
      if (eRole & kRoleContentOrPresentation)
        m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_NO_PORT_LEFT_FOR_VIDEOCONT);
      else
        m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_NO_PORT_LEFT_FOR_VIDEO);
    }
    else if (capInfo.IsType(cmCapData))
      m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_NO_PORT_LEFT_FOR_FECC);

    //send to card
    mcReqStopAllProcesses* pStopAllProcessors = new mcReqStopAllProcesses;
    pStopAllProcessors->stopAllProcessorsReason = rejectOutChannel;
    SendStopAllProcessorsToCard(pStopAllProcessors,pChannel);
    PDELETE(pStopAllProcessors);
    DBGPASSERT(m_pmcCall->GetConnectionId());
    DBGPASSERT(channelIndex);
  }
  else //rejectReason == rjOLCRejectByRemote
  {
    if (eRole & kRoleContentOrPresentation)
    {
      PTRACE(eLevelError,"RejectChannel: OLC did not complete");
      //handle the bridges and the monitoring
      m_bIsOutContentChanReject = TRUE;
      m_pTaskApi->SendCloseChannelToConfLevel(type,cmCapTransmit,eRole);
    }
    else //RolePeople
    {
      if(type == cmCapAudio)
      {
        if (IsValidTimer(AUDCONNECTTOUT))
        {
          PTRACE(eLevelInfoNormal,"DeleteTimer(AUDCONNECTTOUT);");
          DeleteTimer(AUDCONNECTTOUT);
        }
      }
      if(type == cmCapVideo)
      {
        //as was in MGC
        if (pChannel->IsOutgoingDirection() == TRUE)
          ON(m_bVideoOutRejected);
        else
          ON(m_bVideoInRejected);
        if (m_pParty->IsPartyInChangeVideoMode() && !m_pRmtCapH323->IsECS())
          m_pTaskApi->SendH323LogicalChannelReject(cmCapVideo, cmCapTransmit, kRolePeople);
        if (m_pRmtCapH323->IsECS() && m_remoteCapIndNotHandle)
        {
          PTRACE (eLevelError, "CH323Cntl::RejectChannel - handle ECS now!");
          ConnectPartyToConf();
        }
        //Lior's suggestion
              //ConnectPartyToConf();
                //was before Lior's suggestion
        //if(m_pParty->IsPartyInChangeVideoMode())
        //  m_pTaskApi->SendH323LogicalChannelReject(cmCapVideo, cmCapTransmit, kRolePeople);
        //if (m_pRmtCapH323->IsECS() && m_remoteCapIndNotHandle)
        //{
        //  PTRACE (eLevelError, "CH323Cntl::RejectChannel - handle ECS now!");
        //  ConnectPartyToConf();
        //}
      }
      else if(type == cmCapData)
      {
        m_bIsDataOutRejected = TRUE;
        ConnectPartyToConf();
      }
    }
  }

  CloseChannel(pChannel,arrayIndex,rejectReason,isNeedToCloseUdp);
  return;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323OutgoingChnlResponseInd(CSegment* pParam)
{
  CChannel* pSameSessionChannel = NULL;
  CChannel* pCurrentChannel     = NULL;

  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323OutgoingChnlResponseInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  ////////////parameters for party monitoring///////////////////////
  DWORD actualRate = 0xFFFFFFFF;
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;
  // IpV6 - Monitoring
  mcTransportAddress partyAddr;
  mcTransportAddress mcuAddr;

  memset(&partyAddr,0,sizeof(mcTransportAddress));
  memset(&mcuAddr,0,sizeof(mcTransportAddress));

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;


  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323OutgoingChnlResponseInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323OutgoingChnlResponseInd - srcUnitId incorrect");
    return;
  }

  mcIndOutgoingChannelResponse poutChnlResponse;

  DWORD  structLen = sizeof(mcIndOutgoingChannelResponse);
  memset(&poutChnlResponse,0,structLen);
  pParam->Get((BYTE*)(&poutChnlResponse),structLen);

  APIS8 arrayIndex = NA;
  int res = SetCurrentChannel(channelIndex,mcChannelIndex,&pCurrentChannel,&arrayIndex);

  if(pCurrentChannel == NULL)
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323OutgoingChnlResponseInd - Outgoing connected channel was not found");
    DBGPASSERT(channelIndex);
    return;
  }

  if (status != 0)
  {
    RejectChannel(pCurrentChannel,status,mcChannelIndex,channelIndex,arrayIndex);
    return;
  }

  if (res) //failure
  {
    PTRACE2INT(eLevelError,  "CH323Cntl::OnH323OutgoingChnlResponseInd: channel %d not found",channelIndex);
    DWORD isReturnUnExpectedMsg = GetSystemCfgFlagInt<DWORD>(CFG_KEY_IP_UNEXPECTED_MESSAGE);
    if (isReturnUnExpectedMsg == YES)
    {
      int totalSize =
        sizeof(mcReqUnexpectedMessageBase) + sizeof(mcIndOutgoingChannelResponse);

      mcReqUnexpectedMessage *pUnexpectedMessage = (mcReqUnexpectedMessage *)new BYTE[totalSize];

      pUnexpectedMessage->badOpcode   = H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND;
      pUnexpectedMessage->sizeMessage = sizeof(mcIndOutgoingChannelResponse);
      memcpy(pUnexpectedMessage->message,&poutChnlResponse,sizeof(mcIndOutgoingChannelResponse));

      CSegment* pMsg = new CSegment;
      pMsg->Put((BYTE*)(pUnexpectedMessage),totalSize);
      m_pCsInterface->SendMsgToCS(H323_CS_SIG_UNEXPECTED_MESSAGE_REQ,pMsg,m_serviceId,
        m_serviceId,m_pDestUnitId,m_callIndex,pCurrentChannel->GetCsIndex(),pCurrentChannel->GetIndex(),0);
      POBJDELETE(pMsg);

      PDELETEA(pUnexpectedMessage);
    }
    DBGPASSERT(channelIndex);
    return;
  }

  ERoleLabel    eRole = (ERoleLabel)pCurrentChannel->GetRoleLabel();
  cmCapDataType type  = pCurrentChannel->GetType();

    pCurrentChannel->SetCsIndex(channelIndex);



  //When the card recsive callDrop he close the channels so I do not need to send channelDrop on the outgoingChannel
  if(m_pmcCall->GetCallCloseInitiator() == McInitiator && m_isReceiveCallDropMessage)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnH323OutgoingChnlResponseInd. Call drop sent --> the channel will be close by the card - ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }

  //we received close outgoingChannel before we recieved channelResponse - it can happened in changeMode.
  if(pCurrentChannel->IsCsChannelStateDisconnecting())
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnH323OutgoingChnlResponseInd. State is disconnecting - closing the channel - ",m_pCsRsrcDesc->GetConnectionId());
    pCurrentChannel->SetChannelCloseInitiator((DWORD)McInitiator);
    SendChannelDropReq(pCurrentChannel);
    return;
  }

    pCurrentChannel->SetStatus(status);
  pCurrentChannel->SetCsChannelState(kConnectedState); //stream is still off

  DWORD sameSessionChannelIndex = GetChannelIndexInList(true, type, !pCurrentChannel->IsOutgoingDirection(), eRole);
    pSameSessionChannel = m_pmcCall->GetSpecificChannel(sameSessionChannelIndex);
  // IpV6
  pCurrentChannel->SetRmtAddress(poutChnlResponse.destRtpAddress.transAddr);
  // IpV6 - Monitoring
  memcpy(&partyAddr, &(poutChnlResponse.destRtpAddress.transAddr), sizeof(mcTransportAddress));

  if(m_pmcCall->GetIsOrigin())//dial out
    memcpy(&mcuAddr, (m_pH323NetSetup->GetTaSrcPartyAddr()), sizeof(mcTransportAddress));

  else
  {
    if (m_pmcCall->GetDestTerminalParams().callSignalAddress.ipVersion == eIpVersion4)
      memcpy(&mcuAddr, &(m_pmcCall->GetDestTerminalParams().callSignalAddress), sizeof(mcTransportAddress));
  }

  /*
   * In case Dynamic Payload Type was received in OLC ACK response,
   * set the new dynamic payload type to the channel only if bridge declares we support this ability.
   * */
  if ( poutChnlResponse.dynamicPayloadType && m_pLocalCapH323->GetDynamicPTRepControl() )
  {
	  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323OutgoingChnlResponseInd - DPTR = ",poutChnlResponse.dynamicPayloadType);
	  pCurrentChannel->SetDynamicPayloadType(poutChnlResponse.dynamicPayloadType);
  }

  switch(poutChnlResponse.channelType)
  {
    case cmCapAudio:
    {
      mcuAddr.port = m_UdpAddressesParams.AudioChannelPort;
      break;
    }
    case cmCapData:
    {
      mcuAddr.port = m_UdpAddressesParams.FeccChannelPort;
      break;
    }
    case cmCapVideo:
    {
      if (eRole == kRolePeople)
      {
        mcuAddr.port = m_UdpAddressesParams.VideoChannelPort;
        break;
      }
      else
      {
        mcuAddr.port = m_UdpAddressesParams.ContentChannelPort;
        break;
      }
    }
    default:
    {
      PASSERTMSG(poutChnlResponse.channelType,"CH323Cntl::OnH323OutgoingChnlResponseInd - Channel Type");
      break;
    }
  }


  EIpChannelType channelType = ::CalcChannelType(type,pCurrentChannel->IsOutgoingDirection(),
                                pCurrentChannel->GetRoleLabel(),pCurrentChannel->GetCapNameEnum());
  CPrtMontrBaseParams *pPrtMonitrParams = CPrtMontrBaseParams::AllocNewClass(channelType);
  CCapSetInfo capInfo = pCurrentChannel->GetCapNameEnum();
  SetPartyMonitorBaseParams(pPrtMonitrParams,channelType,actualRate,&partyAddr,&mcuAddr,(DWORD)capInfo.GetIpCapCode());
  LogicalChannelConnect(pPrtMonitrParams,(DWORD)channelType);
  POBJDELETE(pPrtMonitrParams);

  //ecs:
  if (::isApiTaNull(&partyAddr))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::OnH323OutgoingChnlResponseInd: Ip is zero");
    pCurrentChannel->SetStreamState(kMuteByRemote);
  }

  if (m_pmcCall->GetMasterSlaveStatus() == cmMSSlave)
  {
    if(pCurrentChannel->GetIsEncrypted())
    { // In this case we will create the session and encrypted session key with
      // Alg functions using the shared secret(Master key)
      APIU8 sessionKey[sizeOf128Key];
      memset(sessionKey,'0',sizeOf128Key);
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323OutgoingChnlResponseInd - master key= ");
      PrintDHToTrace(sizeOf128Key,(BYTE *)m_pDHKeyManagement->GetEncrCallKey()->GetArray());

      BOOL isOpenSSLFunc = NO;
      CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_OPENSSL_ENC_FUNC, isOpenSSLFunc);
      if (isOpenSSLFunc != NO)
        DecryptCipherKeyOpenSSL(m_pDHKeyManagement->GetEncrCallKey()->GetArray(),poutChnlResponse.EncryptedSession235Key , sessionKey);
      else
      DecryptCipherKey(m_pDHKeyManagement->GetEncrCallKey()->GetArray(),poutChnlResponse.EncryptedSession235Key , sessionKey);

      PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323OutgoingChnlResponseInd - session key= ");
      PrintDHToTrace(sizeOf128Key,(BYTE *)sessionKey);


      pCurrentChannel->SetH235SessionKey(sessionKey);
      pCurrentChannel->SetH235EncryptedSessionKey(poutChnlResponse.EncryptedSession235Key);
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323OutgoingChnlResponseInd - Encrypted session key = ");
      PrintDHToTrace(sizeOf128Key,(BYTE *)poutChnlResponse.EncryptedSession235Key);
      if(!m_bIsAvaya)
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323OutgoingChnlResponseInd - Full key (taking the LSB) = ");
        PrintDHToTrace(HALF_KEY_SIZE,(BYTE *)m_pDHKeyManagement->GetDHResultSharedSecret()->GetArray());
      }
    }
  }

    if (Rtp_FillAndSendUpdatePortOpenRtpStruct(pCurrentChannel))
  Cm_FillAndSendOpenUdpPortOrUpdateUdpAddrStruct(pCurrentChannel, poutChnlResponse.destRtpAddress.transAddr);
    else
      m_pTaskApi->EncryptionDisConnect(FIPS140_STATUS_FAILURE);

  //update video rate on CP
  if (m_pTargetModeH323->GetConfType() == kCp && kRolePeople == eRole && cmCapVideo == type)
  {
    DWORD newVideoBitRate = pCurrentChannel->GetRate();
    m_pCurrentModeH323->SetVideoBitRate(newVideoBitRate, cmCapTransmit, kRolePeople);
  }
}

////////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::IsNeedToSendFlowControl()
{
  //we need to send different rate when the remote is a MCU or RadVision or terminal.
  BOOL ret = TRUE;

  switch (m_remoteIdent)
  {
    case RvMCU:
    case RvGWOrProxy:
    case PolycomMGC:
    case PolycomRMX:
    case VIU:
        case DstH323Mcs:
      ret = FALSE;
      break;
	default:
	  // Note: some enumeration value are not handled in switch. Add default to suppress warning.
	  break;
  }

  return ret;
}

////////////////////////////////////////////////////////////////////////////
/*void CH323Cntl::UpdateRateIfNeeded(CChannel *pCurrentChannel,BOOL bIsAfterReCpas)
{
  CChannel *pIncomingChannel = NULL;

  ERoleLabel    eRole     = (ERoleLabel)pCurrentChannel->GetRoleLabel();
  cmCapDataType type      = pCurrentChannel->GetType();

  if(!pCurrentChannel->IsOutgoingDirection())
    pIncomingChannel = pCurrentChannel;
  else
  {
    // Finding the incoming channel
        DWORD index = GetChannelIndexInList(true, type, !pCurrentChannel->IsOutgoingDirection() , eRole);

    if(index == m_maxCallChannel)
        {
            PTRACE(eLevelInfoNormal,"CH323Cntl::UpdateRateIfNeeded : same session channel not found");
            pIncomingChannel = NULL;
      return;
        }
        else
            pIncomingChannel = m_pmcCall->GetSpecificChannel(index);
  }


  if(IsNeedToSendFlowControl() && (pIncomingChannel != NULL))
  {
    DWORD wantedBitRate  = m_pLocalCapH323->GetMaxVideoBitRate(pIncomingChannel->GetCapNameEnum(), cmCapReceive, kRolePeople);
    if (wantedBitRate == 0)
      return;//we don't send flow control on people channel with rate 0!
    DWORD rmtBitRate   = m_pTargetModeH323->GetMediaBitRate(cmCapVideo,cmCapReceive,kRolePeople);
    DWORD defencePercent = 95;//::GetpSystemCfg()->GetDefensePercent();

    //In case we send recaps when  content is on we should send flow control to reduce the people rate with
    //the content rate.
    if(bIsAfterReCpas && m_curConfContTdmRate)
      wantedBitRate = (m_curPeopleRate * defencePercent) / 100;
    else
      wantedBitRate = (wantedBitRate * defencePercent) / 100;

    //Only if 94% is lowest from the rate the remote opened in the incoming channel
    if(wantedBitRate < rmtBitRate)
    {
      CComModeH323 *pTmpModeH323 = new CComModeH323;
      *pTmpModeH323 = *m_pCurrentModeH323;
      pTmpModeH323->SetVideoBitRate((DWORD)wantedBitRate,cmCapReceive,kRolePeople);
      //m_pCurrentModeH323->SetVideoBitRate((DWORD)wantedBitRate,cmCapReceive,kRolePeople);
      //Do not updet the incoming rate otherwise we will send to the conference after the reduce defencePercent
      //and in case of incoming>outgoing and we neeed to send recap, we will send recap with this value instead
      //of the 100%
      //pIncomingChannel->rate = wantedBitRate;
      UpdateVideoOutRates();

      if ((m_pTargetModeH323->GetConfType() == kCp) )
      {//in case the flow control was to increase video in, there can be a situation,
      // that the in is higher than the out:
        DWORD inIndex  = GetChannelIndexInList(true, cmCapVideo, FALSE);
        DWORD outIndex = GetChannelIndexInList(true, cmCapVideo, TRUE);
        CChannel* pChannelIn = m_pmcCall->GetSpecificChannel(inIndex);
        CChannel* pChannelOut = m_pmcCall->GetSpecificChannel(outIndex);
        if((inIndex < m_maxCallChannel) &&  (outIndex < m_maxCallChannel))
        {// if the video channels opened
          WORD incomingVideoRate = pChannelIn->GetRate();
          WORD outgoingVideoRate = pChannelOut->GetRate();
          if (incomingVideoRate > outgoingVideoRate)
            UpdatePartyVideoRate();
        }
      }
      SendFlowControlReq(cmCapVideo,FALSE,kRolePeople,(DWORD)wantedBitRate);
      m_pTaskApi->Rmt323CommModeUpdateDB(pTmpModeH323);
      POBJDELETE (pTmpModeH323);
    }
  }
}*/

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnChannelConnected(CChannel *pCurrentChannel)
{
  cmCapDirection eDirection = (pCurrentChannel->IsOutgoingDirection())? cmCapTransmit: cmCapReceive;
  UpdateCurrentScmH323(pCurrentChannel);

  BYTE bInformConf = FALSE;

  CChannel *pSameSessionChannel = FindChannelInList(pCurrentChannel->GetType(), !pCurrentChannel->IsOutgoingDirection(), (ERoleLabel)pCurrentChannel->GetRoleLabel());

  if (pSameSessionChannel && pSameSessionChannel->IsChannelConnected())
  {
    bInformConf = TRUE;
    if (pCurrentChannel->GetType() == cmCapVideo)
    {
      BYTE bContentAlreadyActiveForParty = (m_pParty->GetContentRate() != 0);

      if (pCurrentChannel->GetRoleLabel() & kRoleContentOrPresentation)
      {
        DWORD flowControlRate = 0;
        if (!bContentAlreadyActiveForParty)
          SetNewVideoRateInH239Call(0);

        else
        {
          PTRACE2(eLevelInfoNormal,"CH323Cntl::OnChannelConnected - content in was connected after the content was already active for the party. Name - ",PARTYNAME);

          /*In h239, the in channel can be opened only after the party got ack on
           the presentationTokenRequest. So if this is the speaker, we need to send
           f.c on the new content rate and not f.c. to 0.
           However, in EPC, an ep isn't allowed to reopen the content channel while
           it is the speaker*/
           if ((pCurrentChannel->GetRoleLabel() == kRolePresentation) && m_bIsContentSpeaker)
            flowControlRate = m_pParty->GetContentRate();
        }
        if ((pCurrentChannel->GetRoleLabel() == kRolePresentation) && m_bIsContentSpeaker)
        {
          BYTE bIsFlowControlSent = SendFlowControlReq(cmCapVideo, FALSE, (ERoleLabel)pCurrentChannel->GetRoleLabel(), flowControlRate);
          if (bIsFlowControlSent)
          {
            ON(m_isH239FlowCntlSent);
            m_pCurrentModeH323->SetVideoBitRate(flowControlRate, cmCapReceive, kRolePresentation);
          }
        }


        if (m_eContentInState == eSendStreamOn && eDirection == cmCapReceive)
        {
          TRACEINTO << "CH323Cntl::OnChannelConnected - Incoming content channel on the fly";
          eContentState eTempContentInState = eStreamOn;
                    SendContentOnOffReqForRtp();
          //m_pTaskApi->SendContentOnOffAck(STATUS_OK,eTempContentInState);
        }



        //In case the content closed and reopen and it was the speaker (Ipower do it) we should withdraw it's
        //token (only for EPC)
/*        if ((pCurrentChannel->GetRoleLabel() == kRoleContent)  && m_bIsContentSpeaker)
        {
          OnPartyRoleTokenReq(kPresentationTokenRequest);
          m_bIsContentSpeaker = 0;
        }
*/
        if ((pCurrentChannel->GetRoleLabel() == kRolePresentation) && (eDirection == cmCapReceive) )
          bInformConf = FALSE;
      }

      else if (pCurrentChannel->GetRoleLabel() == kRolePeople)
      {
        if (bContentAlreadyActiveForParty)
        {// flow control to video in with m_curVideoRate
          DWORD flowControlRate = m_curPeopleRate;
          SendFlowControlReq(cmCapVideo,FALSE,kRolePeople,flowControlRate);
          PTRACE2(eLevelInfoNormal,"CH323Cntl::OnChannelConnected - video in was connected after the content was already active for the party. Name - ",PARTYNAME);
        }
        /*else
          UpdateRateIfNeeded(pCurrentChannel,FALSE);*/

        //In case the remote has content and presentation he should open the presentation
        if ((m_pLocalCapH323->IsH239() && m_pRmtCapH323->IsH239()) || (m_pLocalCapH323->IsEPC() && m_pRmtCapH323->IsEPC()))
        {
          //check if the presentation out is opened
          BYTE bOutChannelAlreadyOpen = FALSE;
          if (FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation))
              bOutChannelAlreadyOpen = TRUE;
          if (bOutChannelAlreadyOpen == FALSE && m_pTargetModeH323->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation))
          {
            ERoleLabel eRole;
            CCapSetInfo capInfo   = GetCommonContentMode(eRole);
            if(capInfo.GetIpCapCode() == eUnknownAlgorithemCapCode)
            {
              PTRACE(eLevelInfoNormal,"CH323Cntl::OnChannelConnected - Remote and local caps mismatch on Content protocl - Do not open outgoing H239 channel.");
              m_bIsContentRejected = TRUE;// is that the best flag? consult.
              ConnectPartyToConf();
            }
            else
            {
              BOOL bIsAnnexT = GetSystemCfgFlagInt<BOOL>(CFG_KEY_H263_ANNEX_T); //&& m_pLocalCapH323->GetIsAnnexT();
              // we can open content: 1. if its not Sony EP, 2. if its sony but the EP support Annex T, 3. if annex T is not supported at the system
					if ((m_remoteIdent != SonyEp) || ((m_remoteIdent == SonyEp) && (m_pRmtCapH323->IsAnnex(typeAnnexT,kRoleContentOrPresentation))) || !bIsAnnexT)
              {
                PTRACE(eLevelInfoNormal,"CH323Cntl::OnChannelConnected - Open content out.");
                SetContentTargetTransmitMode(capInfo, eRole);//good for EPC and H239
                BYTE bIsMcmsOrigin    = TRUE;
                OnPartyOutgoingChannelReq(capInfo, eRole, bIsMcmsOrigin);
              }
          else
          {
            PTRACE2(eLevelInfoNormal,"CH323Cntl::OnChannelConnected - Remote and local caps mismatch on AnnexT (SONY patch) - Do not open outgoing H239 channel. Name - ",PARTYNAME);
						m_bIsContentRejected = TRUE;// is that the best flag? consult.
						ConnectPartyToConf();
					}
				}
          }
        }
      }
    }
  }

  else if ((pCurrentChannel->GetRoleLabel() == kRolePresentation) && (eDirection == cmCapTransmit))
    bInformConf = TRUE;
  else if(m_bVideoOutRejected)
    //In case incoming open with protocol that the remote caps are not supported we will reject the outgoing channel,
    //so when the incoming channel connect we should connect to conf
    bInformConf = TRUE;
  else if (m_bIsDataInRejected)
    bInformConf = TRUE;
  // VNGFE-787
  else if (m_isCodianVcr && m_isVideoOutgoingChannelConnected)
    bInformConf = TRUE;

  if (bInformConf)
    ConnectPartyToConf();

  else if ((pCurrentChannel->GetRoleLabel() == kRolePresentation) && (eDirection == cmCapReceive))
  {
    //In case it is the presentation that open, we did not inform to the conf so the remote commdoe does not update
    //so we can see the channel opened but in the remote commmode there is no expression about it.
    CMedString strBase;
    strBase<< m_pCurrentModeH323->GetMediaBitRate(cmCapVideo, cmCapReceive,kRoleContentOrPresentation) <<
        ", m_pCurrentModeH323 content rate (Tx)- " << m_pCurrentModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation);
    PTRACE2(eLevelInfoNormal,"CH323Cntl::OnChannelConnected :  m_pCurrentModeH323 content rate (Rcv)- ", strBase.GetString());

    m_pTaskApi->Rmt323CommModeUpdateDB(m_pCurrentModeH323);
  }

  if (m_remoteIdent == PolycomMGC
    && (pCurrentChannel->GetType() == cmCapVideo) && (pCurrentChannel->GetRoleLabel() == kRolePeople) && (eDirection == cmCapTransmit)
    && ((m_pTargetModeH323->GetConfType() == kVideoSwitch) || (m_pTargetModeH323->GetConfType() == kVSW_Fixed)))

  {
    DWORD newVidRate = (DWORD) (m_pCurrentModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit) * MGC_CASCADE_NEW_VID_RATE);
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnChannelConnected - Cascade to MGC in VSW conf. Initiate decreasion of video out rate. New video rate - ",newVidRate);
    m_pTaskApi->UpdatePartyH323VideoBitRate(newVidRate, cmCapTransmit, kRolePeople);
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323IncomingChnlInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  if(!m_McmsOpenChannels)
    DeleteTimer(MCMSOPENCHANNELS);

  BYTE  rejectChannel = 0;
  DWORD len = pParam->GetLen();
  // VNGSW-167
//  if(len > 1200)
//    PASSERTMSG(len,"CH323Cntl::OnH323IncomingChnlInd - Len > 1200");
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;
  // VNGFE-787
  BYTE  isCodianTimerValid = 1;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;
  if(!channelIndex) //andrewk debug
    PASSERTMSG(channelIndex,"CH323Cntl::OnH323IncomingChnlInd - channelIndex");
    //PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlInd - channelIndex = ", channelIndex);

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323IncomingChnlInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323IncomingChnlInd - srcUnitId incorrect");
    return;
  }

  DWORD nMsgLen =  pParam->GetWrtOffset() - pParam->GetRdOffset();
  mcIndIncomingChannel *pInChnlInd = (mcIndIncomingChannel*)new BYTE[nMsgLen];

  memset(pInChnlInd,0,nMsgLen);
  pParam->Get((BYTE*)pInChnlInd,nMsgLen);

  if (SaveIncomingChannelForFurtherUse(pInChnlInd,callIndex,channelIndex,mcChannelIndex,stat1,srcUnitId))
  {
    PDELETEA(pInChnlInd);
    return;
  }
  // check for reject reasons to the incoming channel
  rejectChannel = RejectIncomingChannelIfNeeded(pInChnlInd,status,channelIndex, mcChannelIndex);

  channelSpecificParameters* pChannelParams = (channelSpecificParameters *)&(pInChnlInd->channelSpecificParams);
  CCapSetInfo capInfo = (CapEnum)pInChnlInd->capTypeCode;
  cmCapDataType eType = capInfo.GetCapType();
  ERoleLabel    eRole = kRolePeople;

  CBaseCap *pBaseInCap = CBaseCap::AllocNewCap((CapEnum)capInfo, (BYTE*)pChannelParams);
  if(pBaseInCap)
    eRole = pBaseInCap->GetRole();

  if (eType == cmCapVideo)
  {
    if (eRole == kRolePeople)
      m_bVideoInRejected = FALSE;
    else
      m_bIsContentRejected = FALSE;
  }
  else if(eType == cmCapData)
    m_bIsDataInRejected = FALSE;

  capBuffer *pCapBuffer = NULL;

  // set the incoming channel into the Target Mode (H323 Target receive SCM)
  if(rejectChannel == FALSE)
  {
    // VNGR-787
    if (m_isCodianVcr && eType == cmCapAudio)
    {
      // This is always the first channel to be opened from the remote
      // In this case we'll start a timer for the Codian video incoming channel - If the timer pops
      // and no video incoming channel was opened - We will implement our solution
      // Otherwise the flow will remain the same and Alg. fix should take care of the codian issue.
      // Test
//      CSegment* pParam = NULL;
//      OnCodianVcrVidChannelTimeout(pParam);
      StartTimer(CODIANVIDCHANTOUT,2*SECOND);
    }
    if (m_isCodianVcr && eType == cmCapVideo && eRole == kRolePeople)
    {
      // If the timer is still valid - We need to terminate it and continue the flow as usual
      // Other wise - We need only to update all channel params and update the Decoder (Will still be muted)
      if(IsValidTimer(CODIANVIDCHANTOUT))
        DeleteTimer(CODIANVIDCHANTOUT);
      else
      {
        // This means that we already opened the outgoing channel
        isCodianTimerValid = 0;
      }

    }

    if (m_isCodianVcr && !isCodianTimerValid && eType == cmCapVideo)
    {
      PTRACE(eLevelError,"CH323Cntl::OnH323IncomingChnlInd: Updating codian video in channel ONLY!!!");
      // In this case we only update the channel in our list and send response
      DWORD structureSize;

      if (pBaseInCap)
      {
        structureSize = pBaseInCap->SizeOf();//pInChnlInd->sizeOfChannelParams;
      }
      else
      {
        PASSERT(1);
        structureSize = 0;
      }

      if(structureSize == 0)
        structureSize = capInfo.GetH323CapStructSize();

      EResult eResOfSet = kSuccess;
      pCapBuffer = (capBuffer*)new BYTE[structureSize + sizeof(capBufferBase)];
      memset(pCapBuffer,0,(structureSize + sizeof(capBufferBase)));
      pCapBuffer->capLength = structureSize;
      pCapBuffer->capTypeCode = (CapEnum)capInfo;

      int rIndex;
      CChannel *pMcChannel = new CChannel;

      BOOL bIsOutgoingChannel = FALSE;
      BOOL bRval        = FALSE;
      WORD isLpr        = pInChnlInd->bIsLPR;
//      ON(m_OneOfTheMediaChannelWasConnected);

      DWORD bitRate = m_pTargetModeH323->GetMediaBitRate(eType);
      //The IP version must be checked!!!
      pMcChannel->SetRmtAddressVer(pInChnlInd->rmtRtpAddress.transAddr.ipVersion);
      // IpV6
      pMcChannel->SetRmtAddress(pInChnlInd->rmtRtpAddress.transAddr);

/*      pMcChannel->SetRmtAddressVer(pInChnlInd->rmtRtpAddress.transAddr.ipVersion);
      pMcChannel->SetRmtAddressPort(pInChnlInd->rmtRtpAddress.transAddr.port);
      pMcChannel->SetRmtAddressIPv4(pInChnlInd->rmtRtpAddress.transAddr.addr.v4.ip);  */
//      rIndex = InitMcChannelParams(bIsOutgoingChannel,pMcChannel,capInfo,eRole,bitRate,pInChnlInd,status, channelIndex,isLpr);
      rIndex = InitMcChannelParams(bIsOutgoingChannel,pMcChannel,capInfo,eRole,bitRate, pInChnlInd,status, channelIndex,isLpr);
      if(status == STATUS_OK)
        pMcChannel->SetCsChannelState(kBeforeResponseReq);
      if(rIndex < m_maxCallChannel)
      {
        bRval = AllocateAndSetChannelParams(pCapBuffer, pMcChannel);
        if(bRval == FALSE)
        {
          PDELETE(pMcChannel);
          PASSERT(1);
        }
        else
        {
        OnPartyIncomingChnlResponseReq(pMcChannel, FALSE);
        }
        // The meaning of this flag is that when we finish the SIGNALING PART connection of this incoming channel
        // We will only update our inner records - As far as the Party is concerned - Nothing happened
        m_isRealIncVidChanSentFromCodianVcr = 1;
      }
      else
        PDELETE(pMcChannel);

    }
    else
    {
      TRACEINTO << "It is regular case";
      DWORD structureSize;

      if (pBaseInCap)
      {
        structureSize = pBaseInCap->SizeOf();//pInChnlInd->sizeOfChannelParams;
      }
      else
      {
        PASSERT(1);
        structureSize = 0;
      }

      if(structureSize == 0)
        structureSize = capInfo.GetH323CapStructSize();

      EResult eResOfSet = kSuccess;
      pCapBuffer = (capBuffer*)new BYTE[structureSize + sizeof(capBufferBase)];
      memset(pCapBuffer,0,(structureSize + sizeof(capBufferBase)));
      pCapBuffer->capLength = structureSize;
      pCapBuffer->capTypeCode = (CapEnum)capInfo;

      CBaseCap *pBaseCap   = CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE*)pCapBuffer->dataCap);

      if (pBaseCap && pBaseInCap)
      {
        if (capInfo.IsType(cmCapAudio) || capInfo.IsType(cmCapVideo) || capInfo.IsType(cmCapData))
        {
          eResOfSet &= pBaseCap->SetDefaults(cmCapReceive,eRole);
          eResOfSet &= pBaseCap->CopyQualities(*pBaseInCap);

          if(eResOfSet)
              m_pTargetModeH323->SetMediaMode(pCapBuffer,capInfo.GetCapType(),cmCapReceive,eRole, true);

          const capBuffer *pLocalCapBuffer = m_pLocalCapH323->GetFirstMediaCapBufferAccording2CapEnum((CapEnum)capInfo, eRole);
          if (pLocalCapBuffer != NULL)
            m_pTargetModeH323->SetMediaMode(pLocalCapBuffer,capInfo.GetCapType(),cmCapReceive,eRole, true);

          if(capInfo.IsType(cmCapData))
          {
            if(IsValidTimer(MCMSOPENDATACHANNELS))
              DeleteTimer(MCMSOPENDATACHANNELS);//the remote open the data chnl so we delete the timer
            m_pTargetModeH323->SetDataBitRate(m_pLocalCapH323->GetMaxDataRate());
          }

          if (eRole & kRoleContentOrPresentation &&  m_pTargetModeH323->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation)) //this is true also in case of duo
            SetContentTargetTransmitMode(capInfo, eRole);
        }
        else
          PTRACE(eLevelError,"CH323Cntl::OnH323IncomingChnlInd: Unknown alg!!!");
      }
      POBJDELETE(pBaseCap);


      int rIndex;
      CChannel *pMcChannel = new CChannel;
      CChannel *pMrmpChannel = NULL; AUTO_DELETE(pMrmpChannel);
      if (GetTargetMode()->GetConfMediaType() == eMixAvcSvc && eRole == kRolePeople && eType == cmCapVideo)
      {
          pMrmpChannel = new CChannel;
      }
      BOOL bIsOutgoingChannel = FALSE;
      BOOL bRval        = FALSE;
    WORD isLpr        = pInChnlInd->bIsLPR;
      //ON(m_OneOfTheMediaChannelWasConnected);

      DWORD bitRate = m_pTargetModeH323->GetMediaBitRate(eType);
      if(m_pTargetModeH323->GetConfType() == kCop && eRole == kRolePeople && eType == cmCapVideo)
      {
        CBaseCap *pChannelInCap = CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE *)pChannelParams);
        if(pChannelInCap)
        {
        bitRate = pChannelInCap->GetBitRate();
        POBJDELETE(pChannelInCap);
          PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlInd bitRate ",bitRate);
        }
        else
          PASSERTMSG(1, "AllocNewCap return NULL");


      }

      //The IP version must be checked!!!
      pMcChannel->SetRmtAddressVer(pInChnlInd->rmtRtpAddress.transAddr.ipVersion);
    // IpV6
    pMcChannel->SetRmtAddress(pInChnlInd->rmtRtpAddress.transAddr);

    rIndex = InitMcChannelParams(bIsOutgoingChannel,pMcChannel,capInfo,eRole,bitRate, pInChnlInd,status, channelIndex, isLpr);
      if(status == STATUS_OK)
        pMcChannel->SetCsChannelState(kBeforeResponseReq);

      //@#@ - need to add it anywhere
//      // do it for video channel only
//      TRACEINTOFUNC << "mix_mode: before OpenSvcChannel GetTargetMode()->GetConfMediaType()=" << GetTargetMode()->GetConfMediaType()
//               << " eRole=" << eRole << " eType=" << eType;
//      if (GetTargetMode()->GetConfMediaType() == eMixAvcSvc && eRole == kRolePeople && eType == cmCapVideo)
//      {
//          InitMcChannelParams(bIsOutgoingChannel,pMrmpChannel, eSvcCapCode ,eRole,bitRate,pInChnlInd,status, channelIndex, isLpr);
//          if(status == STATUS_OK)
//          {
//              pMrmpChannel->SetCsChannelState(kBeforeResponseReq);
//          }
//      }

      if (m_pmcCall->GetMasterSlaveStatus() == cmMSSlave)
      {
        if(pMcChannel->GetIsEncrypted())
        { // In this case we will create the session and encrypted session key with
          // Alg functions using the shared secret(Master key)
          APIU8 sessionKey[sizeOf128Key];
          memset(sessionKey,'0',sizeOf128Key);
          PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlInd - master key= ");
          PrintDHToTrace(sizeOf128Key,(BYTE *)m_pDHKeyManagement->GetEncrCallKey()->GetArray());

          BOOL isOpenSSLFunc = NO;
          CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_OPENSSL_ENC_FUNC, isOpenSSLFunc);
          if (isOpenSSLFunc != NO)
            DecryptCipherKeyOpenSSL(m_pDHKeyManagement->GetEncrCallKey()->GetArray(),pInChnlInd->EncryptedSession235Key , sessionKey);
          else
            DecryptCipherKey(m_pDHKeyManagement->GetEncrCallKey()->GetArray(),pInChnlInd->EncryptedSession235Key, sessionKey);


          PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlInd - session key= ");
          PrintDHToTrace(sizeOf128Key,(BYTE *)sessionKey);


          pMcChannel->SetH235SessionKey(sessionKey);
          pMcChannel->SetH235EncryptedSessionKey(pInChnlInd->EncryptedSession235Key);
          PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlInd - Encrypted session key = ");
          PrintDHToTrace(sizeOf128Key,(BYTE *)pInChnlInd->EncryptedSession235Key);
          if(!m_bIsAvaya)
          {
            PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlInd - Full key (taking the LSB) = ");
            PrintDHToTrace(HALF_KEY_SIZE,(BYTE *)m_pDHKeyManagement->GetDHResultSharedSecret()->GetArray());
          }
        }
      }
      if(rIndex < m_maxCallChannel)
      {
        bRval = AllocateAndSetChannelParams(pCapBuffer, pMcChannel);
        if(bRval == FALSE)
        {
          PDELETE(pMcChannel)
        }
        else
        {
        	// check if to keep the VSW stream
            if (GetTargetMode()->GetConfMediaType() == eMixAvcSvc && (eRole == kRolePeople && eType == cmCapVideo))
            {
            	GetTargetMode()->UpdateHdVswForAvcInMixMode(pBaseInCap);
            }


          if (Rtp_FillAndSendUpdatePortOpenRtpStruct(pMcChannel))
          {
              Cm_FillAndSendOpenUdpPortOrUpdateUdpAddrStruct(pMcChannel, pInChnlInd->rmtRtpAddress.transAddr);
              // if video and mix conference, open MRMP channel too
              TRACEINTOFUNC << "mix_mode: before OpenSvcChannel GetTargetMode()->GetConfMediaType()=" << GetTargetMode()->GetConfMediaType() << " eRole=" << eRole << " eType=" << eType;
              if (GetTargetMode()->GetConfMediaType() == eMixAvcSvc &&
                      ((eRole == kRolePeople && eType == cmCapVideo) || (eType == cmCapAudio)))
              {
                  OpenInternalChannelsByMedia(eType, bitRate, kConnecting);
//                  OpenSvcChannel(pMrmpChannel, pInChnlInd->rmtRtpAddress.transAddr); // @#@
              }
          }
          else
            m_pTaskApi->EncryptionDisConnect(FIPS140_STATUS_FAILURE);

          // request outgoing channel for all media
          // in simulation (cascade) we will enable to open channel only after we recive closing of incoming channel

          BYTE bOutChannelAlreadyOpen = FALSE;
          if (FindChannelInList(pMcChannel->GetType(), TRUE, eRole))
            bOutChannelAlreadyOpen = TRUE;

          if(!m_McmsOpenChannels )
          {
//            if( (pMcChannel->GetType() == cmCapAudio) && m_isIncomingAudioHasDisconnectedOnce && !m_pParty->IsRemoteCapsEcs())
//            {
//              DWORD index = GetChannelIndexInList(capInfo.GetCapType(),TRUE,eRole);
//              if (index < m_maxCallChannel)
//              {//need to close the audio outgoing channel
//                CloseOutgoingChannel(cmCapAudio);
//                if(IsValidTimer(OTHERMEDIACONNECTED))
//                  DeleteTimer(OTHERMEDIACONNECTED);
//                StartTimer(AUDCONNECTTOUT, 30*SECOND);
//                PDELETE(pCapBuffer);
//                PDELETEA(pInChnlInd);
//                return;
//              }
//            }

            BYTE bIsMcmsOrigin = FALSE;
            if (pMcChannel->GetRoleLabel() & kRoleContentOrPresentation)
              bIsMcmsOrigin = TRUE;
            BYTE isopen4cif = YES;
            if (eType == cmCapVideo)
            {

              if( pBaseInCap && pBaseInCap->GetCapCode() == eH264CapCode)
              {
                if( ((CH264VideoCap*)pBaseInCap)->IsCapableOfHD720() || ((CH264VideoCap*)pBaseInCap)->IsCapableOfHD1080() || ((CH264VideoCap*)pBaseInCap)->IsCapableOfHD720At50() || ((CH264VideoCap*)pBaseInCap)->IsCapableOfHD1080At60() )
                  isopen4cif = NO;
              }
              else if(NULL == pBaseInCap)
                PASSERTMSG(1, "pBaseInCap is NULL");
            }
            if(!bOutChannelAlreadyOpen)
              OnPartyOutgoingChannelReq( capInfo.GetIpCapCode(), pMcChannel->GetRoleLabel(),bIsMcmsOrigin,TRUE,FALSE,isopen4cif);
          }
        }
      }
      else
        PDELETE(pMcChannel);
    }
  }
  else // Channel must be reject
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323IncomingChnlInd - channel reject");
    BaseCapStruct *pChanCapStruct = (BaseCapStruct *)pInChnlInd->channelSpecificParams;

    cmCapDataType eType = capInfo.GetCapType();
    OnPartyIncomingChnlResponseReq(m_maxCallChannel, eType, rejectChannel,(ERoleLabel)pChanCapStruct->header.roleLabel,channelIndex);
  }

  PDELETEA(pCapBuffer);
  POBJDELETE(pBaseInCap);
  PDELETEA(pInChnlInd);

  //
}


////////////////////////////////////////////////////////////////////////////
// its better to put this function in the caps (which will be responsible to find the common protocol mode between the Party and the EP).
CapEnum CH323Cntl::GetCommonContentMode(ERoleLabel &eRole)
{
  CapEnum commonContentMode = eUnknownAlgorithemCapCode;
  APIS32  H264mode          = H264_standard;
  eRole = kRolePresentation;
  if (m_pLocalCapH323->IsH239() && m_pRmtCapH323->IsH239())
  {
    CBaseVideoCap* pScmCap = (CBaseVideoCap*)m_pTargetModeH323->GetMediaAsCapClass(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
    if (!pScmCap)
    {
        PTRACE(eLevelError, "CH323Cntl::GetCommonContentMode - pScmCap is NULL");
        return commonContentMode;
    }
    commonContentMode = pScmCap->GetCapCode();
    APIU16 uProfile = 0;
    //HP content:
    if ((commonContentMode == eH264CapCode) && !(m_pLocalCapH323->IsH264HighProfileContent() && m_pLocalCapH323->IsH264BaseProfileContent()))
        uProfile = ((CH264VideoCap*)pScmCap)->GetProfile();

	POBJDELETE(pScmCap);

    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
    if (pCommConf && !(pCommConf->GetIsPreferTIP()) && (commonContentMode == eH264CapCode) && (m_pTargetModeH323->IsTIPContentEnableInH264Scm() == TRUE))
    {
    	//just for TipCompatibility:video&content!
    	H264mode = H264_tipContent;
    }
    BYTE rValRemote   = m_pRmtCapH323->AreCapsSupportProtocol(commonContentMode, cmCapVideo, kRolePresentation, H264mode, uProfile);
    if(rValRemote)
    {
      return commonContentMode;
    }

    // check if we have additional content protocol in the local caps and compare it to remote content type.
    // need to find a better function in the cap that does this part of the code.
    if(commonContentMode == eH264CapCode)
      commonContentMode = eH263CapCode;
    else if(commonContentMode == eH263CapCode)
      commonContentMode = eH264CapCode;


    rValRemote      = m_pRmtCapH323->AreCapsSupportProtocol(commonContentMode, cmCapVideo, kRolePresentation, H264mode);
    BYTE rValLocal  = m_pLocalCapH323->AreCapsSupportProtocol(commonContentMode, cmCapVideo, kRolePresentation, H264mode);

    if(rValRemote && rValLocal)
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::GetCommonContentMode - remote and local support same mode.");
      return commonContentMode;
    }

    commonContentMode = eUnknownAlgorithemCapCode;
  }
  else if(m_pLocalCapH323->IsEPC() && m_pRmtCapH323->IsEPC())
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::GetCommonContentMode - EPC. Fixed mode H263");
    commonContentMode = eH263CapCode;
    eRole = kRoleContent;
  }
  return commonContentMode;
}

////////////////////////////////////////////////////////////////////////////
/* This function can support EPC and H239.
 * In fact it had a bug that the target SCM is set with role content although the function name is H239 which the role of it is presentation
 * But since in probably all of our calls we check for ContentOrPresentation its OK */
void CH323Cntl::SetContentTargetTransmitMode(CCapSetInfo capInfo, ERoleLabel eRole)
{
	 PTRACE(eLevelInfoNormal,"CH323Cntl::SetContentTargetTransmitMode");
	if ((eRole == kRolePresentation) && m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePresentation) )
		return;

	if ((eRole == kRoleContent) && m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapTransmit, kRoleContent) )
		return;

	BYTE doesPartyMeetConfContentHDResolution = TRUE;
	CBaseVideoCap* pXmitVideoContCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfo,NULL);
	if (pXmitVideoContCap)
	{// Here we check the local and remote caps
		cmCapDataType eType = capInfo.GetCapType();
		BOOL bHD1080PartyContentEnabled = FALSE;
		BYTE hdContentMpi = 0;
		BYTE hdRemoteContentMpi = m_pRmtCapH323->IsCapableOfHDContent1080();
		BYTE hdLocalContentMpi = m_pLocalCapH323->IsCapableOfHDContent1080();
		BOOL bIsHighProfileContent = m_pLocalCapH323->IsH264HighProfileContent() && m_pRmtCapH323->IsH264HighProfileContent();  //HP content

		//BYTE bContentAsVideo   = m_pTargetModeH323->GetIsShowContentAsVideo();
		DWORD contentRate = m_pLocalCapH323->GetMaxContRate();

		if(hdRemoteContentMpi && hdLocalContentMpi)
		{
			PTRACE(eLevelInfoNormal,"CH323Cntl::SetContentTargetTransmitMode add HD1080 to caps");
			bHD1080PartyContentEnabled = TRUE;
		}
		else
		{

            hdRemoteContentMpi = m_pRmtCapH323->IsCapableOfHDContent720();
            hdLocalContentMpi = m_pLocalCapH323->IsCapableOfHDContent720();

            //hdContentMpi = MAX(hdRemoteContentMpi,hdLocalContentMpi); //Anna - vngfe - 4822, to open content channels symmetrically
            //PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SetContentTargetTransmitMode add HD720 to caps, hdContentMpi - ",hdContentMpi);
		}

		hdContentMpi = MAX(hdRemoteContentMpi,hdLocalContentMpi); //Anna - vngfe - 4822, to open content channels symmetrically

		EResult eResOfSet;
		if (m_pTargetModeH323->IsTIPContentEnableInH264Scm() == TRUE)
		{
			eResOfSet = ((CH264VideoCap *)pXmitVideoContCap)->SetTIPContent(eRole, cmCapTransmit/*,bContentAsVideo*/);
		}
		else if ( m_pTargetModeH323->GetTipContentMode() == eTipCompatiblePreferTIP)
		{
			eResOfSet = ((CH264VideoCap *)pXmitVideoContCap)->SetTIPContent(eRole, cmCapTransmit/*,bContentAsVideo*/,FALSE);
		}
		else //just for TipCompatibility:video&content!
		{
			eResOfSet = pXmitVideoContCap->SetContent(eRole, cmCapTransmit, bHD1080PartyContentEnabled, hdContentMpi, bIsHighProfileContent);
		}

		eResOfSet &= pXmitVideoContCap->SetBitRate(contentRate);
		if(eResOfSet)
		{
/*
// This section is in comment until we'll have highets common in H239:
			if (eRole == kRolePresentation)
			{//H239: intersect local caps with remote caps
				const BaseCapStruct* pRemoteBestStruct = m_pRmtCapH323->GetBestCapStruct((CapEnum)capInfo, cmCapReceive, kRolePresentation);
				if (pRemoteBestStruct)
				{
					CBaseVideoCap* pRemoteBestCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE *)pRemoteBestStruct);
					if (pRemoteBestCap)
					{
						pRemoteBestCap->NullifySqcifMpi();

						int sizeOfNewStruct = pXmitVideoContCap->SizeOf();
						BYTE* pNewStruct	= new BYTE[sizeOfNewStruct];
						CBaseVideoCap* pIntersectVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfo, pNewStruct);
						if (pIntersectVideoCap)
						{
							EResult eResOfSet = kSuccess;
							eResOfSet &= pIntersectVideoCap->SetDefaults(cmCapTransmit, kRolePresentation);
							eResOfSet &= pIntersectVideoCap->CopyQualities(*pXmitVideoContCap);
							if (eResOfSet)
							{
								BaseCapStruct* pIntersectStruct = pIntersectVideoCap->GetStruct();
								pXmitVideoContCap->Intersection(*pRemoteBestCap, (BYTE**)(&pIntersectStruct) );
								BaseCapStruct* pPrevStruct = pIntersectVideoCap->GetStruct();
								if (pIntersectStruct != pPrevStruct)
									//it happens in case of 263+, in which Intersection does reallocate to pIntersectStruct,
									pIntersectVideoCap->SetStruct(pIntersectStruct);
							}
						}

						if (pIntersectVideoCap)
						{
							*pXmitVideoContCap = *pIntersectVideoCap;
							POBJDELETE(pIntersectVideoCap);
						}
					}
					POBJDELETE(pRemoteBestCap);
				}
			}
*/
			int sizeOfXmitContent =  pXmitVideoContCap->SizeOf();
			BYTE* pXmitContent = (BYTE *)pXmitVideoContCap->GetStruct();
			m_pTargetModeH323->SetMediaMode(capInfo,sizeOfXmitContent,pXmitContent,eType,cmCapTransmit,eRole, true);
			m_pTargetModeH323->Dump ("CH323Cntl::SetContentTargetTransmitMod" , eLevelInfoNormal);

			CMedString strBase;
			strBase<< m_pTargetModeH323->GetMediaBitRate(cmCapVideo, cmCapReceive,kRoleContentOrPresentation) <<
					", m_pTargetModeH323 content rate (Tx)- " << m_pTargetModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation);
			PTRACE2(eLevelInfoNormal,"CH323Cntl::SetContentTargetTransmitMode :  m_pTargetModeH323 content rate (Rcv)- ", strBase.GetString());

			pXmitVideoContCap->FreeStruct();
			BYTE presentationProtocol = m_pTargetModeH323->GetContentProtocolMode();
				DWORD possibleContentRate = m_pRmtCapH323->GetMaxContentBitRate();
				CCommConf* pCommConf = NULL;
				pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
				if (pCommConf)
				{
					//TIP
					if ((pCommConf->GetIsTipCompatible() == eTipCompatibleVideoAndContent || pCommConf->GetIsTipCompatible() == eTipCompatiblePreferTIP) )
					{
					//	TRACEINTO << "confIPContentRate= " << confIPContentRate<< " possibleContentRate= " <<possibleContentRate ;
						BOOL bIsTipLegacy = IsPartyLegacyForTipContent(possibleContentRate) ;
						if(bIsTipLegacy)
						{
							m_pTargetModeH323->SetMediaOff(cmCapVideo,cmCapReceiveAndTransmit,kRoleContentOrPresentation);
							PTRACE(eLevelInfoNormal, "CH323Cntl::SetContentTargetTransmitMode -TIP legacy! - set content mode off");
						}
						POBJDELETE(pXmitVideoContCap);
						return;
					}

					DWORD confContentRate = m_pTargetModeH323->GetMediaBitRate(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
					eEnterpriseMode ContRatelevel = (eEnterpriseMode)pCommConf->GetEnterpriseMode();
					BYTE lConfRate = pCommConf->GetConfTransferRate();
					CUnifiedComMode localUnifedCommMode(pCommConf->GetEnterpriseModeFixedRate(),lConfRate,pCommConf->GetIsHighProfileContent());
					DWORD actualPartyContentRate  = 0;
					DWORD confIPContentRate = localUnifedCommMode.GetContentModeAMCInIPRate(lConfRate,ContRatelevel,
								(ePresentationProtocol)pCommConf->GetPresentationProtocol(),
								pCommConf->GetCascadeOptimizeResolution(), pCommConf->GetConfMediaType());
					actualPartyContentRate  = min(confContentRate,possibleContentRate);

					if(presentationProtocol == eH264Fix || presentationProtocol == eH264Dynamic)
					{
					   BYTE isHD720content = m_pRmtCapH323->IsCapableOfHDContent720();
					   if(!isHD720content)
					   {
						  OFF(doesPartyMeetConfContentHDResolution);
					   }
					   else
					   {
						  if(presentationProtocol == eH264Fix)
						  {
							 BYTE HD720ConfMpi =  m_pTargetModeH323->isHDContent720Supported(cmCapTransmit);
							 BYTE HD1080ConfMpi=  m_pTargetModeH323->isHDContent1080Supported(cmCapTransmit);
							 BYTE HDConfMPI = HD1080ConfMpi;
							 BYTE isConfSupportingHD1080 = TRUE;

							 if(!bHD1080PartyContentEnabled  && HD1080ConfMpi)
							 {
								 OFF(doesPartyMeetConfContentHDResolution);
								 HDConfMPI = HD720ConfMpi;
							 }

							 if(!HD1080ConfMpi)
							 {
								 HDConfMPI = HD720ConfMpi;
								 OFF(isConfSupportingHD1080);
							 }

							 if(hdContentMpi > HDConfMPI)
							 {
								 OFF(doesPartyMeetConfContentHDResolution);
							 }
							 if(!doesPartyMeetConfContentHDResolution)
							 {
								 CLargeString pstr;
								 pstr << "\n Party Name: " << PARTYNAME;
								 pstr << "\n Is Party Supporting HD Content 1080: " << bHD1080PartyContentEnabled << ", HD Party MPI: " << hdContentMpi;
								 pstr << "\n Is Conference Supporting HD Content 1080: " << isConfSupportingHD1080 << ", HD Conf MPI: " << HDConfMPI;
								 PTRACE2(eLevelInfoNormal,"CH323Cntl::SetContentTargetTransmitMode, Party does not meet Conf Content HD requirements: ",pstr.GetString());
							 }
						  }
					}

					   DWORD contentThresholdRate = 0;
					BOOL isPartyMeetContentRateThreshold = ::isPartyMeetContentRateThreshold(confIPContentRate/10,actualPartyContentRate/10,pCommConf->GetEnterpriseMode(),pCommConf->GetPresentationProtocol(), contentThresholdRate);
				   if( !doesPartyMeetConfContentHDResolution || !isPartyMeetContentRateThreshold)
				   {
					   m_pTargetModeH323->SetMediaOff(cmCapVideo,cmCapReceiveAndTransmit,kRoleContentOrPresentation);
					   PTRACE(eLevelInfoNormal, "CH323Cntl::SetContentTargetTransmitMode - h264 content set content mode off");
				   }
				}
			}
		}
	}

	POBJDELETE(pXmitVideoContCap);
	//We don't update here the rate in Rh323, because in this point, no stream on will
	// be sent until the funcion SetNewVideoRateInEPCOrDuoCall will be called.
	// In this function, this rate will be updated.
}

////////////////////////////////////////////////////////////////////////////
BOOL CH323Cntl::IsRemoteHasContentXGA()
{ // check if remote supports in TIP content (fix with 512k XGA).
	 WORD maxMBPS = 0;
	 WORD maxFS = 0;
	 WORD maxDPB = 0;
	 WORD maxBRandCPB = 0;
	 WORD maxSAR = 0;
	 WORD maxStaticMB=0;
	 ERoleLabel eRole = kRoleContentOrPresentation;
	 DWORD profile = 0 ;
	 m_pRmtCapH323->GetMaxH264CustomParameters(m_pRmtCapH323->GetMaxH264Level(eRole), maxMBPS, maxFS, maxDPB, maxBRandCPB, maxSAR, maxStaticMB, eRole,profile);

	// check if EP is supported in XGA (XGA =>  3072FS / 256FACTOR = 12)
	 TRACEINTO << "remotemaxFS = " <<maxFS ;
	return (maxFS >= H264_XGA_FS_AS_DEVISION) ? true : false ;
}

//////////////////////////////////////////////////////////////////////////
BOOL CH323Cntl::IsPartyLegacyForTipContent (DWORD partyContentRate)
{
	DWORD tipContentRate = CUnifiedComMode::TranslateAMCRateIPRate(AMC_512k); //BRIDGE-15334 - 512k instead of 1024k.
	BOOL bIsRemoteHasContentXGA = IsRemoteHasContentXGA() ;

	TRACEINTO << "partyContentRate " << partyContentRate << " IsRemoteHasContentXGA() " << (DWORD)bIsRemoteHasContentXGA ;

	//Tip content legacy will be if the EP doesn't meet TIP content requirments (0.5M XGA)
	if((partyContentRate < tipContentRate || !bIsRemoteHasContentXGA)  )
	{
		TRACEINTO << "Remote doesn't meet TIP content treshold. Setting to content legacy";
		return TRUE ;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::SaveIncomingChannelForFurtherUse(mcIndIncomingChannel *pInChnlInd,APIU32 callIndex,APIU32 channelIndex,APIU32 mcChannelIndex,APIU32 stat1,APIU16 srcUnitId)
{
  channelSpecificParameters* pChannelParams = (channelSpecificParameters *)&(pInChnlInd->channelSpecificParams);

  CCapSetInfo capInfo = (CapEnum)pInChnlInd->capTypeCode;
  BYTE  rejectChannel = FALSE;

  CBaseCap* pChannelInCap = CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE *)pChannelParams); AUTO_DELETE(pChannelInCap);
  if (pChannelInCap == NULL)
  {
    PASSERTMSG(m_pmcCall->GetConnectionId(),"CH323Cntl::SaveIncomingChannelForFurtherUse - pChannelInCap == NULL");
    return FALSE;
  }

  cmCapDataType eType = capInfo.GetCapType();
  ERoleLabel    eRole = pChannelInCap->GetRole();
    CChannel* currInChannel = FindChannelInList(eType, FALSE, eRole);
    if (currInChannel) // that means that this channel is the second video in channel
    {
    if( (currInChannel->IsCsChannelStateDisconnecting()))
    {
      BYTE indexOfMediaTypeInArray=ConvertChannelTypeEnumToArrayIndex(eType,eRole);
        PTRACE2INT(eLevelInfoNormal, "CH323Cntl::SaveIncomingChannelForFurtherUse - Save the channel until the closing of the previous one -  ", m_pCsRsrcDesc->GetConnectionId());
      m_pNewInChanSeg[indexOfMediaTypeInArray] = new CSegment;
      *(m_pNewInChanSeg[indexOfMediaTypeInArray])<<(APIU32)callIndex<<(APIU32)channelIndex<<(APIU32)mcChannelIndex<<(APIU32)stat1<<(APIU16)srcUnitId;
      (m_pNewInChanSeg[indexOfMediaTypeInArray])->Put((BYTE*)pInChnlInd, sizeof(mcIndIncomingChannel)+pInChnlInd->sizeOfChannelParams);
        return TRUE;
      }
    }

  POBJDELETE(pChannelInCap);
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// this function check if the incoming channel need to be rejected
// return TRUE for reject
// FALSE to accept
BYTE CH323Cntl::RejectIncomingChannelIfNeeded(mcIndIncomingChannel *pInChnlInd, APIS32 status, APIU32 channelIndex, APIU32 mcChannelIndex)
{
  if (m_encAlg != kUnKnownMediaType)
  {
    if(pInChnlInd->bIsEncrypted == FALSE) //the channel is not encrypted
    {
      PTRACE(eLevelError,"CH323Cntl::RejectIncomingChannelIfNeeded - channel is not encrypted - REJECT");
      return TRUE;
    }
    else if(m_encAlg != pInChnlInd->encryptionAlgorithm) //different encryption type
    {
      PTRACE(eLevelError,"CH323Cntl::RejectIncomingChannelIfNeeded - differnt encryption type - REJECT");
      return TRUE;
    }
  }

  if (m_encAlg == kUnKnownMediaType && (!m_pTargetModeH323->GetIsEncrypted()) && pInChnlInd->bIsEncrypted)
  {
    PTRACE(eLevelError,"CH323Cntl::RejectIncomingChannelIfNeeded - ENCRYPTION CHANNEL AND NO ENCRYPTION CONF - REJECT");
    return TRUE;
  }
     //In case of ECS, there are EPs that try to reopen the incoming channel, before the caps exchane is finished:
  if (m_pParty->IsRemoteCapsEcs() && (m_CapabilityNegotiation != kCompleteCapNegotiation))
    return TRUE;
  channelSpecificParameters *pChannelParams = (channelSpecificParameters *)&(pInChnlInd->channelSpecificParams);

  CCapSetInfo capInfo = (CapEnum)pInChnlInd->capTypeCode;
  BYTE  rejectChannel = FALSE;

  CBaseCap *pChannelInCap = CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE *)pChannelParams);
  if (pChannelInCap == NULL)
  {
    PASSERTMSG(m_pmcCall->GetConnectionId(),"CH323Cntl::RejectIncomingChannelIfNeeded - pChannelInCap == NULL");
    return TRUE;
  }

  cmCapDataType eType = capInfo.GetCapType();
  ERoleLabel    eRole = pChannelInCap->GetRole();

  char* msgStrReason = new char[MediumPrintLen];

  // check if the incoming channel is a known one
  if (capInfo.GetIpCapCode() == eUnknownAlgorithemCapCode)
  {
    DBGPASSERT(m_pCsRsrcDesc->GetConnectionId());
    TRACEINTO << ", RejectReason = Unknown Algorithm";
    rejectChannel = TRUE;
  }

  if (eType == cmCapAudio)
  {
    int framePerPacket = pChannelInCap->GetFramePerPacket();

    if(capInfo.CheckFramePerPacket(framePerPacket) == FALSE)
    {
      PTRACE2(eLevelError, "CH323Cntl::RejectIncomingChannelIfNeeded - The framePerPacket is not as expected (not Quotient). Name - ", PARTYNAME);
    }
    if (capInfo.GetMaxFramePerPacket() == NonFrameBasedFPP)
    {
      // If NonFrameBasedFPP Audio alg. it should be minimum of 10 FPP and divide exactly by 10.
      // Condition 1 is part of Condition 2 but we decided to separate them in order to emphasize the validity check.
      if (((audioCapStructBase*)pChannelParams)->maxValue < 10)
      {
        PTRACE2(eLevelError, "CH323Cntl::RejectIncomingChannelIfNeeded - The framePerPacket is not as expected (Less then 10 FPP). Name - ", PARTYNAME);
        rejectChannel = TRUE;
      }
      DWORD resMod =((audioCapStructBase*)pChannelParams)->maxValue%10;
      if (resMod != 0)
      {
        PTRACE2(eLevelError, "CH323Cntl::RejectIncomingChannelIfNeeded - The framePerPacket (NonFrameBasedFPP) does not divide by 10. Name - ", PARTYNAME);
        rejectChannel = TRUE;
      }
    }

  }

  // check if the incoming channel arrives when the call is closing
  if(m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    TRACEINTO << ", RejectReason = bIsClosing process";
    rejectChannel = TRUE;
  }

  //In case the incoming channel was rejected by the stack controler
  else if (status != STATUS_OK)
  {
    TRACEINTO << ", RejectReason = resource problem " << status;
    rejectChannel = TRUE;

    if (eType == cmCapAudio)
      m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_NO_PORT_LEFT_FOR_AUDIO);
    else if (eType == cmCapVideo)
    {
      if (eRole & kRoleContentOrPresentation)
        m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_NO_PORT_LEFT_FOR_VIDEOCONT);
      else
        m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_NO_PORT_LEFT_FOR_VIDEO);
    }
    else if (eType == cmCapData)
      m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_NO_PORT_LEFT_FOR_FECC);

    DBGPASSERT(m_pCsRsrcDesc->GetConnectionId());
  }
  //Identify if it is a duo channel:
  if ((eType == cmCapVideo) && (eRole == kRolePeople) )
  {
    CChannel* inVideoChannel = FindChannelInList(eType, FALSE, eRole);
    if (inVideoChannel) // that means that this channel is the second video in channel
    {
      if (!(inVideoChannel->IsCsChannelStateDisconnecting()))
      {
        //we allow second video channel only in case of a duo conference
        rejectChannel = TRUE;
      }


    }
  }

  if (eType == cmCapVideo)
  {
    if (pChannelInCap->GetBitRate() <= 0)
    {
      rejectChannel = TRUE;
      TRACEINTO << ", RejectReason = " << "channel rate <= 0";
    }
  }

  DWORD index = GetChannelIndexInList(true, eType, FALSE, eRole);
  // check if the incoming channel is already open
  if(index < m_maxCallChannel  && (rejectChannel == FALSE))
  {
    TRACEINTO << ", RejectReason = " << "channel already open";
    rejectChannel = TRUE;
  }

  // check if the parameters are valid for regular calls.
  if (rejectChannel == FALSE)
  {
    CSecondaryParams secParams;
    BYTE isAnnexes =TRUE;


    rejectChannel = !m_pLocalCapH323->IsChannelsParamsOK(pChannelParams, capInfo.GetPayloadType() ,secParams, pInChnlInd->channelName,isAnnexes);

    if(rejectChannel)
    {
      TRACEINTO << ", RejectReason = " << "wrong parameters";
      if(capInfo.IsType(cmCapVideo))
      {
        if(eRole & kRoleContentOrPresentation)
          m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_H239_RMT_DIFF_CAPCODE, secParams);
        else
          m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_RMT_DIFF_CAPCODE, secParams);
      }
    }
  }

  // check if the parameters are valid for SWCP calls.

  //take care if we are rejecting incoming video channel - this function is only for incoming channels
  if (rejectChannel == TRUE && eType == cmCapVideo && eRole == kRolePeople)
    m_bVideoInRejected = TRUE;

  // LPR
  if (eType != cmCapVideo)
  {
    if (pInChnlInd->bIsLPR != 0)
    {
      TRACEINTO << ", RejectReason = " << "LPR support on non LPR media";
      rejectChannel = TRUE;
      DBGPASSERT(m_pmcCall->GetConnectionId());

    }
  }
  if (!m_pRmtCapH323->IsLPR())
  {
    if (pInChnlInd->bIsLPR != 0)
    {
      TRACEINTO << ", RejectReason = " << "LPR support on channel but not supported by remote caps";
      rejectChannel = TRUE;
      DBGPASSERT(m_pmcCall->GetConnectionId());

    }
  }
  if (m_pRmtCapH323->IsLPR() && !m_pTargetModeH323->GetIsLpr())
  {
    if (pInChnlInd->bIsLPR != 0)
    {
      TRACEINTO << ", RejectReason = " << "LPR supported by remote caps but not by RMX";
      rejectChannel = TRUE;
      DBGPASSERT(m_pmcCall->GetConnectionId());

    }
  }

  // if we reject lets print the information
  if(rejectChannel)
  {
    TRACEINTO << "CH323Cntl::RejectIncomingChannelIfNeeded \n" << "  Party Name " << PARTYNAME
    << ", McmsConnId = " << m_pCsRsrcDesc->GetConnectionId()
    << ", mcms ChannelIndex = " << channelIndex
    << ", ChannelIndex = " << mcChannelIndex
    << ", CapCode = " << capInfo.GetH323CapName();
  }

  POBJDELETE(pChannelInCap);
  PDELETEA (msgStrReason);
  return rejectChannel;
}

////////////////////////////////////////////////////////////////////////////
BOOL CH323Cntl::AllocateAndSetChannelParams(capBuffer *pCapBuffer, CChannel *pMcChannel)
{
  BOOL res = TRUE;
  res = AllocateAndSetChannelParams(pCapBuffer->capLength,(char *)&pCapBuffer->dataCap,pMcChannel);

  return res;
}

////////////////////////////////////////////////////////////////////////////
BOOL CH323Cntl::AllocateAndSetChannelParams(int size,char *pData, CChannel *pMcChannel)
{
  // allocate the channel params structure
    PTRACE2INT (eLevelInfoNormal, "CH323Cntl::AllocateAndSetChannelParams - size:", size);

  pMcChannel->SetChannelParams(size,pData);
  if(!pMcChannel->GetChannelParams())
  {
    PASSERTMSG(m_pCsRsrcDesc->GetConnectionId(), "CH323Cntl::AllocateAndSetChannelParams - No channel params");
    return FALSE;
  }
  return TRUE;
}


////////////////////////////////////////////////////////////////////////////
// this function Initialize the McChannel parameters and insert it to the m_pmcCall->pChannelsArray
// return 0 when failed
// and none zero for success
int CH323Cntl::InitMcChannelParams(BOOL bIsOutgoingChannel, CChannel *pMcChannel, CCapSetInfo capInfo,ERoleLabel eRole,
                   DWORD rate, mcIndIncomingChannel *pinChnlInd, APIS32 status, APIU32 CsChannelIndex, APIU16 isLpr)
{
  int i = 0;
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::InitMcChannelParams - CsChannelIndex - ", CsChannelIndex);

  if ((CapEnum)capInfo == eSvcCapCode)
  {
//      m_pmcCall->SetMrmpChannel(pMcChannel);
      TRACEINTO << "mix_mode: I should not get here!!!";
  }
  else
  {
      // enter the channel structure to the channel array
      for (i = 0; i < m_maxCallChannel; i++)
      {
        if (m_pmcCall->GetSpecificChannel(i) == pMcChannel)  // chanel already exist
          break;
        if (!m_pmcCall->GetSpecificChannel(i))
        {
          m_pmcCall->SetSpecificChannel(i,pMcChannel);
          m_pmcCall->IncreaseChannelsCounter();
          break;
        }
      }

      if (i >= m_maxCallChannel)
      {
        PTRACE(eLevelError,"CH323Cntl::InitMcChannelParams: channel number exceeded");
        PASSERT(m_pCsRsrcDesc->GetConnectionId());
        return i;
      }
  }

  pMcChannel->SetIndex((DWORD)i+1);
  pMcChannel->InitChannelParams(m_pmcCall, bIsOutgoingChannel, capInfo, eRole, rate, m_pLocalCapH323->GetMaxDataRate(), pinChnlInd, status, CsChannelIndex, isLpr, m_encAlg);

  /*
  pMcChannel->SetChannelParams(0,NULL);
  pMcChannel->SetCallPointer(m_pmcCall);
  pMcChannel->SetCsChannelState(kConnectingState);
  pMcChannel->SetStreamState(kStreamOffState);
  pMcChannel->SetChannelCloseInitiator((DWORD)NoInitiator);
  pMcChannel->SetMaxSkew(0);
  pMcChannel->SetChannelDirection(bIsOutgoingChannel);    //  Pm response
  pMcChannel->SetPayloadType(capInfo.GetPayloadType());
  pMcChannel->SetCapNameEnum(capInfo);
  pMcChannel->SetType(capInfo.GetCapType());
  pMcChannel->SetRoleLabel(eRole);
  pMcChannel->SetIsRejectChannel(FALSE);
  pMcChannel->SetIsStreamOnSent(FALSE);
  pMcChannel->SetIsEncrypted(FALSE);
  pMcChannel->SetEncryptionType(kUnKnownMediaType);

  // LPR
  pMcChannel->SetIsLprSupported(isLpr);

  if(bIsOutgoingChannel)
  {
    pMcChannel->SetSizeOfChannelParams(0);
    pMcChannel->SetIsActive(TRUE);
    pMcChannel->SetSessionId(-1);
    pMcChannel->SetCsIndex(0);
    pMcChannel->SetStatus(0);
    if(m_encAlg != kUnKnownMediaType)
    {
      pMcChannel->SetIsEncrypted(TRUE);
      pMcChannel->SetEncryptionType(m_encAlg);
    }
  }
  else
  {
    pMcChannel->SetSizeOfChannelParams(pinChnlInd->sizeOfChannelParams);
    if(pinChnlInd->bIsActive)
      pMcChannel->SetIsActive(TRUE);
    else
      pMcChannel->SetIsActive(FALSE);

    pMcChannel->SetSessionId(pinChnlInd->sessionId);
    pMcChannel->SetCsIndex(CsChannelIndex);
    pMcChannel->SetStatus(status);
    pMcChannel->SetIsEncrypted(pinChnlInd->bIsEncrypted);
    pMcChannel->SetEncryptionType((EenMediaType)pinChnlInd->encryptionAlgorithm);
    pMcChannel->SetDynamicPayloadType(pinChnlInd->dynamicPayloadType);
    pMcChannel->SetIsH263Plus(pinChnlInd->bUsedH263Plus);
  }

  if ((capInfo.GetPayloadType() != _AnnexQ) && (capInfo.GetPayloadType() != _RvFecc))
    pMcChannel->SetRate(rate);
  else
    pMcChannel->SetRate(m_pLocalCapH323->GetMaxDataRate());
*/
  return i;
}

///////////////////////////////////////////////////////////////////////////////////
int CH323Cntl::UpdateMcChannelParams(CChannel *pMcChannel, CCapSetInfo capInfo, DWORD rate)
{
  int i = 0;
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::UpdateMcChannelParams - CsChannelIndex - ", pMcChannel->GetCsIndex());
  // search the channel structure in the channel array
  for (i = 0; i < m_maxCallChannel; i++)
  {
    if (m_pmcCall->GetSpecificChannel(i) == pMcChannel)
      break;
  }

  if (i >= m_maxCallChannel)
  {
    PTRACE(eLevelError,"CH323Cntl::UpdateMcChannelParams: channel not exist");
    PASSERT(m_pCsRsrcDesc->GetConnectionId());
    return i;
  }
  CChannel *pSameSessionChannel = FindChannelInList(pMcChannel->GetType(),
      !pMcChannel->IsOutgoingDirection(), (ERoleLabel)pMcChannel->GetRoleLabel());
  pMcChannel->SetPayloadType(capInfo.GetPayloadType());
    AllocateDynamicPayloadType(pMcChannel, capInfo);

  //  if (pSameSessionChannel && CiscoGW == m_remoteIdent)
//    {
//      APIU8 dynamicPayload = pSameSessionChannel->GetPayloadType();
//      PTRACE2INT (eLevelInfoNormal, "CH323Cntl::UpdateMcChannelParams - cisco GW incoming channel pt: ",pSameSessionChannel->GetPayloadType());
//      pMcChannel->SetPayloadType(dynamicPayload);
//    }
//    else
//      pMcChannel->SetPayloadType(capInfo.GetPayloadType());


  pMcChannel->SetCapNameEnum(capInfo);
  pMcChannel->SetType(capInfo.GetCapType());

  if ((capInfo.GetPayloadType() != _AnnexQ) && (capInfo.GetPayloadType() != _RvFecc))
    pMcChannel->SetRate(rate);
  else
    pMcChannel->SetRate(m_pLocalCapH323->GetMaxDataRate());

  return i;
}

///////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyIncomingChnlResponseReq(CChannel* pChannel, BYTE bRejectChannel)
{
  DWORD arrayIndex    = GetChannelIndexInList(pChannel);
  cmCapDataType eType = pChannel->GetType();
  ERoleLabel eRole    = pChannel->GetRoleLabel();
  OnPartyIncomingChnlResponseReq(arrayIndex, eType, bRejectChannel, eRole);

}


///////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::sendFlowControlIfRateExceeded()
{
  // check for video channel + connected
//  PTRACE(eLevelInfoNormal,"CH323Cntl::sendFlowControlIfRateExceeded - ");
  CChannel* pVideoChannel =  FindChannelInList(cmCapVideo,FALSE,kRolePeople);
  if (!pVideoChannel || pVideoChannel->GetCsChannelState() != kConnectedState)
  {
    //no video channel -> nothing to do
    PTRACE(eLevelInfoNormal,"CH323Cntl::sendFlowControlIfRateExceeded - no video channel ");
    return;
  }
//  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::sendFlowControlIfRateExceeded - pVideoChannel->GetRate(); ",pVideoChannel->GetRate());
  // check for audio channel + connected
  CChannel* pAudioChannel =  FindChannelInList(cmCapAudio,FALSE,kRolePeople);
  if (!pAudioChannel || pAudioChannel->GetCsChannelState() != kConnectedState)
  {
    //no audio channel -> nothing to do
    return;
  }

  DWORD videoRate = pVideoChannel->GetRate();
  DWORD audioRate = (pAudioChannel->GetRate())*10;
  DWORD call_rate = m_pTargetModeH323->GetTotalVideoRate();
  // VNGFE-6950
  if(call_rate <= audioRate)
  {
	  CMedString msg;
	  msg << "CH323Cntl::sendFlowControlIfRateExceeded - call_rate <= audioRate : call_rate = " << videoRate << " , audio rate = " << audioRate;
	  PTRACE(eLevelInfoNormal,msg.GetString());
	  return;
  }
  if (videoRate + audioRate > call_rate)
  {
    DWORD newVideoBitRate = call_rate - audioRate;
    m_pCurrentModeH323->SetVideoBitRate(newVideoBitRate, cmCapReceive, kRolePeople);
      CMedString msg;
      msg << "CH323Cntl::sendFlowControlIfRateExceeded - incoming video rate["<<videoRate<<"]+audio rate["<<audioRate<<"]=["<<videoRate+audioRate<<"] exceeds call rate["<<call_rate<<"], updating max rate to[" << newVideoBitRate << "]";
    PTRACE(eLevelInfoNormal,msg.GetString());
    SendFlowControlReq(cmCapVideo, FALSE,kRolePeople, newVideoBitRate);
  }

}

///////////////////////////////////////////////////////////////////////////////////
//The parameter channelIndexForReject is used only in case of rejecting a non-duo channel
void CH323Cntl::OnPartyIncomingChnlResponseReq(DWORD arrayIndex, cmCapDataType eType, BYTE rejectChannel,ERoleLabel label,APIU32 channelIndexForReject)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyIncomingChnlResponseReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  APIS32 status = 0;

  mcReqIncomingChannelResponse *pInChnlResponseReq = new mcReqIncomingChannelResponse;

  PASSERT_AND_RETURN(!pInChnlResponseReq);

  memset(pInChnlResponseReq, 0, sizeof(mcReqIncomingChannelResponse));

  CChannel* pMcChannel = m_pmcCall->GetSpecificChannel(arrayIndex);

  if (m_pmcCall->GetIsClosingProcess() && !rejectChannel) //if we reject only because of closing call, and not because of channel params:
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnPartyIncomingChnlResponseReq bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    status = -1;
    //In case we send open_udp and after that the call has disconnected, we will reject the channel and send close_udp.
    //When we will get the close_udp_ack, we need to do nothing regarding the cs, because we now reject the channel towards cs.
    //Therefore, we won't get start_close_channel from cs on this channel.
    //Need to notice that in usual reject channel we remove the channel from the array. But in this case we don't do it,
    //because the cm has to answer on close_udp_req. Therfore, we assign a specific state for this channel.
    if (pMcChannel && pMcChannel->GetCmUdpChannelState() == kRecieveOpenAck)
      pMcChannel->SetCsChannelState(kNoNeedToDisconnect);
  }

  memset(pInChnlResponseReq, 0, sizeof(mcReqIncomingChannelResponse));
  // IpV6
  enIpVersion eIpAddrMatch = CheckForMatchBetweenPartyAndUdp(m_pH323NetSetup->GetIpVersion(),m_UdpAddressesParams.IpType);
  pInChnlResponseReq->localRtpAddressIp.unionProps.unionType = eIpAddrMatch;
  pInChnlResponseReq->localRtpAddressIp.unionProps.unionSize = sizeof(ipAddressIf);

  if (!rejectChannel && pMcChannel)
  {
    //Inorder to prevent sending twice Response msg to CS for the same channel
    if(pMcChannel->GetCsChannelState() == kBeforeResponseReq)
    {
    BYTE bIsSupportDBC2     = IsSupportingDBC2(pMcChannel);
    pMcChannel->SetIsDbc2(bIsSupportDBC2);
      pInChnlResponseReq->bIsEncrypted    = pMcChannel->GetIsEncrypted();
      pInChnlResponseReq->encryptionAlgorithm = pMcChannel->GetEncryptionType();

      if (m_pmcCall->GetMasterSlaveStatus() == cmMSMaster)
      {
        if(pMcChannel->GetIsEncrypted())
        {
          memcpy(&(pInChnlResponseReq->EncryptedSession235Key),pMcChannel->GetH235EncryptedSessionKey(),sizeOf128Key);
          PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyIncomingChnlResponseReq session key = ");
          PrintDHToTrace(LengthEncAuthKey,(BYTE *)pInChnlResponseReq->EncryptedSession235Key);
          if(!m_bIsAvaya && m_encAlg != kUnKnownMediaType)
          {
            if((m_pDHKeyManagement->GetDHResultSharedSecret() ) && ( (BYTE *)m_pDHKeyManagement->GetDHResultSharedSecret()->GetArray()) )
            {
              PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyIncomingChnlResponseReq - Full key (taking the LSB) = ");
              PrintDHToTrace(LengthEncAuthKey,(BYTE *)m_pDHKeyManagement->GetDHResultSharedSecret()->GetArray());
            }
          }
        }
        else
          memset(&(pInChnlResponseReq->EncryptedSession235Key),'0',sizeOf128Key);
      }
      else
        memset(&(pInChnlResponseReq->EncryptedSession235Key),'0',sizeOf128Key);

      pInChnlResponseReq->channelIndex = pMcChannel->GetCsIndex();
      pInChnlResponseReq->channelDirection = pMcChannel->IsOutgoingDirection();
      pInChnlResponseReq->dataType = pMcChannel->GetType();
      pInChnlResponseReq->dynamicPayloadType = pMcChannel->GetDynamicPayloadType();

    pInChnlResponseReq->localRtpAddressIp.transAddr.ipVersion = eIpAddrMatch;
      if (pInChnlResponseReq->localRtpAddressIp.transAddr.ipVersion == eIpVersion4)
      {// Case IpV4
    	  std::string strCloudIp = GetCloudIp();
       	  //std::string strCloudIp = GetSystemCfgFlagStr<std::string>(m_serviceId, CFG_KEY_CLOUD_IP);
		  if (strCloudIp.length() > 0)
			  pInChnlResponseReq->localRtpAddressIp.transAddr.addr.v4.ip = ::SystemIpStringToDWORD(strCloudIp.c_str());
		  else
      pInChnlResponseReq->localRtpAddressIp.transAddr.addr.v4.ip = m_UdpAddressesParams.IpV4Addr.ip;
      }
      else
      { // Case IpV6
      // --- UDP: array of addresses ---

      BYTE  place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(m_pH323NetSetup->GetTaDestPartyAddr(), m_UdpAddressesParams.IpV6AddrArray);

      pInChnlResponseReq->localRtpAddressIp.transAddr.addr.v6.scopeId = m_UdpAddressesParams.IpV6AddrArray[place].scopeId;
      memcpy(pInChnlResponseReq->localRtpAddressIp.transAddr.addr.v6.ip,m_UdpAddressesParams.IpV6AddrArray[place].ip,16);

      }
      pInChnlResponseReq->localRtpAddressIp.transAddr.distribution = eDistributionUnicast;
      pInChnlResponseReq->localRtpAddressIp.transAddr.transportType = eTransportTypeUdp;
      switch(pInChnlResponseReq->dataType)
      {
        case cmCapAudio:
        {
          pInChnlResponseReq->localRtpAddressIp.transAddr.port = m_UdpAddressesParams.AudioChannelPort;
          break;
        }
        case cmCapData:
        {
          pInChnlResponseReq->localRtpAddressIp.transAddr.port = m_UdpAddressesParams.FeccChannelPort;
          break;
        }
        case cmCapVideo:
        {
          if (label == kRolePeople)
          {
            pInChnlResponseReq->localRtpAddressIp.transAddr.port = m_UdpAddressesParams.VideoChannelPort;
            break;
          }
          else
          {
            pInChnlResponseReq->localRtpAddressIp.transAddr.port = m_UdpAddressesParams.ContentChannelPort;
            break;
          }
        }
        default:
        {
          PASSERTMSG(pInChnlResponseReq->dataType,"CH323Cntl::IncomingChnlResponseReq - Channel Type");
          break;
        }
      }
      CSegment* pMsg = new CSegment;
      pMsg->Put((BYTE*)(pInChnlResponseReq),sizeof(mcReqIncomingChannelResponse));
      m_pCsInterface->SendMsgToCS(H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ,pMsg,m_serviceId,
        m_serviceId,m_pDestUnitId,m_callIndex,pMcChannel->GetCsIndex(),pMcChannel->GetIndex(),status);

      pMcChannel->SetCsChannelState(kBeforeConnectedInd);
      POBJDELETE(pMsg);
    }
    else if( pMcChannel )
      PTRACE2INT(eLevelError,"CH323Cntl::OnPartyIncomingChnlResponseReq - Status OK -No need to send response ", pMcChannel->GetCsIndex());
    else
      PTRACE(eLevelError,"CH323Cntl::OnPartyIncomingChnlResponseReq - Status OK - pMcChannel is NULL");

  }

  else
  { // Reject channel
    status  = -1;
    if(pMcChannel && pMcChannel->GetCsChannelState() == kNoNeedToDisconnect)
    {
      PTRACE2INT(eLevelError,"CH323Cntl::OnPartyIncomingChnlResponseReq -Reject case- No need to reject channel again ", pMcChannel->GetCsIndex());
    }
    else
    {

      if (pMcChannel) //only for incoming duo channel, we have the parameters in pMcChannel
      {
        PTRACE2INT(eLevelError,"CH323Cntl::OnPartyIncomingChnlResponseReq - pMcChannel<OK> ", pMcChannel->GetCsIndex());
        pInChnlResponseReq->channelIndex = pMcChannel->GetCsIndex();
      }
      else //otherwise, pMcChannel wasn't created
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyIncomingChnlResponseReq - pMcChannel<NULL> ");
        pInChnlResponseReq->channelIndex  = channelIndexForReject;
      }
      pInChnlResponseReq->channelDirection = FALSE;
      pInChnlResponseReq->dataType = eType;
      pInChnlResponseReq->dynamicPayloadType = 0;//pMcChannel->GetDynamicPayloadType();
    pInChnlResponseReq->localRtpAddressIp.transAddr.ipVersion = eIpAddrMatch;
      if (pInChnlResponseReq->localRtpAddressIp.transAddr.ipVersion == eIpVersion4)
      { // Case IpV4
      pInChnlResponseReq->localRtpAddressIp.transAddr.addr.v4.ip = m_UdpAddressesParams.IpV4Addr.ip;
      }
      else
      { // Case IpV6
      // --- UDP: array of addresses ---

      BYTE  place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(m_pH323NetSetup->GetTaDestPartyAddr(), m_UdpAddressesParams.IpV6AddrArray);

      pInChnlResponseReq->localRtpAddressIp.transAddr.addr.v6.scopeId = m_UdpAddressesParams.IpV6AddrArray[place].scopeId;
      memcpy(pInChnlResponseReq->localRtpAddressIp.transAddr.addr.v6.ip,m_UdpAddressesParams.IpV6AddrArray[place].ip,16);

      }
      pInChnlResponseReq->localRtpAddressIp.transAddr.distribution = eDistributionUnicast;
      pInChnlResponseReq->localRtpAddressIp.transAddr.transportType = eTransportTypeUdp;
      switch(pInChnlResponseReq->dataType)
      {
        case cmCapAudio:
        {
          pInChnlResponseReq->localRtpAddressIp.transAddr.port = m_UdpAddressesParams.AudioChannelPort;
          break;
        }
        case cmCapData:
        {
          pInChnlResponseReq->localRtpAddressIp.transAddr.port = m_UdpAddressesParams.FeccChannelPort;
          break;
        }
        case cmCapVideo:
        {
          if (label == kRolePeople)
          {
            pInChnlResponseReq->localRtpAddressIp.transAddr.port = m_UdpAddressesParams.VideoChannelPort;
            break;
          }
          else
          {
            pInChnlResponseReq->localRtpAddressIp.transAddr.port = m_UdpAddressesParams.ContentChannelPort;
            break;
          }
        }
        default:
        {
          PASSERTMSG(pInChnlResponseReq->dataType,"CH323Cntl::OnPartyIncomingChnlResponseReq - Channel Type");
          break;
        }
      }
      CSegment* pMsg = new CSegment;
      pMsg->Put((BYTE*)(pInChnlResponseReq),sizeof(mcReqIncomingChannelResponse));
      m_pCsInterface->SendMsgToCS(H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ,pMsg,m_serviceId,
                        m_serviceId,m_pDestUnitId,m_callIndex,pInChnlResponseReq->channelIndex,0,status);
      POBJDELETE(pMsg);

      if (eType == cmCapVideo)
      {
        if (label == kRolePeople)
        {
          if (m_bVideoInRejected == FALSE)
          {
            m_bVideoInRejected = TRUE;
            ConnectPartyToConf();
          }//else - there is no point to call to ConnectPartyToConf
        }
        else
        {
          if (m_bIsContentRejected == FALSE)
          {
            m_bIsContentRejected = TRUE;
            ConnectPartyToConf();
          }
        }
      }
      else if(eType == cmCapData)
      {
        m_bIsDataInRejected = TRUE;
        ConnectPartyToConf();
      }

      if (pMcChannel)// if this channel has inserted to the channels array, we need to delete it
      {   // UDP
        if ((pMcChannel->GetCmUdpChannelState() == kNotSendOpenYet ||  pMcChannel->GetCmUdpChannelState() == kRecieveCloseAck))
          m_pmcCall->RemoveChannel(arrayIndex);
        else
          pMcChannel->SetCsChannelState(kNoNeedToDisconnect);

      }
    }
  }
  PDELETE(pInChnlResponseReq);
}

/////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323IncomingChnlConnectedInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlConnectedInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  DWORD actualRate=0xFFFFFFFF;

  // IpV6 - Monitoring
  mcTransportAddress partyAddr;
  mcTransportAddress mcuAddr;

  memset(&partyAddr,0,sizeof(mcTransportAddress));
  memset(&mcuAddr,0,sizeof(mcTransportAddress));

  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323IncomingChnlConnectedInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323IncomingChnlConnectedInd - srcUnitId incorrect");
    return;
  }
  mcIndIncomingChannelConnected pinChnlConnect;

  DWORD  structLen = sizeof(mcIndIncomingChannelConnected);
  memset(&pinChnlConnect,0,structLen);
  pParam->Get((BYTE*)(&pinChnlConnect),structLen);

  DWORD receivedIndex = mcChannelIndex;

  CChannel *pChannel  = NULL;
  APIS8  arrayIndex = NA;
  int res = SetCurrentChannel(channelIndex,mcChannelIndex,&pChannel,&arrayIndex);

  if(pChannel == NULL)
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323IncomingChnlConnectedInd - Incoming connected channel was not found");
    DBGPASSERT(receivedIndex);
    return;
  }
  //else
//  m_pLoadMngrConnector->ChannelConnectExitCriticalSection(pChannel->index);

  if(status != STATUS_OK)
  {
    RejectChannel(pChannel,status,mcChannelIndex, channelIndex,arrayIndex);
    return;
  }

  pChannel->SetCsChannelState(kConnectedState); // (stream is still off)
  ERoleLabel eRole = pChannel->GetRoleLabel();

  if(m_pmcCall->GetIsOrigin())//dial out
  {
    // IpV6 - Monitoring
    memcpy(&mcuAddr, (m_pH323NetSetup->GetTaSrcPartyAddr()), sizeof(mcTransportAddress));
    memcpy(&partyAddr, (m_pH323NetSetup->GetTaDestPartyAddr()), sizeof(mcTransportAddress));
  }
  else
  {
    // IpV6 - Monitoring
    memcpy(&mcuAddr, &(m_pmcCall->GetDestTerminalParams().callSignalAddress), sizeof(mcTransportAddress));
    memcpy(&partyAddr, &(m_pmcCall->GetSrcTerminalParams().callSignalAddress), sizeof(mcTransportAddress));


  }

  switch(pChannel->GetType())
  {
    case cmCapAudio:
    {
      mcuAddr.port = m_UdpAddressesParams.AudioChannelPort;
      break;
    }
    case cmCapData:
    {
      mcuAddr.port = m_UdpAddressesParams.FeccChannelPort;
      break;
    }
    case cmCapVideo:
    {
      if (eRole == kRolePeople)
      {
        mcuAddr.port = m_UdpAddressesParams.VideoChannelPort;
        break;
      }
      else
      {
        mcuAddr.port = m_UdpAddressesParams.ContentChannelPort;
        break;
      }
    }
    default:
    {
      PASSERTMSG(pChannel->GetType(),"CH323Cntl::OnH323IncomingChnlConnectedInd - Channel Type");
      break;
    }
  }

  // H239 Addition
  if (pChannel->GetType() == cmCapVideo && (eRole & kRoleContentOrPresentation) && m_eContentInState == eNoChannel)
    m_eContentInState = eStreamOff;


  EIpChannelType channelType        = ::CalcChannelType(pChannel->GetType(),pChannel->IsOutgoingDirection(),
                                pChannel->GetRoleLabel(),pChannel->GetCapNameEnum());
  CPrtMontrBaseParams *pPrtMonitrParams = CPrtMontrBaseParams::AllocNewClass(channelType);
  CCapSetInfo capInfo = pChannel->GetCapNameEnum();
  SetPartyMonitorBaseParams(pPrtMonitrParams,channelType,actualRate,&partyAddr,&mcuAddr,(DWORD)capInfo);
  LogicalChannelConnect(pPrtMonitrParams,(DWORD)channelType);
  POBJDELETE(pPrtMonitrParams);

  //FOR VSW AUTO: if we asked for annex I_NS, since it is not opened directly in the channel, we should inform the RTP about it and also update the communication mode
  /*if ((pChannel->GetType() == cmCapVideo) && (pChannel->GetRoleLabel() == kRolePeople))
  {
    CapEnum protocol = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapReceive);
    if (protocol == eH263CapCode)
    {
      //if we asked it, it's in our capabilities.
      CH263VideoCap* pVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode,m_pTargetModeH323->GetMediaMode(cmCapVideo,cmCapReceive).GetDataCap());
      if (pVideoCap)
      {
        BYTE bAddAnnexI = m_pLocalCapH323->IsNsAnnexI() && m_pRmtCapH323->IsNsAnnexI();
        BYTE bAddAnnexF = m_pLocalCapH323->IsAnnex(typeAnnexF) && m_pRmtCapH323->IsAnnex(typeAnnexF) && !pVideoCap->IsAnnex(typeAnnexF);
        BYTE bAddAnnexT = m_pLocalCapH323->IsAnnex(typeAnnexT) && m_pRmtCapH323->IsAnnex(typeAnnexT) && !pVideoCap->IsAnnex(typeAnnexT);
        if (bAddAnnexI || bAddAnnexF || bAddAnnexT)
        {
          OnPartyNewChannelModeReq();

          const CMediaModeH323& rTargetMediaMode = m_pTargetModeH323->GetMediaMode(cmCapVideo, cmCapReceive); //the current mode will be updated according to target mode
          BYTE* dataCap = rTargetMediaMode.GetDataCap();
          CH263VideoCap* pVideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap(eH263CapCode, dataCap);
          pVideoCap->SetAnAnnexInMask(typeAnnexI_NS);
        }
        POBJDELETE(pVideoCap);
      }
    }
  }*/

  //send flow control if audio+video are opened and larger than conf rate
  if ( (m_pTargetModeH323->GetConfType() == kCp || m_pTargetModeH323->GetConfType() == kCop )  &&
        kRolePeople == eRole)
  {
    if (cmCapVideo == pChannel->GetType())
    {
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlConnectedInd - pChannel->GetRate() = ",pChannel->GetRate()/10);
      m_pCurrentModeH323->SetVideoBitRate(pChannel->GetRate(), cmCapReceive, kRolePeople);
    }
    sendFlowControlIfRateExceeded();
  }

  bool isEnableHdVsw = false;
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  if (pCommConf)
  {
	  if ((pCommConf->GetConfMediaType() == eMixAvcSvc) &&
			  (pCommConf->GetEnableHighVideoResInAvcToSvcMixMode() || pCommConf->GetEnableHighVideoResInSvcToAvcMixMode()))
		  isEnableHdVsw = true;
  }

  if (cmCapVideo == pChannel->GetType() && pChannel->GetRoleLabel() == kRolePeople && isEnableHdVsw)
  {
	  CVideoOperationPointsSet* pOperationPointsSet = m_pTargetModeH323->GetOperationPoints();
	  const VideoOperationPoint *pVideoOperationPoint = pOperationPointsSet->GetHighestOperationPoint(m_pTargetModeH323->GetPartyId());

      DWORD targetModeVidRate = m_pTargetModeH323->GetVideoBitRate(cmCapReceive, kRolePeople);
      DWORD newVideoRate = pVideoOperationPoint ? pVideoOperationPoint->m_maxBitRate*10 : 0;

      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlConnectedInd - HdVsw, Target mode video rate = ", targetModeVidRate);
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlConnectedInd - HdVsw, Highest operation point video rate = ", newVideoRate);


      if (targetModeVidRate > newVideoRate)
      {
    	  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlConnectedInd - HdVsw, Setting video rate to ", newVideoRate);
    	  m_pCurrentModeH323->SetVideoBitRate(newVideoRate, cmCapReceive, kRolePeople);
    	  m_pTargetModeH323->SetVideoBitRate(newVideoRate, cmCapReceive, kRolePeople);
    	  m_pLocalCapH323->SetVideoBitRate(newVideoRate);
    	  SendFlowControlReq(cmCapVideo, FALSE,kRolePeople, newVideoRate);
      }
  }

  OnChannelConnected(pChannel);

  if ( (pChannel->GetType() == cmCapVideo) && (pChannel->GetRoleLabel() & kRoleContentOrPresentation) )
  {//check if it is reopening of the content in
    if (m_bIsContentRejected) //might indicates that the content in was closed before
    {
      if (m_bContentInClosedWhileChangeVidMode == FALSE)
      {
        if (GetChannelIndexInList(true, cmCapVideo, TRUE, kRoleContentOrPresentation) >= m_maxCallChannel)
        {//that means that the out is opened so m_bIsContentRejected absolutely indicates that the in was closed before
          CVidModeH323& rContentMode = (CVidModeH323&)m_pTargetModeH323->GetMediaMode(cmCapVideo,cmCapReceive,kRoleContentOrPresentation);
          PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323IncomingChnlConnectedInd - reconnect content - ",PARTYNAME);
//            m_pTaskApi->OnH323EndContentReConnect(rContentMode);
          m_bIsContentRejected = FALSE;
        }
      }
      else //If the channel was closed and reopened while change video mode process, we don't report it to conf3
      {
        DeleteTimer(REOPEN_CONTENT_IN_TIMER);
        m_bContentInClosedWhileChangeVidMode = FALSE;
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323VideoUpdatePicInd(CSegment* pParam) // 1 - From Cs | 2 - from RTP
{ // This indication takes care of CS indications - Pass to the RTP.
  //PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323VideoUpdatePicInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323VideoUpdatePicInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323VideoUpdatePicInd - srcUnitId incorrect");
    return;
  }
  if (m_pmcCall->GetIsClosingProcess() == TRUE)
  { // if the call is in closing process no need to send fast update.
    PTRACE2INT(eLevelError,"CH323Cntl::OnH323VideoUpdatePicInd bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }

  CChannel *pChannel = NULL;
  APIS8 arrayIndex = NA;
  int res = SetCurrentChannel(channelIndex,mcChannelIndex,&pChannel,&arrayIndex);
  if(pChannel == NULL)
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323VideoUpdatePicInd - Incoming connected channel was not found");
    DBGPASSERT(channelIndex);
    return;
  }

  if (!res && (pChannel->GetType() == cmCapVideo))
  {
//    if((rVideoSyncLost==1)||(rVideoSyncLost==2))
//    {
      ERoleLabel eRole = pChannel->GetRoleLabel();

      //const char* roleStr = GetRoleStr(pChannel->GetRoleLabel());
      //const int roleStrSize = strlen(roleStr) + 1;
      //ALLOCBUFFER(s2, MediumPrintLen+ roleStrSize);
      //sprintf(s2,"Video role is %s",roleStr);
      //PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323VideoUpdatePicInd: ",s2);
      //DEALLOCBUFFER(s2);
      BYTE bIsGradualIntra = FALSE;
      CSegment*  seg = new CSegment;
      *seg << (WORD)Fast_Update << (WORD)eRole << (WORD)1 << bIsGradualIntra;

      m_pTaskApi->IpRmtH230(seg); // forward task to party manager
      POBJDELETE(seg);
//    }
  }
  else if (!res && pChannel->GetType() != cmCapVideo)
    PTRACE(eLevelError,"CH323Cntl::OnH323VideoUpdatePicInd: Channel is not video");
  else
    PTRACE(eLevelError,"CH323Cntl::OnH323VideoUpdatePicInd: Channel not found");
}

//==============================================================================================================
// PropagatePacketLostStatus
// ===========================
//
// Propagates the packet loss state from the card manager to the conference, after adjusting for LPR
// state.  This is done as follows:
// 1) If LPR is active inbound and/or outbound, packet loss states are first promoted to be at least at Major.
// 2) Current cell state is defined to be the inbound state.
// 3) Current layout state is defined to be the worst (max) between the inbound and outbound states.
// 4) If current states differ from last states, the conference is updated with the new states (via the party)
// 5) Finally, new packet loss states are preserved as the last states in the relevant members.
//==============================================================================================================
void CH323Cntl::PropagatePacketLostStatus(const eRtcpPacketLossStatus InLoss, const eRtcpPacketLossStatus OutLoss, const BYTE InLpr, const BYTE OutLpr)
{
	DBGPASSERT_AND_RETURN(m_pParty == NULL);
	BYTE shouldSendStatus = FALSE;

	CMedString entryLog;
	entryLog 	<< "CH323Cntl::PropagatePacketLostStatus - " << m_pParty -> GetFullName()
				<< " InLoss:" << InLoss << " OutLoss:" << OutLoss << " InLpr:" << InLpr << " OutLpr:" << OutLpr;
	PTRACE(eLevelInfoNormal,entryLog.GetString());

	//=============================================
	// Adjusting quality indications to LPR state
	//=============================================
	eRtcpPacketLossStatus	adjInLoss 			= (InLpr? 	max(InLoss,	ePacketLossMajor) 	: 	InLoss);
	eRtcpPacketLossStatus	adjOutLoss 			= (OutLpr? 	max(OutLoss,ePacketLossMajor) 	: 	OutLoss);
	eRtcpPacketLossStatus	layoutInd 			= max(adjInLoss, adjOutLoss);

	//===============================================================================
	// Deciding whether a new quality indication should be propagated to conference
	// Based on whether cell state OR layout state has changed.
	//===============================================================================
	if (adjInLoss != m_adjInboundPacketLossStatus ||
		layoutInd != max(m_adjInboundPacketLossStatus, m_adjOutboundPacketLossStatus))
	{
		shouldSendStatus = true;
	}

	//===============================================================================================================
	// Sending quality indication (Only for MPMx,MpmRx, for other boards the indications are calculated only for the log)
	//===============================================================================================================
	if (shouldSendStatus)
	{
		CSmallString sendLog;
		if (IsFeatureSupportedBySystem(eFeatureIndicationOnLayout_PacketLost))
		{
			m_pParty -> InformConfOnPacketLossState(adjInLoss, layoutInd);
			sendLog	<< "CH323Cntl::PropagatePacketLostStatus - sent CellInd:" << adjInLoss << " LayoutInd:" << layoutInd;
		}
		else
		{
			sendLog	<< "CH323Cntl::PropagatePacketLostStatus - Not MPMx, only logging CellInd:" << adjInLoss << " LayoutInd:" << layoutInd;
		}
		PTRACE(eLevelInfoNormal,sendLog.GetString());
	}

	//===================================
	// Updating quality related members
	//===================================
	m_cmInboundPacketLossStatus		= InLoss;
	m_cmOutboundPacketLossStatus	= OutLoss;
	m_adjInboundPacketLossStatus	= adjInLoss;
	m_adjOutboundPacketLossStatus	= adjOutLoss;

}

void CH323Cntl::OnH323PacketLostStatusConnected(CSegment* pParam)
{
	DBGPASSERT_AND_RETURN(m_pParty == NULL);
	CSmallString entryLog;
	entryLog << "CH323Cntl::OnH323PacketLostStatusConnected - " << m_pParty -> GetFullName();
	PTRACE(eLevelInfoNormal,entryLog.GetString());

	//========================================
	// Preparing loss status for propagation
	//========================================
	RTCP_PACKET_LOSS_STATUS_IND_S packetLossStatus;
	eRtcpPacketLossStatus 	InLoss 			= m_cmInboundPacketLossStatus;
	eRtcpPacketLossStatus 	OutLoss			= m_cmOutboundPacketLossStatus;
	BYTE 					CommandValid	= TRUE;
	memset(&packetLossStatus, 0, sizeof(packetLossStatus));
	pParam -> Get((BYTE*)&packetLossStatus, sizeof(packetLossStatus));

	if (packetLossStatus.mediaDirection == cmCapReceive)
	{
		PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnH323PacketLostStatusConnected - got new inbound:", packetLossStatus.ePacketLossStatus);
		InLoss = packetLossStatus.ePacketLossStatus;
	}
	else if (packetLossStatus.mediaDirection == cmCapTransmit)
	{
		PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnH323PacketLostStatusConnected - got new outbound:", packetLossStatus.ePacketLossStatus);
		OutLoss = packetLossStatus.ePacketLossStatus;
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnH323PacketLostStatusConnected - Handling Transmit OR Receive directions only, received direction: ", packetLossStatus.mediaDirection);
		CommandValid = FALSE;
	}

	//====================================
	// Conditionally propagating to conf
	//====================================
	if (CommandValid)
	{
		// Full implementation including LPR should be:
		// PropagatePacketLostStatus(InLoss, OutLoss, m_inboundLprActive, m_outboundLprActive);
		//
		// However due to SRS requirements LPR is omitted
		PTRACE(eLevelInfoNormal, "CH323Cntl::OnH323PacketLostStatusConnected - propagating packet loss indication, ignoring LPR state in video indication on layout due to SRS requirements");
		PropagatePacketLostStatus(InLoss, OutLoss, FALSE, FALSE);
	}
}

void CH323Cntl::OnH323PacketLostStatusImproperState(CSegment* pParam)
{
	DBGPASSERT_AND_RETURN(m_pParty == NULL);
	CSmallString entryLog;
	entryLog << "CH323Cntl::OnH323PacketLostStatusImproperState - " << m_pParty -> GetFullName() << " - does nothing for state " << m_state;
	PTRACE(eLevelInfoNormal,entryLog.GetString());
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323RtpVideoUpdatePicInd(CSegment* pParam)
{
  //PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323RtpVideoUpdatePicInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  if( m_pmcCall->GetIsClosingProcess() == TRUE)
  { // if the call is in closing process no need to send fast update.
    PTRACE2INT(eLevelError,"CH323Cntl::OnH323RtpVideoUpdatePicInd bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }

  TRtpVideoUpdatePictureInd rtpVideoUpdateInd;
  DWORD structLen = sizeof(TRtpVideoUpdatePictureInd);
  memset(&rtpVideoUpdateInd,0,structLen);

  if((structLen + pParam->GetRdOffset()) > pParam->GetLen())
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323RtpVideoUpdatePicInd - Segment size too small Id = ",pParam->GetLen());
    DBGPASSERT(1);
    return;
  }
  pParam->Get((BYTE*)(&rtpVideoUpdateInd),structLen);

  cmCapDataType mediaType = cmCapEmpty;
  BOOL isTransmitted = FALSE;
  ERoleLabel eRole =  kRoleUnknown;
  if (rtpVideoUpdateInd.unChannelType == (APIU32)kIpVideoChnlType)
  {
    mediaType = cmCapVideo;
    eRole = kRolePeople;
  }
  else if (rtpVideoUpdateInd.unChannelType == (APIU32)kIpContentChnlType)
  {
    mediaType = cmCapVideo;
    eRole = kRoleContentOrPresentation;
  }
  if (rtpVideoUpdateInd.unChannelDirection == cmCapTransmit)
    isTransmitted = TRUE;

  CChannel* pChannel = FindChannelInList(mediaType, isTransmitted, eRole);

  if(pChannel == NULL)
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323RtpVideoUpdatePicInd - channel was not found");
    return;
  }

  if (rtpVideoUpdateInd.unChannelType == (APIU32)kIpVideoChnlType || rtpVideoUpdateInd.unChannelType == (APIU32)kIpContentChnlType)
  {
    WORD rtpReport = 2;
    if(rtpVideoUpdateInd.unChannelDirection == cmCapTransmit)// Outgoing channel - Ask VB
      rtpReport = 2;
    else// incoming channel ask CS to send the remote
      rtpReport = 3;

    //const char* roleStr = GetRoleStr(pChannel->GetRoleLabel());
    //const int roleStrSize = strlen(roleStr) + 1;
    //ALLOCBUFFER(s2, MediumPrintLen+ roleStrSize);
    //sprintf(s2,"Video role is %s",roleStr);
    //PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323RtpVideoUpdatePicInd: ",s2);
    //DEALLOCBUFFER(s2);
    BYTE bIsGradualIntra = FALSE;
    CSegment*  seg = new CSegment;
    *seg << (WORD)Fast_Update << (WORD)pChannel->GetRoleLabel() << rtpReport << bIsGradualIntra;

    m_pTaskApi->IpRmtH230(seg); // forward task to party manager
    POBJDELETE(seg);
  }
  else if (pChannel->GetType() != cmCapVideo)
    PTRACE(eLevelError,"CH323Cntl::OnH323RtpVideoUpdatePicInd: Channel is not video");
  else
    PTRACE(eLevelError,"CH323Cntl::OnH323RtpVideoUpdatePicInd: Channel not found");
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnPartyVideoUpdatePicReq(CSegment* pParam)
{
  if( m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyVideoUpdatePicReq bIsClosing process - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }

    // vngr-7017 "bombing" logger on change layout - change trace to DEBUG level
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyVideoUpdatePicReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  ERoleLabel eRole;
  DWORD tempDword;
  *pParam >> (DWORD &)tempDword;
  eRole = (ERoleLabel)tempDword;

  APIS32 status = 0;

  // find incoming vide channel (people or content)
  CChannel* pVideoChannel =  FindChannelInList(cmCapVideo,FALSE,eRole);

  if (m_isCodianVcr)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyVideoUpdatePicReq: CodianVCR - Block message");
    return;
  }
  if (pVideoChannel == NULL)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyVideoUpdatePicReq: No channel found");
    return;
  }

  if (pVideoChannel->GetCsChannelState() != kConnectedState)
  {
    PTRACE2(eLevelInfoNormal, "CH323Cntl::OnPartyVideoUpdatePicReq - Channel isn't in connected state. ", PARTYNAME);
    return;
  }
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_VIDEO_UPDATE_PIC_REQ,NULL,m_serviceId,
            m_serviceId,m_pDestUnitId,m_callIndex,pVideoChannel->GetCsIndex(),pVideoChannel->GetIndex(),status);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323IncomingMediaInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323IncomingMediaInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnConfOrPartyDisconnectMediaChannel(WORD channelType,cmCapDirection channelDirection,ERoleLabel eRole)
{
  if (channelDirection & cmCapTransmit)
    CloseOutgoingChannel((cmCapDataType)channelType,eRole);
  if (channelDirection & cmCapReceive)
    CloseIncomingChannel((cmCapDataType)channelType,eRole);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnConfStreamOffMediaChannel(WORD channelType,WORD channelDirection,ERoleLabel eRole)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnConfStreamOffMediaChannel - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  BYTE isTransmitting = (channelDirection == cmCapTransmit) ? TRUE : FALSE;

  DWORD index = GetChannelIndexInList(true, (cmCapDataType)channelType,isTransmitting,eRole);
  if(index >= m_maxCallChannel)
    PTRACE(eLevelError,"CH323Cntl::OnConfStreamOffMediaChannel : The channel wasn't found !!!");
  else
  {
    CChannel* pChannel = m_pmcCall->GetSpecificChannel(index);
    m_channelTblIndex = index;
    OnPartyStreamOffReq(TRUE); //TRUE => StreamOffWithoutDisconnection
  }
}

////////////////////////////////////////////////////////////////////////////
/*void  CH323Cntl::OnConfStreamOnMediaChannel(WORD channelType, WORD channelDirection, ERoleLabel eRole)
{
  PTRACE2(eLevelInfoNormal,"CH323Cntl::OnConfStreamOnMediaChannel - ",PARTYNAME);
  if(! ((channelType == cmCapAudio) && m_isIncomingAudioHasDisconnectedOnce))
  {
  }
    //in case conf3ctl sent this request before the message of closing audio
    // channel has arrived to conf3ctl, we don't send stream_on_req for audio
#ifdef __CARMEL_CONTENT_
    m_pH323->SetMediaStream((cmCapDataType)channelType,eRole);
#endif
}*/

////////////////////////////////////////////////////////////////////////////
CChannel* CH323Cntl::FindChannelInList(DWORD receivedIndex) const
{
   return m_pmcCall->FindChannelInList(receivedIndex);
}

////////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::GetChannelIndexInList(bool aIsExternal, cmCapDataType eType,BYTE bIsTransmitting,ERoleLabel eRole) const
{
  // sometimes the bIsTransmitting is TRUE (255) and sometimes it's 1
  bIsTransmitting = bIsTransmitting ? TRUE: FALSE;
  return m_pmcCall->GetChannelIndexInList(aIsExternal, eType, bIsTransmitting,eRole);
}

////////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::GetChannelIndexInList(CChannel* pChannel) const
{
  return m_pmcCall->GetChannelIndexInList(pChannel, true);
}

////////////////////////////////////////////////////////////////////////////
CChannel* CH323Cntl::FindChannelInList(cmCapDataType eType, BYTE bIsTransmiting, ERoleLabel eRole, bool aIsExternal) const
{
  return m_pmcCall->FindChannelInList(eType, bIsTransmiting, eRole, aIsExternal);
}

////////////////////////////////////////////////////////////////////////////
int CH323Cntl::SetCurrentChannel(APIU32 channelIndex, APIU32 mcChannelIndex, CChannel **ppChannel, APIS8 *pArrayIndex) const
{
  return m_pmcCall->SetCurrentChannel(channelIndex,mcChannelIndex,ppChannel,pArrayIndex);
}

unsigned int CH323Cntl::GetMrmpChannelHandle(cmCapDataType eType) const
{
    CChannel* pChannel = FindChannelInList(eType, FALSE, kRolePeople, false);
    if (pChannel)
    {
        return pChannel->GetChannelHandle();
    }
    return INVALID_CHANNEL_HANDLE;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyStreamOffReq(BYTE bIsStreamOffWithoutDisconnection)
{
  PTRACE2(eLevelInfoNormal,"CH323Cntl::OnPartyStreamOffReq - ",PARTYNAME);
    // Need to update specific channel closing process initiator to McInitiator
  CChannel *pChannel = m_pmcCall->GetSpecificChannel(m_channelTblIndex);
  DBGPASSERT_AND_RETURN(pChannel == NULL);

  if ((pChannel->GetCsChannelState() == kDisconnectedState)||(pChannel->IsCsChannelStateDisconnecting()))
  {
    TRACEINTO << "CH323Cntl::OnPartyStreamOffReq: channel is in disconnected mode " << m_channelTblIndex+1;
    return;
  }

  // if we want to disconnect only the stream and not the channel, we check if the stream is already disconnected (or disconnecting)
  if (bIsStreamOffWithoutDisconnection && (pChannel->GetStreamState() == kStreamOffState))
  {
    TRACEINTO << "CH323Cntl::OnPartyStreamOffReq: channel's stream is in disconnected mode " << m_channelTblIndex+1;
    return;
  }

  if (bIsStreamOffWithoutDisconnection == FALSE) //disconnecting channel
  {
    if (!pChannel->IsCsChannelStateDisconnecting()) // Check this in order to save the specific disconnecting state if exist.
      pChannel->SetCsChannelState(kDisconnectingState);
    pChannel->SetChannelCloseInitiator(McInitiator);

    //In case we need to drop only the channel and not all the call we should send streamOff to the RTP.
    //We should do it for case we will receive cahnnelCloseInd with -1 (reject) and in case we willnot send the RTP
    //streamOff it can cause for duplicate internal TSS.
  /*  if(pChannel->GetStreamState() == kStreamOnState)
      SendStreamOffReq(pChannel); */
    pChannel->SetIsStreamOnSent(FALSE);
    StartCloseChannel(pChannel, FALSE);
    pChannel->SetCsChannelState(kWaitToSendChannelDrop);
    if (pChannel->GetRoleLabel() == kRolePeople && !pChannel->IsOutgoingDirection())
    {// close internal channels when closing the receive channel (internal channels are tied to the receive channel)
        CloseInternalChannels(pChannel->GetType());
    }
    Cm_FillAndSendCloseUdpPortStruct(pChannel);
  }
  else
  {
    pChannel->SetIsStreamOnSent(FALSE);
    if ((pChannel->GetType() == cmCapAudio) && (pChannel->IsOutgoingDirection()) )
      OFF(m_isAudioOutgoingChannelConnected);

    //SendStreamOffReq(pChannel);
  }

  pChannel->SetStreamState(kStreamOffState);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::CloseStream(CChannel *pChannel)
{
//  TRACEINTO << "CH323Cntl::CloseStream - ";
  PrintChannelDetails(pChannel);
  pChannel->SetIsStreamOnSent(FALSE);

  pChannel->SetStreamState(kStreamOffState);

  if (m_pmcCall->GetIsClosingProcess() == FALSE)
  {
    if (pChannel->GetType() == cmCapAudio)
    {
      if( pChannel->IsOutgoingDirection() ) //outgoing channel
      {
        /* Operations to handle no matter who is the channelCloseInitiator */
        //in order to send  block to the encoder
        m_pTaskApi->SendCloseChannelToConfLevel(cmCapAudio, cmCapTransmit, (ERoleLabel)pChannel->GetRoleLabel());
      }
      else //incoming channel
      {
        if(pChannel->GetChannelCloseInitiator() == PmInitiator)
        {
          m_pCurrentModeH323->SetMediaOff(cmCapAudio);
          if(IsValidTimer(OTHERMEDIACONNECTED))
            DeleteTimer(OTHERMEDIACONNECTED);
          if ( m_isAudioConnected || (m_state == CHANGEMODE) || (m_state == CONNECT))
          //if we have already sent "connect party" to conf:
            m_pTaskApi->SendCloseChannelToConfLevel(cmCapAudio, cmCapReceive, (ERoleLabel)pChannel->GetRoleLabel());
        }
      }
    }
        //else //Video or Data
        //    m_pTaskApi->SendCloseChannelToConfLevel(pChannel->GetType(), pChannel->GetChannelCmDirection(), (ERoleLabel)pChannel->GetRoleLabel());
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::PrintChannelDetails(CChannel *pChannel, BYTE isCloseStream)
{
  CMedString str;
  if (isCloseStream)
    str << "CloseStream - ";
  else
    str <<"StartCloseChannel - ";

  str << "McmsConnId is = " << m_pCsRsrcDesc->GetConnectionId()
        << ", mcms ChannelIndex is = " << pChannel->GetIndex()
        << ", ChannelIndex is = " << pChannel->GetCsIndex();

  PTRACE2(eLevelInfoNormal,"CH323Cntl::PrintChannelDetails :  ",str.GetString());
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::StartCloseChannel(CChannel *pChannel, BYTE bRejectByCs)
{
//  TRACEINTO << "CH323Cntl::StartCloseChannel - ";
  PrintChannelDetails(pChannel,1);


  //if(pChannel->GetStreamState() == kStreamOnState) //need to close the strean first and then close the channel
  CloseStream(pChannel);

  if (m_pmcCall->GetIsClosingProcess() == FALSE)
  {
    if (pChannel->GetType() == cmCapAudio)
    {
      if( !pChannel->IsOutgoingDirection() ) //incoming channel
      {
        if(pChannel->GetChannelCloseInitiator() == PmInitiator)
        {
          ON(m_isIncomingAudioHasDisconnectedOnce);
        }
      }
    }
  }

  if ((pChannel->GetType() == cmCapAudio) && (pChannel->IsOutgoingDirection()) )
    OFF(m_isAudioOutgoingChannelConnected);
  //change the value to 0 to stop sending Update picture to card.
  if ((pChannel->GetType() == cmCapVideo) && (pChannel->IsOutgoingDirection()) && (pChannel->GetRoleLabel() == (DWORD)kRolePeople))
    OFF(m_isVideoOutgoingChannelConnected);
  //change the value to 0 to stop sending Update picture to card.
  if ((pChannel->GetType() == cmCapVideo) && (pChannel->IsOutgoingDirection()) && (pChannel->GetRoleLabel() & (DWORD)kRoleContentOrPresentation))
    OFF(m_isVideoContentOutgoingChannelConnected);


  if(pChannel->GetChannelCloseInitiator() != McInitiator)
  {
    if (m_pmcCall->GetIsClosingProcess() == FALSE)
    {
      if(pChannel->GetType() == cmCapVideo)
      {
        if ((ERoleLabel)pChannel->GetRoleLabel() & kRoleContentOrPresentation)
        {
          //In case the content channel close we should reset the timer (if we send channelNewRate but the
          //remote start to close the channel)
          if (pChannel->GetRoleLabel() == (DWORD)kRoleContent)//in presentation, closing this channel might be a part of the change content process
          {
            if (IsValidTimer(H323CHANGERATETOUT))
              DeleteTimer(H323CHANGERATETOUT);
          }

          //In case we receive streamOffInd from the remote in order to close the content In channel or
          //content out channel - I will close the content out/In and disconnect from the bridge.
          if (!pChannel->IsOutgoingDirection() && (pChannel->GetRoleLabel() == (DWORD)kRoleContent)) //in channel: only in EPC it is a problem
          {
            m_bIsContentRejected = TRUE;
            /*If the party is in change video mode, we don't report to the conf3 about
            closing content in, since the ep might reopen it, before the change video
            mode is finished. If the ep won't reopen it, we report the conf3, after
            reporting the the change video mode was finished.
            (In general, only iPower does it, and it reopens the content again, before
            reopening the video).*/
            if (m_pParty->IsPartyInChangeVideoMode() && !m_bIsContentSpeaker)
            {
              PTRACE2(eLevelInfoNormal,"CH323Cntl::StartCloseChannel - close content while changeVidMode - ",PARTYNAME);
              m_bContentInClosedWhileChangeVidMode = TRUE;
              //the period of the timer ensure that it pops out, only after the change video mode processs is finished
              StartTimer(REOPEN_CONTENT_IN_TIMER, PARTY_CHANGEVIDEO_TIME);
            }
            else
            {
              PTRACE(eLevelInfoNormal,"CH323Cntl::StartCloseChannel: send close contentBridge.");
              m_bIsContentRejected = TRUE;
              m_pTaskApi->SendCloseChannelToConfLevel(cmCapVideo, cmCapReceive, kRoleContent);
              if(m_bIsContentSpeaker)
                OnPartyRoleTokenReq(kPresentationTokenRequest);
            }
          }
          m_bFirstFlowControlIndIndOnContent = TRUE;//reset in case the channel will be reopned
          m_bWaitForFlowCntlIndIndOnContent  = FALSE;
        }//end content
      }
    }
  }

  if ((pChannel->GetCsChannelState() == kBeforeResponseReq) && (pChannel->GetChannelDirection() == cmCapReceive) && (bRejectByCs == FALSE))
    OnPartyIncomingChnlResponseReq(pChannel, TRUE);
  else
  {
    if (!pChannel->IsCsChannelStateDisconnecting()) // Check this in order to save the specific disconnecting state if exist.
      pChannel->SetCsChannelState(kDisconnectingState);
  }

  return;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendChannelDropReq(CChannel* pChannel)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SendChannelDropReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  cmCapDataType eType         = pChannel->GetType();
  BOOL      bTransmitting = pChannel->IsOutgoingDirection()? YES: NO;
  ERoleLabel    eRole     = (ERoleLabel)pChannel->GetRoleLabel();
  cmCapDirection  direction = CalcCmCapDirection(bTransmitting);

  mcReqChannelDrop* pChnlDrop = new mcReqChannelDrop;
  pChnlDrop->channelDirection = (DWORD)direction;
  pChnlDrop->channelIndex = pChannel->GetCsIndex();
  pChnlDrop->channelType = (DWORD)eType;

//  m_pLoadMngrConnector->RequestChannelCloseEnterSection(pChannel->index);

  //In case we decided to send channel drop we should set media to off in the current that we will know that
  //this channel is in closing process and we update to the conf (when another chennel connet) this channel
  //not be in active mode
  //In case we the reomte reject the closing we will activate the media in the current.
  m_pCurrentModeH323->SetMediaOff(eType ,direction, eRole);

  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)(pChnlDrop), sizeof(mcReqChannelDrop));
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_CHNL_DROP_REQ,pMsg,m_serviceId,
              m_serviceId,m_pDestUnitId,m_callIndex,pChannel->GetCsIndex(),pChannel->GetIndex(),0);
  POBJDELETE(pMsg);

  PDELETE (pChnlDrop);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323StartChannelCloseInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323StartChannelCloseInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323StartChannelCloseInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323StartChannelCloseInd - srcUnitId incorrect");
    return;
  }
  mcIndStartChannelClose pStartChannelClosedInd;

  DWORD  structLen = sizeof(mcIndStartChannelClose);
  memset(&pStartChannelClosedInd,0,structLen);
  pParam->Get((BYTE*)(&pStartChannelClosedInd),structLen);

  APIS8 arrayIndex = NA;
  CChannel* pChannel = NULL;
  int retval;

  retval = SetCurrentChannel(channelIndex,mcChannelIndex, &pChannel, &arrayIndex);
  if(! retval)
  {
    //In case we receive to close the channel while we are waiting to the response of the channel about new rate,
    //we should finish the change mode process, we will not receive the answer from the remote because he already
    //start closing process.
    if(m_pmcCall->GetCallCloseInitiator() != McInitiator)
    {
      if(pChannel->GetType() == cmCapVideo)
      {
        if (((ERoleLabel)pChannel->GetRoleLabel() & kRoleContentOrPresentation) && m_bWaitForFlowCntlIndIndOnContent)
        {
          SendEndChangeContentToConfLevel();
          m_bWaitForFlowCntlIndIndOnContent = FALSE;
        }
        else if(((ERoleLabel)pChannel->GetRoleLabel() == kRolePeople) && m_bWaitForFlowCntlIndIndOnPeople)
        {
          SendEndChangeContentToConfLevel();
          m_bWaitForFlowCntlIndIndOnPeople = FALSE;
        }
      }
    }

    if(pChannel->GetChannelCloseInitiator() == NoInitiator)
    {
      pChannel->SetChannelCloseInitiator(PmInitiator);
      StartCloseChannel(pChannel, FALSE);
      pChannel->SetCsChannelState(kWaitToSendChannelDrop);
      if (pChannel->GetRoleLabel() == kRolePeople)
      {
          CloseInternalChannels(pChannel->GetType());
      }
      Cm_FillAndSendCloseUdpPortStruct(pChannel);
    }
    else if(pChannel->GetChannelCloseInitiator() == McInitiator)
    { //McInitiator could be in two cases:
      //1. MCMS try to close one channel - secondary or VIU - Mcms sent channeldrop
      //2. MCMS try to close the call, therefore Mcms initiate the close of the channels - Mcms didn't send chanDrop
      PTRACE(eLevelError,"CH323Cntl::OnH323StartChannelCloseInd - channel already dropped by MCMS");
      //if(m_isReceiveCallDropMessage)
      ECmUdpChannelState eUdpState = pChannel->GetCmUdpChannelState();
      if ( (eUdpState == kRecieveCloseAck) || (eUdpState == kNotSendOpenYet) )
      {   //second case: In case channel is closed before we sent open udp
        //While Mcms received calldrop and start closing the channels, the remote start closing the call either -
        //concurrent close - the card wait the channelDropReq to continue closing the channels.
        SendChannelDropReq(pChannel);
      }
      else //in order to send call drop to cs after the cm answer.
        pChannel->SetCsChannelState(kWaitToSendChannelDrop);
    }
    else if(pChannel->GetChannelCloseInitiator() == PmInitiator)
    {
      PTRACE(eLevelError,"CH323Cntl::OnH323StartChannelCloseInd - second close by remote");
      DBGPASSERT(pChannel->GetCsIndex());
    }
    else //GkInitiator
      PTRACE(eLevelError,"CH323Cntl::OnH323StartChannelCloseInd - channel already dropped by Gk");
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::HandleContentChannel(CChannel* pChannel,APIS32 chanStatus)
{
  if (m_bIsOutContentChanReject == TRUE)
  {
    //If we dropped the incoming content channel because of the remote rejected our outgoing content channel,
    //we do not need to wait to the content to connect, we need to connect the party to conf.
    PTRACE(eLevelInfoNormal,"CH323Cntl::HandleContentChannel: Out content channel was rejected");
    m_bIsOutContentChanReject = FALSE;
    m_bIsContentRejected    = TRUE;
    ConnectPartyToConf();
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::CloseChannel(CChannel* pChannel,APIS8 arrayIndex,APIS32 chanStatus, BYTE isNeedToCloseUdp)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CloseChannel - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  cmCapDataType eType         = pChannel->GetType();
  BOOL      bTransmitting = pChannel->IsOutgoingDirection()? YES: NO;
  ERoleLabel    eRole     = (ERoleLabel)pChannel->GetRoleLabel();

  pChannel->SetCsChannelState(kDisconnectedState); //even though we delete it afterwards (just for the record)

/*  if(m_pmcCall->pChannelsArray[arrayIndex]->pSameSessionChannel)
  {
    m_pmcCall->pChannelsArray[arrayIndex]->pSameSessionChannel->pSameSessionChannel = NULL;
    // because in some cases when the MCMS open channels the card return the samesession as the channel that got opened.
    if(m_pmcCall->pChannelsArray[arrayIndex]->pSameSessionChannel)
    {
      for (int i=0; i< MaxAssociatedChannels; i++)
        m_pmcCall->pChannelsArray[arrayIndex]->pSameSessionChannel->pAssociatedChannels[i] = NULL;
    }
  }*/

  if(bTransmitting)
  {
    if(eType == cmCapAudio)
      OFF(m_isAudioOutgoingChannelConnected);
    else if(eType  == cmCapVideo)
    {
      if (eRole & kRoleContentOrPresentation)
        OFF(m_isVideoContentOutgoingChannelConnected);


      else
        OFF(m_isVideoOutgoingChannelConnected);
    }
    else if(eType == cmCapData)
    {
      OFF(m_isDataOutgoingChannelConnected);

      m_bIsDataOutRejected = FALSE;
    }
  }
  else // receive:
  {
    if( (eType == cmCapVideo) && (eRole == kRolePeople) )
      m_bVideoInRejected = FALSE;//initialize, for further use
    else if(eType == cmCapData)
      m_bIsDataInRejected = FALSE;
  }
  cmCapDirection  direction = CalcCmCapDirection(bTransmitting);
  m_pCurrentModeH323->SetMediaOff(eType ,direction, eRole);

  if(m_McmsOpenChannels && m_pParty->GetChangeModeState() != eReopenOut)
    m_pTargetModeH323->SetMediaOff(eType ,direction, eRole);

  if( (eType == cmCapVideo) && (eRole & kRoleContentOrPresentation) )
    HandleContentChannel(pChannel,chanStatus);

  EIpChannelType channelType = ::CalcChannelType(eType,bTransmitting,eRole,pChannel->GetCapNameEnum());

  if(!pChannel->GetIsRejectChannel()) //In case of reject channel the conf does not know this channel yet.
    LogicalChannelDisConnect(channelType, eType, bTransmitting, eRole);

  if (!isNeedToCloseUdp)
    m_pmcCall->RemoveChannel(arrayIndex);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323ChannelCloseInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ChannelCloseInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIS8 arrayIndex = NA;
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;
  BOOL   bCheckSendCallDrop = FALSE;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323ChannelCloseInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323ChannelCloseInd - srcUnitId incorrect");
    return;
  }
  mcIndChannelClosed pChannelClosedInd;

  DWORD  structLen = sizeof(mcIndChannelClosed);
  memset(&pChannelClosedInd,0,structLen);
  pParam->Get((BYTE*)(&pChannelClosedInd),structLen);

  CChannel* pChannel = NULL;
  int retval = SetCurrentChannel(channelIndex,mcChannelIndex, &pChannel, &arrayIndex);

/*  if (!retval)
    if(m_pLoadMngrConnector->GetChannelCloseReqId(pChannel->index))
      m_pLoadMngrConnector->ChannelCloseExitCriticalSection(pChannel->index);
*/
  if(pChannel == NULL)
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323ChannelCloseInd - Channel was not found");
    DBGPASSERT((APIU8)arrayIndex);
  }

  else
  {
    TRACEINTO << "found channel Conn Id = " << m_pCsRsrcDesc->GetConnectionId() << ", status - " << status;
    bCheckSendCallDrop = (pChannel->GetCsChannelState() == kCheckSendCallDrop) ? TRUE : FALSE;
    cmCapDataType eType         = pChannel->GetType();
    APIS32      chanStatus  = status;
    ERoleLabel  eRoleLabel = pChannel->GetRoleLabel();

    if(chanStatus != channelCloseProcessRejectedByRemote)
    {
      if(retval)
          PTRACE(eLevelError,"CH323Cntl::OnH323ChannelCloseInd: closing channel not find in array");
      else
      {
        cmCapDirection closedChannelDirection = CalcCmCapDirection(pChannel->IsOutgoingDirection());//we need to do it here, before pChannel is deleted by the function CloseChannel
        PTRACE(eLevelInfoNormal,  "CH323Cntl::OnH323ChannelCloseInd, VNGFE-8204: before close channel function");
    	CloseChannel(pChannel,arrayIndex,chanStatus);

    	if ((eType == cmCapVideo) && (closedChannelDirection == cmCapReceive) && (eRoleLabel & kRoleContentOrPresentation))
          m_eContentInState = eNoChannel;
        if ( (eType == cmCapAudio) && (closedChannelDirection == cmCapTransmit) )
        { // if we closed the outgoing channel try to open another one
          //audio shuffle:
          BYTE bRemoteCapsHaveAudio = (m_pRmtCapH323->GetNumOfAudioCap() != 0);
          if (!m_pParty->IsRemoteCapsEcs() && bRemoteCapsHaveAudio)
          {
            if (m_isIncomingAudioHasDisconnectedOnce || m_isOutgoingAudioHasDisconnectedOnce)
            OpenOutgoingChannel(cmCapAudio); //reopen the outgoing audio channel
          }
        }

        BYTE indexOfMediaTypeInArray=ConvertChannelTypeEnumToArrayIndex(eType,eRoleLabel);
                if ( eType != cmCapAudio)
            m_pTaskApi->SendCloseChannelToConfLevel(eType, closedChannelDirection, eRoleLabel);
          if (  m_pNewInChanSeg[indexOfMediaTypeInArray] && (closedChannelDirection == cmCapReceive))
        {
            CSegment* seg = new CSegment;
            APIU32 callIndex = 0;
            APIU32 channelIndex = 0;
            APIU32 mcChannelIndex = 0;
            APIU32 stat1 = 0;
            APIU16 srcUnitId = 0;
            *(m_pNewInChanSeg[indexOfMediaTypeInArray]) >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;
            DWORD nMsgLen =  m_pNewInChanSeg[indexOfMediaTypeInArray]->GetWrtOffset() - m_pNewInChanSeg[indexOfMediaTypeInArray]->GetRdOffset();
            BYTE* tmp = new BYTE[nMsgLen]; AUTO_DELETE_ARRAY(tmp);
            mcIndIncomingChannel *pInChnlInd = (mcIndIncomingChannel*)tmp;
            memset(pInChnlInd,0,nMsgLen);
            m_pNewInChanSeg[indexOfMediaTypeInArray]->Get((BYTE*)pInChnlInd,nMsgLen);
            *(seg)<<(APIU32)callIndex<<(APIU32)channelIndex<<(APIU32)mcChannelIndex<<(APIU32)stat1<<(APIU16)srcUnitId;
             (seg)->Put((BYTE*)pInChnlInd, sizeof(mcIndIncomingChannel)+pInChnlInd->sizeOfChannelParams);
            POBJDELETE(m_pNewInChanSeg[indexOfMediaTypeInArray]);
          PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ChannelCloseInd: a new in channel request is in line-start process it - ", indexOfMediaTypeInArray);
          DispatchEvent(H323_CS_SIG_INCOMING_CHANNEL_IND, seg);
        }

      }
    }
    else
    {
      pChannel->SetChannelCloseInitiator((DWORD)NoInitiator);
      // VNGR-4217
      pChannel->SetCsChannelState(kConnectedState); // (stream is still off)

      cmCapDirection  direction = CalcCmCapDirection(pChannel->IsOutgoingDirection());
      //Update the current mode from the target mode (we deactivate the media when sending channel drop)
      UpdateCurrentScmH323(pChannel);
      PTRACE(eLevelError,"CH323Cntl::OnH323ChannelCloseInd: Remote did not close channel");
      if (m_pParty->IsPartyInChangeVideoMode() && (eType == cmCapVideo) && (pChannel->GetRoleLabel() == kRolePeople) )
        m_pTaskApi->SendH323LogicalChannelReject(cmCapVideo, cmCapTransmit, kRolePeople);
    }
  }

  // If two following conditions are ...together, we have to start
  // the drop_call prosidure:
  // 1). If any other of the still opened channels are NOT in state of
  //     disconnection which was initiated by MCMS request.
  // 2). If 'Call_Drop' request was recieved while MCMS channels' disconnection
  //     process was in progress( 'drop_call' waiting to be executed).
  if ( !IsChannelInDisconnectProcessByMcms() && m_isCallDropRequestWaiting )
  {
    OFF(m_isCallDropRequestWaiting);
    OnPartyCallDropReq();
  }
  else if (bCheckSendCallDrop)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323ChannelCloseInd: Should check sending call_Drop");
    SendCallDropIfNeeded();
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyCallCloseConfirmReq()
{
  m_isCloseConfirm = TRUE;

  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCallCloseConfirmReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    if(!m_isCallingThroughGk)////Michael - HOMOLOGATION
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_CALL_CLOSE_CONFIRM_REQ,NULL,m_serviceId,
            m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);

  if (m_isCallingThroughGk)
    CreateAndSendDRQReq(cmRASDisengageReasonNormalDrop);
  else if (m_isReceiveCallDropMessage)
    RemoveAndDisconnectCall();
  //else - we remove the call when receiving callDrop.
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyCallCloseConfirmReqIfNeeded()
{
  TRACEINTO << "mix_mode: num of external channels: " << m_pmcCall->GetChannelsCounter() << " num of internal channels: " << m_pmcCall->GetNumOfInternalChannels();
  BYTE bSend = TRUE;
  CChannel* pChannel;
  for(int i=0; (i < m_maxCallChannel) && bSend; i++)
  {
    pChannel = m_pmcCall->GetSpecificChannel(i);
    if (pChannel)
    {
      if ( (pChannel->GetCsChannelState() == kWaitToSendChannelDrop) || (pChannel->GetCmUdpChannelState() == kSendClose) )
        bSend = FALSE;
    }
  }
  if (bSend)
  {// check internal channels
      if (m_pmcCall->GetNumOfInternalChannels() > 0)
      {
          TRACEINTO << "mix_mode: Waiting to close internal channels... number of still open channels=" << m_pmcCall->GetNumOfInternalChannels();
          bSend = FALSE;
      }
  }
  if (bSend)
  {
      TRACEINTO << "Send close confirm";
      OnPartyCallCloseConfirmReq();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: The function used to init the mccall structure for the case of h323DialInReject (for some reason)
//---------------------------------------------------------------------------------------------------
void  CH323Cntl::OnPartyCallDropDialIn()
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropDialIn - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  OnPartyCallDropReq();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: The function used to init the mccall structure for the case of h323DialIn (for some reason)
//---------------------------------------------------------------------------------------------------
void  CH323Cntl::OnPartyCallAnswerDialIn()
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCallAnswerDialIn - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  SetDialIn_mcCall();
  RecieveInfoFor_mcCall();

  m_state = SETUP;

  CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
  CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
  if (pServiceParams == NULL)
  {
    PASSERTMSG(m_pmcCall->GetConnectionId(), "CH323Cntl::OnPartyCallAnswerDialIn - IP Service does not exist!!!");
    return;
  }

  BOOL bIsGkExternal = pServiceParams->isGKExternal();
  BOOL bIsPrimaryNet = m_pH323NetSetup->IsItPrimaryNetwork();
  if (bIsGkExternal && bIsPrimaryNet)
  {
    const mcTransportAddress* pTransportAddress = m_pH323NetSetup->GetTaSrcPartyAddr();
    m_pmcCall->SetSrcTerminalCallSignalAddress(*pTransportAddress);

    //In case of Avaya we should first send the card to authenticate the offering "OnH323CtAuthenticationReq"
    //and only if the message is authenticated we can continue the flow.
  /*  if(m_bIsAvaya && m_pmcCall->bIsAuthenticated)
      OnH323CtAuthenticationReq();
    else
    {*/
      m_CallConnectionState = GkARQ;
      CreateAndSendARQReq();
  //  }
  }
  else
    OnPartyCallAnswerReq();


  if(m_onGatewayCall || m_remoteIdent == PolycomMGC)
       StartTimer(PARTYCONNECTING, 110*SECOND);// for gateway calls we need much longer time
  else  // NOT GW CALL.
    StartTimer(PARTYCONNECTING, 50*SECOND);//20(stack)+30
}

////////////////////////////////////////////////////////////////////////////
/*
  The function used to init the mccall structure
  for the case of h323DialInFailure (for some reason)
*/
void  CH323Cntl::OnPartyCallAnswerDialInFailure(int reason, char* alternativeAlias)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyCallAnswerDialInFailure - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  m_state   = DISCONNECT;
  SetDialIn_mcCall();

  m_pmcCall->SetCallCloseInitiator(McInitiator);
  m_pmcCall->SetIsClosingProcess(TRUE);

  m_pmcCall->SetCallStatus(-1);

  if ( reason == int(cmReasonTypeCallForwarded))  //in case of call forward
  {
    m_pmcCall->SetCallStatus(-2);
    //StartTimer(FORWARDINGTIMER, 30 * SECOND);  //???
    OnPartyCallAnswerReq(reason, 1);
  }
  else
  {
    //StartTimer(PARTYDISCONNECTTOUT, 20*SECOND);
    OnPartyCallAnswerReq(reason);
  }

//  if( m_pH323->GetRsrcId() == INCALLENTRY )
//    RemoveAndDisconnectCall();
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnPartyCallDropReq()
{
  BYTE bIsSendCallDrop = FALSE;
  PASSERT_AND_RETURN(NULL == m_pmcCall);

  if(IsValidTimer(PARTYCONNECTING))
    DeleteTimer(PARTYCONNECTING);

  DWORD msgLen = MediumPrintLen + strlen(PARTYNAME)+1;
  ALLOCBUFFER(s1,msgLen);
  sprintf(s1, "Party Name is %s, and McmsConnId is = %d",PARTYNAME, m_pCsRsrcDesc->GetConnectionId());
  PTRACE2(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq", s1);
  DEALLOCBUFFER(s1);

  OFF(m_isIncomingAudioHasDisconnectedOnce);
  OFF(m_isOutgoingAudioHasDisconnectedOnce);

  if((m_isCloseConfirm) &&
    ((m_pmcCall->GetCallCloseInitiator() == GkInitiator) ||
     (m_pmcCall->GetCallCloseInitiator() == McInitiator)))
  {// closing the call in case the card finish the disconnecting process and report the call_idle.
    // and only after that arrives the call_drop
    //we wait for RemoveAndDisconnectCall function.
    PTRACE(eLevelError,"CH323Cntl::OnPartyCallDropReq - Call Drop after EP is already finished disconnecting");
    RemoveAndDisconnectCall();
    return;
  }
  // VNGR-4217
  m_pmcCall->SetIsClosingProcess(TRUE);
  // If MCMS has requested to close any media channels and the process of
  // the channel disconnection didn't finish yet, we don't start call disconnection
  // process, if 'call_drop' request has been received at such point of time.
  // In this case we wait for the end of channel disconnection and then
  // we can start call drop procedure.

  if (IsChannelInDisconnectProcessByMcms() )
  {
    PTRACE(eLevelError,"CH323Cntl::OnPartyCallDropReq - CALL_DROP is wating for MCMS disconnection channel process to be finished");
    ON(m_isCallDropRequestWaiting);

    // start timer to ensure returned and destroy of the party
    if (IsValidTimer(PARTYDISCONNECTTOUT))
      DeleteTimer(PARTYDISCONNECTTOUT);
    StartTimer(PARTYDISCONNECTTOUT, 60*SECOND);

    return;
  }
//  m_pmcCall->SetIsClosingProcess(TRUE);
  ON (m_isReceiveCallDropMessage);
  BYTE bWaitForCloseUdp = FALSE;

  CloseInternalChannels(cmCapAudio);
  CloseInternalChannels(cmCapVideo);

  CChannel  *pChannel = NULL;
  int     numerOfOpenChannels = m_pmcCall->GetChannelsCounter();
  for(int i=0; i < m_maxCallChannel; i++)
  {
    //close the channels send callDrop and wait to channelCloseInd
	pChannel = m_pmcCall->GetSpecificChannel(i);
	if (pChannel)
    {
      numerOfOpenChannels--;
      //pChannel = m_pmcCall->GetSpecificChannel(i);

      if(pChannel->GetChannelCloseInitiator() == NoInitiator)
      {
        pChannel->SetChannelCloseInitiator(McInitiator);
        Cm_FillAndSendCloseUdpPortStruct(pChannel);
        if(pChannel->GetCmUdpChannelState() != kNotSendOpenYet && pChannel->GetCmUdpChannelState() != kRecieveCloseAck)
          bWaitForCloseUdp = TRUE; //we will send call drop only after we get ack on close udp for all the channels
        StartCloseChannel(pChannel, FALSE);
      }
      else if(pChannel->GetChannelCloseInitiator() == PmInitiator)
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - channel already dropped by remote");
      else if(pChannel->GetChannelCloseInitiator() == McInitiator)
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - second close by MCMS");
        DBGPASSERT(pChannel->GetCsIndex());
      }
      else //GkInitiator
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - channel already dropped by Gk");
    }
  }
  DBGPASSERT(numerOfOpenChannels); //when we get out from the loop we should send close channel to all the
                     //opening channels.


  // start timer to ensure returned and destroy of the party
  if (IsValidTimer(PARTYDISCONNECTTOUT))
    DeleteTimer(PARTYDISCONNECTTOUT);
  StartTimer(PARTYDISCONNECTTOUT, 60*SECOND);

  switch(m_CallConnectionState)
  {
    case GkARJ: //ARQ reject: need to release port. not do the DRQ
    case ReleasePort://ARQ failure: need to release port. not do the DRQ
    case ZeroIp://zero Ip with no gatekeeper
    {
      if ((m_CallConnectionState == GkARJ) || (m_CallConnectionState == ReleasePort))
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - state of ARQ reject or Failure");

      else if (m_CallConnectionState == ZeroIp)
        PTRACE(eLevelError,"CH323Cntl::OnPartyCallDropReq - state of The Ip address is Zero!");

      DisconnectForCallWithoutSetup();
      return;
    }

    case GkARQ:  //in case of ARQ req: Gk has not responded
    case McmsNotAcceptAcfParams:
    {
      if (m_CallConnectionState == GkARQ)
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - state of Gatekeeper has failed to Response until now");
        m_CallConnectionState = GkDRQafterARQ;
      }
      else if (m_CallConnectionState == McmsNotAcceptAcfParams)
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - state of Gk is: Mcms Not Accept Acf Params");
      }
      CreateAndSendDRQReq(cmRASDisengageReasonNormalDrop);
      return;
    }

    case GetPort:
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - state of GetPort Failure");
      if( m_getPortInd.srcCallSignalAddress.transAddr.port != NO_PORT_LEFT)//add for cases when the operator disconnect before
      {                        //the GetPortIndication returned from the card.
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - maybe getPortInd hadn't arrive yet");
        StartTimer(H323_GETPORTFAILURE, 3*SECOND);
        m_CallConnectionState = GetPortDisconnecting;
      }
      else
        RemoveAndDisconnectCall();
      return;
    }

    case ControlPortsDeficiency:
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - state of Control Port Deficiency");
      RemoveAndDisconnectCall();
      return;
    }

    case RouteCallToGK:
    {
      m_state   = DISCONNECT;
      m_pmcCall->SetCallStatus(-2);

      //StartTimer(FORWARDINGTIMER, 30 * SECOND);
      CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
      CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
      if (pServiceParams == NULL)
        PASSERTMSG(m_pmcCall->GetConnectionId(), "CH323Cntl::OnPartyCallDropReq - IP Service does not exist!!!");

      else
      {
//        char* alternativeAlias = NULL;
        //DWORD gkIp = pServiceParams->GetGKIp();
        //if (gkIp != 0)
                if (pServiceParams->isGKExternal())
        {
/*          ALLOCBUFFER(dstAddr, 18);
          strncat(dstAddr,"TA:",3);

          ALLOCBUFFER(destIpAddress, 15);
          SystemDWORDToIpString(gkIp, destIpAddress);
          strcat(dstAddr,destIpAddress);
          DEALLOCBUFFER (destIpAddress);*/
          OnPartyCallAnswerReq(cmReasonTypeRouteCallToGatekeeper, 1);//dstAddr);
//          DEALLOCBUFFER (dstAddr);
        }

//        PDELETEA(alternativeAlias);
      }
      return;
    }

	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }

  if(m_pmcCall->GetCallCloseInitiator() == McInitiator)
  {
    if (m_isReceiveCallIdleMessage) //callIdle after the outgoingchannel timer finish!
    {
      if (!m_pmcCall->GetChannelsCounter())
        OnPartyCallCloseConfirmReq();
    }
    else
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - callCloseInitiator = mcInitiator");
      bIsSendCallDrop = TRUE;
    }
  }
  else if (m_pmcCall->GetCallCloseInitiator() == NoInitiator)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - callCloseInitiator = NoInitiator");
  //  m_pmcCall->SetCallCloseInitiator(McInitiator);
    bIsSendCallDrop         = TRUE;

  }
  else if(m_pmcCall->GetCallCloseInitiator() == GkInitiator)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - callCloseInitiator = GkInitiator");
    bIsSendCallDrop         = TRUE;
  }
  else
  { //closing initiator is the EP - getting here from conf after party notify conference on closing
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - callCloseInitiator = PmInitiator");
    if(!m_pmcCall->GetChannelsCounter() && !m_pmcCall->GetNumOfInternalChannels())
      OnPartyCallCloseConfirmReq();
    else
    {
      PTRACE(eLevelError,"CH323Cntl::OnPartyCallDropReq - Channels counter isn't zero");
      for (int i=0; i<m_maxCallChannel; i++)
      {
    	CChannel* pChannel = m_pmcCall->GetSpecificChannel(i);
        if (pChannel)
        {
          DBGPASSERT(pChannel->GetCsIndex());
          break;
        }
      }
    }
  }

  if (bWaitForCloseUdp || m_pmcCall->GetNumOfInternalChannels())
  {
    if(bIsSendCallDrop)
      ON(m_isCallDropRequestWaiting);

    PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyCallDropReq - we delay the call drop until all udp channels are closed !!!");

    bIsSendCallDrop = FALSE;
  }

  if (bIsSendCallDrop)
  {
      TRACEINTO << "SendCallDrop";
    SendCallDrop();
  }


}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendCallDrop()
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SendCallDrop - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());

  OFF(m_isCallDropRequestWaiting);

  if (m_pmcCall->GetCallCloseInitiator() == NoInitiator)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::SendCallDrop - callCloseInitiator = mcInitiator");
    m_pmcCall->SetCallCloseInitiator(McInitiator);
  }
  mcReqCallDrop* pCallDrop = new mcReqCallDrop;
  pCallDrop->rejectCallReason = m_CallConnectionState;
  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)(pCallDrop),sizeof(mcReqCallDrop));
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_CALL_DROP_REQ,pMsg,m_serviceId,
                m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
  POBJDELETE(pMsg);
  PDELETE(pCallDrop);
}

////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsChannelInDisconnectProcessByMcms()
{
  BYTE isMcmsDisconnectChannelInProgress = 0;

  // We need to check if any of the still opened channels are in state of
  // disconnection which was initiated by MCMS request.
  for (int i=0; i<m_maxCallChannel; i++)
  {
  	CChannel* pChannel = (m_pmcCall) ? m_pmcCall->GetSpecificChannel(i) : NULL; //KW 1235
    if (pChannel)
    {
      if (pChannel->GetChannelCloseInitiator() == McInitiator )
      {
        isMcmsDisconnectChannelInProgress = 1;
        return isMcmsDisconnectChannelInProgress;
      }
    }
  }

//  // check internal channels
//  if (m_pmcCall->GetNumOfInternalChannels() > 0)
//  {
//      TRACEINTO << "mix_mode: Waiting to close internal channels... number of still open channels=" << m_pmcCall->GetNumOfInternalChannels();
//      isMcmsDisconnectChannelInProgress = 1;
//      return isMcmsDisconnectChannelInProgress;
//  }

  return isMcmsDisconnectChannelInProgress;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Status is the enum callDisconnectedStateModes that indicate of the reason
//             for the disconnect of the call
//---------------------------------------------------------------------------------------------------
void  CH323Cntl::OnH323CallIdleInd(CSegment* pParam)
{
//  m_pLoadMngrConnector->CallCloseExitCriticalSection();
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  m_pmcCall->SetIsClosingProcess(TRUE);
  m_isReceiveCallIdleMessage    = TRUE;

  OFF(m_isIncomingAudioHasDisconnectedOnce);
  OFF(m_isOutgoingAudioHasDisconnectedOnce);

  // Trace of the Call Idle
  TRACEINTO << "CH323Cntl::OnH323CallIdleInd - Party Name is " << PARTYNAME
        << ", McmsConnId is = " << m_pCsRsrcDesc->GetConnectionId()
        << ", Call Close Initiator is = " << g_initiatorOfCloseStrings[m_pmcCall->GetCallCloseInitiator()];

  if (IsValidTimer(FORWARDINGTIMER))    //in case of forwarding: call idle has come befor the FORWARDINGTIMER end
    DeleteTimer(FORWARDINGTIMER);

  LogicalChannelDisConnect(H225);
  LogicalChannelDisConnect(H245);

  if(m_pmcCall->GetCallCloseInitiator() == PmInitiator)
  { //in case this is the second call_idle that we received for the same call,
    //and when the first one was sent, when we came to the func OnPartyCallDropReq, at
    //least one of the channels was opened. Therefore, call_close_confirm_req wasn't sent.
    //We should send callClose Confirm now ONLY if all the channel had been closed. If not we will wait to the next
    //callIdle (In case MCMS open two outgoing channels and then callIdle - the card should reject both of them and
    //will send for each of them callidle)

    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323CallIdleInd - PmInitiator\n");

    if (m_isReceiveCallDropMessage && !m_isCloseConfirm && !m_pmcCall->GetChannelsCounter())
      OnPartyCallCloseConfirmReq();
  }

  else if(m_pmcCall->GetCallCloseInitiator() == NoInitiator)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323CallIdleInd - NoInitiator");
    *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

    status = (APIS32)stat1;

    if(m_pmcCall->GetIsOrigin())// dial out
    {// in dial in we recive the useruser at the offering ind, in dial out at the call connected ind.
      if(!m_pmcCall->GetCallIndex())
      {
        m_callIndex = callIndex;
        m_pmcCall->SetCallIndex(m_callIndex);
      }
    }
    if (callIndex != m_callIndex)
    {
      PASSERTMSG(callIndex,"CH323Cntl::OnH323CallIdleInd - Call Index incorrect");
      return;
    }
    if (srcUnitId != m_pDestUnitId)
    {
      PASSERTMSG(srcUnitId,"CH323Cntl::OnH323CallIdleInd - srcUnitId incorrect");
      return;
    }
    m_pmcCall->SetCallCloseInitiator(PmInitiator);

    APIS16 status1 = disconnectCauseEnum2Opcode(status);
    if (status == -2)
    {
      PTRACE(eLevelError,"CH323Cntl::OnH323CallIdleInd - Call was rejected - lack of control ports");
      DBGPASSERT(GetConnectionId());
      status1 = H323_CALL_CLOSED_NO_CONTROL_PORT_LEFT;
      m_CallConnectionState = ControlPortsDeficiency;
    }

    CSmallString msg;
    ::GetCallCloseStateModeName(status, msg);
    PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323CallIdleInd -  Disconnect cause is = ", msg.GetString());
    m_pTaskApi->H323PartyDisConnect(status1);   // 0 == Delete Party
  }
  else if (m_pmcCall->GetCallCloseInitiator() == McInitiator)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323CallIdleInd - McInitiator");

    if ( ((m_isReceiveCallDropMessage) || (m_isCallAnswerReject)) &&
        (!m_pmcCall->GetChannelsCounter()))
    {
      if (m_isCallingThroughGk)
        CreateAndSendDRQReq(cmRASDisengageReasonNormalDrop);
      else
        RemoveAndDisconnectCall();
    }
    //else - do not do anything - wait to callDrop.
  }
  else //m_pmcCall->callCloseInitiator == GkInitiator
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323CallIdleInd - GkInitiator");
    // if the Initiator is Gk and we received Call Idle (before sending Call Drop), we confirm the closing of the call.
    if(!m_isReceiveCallDropMessage)
      OnPartyCallCloseConfirmReq();
    else if(!m_pmcCall->GetChannelsCounter())
    {
      if(m_isCallingThroughGk)
        CreateAndSendDRQReq(cmRASDisengageReasonNormalDrop);
      else
        RemoveAndDisconnectCall();
    }
  }

  // BRIDGE-15573: disable CsKeepAlive
  if (IsValidTimer(PARTYCSKEEPALIVEFIRSTTOUT))
	  DeleteTimer(PARTYCSKEEPALIVEFIRSTTOUT);
  if (IsValidTimer(PARTYCSKEEPALIVESECONDTOUT))
	  DeleteTimer(PARTYCSKEEPALIVESECONDTOUT);
}

/////////////////////////////////////////////////////////////////////////////
//in case of call forward and the timer ended befor call idle come
void  CH323Cntl::OnH323ForwardDisconnect(CSegment* pParam)
{
  m_pmcCall->SetIsClosingProcess(TRUE);

  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ForwardDisconnect - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  RemoveAndDisconnectCall();
}

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::RemoveCsControllerResource()
{
  CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
  if (pRoutingTbl== NULL) {
    PASSERT_AND_RETURN(m_pCsRsrcDesc->GetConnectionId());
  }

  pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pCsRsrcDesc);
  pRoutingTbl->RemoveAllPartyRsrcs(m_pCsRsrcDesc->GetConnectionId());
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323ChannelNewRateInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ChannelNewRateInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIS8 arrayIndex = NA;
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323ChannelNewRateInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323ChannelNewRateInd - srcUnitId incorrect");
    return;
  }
  mcIndChannelNewRate pchnlNewRateInd;

  DWORD  structLen = sizeof(mcIndChannelNewRate);
  memset(&pchnlNewRateInd,0,structLen);
  pParam->Get((BYTE*)(&pchnlNewRateInd),structLen);

  CChannel  *pChannel = NULL;
  APIU32    contentRate = 0;

  int retval = SetCurrentChannel(channelIndex,mcChannelIndex, &pChannel, &arrayIndex);

  if(pChannel == NULL)
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323ChannelNewRateInd - Channel was not found");
    DBGPASSERT((APIU8)arrayIndex);
    return;
  }
  if (pChannel->GetType() == cmCapVideo)
  {
     if( (kRolePeople == pChannel->GetRoleLabel()) && (cmCapTransmit == pChannel->GetChannelDirection()))
     {
        m_pTaskApi->UpdatePartyH323VideoBitRate(pchnlNewRateInd.rate, cmCapTransmit, kRolePeople);
     }
  }
    if (pChannel->GetType() == cmCapVideo &&  pChannel->GetRoleLabel() == kRolePresentation && !( m_pParty->IsCallGeneratorParty() ) )
    {
        if (IsSlaveCascadeModeForH239() && pchnlNewRateInd.rate && pchnlNewRateInd.rate != m_lastContentRateFromMasterForThisToken)
        {
            //This is a request from the master Notify conf on rate from master
            m_pTaskApi->NotifyConfOnMasterActionsRegardingContent(MASTER_RATE_CHANGE, pchnlNewRateInd.rate);
            m_lastContentRateFromMasterForThisToken = pchnlNewRateInd.rate;
        }


        else if (IsRemoteACascadedMcuWithH239Enabled() && m_bWaitForFlowCntlIndIndOnContent)
        {
            //Response from slave - end change mode
            SendEndChangeContentToConfLevel(statOK);
            m_bWaitForFlowCntlIndIndOnContent = FALSE;
        }
    }
    if (IsRemoteIsSlaveMGCWithContent())
    {
      //In case of master we should return flowControlInd to the slave
      PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323ChannelNewRateInd:MASTER - answer FlowControlIndInd : Name - ",PARTYNAME);
      OnPartyFlowControlReq(pChannel,pchnlNewRateInd.rate);
    }

}

///////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::SendFlowControlReq(cmCapDataType eType,BYTE bIsTransmit,ERoleLabel eRole,DWORD newRate)
{
  BYTE bSent = FALSE;
  CChannel *pChannel = FindChannelInList(eType,bIsTransmit,eRole);
  if (pChannel)
  {
    if (pChannel->GetCsChannelState() != kConnectedState)
      PTRACE2INT(eLevelError,"CH323Cntl::SendFlowControlReq: The channel isn't connected - ", m_pCsRsrcDesc->GetConnectionId());
    else
    {
      //verify that on video flow control, audio+video rate not exceeding
      //conference rate
      if (eType == cmCapVideo && kRolePeople == eRole)
      {
        // check for audio channel + connected
        CChannel* pAudioChannel =  FindChannelInList(cmCapAudio,bIsTransmit,kRolePeople);
        if (pAudioChannel && pAudioChannel->GetCsChannelState() != kConnectedState)
        {
          DWORD audioRate = pAudioChannel->GetRate() * 10;
          DWORD call_rate = m_pTargetModeH323->GetCallRate() * 10;

          if (newRate + audioRate > call_rate)
          {
            newRate = call_rate - audioRate;
            PTRACE2INT(eLevelError,"CH323Cntl::SendFlowControlReq rate exceeds call rate, updating rate to ",newRate);
          }
        }
      }

      bSent = TRUE;
      OnPartyFlowControlReq(pChannel,newRate);
      if (eRole & kRoleContentOrPresentation)
        m_targetConfContRate = newRate;
      //for people, it is saved in m_curPeopleRate
    }
  }
  else
    PTRACE2INT(eLevelError,"CH323Cntl::SendFlowControlReq - Channel not found - ", m_pCsRsrcDesc->GetConnectionId());

  return bSent;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnPartyFlowControlReq(CChannel *pChannel,DWORD newVidRate)
{
  if( m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnPartyFlowControlReq:  bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }

  if(pChannel)
  {
    if (pChannel->GetCsChannelState() != kConnectedState)
    {
      PTRACE2INT(eLevelError, "CH323Cntl::OnPartyFlowControlReq - Channel isn't in connected state. ", m_pCsRsrcDesc->GetConnectionId());
      return;
    }
        if (!pChannel->IsOutgoingDirection() &&  pChannel->GetType() == cmCapVideo && pChannel->GetRoleLabel() == kRolePeople)
        {
            //Check constraint for incoming video people channel
            if (m_confPeopleFlowControlConstraint)
            {
                newVidRate = min(newVidRate, m_confPeopleFlowControlConstraint);
            }
        }


    mcReqChannelNewRate* pChnlNewRateReq = new mcReqChannelNewRate;
    pChnlNewRateReq->channelType = pChannel->GetType();
    pChnlNewRateReq->channelIndex = pChannel->GetCsIndex();
    pChnlNewRateReq->channelDirection = pChannel->IsOutgoingDirection();
    pChnlNewRateReq->rate = newVidRate;
    CSegment* pMsg = new CSegment;
    pMsg->Put((BYTE*)(pChnlNewRateReq),sizeof(mcReqChannelNewRate));
    m_pCsInterface->SendMsgToCS(H323_CS_SIG_CHNL_NEW_RATE_REQ,pMsg,m_serviceId,
                m_serviceId,m_pDestUnitId,m_callIndex,pChannel->GetCsIndex(),pChannel->GetIndex(),0);
    POBJDELETE(pMsg);
    PDELETE (pChnlNewRateReq);
  }
  else
    PTRACE(eLevelError,"CH323Cntl::OnPartyFlowControlReq - Channel not found");
}

///////////////////////////////////////////////////////////////////////////////////////////
BOOL  CH323Cntl::CheckFlowControlDetails(DWORD newVidRate, CChannel *pChannel)
{
    if (pChannel->IsOutgoingDirection() && newVidRate && (newVidRate < pChannel->GetRate()) )
  {
    PTRACE(eLevelError,"CH323Cntl::CheckFlowControlDetails - We don't send flow control to decrease outgoing channel rate");
    return FALSE;
  }
  else if (newVidRate > pChannel->GetRate())
  {
    CMedString mstr;
    mstr << "newVidRate = " << newVidRate;
    mstr << ",pChannel->GetRate() = "<< pChannel->GetRate();
    FPTRACE2(eLevelInfoNormal,"CH323Cntl::CheckFlowControlDetails - new rate is equal or bigger than party rate",mstr.GetString());
    return FALSE;
  }
  return TRUE;
}

// v4.1C <--> v6 merge ///////////////////////////////////////////////////////////////////////////////////////////
//// 1) Flow control is sent only in VSW_IP conferences
//// 2) Flow control for outgoing channel is sent only in case of moved party.
//
//BYTE  CH323Cntl::OnConfFlowControlReq(DWORD newVidRate, BYTE outChannel)
//{
//  if(m_pmcCall->GetIsClosingProcess() == TRUE)
//  {
//    PTRACE2INT(eLevelError,"CH323Cntl::OnConfFlowControlReq:  bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
//    return FALSE;
//  }
//  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnConfFlowControlReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
//
//  CChannel* pChannel = FindChannelInList(cmCapVideo, outChannel);
//  if(pChannel)
//  {
//    if (pChannel->GetCsChannelState() != kConnectedState)
//    {
//      PTRACE2INT(eLevelError, "CH323Cntl::OnConfFlowControlReq - Channel isn't in connected state - ", m_pCsRsrcDesc->GetConnectionId());
//      return FALSE;
//    }
//
//    BOOL bIsGoodDetail = TRUE;
//    bIsGoodDetail = CheckFlowControlDetails(newVidRate, pChannel);
//
//    if(!bIsGoodDetail)
//      return FALSE;
//
//    DWORD newVidRateToSend = newVidRate ? newVidRate : pChannel->GetRate();
//
//     lower the rate     if( (!pChannel->IsOutgoingDirection() && newVidRate && (newVidRate < m_pParty->GetVideoRate())) || //lower the rate for incoming channel
//      (pChannel->IsOutgoingDirection() && !newVidRate) )//or return to the origin rate for outgoing channel
//    {// send flow control and only then send stream on
//      OnPartyFlowControlReq(pChannel,newVidRateToSend);
//    }
//
//     upper the rate     if( (pChannel->IsOutgoingDirection() && newVidRate && (newVidRate > pChannel->GetRate()) || //upper the rate for outgoing channel
//      (!pChannel->IsOutgoingDirection() && !newVidRate) ) )//or return to the origin rate for incoming channel
//    {// send flow control and only then send stream on
//      OnPartyFlowControlReq(pChannel,newVidRateToSend);
//    }
//
//    if (pChannel->IsOutgoingDirection() && newVidRate) //outgoing channel
//    { //update the rates at the party's level
//      m_pParty->UpdateVideoRate(newVidRateToSend);
//    }
//
//     For incoming channel, the rates at the party's level aren't updated.
//          The reason is that the party should store the originate rates, so in case a request
//      to return to the origin incoming rate is sent, the origin outgoing rates must be
//      restored as well.
//    return TRUE;
//  }
//  else
//  {
//    PTRACE(eLevelError,"CH323Cntl::OnConfFlowControlReq - Channel not found");
//    return FALSE;
//  }
//}
// End v4.1c <--> v6 merge
///////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323ChannelMaxSkewInd(CSegment* pParam)
{
  if( m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnH323ChannelMaxSkewInd bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }

  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ChannelMaxSkewInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323ChannelMaxSkewInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323ChannelMaxSkewInd - srcUnitId incorrect");
    return;
  }
  mcIndChannelMaxSkew pchnlMaxSkewInd;

  DWORD  structLen = sizeof(mcIndChannelMaxSkew);
  memset(&pchnlMaxSkewInd,0,structLen);
  pParam->Get((BYTE*)(&pchnlMaxSkewInd),structLen);

  CChannel* pCurrrentChannel = FindChannelInList(mcChannelIndex);
  CChannel* pSecondChannel   = FindChannelInList(pchnlMaxSkewInd.secondChannelIndex);

  if (pCurrrentChannel && pSecondChannel)
  {
    CChannel* pCurrentSameSessionChannel = FindChannelInList(pCurrrentChannel->GetType(), !pCurrrentChannel->IsOutgoingDirection(), (ERoleLabel)pCurrrentChannel->GetRoleLabel()); ;
    CChannel* pSecondSameSessionChannel = FindChannelInList(pSecondChannel->GetType(), !pSecondChannel->IsOutgoingDirection(), (ERoleLabel)pSecondChannel->GetRoleLabel()); ;
    if (pCurrentSameSessionChannel && pSecondSameSessionChannel)
    {
      if( pSecondSameSessionChannel->GetCsChannelState() != kConnectedState)
      {
        PTRACE2(eLevelInfoNormal, "CH323Cntl::OnH323ChannelMaxSkewInd - Same session channel isn't in connected state. ", PARTYNAME);
        return;
      }
      mcReqChannelMaxSkew* pChnlMaxSkewReq = new mcReqChannelMaxSkew;
      pChnlMaxSkewReq->skew = pchnlMaxSkewInd.skew;
      pChnlMaxSkewReq->channelIndex = pSecondSameSessionChannel->GetCsIndex();
      pChnlMaxSkewReq->channelDirection = pSecondSameSessionChannel->IsOutgoingDirection();
      pChnlMaxSkewReq->channelType = pSecondSameSessionChannel->GetCsChannelState();

      CSegment* pMsg = new CSegment;
      pMsg->Put((BYTE*)(pChnlMaxSkewReq),sizeof(mcReqChannelMaxSkew));
      m_pCsInterface->SendMsgToCS(H323_CS_SIG_CHAN_MAX_SKEW_REQ,pMsg,m_serviceId,
          m_serviceId,m_pDestUnitId,m_callIndex,pSecondSameSessionChannel->GetCsIndex(),pSecondSameSessionChannel->GetIndex(),0);
      POBJDELETE(pMsg);

      PDELETE (pChnlMaxSkewReq);
    }
    else
      PTRACE(eLevelError,"CH323Cntl::OnH323ChannelMaxSkewInd - Channel without pSameSession");
  }
  else
    PTRACE(eLevelError,"CH323Cntl::OnH323ChannelMaxSkewInd - Channel is not found");
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnPartyGetPortReq()
{
  TRACESTR(eLevelInfoNormal) << " CH323Cntl::OnPartyGetPortReq - Conn Id = " << m_pCsRsrcDesc->GetConnectionId() << ",  Name - " << PARTYNAME;
//  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyGetPortReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  RecieveInfoFor_mcCall();

  m_CallConnectionState = GetPort;

  mcReqGetPort* pGetPort = new mcReqGetPort;
  // IpV6
  DWORD sourceIp = 0;
  pGetPort->srcIpAddress = sourceIp;

  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)(pGetPort),sizeof(mcReqGetPort));
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_GET_PORT_REQ,pMsg,m_serviceId,
              m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
  POBJDELETE(pMsg);
  PDELETE (pGetPort);


	StartTimer(PARTYCONNECTING, 200*SECOND); //20(stack)+30. Changed for homologation H323 from 50 to 200 to support timer T301 = 180 sec
}

//////////////////////////////////////////////////////////////////////////////shiraITP - 15 -CH323Cntl::OnH323GetPortInd
void  CH323Cntl::OnH323GetPortInd(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323GetPortInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  // only stat1 srcUnitId are valid values at this place.
  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  m_pDestUnitId = srcUnitId;

  DWORD  structLen = sizeof(mcIndGetPort);
  memset(&m_getPortInd,0,structLen);
  pParam->Get((BYTE*)(&m_getPortInd),structLen);

  m_NumOfGetPortMsg++;
  //for card debug
  if (m_NumOfGetPortMsg > 1)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnH323GetPortInd: received %d times",m_NumOfGetPortMsg);
    DWORD isReturnUnExpectedMsg = GetSystemCfgFlagInt<DWORD>(CFG_KEY_IP_UNEXPECTED_MESSAGE);
    if (isReturnUnExpectedMsg == YES)
    {
        int totalSize =
        sizeof(mcReqUnexpectedMessageBase) + sizeof(mcIndGetPort);

      mcReqUnexpectedMessage *pUnexpectedMessage = (mcReqUnexpectedMessage *)new BYTE[totalSize];

      pUnexpectedMessage->badOpcode   = H323_CS_SIG_GET_PORT_IND;
      pUnexpectedMessage->sizeMessage = sizeof(mcIndGetPort);
      memcpy(pUnexpectedMessage->message,&m_getPortInd,sizeof(mcIndGetPort));

      CSegment* pMsg = new CSegment;
      pMsg->Put((BYTE*)(pUnexpectedMessage),sizeof(totalSize));
      m_pCsInterface->SendMsgToCS(H323_CS_SIG_UNEXPECTED_MESSAGE_REQ,pMsg,m_serviceId,
                      m_serviceId,m_pDestUnitId,m_callIndex,0,0,status);
      POBJDELETE(pMsg);
      PDELETEA(pUnexpectedMessage);
    }

    DBGPASSERT_AND_RETURN(m_NumOfGetPortMsg);
  }

  // Handle two cases
  // 1. A call Drop message has receive from the party before getPortInd arrive
  // 2. The is no port left in the card.
  if(m_CallConnectionState == GetPortDisconnecting)
  {
      PTRACE(eLevelError,"CH323Cntl::OnH323GetPortInd - Call Is Being Disconnected");
    m_CallConnectionState = GetPort;
    OnH323GetPortFailed();
    return;
  }
  if(m_getPortInd.srcCallSignalAddress.transAddr.port == NO_PORT_LEFT)
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323GetPortInd: The call failed due to ports deficiency");
    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_NO_PORT_LEFT);
    m_pmcCall->SetCallCloseInitiator((DWORD)McInitiator);
    m_pmcCall->SetIsClosingProcess(TRUE);
    return;
  }
  m_CallConnectionState = Idle;

  m_pmcCall->UpdateSourceCallSignalAddressPort(m_getPortInd.srcCallSignalAddress.transAddr.port);
  m_pH323NetSetup->SetLocalH225Port(m_getPortInd.srcCallSignalAddress.transAddr.port);
  // IpV6
  const mcTransportAddress* pDestPartyTa = m_pH323NetSetup->GetTaDestPartyAddr();
  m_pmcCall->SetConferenceId(m_pH323NetSetup->GetH323ConfIdAsGUID());

  m_pmcCall->SetCallId(m_pH323NetSetup->GetCallId());

  CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
  CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
  if (pServiceParams == NULL)
  {
    PASSERTMSG(m_pmcCall->GetConnectionId(), "CH323Cntl::OnH323GetPortInd - IP Service does not exist!!!");
    return;
  }

  BOOL bIsGkExternal = pServiceParams->isGKExternal();
  BOOL bIsPrimaryNet = m_pH323NetSetup->IsItPrimaryNetwork();
  if (bIsGkExternal && bIsPrimaryNet)
  {
    SetSrcSigAddressAccordingToDestAddress();
        m_pmcCall->UpdateSourceCallSignalAddressPort(m_getPortInd.srcCallSignalAddress.transAddr.port);
        m_pH323NetSetup->SetLocalH225Port(m_getPortInd.srcCallSignalAddress.transAddr.port);
    m_CallConnectionState = GkARQ;
    CreateAndSendARQReq();
  }

  else if(isApiTaNull(const_cast<mcTransportAddress*>(pDestPartyTa)) != TRUE)//destIpAddress[0] != 0)
  {// No Gatekeeper
//    m_pmcCall->bIsAuthenticated = FALSE;
    m_CallConnectionState = Idle;
    OnPartyCallSetupReq();
  }

  else
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323GetPortInd: disconnect call by mcms because dest ip is zero");
    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_NO_DESTINATION_IP_ADDRESS);

    m_pmcCall->SetCallCloseInitiator((DWORD)McInitiator);
    m_pmcCall->SetIsClosingProcess(TRUE);

    m_CallConnectionState = ZeroIp;
    return;
  }
}


////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::CreateAndSendARQReq()
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CreateAndSendARQReq - ConnId:", m_pCsRsrcDesc->GetConnectionId());

  //calculate len
  char* sourceAliases = "";
  int sourceAliasesLen = 0;
  if (m_pmcCall->GetNumOfSrcAliases())
  {
    sourceAliases = m_pmcCall->GetSourceInfoAlias();
    sourceAliasesLen = strlen(sourceAliases);
  }

  int dynamicLen = (sourceAliasesLen + 1); //for ';' after the source aliases

  BYTE bIsCallWithIp = FALSE;
  if (m_pmcCall->GetIsOrigin()) // dial out.
  {
    if (!::isApiTaNull(m_pH323NetSetup->GetTaDestPartyAddr()))
      bIsCallWithIp = TRUE;
  }

  char* destAliases = m_pmcCall->GetDestinationInfoAlias();
  int destAliasesLen = strlen(destAliases);
  if (!bIsCallWithIp)
    dynamicLen += (destAliasesLen + 1); //for ';' after the dest aliases

  dynamicLen += 1; //for \0 +

  DWORD arqStructLen = sizeof(gkReqRasARQ) - 1 + dynamicLen;
  gkReqRasARQ* pARQReq = (gkReqRasARQ*)new BYTE[arqStructLen];

  if (pARQReq)
  {
    memset(pARQReq, '\0', arqStructLen);

    if (!::isApiTaNull(m_pH323NetSetup->GetTaSrcPartyAddr())) // dial-in or dial-out to an IP address
      memcpy(&(pARQReq->srcCallSignalAddress.transAddr), &(m_pmcCall->GetSrcTerminalParams().callSignalAddress), sizeof(mcTransportAddress));
    else // dial-out to alias
    {
      if (m_pmcCall->GetIsOrigin())
        memcpy(&(pARQReq->srcCallSignalAddress.transAddr), &(m_pmcCall->GetSrcTerminalParams().callSignalAddress), sizeof(mcTransportAddress));
      else
        memset(&pARQReq->srcCallSignalAddress, 0, sizeof(mcXmlTransportAddress)); // the gate-keeper need to discard this field in cases of routed
    }

    if (bIsCallWithIp) // dial-out to an ip address
      memcpy(&(pARQReq->destCallSignalAddress.transAddr), &(m_pmcCall->GetDestTerminalParams().callSignalAddress), sizeof(mcTransportAddress));
    else // dial-out with an alias or dial-in
    {
      memset(&pARQReq->destCallSignalAddress, 0, sizeof(mcXmlTransportAddress)); // the gate-keeper need to discard this field in cases of routed
     pARQReq->destCallSignalAddress.transAddr.ipVersion = m_pH323NetSetup->GetIpVersion(); 	 //BRIDGE-15392: set dst IP version
    }
    //fill srcAndDestInfo
    pARQReq->srcInfoLength = sourceAliasesLen + 1; //for ';'
	memcpy(pARQReq->srcAndDestInfo, sourceAliases, sourceAliasesLen);
	pARQReq->srcAndDestInfo[sourceAliasesLen] = '\0';
	strcat(pARQReq->srcAndDestInfo, ";");

    if (!bIsCallWithIp)
    {
    	pARQReq->destInfoLength = destAliasesLen + 1;//for ';'
		strncat(pARQReq->srcAndDestInfo, destAliases, destAliasesLen);
		strcat(pARQReq->srcAndDestInfo, ";");
    }
    else
      pARQReq->destInfoLength = 0;

    //fill srcInfoTypes and destInfoTypes
    DWORD* pSrcInfoAliasType = m_pmcCall->GetSrcInfoAliasType();
    if (pSrcInfoAliasType)
      for (int k = 0; k < MaxNumberOfAliases; k++)
        pARQReq->srcInfoTypes[k] = pSrcInfoAliasType[k];

    if (!bIsCallWithIp)
    {
      DWORD* pDestInfoAliasType = m_pmcCall->GetDestInfoAliasType();
      if (pDestInfoAliasType)
        for (int k = 0; k < MaxNumberOfAliases; k++)
          pARQReq->destInfoTypes[k] = pDestInfoAliasType[k];
    }

    //Fill additional values
    memcpy(pARQReq->cid, m_pH323NetSetup->GetH323ConfIdAsGUID(), Size16);
    memcpy(pARQReq->callId, m_pH323NetSetup->GetCallId(), Size16);
    pARQReq->bandwidth = (2 * (m_pH323NetSetup->GetMaxRate())); // mult by 2 for transmit + receive
    BOOL bIsCpRegardToIncomingSetupRate = 0;
    CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    std::string key = "CP_REGARD_TO_INCOMING_SETUP_RATE"; //yael!!!
    pSysConfig->GetBOOLDataByKey(key, bIsCpRegardToIncomingSetupRate);
    if (bIsCpRegardToIncomingSetupRate && (m_pTargetModeH323->GetConfType() == kCp || m_pTargetModeH323->GetConfType() == kCop) && m_pH323NetSetup->GetRemoteSetupRate()
        < m_pH323NetSetup->GetMaxRate() && m_pH323NetSetup->GetRemoteSetupRate() != 0)
    {
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CreateAndSendARQReq - (m_pH323NetSetup->GetRemoteSetupRate() in",m_pH323NetSetup->GetRemoteSetupRate());
      pARQReq->bandwidth = (2 * (m_pH323NetSetup->GetRemoteSetupRate()));
    }
    pARQReq->callType = m_pmcCall->GetCallType();
    pARQReq->callModel = m_pmcCall->GetCallModelType();
    pARQReq->bIsDialIn = !m_pmcCall->GetIsOrigin();
    pARQReq->bCanMapAlias = m_pmcCall->GetCanMapAlias();

    m_CallConnectionState = GkARQ;

    //Send req to GK manager:
    CSegment* pSeg = new CSegment;

    *pSeg << arqStructLen;
    pSeg->Put((BYTE*)pARQReq, arqStructLen);

    HeaderToGkManagerStruct headerToGkManager = SetHeaderToGkManagerStruct();
    DWORD headerLen = sizeof(HeaderToGkManagerStruct);
    pSeg->Put((BYTE*)&headerToGkManager, headerLen);

    SendReqToGkManager(pSeg, H323_CS_RAS_ARQ_REQ);

    //ARQ STATE
    BYTE gkState = eGKAdmission;
    DWORD reqBandwidth = (pARQReq->bandwidth) / 1000; //kbps
    GatekeeperStatus(gkState, reqBandwidth);
    PDELETEA(pARQReq);
  }
  else
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::CreateAndSendARQReq - pARQReq NULL pointer !");
    PASSERT(GetConnectionId());
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::DisconnectCallBecauseInsufficientBandwidth()
{
  m_pmcCall->SetIsClosingProcess(TRUE);
  m_pmcCall->SetCallCloseInitiator(McInitiator);
  if (IsValidTimer(PARTYCONNECTING))
    DeleteTimer(PARTYCONNECTING);
  if (IsValidTimer(AUDCONNECTTOUT))
    DeleteTimer(AUDCONNECTTOUT);
  if(IsValidTimer(OTHERMEDIACONNECTED))
    DeleteTimer(OTHERMEDIACONNECTED);
  if(IsValidTimer(CODIANVIDCHANTOUT))
    DeleteTimer(CODIANVIDCHANTOUT);
  m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_SMALL_BANDWIDTH);
  PASSERTMSG(m_pmcCall->GetConnectionId(), "CH323Cntl::DisconnectCallBecauseInsufficientBandwidth");
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::CloseAllChannelsBesideAudio()
{
    // Video Channel
    if (m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople))
    {
        PTRACE2INT(eLevelInfoNormal, "CH323Cntl::CloseAllChannelsBesideAudio - Close Video", m_pmcCall->GetConnectionId());
        CloseOutgoingChannel(cmCapVideo);
    }
    // Presentation Channel
    if (m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
    {
        PTRACE2INT(eLevelInfoNormal, "CH323Cntl::CloseAllChannelsBesideAudio - Close Presentation", m_pmcCall->GetConnectionId());
        CloseOutgoingChannel(cmCapVideo, kRoleContentOrPresentation);
    }
    // Data Channel
    if (m_pCurrentModeH323->IsMediaOn(cmCapData, cmCapTransmit, kRolePeople))
    {
        PTRACE2INT(eLevelInfoNormal, "CH323Cntl::CloseAllChannelsBesideAudio - Close Data", m_pmcCall->GetConnectionId());
        CloseOutgoingChannel(cmCapData);
    }
  CSecondaryParams secParams;
  m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_AVF_INSUFFICIENT_BANDWIDTH, secParams);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::HandleBandwidthAvaya(gkIndBRQFromGk* pBRQfromGKIndSt)
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::HandleBandwidthAvaya: bandwidth received = ", pBRQfromGKIndSt->bandwidth / 100);
  BOOL bChangeAudio = TRUE;
  BOOL bWithoutVideo = FALSE;

  if(pBRQfromGKIndSt->bandwidth < (int)m_pmcCall->GetBandwidth())
  {
    // handling for lower bandwidth
    PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidthAvaya - lower bandwidth");
    m_pmcCall->SetBandwidth(pBRQfromGKIndSt->bandwidth);

    //here put the handling for low rate
    if (pBRQfromGKIndSt->bandwidth < rate128K)
    {
      // close the call.
      // In Avaya environment bandwidth<128K means disconnect call.
      PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidthAvaya - Close the call because of insufficient bandwidth");
      DisconnectCallBecauseInsufficientBandwidth();
    }
    else // bandwidth >=  rate128K
    {
      if (m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople)
        || m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation)
          || m_pCurrentModeH323->IsMediaOn(cmCapData, cmCapReceiveAndTransmit, kRolePeople))
          {
        //close all outgoing channels beside audio channel
        PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidthAvaya - Download to secondary. Close all channels beside the audio because of insufficient bandwidth.");
        m_pTaskApi->DowngradeToSecondary(SECONDARY_CAUSE_AVF_INSUFFICIENT_BANDWIDTH);
          }
          else // video channels are close
          {
            PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidthAvaya - There are non-audio channels that open");

        if(m_pCurrentModeH323->IsMediaOn(cmCapAudio, cmCapReceiveAndTransmit, kRolePeople))
            {
              PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidthAvaya - Audio on");
              bChangeAudio = FALSE;
            }
            else
              PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidthAvaya - Audio off");

            BOOL bWithoutVideo = FALSE;
            HandleBandwidth(bWithoutVideo, bChangeAudio);
            if (bWithoutVideo)
        {
          CSecondaryParams secParams;
          m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_AVF_INSUFFICIENT_BANDWIDTH, secParams);
        }
            //if (m_CapabilityNegotiation & kLocalCapsSent)
            //{
            //  PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidthAvaya - Should send caps again");
        //  OnPartyCreateControl(TRUE);
            //}
            }
    }
  }
  else
    PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidthAvaya - equal or higher bandwidth");
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::HandleBandwidth(BOOL &bWithoutVideo, BOOL bChangeAudio /*default: TRUE*/)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::HandleBandwidth - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  DWORD mcCallBw       = m_pmcCall->GetBandwidth();
  DWORD acfBandwidth   = mcCallBw / 2; //gk returns bw for both sides
  DWORD setupBandwidth = m_pH323NetSetup->GetMaxRate();
  if (setupBandwidth < rate16K)
    setupBandwidth = rate16K;
  EConfType   confType = m_pTargetModeH323->GetConfType();
  bWithoutVideo = FALSE;

  if (acfBandwidth < setupBandwidth)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::HandleBandwidth: bandwidth below minimum = ", mcCallBw/100);
    CCommConf* pCommConf = NULL;
  /*  if(m_pParty->IsGateway())
      pCommConf = ::GetpGateWayDB()->GetCurrentConf(m_pParty->GetConfId());
    else*/
      pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
    if (pCommConf)
    {
      CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
      if (pConfParty)
      {
        BYTE networkType = pCommConf->GetNetwork();
        DWORD confRate   = 0;
        DWORD videoRate  = 0;
        BYTE isTreatAudRateAsZero = bChangeAudio; // TRUE = Take audio bit raOnPartyCallSetupReqte as 0
        // Recalculating the rates
        ReCalculateRateForIpCpDialInCalls(pConfParty, m_pTargetModeH323, networkType, acfBandwidth, confRate, videoRate,isTreatAudRateAsZero);
        if (videoRate < rate56K)
        {
          PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidth - Video rate < 56K - can't open video");
          bWithoutVideo = TRUE;
        }

        if (m_pTargetModeH323->GetConfType() == kVideoSwitch || m_pTargetModeH323->GetConfType() == kVSW_Fixed)
        {
          PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidth - Conf type is video switch - can't open video");
          bWithoutVideo = TRUE;
        }

        //update ratess in party
        m_pParty->UpdateVideoRate(videoRate/100);
        videoRate = videoRate/100;
        PTRACE2INT(eLevelError,"CH323Cntl::HandleBandwidth: ConfRate = ", confRate);
        PTRACE2INT(eLevelError,"CH323Cntl::HandleBandwidth: Video Rate = ", videoRate);
        // Updating NetSetUp rate
        m_pH323NetSetup->SetMaxRate(confRate);
        // Updating the Target mode - Choosing new Audio rate and settingnew Video rate
        BYTE isReplaceAudio = bChangeAudio; // TRUE = Replace existing audio alg.
        m_pTargetModeH323->SetVideoBitRate(videoRate, cmCapReceiveAndTransmit);
        m_pTargetModeH323->SetAudioAlg(FALSE, videoRate, pConfParty->GetName(),isReplaceAudio);
        m_pTargetModeH323->SetTotalVideoRate(videoRate);
        m_pCurrentModeH323->SetTotalVideoRate(videoRate);
        // Updating the Target mode - set new video params according to new rate if needed
		CapEnum capCodeRec = (CapEnum)(m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapReceive));
		CapEnum capCodeTx  = (CapEnum)(m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit));
		if( (capCodeRec == eH264CapCode) && (capCodeTx == eH264CapCode) )
		{
			H264VideoModeDetails h264VidModeDetails;
			eVideoQuality vidQuality = pCommConf->GetVideoQuality();
			BYTE isHighprofile = (m_pTargetModeH323->IsH264HighProfile(cmCapReceive) || m_pTargetModeH323->IsH264HighProfile(cmCapTransmit));
			::GetH264VideoParams(h264VidModeDetails, videoRate*100, vidQuality, eHD1080At60Asymmetric, isHighprofile);

			long newFs = h264VidModeDetails.maxFS;
			if( newFs == (long)INVALID )
			{
				CH264Details thisH264Details = h264VidModeDetails.levelValue;
				newFs = thisH264Details.GetDefaultFsAsDevision();
			}

			APIU16 profile = H264_Profile_BaseLine;
			APIU8 level = 0;
			long oldFs = 0, oldMbps = 0, sar = 0, staticMB = 0, dbp = 0, brAndCpb = 0;
			m_pTargetModeH323->GetH264Scm(profile, level, oldMbps, oldFs, dbp, brAndCpb, sar, staticMB, cmCapReceive);
			if( oldFs == (long)INVALID )
			{
				CH264Details thisH264Details = h264VidModeDetails.levelValue;
				oldFs = thisH264Details.GetDefaultFsAsDevision();
			}

			if( newFs < oldFs ) // => change resolution in target mode
			{
				// update target mode - fs and mbps
				PTRACE(eLevelInfoNormal,"CH323Cntl::HandleBandwidth :  change resolution due to bandwidth restriction");
				m_pTargetModeH323->SetH264Scm(profile , h264VidModeDetails.levelValue ,h264VidModeDetails.maxMBPS, h264VidModeDetails.maxFS, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, h264VidModeDetails.maxStaticMbps, cmCapReceiveAndTransmit);
			}
		}
        PTRACE2INT(eLevelInfoNormal,"CH323Cntl::HandleBandwidth :  Party total video rate - ",videoRate);
        BYTE is4cifEnabledinOriginalCapsTx = FALSE;
        BYTE is4cifEnabledinOriginalCapsRx = FALSE;
        BYTE isH263H621inLocalCAps = m_pLocalCapH323->IsFoundOrH263H261();
        if(isH263H621inLocalCAps && m_pLocalCapH323->Get4CifMpi() != -1 )
        {
          is4cifEnabledinOriginalCapsTx = TRUE;
          PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidth - 4ciftr");
        }
        if( isH263H621inLocalCAps && m_pLocalCapH323->GetMpi(eH263CapCode,k4Cif) != ((APIU8)-1))
        {
          is4cifEnabledinOriginalCapsRx = TRUE;
          PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidth - rec");
        }
        // Re-Creating the Local capabilities
        POBJDELETE( m_pLocalCapH323);
        m_pLocalCapH323 = new CCapH323;

        // in case of audio only
        // we build only audio capabilities
        if (pConfParty->GetVoice() || bWithoutVideo)
        {
          m_pLocalCapH323->CreateAudioOnlyCap(videoRate, const_cast<CComModeH323*>(m_pTargetModeH323), pConfParty->GetName());
        }
        else
        {
          // Checking for content new rate
            CUnifiedComMode unifiedComMode(pCommConf->GetEnterpriseModeFixedRate(),pCommConf->GetConfTransferRate(),pCommConf->GetIsHighProfileContent());
            BYTE lConfRate = 0;
              lConfRate = CUnifiedComMode::TranslateIPRateToXferRate(confRate);
              eEnterpriseMode ContRatelevel = (eEnterpriseMode)pCommConf->GetEnterpriseMode();
              DWORD H323AMCRate = unifiedComMode.GetContentModeAMCInIPRate(lConfRate,ContRatelevel,
            		  (ePresentationProtocol)pCommConf->GetPresentationProtocol(),
            		  pCommConf->GetCascadeOptimizeResolution(), pCommConf->GetConfMediaType());
          if (H323AMCRate == 0)
          {
            m_pTargetModeH323->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
          }
          else
          {
            CapEnum H239Protocol = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
            BOOL bContentHD1080Enabled = FALSE;
            BYTE HDResRcvMpi = 0;
            BYTE HDResTxMpi = 0;
            BOOL bIsRcvContentHighProfile = m_pTargetModeH323->IsH264HighProfileContent(cmCapReceive);
            BOOL bIsTxContentHighProfile = m_pTargetModeH323->IsH264HighProfileContent(cmCapTransmit);
            HDResRcvMpi = m_pTargetModeH323->isHDContent1080Supported(cmCapReceive);
            HDResTxMpi  = m_pTargetModeH323->isHDContent1080Supported(cmCapTransmit);
            if(HDResRcvMpi && HDResTxMpi)
            {
              bContentHD1080Enabled = TRUE;
            }
            if(!bContentHD1080Enabled)
            {
              HDResRcvMpi = m_pTargetModeH323->isHDContent720Supported(cmCapReceive);
              HDResTxMpi  = m_pTargetModeH323->isHDContent720Supported(cmCapTransmit);
            }

             if (!IsTIPContentEnable())
             {
                m_pTargetModeH323->SetContent(H323AMCRate,cmCapReceive,H239Protocol,bContentHD1080Enabled,HDResRcvMpi,bIsRcvContentHighProfile);
                m_pTargetModeH323->SetContent(H323AMCRate,cmCapTransmit,H239Protocol,bContentHD1080Enabled,HDResTxMpi,bIsTxContentHighProfile);
             }
             else
             {
            	 CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
            	 if (pCommConf && pCommConf->GetIsPreferTIP())
            	 {
            		 m_pTargetModeH323->SetTIPContent(H323AMCRate,cmCapReceive,FALSE);
            		 m_pTargetModeH323->SetTIPContent(H323AMCRate,cmCapTransmit,FALSE);
            	 }
            	 else //eTipCompatibleVideoAndContent
            	 {
            		 m_pTargetModeH323->SetTIPContent(H323AMCRate,cmCapReceive);
            		 m_pTargetModeH323->SetTIPContent(H323AMCRate,cmCapTransmit);
            	 }
             }

          }

          // create with default caps and enable H263 4cif according to video parameters
          eVideoQuality vidQuality = pCommConf->GetVideoQuality();
          BYTE highestframerate = ((BYTE)eCopVideoFrameRate_None);
          if(m_pTargetModeH323->GetConfType() == kCop)
          {
                CVidModeH323* copHighetlevel= m_pCopVideoModes->GetVideoMode(0);
                WORD profile;
                BYTE level;
                long maxMBPS,maxFS,maxDPB,maxBR,maxSAR,maxStaticMbps,brandcpb;
                copHighetlevel->GetH264Scm(profile,level,maxMBPS,maxFS,maxDPB,maxSAR,brandcpb,maxStaticMbps);
                highestframerate = GetCopFrameRateAccordingtoMbpsAndFs(level,maxMBPS,maxFS);

          }
              m_pLocalCapH323->CreateWithDefaultVideoCaps(videoRate, m_pTargetModeH323, PARTYNAME, vidQuality,FALSE,0/*service id*/,((ECopVideoFrameRate)highestframerate));
              if(!isH263H621inLocalCAps)
            {
              PTRACE(eLevelInfoNormal, "CH323Cntl::HandleBandwidth - removeh263andh261");
              m_pLocalCapH323->RemovePeopleCapSet(eH263CapCode);
              m_pLocalCapH323->RemovePeopleCapSet(eH261CapCode);
            }
            if(!is4cifEnabledinOriginalCapsTx)
            {
              m_pLocalCapH323->Set4CifMpi ((APIS8)-1);

            }
            if(!is4cifEnabledinOriginalCapsRx)
              m_pLocalCapH323->SetH263FormatMpi(k4Cif, -1, kRolePeople);

        }
        m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
        COstrStream msg;
        m_pLocalCapH323->Dump(msg);
        PTRACE2(eLevelInfoNormal,"CH323Cntl::HandleBandwidth - new caps ", msg.str().c_str());
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323ARQInd(CSegment* pSeg)
{
  if (m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ARQInd bIsClosing process - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ARQInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  ArqIndToPartyStruct stToParty;
  DWORD stSize = sizeof (ArqIndToPartyStruct);
  memset(&stToParty, '\0', stSize);
  pSeg->Get((BYTE*)(&stToParty), stSize);

  gkIndRasARQ* pARQIndSt = new gkIndRasARQ; AUTO_DELETE(pARQIndSt);
  DWORD arqLen = sizeof(gkIndRasARQ);
  memset(pARQIndSt, 0 ,arqLen);
  pSeg->Get((BYTE*)(pARQIndSt), arqLen);

  APIS32 status = stToParty.status;

  //The flag of Avaya is now based on the info received in ACF/ARJ
  m_bIsAvaya = (pARQIndSt->avfFeVndIdInd.fsId == H460_K_FsId_Avaya) ? TRUE : FALSE;
  if (AVF_DEBUG_MODE == TRUE)
    m_bIsAvaya = TRUE;

  if (m_bIsAvaya)
    if (m_encAlg != kUnKnownMediaType)
      SetNonEncPartyInEncrConf();

  if (status != STATUS_OK)
  {
    APIU32 rejectReason = pARQIndSt->rejectInfo.rejectReason;
    CSmallString msg;
    ::GetRasRejectReasonTypeName(rejectReason, msg);
    PTRACE2 (eLevelInfoNormal,"CH323Cntl::OnH323ARQInd: reject reason ARJ = ", msg.GetString() );

    if (rejectReason == cmRASReasonRouteCallToGatekeeper)
    { //in this case we send facility route call to GK with the GK IP
			m_numOfGkRoutedAddres = pARQIndSt->numOfGkRoutedAddres;
			if(m_numOfGkRoutedAddres > 0)
			{
				if (pARQIndSt->gkRouteAddress.transAddr.ipVersion == eIpVersion4)
				{
					m_gkRouteAddress.addr.v4.ip	= pARQIndSt->gkRouteAddress.transAddr.addr.v4.ip;
				}
				else
				{		// Case IpV6
					m_gkRouteAddress.addr.v6.scopeId = pARQIndSt->gkRouteAddress.transAddr.addr.v6.scopeId;
					APIU32 i = 0;
					for (i = 0; i < IPV6_ADDRESS_BYTES_LEN; i++)
					{
						m_gkRouteAddress.addr.v6.ip[i] = pARQIndSt->gkRouteAddress.transAddr.addr.v6.ip[i];
					}
				}
				m_gkRouteAddress.port	= pARQIndSt->gkRouteAddress.transAddr.port;
				m_gkRouteAddress.distribution = eDistributionUnicast;
				m_gkRouteAddress.transportType = eTransportTypeTcp;
				m_gkRouteAddress.ipVersion = pARQIndSt->gkRouteAddress.transAddr.ipVersion;
			}

      m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_GATEKEEPER_REJECT_ARQ);
      m_pmcCall->SetCallCloseInitiator(GkInitiator);
      m_pmcCall->SetIsClosingProcess(TRUE);
      m_CallConnectionState = RouteCallToGK;
    }
  /*  else if (rejectReason == cmRASReasonPermissionDenied)
    {
      if(m_bIsAvaya)
      {
        //Need to change the master key
        BOOL reNegotiateDhKey = TRUE;
        OnH323ARQReq(reNegotiateDhKey);
      }
    }*/
    else if(rejectReason == cmRASReasonCalledPartyNotRegistered)
    {
        m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_REMOTE_UNREACHABLE);
        m_pmcCall->SetCallCloseInitiator(GkInitiator);
        m_pmcCall->SetIsClosingProcess(TRUE);
        m_CallConnectionState = GkARJ;
    }
    else
    {
      m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_GATEKEEPER_REJECT_ARQ);
      m_pmcCall->SetCallCloseInitiator(GkInitiator);
      m_pmcCall->SetIsClosingProcess(TRUE);
      m_CallConnectionState = GkARJ;
    }
  }
  else
  {
    m_pmcCall->SetReferenceValueForGk(pARQIndSt->crv);
    m_pmcCall->SetCallType(pARQIndSt->callType);
    m_pmcCall->SetCallModelType(pARQIndSt->callModel);
    m_pmcCall->SetBandwidth(pARQIndSt->bandwidth);
    if( /*(pARQIndSt->destCallSignalAddress.ip != 0) && */(m_pmcCall->GetIsOrigin()) )
    //according to the standard, the dest in the ACF should always (dialIn / dialOut) be
    //the remote. ThereforeOpenChannelFromMcms, we change the m_pmcCall->destTerminal only in case of dialOut,
    //because only then, it is the remote.
    {
      // src & dest addresses settings

      CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
      CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
      if (pServiceParams == NULL)
      {
        PASSERTMSG(m_pmcCall->GetConnectionId(), "CH323Cntl::OnH323ARQInd - IP Service does not exist!!!");
        return;
      }

      // verify we can communicate dest address
      if (!::isIpVersionMatchBetweenPartyAndService(&pARQIndSt->destCallSignalAddress.transAddr, pServiceParams))
      {
        PTRACE(eLevelError, "CH323Cntl::OnH323ARQInd - service addres have no matched ip as remote ip!!");
        m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_BY_MCU);
        m_pmcCall->SetCallCloseInitiator(McInitiator);
        m_pmcCall->SetIsClosingProcess(TRUE);
      }

      m_pmcCall->SetDestTerminalCallSignalAddress(pARQIndSt->destCallSignalAddress.transAddr);
      m_pH323NetSetup->SetTaDestPartyAddr(&pARQIndSt->destCallSignalAddress.transAddr);
      m_pH323NetSetup->SetIpVersion((enIpVersion)pARQIndSt->destCallSignalAddress.transAddr.ipVersion);

            //SetSrcSigAddressAccordingToDestAddress();
    }


    BOOL bWithoutVideo = FALSE;
    HandleBandwidth(bWithoutVideo);
    if (bWithoutVideo)
    {
      CSecondaryParams secParams;
      m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_GK_RETURNED_SMALL_BANDWIDTH, secParams);
    }

    if (m_pmcCall->GetReferenceValueForGk() == 0)
    {
      PTRACE(eLevelError,"CH323Cntl::OnH323ARQInd: Call refernce value is zero");
      m_CallConnectionState     = McmsNotAcceptAcfParams;

      m_pmcCall->SetIsClosingProcess(TRUE);
      m_pmcCall->SetCallCloseInitiator(McInitiator);
      m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_GATEKEEPER_REJECT_ARQ);
    }

    else
    {
      ON(m_isCallingThroughGk);
      m_CallConnectionState = CallThroughGk;

      m_pmcCall->SetReferenceValueForEp(m_pmcCall->GetReferenceValueForGk());//If the call went through the ARQ, the crv in the CALL_SETUP_REQ should be the same as the one that was sent in the ARQ.
      m_pmcCall->SetReferenceValueForGk(0);//The call task has different CRV at the GkIf

      m_pmcCall->SetConferenceId(pARQIndSt->conferenceId);
      m_pH323NetSetup->SetH323ConfIdAsGUID(pARQIndSt->conferenceId);
      m_pmcCall->SetCallId(pARQIndSt->callId);
      m_pH323NetSetup->SetCallId(pARQIndSt->callId);

      UpdateGkCallIdInCdr(pARQIndSt->callId);

      if (m_pmcCall->GetIsOrigin()) // dial out.
      {
         //Can Map alias:
        m_pDestExtraCallInfoTypes = new APIU32[MaxNumberOfAliases];
        m_pDestInfo = new char[MaxAddressListSize];
        m_pDestExtraCallInfo = new char[MaxAddressListSize];
        m_pRemoteExtensionAddress = new char[MaxAliasLength];

		m_pDestInfo[0] = '\0';

        if (m_pmcCall->GetCanMapAlias())
        {
          strncpy(m_pDestInfo, pARQIndSt->destInfo, MaxAddressListSize );
          m_pDestInfo[MaxAddressListSize-1] = '\0';
          strncpy(m_pDestExtraCallInfo, pARQIndSt->destExtraCallInfo, MaxAddressListSize );
          m_pDestExtraCallInfo[MaxAddressListSize-1] = '\0';
          strncpy(m_pRemoteExtensionAddress, pARQIndSt->remoteExtensionAddress, MaxAliasLength );
          m_pRemoteExtensionAddress[MaxAliasLength-1] = '\0';

          for (int i=0; i<MaxNumberOfAliases; i++)
            m_pDestExtraCallInfoTypes[i] = pARQIndSt->destExtraCallInfoTypes[i];
        }

      /*  WORD masterLen;
        *pSeg >> masterLen;

        //Only in avaya we can have master len, there are cases that alyhough we in avaya
        //we don't have master so we should continue as we not in avaya mode
        if(masterLen)
        {
          unsigned char *pMaster  = new unsigned char[masterLen];
          unsigned char *pAuthKey = new unsigned char[LengthEncAuthKey];
          unsigned char *pEncKey  = new unsigned char[LengthEncAuthKey];
          pSeg->Get((BYTE *)pMaster,masterLen);
          CreateAuthKey(pMaster,m_nonceArq,Pf1NonceSize,pARQReq.nonce,Pf1NonceSize,pAuthKey);
          CreateElementEncKey(pMaster,m_nonceArq,Pf1NonceSize,pARQReq.nonce,Pf1NonceSize,pEncKey);

          SetAuthEncrParams(m_pDHKeyManagement,pAuthKey,pEncKey,pARQReq.indexTblAuth);

          memcpy(m_sId,pARQReq.sid,2);

          PDELETEA(pMaster);
          PDELETEA(pAuthKey);
          PDELETEA(pEncKey);

          m_pmcCall->bIsAuthenticated = TRUE;
          OnH323RasAuthenticationReq();
        }
        else
        {
        m_pmcCall->bIsAuthenticated = FALSE;*/

        char *pPrefix = NULL;
        if (stToParty.gwPrefix[0] != '\0')
        {
          pPrefix = new char[PHONE_NUMBER_DIGITS_LEN + 1];
          memset(pPrefix, '\0', PHONE_NUMBER_DIGITS_LEN + 1);
          strncpy(pPrefix, stToParty.gwPrefix, PHONE_NUMBER_DIGITS_LEN);
        }

        CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
        if(pCommConf) //KW 5062
        {
        CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
        //OFIr: looks like not getting here not in dial in and not in dial out
        if (pConfParty)
        {
        	TRACEINTO << "OnH323ARQInd m_pH323NetSetup->GetH323ConfIdAsGUID()=" << m_pH323NetSetup->GetH323ConfIdAsGUID();
        	TRACEINTO << "OnH323ARQIndpIpNetSetup->GetCallId()=" <<GetGuidFormat( m_pH323NetSetup->GetCallId());

        	pConfParty->SetCorrelationId(GetGuidFormat(m_pH323NetSetup->GetCallId()));
        	PlcmCdrEventCallStartExtendedHelper cdrEventCallStartExtendedHelper;
        	cdrEventCallStartExtendedHelper.SetNewIsdnUndefinedParty_BasicAndContinue(*pConfParty, H323_INTERFACE_TYPE, *pCommConf);
        	pCommConf->SendCdrEvendToCdrManager((ApiBaseObjectPtr)&cdrEventCallStartExtendedHelper.GetCdrObject(), false, cdrEventCallStartExtendedHelper.GetCdrObject().m_partyDetails.m_id);
        }
		else
		{
			PASSERTMSG(1,"pConfParty is NULL");
		}
        }
        else
		{
			PASSERTMSG(1,"pCommConf is NULL");
		}


        OnPartyCallSetupReq (pPrefix, m_pDestExtraCallInfoTypes, m_pDestInfo, m_pDestExtraCallInfo, m_pRemoteExtensionAddress);

        PDELETEA(pPrefix);
        PDELETEA(m_pDestInfo);
        PDELETEA(m_pDestExtraCallInfo);
        PDELETEA(m_pRemoteExtensionAddress);
        PDELETEA(m_pDestExtraCallInfoTypes);
        //}
      }
      else  // dial In
      {
        if (m_bIsAvaya)
        {
          if (pARQIndSt->avfFeVndIdInd.fsId != H460_K_FsId_Avaya)
            PTRACE2INT(eLevelError,"CH323Cntl::OnH323ARQInd - Fs Id isn't Avaya. Connection Id = ", m_pmcCall->GetConnectionId());
          PTRACE (eLevelInfoNormal, "CH323Cntl::OnH323ARQInd - Dial in - Avaya Mode - update avf");

          m_remoteVendor.countryCode    = pARQIndSt->avfFeVndIdInd.countryCode;
          m_remoteVendor.t35Extension   = pARQIndSt->avfFeVndIdInd.t35Extension;
          m_remoteVendor.manufactorCode = pARQIndSt->avfFeVndIdInd.manfctrCode;
          if (pARQIndSt->avfFeVndIdInd.productId)
            strncpy(m_remoteVendor.productId, pARQIndSt->avfFeVndIdInd.productId,H460_C_ProdIdMaxSize);
          if (pARQIndSt->avfFeVndIdInd.versionId)
            strncpy(m_remoteVendor.versionId, pARQIndSt->avfFeVndIdInd.versionId,H460_C_VerIdMaxSize);
          if (pARQIndSt->avfFeVndIdInd.bSipCM == AVAYA_SIP_CM_FLAG_ON)
            m_remoteVendor.isAvayaSipCm = AVAYA_SIP_CM_FLAG_ON;
        }
        DumpRemoteVendorSt(m_remoteVendor);
        OnPartyCallAnswerReq();
      }

      if (pARQIndSt->irrFrequency )
      {//IRR_TIMER must be at least GapFromRealIrrInterval in seconds
        m_irrFrequency = pARQIndSt->irrFrequency;
		//-S- HOMOLOGATION 3.0 RAS_TE_STA_02 -----------------------//
        m_irrFrequency = max(GapFromRealIrrInterval, m_irrFrequency);
		//-E- HOMOLOGATION 3.0 RAS_TE_STA_02 -----------------------//
        StartTimer(IRR_TIMER, m_irrFrequency * SECOND);
      }
    }

    //In case it is avaya the conf will we update after we receive authentucationInd
    if(!m_bIsAvaya)
    {
      BYTE gkState = eGKAdmitted;
      DWORD reqBandwidth = 0xFFFFFFFF;
      DWORD allocBandwidth = (m_pmcCall->GetBandwidth())/1000; //kbps
      WORD requestInfoInterval = pARQIndSt->irrFrequency;
      BYTE gkRouted = m_pmcCall->GetCallModelType();

      GatekeeperStatus(gkState, reqBandwidth, allocBandwidth, requestInfoInterval, gkRouted);
    }
  }

  PDELETE(pARQIndSt);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::UpdateGkCallIdInCdr(char* callId)
{
  BYTE gkCallId[SIZE_OF_CALL_ID];
  memset(gkCallId,'\0',SIZE_OF_CALL_ID);
  memcpy(gkCallId, callId, SIZE_OF_CALL_ID);
  m_pTaskApi->UpdateGkCallIdInCdr(gkCallId);
}


////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnGkManagerStopIrrTimer(CSegment* pSeg)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnGkManagerStopIrrTimer - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  if (IsValidTimer(IRR_TIMER))
    DeleteTimer(IRR_TIMER);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnGkManagerRemoveCallReq(CSegment* pSeg)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnGkManagerRemoveCallReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  if (m_pmcCall->GetIsClosingProcess())
    return; //in order not to start the disconnecting process again.

  if (IsValidTimer(IRR_TIMER))
    DeleteTimer(IRR_TIMER);

  m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_GATEKEEPER_FAILURE);
}

///////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnGkManagerKeepAliveReq(CSegment* pSeg)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnGkManagerKeepAliveReq : Name - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());

    CSegment* pSegToParty = new CSegment;

  HeaderToGkManagerStruct headerToGkManager = SetHeaderToGkManagerStruct();
  DWORD headerLen = sizeof(HeaderToGkManagerStruct);
  pSegToParty->Put((BYTE*)&headerToGkManager, headerLen);

  SendReqToGkManager(pSegToParty, GK_MANAGER_PARTY_KEEP_ALIVE_IND);
}
 ///////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnRtpDtmfInputInd(CSegment* pSeg)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnRtpDtmfInputInd : Name - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: The function exist for immediatly failure of ARQ that can be detect at the
//             GateKeeperManager and not be send to the GateKeeper itself.
//---------------------------------------------------------------------------------------------------
void  CH323Cntl::OnH323GKFailInd(CSegment* pSeg)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323GKFailInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  DWORD failOpcode = 0;
  DWORD tempStatus = 0;
  *pSeg >> failOpcode
      >> tempStatus;

  eGkFailOpcode eStatus = (eGkFailOpcode)tempStatus;

  if (IsValidTimer(IRR_TIMER))
    DeleteTimer(IRR_TIMER);

  switch (failOpcode)
  {
    case H323_CS_RAS_ARQ_REQ:
    {
      if ( (m_CallConnectionState != GkDRQafterARQ ) && (m_CallConnectionState != GkARQ))
      {
        PTRACE(eLevelError,"CH323Cntl::OnH323GKFailInd: failure on ARQ request");
        return;
      }

      if( (m_pmcCall->GetIsClosingProcess() == TRUE) && (m_CallConnectionState == GkDRQafterARQ))
      {// in case of timeout to ArqReq in state of GkDRQafterARQ, party should not be informed
      //about starting of disconnecting process because it initiated this process.
        PTRACE(eLevelError,"CH323Cntl::OnH323GKFailInd: failure on ARQ request");
        DisconnectForCallWithoutSetup();
                m_CallConnectionState = ReleasePort; //should not sent DRQ
      }

      else if (eStatus == eArqTimeoutStatus)
      {
            PTRACE(eLevelError,"CH323Cntl::OnH323GKFailInd: failure on ARQ request - ARQ Timeout");
            // Handle Arqtimeout only when state is on GkARQ!
             if(m_CallConnectionState == GkARQ)
            {
                if(FALSE == CheckAndMakeH323CallOnGKFail())
                {
                    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_ARQTIMEOUT);
                    //m_CallConnectionState already been set in function CheckAndmakeH323CallOnGKFail()
                }
            }
             else
            {
                m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_ARQTIMEOUT);
                m_CallConnectionState = ReleasePort;
            }
        }

      else if (eStatus == eAltGkProcessStatus)
      {
            PTRACE(eLevelError,"CH323Cntl::OnH323GKFailInd: failure on ARQ request - Alt GK is in process");
            if( FALSE == CheckAndMakeH323CallOnGKFail())
            {
                m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_ALT_GK_FAILURE);
            }
      }

      else if (eStatus == eRegistrationProblemStatus) //Card isn't registered  - Arq wasn't sent
      {
                //---HOMOLOGATION --Start-------------------------------------------------------------------------------//
				//PTRACE(eLevelError,"CH323Cntl::OnH323GKFailInd: failure on ARQ request - card isn't registered");
				//m_pTaskApi->H323PartyDisConnect(CALLER_NOT_REGISTERED);
                PTRACE(eLevelError,"CH323Cntl::OnH323GKFailInd: failure on ARQ request - card isn't registered");
                if (m_pmcCall->GetIsOrigin()) // dial out.
                    OnPartyCallSetupReq();
                else
                    OnPartyCallAnswerReq();
                m_CallConnectionState = Idle; //
                //---HOMOLOGATION --End  -------------------------------------------------------------------------------//
      }
      //  m_CallConnectionState = ReleasePort; //should not sent DRQ
      break;
    }

    case H323_CS_RAS_IRR_REQ:
    {
      PTRACE(eLevelError,"CH323Cntl::OnH323GKFailInd: failure on IRR request - call wasn't found in table");
      //the Irr Timer was already deleted at the beginning of the function
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::CreateAndSendDRQReq(BYTE disengageReason)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CreateAndSendDRQReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  if (IsValidTimer(PARTYDISCONNECTTOUT))
    DeleteTimer(PARTYDISCONNECTTOUT);
  StartTimer (DRQ_TIMER, 35*SECOND); //18(stack) + 10(gk manager) + 7 spare

  if(m_CallConnectionState == CallThroughGk)
    m_CallConnectionState = GkDRQ;

	//if (IsValidTimer(IRR_TIMER))
	//	DeleteTimer(IRR_TIMER);

  gkReqRasDRQ* pDRQReq = new gkReqRasDRQ;
  memset(pDRQReq, '\0', sizeof(gkReqRasDRQ));

  pDRQReq->bIsDialIn       = !m_pmcCall->GetIsOrigin();
  pDRQReq->disengageReason = (cmRASDisengageReason)disengageReason;

  //Send req to GK manager:
  CSegment* pSeg = new CSegment;

  DWORD drqStructLen = sizeof(gkReqRasDRQ);
  *pSeg << drqStructLen;
  pSeg->Put((BYTE*)pDRQReq, drqStructLen);

  HeaderToGkManagerStruct headerToGkManager = SetHeaderToGkManagerStruct();
  DWORD headerLen = sizeof(HeaderToGkManagerStruct);
  pSeg->Put((BYTE*)&headerToGkManager, headerLen);

  SendReqToGkManager(pSeg, H323_CS_RAS_DRQ_REQ);
  PDELETE (pDRQReq);

    //DRQ STATE
    BYTE gkState = eGKDisengage ;
    GatekeeperStatus(gkState);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnGkMangerSendDrqIndOrFail(CSegment* pParam)
{
    //Michael - HOMOLOGATION--------------------------------------------------------------//
    if(TRUE == m_isCloseConfirm)
        m_pCsInterface->SendMsgToCS(H323_CS_SIG_CALL_CLOSE_CONFIRM_REQ,NULL,m_serviceId,
                                    m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
    //------------------------------------------------------------------------------------//
  if ( (m_CallConnectionState == McmsNotAcceptAcfParams) || //4.5 call drop adjustment
     (m_CallConnectionState == GkDRQafterARQ )) //In case that in state GkARQ there is call drop
    DisconnectForCallWithoutSetup();
  else
    RemoveAndDisconnectCall();

  m_CallConnectionState = Idle;
}

////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsRejectBRQ(gkIndBRQFromGk* pBRQfromGKIndSt
                          , int           * par_pOutMinMaxPossibleRate)//HOMOLOGATION. A negative value indicates MIN-rate from required (from GK); Positive: MAX-rate;
{
  BYTE bIsRejected = FALSE;

  if (m_pmcCall->GetIsOrigin()) // dial out.
  {
  //  EConfType confType = m_pTargetModeH323->GetConfType();
  //  if (confType == kCp)
  //  {
      //We should be after the setup and before the callConnectedInd
      if(m_state == SETUP && !m_isCallConnetIndArrived)
      {
        m_pmcCall->SetBandwidth(pBRQfromGKIndSt->bandwidth);
        BOOL bWithoutVideo = FALSE;
        HandleBandwidth(bWithoutVideo);
        if (bWithoutVideo)
        {
          CSecondaryParams secParams;
          m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_GK_RETURNED_SMALL_BANDWIDTH, secParams);
        }
      }
      else
      {
        bIsRejected = TRUE;
        PTRACE(eLevelError,"CH323Cntl::IsRejectBRQ - BRQ received not in the correct stage - should be rejected");
      }
  //  }
  //  else
  //  {
  //    bIsRejected = TRUE;
  //    PTRACE(eLevelError,"CH323Cntl::IsRejectBRQ - not cp conference - should be rejected");
  //  }
  }
  else
  {
     //--HOMOLOGATION. RAS_TE_BND_06, 07, 08 ---------------------------------------------------//
     //
        *par_pOutMinMaxPossibleRate = 0;
        CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
        std::string key = "BRQ_ON_CALL_START";
        DWORD hml_brq;
        sysConfig->GetDWORDDataByKey(key, hml_brq);
        //if( hml_brq )
        {
            if(  (pBRQfromGKIndSt->bandwidth > (POLYCOM_BANDWITH_MAX)) //In Bytes
               ||(pBRQfromGKIndSt->bandwidth < (POLYCOM_BANDWITH_MIN)) //In Bytes
              )
            {
                if(pBRQfromGKIndSt->bandwidth > (POLYCOM_BANDWITH_MAX))
                    *par_pOutMinMaxPossibleRate = POLYCOM_BANDWITH_MAX;

                if(pBRQfromGKIndSt->bandwidth < POLYCOM_BANDWITH_MIN)
                    *par_pOutMinMaxPossibleRate = (POLYCOM_BANDWITH_MIN) * -1;

                bIsRejected = TRUE;
                PTRACE2INT(eLevelError,"CH323Cntl::IsRejectBRQ - dial in - should be rejected - NOT in range [bandwidth: "
                           ,pBRQfromGKIndSt->bandwidth);
            }
            else
            {
                bIsRejected = FALSE;
                *par_pOutMinMaxPossibleRate = pBRQfromGKIndSt->bandwidth;//HOMOLOGATION

                PTRACE2INT(eLevelError,"CH323Cntl::IsRejectBRQ - dial in m_state: ", m_state);
                PTRACE2INT(eLevelError,"CH323Cntl::IsRejectBRQ - dial in m_isCallConnetIndArrived: ", m_isCallConnetIndArrived);

                //if(m_state == SETUP && !m_isCallConnetIndArrived)
                {
                    m_pmcCall->SetBandwidth(pBRQfromGKIndSt->bandwidth);
                    BOOL bWithoutVideo = FALSE;
                    HandleBandwidth(bWithoutVideo);
                    if (bWithoutVideo)
                    {
                        CSecondaryParams secParams;
                        m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_GK_RETURNED_SMALL_BANDWIDTH, secParams);
                    }
                }
                //else
                //{
                //    bIsRejected = TRUE;
                //    PTRACE(eLevelError,"CH323Cntl::IsRejectBRQ - BRQ received not in the correct stage - should be rejected");
                //}

            }
        }
     //   else
     ////-- HOMOLOGATION.END.---------------------------------------------------------------------//
     //   {
		   // bIsRejected = TRUE;
		   // PTRACE(eLevelError,"CH323Cntl::IsRejectBRQ - dial in - should be rejected");
     //   }
	}
  return bIsRejected;
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323GkBRQInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323GkBRQInd - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());

  BYTE bIsRejected = FALSE;
    int  nMinMaxRate = 0;//HOMOLOGATION. A negative value indicates MIN-rate from required (from GK); Positive: MAX-rate;

  gkIndBRQFromGk* pBRQfromGKIndSt = new gkIndBRQFromGk;
  DWORD structLen = sizeof(gkIndBRQFromGk);
  memset(pBRQfromGKIndSt, 0 ,structLen);
  pParam->Get((BYTE*)(pBRQfromGKIndSt), structLen);

  if (m_bIsAvaya || pBRQfromGKIndSt->avfFeMaxNonAudioBitRateInd.fsId == H460_K_FsId_Avaya)
    HandleBandwidthAvaya(pBRQfromGKIndSt);
  else
		bIsRejected = IsRejectBRQ(pBRQfromGKIndSt, &nMinMaxRate);//HOMOLOGATION. a negative value indicates MIN-rate from required (from GK); Positive: MAX-rate;

  //Send req to GK manager:
  gkReqBRQResponse* pBRQResponse = new gkReqBRQResponse;
  pBRQResponse->hsRas   = pBRQfromGKIndSt->hsRas;
  pBRQResponse->bandwidth = m_pmcCall->GetBandwidth();

  CSegment* pSeg = new CSegment;

  DWORD brqStructLen = sizeof(gkReqBRQResponse);
  *pSeg << brqStructLen;
  pSeg->Put((BYTE*)pBRQResponse, brqStructLen);

  APIS32 status = STATUS_OK;
  if (bIsRejected)
        //HOMOLOGATION. a negative value indicates MIN-rate from required (from GK); Positive: MAX-rate;
   		status = nMinMaxRate;//see: rate4K ...rate6144K
  HeaderToGkManagerStruct headerToGkManager = SetHeaderToGkManagerStruct(status);
  DWORD headerLen = sizeof(HeaderToGkManagerStruct);
  pSeg->Put((BYTE*)&headerToGkManager, headerLen);

  SendReqToGkManager(pSeg, H323_CS_RAS_BRQ_RESPONSE_REQ);
  PDELETE (pBRQResponse);

  PDELETE (pBRQfromGKIndSt);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323GkDRQInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323GkDRQInd - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());

  WORD bIsMessageFromRealGk;
  *pParam >> bIsMessageFromRealGk; // if it is 0, it means that message is from Gk Manager because of error handling flow

  if (bIsMessageFromRealGk)
  {
    //1- get structure:
    gkIndDRQFromGk* pGkDRQIndSt = new gkIndDRQFromGk;
    DWORD structLen = sizeof(gkIndDRQFromGk);
    memset(pGkDRQIndSt, 0 ,structLen);
    pParam->Get((BYTE*)(pGkDRQIndSt), structLen);

    gkReqDRQResponse* pGkDRQResponseSt = new gkReqDRQResponse;
    pGkDRQResponseSt->hsRas = pGkDRQIndSt->hsRas;

    //2- Send req to GK manager:
    CSegment* pSeg = new CSegment;

    DWORD drqStructLen = sizeof(gkReqDRQResponse);
    *pSeg << drqStructLen;
    pSeg->Put((BYTE*)pGkDRQResponseSt, drqStructLen);

    HeaderToGkManagerStruct headerToGkManager = SetHeaderToGkManagerStruct();
    DWORD headerLen = sizeof(HeaderToGkManagerStruct);
    pSeg->Put((BYTE*)&headerToGkManager, headerLen);

    SendReqToGkManager(pSeg, H323_CS_RAS_DRQ_RESPONSE_REQ);
    PDELETE (pGkDRQResponseSt);
    PDELETE (pGkDRQIndSt);
  }

  //3- handle party members:
  if (IsValidTimer(IRR_TIMER))
    DeleteTimer(IRR_TIMER);

  if (m_pmcCall->GetCallCloseInitiator() == NoInitiator)
    m_pmcCall->SetCallCloseInitiator(GkInitiator);
  else
    PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323GkDRQInd - The call has already has initiator - ", g_initiatorOfCloseStrings[m_pmcCall->GetCallCloseInitiator()]);

  m_pmcCall->SetIsClosingProcess(TRUE);

  if(m_CallConnectionState == McmsNotAcceptAcfParams)//4.5 call drop adjustment
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323GkDRQInd - state of Gk: Mcms Not Accept Acf Params");
    DisconnectForCallWithoutSetup();
  }
  else if (m_CallConnectionState != GkDRQ)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323GkDRQInd: disconnect call by GateKeeper");
    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_GATEKEEPER_DRQ);   // 0 == Disconnect Party
    m_CallConnectionState    = GkDRQ;
    OFF(m_isCallingThroughGk);
  }
  else
    RemoveAndDisconnectCall();
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323GkIRQInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323GkIRQInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  gkIndGKIRQ* pGkIRQIndSt = new gkIndGKIRQ;
  DWORD structLen = sizeof(gkIndGKIRQ);
  memset(pGkIRQIndSt, 0 ,structLen);
  pParam->Get((BYTE*)(pGkIRQIndSt), structLen);

  CreateAndSendIrrReq(H323_CS_RAS_IRR_RESPONSE_REQ, pGkIRQIndSt->hsRas);
  PDELETE(pGkIRQIndSt);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnIrrTimeout(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnIrrTimeout - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    CreateAndSendIrrReq(H323_CS_RAS_IRR_RESPONSE_REQ); // Michael - Change for Homologation
//	CreateAndSendIrrReq(H323_CS_RAS_IRR_REQ);
  StartTimer(IRR_TIMER, m_irrFrequency * SECOND);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::CreateAndSendIrrReq(DWORD opcode, int hsRas)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CreateAndSendIrrReq - ConnId:", m_pCsRsrcDesc->GetConnectionId());

  char* sourceAliases = m_pmcCall->GetSourceInfoAlias();
    //WORD srcInfoLength  = strlen(sourceAliases);
    //WORD wAliasesLen = strlen(sourceAliases);//Michael - HOMOLOGATION
    //srcInfoLength += 2; //for ';' //yael: new
    //if (srcInfoLength > MaxAddressListSize - 1)
    //    srcInfoLength = MaxAddressListSize - 1;

    DWORD irrStructLen = sizeof(gkReqRasIRR);// - sizeof(char) + srcInfoLength;
  gkReqRasIRR* pIrrReq = (gkReqRasIRR*)new BYTE[irrStructLen];
  memset(pIrrReq, '\0', irrStructLen);

  if (m_pmcCall->GetIsOrigin()) // dial out.
  {
    memcpy(&pIrrReq->srcCallSignalAddress, &m_getPortInd.srcCallSignalAddress, sizeof(mcXmlTransportAddress));
    memcpy(&pIrrReq->destCallSignalAddress.transAddr, &m_pmcCall->GetDestTerminalParams().callSignalAddress, sizeof(mcTransportAddress));
  }
  else // dial in (The src is the ARQ sender. with Gk the requesting terminal is always the source and in dial-in it is the MCU)
  {
        memcpy(&pIrrReq->srcCallSignalAddress.transAddr,  	&m_pmcCall->GetSrcTerminalParams().callSignalAddress,  sizeof(mcTransportAddress));
        memcpy(&pIrrReq->destCallSignalAddress.transAddr, 	&m_pmcCall->GetDestTerminalParams().callSignalAddress, sizeof(mcTransportAddress));
  }

    //Michael - HOMOLOGATION 2 --------------------------------------//
    //if(0 != wAliasesLen)
    //{
    //    memcpy(pIrrReq->srcInfo, sourceAliases, srcInfoLength-2);
    //    pIrrReq->srcInfo[srcInfoLength-2] = ';';
    //    pIrrReq->srcInfo[srcInfoLength-1] 	= '\0';
    //    pIrrReq->srcInfoLength = srcInfoLength-1;
    //}
    //else
    {
        pIrrReq->srcInfo[0] 	= '\0';
        pIrrReq->srcInfoLength  = 0;
    }
    //----------------------------------------------------------------//

  DWORD* pSrcInfoAliasType = m_pmcCall->GetSrcInfoAliasType();
  if (pSrcInfoAliasType)
    for (int k = 0; k < MaxNumberOfAliases; k++)
      pIrrReq->srcInfoTypes[k] = pSrcInfoAliasType[k];

  pIrrReq->hsRas = hsRas;
  pIrrReq->bIsNeedResponse = FALSE;
  pIrrReq->bandwidth = m_pmcCall->GetBandwidth(); //allocated bandwidth by gk
  pIrrReq->callType = m_pmcCall->GetCallType();
  pIrrReq->callModel = m_pmcCall->GetCallModelType();
  pIrrReq->bIsAnswer = !m_pmcCall->GetIsOrigin();
  //-S- HOMOLOGATION 3.0 RAS_TE_STA_02 -------------------------//
	memcpy(pIrrReq->conferenceId, m_pmcCall->GetConferenceId(),	Size16);  
	memcpy(pIrrReq->callId, m_pmcCall->GetCallId(),	Size16);
    pIrrReq->n931Crv         = m_pH323NetSetup->GetCallReferenceValue();//HOMOLOGATION. Test #RAS_TE_STA_02
  //-E- HOMOLOGATION 3.0 RAS_TE_STA_02 -------------------------//

  CSegment* pSeg = new CSegment;
  *pSeg << irrStructLen;
  pSeg->Put((BYTE*)pIrrReq, irrStructLen);

  HeaderToGkManagerStruct headerToGkManager = SetHeaderToGkManagerStruct();
  DWORD headerLen = sizeof(HeaderToGkManagerStruct);
  pSeg->Put((BYTE*)&headerToGkManager, headerLen);

  SendReqToGkManager(pSeg, opcode);
  PDELETEA(pIrrReq);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::CreateAndSendBRQReq(int newBandwidth)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CreateAndSendBRQReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  m_gkRequestedBrqBw = newBandwidth;

  gkReqRasBRQ* pBRQReq = new gkReqRasBRQ;
  pBRQReq->callType  = m_pmcCall->GetCallType();
  pBRQReq->bandwidth = newBandwidth;
  pBRQReq->bIsDialIn = !m_pmcCall->GetIsOrigin();

  CSegment* pSeg = new CSegment;
  DWORD brqStructLen = sizeof(gkReqRasBRQ);
  *pSeg << brqStructLen;
  pSeg->Put((BYTE*)pBRQReq, brqStructLen);

  HeaderToGkManagerStruct headerToGkManager = SetHeaderToGkManagerStruct();
  DWORD headerLen = sizeof(HeaderToGkManagerStruct);
  pSeg->Put((BYTE*)&headerToGkManager, headerLen);

  SendReqToGkManager(pSeg, H323_CS_RAS_BRQ_REQ);
  PDELETE (pBRQReq);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323BRQInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323BRQInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  gkIndRasBRQ* pBRQIndSt = new gkIndRasBRQ;
  DWORD structLen = sizeof(gkIndRasBRQ);
  memset(pBRQIndSt, 0 ,structLen);
  pParam->Get((BYTE*)(pBRQIndSt), structLen);

  DWORD confRate  = 0;
  CalculateNewBandwidth(confRate);
  confRate  = confRate / 1000; //divided by 1000 to get the values in k per seconds

  if (pBRQIndSt->rejectInfo.rejectReason)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnH323BRQInd: Gk rejected new bandwidth. confRate = ", confRate);
    DWORD mcCallBw =  m_pmcCall->GetBandwidth();
    if (confRate < mcCallBw)
    {//we wanted to lower the rate
      CSmallString str;
      str << " BRQ = " << confRate << ", ARQ = " << mcCallBw;
      PTRACE2(eLevelError,"CH323Cntl::OnH323BRQInd: GK reject BRQ, but BRQ was to reduce call bandwidth", str.GetString());
    }

    else
    {//we wanted to upgrade the rate
            if (pBRQIndSt->bandwidth > 0)
            {
                //No need to disconnect.
                DWORD gkBrjRate = pBRQIndSt->bandwidth/1000;
                HandleBCFAsFlowControl(gkBrjRate);
                PTRACE2INT(eLevelError,"CH323Cntl::OnH323BRQInd: Gk rejected BRQ with lower bandwidth than requested. handle as flow control. Returned bandwidth = ", gkBrjRate);
                //m_pTaskApi->DowngradeToSecondary(SECONDARY_CAUSE_GK_RETURNED_SMALL_BANDWIDTH);
            }
            else
            {
                m_pmcCall->SetIsClosingProcess(TRUE);
                m_pmcCall->SetCallCloseInitiator(McInitiator);
                m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_SMALL_BANDWIDTH);
            }

    }
  }

  else
  {
    BYTE bResendBRQ = FALSE;

    int brqIndBandwidth = pBRQIndSt->bandwidth;
    DWORD gkBcfRate = brqIndBandwidth / 1000;
    if (gkBcfRate < confRate)
    {
      if (!m_pParty->IsPartyInChangeVideoMode())
      {
        if (m_gkRequestedBrqBw == brqIndBandwidth)
        {//that means that the bw changed between sending BRQ and receiving BCF, as a result of change mode/ flow control
          PTRACE2INT(eLevelError,"CH323Cntl::OnH323BRQInd: Bandwidth need to be changed. Returned bandwidth = ", gkBcfRate);
          CreateAndSendBRQReq(confRate*1000);
          bResendBRQ = TRUE;
        }
        else
        {
                    HandleBCFAsFlowControl(gkBcfRate);
          PTRACE2INT(eLevelError,"CH323Cntl::OnH323BRQInd: Gk returned lower bandwidth than requested. handle as flow control. Returned bandwidth = ", gkBcfRate);
          //m_pTaskApi->DowngradeToSecondary(SECONDARY_CAUSE_GK_RETURNED_SMALL_BANDWIDTH);
        }
      }
      else//in case of change mode, we'll send again the request when the change mode process is finished.
        PTRACE2INT(eLevelError,"CH323Cntl::OnH323BRQInd: Gk returned lower bandwidth than requested. Wait until the end of change mode process. Returned bandwidth = ", gkBcfRate);
    }

    if (!bResendBRQ)
      m_pmcCall->SetBandwidth(brqIndBandwidth); //update allocated bandwidth
  }

  //update operator:
  BYTE  gkState   = eGKCallNone;
  DWORD requestBw   = confRate;
    DWORD allocatedBw = (m_pmcCall->GetBandwidth())/1000; //kbps
    GatekeeperStatus(gkState, requestBw, allocatedBw);

  PDELETE(pBRQIndSt);
}
/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::HandleBCFAsFlowControl (DWORD gkBcfRate)
{
    DWORD outAudBitRate = 0;
    CChannel* pOutAudio = FindChannelInList(cmCapAudio, TRUE);
    if (pOutAudio)
        outAudBitRate = pOutAudio->GetRate();
    DWORD allocatedBw = gkBcfRate; //kbps
    allocatedBw /= 2;

    if (!m_pParty->GetIsVoice())
    {
        DWORD newVidRate = allocatedBw - outAudBitRate;
        m_pTaskApi->UpdatePartyH323VideoBitRate(newVidRate*10, cmCapTransmit, kRolePeople);
    }
    else
	    PTRACE(eLevelInfoNormal,"CH323Cntl::HandleBCFAsFlowControl - AudioOnly party.");
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnGkManagerHoldGkReq(CSegment* pParam)
{
  DWORD opcode;
  *pParam >>opcode;

  switch (opcode)
  {
    case H323_CS_RAS_ARQ_REQ:
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnGkManagerHoldGkReq. Hold ARQ for connection Id = ", m_pmcCall->GetConnectionId());
      if (m_pmcCall->GetIsClosingProcess())
        return;
      if (m_CallConnectionState == GkARQ)
      {
        if(IsValidTimer(PARTYCONNECTING))
          DeleteTimer(PARTYCONNECTING);
        StartTimer(PARTYCONNECTING, 60*SECOND);
      }
      break;

    case H323_CS_RAS_DRQ_REQ:
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnGkManagerHoldGkReq. Hold DRQ for connection Id = ", m_pmcCall->GetConnectionId());
      if (IsValidTimer(DRQ_TIMER))
        DeleteTimer(DRQ_TIMER);
      StartTimer (DRQ_TIMER, 60*SECOND);
      m_pTaskApi->IncreaseDisconnctingTimerInPartyCntl();
      break;

    default:
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnGkManagerHoldGkReq - Illegal opcode = ", opcode);
      break;
  }
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnGkManagerResendGkReq(CSegment* pParam)
{
  DWORD opcode;
  *pParam >>opcode;

  if (m_pmcCall->GetIsClosingProcess())
    return;

  switch (opcode)
  {
    case H323_CS_RAS_ARQ_REQ:
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnGkManagerResendGkReq. Resend ARQ for connection Id = ", m_pmcCall->GetConnectionId());
      if (m_CallConnectionState == GkARQ)
        CreateAndSendARQReq();
      break;

    case H323_CS_RAS_BRQ_REQ:
    {
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnGkManagerResendGkReq. Resend BRQ for connection Id = ", m_pmcCall->GetConnectionId());
      SendBrqIfNeeded();
    }
    break;

    default:
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnGkManagerResendGkReq - Illegal opcode = ", opcode);
      break;
  }
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323PartyMonitoringInd(CSegment* pSeg)
{

  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323PartyMonitoringInd: ConId =   \n",m_pmcCall->GetConnectionId());

  if(m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323PartyMonitoringInd-- no monitoring party in CLOSING process!!");
    return;
  }

  TCmPartyMonitoringInd* pPartyMonitoringInd = (TCmPartyMonitoringInd *)pSeg->GetPtr();


  CChannel *pChannel = NULL;
//  DWORD channelIndex = -1;

  TRtpChannelMonitoringInd *pChannelMonitoring = (TRtpChannelMonitoringInd *)pPartyMonitoringInd->acMonitoringData;

    for(DWORD i=0;i<pPartyMonitoringInd->unNumOfChannels;i++)
  {
    // check if channel monitoring has a valid data and the channel exists.
    ERoleLabel eRole = kRolePeople;
    APIU32 channelType = pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType;
    if (channelType == (APIU32)kIpContentChnlType )
      eRole = kRoleContentOrPresentation;
    BOOL bChanDirection = ( (pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection == cmCapReceive) ? 0 : 1 );
    pChannel = m_pmcCall->FindChannelInList(
              ::ChannelTypeToDataType(
                  (kChanneltype)channelType,eRole),
                  bChanDirection,eRole);
    if (pChannel && pChannelMonitoring->tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel)
    {
      cmCapDataType   eType   = cmCapEmpty;

      // IpV6 - Monitoring
      mcTransportAddress localIp;
      mcTransportAddress remoteIp;

      memset(&localIp,0,sizeof(mcTransportAddress));
      memset(&remoteIp,0,sizeof(mcTransportAddress));

      EIpChannelType channType = ::CalcChannelType(pChannel->GetType(),pChannel->IsOutgoingDirection(),pChannel->GetRoleLabel(),pChannel->GetCapNameEnum());
      if (channType > FECC_OUT)
        continue; //illegal parameters from the card

      CPrtMontrBaseParams *pPrtMonitrParams = CPrtMontrBaseParams::AllocNewClass(channType);

      cmCapDirection eDirection = (pChannel->IsOutgoingDirection())? cmCapTransmit : cmCapReceive;
      CapEnum targetCapCode = (CapEnum)m_pTargetModeH323->GetMediaMode(pChannel->GetType(),eDirection,pChannel->GetRoleLabel()).GetType();
      char *pChannelParams = NULL;

      //In case there are the same I should check with the target mode - it could update without change the channel
      if(pChannel->GetCapNameEnum() == targetCapCode)
        pChannelParams = (char *)m_pTargetModeH323->GetMediaMode(pChannel->GetType(),eDirection,pChannel->GetRoleLabel()).GetDataCap();
      else//The channel should be closed but it did not closed yet.
        pChannelParams = pChannel->GetChannelParams();

      eType         = pChannel->GetType();
      // IpV6
      enIpVersion eIpAddrMatch = CheckForMatchBetweenPartyAndUdp(m_pH323NetSetup->GetIpVersion(),m_UdpAddressesParams.IpType);
      localIp.ipVersion = eIpAddrMatch;
      remoteIp.ipVersion = eIpAddrMatch;
      if (eIpAddrMatch == eIpVersion4)
      {
        localIp.addr.v4.ip = m_UdpAddressesParams.IpV4Addr.ip;
        remoteIp.addr.v4.ip = (pChannel->GetRmtAddress())->addr.v4.ip;

      }
      else
      {
        // --- UDP: array of addresses ---

        BYTE  place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(m_pH323NetSetup->GetTaDestPartyAddr(), m_UdpAddressesParams.IpV6AddrArray);

        memcpy(localIp.addr.v6.ip, m_UdpAddressesParams.IpV6AddrArray[place].ip, 16);
        localIp.addr.v6.scopeId = m_UdpAddressesParams.IpV6AddrArray[place].scopeId;
        memcpy(&remoteIp, (pChannel->GetRmtAddress()), sizeof(mcTransportAddress));
        remoteIp.addr.v6.scopeId = pChannel->GetRmtAddress()->addr.v6.scopeId;

      }

      localIp.port      = GetLclPort( eType, cmCapReceive, pChannel->GetRoleLabel());
      remoteIp.port     = pChannel->GetRmtAddressPort();  //!!!! here will be a problem with video-content

      UpdatePartyMonitoring((TRtpCommonChannelMonitoring*)pChannelMonitoring,pPrtMonitrParams,m_oldMediaBytes,pChannel->GetCapNameEnum(),channType,
                    pChannel->GetCsIndex(),m_pmcCall->GetCallIndex(),m_oldFrames,pChannel->GetRate(),pChannelParams,
                    &remoteIp,&localIp);//10

      LogicalChannelConnect(pPrtMonitrParams,(DWORD)channType,0);

      if( (channType==VIDEO_OUT) || (channType==VIDEO_CONT_OUT) )
      {
        BYTE intraSyncFlag;
        BYTE videoBCHSyncFlag;
        BYTE protocolSyncFlag;
        WORD bchOutOfSyncCount;
        WORD protocolOutOfSyncCount;
        DWORD streamVideoSyncParams;

        streamVideoSyncParams = ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unStreamVideoSync;

        VideoSyncParamsParser(streamVideoSyncParams,intraSyncFlag, videoBCHSyncFlag, bchOutOfSyncCount,
                    protocolSyncFlag, protocolOutOfSyncCount);

        m_pTaskApi->IpPartyMonitoringUpdateDB((BYTE)channelType, intraSyncFlag, videoBCHSyncFlag, bchOutOfSyncCount,
                              protocolSyncFlag, protocolOutOfSyncCount);
      }

      POBJDELETE(pPrtMonitrParams);
    }
    // Move the pointer to the next channel
    pChannelMonitoring = (TRtpChannelMonitoringInd *)((BYTE *)pChannelMonitoring + sizeof(TRtpVideoChannelMonitoring));
  }

}
/////////////////////////////////////////////////////////////////////////////
// IP_CM_PARTY_PACKET_LOSS_IND. Indicates on RTCP RR Packet loss
void  CH323Cntl::OnCmPacketLossInd(CSegment* pParam)
{
  // Retrieve data from segment received
  TCmPartyPacketLossInd dataStruct;
    memset(&dataStruct,0,sizeof(TCmPartyPacketLossInd));
    pParam->Get( (BYTE*)&dataStruct, sizeof(TCmPartyPacketLossInd));
    kChanneltype channelType = (kChanneltype)dataStruct.media_type;
    cmCapDirection channelDirection = (cmCapDirection)dataStruct.media_direction;

    CSmallString cLog;
    cLog << "channel type " << channelType << ", channel direction " << channelDirection;
    PTRACE2(eLevelInfoNormal,"CH323Cntl::OnCmPacketLossInd: ",cLog.GetString());

  if (m_pCurrentModeH323->GetIsLpr())
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnCmPacketLossInd - Lpr enabled. Should be handled by Lpr mechanism");
  }
  else
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnCmPacketLossInd - Currently isn't implemented in H323");  // currently implemented only for sip.
  }

}
////////////////////////////////////////////////////////////////////////////
void CH323Cntl::RetriveCNAMEInfoIfNeeded(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal,"CH323Cntl::RetriveCNAMEInfoIfNeeded");

  TCmRtcpCnameInfoAsStringInd dataStruct;

  memset(&dataStruct,0,sizeof(TCmRtcpCnameInfoAsStringInd));

  pParam->Get((BYTE*)&dataStruct, sizeof(TCmRtcpCnameInfoAsStringInd));

  PTRACE2(eLevelInfoNormal,"CH323Cntl::RetriveCNAMEInfoIfNeeded ",dataStruct.cCname );
  if(m_IsNeedToExtractInfoFromRtcpCname)
  {
    RetriveCnameInfoFromEpIfPosible(dataStruct.cCname);
    m_IsNeedToExtractInfoFromRtcpCname = FALSE;

  }
  else
    PTRACE(eLevelInfoNormal,"CH323Cntl::RetriveCNAMEInfoIfNeeded -no need to retrive new info");
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::FastUpdate(ERoleLabel eRole)
{
  CSegment *pParam = new CSegment;
  *pParam << (DWORD)eRole;
  DispatchEvent(H323_CS_SIG_VIDEO_UPDATE_PIC_REQ, pParam);
  POBJDELETE(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnAudioConnectTimeOutSetup(CSegment* pParam)
{
  if (m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnAudioConnectTimeOutSetup bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnAudioConnectTimeOutSetup - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());


  // if all channels are connected -> send 'connect'
  // to conf with remote caps & scm
  if( AreAllChannelsConnected())
  {
    ConnectPartyToConf();
    return;
  }
    // if at least audio channeles were opened
  if(m_pCurrentModeH323->IsMediaOn(cmCapAudio) && m_isAudioOutgoingChannelConnected)
  {
    //if video channels(in and out), were opened  we send the conference party scm with video
        //if only audio connected,party will be secondary
    m_pCurrentModeH323->SetMediaOff(cmCapData);

    m_state = CONNECT;

    ConnectPartyToConf();
  }
  else
  { // if audio was not opened after time out
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnAudioConnectTimeOutSetup: disconnect call because audio channels weren't opened before timeOut");
      m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_AUDIO_CHANNELS_NOT_OPEN);
    m_pmcCall->SetCallCloseInitiator(McInitiator);
    m_pmcCall->SetIsClosingProcess(TRUE);
  }
}


//////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnOtherMediaConnectTimeOutSetup(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnOtherMediaConnectTimeOutSetup - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  m_state = CONNECT;

  if ( m_pLocalCapH323->OnType(cmCapVideo) && m_pRmtCapH323->OnType(cmCapVideo) &&
    m_pLocalCapH323->IsVideoCapCodeMatch(m_pRmtCapH323) &&
    m_pCurrentModeH323->IsMediaOn(cmCapVideo) && m_isVideoOutgoingChannelConnected )
  { //In case both support in video and the video is on we call the update.
    UpdateParamsBeforeConnectingToConf(TRUE);
  }

  PTRACE(eLevelInfoNormal,"CH323Cntl::OnOtherMediaConnectTimeOutSetup, H323EndMediaChannelsConnect - StatOK");
  m_pTaskApi->H323EndMediaChannelsConnect(*m_pRmtCapH323, *m_pCurrentModeH323, statOK, IsLateReleaseOfVideoResources());

  SendBrqIfNeeded();
}

//////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnOtherMediaConnectTimeOutConnect(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnOtherMediaConnectTimeOutConnect - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  UpdateParamsBeforeConnectingToConf(FALSE);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::InitAllChannelsSeqNum()
{
    CChannel* pChannel = NULL;
    for(DWORD i=0; i < m_pmcCall->GetChannelsCounter(); i++)
    {
        pChannel = m_pmcCall->GetSpecificChannel(i);
        if (pChannel)
        {
            pChannel->SetSeqNumRtp(0);
            pChannel->SetSeqNumCm(0);
        }
    }

    if (m_pTargetModeH323->GetConfMediaType() == eMixAvcSvc)
    {
        for (int i = 0; i < MAX_INTERNAL_CHANNELS; i++)
        {
            pChannel = m_pmcCall->GetSpecificChannel(i, false);
            if (pChannel)
            {
                pChannel->SetSeqNumRtp(0);
                pChannel->SetSeqNumCm(0);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////
/* Timer: MFARESPONSE_TOUT.
 * ACK from MFA has not been received.
 * MFA_RESPONSE_TIME * SECOND
 * H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ || CONFPARTY_CM_OPEN_UDP_PORT_REQ || CONFPARTY_CM_CLOSE_UDP_PORT_REQ
 * */
void CH323Cntl::OnMfaReqToutAnycase(CSegment* pParam)
{
  CChannel  *pChannel = NULL;
  ECmUdpChannelState eUdpChannelState;
  // Setting the digits for the MCU_INTERNAL_PROBLEM
  MipHardWareConn mipHwConn = eMipNoneHw;
  MipMedia    mipMedia = eMipNoneMedia;
  MipDirection  mipDirect = eMipNoneDirction;
  MipTimerStatus  mipTimerStat = eMpiNoTimerAndStatus;
  MipAction   mipAction = eMipNoAction;
  BYTE      firstMipError = 0;

  if(m_pmcCall->GetIsClosingProcess() == TRUE)
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnMfaReqToutAnycase, Closing Process, - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  else
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnMfaReqToutAnycase, Normal State, - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  // print the request IDs of the timed out request
  std::list<DWORD>::iterator it;
  for (it = m_MfaReqIds.begin(); it != m_MfaReqIds.end(); ++it)
      TRACEINTO << "!!!!!! MFA request with no response Req:" << *it;


  for(DWORD i=0; i < m_pmcCall->GetChannelsCounter(); i++)
  {
    pChannel = m_pmcCall->GetSpecificChannel(i);
    if (pChannel)
    {
      eUdpChannelState = pChannel->GetCmUdpChannelState();
      if ((eUdpChannelState == kSendOpen || eUdpChannelState == kSendClose || pChannel->GetRtpPortChannelState() == kRtpPortOpenSent ||
        pChannel->GetRtpPortChannelState() == kRtpPortUpdateSent || eUdpChannelState == kNeedsToBeClosed) && (firstMipError == 0))
        {
            DWORD InternqalProblemOpcode = 0;
          TranslateAckToMipErrorNumber(mipHwConn, mipMedia, mipDirect, mipTimerStat, mipAction, NULL, pChannel);
          char mipNum[200];
                      memset(mipNum, '\0', 200);
          InternqalProblemOpcode = (mipHwConn*10000) + (mipMedia*1000) + (mipDirect*100) + (mipTimerStat*10) + mipAction;
          if( mipHwConn == 2 /*RTP*/ )
          {
            sprintf(mipNum," mipHwConn = %d mipMedia = %d mipDirect = %d  mipTimerStat = %d mipAction = %d ReqNum = %d InternalMcuOpcode = %d"
                ,(BYTE)mipHwConn,(BYTE)mipMedia,(BYTE)mipDirect,(BYTE)mipTimerStat, (BYTE)mipAction, (DWORD)pChannel->GetSeqNumRtp() , (DWORD)InternqalProblemOpcode );
          }
          else
          {
            sprintf(mipNum," mipHwConn = %d mipMedia = %d mipDirect = %d  mipTimerStat = %d mipAction = %d ReqNum = %d InternalMcuOpcode = %d"
                  ,(BYTE)mipHwConn,(BYTE)mipMedia,(BYTE)mipDirect,(BYTE)mipTimerStat, (BYTE)mipAction, (DWORD)pChannel->GetSeqNumCm() , (DWORD)InternqalProblemOpcode );

          }
          PTRACE2(eLevelInfoNormal,"CH323Cntl::OnMfaReqToutAnycase : ", mipNum);
          pChannel->SetSeqNumRtp(0);
          pChannel->SetSeqNumCm(0);
          firstMipError = 1;
        }

      if (eUdpChannelState == kSendClose) //timer on ack for close udp
      {
        DBGPASSERT(pChannel->GetCsIndex());
        pChannel->SetCmUdpChannelState(kRecieveCloseAck); //ACK has not been received
        if (pChannel->GetCsChannelState() == kWaitToSendChannelDrop)
        {
            SendChannelDropReq(pChannel);
            if(m_pmcCall->GetIsClosingProcess() == TRUE)
              pChannel->SetCsChannelState(kCheckSendCallDrop);
        }
        else
        {
          if (pChannel->GetCsChannelState() == kNoNeedToDisconnect || pChannel->GetCsChannelState() == kDisconnectedState)
            m_pmcCall->RemoveChannel(pChannel);
          SendCallDropIfNeeded();
        }
      }
      else if ((eUdpChannelState == kNeedsToBeClosed) || (eUdpChannelState == kSendOpen))//timer on ack for open udp
      {
        PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnMfaReqToutAnycase, Ack for open has never received, - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
                DBGPASSERT(pChannel->GetCsIndex());
        pChannel->SetCmUdpChannelState(kNotSendOpenYet);
      }
    }
  }

  m_pTaskApi->SetFaultyResourcesToPartyControlLevel(STATUS_FAIL,mipHwConn, mipMedia, mipDirect, mipTimerStat, mipAction);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnAudioConnectTimeOutConnect(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnAudioConnectTimeOutConnect - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::UpdateCurrentScmH323(CChannel *pCurrentChannel)
{
	cmCapDataType type = pCurrentChannel->GetType();
	cmCapDirection direction = ::CalcCmCapDirection(pCurrentChannel->IsOutgoingDirection());
	ERoleLabel eRole = (ERoleLabel)pCurrentChannel->GetRoleLabel();

  TRACEINTOFUNC << "media type = " << ::GetTypeStr(type);

	if (m_pTargetModeH323->IsMediaOn(type,direction,eRole))
	{
		if (direction==cmCapReceive)
		{
			m_pCurrentModeH323->SetMediaMode(pCurrentChannel->GetCapNameEnum(), pCurrentChannel->GetSizeOfChannelParams(),
                          (BYTE*)pCurrentChannel->GetChannelParams(), type, direction ,eRole, true);
      const std::list <StreamDesc> streamsDescList = m_pTargetModeH323->GetStreamsListForMediaMode(type,direction,eRole);
      if (streamsDescList.size() > 0)
      {// copy streams information
          TRACEINTOFUNC << "mix_mode: Update streams in current SCM for media type = " << ::GetTypeStr(type);
          m_pCurrentModeH323->SetStreamsListForMediaMode(streamsDescList, type,direction,eRole);
          m_pCurrentModeH323->Dump("CH323Cntl::UpdateCurrentScmH323 mix_mode: Update streams in current SCM", eLevelInfoNormal);
      }
		}
		else
		{
			const CMediaModeH323& rTargetMediaMode = m_pTargetModeH323->GetMediaMode(type,direction,eRole);
      m_pCurrentModeH323->SetMediaMode(rTargetMediaMode,type,direction,eRole, true);
			if (m_pTargetModeH323->GetConfType() == kCop && type==cmCapVideo && eRole==kRolePeople && direction==cmCapTransmit)
			{
				m_pCurrentModeH323->SetCopTxLevel(m_pTargetModeH323->GetCopTxLevel());
				PTRACE2INT(eLevelInfoNormal,"CH323Cntl::UpdateCurrentScmH323 - cop level:",m_pCurrentModeH323->GetCopTxLevel());
			}
		}
		APIU8 payloadType = pCurrentChannel->GetDynamicPayloadType();
		if (payloadType == 0)
			payloadType = pCurrentChannel->GetPayloadType();
		PTRACE2INT(eLevelInfoNormal,"CH323Cntl::UpdateCurrentScmH323 - Payload:",payloadType);	//temporary trace
		m_pCurrentModeH323->SetPayloadTypeForMediaMode(type, direction, eRole, (payload_en)payloadType);
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::CreateTargetComMode(BYTE bIsDataOnly)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CreateTargetComMode - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  WORD conf;

  /////////////////////////////////////////////////////////////////////////////
  // This function is used when we get remote caps (in m_pRmtCapH323)
  // from the end-point. We already have local caps (m_pLocalCapH323) ready
  // at this point ,so we can generate target communication mode from
  // local & remote caps. Target comMode is the mode we want to get for communication.
  // Thefore when all desiarable channals will be opened we will
  // have m_pTargetMode323 equal to m_pCurrentMode323.
  // So m_pCurrentMode323 contains parameters of the currently opened channals
  /////////////////////////////////////////////////////////////////////////////
  if(m_pTargetModeH323->GetConfType() == kCp || m_pTargetModeH323->GetConfType() == kCop)
    conf =  CP;
  else if(m_pTargetModeH323->IsAutoVideoResolution())
    conf = VSW_AUTO;
  else
    conf =  VSW; //FIX

  BYTE rval = 0;

  //In case we need to find match cap for data only -> it means all the other channels already open
  //so we do not need to update and improve other caps.
  if(!bIsDataOnly)
  {
    // Make sure that the Target mode and the first capability are the same
    // Because of audio only parties for example, ...
    // so any case we need to update the Xmit audio mode not only in:
    // In case of auto audio the conference scm is nullify, I need to update the target via the first cap audio
    m_pLocalCapH323->UpdateTargetMode((CAudModeH323&)m_pTargetModeH323->GetMediaMode(cmCapAudio,cmCapTransmit));

    //We need to ensure the initiate audio mode match to the remote cap. We shouldn't be hard with the audio caps.
    m_pLocalCapH323->EnsureAudioTargetMode((CAudModeH323&)m_pTargetModeH323->GetMediaMode(cmCapAudio,cmCapTransmit),m_pRmtCapH323);

    //We need to ensure the initiate video mode match to the remote cap.
    m_pLocalCapH323->EnsureVideoTargetMode((CVidModeH323&)m_pTargetModeH323->GetMediaMode(cmCapVideo,cmCapTransmit),m_pRmtCapH323);

    //in case of cascade get ride off the fecc
    if(m_pTargetModeH323->IsMediaOn(cmCapData, cmCapReceiveAndTransmit) && !m_pLocalCapH323->GetNumOfFeccCap())
      m_pTargetModeH323->SetMediaOff( cmCapData, cmCapReceiveAndTransmit);

    //We need to ensure the initiate FECC mode match to the remote cap.
    m_pLocalCapH323->EnsureDataTargetMode((CDataModeH323&)m_pTargetModeH323->GetMediaMode(cmCapData,cmCapTransmit),m_pRmtCapH323);

    // We have to make sure that we remove the Content caps from the Scm set before we build the new comm mode
    if ((m_pTargetModeH323->IsMediaOn(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation)) &&
            (m_pLocalCapH323->IsH239() == FALSE && m_pLocalCapH323->IsEPC() == FALSE))
      m_pTargetModeH323->SetMediaOff(cmCapVideo,cmCapReceiveAndTransmit,kRoleContentOrPresentation);

    //We need to ensure the initiate content mode match to the remote cap (now we have two set - H264 and H263).
    m_pLocalCapH323->EnsureContentTargetMode((CVidModeH323&)m_pTargetModeH323->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation),m_pRmtCapH323);
  }
  rval = m_pRmtCapH323->BuildComMode(m_pTargetModeH323,conf,bIsDataOnly, FALSE);
  if ( bIsDataOnly && rval == FOUND_DATA)
    return rval;

  if((rval != FULL_MATCH) && (rval != AUDIO_VIDEO))
    rval = m_pRmtCapH323->BuildComMode(m_pLocalCapH323,conf,m_pTargetModeH323);

  return rval;
}

////////////////////////////////////////////////////////////////////////////
/* If the remote has less video cap we update our video rate to the lowest
The problem with that function is that it asume the Terminals on the lan always has the same rate
for different type of video rate (ex. video channel 261, cif=1 or cif=2 or Qcif=1
will have the same video rate.
*/
BYTE CH323Cntl::UpdateVideoOutRates(CapEnum h323CapCode)
{
  BYTE bSuccess = TRUE;
  CCapSetInfo capInfo(h323CapCode);
  //If the mcms is the initiator of the channel we need to look at the transmit because this is what we initialize,
  //else the receive has been initialize when incoming channel opened
  //if(m_McmsOpenChannels)
  //  capInfo= (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit);
  //else


  DWORD remoteMaxVideoPeopleRate = m_pRmtCapH323->GetMaxVideoBitRate(capInfo, cmCapReceive, kRolePeople);
  DWORD localMaxVideoPeopleRate  = m_pLocalCapH323->GetMaxVideoBitRate(capInfo, cmCapReceiveAndTransmit, kRolePeople);
//  remoteMaxVideoPeopleRate = 0;
/* If it's COP, and we change the protocol of outgoing channel, there might be a chance
   that the new protocol isn't in our local caps. So we take the rate from other place.
   This can happen in the following scenario: VSW auto, with mode that was change to H263.
   Then move to COP, with H261 as the port mode.

  if((localMaxVideoPeopleRate == 0) && (m_pParty->IsPartyInChangeCopMode())
      localMaxVideoPeopleRate = m_pParty->GetVideoRate(); */
  if(localMaxVideoPeopleRate == 0 && m_pTargetModeH323->GetConfType() == kCop)
  {
    localMaxVideoPeopleRate = m_pTargetModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRolePeople);
    PTRACE(eLevelInfoNormal,"CH323Cntl::UpdateVideoOutRates");
  }

  DWORD newVideoRate = 0;

  if((localMaxVideoPeopleRate !=0) && (remoteMaxVideoPeopleRate !=0))
  {
    DWORD isRegard = GetSystemCfgFlagInt<DWORD>(CFG_KEY_CP_REGARD_TO_INCOMING_SETUP_RATE);
    if( (m_pTargetModeH323->GetConfType() == kCp) && !m_pmcCall->GetIsOrigin() && isRegard)//cp && dial in
    {
      WORD remoteSetupRate = (WORD)(m_pH323NetSetup->GetRemoteSetupRate() / 100);

      if (remoteSetupRate && (remoteSetupRate < remoteMaxVideoPeopleRate) )
      {//change the parameter of remoteMaxVideoPeopleRate
        //need to remove audio rate
        WORD audioRate = m_pTargetModeH323->GetMediaBitRate(cmCapAudio,cmCapReceive) * 10;
        remoteMaxVideoPeopleRate = remoteSetupRate - audioRate;
      }
    }

    BYTE bNeedToUpdate = FALSE;

    if (m_pTargetModeH323->GetConfType() == kCp)
    {
      DWORD oldVideoRate = m_pParty->GetVideoRate();

      newVideoRate = ChangeVideoRateInCp(oldVideoRate);

      // VNGR-4159
      if (newVideoRate <= 0)
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::UpdateVideoOutRates - No rate left for video ");
        bSuccess = FALSE;
        return bSuccess;
      }
      if (newVideoRate != oldVideoRate)
      {
        PTRACE2INT(eLevelInfoNormal,"CH323Cntl::UpdateVideoOutRates - local rate was change to ", newVideoRate);
        bNeedToUpdate = TRUE;
        localMaxVideoPeopleRate = newVideoRate;
      }
    }
    else
    { //VSW
      DWORD outAudBitRate = m_pTargetModeH323->GetMediaBitRate(cmCapAudio, cmCapTransmit);
      DWORD outVidBitRate = m_pTargetModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit);

      DWORD call_rate = m_pTargetModeH323->GetCallRate();

      if (outVidBitRate + outAudBitRate*10 > call_rate*10)
      {
        DWORD newVideoBitRate = (call_rate - outAudBitRate)*10;
        PTRACE2INT(eLevelError,"CH323Cntl::UpdateVideoOutRates - actual rate exceeds call rate, updating max rate to ",newVideoBitRate);
        if(m_pTargetModeH323->GetConfType() == kCop)
          m_pCurrentModeH323->SetVideoBitRate(newVideoBitRate, cmCapTransmit, kRolePeople );
        m_pTaskApi->UpdatePartyH323VideoBitRate(newVideoBitRate, cmCapTransmit, kRolePeople);
      }
    }

    if((WORD)localMaxVideoPeopleRate > remoteMaxVideoPeopleRate)
    {
      bNeedToUpdate |= DecideOnUpdatingOutRateAccordingToRemoteType(h323CapCode);
      if (bNeedToUpdate)
        newVideoRate = (DWORD)remoteMaxVideoPeopleRate;
    }
    if (newVideoRate)
      m_pParty->UpdateVideoRate(newVideoRate);
  }

  else
  {
    if (localMaxVideoPeopleRate <= 0)
      PTRACE(eLevelError,"CH323Cntl::UpdateVideoOutRates - local rate = zero");
    if (remoteMaxVideoPeopleRate <= 0)
      PTRACE(eLevelError,"CH323Cntl::UpdateVideoOutRates - remote rate = zero");
    bSuccess = FALSE;
  }
  return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
/* returned value:
FALSE => don't update outgoing rate (m_pParty rate) although the remote
has lower capabilities, because the remote is a "special endpoint". */
BYTE CH323Cntl::DecideOnUpdatingOutRateAccordingToRemoteType(CapEnum h323CapCode)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::DecideOnUpdatingOutRateAccordingToRemoteType - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  BYTE isIpOnlyConf = m_pParty->IsIpOnlyConf();
  CCapSetInfo capInfo(h323CapCode); //(CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo);
  DWORD localMaxVideoPeopleRate  = m_pLocalCapH323->GetMaxVideoBitRate(capInfo, cmCapReceiveAndTransmit, kRolePeople);
  DWORD remoteMaxVideoPeopleRate = m_pRmtCapH323->GetMaxVideoBitRate(capInfo, cmCapReceive, kRolePeople);

  switch (m_remoteIdent)
  {
    case RvMCU:
    case RvGWOrProxy:
    case VconEp:
    {
      //case 1: cascade to VS 128kbs conference which in the RV side:
      //This is a special cases in which we allow the outgoing channel
      //to have higher bit rate than the remote caps.
      if (isIpOnlyConf)//ip only conference
      {
        int isdnLocalMaxVideoRate = localMaxVideoPeopleRate - 16; //minus FAS+BUS
        if (isdnLocalMaxVideoRate - remoteMaxVideoPeopleRate < 10)
        {
          if (m_remoteIdent == RvGWOrProxy)//This is actually a call to Rv MCU through Proxy. Therefore,
            //we didn't recognize Rv MCU in the vendor field under the setup message.
          {
            PTRACE(eLevelInfoNormal,"CH323Cntl::DecideOnUpdatingOutRateAccordingToRemoteType - RV MCU");
            m_remoteIdent = RvMCU;
          }
          return FALSE; //at 128kbs Rv MCU declares on 1100 and we declare on 1120
        }

        else if( (m_remoteIdent == VconEp) && (localMaxVideoPeopleRate - remoteMaxVideoPeopleRate < 230))
        {// Vcon has 3 declared rates:
          // up to 128 its 105.2k (our is no more than 112k) => gap 6.8k
          // up to 384 its 345.9k (our is no more than 368k) => gap 22.1k
          // above it its 1428.8k (and we have no problem until E1)
          // Vcon can't do E1.
          // Since I failed to reproduce Vcon calculation I hard coded the max gap.
          // This calculation doesn't take in account G7231 since
          // we can't set it at the conference level.
          PTRACE(eLevelInfoNormal,"CH323Cntl::DecideOnUpdatingOutRateAccordingToRemoteType - Vcon Ep");
          return FALSE;
        }
      }

      else if (localMaxVideoPeopleRate - remoteMaxVideoPeopleRate < 10)
      {
        if (m_remoteIdent == RvGWOrProxy)//This is actually a call to Rv MCU through Proxy. Therefore,
          //we didn't recognize Rv MCU in the vendor field under the setup message.
        {
          PTRACE(eLevelInfoNormal,"CH323Cntl::DecideOnUpdatingOutRateAccordingToRemoteType - RV MCU");
          m_remoteIdent = RvMCU;
        }
        return FALSE; //at 128kbs Rv MCU declares on 1100 and we declare on 1104
      }

      else if( (m_remoteIdent == VconEp) && (localMaxVideoPeopleRate - remoteMaxVideoPeopleRate < MaxGapRateForVcon))
      {// Vcon has 3 declared rates:
        // up to 128 its 105.2k (our is no more than 112k) => gap 6.8k
        // up to 384 its 345.9k (our is no more than 377k) => gap 31.1k
        // above it its 1428.8k (and we have no problem until E1)
        // Vcon can't do E1.
        PTRACE(eLevelInfoNormal,"CH323Cntl::DecideOnUpdatingOutRateAccordingToRemoteType - Vcon Ep");
        return FALSE;
      }

      if (m_remoteIdent == RvGWOrProxy)
        m_remoteIdent = Regular; //we don't need this flag in this point, because this is not RV MVU

      break;
    }

	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }
  return TRUE; // because m_remoteIdent == Regular
}

///////////////////////////////////////////////////////////////////////////////
void CH323Cntl::UpdateRemoteCapsAccordingToRemoteType(CCapH323 &pTmpRmtCaps)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::UpdateRemoteCapsAccordingToRemoteType - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  if (m_remoteIdent == VconEp)
  {
  /*We decide to change Vcon caps rate according to highest possible local rate for the
  specific call rate, and not according to the current local rate. The reason is to ebable
  move between different confs with same call rate but with different audio algorithms.*/
    DWORD remoteRate = 0;
    DWORD highestPossibleLocalRate = 0;
    DWORD confRate = m_pmcCall->GetMaxRate();
    highestPossibleLocalRate = confRate - LOWEST_AUDIO_RATE;
    highestPossibleLocalRate /= 100;

    for (CapEnum protocol = FIRST_VIDEO_CAP; protocol < FIRST_DATA_CAP; protocol++)
    {
      remoteRate = m_pRmtCapH323->GetMaxVideoBitRate(protocol, cmCapReceive);
      if (remoteRate && (highestPossibleLocalRate - remoteRate < MaxGapRateForVcon))
      {// Vcon has 3 declared rates:
        // up to 128 its 105.2k (our is no more than 112k) => gap 6.8k
        // up to 384 its 345.9k (our is no more than 377k) => gap 31.1k
        // above it its 1428.8k (and we have no problem until E1)
        // Vcon can't do E1.
        CCapSetInfo capInfo = protocol;
        PTRACE2(eLevelInfoNormal,"CH323Cntl::UpdateRemoteCapsAccordingToRemoteType - Vcon Ep. Updated protocol = ", capInfo.GetH323CapName());
        //We remove the specific protocol, because Vcon has different rates for different protocols.
        pTmpRmtCaps.SetVideoBitRate(highestPossibleLocalRate, kRolePeople, protocol);
      }
    }
  }
  else if(m_remoteIdent == NetMeeting)
  {
    //In case of Net meeting I want to update the remote cap with only the incoming parameters so it will not have
    //change mode.
    BuildNewCapsFromNewTargetModeAndCaps(&pTmpRmtCaps);

    CapEnum h323CapCode = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo,cmCapReceive,kRolePeople);
    if (h323CapCode == eH263CapCode)
    {
      const capBuffer* pVideoCapBuffer = pTmpRmtCaps.GetFirstMediaCapBuffer(cmCapVideo, kRolePeople);
      if (pVideoCapBuffer)
      {
        CH263VideoCap* pVideoCap = (CH263VideoCap*)CBaseCap::AllocNewCap(eH263CapCode, (BYTE*)pVideoCapBuffer->dataCap);
        if (pVideoCap)
        {
          if (pVideoCap->IsAnnex(typeAnnexF))
            pVideoCap->RemoveTheSingleAnnexFromMask(typeAnnexF);
          POBJDELETE(pVideoCap);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
void CH323Cntl::UpdatePartyVideoRate()
{
  DWORD inIndex  = GetChannelIndexInList(true, cmCapVideo, FALSE);
  DWORD outIndex = GetChannelIndexInList(true, cmCapVideo, TRUE);

  if((inIndex < m_maxCallChannel) &&  (outIndex < m_maxCallChannel))
  {// if the video channels opened
    WORD incomingVideoRate = (m_pmcCall->GetChannelsArray())[inIndex]->GetRate();
    WORD outgoingVideoRate = (m_pmcCall->GetChannelsArray())[outIndex]->GetRate();

    if ( (m_pTargetModeH323->GetConfType() == kCp) && (incomingVideoRate > outgoingVideoRate) )
    {//only in cp, we allow this case!!
      DWORD videoRate;
        DWORD remoteMaxVideoPeopleRate;

      //set the incoming video rate in rh323 and the party rate according to incomingVideoRate
      remoteMaxVideoPeopleRate = incomingVideoRate;
        remoteMaxVideoPeopleRate = remoteMaxVideoPeopleRate % 8 ? remoteMaxVideoPeopleRate / 8 + 1 : remoteMaxVideoPeopleRate / 8;
      remoteMaxVideoPeopleRate = remoteMaxVideoPeopleRate  * 8;

      videoRate = (DWORD)remoteMaxVideoPeopleRate;
      m_pParty->UpdateVideoRate(videoRate);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::AreAllChannelsConnected(void)
{
  // This function checks if ALL channels (audio and video,incomming and outgoing) are connected
  BYTE result   = FALSE;
  WORD bIsVideo = FALSE;
  WORD bIsData  = FALSE;
  WORD bIsContent  = FALSE;
  CMedString str;
  // 1) check if all INCOMMING media  channels are opened (== m_pCurrentModeH323 is ready)
  // 2) check if all OUTGOING media  channels are opened ( m_isAudioOutgoingChannelConnected == 1 &&
  //                                                       m_isVideoOutgoingChannelConnected == 1 )
  // 3) In case mcms is the initiate of the opening channel and there is no video || data || content it means
  //    that the channel are not going to be opened, there was no matching.

  if(m_pLocalCapH323->OnType(cmCapVideo) && m_pRmtCapH323->OnType(cmCapVideo) && m_pLocalCapH323->IsVideoCapCodeMatch(m_pRmtCapH323))
    ON(bIsVideo);
  str << "bIsVideo=" << bIsVideo;
  //In case the content channel is delay and we should wait to him to be open - we initialize the flags via the caps.
  //In case the content channel (EPC or 239) will open we turn off the wrong flag.
  if (((m_pLocalCapH323->IsH239() && m_pRmtCapH323->IsH239()) || (m_pLocalCapH323->IsEPC() && m_pRmtCapH323->IsEPC()))
      && m_pTargetModeH323->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation) )
    ON(bIsContent);
  str << " bIsContent=" << bIsContent;
  //If both have data caps and both have T120 or both have FECC.
  //In case one has T120 and the other has FECC --> It is not good!!!!
  if( (m_pLocalCapH323->GetNumOfFeccCap() && m_pRmtCapH323->GetNumOfFeccCap()))
    ON(bIsData);
  str << " bIsData=" << bIsData;
  //In case incoming channel is in connecting state and the out reject (because mismatch protocol) - as in GW.
  //We want to wait the incoming channel connected.
  CChannel  *pInVidChane = FindChannelInList(cmCapVideo,FALSE,kRolePeople);
  ECsChannelState inVidChanState = kDisconnectedState;
  if(pInVidChane)
    inVidChanState = (ECsChannelState)pInVidChane->GetCsChannelState();

  result  = (m_pCurrentModeH323->IsMediaOn(cmCapAudio) && m_isAudioOutgoingChannelConnected);
  str << " AudioRes=" << result;
  result &= ((bIsVideo && m_pCurrentModeH323->IsMediaOn(cmCapVideo) && m_isVideoOutgoingChannelConnected)||
           (bIsVideo == FALSE) || (m_bVideoInRejected) ||
         ((m_bVideoOutRejected) && (!(inVidChanState > kFirstConnectingState && inVidChanState < kLastConnectingState))) ||
         (m_McmsOpenChannels && (m_pTargetModeH323->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople))) ||
         (m_isCodianVcr && (m_isVideoOutgoingChannelConnected))); // VNGFE-787 - Ack as if Vid in channel is open
  str << " VideoRes=" << result;

  // check internal channels state
  TRACEINTO << "mix_mode: Check internal channels state";
  if (GetTargetMode()->GetConfMediaType() == eMixAvcSvc)
  {
      result &= m_pmcCall->AreAllInternalChannelsConnected();
  }


  result &=  (m_pParty->IsPartyInChangeVideoMode() ||
         (bIsData && m_pCurrentModeH323->IsMediaOn(cmCapData) && m_isDataOutgoingChannelConnected)||
           (bIsData == FALSE) ||
         (m_bIsDataInRejected && m_isDataOutgoingChannelConnected) ||
         (m_bIsDataOutRejected && m_pCurrentModeH323->IsMediaOn(cmCapData)) ||
         (m_bIsDataInRejected && m_bIsDataOutRejected) ||
         (m_McmsOpenChannels && (m_pTargetModeH323->IsMediaOff(cmCapData,cmCapTransmit))));
  str << " DataRes=" << result;
/*
  result &= (m_pParty->IsPartyInChangeVideoMode() ||
         (m_pCurrentModeH323->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContent) && m_isVideoContentOutgoingChannelConnected)||
           (m_bIsContentRejected ||
         (m_McmsOpenChannels && (m_pTargetModeH323->IsMediaOff(cmCapVideo,cmCapTransmit,kRoleContent)))));
*/
  result &= ((bIsContent && m_isVideoContentOutgoingChannelConnected)||
           (bIsContent == FALSE) || m_bIsContentRejected ||
         (m_McmsOpenChannels && (m_pTargetModeH323->IsMediaOff(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation))));
  str << " ContentRes=" << result;

  PTRACE2(eLevelInfoNormal,"CH323Cntl::AreAllChannelsConnected :  ",str.GetString());

  return result;
}

//////////////////////////// VNGFE-910////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::AreAudioVideoChannelsConnected()
{
  // This function checks if ALL channels (audio and video,incomming and outgoing) are connected
  BYTE result   = FALSE;
  WORD bIsVideo = FALSE;


  // 1) check if all INCOMMING media  channels are opened (== m_pCurrentModeH323 is ready)
  // 2) check if all OUTGOING media  channels are opened ( m_isAudioOutgoingChannelConnected == 1 &&
  //                                                       m_isVideoOutgoingChannelConnected == 1 )
  // 3) In case mcms is the initiate of the opening channel and there is no video || data || content it means
  //    that the channel are not going to be opened, there was no matching.

  if(m_pLocalCapH323->OnType(cmCapVideo) && m_pRmtCapH323->OnType(cmCapVideo) && m_pLocalCapH323->IsVideoCapCodeMatch(m_pRmtCapH323))
    ON(bIsVideo);

  //In case incoming channel is in connecting state and the out reject (because mismatch protocol) - as in GW.
  //We want to wait the incoming channel connected.
  CChannel  *pInVidChane = FindChannelInList(cmCapVideo,FALSE,kRolePeople);
  ECsChannelState inVidChanState = kDisconnectedState;
  if(pInVidChane)
    inVidChanState = (ECsChannelState)pInVidChane->GetCsChannelState();

  result  = (m_pCurrentModeH323->IsMediaOn(cmCapAudio) && m_isAudioOutgoingChannelConnected);

  result &= ((bIsVideo && m_pCurrentModeH323->IsMediaOn(cmCapVideo) && m_isVideoOutgoingChannelConnected)||
           (bIsVideo == FALSE) || (m_bVideoInRejected) ||
         ((m_bVideoOutRejected) && (!(inVidChanState > kFirstConnectingState && inVidChanState < kLastConnectingState))) ||
         (m_McmsOpenChannels && (m_pTargetModeH323->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople))) ||
         (m_isCodianVcr && (m_isVideoOutgoingChannelConnected))); // VNGFE-787 - Ack as if Vid in channel is open


  TRACEINTO << "CH323Cntl::AreAudioVideoChannelsConnected - Result = " << (DWORD)result << "\n";

  return result;
}
////////////////////////////////////////////////////////////////////////////
void CH323Cntl::ConnectPartyToConf(void)
{
  if( m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf bIsClosing process - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }
  // VNGFE-787
  if( m_isRealIncVidChanSentFromCodianVcr)
  {
    // In this case we already simulated the incoming channel connected and therefor we will ignore the indication!!!
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf - Ignore codian incoming channel - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  }

//  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  BYTE bReleaseResourcesInConfCntl = ((m_remoteIdent != RvGWOrProxy) && (m_remoteIdent != VIU));
  if(m_isAlreadySentMultipointToMGC == FALSE && m_remoteIdent == PolycomMGC)
  {
    SendRemoteNumbering();
    m_isAlreadySentMultipointToMGC = TRUE;
    PTRACE2(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf: remote is MGC send multipoint command. Name - ",PARTYNAME);
  }
  // we are waiting for all channels (incoming & outgoing) to be opened
  // before we send "connect party" message to conference or
  // if AUDIO IN/OUT channels were connected and the remote has no video and data capabilities
    // we can stop the timer and connect the remote as secondary (condition1).

  BYTE bRmtSupportsVideo = m_pRmtCapH323->OnType(cmCapVideo);
  BYTE bRmtSupportsData  = m_pRmtCapH323->OnType(cmCapData);
  BYTE bRmtSupportsContent = m_pRmtCapH323->IsSupportPeopleAndContent() || m_pRmtCapH323->IsH239();

  BYTE bPartySupportsOnlyAudio = IsCallConnectedAudioOnly();

  BYTE bAllChannelsConnected    = AreAllChannelsConnected();
  BYTE bPartyConnectedOnlyAudioMedia = IsPartyConnectedOnlyAudioMedia();

  char buf[1000];
  sprintf(buf, "CH323Cntl::ConnectPartyToConf - Conn Id: %d bAllChannelsConnected: %d, bPartySupportsOnlyAudio %d, bPartyConnectedOnlyAudioMedia %d, isAudioMediaOn %d, isOutgoingAudioChannelConnected %d",
          m_pCsRsrcDesc->GetConnectionId(), bAllChannelsConnected, bPartySupportsOnlyAudio, bPartyConnectedOnlyAudioMedia,
          m_pCurrentModeH323->IsMediaOn(cmCapAudio), m_isAudioOutgoingChannelConnected);
  PTRACE2(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf - ", buf);
    WORD statusToConnectWith = statOK;
    // VNGFE-910
    // In case we find that the video (one side channel) is connected and we ignore the incoming video channel
    // we will open our outgoing H239 channel towards the Codian IpVCR
    // since its Codian VCR only we dont need to change the code here and to add EPC
    if (m_isCodianVcr && AreAudioVideoChannelsConnected())
    {
      WORD bIsH239  = FALSE;
      if (m_pLocalCapH323->IsH239() && m_pRmtCapH323->IsH239() &&  m_pTargetModeH323->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation))
        ON(bIsH239);
      if (!m_isVideoContentOutgoingChannelConnected && bIsH239)
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf - Conn Id = Opening Codian H239 OutGoing Channel");
      //check if the presentation out is opened
      BYTE bOutChannelAlreadyOpen = FALSE;
      if (FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation))
        bOutChannelAlreadyOpen = TRUE;
      if (bOutChannelAlreadyOpen == FALSE )
      {
        ERoleLabel eRole;
        CCapSetInfo capInfo   = GetCommonContentMode(eRole);
        eRole = kRolePresentation;
        if(capInfo.GetIpCapCode() == eUnknownAlgorithemCapCode)
        {
          PTRACE(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf - Remote and local caps mismatch on Content protocl - Do not open outgoing H239 channel.");
        }
        else
        {
          PTRACE(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf - Open content out.");
          SetContentTargetTransmitMode(capInfo, eRole);
          BYTE bIsMcmsOrigin    = TRUE;
          OnPartyOutgoingChannelReq(capInfo, eRole, bIsMcmsOrigin);
      }
      }
      }
    }
  if(bAllChannelsConnected || bPartySupportsOnlyAudio || bPartyConnectedOnlyAudioMedia)
  {
    BYTE bUpdateVidParams = bAllChannelsConnected && bRmtSupportsVideo;

    UpdateParamsBeforeConnectingToConf(bUpdateVidParams);

    m_state = CONNECT;

    if ((m_isIncomingAudioHasDisconnectedOnce || m_isOutgoingAudioHasDisconnectedOnce) && !m_pRmtCapH323->IsECS())
    {//reopen audio channels
      OFF(m_isIncomingAudioHasDisconnectedOnce);
      OFF(m_isOutgoingAudioHasDisconnectedOnce);
        }
        StartCsPartyErrHandlingLoop();

	//Removed fix for VNGR-24227, since it caused VNGR-26509
        m_pTaskApi->H323EndMediaChannelsConnect(*m_pRmtCapH323, *m_pCurrentModeH323, statusToConnectWith, IsLateReleaseOfVideoResources(), FALSE, m_isCodianVcr);
    }


//if audio channels were opened, we don't wait to video or data channels, and start the audio
  else if (m_pCurrentModeH323->IsMediaOn(cmCapAudio) && m_isAudioOutgoingChannelConnected)
  {
    //timers:
    if(IsValidTimer(AUDCONNECTTOUT))
    {
      PTRACE(eLevelInfoNormal,"DeleteTimer(AUDCONNECTTOUT);");
      DeleteTimer(AUDCONNECTTOUT);
    }

    StartTimer(OTHERMEDIACONNECTED, 20*SECOND);

    //if remote as data cap + conf is T.120 we open a timer
    BYTE bIsTargetHasFecc = FALSE;
    if (m_pTargetModeH323->IsMediaOn(cmCapData,cmCapTransmit))
    {//need to check if it is on because of the FECC and not a regular data channel.
      CapEnum dataType = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapData,cmCapTransmit);
      if ((dataType == eAnnexQCapCode) || (dataType == eRvFeccCapCode) )
        bIsTargetHasFecc = TRUE;
    }

    BYTE bOpenFeccOut = FALSE;
    if (bIsTargetHasFecc) //check that the out channel isn't opened yet
      bOpenFeccOut = m_pCurrentModeH323->IsMediaOff(cmCapData, cmCapTransmit);
    // VNGR-4217
    if ( m_bIsDataOutRejected == TRUE)
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf - Outgoing data channel rejected once - No need to start data timer");
    }
    else
    {
    if( ((m_pRmtCapH323->OnCap(eAnnexQCapCode) || m_pRmtCapH323->OnCap(eRvFeccCapCode)) && bOpenFeccOut))
    {
      if (IsValidTimer(MCMSOPENDATACHANNELS) == FALSE) //if the timer isn't initialized yet
      {
        // VNGFE-910
        // In case of Codian - No need to wait for FECC timer - Open immediatly
        if (m_isCodianVcr)
        {
          PTRACE(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf - Codian case - Connect the data immediatly");
          //check if the presentation out is opened
          BYTE bOutChannelAlreadyOpen = FALSE;
          if (FindChannelInList(cmCapData, TRUE, kRolePeople))
            bOutChannelAlreadyOpen = TRUE;
          if (bOutChannelAlreadyOpen == FALSE)
          {
            CSegment seg;
            OnTimerOpenDataChannelFromMcms(&seg);
          }
        }
        else
        {
          PTRACE(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf - Start data timer");
          StartTimer(MCMSOPENDATACHANNELS, 10*SECOND);
        }
      }
    }
    else if (IsValidTimer(MCMSOPENDATACHANNELS))
      DeleteTimer(MCMSOPENDATACHANNELS);
    }

    //inform conf3:
    // first audio channel in party and the party not in change mode
    //change mode is for video and we should wait till the video finish to connect
    if (!m_isAudioConnected && !m_pParty->IsPartyInChangeVideoMode())
    {
      ON(m_isAudioConnected);
            statusToConnectWith = statSecondary;
      PTRACE(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf, H323EndMediaChannelsConnect - statSecondary");
            m_pTaskApi->H323EndMediaChannelsConnect(*m_pRmtCapH323, *m_pCurrentModeH323, statusToConnectWith, IsLateReleaseOfVideoResources());
    }
    else if ((m_isIncomingAudioHasDisconnectedOnce || m_isOutgoingAudioHasDisconnectedOnce) && !m_pRmtCapH323->IsECS())
    {//reopen audio channel
      OFF(m_isIncomingAudioHasDisconnectedOnce);
      OFF(m_isOutgoingAudioHasDisconnectedOnce);
            PTRACE(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf, H323EndMediaChannelsConnect - Reconnect the Audio");
            m_pTaskApi->H323EndMediaChannelsConnect(*m_pRmtCapH323, *m_pCurrentModeH323, statusToConnectWith, IsLateReleaseOfVideoResources() , TRUE);
    }
        StartCsPartyErrHandlingLoop();
    DWORD partyKeepAliveFirstTimerVal = GetSystemCfgFlagInt<DWORD>(CFG_KEY_H323_CS_ERROR_HANDLE_FIRST_TIMER_VAL);
  }
  else
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::ConnectPartyToConf - Wait to all channels connected - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());
  bAllChannelsConnected |= m_pParty->IsPartyInChangeVideoMode();
  if((bAllChannelsConnected) && m_remoteCapIndNotHandle)
  {
    HandleCapIndication(m_bPrevCapsAreFull,m_bPrevCapsHaveAudio, bAllChannelsConnected);
    m_remoteCapIndNotHandle = FALSE;
  }
}


////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsCallConnectedAudioOnly()
{
  BYTE bRmtSupportsVideo  = m_pRmtCapH323->OnType(cmCapVideo);
  BYTE bRmtSupportsData   = m_pRmtCapH323->OnType(cmCapData);
  BYTE bRmtSupportsContent = m_pRmtCapH323->IsSupportPeopleAndContent() || m_pRmtCapH323->IsH239();

  BYTE bPartySupportsOnlyAudio = (m_pCurrentModeH323->IsMediaOn(cmCapAudio) && m_isAudioOutgoingChannelConnected &&
                (bRmtSupportsVideo == FALSE) && (bRmtSupportsData == FALSE) && (bRmtSupportsContent == FALSE));

  return bPartySupportsOnlyAudio;
}

////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsPartyConnectedOnlyAudioMedia()
{
  //Audio only could be with T120 - so I must first check if there is T120 or not
  BYTE  audioOnlyConnect = 0;
  DWORD temp = 1;

  ///////////////////////////////////////////////////////////////////////////////
  // (1). Party video channel was rejected or there is no video at all. Audio and T120 (if reserved) channels
  //    are already connected.

  BYTE bAudioChannelsConnected = (m_pCurrentModeH323->IsMediaOn(cmCapAudio) && m_isAudioOutgoingChannelConnected);

  // Video call
  //In case incoming channel is in connecting state and the out reject (because mismatch protocol) - as in GW.
  //We want to wait the incoming channel connected.
  CChannel  *pInVidChane = FindChannelInList(cmCapVideo,FALSE,kRolePeople);
  ECsChannelState inVidChanState = kDisconnectedState;
  if(pInVidChane)
    inVidChanState = (ECsChannelState)pInVidChane->GetCsChannelState();

  BYTE bLocalSupportsVideo  = m_pLocalCapH323->OnType(cmCapVideo);
  BYTE bRmtSupportsVideo    = m_pRmtCapH323->OnType(cmCapVideo);
  BYTE bVideoChannelRejected  = ( (m_bVideoInRejected ||
                  (m_bVideoOutRejected && (!(inVidChanState > kFirstConnectingState && inVidChanState < kLastConnectingState)))) &&
                  bLocalSupportsVideo && bRmtSupportsVideo);

  // Video Content call
  BYTE bLocalSupportsContent   = m_pLocalCapH323->IsH239();
  BYTE bRmtSupportsContent   = m_pRmtCapH323->IsH239();
  BYTE bIsContentCall      = bLocalSupportsContent & bRmtSupportsContent;
  BYTE bContentChannelRejected = bIsContentCall && m_bIsContentRejected;

  BYTE bLocalSupportsFecc = m_pLocalCapH323->OnType(cmCapData) && m_pLocalCapH323->GetNumOfFeccCap();
  BYTE bRmtSupportsFecc   = m_pRmtCapH323->OnType(cmCapData) && m_pRmtCapH323->GetNumOfFeccCap();

  if(bLocalSupportsFecc && bRmtSupportsFecc)
  {
    //In case both are supprted data FECC but the current data mode is OFF!!!
    audioOnlyConnect = (bVideoChannelRejected && (bContentChannelRejected || !bIsContentCall) && bAudioChannelsConnected && m_pCurrentModeH323->IsMediaOff(cmCapData));
  }
  else  // not a data call
  {
    // (2). Party was reserved as Audio-Only (with out data). Audio channels are already connected
    audioOnlyConnect = (bAudioChannelsConnected && m_pParty->GetIsVoice());
    if (audioOnlyConnect)
      return 1;

    audioOnlyConnect = (bVideoChannelRejected && (bContentChannelRejected || !bIsContentCall) && bAudioChannelsConnected);
  }

  return audioOnlyConnect;
}


////////////////////////////////////////////////////////////////////////////
//Added for Highest Common:
//This function is been called after all the channels has been connected or after the party
//has changed the changed mode process
void CH323Cntl::UpdateParamsBeforeConnectingToConf(BYTE bUpdateVidParams)
 {
  //if (bUpdateVidParams)
  //  UpdatePartyVideoRate();

  if (IsValidTimer(AUDCONNECTTOUT))
  {
    PTRACE(eLevelInfoNormal,"DeleteTimer(AUDCONNECTTOUT);");
    DeleteTimer(AUDCONNECTTOUT);
  }
  if (IsValidTimer(OTHERMEDIACONNECTED))
    DeleteTimer(OTHERMEDIACONNECTED);

  if (IsValidTimer(MCMSOPENDATACHANNELS))
    DeleteTimer(MCMSOPENDATACHANNELS);

  OFF(m_McmsOpenChannels);

  SendBrqIfNeeded();
}

/////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendBrqIfNeeded()
{
  if (m_isCallingThroughGk && !m_bIsAvaya)
  {
    DWORD confRate = 0;
    if (CalculateNewBandwidth(confRate))
      CreateAndSendBRQReq(confRate);
  }
}

/////////////////////////////////////////////////////////////////////////////
/*  insert the alias of the card which used in that call from the gui to the TCall member parameter
  keeping the following way "aaa,bbb,ccc;" for the aliases*/
void CH323Cntl::InsertLocalAlias()
{
  CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
  CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
  if (pServiceParams == NULL)
  {
    PASSERTMSG(m_pmcCall->GetConnectionId(), "CH323Cntl::InsertLocalAlias - IP Service does not exist!!!");
    return;
  }

  ALIAS_S* pAliasesStruct = pServiceParams->GetAliasArray();
  int num = 0;

  unsigned char* aliasContent;
  for (int i = 0; i < MAX_ALIAS_NAMES_NUM; i++)
  {
    aliasContent = pAliasesStruct[i].aliasContent;
    if (aliasContent[0] != '\0')
      num++;
    else
      break;
  }

  CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
  BOOL bIsDialWithConfName = 0;
  std::string key = "H323_ENABLE_CONFERENCE_DIALIN_IDENTIFY";
  pSysConfig->GetBOOLDataByKey(key, bIsDialWithConfName);
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  BYTE ITPLayoutMode;
  if(pCommConf)
  	ITPLayoutMode = pCommConf->GetTelePresenceLayoutMode();
  else
  	PASSERT_AND_RETURN(NULL == pCommConf);

  if ( (num == 0) && bIsDialWithConfName == NO )
    return;

  ALLOCBUFFER(info, MaxAddressListSize);
  memset(info, '\0', MaxAddressListSize);

  DWORD type[MaxNumberOfAliases];
  int sizeToMemset = sizeof(type);
  memset(type, 0, sizeToMemset);

  BOOL  flag = FALSE;
  int i  = 0;
  BYTE bAddedAnotherAlias = TRUE;

  if (m_pmcCall->GetIsOrigin())  //dial out
  {
  /*  // copy the dial in alias for GW calls
    if(m_onGatewayCall)
    {
      strcat(info, m_pH323NetSetup->GetLocalDisplayName());
      strcat(info,",");
      *type = cmAliasTypeH323ID;
      i++;
      if(n < MaxNumberOfAliases)
        n++;
      bAddedAnotherAlias = TRUE;
    }
    else
    {*/

      if(bIsDialWithConfName)
      {// enable cascade with conference name (Unicom project).
        PTRACE(eLevelInfoNormal,"CH323Cntl::InsertLocalAlias -add conf name");
        ALLOCBUFFER(pPartyName,2*H243_NAME_LEN+50);
        char *pPointer2Name = NULL;

	 //Bridge-8006
	 if (IsCallGeneratorConf())
        	snprintf(pPartyName, 2*H243_NAME_LEN+49, "%s_(%03d)", pCommConf->GetDisplayName(), pCommConf->NextCGPartiesCounter());
	 else
	 {
	 	strncpy(pPartyName, pCommConf->GetDisplayName(), 2*H243_NAME_LEN+50);
	 	pPartyName[2 * H243_NAME_LEN + 50 - 1] = '\0';
	 }

        PTRACE2(eLevelInfoNormal,"CH323Cntl::InsertLocalAlias -add conf name", pPartyName);
        //pPointer2Name = strchr(pPartyName, ',');
        //if(pPointer2Name)
        //{
          //*pPointer2Name = '\0';
          if( ITPLayoutMode == eTelePresenceLayoutRoomSwitch && !strstr(pCommConf->GetDisplayName(),"ROOMSWITCH") )
            strcat(info, "ROOMSWITCH");
          strcat(info, pPartyName);
          strcat(info,",");
          type[0] = cmAliasTypeH323ID;
          i++;
          if (num < MaxNumberOfAliases)
            num++;
          bAddedAnotherAlias = TRUE;
        //}
        DEALLOCBUFFER(pPartyName);
      }
      else if(ITPLayoutMode == eTelePresenceLayoutRoomSwitch)
      {
        strcat(info, "ROOMSWITCH");
        strcat(info,",");
        type[0] = cmAliasTypeH323ID;
        i++;
        if (num < MaxNumberOfAliases)
        num++;
        bAddedAnotherAlias = TRUE;


      }
    //}
  }
  else if(ITPLayoutMode == eTelePresenceLayoutRoomSwitch || strstr(pCommConf->GetDisplayName(),"ROOMSWITCH") )
  {
    ALLOCBUFFER(pPartyName,2*H243_NAME_LEN+50);
    //char *pPointer2Name = NULL;
    strcat(info, "ROOMSWITCH");
    strncpy(pPartyName, pCommConf->GetDisplayName(), 2*H243_NAME_LEN+50);
	pPartyName[sizeof(pPartyName) - 1]='\0';
    strcat(info, pPartyName);
    type[0] = cmAliasTypeH323ID;
    i++;
    if (num < MaxNumberOfAliases)
    num++;
     // bAddedAnotherAlias = TRUE;
    DEALLOCBUFFER(pPartyName);

  }

  ALIAS_S alias;
  int j=0;

  for( ; i< num && j< MAX_ALIAS_NAMES_NUM; i++, j++ )
  {

    memcpy(&alias, &(pAliasesStruct[j]), sizeof(ALIAS_S));
    if (alias.aliasContent[0] != '\0')
    {
      if( (num == 1) && (alias.aliasType == PARTY_H323_ALIAS_PARTY_NUMBER_TYPE) )
      {  //party number should be written twice: first as E164 type and second as party number type
        const char* aliasContent = (char*)alias.aliasContent;

        // overflow check
        if(  (strlen(info) + strlen(aliasContent) + 1) < MaxAddressListSize)
        {
          strcat(info, aliasContent);
          strcat(info, "," );
          type[i] = (DWORD)cmAliasTypeE164;
          flag = TRUE;
        }
        else
        {
          DBGPASSERT( strlen(info) + strlen(aliasContent) + 1 );
          PTRACE2INT (eLevelError, "CH323Cntl::InsertLocalAlias - aliases length overflow", strlen(info) + strlen(aliasContent) + 1 );
        }
      }

      const char* aliasContent = (char*)alias.aliasContent;

      // overflow check
      if(  (strlen(info) + strlen(aliasContent) + 1) < MaxAddressListSize)
      {
        strcat(info, aliasContent);

        if (!flag) //the usual case
          type[i] = alias.aliasType;
        else
          type[i+1] = alias.aliasType;
      }
      else
      {
        DBGPASSERT( strlen(info) + strlen(aliasContent) + 1 );
        PTRACE2INT (eLevelError, "CH323Cntl::InsertLocalAlias - aliases length overflow", strlen(info) + strlen(aliasContent) + 1 );
      }

      }

      if (i == 0)
      {
        if (strlen(info) == 0) //for undefined dial in
        {
          info[0] = '\0';
          type[0]  = (DWORD)cmAliasTypeE164;
        }
      }

      if (i != num-1)
        strcat (info, "," );
  }

  if (m_pmcCall->GetIsOrigin())  //dial out
  {
    m_pmcCall->SetSourceInfoAlias(info);
    m_pmcCall->SetSrcInfoAliasType(type);
  }
  else
  {
    m_pmcCall->SetDestinationInfoAlias(info);
    m_pmcCall->SetDestInfoAliasType(type);
  }

  DEALLOCBUFFER(info);
}

/////////////////////////////////////////////////////////////////////////////
/* insert the destination alias to the TCall from the h323netsetup
  get the endpoint alias  */
void CH323Cntl::InsertRemoteAlias()
{
  ALLOCBUFFER(info, MaxAddressListSize);
  memset(info, '\0', MaxAddressListSize);

  strncpy(info, m_pH323NetSetup->GetH323PartyAlias(), MaxAddressListSize-1);

  if (strlen(info) == 0)  //for undefined dial in
  {
    DEALLOCBUFFER(info);
    return;
  }

  DWORD remoteTypes[MaxNumberOfAliases];
  memset(remoteTypes, 0, sizeof(remoteTypes));

  // init the type of the endpoint alias the type in the GUI is not according to the type in the alias
  switch(m_pH323NetSetup->GetH323PartyAliasType())
  {
    case PARTY_H323_ALIAS_H323_ID_TYPE:
      remoteTypes[0] = (DWORD)cmAliasTypeH323ID;
      break;
    case PARTY_H323_ALIAS_E164_TYPE:
      remoteTypes[0]  = (DWORD)cmAliasTypeE164;
      break;
    case PARTY_H323_ALIAS_URL_ID_TYPE:
      remoteTypes[0]  = (DWORD)cmAliasTypeURLID;
      break;
    case PARTY_H323_ALIAS_TRANSPORT_ID_TYPE:
      remoteTypes[0]  = (DWORD)cmAliasTypeTransportAddress;
      break;
    case PARTY_H323_ALIAS_EMAIL_ID_TYPE:
      remoteTypes[0]  = (DWORD)cmAliasTypeEMailID;
      break;
    case PARTY_H323_ALIAS_PARTY_NUMBER_TYPE:
    //party number should be written twice: first as E164 type and second as party number type
      remoteTypes[0]  = (DWORD)cmAliasTypeE164;
      strcat(info, ",");
      strncat(info, m_pH323NetSetup->GetH323PartyAlias(), MaxAddressListSize-1);
      remoteTypes[1] = (DWORD)cmAliasTypePartyNumber;
      break;
    default:
      remoteTypes[0]  = (DWORD)cmAliasTypeH323ID;
      break;
  }

  if (!m_pmcCall->GetIsOrigin())  //dial in
  {
    m_pmcCall->SetSourceInfoAlias(info);
    m_pmcCall->SetSrcInfoAliasType(remoteTypes);
  }
  else
  {
    m_pmcCall->SetDestinationInfoAlias(info);
    m_pmcCall->SetDestInfoAliasType(remoteTypes);
  }
  DEALLOCBUFFER(info);
}

/////////////////////////////////////////////////////////////////////////////
//Add For GK request from the card that can be answered although the call idle didn't arrive.
void  CH323Cntl::OnDrqTimer(CSegment* pParam)
{
  PTRACE2INT(eLevelError,"CH323Cntl::OnDrqTimer : TIME OUT - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  DBGPASSERT(m_pmcCall->GetConnectionId());
    //Michael - HOMOLOGATION-------------------------------------------------------------//
    if(TRUE == m_isCloseConfirm)
        m_pCsInterface->SendMsgToCS(H323_CS_SIG_CALL_CLOSE_CONFIRM_REQ,NULL,m_serviceId,
                                m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
	//-----------------------------------------------------------------------------------//
  if ( (m_CallConnectionState == McmsNotAcceptAcfParams) ||
     (m_CallConnectionState == GkDRQafterARQ )) //In case that in state GkARQ there is call drop
    DisconnectForCallWithoutSetup();
  else
  {
    CSegment* pSeg = new CSegment;
    HeaderToGkManagerStruct headerToGkManager = SetHeaderToGkManagerStruct();
    DWORD headerLen = sizeof(HeaderToGkManagerStruct);
    pSeg->Put((BYTE*)&headerToGkManager, headerLen);
    SendReqToGkManager(pSeg, H323_CS_RAS_REMOVE_GK_CALL_REQ);

    RemoveAndDisconnectCall(statTout);
  }
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnTimerClear(CSegment* pParam)
{
  PTRACE2INT(eLevelError,"CH323Cntl::OnTimerClear : TIME OUT - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  PASSERT_AND_RETURN(NULL == m_pmcCall);
  int status = ctCallDropNoAnswer;
  DWORD faultyFlag = 0;
  DBGPASSERT(m_pmcCall->GetConnectionId());
  // Setting the digits for the MCU_INTERNAL_PROBLEM
  MipHardWareConn mipHwConn;
  MipMedia    mipMedia;
  MipDirection  mipDirect;
  MipTimerStatus  mipTimerStat;
  MipAction   mipAction;
  BYTE isClearTimer = 0;
  if(m_pmcCall->GetChannelsCounter())
  {
    PTRACE(eLevelError,"CH323Cntl::OnTimerClear - channelsCounter isn't zero");
    for (int i=0; i<m_maxCallChannel; i++)
    {
	  CChannel* pChannel = m_pmcCall->GetSpecificChannel(i);
	  if (pChannel)
      {
        DBGPASSERT(pChannel->GetCsIndex());
        status = ctCallDropWithOpenChannel;
        if (pChannel->GetCmUdpChannelState() != kRecieveCloseAck)
        {
          isClearTimer = 1;
          TranslateAckToMipErrorNumber(mipHwConn, mipMedia, mipDirect, mipTimerStat, mipAction, NULL, pChannel,isClearTimer);
          char mipNum[200];
          memset(mipNum, '\0', 200);
          sprintf(mipNum," mipHwConn = %d mipMedia = %d mipDirect = %d  mipTimerStat = %d mipAction = %d "
          ,(BYTE)mipHwConn,(BYTE)mipMedia,(BYTE)mipDirect,(BYTE)mipTimerStat, (BYTE)mipAction);
          PTRACE2(eLevelInfoNormal,"CH323Cntl::OnTimerClear : ", mipNum);

          faultyFlag = STATUS_FAIL;
          break;
        }
      }


    }
  }

  if (faultyFlag == STATUS_FAIL)
  {
    m_pTaskApi->SetFaultyResourcesToPartyControlLevel(faultyFlag, mipHwConn, mipMedia, mipDirect, mipTimerStat, mipAction);
  }

  if(m_isCallDropRequestWaiting) // mcms hasn't send call_drop_req
    PASSERTMSG(GetConnectionId(),"CH323Cntl::OnTimerClear - mcms hasn't send call_drop_req");

  if (m_isCallingThroughGk)
    CreateAndSendDRQReq(cmRASDisengageReasonUndefinedReason);
  else
    RemoveAndDisconnectCall(status);
}
////////////////////////////////////////////////////////////////////////
void CH323Cntl::RemoveAndDisconnectCall(int status)
{
  m_state = IDLE;
  int disconnectedStatus = statOK;
  if(status)
    disconnectedStatus = statTout;
    //---HOMOLOGATION 2 ----------//
    if (IsValidTimer(IRR_TIMER))
        DeleteTimer(IRR_TIMER);
    //----------------------------//
  if (IsValidTimer(PARTYDISCONNECTTOUT))
    DeleteTimer(PARTYDISCONNECTTOUT);
  if (IsValidTimer(DRQ_TIMER))
    DeleteTimer(DRQ_TIMER);
  if (IsValidTimer(PARTYCSKEEPALIVEFIRSTTOUT))
    DeleteTimer(PARTYCSKEEPALIVEFIRSTTOUT);
  if (IsValidTimer(PARTYCSKEEPALIVESECONDTOUT))
    DeleteTimer(PARTYCSKEEPALIVESECONDTOUT);
  if (IsValidTimer(LPRTOUT))
    DeleteTimer(LPRTOUT);
  if (IsValidTimer(LOCALLPRTOUT))
    DeleteTimer(LOCALLPRTOUT);
  if (IsValidTimer(CAPABILITIESTOUT))
    DeleteTimer(CAPABILITIESTOUT);


  if(status)
  { //notify the card about disconnect timer expired.
    mcReqCallDropTimerExpired *pCallDropTimerExpired = new mcReqCallDropTimerExpired;
    memset(pCallDropTimerExpired,0,sizeof(mcReqCallDropTimerExpired));
    switch(status)
    {
      case statTout:
        pCallDropTimerExpired->reason = ct_DRQ_NoAnswer;
        break;
      case ctCallDropNoAnswer:
        pCallDropTimerExpired->reason = ctCallDropNoAnswer;
        break;
      case ctCallDropWithOpenChannel:
        pCallDropTimerExpired->reason = ctCallDropWithOpenChannel;
        break;
    }
    CSegment* pMsg = new CSegment;
    pMsg->Put((BYTE*)(pCallDropTimerExpired),sizeof(mcReqCallDropTimerExpired));
    m_pCsInterface->SendMsgToCS(H323_CS_SIG_CALL_DROP_TIMER_EXPIRED_REQ,pMsg,m_serviceId,
                  m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
    POBJDELETE(pMsg);
    PDELETE(pCallDropTimerExpired);
  }
  TRACEINTO << "mix_mode: send m_pTaskApi->EndH323DisConnect";
  m_pTaskApi->EndH323DisConnect(disconnectedStatus);
}

////////////////////////////////////////////////////////////////////////
void CH323Cntl::CloseIncomingChannel(cmCapDataType channelTypeH323, ERoleLabel eRole)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CloseIncomingChannel - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  // if this the case of a closing process we are not sending rtp_stream_off_req
  // that are the result of change mode that turn the party to secondery
  if((m_pmcCall->GetIsClosingProcess() == TRUE) || m_isReceiveCallDropMessage)
    return;

  DWORD index = GetChannelIndexInList(true, channelTypeH323, FALSE, eRole);
  if(index >= m_maxCallChannel)
    PTRACE(eLevelInfoNormal,"CH323Cntl::CloseIncomingChannel : channel not found !!!");
  else
  {
    m_channelTblIndex = index;
    OnPartyStreamOffReq();
  }
}

////////////////////////////////////////////////////////////////////////
void CH323Cntl::CloseOutgoingChannel(cmCapDataType channelTypeH323, ERoleLabel eRole)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CloseOutgoingChannel - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  // if this the case of a closing process we are not sending rtp_stream_off_req
  // that are the result of change mode that turn the party to secondery
  if((m_pmcCall->GetIsClosingProcess() == TRUE) || m_isReceiveCallDropMessage)
    return;

  DWORD index = GetChannelIndexInList(true, channelTypeH323, TRUE, eRole);
  if(index >= m_maxCallChannel)
  {
    if(eRole == kRolePeople)
      PTRACE(eLevelError,"CH323Cntl::CloseOutgoingChannel : people channel not found !!!");
    else
      PTRACE(eLevelError,"CH323Cntl::CloseOutgoingChannel : content channel not found !!!");
  }
  else
  {
    CChannel* pChannel = m_pmcCall->GetSpecificChannel(index);
    PASSERT_AND_RETURN(NULL == pChannel);

    if(pChannel->GetCsIndex())
    {
      m_channelTblIndex = index;
      OnPartyStreamOffReq();
    }
    else if(pChannel->IsCsChannelStateConnecting()) //sending outgoingChannelReq and we wait the answer.
    {
      pChannel->SetCsChannelState(kDisconnectingState);
      PTRACE(eLevelError,"CH323Cntl::CloseOutgoingChannel : there is no pmIndex - we wait the response");
    }
    else if(pChannel->IsCsChannelStateDisconnecting()) //Channel is already disconnecting
    {
      PTRACE(eLevelError,"CH323Cntl::CloseOutgoingChannel : Channel is already disconnecting");
    }
    else
    {
      PTRACE2INT(eLevelError,"CH323Cntl::CloseOutgoingChannel : there is no pmIndex - channelState = %d",pChannel->GetCsChannelState());
      DBGPASSERT(index);
    }
  }
}

////////////////////////////////////////////////////////////////////////
/* The prepares of the function is to translate enum from the 323card
  to opcode that used as the agreeable language between the mcms tasks.
  */
WORD CH323Cntl::disconnectCauseEnum2Opcode(long param)
{
  switch(param)
  {
    case callCloseReasonRemoteBusy:
      return H323_CALL_CLOSED_REMOTE_BUSY;
    case callCloseReasonNormal:
      return H323_CALL_CLOSED_NORMAL;
    case callCloseReasonRemoteReject:
      return H323_CALL_CLOSED_REMOTE_REJECT;
    case callCloseReasonRemoteUnreachable:
      return H323_CALL_CLOSED_REMOTE_UNREACHABLE;
    case callCloseReasonUnknownReason:
      return H323_CALL_CLOSED_UNKNOWN_REASON;
    case callCloseReasonClosedByMcms://menas closed by cs
      return H323_CALL_CLOSED_BY_MCU;
    case callStateModesNeedMaintenance: //means incomplete address
      return H323_CALL_CLOSED_FAULTY_DESTINATION_ADDRESS;
    case  callCloseReasonNoAnswer:
      return H323_CALL_CLOSED_REMOTE_NO_ANSWER;
  }
  return H323_CALL_CLOSED_UNKNOWN_REASON;
}


////////////////////////////////////////////////////////////////////////
/* The function initiate the localCapH323 to finish the process
  of DIALIN curulell to the 320 DIALIN and that can plug-in to H323
  DIALIN process.
*/
void CH323Cntl::UpdateLocalCapH323(CCapH323& capH323)
{
  *m_pLocalCapH323 = capH323;
  m_maxCallChannel = g_kCallWithoutTwoMedia;
  if(m_pLocalCapH323->OnType(cmCapData))
    m_maxCallChannel += 2;
  if(m_pLocalCapH323->IsH239() )
    m_maxCallChannel += 2;
}

//////////////////////////////////////////////////////////////////////
void CH323Cntl::OnCallReleasePortReq()
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnCallReleasePortReq - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());
  mcReqReleasePort* ptr = new mcReqReleasePort;
  ptr->port = m_getPortInd.srcCallSignalAddress.transAddr.port;

  void* ip;
  if (m_pmcCall->GetIsOrigin())  //dial out
    ip = (void*)(m_pmcCall->GetSrcTerminalParams().callSignalAddress.addr.v4.ip);

  else  //dial in
    ip = (void*)(m_pmcCall->GetDestTerminalParams().callSignalAddress.addr.v4.ip);

  ptr->haCall = ip;
  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)(ptr),sizeof(mcReqReleasePort));
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_RELEASE_PORT_REQ,pMsg,m_serviceId,
            m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
  POBJDELETE(pMsg);
  PDELETE (ptr);
}

//////////////////////////////////////////////////////////////////////
void CH323Cntl::SetDialIn_mcCall()
{
  m_pmcCall->SetConferenceId(m_pH323NetSetup->GetH323ConfIdAsGUID());

  m_pmcCall->SetCallId(m_pH323NetSetup->GetCallId());

  m_pmcCall->SetConnectionId(m_pCsRsrcDesc->GetConnectionId());

  m_pmcCall->SetCallIndex(m_pH323NetSetup->GetCallIndex());
  m_pmcCall->SetRate(m_pH323NetSetup->GetMaxRate());
  m_pmcCall->SetMaxRate(m_pH323NetSetup->GetMaxRate());

  m_pmcCall->SetSourceInfoAlias(m_pH323NetSetup->GetH323SrcPartyAliases());

  // IpV6

  m_pmcCall->SetCallTransientDisplay(m_pH323NetSetup->GetLocalDisplayName());
  m_pmcCall->SetCallTransientUserUser(m_pH323NetSetup->GetH323userUser());

//  m_pmcCall->conferenceGoal = (cmConferenceGoalType)m_pH323NetSetup->GetConferenceGoal();
  m_pmcCall->SetCallType((cmCallType)m_pH323NetSetup->GetConnectionType());
  m_pmcCall->SetIsActiveMc(m_pH323NetSetup->GetbIsActiveMc());
  m_pmcCall->SetIsOrigin(m_pH323NetSetup->GetbIsOrigin());

  m_pmcCall->SetSetupH245Address(*(m_pH323NetSetup->GetTaSrcPartyAddr()));
                                      //also the multiple response
  m_pmcCall->SetH245Establish(m_pH323NetSetup->GetbH245Establish());
}

//////////////////////////////////////////////////////////////////////
void CH323Cntl::RecieveInfoFor_mcCall()
{
  m_pmcCall->SetConnectionId(m_pCsRsrcDesc->GetConnectionId());
  m_pmcCall->SetCanMapAlias(TRUE);
  InsertLocalAlias();
  InsertRemoteAlias();

  //set local address
  CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
  CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
  if (pServiceParams == NULL)
  {
    PASSERTMSG(m_pmcCall->GetConnectionId(), "CH323Cntl::OnPartyGetPortReq - IP Service does not exist!!!");
    return;
  }

  /* V4.1c <--> V6 merge
  BOOL bIsPrimNetwork = m_pH323NetSetup->IsItPrimaryNetwork();

  ipAddress localAddress = pServiceParams->GetIPAddress(bIsPrimNetwork);
  */
  mcTransportAddress localAddress;
  memset(&localAddress,0,sizeof(mcTransportAddress));

  WORD      localPort    = m_pH323NetSetup->GetLocalH225Port();

  if( !::isApiTaNull(m_pH323NetSetup->GetTaDestPartyAddr()) )
  {
    // if we have dest address we must have source address

    if (m_pmcCall->GetIsOrigin()) //dial out
    {
      // dial out - source is local address
      localAddress.port = localPort;
      memcpy(&localAddress,m_pH323NetSetup->GetTaSrcPartyAddr(),sizeof(mcTransportAddress));
      m_pmcCall->SetSrcTerminalCallSignalAddress(localAddress/* V4.1c <--> V6 merge, localPort */);
      m_pmcCall->SetDestTerminalCallSignalAddress( *(m_pH323NetSetup->GetTaDestPartyAddr()) );
    }
    else // dialin
    {
      // dial in - destination is local address
      localAddress.port = localPort;
      memcpy(&localAddress,m_pH323NetSetup->GetTaDestPartyAddr(),sizeof(mcTransportAddress));
      m_pmcCall->SetDestTerminalCallSignalAddress(localAddress/* V4.1c <--> V6 merge, localPort*/);
      // source address is set in OnPartyCallAnswerDialIn()
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
void CH323Cntl::LogicalChannelUpdate(DWORD channelType, DWORD vendorType)
{
  m_pTaskApi->IpLogicalChannelUpdate(channelType,m_pH323NetSetup,vendorType);
}

/////////////////////////////////////////////////////////////////////////////
void CH323Cntl::LogicalChannelConnect(CPrtMontrBaseParams *pPrtMonitor, DWORD channelType, DWORD vendorType)
{
  m_pTaskApi->IpLogicalChannelConnect(pPrtMonitor,channelType,vendorType);
}

/////////////////////////////////////////////////////////////////////////
void  CH323Cntl::LogicalChannelDisConnect(DWORD channelType, cmCapDataType eType, BYTE bTransmitting, ERoleLabel eRole)
{
  if(!m_pmcCall->GetIsOrigin())
    if( ((CH323PartyIn *)m_pParty)->GetIsReject() || ((CH323PartyIn *)m_pParty)->GetIsLobbySetup() )
      return;// case of lobby reject the call. Party is still in the responsebility of the Lobby Task.

  m_pTaskApi->IpLogicalChannelDisConnect(channelType, eType, bTransmitting, eRole);
}

//////////////////////////////////////////////////////////////////////////
void  CH323Cntl::GatekeeperStatus(BYTE gkState, DWORD reqBandwidth, DWORD allocBandwidth, WORD requestInfoInterval, BYTE gkRouted)
{
  m_pTaskApi->H323GateKeeperStatus(gkState, reqBandwidth, allocBandwidth, requestInfoInterval, gkRouted);
}

//////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323ChannelOffInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ChannelOffInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323ChannelOffInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323ChannelOffInd - srcUnitId incorrect");
    return;
  }
  CChannel*   pChannel = NULL;
  pChannel = FindChannelInList(mcChannelIndex);
  if(pChannel)
  {
    if (pChannel->GetType() == cmCapAudio)
    {
      m_isAudioMuted = TRUE;
      CSegment*  seg = new CSegment;
      *seg <<(WORD)AIM;
      m_pTaskApi->IpRmtH230(seg); // forward task to party manager
      POBJDELETE(seg);
    }
    else if (pChannel->GetType() == cmCapVideo && pChannel->GetChannelDirection() == cmCapReceive)
    {//don't forward to the rest of the participants, because h323cntl send it
		if (pChannel->GetRoleLabel() == (DWORD)kRolePresentation && IsSlaveCascadeModeForH239() && !(m_pParty->IsCallGeneratorParty()))
		{
			CChannel* pOutPresentationChannel = FindChannelInList(cmCapVideo, TRUE, kRolePresentation);
			if (pOutPresentationChannel)
			{
				PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323ChannelOffInd: incoming presentation channel is OFF");
				m_pCurrentModeH323->SetVideoBitRate (0, cmCapReceive, kRolePresentation);
				m_LastContentRateFromMaster = 0;
				// V4.1c <--> V6 merge m_lastContentRateToSlave = 0;
				m_pTaskApi->SendTokenMessageToConfLevel(PARTY_TOKEN_RELEASE,0,0,LABEL_CONTENT,0);
				//m_pTaskApi->NotifyConfOnMasterActionsRegardingContent(MASTER_RATE_CHANGE, 0);
			}
			else
				PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323ChannelOffInd: presentation channel - out is closed - ignore");
		}
		else if (pChannel->GetRoleLabel() == (DWORD)kRolePeople)
		{
			m_pTaskApi->MuteVideo(eOn); //eOn = mute
			m_isVideoMuted = YES;
		}
    }
  }
}

//////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323ChannelOnInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ChannelOnInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323ChannelOnInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323ChannelOnInd - srcUnitId incorrect");
    return;
  }
  CChannel* pChannel = NULL;
  pChannel = FindChannelInList(mcChannelIndex);
  if(pChannel)
  {
    if(pChannel->GetType() == cmCapAudio)
    {
      m_isAudioMuted = FALSE;
      CSegment*  seg = new CSegment;
      *seg <<(WORD)AIA;
      m_pTaskApi->IpRmtH230(seg); // forward task to party manager
      POBJDELETE(seg);
    }
    else if (pChannel->GetType() == cmCapVideo)
    {
		if ((pChannel->GetRoleLabel() & kRoleContentOrPresentation) && !( m_pParty->IsCallGeneratorParty() ))
		{//forward to the rest of the participants
		PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323ChannelOnInd: presentation channel");
		/*      if (m_isVideoContentOutgoingChannelConnected && !m_isH239FlowCntlSent && m_bIsContentSpeaker)
		{
		  BYTE bIsFlowControlSent = SendFlowControlReq(cmCapVideo, FALSE, kRoleContentOrPresentation, m_curConfContRate);
		  ON(m_isH239FlowCntlSent);

		  if(bIsFlowControlSent)
		  {
			  m_pCurrentModeH323->SetVideoBitRate(m_curConfContRate, cmCapReceive, kRoleContentOrPresentation);
		  }
		}*/
		m_pTaskApi->SendContentMediaProducerStatusToConfLevel(pChannel->GetIndex(), 1/*bIsActive*/);
		}
		else if (pChannel->GetRoleLabel() == (DWORD)kRolePeople)
		{
			m_pTaskApi->MuteVideo(eOff); //eOff = unmute
			m_isVideoMuted = NO;
		}
    }
  }
}
//////////////////////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::GetLclPort(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole) const
{
  DWORD port = 0;
  if(eDirection == cmCapTransmit)
  {
    /*
    CSipChannel* pChannel = m_pCall->GetChannel(eMediaType, eDirection);
    if (pChannel)
    {
      mcTransportAddress address = pChannel->GetAddress();
      port = address.port;
    }
    */
  }
  else
  {
    switch (eMediaType)
    {
      case cmCapAudio:
        port  = m_UdpAddressesParams.AudioChannelPort;
        break;
      case cmCapVideo:
        if (eRole == kRolePeople)
          port  = m_UdpAddressesParams.VideoChannelPort;
        else
          port  = m_UdpAddressesParams.ContentChannelPort;
        break;
      case cmCapData:
        port  = m_UdpAddressesParams.FeccChannelPort;
        break;
      default:
        port  = 0;
    }
  }

  return port;
}
//////////////////////////////////////////////////////////////////////////////////
/*  This function is used in cases of allocating port by the 323 software on card
  but the call drop arrive from the operator before the getPortInd.
  If we dont receive in 3 second a get port Indication we can continue in disconnecting and drop process.
  If we do get the port we shell release it and end the process.  */
void CH323Cntl::OnH323GetPortFailed(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323GetPortFailed - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  if(m_CallConnectionState == GetPort)//destroy the timer arrive from getPortInd
  {
    DeleteTimer(H323_GETPORTFAILURE);
    if(m_getPortInd.srcCallSignalAddress.transAddr.port != NO_PORT_LEFT)
      DisconnectForCallWithoutSetup();
  }
  else
  {
    PTRACE(eLevelError,"CH323Cntl::OnH323GetPortFailed - Timer Pop Out");
    RemoveAndDisconnectCall();
  }
  return;
}


/////////////////////////////////////////////////////////////////////////////////////
/* This function notify the Party on starting a process of disconnecting
   after timeout at setup request
*/
void CH323Cntl::OnPartyConnectingTimeout(CSegment* pParam)
{
  if (m_pmcCall->GetIsClosingProcess())
    return;

  if (m_CallConnectionState == GkARQ)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnPartyConnectingTimeout: To call or not to call, it's a question. --(EXT-4632)");
    BOOL bContinueCalls = CheckAndMakeH323CallOnGKFail();
    if(bContinueCalls)
    {
        //Since calls continue, we need to restart this timer with a smaller length to handle the capability exchange!
        if(IsValidTimer(PARTYCONNECTING))
        DeleteTimer(PARTYCONNECTING);

        if(m_onGatewayCall || m_remoteIdent == PolycomMGC)
            StartTimer(PARTYCONNECTING, 90*SECOND);
        else
        StartTimer(PARTYCONNECTING, 30*SECOND);

        return;
    }

    PTRACE2INT(eLevelError,"CH323Cntl::OnPartyConnectingTimeout: disconnect call because no answer for ARQ has arrived - ", m_pCsRsrcDesc->GetConnectionId());
    m_pmcCall->SetIsClosingProcess(TRUE);
    m_pmcCall->SetCallCloseInitiator(McInitiator);
    DBGPASSERT(m_pmcCall->GetConnectionId());
    m_CallConnectionState = ReleasePort;
    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_ARQTIMEOUT);
  }
  else
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnPartyConnectingTimeout: disconnect call because it didn't get to the point of finish capabilities exchange - ", m_pCsRsrcDesc->GetConnectionId());
    m_pmcCall->SetIsClosingProcess(TRUE);
    m_pmcCall->SetCallCloseInitiator(McInitiator);
    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_REMOTE_HAS_NOT_SENT_CAPABILITY);
  }
}

//////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnTimerOpenChannelFromMcms(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnTimerOpenChannelFromMcms - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  OpenChannelFromMcms();
}

//////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnTimerOpenDataChannelFromMcms(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnTimerOpenDataChannelFromMcms - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  OpenDataChannelFromMcms();
}

/////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OpenDataChannelFromMcms()
{
      if (!m_pRmtCapH323 || m_pRmtCapH323->IsECS() || !m_pRmtCapH323->GetNumOfDataCap())
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OpenDataChannelFromMcms: No Remote Data caps");
    return;
  }
  if(m_pmcCall->GetIsClosingProcess())
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OpenDataChannelFromMcms: in closing process - Conn Id = ",m_pCsRsrcDesc->GetConnectionId() );
  }
  else
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OpenDataChannelFromMcms - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

    BYTE bIsDataOnly = TRUE;
    BYTE rval = CreateTargetComMode(bIsDataOnly);

    if(m_pTargetModeH323->IsMediaOn(cmCapData,cmCapTransmit) && m_pRmtCapH323->GetNumOfDataCap())
    {
      CCapSetInfo capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapData,cmCapTransmit);
      // BRIDGE-4551
      if(m_pRmtCapH323->GetNumOfFeccCap() /*containing annexQ or rvFecc*/ && !(m_pRmtCapH323->IsSupportAnnexQ()) && capInfo.GetIpCapCode() == eAnnexQCapCode)
	  {
		  //m_pTargetModeH323->Dump("CH323Cntl::OpenDataChannelFromMcms Target mode dump before", eLevelInfoNormal);
		  DWORD dataBitRate = m_pTargetModeH323->GetMediaBitRate(cmCapData);
		  m_pTargetModeH323->SetMediaOff(cmCapData, cmCapReceiveAndTransmit);
		  m_pTargetModeH323->SetFECCMode(eRvFeccCapCode, dataBitRate, cmCapReceiveAndTransmit);
		  //m_pTargetModeH323->Dump("CH323Cntl::OpenDataChannelFromMcms  Target mode dump after ", eLevelInfoNormal);
		  capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapData,cmCapTransmit);
	  }
      OnPartyOutgoingChannelReq( capInfo.GetIpCapCode(), kRolePeople,TRUE);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OpenChannelFromMcmsIfNeeded()
{
  // if dial in and remote is accord mcu
  // because bIsOrigin is 1 and TRUE is 255 we can't compare between the two (comment)
  DWORD mobileRate = GetSystemCfgFlagInt<DWORD>(CFG_KEY_IP_MOBILE_PHONE_RATE);

  if ((IsSlaveCascadeModeForH239() && (m_remoteIdent == PolycomMGC || m_remoteIdent == PolycomRMX)) ||
        (m_remoteIdent == EricssonVigSip && mobileRate != 0) ||(IsRemoteIsSlaveMGC() && !m_pmcCall->GetIsOrigin()) )
  {
    OpenChannelFromMcms();
  }
  else
  { //if dial out and remote Accord MCU or not (it doesn't matter)
    BYTE bNotStartTimer = FALSE;
    if (m_pParty->IsRemoteCapsEcs())
    {
      bNotStartTimer = ECSDecideOnOpeningOutChannels();
      m_pParty->ResetEcs();
    }
    // We will add another check - If there are any channels in connection state - No need to start the timer
    // Since no need to invoke the opening from our side if the channles flow started.
    CChannel* pChannel = NULL;
    if(m_pmcCall->GetChannelsCounter())
    {
      PTRACE(eLevelError,"CH323Cntl::OpenChannelFromMcmsIfNeeded - channelsCounter isn't zero");
      for (int i=0; i< (int)m_pmcCall->GetChannelsCounter(); i++)
      {
        pChannel = m_pmcCall->GetSpecificChannel(i);
        if (pChannel)
        {
          if (pChannel->GetCsChannelState() != kDisconnectedState)
          {
            bNotStartTimer = TRUE;
            break;
          }
        }


      }
    }

    if (bNotStartTimer == FALSE)
    {
      if (m_remoteIdent == PolycomNPG ||
                m_remoteIdent == RVTestapplication ||
                m_remoteIdent == EricssonVIG ||
                m_remoteIdent == AvistarGW)
        StartTimer(MCMSOPENCHANNELS, 1*SECOND);
      else
        StartTimer(MCMSOPENCHANNELS, 20*SECOND);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OpenChannelFromMcms()
{
	if  (!m_isH245Connected)
	{
		PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms - H245 isn't connected yet");
		return;
	}
	if (!m_pRmtCapH323 || m_pRmtCapH323->IsECS())
	{
		PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms: No Remote caps Or ECS");
		return;
	}
	if(m_pmcCall->GetIsClosingProcess())
	{
		PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms: in closing process");
	}
	//else if(!m_OneOfTheMediaChannelWasConnected)
	else
	{
		//PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms: ",m_OneOfTheMediaChannelWasConnected);
		PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms");
		BYTE bIsDataOnly = FALSE;

		//HDCP
		BYTE bIsCp  = (m_pTargetModeH323->GetConfType() == kCp);
		CapEnum scmProtocol = eUnknownAlgorithemCapCode;
		scmProtocol= (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo, cmCapTransmit);
		if (bIsCp && (scmProtocol == eH264CapCode) && m_pLocalCapH323->OnCap(eH264CapCode))
		{
			CBaseVideoCap* pIntersect = NULL;

			////////////////////////////HD1080At60/////////////////////////////////////////////////
			// Currently we support HD1080At60 only in MPMx based system and only in asymmetric mode
			BYTE bRemoteSupportHD1080at60Fps =  m_pRmtCapH323->IsCapableOfHD1080();
			if(m_pTargetModeH323->IsHd1080At60Enabled() && bRemoteSupportHD1080at60Fps)
			{
				PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms - Possible HD1080 60 fps");
				CComModeH323 scmWithHD1080At60;
				scmWithHD1080At60 = *m_pTargetModeH323;
				APIU8 prof = scmWithHD1080At60.GetH264Profile(cmCapTransmit);
				scmWithHD1080At60.SetScmToCpHD1080At60(cmCapReceiveAndTransmit);
				scmWithHD1080At60.SetH264Profile(prof,cmCapTransmit);
				pIntersect = m_pRmtCapH323->FindIntersectionBetweenCapsAndVideoScm(&scmWithHD1080At60);
				if (pIntersect != NULL)
				{
					PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms - Found HD1080 60 fps");
					*m_pTargetModeH323 = scmWithHD1080At60;
				}
			}
			/////////////////////HD720@60 Asymmetric
			//In MPM+ Based system the HD720 60fps  call is asymmetric
			BYTE bRemoteSupportHD720At60 = m_pRmtCapH323->IsCapableOfHD720At60();
			BYTE bLocalSupportHD720At60 = m_pTargetModeH323->IsHd720At60Enabled();
			if (bRemoteSupportHD720At60 && bLocalSupportHD720At60)
			{
				CComModeH323* pScmWithHD720At60 = new CComModeH323;
				*pScmWithHD720At60 = *m_pTargetModeH323;
				pScmWithHD720At60->SetScmToHdCp(cmCapReceiveAndTransmit);
				pIntersect = m_pRmtCapH323->FindIntersectionBetweenCapsAndVideoScm(pScmWithHD720At60);
				if (pIntersect != NULL)
				{
					PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms - Found HD720At60");
					*m_pTargetModeH323 = *pScmWithHD720At60;
				}
				POBJDELETE(pScmWithHD720At60);
			}
		}
		BYTE rval = CreateTargetComMode(bIsDataOnly);


		if(rval == NOCAP)
		{
			PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms: Has failed to open channels");
			return;
		}

		ON(m_McmsOpenChannels);

		if(!m_pTargetModeH323->IsMediaOff(cmCapAudio,cmCapTransmit) && m_pRmtCapH323->GetNumOfAudioCap() )
		{
			PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms: cmCapAudio is ON");
			CCapSetInfo capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapAudio,cmCapTransmit);
			OnPartyOutgoingChannelReq( capInfo.GetIpCapCode(), kRolePeople,TRUE);
		}
		if(!m_pTargetModeH323->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople) && m_pRmtCapH323->GetNumOfVideoCap())
		{
			PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms: cmCapVideo is ON");
			CCapSetInfo capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople);
			OnPartyOutgoingChannelReq( capInfo.GetIpCapCode(), kRolePeople,TRUE);
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms: SetSecondaryCause");
			//If video media is off it the call will move to secondary. I think the cause we do not found match can be
			//only protocol different.
			CSecondaryParams secParams;
			m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_RMT_DIFF_CAPCODE,secParams);
		}

		if(!m_pTargetModeH323->IsMediaOff(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation) && (m_pRmtCapH323->IsH239() || m_pRmtCapH323->IsEPC()) )
		{
			PTRACE(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms: Open content channel");
			ERoleLabel roleToOpen = kRolePresentation;
			if(m_pLocalCapH323->IsH239() && m_pRmtCapH323->IsH239())
				roleToOpen = kRolePresentation;
			else if(m_pLocalCapH323->IsEPC() && m_pRmtCapH323->IsEPC())
				roleToOpen = kRoleContent;
			if(IsRemoteIsSlaveMGC() && !m_pmcCall->GetIsOrigin() && m_pLocalCapH323->IsEPC() && m_pRmtCapH323->IsEPC())
				roleToOpen = kRoleContent;

			CCapSetInfo capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation);
			OnPartyOutgoingChannelReq( capInfo.GetIpCapCode(), roleToOpen, TRUE);
		}
		if(!m_pTargetModeH323->IsMediaOff(cmCapData,cmCapTransmit) && m_pRmtCapH323->GetNumOfDataCap())
		{
			CCapSetInfo capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapData,cmCapTransmit);
			OnPartyOutgoingChannelReq( capInfo.GetIpCapCode(), kRolePeople,TRUE);
		}
	}
	//else
	//  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OpenChannelFromMcms - NOT DONE!!!: ", m_OneOfTheMediaChannelWasConnected);
}
/*
/////////////////////////////////////////////////////////////////////////////////////
// sending the cap_req tp the EP after receiving them from the party in a delay capabolity mode ( GW call)
void CH323Cntl::SendDelayCapAtGatewayCallReq(CComMode& pTargetMode, CCapH323& capH323)
{
  PTRACE(eLevelInfoNormal,"CH323Cntl::SendDelayCapAtGatewayCallReq: ");

    POBJDELETE(m_pLocalCapH323);
  m_pLocalCapH323   = new CCapH323;
    *m_pLocalCapH323  = capH323;

  if(m_bIsRemoveGeneric)
    RemoveGenericCapAndUpdateConfLevel();

  OnPartyCreateControl();
}
*/
/////////////////////////////////////////////////////////////////////////////////////
// set the H323netSetup after chanching it from the GW conf
void CH323Cntl::SetMaxRateAtNetsetup(DWORD confRate)
{
  m_pH323NetSetup->SetMaxRate(confRate);
}

/////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendConferenceResponse(ConferenceResponseEnum response)
{
    mcReqConferenceRes* pConfResp = new  mcReqConferenceRes;

    pConfResp->ConResOpcode = (APIU32)response;
    int i = 0;
    for (i = 0; i < m_maxCallChannel; i++)
    {
  	    CChannel* pChannel =(m_pmcCall) ? m_pmcCall->GetSpecificChannel(i) : NULL; //KW 1237
  	    if (pChannel)
        {
            if ((pChannel->GetType() == cmCapVideo) &&
                        (! pChannel->IsOutgoingDirection()))
            {
                //table_entry = i;
                break;
            }
        }
    }
    if (i >= m_maxCallChannel)
    {
        PTRACE(eLevelInfoNormal,"CH323Cntl::SendConferenceResponse: No channel found");
        PDELETE(pConfResp);
        return;
    }
  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)(pConfResp),sizeof(mcReqConferenceRes));
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_CONFERENCE_RES_REQ,pMsg,m_serviceId,
          m_serviceId,m_pDestUnitId,m_callIndex,m_pmcCall->GetSpecificChannel(i)->GetCsIndex(),m_pmcCall->GetSpecificChannel(i)->GetIndex(),0);
  POBJDELETE(pMsg);
    PDELETE(pConfResp);
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::SendRemoteNumbering()
{
  if (m_pTargetModeH323->IsMediaOn(cmCapData, cmCapTransmit) || (m_remoteIdent == PolycomMGC && !IsSlaveCascadeModeForH239() ))
  {
    SendMultipointModeComTerminalID();
    SendUpdateMtPairReq();

  }
}

////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::SendMultipointModeComTerminalID() const
{
    if (IsSlaveCascadeModeForH239() || (m_pParty->IsGateway() && (strstr(m_remoteVendor.productId,"HDX") || strstr(m_remoteVendor.productId,"VSX"))) )
    {
        PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SendMultipointModeComTerminalID Don't send numbering - slave mode - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
        return;
    }


  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SendMultipointModeComTerminalID - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  mcReqMultipointModeComTerminalIDMessage *pReq = new mcReqMultipointModeComTerminalIDMessage;
  memset(pReq, 0, sizeof(mcReqMultipointModeComTerminalIDMessage));

  if (IsRemoteIsSlaveMGC())
  {
    WORD mcuNum = m_pParty->AllocateMcuNumber();

    pReq->mcuID      = mcuNum;
    pReq->terminalID = 0;
//    static int i = 1;
//    pReq->mcuID      = m_pParty->GetMcuNum()+i;
//    pReq->terminalID = 0;
//    i++;
  }
  else
  {
    pReq->mcuID      = m_pParty->GetMcuNum();
    pReq->terminalID = m_pParty->GetTerminalNum();
  }

  //---CONFERENCE REQUEST-RESPONCE---------------------------------------------------------------------//
  if(TandbergEp == m_remoteIdent)
  {
    pReq->mcuID      = 0xFFFFFFFF;// if(0xFFFFFFFF == mcuID)&&(0xFFFFFFFE == terminalID) - TandbergEp
    pReq->terminalID = 0xFFFFFFFE;//
  }
  //---------------------------------------------------------------------------------------------------//

  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)pReq,sizeof(mcReqMultipointModeComTerminalIDMessage));

  m_pCsInterface->SendMsgToCS(H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ, pMsg, m_serviceId, m_serviceId, m_pDestUnitId, m_callIndex, 0,0,0);

  PDELETE(pReq);
  POBJDELETE(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendConferenceIndReq(WORD opcode, WORD mcuNum, WORD terminalNum, PartyRsrcID partyId)
{
    mcReqConferenceInd* pConfIndReq = new  mcReqConferenceInd; AUTO_DELETE(pConfIndReq);

  switch(opcode)
  {
    case SEND_VIN:
    {
      pConfIndReq->ConIndOpcode = (APIU32)terminalYouAreSeeingIndication;
      pConfIndReq->val1 = mcuNum;
      pConfIndReq->val2 = terminalNum;

      m_speakerMcuNum   = mcuNum;
      m_speakerTermNum  = terminalNum;
      m_speakerPartyId	= partyId;

      //in RMX2000C when SEND_VIN received from conf it means that the token request
      //is from PCM and we should let RTP know that they should actualy move the camera
      m_isCameraControl = 1;

      TRACEINTO << "CH323Cntl::SendConferenceIndReq "
                    << "- McuNum"       	<< mcuNum
                    << ", TermNum:"         << terminalNum
                    << ", PartyId:"  		<< partyId
                    << ", isCameraControl: "<< (m_isCameraControl ? "yes" : "no");

      break;
    }
    default:
    {
      TRACEINTO <<"CH323Cntl::SendConferenceIndReq, No such opcode = " << opcode;
      return;
    }
  }

  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)pConfIndReq, sizeof(mcReqConferenceInd));
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_CONFERENCE_IND_REQ, pMsg, m_serviceId,
                  m_serviceId, m_pDestUnitId, m_callIndex, 0,0,0);
  POBJDELETE(pMsg);
    PDELETE(pConfIndReq);
}

///////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323ConferenceReqInd(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ConferenceReqInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323ConferenceReqInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323ConferenceReqInd - srcUnitId incorrect");
    return;
  }
  mcIndConferenceReq pConfReq;

  DWORD  structLen = sizeof(mcIndConferenceReq);
  memset(&pConfReq,0,structLen);
  pParam->Get((BYTE*)(&pConfReq),structLen);

    CSegment  *pSeg     = new CSegment;

  switch (pConfReq.ConReqOpcode)
  {
    case broadcastMyLogicalChannelRequest :
    {
      *pSeg << (DWORD)MCV;
      *pSeg << mcChannelIndex;
      m_pTaskApi->H323RmtCI(pSeg); // forward task to party manager
      break;
    }
    default :
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323ConferenceReqInd - Unknown/Unsupported Request");
      break;
    }
  }

    POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323ConferenceComInd(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ConferenceComInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323ConferenceComInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323ConferenceComInd - srcUnitId incorrect");
    return;
  }
  mcIndConferenceCom pConfComm;

  DWORD  structLen = sizeof(mcIndConferenceCom);
  memset(&pConfComm,0,structLen);
  pParam->Get((BYTE*)(&pConfComm),structLen);
    CSegment              *pSeg     = new CSegment;
  switch (pConfComm.ConComOpcode)
  {
    case cancelBroadcastMyLogicalChannelCommand :
    {
      *pSeg << (DWORD)Cancel_MCV;
      *pSeg << mcChannelIndex;
      m_pTaskApi->H323RmtCI(pSeg); // forward task to party manager
      break;
    }
    default :
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323ConferenceComInd - Unknown/Unsupported Request");
      break;
    }
  }
    POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323ConferenceResInd(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ConferenceResInd - ", m_pCsRsrcDesc->GetConnectionId());
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323ConferenceIndInd(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323ConferenceIndInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  if( m_pmcCall->GetIsClosingProcess() == TRUE) //If the call is in disconnection process this message is not relevant.
    return;

  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323ConferenceIndInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323ConferenceIndInd - srcUnitId incorrect");
    return;
  }
  mcIndConferenceInd pConfInd;

  DWORD  structLen = sizeof(mcIndConferenceInd);
  memset(&pConfInd,0,structLen);
  pParam->Get((BYTE*)(&pConfInd),structLen);

  switch( pConfInd.ConIndOpcode )
  {
    case terminalNumberAssignIndication :
    {

      if(IsSlaveCascadeModeForH239())
      {
        m_mcuNumFromMaster      = pConfInd.val1;
        m_terminalNumFromMaster     = pConfInd.val2;
      }
      else
        PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323ConferenceIndInd - terminalNumberAssignIndication not in cascade mode.");
      break;
    }
    default :
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323ConferenceIndInd - Unknown/Unsupported Request");
      break;
    }
  }
}

//  All generic NonStandard events
/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323NonStandardReqInd(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323NonStandardReqInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    OnH323NonStandard(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323NonStandardComInd(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323NonStandardComInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    OnH323NonStandard(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323NonStandardResInd(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323NonStandardResInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    OnH323NonStandard(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323NonStandardIndInd(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323NonStandardIndInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    OnH323NonStandard(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323NonStandard(CSegment* pParam)
{
    m_pTaskApi->H323NonStandardInd(pParam);
}

///////////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsSupportingDBC2(CChannel* pChannel)
{
  cmCapDataType eType   = pChannel->GetType();
  CapEnum capCode     = pChannel->GetCapNameEnum();
  BYTE bLocalSupportDBC2  = m_pLocalCapH323->IsDBC2();
  BYTE bRmtSupportDBC2  = m_pRmtCapH323->IsDBC2();
  ERoleLabel eRole    = (ERoleLabel)pChannel->GetRoleLabel();

  if((eType == cmCapVideo) && (capCode == eH263CapCode) && (eRole == kRolePeople) && bLocalSupportDBC2 && bRmtSupportDBC2)
    return TRUE;
  else
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323DBC2CommandInd(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323DBC2CommandInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
#ifdef _CARMEL_VIDEO__
  H323_MIIND_DBC2_COMMAND_S *pDBC2Ind = (H323_MIIND_DBC2_COMMAND_S*)(pParam->GetPtr());

  DWORD opcode    = pDBC2Ind->header.serialHeader.H323opcode;
  TChannel* pChannel = NULL;

  switch(opcode)
  {
  case IP_DBC2_COMMAND_CT_ON_IND:
  case IP_DBC2_COMMAND_CT_OFF_IND:
    pChannel = FindChannelInList(cmCapVideo, TRUE);;
    break;
  case IP_DBC2_COMMAND_RTP_ON_IND:
  case IP_DBC2_COMMAND_RTP_OFF_IND:
    pChannel = FindChannelInList(cmCapVideo, FALSE);;
    break;
  default:
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnConfDBC2Command - unknown opcode");
  }

  if ((pChannel) && (pChannel->channelCloseInitiator == NoInitiator))
  {
    if(pChannel->bIsDBC2)
    {
      DWORD refreshRate = pDBC2Ind->refreshRate;
      DWORD interLeave  = pDBC2Ind->interLeave;
      DWORD mpiLimit    = pDBC2Ind->mpiLimit;
      DWORD motionVector  = pDBC2Ind->motionVector;
      DWORD noEncapsulation = pDBC2Ind->noEncapsulation;
      DWORD overlap   = pDBC2Ind->overlap;

      CSegment *pSeg     = new CSegment;

      *pSeg << opcode;
      *pSeg << refreshRate;
      *pSeg << interLeave;
      *pSeg << mpiLimit;
      *pSeg << motionVector;
      *pSeg << noEncapsulation;
      *pSeg << overlap;

      m_pTaskApi->H323DBC2Command(pSeg);

      POBJDELETE(pSeg);
    }
    else
      PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323DBC2CommandInd: channel doesn't support DBC2. ");
  }
  else
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323DBC2CommandInd: channel wasn't found or in disconnecting mode ");

#endif
}
/////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnConfDBC2Command(DWORD opcode,DWORD refreshRate,DWORD interLeave,DWORD mpiLimit,DWORD motionVector,
                   DWORD noEncapsulation,DWORD overlap)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnConfDBC2Command - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
#ifdef _CARMEL_VIDEO__
  H323_MCREQ_DBC2_COMMAND_S *pDBC2Req = new H323_MCREQ_DBC2_COMMAND_S;
  TChannel* pChannel = NULL;

  BYTE requestIndex = TRUE;
  switch(opcode)
  {
  case IP_DBC2_COMMAND_CT_ON_REQ:
  case IP_DBC2_COMMAND_CT_OFF_REQ:
    requestIndex = TRUE;
    pChannel = FindChannelInList(cmCapVideo, FALSE);;
    break;
  case IP_DBC2_COMMAND_RTP_ON_REQ:
  case IP_DBC2_COMMAND_RTP_OFF_REQ:
    requestIndex = FALSE;
    pChannel = FindChannelInList(cmCapVideo, TRUE);;

    break;
  default:
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnConfDBC2Command - unknown opcode");
  }

  if ((pChannel) && (pChannel->channelCloseInitiator == NoInitiator))
  {
    pDBC2Req->refreshRate = refreshRate;
    pDBC2Req->interLeave  = interLeave;
    pDBC2Req->mpiLimit    = mpiLimit;
    pDBC2Req->motionVector  = motionVector;
    pDBC2Req->noEncapsulation = noEncapsulation;
    pDBC2Req->overlap   = overlap;

    m_pH323->InitiatePmHeader(&pDBC2Req->header.pmHeader,pChannel);
    m_pH323->H323McmsReq((BYTE*)pDBC2Req,opcode,EMBDPARAMSIZE(H323_MCREQ_DBC2_COMMAND_S),requestIndex);
  }
  else
    PTRACE(eLevelInfoNormal|EPC_TRACE,"CH323Cntl::OnConfDBC2Command: channel wasn't found or on disconnecting mode");

  PDELETE(pDBC2Req);
#endif
}


/////////////////////////////////////////////////////////////////////////////
//This function is in case of call forward. Finding the alternative aliases of the alternative card
/*char* CH323Cntl::findAlternativeAdress(CIPSpan* pSpan, WORD boardID)
{ //need to be ordered in that way TA:192.168.5.106,NAME:Yossi-Vcon,TEL:11,11
***  WORD n = pSpan->GetAliasNamesNumber();
  char *info = NULL;
  WORD type;

  if (n!=0)
    if( (n > MaxNumberOfAliases) || (n < MinAliasNumber) )
      return FALSE;

  ALLOCBUFFER(dstAddr, (n+2)*(ALIAS_NAME_LEN+7) );
  strncat(dstAddr,"TA:",4);
  char* destIpAddress = (char*)::GetH323IPaddress(boardID);
  strcat(dstAddr,destIpAddress);
  PDELETEA(destIpAddress);

  if(n)
    strncat(dstAddr,",",2);

  BOOL IsE164 = FALSE;  //only one E-164 should come with and also without TEL

  for( int i = 0; i< n ; i++ )
  {
    CH323Alias* pH323Alias = pSpan->GetAlias(i);
    if (pH323Alias)
    {
        info = pH323Alias->m_aliasName;
      type = pH323Alias->GetAliasType();

      switch(type)
      {
        case PARTY_H323_ALIAS_E164_TYPE :
        {
          strncat(dstAddr,"TEL:",6);
          strcat(dstAddr, info);
          if(!IsE164) //if this is the first E164 alias
          {
            strncat(dstAddr,",",2);
            strcat(dstAddr, info);  //without TEL
            IsE164 = TRUE;
          }
          if(i != n-1)
            strncat(dstAddr,",",2);
          break;
        }
        case PARTY_H323_ALIAS_H323_ID_TYPE :
        {
          strncat(dstAddr,"NAME:",7);
          strncat(dstAddr, info,IP_STRING_LEN);
          if(i != n-1)
            strncat(dstAddr,",",2);
          break;
        }
      }// endswitch
    }
  }
  return dstAddr;*/
//}
/*
//////////////////////////////////////////////////////////////////////////
//This function is in case of facility route call to GK. Finding the ip of the Gk
char* CH323Cntl::FindGKAdress()
{ //need to be ordered in that way TA:192.168.5.106
  ALLOCBUFFER(dstAddr, 18 );
  strncat(dstAddr,"TA:",3);

  DWORD GkIp;
  GkIp = GetCurrentGkIP(m_pRsrcDesc->m_boardId);
  if(!GkIp)
    PTRACE(eLevelError|H323CNTL_TRACE , "CH323Cntl::FindAlternativeGKAdress - could not get GkIP");

  CLanCfg *pLanCfg = new CLanCfg;
  char* destIpAddress = pLanCfg->TranslDwordToString(GkIp);

  strcat(dstAddr,destIpAddress);

  PDELETEA (destIpAddress);
  POBJDELETE(pLanCfg);
  return dstAddr;
}
*/

//////////////////////////////////////////////////////////////////////////
//Add for BRQ*/
BYTE CH323Cntl::CalculateNewBandwidth(DWORD& channelsTotalRate)
{
  DWORD  audBitrate = 0;
  DWORD outAudBitRate=0;
  DWORD channelsTotalRateOnlyConsiderOut=0;
  CChannel* pInAudio = FindChannelInList(cmCapAudio, FALSE);
  if (pInAudio)
    audBitrate = pInAudio->GetRate() * _K_;
  CChannel* pOutAudio = FindChannelInList(cmCapAudio, TRUE);
  if (pOutAudio)
    outAudBitRate = pOutAudio->GetRate() * _K_;

  DWORD outVidBitrate = m_pParty->GetVideoRate() * 100;

  DWORD  inVidBitrate = 0;
  CChannel* pChannel = FindChannelInList(cmCapVideo,FALSE,kRolePeople);
  if(pChannel)
    inVidBitrate = pChannel->GetRate() * 100;


  BOOL audioOnlyConnect = FALSE;
  if (pInAudio && m_isAudioOutgoingChannelConnected)
  {
    if (m_pParty->GetIsVoice() ||
      (!m_pRmtCapH323->OnType(cmCapVideo) && !m_pRmtCapH323->OnType(cmCapData)) ||
      m_pCurrentModeH323->IsSecondary())
      audioOnlyConnect = TRUE;
  }

  if (audioOnlyConnect)
  {
    if (pOutAudio)
    {
      channelsTotalRate=audBitrate*2;
      channelsTotalRateOnlyConsiderOut=outAudBitRate;
    }
    else
    {
      channelsTotalRate=audBitrate*2;
      channelsTotalRateOnlyConsiderOut = audBitrate;
    }
  }
  else if(pOutAudio)
  {
    channelsTotalRate = inVidBitrate + outVidBitrate + (audBitrate*2);
    channelsTotalRateOnlyConsiderOut = outVidBitrate + outAudBitRate;

  }
  else
  {
    channelsTotalRate = inVidBitrate + outVidBitrate + (audBitrate*2);
    channelsTotalRateOnlyConsiderOut = outVidBitrate + audBitrate;
  }

  DWORD allocatedRate = (m_pmcCall->GetBandwidth() / 2);

  BYTE bIsBrqNedded = FALSE;
  //PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CalculateNewBandwidth - TOTAL RATE = ",channelsTotalRate);
  //PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CalculateNewBandwidth - ALLOCATE RATE = ",allocatedRate);
  if (allocatedRate < channelsTotalRateOnlyConsiderOut)
  {
    if(m_gkRequestedBrqBw != (int)channelsTotalRate )
    {
    	TRACEINTO << "CH323Cntl::CalculateNewBandwidth - m_gkRequestedBrqBw = "<< m_gkRequestedBrqBw <<
    			     ", channelsTotalRate = " << channelsTotalRate;
    ON(bIsBrqNedded);
    }
  }

  else if (allocatedRate > channelsTotalRateOnlyConsiderOut)
  {
    if (pChannel) //if the video in channel is opened
    {
      //m_pLocalCapH323->GetH261MaxBitRate is in 0.1k so we need to mult it by 100 in CalculateCardRate function

      CCapSetInfo capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo);
      int mcmsVideoRate    = m_pLocalCapH323->GetMaxVideoBitRate(capInfo, cmCapReceiveAndTransmit);

      DWORD rateAfterDefence = mcmsVideoRate;

      // If the new bandwidth is lower than the original only because of the defence rate for video in, don't send BRQ.
      DWORD totalGap   = (allocatedRate - channelsTotalRateOnlyConsiderOut)/100;
      DWORD defenceGap = mcmsVideoRate - rateAfterDefence;
      //the only case that it is small and not equal is when the outgoing rate is changed as a result of the function ChangeVideoRateInCp
      if (totalGap > defenceGap)
      {
    	  if( m_gkRequestedBrqBw != (int)channelsTotalRate )
    	  {
    	    TRACEINTO << "CH323Cntl::CalculateNewBandwidth - m_gkRequestedBrqBw = "<< m_gkRequestedBrqBw <<
    	    			     ", channelsTotalRate = " << channelsTotalRate;
        ON(bIsBrqNedded);
    	  }
      }
    }
  }
  return bIsBrqNedded;
}

///////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323DTMFInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323DTMFInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIS8 arrayIndex;
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323DTMFInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323DTMFInd - srcUnitId incorrect");
    return;
  }
  mcIndDtmfBuff p_DtmfInd;

  DWORD  structLen = sizeof(mcIndDtmfBuff);
  memset(&p_DtmfInd,0,structLen);
  pParam->Get((BYTE*)(&p_DtmfInd),structLen);

  if (!m_isH245Connected)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::OnH323DTMFInd - H245 isn't connected yet");
    return;
  }

  APIU8* buffer = new APIU8[DtmfBuffLen+1];
  strncpy((char*)buffer,(char*)p_DtmfInd.dtmfBuffer,DtmfBuffLen);
  buffer[DtmfBuffLen] = '\0';

  unsigned long unLength = strlen((const char *)buffer);
  if (unLength < DtmfBuffLen)
    unLength =unLength;
  else
    unLength=DtmfBuffLen;
  m_pTaskApi->sendPartyDTMFInd(buffer, unLength);
  PDELETEA(buffer);
}

//////////////////////////////////////////////////////////////////////////
BOOL CH323Cntl::IsChannelConnected(cmCapDataType dataType,cmCapDirection direction, ERoleLabel eRole)
{
  BYTE bIsTransmiting = CalcIsTransmiting(direction);
  CChannel* pChannel = FindChannelInList(dataType, bIsTransmiting, eRole);

  if (pChannel && pChannel->IsChannelConnected())
    return TRUE;
  else
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::IsSymmetricProtocol(CCapSetInfo capInfo, ERoleLabel eRole, channelSpecificParameters* pChannelParams)
{
  DWORD   bSymmetric    = TRUE;
  DWORD   length    = 0;

  cmCapDataType dataType = capInfo.GetCapType();

  BYTE bCheck = FALSE;

  if ((dataType == cmCapVideo) && (eRole == kRolePeople))
  {
    if ((m_pTargetModeH323->GetConfType() == kVideoSwitch) || (m_pTargetModeH323->GetConfType() == kVSW_Fixed))
       bCheck = TRUE; //the only case that it needs to be symetric in video
  }

  else
  {
    CChannel* pChannel = FindChannelInList(capInfo.GetCapType(), TRUE, eRole);
    if (pChannel)
    {
      bCheck = ((pChannel->IsCsChannelStateConnecting()) || (pChannel->GetCsChannelState() == kConnectedState));
      if (dataType == cmCapAudio)
        bCheck &= !m_isIncomingAudioHasDisconnectedOnce;
    }
  }

  if (bCheck == FALSE)
    return TRUE;

  length = m_pTargetModeH323->GetMediaLength(dataType,cmCapTransmit,eRole);
  if(length == 0)
    return FALSE;

  capBuffer *pCapBuffer;

  // in real OOP the cupBuffer would have not created in this file!!
  pCapBuffer = (capBuffer *)new BYTE[length + sizeof(capBufferBase)];
  if(pCapBuffer != NULL)
  {
    m_pTargetModeH323->CopyMediaToCapBuffer(pCapBuffer,dataType,cmCapTransmit,eRole);
    CCapSetInfo currecntInfo = (CapEnum)pCapBuffer->capTypeCode;
    if(currecntInfo.GetIpCapCode() != capInfo.GetIpCapCode())
    {
      bSymmetric = FALSE;
      TRACEINTO << "Channel Different in type"
              << " Outgoing " << currecntInfo.GetH323CapName()
            << " Incoming " << capInfo.GetH323CapName();
    }

    else
    {
      CBaseCap* pCurrentCap = CBaseCap::AllocNewCap((CapEnum)currecntInfo, pCapBuffer->dataCap);
      CBaseCap* pChannelCap = CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE *)pChannelParams);

      // is channel params OK check if the parameters are part of the capability
      if (pCurrentCap && pChannelCap)
      {
        BYTE bRes = FALSE;
        DWORD details = 0;
        DWORD valuesToCompare;

        if(dataType == cmCapVideo)
          valuesToCompare = kAnnexes;
        else
          valuesToCompare = kBitRate|kFormat;

        bRes = pCurrentCap->IsContaining(*pChannelCap, valuesToCompare, &details);
        if (bRes == FALSE)
        {
          bSymmetric = FALSE;
          cmCapDataType eType = pCurrentCap->GetType();
//          DumpDetailsToStream(eType,details,msg);
        }
      }

      POBJDELETE(pCurrentCap);
      POBJDELETE(pChannelCap);
    }

    if(bSymmetric == FALSE)
      PTRACE(eLevelInfoNormal,"CH323Cntl::IsSymmetric: Failure details: ");
  }

  if(pCapBuffer)
    PDELETEA(pCapBuffer);

  return bSymmetric;
}


//////////////////////////////////////////////////////////////////////////
// When we close outgoing channel we check if we can open a new outgoing channel
// This will happen if several conditions will occur
// 1. The call is not in the process of disconnecting
// 2. Incoming channel from the same type is connecting or connected.
// 3. No previous channel from the same type.

// Open questions:
//    To verify that the card will reject outgoing channel if its in call disconnecting process.
//    Should the Outgoing channel be symmetric to the incoming or just be open from SCM
//        (I am in fever the SCM for future use)

void CH323Cntl::OpenOutgoingChannel(cmCapDataType dataType, ERoleLabel eRole)
{
  if (m_pmcCall->GetIsClosingProcess() == TRUE)
    return;

  CChannel* pChannel = FindChannelInList(dataType,FALSE,eRole);

  if(pChannel != NULL)
  {
    if((pChannel->IsCsChannelStateConnecting()) || (pChannel->GetCsChannelState() == kConnectedState))
    { // try to open outgoing channel only if the incoming channel has been opened or is
      // about to be opened
      if (m_pTargetModeH323->IsMediaOn(dataType, cmCapReceive))
      {
        CapEnum protocol = (CapEnum)m_pTargetModeH323->GetMediaType(dataType, cmCapReceive,eRole);
//        if (dataType == cmCapAudio)
//        {/* In case of ECS from remote, or audio shuffle, we should check if remote
//             supports this protocol, because its audio capabilities might be changed.
//           We must check according to the incoming protocol, because asymmetric audio
//           channels aren't supported in audio bridge. */
//          BYTE bRemoteSupport = m_pRmtCapH323->AreCapsSupportProtocol(protocol, cmCapAudio);
//          if (bRemoteSupport == FALSE)
//          {
//            PTRACE2(eLevelInfoNormal,"CH323Cntl::OpenOutgoingChannel: Remote doesn't support incoming protocol. Name - ",PARTYNAME);
//            return;
//          }
//        }
        BYTE bIsMcmsOrigin = FALSE;

        if (eRole & kRoleContentOrPresentation)
          bIsMcmsOrigin = TRUE;
        OnPartyOutgoingChannelReq(protocol, eRole, bIsMcmsOrigin,TRUE);
      }
    }
    else
      PTRACE2(eLevelInfoNormal,"CH323Cntl::OpenOutgoingChannel: Incoming channel isn't at connecting or connected state. Name - ",PARTYNAME);
  }
  else
    PTRACE2(eLevelInfoNormal,"CH323Cntl::OpenOutgoingChannel: Incoming channel wasn't found. Name - ",PARTYNAME);
}

////////////////////////////////////////////////////////////////////////////////
//Disconnect a call which the setup message wasn't sent for it.
void CH323Cntl::DisconnectForCallWithoutSetup()
{
  if (m_pmcCall->GetIsOrigin()) // dial out.
  {
    OnCallReleasePortReq();
    RemoveAndDisconnectCall();
  }
  else //dial in
  {
    m_pmcCall->SetCallCloseInitiator(McInitiator);
    m_pmcCall->SetCallStatus(-1);
    OnPartyCallAnswerReq();
    //In Call_Answer_Req with -1 ,we call to RemoveAndDisconnectCall only after Call_Idle
  }
}

/////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323RtpBadSpontaneuosInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323BadSpontaneuosInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  TRtpBadSpontaneousInd pSpontaneuos;
  DWORD  structLen = sizeof(TRtpBadSpontaneousInd);
  memset(&pSpontaneuos,0,structLen);
  pParam->Get((BYTE*)(&pSpontaneuos),structLen);

  if (pSpontaneuos.unBadSpontReason < lastBadSpontanIndReason)
  {
    CMedString str;
    str << "Channel type: " << pSpontaneuos.unChannelType
        << "Channel direction: " << pSpontaneuos.unChannelDirection
        << "Reason: " << g_badSpontanIndReasonStrings[pSpontaneuos.unBadSpontReason];
    PTRACE2(eLevelError,"CH323Cntl::OnH323BadSpontaneuosInd:  = ", str.GetString());
  }
  else
    PTRACE(eLevelError,"CH323Cntl::OnH323BadSpontaneuosInd - Unsupported BadSpontaneuosIndication");
}

/////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323CsBadSpontaneuosInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323CsBadSpontaneuosInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  mcIndBadSpontan pSpontaneuos;
  DWORD  structLen = sizeof(mcIndBadSpontan);
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  memset(&pSpontaneuos,0,structLen);
  pParam->Get((BYTE*)(&pSpontaneuos),structLen);

  if (pSpontaneuos.status < lastBadSpontanIndReason)
  {
    CSmallString str;
    str <<  g_badSpontanIndReasonStrings[pSpontaneuos.status];
    PTRACE2(eLevelError,"CH323Cntl::OnH323CsBadSpontaneuosInd:  Reason= ", str.GetString());
  }
  else
    PTRACE(eLevelError,"CH323Cntl::OnH323CsBadSpontaneuosInd - Unsupported BadSpontaneuosIndication");

  switch(pSpontaneuos.status)
  {
    case noConnWithRemoteInd:
    {
      if(m_pmcCall->GetIsClosingProcess() != TRUE)
      {
        BOOL bIsEnableRoundTripDelay = GetSystemCfgFlagInt<BOOL>(CFG_KEY_IP_ENABLE_ROUNDTRIPDELAY);
        if(bIsEnableRoundTripDelay)
        {
          m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_REMOTE_STOP_RESPONDING);// need to put here the true reason
          // the connection with the remote is lost.
          m_pmcCall->SetCallCloseInitiator(McInitiator);
          m_pmcCall->SetIsClosingProcess(TRUE);
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }

}

///////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendStopAllProcessorsToCard(mcReqStopAllProcesses* pStopAllProcessors, CChannel *pMcChannel)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SendStopAllProcessorsToCard - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)(pStopAllProcessors),sizeof(mcReqStopAllProcesses));
  m_pCsInterface->SendMsgToCS(IP_CS_SIG_STOP_ALL_PROCESSES_REQ,pMsg,m_serviceId,
          m_serviceId,m_pDestUnitId,m_callIndex,pMcChannel->GetCsIndex(),pMcChannel->GetIndex(),0);
  POBJDELETE(pMsg);
}

////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::FindSiteAndVisualNamePlusProductIdAndSendToConfLevel(char* sCallConnectedDisplay)
{
  PTRACE(eLevelInfoNormal,"CH323Cntl::FindSiteAndVisualNamePlusProductIdAndSendToConfLevel ");
  eTelePresencePartyType  eLocalTelePresencePartyType = eTelePresencePartyNone;

  char* siteName = new char[MAX_SITE_NAME_ARR_SIZE]; //33
  memset(siteName,'\0',MAX_SITE_NAME_ARR_SIZE);
  CH323Alias* pAlias = NULL;

  if (IsValidStringUTF8(sCallConnectedDisplay,"NOT_EMPTY"))
  {
    strncpy(siteName, sCallConnectedDisplay, MAX_SITE_NAME_ARR_SIZE);
    siteName[MAX_SITE_NAME_ARR_SIZE - 1] = '\0';
  }
  else if (m_pH323NetSetup->GetH323PartyAlias())
  {
    const char* remoteAlias = m_pH323NetSetup->getBestH323PartyAlias();
    if(IsValidStringUTF8(remoteAlias,"NOT_EMPTY"))
    {
      strncpy(siteName, remoteAlias, MAX_SITE_NAME_ARR_SIZE);
      siteName[MAX_SITE_NAME_ARR_SIZE - 1] = '\0';
    }
    else siteName[0] = '\0';
  }
  else siteName[0] = '\0';

  //replace invalid characters in the name
  for (int i=0; H323InvalidDisplayChars[i] != '\0'; i++)
  {
    CSmallString::ReplaceChar(siteName, H323InvalidDisplayChars[i],'.');
  }

  BYTE bIsVisualName = TRUE;
  BYTE bIsProductId = TRUE;
  BYTE bIsVersionId = FALSE;

  int versionIdLen = H460_C_VerIdMaxSize + 1;
  ALLOCBUFFER(rmtVendorVersionId,H460_C_VerIdMaxSize + 1);// = new char[versionIdLen];
  memset(rmtVendorVersionId,'\0',H460_C_VerIdMaxSize + 1);//versionIdLen);
  strncpy(rmtVendorVersionId,m_remoteVendor.versionId,H460_C_VerIdMaxSize);

  //int Verlen  = strlen(m_remoteVendor.versionId);
  int Verlen  = strlen(rmtVendorVersionId);

  char* pVersionId = NULL;

  if(Verlen)
  {
    pVersionId = new char[Verlen+1];
    memset(pVersionId,'\0',Verlen+1);

    IdentifyVersionId(m_remoteVendor.versionId,&pVersionId,m_remoteVendor.productId,H460_C_VerIdMaxSize);
    if (pVersionId[0] != '\0')
    {
      bIsVersionId = TRUE;
      FPTRACE2(eLevelInfoNormal,"CH323Cntl::FindSiteAndVisualNamePlusProductIdAndSendToConfLevel : The VersionId is ",pVersionId );
      if(strstr((const char*)m_remoteVendor.versionId,"RPX"))
      {
         m_IsNeedToExtractInfoFromRtcpCname = FALSE;
           eLocalTelePresencePartyType = eTelePresencePartyRPX;
         PTRACE2(eLevelInfoNormal,"CH323Cntl::FindSiteAndVisualNamePlusProductIdAndSendToConfLevel - Identify party as RPX: Name - ",PARTYNAME);
      }
      if(strstr((const char*)m_remoteVendor.versionId,"FLEX"))
      {
         m_IsNeedToExtractInfoFromRtcpCname = TRUE; //N.A. - changed to true For later Maui detection from rtcp Cname
         eLocalTelePresencePartyType = eTelePresencePartyFlex;
         PTRACE2(eLevelInfoNormal,"CH323Cntl::FindSiteAndVisualNamePlusProductIdAndSendToConfLevel - Identify party as FLEX: Name - ",PARTYNAME);
      }
    }
  }
  else
  {
    bIsVersionId = FALSE;
  }

	char						   visual_partyName[MAX_SITE_NAME_ARR_SIZE] = {0};
	int ret =  HtmlToUtf8(&siteName, visual_partyName, strlen(siteName));
   if(1 == ret)
	m_pTaskApi->SendSiteAndVisualNamePlusProductIdToPartyControl(bIsVisualName, visual_partyName, bIsProductId, m_remoteVendor.productId,bIsVersionId,pVersionId, eLocalTelePresencePartyType,m_remoteVendor.isCopMcu);
   else
	  m_pTaskApi->SendSiteAndVisualNamePlusProductIdToPartyControl(bIsVisualName, siteName, bIsProductId, m_remoteVendor.productId,bIsVersionId,pVersionId, eLocalTelePresencePartyType,m_remoteVendor.isCopMcu);

    PDELETEA(siteName);
  POBJDELETE(pAlias);
  PDELETEA(pVersionId);
  PDELETEA(rmtVendorVersionId);

}

/////////////////////////////////////////////////////////////////////////////////////
//answer contentRateChanged to the conf:
void  CH323Cntl::SendEndChangeContentToConfLevel(EStat status)
{
  if (status != statOK)
    PTRACE(eLevelInfoNormal,"CH323Cntl::SendEndChangeContentToConfLevel - failure");

  // we create a temporary communication mode and set it's video bit rate (people + content)
  // to the values include tdm because these are the values we received from the conference level.
  // we also set the conference content rate even if this party is not the speaker
  // (and the real rate is zero).

  // copy the current mode and add the wanted rates to send with the failure message.
  // the target mode contains values in transmit too, and we don't want to send them to the party control
  // But the current mode contains values only in the receive
  CComModeH323 *pTmpModeH323 = new CComModeH323;
  *pTmpModeH323 = *m_pCurrentModeH323;
  // in EPC we set mode only if both content channels are connected
  if (m_isVideoContentOutgoingChannelConnected)
    pTmpModeH323->SetVideoBitRate(m_curConfContRate,cmCapTransmit,kRoleContentOrPresentation);
  if (m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) )
  {
    //if(m_pTargetModeH323->IsNewContentModeDiffersOnlyInRes((const CComModeH323&)(*m_pCurrentModeH323),cmCapReceive))
      // {
       //     pTmpModeH323->CopyMediaMode(*m_pTargetModeH323, cmCapVideo, cmCapReceive, kRolePresentation);
      // }
    pTmpModeH323->SetVideoBitRate(m_curConfContRate,cmCapReceive,kRoleContentOrPresentation);
  }

  // VNGR-8295
  // In case we are still in LPR mode and the rate was reduced to 64k - We need to update the mode with the correct rate
  // before the content - So the encoder will be updated.
  if (m_curPeopleRate == 640 && m_isLprModeOn)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::SendEndChangeContentToConfLevel - Setting people rate to 64k - LPR restrictions");
    pTmpModeH323->SetVideoBitRate(m_curPeopleRate,cmCapTransmit,kRolePeople);
  }

  //send to conf level the target mode that EP failed to reach.
  m_pTaskApi->SendEndChangeContentToConfLevel(pTmpModeH323, (int)status);// status converted to int to emphasize that enum can't be pass in by segments since its has different declaration between Psos and XP.

  //Because we lie to the conf about the rate of the content we should send to updateDb the exact rate that the
  //remote opened.
  //m_pTaskApi->Rmt323CommModeUpdateDB(m_pCurrentModeH323);

  POBJDELETE (pTmpModeH323);
}

///////////////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::UpdateNetSetUp(CH323NetSetup& pH323NetSetup)
{
  *m_pH323NetSetup = pH323NetSetup;
}

/////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::ECSDecideOnOpeningOutChannels()
{//we open an outgoing channel only if the incoming one wasn't disconnected
  BYTE bOpenChannel = FALSE;

  CChannel* pChannel = FindChannelInList(cmCapAudio, FALSE);
  if (pChannel && (pChannel->GetCsChannelState() == kConnectedState) && !m_isIncomingAudioHasDisconnectedOnce)
  {
    OpenOutgoingChannel(cmCapAudio);
    bOpenChannel = TRUE;
  }

  pChannel = FindChannelInList(cmCapVideo, FALSE);
  if (pChannel && (pChannel->GetCsChannelState() == kConnectedState))
  {
    OpenOutgoingChannel(cmCapVideo);
    bOpenChannel = TRUE;
  }

  pChannel = FindChannelInList(cmCapVideo, FALSE, kRoleContentOrPresentation);

  if (pChannel && (pChannel->GetCsChannelState() == kConnectedState) )
  {
    OpenOutgoingChannel(cmCapVideo, pChannel->GetRoleLabel());
    bOpenChannel = TRUE;
  }

  pChannel = FindChannelInList(cmCapData, FALSE);
  if (pChannel && (pChannel->GetCsChannelState() == kConnectedState))
  {
    OpenOutgoingChannel(cmCapData);
    bOpenChannel = TRUE;
  }
  return bOpenChannel;
}
/*
////////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323NotifyInd(CSegment* pParam)
{
  tmDsIndNotify *pNotify = NULL;
  pNotify = (tmDsIndNotify*)(pParam->GetPtr());

  ALLOCBUFFER(buf, 32);
  sprintf(buf,"%d", pNotify->rate);
  PTRACE2(eLevelInfoNormal,"CH323Cntl::OnNotifyInd - rate is %s", buf);
}
*/
////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
/* Highest Common Functions: */
/////////////////////////////////////////////////////////////////////////////////////

BYTE CH323Cntl::UpdateTargetMode(CComModeH323* pNewScm, cmCapDataType type, cmCapDirection direction, ERoleLabel eRole)
{
  if (type) //update only a specific type
  {
    if((eRole & kRoleContentOrPresentation))
    {
      DWORD contentRate =  m_pTargetModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
      pNewScm->SetVideoBitRate(contentRate, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
    }
    //protection:
    if (pNewScm->IsMediaOn(type, cmCapReceive, eRole))
      if (pNewScm->GetMediaBitRate(type, cmCapReceive, eRole) == 0)
      {
        PTRACE2INT(eLevelError, "CH323Cntl::UpdateTargetMode - the receive bitrate of the new mode is zero - ", m_pCsRsrcDesc->GetConnectionId());
        return FALSE;
      }
    if (pNewScm->IsMediaOn(type, cmCapTransmit, eRole))
      if (pNewScm->GetMediaBitRate(type, cmCapTransmit, eRole) == 0)
      {
        PTRACE2INT(eLevelError, "CH323Cntl::UpdateTargetMode - the transmit bitrate of the new mode is zero - ", m_pCsRsrcDesc->GetConnectionId());
        return FALSE;
      }

    //If the rate is ok => update
    if (direction & cmCapReceive )
    {
      m_pTargetModeH323->SetMediaMode(pNewScm->GetMediaMode(type, cmCapReceive, eRole), type, cmCapReceive, eRole, true);
//      if (pNewScm->GetConfMediaType() == eMixAvcSvc && (pNewScm->GetMediaType(cmCapVideo, cmCapReceive) != eSvcCapCode))
      const std::list <StreamDesc> streamsDescList = pNewScm->GetStreamsListForMediaMode(type, cmCapReceive, eRole);
      if (streamsDescList.size() > 0)
      {// copy streams information
          TRACEINTOFUNC << "mix_mode: Update streams in target SCM for media type = " << ::GetTypeStr(type);
          m_pTargetModeH323->SetStreamsListForMediaMode(streamsDescList, type, cmCapReceive, eRole);
          m_pTargetModeH323->Dump("CH323Cntl::UpdateTargetMode mix_mode: Update streams in target SCM", eLevelInfoNormal);
      }
    }
    if (direction & cmCapTransmit )
      m_pTargetModeH323->SetMediaMode(pNewScm->GetMediaMode(type, cmCapTransmit, eRole),type, cmCapTransmit, eRole, true);
  }
  else //update all
    *m_pTargetModeH323 = *pNewScm;

  if (pNewScm->GetContentProtocolMode() != eNoPresentation)
  {
    m_pTargetModeH323->SetTipContentMode(pNewScm->GetTipContentMode());
    m_pTargetModeH323->SetContentProtocolMode(pNewScm->GetContentProtocolMode());
  }

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OpenContentOutChannelFromMcms()
{
  PTRACE(eLevelInfoNormal, "CH323Cntl::OpenContentOutChannelFromMcms");
  if(!m_pTargetModeH323->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople))
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::OpenContentOutChannelFromMcms - Target is ON");
    if(m_pCurrentModeH323->IsMediaOff(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation))
    {
      PTRACE(eLevelInfoNormal, "CH323Cntl::OpenContentOutChannelFromMcms - Current is OFF");
      if(FindChannelInList(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation) == NULL)
      {
        PTRACE(eLevelInfoNormal, "CH323Cntl::OpenContentOutChannelFromMcms - No channel in list");
        CCapSetInfo capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation);

          ERoleLabel roleToOpen = kRolePresentation;
          if(m_pLocalCapH323->IsH239() && m_pRmtCapH323->IsH239())
            roleToOpen = kRolePresentation;
          else if(m_pLocalCapH323->IsEPC() && m_pRmtCapH323->IsEPC())
            roleToOpen = kRoleContent;

          OnPartyOutgoingChannelReq( capInfo.GetIpCapCode(), roleToOpen, TRUE);
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::UpdateLocalCapsFromTargetMode(BYTE bIsVideoCapEqualScm)
{
  PTRACE(eLevelInfoNormal, "CH323Cntl::UpdateLocalCapsFromTargetMode");
  m_pTargetModeH323->Dump("CH323Cntl::UpdateLocalCapsFromTargetMode - m_pTargetModeH323 - ",eLevelInfoNormal);

  // Re-Creating the Local capabilities
        CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  eVideoQuality vidQuality;
  if(pCommConf)
    vidQuality = pCommConf->GetVideoQuality();
  else
    PASSERT_AND_RETURN(NULL == pCommConf);

  BYTE isH263H621inLocalCAps = m_pLocalCapH323->IsFoundOrH263H261();
  BYTE is4cifEnabledinOriginalCapsTx = FALSE;
  BYTE is4cifEnabledinOriginalCapsRx = FALSE;
  if(isH263H621inLocalCAps && m_pLocalCapH323->Get4CifMpi() != -1 )
  {
    is4cifEnabledinOriginalCapsTx = TRUE;
    PTRACE(eLevelInfoNormal, "CH323Cntl::UpdateLocalCapsFromTargetMode - 4ciftr");
  }
  if( isH263H621inLocalCAps && m_pLocalCapH323->GetMpi(eH263CapCode,k4Cif) != ((APIU8)-1))
  {
    is4cifEnabledinOriginalCapsRx = TRUE;
    PTRACE(eLevelInfoNormal, "CH323Cntl::UpdateLocalCapsFromTargetMode - rec");
  }
  POBJDELETE( m_pLocalCapH323);
  m_pLocalCapH323 = new CCapH323;

    // create with default caps and enable H263 4cif according to video parameters
  DWORD initialVideoRate = m_pTargetModeH323->GetMediaBitRate(cmCapVideo, cmCapReceive);

  BYTE highestframerate = ((BYTE)eCopVideoFrameRate_None);
  if(m_pTargetModeH323->GetConfType() == kCop)
  {
    CVidModeH323* copHighetlevel= m_pCopVideoModes->GetVideoMode(0);
    WORD profile;
    BYTE level;
    long maxMBPS,maxFS,maxDPB,maxBR,maxSAR,maxStaticMbps,brandcpb;
    copHighetlevel->GetH264Scm(profile,level,maxMBPS,maxFS,maxDPB,maxSAR,brandcpb,maxStaticMbps);
    highestframerate = GetCopFrameRateAccordingtoMbpsAndFs(level,maxMBPS,maxFS);

   }
  m_pLocalCapH323->CreateWithDefaultVideoCaps(initialVideoRate, m_pTargetModeH323, PARTYNAME, vidQuality,FALSE,0/*service id*/,((ECopVideoFrameRate)highestframerate));


  //BRIDGE-15543 Our scm should contain real video rate but our caps should reflect the total call rate in the video caps.
  DWORD totalCallRate = m_pTargetModeH323->GetTotalBitRate(cmCapReceive) / 100;
  TRACEINTO << "Setting m_pLocalCapH323 video rate with totalCallRate: " << (int)totalCallRate;
  m_pLocalCapH323->SetVideoBitRate(totalCallRate);


  if(!isH263H621inLocalCAps && !bIsVideoCapEqualScm)
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::UpdateLocalCapsFromTargetMode - removeh263andh261");
    m_pLocalCapH323->RemovePeopleCapSet(eH263CapCode);
    m_pLocalCapH323->RemovePeopleCapSet(eH261CapCode);
  }
  if(!is4cifEnabledinOriginalCapsTx && !bIsVideoCapEqualScm)
  {
    m_pLocalCapH323->Set4CifMpi ((APIS8)-1);

  }
  if(!is4cifEnabledinOriginalCapsRx && !bIsVideoCapEqualScm)
    m_pLocalCapH323->SetH263FormatMpi(k4Cif, -1, kRolePeople);

  if (bIsVideoCapEqualScm)
  {
    PTRACE(eLevelInfoNormal, "CH323Cntl::UpdateLocalCapsFromTargetMode -CREATE EXACT CAPS");
    m_pLocalCapH323->SetVideoCapsExactlyAccordingToScm(m_pTargetModeH323);
           }
  else
  {
     if( m_pTargetModeH323->GetConfType() == kCp )
         m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
  }
}


/////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OpenVideoOutChannelFromMcms(BYTE bOpenWithoutCheck)
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OpenVideoOutChannelFromMcms - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());
  if(!m_pTargetModeH323->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople))
  {
    BYTE bOpen = bOpenWithoutCheck;
    if (bOpenWithoutCheck == FALSE)
    {//in case of cascade, we open only in 2 cases:
      if (m_remoteIdent == PolycomRMX || m_remoteIdent == PolycomMGC)
      {
        //case 1. This is the dial in: Same policy as in opening channels without highest common (dial in always open the out channel)
        if (!m_pmcCall->GetIsOrigin())
          bOpen = TRUE;
        //case 2. This is the dial out:
        else if( FindChannelInList(cmCapVideo,FALSE,kRolePeople))
        {
          CChannel* pIncomingChannel = FindChannelInList(cmCapVideo,FALSE,kRolePeople);

         if(pIncomingChannel)
         {
          channelSpecificParameters* pInChannelParams = (channelSpecificParameters *)(pIncomingChannel->GetChannelParams());
          CCapSetInfo capInfo = pIncomingChannel->GetCapNameEnum();
          //case 2.1: But the dial in is the change mode initiator: So in this case, dial in has already reopened its out channel (as a result of "case 1"), so there is no point in waiting to it to open
          if (IsSymmetricProtocol(capInfo, kRolePeople, pInChannelParams))
            bOpen = TRUE;
          //else:
          //case 2.1: Dial out is the change mode initiator: that means the dial in hasn't reopened the channel yet, so the dial in will open its out channel, as a result of "case 1"
        }


        }
      }
    }

    if (bOpen)
    {
      CCapSetInfo capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople);
      OnPartyOutgoingChannelReq( capInfo.GetIpCapCode(), kRolePeople, TRUE);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetH264Mbps(channelSpecificParameters* pChannelParams)
{
  APIS32 mbps = pChannelParams->p264.customMaxMbpsValue;
  if (mbps == -1)
  {
    APIU8 level = pChannelParams->p264.levelValue;
    CH264Details h264Details = level;
    mbps = h264Details.GetDefaultMbpsAsProduct();//in new channel mode, we send the product to the card
  }
  else
    mbps = ConvertMaxMbpsToProduct(mbps);//in new channel mode, we send the product to the card

  pChannelParams->p264.customMaxMbpsValue = mbps;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetVideoParamInCaps(H264VideoModeDetails h264VidModeDetails, BYTE cif4Mpi, DWORD videoRate)
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::SetVideoParamInCaps rate ", videoRate);
  h264VidModeDetails.profileValue = H264_Profile_None;//do not change profile

  if (m_pTargetModeH323->GetConfType() == kCp)
    m_pLocalCapH323->SetLevelAndAdditionals(h264VidModeDetails,H264_ALL_LEVEL_DEFAULT_SAR);
  else
    m_pLocalCapH323->SetLevelAndAdditionals(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR);

  m_pLocalCapH323->Set4CifMpi(cif4Mpi);
  if(cif4Mpi == ((APIU8)-1))
    m_pLocalCapH323->SetH263FormatMpi(k4Cif, -1, kRolePeople);

  m_pLocalCapH323->SetVideoBitRate(videoRate);

  BYTE bIsReCap = 1;
  OnPartyCreateControl(bIsReCap);
}

/////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetLocalCapToAudioOnly()
{
  // the function is still missing since it doesn't handle different types of remote vendors (like removing G7221C and all the
  // additional changes we are doing in function "ChangeCapsAccordingToRemoteVendor")
  PTRACE(eLevelInfoNormal, "CH323Cntl::SetLocalCapToAudioOnly");
  POBJDELETE(m_pLocalCapH323);
  m_pLocalCapH323 = new CCapH323;
  m_pLocalCapH323->CreateAudioOnlyCap(m_pParty->GetVideoRate(), m_pTargetModeH323, PARTYNAME);
    BYTE bIsReCap = 1;
  OnPartyCreateControl(bIsReCap);
}

////////////////////////////////////////////////////////////////////////////////////////////
WORD CH323Cntl::GetFeccMediaToDeclare(CapEnum &feccMedia)
{
  WORD numToDeclare = 0;
  //In case the in is open we declared the fecc cap code that open in the in
  if(m_pCurrentModeH323->IsMediaOn(cmCapData,cmCapReceive))
  {
    feccMedia = (CapEnum)m_pCurrentModeH323->GetMediaType(cmCapData,cmCapReceive);
    numToDeclare = 1;
    return numToDeclare;
  }
  else //The receive is close
  {
    if(m_pCurrentModeH323->IsMediaOn(cmCapData,cmCapTransmit))
    {
      //In case out is open and the in is close we want to close the outgoing channel and declare again
      //rv and annexQ FECC
      CloseOutgoingChannel(cmCapData);
      numToDeclare = 2;
    }
    else//the receive and the transmit are closed
    {
      //if the local caps there were fecc we want to declared both rv and annexQ.
      if(m_pLocalCapH323->IsFECC())
        numToDeclare = 2;
    }
  }
  return numToDeclare;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::BuildNewCapsFromNewTargetModeAndCaps(CCapH323* pCaps,const CCapH323* pNewLocalCaps)
{
  WORD confType = m_pParty->GetConfDualStreamMode();
  if (confType == confTypeH239)
  {   //1- if the second video channel is opened with role content and not presentation => treat it as an EPC call in the re-caps
    CChannel* pChannel = FindChannelInList(cmCapVideo, FALSE, kRoleContentOrPresentation);
    if (pChannel)
    {
      if (pChannel->GetRoleLabel() == (DWORD)kRoleContent)
        confType = confTypePPCVersion1;
    }
    //2 - If content channel wasn't opened before:
    else
    {// 2.1)In case we removed the 239 because the remote does not support 239 but it support EPC we should send only EPC.
/*      if(!m_pLocalCapH323->IsH239() && m_pLocalCapH323->IsEPC())
        confType = confTypePPCVersion1;
     // 2.2)In case we didn't remove any of them, we should declare according to remote caps
      else if (!m_pRmtCapH323->IsH239() && m_pRmtCapH323->IsEPC())
        confType = confTypePPCVersion1;*/
    }
  }

  CapEnum feccMedaiType = eUnknownAlgorithemCapCode;
  WORD numOfFecc = GetFeccMediaToDeclare(feccMedaiType);
  BYTE highestframerate = ((BYTE)eCopVideoFrameRate_None);
  if(m_pTargetModeH323->GetConfType() == kCop)
  {
    CVidModeH323* copHighetlevel= m_pCopVideoModes->GetVideoMode(0);
    WORD profile;
    BYTE level;
    long maxMBPS,maxFS,maxDPB,maxBR,maxSAR,maxStaticMbps,brandcpb;
    copHighetlevel->GetH264Scm(profile,level,maxMBPS,maxFS,maxDPB,maxSAR,brandcpb,maxStaticMbps);
    highestframerate = GetCopFrameRateAccordingtoMbpsAndFs(level,maxMBPS,maxFS);

     }
  pCaps->BuildNewCapsFromComModeAndCaps(*m_pTargetModeH323, confType,numOfFecc,feccMedaiType,pNewLocalCaps,m_serviceId,((ECopVideoFrameRate)highestframerate));

}

/////////////////////////////////////////////////////////////////////////////////////////////
WORD CH323Cntl::GetChangeFromNewScmMode(CComModeH323* pNewScm,cmCapDirection direction,cmCapDataType dataType,ERoleLabel eRole) const
{
  WORD differentValue = 0;
  differentValue = m_pTargetModeH323->GetChangeFromNewScmMode(pNewScm,direction,dataType,eRole);

  return differentValue;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::OnH323StreamStatusInd(CSegment* pParam)
{
  if( m_pmcCall->GetIsClosingProcess() == TRUE)
  { // if the call is in closing process no need to send fast update.
    PTRACE2INT(eLevelError,"CH323Cntl::OnH323StreamStatusInd bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }

  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnH323StreamStatusInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  TRtpStreamStatusInd rtpStreamStatusInd;
  DWORD  structLen = sizeof(TRtpStreamStatusInd);
  memset(&rtpStreamStatusInd,0,structLen);
  pParam->Get((BYTE*)(&rtpStreamStatusInd),structLen);

  CChannel* pChannel = NULL;
  BYTE bReportOnViolation   = FALSE;
  BYTE bFoundLightViolation = FALSE;
  ERoleLabel eRole;
  CSecondaryParams secParams;
  BOOL isTransmitted = FALSE;
  if (rtpStreamStatusInd.unChannelDirection == cmCapTransmit)
    isTransmitted = TRUE;
  pChannel = m_pmcCall->FindChannelInList(::ChannelTypeToDataType((kChanneltype)rtpStreamStatusInd.unChannelType,eRole), isTransmitted, eRole);
  if (!pChannel)
  {
    PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnH323StreamStatusInd - Channel wasn't found - ",m_pCsRsrcDesc->GetConnectionId());
    DBGPASSERT(rtpStreamStatusInd.unChannelType);
    return;
  }

  CMedString cLog;
  cLog << "party " << PARTYNAME << ", channel type " << rtpStreamStatusInd.unChannelType << ", channel direction " << rtpStreamStatusInd.unChannelDirection;

  const CVidModeH323 & vidMode = (const CVidModeH323 &)m_pTargetModeH323->GetMediaMode(cmCapVideo,cmCapReceive);

  if (rtpStreamStatusInd.unStreamStatus != 0)
  {
    PTRACE2INT(eLevelError, "CH323Cntl::OnH323StreamStatusInd - Stream status isn't ok - ",m_pCsRsrcDesc->GetConnectionId());

    if (pChannel->GetCsChannelState() != kConnectedState)
    {
      PTRACE2(eLevelInfoNormal, "CH323Cntl::OnH323StreamStatusInd - Channel isn't in connected state - Ignore the violation: Name - ",PARTYNAME);
      return;
    }

    if (pChannel->GetRoleLabel() & kRoleContentOrPresentation)
    {
      PTRACE2(eLevelError, "CH323Cntl::OnH323StreamStatusInd - Do not support content channel: Name - ",PARTYNAME);
      DBGPASSERT(m_pmcCall->GetConnectionId());
      return;
    }

    if (rtpStreamStatusInd.unResolution)
    {
      if (pChannel->GetPayloadType() == _H264)
      {
        APIS32 mbpsMask = 0xFFFF00;//the second and third bytes from the left
        APIS32 mbps = rtpStreamStatusInd.bunViolation & rtpStreamStatusInd.unFramesPerSec & rtpStreamStatusInd.unResolution & rtpStreamStatusInd.unPayloadType & mbpsMask;
        mbps =  mbps >> 8; //1 byte to the right
        if (mbps)
        {
          bFoundLightViolation = TRUE; //regarding to violation in mbps depends on a flag
          cLog << "Remote transmitted resolution is " << rtpStreamStatusInd.unResolution << ", signaling resolution is " <<  vidMode.GetFormat();
          PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323StreamStatusInd: ",cLog.GetString());
          TRACEINTO << "CH323Cntl::OnH323StreamStatusInd - The MBPS isn't ok " << mbps;

        }
      }
      else
      {
        APIS32 payloadTypeMask = 0xFF;   //the first byte from the right
        APIS32 payloadType = rtpStreamStatusInd.bunViolation & rtpStreamStatusInd.unFramesPerSec & rtpStreamStatusInd.unResolution & rtpStreamStatusInd.unPayloadType & payloadTypeMask;
        if( (!bFoundLightViolation && !bReportOnViolation) && payloadType )
        {
          if(secParams.m_problemParam == 0)//In case there was no other initialize
          {
            secParams.m_problemParam    = PayloadType;
            secParams.m_rmtProblemValue = payloadType;
            secParams.m_currProblemValue  = pChannel->GetPayloadType();
          }

          bReportOnViolation = TRUE; //not depending on any flag
          PTRACE2INT(eLevelError, "CH323Cntl::OnH323StreamStatusInd - The payload type isn't ok : ", payloadType);
        }

        APIS32 resolutionMask = 0xFF00;   //the second byte from the right
        APIS32 resolution = rtpStreamStatusInd.bunViolation & rtpStreamStatusInd.unFramesPerSec & rtpStreamStatusInd.unResolution & rtpStreamStatusInd.unPayloadType & resolutionMask;
        resolution = resolution >> 8; //1 byte to the right
        if (resolution)
        {

          bFoundLightViolation = TRUE; //regarding to violation in resolution depends on a flag
          cLog << "Remote transmitted resolution is " << rtpStreamStatusInd.unResolution << ", signaling resolution is " <<  vidMode.GetFormat();
          PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323StreamStatusInd: ",cLog.GetString());
            PTRACE(eLevelError, "CH323Cntl::OnH323StreamStatusInd - The format isn't ok : Unknown");
        }

        APIS32 frameRateMask  = 0xFF0000; //the second byte from the left
        APIS32 frameRate = rtpStreamStatusInd.bunViolation & rtpStreamStatusInd.unFramesPerSec & rtpStreamStatusInd.unResolution & rtpStreamStatusInd.unPayloadType & frameRateMask;
        frameRate = frameRate >> 16; //2 bytes to the right
        if (frameRate)
        {
          bFoundLightViolation = TRUE; //regarding to violation in fps depends on a flag
          cLog << "Remote transmitted FPS is " << rtpStreamStatusInd.unFramesPerSec << ", signaling FPS is " <<  vidMode.GetFormatMpi(vidMode.GetFormat());
          PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323StreamStatusInd: ",cLog.GetString());
          TRACEINTO <<  "CH323Cntl::OnH323StreamStatusInd - The frame rate isn't ok : " << frameRate;
        }
      }
    }
    if( (!bFoundLightViolation && !bReportOnViolation) && rtpStreamStatusInd.unBitRate )
    {
      bFoundLightViolation = TRUE; //regarding to violation in rate depends on a flag
      cLog << "Remote transmitted rate is " << rtpStreamStatusInd.unBitRate << ", signaling rate is " <<  vidMode.GetBitRate();
      PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323StreamStatusInd: ",cLog.GetString());
      TRACEINTO << "CH323Cntl::OnH323StreamStatusInd - The bit rate isn't ok : ";
    }
    if( (!bFoundLightViolation && !bReportOnViolation) && rtpStreamStatusInd.unAnnexesAndResolutionMask )//not depending on any flag
    {
      DWORD errorNo = rtpStreamStatusInd.unAnnexesAndResolutionMask & (H263_Annexes_Number-1);// check the number of annexes
      if(errorNo)// there is an error in the annexes
      {
        secParams.m_problemParam    = Annexes;
        CH263VideoCap* pCap = (CH263VideoCap*) CBaseCap::AllocNewCap(eH263CapCode, NULL);

        bReportOnViolation      = TRUE;
        cLog << "The annexes aren't ok";
        PTRACE2(eLevelInfoNormal, "CSipCntl::OnH323StreamStatusInd - ", cLog.GetString());
        if(pCap) //In case there was no other initialize
        {
          secParams.m_rmtProblemValue = pCap->GetAnnex(rtpStreamStatusInd.unAnnexesAndResolutionMask);
          pCap->FreeStruct();
        }
        POBJDELETE(pCap);
      }
    }
    else if(rtpStreamStatusInd.unAnnexesAndResolutionMask)// Custom - monitoring only
    {
      cLog << "Remote transmitted custom format is " << (rtpStreamStatusInd.unAnnexesAndResolutionMask & 0xFFFFFFEA) << ", signaling custome format is NONE";
      PTRACE2(eLevelInfoNormal,"CH323Cntl::OnH323StreamStatusInd: ",cLog.GetString());
    }

    if (bFoundLightViolation && !bReportOnViolation)
    {
      if (m_pParty->IsPartyInChangeVideoMode() )
      {
        if ( GetSystemCfgFlagInt<BOOL>(CFG_KEY_IGNORE_STREAM_VIOLATION_IN_CHANGE_MODE) == FALSE)
          bReportOnViolation = TRUE;
        else
          PTRACE(eLevelInfoNormal, "CH323Cntl::OnH323StreamStatusInd - The flag IGNORE_STREAM_VIOLATION_IN_CHANGE_MODE is on");
      }
      else
      {
        if ( GetSystemCfgFlagInt<BOOL>(CFG_KEY_IGNORE_STREAM_VIOLATION) == FALSE)
          bReportOnViolation = TRUE;
        else
          PTRACE(eLevelInfoNormal, "CH323Cntl::OnH323StreamStatusInd - The flag IGNORE_STREAM_VIOLATION is on");
      }
    }

    rtpStreamStatusInd.unStreamStatus = bReportOnViolation;
  }

  if (bReportOnViolation || m_pParty->IsPartyInChangeVideoMode())
    m_pTaskApi->SendIpStreamViolation(rtpStreamStatusInd.unStreamStatus,SECONDARY_CAUSE_STREAM_VIOLATION,secParams);  //inform the H323Part
}

////////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnTimerRopenContentIn(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnTimerRopenContentIn - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());

  if (m_bContentInClosedWhileChangeVidMode && m_pCurrentModeH323->IsMediaOff(cmCapVideo,cmCapReceive,kRoleContentOrPresentation))
    m_pTaskApi->SendCloseChannelToConfLevel(cmCapVideo, cmCapReceive, kRoleContent);//bla bla
  m_bContentInClosedWhileChangeVidMode = FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD CH323Cntl::GetConnectionId() const
{
  return m_pmcCall->GetConnectionId();
}

////////////////////////////////////////////////////////////////////////////
BOOL CH323Cntl::CheckCascadeParams()
{
  BOOL bIsGoodParams = TRUE;
    CLargeString str;

    str << "CH323Cntl::CheckCascadeParams \nm_pmcCall->GetMasterSlaveStatus() = ";
    if (m_pmcCall->GetMasterSlaveStatus() == (int)cmMSMaster)
        str << "MASTER";
    else
        str << "SLAVE";

    if(m_pParty->GetCascadeMode() == MASTER)
        str << " \nm_pParty->GetCascadeMode() = MASTER";
    else
        str << " \nm_pParty->GetCascadeMode() = SLAVE";
    str << " \nm_pmcCall->GetRmtType() = " << m_pmcCall->GetRmtType();
    //PTRACE (eLevelInfoNormal, str.GetString());

  if(m_pmcCall->GetRmtType() == (DWORD)cmEndpointTypeMCU ||
       m_pmcCall->GetRmtType() == (DWORD)cmEndpointTypeGateway)
  {
    if (m_pmcCall->GetMasterSlaveStatus() == (int)cmMSMaster)
    {
      if(m_pParty->GetCascadeMode() == MASTER)
        bIsGoodParams = TRUE;
      else
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::EndConnectToChairCntl. MCU should be SLAVE");
        bIsGoodParams = FALSE;
      }
    }
    else if(m_pmcCall->GetMasterSlaveStatus() == (int)cmMSSlave)
    {
      if(m_pParty->GetCascadeMode() == SLAVE)
        bIsGoodParams = TRUE;
      else
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::EndConnectToChairCntl. MCU should be MASTER");
        bIsGoodParams = FALSE;
      }
    }
    else //cmMSError
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::EndConnectToChairCntl. No master slave type. Reject the call");
      bIsGoodParams = FALSE;
    }
  }
  return bIsGoodParams;
}
/*
////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendCapabilitiesAck(INT32 lastSequenceNumber)
//{

  PTRACE(eLevelInfoNormal,"CH323Cntl::SendCapabilitiesAck");
  H323_MCREQ_CAPABILITIES_IND_RESP_S * pCapabilitiesAckReq = new H323_MCREQ_CAPABILITIES_IND_RESP_S;

  pCapabilitiesAckReq->header.pmHeader.status = STATUS_OK;
  pCapabilitiesAckReq->SequenceNumber = lastSequenceNumber;
  m_pH323->H323McmsReq((BYTE*)pCapabilitiesAckReq, H323_CAPABILITIES_IND_RESPONSE_REQ,
            EMBDPARAMSIZE(H323_MCREQ_CAPABILITIES_IND_RESP_S));
  PDELETE(pCapabilitiesAckReq);
//}

*/
////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnChannelsECSTimer()
{
  if (m_remoteCapIndNotHandle)
  {
    PTRACE (eLevelInfoNormal, "CH323Cntl::OnChannelsECSTimer - Channels failed to close - handle ECS now!");
    ConnectPartyToConf();
    if (m_remoteCapIndNotHandle)
    {
      HandleCapIndication(m_bPrevCapsAreFull,m_bPrevCapsHaveAudio, TRUE);
      m_remoteCapIndNotHandle = FALSE;
    }
  }
  else
  {
    PTRACE (eLevelError, "CH323Cntl::OnChannelsECSTimer - ECS was handled but timer jumped anyway!");
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetEncryptionInStructToZero(encTokensHeaderStruct& encryTokens)
{
  encryTokens.numberOfTokens  = 0;
  encryTokens.dynamicTokensLen = 0;
  encryTokens.xmlDynamicProps.numberOfDynamicParts = 0;
  encryTokens.xmlDynamicProps.sizeOfAllDynamicParts = 0;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetConnectionIdForReject(DWORD DinRejectConnectionId)
{

  PTRACE(eLevelInfoNormal,"CH323Cntl::SetConnectionIdForReject ");
  POBJDELETE(m_pCsRsrcDesc);
  m_pCsRsrcDesc = new CRsrcParams(DinRejectConnectionId,  DinRejectConnectionId,
                  DUMMY_CONF_ID, eLogical_ip_signaling);

  CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
  if ( pRoutingTbl== NULL )
  {
    PASSERT_AND_RETURN(GetConnectionId());
  }
  if (CPObject::IsValidPObjectPtr(m_pTaskApi))
  {
//    if(CPObject::IsValidPObjectPtr(m_pTaskApi->m_pStateMachine))
//    {
      CPartyRsrcRoutingTblKey routingKey = CPartyRsrcRoutingTblKey(DinRejectConnectionId, DinRejectConnectionId, eLogical_ip_signaling);
      pRoutingTbl->AddPartyRsrcDesc(routingKey);

      WORD status = pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pCsRsrcDesc, m_pTaskApi);
      if (status != STATUS_OK)
        DBGPASSERT(status);
      m_pmcCall->SetConnectionId(DinRejectConnectionId);
      m_pCsInterface->Create(m_pCsRsrcDesc);
//    }
//    else
//      DBGPASSERT(2);
  }
  else
    PASSERTMSG(GetConnectionId(),"CH323Cntl::SetConnectionIdForReject - m_pTaskApi not valid");
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetMcCallNetSetupParams()
{
  m_pmcCall->SetMaxRate(m_pH323NetSetup->GetMaxRate());
  m_pmcCall->SetMinRate(m_pH323NetSetup->GetMinRate());

  const char* srcparty = m_pH323NetSetup->GetSrcPartyAddress();
  if(strlen(srcparty) < MaxAddressListSize)
    m_pmcCall->SetSrcTerminalPartyAddr(srcparty,strlen(srcparty));
  else
  {
    m_pmcCall->SetSrcTerminalPartyAddr("'\0'",1);
    PASSERTMSG(strlen(srcparty),"CH323Cntl::SetMcCallNetSetupParams - strlen(srcparty) > MaxAddressListSize");
  }

  const char* dstparty = m_pH323NetSetup->GetDestPartyAddress();
  if(strlen(dstparty) < MaxAddressListSize)
    m_pmcCall->SetDestTerminalPartyAddr(dstparty,strlen(dstparty));

  else
  {
    m_pmcCall->SetDestTerminalPartyAddr("'\0'",1);
    PASSERTMSG(strlen(dstparty),"CH323Cntl::SetMcCallNetSetupParams - strlen(dstparty) > MaxAddressListSize");
  }

  m_pmcCall->SetChannelsCounter(0);
  m_pmcCall->SetReferenceValueForEp(0); //the stack allocates this value
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetConfType(EConfType confType)
{
  m_pCurrentModeH323->SetConfType(confType);
}

///////////////////////////////////////////////////////////////////////////////////
//we can sent CALL_DROP_REQ to CS only after we got ACK_IND on CONFPARTY_CM_CLOSE_UDP_PORT_REQ for A-L-L the channels
void CH323Cntl::SendCallDropIfNeeded()
{
    TRACEINTO << "mix_mode: num of external channels: " << m_pmcCall->GetChannelsCounter() << " num of internal channels: " << m_pmcCall->GetNumOfInternalChannels();
  BYTE bSend = TRUE;
  CChannel* pChannel;
  for(int i=0; (i < m_maxCallChannel) && bSend; i++)
  {
    pChannel = m_pmcCall->GetSpecificChannel(i);
    if (pChannel)
    {
      if ( (pChannel->GetCsChannelState() == kWaitToSendChannelDrop) || (pChannel->GetCmUdpChannelState() == kSendClose) )
        bSend = FALSE;
    }
  }
  if (bSend)
  {// check internal channels
      if (m_pmcCall->GetNumOfInternalChannels() > 0)
      {
          TRACEINTO << "mix_mode: Waiting to close internal channels... number of still open channels=" << m_pmcCall->GetNumOfInternalChannels();
          bSend = FALSE;
      }
  }
  if (bSend)
  {
      TRACEINTO << "SendCallDrop";
    SendCallDrop();
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyCsErrHandleKeepAliveFirstTout(CSegment* pParam)
{
  //PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyCsErrHandleKeepAliveFirstTout - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());
  // Sending the request
  m_pCsInterface->SendMsgToCS(H323_CS_PARTY_KEEP_ALIVE_REQ,NULL,m_serviceId,
                m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);

  if (m_isKeepAliveIndArrived == 1)
  {
    //PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyCsErrHandleKeepAliveFirstTout : Begin again - Conn Id =  ",m_pCsRsrcDesc->GetConnectionId());
    m_isKeepAliveIndArrived = 0;
  }

  if (m_keepAliveTimerCouter == 0)
  {   // The first time the 35 second timer jumped
    PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyCsErrHandleKeepAliveFirstTout : First time - Conn Id =  ",m_pCsRsrcDesc->GetConnectionId());
    m_keepAliveTimerCouter++;
    DWORD partyKeepAliveFirstTimerVal = GetSystemCfgFlagInt<DWORD>(CFG_KEY_H323_CS_ERROR_HANDLE_FIRST_TIMER_VAL);
    StartTimer(PARTYCSKEEPALIVEFIRSTTOUT,partyKeepAliveFirstTimerVal*SECOND);
  }
  else if (m_keepAliveTimerCouter == 1)
  {
    PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyCsErrHandleKeepAliveFirstTout : Second time - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    m_keepAliveTimerCouter++;
    DWORD partyKeepAliveSecondTimerVal = GetSystemCfgFlagInt<DWORD>(CFG_KEY_H323_CS_ERROR_HANDLE_SECOND_TIMER_VAL);
    StartTimer(PARTYCSKEEPALIVESECONDTOUT,partyKeepAliveSecondTimerVal*SECOND);
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyCsErrHandleKeepAliveSecondTout(CSegment* pParam)
{
  BOOL isDebugMode = FALSE;
  CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, isDebugMode);

  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyCsErrHandleKeepAliveSecondTout - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  // In this case we will disconnect the party regardelss to whatever stage it's in.
  // This means that the CS party Call task is dead.

  if (m_keepAliveTimerCouter == 0 || isDebugMode)
  {   // we receive keep alive indication during the second timer start the loop again
    if(m_keepAliveTimerCouter == 0)
      PTRACE(eLevelInfoNormal, "CH323Cntl::OnPartyCsErrHandleKeepAliveSecondTout : Start the loop again");
    else
      PTRACE(eLevelInfoNormal, "CH323Cntl::OnPartyCsErrHandleKeepAliveSecondTout : Debug mode, ignore the keep alive error.");
    // Sending the request
    m_pCsInterface->SendMsgToCS(H323_CS_PARTY_KEEP_ALIVE_REQ,NULL,m_serviceId,
            m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
    m_keepAliveTimerCouter++;
    DWORD partyKeepAliveFirstTimerVal = GetSystemCfgFlagInt<DWORD>(CFG_KEY_H323_CS_ERROR_HANDLE_FIRST_TIMER_VAL);
    StartTimer(PARTYCSKEEPALIVEFIRSTTOUT,partyKeepAliveFirstTimerVal*SECOND);
    return;
  }

  if (m_keepAliveTimerCouter != 2)
  {
    DBGPASSERT(m_keepAliveTimerCouter);
  }
  // Need to have a meeting on all disconnect reasons.
  m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_BY_MCU);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyCsErrHandleKeepAliveInd(CSegment* pParam)
{

  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  // only stat1 srcUnitId are valid values at this place.
  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;
  status = (APIS32)stat1;

  if (status == -1)
  {
    PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyCsErrHandleKeepAliveInd : Status = -1 - ",m_pCsRsrcDesc->GetConnectionId());
    DBGPASSERT(m_pCsRsrcDesc->GetConnectionId());
    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_BY_MCU);
  }
  else if (status == -2)
  {
    PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyCsErrHandleKeepAliveInd : Status = -2 - ",m_pCsRsrcDesc->GetConnectionId());
    DBGPASSERT(m_pCsRsrcDesc->GetConnectionId());
    m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_BY_MCU);
  }
  else
  {
    // Renew the timer
    m_keepAliveTimerCouter  = 0;
    m_isKeepAliveIndArrived = 1;
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::StartCsPartyErrHandlingLoop()
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::StartCsPartyErrHandlingLoop - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  m_pCsInterface->SendMsgToCS(H323_CS_PARTY_KEEP_ALIVE_REQ,NULL,m_serviceId,
            m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
  // Renew the timer
  m_keepAliveTimerCouter=0;
  DWORD partyKeepAliveFirstTimerVal = GetSystemCfgFlagInt<DWORD>(CFG_KEY_H323_CS_ERROR_HANDLE_FIRST_TIMER_VAL);
  StartTimer(PARTYCSKEEPALIVEFIRSTTOUT,partyKeepAliveFirstTimerVal*SECOND);
}

char * CH323Cntl::GetRejectReasonAsString(APIS32 rjReason)
{
    rejectReasonChannel eRjRejectReason = (rejectReasonChannel)rjReason;

    switch(rjReason)
    {
      case rjNoReject:
                return "rjNoReject";
            case rjAllocatePortFailed:
                return "rjAllocatePortFailed";
            case rjAllocateEntryFailed:
                return "rjAllocateEntryFailed";
            case rjGetPartitionFailed:
                return "rjGetPartitionFailed";
            case rjChannelNewFailed:
                return "rjChannelNewFailed";
            case rjExtraChannel:
                return "rjExtraChannel";
            case rjInvalidProtocolType:
                return "rjInvalidProtocolType";
            case rjUnexpectedChannel:
                return "rjUnexpectedChannel";
            case rjNoChannelType:
                return "rjNoChannelType";
            case rjUnkownPaylodType:
                return "rjUnkownPaylodType";
            case rjCallAlreadyDisconnected:
                return "rjCallAlreadyDisconnected";
            case rjChanAlreadyDisconnected:
                return "rjChanAlreadyDisconnected";
             case rjOLCRejectByRemote:
                return "rjOLCRejectByRemote";
            case rjTMBadIndication:
                return "rjTMBadIndication";
            default:
                return "Unknown Reject Reason!";
        }

}

////////////////////////////////////////////////////////////////////////////
/************************************************************************/
/*        MFA     &      CM       Functions                       */
/************************************************************************/
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
/*                   MFA                          */
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::ShouldInitTimerForSendMsg(DWORD opcode)
{
    BYTE bInitTimer = ( (opcode == H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ) ||
              (opcode == H323_RTP_UPDATE_CHANNEL_REQ) ||
              (opcode == CONFPARTY_CM_OPEN_UDP_PORT_REQ)  ||
              (opcode == CONFPARTY_CM_CLOSE_UDP_PORT_REQ) ||
              (opcode == CONF_PARTY_MRMP_OPEN_CHANNEL_REQ) ||
              (opcode == CONF_PARTY_MRMP_UPDATE_CHANNEL_REQ) ||
              (opcode == CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ) ||
              (opcode == TB_MSG_OPEN_PORT_REQ) ||
              (opcode == TB_MSG_CLOSE_PORT_REQ)

               );

    if (bInitTimer)
    {
        return TRUE;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ
BYTE CH323Cntl::Rtp_FillAndSendUpdatePortOpenRtpStruct(CChannel *pMcChannel, BYTE isUpdate, BOOL isInternal, BYTE index)
{
  DWORD seqNum=0;
  DWORD connId = 0;
  if (isInternal)
      connId = GetAvcToSvcArtConnectionId(index);
  else
      connId = m_pMfaInterface->GetConnectionId();
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::Rtp_FillAndSendUpdatePortOpenRtpStruct - ", connId);
  CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

  TUpdatePortOpenRtpChannelReq* pStruct = new TUpdatePortOpenRtpChannelReq;
  memset(pStruct,0,sizeof(TUpdatePortOpenRtpChannelReq));
  pStruct->bunIsRecovery      = FALSE;
  pStruct->unChannelType      = ::DataTypeToChannelType(pMcChannel->GetType(),pMcChannel->GetRoleLabel());
  pStruct->unChannelDirection = ::CalcCmCapDirection(pMcChannel->IsOutgoingDirection());
  pStruct->unCapTypeCode      = (APIU32)pMcChannel->GetCapNameEnum();
  pStruct->unEncryptionType   = pMcChannel->GetEncryptionType();

  pStruct->updateSsrcParams.unUpdatedSSRC		= 0;
  pStruct->updateSsrcParams.bReplaceSSRC		= FALSE;
  pStruct->updateSsrcParams.bDoNotChangeSsrc	= FALSE;
  pStruct->mediaMode = eMediaModeTranscoding;
  if (GetTargetMode()->GetConfMediaType() == eMixAvcSvc &&
		  pStruct->unChannelType == kIpVideoChnlType &&
		  pStruct->unChannelDirection == cmCapReceive)
  {
	if (GetTargetMode()->IsHdVswInMixMode())
		pStruct->mediaMode = eMediaModeTranscodingAndVsw;
  }


  memset(&pStruct->sdesCap, 0, sizeof(sdesCapSt));

  if (isInternal)
  {
      // get the SSRC from the channel
      int i=0;
      const std::list <StreamDesc> streamsDescList = pMcChannel->GetStreams();
      std::list <StreamDesc>::const_iterator itr_streams;

      for(itr_streams=streamsDescList.begin(); itr_streams!=streamsDescList.end(); itr_streams++)
      {
          pStruct->updateSsrcParams.unUpdatedSSRC = itr_streams->m_pipeIdSsrc;
          pStruct->updateSsrcParams.bReplaceSSRC = TRUE;
          i++;
      }

      // sanity check
      if (i > 1)
      {
          pMcChannel->Dump("mix_mode: Something is wrong - more than one SSRC for the channel");
      }
  }

  if (pMcChannel->GetIsEncrypted())
  {
    if (m_pmcCall->GetMasterSlaveStatus() == cmMSMaster)
    {
      PTRACE2INT(eLevelInfoNormal, "CH323Cntl::Rtp_FillAndSendUpdatePortOpenRtpStruct - Encryption Master ",m_pMfaInterface->GetConnectionId());
      if ( !pMcChannel->IsOutgoingDirection())
      { // Incoming channel:
        // In this case we will create the session and encrypted session key with
        // Alg functions using the shared secret(Master key)
        APIU8 sessionKey[sizeOf128Key];
        APIU8 encSessionKey[sizeOf128Key];
        memset(sessionKey,'0',sizeOf128Key);
        memset(encSessionKey,'0',sizeOf128Key);


        BOOL isOpenSSLFunc = NO;
        CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_OPENSSL_ENC_FUNC, isOpenSSLFunc);
        DWORD fips140Status = STATUS_OK;
        if (isOpenSSLFunc != NO)
          fips140Status = CreateCipherKeyOpenSSL(m_pDHKeyManagement->GetEncrCallKey()->GetArray(),sessionKey,encSessionKey);
        else
          fips140Status = CreateCipherKey(m_pDHKeyManagement->GetEncrCallKey()->GetArray(), sessionKey, encSessionKey);

        if(fips140Status)
        {
          PDELETE(pStruct);

          CMedString errorString;
          errorString << "CH323Cntl::Rtp_FillAndSendUpdatePortOpenRtpStruct: FIPS140 Test Failure - Disconnect the call!\n";

          ALLOCBUFFER(errStr,128);
          ERR_load_crypto_strings();
          ERR_load_FIPS_strings();
          ERR_error_string_n(ERR_get_error(),errStr,128);
          errorString << errStr;

          PTRACE(eLevelInfoNormal,errorString.GetString());

          DEALLOCBUFFER(errStr);

          return FALSE;
        }
        // ===== 1. get the unSimulationErrCode from SysConfig
        std::string eSimValue;
        pSysConfig->GetDataByKey(CFG_KEY_FIPS140_SIMULATE_CONFPARTY_PROCESS_ERROR, eSimValue);
        eConfPartyFipsSimulationMode fips140SimulationConfPartyError = eInactiveSimulation;
        fips140SimulationConfPartyError = ::TranslateSysConfigDataToEnumForConfParty(eSimValue);

        if(fips140SimulationConfPartyError == eFailPartyCipherFipsTest)
        {
          PDELETE(pStruct);
          PTRACE(eLevelError,"CH323Cntl::Rtp_FillAndSendUpdatePortOpenRtpStruct: simulate TEST_FAILED - Disconnect the call!");
          return FALSE;
        }

        pMcChannel->SetH235SessionKey(sessionKey);
        pMcChannel->SetH235EncryptedSessionKey(encSessionKey);
        memcpy(&(pStruct->aucSessionKey),sessionKey , sizeOf128Key);
      }
      else
        memcpy(&(pStruct->aucSessionKey),pMcChannel->GetH235SessionKey(),sizeOf128Key);
    }
    else
    {
      // We are slaves
      PTRACE2INT(eLevelInfoNormal, "CH323Cntl::Rtp_FillAndSendUpdatePortOpenRtpStruct - Encryption Slave",m_pMfaInterface->GetConnectionId());
      memcpy(&(pStruct->aucSessionKey),pMcChannel->GetH235SessionKey() , sizeOf128Key);
    }
  }
  else
    memset(&(pStruct->aucSessionKey),'0',sizeOf128Key);

  pStruct->unSequenceNumber   = 0;  //EYAL???
  pStruct->unTimeStamp        = 0;  //EYAL???
  pStruct->unSyncSource       = 0;  //EYAL???

  Rtp_FillUpdatePortOpenRtpChannelStruct(&pStruct->tUpdateRtpSpecificChannelParams, pMcChannel);

  // LPR structure
  if ( pMcChannel->GetIsLprSupported())
  {
    lprCapCallStruct* plpCallStr = NULL;
    if (pStruct->unChannelDirection == cmCapTransmit)
      plpCallStr = m_pmcCall->GetLprCapStruct(0);
    else
      plpCallStr = m_pmcCall->GetLprCapStruct(1);

    pStruct->tLprSpecificParams.bunLprEnabled = 1;
    pStruct->tLprSpecificParams.unVersionID = plpCallStr->versionID;
    pStruct->tLprSpecificParams.unMinProtectionPeriod = plpCallStr->minProtectionPeriod;
    pStruct->tLprSpecificParams.unMaxProtectionPeriod = plpCallStr->maxProtectionPeriod;
    pStruct->tLprSpecificParams.unMaxRecoverySet = plpCallStr->maxRecoverySet;
    pStruct->tLprSpecificParams.unMaxRecoveryPackets = plpCallStr->maxRecoveryPackets;
    pStruct->tLprSpecificParams.unMaxPacketSize = plpCallStr->maxPacketSize;
  }
  else
  {
    pStruct->tLprSpecificParams.bunLprEnabled = 0;
    pStruct->tLprSpecificParams.unVersionID = 0;
    pStruct->tLprSpecificParams.unMinProtectionPeriod = 0;
    pStruct->tLprSpecificParams.unMaxProtectionPeriod = 0;
    pStruct->tLprSpecificParams.unMaxRecoverySet = 0;
    pStruct->tLprSpecificParams.unMaxRecoveryPackets = 0;
    pStruct->tLprSpecificParams.unMaxPacketSize = 0;

  }
  // Updating RTP channel state
  if (isUpdate)
    pMcChannel->SetRtpPortChannelState(kRtpPortUpdateSent);
  else
    pMcChannel->SetRtpPortChannelState(kRtpPortOpenSent);

  pStruct->useRtcp = m_useRtcp;

  TRACEINTO << "pMcChannel rtpPortChannelState = " << pMcChannel->GetRtpPortChannelState();
  pMcChannel->Dump("CH323Cntl::Rtp_FillAndSendUpdatePortOpenRtpStruct ");

  seqNum = SendMsgToMpl((BYTE*)(pStruct), sizeof(TUpdatePortOpenRtpChannelReq), H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ, isInternal, index);
  pMcChannel->SetSeqNumRtp(seqNum);
  PDELETE(pStruct);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::Rtp_FillUpdatePortOpenRtpChannelStruct(TUpdateRtpSpecificChannelParams* pUpdateRtpSpecificChannelParams, CChannel *pMcChannel)
{
  //init filed which are depends on dataType or on capEnum:
    if(pMcChannel->GetType() == cmCapVideo && pMcChannel->GetChannelDirection() == cmCapTransmit && (!(pMcChannel->GetRoleLabel() & kRoleContentOrPresentation))) //FE-7959 (kRoleContentOrPresentation)
    {
        TRACEINTO << "For video transmit, set IsH263Plus=FALSE as encoder does not support it";
        pUpdateRtpSpecificChannelParams->bunIsH263Plus = FALSE;
    }
    else
    {
        pUpdateRtpSpecificChannelParams->bunIsH263Plus = pMcChannel->IsH263Plus();
    }
  pUpdateRtpSpecificChannelParams->bunIsFlipIntraBit = 0;
  pUpdateRtpSpecificChannelParams->unAnnexesMask = 0;

  pUpdateRtpSpecificChannelParams->nHcMpiCif     = -1;
  pUpdateRtpSpecificChannelParams->nHcMpiQCif   = -1;
  pUpdateRtpSpecificChannelParams->nHcMpi4Cif   = -1;
  pUpdateRtpSpecificChannelParams->nHcMpi16Cif  = -1;
  pUpdateRtpSpecificChannelParams->nHcMpiVga    = -1;
  pUpdateRtpSpecificChannelParams->nHcMpiNtsc   = -1;
  pUpdateRtpSpecificChannelParams->nHcMpiSvga   = -1;
  pUpdateRtpSpecificChannelParams->nHcMpiXga    = -1;
  pUpdateRtpSpecificChannelParams->nHcMpiSif    = -1;
  pUpdateRtpSpecificChannelParams->nHcMpiQvga   = -1;
  pUpdateRtpSpecificChannelParams->b32StreamRequiresEndOfFrameParsing = FALSE;

  pUpdateRtpSpecificChannelParams->unMaxFramesPerPacket = 0;
  pUpdateRtpSpecificChannelParams->unCustomMaxMbpsValue = 0;
  pUpdateRtpSpecificChannelParams->bunIsCCEnabled     = 0;
  pUpdateRtpSpecificChannelParams->bunContentEnabled = 0;

  // update the payload type
  APIU8 dynamicPayloadType    = pMcChannel->GetDynamicPayloadType();
  if (dynamicPayloadType != 0)
    pUpdateRtpSpecificChannelParams->unPayloadType  = dynamicPayloadType;
  else
    pUpdateRtpSpecificChannelParams->unPayloadType  = pMcChannel->GetPayloadType();
  pUpdateRtpSpecificChannelParams->unDtmfPayloadType  = _UnKnown;
  //if FECC channel and the payload type is 205 or 206, we should replace it with 254 (due to VCON HD5000, FX and Athera bugs).
  if(pMcChannel->GetType() == cmCapData)
  {
    if((pMcChannel->GetDynamicPayloadType() == _AnnexQ) || (pMcChannel->GetDynamicPayloadType() == _RvFecc) || (pMcChannel->GetDynamicPayloadType() == 0))
      pUpdateRtpSpecificChannelParams->unPayloadType = 254;// currently we can't do new H file definition.

    // eanbled disabled close caption (indication going over the FECC channel)
    BOOL IsEnabledCloseCaption = NO;
    CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    pSysConfig->GetBOOLDataByKey("ENABLE_CLOSED_CAPTION", IsEnabledCloseCaption);
    if(IsEnabledCloseCaption)
      pUpdateRtpSpecificChannelParams->bunIsCCEnabled     = 1;
  }

  //fill general fields
  pUpdateRtpSpecificChannelParams->unBitRate      = pMcChannel->GetRate();
  pUpdateRtpSpecificChannelParams->unDestMcuId    = m_pParty->GetMcuNum();
  pUpdateRtpSpecificChannelParams->unDestTerminalId = m_pParty->GetTerminalNum();

  //fill specific fields
  channelSpecificParameters* pChannelParams = (channelSpecificParameters*)pMcChannel->GetChannelParams();
  cmCapDataType dataType = pMcChannel->GetType();
  CapEnum capEnum = pMcChannel->GetCapNameEnum();
  pUpdateRtpSpecificChannelParams->unPacketPayloadFormat = E_PACKET_PAYLOAD_FORMAT_SINGLE_UNIT;

  //==================
  // Traffic shaping
  //==================
  UpdateTrafficShapingParams(*pUpdateRtpSpecificChannelParams);

  if (dataType == cmCapVideo)
  {
    CBaseVideoCap *pBaseVideoCap = (CBaseVideoCap*)CBaseCap::AllocNewCap(capEnum,(BYTE*)pChannelParams);

    //fill unAnnexesAndResolutionMask
    if (capEnum == eH263CapCode)
      memcpy(&(pUpdateRtpSpecificChannelParams->unAnnexesMask), &(pChannelParams->p263.annexesMask), sizeof(pChannelParams->p263.annexesMask));

    //fill mpi
    if(pBaseVideoCap)
    {
    pUpdateRtpSpecificChannelParams->nHcMpiCif   = pBaseVideoCap->GetFormatMpi(kCif);
    pUpdateRtpSpecificChannelParams->nHcMpiQCif  = pBaseVideoCap->GetFormatMpi(kQCif);
    pUpdateRtpSpecificChannelParams->nHcMpi4Cif  = pBaseVideoCap->GetFormatMpi(k4Cif);
    pUpdateRtpSpecificChannelParams->nHcMpi16Cif = pBaseVideoCap->GetFormatMpi(k16Cif);
    pUpdateRtpSpecificChannelParams->nHcMpiVga   = pBaseVideoCap->GetFormatMpi(kVGA);
    pUpdateRtpSpecificChannelParams->nHcMpiNtsc  = pBaseVideoCap->GetFormatMpi(kNTSC);
    pUpdateRtpSpecificChannelParams->nHcMpiSvga  = pBaseVideoCap->GetFormatMpi(kSVGA);
    pUpdateRtpSpecificChannelParams->nHcMpiXga   = pBaseVideoCap->GetFormatMpi(kXGA);
    pUpdateRtpSpecificChannelParams->nHcMpiSif   = pBaseVideoCap->GetFormatMpi(kSIF);
    pUpdateRtpSpecificChannelParams->nHcMpiQvga  = pBaseVideoCap->GetFormatMpi(kQVGA);
    }
    else
    {
      PASSERTMSG(NULL == pBaseVideoCap, "AllocNewCap return NULL!!!");
    }

    //fill unCustomMaxMbpsValue
    if (capEnum == eH264CapCode)
    {
      APIS32 mbps = pChannelParams->p264.customMaxMbpsValue;
      if (mbps == -1)
      {
        APIU8 level = pChannelParams->p264.levelValue;
        CH264Details h264Details = level;
        mbps = h264Details.GetDefaultMbpsAsProduct();//we send the product to the card
      }
      else
        mbps = ConvertMaxMbpsToProduct(mbps);//we send the product to the card

      pUpdateRtpSpecificChannelParams->unCustomMaxMbpsValue = mbps;

      APIU8 profile = pChannelParams->p264.profileValue;
      if (profile == H264_Profile_High)
        pUpdateRtpSpecificChannelParams->unPacketPayloadFormat = E_PACKET_PAYLOAD_FORMAT_FRAGMENTATION_UNIT;

    }
    POBJDELETE(pBaseVideoCap);
  // H239 addition
//    pUpdateRtpSpecificChannelParams->bunContentEnabled = (APIU32)eStreamOff;
    //specific cases to send stream on:
    if (pMcChannel->GetRoleLabel()  & kRoleContentOrPresentation)
    {
			PTRACE2INT(eLevelInfoNormal, "CH323Cntl::Rtp_FillUpdatePortOpenRtpChannelStruct, m_isContentOn", m_isContentOn);
      if ( !pMcChannel->IsOutgoingDirection() && (m_eContentInState == eWaitToSendStreamOn ||  m_eContentInState ==eSendStreamOn)) //receive + channel has opened after acquire ack has sent
      {
        pUpdateRtpSpecificChannelParams->bunContentEnabled = (APIU32)eStreamOn;
        m_eContentInState = eSendStreamOn;
                //SendContentOnOffReqForRtp();

      }
			else if ((pMcChannel->IsOutgoingDirection() && (pMcChannel->GetStreamState() == kStreamUpdate)) ||  ( pMcChannel->IsOutgoingDirection() && m_isContentOn ) ) //transmit + stream state need to be updated
        pUpdateRtpSpecificChannelParams->bunContentEnabled = (APIU32)eStreamOn;

    } else {

      // For Tandberg 6000E
      if (!strncmp(m_remoteVendor.versionId, Tandberg6000EVersionID, strlen(Tandberg6000EVersionID))) {

        PTRACE2(eLevelInfoNormal, "CH323Cntl::Rtp_FillUpdatePortOpenRtpChannelStruct, m_remoteVendor.versionId:", m_remoteVendor.versionId);

        if (!pMcChannel->IsOutgoingDirection())
          pUpdateRtpSpecificChannelParams->b32StreamRequiresEndOfFrameParsing = TRUE;
      }
    }
  }

  if (dataType == cmCapAudio)
  {
    CBaseAudioCap *pBaseAudioCap = (CBaseAudioCap*)CBaseCap::AllocNewCap(capEnum,(BYTE*)pChannelParams);
    if(pBaseAudioCap)
    {
    pUpdateRtpSpecificChannelParams->unMaxFramesPerPacket = pBaseAudioCap->GetMaxFramePerPacket();
    POBJDELETE(pBaseAudioCap);
  }
    else
      PASSERTMSG(NULL == pBaseAudioCap, "AllocNewCap return NULL");
  }
  /*Init the data structure for TIP to avoid the valgrind errors. */
   /*Use of uninitialized value in APITraceRtpUpdateRtpSpecificChannelParamsReq*/
  /*David Liang 2012 */
  pUpdateRtpSpecificChannelParams->openTipPortParams.bnTipIsEnabled = FALSE;
  pUpdateRtpSpecificChannelParams->openTipPortParams.eTipPosition = eTipNone;
  pUpdateRtpSpecificChannelParams->unMsftClient = MSFT_CLIENT_DUMMY;

}

////////////////////////////////////////////////////////////////////////////
// H323_RTP_UPDATE_CHANNEL_REQ
void CH323Cntl::Rtp_FillAndSendUpdateRtpChannelStruct(CChannel *pMcChannel)
{
  DWORD seqNum = 0;
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::Rtp_FillAndSendUpdateRtpChannelStruct @#@- ",m_pMfaInterface->GetConnectionId());
  TUpdateRtpChannelReq* pStruct = new TUpdateRtpChannelReq;
  memset(pStruct, 0, sizeof(TUpdateRtpChannelReq));

  pStruct->unChannelType      = ::DataTypeToChannelType(pMcChannel->GetType(),pMcChannel->GetRoleLabel());
  pStruct->unChannelDirection = ::CalcCmCapDirection(pMcChannel->IsOutgoingDirection());

  // encryption fields
  pStruct->unEncryptionType   = pMcChannel->GetEncryptionType();
  if (pMcChannel->GetIsEncrypted())
   	memcpy(&(pStruct->aucSessionKey),pMcChannel->GetH235SessionKey() , sizeOf128Key);

  pStruct->mediaMode = eMediaModeTranscoding;
  if (GetTargetMode()->GetConfMediaType() == eMixAvcSvc &&
		  pStruct->unChannelType == kIpVideoChnlType &&
		  pStruct->unChannelDirection == cmCapReceive)
  {
	if (GetTargetMode()->IsHdVswInMixMode())
		pStruct->mediaMode = eMediaModeTranscodingAndVsw;
  }

  Rtp_FillUpdatePortOpenRtpChannelStruct(&pStruct->tUpdateRtpSpecificChannelParams, pMcChannel);
  seqNum = SendMsgToMpl((BYTE*)(pStruct), sizeof(TUpdateRtpChannelReq), H323_RTP_UPDATE_CHANNEL_REQ);
  pMcChannel->SetSeqNumRtp(seqNum);
  PDELETE(pStruct);
}

//////////////////////////////////////////////////////////////////////////
// H323_RTP_UPDATE_CHANNEL_RATE_REQ
void CH323Cntl::Rtp_FillAndSendUpdateRtpChannelRateStruct(CChannel *pMcChannel)
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::Rtp_FillAndSendUpdateRtpChannelRateStruct - ",m_pMfaInterface->GetConnectionId());
  TUpdateRtpChannelRateReq* pStruct = new TUpdateRtpChannelRateReq;
  memset(pStruct, 0, sizeof(TUpdateRtpChannelRateReq));

  pStruct->unChannelType      = ::DataTypeToChannelType(pMcChannel->GetType(),pMcChannel->GetRoleLabel());
  pStruct->unChannelDirection = ::CalcCmCapDirection(pMcChannel->IsOutgoingDirection());
  pStruct->unNewChannelRate   = pMcChannel->GetRate();

  SendMsgToMpl((BYTE*)(pStruct), sizeof(TUpdateRtpChannelRateReq), H323_RTP_UPDATE_CHANNEL_RATE_REQ);

  PDELETE(pStruct);
}

////////////////////////////////////////////////////////////////////////////
/*void CH323Cntl::SendStreamOffReq(CChannel *pChannel)
{
  PTRACE2(eLevelInfoNormal,"CH323Cntl::SendStreamOffReq - ",PARTYNAME);
  TStreamOffReq*  pStreamOffReq = new TStreamOffReq;

  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)(&pStreamOffReq),sizeof(TStreamOffReq));
  m_pMfaInterface->SendMsgToMPL(H323_RTP_STREAM_OFF_REQ,pMsg);
  POBJDELETE(pMsg);
  PDELETE(pStreamOffReq);
}*/

////////////////////////////////////////////////////////////////////////////
//H323_RTP_UPDATE_MT_PAIR_REQ
void  CH323Cntl::SendUpdateMtPairReq()
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SendUpdateMtPairReq - ",m_pMfaInterface->GetConnectionId());
  TUpdateMtPairReq *pStruct = new TUpdateMtPairReq;

  pStruct->unDestMcuId      = m_pParty->GetMcuNum();
  pStruct->unDestTerminalId = m_pParty->GetTerminalNum();

    // for Call Generator
  if (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator)
    { // we use this opcode for forwarding party's alias name to MM (for "CG audio improvements" feature)
      const char *pPartyAlias = m_pH323NetSetup->GetH323PartyAlias();
      DWORD partyAlias = atoi(pPartyAlias);
      pStruct->unDestMcuId = partyAlias;
      TRACEINTO << "CH323Cntl::SendUpdateMtPairReq - partyAlias: " << partyAlias;
    }

  SendMsgToMpl((BYTE*)(pStruct), sizeof(TUpdateMtPairReq), H323_RTP_UPDATE_MT_PAIR_REQ);

  PDELETE(pStruct);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendContentOnOffReqForRtp()
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SendContentOnOffReqForRtp - Stream state = ",(APIU32)m_eContentInState);
  if (m_eContentInState == eWaitToSendStreamOn)
  {
    PASSERT_AND_RETURN((APIU32)eWaitToSendStreamOn);
  }

  DWORD   dwOpcode  = 0;
  APIU32  bunIsOnOff  = 0;

  if (m_eContentInState == eSendStreamOn)
  {
    bunIsOnOff = 1;
    dwOpcode = ART_CONTENT_ON_REQ;
  }
  else if (m_eContentInState == eSendStreamOff)
  {
    bunIsOnOff = 0;
    dwOpcode = ART_CONTENT_OFF_REQ;
  }
  else
  {
    PASSERTMSG((DWORD)m_eContentInState,"CH323Cntl::SendContentOnOffReqForRtp - Wrong status");
    return;
  }

  TContentOnOffReq* pStruct = new TContentOnOffReq;
  pStruct->unChannelDirection = cmCapReceive;
  pStruct->bunIsOnOff = bunIsOnOff;
  SendMsgToMpl((BYTE*)(pStruct), sizeof(TContentOnOffReq), dwOpcode);
  PDELETE(pStruct);
}
//////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendCGContentOnOffReqForRtp(BYTE isOn)
{
  if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::SendCGContentOnOffReqForRtp - ERROR - system is not CG!!");
    return;
  }

  DWORD dwOpcode = 0;
  TContentOnOffReq* pStruct = new TContentOnOffReq;
  pStruct->unChannelDirection = cmCapTransmit;
  if(isOn)
  {
    pStruct->bunIsOnOff = 1;
    dwOpcode = ART_CONTENT_ON_REQ;
  }
  else
  {
    pStruct->bunIsOnOff = 0;
    dwOpcode = ART_CONTENT_OFF_REQ;

  }
  SendMsgToMpl((BYTE*)(pStruct), sizeof(TContentOnOffReq), dwOpcode);
  PDELETE(pStruct);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendEvacuateReqForRtpOnH239Stream()
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SendEvacuateReqForRtpOnH239Stream - Con Id ",m_pCsRsrcDesc->GetConnectionId());

  TEvacuateReq* pStruct = new TEvacuateReq;

  pStruct->unChannelType = (DWORD)kIpContentChnlType;
  pStruct->unChannelDirection = cmCapReceive;

  SendMsgToMpl((BYTE*)(pStruct), sizeof(TEvacuateReq), ART_EVACUATE_REQ);
  PDELETE(pStruct);
}

///////////////////////////////////////////////////////////////////////////////////////////
void  CH323Cntl::PartyMonitoringReq()
{
	PTRACE2INT(eLevelInfoNormal,"CH323Cntl::PartyMonitoringReq m_state:",m_state);

	if (CPObject::IsValidPObjectPtr(m_pCsRsrcDesc))
		PTRACE(eLevelInfoNormal,"CH323Cntl::PartyMonitoringReq m_pCsRsrcDesc not null");
	else
		PTRACE(eLevelInfoNormal,"CH323Cntl::PartyMonitoringReq m_pCsRsrcDesc is null - don't send monitoring request!");

	if (m_state != SETUP)
		PTRACE(eLevelInfoNormal,"CH323Cntl::PartyMonitoringReq m_state != SETUP");
	else
		PTRACE(eLevelInfoNormal,"CH323Cntl::PartyMonitoringReq m_state == SETUP");

	if (m_state != IDLE)
		PTRACE(eLevelInfoNormal,"CH323Cntl::PartyMonitoringReq m_state != IDLE");
	else
		PTRACE(eLevelInfoNormal,"CH323Cntl::PartyMonitoringReq m_state == IDLE");

	if (CPObject::IsValidPObjectPtr(m_pCsRsrcDesc) && m_state != SETUP && m_state != IDLE)
	{
		PTRACE2INT(eLevelInfoNormal,"CH323Cntl::PartyMonitoringReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
		TPartyMonitoringReq* pStruct = new TPartyMonitoringReq;
		pStruct->ulTipChannelPartyID = 0;

		SendMsgToMpl((BYTE*)(pStruct), sizeof(TPartyMonitoringReq), H323_RTP_PARTY_MONITORING_REQ);

		PDELETE(pStruct);
	}
	else
		PTRACE(eLevelInfoNormal,"CH323Cntl::PartyMonitoringReq - monitoring request was not sent");

}

/////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnPartyNewChannelModeReq(DWORD specificVideoRate)
{//sending H323_NEW_CHANNEL_MODE_REQ
  DWORD seqNum = 0;
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyNewChannelModeReq - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());
  CChannel *pMcChannel = FindChannelInList(cmCapVideo, FALSE); //incoming channel
  if (pMcChannel == NULL)
  {
    PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyNewChannelModeReq - Channel wasn't found - ", m_pCsRsrcDesc->GetConnectionId());
    return;
  }
  else
  {
    if (pMcChannel->GetCsChannelState() != kConnectedState)
    {
      PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnPartyNewChannelModeReq - Channel isn't in connected state - ", m_pCsRsrcDesc->GetConnectionId());
      return;
    }
  }

  BYTE bIsNsAnnexI     = FALSE;
  BYTE bIsAnnexFOnlyInCaps = FALSE;
  BYTE bIsAnnexTOnlyInCaps = FALSE;

  WORD length = m_pTargetModeH323->GetMediaLength(cmCapVideo, cmCapReceive);
  WORD capCode = m_pTargetModeH323->GetMediaMode(cmCapVideo,cmCapReceive).GetType();
  if((CapEnum)capCode == eH263CapCode)
  {
    CBaseVideoCap* pVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap(eH263CapCode,m_pTargetModeH323->GetMediaMode(cmCapVideo,cmCapReceive).GetDataCap());
    if(pVideoCap)
    {
    if (((CH263VideoCap*)pVideoCap)->IsAnnex(typeAnnexI_NS) || m_pLocalCapH323->IsNsAnnexI() )
    {
      length += sizeof(h263OptionsStruct); //for annex I
      bIsNsAnnexI = TRUE;
    }

    //added here code for FX, sabre, view station:
    //they have a bug, that don't open a channel without the annexes, but do transmit the annexes
    if (((CH263VideoCap*)pVideoCap)->IsAnnex(typeAnnexF) == FALSE)
    {
      if (m_pLocalCapH323->IsAnnex(typeAnnexF) && m_pRmtCapH323->IsAnnex(typeAnnexF))
      {
        length += sizeof(h263OptionsStruct);
        bIsAnnexFOnlyInCaps = TRUE;
      }
    }

    if (((CH263VideoCap*)pVideoCap)->IsAnnex(typeAnnexT) == FALSE)
    {
      if (m_pLocalCapH323->IsAnnex(typeAnnexT) && m_pRmtCapH323->IsAnnex(typeAnnexT))
      {
        length += sizeof(h263OptionsStruct);
        bIsAnnexTOnlyInCaps = TRUE;
      }
    }
    POBJDELETE(pVideoCap);
  }
    else
      PASSERTMSG(NULL == pVideoCap, "AllocNewCap return NULL!!!");

  }

  Rtp_FillAndSendUpdateRtpChannelStruct(pMcChannel);
}


////////////////////////////////////////////////////////////////////////////
/*                    FECC                */
////////////////////////////////////////////////////////////////////////////

void CH323Cntl::OnH323FeccTokenInd(CSegment *pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323FeccTokenInd - ",m_pMfaInterface->GetConnectionId());

  TRtpFeccTokenRequestInd *pFeccToken = new TRtpFeccTokenRequestInd;
  DWORD  structLen = sizeof(TRtpFeccTokenRequestInd);
  memset(pFeccToken, 0, structLen);
  pParam->Get((BYTE*)pFeccToken, structLen);

  CChannel *pChannel = FindChannelInList(cmCapData, FALSE, kRolePeople); //incoming channel
    CChannel *pVideoOutChannel = FindChannelInList(cmCapVideo, TRUE, kRolePeople); //incoming channel
  if (pChannel && pVideoOutChannel && !pVideoOutChannel->IsCsChannelStateDisconnecting())
  {
    /* In cop the FECC feature is only through PCM - there is not always
     * a valid speaker when a fecc token ind received
    if (m_speakerMcuNum == 0 && m_speakerTermNum == 0)
    {
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323FeccTokenInd - Sending reject - No VIN ",m_pMfaInterface->GetConnectionId());
      OnPartyFeccTokenReq(kTokenReject);
    }
    else
    */
      m_pTaskApi->OnIpDataTokenMsg(pFeccToken->unTokenOpcode,LSD_6400,pFeccToken->unIsCameraControl);
  }
  else
    PTRACE(eLevelError,"CH323Cntl::OnH323FeccTokenInd - Data in channel wasn't found or video out channel wasn't found");

  PDELETE(pFeccToken);


}
////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323FeccKeyInd(CSegment *pParam)
{
//  if (!m_pParty->GetIsLeader())
//  {
//    CSmallString cstr;
//    cstr << "party: " << PARTYNAME << "received fecc key but it is not the leader!!!";
//    PASSERTMSG(1,cstr.GetString());
//    return;
//  }


  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323FeccKeyInd - ",m_pMfaInterface->GetConnectionId());

  TRtpFeccTokenRequestInd *pFeccToken = new TRtpFeccTokenRequestInd;
  DWORD  structLen = sizeof(TRtpFeccTokenRequestInd);
  memset(pFeccToken, 0, structLen);
  pParam->Get((BYTE*)pFeccToken, structLen);

  CChannel *pChannel = FindChannelInList(cmCapData, FALSE, kRolePeople); //incoming channel
  if (pChannel)
  {
    m_pTaskApi->OnIpFeccKeyMsg(pFeccToken->unTokenOpcode);
  }
  else
    PTRACE(eLevelError,"CH323Cntl::OnH323FeccKeyInd - Data channel wasn't found");

  PDELETE(pFeccToken);
}

////////////////////////////////////////////////////////////////////////////
//H323_RTP_FECC_TOKEN_RESPONSE_REQ
void CH323Cntl::OnPartyFeccTokenReq(feccTokenEnum eFeccTokenOpcode, WORD isCameraControl)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnPartyFeccTokenReq - ",m_pMfaInterface->GetConnectionId());
  TFeccTokenResponseReq *pStruct = new TFeccTokenResponseReq;

  CChannel *pChannel = FindChannelInList(cmCapData, FALSE, kRolePeople); //incoming channel

  if (pChannel && pChannel->IsChannelConnected())
  {
    pStruct->unResponse							= (APIU32)eFeccTokenOpcode;
    pStruct->unDestMcuId						= m_speakerMcuNum;
    pStruct->unDestTerminalId					= m_speakerTermNum;
    pStruct->tDestTerminalPhysicalRtp.party_id	= (APIU32)m_speakerPartyId;

	CRsrcDesc *pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_speakerPartyId, eLogical_rtp);
	if(IsValidPObjectPtr(pRsrcDesc))
	{
		ConnectionID connId = pRsrcDesc->GetConnectionId();
		pStruct->tDestTerminalPhysicalRtp.connection_id	= (APIU32)connId;
	}

    pStruct->unIsCameraControl  = isCameraControl;

    SendMsgToMpl((BYTE*)(pStruct), sizeof(TFeccTokenResponseReq), H323_RTP_FECC_TOKEN_RESPONSE_REQ);

    m_isCameraControl = 0;
  }

  else
  {
    PTRACE(eLevelError,"CH323Cntl::OnPartyFeccTokenReq: Incoming fecc channel wasn't found");
  }

  PDELETE(pStruct);
}

////////////////////////////////////////////////////////////////////////////
/*                   CM                         */
////////////////////////////////////////////////////////////////////////////
// CONFPARTY_CM_OPEN_UDP_PORT_REQ
// CONFPARTY_CM_UPDATE_UDP_ADDR_REQ
void CH323Cntl::Cm_FillAndSendOpenUdpPortOrUpdateUdpAddrStruct(CChannel* pMcChannel, mcTransportAddress rmtAddress)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::Cm_FillAndSendOpenUdpPortOrUpdateUdpAddrStruct - ",m_pMfaInterface->GetConnectionId());
  DWORD seqNum = 0;
  pMcChannel->SetCmUdpChannelState(kSendOpen);
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

  TOpenUdpPortOrUpdateUdpAddrMessageStruct* pStruct = new TOpenUdpPortOrUpdateUdpAddrMessageStruct;
  memset(pStruct, 0, sizeof(TOpenUdpPortOrUpdateUdpAddrMessageStruct));

  mcReqCmOpenUdpPortOrUpdateUdpAddr* pUdpSt  = &pStruct->tCmOpenUdpPortOrUpdateUdpAddr;
  pUdpSt->channelType    = ::DataTypeToChannelType(pMcChannel->GetType(),pMcChannel->GetRoleLabel());
  pUdpSt->channelDirection = ::CalcCmCapDirection(pMcChannel->IsOutgoingDirection());

  pUdpSt->ice_channel_rtcp_id = 0;
  pUdpSt->ice_channel_rtp_id = 0;

  // TIP
  //pUdpSt->bIsTipEnable = FALSE;

  //fill local address
  // IpV6
  enIpVersion eIpAddrMatch = CheckForMatchBetweenPartyAndUdp(m_pH323NetSetup->GetIpVersion(),m_UdpAddressesParams.IpType);

  pUdpSt->CmLocalUdpAddressIp.ipVersion = eIpAddrMatch;
  if (pUdpSt->CmLocalUdpAddressIp.ipVersion == eIpVersion4) //v4
    pUdpSt->CmLocalUdpAddressIp.addr.v4.ip = m_UdpAddressesParams.IpV4Addr.ip;
  else
  {
    // --- UDP: array of addresses ---

    BYTE  place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(m_pH323NetSetup->GetTaDestPartyAddr(), m_UdpAddressesParams.IpV6AddrArray);

    memcpy(pUdpSt->CmLocalUdpAddressIp.addr.v6.ip, m_UdpAddressesParams.IpV6AddrArray[place].ip, 16);
    pUdpSt->CmLocalUdpAddressIp.addr.v6.scopeId = m_UdpAddressesParams.IpV6AddrArray[place].scopeId;

  }

  //fill remote address
  pUdpSt->CmRemoteUdpAddressIp.ipVersion = eIpAddrMatch;
  if (pUdpSt->CmRemoteUdpAddressIp.ipVersion == eIpVersion4) //v4
    pUdpSt->CmRemoteUdpAddressIp.addr.v4.ip = rmtAddress.addr.v4.ip;
  else
  {
    memcpy(pUdpSt->CmRemoteUdpAddressIp.addr.v6.ip, rmtAddress.addr.v6.ip, 16);
    pUdpSt->CmRemoteUdpAddressIp.addr.v6.scopeId = rmtAddress.addr.v6.scopeId;
  }

  //fill fields for both local and remote addresses
  pUdpSt->CmLocalUdpAddressIp.distribution  = eDistributionUnicast;// Don't Care
  pUdpSt->CmRemoteUdpAddressIp.distribution = eDistributionUnicast;// Don't Care
  pUdpSt->CmLocalUdpAddressIp.transportType = eTransportTypeUdp;
  pUdpSt->CmRemoteUdpAddressIp.transportType = eTransportTypeUdp;
  pUdpSt->connMode = eUdp;

  cmCapDataType dataType = pMcChannel->GetType();
  ERoleLabel eRole = pMcChannel->GetRoleLabel();
  switch (dataType)
  {
    case cmCapAudio:
      pUdpSt->CmLocalUdpAddressIp.port  = m_UdpAddressesParams.AudioChannelPort;
      pUdpSt->CmRemoteUdpAddressIp.port = rmtAddress.port;
      pUdpSt->LocalRtcpPort       = m_UdpAddressesParams.AudioChannelPort + 1;
      pUdpSt->RemoteRtcpPort        = rmtAddress.port + 1;
      break;
    case cmCapVideo:
      if (eRole == kRolePeople)
      {
        pUdpSt->CmLocalUdpAddressIp.port  = m_UdpAddressesParams.VideoChannelPort;
        pUdpSt->CmRemoteUdpAddressIp.port = rmtAddress.port;
        pUdpSt->LocalRtcpPort       = m_UdpAddressesParams.VideoChannelPort + 1;
        pUdpSt->RemoteRtcpPort        = rmtAddress.port + 1;
      }
      else
      {
        pUdpSt->CmLocalUdpAddressIp.port  = m_UdpAddressesParams.ContentChannelPort;
        pUdpSt->CmRemoteUdpAddressIp.port = rmtAddress.port;
        pUdpSt->LocalRtcpPort       = m_UdpAddressesParams.ContentChannelPort + 1;
        pUdpSt->RemoteRtcpPort        = rmtAddress.port + 1;
      }
      break;
    case cmCapData:
      pUdpSt->CmLocalUdpAddressIp.port  = m_UdpAddressesParams.FeccChannelPort;
      pUdpSt->CmRemoteUdpAddressIp.port = rmtAddress.port;
      pUdpSt->LocalRtcpPort       = m_UdpAddressesParams.FeccChannelPort + 1;
      pUdpSt->RemoteRtcpPort        = rmtAddress.port + 1;
      break;
    default:
      pUdpSt->CmLocalUdpAddressIp.port  = 0;
      pUdpSt->CmRemoteUdpAddressIp.port = 0;
      pUdpSt->LocalRtcpPort       = 0;
      pUdpSt->RemoteRtcpPort        = 0;
      break;
  }
  if(pCommConf)
  pUdpSt->uRtpKeepAlivePeriod = pCommConf->GetNatKAPeriod();
  else
    PASSERT(NULL == pCommConf);

  if (pUdpSt->uRtpKeepAlivePeriod)
	  PTRACE2INT(eLevelInfoNormal, "Setting RTP KeepAlive uRtpKeepAlivePeriod=", pUdpSt->uRtpKeepAlivePeriod);

  if(m_useRtcp == 0)
  {
	  pUdpSt->uRtpKeepAlivePeriod = 0;
	  PTRACE2(eLevelError, "Setting RTP KeepAlive to 0 for MXP otherwise no media  ", PARTYNAME);
  }

  pMcChannel->SetRtcpPort(pUdpSt->LocalRtcpPort);
  pMcChannel->SetRtcpRmtPort(pUdpSt->RemoteRtcpPort);

  if (dataType == cmCapAudio)
    pUdpSt->tosValue[MEDIA_TOS_VALUE_PLACE] = m_pQos->m_bIpAudio;
  else if (dataType == cmCapVideo)
    pUdpSt->tosValue[MEDIA_TOS_VALUE_PLACE] = m_pQos->m_bIpVideo;
  else
    pUdpSt->tosValue[MEDIA_TOS_VALUE_PLACE] = 0;

  BOOL isRtcpQosIsEqualToRtp = NO;
  std::string key = "RTCP_QOS_IS_EQUAL_TO_RTP";
  CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, isRtcpQosIsEqualToRtp);
  if( isRtcpQosIsEqualToRtp )
      pUdpSt->tosValue[RTCP_TOS_VALUE_PLACE] = pUdpSt->tosValue[MEDIA_TOS_VALUE_PLACE];
  else
      pUdpSt->tosValue[RTCP_TOS_VALUE_PLACE] = m_pQos->m_bIpRtcp;

  pUdpSt->RtcpCnameMask = m_RtcpCnameMask;
  memset(&(pStruct->physicalPort.physical_id), 0, sizeof(PHYSICAL_RESOURCE_INFO_S));
  pStruct->physicalPort.connection_id = m_pMfaInterface->GetConnectionId();
  pStruct->physicalPort.party_id = m_pCsRsrcDesc->GetPartyRsrcId();

  memset(&pStruct->tCmOpenUdpPortOrUpdateUdpAddr.sdesCap, 0, sizeof(sdesCapSt));

  pStruct->tCmOpenUdpPortOrUpdateUdpAddr.capProtocolType    = ::DataTypeToChannelType(pMcChannel->GetType(),pMcChannel->GetRoleLabel());

  ///////////////////////////////////////////////
  // Disconnection detect timer taken from SIP
  CCall* pCall = GetCallParams();
  if(pCall&&(kIpAudioChnlType == pStruct->tCmOpenUdpPortOrUpdateUdpAddr.channelType ||kIpVideoChnlType == pStruct->tCmOpenUdpPortOrUpdateUdpAddr.channelType)
 	&&(cmCapTransmit ==pStruct->tCmOpenUdpPortOrUpdateUdpAddr.channelDirection))
  {
	pStruct->tCmOpenUdpPortOrUpdateUdpAddr.ulDetectionTimerLen = pCall->GetMediaDetectionTimer();

	if(kIpVideoChnlType == pStruct->tCmOpenUdpPortOrUpdateUdpAddr.channelType)
		pCall->SetMediaDetectioHasVideo(TRUE);
  }
  else
  {
	pStruct->tCmOpenUdpPortOrUpdateUdpAddr.ulDetectionTimerLen = 0;
  }

  seqNum = SendMsgToMpl((BYTE*)(pStruct), sizeof(TOpenUdpPortOrUpdateUdpAddrMessageStruct), CONFPARTY_CM_OPEN_UDP_PORT_REQ);
  pMcChannel->SetSeqNumCm(seqNum);
  PDELETE(pStruct);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OpenSvcChannel(CChannel *pChannel, mcTransportAddress &rmtAddress, BYTE isUpdate)
{
    char s[50];
    DWORD seqNum = 0;
    kChanneltype channelType;

    TRACEINTOFUNC << "mix_mode: open MRMP channel";
    if (GetTargetMode()->GetConfMediaType() != eMixAvcSvc)
        return;

    sprintf(s, "\nDest address is 0x%x", rmtAddress.addr.v4.ip);
//    TRACEINTOFUNC << "IsUpdate: " << ( isUpdate ? "true" : "false" ) << s;

    MrmpOpenChannelRequestStruct* pStruct = new MrmpOpenChannelRequestStruct;
    memset(pStruct,0,sizeof(MrmpOpenChannelRequestStruct));

    channelType = ::DataTypeToChannelType(pChannel->GetType(), pChannel->GetRoleLabel());
    TRACEINTO << "mix_mode: channelType = " << channelType << " media type = " << ::GetTypeStr(pChannel->GetType()) << " role = " << ::GetRoleStr(pChannel->GetRoleLabel());
    if (channelType == kIpAudioChnlType)
    {
        pStruct->m_channelType = kAvcToSacChnlType;
        TRACEINTO << "mix_mode: channelType set to " << kAvcToSacChnlType;
    }
    else
    {
        pStruct->m_channelType = kSvcAvcChnlType;
        TRACEINTO << "mix_mode: channelType set to " << kSvcAvcChnlType;
    }

    pStruct->m_channelDirection = cmCapReceive;
    pStruct->m_partyId = m_pCsRsrcDesc->GetPartyRsrcId();;

    //fill local address
    // IpV6
    enIpVersion eIpAddrMatch = CheckForMatchBetweenPartyAndUdp(
            m_pH323NetSetup->GetIpVersion(), m_UdpAddressesParams.IpType);

    pStruct->m_localAddress.ipVersion = eIpAddrMatch;
    if (pStruct->m_localAddress.ipVersion == eIpVersion4) //v4
        pStruct->m_localAddress.addr.v4.ip
                = m_UdpAddressesParams.IpV4Addr.ip;
    else {
        // --- UDP: array of addresses ---

        BYTE place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(
                m_pH323NetSetup->GetTaDestPartyAddr(),
                m_UdpAddressesParams.IpV6AddrArray);

        memcpy(pStruct->m_localAddress.addr.v6.ip,
                m_UdpAddressesParams.IpV6AddrArray[place].ip, 16);
        pStruct->m_localAddress.addr.v6.scopeId
                = m_UdpAddressesParams.IpV6AddrArray[place].scopeId;

    }
    pStruct->m_localAddress.transportType = eTransportTypeUdp;

    //fill remote address
    pStruct->m_remoteAddress.ipVersion = eIpAddrMatch;
    if (pStruct->m_remoteAddress.ipVersion == eIpVersion4) //v4
        pStruct->m_remoteAddress.addr.v4.ip = rmtAddress.addr.v4.ip;
    else {
        memcpy(pStruct->m_remoteAddress.addr.v6.ip, rmtAddress.addr.v6.ip,
                16);
        pStruct->m_remoteAddress.addr.v6.scopeId
                = rmtAddress.addr.v6.scopeId;
    }



    //fill fields for both local and remote addresses
   pStruct->m_localAddress.distribution  = eDistributionUnicast;// Don't Care
   pStruct->m_localAddress.distribution = eDistributionUnicast;// Don't Care

   pStruct->m_remoteAddress.transportType = eTransportTypeUdp;


   // fill local rtcp address




    pStruct->m_localRtcpAddress.ipVersion = eIpAddrMatch;

    bool bIsLocalRtcpAddressIpV4 = ( (pStruct->m_localRtcpAddress.ipVersion == (APIU32)eIpVersion4) ? true : false );
    if (true == bIsLocalRtcpAddressIpV4) //v4
    {
        pStruct->m_localRtcpAddress.addr.v4.ip = m_UdpAddressesParams.IpV4Addr.ip;
    }

    else
    {
        // --- UDP: array of addresses ---
        // First we will look for the best IpV6 address match -> Meaning we will match ScopeId's
        BYTE place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(
                m_pH323NetSetup->GetTaDestPartyAddr(),
                m_UdpAddressesParams.IpV6AddrArray);
        memcpy(pStruct->m_localRtcpAddress.addr.v6.ip, m_UdpAddressesParams.IpV6AddrArray[place].ip, 16);
        pStruct->m_localRtcpAddress.addr.v6.scopeId = m_UdpAddressesParams.IpV6AddrArray[place].scopeId;
    }
    pStruct->m_localRtcpAddress.transportType = eTransportTypeUdp;


    //fill remote rtcp address
    pStruct->m_remoteRtcpAddress.ipVersion = eIpAddrMatch;

    bool bIsRemoteRtcpAddressIpV4 = ( (pStruct->m_remoteRtcpAddress.ipVersion == (APIU32)eIpVersion4) ? true : false );
    if (true == bIsRemoteRtcpAddressIpV4) //v4
    {
        pStruct->m_remoteRtcpAddress.addr.v4.ip = rmtAddress.addr.v4.ip;
    }
    else
    {
        memcpy(pStruct->m_remoteRtcpAddress.addr.v6.ip, rmtAddress.addr.v6.ip, 16);
        pStruct->m_remoteRtcpAddress.addr.v6.scopeId = rmtAddress.addr.v6.scopeId;
    }


    // send to trace (local and remote RTCP addresses)
    if (bIsLocalRtcpAddressIpV4 && bIsRemoteRtcpAddressIpV4)
    {
        TRACEINTOFUNC << "Local RTCP address type: IPv4, remote RTCP address type: IPv4";
    }

    else
    {
        TRACEINTOFUNC
            << "Local RTCP address type: " << ( bIsLocalRtcpAddressIpV4 ? "IPv4" : "IPv6" )
            << ", remote RTCP address type: " << ( bIsRemoteRtcpAddressIpV4 ? "IPv4" : "IPv6" );

        if (!bIsLocalRtcpAddressIpV4)
        {
            BYTE place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(
                    m_pH323NetSetup->GetTaDestPartyAddr(),
                    m_UdpAddressesParams.IpV6AddrArray);
            TRACEINTOFUNC << "Local RTCP address type: Ipv6, place: " << place;
        }

        if (!bIsRemoteRtcpAddressIpV4)
        {
            TRACEINTOFUNC << "Remote RTCP address type: Ipv6, scope: " << rmtAddress.addr.v6.scopeId;
        }
    }


    //fill fields for both local and remote addresses
    pStruct->m_localRtcpAddress.distribution  = eDistributionUnicast;// Don't Care
    pStruct->m_localRtcpAddress.distribution = eDistributionUnicast;// Don't Care

    pStruct->m_remoteRtcpAddress.transportType = eTransportTypeUdp;

    // end rtcp

    cmCapDataType dataType = cmCapVideo;
    ERoleLabel eRole = kRolePeople;

    switch (dataType)
    {
        case cmCapVideo:

            if (eRole == kRolePeople)
            {
                pStruct->m_localAddress.port  = m_UdpAddressesParams.VideoChannelPort;
                pStruct->m_remoteAddress.port = 0; //rmtAddress.port;
                pStruct->m_localRtcpAddress.port              = m_UdpAddressesParams.VideoChannelPort + 1;
                pStruct->m_remoteRtcpAddress.port             = 0; //rmtAddress.port + 1;;
            }
            break;

        default:
            pStruct->m_localAddress.port  = 0;
            pStruct->m_remoteAddress.port = 0;
            pStruct->m_localRtcpAddress.port              = 0;
            pStruct->m_remoteRtcpAddress.port             = 0;
            break;
    }

//    pChannel->SetAddress(pStruct->m_localAddress); //@#@
    pChannel->SetRmtAddress(pStruct->m_remoteAddress);
    pChannel->SetRtcpPort(pStruct->m_localRtcpAddress.port);
    pChannel->SetRtcpRmtPort(pStruct->m_remoteRtcpAddress.port);


    memset(&(pStruct->physicalId), 0, sizeof(PHYSICAL_RESOURCE_INFO_S));

    pStruct->m_capTypeCode=eSvcCapCode;
    pStruct->m_PayloadType=pChannel->GetPayloadType();

    // used for audio only
    pStruct->m_dtmfPayloadType=255; // @#@ 0;

    memset(pStruct->m_ssrcInfo, 0, (sizeof(IncomingSsrcInfo)*MAX_SSRC_PER_INCOMING_CHANNEL));

    pStruct->m_operationPointsSetId=m_pParty->GetConfId();//1;

//    int numOfSsrcIds=0;
//    APIU32* ssrcIds=NULL;
//    GetTargetMode()->Dump("mix_mode: openSvcChannel target mode", eLevelInfoNormal);
//    GetTargetMode()->GetSsrcIds(pChannel->GetType(), cmCapReceive, ssrcIds, &numOfSsrcIds);
//    for (int i=0; i<MAX_NUM_RECV_STREAMS_FOR_MIX_AVC_VIDEO && i<numOfSsrcIds; ++i)
//    {
//        pStruct->m_ssrcInfo[i].m_ssrc = ssrcIds[i];
//        TRACEINTO << "Setting ssrc[" << i << "] = " <<   pStruct->m_ssrcInfo[i].m_ssrc;
//    }

    const std::list <StreamDesc> streamsDescList = pChannel->GetStreams();
    std::list <StreamDesc>::const_iterator itr_streams;
    int i = 0;
    for(itr_streams=streamsDescList.begin(); itr_streams!=streamsDescList.end(); itr_streams++, i++)
    {
		pStruct->m_ssrcInfo[i].m_ssrc = itr_streams->m_pipeIdSsrc;
		TRACEINTO << "Setting ssrc[" << i << "] = " <<   pStruct->m_ssrcInfo[i].m_ssrc;
    }

    pStruct->m_videoFlag=YES;

    FillMrmpOpenChannelPhysicalIdInfo(pStruct, ((kChanneltype)channelType), pChannel->GetChannelDirection(), GetTargetMode()->IsHdVswInMixMode());

    // set max bit rate
    DWORD audioRate = GetTargetMode()->GetMediaBitRate(cmCapAudio, cmCapReceive);
    DWORD contentRate = GetTargetMode()->GetMediaBitRate(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
    DWORD commonBR = GetTargetMode()->GetMediaBitRate(cmCapVideo, cmCapReceive, kRolePeople); // @#@ - is this correct
    // Bella: commonBR  and contentRate are in (kbps * 10) units and audio in kbps
    if(contentRate > commonBR)
        contentRate = 0;
    pStruct->m_maxVideoBR = (commonBR - contentRate) / 10 - audioRate;
    TRACEINTO << "mix_mode: audio bit rate=" << audioRate
                << " content bit rate=" << contentRate << " common bit rate=" << commonBR
                << " max video bit rate=(commonBR - contentRate) / 10 - audioRate=" << pStruct->m_maxVideoBR;

    // uRtpKeepAlivePeriod
    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
    if(pCommConf)
    {
        pStruct->uRtpKeepAlivePeriod = pCommConf->GetNatKAPeriod();
        TRACEINTOFUNC << "\npCommConf->GetNatKAPeriod(): " << pCommConf->GetNatKAPeriod()
                      << "\nuRtpKeepAlivePeriod:         " << pStruct->uRtpKeepAlivePeriod;
    }
    else
    {
        TRACEINTOFUNC << "GetCurrentConf return NULL";
    }

    ///////////////////////////////////////////////
    // temporary: init other params with zero
    for (int i=0; i<NumberOfTosValues; i++)
    {
        pStruct->tosValue[i] = 0;
    }
    pStruct->ice_channel_rtp_id  = 0;
    pStruct->ice_channel_rtcp_id = 0;

    if(isUpdate)
    {
        seqNum = SendMsgToMpl((BYTE*)(pStruct), sizeof(MrmpOpenChannelRequestStruct), CONF_PARTY_MRMP_UPDATE_CHANNEL_REQ);
    }
    else
    {
        seqNum = SendMsgToMpl((BYTE*)(pStruct), sizeof(MrmpOpenChannelRequestStruct), CONF_PARTY_MRMP_OPEN_CHANNEL_REQ);
    }
    pChannel->SetSeqNumCm(seqNum);
    PDELETE(pStruct);
}


////////////////////////////////////////////////////////////////////////////
void CH323Cntl::CloseSvcChannel(CChannel *pChannel)
{
    if (GetTargetMode()->GetConfMediaType() != eMixAvcSvc)
        return;

    // content channels should use the regular functions
    kChanneltype channelType = kSvcAvcChnlType;

    DWORD seqNum = 0;
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CloseSvcChannel - ConnectionId: ", m_pMrmpInterface->GetConnectionId());

    MrmpCloseChannelRequestStruct* pCloseStruct = new MrmpCloseChannelRequestStruct;
    MrmpCloseChannelRequestMessage*  pUdpCloseSt=&pCloseStruct->tMrmpCloseChannelRequestMessage;

    // ======== fill params

    // ----- channelHandle, m_channelDirection
    pUdpCloseSt->channelHandle=pChannel->GetChannelHandle();
    pUdpCloseSt->m_channelDirection = cmCapReceive;

    channelType = ::DataTypeToChannelType(pChannel->GetType(), pChannel->GetRoleLabel());
    TRACEINTO << "mix_mode: channelType = " << channelType << " media type = " << ::GetTypeStr(pChannel->GetType()) << " role = " << ::GetRoleStr(pChannel->GetRoleLabel());
    if (channelType == kIpAudioChnlType)
    {
        pUdpCloseSt->m_channelType = kAvcToSacChnlType;
        TRACEINTO << "mix_mode: channelType set to " << kAvcToSacChnlType;
    }
    else
    {
        pUdpCloseSt->m_channelType = kSvcAvcChnlType;
        TRACEINTO << "mix_mode: channelType set to " << kSvcAvcChnlType;
    }

    // ----- m_localAddress
    enIpVersion eIpAddrMatch = eIpVersion4;
    if (m_state == IP_CHANGEMODE)
        eIpAddrMatch = CheckForMatchBetweenPartyAndUdp(m_pH323NetSetup->GetIpVersion(), m_UdpAddressesParams.IpType);

    pUdpCloseSt->m_localAddress.ipVersion = eIpAddrMatch;

    bool bIsLocalAddressIpV4 = ( (pUdpCloseSt->m_localAddress.ipVersion == (APIU32)eIpVersion4) ? true : false );
    if (true == bIsLocalAddressIpV4) //v4
    {
        pUdpCloseSt->m_localAddress.addr.v4.ip = m_UdpAddressesParams.IpV4Addr.ip;
    }
    else
    {
        // --- UDP: array of addresses ---
        // First we will look for the best IpV6 address match -> Meaning we will match ScopeId's
        BYTE place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(
                m_pH323NetSetup->GetTaDestPartyAddr(),
                m_UdpAddressesParams.IpV6AddrArray);
        memcpy(pUdpCloseSt->m_localAddress.addr.v6.ip, m_UdpAddressesParams.IpV6AddrArray[place].ip, 16);
        pUdpCloseSt->m_localAddress.addr.v6.scopeId = m_UdpAddressesParams.IpV6AddrArray[place].scopeId;
    }

    pUdpCloseSt->m_localAddress.transportType = eTransportTypeUdp;
    pUdpCloseSt->m_localAddress.distribution  = eDistributionUnicast;// Don't Care
    pUdpCloseSt->m_localAddress.distribution = eDistributionUnicast;// Don't Care


    // ----- m_remoteAddress
    pUdpCloseSt->m_remoteAddress.ipVersion = eIpAddrMatch;

    bool bIsRemoteAddressIpV4 = ( (pUdpCloseSt->m_remoteAddress.ipVersion == (APIU32)eIpVersion4) ? true : false );
    if (true == bIsRemoteAddressIpV4) //v4
    {
        pUdpCloseSt->m_remoteAddress.addr.v4.ip = pChannel->GetRmtAddress()->addr.v4.ip;
    }
    else
    {
        memcpy(pUdpCloseSt->m_remoteAddress.addr.v6.ip, pChannel->GetRmtAddress()->addr.v6.ip, 16);
        pUdpCloseSt->m_remoteAddress.addr.v6.scopeId = pChannel->GetRmtAddress()->addr.v6.scopeId;
    }

    pUdpCloseSt->m_remoteAddress.transportType = eTransportTypeUdp;

    // send to trace (local and remote addresses)
    if (bIsLocalAddressIpV4 && bIsRemoteAddressIpV4)
    {
        TRACEINTOFUNC << "Local address type: IPv4, remote address type: IPv4";
    }

    else
    {
        TRACEINTOFUNC
            << "Local address type: " << ( bIsLocalAddressIpV4 ? "IPv4" : "IPv6" )
            << ", remote address type: " << ( bIsRemoteAddressIpV4 ? "IPv4" : "IPv6" );

        if (!bIsLocalAddressIpV4)
        {
            BYTE place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(
                    m_pH323NetSetup->GetTaDestPartyAddr(),
                    m_UdpAddressesParams.IpV6AddrArray);
        }
    }



    // ----- m_localRtcpAddress
    pUdpCloseSt->m_localRtcpAddress.ipVersion = eIpAddrMatch;

    bool bIsLocalRtcpAddressIpV4 = ( (pUdpCloseSt->m_localRtcpAddress.ipVersion == (APIU32)eIpVersion4) ? true : false );
    if (true == bIsLocalRtcpAddressIpV4) //v4
    {
        pUdpCloseSt->m_localRtcpAddress.addr.v4.ip = m_UdpAddressesParams.IpV4Addr.ip;
    }

    else
    {
        // --- UDP: array of addresses ---
        // First we will look for the best IpV6 address match -> Meaning we will match ScopeId's
        BYTE place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(
                m_pH323NetSetup->GetTaDestPartyAddr(),
                m_UdpAddressesParams.IpV6AddrArray);
        memcpy(pUdpCloseSt->m_localRtcpAddress.addr.v6.ip, m_UdpAddressesParams.IpV6AddrArray[place].ip, 16);
        pUdpCloseSt->m_localRtcpAddress.addr.v6.scopeId = m_UdpAddressesParams.IpV6AddrArray[place].scopeId;
    }

    pUdpCloseSt->m_localRtcpAddress.transportType = eTransportTypeUdp;
    pUdpCloseSt->m_localRtcpAddress.distribution  = eDistributionUnicast;// Don't Care
    pUdpCloseSt->m_localRtcpAddress.distribution = eDistributionUnicast;// Don't Care


    // ----- m_remoteRtcpAddress
    pUdpCloseSt->m_remoteRtcpAddress.ipVersion = eIpAddrMatch;

    bool bIsRemoteRtcpAddressIpV4 = ( (pUdpCloseSt->m_remoteRtcpAddress.ipVersion == (APIU32)eIpVersion4) ? true : false );
    if (bIsRemoteRtcpAddressIpV4) //v4
    {
        pUdpCloseSt->m_remoteRtcpAddress.addr.v4.ip = pChannel->GetRmtAddress()->addr.v4.ip;
    }
    else
    {
        memcpy(pUdpCloseSt->m_remoteRtcpAddress.addr.v6.ip, pChannel->GetRmtAddress()->addr.v6.ip, 16);
        pUdpCloseSt->m_remoteRtcpAddress.addr.v6.scopeId = pChannel->GetRmtAddress()->addr.v6.scopeId;
    }

    pUdpCloseSt->m_remoteRtcpAddress.transportType = eTransportTypeUdp;

    // send to trace (local and remote RTCP addresses)
    if (bIsLocalRtcpAddressIpV4 && bIsRemoteRtcpAddressIpV4)
    {
        TRACEINTOFUNC << "Local RTCP address type: IPv4, remote RTCP address type: IPv4";
    }

    else
    {
        TRACEINTOFUNC
            << "Local RTCP address type: " << ( bIsLocalRtcpAddressIpV4 ? "IPv4" : "IPv6" )
            << ", remote RTCP address type: " << ( bIsRemoteRtcpAddressIpV4 ? "IPv4" : "IPv6" );

        if (!bIsLocalAddressIpV4)
        {
            BYTE place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(
                    m_pH323NetSetup->GetTaDestPartyAddr(),
                    m_UdpAddressesParams.IpV6AddrArray);
        }

    }

    // ----- m_localAddress, m_remoteAddress, m_localRtcpAddress, m_remoteRtcpAddress
    cmCapDataType dataType = cmCapVideo;
    ERoleLabel eRole = pChannel->GetRoleLabel();

    switch (dataType)
    {
        case cmCapAudio:
        {
            pUdpCloseSt->m_localAddress.port  = m_UdpAddressesParams.AudioChannelPort;
            /*code review*/
            //pUdpCloseSt->m_remoteAddress.port = rmtAddress.port;
            pUdpCloseSt->m_remoteAddress.port = pChannel->GetRmtAddress()->port;
            pUdpCloseSt->m_localRtcpAddress.port              = m_UdpAddressesParams.AudioChannelPort + 1;
            /*code review*/
            //pUdpCloseSt->m_remoteRtcpAddress.port           = rmtRtcpPort;
            pUdpCloseSt->m_remoteRtcpAddress.port             = pChannel->GetRtcpRmtPort();
            break;
        }

        case cmCapVideo:
        {
            if (eRole == kRolePeople)
            {
                pUdpCloseSt->m_localAddress.port  = m_UdpAddressesParams.VideoChannelPort;
                /*code review*/
                //pUdpCloseSt->m_remoteAddress.port = rmtAddress.port;
                pUdpCloseSt->m_remoteAddress.port = pChannel->GetRmtAddress()->port;
                pUdpCloseSt->m_localRtcpAddress.port              = m_UdpAddressesParams.VideoChannelPort + 1;
                /*code review*/
                //pUdpCloseSt->m_remoteRtcpAddress.port           = rmtRtcpPort;
                pUdpCloseSt->m_remoteRtcpAddress.port             = pChannel->GetRtcpRmtPort();

            }
            else
            {
                pUdpCloseSt->m_localAddress.port  = m_UdpAddressesParams.ContentChannelPort;
                /*code review*/
                //pUdpCloseSt->m_remoteAddress.port = rmtAddress.port;
                pUdpCloseSt->m_remoteAddress.port = pChannel->GetRmtAddress()->port;
                pUdpCloseSt->m_localRtcpAddress.port              = m_UdpAddressesParams.ContentChannelPort + 1;
                /*code review*/
                //pUdpCloseSt->m_remoteRtcpAddress.port           = rmtRtcpPort;
                pUdpCloseSt->m_remoteRtcpAddress.port             = pChannel->GetRtcpRmtPort();
            }
            break;
        }

        default:
        {
            pUdpCloseSt->m_localAddress.port  = 0;
            pUdpCloseSt->m_remoteAddress.port = 0;
            pUdpCloseSt->m_localRtcpAddress.port              = 0;
            pUdpCloseSt->m_remoteRtcpAddress.port             = 0;
            break;
        }
    }

    memset(&(pCloseStruct->physicalId[0]), 0, sizeof(PHYSICAL_RESOURCE_INFO_S));//@#@ - physicalId
    pCloseStruct->m_allocatedPhysicalResources = 0;
    seqNum = SendMsgToMpl((BYTE*)(pCloseStruct), sizeof(MrmpCloseChannelRequestStruct), CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ);
    pChannel->SetSeqNumCm(seqNum);
    PDELETE(pCloseStruct);
}

////////////////////////////////////////////////////////////////////////////
// CONFPARTY_CM_CLOSE_UDP_PORT_REQ
void CH323Cntl::Cm_FillAndSendCloseUdpPortStruct(CChannel *pMcChannel)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::Cm_FillAndSendCloseUdpPortStruct - ConnectionId=", m_pMfaInterface->GetConnectionId());
  DWORD seqNum = 0;
  if (pMcChannel->GetCmUdpChannelState() == kNotSendOpenYet || pMcChannel->GetCmUdpChannelState() == kRecieveCloseAck)
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::Cm_FillAndSendCloseUdpPortStruct - OPEN_UDP was not sent yet or CLOSE_UDP_ACK was already received for ConnectionId=", m_pMfaInterface->GetConnectionId());
    return;
  }

  if (pMcChannel->GetCmUdpChannelState() == kRecieveOpenAck)
  {
    pMcChannel->SetCmUdpChannelState(kSendClose);

    TCloseUdpPortMessageStruct* pStruct = new TCloseUdpPortMessageStruct;
    memset(pStruct, 0, sizeof(TCloseUdpPortMessageStruct));

    mcReqCmCloseUdpPort* pUdpSt = &pStruct->tCmCloseUdpPort;
    pUdpSt->channelType = ::DataTypeToChannelType(pMcChannel->GetType(), pMcChannel->GetRoleLabel());
    pUdpSt->channelDirection = ::CalcCmCapDirection(pMcChannel->IsOutgoingDirection());
    pUdpSt->CmLocalUdpAddressIp.transportType = eUnknownTransportType;

    cmCapDataType dataType = pMcChannel->GetType();
    ERoleLabel eRole = pMcChannel->GetRoleLabel();
    WORD localUdpport = 0;
    switch (dataType)
    {
      case cmCapAudio:
        localUdpport = m_UdpAddressesParams.AudioChannelPort;
        break;
      case cmCapVideo:
        localUdpport = (eRole == kRolePeople) ? m_UdpAddressesParams.VideoChannelPort : m_UdpAddressesParams.ContentChannelPort;
        break;
      case cmCapData:
        localUdpport = m_UdpAddressesParams.FeccChannelPort;
        break;
      default:
        localUdpport = 0;
        break;
    }

    enIpVersion eIpAddrMatch = CheckForMatchBetweenPartyAndUdp(m_pH323NetSetup->GetIpVersion(), m_UdpAddressesParams.IpType);
    if (eIpAddrMatch == eIpVersion4)
    {
      (pUdpSt->CmLocalUdpAddressIp).ipVersion = eIpVersion4;
      (pUdpSt->CmLocalUdpAddressIp).addr.v4.ip = m_UdpAddressesParams.IpV4Addr.ip;
      (pUdpSt->CmLocalUdpAddressIp).port = localUdpport;
    }
    else
    {
      // --- UDP: array of addresses ---
      BYTE place = ::FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(m_pH323NetSetup->GetTaDestPartyAddr(), m_UdpAddressesParams.IpV6AddrArray);

      memcpy((pUdpSt->CmLocalUdpAddressIp).addr.v6.ip, m_UdpAddressesParams.IpV6AddrArray[place].ip, 16);
      (pUdpSt->CmLocalUdpAddressIp).port = localUdpport;
      (pUdpSt->CmLocalUdpAddressIp).ipVersion = eIpVersion6;

    }
    pUdpSt->LocalRtcpPort = pMcChannel->GetRtcpPort();

    memcpy(&(pUdpSt->CmRemoteUdpAddressIp), pMcChannel->GetRmtAddress(), sizeof(mcTransportAddress));
    pUdpSt->RemoteRtcpPort = pMcChannel->GetRtcpRmtPort();

    pUdpSt->ice_channel_rtp_id = 0;
    pUdpSt->ice_channel_rtcp_id = 0;

    memset(&(pStruct->physicalPort.physical_id), 0, sizeof(PHYSICAL_RESOURCE_INFO_S));
    pStruct->physicalPort.connection_id = m_pMfaInterface->GetConnectionId();
    pStruct->physicalPort.party_id = m_pCsRsrcDesc->GetPartyRsrcId();

    ///////////////////////////////////////////////
    // in case of downgrade to AudioOnly
    if(kIpVideoChnlType == pStruct->tCmCloseUdpPort.channelType)
    {
    	CCall* pCall = GetCallParams();
		if(pCall && cmCapTransmit ==pStruct->tCmCloseUdpPort.channelDirection)
		{
		   pCall->SetMediaDetectioHasVideo(FALSE);
		}
    }

    seqNum = SendMsgToMpl((BYTE*)(pStruct), sizeof(TCloseUdpPortMessageStruct), CONFPARTY_CM_CLOSE_UDP_PORT_REQ);
    pMcChannel->SetSeqNumCm(seqNum);
    PDELETE(pStruct);
  }
  else if (pMcChannel->GetCmUdpChannelState() != kSendClose)
  {
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::Cm_FillAndSendCloseUdpPortStruct UdpChannelState - ", pMcChannel->GetCmUdpChannelState());
    pMcChannel->SetCmUdpChannelState(kNeedsToBeClosed);
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnMfaAckDisconnectInternalArt(CSegment* pParam)
{
  ACK_IND_S* pAckIndStruct = new ACK_IND_S;
  if(pAckIndStruct)
  {
	  *pParam >> pAckIndStruct->ack_base.ack_opcode
	  	  	  >> pAckIndStruct->ack_base.ack_seq_num
	  	  	  >> pAckIndStruct->ack_base.status
	  	  	  >> pAckIndStruct->ack_base.reason
	  	  	  >> pAckIndStruct->media_type
	  	  	  >> pAckIndStruct->media_direction;
	  DWORD InternqalProblemOpcode = 0;

	  TRACEINTO << "mix_mode: ACK opcode = " << pAckIndStruct->ack_base.ack_opcode << " m_state =" << m_state;

	  *pParam >> pAckIndStruct->channelHandle;
	  ConnectionID connectionId;
	  *pParam >> connectionId;
	  TRACEINTO << "mix_mode: ConnectionId = " << connectionId;

	  InternalTranslatorInterface* curTranslatorInterface=NULL;
	  if(pAckIndStruct->ack_base.ack_opcode == TB_MSG_CLOSE_PORT_REQ   )
	  {

		ReduceReqCounter(pAckIndStruct);  // to close:index  0 or index 1
		if (GetArtInterfaceByConnId(connectionId, curTranslatorInterface))
		{
			curTranslatorInterface->state=STATE_OFF; // // ey_20866 this is a bug and it's temporary - should decide which one
			TRACEINTOFUNC <<"mix_mode: curTranslatorInterface->state:"<<(curTranslatorInterface->state==STATE_ON ? "STATE_ON":"STATE_OFF");
			 if(AreAllInternalArtsDisconnected())
			 {
				TRACEINTOFUNC << "mix_mode: Received ACK for TB_MSG_CLOSE_PORT_REQ all ARTS are closed";
				m_pTaskApi->IPPartyInternalArtsDisconnected();
			 }
		}
	  }
	  else
	  {
		  TRACEINTOFUNC << "Ignore ACK opcode = " << pAckIndStruct->ack_base.ack_opcode;
	  }

	  POBJDELETE(pAckIndStruct);
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnMfaAck(CSegment* pParam)
{
  ACK_IND_S* pAckIndStruct = new ACK_IND_S;
  *pParam >> pAckIndStruct->ack_base.ack_opcode
      >> pAckIndStruct->ack_base.ack_seq_num
      >> pAckIndStruct->ack_base.status
      >> pAckIndStruct->ack_base.reason
      >> pAckIndStruct->media_type
      >> pAckIndStruct->media_direction;
  DWORD InternqalProblemOpcode = 0;

  if (pAckIndStruct->ack_base.ack_opcode == H323_RTP_UPDATE_MT_PAIR_REQ ||
      pAckIndStruct->ack_base.ack_opcode == VIDEO_DECODER_UPDATE_PARAM_REQ ||
      pAckIndStruct->ack_base.ack_opcode == IP_RTP_SET_FECC_PARTY_TYPE)
  {
    PDELETE(pAckIndStruct);
    return; //ack on this message is not handled at MCMS
  }

  TRACEINTO << "mix_mode: ACK opcode = " << pAckIndStruct->ack_base.ack_opcode;

  *pParam >> pAckIndStruct->channelHandle;
  ConnectionID connectionId;
  *pParam >> connectionId;
  TRACEINTO << "mix_mode: ConnectionId = " << connectionId;

  InternalTranslatorInterface* curTranslatorInterface=NULL;
  if (pAckIndStruct->ack_base.ack_opcode == TB_MSG_OPEN_PORT_REQ)
  {
      TRACEINTOFUNC << "mix_mode: Received ACK for open ART";
      ReduceReqCounter(pAckIndStruct);
      if (GetArtInterfaceByConnId(connectionId, curTranslatorInterface))
      {
        curTranslatorInterface->state = STATE_ON;
        TRACEINTOFUNC << "mix_mode: curTranslatorInterface->state:" << (curTranslatorInterface->state==STATE_ON ? "STATE_ON":"STATE_OFF");
        if (AreAllInternalArtsConnected())
        {
            TRACEINTOFUNC << "mix_mode: Received ACK for TB_MSG_OPEN_PORT_REQ - all ARTS are opened.";
            m_pTaskApi->IPPartyInternalArtsConnected();

        }
    }
    return;
  }
  else if(pAckIndStruct->ack_base.ack_opcode == TB_MSG_CLOSE_PORT_REQ 	)
  {

      ReduceReqCounter(pAckIndStruct);
    if (GetArtInterfaceByConnId(connectionId, curTranslatorInterface))
    {
        curTranslatorInterface->state=STATE_OFF; // // ey_20866 this is a bug and it's temporary - should decide which one
        TRACEINTOFUNC <<"mix_mode: curTranslatorInterface->state:"<<(curTranslatorInterface->state==STATE_ON ? "STATE_ON":"STATE_OFF");
        if (m_state!=IP_DISCONNECTING && m_state!=DISCONNECT)
        {
            // remove art from resource table
            RemoveArtByConnId(connectionId);

            // if all disconnecting arts are already disconnected, notify party
            if (!IsAtLeastOneInternalArtDisconnecting())
                m_pTaskApi->IPPartyInternalArtsDisconnected();
        }
        else if (AreAllInternalArtsDisconnected())
         {
            TRACEINTOFUNC << "mix_mode: Received ACK for TB_MSG_CLOSE_PORT_REQ all ARTS are closed";
            m_pTaskApi->IPPartyInternalArtsDisconnected();
         }
    }
    return;

  }
//  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnMfaAck - Received ACK on ", pAckIndStruct->ack_base.ack_opcode);


  BYTE bIsTransmiting = CalcIsTransmiting((cmCapDirection)pAckIndStruct->media_direction);
  CChannel* pChannel = NULL;
  ERoleLabel eRole  = kRolePeople;
  cmCapDataType eDataType = ::ChannelTypeToDataType((kChanneltype)pAckIndStruct->media_type, eRole);

  // check MRMP channel
  if (pAckIndStruct->ack_base.ack_opcode == CONF_PARTY_MRMP_OPEN_CHANNEL_REQ)
  {
      TRACEINTO <<  "mix_mode: ack_opcode=CONF_PARTY_MRMP_OPEN_CHANNEL_REQ pAckIndStruct->channelHandle=" << pAckIndStruct->channelHandle << "bIsTransmiting=" << (int)bIsTransmiting;
      CChannel* pMrmpChannel = FindChannelInList(eDataType, bIsTransmiting, eRole, false);

      if (pMrmpChannel)
      {
          pMrmpChannel->SetCsChannelState(kConnectedState);
          if (pAckIndStruct->channelHandle != INVALID_CHANNEL_HANDLE)
          {
              pMrmpChannel->SetChannelHandle(pAckIndStruct->channelHandle);
              TRACEINTO << "mix_mode: Setting MRMP channel handle to " << pAckIndStruct->channelHandle;
              pMrmpChannel->Dump("mix_mode: Setting MRMP channel");
          }
      }
      ReduceReqCounter(pAckIndStruct);
      PDELETE(pAckIndStruct);
      if (m_pParty->GetChangeModeState() == eConfRequestMoveToMixed)
      {// this is upgrade
    	  ConnectPartyToConfWhenUpgradeToMix();
      }
      else
      {
          ConnectPartyToConf();
      }
      return;
  }

  // check MRMP update channel
  if (pAckIndStruct->ack_base.ack_opcode == CONF_PARTY_MRMP_UPDATE_CHANNEL_REQ)
  {
      TRACEINTO <<  "mix_mode: ack_opcode=CONF_PARTY_MRMP_UPDATE_CHANNEL_REQ pAckIndStruct->channelHandle=" << pAckIndStruct->channelHandle;
      CChannel* pMrmpChannel = FindChannelInList(eDataType, bIsTransmiting, eRole, false);
      if (pMrmpChannel)
      {
          pMrmpChannel->SetCsChannelState(kConnectedState);
      }
      ReduceReqCounter(pAckIndStruct);
      PDELETE(pAckIndStruct);
      return;
  }

  //below code can be used in channel closing
  if (pAckIndStruct->ack_base.ack_opcode == CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ)
  {
      TRACEINTOFUNC << "mix_mode: Received ACK for MRMP channel m_isCallDropRequestWaiting=" << (int)m_isCallDropRequestWaiting;
      int i = m_pmcCall->GetChannelIndexInList(false, eDataType,bIsTransmiting, eRole);
      CChannel* pMrmpChannel = m_pmcCall->GetSpecificChannel(i, false);
      if (pMrmpChannel)
      {
          pMrmpChannel->SetCsChannelState(kDisconnectedState);
          m_pmcCall->RemoveChannelInternal(i);
      }
      ReduceReqCounter(pAckIndStruct);
      PDELETE(pAckIndStruct);

      // BRIDGE-9448
      if (!bIsTransmiting && eRole == kRolePeople)
      {
          CChannel* pExternalUdpChannel = FindChannelInList(eDataType, FALSE, kRolePeople, true);

	      if (pExternalUdpChannel
		  	&& pExternalUdpChannel->GetCsChannelState() == kWaitToSendChannelDrop
		  	&& pExternalUdpChannel->GetCmUdpChannelState() == kRecieveCloseAck)
	      {
	          SendChannelDropReq(pExternalUdpChannel);

	          if(m_pmcCall->GetIsClosingProcess() == TRUE)
	            pExternalUdpChannel->SetCsChannelState(kCheckSendCallDrop);
	      }
      }

      if (m_isCallDropRequestWaiting || m_pmcCall->GetIsClosingProcess())
        SendCallDropIfNeeded();

      //BRIDGE-11174
	  if (m_pmcCall->GetCallCloseInitiator() == PmInitiator && m_pmcCall->GetIsClosingProcess())
		OnPartyCallCloseConfirmReqIfNeeded();
	  else if (m_isCallDropRequestWaiting || m_pmcCall->GetIsClosingProcess())
		SendCallDropIfNeeded();

      return;
  }

  kChanneltype channelType = (kChanneltype)pAckIndStruct->media_type;
  bool isInternalChannel = false;
  if (channelType == kAvcToSacChnlType)
  {
      pChannel = m_pmcCall->GetInternalChannel(eDataType, (BOOL)bIsTransmiting, eRole);
      isInternalChannel = true;
  }
  else  if (IsInternalArtConnId(connectionId))
  {// these are channels from avcToSvcTranslator ARTs
      pChannel = m_pmcCall->GetInternalChannel(eDataType, (BOOL)bIsTransmiting, eRole, connectionId);
      isInternalChannel = true;
  }
  else
  {
      pChannel = FindChannelInList(eDataType, bIsTransmiting, eRole, true);
  }


  if (!pChannel)
  {
    if ((pAckIndStruct->ack_base.ack_opcode == ART_CONTENT_OFF_REQ && pAckIndStruct->ack_base.status == STATUS_OK) ||
        ((pAckIndStruct->ack_base.ack_opcode == H323_RTP_LPR_MODE_CHANGE_REQ
          || pAckIndStruct->ack_base.ack_opcode == H323_RTP_LPR_MODE_RESET_REQ) && pAckIndStruct->ack_base.status == STATUS_OK))
    {
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnMfaAck - Fowarding ART_CONTENT_OFF_REQ/LPR ACKSS even if channel is closed ",m_pMfaInterface->GetConnectionId());
      if (pAckIndStruct->ack_base.ack_opcode == ART_CONTENT_OFF_REQ)
      m_eContentInState = eNoChannel;
    }
    else
    {
      TRACEINTO << "CH323Cntl::OnMfaAck: channel wasn't found - Opcode = " << pAckIndStruct->ack_base.ack_opcode <<
           ", Media Type = " << pAckIndStruct->media_type <<
           ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();
      ReduceReqCounter(pAckIndStruct);
      DBGPASSERT(pAckIndStruct->media_type);
      PDELETE(pAckIndStruct);
      return;
    }
  }


  BYTE bRejectChannel = FALSE;
  CMedString message;
  //
  // In case status is not OK - We need to inform the partyCntl - ON(m_isFaulty)
  if (pAckIndStruct->ack_base.status != STATUS_OK)
  {
    // Setting the digits for the MCU_INTERNAL_PROBLEM
    MipHardWareConn mipHwConn;
    MipMedia    mipMedia;
    MipDirection  mipDirect;
    MipTimerStatus  mipTimerStat;
    MipAction   mipAction;

    if (pAckIndStruct->ack_base.ack_opcode == ART_CONTENT_ON_REQ || pAckIndStruct->ack_base.ack_opcode == ART_CONTENT_OFF_REQ ||
      pAckIndStruct->ack_base.ack_opcode == ART_EVACUATE_REQ || pAckIndStruct->ack_base.ack_opcode == H323_RTP_LPR_MODE_CHANGE_REQ
      || pAckIndStruct->ack_base.ack_opcode == H323_RTP_LPR_MODE_RESET_REQ || pAckIndStruct->ack_base.ack_opcode == H323_RTP_FECC_TOKEN_RESPONSE_REQ)
    {
      TRACEINTO << "CH323Cntl::OnMfaAck: Do not send faulty rsrc to pc level - Opcode = " << pAckIndStruct->ack_base.ack_opcode;
    }
    else
    {
      TranslateAckToMipErrorNumber(mipHwConn, mipMedia, mipDirect, mipTimerStat, mipAction, pAckIndStruct,pChannel);
      char mipNum[200];
      memset(mipNum, '\0', 200);
      InternqalProblemOpcode = (mipHwConn*10000) + (mipMedia*1000) + (mipDirect*100) + (mipTimerStat*10) + mipAction;
      if(mipHwConn == 2 /*RTP*/)
      {
        sprintf(mipNum," mipHwConn = %d mipMedia = %d mipDirect = %d  mipTimerStat = %d mipAction = %d ReqNum = %d internalMcuOpcode = %d"
            ,(BYTE)mipHwConn,(BYTE)mipMedia,(BYTE)mipDirect,(BYTE)mipTimerStat, (BYTE)mipAction, (DWORD)pChannel->GetSeqNumRtp(),(DWORD)InternqalProblemOpcode);
        pChannel->SetSeqNumRtp(0);
      }
      else
      {
        sprintf(mipNum," mipHwConn = %d mipMedia = %d mipDirect = %d  mipTimerStat = %d mipAction = %d ReqNum = %d internalMcuOpcode = %d"
              ,(BYTE)mipHwConn,(BYTE)mipMedia,(BYTE)mipDirect,(BYTE)mipTimerStat, (BYTE)mipAction, (DWORD)pChannel->GetSeqNumCm(),(DWORD)InternqalProblemOpcode);
        pChannel->SetSeqNumCm(0);

      }
      PTRACE2(eLevelInfoNormal,"CH323Cntl::OnMfaAck : ", mipNum);
      m_pTaskApi->SetFaultyResourcesToPartyControlLevel(STATUS_FAIL, mipHwConn, mipMedia, mipDirect, mipTimerStat, mipAction);
    }

    DBGPASSERT(m_pmcCall->GetConnectionId());
    static const CProcessBase * process = CProcessBase::GetProcess();
    const std::string &str = process->GetOpcodeAsString(pAckIndStruct->ack_base.ack_opcode);
    message << "CH323Cntl::OnMfaAck : " << str.c_str() << " - status is NOT ok -  Media Type = " << pAckIndStruct->media_type <<
                  ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();
//    CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, pAckIndStruct->ack_base.ack_opcode, MAJOR_ERROR_LEVEL, message.GetString(), TRUE);
    TRACEINTO << message.GetString();
  }

  //this function will also set the seq num to 0 and this is why we call it after we checked the seq num in a case of failed ack!
  ReduceReqCounter(pAckIndStruct);
  if ((pAckIndStruct->ack_base.ack_opcode == H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ))
  {
    if (pAckIndStruct->ack_base.status == STATUS_OK)
    {
      TRACEINTO << "CH323Cntl::OnMfaAck : H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ - status is ok -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << connectionId;
      CObjString cLog;
      cLog << "H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ - status is ok -  Media Type = ";
      ::GetChannelTypeName(pAckIndStruct->media_type,cLog);
      TRACEINTO << cLog.GetString() << ", Direction = " << ::GetDirectionStr((cmCapDirection)pAckIndStruct->media_direction) << ", ConnId = " << connectionId;

      if (pChannel->GetRtpPortChannelState() == kRtpPortOpenSent)
        pChannel->SetRtpPortChannelState(kRtpPortReceivedOpenAck);
      else if(pChannel->GetRtpPortChannelState() == kRtpPortUpdateSent)
        pChannel->SetRtpPortChannelState(kRtpPortReceivedUpdateAck);
      else
      {
          pChannel->Dump("ACK received in wrong RTP state");
        pChannel->SetRtpPortChannelState(kRtpPortReceivedUpdateAck);  //not correct bug just to not add risk to ver 7
        PASSERTMSG(pAckIndStruct->media_direction, "CH323Cntl::OnMfaAck : ack received in wrong state");

      }

      if (isInternalChannel)
      {
          pChannel->SetCsChannelState(kConnectedState);
          pChannel->Dump("mix_mode: OnMfaAck Setting CsChannelState to connected");
          if (m_pParty->GetChangeModeState() == eConfRequestMoveToMixed)
          {// this is upgrade
        	  ConnectPartyToConfWhenUpgradeToMix();
          }
          else
        	  ConnectPartyToConf();
          return;
      }

//      eContentState eTempContentInState = m_eContentInState;
      if (pChannel->GetCmUdpChannelState() == kRecieveOpenAck)
      {
        if ((cmCapDirection)pAckIndStruct->media_direction == cmCapTransmit)
        {
          BYTE bIsNewChannel = TRUE;
          if((kChanneltype)pAckIndStruct->media_type == kIpAudioChnlType)
            ON(m_isAudioOutgoingChannelConnected); //outgoing channel is connected only after CM and RTP response with Ack.
          if((kChanneltype)pAckIndStruct->media_type == kIpVideoChnlType)
            ON(m_isVideoOutgoingChannelConnected); //outgoing channel is connected only after CM and RTP response with Ack.
          if((kChanneltype)pAckIndStruct->media_type == kIpFeccChnlType)
            ON(m_isDataOutgoingChannelConnected); //outgoing channel is connected only after CM and RTP response with Ack.
          if((kChanneltype)pAckIndStruct->media_type == kIpContentChnlType)
          {
	     	PTRACE(eLevelInfoNormal,"CH323Cntl::OnMfaAck - Ack on content out stream");
            if (pChannel->GetStreamState() == kStreamUpdate)
            {
              PTRACE(eLevelInfoNormal,"pChannel->GetStreamState() == kStreamUpdate");
              pChannel->SetStreamState(kStreamOnState);
              m_pTaskApi->MfaUpdatedPresentationOutStream();
              bIsNewChannel = FALSE; //stream was only updated
            }
            else
                        {
                            ON(m_isVideoContentOutgoingChannelConnected); //outgoing channel is connected only after CM and RTP response with Ack.
                            if(m_isContentOn)
                            {
                              	  PTRACE(eLevelInfoNormal,"CH323Cntl::OnMfaAck : m_isContentOn = On");
                               	  m_pTaskApi->MfaUpdatedPresentationOutStream();

                            }
                            else
                               	PTRACE(eLevelInfoNormal,"CH323Cntl::OnMfaAck : m_isContentOn = Off");
                        }

          }
          if (bIsNewChannel)
            OnChannelConnected(pChannel);
                    else if (eChangeOutgoing == m_pParty->GetChangeModeState()  && (kChanneltype)pAckIndStruct->media_type == kIpVideoChnlType )
                        ConnectPartyToConf(); // New Change mode - only Update RTP
        }

        else //receive
          OnPartyIncomingChnlResponseReq(pChannel, bRejectChannel);
      }
			else
			{
				PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnMfaAck - Channel Udp state = ", pChannel->GetCmUdpChannelState());
				if ((cmCapDirection)pAckIndStruct->media_direction == cmCapTransmit)
				{
					if((kChanneltype)pAckIndStruct->media_type == kIpContentChnlType)
					{
						PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnMfaAck - StreamState() =",pChannel->GetStreamState());
						//ON(m_isVideoContentOutgoingChannelConnected); //outgoing channel is connected only after CM and RTP response with Ack. - ????? Is needev here
						if(m_isContentOn)
						{
						  	  PTRACE(eLevelInfoNormal,"CH323Cntl::OnMfaAck : m_isContentOn = On");
						   	  m_pTaskApi->MfaUpdatedPresentationOutStream();

						}
					}
				}
      }
      // This means we sent CONTENT_ON on the fly (When we opened the channel - Tandberg case).
/*      if ((kChanneltype)pAckIndStruct->media_type == kIpContentChnlType && (cmCapDirection)pAckIndStruct->media_direction == cmCapReceive && m_eContentInState == eSendStreamOn )
      {
        TRACEINTO << "CH323Cntl::OnMfaAck : H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ- Incoming content channel on the fly";
        eTempContentInState = eStreamOn;
        m_pTaskApi->SendContentOnOffAck(pAckIndStruct->ack_base.status,eTempContentInState);
      }*/
    }
    else
    {
      if ((cmCapDirection)pAckIndStruct->media_direction == cmCapReceive)
      {
        bRejectChannel = TRUE;
        OnPartyIncomingChnlResponseReq(pChannel, bRejectChannel);
      }
    }
  }

  else if (pAckIndStruct->ack_base.ack_opcode == CONFPARTY_CM_OPEN_UDP_PORT_REQ)
  {
    ECmUdpChannelState eUdpChannelState = pChannel->GetCmUdpChannelState();
    if ((eUdpChannelState != kSendOpen) && (eUdpChannelState != kNeedsToBeClosed))
    {
      TRACEINTO << "CH323Cntl::OnMfaAck : CONFPARTY_CM_OPEN_UDP_PORT_REQ - we didn't wait for ack -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId() <<
                    ", eUdpChannelState = " << eUdpChannelState;
    }

    else
    {
      if (pAckIndStruct->ack_base.status == STATUS_OK)
      {
        TRACEINTO << "CH323Cntl::OnMfaAck : CONFPARTY_CM_OPEN_UDP_PORT_REQ - status is ok -  Media Type = " << pAckIndStruct->media_type <<
                      ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();

        BOOL bNeedsToBeClosed = (pChannel->GetCmUdpChannelState() == kNeedsToBeClosed);
        pChannel->SetCmUdpChannelState(kRecieveOpenAck);

        if(bNeedsToBeClosed)
        {
          Cm_FillAndSendCloseUdpPortStruct(pChannel);
          if (pChannel->GetRoleLabel() == kRolePeople)
          {
              CloseInternalChannels(pChannel->GetType());
          }

/*          if(!bIsTransmiting)
          {
            bRejectChannel = TRUE;
            OnPartyIncomingChnlResponseReq(pChannel, bRejectChannel);
          }*/
        }
        else if (pChannel->GetRtpPortChannelState() == kRtpPortReceivedOpenAck || pChannel->GetRtpPortChannelState() == kRtpPortReceivedUpdateAck)
        {
          if ((cmCapDirection)pAckIndStruct->media_direction == cmCapTransmit)
          {
            if((kChanneltype)pAckIndStruct->media_type == kIpAudioChnlType)
              ON(m_isAudioOutgoingChannelConnected); //outgoing channel is connected only after CM and RTP response with Ack.
            if((kChanneltype)pAckIndStruct->media_type == kIpVideoChnlType)
              ON(m_isVideoOutgoingChannelConnected); //outgoing channel is connected only after CM and RTP response with Ack.
            if((kChanneltype)pAckIndStruct->media_type == kIpFeccChnlType)
              ON(m_isDataOutgoingChannelConnected); //outgoing channel is connected only after CM and RTP response with Ack.
            if((kChanneltype)pAckIndStruct->media_type == kIpContentChnlType)
                        {
              ON(m_isVideoContentOutgoingChannelConnected); //outgoing channel is connected only after CM and RTP response with Ack.
                        }

            OnChannelConnected(pChannel);
          }
          else //receive
            OnPartyIncomingChnlResponseReq(pChannel, bRejectChannel);
        }
      }
      else
      {
        pChannel->SetCmUdpChannelState(kNotSendOpenYet);

        PTRACE2INT(eLevelInfoNormal,"::OnMfaAck : CONFPARTY_CM_OPEN_UDP_PORT_REQ - status is not ok - UdpChannelState - ", pChannel->GetCmUdpChannelState());

        if ((cmCapDirection)pAckIndStruct->media_direction == cmCapReceive)
        {
          bRejectChannel = TRUE;
          OnPartyIncomingChnlResponseReq(pChannel, bRejectChannel);
        }

        if(m_isCallDropRequestWaiting)
          SendCallDropIfNeeded();
      }
    }
  }

  else if (pAckIndStruct->ack_base.ack_opcode == CONFPARTY_CM_CLOSE_UDP_PORT_REQ)
  {
    if (pAckIndStruct->ack_base.status == STATUS_OK)
      TRACEINTO << "CH323Cntl::OnMfaAck : CONFPARTY_CM_CLOSE_UDP_PORT_REQ - status is ok -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();

    //if we don't wait for this ack to send CHANNEL_DROP that means that we wait for this ack to send CALL_DROP
    ECmUdpChannelState eUdpChannelState = pChannel->GetCmUdpChannelState();

    if (eUdpChannelState == kSendClose)
    {
      pChannel->SetCmUdpChannelState(kRecieveCloseAck);

      if (pChannel->GetCsChannelState() == kWaitToSendChannelDrop)
      {
        // BRIDGE-9448
        BYTE bToWaitMrmp = FALSE;

        if (!bIsTransmiting && eRole == kRolePeople)
        {
          CChannel* pMrmpChannel = FindChannelInList(eDataType, FALSE, kRolePeople, false);

          if (pMrmpChannel)
          {
            bToWaitMrmp = TRUE;
          }
        }

        if (bToWaitMrmp)
        {
          TRACEINTO << "MRMP channel is not closed yet, will send channel drop when it's closed.";
        }
        else
        {
          SendChannelDropReq(pChannel);
          if(m_pmcCall->GetIsClosingProcess() == TRUE)
            pChannel->SetCsChannelState(kCheckSendCallDrop);
        }
      }
      else
      {
        if (pChannel->GetCsChannelState() == kNoNeedToDisconnect  || pChannel->GetCsChannelState() == kDisconnectedState)
          m_pmcCall->RemoveChannel(pChannel);
        if(m_pmcCall->GetIsClosingProcess() == TRUE)
          SendCallDropIfNeeded();
      }
    }
    else
      TRACEINTO << "CH323Cntl::OnMfaAck : CONFPARTY_CM_CLOSE_UDP_PORT_REQ - we didn't wait for ack -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId() <<
                    ", eUdpChannelState = " << eUdpChannelState;
  }
  else if (pAckIndStruct->ack_base.ack_opcode == ART_CONTENT_ON_REQ || pAckIndStruct->ack_base.ack_opcode == ART_CONTENT_OFF_REQ)
  {
    eContentState eTempContentInState = m_eContentInState;
    if (pAckIndStruct->ack_base.status == STATUS_OK)
    {
      TRACEINTO << "CH323Cntl::OnMfaAck : ART_CONTENT_ON_REQ/ART_CONTENT_OFF_REQ - status is ok -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();

      if( m_pParty->IsCallGeneratorParty() )
        return;

      if (pAckIndStruct->media_direction != cmCapReceive)
        PASSERTMSG(pAckIndStruct->media_direction, "CH323Cntl::OnMfaAck : ART_CONTENT_ON_REQ/ART_CONTENT_OFF_REQ - Wrong direction");
      // Here we will update the stream state accordingly and send a message to the party saying the the ack was received

      if (pAckIndStruct->media_direction == cmCapReceive)
      {
        if (m_eContentInState != eNoChannel)
        {
          if (pAckIndStruct->ack_base.ack_opcode == ART_CONTENT_ON_REQ)
            eTempContentInState = eStreamOn;
          else
            eTempContentInState = eStreamOff;
        }
        if (pAckIndStruct->ack_base.ack_opcode == ART_CONTENT_ON_REQ)
          TRACEINTO << "CH323Cntl::OnMfaAck : ART_CONTENT_ON_REQ - eTempContentInState = " << m_eContentInState;
        else
          TRACEINTO << "CH323Cntl::OnMfaAck : ART_CONTENT_OFF_REQ - eTempContentInState = " << m_eContentInState;
      }

    }
    else
    {
      TRACEINTO << "CH323Cntl::OnMfaAck : ART_CONTENT_ON_REQ/ART_CONTENT_OFF_REQ - status is NOT ok -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();
      DBGPASSERT(m_pmcCall->GetConnectionId());
    }

    m_pTaskApi->SendContentOnOffAck(pAckIndStruct->ack_base.status,eTempContentInState);
  }
  else if (pAckIndStruct->ack_base.ack_opcode == ART_EVACUATE_REQ)
  {
    if (pAckIndStruct->ack_base.status == STATUS_OK)
    {
      TRACEINTO << "CH323Cntl::OnMfaAck : ART_EVACUATE_REQ - status is ok -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();
    }
    else
    {
      TRACEINTO << "CH323Cntl::OnMfaAck : ART_EVACUATE_REQ - status is NOT ok -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();
      DBGPASSERT(m_pmcCall->GetConnectionId());
    }
    if (m_eContentInState == eStreamOff || m_eContentInState == eSendStreamOff)
      m_pTaskApi->SendContentEvacuateAck(pAckIndStruct->ack_base.status);
  }
  else if (pAckIndStruct->ack_base.ack_opcode == H323_RTP_LPR_MODE_CHANGE_REQ) // LPR
  {
    if (pAckIndStruct->ack_base.status == STATUS_OK)
    {
      TRACEINTO << "CH323Cntl::OnMfaAck : H323_RTP_LPR_MODE_CHANGE_REQ - status is ok -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();
      if (IsValidTimer(LPRTOUT))
          DeleteTimer(LPRTOUT);

      // if m_LprModeTimeout == 0 No timer is needed since the LPR is permanent
      if (m_LprModeTimeout)
        StartTimer(LPRTOUT,  m_LprModeTimeout*SECOND);
      else
            {
                DWORD lprDefaultTimeOut = GetSystemCfgFlagInt<DWORD>("LPR_ACTIVITY_MAX_DURATION_IN_SECONDS");
                if (lprDefaultTimeOut)
                    StartTimer(LPRTOUT,  lprDefaultTimeOut*SECOND);
            }


      SendLprAckToParty();

    }
    else
    {
      TRACEINTO << "CH323Cntl::OnMfaAck : H323_RTP_LPR_MODE_CHANGE_REQ - status is NOT ok -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();
      DBGPASSERT(m_pmcCall->GetConnectionId());
    }
  }
  else if (pAckIndStruct->ack_base.ack_opcode == H323_RTP_LPR_MODE_RESET_REQ) // LPR
  {
    if (pAckIndStruct->ack_base.status == STATUS_OK)
    {
      TRACEINTO << "CH323Cntl::OnMfaAck : H323_RTP_LPR_MODE_RESET_REQ - status is ok -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();

      MfaH323LprAck();
    }
    else
    {
      TRACEINTO << "CH323Cntl::OnMfaAck : H323_RTP_LPR_MODE_RESET_REQ - status is NOT ok -  Media Type = " << pAckIndStruct->media_type <<
                    ", Direction = " << pAckIndStruct->media_direction << ", ConnId = " << m_pMfaInterface->GetConnectionId();
      DBGPASSERT(m_pmcCall->GetConnectionId());
    }
  }

  PDELETE(pAckIndStruct);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::TranslateAckToMipErrorNumber(MipHardWareConn& mipHwConn, MipMedia& mipMedia, MipDirection& mipDirect,
                        MipTimerStatus& mipTimerStat, MipAction& mipAction, ACK_IND_S* pAckIndStruct,
                         CChannel* pChannel, BYTE isClearTimer)
{
  CLargeString cstr;
  CLargeString responsibilityStr;

  DWORD faultOpcode =0;

  cstr << "Party:" << m_pCsRsrcDesc->GetPartyRsrcId() << " Conf:" << m_pCsRsrcDesc->GetConfRsrcId();
  CProcessBase *pProcess = CProcessBase::GetProcess();
  DWORD opcode = 0;

  if (pAckIndStruct != NULL)
  {
    opcode =pAckIndStruct->ack_base.ack_opcode;
    faultOpcode = ACK_FAILED;
    cstr << " receives Failure Status for opcode: ";
    switch(pAckIndStruct->ack_base.ack_opcode)
    {

      case CONFPARTY_CM_OPEN_UDP_PORT_REQ:
      {
        mipHwConn = eMipUdp;
        mipAction = eMipOpen;
        break;
      }
      case CONFPARTY_CM_CLOSE_UDP_PORT_REQ:
      {
        mipHwConn = eMipUdp;
        mipAction = eMipClose;
        break;
      }
      case H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
      {
        mipHwConn = eMipRtp;
        if (pChannel->GetRtpPortChannelState() == kRtpPortOpenSent)
          mipAction = eMipOpen;
        else
          mipAction = eMipUpdate;

        break;
      }
      default:
      {
        mipHwConn = eMipNoneHw;
        mipAction = eMipNoAction;
      }
    }

    mipMedia = (MipMedia)pAckIndStruct->media_type;
    mipDirect = (MipDirection)pAckIndStruct->media_direction;
    mipTimerStat = eMipStatusFail;

  }
  else if (pChannel != NULL)
  {

    if (isClearTimer)
    {
      mipHwConn = eMipUdp;
      mipAction = eMipDisconnect;
    }
    else if (pChannel->GetRtpPortChannelState() == kRtpPortOpenSent)
    {
      opcode = H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ;
      mipHwConn = eMipRtp;
      mipAction = eMipOpen;
    }
    else if (pChannel->GetCmUdpChannelState() == kSendOpen || pChannel->GetCmUdpChannelState() == kSendClose)
    {
      mipHwConn = eMipUdp;
      if (pChannel->GetCmUdpChannelState() == kSendOpen)
      {
        opcode = CONFPARTY_CM_OPEN_UDP_PORT_REQ;
        mipAction = eMipOpen;
      }
      else
      {
        opcode = CONFPARTY_CM_CLOSE_UDP_PORT_REQ;
        mipAction = eMipDisconnect;
      }

    }
    else if (pChannel->GetRtpPortChannelState() == kRtpPortUpdateSent)
    {
      opcode = H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ;
      mipHwConn = eMipRtp;
      mipAction = eMipUpdate;
    }
    switch (pChannel->GetType())
    {
      case cmCapAudio:
      {
        mipMedia = eMipAudio;
        break;
      }
      case cmCapVideo:
      {
        if (pChannel->GetRoleLabel() == kRolePeople)
          mipMedia = eMipVideo;
        else
          mipMedia = eMipContent;
        break;
      }
      case cmCapData:
      {
        mipMedia = eMipFecc;
        break;
      }
      default:
        mipMedia = eMipNoneMedia;
    }
    mipDirect = (MipDirection)pChannel->GetChannelDirection();
    mipTimerStat = eMipTimer;

    faultOpcode=ACK_NOT_RECEIVED;
    if (opcode)
    {
      cstr << " Did not receive ACK for opcode: ";
      responsibilityStr << "( Responsibility: embedded )";
    }
    else
    {
      cstr << " Did not receive all acks in H.323 disconnection ";
      responsibilityStr << "( Responsibility: IPParty )";
    }


  }

    if (opcode)
    {
      if (opcode == H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ)
        cstr << "H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ";
      else
        cstr << pProcess->GetOpcodeAsString(opcode);

      cstr << " Req:";
      if (pChannel != NULL)
      {
        if (mipHwConn == eMipRtp)
          cstr << pChannel->GetSeqNumRtp();
        else
          cstr << pChannel->GetSeqNumCm();

      }
      else
        cstr << "?";
    }

    cstr << responsibilityStr;
    DumpMcuInternalDetailed(cstr,faultOpcode);
//    CLargeString cstr1;
//    cstr1 << "################################################ MCU INTERNAL PROBLEM ######################################################\n";
//    cstr1 << "##\n";
//    cstr1 << "##\n";
//    //       Party:1 Conf:1 receives Failure Status for opcode: TB_MSG_OPEN_PORT_REQ from: video encoder req:53
//    cstr1 << "##   " << cstr << "\n";
//    cstr1 << "##\n";
//    cstr1 << "##\n";
//    cstr1 << "############################################################################################################################";
//
//
//    PTRACE(eLevelInfoNormal,cstr1.GetString());
//
//    CLargeString cstr2;
//    cstr2 << "McuInternalProblem - " <<  cstr;
//    CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
//              faultOpcode,
//              MAJOR_ERROR_LEVEL,
//              cstr2.GetString(),
//              TRUE);

}


////////////////////////////////////////////////////////////////////////////
// To be used in ChangeMode of type ChangeOutgoing
BYTE  CH323Cntl::UpdateVideoOutgoingChannelAccordingToTargetScm()
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::UpdateVideoOutgoingChannelAccordingToTargetScm - ",m_pmcCall->GetConnectionId());

  int  totalSize = 0;
    CapEnum scmProtocol = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople);;
  CCapSetInfo capInfoTemp = scmProtocol;
  cmCapDataType eType = cmCapVideo;

  m_bVideoOutRejected = FALSE;
  //If we need to open capability that the remote does not support we shouldn't continue with the opening.
  //For example: The remote cap has only 261 and the local has 261 and 263. the remote open 263, we can't open
  //263 either because the remote does not knoe how to receive such capability.

  if(!m_pRmtCapH323->OnCap(scmProtocol))
  {
    CSecondaryParams secParams;
    BYTE bSetNoVideoAndConToConf = FALSE;
    PTRACE2(eLevelInfoNormal,"CH323Cntl::UpdateVideoOutgoingChannelAccordingToTargetScm  - Can't open with a protocol that isn't supported by remote capabilities  : Name - ",PARTYNAME);
    if(!m_pLocalCapH323->FindSecondBestCap(m_pRmtCapH323, scmProtocol))
    {
            bSetNoVideoAndConToConf = TRUE;
      m_bVideoOutRejected = TRUE;
    }
    m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_RMT_DIFF_CAPCODE,secParams);
    if(bSetNoVideoAndConToConf)
    {
      return FALSE;
    }
  }

  CCapSetInfo capInfo = (CapEnum)m_pTargetModeH323->GetMediaType(cmCapVideo,cmCapTransmit,kRolePeople);
  if( m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::UpdateVideoOutgoingChannelAccordingToTargetScm  bIsClosing process - ",m_pmcCall->GetConnectionId());
    return FALSE;
  }

  // if the outgoing channel is not open - Open it
  DWORD index = GetChannelIndexInList(true, cmCapVideo ,TRUE, kRolePeople);
  if(index >= m_maxCallChannel)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::UpdateVideoOutgoingChannelAccordingToTargetScm   - channel is not open - ",m_pmcCall->GetConnectionId());
        OpenOutgoingChannel(cmCapVideo);
    return FALSE;
  }

  //Decide on the out rate:
  BYTE bUpdateSuccess = UpdateVideoOutRates(scmProtocol);
  if (bUpdateSuccess == FALSE)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::UpdateVideoOutgoingChannelAccordingToTargetScm  - UpdateVideoOutRates failed - ",m_pmcCall->GetConnectionId());
    return FALSE;
  }

  DWORD rate = 0;
    if (kCop == m_pTargetModeH323->GetConfType())
        rate = m_pTargetModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRolePeople);
    else
  rate = m_pParty->GetVideoRate();

  // Set of McChannel Params
  CChannel *pMcChannel = FindChannelInList(cmCapVideo, TRUE, kRolePeople);
  if(!pMcChannel)
    PASSERTMSG(m_pCsRsrcDesc->GetConnectionId(),"CH323Cntl::UpdateVideoOutgoingChannelAccordingToTargetScm  - pMcChannel allocation failed");
  else
  {
    int rIndex;
    //ON(m_OneOfTheMediaChannelWasConnected);
    rIndex = UpdateMcChannelParams(pMcChannel, capInfo, rate);
    BYTE bIsSupportDBC2   = IsSupportingDBC2(pMcChannel);
    pMcChannel->SetIsDbc2(bIsSupportDBC2);
    int length = 0;
    if(rIndex < m_maxCallChannel)
    {
      if (m_pTargetModeH323->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople))
        length = m_pTargetModeH323->GetMediaLength(cmCapVideo, cmCapTransmit, kRolePeople);
      else
        length = m_pTargetModeH323->GetMediaLength(cmCapVideo, cmCapReceive, kRolePeople);
            if (length)
            {
                channelSpecificParameters *pOutChannelParams = (channelSpecificParameters *)(new BYTE[length]);
                memset(pOutChannelParams, 0, length);
                CBaseVideoCap* pOutVideoCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfoTemp ,(BYTE*)pOutChannelParams);
                if (pOutVideoCap)
                {
                    EResult eResOfSet = kSuccess;
                    eResOfSet &= pOutVideoCap->SetDefaults(cmCapTransmit,kRolePeople);

                    length = m_pTargetModeH323->GetMediaLength(cmCapVideo, cmCapTransmit, kRolePeople);
                    capBuffer *pCapBuffer = (capBuffer *)new BYTE[length + sizeof(capBufferBase)];
                    m_pTargetModeH323->CopyMediaToCapBuffer(pCapBuffer, cmCapVideo,cmCapTransmit,kRolePeople);
                    CBaseCap * pBaseCap = CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE*)pCapBuffer->dataCap);
                    eResOfSet &= pOutVideoCap->CopyQualities(*pBaseCap);

                    eResOfSet &= pOutVideoCap->SetBitRate(rate);
                    pOutVideoCap->SetAdditionalXmlInfo();

                    PDELETEA(pCapBuffer);
                    POBJDELETE(pBaseCap);
                    if (eResOfSet == kFailure)
                        PTRACE(eLevelInfoNormal,"CH323Cntl::UpdateVideoOutgoingChannelAccordingToTargetScm  : Couldn't set video struct!!");
                }
                POBJDELETE(pOutVideoCap);

                AllocateAndSetChannelParams(length, (char *)pOutChannelParams ,pMcChannel); //set channels params in pMcChannel
        AllocateDynamicPayloadType(pMcChannel,capInfo);
        SetIsH263PlusForOutgoingChannel(pMcChannel);
        m_pTargetModeH323->SetMediaMode(capInfo, length, (BYTE *)pOutChannelParams,cmCapVideo ,cmCapTransmit ,kRolePeople, true);

        PDELETEA(pOutChannelParams);
            }
        }
    }
    return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//26.12.2006 Changes by VK. Stress Test
void CH323Cntl::StartStressTestTimerOnce()
{
  if (!s_lStressTestTimeoutCounter)
  {
    PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::StartStressTestTimerOnce - Start first time the timer.");
    if (IsValidTimer(STRESSTESTTOUT))
      DeleteTimer(STRESSTESTTOUT);
    StartTimer(STRESSTESTTOUT, STRESS_TEST_TIME * SECOND);
  }
}

////////////////////////////////////////////////////////////////////////////
//26.12.2006 Changes by VK. Stress Test
//          Capability sets
//  Take the audio capability which was opened (e.g. eG711Ulaw64kCapCode)
//  1. Audio + Video - opened + eH261CapCode
//  2. Video only  - eH261CapCode
//  3. Audio + Video - opened + eH261CapCode
//  4. Audio only    - opened
//  5. Audio + Video - opened + eH263CapCode
//  6. Video only    - eH263CapCode
//  7. Audio + Video - opened + eH263CapCode
//  8. Audio only    - opened
void CH323Cntl::OnStressTestTimeout(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Stress test timer raised.");
  DeleteTimer(STRESSTESTTOUT);
  if (m_state != CONNECT)
  {
    PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - The state of H323Control is not CONNECT.");
    StartTimer(STRESSTESTTOUT, STRESS_TEST_TIME * SECOND);
    return;
  }
  if (!s_lStressTestTimeoutCounter)
  {
    //Save audio caps
    if (m_pCurrentModeH323->IsMediaOn(cmCapAudio, cmCapTransmit, kRolePeople))
    {
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Save audio settings.");
      CMediaModeH323& rMediaModeObj = m_pCurrentModeH323->GetMediaMode(cmCapAudio, cmCapTransmit,         kRolePeople);
      CBaseCap* pBaseCapObj = rMediaModeObj.GetAsCapClass();
      if(pBaseCapObj)
      {
        g_eAudioOpenedCapEnumValue = pBaseCapObj->GetCapCode();
		delete pBaseCapObj;
      }
      else
        PASSERTMSG(1, "GetAsCapClass return NULL");
    }
    else
    {
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Audio is not opened yet.");
      StartTimer(STRESSTESTTOUT, STRESS_TEST_TIME * SECOND);
      return;
    }
  }
  int nStressSet = s_lStressTestTimeoutCounter % 8 + 1;
  PTRACE2INT(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Capability set = ", nStressSet);
  s_lStressTestTimeoutCounter++;
  PTRACE2INT(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - New counter of timeouts = ", s_lStressTestTimeoutCounter);
  switch (nStressSet)
  {
    case 1:     //Audio + Video
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Build set N1.");
      StressTestCase(g_eAudioOpenedCapEnumValue, eH261CapCode, FALSE, TRUE, TRUE);
      break;
    case 2:     //Video only
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Build set N2.");
      StressTestCase(eUnknownAlgorithemCapCode, eH261CapCode, TRUE, FALSE, FALSE);
      break;
    case 3:     //Audio + Video
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Build set N3.");
      StressTestCase(g_eAudioOpenedCapEnumValue, eH261CapCode, FALSE, FALSE, TRUE);
      break;
    case 4:     //Audio only
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Build set N4.");
      StressTestCase(g_eAudioOpenedCapEnumValue, eUnknownAlgorithemCapCode, FALSE, TRUE, TRUE); //???
      break;
    case 5:     //Audio + Video
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Build set N5.");
      StressTestCase(g_eAudioOpenedCapEnumValue, eH263CapCode, FALSE, FALSE, TRUE);
      break;
    case 6:     //Video only
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Build set N6.");
      StressTestCase(eUnknownAlgorithemCapCode, eH263CapCode, TRUE, FALSE, FALSE);
      break;
    case 7:     //Audio + Video
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Build set N7.");
      StressTestCase(g_eAudioOpenedCapEnumValue, eH263CapCode, FALSE, FALSE, TRUE);
      break;
    case 8:     //Audio only
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Build set N8.");
      StressTestCase(g_eAudioOpenedCapEnumValue, eUnknownAlgorithemCapCode, FALSE, TRUE, TRUE); //???
      break;
    default:
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::OnStressTestTimeout - Undefined Set - ERROR.");
      break;
  }
}

////////////////////////////////////////////////////////////////////////////
//26.12.2006 Changes by VK. Stress Test
void CH323Cntl::StressTestCase(CapEnum eAudioCap, CapEnum eVideoCap, BOOL bAudioClose, BOOL bVideoClose, BOOL bRegTimer)
{
  DWORD dwConfRate = m_pmcCall->GetRate();
  PTRACE2INT(eLevelInfoNormal, "Stress Test CH323Cntl::StressTestCase - Conference rate = ", dwConfRate);
  POBJDELETE(m_pLocalCapH323);
  PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::StressTestCase - Build new capability set.");
  m_pLocalCapH323 = new CCapH323;
  m_pLocalCapH323->SetStressTestSpecificCaps(eAudioCap, eVideoCap, dwConfRate);
  PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::StressTestCase - Build sorted capabilities.");
  m_pLocalCapH323->BuildSortedCap();
  PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::StressTestCase - Update conference about new local capabilities.");
  m_pTaskApi->UpdateLocalCapsInConfLevel(*m_pLocalCapH323);
  PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::StressTestCase - Close specific outgoing channels.");
  CloseSpecificOutgoingChannels(bAudioClose, bVideoClose);
  PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::StressTestCase - Ask for recaps.");
  OnPartyCreateControl(TRUE);
  PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::StressTestCase - Restart the timer.");
  if (bRegTimer)
    StartTimer(STRESSTESTTOUT, STRESS_TEST_TIME * SECOND);
  else
    StartTimer(STRESSTESTTOUT, STRESS_TEST_TIME_REOPEN * SECOND);
}

////////////////////////////////////////////////////////////////////////////
//26.12.2006 Changes by VK. Stress Test
void CH323Cntl::CloseSpecificOutgoingChannels(BOOL bAudioClose, BOOL bVideoClose)
{
  PTRACE2INT(eLevelInfoNormal, "Stress Test CH323Cntl::CloseSpecificOutgoingChannels - Connection ID = ", m_pmcCall->GetConnectionId());
  //Audio Channel
    if (bAudioClose == TRUE && m_pCurrentModeH323->IsMediaOn(cmCapAudio, cmCapTransmit, kRolePeople))
  {
        PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::CloseSpecificOutgoingChannels - Close Audio");
        CloseOutgoingChannel(cmCapAudio);
  }
    // Video Channel
    if (bVideoClose == TRUE && m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople))
    {
        PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::CloseSpecificOutgoingChannels - Close Video");
        CloseOutgoingChannel(cmCapVideo);
    }
    // Presentation Channel
    if (m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
    {
        PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::CloseSpecificOutgoingChannels - Close Content");
        CloseOutgoingChannel(cmCapVideo, kRoleContentOrPresentation);
    }
    // Data Channel
    if (m_pCurrentModeH323->IsMediaOn(cmCapData, cmCapTransmit, kRolePeople))
    {
        PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::CloseSpecificOutgoingChannels - Close Data");
        CloseOutgoingChannel(cmCapData);
    }
}

////////////////////////////////////////////////////////////////////////////
//26.12.2006 Changes by VK. Stress Test
BOOL CH323Cntl::IsIncomingVideoExist()
{
  BYTE bIsTransmiting = CalcIsTransmiting(cmCapReceive);
  CChannel* pChannel = FindChannelInList(cmCapVideo, bIsTransmiting, kRolePeople);
  PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::IsIncomingVideoExist - Find incoming video channel.");

  if (pChannel)
  {
    PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::IsIncomingVideoExist - The incoming video channel found.");
    if (pChannel->GetCsChannelState() == kConnectedState || pChannel->IsCsChannelStateConnecting())
    {
      PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::IsIncomingVideoExist - The incoming video channel exists.");
      return TRUE;
    }
  }

  PTRACE(eLevelInfoNormal, "Stress Test CH323Cntl::IsIncomingVideoExist - The incoming video channel doesn't exist.");
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendAVFFacilityEndOfMove(const char * sNumericID)
{
    //H323_CS_FACILITY_REQ
    if (!m_bIsAvaya)
      return;
    mcReqFacility* pFacilityAVFReq = new mcReqFacility;
    pFacilityAVFReq->avfStandardId = AVF_Entry_Queue_Transfer;
  strncpy (pFacilityAVFReq->e164ConferenceId, sNumericID, MaxAliasLength - 1); //WK 1243 (-1)
    CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)pFacilityAVFReq, sizeof(mcReqFacility));
  m_pCsInterface->SendMsgToCS(H323_CS_FACILITY_REQ, pMsg, m_serviceId,
                  m_serviceId, m_pDestUnitId, m_callIndex, 0,0,0);
  POBJDELETE(pMsg);
    PDELETE(pFacilityAVFReq);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
enIpVersion CH323Cntl::CheckForMatchBetweenPartyAndUdp(enIpVersion eIpVer,eIpType eipType)
{
  enIpVersion ipMatch = eIpVersion4;

  if ( eIpVer == eIpVersion4 && (eipType == eIpType_IpV4 || eipType == eIpType_Both))
  {
    ipMatch = eIpVersion4;
  }
  else if ( eIpVer == eIpVersion6 && (eipType == eIpType_IpV6 || eipType == eIpType_Both))
  {
    ipMatch = eIpVersion6;
  }

  return ipMatch;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::GetRmtPossibleContentRate() const
{
  DWORD  ContentRate = 0;
  ContentRate = m_pRmtCapH323->GetMaxContentBitRate();

  return ContentRate;
}

////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsLateReleaseOfVideoResources () const
{
    //In Avaya enviroment - only Audio only EPs will send 64K rate in the incoming setup
    // for other endpoints we should wait until video capabilities before releasing video resources
    DWORD remoteSetupRate = (DWORD)(m_pH323NetSetup->GetRemoteSetupRate());
  BYTE isLateReleaseOfVideoResources =  ((m_state == SETUP || m_state == CONNECT) && m_bIsAvaya &&
            (!m_pmcCall->GetIsOrigin()) &&
            remoteSetupRate > AUDIO_ONLY_SETUP_RATE);

  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::IsLateReleaseOfVideoResources - isLateReleaseOfVideoResources=", isLateReleaseOfVideoResources);
  return isLateReleaseOfVideoResources;
}

////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsSlaveCascadeModeForH239() const
{
    if((m_pmcCall->GetRmtType() == cmEndpointTypeMCU) &&
        cmMSSlave == m_pmcCall->GetMasterSlaveStatus())
        return TRUE;
    else if (m_bDisguiseAsEPMode)
        return TRUE;
    else
        return FALSE;
}
////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsRemoteIsSlaveMGCWithContent() const
{
  BYTE ans = FALSE;
  if (IsRemoteIsSlaveMGC())
  {
    CChannel* pChannel = FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation);
    if(pChannel && pChannel->GetRoleLabel() == kRoleContent)
    {
      ans = TRUE;
    }
  }

  return ans;
}
////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsRemoteIsSlaveMGC() const
{
  return (m_remoteIdent == PolycomMGC && cmMSMaster == m_pmcCall->GetMasterSlaveStatus());
}
////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsRemoteACascadedMcuWithH239Enabled()
{
    return ((m_remoteIdent == PolycomRMX || m_remoteIdent == PolycomMGC || m_remoteIdent == DstH323Mcs) && m_pRmtCapH323->IsH239());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// VNGFE-787
void CH323Cntl::SetIsCodianVcr(BYTE isCodianVcr)
{
  m_isCodianVcr = isCodianVcr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::GetIsCodianVcr()
{
  return m_isCodianVcr;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnRtpLprChangeModeInd(CSegment* pParam)
{
  APIU32              unChannelType;        //Video/Audio/Content/FECC
  APIU32              unChannelDirection;
  APIU16              usLossProtection;
  APIU32              unMTBF;
  APIU32              unCongestionCeiling;
  APIU16              usFill;
  APIU16              usModeTimeout;
  APIU16              usAlignment;
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnRtpLprChangeModeInd - Connection ID = ", GetConnectionId());
  // Passing the LPR ind from the rtp to the EP
  TLprModeChangeInd *pLprModeChange = new TLprModeChangeInd;
  DWORD  structLen = sizeof(TLprModeChangeInd);
  memset(pLprModeChange, 0, structLen);
  pParam->Get((BYTE*)pLprModeChange, structLen);

  BYTE bAllChannelsConnected    = AreAllChannelsConnected();
  if (bAllChannelsConnected)
  {
    CChannel *pChannel = FindChannelInList(cmCapVideo, FALSE, kRolePeople); //incoming channel
    if (pChannel)
    {
      mcReqLPRModeChange* pLprChangeModeReq = new mcReqLPRModeChange;
      pLprChangeModeReq->lossProtection = pLprModeChange->usLossProtection;
      pLprChangeModeReq->mtbf = pLprModeChange->unMTBF;
      pLprChangeModeReq->congestionCeiling = pLprModeChange->unCongestionCeiling;
      pLprChangeModeReq->fill = pLprModeChange->usFill;
      pLprChangeModeReq->modeTimeout = pLprModeChange->usModeTimeout;
        CSegment* pMsg = new CSegment;
      pMsg->Put((BYTE*)pLprChangeModeReq, sizeof(mcReqLPRModeChange));
      m_pCsInterface->SendMsgToCS(H323_CS_SIG_LPR_MODE_CHANGE_REQ, pMsg, m_serviceId,
                      m_serviceId, m_pDestUnitId, m_callIndex, 0,0,0);
      POBJDELETE(pMsg);
        PDELETE(pLprChangeModeReq);

      BYTE isSync = 0;
      BYTE isLocal = 0;
      if (pLprModeChange->usLossProtection == 0 && pLprModeChange->unMTBF == 0)
        isSync = 1;

      UpdateDbLprSyncStatus(isSync,isLocal);

      if (IsValidTimer(LOCALLPRTOUT))
        DeleteTimer(LOCALLPRTOUT);

      if (pLprModeChange->usModeTimeout != 0)
        StartTimer(LOCALLPRTOUT,pLprModeChange->usModeTimeout);

    }
    else
      PASSERTMSG(GetConnectionId(), "CH323Cntl::OnRtpLprChangeModeInd - No incoming video channel !!!");
  }
  else
    PTRACE(eLevelError,"CH323Cntl::OnRtpLprChangeModeInd: LPR RTP IND ARRIVED BEFORE PARTY IS FULLY CONNECTED!!! ");

  PDELETE(pLprModeChange);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323LprChangeModeAckInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323LprChangeModeAckInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"CH323Cntl::OnH323LprChangeModeAckInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"CH323Cntl::OnH323LprChangeModeAckInd - srcUnitId incorrect");
    return;
  }
  if (m_pmcCall->GetIsClosingProcess() == TRUE)
  { // if the call is in closing process no need to send fast update.
    PTRACE2INT(eLevelError,"CH323Cntl::OnH323LprChangeModeAckInd bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }
  CChannel *pChannel = FindChannelInList(cmCapVideo, FALSE, kRolePeople); //incoming channel
  if (pChannel == NULL)
    PASSERTMSG(GetConnectionId(), "CH323Cntl::OnH323LprChangeModeAckInd - No incoming video channel !!!");

}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323LprChangeModeIndSetup(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323LprChangeModeIndSetup - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
    SendLprAckToParty();
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323LprChangeModeInd(CSegment* pParam)
{
	CLargeString logInfo1;
	logInfo1 << "Connection Id = " << m_pCsRsrcDesc->GetConnectionId();

	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIS32 status = 0;
	APIU16 srcUnitId = 0;

	*pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;
	status = (APIS32)stat1;

	if (callIndex != m_callIndex)
	{
		PASSERTMSG(callIndex,"CH323Cntl::OnH323LprChangeModeInd - Call Index incorrect");
		return;
	}
	if (srcUnitId != m_pDestUnitId)
	{
		PASSERTMSG(srcUnitId,"CH323Cntl::OnH323LprChangeModeInd - srcUnitId incorrect");
		return;
	}
	if (m_pmcCall->GetIsClosingProcess() == TRUE)
	{ // if the call is in closing process no need to send fast update.
		PTRACE2INT(eLevelError,"CH323Cntl::OnH323LprChangeModeInd bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
		return;
	}

	mcIndLPRModeChange lprChangeModeInd;
	DWORD  structLen = sizeof(mcIndLPRModeChange);
	memset(&lprChangeModeInd,0,structLen);
	pParam->Get((BYTE*)(&lprChangeModeInd),structLen);

	// Actions to perform:
	// 1. Compute new desired rate for video
	// 2. In case of CP conf - Update the participant encoder (people)
	// 3. In case of VSW conf - Send to the video bridge request for spreading the new rate (people)

	BYTE isSync = 0;
	BYTE isLocal = 1;

	// Compute !!!!!
	if( lprChangeModeInd.congestionCeiling <= 0 )
	{
		PASSERTMSG((m_pCsRsrcDesc->GetConnectionId()),"CH323Cntl::OnH323LprChangeModeInd - congestionCeiling is not valid");
		return;
	}

	LPRParams *pLprParam = NULL;
	DWORD newTotalVideoRate = 0;

	// This means - Stop encoding LPR and return to initial state Or we received a DBA (flow control) command from the EP.
	if (lprChangeModeInd.lossProtection == 0 && lprChangeModeInd.mtbf == 0)
	{
		if (m_pParty->GetVideoRate() < lprChangeModeInd.congestionCeiling)
		{
			newTotalVideoRate = m_pParty->GetVideoRate();
			// this error can cause send UpdateRtpWithLprInfo bigger than the channel bondiries.
			DBGPASSERT(m_pCsRsrcDesc->GetPartyRsrcId());
			PTRACE2INT(eLevelError,"CH323Cntl::OnH323LprChangeModeInd congestionCeiling > m_pParty->GetVideoRate()!!, connId - ",m_pCsRsrcDesc->GetConnectionId());
		}
		else
			newTotalVideoRate = lprChangeModeInd.congestionCeiling;

		logInfo1 << ", No LPR redundant information is added" << "\n";
		isSync = 1;
		//UpdateDbLprSyncStatus(isSync,isLocal);
		m_realLprRate = 0;
	}
	else
	{
		// Get LPR correct rate
		pLprParam = ::lookupLprParams(lprChangeModeInd.lossProtection, (lprChangeModeInd.mtbf*100),	0, (m_pmcCall->GetMaxRate()/1000));

		if (pLprParam == NULL)
		{
			PASSERTMSG(m_pCsRsrcDesc->GetConnectionId(),"CH323Cntl::OnH323LprChangeModeInd - pLprParam == NULL - Not valid!!");
			return;
		}
		// Computing the new value
		//1.  ConfParty will use the LossProtection parameter from the LPR command to acquire the D (data packets ratio) and R (redundancy packet ratio) from the LPR tables.
		//2.  M = R/ (D+R).
		//3.  NewTotalBitRate = video or content/ (1+M). The sum of both of them should be <= from the CongestionCeiling parameter.

		// Total rate of Content and Video
		newTotalVideoRate 	= (((lprChangeModeInd.congestionCeiling)*(pLprParam->numData + pLprParam->numRecovery))/((pLprParam->numData + (2*pLprParam->numRecovery))));
		logInfo1 << ", Considering LPR redundant information" << "\n";

		if (newTotalVideoRate > lprChangeModeInd.congestionCeiling)
		{
			PASSERTMSG((newTotalVideoRate),"CH323Cntl::OnH323LprChangeModeInd - Combined rate bigger then congestionCeiling");
			return;
		}
		m_isLprModeOn = 1;
		// UpdateDbLprSyncStatus(isSync,isLocal);
	}

	// calc the new rates without LPR correction on the content.
	DWORD newPeopleRate  = 0;
	DWORD newContentRate = 0;
	newContentRate = m_curConfContRate;
	newPeopleRate  = newTotalVideoRate - m_curConfContRate;
	logInfo1 << "new Total Video Rate - " << newTotalVideoRate << ", new People Rate - " << newPeopleRate << ", current content rate = " << m_curConfContRate;
	PTRACE2PARTYID(eLevelInfoNormal,"CH323Cntl::OnH323LprChangeModeInd - \n", logInfo1.GetString(), m_pCsRsrcDesc->GetPartyRsrcId());

	//-- VNGFE-8204
	BYTE bLprOnContent 	= FALSE;
	BYTE bSetPeopleRate = FALSE;
	DynamicContentLprHandling(lprChangeModeInd, bLprOnContent, bSetPeopleRate, newPeopleRate, newTotalVideoRate, newContentRate);

	if(bSetPeopleRate == FALSE)
	{
		bool reCalcContent = false;
		SetNewPeopleRateLpr(newPeopleRate, newTotalVideoRate, newContentRate, reCalcContent);
	}
	TRACEINTO << "(newTotalVideoRate-newContentRate) will be the new people rate: " << newPeopleRate  << ", newTotalVideoRate = " << newTotalVideoRate << ", conference induced people rate = " << m_pParty->GetVideoRate() - m_curConfContRate;

	// Saving the last original LPR rate.
    m_realLprRate = newTotalVideoRate;
	if ((m_pTargetModeH323->GetConfType() == kVideoSwitch) || (m_pTargetModeH323->GetConfType() == kVSW_Fixed))
	{
		// VSW:
		BOOL bEnableFlowControlVSW = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_VSW_FLOW_CONTROL);
		if(!bEnableFlowControlVSW)
		{
			PTRACE(eLevelError,"CH323Cntl::OnH323LprChangeModeInd: SUPPORT_VSW_FLOW_CONTROL is false");
			return;
		}
	}
	else
	{
		// Currently the Total allowed video rate is the congestionCeiling
		SetTotalVideoRate(newTotalVideoRate);
	}

	// Sending new rates to video and content bridges
	m_pTaskApi->UpdatePeopleLprRate(newPeopleRate, cmCapTransmit, lprChangeModeInd.lossProtection,
								lprChangeModeInd.mtbf,lprChangeModeInd.congestionCeiling ,lprChangeModeInd.fill , lprChangeModeInd.modeTimeout, newTotalVideoRate, (DWORD)bLprOnContent, newContentRate);

	// save last LPR indication
	m_lprModeChangeData.lossProtection		= lprChangeModeInd.lossProtection;
	m_lprModeChangeData.mtbf 				= lprChangeModeInd.mtbf;
	m_lprModeChangeData.congestionCeiling	= lprChangeModeInd.congestionCeiling;
	m_lprModeChangeData.fill 				= lprChangeModeInd.fill;
	m_lprModeChangeData.modeTimeout 		= lprChangeModeInd.modeTimeout;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CH323Cntl::DynamicContentLprHandling(mcIndLPRModeChange& lprChangeModeInd, BYTE& bLprOnContent, BYTE& bSetPeopleRate, DWORD& newPeopleRate, DWORD& newTotalVideoRate, DWORD& newContentRate)
{
	bool isCpAvcConf = ((m_pTargetModeH323->GetConfType() == kCp) && (m_pTargetModeH323->GetConfMediaType() == eAvcOnly));
	BOOL bEnableDynamicChangeOfContentBitRateByLpr = GetSystemCfgFlagInt<BOOL>(CFG_KEY_LPR_CONTENT_RATE_ADJUST_WEAK_LPR);
	if(bEnableDynamicChangeOfContentBitRateByLpr && isCpAvcConf)
	{
		bool isConfCascaded = DynamicContentLprHandlingInCascade(lprChangeModeInd, bSetPeopleRate, newPeopleRate, newTotalVideoRate, newContentRate);
		if(isConfCascaded == false)
		{
			DynamicContentLprHandlingInSingleConf(bLprOnContent, newPeopleRate, newTotalVideoRate, newContentRate);
			bSetPeopleRate = TRUE;
		}
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CH323Cntl::DynamicContentLprHandlingInCascade(mcIndLPRModeChange& lprChangeModeInd, BYTE& bSetPeopleRate, DWORD& newPeopleRate, DWORD& newTotalVideoRate, DWORD newContentRate)
{
	CCommConf* pCommConf = NULL;
	pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	PASSERT_AND_RETURN_VALUE(!pCommConf, false);
	BYTE isConfCascaded = FALSE;
	if(pCommConf->GetCurrentConfCascadeMode() != CASCADE_MODE_NONE)
		isConfCascaded = TRUE;

	if(isConfCascaded)
	{
		CLargeString logInfo;
		logInfo << "VNGFE-8204 Cascade Conference \n";

		if (lprChangeModeInd.lossProtection > 2)
		{// if we are handling more than 2% packet loss and the total media rate exceeded the line rate then we should use weak LPR protection
			// calculate the new people rate to check exceeded
			logInfo << "VNGFE-8204 lossProtection > 2 \n";
			bool reCalcContent 			= false;
			bool rValForcePeopleTo64K	= SetNewPeopleRateLpr(newPeopleRate, newTotalVideoRate, newContentRate, reCalcContent);
			bSetPeopleRate				= TRUE;

			if(rValForcePeopleTo64K)
			{// New values from algorithm calculation which will be limit up to 15% redundant information
				logInfo << "VNGFE-8204 limit LPR protection \n";
				lprChangeModeInd.lossProtection = 2;
				lprChangeModeInd.mtbf 			= 8;
			}
		}
		PTRACE2PARTYID(eLevelInfoNormal,"CH323Cntl::DynamicContentLprHandlingInCascade - \n", logInfo.GetString(), m_pCsRsrcDesc->GetPartyRsrcId());
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CH323Cntl::DynamicContentLprHandlingInSingleConf(BYTE& bLprOnContent, DWORD& newPeopleRate, DWORD& newTotalVideoRate, DWORD& newContentRate)
{
	CLargeString logInfo;

	bool reCalcContent 			= true;
	bool rValForcePeopleTo64K	= SetNewPeopleRateLpr(newPeopleRate, newTotalVideoRate, newContentRate, reCalcContent);
	if(rValForcePeopleTo64K)// on the LPR indication flow, first change of the content rate.
		bLprOnContent = TRUE;

	bool bIsForceThreshold 			= false;
	bool bImproveContent			= false;
	DWORD dwContentThresholdRate 	= 0;
	CalcLprInfluanceNewContentRateWithThreshold(bIsForceThreshold, bImproveContent, newTotalVideoRate, newContentRate, newPeopleRate, dwContentThresholdRate);

	if(bIsForceThreshold || bImproveContent)// content rate was increase
	{
		bLprOnContent 			= TRUE;
		reCalcContent			= false;
		rValForcePeopleTo64K	= SetNewPeopleRateLpr(newPeopleRate, newTotalVideoRate, newContentRate, reCalcContent);
		if(bIsForceThreshold)
			logInfo << "VNGFE-8204. content rate was too low. it didn't match conference minimum threshold. increase it.\n";
		else
			logInfo << "VNGFE-8204. content rate was improved by reducing people to 64K.\n";
	}

	if(newContentRate == m_curConfContRate)
	{
		if(bLprOnContent)
			logInfo << "VNGFE-8204. LPR on content is TRUE, but new content rate and old content rate are the same. Don't send content change rate to Conf.\n";
		bLprOnContent = FALSE;// new content rate equel to old one, no change, no need to send LPR indication to Conf.
	}

	logInfo << "VNGFE-8204 - newTotalVideoRate = " << newTotalVideoRate  << ", newContentRate = " << newContentRate << ", newPeopleRate = " << newPeopleRate << "\n";
	logInfo << "VNGFE-8204 - m_pParty->GetVideoRate() = " << m_pParty->GetVideoRate() << ", conference content rate = " << m_curConfContRate << "\n";
	logInfo << "VNGFE-8204 - LPR on content is - " << (int)bLprOnContent;
	PTRACE2PARTYID(eLevelInfoNormal,"CH323Cntl::DynamicContentLprHandlingInSingleConf - \n", logInfo.GetString(), m_pCsRsrcDesc->GetPartyRsrcId());

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::CalcLprInfluanceNewContentRateWithThreshold(bool& bIsForceThreshold, bool& bImproveContent, DWORD newTotalVideoRate, DWORD& newContentRate, DWORD& newPeopleRate, DWORD& dwContentThresholdRate)
{
	CCommConf* pCommConf = NULL;
	pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	PASSERT_AND_RETURN(!pCommConf);

	if (pCommConf && m_curConfContRate)
	{
		CLargeString logInfo;

		CUnifiedComMode 	localUnifedCommMode(pCommConf->GetEnterpriseModeFixedRate(),pCommConf->GetConfTransferRate());
		const eEnterpriseMode 					ContRatelevel 			= (eEnterpriseMode)pCommConf->GetEnterpriseMode();
		const BYTE 								lConfRate 				= pCommConf->GetConfTransferRate();
		const eConfMediaType 					mediaType 				= pCommConf->GetConfMediaType();
		const ePresentationProtocol 			presentationProtocol	= (ePresentationProtocol)pCommConf->GetPresentationProtocol();
		const eCascadeOptimizeResolutionEnum	resolutionLevel			= pCommConf->GetCascadeOptimizeResolution();

		logInfo << "ContRatelevel:" << ContRatelevel << ", lConfRate:" << (int)lConfRate << "\n";
		DWORD confIpContentRate = localUnifedCommMode.GetContentModeAMCInIPRate(lConfRate, ContRatelevel, presentationProtocol, resolutionLevel, mediaType);

		BOOL isPartyMeetContentRateThreshold = ::isPartyMeetContentRateThreshold(confIpContentRate/10, newContentRate/10, pCommConf->GetEnterpriseMode(), pCommConf->GetPresentationProtocol(), dwContentThresholdRate);

		logInfo << "VNGFE-8204 - isPartyMeetContentRateThreshold:" << (int)isPartyMeetContentRateThreshold << ", confIpContentRate:" << confIpContentRate
				<< ", newContentRate:" << newContentRate << "\n";
		if (!isPartyMeetContentRateThreshold)
		{
			newContentRate	= dwContentThresholdRate*10;// values in isPartyMeetContentRateThreshold are in K/sec and in H323 party are in 100bit/sec
			newPeopleRate	= newTotalVideoRate - newContentRate;
			logInfo << "VNGFE-8204 - update content rate:" << newContentRate << ", update people rate: " << newPeopleRate << "\n";
			bIsForceThreshold = true;
		}
		else
		{
		    if(newPeopleRate > minPeopleRate)// if people rate is already 64K, the attempt to increase to content rate was already done.
		    {
				DWORD lprContentPotentialRate = newTotalVideoRate - minPeopleRate;
				if(lprContentPotentialRate > newContentRate)
				{
					lprContentPotentialRate = min (lprContentPotentialRate, confIpContentRate);
					newPeopleRate			= newTotalVideoRate - lprContentPotentialRate;	//	newPeopleRate >= minPeopleRate;

					logInfo << "VNGFE-8204 - possible to increase content rate. confIpContentRate = " << confIpContentRate  << ", newContentRate = " << newContentRate << ", lprContentPotentialRate = " << lprContentPotentialRate << ", newPeopleRate = " << newPeopleRate << "\n";
					newContentRate			= lprContentPotentialRate;
					bImproveContent 		= true;
				}
		    }
		}
		PTRACE2PARTYID(eLevelInfoNormal,"CH323Cntl::CalcLprInfluanceNewContentRateWithThreshold - \n", logInfo.GetString(), m_pCsRsrcDesc->GetPartyRsrcId());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function return true if the people rate was force to 64K
bool CH323Cntl::SetNewPeopleRateLpr(DWORD& newPeopleRate, DWORD& newTotalVideoRate, DWORD& newContentRate, bool reCalcContent)
{
    if ((newTotalVideoRate <= newContentRate + minPeopleRate) || (newTotalVideoRate <= newContentRate))
	{
		PTRACE(eLevelError,"CH323Cntl::SetNewPeopleRateLpr: ACHIVED LOWER TRESHOLD OF 64K LIVE VIDEO - WE MAKE PEOPLE RATE 64K FOR THIS LPR/DBA REQUEST!!! ");
		newPeopleRate = minPeopleRate;
		m_isLprContentForceReductionTo64 = 1;
		if(reCalcContent)
			newContentRate		= newTotalVideoRate - newPeopleRate; // VNGFE-8204
		else
			newTotalVideoRate	= newPeopleRate + newContentRate; // VNGFE-6950
		return true;// force
	}
	else
	{
		newPeopleRate = newTotalVideoRate - newContentRate;
		m_isLprContentForceReductionTo64 = 0;
	}
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::UpdateRtpWithLprInfo()
{
	// in change content mode flow (change out), we close and re-open the content channel and memset the LPR data.
	// we need to re-inform the RTP on that data.
	PTRACEPARTYID(eLevelInfoNormal,"CH323Cntl::UpdateRtpWithLprInfo",m_pCsRsrcDesc->GetPartyRsrcId());
	UpdateLprModeReqToRtp(cmCapVideo, TRUE, kRoleContentOrPresentation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: //added by Jason
void CH323Cntl::OnH323NewITPSpeakerAckInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"ITP_CASCADE: CH323Cntl::OnH323NewITPSpeakerAckInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"ITP_CASCADE: CH323Cntl::OnH323NewITPSpeakerAckInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"ITP_CASCADE: CH323Cntl::OnH323NewITPSpeakerAckInd - srcUnitId incorrect");
    return;
  }
  if (m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelError,"ITP_CASCADE: CH323Cntl::OnH323NewITPSpeakerAckInd bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }

  // Sending Ack to Party to end the ITP speaker change process
  m_pTaskApi->SendNewITPSpeakerAckInd();

}

////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature:  //added by Jason
void CH323Cntl::OnH323NewITPSpeakerInd(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"ITP_CASCADE: CH323Cntl::OnH323NewITPSpeakerInd - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  APIU32 callIndex = 0;
  APIU32 channelIndex = 0;
  APIU32 mcChannelIndex = 0;
  APIU32 stat1 = 0;
  APIS32 status = 0;
  APIU16 srcUnitId = 0;

  *pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

  status = (APIS32)stat1;

  if (callIndex != m_callIndex)
  {
    PASSERTMSG(callIndex,"ITP_CASCADE: CH323Cntl::OnH323NewITPSpeakerInd - Call Index incorrect");
    return;
  }
  if (srcUnitId != m_pDestUnitId)
  {
    PASSERTMSG(srcUnitId,"ITP_CASCADE: CH323Cntl::OnH323NewITPSpeakerInd - srcUnitId incorrect");
    return;
  }
  if (m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelError,"ITP_CASCADE: CH323Cntl::OnH323NewITPSpeakerInd bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    return;
  }

  mcIndNewITPSpeaker newITPSpeakerInd;

  DWORD  structLen = sizeof(mcIndNewITPSpeaker);
  memset(&newITPSpeakerInd,0,structLen);
  pParam->Get((BYTE*)(&newITPSpeakerInd),structLen);

  // check the validity of parameters
  if( newITPSpeakerInd.ITPType > eTelePresencePartyCTS || newITPSpeakerInd.numOfActiveLinks > 4)
  {
	  PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323Cntl::OnH323NewITPSpeakerInd - ITPType or numOfActiveLinks is not valid");
	  return;
  }

  eTelePresencePartyType itpType = (eTelePresencePartyType)newITPSpeakerInd.ITPType;
  DWORD numOfActiveLinks = newITPSpeakerInd.numOfActiveLinks;

  // Sending ITP speaker change info to Party, and wait for Party to send Ack
  m_pTaskApi->SendNewITPSpeakerInd(itpType, numOfActiveLinks);
}
//added by Jason for ITP-Multiple channels end

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetLprMode(WORD lprOnOff)
{
  m_isLprModeOn = lprOnOff;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323LprTout(CSegment* pParam)
{
    CancelLpr();
}
////////////////////////////////////////////////////////////////////////////
void CH323Cntl::CancelLpr()
{
    if (!m_isLprModeOn)
    {
         PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CancelLpr - LPR Not active! Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
        return;
    }

    PTRACE2INT(eLevelInfoNormal,"CH323Cntl::CancelLpr - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

    DWORD newPeopleRate=0;
    if ((m_pTargetModeH323->GetConfType() == kVideoSwitch) || (m_pTargetModeH323->GetConfType() == kVSW_Fixed))
        newPeopleRate = m_pCurrentModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit);
    else
        newPeopleRate = m_pCurrentModeH323->GetTotalVideoRate() - m_curConfContRate;

    TRACEINTO << "CH323Cntl::CancelLpr - newPeopleRate = " << newPeopleRate << "\n";
    SendFlowControlReq (cmCapVideo, TRUE , kRolePeople ,newPeopleRate);

}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::MfaH323LprAck()
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::MfaH323LprAck - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  DWORD newPeopleRate = 0;
  newPeopleRate = m_pParty->GetVideoRate() - m_curConfContRate;
  TRACEINTO << "CH323Cntl::OnH323LprTout - newPeopleRate = " << newPeopleRate << "\n";

  if ((m_pTargetModeH323->GetConfType() == kVideoSwitch) || (m_pTargetModeH323->GetConfType() == kVSW_Fixed))
  {
    // VSW:
    BOOL bEnableFlowControlVSW = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_VSW_FLOW_CONTROL);
    if(!bEnableFlowControlVSW)
    {
      PTRACE(eLevelError,"CH323Cntl::MfaH323LprAck: SUPPORT_VSW_FLOW_CONTROL is false");
      return;
    }
  }
  // Sending new rates to video  bridge
  m_pTaskApi->UpdatePartyH323VideoBitRate(newPeopleRate, cmCapTransmit, kRolePeople);
}


////////////////////////////////////////////////////////////////////////////
void CH323Cntl::UpdateLprModeReqToRtp(cmCapDataType dataType, BYTE bOutDirection, ERoleLabel eRole)
{
	if(m_lprModeChangeData.congestionCeiling != 0)
	{
		CChannel* pChannel = FindChannelInList(dataType, bOutDirection, eRole);
		if(pChannel)
		{
			if (pChannel->GetCsChannelState() == kConnectedState)
			{
				kChanneltype eChannelType = kEmptyChnlType;
				eChannelType = ::DataTypeToChannelType(dataType, eRole);
				  TLprModeChangeReq *pStruct	= new TLprModeChangeReq;

				  pStruct->unChannelDirection	= (APIU32)cmCapTransmit;
				  pStruct->usLossProtection		= m_lprModeChangeData.lossProtection;
				  pStruct->unMTBF				= m_lprModeChangeData.mtbf;
				  pStruct->unCongestionCeiling  = m_lprModeChangeData.congestionCeiling;
				  pStruct->usFill         		= m_lprModeChangeData.fill;
				  pStruct->usModeTimeout      	= m_lprModeChangeData.modeTimeout;
				  pStruct->usAlignment      	= 0;
				  pStruct->unChannelType      	= (APIU32)eChannelType;

				  SendMsgToMpl((BYTE*)(pStruct), sizeof(TLprModeChangeReq), H323_RTP_LPR_MODE_CHANGE_REQ);
				  PDELETE(pStruct);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendLprReqToMfa(WORD status, DWORD lossProtection, DWORD mtbf, DWORD newPeopleRate, DWORD fill, DWORD modeTimeout, BYTE isReset)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::SendLprReqToMfa - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());
  if (status)
  {
    PASSERTMSG(status, "CH323Cntl::SendLprReqToMfa - Status fail");
    return;
  }
  DWORD congestionCeiling = 0;
/*  if (isReset)
  {
    DWORD activeContentRate = m_pCurrentModeH323->GetMediaBitRate(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
    congestionCeiling = newPeopleRate + activeContentRate;
  }
  else*/
    congestionCeiling = newPeopleRate;

  TLprModeChangeReq *pStruct = new TLprModeChangeReq;


  pStruct->unChannelType      = (APIU32)kIpVideoChnlType;
  pStruct->unChannelDirection   = (APIU32)cmCapTransmit;
  pStruct->usLossProtection   = lossProtection;
  pStruct->unMTBF         = mtbf;
  pStruct->unCongestionCeiling  = congestionCeiling;
  pStruct->usFill         = fill;
  pStruct->usModeTimeout      = modeTimeout;
  pStruct->usAlignment      = 0;

  BYTE isSync = 0;
  BYTE isLocal = 1;
  if (isReset)
  {
    isSync = 1;
    UpdateDbLprSyncStatus(isSync,isLocal);
    SendMsgToMpl((BYTE*)(pStruct), sizeof(TLprModeChangeReq), H323_RTP_LPR_MODE_RESET_REQ);
  }
  else
    {
        isSync = 0;
        if (lossProtection == 0 && mtbf == 0)
            isSync = 1;
        UpdateDbLprSyncStatus(isSync,isLocal);
    SendMsgToMpl((BYTE*)(pStruct), sizeof(TLprModeChangeReq), H323_RTP_LPR_MODE_CHANGE_REQ);
    }


  // Sending the request to the Content RTP as well
  pStruct->unChannelType      = (APIU32)kIpContentChnlType;

  if (isReset)
    SendMsgToMpl((BYTE*)(pStruct), sizeof(TLprModeChangeReq), H323_RTP_LPR_MODE_RESET_REQ);
  else
    SendMsgToMpl((BYTE*)(pStruct), sizeof(TLprModeChangeReq), H323_RTP_LPR_MODE_CHANGE_REQ);

  PDELETE(pStruct);

  // Remember the modeTimeout for the starting of the timer when receiving the ack
  m_LprModeTimeout = modeTimeout;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CH323Cntl::OnConfFlowControlReq(DWORD newVidRate, BYTE outChannel, lPRModeChangeParams* pLprChangeModeParams)
{
  if(m_pmcCall->GetIsClosingProcess() == TRUE)
  {
    PTRACE2INT(eLevelError,"CH323Cntl::OnConfFlowControlReq:  bIsClosing process - ",m_pCsRsrcDesc->GetConnectionId());
    return FALSE;
  }
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnConfFlowControlReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  CChannel* pChannel = FindChannelInList(cmCapVideo, outChannel);
  if(pChannel)
  {
    if (pChannel->GetCsChannelState() != kConnectedState)
    {
      PTRACE2INT(eLevelError, "CH323Cntl::OnConfFlowControlReq - Channel isn't in connected state - ", m_pCsRsrcDesc->GetConnectionId());
      return FALSE;
    }

    BOOL bIsGoodDetail = TRUE;
    bIsGoodDetail = CheckFlowControlDetails(newVidRate, pChannel);
    if(!bIsGoodDetail)
    {
      if (pChannel->IsOutgoingDirection() && newVidRate && (newVidRate < pChannel->GetRate()) && pLprChangeModeParams )
      {
        PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnConfFlowControlReq -we don't send flow control but we do update RTP lpr to lower rate = ",m_pCsRsrcDesc->GetConnectionId());
        SendLprReqToMfa(statOK, pLprChangeModeParams->lossProtection, pLprChangeModeParams->mtbf, pLprChangeModeParams->congestionCeiling
                , pLprChangeModeParams->fill, pLprChangeModeParams->modeTimeout);
      }
    }
    if(!bIsGoodDetail)
    {
      if (pChannel->IsOutgoingDirection() && newVidRate && (newVidRate < pChannel->GetRate()) && pLprChangeModeParams )
      {
        PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnConfFlowControlReq -we don't send flow control but we do update RTP lpr to lower rate = ",m_pCsRsrcDesc->GetConnectionId());
        SendLprReqToMfa(statOK, pLprChangeModeParams->lossProtection, pLprChangeModeParams->mtbf, pLprChangeModeParams->congestionCeiling
                , pLprChangeModeParams->fill, pLprChangeModeParams->modeTimeout);
      }
      }

    if(!bIsGoodDetail)
      return FALSE;

    DWORD newVidRateToSend = newVidRate ? newVidRate : pChannel->GetRate();

    TRACEINTO << "CH323Cntl::OnConfFlowControlReq  - newVidRateToSend = " << newVidRateToSend << "\n";
        OnPartyFlowControlReq(pChannel,newVidRateToSend);
    if (pChannel->IsOutgoingDirection() && newVidRate) //outgoing channel
    { //update the rates at the party's level
      m_pParty->UpdateVideoRate(newVidRateToSend);
    }

    /* For incoming channel, the rates at the party's level aren't updated.
          The reason is that the party should store the originate rates, so in case a request
      to return to the origin incoming rate is sent, the origin outgoing rates must be
      restored as well. */
        if (pLprChangeModeParams)
    SendLprReqToMfa(statOK, pLprChangeModeParams->lossProtection, pLprChangeModeParams->mtbf, pLprChangeModeParams->congestionCeiling
        , pLprChangeModeParams->fill, pLprChangeModeParams->modeTimeout);

    return TRUE;
  }
  else
  {
    PTRACE(eLevelError,"CH323Cntl::OnConfFlowControlReq - Channel not found");
    return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendLprAckToParty()
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::SendLprAckToParty - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());
  // Sending the request
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_LPR_MODE_CHANGE_RES_REQ,NULL,m_serviceId,
      m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
}


////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendNewITPSpeakerToParty(eTelePresencePartyType  itpType, DWORD numOfActiveLinks)  //added by Jason for ITP-Multiple channels begin
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::SendNewITPSpeakerToParty - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());
  mcReqNewITPSpeaker* pNewITPSpeakerReq = new mcReqNewITPSpeaker;
  pNewITPSpeakerReq->ITPType = itpType;
  pNewITPSpeakerReq->numOfActiveLinks = numOfActiveLinks;
  CSegment* pMsg = new CSegment;
  pMsg->Put((BYTE*)pNewITPSpeakerReq, sizeof(mcReqNewITPSpeaker));

  // Sending the request
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_NEW_ITP_SPEAKER_REQ, pMsg,m_serviceId,
      m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
  POBJDELETE(pMsg);
  PDELETE(pNewITPSpeakerReq);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendNewITPSpeakerAckToParty()  //added by Jason for ITP-Multiple channels end
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::SendNewITPSpeakerAckToParty - Conn Id = ", m_pCsRsrcDesc->GetConnectionId());
  // Sending the request
  m_pCsInterface->SendMsgToCS(H323_CS_SIG_NEW_ITP_SPEAKER_ACK_REQ,NULL,m_serviceId,
      m_serviceId,m_pDestUnitId,m_callIndex,0,0,0);
}


////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetRssInfo(CSegment* pMsg)
{
  PTRACE(eLevelInfoNormal, "CH323Cntl::SetRssInfo");
  BYTE isStreaming = FALSE;
  WORD stringLen = 0;
  char exchangeConfId[512];
  memset(exchangeConfId, 0, 512);

    *pMsg >> isStreaming;
    *pMsg >> stringLen;
  if (stringLen)
  {
    pMsg->Get((BYTE *)exchangeConfId, stringLen);
  }

  if(isStreaming)
  {
    PTRACE2INT(eLevelInfoNormal, "CH323Cntl::SetRssInfo streaming - ", isStreaming);
    m_isStreaming = isStreaming;
  }

  if(stringLen)
  {
    PTRACE2(eLevelInfoNormal, "CH323Cntl::SetRssInfo exchange ID - ", exchangeConfId);
    m_pExchangeConfId = new BYTE[stringLen+1];
    memcpy(m_pExchangeConfId, exchangeConfId, stringLen );
    m_pExchangeConfId[stringLen] = '\0';
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendRssRequest()
{
  PTRACE(eLevelInfoNormal, "CH323Cntl::SendRssRequest");

  if(m_isStreaming)
  {
    mcReqRssCommand* pIsStreamingRssReq = new mcReqRssCommand;
    pIsStreamingRssReq->subOpcode     = eRssCmdLiveStream;
    pIsStreamingRssReq->data.length   = 0;

      CSegment* pSendMsg1 = new CSegment;
      pSendMsg1->Put((BYTE*)pIsStreamingRssReq, sizeof(mcReqRssCommand));
    m_pCsInterface->SendMsgToCS(H323_CS_SIG_RSS_CMD_REQ, pSendMsg1, m_serviceId,
                    m_serviceId, m_pDestUnitId, m_callIndex, 0,0,0);
    POBJDELETE(pSendMsg1);
      PDELETE(pIsStreamingRssReq);
  }

  if(m_pExchangeConfId != NULL)
  {
    DWORD stringLen = strlen((const char*)m_pExchangeConfId);
    DWORD msgSize = sizeof(mcReqRssCommand) + stringLen;
    mcReqRssCommand* pExchangeConfIdRssReq = (mcReqRssCommand*)new BYTE[msgSize];
    pExchangeConfIdRssReq->subOpcode    = eRssCmdExchangeID;
    pExchangeConfIdRssReq->data.length    = stringLen;
    memcpy(pExchangeConfIdRssReq->data.paramBuffer, m_pExchangeConfId, stringLen);

      CSegment* pSendMsg2 = new CSegment;
      pSendMsg2->Put((BYTE*)pExchangeConfIdRssReq, msgSize);
    m_pCsInterface->SendMsgToCS(H323_CS_SIG_RSS_CMD_REQ, pSendMsg2, m_serviceId,
                    m_serviceId, m_pDestUnitId, m_callIndex, 0,0,0);
    POBJDELETE(pSendMsg2);
    PDELETEA(pExchangeConfIdRssReq);
  }
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::UpdateDbLprSyncStatus(BYTE isSynced, BYTE isLocal)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::UpdateDbLprSyncStatus: Name - ",m_pCsRsrcDesc->GetConnectionId());
  DWORD  par = 0;

  PTRACE(eLevelInfoNormal, "CH323Cntl::UpdateDbLprSyncStatus - ignoring LPR state in video indication on layout due to SRS requirements");
  //=========================================================
  // Checking if video quality indication should be updated
  // This is disabled due to SRS requirements
  //=========================================================
  //const	BYTE lprActive = !isSynced;
  //BYTE*	pLastLprActive;
  //if (isLocal)
  //{
  //	pLastLprActive = &m_outboundLprActive;
  //}
  //else
  //{
  //	pLastLprActive = &m_inboundLprActive;
  //}
  //if (lprActive != *pLastLprActive)
  //{
  //	*pLastLprActive = lprActive;
  //	PropagatePacketLostStatus(m_cmInboundPacketLossStatus, m_cmOutboundPacketLossStatus, m_inboundLprActive, m_outboundLprActive);
  //}
  //=========================================================

  if (isLocal)
    par = LOCALLPRVID;
  else
    par = REMOTELPRVID;

  par <<= 16;
  if (isSynced)
    par |= 1;
  m_pTaskApi->UpdateLprDB(LPR_SYNC,par,1);
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnRtpLprTout(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnRtpLprTout: Name - ",m_pCsRsrcDesc->GetConnectionId());
  BYTE isSync = 1;
  BYTE isLocal = 0;
  UpdateDbLprSyncStatus(isSync,isLocal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnCodianVcrVidChannelTimeout(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::OnCodianVcrVidChannelTimeout - Connection ID = ", m_pmcCall->GetConnectionId());
  // Here we will open an outgoing channel towards Codian VCR
  CapEnum h323CapCode = eH264CapCode;
  if(!m_pLocalCapH323->FindSecondBestCap(m_pRmtCapH323, h323CapCode))
  {
    CSecondaryParams secParams;
    m_bVideoOutRejected = TRUE;
    m_pTaskApi->SetSecondaryCause(SECONDARY_CAUSE_RMT_DIFF_CAPCODE,secParams);
  }
  else
  {
    OnPartyOutgoingChannelReq( h323CapCode, kRolePeople,TRUE,FALSE,TRUE);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetTotalVideoRate(DWORD rate)
{
  //PTRACE2INT(eLevelInfoNormal, "CH323Cntl::SetTotalVideoRate - new total video rate is = ", rate);
  m_pCurrentModeH323->SetTotalVideoRate(rate);
  m_pTargetModeH323->SetTotalVideoRate(rate);
}

////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::ConvertChannelTypeEnumToArrayIndex(cmCapDataType eType,ERoleLabel eRole )
{
  BYTE index =0;
  if( eType == cmCapVideo &&  eRole == kRolePeople )
    index=1;
  else if( eType == cmCapVideo )
    index=2;
  else if( eType == cmCapData )
    index=3;

  return index;
}

////////////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::GetCallRateAllowedByRemote()
{
  DWORD remoteVideoRate = m_pRmtCapH323->GetMaxVideoBitRate(eH264CapCode,cmCapReceiveAndTransmit ,kRolePeople);
  DWORD audRate = 0;
  CChannel* pOutAudio = FindChannelInList(cmCapAudio, TRUE);
  if (pOutAudio)
  {
    audRate = pOutAudio->GetRate();
  }
  else
  {
    audRate = m_pLocalCapH323->GetAudioDesiredRate();
  }
  DWORD tmpCallRate = (remoteVideoRate/10) + audRate;
  return tmpCallRate;
}

////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetH264ModeInLocalCapsAndScmAccordingToVideoPartyType(eVideoPartyType videoPartyType,APIS8 cif4Mpi)
{
  DWORD Callrate = m_pmcCall->GetRate();

  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  PASSERTMSG_AND_RETURN(NULL == pCommConf,"GetCurrentConf return NULL");
  eVideoQuality vidQuality = pCommConf->GetVideoQuality();
  Eh264VideoModeType videoModeType = TranslateCPVideoPartyTypeToMaxH264VideoModeType(videoPartyType);

  H264VideoModeDetails h264VidModeDetails;
  CH264VideoMode* pH264VidMode = new CH264VideoMode();
  pH264VidMode->GetH264VideoParams(h264VidModeDetails, Callrate, vidQuality, (Eh264VideoModeType)videoModeType);
  h264VidModeDetails.profileValue = H264_Profile_None;

  m_pLocalCapH323->SetLevelAndAdditionals(h264VidModeDetails,H264_ALL_LEVEL_DEFAULT_SAR);
  if(cif4Mpi != -1)
    m_pLocalCapH323->Set4CifMpi(2);
  else
    m_pLocalCapH323->Set4CifMpi(-1);

  //Set SCM as well
  m_pTargetModeH323->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR);

  POBJDELETE(pH264VidMode);
}

////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetH264ModeInLocalCapsForTandEP(RemoteVendorSt& stRemoteVendor)
{
  BYTE isFlagSetForThisUser = ::IsSetStaticMbForUser(stRemoteVendor.productId);

  DWORD Callrate = m_pmcCall->GetRate();
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  PASSERTMSG_AND_RETURN(NULL == pCommConf,"GetCurrentConf return NULL");
  eVideoQuality vidQuality = pCommConf->GetVideoQuality();

  eVideoPartyType videoPartyType = m_pLocalCapH323->GetCPVideoPartyType();
  Eh264VideoModeType videoModeType = TranslateCPVideoPartyTypeToMaxH264VideoModeType(videoPartyType);
  H264VideoModeDetails h264VidModeDetails;
  CH264VideoMode* pH264VidMode = new CH264VideoMode();

  FPTRACE2INT(eLevelInfoNormal,"CH323Cntl::SetH264ModeInLocalCapsAndScmForTandEP videoModeType !!!!!! = ", videoModeType);

  if(isFlagSetForThisUser)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::SetH264ModeInLocalCapsAndScmForTandEP - Static MB is set for this user - don't change caps");
  }
  else
  {
    if((videoModeType == eHD720Symmetric || videoModeType == eHD1080Asymmetric) && m_pTargetModeH323->GetConfType() == kCp)
    {

      pH264VidMode->GetH264VideoParamsForTand(h264VidModeDetails, Callrate, vidQuality, (Eh264VideoModeType)videoModeType);
      h264VidModeDetails.profileValue = H264_Profile_None;

      m_pLocalCapH323->SetLevelAndAdditionals(h264VidModeDetails,H264_ALL_LEVEL_DEFAULT_SAR);

      PTRACE(eLevelInfoNormal,"CH323Cntl::SetH264ModeInLocalCapsAndScmForTandEP - Changed HD resolution");
    }
    else
      PTRACE(eLevelInfoNormal,"CH323Cntl::SetH264ModeInLocalCapsAndScmForTandEP - Not HD MPM+ resolution - don't change caps");
  }
  POBJDELETE(pH264VidMode);
}
////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendStartPartyPreviewReqToCM(DWORD RemoteIPAddress,WORD VideoPort,cmCapDirection Direction,CapEnum capEnum)
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::SendStartPartyPreviewReqToCM - ",m_pMfaInterface->GetConnectionId());
  mcReqCmStartPreviewChannel* pStruct = new mcReqCmStartPreviewChannel;
  memset(pStruct,0,sizeof(mcReqCmStartPreviewChannel));

  //Remote ip
  pStruct->remotePreviewAddress.ipVersion = eIpVersion4;
  pStruct->remotePreviewAddress.port = VideoPort;
  pStruct->remotePreviewAddress.distribution = eDistributionUnicast;
  pStruct->remotePreviewAddress.transportType = eTransportTypeUdp;
  pStruct->remotePreviewAddress.addr.v4.ip = RemoteIPAddress;

  pStruct->channelType = (APIU32)kIpVideoChnlType;
  pStruct->channelDirection =(APIU32)Direction;

  if(capEnum == eH261CapCode)
    pStruct->payloadType = H261previewStreamPT;
  if(capEnum == eH263CapCode)
  {
    CChannel *pChannel = NULL;
    if(Direction == cmCapReceive)
      pChannel = FindChannelInList(cmCapVideo, FALSE, kRolePeople); //incoming channel
    else //Transmit
      pChannel = FindChannelInList(cmCapVideo, TRUE, kRolePeople); //outgoing channel
    if(CPObject::IsValidPObjectPtr(pChannel))
    {
      if(pChannel->IsH263Plus())
        pStruct->payloadType = H263PLUSpreviewStreamPT;
      else
        pStruct->payloadType = H263previewStreamPT;
    }
  }

  if(capEnum == eH264CapCode)
    pStruct->payloadType = H264previewStreamPT;


  SendMsgToMpl((BYTE*)(pStruct), sizeof(mcReqCmStartPreviewChannel), IP_CM_START_PREVIEW_CHANNEL);
  PDELETE(pStruct);

}
////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SendStopPartyPreviewReqToCM(cmCapDirection Direction)
{
  PTRACE2INT(eLevelInfoNormal, "CH323Cntl::SendStopPartyPreviewReqToCM - ",m_pMfaInterface->GetConnectionId());

  mcReqCmCloseUdpPort *pStruct = new mcReqCmCloseUdpPort;
  memset(pStruct,0,sizeof(mcReqCmCloseUdpPort));

  pStruct->channelType = (APIU32)kIpVideoChnlType;
  pStruct->channelDirection =(APIU32)Direction;

  SendMsgToMpl((BYTE*)(pStruct), sizeof(mcReqCmCloseUdpPort), IP_CM_STOP_PREVIEW_CHANNEL);
  PDELETE(pStruct);

}
////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::UpdateRtpOnLeaderStatus(BYTE isLeader)
{
  FECC_PARTY_TYPE_S* pStruct = new FECC_PARTY_TYPE_S;
  memset(pStruct,0,sizeof(FECC_PARTY_TYPE_S));

  if (isLeader)
  {
    pStruct->uFeccPartyType = eFeccPartyTypeChairperson;
  }
  else if (m_pTargetModeH323->GetConfType() == kCop)
  {
    pStruct->uFeccPartyType = eFeccPartyTypeNone;
  }
  else  // CP / VSW
  {
    pStruct->uFeccPartyType = eFeccPartyTypeRegular;
  }

  SendMsgToMpl((BYTE*)(pStruct), sizeof(FECC_PARTY_TYPE_S), IP_RTP_SET_FECC_PARTY_TYPE);
  PDELETE(pStruct);
}
////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetCopVideoTxModes(CCopVideoTxModes* pCopVideoModes)
{
  POBJDELETE(m_pCopVideoModes);
  m_pCopVideoModes = new CCopVideoTxModes(*pCopVideoModes);
  m_pCopVideoModes->Dump("CH323Cntl::SetCopVideoTxModes", eLevelInfoNormal);

}
////////////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::TranslateH239OpcodeToPPCOpcode(mcIndRoleToken *pRoleToken)
{
  DWORD Opcode = 0;
  APIU32 H239Opcode = pRoleToken->subOpcode;
  BYTE Ack_Nak = pRoleToken->bIsAck;
  BYTE  symmetryBreaking = pRoleToken->randNumber;

  PTRACE(eLevelInfoNormal,"CH323Cntl::TranslateH239OpcodeToPPCOpcode");

  switch (H239Opcode)
  {
    case kPresentationTokenRequest:
    {
      if (IsSlaveCascadeModeForH239())
        //Withdraw
        Opcode = PARTY_TOKEN_WITHDRAW;
            else
                Opcode = PARTY_TOKEN_ACQUIRE;
      break;
    }
    case kPresentationTokenResponse:
    {
       if (IsSlaveCascadeModeForH239())
       {
         OPCODE subOpCode;
         if (Ack_Nak)
         {
              PTRACE(eLevelInfoNormal,"CH323Cntl::TranslateH239OpcodeToPPCOpcode: Role token ACK - send to  conf level!");
              Opcode = PARTY_TOKEN_ACQUIRE_ACK;
           }
         else
         {
            PTRACE(eLevelInfoNormal,"CH323Cntl::TranslateH239OpcodeToPPCOpcode: Role token NAK - send to  conf level!");
           Opcode = PARTY_TOKEN_ACQUIRE_NAK;
         }
       }
       else
       {
         PTRACE2INT(eLevelInfoNormal,"CH323Cntl::TranslateH239OpcodeToPPCOpcode #1 " ,Ack_Nak);
         if (Ack_Nak)
           Opcode = PARTY_TOKEN_WITHDRAW_ACK;
       }
      break;
    }
    case kPresentationTokenRelease:
    {
      Opcode = PARTY_TOKEN_RELEASE;
      break;
    }
    case kPresentationTokenIndicateOwner:
    {
      Opcode = ROLE_PROVIDER_IDENTITY;
      break;
    }
    case kFlowControlReleaseRequest:
    {
      Opcode = FLOW_CONTROL_RELEASE_REQ;
      break;
    }
    default:
    {
      PTRACE2INT(eLevelError,"CH323Cntl::TranslateH239OpcodeToPPCOpcode : Unknown Sub Message Identifier",H239Opcode);
    }
  }
  return Opcode;
}
////////////////////////////////////////////////////////////////////////////////
DWORD CH323Cntl::TranslateEPCOpcodeToPPCOpcode(mcIndRoleToken *pRoleToken)
{
  //No cascade in EPC

  DWORD Opcode = 0;
  APIU32 EPCOpcode = pRoleToken->subOpcode;

  switch (EPCOpcode)
  {
    case kRoleTokenAcquireInd:
    {
      Opcode = PARTY_TOKEN_ACQUIRE;
      break;
    }
    case kRoleTokenReleaseInd:
    {
      Opcode = PARTY_TOKEN_RELEASE;
      break;
    }
    case kRoleProviderIdentityInd:
    {
      Opcode = ROLE_PROVIDER_IDENTITY;
      break;
    }
    case kRoleTokenWithdrawAckInd:
    {
      Opcode = PARTY_TOKEN_WITHDRAW_ACK;
      break;
    }
    default:
    {
      PTRACE2INT(eLevelError,"CIsdnVideoParty::TranslateH239OpcodeToPPCOpcode : Unknown Sub Message Identifier'",EPCOpcode);
    }
  }
  return Opcode;
}
////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::TranslatePPCOpcodeToH239Opcode(DWORD EPCOpcode,mcReqRoleTokenMessage **pReq)
{

  switch (EPCOpcode)
  {
    case CONTENT_ROLE_TOKEN_WITHDRAW:
    {
      (*pReq)->subOpcode = kPresentationTokenRequest;
      (*pReq)->bIsAck = 0;
      break;
    }
    case CONTENT_ROLE_TOKEN_WITHDRAW_ACK:
    {
      (*pReq)->subOpcode = kPresentationTokenResponse;
      (*pReq)->bIsAck = 1;
      break;
    }
    case CONTENT_ROLE_PROVIDER_IDENTITY:
    {
      (*pReq)->subOpcode = kPresentationTokenIndicateOwner;
      (*pReq)->bIsAck = 0;
      break;
    }
    case CONTENT_ROLE_TOKEN_ACQUIRE:
    {
      (*pReq)->subOpcode = kPresentationTokenRequest;
      (*pReq)->bIsAck = 0;
      break;
    }
    case CONTENT_ROLE_TOKEN_ACQUIRE_ACK:
    {
      (*pReq)->subOpcode = kPresentationTokenResponse;
      (*pReq)->bIsAck = 1;
      break;
    }
    case CONTENT_ROLE_TOKEN_ACQUIRE_NAK:
    {
      (*pReq)->subOpcode = kPresentationTokenResponse;
      (*pReq)->bIsAck = 0;
      break;
    }
    case CONTENT_ROLE_TOKEN_RELEASE:
    {
      (*pReq)->subOpcode = kPresentationTokenRelease;
      (*pReq)->bIsAck = 0;
      break;
    }
    case CONTENT_ROLE_TOKEN_RELEASE_ACK:
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::TranslatePPCOpcodeToH239Opcode - CONTENT_ROLE_TOKEN_RELEASE_ACK - not supported in H239 - Do nothing. ");
      break;
    }
    case CONTENT_NO_ROLE_PROVIDER:
    {
      PTRACE(eLevelInfoNormal,"CH323Cntl::TranslatePPCOpcodeToH239Opcode - CONTENT_NO_ROLE_PROVIDER - not supported in H239 - Do nothing. ");
      break;
    }
    case FLOW_CONTROL_RELEASE_NAK:
    {
      (*pReq)->subOpcode = kFlowControlReleaseResponse;
      (*pReq)->bIsAck = 0;
      break;
    }
    default:
      PTRACE2INT(eLevelInfoNormal,"CH323Cntl::TranslatePPCOpcodeToH239Opcode: Unknown sub opcode %d",EPCOpcode);
  }


}
////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::TranslatePPCOpcodeToEPCOpcode(DWORD EPCOpcode,mcReqRoleTokenMessage **pReq)
{
  switch (EPCOpcode)
    {
      case CONTENT_ROLE_TOKEN_WITHDRAW:
      {
        (*pReq)->subOpcode = kRoleTokenWithdrawReq;
        (*pReq)->bIsAck = 0;
        break;
      }
      case CONTENT_ROLE_TOKEN_WITHDRAW_ACK:
      {
        (*pReq)->subOpcode = kRoleTokenWithdrawAckReq;
        (*pReq)->bIsAck = 1;
        break;
      }
      case CONTENT_ROLE_PROVIDER_IDENTITY:
      {
        (*pReq)->subOpcode = kRoleProviderIdentityReq;
        (*pReq)->bIsAck = 0;
        break;
      }
      case CONTENT_ROLE_TOKEN_ACQUIRE:
      {
        (*pReq)->subOpcode = kRoleTokenAcquireReq;
        (*pReq)->bIsAck = 0;
        break;
      }
      case CONTENT_ROLE_TOKEN_ACQUIRE_ACK:
      {
        (*pReq)->subOpcode = kRoleTokenAcquireAckReq;
        (*pReq)->bIsAck = 1;
        break;
      }
      case CONTENT_ROLE_TOKEN_ACQUIRE_NAK:
      {
        (*pReq)->subOpcode = kRoleTokenAcquireNakReq;
        (*pReq)->bIsAck = 0;
        break;
      }
      case CONTENT_ROLE_TOKEN_RELEASE:
      {
        (*pReq)->subOpcode = kRoleTokenReleaseReq;
        (*pReq)->bIsAck = 0;
        break;
      }
      case CONTENT_ROLE_TOKEN_RELEASE_ACK:
      {
        (*pReq)->subOpcode = kRoleTokenReleaseAckReq;
        (*pReq)->bIsAck = 0;
        break;
      }
      case CONTENT_NO_ROLE_PROVIDER:
      {
        (*pReq)->subOpcode = kNoRoleProviderReq;
        (*pReq)->bIsAck = 0;
        break;
      }
      default:
          PTRACE2INT(eLevelInfoNormal,"CH323Cntl::TranslatePPCOpcodeToEPCOpcode: Unknown sub opcode %d",EPCOpcode);

    }
}

////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetSrcSigAddressAccordingToDestAddress()
{
    CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
    CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
    PASSERTMSG_AND_RETURN(NULL == pServiceParams,"couldn't find Ip Service");
    mcTransportAddress sourceAddr;
    memset(&sourceAddr, 0, sizeof(mcTransportAddress));
    ipAddressIf localIpAddr;
    const mcTransportAddress* destCallSignalAddress = m_pH323NetSetup->GetTaDestPartyAddr();
    if(destCallSignalAddress->ipVersion == (APIU32)eIpVersion4 )
    {
        localIpAddr = pServiceParams->GetIpV4Address();
        sourceAddr.ipVersion = eIpVersion4;
    }
    else
    {
      //BRIDGE-15392: fill IPv6 src address in GkServer
    	if(::isApiTaNull(destCallSignalAddress) == FALSE)
	{
	        BYTE place = ::FindIpVersionScopeIdMatchBetweenPartyAndService(destCallSignalAddress, pServiceParams);
	        if (place == 0xFF)
	        {
	            PASSERTMSG(4,"CH323Cntl::SetSrcSigAddressAccordingToDestAddress - No valid ipv6 address in service");
	            return;
	        }
	        localIpAddr = pServiceParams->GetIpV6Address((int)place);
	}
        sourceAddr.ipVersion = eIpVersion6;
    }
    memcpy(&(sourceAddr.addr),&localIpAddr,sizeof(ipAddressIf));
    m_pmcCall->SetSrcTerminalCallSignalAddress(sourceAddr);
    m_pH323NetSetup->SetTaSrcPartyAddr(&sourceAddr);

    char szIP[64];
    ::ipToString(sourceAddr, szIP, 1);
    PTRACE2 (eLevelInfoNormal, "CH323Cntl::SetSrcSigAddressAccordingToDestAddress - set to ", szIP);
}
////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::SetH264VideoParams(H264VideoModeDetails h264VidModeDetails, APIS32 sar, cmCapDirection direction)
{
  m_pTargetModeH323->SetH264VideoParams(h264VidModeDetails, sar,direction);
}


H264VideoModeDetails CH323Cntl::GetH264ModeAccordingToRemoteVideoPartyType(eVideoPartyType videoPartyType) const
{
  DWORD Callrate = m_pmcCall->GetRate();
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  eVideoQuality vidQuality = eVideoQualityAuto;
  if(pCommConf)
    vidQuality = pCommConf->GetVideoQuality();
  else
    PASSERTMSG(NULL == pCommConf,"GetCurrentConf return NULL");

  Eh264VideoModeType videoModeType = TranslateCPVideoPartyTypeToMaxH264VideoModeType(videoPartyType);

  H264VideoModeDetails h264VidModeDetails;
  CH264VideoMode* pH264VidMode = new CH264VideoMode();
  pH264VidMode->GetH264VideoParams(h264VidModeDetails, Callrate, vidQuality, (Eh264VideoModeType)videoModeType);
  POBJDELETE(pH264VidMode);

  return h264VidModeDetails;

}
////////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsWaitForContentOutToBeOpen()
{
  BYTE ret = FALSE;
  CChannel *pChannel = NULL;

  pChannel = FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation); // content out channel
  if (pChannel)
  {
    ret = (((pChannel->GetRtpPortChannelState()==kRtpPortOpenSent)
          || (pChannel->GetCmUdpChannelState()==kSendOpen)
          || pChannel->IsCsChannelStateConnecting())
          && (!pChannel->GetIsRejectChannel())) ? TRUE : FALSE;
  }
  return ret;
}

////////////////////////////////////////////////////////////////////////////
void CH323Cntl::OnH323RtpSelfVideoUpdatePicReq(CSegment* pParam) // Intra request From RTP to encoder
{ // This indication takes care of RTP indications - Pass to the ENCODER.
  PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnH323RtpSelfVideoUpdatePicReq - Conn Id = ",m_pCsRsrcDesc->GetConnectionId());

  APIU32 status = 0;

  *pParam >> status;

  if (status == STATUS_FAIL) {

    CChannel* pChannel = FindChannelInList(cmCapVideo,FALSE,kRolePeople);

    if (pChannel) {
      BYTE bIsGradualIntra = FALSE;
      CSegment*  seg = new CSegment;
      *seg << (WORD)Fast_Update << (WORD)kRolePeople << (WORD)1 << bIsGradualIntra;

      m_pTaskApi->IpRmtH230(seg); // forward task to party manager
      POBJDELETE(seg);
    } else
      PTRACE(eLevelError,"CH323Cntl::OnH323RtpSelfVideoUpdatePicReq: No Channel video was found");

  } else
    PTRACE(eLevelError,"CH323Cntl::OnH323RtpSelfVideoUpdatePicReq - status OK, no need to send Fast_Update");

}// OnH323RtpSelfVideoUpdatePicReq
///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::IsRemoteIsRVpresentation() const
{
  BYTE ans = FALSE;
  if ( !strncmp(m_remoteVendor.productId, "RADVision ViaIP GW", strlen("RADVision ViaIP GW")) )
  {

    ans = TRUE;

  }

  return ans;
}
///////////////////////////////////////////////////////

void  CH323Cntl::SendMuteForVideoChannel(BYTE bIsActive,cmCapDataType eType) const
{
  CChannel *pChannel = FindChannelInList(eType, TRUE, kRolePeople);
  if (pChannel)
  {
      if (bIsActive)
      {//send channelActive
        PTRACE(eLevelInfoNormal,"CH323Cntl::SendMuteForVideoChannel: channel is found, status is active ");
        mcReqChannelOn sChannelOnReq;
        sChannelOnReq.channelType   = pChannel->GetType();

        CSegment* pMsg = new CSegment;
        pMsg->Put((BYTE*)(&sChannelOnReq),sizeof(mcReqChannelOn));
        m_pCsInterface->SendMsgToCS(H323_CS_SIG_CHANNEL_ON_REQ, pMsg, m_serviceId,
                m_serviceId, m_pDestUnitId, m_callIndex, pChannel->GetCsIndex(), pChannel->GetIndex(), 0);
        BYTE bIsGradualIntra = FALSE;
        CSegment*  seg = new CSegment;
        *seg << (WORD)Fast_Update << (WORD)kRolePeople << (WORD)1 << bIsGradualIntra;
        m_pTaskApi->IpRmtH230(seg); // forward task to party manager

        POBJDELETE(seg);
        POBJDELETE(pMsg);
      }
      else
      {
        PTRACE(eLevelInfoNormal,"CH323Cntl::SendMuteForVideoChannel: channel is found, status is inactive ");
        mcReqChannelOff sChannelOffReq;
        sChannelOffReq.channelType    = pChannel->GetType();
        CSegment* pMsg = new CSegment;
        pMsg->Put((BYTE*)(&sChannelOffReq),sizeof(mcReqChannelOff));
        m_pCsInterface->SendMsgToCS(H323_CS_SIG_CHANNEL_OFF_REQ, pMsg, m_serviceId,
                m_serviceId, m_pDestUnitId, m_callIndex, pChannel->GetCsIndex(), pChannel->GetIndex(), 0);
        POBJDELETE(pMsg);
      }

  }
  else
    PTRACE(eLevelInfoNormal,"CH323Cntl::SendMuteForVideoChannel: Outgoing people channel wasn't found");

}
///////////////////////////////////////////////////////////////////////////////////
void CH323Cntl::RemoveTransmitCaps()
{

  PTRACE(eLevelInfoNormal,"CH323Cntl::RemoveTransmitCaps: ");
  m_pLocalCapH323->RemoveTransmitCapsFromCapBuffer();
  COstrStream msg;
  m_pLocalCapH323->Dump(msg);
  //PTRACE2(eLevelInfoNormal,"CH323Cntl::RemoveTransmitCaps - new caps ", msg.str().c_str());
}
//////////////////////////////////////////////////
BYTE CH323Cntl::IsNeedToSendTrasmitCap()
{
  BYTE retVal = FALSE;
  if(m_pTargetModeH323->GetConfType() == kCop)
  {//tbd-check what to do with RMX 1000
    if(m_pCopVideoModes)
    {
      if(strstr(m_remoteVendor.productId, "RMX") && (!strstr(m_remoteVendor.productId, "Polycom RMX1000")|| strstr(m_remoteVendor.productId, "ACCORD MGC / Polycom RMX1000_2000") ))
      {
        if(m_remoteVendor.isCopMcu)
          return TRUE;
      }
    }
  }
//noa temp just for test
  //retVal = TRUE;
  //noa end temp

  return retVal;
}
///////////////////////////////////////////
BYTE CH323Cntl::IsNeedToCreateRemoteTxMode()
{
  if(m_pTargetModeH323->GetConfType() == kCop)
  {
    if(m_pCopVideoModes)
    {
      if(strstr(m_remoteVendor.productId, "RMX"))
      {
        if(m_remoteVendor.isCopMcu)  //TBD -what about RMX 1000
        {
          if(m_pCopRemoteVideoModes)
          {
            PTRACE(eLevelInfoNormal,"CH323Cntl::IsNeedToCreateRemoteTrasmitTrasmitCap:there are already m_pCopRemoteVideoModes ");
            DBGPASSERT(109);
          }
          return TRUE;
        }


      }

    }
  }
  return FALSE;
}
//////////////////////////////////////////////////////
BYTE CH323Cntl::IsNeedToOPenAccordingToRemoteTxModes()
{
  if(m_pTargetModeH323->GetConfType() == kCop)
  {
    if(m_pCopVideoModes)
    {
      if(strstr(m_remoteVendor.productId, "RMX"))
      {
        if( IsRemoteRmx1000or500() && cmMSMaster == m_pmcCall->GetMasterSlaveStatus() )
        {
          PTRACE(eLevelInfoNormal,"CH323Cntl::IsNeedToOPenAccordingToRemoteTxModes -we are the master to RMX 1000 no need to consider Remote TX mode");
          return FALSE;

        }
        else if(m_pCopRemoteVideoModes)
        {
          return TRUE;
        }




      }

    }
  }
  return FALSE;
}
void CH323Cntl::AdjustRMX1000levelsToRMX2000levels()
{
  if(m_pCopRemoteVideoModes)
  {
    int i;
    for (i=0;i<NUMBER_OF_COP_LEVELS;i++)
    {
      int levelBitRate = m_pCopRemoteVideoModes->GetVideoMode(i)->GetBitRate() * 100;
      //CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
      //DWORD levelBitRate = lCapInfo.TranslateReservationRateToIpRate(levelRate);
      //levelBitRate *= 1000;
      DWORD audioRate = CalculateAudioRate(levelBitRate);
      DWORD videoRate = levelBitRate - audioRate;
      videoRate /= 100; // system units
      m_pCopRemoteVideoModes->GetVideoMode(i)->SetBitRate(videoRate);
      CVidModeH323* pVidMode;
      pVidMode = m_pCopRemoteVideoModes->GetVideoMode(i);
      if(pVidMode->GetType() == eH264CapCode)
      {
    	  APIU16 profile = 0;
    	  APIU8 level=0;
        long mbps=0, fs=0, dpb=0, brAndCpb=0, sar=0, staticMB=0;
        pVidMode->GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
        if(level == 85 && profile == H264_Profile_BaseLine)
        {
          PTRACE(eLevelInfoNormal,"CH323Cntl::AdjustRMX1000levelsToRMX2000levels - changing 1080 level to fit our level");
          pVidMode->SetH264Scm(H264_Profile_BaseLine, 71, 490, 32, -1, -1, -1, -1, cmCapTransmit);
        }

      }

    }

  }
  else
    PTRACE(eLevelInfoNormal,"CH323Cntl::AdjustRMX1000levelsToRMX2000levels -no remote mode");

}
///////////////////////////////////////////////////////
BYTE CH323Cntl::IsRemoteRmx1000or500()
{
  if(( strstr(m_remoteVendor.productId, "RMX1000") || strstr(m_remoteVendor.productId, "RMX500")) && !strstr(m_remoteVendor.productId, "ACCORD MGC / Polycom RMX1000_2000") )
    return TRUE;
  return FALSE;
}


//////////////////////////////////////
void CH323Cntl::SetITPRtcpMask()
{
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  BYTE isRoomSwitch = NO;
  if(pCommConf && CPObject::IsValidPObjectPtr(pCommConf) && (pCommConf->GetTelePresenceLayoutMode() == eTelePresenceLayoutRoomSwitch ||
                                                             pCommConf->GetTelePresenceLayoutMode() == eTelePresenceLayoutCpSpeakerPriority))
    isRoomSwitch = YES;

  m_RtcpCnameMask = ::BuildRtcpCnameMask(isRoomSwitch);
}
////////////////////////////////////
APIU16 CH323Cntl::RetriveCnameInfoFromEpIfPosible(char *pCnameString)
{

  PTRACE2(eLevelInfoNormal,"CH323Cntl::RetriveCnameInfoFromEpIfPosible",pCnameString);
  char siteName[MAX_SITE_NAME_ARR_SIZE]; //33
  memset(siteName,'\0',MAX_SITE_NAME_ARR_SIZE);
  char mask[MASK_LENGTH_CHAR + 1];

  if(!strstr(pCnameString,"Polycom"))
    return FALSE;

  char* startmask;
  startmask = strstr(pCnameString, "@");

  if(!startmask)
    return FALSE;
  else
  {
    startmask++;
    char* endOfMask = strstr(pCnameString, " ");
    if(!endOfMask)
      return FALSE;
    endOfMask--;
    if((endOfMask - startmask) != 3 )
    {
      return FALSE;
    }


    strncpy(mask, startmask,( (endOfMask - startmask) + 1) );
    strncpy(siteName,(endOfMask + 2),MAX_SITE_NAME_ARR_SIZE );
    mask[MASK_LENGTH_CHAR] = '\0';
    siteName[MAX_SITE_NAME_ARR_SIZE - 1] = '\0';
  }

  for (int i=0; H323InvalidDisplayChars[i] != '\0'; i++)
  {
    CSmallString::ReplaceChar(siteName, H323InvalidDisplayChars[i],'.');
  }

  //APIU16 CnameMaskWord = atoi(mask);
  DWORD CnameMaskWord = 0;
  CnameMaskWord = strtol (mask,NULL,16);


  if(CnameMaskWord & ITP_MCU)
    return FALSE;

  eTelePresencePartyType  eLocalTelePresencePartyType = eTelePresencePartyNone;

  if(CnameMaskWord & ITP_RPX)
  {
    PTRACE(eLevelInfoNormal,"CH323Cntl::RetriveCnameInfoFromEpIfPosible - RPX");
    eLocalTelePresencePartyType = eTelePresencePartyRPX;
  }
  if(CnameMaskWord & ITP_FLEX)
  {
	  if( (CnameMaskWord & CAM_CAP_MASK) )
	  {
		  PTRACE(eLevelInfoNormal,"CH323Cntl::RetriveCnameInfoFromEpIfPosible - MAUI");
		  eLocalTelePresencePartyType = eTelePresencePartyMaui;
	  }
	  else
	  {
		  PTRACE(eLevelInfoNormal,"CH323Cntl::RetriveCnameInfoFromEpIfPosible - FLEX");
		  eLocalTelePresencePartyType = eTelePresencePartyFlex;
	  }
  }

  BYTE bIsVisualName = TRUE;

  if(strlen(siteName) < 1)
    bIsVisualName = FALSE;

  PTRACE2(eLevelInfoNormal,"CH323Cntl::RetriveCnameInfoFromEpIfPosible",siteName);
  m_pTaskApi->SendSiteAndVisualNamePlusProductIdToPartyControl(bIsVisualName, siteName, FALSE /*bIsProductId*/, ""/*productId*/,FALSE /*bIsVersionId*/,NULL /*pVersionId*/, eLocalTelePresencePartyType);
  return TRUE;


}

/////////////////////////////////////////////////////////////////////////////
BOOL CH323Cntl::IsTIPContentEnable() const
{
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
  if(pCommConf)
  return pCommConf->GetIsTipCompatibleContent();
  else
  {
    PASSERTMSG(NULL == pCommConf,"pCommConf is NULL");
    return FALSE;
  }
}

/////////////////for EXT-4632, VNGR-20237 //////////////////////////////////
BOOL CH323Cntl::CheckAndMakeH323CallOnGKFail()
{
    m_CallConnectionState = ReleasePort;
    if(NULL == m_pmcCall)
    {
        PTRACE(eLevelInfoNormal,"CH323Cntl::CheckAndMakeH323CallOnGKFail: ERROR!--m_pmcCall is NULL!");
        return FALSE;
    }

    if (m_pmcCall->GetIsOrigin()) //Call out
    {
        if(NULL == m_pH323NetSetup)
        {
            PTRACE(eLevelInfoNormal,"CH323Cntl::CheckAndMakeH323CallOnGKFail: ERROR!--m_pH323NetSetup is NULL when calling out!");
            return FALSE;
        }
        BOOL  bFlagForCallOut = GetSystemCfgFlagInt<BOOL>(CFG_KEY_GK_MANDATORY_FOR_CALLS_OUT);
        const mcTransportAddress* pDestPartyTa = m_pH323NetSetup->GetTaDestPartyAddr();
        if(isApiTaNull(const_cast<mcTransportAddress*>(pDestPartyTa)) != TRUE)
        {
            if(bFlagForCallOut)
            {
                PTRACE(eLevelInfoNormal,"CH323Cntl::CheckAndMakeH323CallOnGKFail: Cannot make call out!--GK_MANDATORY_FOR_CALLS_OUT is YES!");
                return FALSE; // Cannot make call out!
            }
            else
            {
                PTRACE(eLevelInfoNormal,"CH323Cntl::CheckAndMakeH323CallOnGKFail: Make call out!--GK_MANDATORY_FOR_CALLS_OUT is NO!");
                m_CallConnectionState = Idle;
		OnPartyCallSetupReq();
                return TRUE; // Can make call out without GK
            }
        }
        else // No IP address.
        {
                PTRACE(eLevelInfoNormal,"CH323Cntl::CheckAndMakeH323CallOnGKFail: Cannot make call out!--There's no IP address for the called participant!");
                return FALSE; // Cannot make call out!
        }
    }
    else
    {
        BOOL  bFlagForCallIn = GetSystemCfgFlagInt<BOOL>(CFG_KEY_GK_MANDATORY_FOR_CALLS_IN);
        if(bFlagForCallIn)
        {
                PTRACE(eLevelInfoNormal,"CH323Cntl::CheckAndMakeH323CallOnGKFail: Cannot let call in!--GK_MANDATORY_FOR_CALLS_IN is YES!");
                return FALSE; // reject call
        }
        else
        {
            PTRACE(eLevelInfoNormal,"CH323Cntl::CheckAndMakeH323CallOnGKFail: Let call in!--GK_MANDATORY_FOR_CALLS_IN is NO!");
            m_CallConnectionState = Idle;
            OnPartyCallAnswerReq(); // answer dial-in calls
            return TRUE;
        }
    }
    PTRACE(eLevelInfoNormal,"CH323Cntl::CheckAndMakeH323CallOnGKFail: If you can see this message, something must be wrong...");
    return FALSE;
}


//////////////////////////////////////////////////////
void CH323Cntl::InitSpeakerParams()
{
	m_speakerMcuNum  = 0;
	m_speakerTermNum = 0;
	m_speakerPartyId = 0;
}

////////////////////////////////////////////////////
void CH323Cntl::OnMrmpRtcpFirInd(CSegment* pParam)
{
    m_pTaskApi->HandleMrmpRtcpFirInd(pParam);
}
//////////////////////////////////////////////////
int CH323Cntl::OpenInternalArts(ENetworkType networkType)
{
    m_state = SETUP;
    return CIpCntl::OpenInternalArts(networkType);
}

/////////////////////////////////////////////////////////////////////////
int CH323Cntl::OpenInternalChannelsByMedia(cmCapDataType aDataType, DWORD bitRate, EConnectionState iChannelsInState /*-1*/)
{
    TRACEINTO << "Open internal channels for " << ::GetTypeStr(aDataType);
    CapEnum protocol = eUnknownAlgorithemCapCode;
    if (aDataType == cmCapAudio)
    {
        if(IsSoftMcu())
            return 0; // no audio channels for soft MCU
        protocol = eSirenLPR_Scalable_48kCapCode;

    }
    else if (aDataType == cmCapVideo)
    {
        protocol = eSvcCapCode;
    }
    else
        // no internal channels for this data type
        return 0;

    int iNumOfChannels = 0;
    int iNumOfSentChannels = 0;
    CChannel* pChannel = NULL;

    // go over target mode and for each stream in the target mode which is not in the current mode,
    // open an internal channel

//    m_pTargetModeH323->Dump("CH323Cntl::OpenInternalChannelsByMedia Target mode", eLevelInfoNormal);
    APIU32* ssrcIds = NULL;
    int numOfSsrcIds = 0;
    m_pTargetModeH323->GetSsrcIds(aDataType, cmCapReceive, ssrcIds, &numOfSsrcIds);

    APIU32* currentSsrcIds = NULL;
    int currentNumOfSsrcIds = 0;
    m_pCurrentModeH323->GetSsrcIds(aDataType, cmCapReceive, currentSsrcIds, &currentNumOfSsrcIds);

    if (numOfSsrcIds > GetNumberOfActiveInternalArts())
    {
        numOfSsrcIds = GetNumberOfActiveInternalArts();
    }

    TRACEINTO << "mix_mode: Open ART channel numOfSsrcIds=" << numOfSsrcIds;

    bool found = false;
    for (int i = 0; i < numOfSsrcIds; i++)
    {
        // check if the channel is already open - i.e. exists in current mode
        found = false;
        for (int j = 0; !found && j < currentNumOfSsrcIds; j++)
        {
            if (ssrcIds[i] == currentSsrcIds[j])
            {
                found = true;
            }
        }
        if (found)
        {// channel is already open, skip to next one
            TRACEINTO << "SSRC " << ssrcIds[i] << " exists in current mode, therefore a channel will not be created for it.";
            continue;
        }


        // add the channel to the channels array
        TRACEINTO << "mix_mode: add the channel to the channels array";
        pChannel = m_pmcCall->AddChannelInternal(m_pTargetModeH323, aDataType, cmCapTransmit, protocol, bitRate, ssrcIds[i]);
        if(NULL == pChannel)
        {
            TRACEINTO << "Channel " << i << " (out of " << numOfSsrcIds << " internal channels) is NULL - was not created!!!";
            delete []ssrcIds;
            delete []currentSsrcIds;
            // @#@ - indicate error somehow!
            return iNumOfSentChannels;
        }

//        if (aDataType == cmCapVideo)
        {// update connection id in channel - needed to identify the channel
            pChannel->SetRtpConnectionId(GetAvcToSvcArtConnectionId(i));
//            TRACEINTO << "@#@ Open ART video channel #" << i << " on connectionId = " << pChannel->GetRtpConnectionId();
        }

        // send message to ART
        TRACEINTO << "mix_mode: Open ART channel #" << i << " on connectionId = " << GetAvcToSvcArtConnectionId(i);
        pChannel->SetCsChannelState(kBeforeResponseReq);

        Rtp_FillAndSendUpdatePortOpenRtpStruct(pChannel, FALSE, TRUE, i);
        iNumOfSentChannels++;
    }
    if (iNumOfSentChannels>0 && currentNumOfSsrcIds==0)
    {
        // add the MRMP channel to the channels array in SipCall
        pChannel = m_pmcCall->AddChannelInternal(m_pTargetModeH323, aDataType, cmCapReceive, protocol, bitRate);
        if(NULL == pChannel)
        {
            TRACEINTOFUNC << "MRMP Channel is NULL in channels array!!!";
            delete []ssrcIds;
            delete []currentSsrcIds;
            // @#@ - indicate error somehow!
            return iNumOfSentChannels;
        }
        // open MRMP channel
        TRACEINTO << "mix_mode: Open MRMP channel";
        mcTransportAddress rmtAddress;
        memset(&rmtAddress, 0, sizeof(mcTransportAddress));
        pChannel->SetCsChannelState(kBeforeResponseReq);
        OpenSvcChannel(pChannel, rmtAddress);
    }

    delete []ssrcIds;
    delete []currentSsrcIds;

    return iNumOfSentChannels;
}

/////////////////////////////////////////////////////////////////////////
void CH323Cntl::UpdateMrmpInternalChannelIfNeeded()
{
    // update the MRMP channel with the physical info
    CChannel* pChannel = NULL;
    pChannel = m_pmcCall->GetInternalChannel(cmCapVideo, FALSE, kRolePeople);
    if (NULL == pChannel)
    {
        TRACEINTO << "mix_mode: MRMP Channel is NULL in channels array!!! No channel to update.";
        return;
    }

    // update MRMP channel
    TRACEINTO << "Update internal Rx MRMP channel.";
    mcTransportAddress rmtAddress;
    memset(&rmtAddress, 0, sizeof(mcTransportAddress));
    pChannel->SetCsChannelState(kBeforeResponseReq);
    OpenSvcChannel(pChannel, rmtAddress, true);
}

/////////////////////////////////////////////////////////////////////////
void CH323Cntl::CloseInternalChannels(cmCapDataType aDataType)
{
    // no internal channels for non-mix conference
    if (m_pCurrentModeH323->GetConfMediaType() != eMixAvcSvc)
        return;

    TRACEINTO << "aDataType=" << aDataType;

    cmCapDataType chType = aDataType;
    if (chType != cmCapAudio && chType != cmCapVideo)
    {
        TRACEINTO << "mix_mode: No channels to close for chType=" << chType;
        return;
    }


    // go over all open channels and close the CM
    CChannel *pChannel = NULL;
    if (m_pmcCall->GetNumOfInternalChannels())
    {
        for (int i=0; i < MAX_INTERNAL_CHANNELS; i++)
        {
            pChannel = m_pmcCall->GetSpecificChannel(i, false);
            TRACEINTO << "mix_mode: closing internal channel #" << i;
            if (pChannel)
                pChannel->Dump("CH323Cntl::CloseInternalChannels");
            if (pChannel && pChannel->GetCsChannelState()==kConnectedState && pChannel->GetType()==chType)
            {
                //            pChannel->Dump("CH323Cntl::CloseInternalChannels");
                if (pChannel->IsOutgoingDirection() == FALSE)
                {// this is MRMP channel - close it
                    TRACEINTO << "mix_mode: MRMP channel - close it";
                    pChannel->SetCsChannelState(kDisconnectingState);
                    CloseSvcChannel(pChannel);
                }
                else
                {// this is ART channel - no message is sent, just mark it as disconnected
                    TRACEINTO << "mix_mode: ART channel - mark it as disconnected and remove";
                    pChannel->SetCsChannelState(kDisconnectedState);
                    m_pmcCall->RemoveChannelInternal(i);
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////
int CH323Cntl::OpenInternalChannels(EConnectionState iChannelsInState)
{
	TRACEINTO<<" mix_mode: ";
	if (m_pTargetModeH323->GetConfMediaType() != eMixAvcSvc)
	{
		TRACEINTO<<"mix_mode: not in mixed mode yet - internal channels will not be opened";
		return 0;
	}

    int iNumOfSentChannels = 0;
    if(!IsSoftMcu())
    {
    	iNumOfSentChannels += OpenInternalChannelsByMedia(cmCapAudio, m_pTargetModeH323->GetMediaBitRate(cmCapAudio), iChannelsInState);
    }
   	iNumOfSentChannels += OpenInternalChannelsByMedia(cmCapVideo, m_pTargetModeH323->GetMediaBitRate(cmCapVideo), iChannelsInState);
    return iNumOfSentChannels;

}

/////////////////////////////////////////////////////////////////////////
BYTE CH323Cntl::UpgradeAvcChannelReq(CComModeH323* pTargetMode)
{
	// update ConfMediaType
	m_pTargetModeH323->SetConfMediaType(pTargetMode->GetConfMediaType());
	m_pCurrentModeH323->SetConfMediaType(pTargetMode->GetConfMediaType());

	// update audio and video
	UpdateTargetMode(pTargetMode, cmCapVideo, cmCapReceive);
	UpdateTargetMode(pTargetMode, cmCapAudio, cmCapReceive);

	BYTE bMessageSent = NO;
	TRACEINTO<<"mix_mode: before open internal channels";
	if (OpenInternalChannels(kConnecting) > 0)
	{
		bMessageSent = YES;
	}
	else
	{
		//DBGPASSERT(YES);
		PTRACE(eLevelError,"mix_mode: CSipCntl::SipNewCallReq: No requests to open internal channels."); // ey_20866
	}

	// upgrade the VSW stream
	if (pTargetMode->IsHdVswInMixMode())
	{
		// get the video channel
	    CChannel *pChannel = this->FindChannelInList(cmCapVideo, FALSE, kRolePeople);
	    if (pChannel)
	    	Rtp_FillAndSendUpdateRtpChannelStruct(pChannel);
	}

	return bMessageSent;
}

void CH323Cntl::ConnectPartyToConfWhenUpgradeToMix()
{
	BYTE bAllChannelsConnected    = AreAllChannelsConnected();
	if (!bAllChannelsConnected)
	{// not all channels connected yet
		return;
	}

	TRACEINTO<<"mix_mode: all channels are connected";
    m_state = CONNECT;

	// update current SCM with streams
	// audio
    const std::list <StreamDesc> streamsDescList = m_pTargetModeH323->GetStreamsListForMediaMode(cmCapAudio,cmCapReceive,kRolePeople);
    if (streamsDescList.size() > 0)
    {// copy streams information
        TRACEINTOFUNC << "mix_mode: Update streams in current SCM for media type = cmCapAudio";
        m_pCurrentModeH323->SetStreamsListForMediaMode(streamsDescList, cmCapAudio,cmCapReceive,kRolePeople);
        m_pCurrentModeH323->Dump("CH323Cntl::UpdateCurrentScmH323 mix_mode: Update streams in current SCM", eLevelInfoNormal);
    }

    // video
    const std::list <StreamDesc> videoStreamsDescList = m_pTargetModeH323->GetStreamsListForMediaMode(cmCapVideo,cmCapReceive,kRolePeople);
    if (videoStreamsDescList.size() > 0)
    {// copy streams information
        TRACEINTOFUNC << "mix_mode: Update streams in current SCM for media type = cmCapAudio";
        m_pCurrentModeH323->SetStreamsListForMediaMode(videoStreamsDescList, cmCapVideo,cmCapReceive,kRolePeople);
        m_pCurrentModeH323->Dump("CH323Cntl::UpdateCurrentScmH323 mix_mode: Update streams in current SCM", eLevelInfoNormal);
    }

//	if (bSendToParty)
    m_pTaskApi->H323PartyChannelsUpdated(m_pCurrentModeH323);
//	else
//		PTRACE(eLevelInfoNormal, "CSipCntl::OnMfaAckChangeMode, Not sent to Party.");
	if (IsValidTimer(PARTYCHANGEMODETOUT))
			DeleteTimer(PARTYCHANGEMODETOUT);

}

///////////////////////////////////////////////////////////////////////////
APIU32	CH323Cntl::GetMasterSlaveTerminalType()
{
	APIU32 masterSlaveTerminalType = SLAVE_NUMBER_DEFAULT;
	if(m_pParty->GetCascadeMode() == SLAVE || IsSlaveCascadeModeForH239() || m_pParty->IsGateway())
    		masterSlaveTerminalType  = SLAVE_NUMBER_DEFAULT;
  	else if (m_pParty->GetCascadeMode() == MASTER)
  	{
  		BOOL bEnableCodianCascading;
  		CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		std::string data;
		sysConfig->GetBOOLDataByKey("ENABLE_CODIAN_CASCADE", bEnableCodianCascading);
    		masterSlaveTerminalType  = ACTIVE_MC_MASTER_NUMBER;
		if(bEnableCodianCascading)
			masterSlaveTerminalType+= 5;
  	}
    	else
        	masterSlaveTerminalType  = DEFAULT_MASTER_NUMBER;
	PTRACE2INT(eLevelInfoNormal,"CH323Cntl::GetMasterSlaveTerminalType:",masterSlaveTerminalType);
	return masterSlaveTerminalType;
}

///////////////////////////////////////////////////////////////////////////
BOOL CH323Cntl::IsCallGeneratorConf() const
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

  	if (pCommConf)
  	{
  		PTRACE2INT(eLevelInfoNormal,"CH323Cntl::IsCallGeneratorConf - Is CG conf:",pCommConf->GetIsCallGeneratorConference());
		return pCommConf->GetIsCallGeneratorConference();
  	}
  	else
  	{
  		PASSERTMSG(NULL == pCommConf,"pCommConf is NULL");
	    return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
void CH323Cntl::Deescalate(CComModeH323* pTargetMode)
{
    // update audio and video
    UpdateTargetMode(pTargetMode, cmCapVideo, cmCapReceive);
    UpdateTargetMode(pTargetMode, cmCapAudio, cmCapReceive);

    DeescalateChannelsByMedia(cmCapAudio);
    DeescalateChannelsByMedia(cmCapVideo);

    const std::list <StreamDesc> videoStreamsDescList = m_pTargetModeH323->GetStreamsListForMediaMode(cmCapVideo,cmCapReceive,kRolePeople);
    if (videoStreamsDescList.size() > 0)
    {// copy streams information
        TRACEINTOFUNC << "mix_mode: Update streams in current SCM for media type = " << ::GetTypeStr(cmCapVideo);
        m_pCurrentModeH323->SetStreamsListForMediaMode(videoStreamsDescList, cmCapVideo,cmCapReceive,kRolePeople);
    }
    const std::list <StreamDesc> audioStreamsDescList = m_pTargetModeH323->GetStreamsListForMediaMode(cmCapAudio,cmCapReceive,kRolePeople);
    if (audioStreamsDescList.size() > 0)
    {// copy streams information
        TRACEINTOFUNC << "mix_mode: Update streams in current SCM for media type = " << ::GetTypeStr(cmCapAudio);
        m_pCurrentModeH323->SetStreamsListForMediaMode(audioStreamsDescList, cmCapAudio,cmCapReceive,kRolePeople);
    }

    // close unused internal ARTs
    CChannel *pAudioChannel = NULL;
    CChannel *pVideoChannel = NULL;
    for (int i = 0; i < NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS && m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface; i++)
    {
        TRACEINTO << "Checking internal ART with connectionId=" << m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->GetConnectionId();
        // find the audio channel
        pAudioChannel = m_pmcCall->GetInternalChannel(cmCapAudio, TRUE, kRolePeople, m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->GetConnectionId());
        pVideoChannel = m_pmcCall->GetInternalChannel(cmCapVideo, TRUE, kRolePeople, m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->GetConnectionId());

        if (pAudioChannel == NULL && pVideoChannel == NULL)
        {
            TRACEINTO << "ART has no connected channels. Close it. index=" << i;
            // @#@ - do it by index
            CloseInternalArtByConnId(m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->GetConnectionId());
        }
    }

}

//////////////////////////////////////////////////////////////////////////
void CH323Cntl::DeescalateChannelsByMedia(cmCapDataType aDataType)
{
    // go over current mode and for each stream in the current mode which is not in the target mode,
    // close the internal channel and the ART

    m_pTargetModeH323->Dump("CH323Cntl::Deescalate Target mode", eLevelInfoNormal);
    APIU32* ssrcIds = NULL;
    int numOfSsrcIds = 0;
    m_pTargetModeH323->GetSsrcIds(aDataType, cmCapReceive, ssrcIds, &numOfSsrcIds);

    APIU32* currentSsrcIds = NULL;
    int currentNumOfSsrcIds = 0;
    m_pCurrentModeH323->GetSsrcIds(aDataType, cmCapReceive, currentSsrcIds, &currentNumOfSsrcIds);

    TRACEINTO << "aDataType=" << ::GetTypeStr(aDataType) << " numOfSsrcIds=" << numOfSsrcIds << " currentNumOfSsrcIds=" << currentNumOfSsrcIds;
    CChannel *pChannel = NULL;
    for (int i = numOfSsrcIds; i < currentNumOfSsrcIds; i++)
    {
        // find the channel
    	pChannel = NULL;
       	if (i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS && m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface)
       	{
       		pChannel = m_pmcCall->GetInternalChannel(aDataType, TRUE, kRolePeople, m_AvcToSvcTranslatorInterface[i].pAvcToSvcTranslatorInterface->GetConnectionId());
       	}
        if (pChannel == NULL)
        {
            TRACEINTO << "No channels to close for index=" << i;
        }
        else
        {
            pChannel->SetCsChannelState(kDisconnectedState);
            m_pmcCall->RemoveChannelInternal(pChannel);

        }
    }

    delete [] ssrcIds;
    delete [] currentSsrcIds;
}


/////////////////////////////////////////////////////////
// Imported from SIP
void  CH323Cntl::OnMediaDisconnectDetectionInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CH323Cntl::OnMediaDisconnectDetectionInd");
	PASSERT_AND_RETURN(!m_pParty || !m_pParty -> GetConfApi() || !GetCallParams());

	TMediaDisconnectedIndStruct  mediaDisconnectDetectionInd;
	DWORD  structLen = sizeof(TMediaDisconnectedIndStruct);
	memset(&mediaDisconnectDetectionInd,0,structLen);
	pParam->Get((BYTE*)(&mediaDisconnectDetectionInd),structLen);

	kChanneltype   channelTpe 	=(kChanneltype) mediaDisconnectDetectionInd.unChannelType;
	BYTE		isRTPChannel	= (BYTE) mediaDisconnectDetectionInd.bIsRTP;

	CMedString str;
	str << " Channel Type= " << (DWORD) channelTpe << "; isRTP=" << (DWORD) isRTPChannel;
	PTRACE2(eLevelInfoNormal,"CH323Cntl::OnMediaDisconnectDetectionInd ", str.GetString());

	BYTE isNeedBye = 0;
	if (!m_isAudioMuted)
	{
		isNeedBye = GetCallParams()->HandleMediaDetectionInd(channelTpe, isRTPChannel);
		if(isNeedBye)
		{
			m_pTaskApi->H323PartyDisConnect(H323_CALL_CLOSED_MEDIA_DISCONNECTED);
		}
	}

	if(!isNeedBye && kIpVideoChnlType == channelTpe && isRTPChannel)
	{
		stMediaDetectionInfo& mediaDetect = GetCallParams()->MediaDetectionInfo();
		if (!m_isVideoMuted && !mediaDetect.inTimeout[MEDIA_DETECTION_VIDEO_RTP])
		{
			//==================================================================================================
			// If video RTP is disconnected long enough to be considered in timeout, we should remove its cell
			//==================================================================================================
			const std::string unmuteKey = CFG_KEY_RETURN_EP_TO_LAYOUT_ON_NO_VIDEO_TIMER;
			DWORD unmuteDuration = 0;
			CSysConfig* pSysConfig = CProcessBase::GetProcess() ? CProcessBase::GetProcess()->GetSysConfig() : NULL;
			BOOL cfgFailure = !pSysConfig || !pSysConfig->GetDWORDDataByKey(unmuteKey, unmuteDuration);
			PASSERT(cfgFailure);

			//==========================================
			// Checking if video reached timeout state
			//==========================================
			if (!cfgFailure && unmuteDuration && mediaDetect.CheckForMediaTimeout(MEDIA_DETECTION_VIDEO_RTP))
			{
				//==================================================================================
				// Muting video cell and starting timer to check when video cell should be resumed
				//==================================================================================
				PTRACE(eLevelInfoNormal,"CSipCntl::OnMediaDisconnectDetectionInd - Muting video cell");
				m_pParty -> GetConfApi() -> IpMuteMedia(m_pParty -> GetPartyId(), 	AUTO, AUTO, eOn,  AUTO,
																					AUTO, AUTO, AUTO, AUTO);
				MediaDisconnectionTimerKick();
			}
			else if (0 == unmuteDuration)
			{
				PTRACE2INT(eLevelInfoNormal,"CSipCntl::OnMediaDisconnectDetectionInd - Not checking for video cell mute, since unmuteDuration is ", unmuteDuration);
			}
		}
		else if (mediaDetect.inTimeout[MEDIA_DETECTION_VIDEO_RTP])
		{
			//====================================================================================================================
			// Video already in timeout state and a new media disconnection has been received - extending media detection period
			//====================================================================================================================
			MediaDisconnectionTimerKick();
		}
	}
}

void CH323Cntl::MediaDisconnectionTimerKick()
{
	PTRACE(eLevelInfoNormal,"CSipCntl::MediaDisconnectionTimerKick");

	const std::string unmuteKey = CFG_KEY_RETURN_EP_TO_LAYOUT_ON_NO_VIDEO_TIMER;
	DWORD unmuteDuration = 0;
	CSysConfig* pSysConfig = CProcessBase::GetProcess() ? CProcessBase::GetProcess()->GetSysConfig() : NULL;
	PASSERT_AND_RETURN(!GetCallParams() || !pSysConfig || !pSysConfig->GetDWORDDataByKey(unmuteKey, unmuteDuration));
	stMediaDetectionInfo& mediaDetect = GetCallParams()->MediaDetectionInfo();
	if (mediaDetect.inTimeout[MEDIA_DETECTION_VIDEO_RTP])
	{
		if (IsValidTimer(MEDIA_DISCONNECTION_RESUME_CELL_TIMER))
		{
			DeleteTimer(MEDIA_DISCONNECTION_RESUME_CELL_TIMER);
		}

		//==================================================================================
		// Raising the timer to at least the frequency emb will send the indications to us
		//==================================================================================
		unmuteDuration = max(unmuteDuration, mediaDetect.detectTimeLen + 2);
		StartTimer(MEDIA_DISCONNECTION_RESUME_CELL_TIMER, unmuteDuration * SECOND);
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipCntl::MediaDisconnectionTimerKick - Media not currently in timeout");
	}
}


void CH323Cntl::OnMediaResume(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipCntl::OnMediaResume - Resuming video cell");
	PASSERT_AND_RETURN(!GetCallParams() || !m_pParty || !m_pParty -> GetConfApi());
	m_pParty -> GetConfApi() -> IpMuteMedia(m_pParty -> GetPartyId(), AUTO, AUTO, eOff,  AUTO,
																				  AUTO, AUTO, AUTO, AUTO);
	GetCallParams()->MediaDetectionInfo().MediaResumed(MEDIA_DETECTION_VIDEO_RTP);
}


/////////////////////////////////////////////////////////
void  CH323Cntl::OnMediaDisconnectDetectionIndInAnycase(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CH323Cntl::OnMediaDisconnectDetectionIndInAnycase, NOT proper state: ", m_state);
	MediaDisconnectionTimerKick();
}

