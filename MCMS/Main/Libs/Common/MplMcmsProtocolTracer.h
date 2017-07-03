#ifndef MPL_MCMS_PROTOCOL_TRACER_H_
#define MPL_MCMS_PROTOCOL_TRACER_H_

#include "PObject.h"
#include "MplMcmsProtocol.h"
#include "IvrApiStructures.h"
#include "TBStructs.h"
#include "VideoStructs.h"
#include "IpRtpReq.h"
#include "H323CsReq.h"
#include "H323CsInd.h"
#include "IpCsContentRoleToken.h"
#include "IpRtpFeccRoleToken.h"
#include "Q931Structs.h"
#include "TipStructs.h"
#include "BfcpStructs.h"
#include "McmsProcesses.h"
#include  "IceCmReq.h"
#include "OpcodesMcmsCardMngrICE.h"

///////////////////////////////////////////////////////////////////////////
class CMplMcmsProtocol;
class CObjString;
class CManDefinedString;

////////////////////////////////////////////////////////////////////////////
class CMplMcmsProtocolTracer : public CPObject
{
	CLASS_TYPE_1(CMplMcmsProtocolTracer, CPObject)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

	virtual ~CMplMcmsProtocolTracer();

private: // forbid copying

	void operator =(const CMplMcmsProtocolTracer&);

public:

	CMplMcmsProtocolTracer();
	CMplMcmsProtocolTracer(CMplMcmsProtocol& mplMcmsProt);

	virtual void        TraceMplMcmsProtocol(const char* message = " ", BOOL Type = MPL_API_TYPE, BOOL traceContent = true);
	virtual void        TraceContent(CObjString* pContentStr1, eProcessType processType = eProcessTypeInvalid);

	void                SetData(CMplMcmsProtocol* data)
	{ m_pMplMcmsProt = data; }

protected:

	void                TraceCommonHeader(CObjString& ostrr) const;
	void                TraceMessageHeader(CObjString& ostr) const;
	void                TraceCSHeader(CObjString& ostr) const;
	void                TracePhysicalInfoHeader(CObjString& ostr) const;
	void                TracePortDescriptionHeader(CObjString& ostr) const;

	void                TraceTPKT_Header(const TPKT_HEADER_S& pTPKT_Header);
	void                TraceStartRecordingRequest(CObjString* pContentStr);

