//+========================================================================+
//                            SIPUTILS.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SIPUTILS.H                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara Avidan                                                |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |  7/4/03    | Utilities fot SIP parties		                       |
//+========================================================================+

#ifndef _SIP_UTILITIES
#define _SIP_UTILITIES

#include "PObject.h"
#include "ObjString.h"
//#include "IpCommonTypes.h"
#include "SipCsInd.h"
#include "SipCsReq.h"
#include "SipDefinitions.h"
#include "BFCPMessage.h"

#include "RvCommonDefs.h"
#include "Capabilities.h"
#include "Segment.h"
#include "NStream.h"

#include "SipStructures.h"
#include "IpCsContentRoleToken.h"

#define ILLEGAL_LAYER_ID -1

#if 0
int GetLayerId(const VIDEO_OPERATION_POINT_SET_S &rOperationPoints, unsigned int width, unsigned int hieght, unsigned int frameRate);
#endif
mcXmlTransportAddress &ExtractMLineMediaIp(eMediaLineInternalType intType, const sipMediaLinesEntrySt *pMediaLinesEntry, mcXmlTransportAddress &dummyMediaIp, BYTE confIsEncrypt = FALSE, int index = -1);
mcXmlTransportAddress &ExtractMLineMediaIp(eMediaLineInternalType intType, const sipSdpAndHeadersSt *pSdp, mcXmlTransportAddress &dummyMediaIp, BYTE confIsEncrypt = FALSE, int index = -1);
unsigned int &ExtractMLineRtcpPort(eMediaLineInternalType intType, const sipSdpAndHeadersSt *pSdp, unsigned int &dummyRtcpPort, BYTE confIsEncrypt = FALSE, int index = -1);
sipMediaLineSt* GetMediaLine(sipSdpAndHeadersSt &sdp, eMediaLineInternalType intType, int index = -1);
//added for ANAT begin
//sipMediaLineSt* GetOtherMediaLineInMidGroup(sipSdpAndHeadersSt &sdp, int mid); 
BOOL IsANATPresentInSDP(const sipSdpAndHeadersSt* pSdp);
BOOL IsAVMCUCall(sipSdpAndHeadersSt* pSdp);

int GetIndexAccordingToInternalTypeAndIpVersion(eMediaLineInternalType intType, sipSdpAndHeadersSt *pSdp, enIpVersion selectedIpVersion);
sipMediaLineSt* GetMLineAccordingToInternalTypeAndIpVersion(eMediaLineInternalType intType, sipSdpAndHeadersSt *pSdp, enIpVersion selectedIpVersion);
sipMediaLineSt* GetMediaLineAtIndex(sipSdpAndHeadersSt &sdp, int index);
enIpVersion GetMediaLineIpVersionlType(const sipSdpAndHeadersSt *pSdp, int index); 
//added for ANAT end
eMediaLineInternalType GetMediaLineInternalType(const sipSdpAndHeadersSt *pSdp, int index); 
eBfcpSetup ConvertBFCPSetupTypeToApi(eBFCPSetup setup);
eBfcpConnection ConvertBFCPConnectionTypeToApi(eBFCPConnection connection);
eBFCPSetup ConvertApiToBFCPSetupType(eBfcpSetup setup);
eBFCPConnection ConvertApiToBFCPConnectionType(eBfcpConnection connection);
eBfcpFloorCtrl GetAnswerFloorCtrl(eBfcpFloorCtrl offererFloorCtrl);
mcXmlTransportAddress &ExtractMLineMediaIpAccordingToMediaType(eMediaLineType intType, const sipMediaLinesEntrySt *pMediaLinesEntry, mcXmlTransportAddress &dummyMediaIp, BYTE confIsEncrypt = FALSE, int index = -1);
mcXmlTransportAddress &ExtractMLineMediaIpAccordingToMediaType(eMediaLineType intType, const sipSdpAndHeadersSt *pSdp, mcXmlTransportAddress &dummyMediaIp, BYTE confIsEncrypt = FALSE, int index = -1);
/*
#include <mlancfg.h>

#ifndef _IPNETSETUP
#include <IpNets.h>
#endif

#ifndef _NSTREAM
#include <nstream.h>
#endif


#ifndef _SIPCAPS
#include "SipCaps.h"
#endif*/

