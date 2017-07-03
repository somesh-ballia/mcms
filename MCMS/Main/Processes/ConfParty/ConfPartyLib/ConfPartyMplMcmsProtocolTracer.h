#ifndef CONFPARTYMPLMCMSPROTOCOL_H_
#define CONFPARTYMPLMCMSPROTOCOL_H_

#include "McmsProcesses.h"
#include "MplMcmsProtocolTracer.h"
#include "Bonding.h"

class CMplMcmsProtocol;

class CConfPartyMplMcmsProtocolTracer : public CMplMcmsProtocolTracer
{
CLASS_TYPE_1(CConfPartyMplMcmsProtocolTracer, CMplMcmsProtocolTracer)
public:
	CConfPartyMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt);
	virtual ~CConfPartyMplMcmsProtocolTracer();
	virtual void TraceContent(CObjString* pContentStr, eProcessType processType = eProcessTypeInvalid);
	virtual const char* NameOf() const { return "CConfPartyMplMcmsProtocolTracer";}

	// Opcodes (Messages) Trace

	// SIP Request trace functions
	void TraceSipInviteReq(CObjString* pContentStr);
	void TraceSubscribeReq(CObjString* pContentStr);
	void TraceSipInviteAckReq(CObjString* pContentStr);
	void TraceSipInviteResponseReq(CObjString* pContentStr);
	void TraceSipByeReq(CObjString* pContentStr);
	void TraceSipBye200OkReq(CObjString* pContentStr);
	void TraceSipRingingReq(CObjString* pContentStr);
	void TraceSipCancelReq(CObjString* pContentStr);
	void TraceSipReInviteReq(CObjString* pContentStr);
	void TraceSipInfoUnionReq(CObjString* pContentStr);
	void TraceSipInfoResReq(CObjString* pContentStr1);
	void TraceSipDialogRecoveryReq(CObjString* pContentStr);
	void TraceSipSendCrlfReq(CObjString* pContentStr);
  void TraceSipSocketActivityReqInd(CObjString* pContentStr);
  void TraceSipCCCPInviteReq(CObjString* pContentStr);

	// SIP Indication functions
	void TraceSipInviteInd(CObjString* pContentStr);
	void TraceSipCCCPInviteInd(CObjString* pContentStr);
	void TraceSipInviteAckInd(CObjString* pContentStr);
	void TraceSipInviteResponseInd(CObjString* pContentStr);
	void TraceSipReInviteInd(CObjString* pContentStr);
	void TraceSipCancelInd(CObjString* pContentStr);
	void TraceSipByeInd(CObjString* pContentStr);
	void TraceSipBye200OkInd(CObjString* pContentStr);
	void TraceSipBadStatusInd(CObjString* pContentStr);
	void TraceSipSessionTimerExpiredInd(CObjString* pContentStr);
	void TraceSipSessionTimerReinviteInd(CObjString* pContentStr);
	void TraceSipTraceInfoInd(CObjString* pContentStr);
	void TraceSipInfoUnionInd(CObjString* pContentStr);
	void TraceSipInfoRespInd(CObjString* pContentStr);
	void TraceSipReferInd(CObjString* pContentStr);
	void TraceSipDialogRecoveryInd(CObjString* pContentStr);
  void TraceCrlfErrInd(CObjString* pContentStr);

	// Cs <-> ConfParty req/ind
	// Requests
	void TraceGetPortReq(CObjString* pContentStr);
	void TraceReleasePortReq(CObjString* pContentStr);
	void TraceCallSetUpReq(CObjString* pContentStr);
	void TraceMcCallTransient(CObjString* callTransientString, mcCallTransient callTransient);
	void TraceH460AvayaFeVndrReqSt(CObjString* h460AvayaString, h460AvayaFeVndrReqSt avfFeVndIdReq);
	void TraceEncTokensHeaderStruct(CObjString* encryptionTokenString, encTokensHeaderStruct *encryTokens);
	void TraceCallAnswerReq(CObjString* pContentStr);
	void TraceCreateControlReq(CObjString* pContentStr,WORD flag);
	void TraceIncomingChannelResponseReq(CObjString* pContentStr);
	void TraceOutgoingChannelReq(CObjString* pContentStr);
	void TraceChannelNewRateReq(CObjString* pContentStr);
	void TraceChannelMaxSkewReq(CObjString* pContentStr);
	void TraceChannelOffReq(CObjString* pContentStr);
	void TraceMultipointModeComTerminalIdMessageReq(CObjString* pContentStr);
	void TraceChannelOnReq(CObjString* pContentStr);
	void TraceRoleTokenMessageReq(CObjString* pContentStr);
	void TraceCallDropTimerExpieredReq(CObjString* pContentStr);
	void TraceChannelDropReq(CObjString* pContentStr);
	void TraceCallDropReq(CObjString* pContentStr);
	void TraceAuthenticationReq(CObjString* pContentStr);
	void TraceStopAllProcessesReq(CObjString* pContentStr);
	void TraceConferenceResponseReq(CObjString* pContentStr);
	void TraceConferenceIndicationReq(CObjString* pContentStr);
	void TraceUnexpectedMessageReq(CObjString* pContentStr);
	void TraceDbc2OmmOnOffReq(CObjString* pContentStr,WORD flag);
	void TraceDtmfInputReq(CObjString* pContentStr);
    void TraceFacilityReq(CObjString* pContentStr);
    void  TraceSipBFCPMsgReq(CObjString* pContentStr);

	// Indications
	void TraceGetPortInd(CObjString* pContentStr);
	void TraceCallReportInd(CObjString* pContentStr, DWORD opcode);
	void TraceCallOfferingInd(CObjString* pContentStr);
	void TraceCallConnectedInd(CObjString* pContentStr);
	void TraceH460AvayaFeVndrIndSt(CObjString* h460AvayaString, h460AvayaFeVndrIndSt avfFeVndIdInd);
	void TraceCallNewRateInd(CObjString* pContentStr);
	void TraceCallCntlConnectedInd(CObjString* pContentStr);
	void TraceCallRoleTokenInd(CObjString* pContentStr);
	void TraceCapabilitiesInd(CObjString* pContentStr);
	void TraceFacilityInd(CObjString* pContentStr);
	void TraceBadSpontanInd(CObjString* pContentStr);
	void TraceAuthenticationInd(CObjString* pContentStr);
	void TraceIncomingChannelInd(CObjString* pContentStr);
	void TraceIncomingChannelConnectedInd(CObjString* pContentStr);
	void TraceOutgoingChannelResponseInd(CObjString* pContentStr);
	void TraceChannelNewRateInd(CObjString* pContentStr);
	void TraceStartChannelCloseInd(CObjString* pContentStr);
	void TraceChannelCloseInd(CObjString* pContentStr);
	void TraceChannelMaxSkewInd(CObjString* pContentStr);
	void TraceFlowControlIndInd(CObjString* pContentStr);
	void TraceDbc2OmmOnOffInd(CObjString* pContentStr,WORD flag);
	void TraceNonStandardInd(CObjString* pContentStr, DWORD opcode);
	void TraceConfReqInd(CObjString* pContentStr);
	void TraceConfComInd(CObjString* pContentStr);
	void TraceConfIndInd(CObjString* pContentStr);

	void TraceIpRtpCmMonitoringInd(CObjString *pContentStr, const char* opcodeString);
	void TraceIpRtpCmVideoChannelsStatisticsInd(CObjString *pContentStr);	//CDR_MCCF

	void TraceIpRtpFeccTokenInd(CObjString *pContentStr);
	void TraceIpRtpFeccKeyInd(CObjString *pContentStr);
	void TraceIpRtpDifferentPayloadTypeInd(CObjString *pContentStr);
	void TraceIpRtpFeccMuteInd(CObjString *pContentStr);
	void TraceIpRtpVideoFastUpdateInd(CObjString *pContentStr);
	void TraceIpRtpBadSpontaneousInd(CObjString *pContentStr);
	void TraceIpRtpStreamStatusInd(CObjString *pContentStr);
	void TraceIpPartyMonitoringAdvance(CObjString& msg,void *pAdvanceMonitor);
	void TraceIpCmRtcpMsgInd(CObjString *pContentStr);
	void TraceIpCmRtcpMsgReq(CObjString *pContentStr);
	void TraceIpCmRtcpMsg(CObjString *pContentStr);
	void TraceRtpUpdatePortOpenChannelReq(CObjString* pContentStr, const char* OpcodeString);
	void TraceRtpUpdateChannelReq(CObjString* pContentStr, const char* OpcodeString);
	void TraceRtpUpdateChannelRateReq(CObjString* pContentStr, const char* OpcodeString);
	void TraceRtpUpdateRtpSpecificChannelParamsReq(CObjString *pContentStr, const TUpdateRtpSpecificChannelParams &pStruct);
	void TraceRtpFeccTokenResponseReq(CObjString *pContentStr);
	void TraceUpdateMtPairReq(CObjString *pContentStr);
	void TraceCmOpenUdpPortOrUpdateUdpAddrReq(CObjString *pContentStr, const char* OpcodeString);
	void TraceCmCloseUdpPortReq(CObjString *pContentStr,  const char* OpcodeString);
	void TraceRtpStreamOnReq(CObjString *pContentStr, const char* OpcodeString) ;
	void TraceRtpStreamOffReq(CObjString *pContentStr, const char* OpcodeString) ;
	void TraceRtpContentOnOffReq(CObjString *pContentStr, const char* OpcodeString);
	void TraceRtpEvacuateStreamReq(CObjString *pContentStr, const char* OpcodeString);
	void TraceIpMediaDetectionInd(CObjString *pContentStr);

    // ISDN
    // ConfParty ==> Bonding Requests
    void TraceBondingInitReq(CObjString *pContentStr);
    void TraceBondingAddChannelReq(CObjString *pContentStr);
    void TraceBondingEndNegotiationInd(CObjString *pContentStr);
    void TraceBondingRemoteLocalAlignmentInd(CObjString *pContentStr);
    void TraceBondingRequesParamsInd(CObjString *pContentStr);
    void TraceResponseBondingAckParamsReq(CObjString *pContentStr);

    // Bonding structs
	void TraceBondingCallParams(CObjString *pContentStr,BND_CALL_PARAMS_S& callParams);
    void TraceBondingNegotiationParams(CObjString *pContentStr,BND_NEGOTIATION_PARAMS_S& negotiationParams);
    void TraceBondingPhoneNumber(CObjString *pContentStr,BND_PHONE_NUM_S& additional_dial_in_phone_num);
    void TraceBondingPhoneNumberList(CObjString *pContentStr,BND_PHONE_LIST_S& phone_list);

    //Party Preview
    void TraceStartPartyPreviewReq(CObjString* pContentStr);
    void TraceStopPartyPreviewReq(CObjString* pContentStr);

	// video bridge to DSP req
	void TraceGraphicsShowTextBoxParams(CObjString* pContentStr);

	// LPR Tracing
	// Req
	void TraceRtpLprModeChangeReq(CObjString *pContentStr, const char* OpcodeString);
	void TraceLPRModeChangeReq(CObjString* pContentStr);

	// Ind
	void TraceIpRtpLprModeChangeInd(CObjString *pContentStr);
	void TraceLPRModeChangeInd(CObjString* pContentStr1);

	//added by Jason for ITP-Multiple channels
	void TraceNewITPSpeakerReq(CObjString* pContentStr);
	void TraceNewITPSpeakerInd(CObjString* pContentStr1);
	
	// DBA
	void TraceIpRtpDbaInd(CObjString *pContentStr);