	void                TraceMplAckInd(CObjString* pContentStr);
	void                TraceDecoderSyncInd(CObjString* pContentStr);
	void                TraceTbMsgConnectDisconnectReq(CObjString* pContentStr);
	void                TraceIvrStartReq(CObjString* pContentStr);
	void                TraceStartIVRParams(CObjString* pContentStr, const SIVRStartIVRParams& startIVRParams);
	void                TraceIvrStopReq(CObjString* pContentStr);
	void                TraceIvrPlayMsgReq(CObjString* pContentStr);
	void                TraceMediaFileParams(CObjString* pContentStr, const SIVRMediaFileParamsStruct& mediaFileParams);
	void                TraceIvrPlayMusicReq(CObjString* pContentStr);
	void                TraceIvrRecordRollCallReq(CObjString* pContentStr);
	void                TraceIvrRecordRollCallInd(CObjString* pContentStr);
	void                TraceIvrStopRecordRollCallReq(CObjString* pContentStr);
	void                TraceIvrShowSlideReq(CObjString* pContentStr);
	void                TraceIvrStopShowSlideReq(CObjString* pContentStr);
	void                TraceIvrFastUpdateReq(CObjString* pContentStr);
	void                TraceTbMsgOpenPortReq(CObjString* pContentStr);
	void                TraceConfMplCreateParty(CObjString* pContentStr);
	void 				TraceConfMplDeleteParty(CObjString* pContentStr);
	void 				TraceTbMsgClosePortReq(CObjString* pContentStr);
	void                TraceAudioPlayToneReq(CObjString* pContentStr);
	void                TraceAudioDtmfIndVal(CObjString* pContentStr);
	void                TraceIpRtpDtmfInputInd(CObjString* pContentStr);
	void                TraceEncoderParams(CObjString* pContentStr, const ENCODER_PARAM_S* pEncoderParams = NULL);
	void                TraceDecoderParams(CObjString* pContentStr, const DECODER_PARAM_S* pDecoderParams = NULL);
	void                TraceUpdateEncoderParams(CObjString* pContentStr);
	void                TraceUpdateDecoderParams(CObjString* pContentStr);
	void                TraceVideoEncChangeLayoutReq(CObjString* pContentStr);
	void                TraceVideoEncChangeLayoutAttributesReq(CObjString* pContentStr);
	void                TraceImageParams(CObjString* pContentStr, const IMAGE_PARAM_S& imageParam);
	void                TraceChangeLayoutAttributes(CObjString* pContentStr, const CHANGE_LAYOUT_ATTRIBUTES_S& tChangeLayoutAttributes);
	void                TraceChangeLayoutAttributes(CObjString* pContentStr, const MCMS_CM_CHANGE_LAYOUT_ATTRIBUTES_S& tChangeLayoutAttributes);
	void                TraceBorder(CObjString* pContentStr, const BORDERS_PARAM_S& border);
	void                TraceBorder(CObjString* pContentStr, const MCMS_CM_BORDERS_PARAM_S& border);
	void                TraceBorderEdge(CObjString* pContentStr, int i, const MCMS_CM_BORDER_PARAM_S& borderEdges);
	void                TraceSpeaker(CObjString* pContentStr, const SPEAKER_PARAM_S& speaker);
	void                TraceSpeaker(CObjString* pContentStr, const MCMS_CM_SPEAKER_PARAM_S& speaker);
	void                TraceBackground(CObjString* pContentStr, const BACKGROUND_PARAM_S& bckgnd);
	void                TraceSiteNames(CObjString* pContentStr, const SITENAMES_PARAM_S& siteNames);
	void                TraceSiteNames(CObjString* pContentStr, const MCMS_CM_SITENAMES_PARAM_S& siteNames);
	void                TraceFadeInFadeOut(CObjString* pContentStr, const FADE_IN_FADE_OUT_PARAM_S& fadeInFadeOut);
	void                TraceVisualAttributes(CObjString* pContentStr, const VISUAL_ATTRIBUTES_S& VisualAttributes);
	void                TraceVisualAttributes(CObjString* pContentStr, const MCMS_CM_VISUAL_ATTRIBUTES_S& VisualAttributes);
	char*               GetResolutionRatioParamString(const DWORD& resolutionRatio);
	void                TraceDecoderDetectedModeParams(CObjString* pContentStr, const DECODER_DETECTED_MODE_PARAM_S& decoderDetectedModeParamsStruct);
	void                TraceH264VideoParams(CObjString* pContentStr, const H264_VIDEO_PARAM_S& h264VideoParams);
	void                TraceH263_H261VideoParams(CObjString* pContentStr, const H263_H261_VIDEO_PARAM_S& h263_h261VideoParams);
	void                TraceUpdateDecoderResolutionReq(CObjString* pContentStr);
	void                TraceAllocateStatusPerUnitReq(CObjString* pContentStr);
	void                TraceNetSetupReq(CObjString* pContentStr);
	void                TraceNetSetupInd(CObjString* pContentStr);
	void                TraceNetConnectInd(CObjString* pContentStr);
	void                TraceNetClearReq(CObjString* pContentStr);
	void                TraceNetClearInd(CObjString* pContentStr);
	void                TraceNetProgressInd(CObjString* pContentStr);
	void                TraceNetAlertInd(CObjString* pContentStr);
	void                TraceNetAlertReq(CObjString* pContentStr);
	void                TraceNetConectReq(CObjString* pContentStr);
	void                TraceNetDisconnectReq(CObjString* pContentStr);
	void                TraceNetDisconnectAckInd(CObjString* pContentStr);
	void                TraceNetProceedInd(CObjString* pContentStr);
	void                TraceNetDisconnectInd(CObjString* pContentStr);
	void                TraceNetCommonParams(CObjString* pContentStr, const NET_COMMON_PARAM_S& netParamStruct);
	void                TraceNetCause(CObjString* pContentStr, const CAUSE_DATA_S& causeDataStruct);
	void                TraceCallingParty(CObjString* pContentStr, const NET_CALLING_PARTY_S& callingPartyStruct);
	void                TraceCalledParty(CObjString* pContentStr, const NET_CALLED_PARTY_S& calledPartyStruct);