#define SIP_MEDIA_LINE_BUFFER_SIZE		20000
#define SIP_INIT_INVITE_RESPONSE_BUFFER_SIZE	80000
BOOL AddCapInMediaLine(capBuffer* pCapBuffer, int capBufferSize, sipMediaLineSt *pMediaLine, int mediaLineBufSize, int &capPos);
BOOL AddCapInBuffer(capBuffer* pCapBuffer, int capBufferSize, char *pCapBuf, int capBufSize, int &capPos);

#define OK_VAL			200
#define LOW_REJECT_VAL  300
#define HIGH_REJECT_VAL	700

#define MAX_WATCHERS_IN_CONF	32

//#define STATUS_TOUT		(-1)

#define MAX_SIP_MEDIA_TYPES (5) //Audio+Video+Data(Fecc)+content+bfcp
#define MAX_SIP_CHANNELS ((MAX_SIP_MEDIA_TYPES)*2 /*+ 2*/) //for bfcp channels
#define CHANNEL_TYPES_COUNT (5) //audio + video + fecc + content + bfcp

#define SIP_SERVICE_METHOD	0
#define SIP_MESSAGE_METHOD	1

#define BAD_CHARACTERS_FOR_SIP_URI " \"<>@:\\"

#define MAX_MRC_HEADER_SIZE					30
#define MAX_SDP_INFORMATION_HEADER_SIZE		32

#define INFO_HEADER_STR_MASTER		"MASTER_MCU_CASCADE_LINK"
#define INFO_HEADER_STR_SLAVE		"SLAVE_MCU_CASCADE_LINK"



// timers
#define ResolvingTimeOut	60

#define MaxMediaLinesPlcmReqTag	10
/* Mask to use with DMA for proprietry header - X-Plcm-Require */
#define m_plcmRequireNone			0x0000
#define m_plcmRequireAvp			0x0001
#define m_plcmRequireSavp			0x0002
#define m_plcmRequireAudio  		0x0004
#define m_plcmRequireVideoMain 		0x0008
#define m_plcmRequireVideoSlides 	0x0010
#define m_plcmRequireBfcpUdp   		0x0020
#define m_plcmRequireBfcpTcp   		0x0040
#define m_plcmRequireFecc   		0x0080

#define X_PLCM_REQUIRE_STR 				"X-Plcm-Require"
#define X_PLCM_SDP_SRTP_STR 			"sdp-srtp"
#define X_PLCM_SDP_RTP_STR 				"sdp-rtp"
#define X_PLCM_SDP_AUDIO_STR 			"sdp-audio"
#define X_PLCM_SDP_VIDEO_MAIN_STR 		"sdp-video-main"
#define X_PLCM_SDP_VIDEO_SLIDES_STR 	"sdp-video-slides"
#define X_PLCM_SDP_BFCP_UDP_STR 		"sdp-bfcp-udp"
#define X_PLCM_SDP_BFCP_TCP_STR 		"sdp-bfcp-tcp"
#define X_PLCM_SDP_FECC_STR 			"sdp-fecc"

#define PLCM_CALL_ID_HEADER "Plcm-Call-ID"
#define PLCM_ORGINAL_TO_HEADER "Plcm-Original-To"

#define MSFT_SVC_MAX_VIDEO_SSRC_IN_SDP_LINE 100 

typedef enum
{
	kSipMediaChannelAudio,
	kSipMediaChannelVideo,
	kSipMediaChannelFecc,
	kSipMediaChannelContent,
	kSipMediaChannelAudioContent,
	kSipMediaChannelVideoContent,
	kSipMediaChannelBfcp,
	kNumOfSipMediaChannelTypes

} ESipMediaChannelType;

//eFeatureRssDialin
enum  enSrsSessionType
{	
	eSrsSessionTypeRegular		=0,
	eSrsSessionTypeRecording		=1,
	eSrsSessionTypePlayback		=2
};