//	void TracePartyKeepAliveInd(CLargeString* pContentStr);

    //ICE
    void TraceIceGeneralIndication (CObjString *pContentStr);
	void TraceIceGeneralRequest (CObjString *pContentStr);
	void TraceIceAckIndication(CObjString *pContentStr);
	void TraceIceErrIndication(CObjString *pContentStr);

    char * GetChannelTypeStr (APIU32 channelType);

    void TraceRtcpVideoPreferencesInd (CObjString *pContentStr);
    void TraceRtcpVideoPreferencesReq (CObjString *pContentStr);
    void TraceRtcpRtpFeedbackReq (CObjString *pContentStr);
    void TraceRtcpRtpFeedbackInd (CObjString *pContentStr);
    void TraceRtcpReceiverBandwidthReq (CObjString *pContentStr);
    void TraceRtcpReceiverBandwidthInd (CObjString *pContentStr);

    // BFCP
    void TraceBfcpMessageReq(CObjString *pContentStr);
    void TraceBfcpMessageInd(CObjString *pContentStr);
    void TraceBfcpTcpTransportMessageInd(CObjString *pContentStr);

    // VSR
    void TraceVsrMessageGen(CObjString *pContentStr);
    void TraceVsrMessageReq(CObjString *pContentStr);
    void TraceVsrMessageInd(CObjString *pContentStr);

    // MS SVC PLI
    void TraceMsSvcPliMessageGen(CObjString *pContentStr);
    void TraceMsSvcPliMessageReq(CObjString *pContentStr);
    void TraceMsSvcPliMessageInd(CObjString *pContentStr);

    // MS SVC Mux/Dmux
    void TraceMsSvcSingleStreamDesc(CObjString *pContentStr, const SingleStreamDesc* pDesc, const BYTE noOfStreamDescs);
    void TraceMsSvcMonitoringStreamDesc(CObjString *pContentStr, const DspInfoForPartyMonitoringReq* pDesc, const BYTE noOfStreamDescs);
    void TraceMsSvcP2PInitReq(CObjString *pContentStr);
    void TraceMsSvcAvMcuInitReq(CObjString *pContentStr);
    void TraceMsSvcAvMcuMuxReq(CObjString *pContentStr);
    void TraceMsSvcAvMcuDmuxReq(CObjString *pContentStr);
    void TraceMsSvcAvMcuMontioringReq(CObjString *pContentStr);

    //FEC and LYNC2013_FEC_RED:
    void TraceRtpFecRedOnOffReq(CObjString *pContentStr);
    void TracePacketLossStatusInd(CObjString *pContentStr);

    // DTLS
    void TraceDtlsStartReq(CObjString *pContentStr, const char* OpcodeString);
    void TraceDtlsEndInd(CObjString *pContentStr);

};

#endif /*CONFPARTYMPLMCMSPROTOCOL_H_*/