	void                PrintHexNum(CObjString* pContentStr, const char* str, int size);

	void                TraceMcmsMplPhysicalRsrcInfo(CObjString* pContentStr, const MCMS_MPL_PHYSICAL_RESOURCE_INFO_S& McmsMplPhysicalRsrcInfoStruct);
	void                TracePhysicalRsrcInfo(CObjString* pContentStr, const PHYSICAL_RESOURCE_INFO_S& PhysicalRsrcInfoStruct);
	void                TracePhysicalUnitParams(CObjString* pContentStr, const PHYSICAL_UNIT_PARAMS_S& PhysicalUnitParamsStruct);
	void                TraceAudioOpenDecoderReq(CObjString* pContentStr);
	void                TraceAudioOpenEncoderReq(CObjString* pContentStr);
	void                TraceUpdateAudioAlgorithmReq(CObjString* pContentStr);
    void	 		    TraceUpdateAudioDecoderReq(CObjString *pContentStr);
	void                TraceAudioMuteReq(CObjString* pContentStr);
	void                TraceUpdateAudioGainReq(CObjString* pContentStr);
	void                TraceUpdateUseSpeakerSsrcForTxReq(CObjString* pContentStr);
	void                TraceUpdateNoiseDetectionReq(CObjString* pContentStr);
	void                TraceUpdateAudioStandaloneReq(CObjString* pContentStr);
	void                TraceUpdateAudioConnectionStatus(CObjString* pContentStr);
	void                TraceUpdateAGC(CObjString* pContentStr);
	void                TraceOpenConfReq(CObjString* pContentStr);
	void                TraceSpeakerInd(CObjString* pContentStr);
	void                TraceCmOpenUdpPortOrUpdateUdpAddrReq(CObjString* pContentStr, const char* OpcodeString);
	void                TraceCmCloseUdpPortReq(CObjString* pContentStr, const char* OpcodeString);
	void                TraceVideoGraphicsShowTextBoxRequest(CObjString* pContentStr);
	void                TraceVideoGraphicsShowIconRequest(CObjString* pContentStr);
	void                TraceAudioEncUpdateSeenImageReq(CObjString* pContentStr);
	void				TraceIceInitInd(CObjString *pContentStr);
	void				TraceUpdateBitRate(CObjString* pContentStr);

	// IP party to MFA requests
	void                TraceCmKillUdpPort(CObjString* pContentStr, const char* OpcodeString);
	void                TraceMediaRecordingPath(CObjString* pContentStr, const char* OpcodeString);
	void                TraceMoveResourceMFAReq(CObjString* pContentStr);

	void                TraceXmlTransportAddress(CObjString* transportAddr, const xmlUnionPropertiesSt& unionProps);
	void                TraceTransportAddrSt(CObjString* transportAddr, const mcXmlTransportAddress& IpAddress);
	void                TraceIpRtpSetFeccPartyType(CObjString* pContentStr, const char* OpcodeString);
	void                TraceCmRtcpVideoPreference(CObjString* pContentStr, const char* OpcodeString);
	void                TraceCmRtcpReceiverBandwidth(CObjString* pContentStr, const char* OpcodeString);

	void                TraceDtlsStartReq(CObjString *pContentStr, const char* OpcodeString);
	void                TraceDtlsEndInd(CObjString *pContentStr);
	void                TraceMrmpStreamIsMust(CObjString *pContentStr);