static ESipMediaChannelType globalMediaArr[] = {kSipMediaChannelAudio,kSipMediaChannelVideo,kSipMediaChannelFecc,kSipMediaChannelContent, kSipMediaChannelBfcp};
static cmCapDirection globalDirectionArr[]	= {cmCapReceive,cmCapTransmit};

extern const char* g_headerFieldStrings[];
extern const char* g_bodyTypeStrings[];
inline const char* GetHeaderFieldStr(enHeaderField hf) {return ((hf<enLastField) && (hf>kFirstField))? g_headerFieldStrings[hf]: "";}
// ***** inline const char* GetBodyTypeStr(sipBodyType bt) {return (bt<kOther)? g_bodyTypeStrings[bt]: "";}
DWORD GetNumberOfMediaLinesOfIntenalType(sipSdpAndHeadersSt *pSdpAndHeaders, eMediaLineInternalType intType, int &sizeOfMediaLines);
BOOL IsPanoramicVideoMediaLine(const sipMediaLineSt *pMediaLine);
DWORD GetNumberOfMediaLinesOfPanoramicType(sipSdpAndHeadersSt *pSdpAndHeaders, int &sizeOfPanoramicMediaLines);
void GetMediaDataTypeAndRole(ESipMediaChannelType sipMediaType, cmCapDataType &retMediaType, ERoleLabel &retRole);
void GetMediaDataTypeAndRole(eMediaLineInternalType sipMediaLineIntType, cmCapDataType &retMediaType, ERoleLabel &retRole);
eMediaLineInternalType GetMediaLineInternalType(ESipMediaChannelType sipMediaType);
eMediaLineInternalType GetMediaLineInternalType(cmCapDataType eDataType, ERoleLabel eRole);
ESipMediaChannelType GetSipMediaChannelType(cmCapDataType aMediaType, ERoleLabel eRole);
bool IsMrcHeader(const sipSdpAndHeadersSt *pSdpAndHeaders, const char *pSearchStrInSessionHeader = "MRD=MRE");
bool IsAvMcuNonCCCPCascade(const sipSdpAndHeadersSt *pSdpAndHeaders,BYTE isMsConfInvite);
BYTE GetCascadeModeFromHeader(const sipSdpAndHeadersSt *pSdpAndHeaders);
void GetPlcmRequireHeaderMask(const sipSdpAndHeadersSt *pSdpAndHeaders, APIU16 *mask, eMediaLineInternalType m_plcmReqMaskMlineOrder[]);
void TurnPlcmReqMaskSavp(APIU16& plcmRequireMask , BOOL bYesNo);
BOOL CheckXciscoInSupportedHeader(const sipSdpAndHeadersSt *pSdpAndHeaders);
BOOL GetIsCallWithDma(const sipSdpAndHeadersSt *pSdpAndHeaders);
BYTE IsWebRtcCallSupported();
BYTE IsWebRtcCall(const sipSdpAndHeaders* pSdpAndHeaders);
BYTE SipGetHeaderValue(sipMessageHeaders *pHeaders, enHeaderField eHeader, char *cValue, size_t valueSize);
BYTE IsGoodConfName(const char* cName);
BYTE IsMediaContainedInSdp(sipSdpAndHeadersSt *pSdpAndHeaders, eMediaLineInternalType intType);
BOOL IsSdpPresent(sipSdpAndHeadersSt *pSdpAndHeaders);
BOOL IsSdesDeclaredInSdp(sipSdpAndHeadersSt *pSdpAndHeaders);
BYTE IsMediaContainedInDirtySdp(sipMediaLinesEntrySt* pMediaLinesEntry, eMediaLineInternalType intType);
eMediaLineSubType ConvertBfcpTransportToMediaLineSubType(enTransportType transType);
enTransportType ConvertBfcpMediaLineSubTypeToTransportType(eMediaLineSubType subType);
int GetLayerId(const VIDEO_OPERATION_POINT_SET_S &rOperationPoints, unsigned int width, unsigned int hieght, unsigned int frameRate,unsigned int bitRate,cmCapDirection direction,DWORD partyId=0,CLargeString* pMsgStr=NULL);
unsigned int GetFrameRateInSignalingValues(unsigned int scpFrameRate);
BYTE CheckIsFactoryAtDomain(const char* pStr);
const char* GetCurrentDomainAt(const char* pStr);
const char* GetCurrentDomain(const char* pStr);
WORD ExtractNumOfScreensFromContactHeader(const char* strContact);
enSrsSessionType   getSrsSessionType(const sipSdpAndHeadersSt  *pSdpAndHeaders);
BOOL  IsMediaAudioOnly(sipSdpAndHeadersSt *pSdpAndHeaders);

