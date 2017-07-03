//IpCommonDefinitions.h

#ifndef __IPCOMMONDEF_H__
#define __IPCOMMONDEF_H__

#include "DataTypes.h"
#include "IpCsSizeDefinitions.h"


//  Conference types supported:
// -----------------------------
#define		confTypeUnspecified		0x0000
#define		confTypePPCVersion0		0x0001
#define		confTypePPCVersion1		0x0002
#define		confTypeH239			0x0004
#define		confTypeDuoVideo		0x0008
#define		confTypeFECC			0x0010
#define		confTypeT120			0x0020
#define		confTypeAudioOnly		0x0040

// Party monitoring:
//------------------

typedef enum {
	unknownChannelMediaType		= 0,
	audioChannelMediaType		= 1,
	videoChannelMediaType		= 2,
	t120ChannelMediaType		= 3,
	feccChannelMediaType		= 4,
	miscChannelMediaType		= 5,

}channelMediaType;


typedef enum{
	ctCallDropNoAnswer			= 1,
	ctCallDropWithOpenChannel,
	ct_DRQ_NoAnswer,
} ctTimerExpiredReason;


typedef enum
{
	ttNoBR	 = 0,
	ttUBR	 = 1,
	ttrtVBR  = 2,
	ttnrtVBR = 3,
	ttABR    = 4,
	ttCBR    = 5
} trafficType_e;

//Transport type
//-------------------------------------------

#define TransportTypeUniDir				0x01
#define	TransportTypeBiDir				0x02
#define TransportTypeIpUdp				0x04
#define TransportTypeIpTcp				0x08

#define MaxNumOfTransportTypes			4

#define MaxTrafficTypeSize				16
#define MaxTransportTypeSize			32

// Max On Board
//-------------
// Calls
//-------
#define MaxCallIncomingChannels			(MaxDirectedChannelsPerCall + 2)
#define MaxCallOutgoingChannels			MaxDirectedChannelsPerCall

#define MaxChannels						(MaxVideoCalls * (MaxVideoDirectedChannelsPerCall + MaxAudioDirectedChannelsPerCall + MaxFeccDirectedChannelsPerCall) * 2 + \
										(MaxAudioCalls - (MaxVideoCalls * 2)) * (MaxAudioDirectedChannelsPerCall * 2))


//#define	ChannelNameSize					64
#define DataSourceNameSize				64

#define SIP_PROXY_ENTRY					1005 // For income SIP Proxy msgs in rsrc table

#define AliasesDelimiter				","
#define AliasesDelimiterChar			','
#define AccordVendorIdPrefix			"ACCORD MGC"
#define AccordVendorId					"Polycom/Polycom RMX 2000/V7.0"
#define AccordVersionId					"H323.4: V7.0"
// AccordHostName and AccordDisplay are use to recognize remote MCU's versions 2.
#define AccordHostName					"Eval_323"
#define AccordDisplay					"ACCORD MGC-100"

#define RADVisionVendorId				"RADVision MCU-323"
#define	ConfigKeySize					16

#define	MaxChanRestoreTimeout			4	// In Hsyncs  //Anat
#define MaxRestoredChannlesInHsync		24	// Depends on controlQueueSCtoRTPElements  //Anat

// Video Types
#define VideoParsing					0x01
#define VideoNoParsing					0x02
#define SoftwareCp						0x04
#define VideoSwitch						0x08

#define MaxIpAddressStringSize			64
#define MaxMcmsIpAddressStringSize		16
#define MaxIPAddressesListSize			16

#define MaxRasCalls						128
//#define MaxNumberOfAliases				6
//#define MaxAliasLength					100
//#define MaxAddressListSize				256
//#define MaxUserUserSize					128
//#define MaxDisplaySize					128
//#define MaxConferenceIdSize				16
//#define Size16							16
#define MaxIpAddressStringLength		18


// Party address list:
// -------------------
typedef char
	partyAddressList[MaxAddressListSize];


typedef struct {
	APIS8				sDisplay[MaxDisplaySize];
	APIS8				userUser[MaxUserUserSize];
	APIS32				sDisplaySize;
	APIS32				userUserSize;

} mcCallTransient;

typedef enum
{
	NetworkNone,
	NetworkH320,
	NetworkH323,
} endPointNetwork_e;

// Ip h323/Sip dtmf api definitions
//----------------------------------


typedef enum {
	dtmfH245		= 0,
	dtmfSipInfo	,
	dtmfRfc2833Rtp,
	dtmfMaxNumber
}dtmfSources_e;


// Call Generator SoftMCU CS parameters
typedef enum
{
	endpointModelHDX9000,
	endpointModelVSX7000,
	// TBD: add the rest of endpoint models
} endpointModel_e;

typedef struct
{
	APIU8  bIsCallGenerator;
	APIU8  eEndpointModel;
	APIU16 reserved;
} mcCallGeneratorParams;


// Task Indications:
// -----------------
//#define	TaskIndMask						0x00FF0000
//
//#define	FirstStackIfInd					0x00010000
//#define	FirstStartupTaskInd				0x00020000
//#define	FirstMcmsTaskInd				0x00030000
//#define	FirstGkManagerTaskInd			0x00040000
//#define	FirstCardManagerTaskInd			0x00050000
//#define	FirstGatekeeperIfTaskInd		0x00080000
//#define	FirstCallTaskInd				0x00090000
//#define	FirstCallTaskDispatchInd		0x00070000
//#define	FirstTimerTaskInd				0x000B0000
//#define	FirstTimerSelectTaskInd			0x000C0000
//#define	FirstHamorTpktRecvTaskInd		0x000D0000
//#define	FirstHamorTpktSendTaskInd		0x000E0000
//#define	FirstMcmsTpktTaskInd			0x000F0000
//#define	FirstCardManagerTpktTaskInd		0x00100000
//#define	FirstGuiTpktTaskInd				0x00110000
//#define	FirstRtpProcessTaskInd			0x00120000
//#define	FirstRtpTdmTaskInd				0x00130000
//#define	FirstSipTaskDispatchInd			0x00140000
//#define	FirstSipTaskInd					0x00150000

#endif //__IPCOMMONDEF_H__