	// Shelf Mngt Ind
	void                TraceSMFatalFailureInd(CObjString* pContentStr, const char* OpcodeString);
	void                TraceNetSetupHeader(CObjString* pContentStr, const NET_SETUP_REQ_HEADER_S& netParamStruct);
	void                TraceMuxInitComm(CObjString* pContentStr);
	void                TraceEndInitComm(CObjString* pContentStr);
	void                TraceSetXmitMode(CObjString* pContentStr);
	void                TraceRmtXmitMode(CObjString* pContentStr);
	void                TraceExchangeCap(CObjString* pContentStr);
	void                TraceRemoteBasCap(CObjString* pContentStr);
	void                TraceH230MBE(CObjString* pContentStr);
	void                TraceMuxRepeatedH230(CObjString* pContentStr);
	void                TraceRemoteMframeSync(CObjString* pContentStr);
	void                TraceLocalMframeSync(CObjString* pContentStr);
	void                TraceSmartRecovery(CObjString* pContentStr);
	void                TraceUnitReconfig(CObjString* pContentStr);
	void                TraceSlotsNumbering(CObjString* pContentStr);
	void                TraceSetEcs(CObjString* pContentStr);
	void                TraceRemoteEcs(CObjString* pContentStr);
	void                TraceEcsBlockType(CObjString* pContentStr, BYTE block_header);
	void                TraceEncKeysInfoReq(CObjString* pContentStr);
	void                TraceAudioUpdateCompressedDelay(CObjString* pContentStr);
	void                TracePCMMessage(CObjString* pContentStr);
	void                TraceConnectDisconnetPCM(CObjString* pContentStr);

	// LPR common params
	void                TraceLprSpecificParams(CObjString* pContentStr, const TLprSpecificParams& _struct);

	void                TraceRecoveryUnit(CObjString* pContentStr);
	void                TraceRecoveryReplaceUnit(CObjString* pContentStr);

	void                TracePortDebugInfoInd(CObjString* pContentStr);
	void                TraceConfPortDebugInfoReq(CObjString* pContentStr);


	// cards
	void                TraceCardConfigReq(CObjString* pContentStr);
	void                TraceSetLogLevelReq(CObjString* out) const;

	BYTE                IsContentSizeValid();

	// SRTP
	void                TraceSdesSpecificParams(CObjString* pContentStr, const sdesCapSt& _struct);

	void                TraceVideoEncIndicationIcon(CObjString* pContentStr, const ICON_ATTR_S& tIcon, iconTypeEnum eIconType, size_t nCell = INVALID);
	void                TraceVideoEncLayoutIndications(CObjString* pContentStr);
	void                TraceVideoEncLayoutIndications(CObjString* pContentStr, const ICONS_DISPLAY_S& tIndications);

	void                TraceH264_SvcParams(CObjString* pContentStr, const H264_SVC_VIDEO_PARAM_S* pencoderH264SvcParams);


	CMplMcmsProtocol*   m_pMplMcmsProt;

private:
	// TIP
	void                TraceTipSystemWideOptionsStruct(CObjString* pContentStr, TipSystemWideOptionsSt& systemWideOptions);
	void                TraceTipAudioMuxStruct(CObjString* pContentStr, TipAudioMuxCntlSt& audioMuxCntl);
	void                TraceTipVideoMuxStruct(CObjString* pContentStr, TipVideoMuxCntlSt& videoMuxCntl);
	void                TraceTipAudioOptionsStruct(CObjString* pContentStr, TipAudioOptionsSt& audioOptions, const char* strDirection);
	void                TraceTipVideoOptionsStruct(CObjString* pContentStr, TipVideoOptionsSt& videoOptions, const char* strDirection);
	void                TraceTipNegotiationStruct(CObjString* pContentStr, TipNegotiationSt& negotiationSt);
	void                TraceTipStartNegotiationReq(CObjString* pContentStr);
	void                TraceTipNegotiationResultInd(CObjString* pContentStr);
	void                TraceTipEndNegotiationReq(CObjString* pContentStr);
	void                TraceTipUpdateCloseCall(CObjString* pContentStr);
	void                TracePartyMonitoringReq(CObjString* pContentStr);
	void                TracePartyVideoChannelsStatisticsReq(CObjString *pContentStr); //CDR_MCCF
	void                TraceTipContentMsgStruct(CObjString* pContentStr, TipContentMsgSt& ContentMsgSt);
	void                TraceTipContentMsgReq(CObjString* pContentStr);
	void                TraceTipContentMsgInd(CObjString* pContentStr);
	void				  TracePartyMrmpPartyMonitoringReq(CObjString *pContentStr);