//void SipSetNortelMode(int cSwitch);
//Amihay: the same field is used in rtcpFeedbackMask
//BYTE SetFirFlag(sipSdpAndHeadersSt &sdp, eMediaLineInternalType intType);
//

#define MIN_ALLOC_HEADERS 8

typedef struct
{
	enSipCodes eReason;
	const char* strReason;
} TRejectReason;

typedef struct
{
	enSipWarningCodes	eWarning;
	const char*			strWarning;
} TWarningCode;

typedef enum
{
	kNoCancel = 0,
		kCancelBeforeInvite,
		kCancelAfterInvite
} ECancelType;

static CapEnum g_SipDialOutRemovedAudioCodecs[] =
{
		eG719Stereo_128kCapCode,
		eG719Stereo_96kCapCode,
		eG719Stereo_64kCapCode,
		eG719_64kCapCode,
		eG719_48kCapCode,
		eG719_32kCapCode,
	
		//BRIDGE-12398
		eSiren14Stereo_48kCapCode,
	    eSiren14Stereo_56kCapCode,
	    eSiren14Stereo_64kCapCode,
	    eSiren14Stereo_96kCapCode,
	    eSiren22Stereo_64kCapCode,
	    eSiren22Stereo_96kCapCode,
	    eSiren22Stereo_128kCapCode,
	    eSirenLPRStereo_64kCapCode,
	    eSirenLPRStereo_96kCapCode,
	    eSirenLPRStereo_128kCapCode,

	    eUnknownAlgorithemCapCode  // Must be last
};

static CapEnum g_TipRemoveCodecs[] =
{
		eSiren22Stereo_128kCapCode,
		eSiren22Stereo_96kCapCode,
		eSiren14Stereo_96kCapCode,
		eSiren14Stereo_64kCapCode,
		eH261CapCode,
		eH263CapCode,
	    eUnknownAlgorithemCapCode  // Must be last
};



static const int g_SipMaxPartialAudioCodecs = 16; //should be more than the number of g_SipStaticPartialAudioCodecs

// Note that Siren7 cannot be statically added to this array, since it is added by cfg, so please check it standalone
static CapEnum g_SipStaticPartialAudioCodecs[] =
{
	eG7221_32kCapCode,
	eG7221_24kCapCode,
	eG7221_16kCapCode,
	eG722_64kCapCode,

	eG711Ulaw64kCapCode,
	eG711Alaw64kCapCode,
	eG729AnnexACapCode,
	eRfc2833DtmfCapCode,

	eG728CapCode,
	eAAC_LDCapCode, //TIP

    eUnknownAlgorithemCapCode  // Must be last
};

// Note that Siren7 cannot be statically added to this array, since it is added by cfg, so please check it standalone
//N.A. VNGFE-7440 && VNGFE-7854
static CapEnum g_SipAudioOnlyPartialAudioCodecs[] =
{
	eG711Alaw64kCapCode,
	eG711Ulaw64kCapCode,
	eG729CapCode,
	eG7221C_48kCapCode,
	eG7221_32kCapCode,
	eRfc2833DtmfCapCode,

    eUnknownAlgorithemCapCode  // Must be last
};


//class CLargeString;
//class CObjString;

extern const TRejectReason g_rejectReasons[];
const char * GetRejectReasonStr(enSipCodes eReason);
extern const TWarningCode g_warningCodes[];
const char * GetWarningCodeStr(enSipWarningCodes eWarning);
//int  GetCardSdpArrayIndex(cmCapDataType eMediaType);

enSipCodes TranslateLobbyRejectReasonToSip(int reason);



///////////////////////////////////////////////////
enum eScpNotificationReasons
{
// SCP Spec define '0' as 'Stream Is Now Provided'
//	eScpNotificationReasonsUndefined = 0,

	eStreamIsNowProvided = 0,
	eStreamNotAvailable,
	eStreamIsMuted,
	eStreamNotProvidedBecauseOfBandwidth,
	eStreamNotProvidedBecauseOfErrorResiliency,
	eStreamNotProvidedBecauseEpDisconnected,
	eStreamNotProvidedBecauseOfIvrSlideState, // according to SCP spec: reason = 6 	The stream is not provided because of invalid call state

	eScpNotificationReasonsMaxNumOfValues
};

const char* GetScpNotificationReasonsEnumValueAsString(eScpNotificationReasons val);

///////////////////////////////////////////////////
enum eScpNotificationType
{
	eScpNotificationTypeUndefined = 0,

	eStreamCannotBeProvided,
	eStreamCanNowBeProvided,

	eScpNotificationTypeMaxNumOfValues
};

const char* GetScpNotificationTypeEnumValueAsString(eScpNotificationType val);

///////////////////////////////////////////////////



class CSipHeaderList;

class CSipHeader: public CPObject
{
CLASS_TYPE_1(CSipHeader,CPObject)
public:
	CSipHeader();
	CSipHeader(enHeaderField eField,BYTE flags,int strLen,const char* headerStr);
	CSipHeader(const CSipHeader & other);
	virtual ~CSipHeader();
	virtual const char* NameOf() const { return "CSipHeader";}

	friend class CSipHeaderList;
	virtual void Serialize(WORD format,CSegment& seg) const;
	virtual void DeSerialize(WORD format,CSegment& seg);
	CSipHeader& operator=(const CSipHeader& other);
	enHeaderField GetHeaderField()	const {return m_eField;}
	int			  GetHeaderStrLen() const {return m_strLen;}
	const char *  GetHeaderStr()	const {return m_strHeader;}
	const char *  GetPrivateHeaderStr(const char* strHeaderType) const;
	void		  GetHeaderStrLower(char *str, int len) const;

private:
	enHeaderField m_eField;
	BYTE		  m_flags;
	int			  m_strLen;
	char		* m_strHeader;
};

class CSipHeaderList: public CPObject
{
CLASS_TYPE_1(CSipHeaderList,CPObject)
public:
	CSipHeaderList(const CSipHeaderList & other);
	CSipHeaderList(const sipMessageHeaders & headers);
	CSipHeaderList(int maxAlloc,int numOfElem,.../*params format: numOfElem X (int eField, int strLen,char * str)*/);
	virtual ~CSipHeaderList();
	virtual const char* NameOf() const { return "CSipHeaderList";}
	virtual void Dump(COstrStream& msg) const;
	virtual void DumpToStream(CObjString* pMsgStr) const;

	virtual void Serialize(WORD format,CSegment& seg) const;
	virtual void DeSerialize(WORD format,CSegment& seg);

	CSipHeaderList& operator=(const CSipHeaderList & other);
	void	AddHeader(const CSipHeader & header);
	void	AddHeaders(const CSipHeaderList& headers);
	void	AddHeader(enHeaderField eField,int strLen,const char* str);
	int		GetTotalLen() const; //for allocate new sipMessageHeaders
	int		GetNumOfElem() const {return m_numOfElem;}
	int 	BuildMessage(sipMessageHeaders* pMsg) const;
	void	ZeroingFlags();
	int		GetNextHeaderIndex(enHeaderField eField);
	const CSipHeader* GetNextHeader(enHeaderField eField);
	const CSipHeader* operator[] (int i) const;
	const CSipHeader* GetNextPrivateOrProprietyHeader(enHeaderField eField,int strLen,const char* strHeaderName);
	bool RemoveNextHeader(enHeaderField eField); //_mccf_

private:

	void		  ReallocIfNeeded();
	void		  CleanArr();
	int			  m_maxAlloc;
	int			  m_numOfElem;
	CSipHeader ** m_pHeadersArr;
};

class CSipBaseStruct: public CPObject
{
CLASS_TYPE_1(CSipBaseStruct,CPObject )
public:
	// Constructors
	CSipBaseStruct();
	virtual ~CSipBaseStruct();
	virtual void Serialize();
	virtual void DeSerialize();
	virtual void Dump(COstrStream& msg) const;
	virtual const char* NameOf() const { return "CSipBaseStruct";}