	void                DumpHex(int data_len);
	void                DumpHex(CObjString* pContentStr, int data_len, char* data_buff);
	const char*         GetEcsOpcodeStr(DWORD identifier);

	bool                CheckPeriodTraceFilter(OPCODE opcode) const;
	bool                IsKeepAlive(OPCODE opcode) const;

	CManDefinedString*  m_ProtStr;
	void TraceOperationPointsSet(CObjString* pContentStr);
	void TraceOperationPoint(CObjString* pContentStr, const VIDEO_OPERATION_POINT_S &videoOperationPointStruct);
	void TraceAudioRelayDecReq(CObjString *pContentStr);
	void TraceAudioRelayEncReq(CObjString *pContentStr);
	void TraceAudioRelayRequests(CObjString *pContentStr);

	void TraceCmMrmpOpenChannelReq(CObjString *pContentStr, const char* OpcodeString);
	void TraceCmMrmpScpStreamReqInd(CObjString *pContentStr, const char* OpcodeString);
	void TraceCmMrmpScpStreamAckReq(CObjString *pContentStr, const char* OpcodeString);
	void TraceCmMrmpScpNotificationReq(CObjString *pContentStr, const char* opcodeString);
	void TraceCmMrmpScpNotificationInd(CObjString *pContentStr, const char* opcodeString);
	void TraceCmMrmpScpNotification(CObjString *pContentStr, const char* opcodeString);
	void TraceCmMrmpScpNotificationAckInd(CObjString *pContentStr, const char* opcodeString);
	void TraceCmMrmpScpNotificationAckReq(CObjString *pContentStr, const char* opcodeString);
	void TraceCmMrmpScpNotificationAck(CObjString *pContentStr, const char* opcodeString);
	void TraceCmMrmpRtcpFirReq(CObjString *pContentStr, const char* OpcodeString);
	void TraceCmMrmpRtcpFirInd(CObjString *pContentStr, const char* OpcodeString);
	void TraceCmMrmpScpPipesMappingNotificationReq(CObjString *pContentStr);
	void TraceCmMrmpPartyMonitoringInd(CObjString *pContentStr);
	void TraceCmMrmpCloseChannelReq(CObjString *pContentStr, const char* OpcodeString);
	void TraceVideoSourcesReq(CObjString *pContentStr,const char* OpcodeString);
	void TraceCmMrmpScpIvrStateNotificationReq(CObjString *pContentStr,const char* OpcodeString);


	void UCS2Dump(const APIS8* pUCS2Buff, WORD iLen, CObjString* psResult);
	void UCS2Dump(const APIS8* pUCS2Buff, WORD iLenSrc, char* psResult, WORD iLenDest);
	void TraceIceInitReq(CObjString *pContentStr);
	void TraceHighUsageCPUInd(CObjString* pContentStr);

	void TraceCGPlayAudioReq(CObjString *pContentStr);

	void TraceWebRtcConnectReq(CObjString *pContentStr);
	void TraceWebRtcConnectInd(CObjString *pContentStr);
	void TraceWebRtcExternalUdpAddr(CObjString *pContentStr, const mcTransportAddress *pAddr);
	void TraceWebRtcInternalUdpAddr(CObjString *pContentStr, const mcReqCmOpenUdpPortOrUpdateUdpAddr* pUdpAddr);
};

#endif  // MPL_MCMS_PROTOCOL_TRACER_H_