	void		SetHeaderField(enHeaderField eField,const char* headerStr);
	const char*	GetHeaderField(enHeaderField eField,int index=0) const;
	BOOL		FindHeaderField(enHeaderField eField, const char* headerStr, BOOL fStart = TRUE);
	int GetNumOfHeaders() const {return (m_pHeaders?  m_pHeaders->GetNumOfElem(): 0);}

protected:
	CSipHeaderList	* m_pHeaders;
	BYTE			* m_buffer;
	int				  m_bufferLen;
};

// This class is currently used only by OnSipCallIn in Lobby 
class CSipInviteStruct: public CSipBaseStruct
{
CLASS_TYPE_1(CSipInviteStruct,CSipBaseStruct)
public:
	CSipInviteStruct();
	virtual ~CSipInviteStruct();
	virtual void Dump(COstrStream& msg) const;
	virtual const char* NameOf() const { return "CSipInviteStruct";}

	void	ReadInviteInd(mcIndInvite *pMsg);
	void	SetCallIndex(DWORD callIndex) {m_callIndex = callIndex;}
	DWORD	GetCallIndex() const {return m_callIndex;}
	void	SetCsServiceId(DWORD csServiceId) {m_csServiceId = csServiceId;}
	DWORD	GetCsServiceId() const {return m_csServiceId;}
	sipSdpAndHeadersSt *GetSdpAndHeaders() const {return m_pSdpAndHeaders;}

private:
	DWORD				m_callIndex;
	DWORD				m_csServiceId;
	sipSdpAndHeadersSt	*m_pSdpAndHeaders;
};


// This class is currently used only by OnSipCallIn in Lobby
class CSipBeNotifyStruct : public CSipBaseStruct
{
	CLASS_TYPE_1(CSipBeNotifyStruct, CSipBaseStruct)

public:
	                    CSipBeNotifyStruct() {}
	virtual            ~CSipBeNotifyStruct() {}
	virtual void        Dump(COstrStream& msg) const;
	virtual const char* NameOf() const       { return "CSipBeNotifyStruct";}

	void                ReadHeaders(BYTE* buffer, int bufferLen);
};
class CSipRegisterStruct: public CSipBaseStruct
{
CLASS_TYPE_1(CSipRegisterStruct,CSipBaseStruct)
public:
	CSipRegisterStruct();
	virtual ~CSipRegisterStruct();
	virtual void Dump(COstrStream& msg) const;
	virtual const char* NameOf() const { return "CSipRegisterStruct";}

	mcReqRegister * BuildRegister();
	void	ReadRegister(mcIndRegisterResp *pMsg);

	int		AddContact(char* contact, char* contactDisplay);
//	int 	AddContact(char** contactList, char* contactDisplay);
	DWORD	GetExpireHeader() const {return m_expire;}
	void    SetExpireHeader(DWORD time) {m_expire = time;}
	DWORD	GetId() const {return m_id;}
	void	SetId(DWORD id) {m_id = id;}
/*
	DWORD	GetProxyIp() const {return m_proxyIp;}
	void	SetProxyIp(DWORD proxyIp) {m_proxyIp = proxyIp;}
	void	SetProxyPort(WORD port) {m_proxyPort = port;}
	DWORD	GetRegistrarIp() const {return m_registrarIp;}
	void	SetRegistrarIp(DWORD registrarIp) {m_registrarIp = registrarIp;}
	void	SetRegistrarPort(WORD port) {m_registrarPort = port; }
*/
	DWORD	GetProxyIpV4() const {return m_proxyAddress.addr.v4.ip;}
	char*	GetProxyIpV6() const {return (char*)m_proxyAddress.addr.v6.ip;}

	void	SetProxyIpV4(DWORD proxyIpV4) {m_proxyAddress.addr.v4.ip = proxyIpV4;}
	void	SetProxyIpV6(char* proxyIpV6) {memcpy(m_proxyAddress.addr.v6.ip, proxyIpV6, IPV6_ADDRESS_BYTES_LEN);}
	void	SetProxyV6scopeId(DWORD scopeId) {m_proxyAddress.addr.v6.scopeId = scopeId;}
	void	SetProxyPort(WORD port) {m_proxyAddress.port = port;}
	void	SetProxyIpVersion(DWORD ipVersion) {m_proxyAddress.ipVersion = ipVersion;}
	WORD    GetProxyPort() {return m_proxyAddress.port;}

	DWORD	GetRegistrarIpV4() const {return m_registrarAddress.addr.v4.ip;}
	char*	GetRegistrarIpV6() const {return (char*)m_registrarAddress.addr.v6.ip;}
	void	SetRegistrarIpV4(DWORD registrarIpV4) {m_registrarAddress.addr.v4.ip = registrarIpV4;}
	void	SetRegistrarIpV6(char* registrarIpV6) {memcpy(m_registrarAddress.addr.v6.ip, registrarIpV6, IPV6_ADDRESS_BYTES_LEN);}
	void	SetRegistrarV6scopeId(DWORD scopeId) {m_registrarAddress.addr.v6.scopeId = scopeId;}
	void	SetRegistrarPort(WORD port) {m_registrarAddress.port = port; }
	void	SetRegistrarIpVersion(DWORD ipVersion) {m_registrarAddress.ipVersion = ipVersion;}


	WORD	GetTransportType() const {return m_transportType;}
	void	SetTransportType(WORD transportType) {m_transportType = transportType;}

	mcTransportAddress* GetRegistrar() {return &m_registrarAddress;};


	enGeneralRegisterOpcode	GetRegisterSubOpcode() const {return m_subOpcode;}
	void    SetRegisterSubOpcode(enGeneralRegisterOpcode subOpcode) {m_subOpcode = subOpcode;}

private:

	DWORD	m_expire;
	DWORD	m_id;
	mcTransportAddress m_proxyAddress;
//	DWORD	m_proxyIp;	//outbound proxy
//	WORD	m_proxyPort;
	mcTransportAddress m_registrarAddress;
//	DWORD	m_registrarIp;	//registrar
//	WORD	m_registrarPort;
	WORD	m_transportType;
	enGeneralRegisterOpcode m_subOpcode;
};

class CSipSubscribeStruct: public CSipBaseStruct
{
CLASS_TYPE_1(CSipSubscribeStruct,CSipBaseStruct)
public:
	CSipSubscribeStruct();
	virtual ~CSipSubscribeStruct();
	virtual void Dump(COstrStream& msg) const;
	virtual const char* NameOf() const { return "CSipSubscribeStruct";}

	mcReqSubscribe * BuildSubscribeReq();

    //void	SetProxyIp(DWORD proxyIp);
	void	SetProxyIpVersion(enIpVersion proxyIpVersion);
	void	SetProxyIpV4(DWORD proxyIpV4);
	void	SetProxyIpV6(char* proxyIpV6);
//	void	SetProxyV6scopeId(DWORD scopeId);
	void	SetProxyPort(WORD port);
	void	SetTransportType(WORD transportType);
	void	SetEvent(char* pEvent);
	char*	GetEvent() const;
	void	SetAccept(char* pAccept);
	void	SetSupported(char* pSupported);
	void	SetExpires(DWORD expires);
	DWORD	GetExpires();
	void	SetId(DWORD id);
	DWORD	GetId() const;
	void	SetSubOpcode(WORD subOpcode);
	WORD	GetSubOpcode() const;

private:

	//DWORD	m_proxyIp;
	union ipAddressIf m_proxyIp;
	enIpVersion m_proxyIpVersion;


	WORD	m_proxyPort;
//	mcTransportAddress m_proxyAddress;
	WORD	m_transportType;
	char*	m_pEvent;
	char*	m_pAccept;
	char*	m_pSupported;
	DWORD	m_expires;
	DWORD	m_Id;
	WORD	m_subOpcode;
};

class CSipUnknownMsgStruct: public CSipBaseStruct
{
CLASS_TYPE_1(CSipUnknownMsgStruct,CSipBaseStruct)
public:
	CSipUnknownMsgStruct();
	virtual ~CSipUnknownMsgStruct();
	virtual void Dump(COstrStream& msg) const;
	virtual const char* NameOf() const { return "CSipUnknownMsgStruct";}

	mcReqUnknownMethod* BuildUnknownMsgReq();

	void	SetMethod(BYTE bType);
	void	SetTransportType(WORD transportType);
	void	SetContentType(char* pType);
	void	SetContent(char* pContent);
	void	SetRegistrarIpV4(DWORD registrarIpV4);
	void	SetRegistrarIpV6(char* registrarIpV6);
	void	SetRegistrarV6scopeId(DWORD scopeId);
    void	SetRegistrarPort(WORD port);
    void 	SetRegistrarIpVersion(DWORD ipVersion);

private:
//	DWORD	m_registrarIp;
//	WORD	m_registrarPort;
	mcTransportAddress m_registrarAddress;
	BYTE	m_method; //MESSAGE/ SERVICE
	WORD	m_transportType;
	char*	m_pContentType;
	char*	m_pContent;
};

class CSipNotifyStruct: public CSipBaseStruct
{
CLASS_TYPE_1(CSipNotifyStruct,CSipBaseStruct)
public:
	CSipNotifyStruct();
	virtual ~CSipNotifyStruct();
	virtual void Dump(COstrStream& msg) const;
	virtual const char* NameOf() const { return "CSipNotifyStruct";}

	mcReqNotify * BuildNotifyReq();
	mcReqNotify * BuildNotifyReq(mcXmlTransportAddress& transportAddress);

	void	SetIp(DWORD ip);
	void	SetPort(WORD port);
	void	SetTransportType(WORD transportType);
	void	SetContentType(char* pContentType);
	void	SetContent(const char* pContent);
	void	SetCSeq(DWORD cseq);
	void	SetCallIndex(DWORD callIndex);


private:

	DWORD	m_ip;
	WORD	m_port;
	WORD	m_transportType;
	char*	m_pContentType;
	char*   m_pContent;
	DWORD	m_CSeq, m_CallIndex;
};

class CSipDistrNotifyStruct: public CSipBaseStruct
{
CLASS_TYPE_1(CSipDistrNotifyStruct,CSipBaseStruct)
public:
	CSipDistrNotifyStruct();
	virtual ~CSipDistrNotifyStruct();
	virtual void Dump(COstrStream& msg) const;
	virtual const char* NameOf() const { return "CSipDistrNotifyStruct";}
// ******	mcReqSipCxNotify * BuildDistrNotifyReq();

	void	SetContentUnzippedOrgSize(DWORD length);
	void	SetContentZipCheckSum(DWORD length);

	void	AddWatcherId(WORD watcherId);
	void	SetContentType(char* pContentType);
	void	SetContent(char* pContent,int length);

private:

	DWORD	m_unzippedOrgSize;
	DWORD   m_zipCheckSum;
	WORD	m_WatchersCount;
	WORD	m_WatchersArray[MAX_WATCHERS_IN_CONF];
	char*	m_pContentType;
	char*   m_pContent;
};

class CSipServiceStruct: public CSipBaseStruct
{
CLASS_TYPE_1(CSipServiceStruct,CSipBaseStruct)
public:
CSipServiceStruct();
	virtual ~CSipServiceStruct();
	virtual const char* NameOf() const { return "CSipServiceStruct";}
	virtual void Dump(COstrStream& msg) const;

	mcReqService * BuildServiceReq();

	void    SetProxyIpV4(DWORD proxyIpV4);
	void 	SetProxyIpV6(char* pProxyIpV6);
	void    SetProxyIpVersion(enIpVersion proxyIpVersion);
	void	SetProxyPort(WORD port);
	void	SetTransportType(WORD transportType);
	void	SetId(DWORD id);
	DWORD	GetId() const;
	void	SetSubOpcode(WORD subOpcode);
	WORD	GetSubOpcode() const;

private:

	union ipAddressIf m_proxyIp;
	enIpVersion m_proxyIpVersion;


	WORD	m_proxyPort;
//	mcTransportAddress m_proxyAddress;
	WORD	m_transportType;

	DWORD	m_Id;
	WORD	m_subOpcode;

};



#endif

