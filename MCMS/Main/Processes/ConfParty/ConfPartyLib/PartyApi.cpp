#include "PartyApi.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
#include "H323Scm.h"
#include "H323Caps.h"
#include "H320Caps.h"
#include "H320ComMode.h"
#include "IPUtils.h"
#include "StatusesGeneral.h"
#include "H323NetSetup.h"
#include "SipScm.h"
#include "SipCaps.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "SipDefinitions.h"
#include "SipNetSetup.h"
#include "OpcodesMcmsAudio.h"
#include "ConfPartyDefines.h"
#include "IsdnNetSetup.h"
#include "PartyRsrcDesc.h"
#include "OpcodesMcmsBonding.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "LPRData.h"
#include "ScpHandler.h"
#include "SysConfigKeys.h"
#include "ScpNotificationWrapper.h"
#include "ScpPipeMappingNotification.h"
#include "Party.h"

#define MAX_NUM_RECV_STREAMS_FOR_MIX_AVC_VIDEO 2
#define MAX_NUM_RECV_STREAMS_FOR_VSW_RELAY     2  /* 1 for video , 1 for audio */

////////////////////////////////////////////////////////////////////////////
//                        CPartyApi
////////////////////////////////////////////////////////////////////////////
CPartyApi::CPartyApi() {
}

//--------------------------------------------------------------------------
CPartyApi::~CPartyApi() {
}

//--------------------------------------------------------------------------
const char* CPartyApi::NameOf() const {
	return "CPartyApi";
}

//--------------------------------------------------------------------------
void CPartyApi::Create(void (*entryPoint)(void*), COsQueue& creatorRcvMbx,
		COsQueue& confRcvMbx, DWORD partyRsrcId, DWORD partyMonitorId,
		DWORD confMonitorId, const char* numericConfId, const char* partyName,
		const char* confName, WORD serviceId, WORD termNum, WORD mcuNum,
		const char* password, WORD voice, WORD isChairEnabled, BYTE isGateWay,
		BYTE isRecording, BYTE isAutoVidBitRate,
		BYTE isNoVideoRsrcForVideoParty) {
	CTaskApi::Create(creatorRcvMbx); // set default stack param i.e. creator rsmbx

	confRcvMbx.Serialize(/*NATIVE,*/m_appParam);

	m_appParam << partyMonitorId << confMonitorId << partyName << confName
			<< numericConfId << termNum << mcuNum << voice << isChairEnabled
			<< isGateWay << isRecording << serviceId << isAutoVidBitRate
			<< isNoVideoRsrcForVideoParty;

	LoadApp(entryPoint); // load application

	CParty* pParty = (CParty*) GetTaskAppPtr();
	PASSERT_AND_RETURN(!pParty);

	if (partyRsrcId == DEFAULT_PARTY_ID)
	{
		partyRsrcId = GetLookupIdParty()->Alloc();

//#ifdef LOOKUP_TABLE_DEBUG_TRACE
//	PBTTRACESTREAM(1, "D.K. Party:" << std::hex << (DWORD)pParty << ", PartyId:" << DEFAULT_PARTY_ID);
//#endif
	}
	else
	{
//#ifdef LOOKUP_TABLE_DEBUG_TRACE
//	PBTTRACESTREAM(1, "D.K. Party:" << std::hex << (DWORD)pParty << ", PartyId:" << partyRsrcId);
//#endif
	}

	pParty->SetPartyId(partyRsrcId);
	GetLookupTableParty()->Add(partyRsrcId, pParty);
}

//--------------------------------------------------------------------------
void CPartyApi::ForceKill() {
	SendOpcodeMsg(FORCE_KILL);
	DestroyOnlyApi();
}

//--------------------------------------------------------------------------/
void CPartyApi::HandlePartyExternalEvent(CSegment* pMsg, OPCODE opCode) {
	SendMsg(pMsg, opCode);
}

//--------------------------------------------------------------------------
void CPartyApi::SetServiceProvider(char* ServiceProvider) {
	CSegment* seg = new CSegment;

	WORD len = 0;
	if (ServiceProvider)
		len = strlen((const char*) ServiceProvider) + 1; // for string used.

	*seg << len;
	if (len)
		seg->Put((unsigned char*) ServiceProvider, len);

	SendMsg(seg, SERVICENAME);
}

//--------------------------------------------------------------------------
WORD CPartyApi::EstablishCallPstn(PartyRsrcID partyRsrcID, CNetSetup* pNetSetup,
		CRsrcParams* pNetRsrcParams, CIsdnPartyRsrcDesc* pPartyRsrcDesc,
		WORD room_id) {
	CSegment* seg = new CSegment;

	*seg << (DWORD) partyRsrcID;

	for (WORD i = 0; i < 1; i++)
		pNetSetup[i].Serialize(NATIVE, *seg);

	pNetRsrcParams->Serialize(NATIVE, *seg);
	pPartyRsrcDesc->Serialize(NATIVE, *seg);
	pPartyRsrcDesc->DumpToTrace();

	*seg << room_id;

	SendMsg(seg, ESTABLISHCALL);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
WORD CPartyApi::EstablishVideoCall(PartyRsrcID partyRsrcID,
		CNetSetup* pNetSetup, CRsrcParams* pNetRsrcParams,
		CIsdnPartyRsrcDesc* pPartyRsrcDesc, WORD numChnl, CComMode* pConfScm,
		CCapH320* pLocalCaps, WORD mcuNum, WORD terminalNum, WORD room_id) {
	CSegment* seg = new CSegment;

	*seg << (DWORD) partyRsrcID << mcuNum << terminalNum;

	for (WORD i = 0; i < 1; i++)
		pNetSetup[i].Serialize(NATIVE, *seg);

	*seg << numChnl;

	for (WORD j = 0; j < numChnl; j++)
		pNetRsrcParams[j].Serialize(NATIVE, *seg);

	pPartyRsrcDesc->Serialize(NATIVE, *seg);

	pConfScm->Serialize(NATIVE, *seg);

	pLocalCaps->Serialize(NATIVE, *seg);

	*seg << room_id;

	SendMsg(seg, ESTABLISHCALL);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CPartyApi::SendVIN(WORD mcuNumber, WORD terminalNumber,
		PartyRsrcID partyId) {
	CSegment* pSeg = new CSegment;
	*pSeg << (OPCODE) SEND_VIN << (WORD) mcuNumber << (WORD) terminalNumber
			<< (PartyRsrcID) partyId;

	SendMsg(pSeg, NUMBERINGMESSAGE);
}

//--------------------------------------------------------------------------
void CPartyApi::ChangeModeH323(CComModeH323* pScmH323, WORD changeModeState,
		CCapH323* pLocalCapH323, CRsrcParams** avcToSvcTranslatorRsrcParams,
		CRsrcParams* pMrmpRsrcParams) {
	TRACEINTO << "mix_mode: sending CONFCHANGEMODE";
	CSegment* seg = new CSegment;

	*seg << changeModeState;

	BYTE bIsLocalCapH323 = (pLocalCapH323 != NULL);
	*seg << bIsLocalCapH323;

	pScmH323->Serialize(NATIVE, *seg);

	if (pLocalCapH323)
		pLocalCapH323->Serialize(NATIVE, *seg);

	SerializeNonMandatoryRsrcParams(seg, pMrmpRsrcParams);
	SerializeNonMandatoryRsrcParamsArray(seg, avcToSvcTranslatorRsrcParams,
			NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS);

	SendMsg(seg, CONFCHANGEMODE);
}

//--------------------------------------------------------------------------
void CPartyApi::ChangeContentMode(CComModeH323* pScmH323, WORD changeModeState,
		BYTE bIsSpeaker) {
	CSegment* seg = new CSegment;

	*seg << changeModeState;
	pScmH323->Serialize(NATIVE, *seg);
	*seg << bIsSpeaker;

	SendMsg(seg, CONFCONTENTCHANGEMODE);
}

//--------------------------------------------------------------------------
void CPartyApi::TokenRecapCollisionEnded()
{
	SendOpcodeMsg(TOKENRECAPCOLLISIONEND);
}

//--------------------------------------------------------------------------
void CPartyApi::Transfer(WORD mode) // shiraITP - 57
		{
	CSegment* seg = new CSegment;

	*seg << mode;

	SendMsg(seg, LOBBYTRANS);
}

//--------------------------------------------------------------------------
void CPartyApi::RejectCall(CNetSetup* pNetSetUp) {
	CSegment* seg = new CSegment;

	pNetSetUp->Serialize(NATIVE, *seg);

	SendMsg(seg, REJECTCALL);
}

//--------------------------------------------------------------------------
void CPartyApi::LobbyDestroy() {
	// This should be used to destroy parties from the lobby (only h323paries for now)
	// the party will only destroy itself if it's still in the middle of lobby transaction (transfer, ident, etc...)

	SendOpcodeMsg(LOBBYDESTROY);
}

//--------------------------------------------------------------------------
void CPartyApi::NetConnect(WORD seqNum, WORD status, BYTE cause) {
	CSegment* seg = new CSegment;

	*seg << seqNum << status << cause;

	SendLocalMessage(seg, NETCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::NetDisConnect(WORD seqNum, BYTE cause) {
	CSegment* seg = new CSegment;

	*seg << seqNum << cause;

	SendLocalMessage(seg, DISCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdateEncryptionCurrentStateInDB(BYTE encryptionCurrentState) {
	CSegment* seg = new CSegment;

	*seg << (WORD) encryptionCurrentState;

	SendLocalMessage(seg, PARTY_ENCRYPTION_STATE);
}

//--------------------------------------------------------------------------
void CPartyApi::EndNetDisConnect(WORD seqNum, WORD status) {
	CSegment* seg = new CSegment;

	*seg << seqNum << status;

	SendLocalMessage(seg, ENDDISCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::H323PartyDisConnect(WORD cause) {
	CSegment* seg = new CSegment;

	*seg << cause;

	SendLocalMessage(seg, DISCONNECTH323);
}

//--------------------------------------------------------------------------
void CPartyApi::SetSecondaryCause(WORD cause, CSecondaryParams& secParams) {
	CSegment* seg = new CSegment;

	*seg << cause;

	secParams.Serialize(NATIVE, *seg);

	SendLocalMessage(seg, SECONDARYCAUSEH323);
}

//--------------------------------------------------------------------------
void CPartyApi::SetRsrcConfIdForInterface(DWORD destResourceConfId) {
	CSegment* seg = new CSegment;

	*seg << destResourceConfId;

	SendMsg(seg, UPDATERSRCCONFID);
}

//--------------------------------------------------------------------------
void CPartyApi::AssignTerminalNum(WORD mcuNum, WORD termNum) {
	CSegment* seg = new CSegment;

	*seg << mcuNum << termNum;

	SendMsg(seg, H323TERMINALINDASSIGNMENT);
}

//--------------------------------------------------------------------------
void CPartyApi::VideoRefresh(WORD ignore_filtering, DWORD remoteSSRC,
		DWORD priorityID) {
	CSegment* seg = new CSegment;
	*seg << ignore_filtering << remoteSSRC << priorityID;
	SendMsg(seg, VIDREFRESH);
}

//--------------------------------------------------------------------------
void CPartyApi::SendH239VideoCaps() {
	SendOpcodeMsg(SEND_H239_VIDEO_CAPS);
}

//--------------------------------------------------------------------------
void CPartyApi::Export(COsQueue* pDestConfMbx, void* pConfPartyCntl,
		void* pConfPartyDesc, EMoveType eCurMoveType) {
	CSegment* seg = new CSegment;

	*seg << pConfPartyCntl << pConfPartyDesc << (WORD) eCurMoveType;

	pDestConfMbx->Serialize(*seg);
	SendMsg(seg, EXPORT);
}

//--------------------------------------------------------------------------
void CPartyApi::SetMoveDestConfParams(WORD confType, WORD mcuNumber,
		WORD terminalNumber, CCopVideoTxModes* pCopVideoTxModes,
		CVideoOperationPointsSet* pVideoOperationPointsSet) {
	CSegment* seg = new CSegment;

	*seg << confType << mcuNumber << terminalNumber;

	BYTE bIsCopModes = (pCopVideoTxModes) ? TRUE : FALSE;
	*seg << bIsCopModes;
	if (bIsCopModes)
		pCopVideoTxModes->Serialize(NATIVE, *seg);
	BYTE bIsOpPointsSet = (pVideoOperationPointsSet) ? TRUE : FALSE;
	*seg << bIsOpPointsSet;
	if (pVideoOperationPointsSet)
		pVideoOperationPointsSet->Serialize(*seg);

	SendMsg(seg, SETMOVEPARAMS);
}

//--------------------------------------------------------------------------
void CPartyApi::DataTokenRequest(WORD bitRate) {
	CSegment* seg = new CSegment;
	*seg << bitRate;
	SendMsg(seg, DATATOKENREQ);
}

//--------------------------------------------------------------------------
void CPartyApi::DataTokenRelease() {
	CSegment* seg = new CSegment;
	SendMsg(seg, DATATOKENRELEASE);
}

//--------------------------------------------------------------------------
void CPartyApi::DataTokenReleaseAndFree() {
}

//--------------------------------------------------------------------------
void CPartyApi::DataTokenAccept(WORD isCameraControl) {
	CSegment* seg = new CSegment;
	*seg << isCameraControl;
	SendMsg(seg, DATATOKENACCEPT);
}

//--------------------------------------------------------------------------
void CPartyApi::DataTokenReject() {
	CSegment* seg = new CSegment;
	SendMsg(seg, DATATOKENREJECT);
}

//--------------------------------------------------------------------------
void CPartyApi::DataTokenWithdraw() {
	CSegment* seg = new CSegment;
	SendMsg(seg, DATATOKENWITHDRAW);
}

//--------------------------------------------------------------------------
void CPartyApi::DataTokenReleaseRequest() {
	CSegment* seg = new CSegment;
	SendMsg(seg, DATATOKENRELEASEREQ);
}

//--------------------------------------------------------------------------
void CPartyApi::OnIpDataTokenMsg(WORD msgType, WORD bitRate,
		WORD isCameraControl) {
	CSegment* seg = new CSegment;

	*seg << msgType << bitRate << isCameraControl;

	SendMsg(seg, IPDATATOKENMSG);
}
//--------------------------------------------------------------------------
void CPartyApi::OnIpFeccKeyMsg(WORD msgType) {
	CSegment* seg = new CSegment;

	*seg << msgType;

	SendMsg(seg, IPFECCKEYMSG);
}

//--------------------------------------------------------------------------
void CPartyApi::DelNetChannel(WORD seqnum) {
	CSegment* seg = new CSegment;

	*seg << seqnum;

	SendMsg(seg, DELNETCHNL);
}

//--------------------------------------------------------------------------
void CPartyApi::LogicalDelNetChannel(WORD discause) {
	CSegment* seg = new CSegment;
	*seg << discause;
	SendMsg(seg, LOGICALDELNETCHNL);
}

//--------------------------------------------------------------------------
void CPartyApi::SendLeaderStatus(BYTE isLeader) {
	CSegment* seg = new CSegment;

	*seg << isLeader;

	SendMsg(seg, SET_PARTY_AS_LEADER);
}

//--------------------------------------------------------------------------
void CPartyApi::PartyActionsOnLeaderChanged(BYTE isLeader) {
	CSegment* seg = new CSegment;

	*seg << isLeader;

	SendMsg(seg, LEADER_CHANGED);
}

//--------------------------------------------------------------------------
void CPartyApi::InformArtFeccPartyType() {
	SendOpcodeMsg(SET_FECC_PARTY_TYPE);
}

//--------------------------------------------------------------------------
void CPartyApi::H323EstablishCall(PartyRsrcID partyRsrcID, CCapH323* pLocalCap,
		CH323NetSetup* pH323NetSetup,  // shiraITP - 13
		WORD cascadeMode, WORD nodeType, DWORD vidRate, CQoS* pQos,
		CComModeH323* pInitiateScm, WORD encAlg, WORD halfKeyType,
		CRsrcParams* pMfaRsrcParams, CRsrcParams* pMrmpRsrcParams,
		CRsrcParams* pCsRsrcParams, CRsrcParams** avcToSvcTranslatorRsrcParams,
		UdpAddresses sUdpAddressesParams, WORD mcuNum, WORD terminalNum,
		CCopVideoTxModes* pCopVideoTxModes, BYTE bNoVideRsrcForVideoParty,
		CTerminalNumberingManager* pTerminalNumberingManager, WORD room_Id,
		eTypeOfLinkParty linkType,
		CSvcPartyIndParamsWrapper &aSsrcIdsForAvcParty) {
	CSegment* seg = new CSegment;

	*seg << (DWORD) partyRsrcID << cascadeMode << nodeType << vidRate << encAlg
			<< halfKeyType << mcuNum << terminalNum << bNoVideRsrcForVideoParty
			<< (DWORD) pTerminalNumberingManager;

	pH323NetSetup->Serialize(NATIVE, *seg);

	pInitiateScm->Serialize(NATIVE, *seg);
	pLocalCap->Serialize(NATIVE, *seg);
	pQos->Serialize(NATIVE, *seg);

	if (pMfaRsrcParams) {
		pMfaRsrcParams->Serialize(NATIVE, *seg);
	}
	pCsRsrcParams->Serialize(NATIVE, *seg);

	SerializeNonMandatoryRsrcParams(seg, pMrmpRsrcParams);
	SerializeNonMandatoryRsrcParamsArray(seg, avcToSvcTranslatorRsrcParams,
			NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS, "!@#  translator");

	int sizeOfUdps = sizeof(UdpAddresses);
	seg->Put((BYTE*) (&sUdpAddressesParams), sizeOfUdps);

	BYTE bIsCopVideoTxModes = pCopVideoTxModes ? TRUE : FALSE;
	*seg << bIsCopVideoTxModes;

	if (bIsCopVideoTxModes)
		pCopVideoTxModes->Serialize(NATIVE, *seg);

	*seg << room_Id;
	*seg << (WORD) linkType;

	if (pInitiateScm->GetConfMediaType() == eMixAvcSvc) {
		aSsrcIdsForAvcParty.Serialize(*seg);
		aSsrcIdsForAvcParty.Print(
				"mix_mode: CPartyApi::H323EstablishCall serialized aSsrcIdsForAvcParty");

	}

	SendMsg(seg, H323ESTABLISHCALL);
}

//--------------------------------------------------------------------------
// Multiple links for ITP in cascaded conference feature: CPartyApi::CreateNewITPSpeakerAckReq (like CPartyApi::H323EstablishCall)
void CPartyApi::CreateNewITPSpeakerAckReq() {
	CSegment* seg = new CSegment;

	SendMsg(seg, PARTY_SEND_ITPSPEAKER_ACKREQ);
}

//--------------------------------------------------------------------------
// Multiple links for ITP in cascaded conference feature: CPartyApi::CreateNewITPSpeakerReq //shiraITP - 101
void CPartyApi::CreateNewITPSpeakerReq(DWORD numOfActiveLinks, BYTE itpType) {
	CSegment* seg = new CSegment;

	*seg << numOfActiveLinks;
	*seg << itpType;

	SendMsg(seg, VB_SEND_ITPSPEAKER_REQ);   // shiraITP - 102
}

//--------------------------------------------------------------------------
void CPartyApi::IpDisconnectMediaChannel(WORD channelType,
		WORD channelDirection, WORD roleLabel) {
	CSegment* seg = new CSegment;

	*seg << channelType;
	*seg << channelDirection;
	*seg << roleLabel;

	SendMsg(seg, IPDISCONNECTCHANNEL);
}

//--------------------------------------------------------------------------
void CPartyApi::InActivateChannel(WORD channelType, WORD channelDirection,
		WORD roleLabel) {
	CSegment* seg = new CSegment;

	*seg << channelType;
	*seg << channelDirection;
	*seg << roleLabel;

	SendMsg(seg, INACTIVATE_CHANNEL);
}

//--------------------------------------------------------------------------
void CPartyApi::SendCloseChannelToConfLevel(WORD channelType, WORD direction,
		WORD roleLabel) {
	CSegment* seg = new CSegment;
	*seg << channelType << direction << roleLabel;

	SendLocalMessage(seg, PARTY_CLOSE_CHANNEL);
}

//--------------------------------------------------------------------------
// the function used to remove protocol from the Local Capability Set
void CPartyApi::SendRemovedProtocolToConfLevel(WORD NuRemovedProtocols,
		WORD ProtocolPayLoadType, WORD bIsCapEnum) {
	CSegment* seg = new CSegment;

	*seg << NuRemovedProtocols << ProtocolPayLoadType << bIsCapEnum;
	SendLocalMessage(seg, REMOVEPROTOCOL);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdateLocalCapsInConfLevel(CCapH323& localCaps) {
	CSegment* seg = new CSegment;

	localCaps.Serialize(NATIVE, *seg);

	SendLocalMessage(seg, UPDATE_CAPS);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyH323VideoBitRate(APIU32 newBitRate,
		cmCapDirection channelDirection, ERoleLabel eRole) {
	CSegment* seg = new CSegment;

	*seg << (DWORD) newBitRate << (WORD) channelDirection << (WORD) eRole;

	SendLocalMessage(seg, UPDATE_VIDEO_RATE);
}

//--------------------------------------------------------------------------
void CPartyApi::H323EndMediaChannelsConnect(CCapH323& rmtCap,
		CComModeH323& currentScm, WORD status, BYTE bReleaseResourcesInConfCntl,
		BYTE bOnlyAudioConnected, BYTE isCodianVcr) {
	CSegment* seg = new CSegment;

	rmtCap.Serialize(NATIVE, *seg);
	currentScm.Serialize(NATIVE, *seg);
	*seg << status << bReleaseResourcesInConfCntl << bOnlyAudioConnected
			<< isCodianVcr;

	SendLocalMessage(seg, ENDCHANNELCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::SendAudioBridgeConnected() {
	CSegment* seg = new CSegment;

	SendMsg(seg, AUDBRDGCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::SendVideoBridgeConnected() {
	CSegment* seg = new CSegment;

	SendMsg(seg, VIDBRDGCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::SendFeccBridgeConnected() {
	CSegment* seg = new CSegment;

	SendMsg(seg, FECCBRDGCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::SendBridgesDisconnected() {
	CSegment* seg = new CSegment;

	SendMsg(seg, BRDGSDISCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::SendBridgesConnected() {
	CSegment* seg = new CSegment;

	SendMsg(seg, BRDGSCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::SendCsResourceAllocated(CRsrcParams* pCsRsrcParams) {
	CSegment* seg = new CSegment;
	pCsRsrcParams->Serialize(NATIVE, *seg);

	SendMsg(seg, ALLOCATE_PARTY_RSRC_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::EndH323DisConnect(WORD status) {
	CSegment* seg = new CSegment;

	*seg << status;

	SendLocalMessage(seg, ENDH323DISCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::IncreaseDisconnctingTimerInPartyCntl() {
	CSegment* seg = new CSegment;
	SendLocalMessage(seg, INCREASE_DISCONNECT_TIMER);
}

//--------------------------------------------------------------------------
void CPartyApi::IpRmtH230(CSegment* ipSeg) {
	TRACEINTO;
	CSegment* seg = new CSegment;

	*seg << *ipSeg;

	//SendLocalMessage(seg, RMTH230);
	SendMsg(seg, RMTH230);
}
//////////////////////////////////////////////////////////////////////
void CPartyApi::IpSingleIntraForAvMcu(CSegment* ipSeg) {
	TRACEINTO;
	CSegment* seg = new CSegment;

	*seg << *ipSeg;

	//SendLocalMessage(seg, RMTH230);
	SendMsg(seg, SINGE_INTRA_AV_MCU);
}

//--------------------------------------------------------------------------
void CPartyApi::IpRmtSpecificStreamsFastUpdateReq(CSegment* ipSeg) {
	TRACEINTO;
	CSegment* seg = new CSegment;

	*seg << *ipSeg;

	//SendLocalMessage(seg, RMTH230);
	SendMsg(seg, STREAMS_INTRA_REQ);
}
//--------------------------------------------------------------------------
void CPartyApi::ForwardIpRmtH230(CSegment* ipSeg, WORD tipPosition) {
	CSegment* seg = new CSegment;
	*seg << tipPosition;
	*seg << *ipSeg;

	SendLocalMessage(seg, FORWARD_RMTH230);
}

//--------------------------------------------------------------------------
void CPartyApi::MuteVideo(WORD onOff) {
	CSegment* seg = new CSegment;

	*seg << onOff;

	SendLocalMessage(seg, VIDEOMUTE);
}

/////////////////////////////////////////////////////////////////////////////
void CPartyApi::UpdateMuteIcon(WORD onOff)
{
	CSegment*  seg = new CSegment;

	*seg << onOff;

	SendLocalMessage(seg,UPDATE_MUTE_ICON);
}
//--------------------------------------------------------------------------
// Interface with GKmanger (temporary) Add by uri
void CPartyApi::sendPartyArqInd(CSegment* pParam, WORD opcode, WORD localFlag) {
	if (localFlag)
		SendLocalMessage(pParam, opcode);
	else
		SendMsg(pParam, opcode);
}

//--------------------------------------------------------------------------
void CPartyApi::IpLogicalChannelUpdate(DWORD channelType,
		CH323NetSetup* pH323NetSetup, DWORD vendorType) {
	CSegment* seg = new CSegment;

	*seg << (DWORD) vendorType << (DWORD) channelType;
	pH323NetSetup->Serialize(NATIVE, *seg);

	SendLocalMessage(seg, IPLOGICALCHANNELUPDATE);
}

//--------------------------------------------------------------------------
void CPartyApi::IpLogicalChannelConnect(CPrtMontrBaseParams* pPrtMonitor,
		DWORD channelType, DWORD vendorType) {
	CSegment* seg = new CSegment;

	*seg << (DWORD) vendorType << (DWORD) channelType;

	pPrtMonitor->Serialize(NATIVE, *seg);

	SendLocalMessage(seg, IPLOGICALCHANNELCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::IpLogicalChannelDisConnect(DWORD channelType, WORD type,
		BYTE bTransmitting, WORD roleLabel) {
	CSegment* seg = new CSegment;

	*seg << channelType << type << bTransmitting << roleLabel;

	SendLocalMessage(seg, IPLOGICALCHANNELDISCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::H323GateKeeperStatus(BYTE gkState, DWORD reqBandwidth,
		DWORD allocBandwidth, WORD requestInfoInterval, BYTE gkRouted) {
	CSegment* seg = new CSegment;

	*seg << gkState << reqBandwidth << allocBandwidth << requestInfoInterval
			<< gkRouted;

	SendLocalMessage(seg, H323GATEKEEPERSTATUS);
}

//--------------------------------------------------------------------------
void CPartyApi::IpPartyMonitoringUpdateDB(BYTE channelType, BYTE intraSyncFlag,
		BYTE videoBCHSyncFlag, WORD bchOutOfSyncCount, BYTE protocolSyncFlag,
		WORD protocolOutOfSyncCount) {
	CSegment* seg = new CSegment;

	*seg << (BYTE) channelType << (BYTE) intraSyncFlag
			<< (BYTE) videoBCHSyncFlag << (WORD) bchOutOfSyncCount
			<< (BYTE) protocolSyncFlag << (WORD) protocolOutOfSyncCount;

	SendLocalMessage(seg, IPPARTYMONITORING);
}

//--------------------------------------------------------------------------
void CPartyApi::Rmt323CommModeUpdateDB(const CComModeH323* pCurrMode) {
	CSegment* seg = new CSegment;
	pCurrMode->Serialize(NATIVE, *seg);
	SendMsg(seg, RMT323COMMODUPDATE);
}

//--------------------------------------------------------------------------
void CPartyApi::SendPartyMonitoringReq(CTaskApp* pTaskApp) {
	TRACEINTOFUNC << "seg with pTaskApp";

	CSegment* seg = new CSegment;

	*seg << pTaskApp;

	SendMsg(seg, IPPARTYMONITORINGREQ);
}

//--------------------------------------------------------------------------
// Added for FLOW CONTROL//
void CPartyApi::SendFlowControlToCs(DWORD newVidRate, BYTE outChannel,
		CLPRParams* lprParams) {
	CSegment* seg = new CSegment;

	*seg << newVidRate << outChannel;

	BYTE isExt = 0;
	if (lprParams)
		isExt = 1;

	*seg << isExt;

	if (isExt)
		*seg << lprParams->GetLossProtection() << lprParams->GetMTBF()
				<< lprParams->GetCongestionCeiling() << lprParams->GetFill()
				<< lprParams->GetModeTimeout();

	SendMsg(seg, PARTY_FLOWCONTROL);
}

//--------------------------------------------------------------------------
// Added for FLOW CONTROL//
void CPartyApi::Send2ChannelsFlowControlToCard(DWORD OutgoingChannelRate,
		DWORD IncomingChannelRate, DWORD TdmRate) {
	CSegment* seg = new CSegment;

	*seg << OutgoingChannelRate << IncomingChannelRate << TdmRate;

	SendMsg(seg, PARTY_FLOWCONTROL_2CHANNELS);
}

//--------------------------------------------------------------------------
// Added for FLOW CONTROL//
void CPartyApi::RemoveSelfFlowControlConstraint() {
	CSegment* seg = new CSegment;
	SendMsg(seg, PARTY_REMOVE_SELF_FLOWCONTROL_CONSTRAINT);
}

//--------------------------------------------------------------------------
void CPartyApi::H323ConnectFeccControl(int rmtType, WORD casacdeMode) {
	CSegment* seg = new CSegment;

	*seg << (WORD) rmtType << (WORD) casacdeMode;

	SendMsg(seg, H323CONNECTCHAIRCNTL);
}

//--------------------------------------------------------------------------
void CPartyApi::RejectH323Call(CH323NetSetup* pH323NetSetup, int reason) {
	CSegment* seg = new CSegment;

	pH323NetSetup->Serialize(NATIVE, *seg);

	*seg << (WORD) reason;

	SendMsg(seg, REJECTCALL);
}

//--------------------------------------------------------------------------
void CPartyApi::IdentifyCall(CH323NetSetup* pH323NetSetup, DWORD bIsEncrypted,
		int indexTblAuth, BYTE* pSid, BYTE* pAuthKey, BYTE* pEncKey,
		DWORD authLen, DWORD encLen) {
	CSegment* seg = new CSegment;

	pH323NetSetup->Serialize(NATIVE, *seg);

	*seg << (DWORD) bIsEncrypted << (DWORD) indexTblAuth << authLen << encLen;

	seg->Put(pSid, 2);
	if (authLen)
		seg->Put(pAuthKey, authLen);

	if (encLen)
		seg->Put(pEncKey, encLen);

	SendMsg(seg, LOBBYNETIDENT);
}

//--------------------------------------------------------------------------
void CPartyApi::SendSiteAndVisualNamePlusProductIdToPartyControl(
		BYTE bIsVisualName, char* siteName, BYTE bIsProductId, char* productId,
		BYTE bIsVersionId, char* VersionId,
		eTelePresencePartyType eLocalTelePresencePartyType,
		BYTE bIsCopMcu/*=FALSE*/, RemoteIdent eRemoteVendorIdent/*=Regular*/) {
	CSegment* pParam = new CSegment;

	*pParam << bIsCopMcu;
	*pParam << bIsVisualName;

	DWORD len = 0;
	if (siteName[0] != '\0')
		len = strlen(siteName) + 1; // for string used.

	*pParam << len;
	*pParam << bIsProductId;
	DWORD len2 = 0;
	if (productId[0] != '\0')
		len2 = strlen(productId) + 1; // for string used.

	*pParam << len2;
	*pParam << bIsVersionId;
	DWORD len3 = 0;
	if (bIsVersionId)
		len3 = strlen(VersionId) + 1; // for string used.

	*pParam << len3;

	if (len)
		pParam->Put((unsigned char*) siteName, len);

	if (len2)
		pParam->Put((unsigned char*) productId, len2);

	if (len3) {
		pParam->Put((unsigned char*) VersionId, len3);
	}

	PTRACE2INT(eLevelInfoNormal,
			"N.A. DEBUG CPartyApi::SendSiteAndVisualNamePlusProductIdToPartyControl eLocalTelePresencePartyType",
			eLocalTelePresencePartyType);
	*pParam << (BYTE) eLocalTelePresencePartyType;

	// speakerIndication
	*pParam << (BYTE) eRemoteVendorIdent;

	SendLocalMessage(pParam, SET_SITE_AND_VISUAL_NAME);
}

//--------------------------------------------------------------------------
// Pass the remote capabilities to the Party
void CPartyApi::SendRemoteCapabilities(CCapH323& rmtCap) {
	CSegment* seg = new CSegment;

	rmtCap.Serialize(NATIVE, *seg);
	SendLocalMessage(seg, REMOTECAPABILITIESIND);
}

// This API func. used when we get NonStandard message indication from
// H.323 card
//--------------------------------------------------------------------------
void CPartyApi::H323NonStandardInd(CSegment* pMess) {
	CSegment* seg = new CSegment;

	*seg << *pMess;

	SendLocalMessage(seg, H323NONSTANDARDIND);
}

//--------------------------------------------------------------------------
void CPartyApi::H323DBC2Command(CSegment* pMess) {
	CSegment* seg = new CSegment;

	*seg << *pMess;

	SendMsg(seg, H323DBC2COMMAND);
}

//--------------------------------------------------------------------------
void CPartyApi::H323DBC2CommandInd(DWORD opcode, CSegment* pMess) {
	CSegment* seg = new CSegment;

	*seg << *pMess;

	SendMsg(seg, opcode);
}

//--------------------------------------------------------------------------
void CPartyApi::H323RmtCI(CSegment* pMess) {
	CSegment* seg = new CSegment;

	*seg << *pMess;

	SendLocalMessage(seg, H323RMTCI);
}

//--------------------------------------------------------------------------
void CPartyApi::sendPartyDTMFInd(unsigned char* buffer, DWORD length,
		DWORD dtmfOpcode) {
	CSegment* seg = new CSegment;
	*seg << length;
	seg->Put((unsigned char*) buffer, length);
	*seg << dtmfOpcode;

	SendLocalMessage(seg, IP_DTMF_INPUT_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::SendOpcodeToIvrSubFeature(DWORD opcode) {
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) IVR_SUB_FEATURE_MESSG << (DWORD) opcode;

	SendMsg(pSeg, IVR_MESSG);
}

//--------------------------------------------------------------------------
void CPartyApi::SendReceivedDtmfToParty(CSegment* pParam) {
	SendMsg(pParam, AUD_DTMF_IND_VAL);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdateIVRGeneralOpcode(WORD opcode) {
	CSegment* pSeg = new CSegment;

	*pSeg << (DWORD) opcode;

	SendMsg(pSeg, IVR_GENERAL_EVENT);
}

//--------------------------------------------------------------------------
void CPartyApi::SendContentMediaProducerStatusToConfLevel(BYTE channelID,
		BYTE status) {
	CSegment* seg = new CSegment;
	*seg << channelID << status;
	SendLocalMessage(seg, MEDIA_PRODUCER_STATUS);
}

//--------------------------------------------------------------------------
void CPartyApi::SetFaultyResourcesToPartyControlLevel(DWORD reason,
		MipHardWareConn mipHwConn, MipMedia mipMedia, MipDirection mipDirect,
		MipTimerStatus mipTimerStat, MipAction mipAction) {
	CSegment* seg = new CSegment;
	*seg << reason;
	*seg << (BYTE) mipHwConn;
	if (mipHwConn != eMipNoneHw) {
		*seg << (BYTE) mipMedia << (BYTE) mipDirect << (BYTE) mipTimerStat
				<< (BYTE) mipAction;
	}

	SendLocalMessage(seg, PARTY_FAULTY_RSRC);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdateGkCallIdInCdr(BYTE* gkCallId) {
	CSegment* pSeg = new CSegment;
	pSeg->Put((unsigned char*) gkCallId, SIZE_OF_CALL_ID);

	SendLocalMessage(pSeg, UPDATE_GK_CALL_ID);
}

//--------------------------------------------------------------------------
void CPartyApi::SendECSToPartyControl() {
	CSegment* seg = new CSegment;
	SendLocalMessage(seg, PARTY_RECEIVE_ECS);
}

//--------------------------------------------------------------------------
void CPartyApi::SendH323LogicalChannelReject(WORD channelType,
		WORD channelDirection, WORD roleLabel) {
	CSegment* seg = new CSegment;

	*seg << channelType;
	*seg << channelDirection;
	*seg << roleLabel;

	SendLocalMessage(seg, REJECT_NEW_CHANNEL);
}

//--------------------------------------------------------------------------
void CPartyApi::SendIpStreamViolation(WORD cardStatus, BYTE reason,
		CSecondaryParams& secParams) {
	CSegment* seg = new CSegment;
	*seg << cardStatus << (WORD) reason;

	secParams.Serialize(NATIVE, *seg);
	SendLocalMessage(seg, IP_STREAM_VIOLATION);
}

//--------------------------------------------------------------------------
void CPartyApi::SendIpDifferentPayload(WORD channelType, WORD channelDirection,
		WORD payload, WORD roleLabel) {
	CSegment* seg = new CSegment;

	*seg << channelType;
	*seg << channelDirection;
	*seg << payload;
	*seg << roleLabel;

	SendMsg(seg, IP_RTP_DIFF_PAYLOAD_TYPE);
}

//--------------------------------------------------------------------------
void CPartyApi::DowngradeToSecondary(WORD reason) {
	CSegment* seg = new CSegment;
	*seg << reason;
	SendLocalMessage(seg, IPPARTYMSECONDARY);
}

//--------------------------------------------------------------------------
// Multiple links for ITP in cascaded conference feature: CPartyApi::SendNewITPSpeakerInd (like IPPARTYMSECONDARY) //shiraITP - 90
void CPartyApi::SendNewITPSpeakerInd(eTelePresencePartyType itpType,
		DWORD numOfActiveLinks) {
	CSegment* seg = new CSegment;

	*seg << numOfActiveLinks;
	*seg << (BYTE) itpType;

	SendLocalMessage(seg, ITPSPEAKERIND);
}

//--------------------------------------------------------------------------/
// Multiple links for ITP in cascaded conference feature: CPartyApi::SendNewITPSpeakerAckInd //shiraITP - 111
void CPartyApi::SendNewITPSpeakerAckInd() {
	CSegment* seg = new CSegment;

	SendLocalMessage(seg, ITPSPEAKERACKIND);
}

//--------------------------------------------------------------------------
void CPartyApi::SendReCapsToPartyControl(CCapH323& rmtCapH323) {
	CSegment* seg = new CSegment;
	rmtCapH323.Serialize(NATIVE, *seg);
	SendLocalMessage(seg, REMOTE_SENT_RE_CAPS);
}

//--------------------------------------------------------------------------
void CPartyApi::IdentifySip(CSipNetSetup* pNetSetup,
		const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD sdpLen) {
	CSegment* pSeg = new CSegment;

	pNetSetup->Serialize(NATIVE, *pSeg);
	*pSeg << sdpLen;
	pSeg->Put((BYTE*) pSdpAndHeaders, sdpLen);

	SendMsg(pSeg, LOBBYNETIDENT);
}

//--------------------------------------------------------------------------
void CPartyApi::RejectSipCall(CSipNetSetup* pNetSetup, int reason, WORD sdpLen,
		const struct sipSdpAndHeaders* pSdpAndHeaders, char* pAltAddress,
		STATUS status) {
	CSegment* pSeg = new CSegment;
	pNetSetup->Serialize(NATIVE, *pSeg);
	*pSeg << (DWORD) reason;
	*pSeg << status;

	*pSeg << sdpLen;
	pSeg->Put((BYTE*) pSdpAndHeaders, sdpLen);
	if (pAltAddress) {
		*pSeg << (WORD) strlen(pAltAddress) << pAltAddress;
	} else
		*pSeg << (WORD) 0;

	SendMsg(pSeg, REJECTCALL);
}

//--------------------------------------------------------------------------
void CPartyApi::SendSipTransMsg(OPCODE event, CSegment* pParam) {
	CSegment* pSeg = new CSegment;
	// VNGFE-2997
	if (XML_EVENT == event) {
		CSegment* pTmp = new CSegment;
		pTmp->CopySegmentFromReadPosition(*pParam);

		*pSeg << (OPCODE) event;
		*pSeg << *pTmp;

		POBJDELETE(pTmp);
		SendLocalMessage(pSeg, SIP_TRANSACTION_MSG);
	} else {
		*pSeg << (OPCODE) event;
		if (pParam)
			*pSeg << *pParam;

		SendLocalMessage(pSeg, SIP_TRANSACTION_MSG);
	}
}

//--------------------------------------------------------------------------
void CPartyApi::SipConfEstablishCall(CRsrcParams** avcToSvcTranslatorRsrcParams,
		CRsrcParams* pMrmpRsrcParams, CRsrcParams* pMfaRsrcParams,
		CRsrcParams* pCsRsrcParams, UdpAddresses sUdpAddressesParams,
		CSipNetSetup* pNetSetup, CSipCaps* pLocalCaps, CIpComMode* pInitialMode,
		CQoS* pQos, BYTE bIsAdvancedVideoFeatures, const char* strConfParamInfo,
		BYTE eTransportType, WORD McuNumber, WORD TerminalNumber,
		CCopVideoTxModes* pCopVideoTxModes, const char* alternativeAddrStr,
		BYTE bNoVideRsrcForVideoParty, CIpComMode* pTargetModeMaxAllocation,
		CSipCaps* MaxLocalCaps, WORD room_Id,
		CSvcPartyIndParamsWrapper& SSRCIdsForAvcParty, BYTE bIsMrcCall,
		BYTE IsAsSipContentEnable, DWORD partyContentRate)
{
	CSegment* pSeg = new CSegment;

	pNetSetup->Serialize(NATIVE, *pSeg);
	pLocalCaps->Serialize(NATIVE, *pSeg);
	pInitialMode->Serialize(NATIVE, *pSeg);
	pQos->Serialize(NATIVE, *pSeg);
	*pSeg << bIsAdvancedVideoFeatures;

	DWORD len = strConfParamInfo ? strlen(strConfParamInfo) + 1 : 0;
	*pSeg << len;
	if (len) {
		pSeg->Put((BYTE*) strConfParamInfo, len);
	}

	*pSeg << eTransportType;

	pMfaRsrcParams->Serialize(NATIVE, *pSeg);
	pCsRsrcParams->Serialize(NATIVE, *pSeg);

	SerializeNonMandatoryRsrcParams(pSeg, pMrmpRsrcParams);
	SerializeNonMandatoryRsrcParamsArray(pSeg, avcToSvcTranslatorRsrcParams,
			NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS);

	int sizeOfUdps = sizeof(UdpAddresses);
	pSeg->Put((BYTE*) (&sUdpAddressesParams), sizeOfUdps);

	*pSeg << McuNumber;
	*pSeg << TerminalNumber;

	BYTE bIsCopVideoTxModes = pCopVideoTxModes ? TRUE : FALSE;
	*pSeg << bIsCopVideoTxModes;

	if (bIsCopVideoTxModes)
		pCopVideoTxModes->Serialize(NATIVE, *pSeg);

	DWORD addrLen = alternativeAddrStr ? strlen(alternativeAddrStr) + 1 : 0;
	*pSeg << addrLen;
	if (addrLen) {
		pSeg->Put((BYTE*) alternativeAddrStr, addrLen);
	}

	*pSeg << bNoVideRsrcForVideoParty;
	pTargetModeMaxAllocation->Serialize(NATIVE, *pSeg);
	MaxLocalCaps->Serialize(NATIVE, *pSeg);
	*pSeg << room_Id;

	CLargeString cstr;

	if (pInitialMode->GetConfMediaType() == eMixAvcSvc || pInitialMode->GetConfMediaType() == eMixAvcSvcVsw) {
		SSRCIdsForAvcParty.Serialize(*pSeg);

		if (pInitialMode->GetConfMediaType() == eMixAvcSvc) {
			SSRCIdsForAvcParty.Print(
					"CPartyApi::SipConfEstablishCall eMixAvcSvc:");
		} else {
			SSRCIdsForAvcParty.Print(
					"CPartyApi::SipConfEstablishCall avc_vsw_relay:");
		}
	}

	*pSeg << bIsMrcCall;
	*pSeg << IsAsSipContentEnable;
    *pSeg << partyContentRate;
	SendMsg(pSeg, SIP_CONF_ESTABLISH_CALL);
}

//--------------------------------------------------------------------------
void CPartyApi::SipConfConnectCall() {
	CSegment* pSeg = new CSegment;
	SendMsg(pSeg, SIP_CONF_CONNECT_CALL);
}

//--------------------------------------------------------------------------
void CPartyApi::SipConfDisconnectChannels(CSipComMode* pNewTargetMode) {
	CSegment* pSeg = new CSegment;

	pNewTargetMode->Serialize(NATIVE, *pSeg);
	SendMsg(pSeg, SIP_CONF_DISCONNECT_CHANNELS);
}

//--------------------------------------------------------------------------
void CPartyApi::ChangeModeIp(CIpComMode* pIpComMode, BYTE changeModeState,
		BYTE bIsContentSpeaker, CSipCaps* pLocalCapSip,
		CRsrcParams** avcToSvcTranslatorRsrcParams,
		CRsrcParams* pMrmpRsrcParams, BYTE IsASSIPContentEnable) {
	CSegment* pSeg = new CSegment;

	BYTE bIsIpComMode = pIpComMode ? TRUE : FALSE;

	*pSeg << changeModeState;
	*pSeg << bIsIpComMode;

	if (bIsIpComMode)
		pIpComMode->Serialize(NATIVE, *pSeg);

	*pSeg << bIsContentSpeaker;

	BYTE bIsLocalCaps = pLocalCapSip ? TRUE : FALSE;

	*pSeg << bIsLocalCaps;

	if (bIsLocalCaps)
		pLocalCapSip->Serialize(NATIVE, *pSeg);

	*pSeg << IsASSIPContentEnable;

	SerializeNonMandatoryRsrcParams(pSeg, pMrmpRsrcParams);
	SerializeNonMandatoryRsrcParamsArray(pSeg, avcToSvcTranslatorRsrcParams,
			NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS);

	SendMsg(pSeg, CONFCHANGEMODE);
}

//--------------------------------------------------------------------------
void CPartyApi::BridgesUpdated(CIpComMode* pIpComMode, UdpAddresses sUdpAddressesParams, DWORD status, BYTE bIsContentSpeaker,BYTE bUpdateMixModeResources, CRsrcParams** avcToSvcTranslatorRsrcParams,CRsrcParams* pMrmpRsrcParams, BYTE bShouldPartyRemoveContent /*= FALSE*/)
{
	CSegment* pSeg = new CSegment;

	// Adds status param for inform the party if change mode is success or not
	*pSeg << status;

	if (status == STATUS_OK) {
		pIpComMode->Serialize(NATIVE, *pSeg);
	}

	int sizeOfUdps = sizeof(UdpAddresses);
	pSeg->Put((BYTE*) (&sUdpAddressesParams), sizeOfUdps);

	*pSeg << bIsContentSpeaker;
	*pSeg << bShouldPartyRemoveContent;

	*pSeg << bUpdateMixModeResources;
	if (bUpdateMixModeResources)
	{
	SerializeNonMandatoryRsrcParams(pSeg, pMrmpRsrcParams);
	SerializeNonMandatoryRsrcParamsArray(pSeg, avcToSvcTranslatorRsrcParams,
			NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS);
	}
	SendMsg(pSeg, SIP_CONF_BRIDGES_UPDATED);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyCallClosed(CSipComMode* pCurrentMode) {
	CSegment* pSeg = new CSegment;
	pCurrentMode->Serialize(NATIVE, *pSeg);
	SendLocalMessage(pSeg, SIP_PARTY_CALL_CLOSED);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyRemoteCloseCall(DWORD reason) {
	CSegment* pSeg = new CSegment;
	*pSeg << reason;
	SendLocalMessage(pSeg, SIP_PARTY_RMT_CLOSE_CALL);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyCallFailed(DWORD reason, DWORD MipErrorNumber) {
	CSegment* pSeg = new CSegment;
	*pSeg << reason;
	if (MipErrorNumber)
		*pSeg << MipErrorNumber;
	SendMsg(pSeg, SIP_PARTY_CALL_FAILED);
	//	SendLocalMessage(pSeg, SIP_PARTY_CALL_FAILED);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyCallReinvite(DWORD reason) {
	CSegment* pSeg = new CSegment;
	*pSeg << reason;
	SendLocalMessage(pSeg, SIP_PARTY_CALL_REINVITE);
}

//--------------------------------------------------------------------------
void CPartyApi::SipTransportError(DWORD expectedReq) {
	CSegment* pSeg = new CSegment;
	*pSeg << expectedReq;
	SendLocalMessage(pSeg, SIP_PARTY_TRANSPORT_ERROR);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyChannelsConnected(CSipComMode* pCurrentMode) {
	CSegment* pSeg = new CSegment;
	pCurrentMode->Serialize(NATIVE, *pSeg);

	SendLocalMessage(pSeg, SIP_PARTY_CHANS_CONNECTED);
}

////////////////////////////////////////////////////////////////////////////
//CDR_MCCF:
void CPartyApi::SipPartyStatisticsInfo() {
	CSegment* pSeg = new CSegment;
	SendLocalMessage(pSeg, SIP_PARTY_STATISTIC_INFO);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyVideoBitRate(DWORD rate, WORD direction, WORD role) {
	CSegment* pSeg = new CSegment;
	*pSeg << rate;
	*pSeg << direction;
	*pSeg << role;
	SendMsg(pSeg, SIP_RATE_CHANGE_BY_REMOTE);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyChannelsUpdated(CSipComMode* pCurrentMode) {
	CSegment* pSeg = new CSegment;
	pCurrentMode->Serialize(NATIVE, *pSeg);
	SendLocalMessage(pSeg, SIP_PARTY_CHANS_UPDATED);
}

//--------------------------------------------------------------------------
void CPartyApi::H323PartyChannelsUpdated(CComModeH323* pCurrentMode) {
	CSegment* pSeg = new CSegment;
	pCurrentMode->Serialize(NATIVE, *pSeg);
	SendLocalMessage(pSeg, H323_PARTY_CHANS_UPDATED);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyChannelsDisconnected(CSipComMode* pCurrentMode) {
	CSegment* pSeg = new CSegment;
	pCurrentMode->Serialize(NATIVE, *pSeg);
	SendLocalMessage(pSeg, SIP_PARTY_CHANS_DISCONNECTED);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyScpRequestFromEP(CSegment* pParam) {
	CSegment* seg = new CSegment(*pParam);
	SendLocalMessage(seg, SIP_PARTY_SCP_REQUEST_FROM_EP);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyScpNotificationIndFromEP(CSegment* pParam) {
	CSegment* seg = new CSegment(*pParam);
	SendLocalMessage(seg, SIP_PARTY_SCP_NOTIFICATION_IND_FROM_EP);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyReceived200Ok(BYTE bRemovedAudio, BYTE bRemovedVideo,
		BYTE isUpdateAnatIpType)  //add param for ANAT
		{
	CSegment* pSeg = new CSegment;
	*pSeg << bRemovedAudio;
	*pSeg << bRemovedVideo;
	*pSeg << isUpdateAnatIpType;
	SendLocalMessage(pSeg, SIP_PARTY_RECEIVED_200OK);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyReInviteResponse(DWORD status, BYTE bRemovedAudio,
		BYTE bRemovedVideo, CSipChanDifArr* pChanDifArr) {
	CSegment* pSeg = new CSegment;
	*pSeg << status;
	*pSeg << bRemovedAudio;
	*pSeg << bRemovedVideo;

	pChanDifArr->Serialize(NATIVE, *pSeg);

	SendLocalMessage(pSeg, SIP_PARTY_REINVITE_RESPONSE);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyOriginalRemoteCaps(CSipCaps* pRemoteCaps) {
	CSegment* pSeg = new CSegment;
	pRemoteCaps->Serialize(NATIVE, *pSeg);
	SendLocalMessage(pSeg, SIP_PARTY_ORIGINAL_RMOTCAP);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyReceivedAck(DWORD status, DWORD isSdp,
		BYTE bRemovedAudio, BYTE bRemovedVideo, CSipChanDifArr* pChanDifArr) {
	CSegment* pSeg = new CSegment;
	*pSeg << status;
	*pSeg << isSdp;
	*pSeg << bRemovedAudio;
	*pSeg << bRemovedVideo;
	if (pChanDifArr) {
		pChanDifArr->Serialize(NATIVE, *pSeg);
	}

	SendMsg(pSeg, SIP_PARTY_RECEIVED_ACK);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyUnMuteClosingChannel(DWORD chanType) {
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) chanType;
	SendMsg(pSeg, SIP_PARTY_UNMUTE_CHANNEL);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyBadStatus(DWORD opcode, int len, BYTE* strDescription) {
	CSegment* pSeg = new CSegment;
	*pSeg << opcode << (DWORD) len;
	pSeg->Put(strDescription, len);
	SendLocalMessage(pSeg, SIP_PARTY_BAD_STATUS);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyReceivedReInvite(CSipChanDifArr* pChanDifArr,
		BYTE isReInviteWithSdp, BYTE isRejectReInvite) {
	CSegment* pSeg = new CSegment;
	*pSeg << isReInviteWithSdp;
	*pSeg << isRejectReInvite;
	pChanDifArr->Serialize(NATIVE, *pSeg);
	SendMsg(pSeg, SIP_PARTY_RECEIVED_REINVITE);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyConnectTout() {
	CSegment* pSeg = new CSegment;
	SendLocalMessage(pSeg, PARTYCONNECTTOUT);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyDisconnectTout() {
	CSegment* pSeg = new CSegment;
	SendLocalMessage(pSeg, PARTYDISCONNECTTOUT);
}

//--------------------------------------------------------------------------
void CPartyApi::SetCapsValuesAccordingToNewAllocation(
		H264VideoModeDetails h264VidModeDetails,
		MsSvcVideoModeDetails msSvcDetails, BYTE cif4Mpi, BYTE bIsAudioOnly,
		BYTE bIsRtv, BYTE bIsMsSvc, DWORD videoRate /*=0*/) {
	CSegment* pSeg = new CSegment;
	WORD len = 0;

	TRACEINTO << "videoRate: " << videoRate;

	if (bIsMsSvc == FALSE)
		len = sizeof(H264VideoModeDetails);
	else
		len = sizeof(MsSvcVideoModeDetails);
	*pSeg << bIsAudioOnly;
	*pSeg << bIsRtv;
	*pSeg << bIsMsSvc;
	*pSeg << videoRate;
	*pSeg << cif4Mpi;
	*pSeg << len;
	if(bIsMsSvc == FALSE)
		pSeg->Put((BYTE*)&h264VidModeDetails, sizeof(H264VideoModeDetails));
	else
		pSeg->Put((BYTE*)&msSvcDetails, sizeof(MsSvcVideoModeDetails));

	SendMsg(pSeg, SET_CAPS_ACCORDING_TO_NEW_ALLOCATION);
}

//--------------------------------------------------------------------------
void CPartyApi::SendInfoToRss(BYTE isStreaming, char* pExchangeConfId) {
	CSegment* pSeg = new CSegment;
	*pSeg << isStreaming;
	WORD strLen = 0;
	if (pExchangeConfId)
		strLen = strlen(pExchangeConfId);

	*pSeg << strLen;
	if (strLen) {
		pSeg->Put((BYTE*) pExchangeConfId, strLen);
	}

	SendMsg(pSeg, SEND_INFO_TO_RSS);
}

//--------------------------------------------------------------------------
// API FROM CONF3CTL TO H323PART //
void CPartyApi::ReConnectStream(WORD dataType) {
	CSegment* seg = new CSegment;

	*seg << dataType;

	SendMsg(seg, RECONNECT_STREAM);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyDnsResAck(DWORD status) {
	CSegment* pSeg = new CSegment;
	*pSeg << status;

	SendMsg(pSeg, SIP_PARTY_DNS_RES);
}

//--------------------------------------------------------------------------
void CPartyApi::ChangeMode(CComMode* pComMode) {
	CSegment* seg = new CSegment;
	pComMode->Serialize(NATIVE, *seg);

	SendMsg(seg, CONFCHANGEMODE);
}

//--------------------------------------------------------------------------
void CPartyApi::ExchangeCap(CCapH320* pNewLocalCap) {
	CSegment* seg = new CSegment;
	pNewLocalCap->Serialize(NATIVE, *seg);

	SendMsg(seg, CONFEXNGCAP);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdateLocalCaps(CCapH320* pNewLocalCap) {
	CSegment* seg = new CSegment;
	pNewLocalCap->Serialize(NATIVE, *seg);

	SendMsg(seg, CONFUPDATELOCALCAPS);
}

//--------------------------------------------------------------------------
void CPartyApi::PartyVideoFreeze() {
	CSegment* seg = new CSegment;
	SendMsg(seg, FREEZPIC);
}

//--------------------------------------------------------------------------
void CPartyApi::IdentifyNetChannel(CIsdnNetSetup* pNetSetUp) {
	CSegment* pSeg = new CSegment;

	pNetSetUp->Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, LOBBYNETIDENT);
}

//--------------------------------------------------------------------------
void CPartyApi::EncryptionDisConnect(WORD cause) {
	CSegment* seg = new CSegment;

	*seg << (WORD) cause;

	SendLocalMessage(seg, DISCONNECTENC); // olga
}

//--------------------------------------------------------------------------
void CPartyApi::MuxEndH320Connect(CCapH320& rmtCap, CComMode currentScm,
		WORD status, BYTE isEncryptionSetupDone) {
	CSegment* seg = new CSegment;

	rmtCap.Serialize(NATIVE, *seg);
	currentScm.Serialize(NATIVE, *seg);
	*seg << status;
	*seg << isEncryptionSetupDone;
	SendLocalMessage(seg, ENDH221CON); // olga
}

//--------------------------------------------------------------------------
void CPartyApi::MuXEndSetXmitComMode(WORD status) {
	CSegment* seg = new CSegment;
	*seg << status;
	SendLocalMessage(seg, ENDCHAGEMODE); // olga
}

//--------------------------------------------------------------------------
void CPartyApi::TimeOutMuxEndH320Connect(CCapH320& rmtCap, CComMode& currentScm,
		WORD reasonTimeOut) {
	CSegment* seg = new CSegment;

	rmtCap.Serialize(NATIVE, *seg);
	currentScm.Serialize(NATIVE, *seg);
	*seg << (WORD) statTout;
	*seg << reasonTimeOut;
	SendLocalMessage(seg, ENDH221CON); // olga
}

//--------------------------------------------------------------------------
void CPartyApi::MuxRmtCap(CCapH320& rmtCap, WORD status) {
	CSegment* seg = new CSegment;

	*seg << status;

	rmtCap.Serialize(NATIVE, *seg);
	SendLocalMessage(seg, RMTCAP);
}

//--------------------------------------------------------------------------
void CPartyApi::MuxRmtH230(CSegment* h221Str) {
	CSegment* seg = new CSegment;

	*seg << *h221Str;

// SendLocalMessage(seg, RMTH230);//olga
	SendMsg(seg, RMTH230);
}

//--------------------------------------------------------------------------
void CPartyApi::MuxSync(WORD localRemote, WORD lostRegain, WORD chnlNum) {
	CSegment* seg = new CSegment;
	*seg << localRemote << lostRegain << chnlNum;

	SendLocalMessage(seg, SYNCLOSS);
}

//--------------------------------------------------------------------------
void CPartyApi::MuxSyncTimer() {
	CSegment* seg = new CSegment;
	SendLocalMessage(seg, SYNCTIMER);
}

//--------------------------------------------------------------------------
void CPartyApi::AudioValidation(WORD onOff) {
	CSegment* seg = new CSegment;

	*seg << onOff;

	SendMsg(seg, AUDVALID);
}

//--------------------------------------------------------------------------
void CPartyApi::SendMMStoParty(WORD onOff) {
	CSegment* seg = new CSegment;

	*seg << onOff;

	SendMsg(seg, SEND_MMS);
}

//--------------------------------------------------------------------------
void CPartyApi::SendMCCtoParty(WORD onOff) {
	CSegment* seg = new CSegment;

	*seg << onOff;

	SendMsg(seg, SEND_MCC);
}

//--------------------------------------------------------------------------
void CPartyApi::MuxRmtXfrMode(CComMode& rmtxfr) {
	// this api function is used to update the conf ongoing DB
	// with a new remote communication mode
	CSegment* seg = new CSegment;

	rmtxfr.Serialize(NATIVE, *seg);

	SendLocalMessage(seg, RMTXFRMODE); // olga
}

//--------------------------------------------------------------------------
// Eitan - new API MUXCNTL --> PARTY indicate to conf level that remote sent all its caps
void CPartyApi::MuxReceivedPartyFullCapSet(CCapH320& rmtCap,
		CComMode& currentRmtScm) {
	CSegment* seg = new CSegment;

	rmtCap.Serialize(NATIVE, *seg);
	currentRmtScm.Serialize(NATIVE, *seg);
	SendLocalMessage(seg, ALLRMTCAPSRECEIVED);
}

//--------------------------------------------------------------------------
void CPartyApi::MuxSyncInitChnl(CCapH320* pRmtCap) {
	CSegment* seg = new CSegment;
	pRmtCap->Serialize(NATIVE, *seg);

	SendLocalMessage(seg, MSYNCFOUND); // olga
}

//--------------------------------------------------------------------------
void CPartyApi::StartH230CommandSeq() {
	CSegment* seg = new CSegment;

	SendMsg(seg, STARTCOMMANDH230SEQ);
}

//--------------------------------------------------------------------------
void CPartyApi::AudioActive(WORD onOff, WORD srcRequest, DWORD mediaMask) {
	CSegment* seg = new CSegment;

	*seg << srcRequest << onOff;
	SendMsg(seg, MEDIAERR); // olga
}

//--------------------------------------------------------------------------
// the function used to add another protocol to the Local Capability Set
void CPartyApi::SendAddedProtocolToConfLevel(WORD NoAddedProtocols,
		WORD ProtocolCapEnum) {
	CSegment* seg = new CSegment;
	*seg << NoAddedProtocols << ProtocolCapEnum;

	SendMsg(seg, ADDPROTOCOL);
}

//--------------------------------------------------------------------------
void CPartyApi::SendContentMessage(OPCODE subOpcode, BYTE mcuNum,
		BYTE terminalNum, BYTE randomNum, const BYTE isSpeakerChange) {
	CSegment* seg = new CSegment;

	*seg << (OPCODE) subOpcode;

	if (subOpcode == CONTENT_ROLE_TOKEN_WITHDRAW)
		*seg << isSpeakerChange; // Now actual usage for H323 parties only (EPC and Duo)

	*seg << mcuNum << terminalNum << randomNum;

	SendMsg(seg, CONFCONTENTTOKENMSG);
}

//--------------------------------------------------------------------------
void CPartyApi::SendContentTokenMediaProducerStatus(const BYTE channelId,
		const BYTE status) {
	CSegment* seg = new CSegment;

	*seg << (OPCODE) CONTENT_MEDIA_PRODUCER_STATUS << channelId << status;

	SendMsg(seg, CONFCONTENTTOKENMSG);
}

//--------------------------------------------------------------------------
void CPartyApi::SendH239FlowControlReleaseRes(const BYTE isAck,
		const WORD bitRate) {
	CSegment* seg = new CSegment;

	*seg << (BYTE) isAck << (WORD) bitRate;

	SendMsg(seg, FLOW_CONTROL_RELEASE_RES);
}
//--------------------------------------------------------------------------
void CPartyApi::UpdateNoResourcesForVideoParty(BYTE bNoResourcesForVideoParty) {
	CSegment* seg = new CSegment;

	*seg << (BYTE) bNoResourcesForVideoParty;

	SendMsg(seg, UPDATE_NO_RESOURCES_FOR_VIDEO_PARTY);
}

//--------------------------------------------------------------------------
void CPartyApi::SendContentTokenRoleProviderIdentity(const BYTE mcuNum,
		const BYTE terminalNum, const BYTE label, const BYTE dataSize,
		const BYTE* pData) {
	CSegment* pSeg = new CSegment;

	*pSeg << (OPCODE) CONTENT_ROLE_PROVIDER_IDENTITY << mcuNum << terminalNum
			<< label << (BYTE) dataSize; // size = msgLen of pParam - all obtained fields

	for (WORD i = 0; i < dataSize; i++)
		*pSeg << pData[i];

	SendMsg(pSeg, CONFCONTENTTOKENMSG);
}

//--------------------------------------------------------------------------
void CPartyApi::SendContentTokenNoRoleProvider(const BYTE mcuNum,
		const BYTE terminalNum) {
	CSegment* pSeg = new CSegment;

	*pSeg << (OPCODE) CONTENT_NO_ROLE_PROVIDER << mcuNum << terminalNum;

	SendMsg(pSeg, CONFCONTENTTOKENMSG);
}

//--------------------------------------------------------------------------
void CPartyApi::SendH239ContentVideoMode(CContentMode* pContentMode) {
	CSegment* seg = new CSegment;

	pContentMode->Serialize(NATIVE, *seg);

	SendMsg(seg, AMC_CONTENT_VIDEO_MODE);
}

//--------------------------------------------------------------------------
void CPartyApi::SendH239LogicalChannelInactive(BYTE controlID) {
	CSegment* seg = new CSegment;

	*seg << (BYTE) controlID;

	SendMsg(seg, AMC_LOGICAL_CHANNEL_INACTIVE);
}

//--------------------------------------------------------------------------
void CPartyApi::SendH239MCS(BYTE controlID) {
	CSegment* seg = new CSegment;

	*seg << (BYTE) controlID;

	SendMsg(seg, AMC_MCS);
}

//--------------------------------------------------------------------------
void CPartyApi::SendPresentationRateChange(DWORD newRate, BYTE bIsSpeaker) {
	CSegment* seg = new CSegment;

	*seg << (OPCODE) CONTENT_RATE_CHANGE << newRate << bIsSpeaker;

	SendMsg(seg, CONTENTRATECHANGE);
}

//--------------------------------------------------------------------------
void CPartyApi::SendEPCContentFreezePicture() {
	SendOpcodeMsg(CONTENTFREEZEPIC);
}

//--------------------------------------------------------------------------
void CPartyApi::SendContentVideoRefresh(WORD ignore_filtering) {
	CSegment* seg = new CSegment;

	*seg << ignore_filtering;

	SendMsg(seg, CONTENTVIDREFRESH);
	// SendOpcodeMsg(CONTENTVIDREFRESH);
}

//--------------------------------------------------------------------------
void CPartyApi::SendContentRateChangeDone() {
	SendOpcodeMsg(CONTENTRATECHANGEDONE);
}

// API for content token sent to conf level
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void CPartyApi::SendTokenMessageToConfLevel(DWORD subOpcode, BYTE MCUNumber,
		BYTE terminalNumber, BYTE label, BYTE randomNumber, DWORD size,
		CSegment* pParam) {
	CSegment* seg = new CSegment;
	*seg << (DWORD) subOpcode << (BYTE) MCUNumber << (BYTE) terminalNumber
			<< (BYTE) label << (BYTE) randomNumber << (DWORD) size;

	if (size)
		*seg << (*pParam);

	SendLocalMessage(seg, PARTY_TOKEN_MESSAGE);
}

//--------------------------------------------------------------------------
void CPartyApi::SendTokenMessageToCallGenerator(DWORD subOpcode, BYTE isAck) {
	if (CProcessBase::GetProcess()->GetProductFamily()
			!= eProductFamilyCallGenerator) {
		TRACEINTO << "Failed, system is not Call Generator";
		return;
	}

	CSegment* seg = new CSegment;
	*seg << (DWORD) subOpcode << (BYTE) isAck;

	SendMsg(seg, CG_PARTY_TOKEN_MESSAGE);
}

//--------------------------------------------------------------------------
void CPartyApi::SendEndChangeContentToConfLevel(const CComModeH323* pTmpMode,
		int status) {
	CSegment* seg = new CSegment;
	*seg << (DWORD) status;

	pTmpMode->Serialize(NATIVE, *seg);
	SendLocalMessage(seg, PARTYENDCHANGEMODE);
}

//--------------------------------------------------------------------------
void CPartyApi::SendContentFlowControlToConfLevel(DWORD newContentRate, DWORD isDba)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)newContentRate;
	*seg << (DWORD)isDba;

	SendLocalMessage(seg, UPDATE_FLOW_CONTROL_RATE);
}

//--------------------------------------------------------------------------
void CPartyApi::SendContentOnOffAck(DWORD status,
		eContentState eTempContentInState) {
	CSegment* seg = new CSegment;
	*seg << status << (DWORD) eTempContentInState;

	SendLocalMessage(seg, HW_CONTENT_ON_OFF_ACK);
}

//--------------------------------------------------------------------------
void CPartyApi::SendContentEvacuateAck(DWORD status) {
	CSegment* seg = new CSegment;
	*seg << status;

	SendLocalMessage(seg, RTP_EVACUATE_ON_OFF_ACK);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePresentationOutStream() {
	SendOpcodeMsg(UPDATE_PRESENTATION_OUT_STREAM);
}

//--------------------------------------------------------------------------
void CPartyApi::MfaUpdatedPresentationOutStream() {
	SendOpcodeMsg(PRESENTATION_OUT_STREAM_UPDATED);
}

//--------------------------------------------------------------------------
// API for call generator
void CPartyApi::CGSendContent() {
	if (CProcessBase::GetProcess()->GetProductFamily()
			!= eProductFamilyCallGenerator) {
		TRACEINTO << "Failed, system is not Call Generator";
		return;
	}

	SendOpcodeMsg(PARTY_CG_START_CONTENT);
}

//--------------------------------------------------------------------------
void CPartyApi::CGStopContent() {
	if (CProcessBase::GetProcess()->GetProductFamily()
			!= eProductFamilyCallGenerator) {
		TRACEINTO << "Failed, system is not Call Generator";
		return;
	}

	SendOpcodeMsg(PARTY_CG_STOP_CONTENT);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyCapabilitiesAndAudioRate(BYTE audioRate,
		CCapH323* pLocalCapH323) {
	CSegment* seg = new CSegment;

	*seg << audioRate;

	BYTE bSendCaps = FALSE;

	if (pLocalCapH323)
		bSendCaps = TRUE;

	*seg << bSendCaps;

	if (bSendCaps)
		pLocalCapH323->Serialize(NATIVE, *seg);

	SendMsg(seg, CONF_UPDATE_CAPS_AND_AUDIORATE);
}

//--------------------------------------------------------------------------
void CPartyApi::StopPartyIVR(DWORD restartIVR) {
	CSegment* seg = new CSegment;

	*seg << restartIVR;

	SendMsg(seg, IVR_PARTY_STOP_IVR);
}

//--------------------------------------------------------------------------
void CPartyApi::BondingEndNegotiation(BYTE need_reallocate,
		DWORD number_of_channels_to_reallocate) {
	CSegment* seg = new CSegment;

	*seg << need_reallocate << number_of_channels_to_reallocate;

	SendMsg(seg, BND_END_NEGOTIATION);
}

//--------------------------------------------------------------------------
void CPartyApi::BondAligned() {
	SendMsg(NULL, BND_REMOTE_LOCAL_ALIGNMENT);
}

//--------------------------------------------------------------------------
void CPartyApi::BondingFailed(DWORD cause) {
	CSegment* seg = new CSegment;

	*seg << cause;
	SendMsg(seg, BONDDISCONNECT);
}

//--------------------------------------------------------------------------
void CPartyApi::OnRsrcAllocatorReAllocateRTMAck(
		CIsdnPartyRsrcDesc* pPartyRsrcDesc, BYTE bAllocationFailed) {
	CSegment* seg = new CSegment;

	pPartyRsrcDesc->Serialize(NATIVE, *seg);

	*seg << bAllocationFailed;

	SendMsg(seg, REALLOCATERTM_ACK);
}

//--------------------------------------------------------------------------
void CPartyApi::OnRsrcAllocatorUpdateRTMChannelAck(DWORD monitor_conf_id,
		DWORD monitor_party_id, DWORD connection_id, DWORD status,
		DWORD channelNum, CIsdnNetSetup* pNetSetup) {
	CSegment* pSeg = new CSegment;
	*pSeg << monitor_conf_id << monitor_party_id << connection_id << status
			<< channelNum;
	pNetSetup->Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, UPDATECHANNELACK);
}

//--------------------------------------------------------------------------
void CPartyApi::OnRsrcAllocatorBoardFullAck(CIsdnPartyRsrcDesc* pPartyRsrcDesc,
		BYTE bAllocationFailed) {
	CSegment* seg = new CSegment;

	pPartyRsrcDesc->Serialize(NATIVE, *seg);

	*seg << bAllocationFailed;

	SendMsg(seg, BOARDFULL_ACK);
}

//--------------------------------------------------------------------------
void CPartyApi::NotifyConfOnMasterActionsRegardingContent(DWORD subOpcode,
		DWORD rate) {
	CSegment* seg = new CSegment;
	*seg << (DWORD) subOpcode << (DWORD) rate;

	SendLocalMessage(seg, CONTENT_MESSAGE_FROM_MASTER);
}

//--------------------------------------------------------------------------
void CPartyApi::OnSmartRecovery() {
	SendMsg(NULL, SMART_RECOVERY);
}

//--------------------------------------------------------------------------
void CPartyApi::VideoReadyForSlide() {
	SendMsg(NULL, VIDEO_READY_FOR_IVR);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePeopleLprRate(DWORD newPeopleRate, cmCapDirection channelDirection
                                    , DWORD lossProtection, DWORD mtbf, DWORD congestionCeiling, DWORD fill, DWORD modeTimeout, DWORD totalVideoRate, DWORD bLprOnContent, DWORD newContentRate)
{
	CSegment* seg = new CSegment;

	*seg << newPeopleRate
	     << (WORD)channelDirection
	     << lossProtection
	     << mtbf
	     << congestionCeiling
	     << fill
	     << modeTimeout
	     << totalVideoRate
	     << bLprOnContent
	     << newContentRate;


	SendMsg(seg, LPR_CHANGE_RATE);
}

//--------------------------------------------------------------------------
void CPartyApi::SetLprModeForVideoOutChannels(WORD status, DWORD lossProtection,
		DWORD mtbf, DWORD congestionCeiling, DWORD fill, DWORD modeTimeout) {
	CSegment* seg = new CSegment;

	*seg << status << lossProtection << mtbf << congestionCeiling << fill
			<< modeTimeout;

	SendMsg(seg, LPR_VIDEO_RATE_UPDATED);
}

//--------------------------------------------------------------------------
void CPartyApi::VideoOutChannelsUpdatedForFecOrRed(WORD status, DWORD type) {
	CSegment* seg = new CSegment;
	*seg << (WORD) status << (DWORD) type;

	SendMsg(seg, FEC_RED_VIDEO_RATE_UPDATED);
}

//--------------------------------------------------------------------------
void CPartyApi::SendFlowControlAndLprToCard(DWORD newVidRate, BYTE outChannel,
		lPRModeChangeParams* pLprChangeModeParams) {
	CSegment* seg = new CSegment;

	*seg << newVidRate << outChannel << pLprChangeModeParams->lossProtection
			<< pLprChangeModeParams->mtbf
			<< pLprChangeModeParams->congestionCeiling
			<< pLprChangeModeParams->fill << pLprChangeModeParams->modeTimeout;

	SendMsg(seg, LPR_PARTY_FLOWCONTROL);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdateLprDB(WORD updType, DWORD param, WORD externFlag) {
	CSegment* seg = new CSegment;

	*seg << (WORD) updType << (DWORD) param << (WORD) externFlag;

	SendLocalMessage(seg, UPDATELPRACTIVATION);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdateChannelLprHeaderDB(BYTE ChannelWithLprHeader) {
	CSegment* seg = new CSegment;

	*seg << (BYTE) ChannelWithLprHeader << (WORD) 1;

	SendLocalMessage(seg, UPDATE_LPR_CHANNEL_HEADER);
}

//--------------------------------------------------------------------------
void CPartyApi::SetDTMFForwarding(BYTE bForwardDtmfs, WORD opcode) {
	CSegment* seg = new CSegment;

	*seg << (BYTE) bForwardDtmfs;

	SendMsg(seg, opcode);
}

//--------------------------------------------------------------------------
void CPartyApi::SendDTMFForwarding(const char* dtmfString, WORD opcode) {
	CSegment* pSeg = new CSegment;
	DWORD dtmfStrLen = strlen(dtmfString);
	*pSeg << dtmfStrLen;
	if (dtmfStrLen) {
		pSeg->Put((BYTE*) dtmfString, dtmfStrLen);
	}

	SendMsg(pSeg, opcode);
}

//--------------------------------------------------------------------------
void CPartyApi::SendPartyStartPreviewReq(CPartyPreviewDrv* pPartyPreviewDrv) {
	COstrStream str;
	pPartyPreviewDrv->Serialize(NATIVE, str);

	CSegment* seg = new CSegment;
	*seg << str.str().c_str();
	SendMsg(seg, START_PREVIEW_PARTY);
}

//--------------------------------------------------------------------------
void CPartyApi::SendPartyStopPreviewReq(WORD Direction) {
	CSegment* seg = new CSegment;

	*seg << Direction;

	SendMsg(seg, STOP_PREVIEW_PARTY);
}

//--------------------------------------------------------------------------
void CPartyApi::SendPartyIntraPreviewReq(WORD Direction) {
	CSegment* seg = new CSegment;

	*seg << Direction;

	SendMsg(seg, REQUEST_PREVIEW_INTRA);
}

//--------------------------------------------------------------------------
void CPartyApi::PartyConnectedToPCM() {
	SendOpcodeMsg(PCM_CONNECTED);
}

//--------------------------------------------------------------------------
void CPartyApi::PartyDisconnectedFromPCM() {
	SendOpcodeMsg(PCM_DISCONNECTED);
}

//--------------------------------------------------------------------------
void CPartyApi::ForwardDtmfForPCM(char* dtmf) {
	CSegment* pSeg = new CSegment;

	DWORD dtmfStrLen = strlen(dtmf);
	*pSeg << dtmfStrLen;
	if (dtmfStrLen) {
		pSeg->Put((BYTE*) dtmf, dtmfStrLen);
	}

	SendMsg(pSeg, DTMF_IND_PCM);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyOnMakeAnswerInd(WORD status, int newRate) {
	CSegment* pSeg = new CSegment;

	*pSeg << status;
	*pSeg << (DWORD) newRate;

	SendMsg(pSeg, MAKE_ANSWER_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyOnMakeOfferInd(WORD status) {
	CSegment* pSeg = new CSegment;

	*pSeg << status;
	SendMsg(pSeg, MAKE_OFFER_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyOnReinviteDataInd(DWORD status) {
	CSegment* pSeg = new CSegment;

	*pSeg << status;
	SendMsg(pSeg, ICE_REINVITE_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyOnProcessAnswerInd(DWORD status) {
	CSegment* pSeg = new CSegment;

	*pSeg << status;
	SendMsg(pSeg, ICE_PROCESS_ANS_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyOnModifyOfferInd(DWORD status) {
	CSegment* pSeg = new CSegment;

	*pSeg << status;
	SendMsg(pSeg, ICE_MODIFY_OFFER_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyOnModifyAnswerInd(DWORD status, int newRate) {
	CSegment* pSeg = new CSegment;

	*pSeg << status;
	*pSeg << (DWORD) newRate;

	SendMsg(pSeg, ICE_MODIFY_ANSWER_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyOnCloseIceSession(DWORD status) {
	CSegment* pSeg = new CSegment;

	*pSeg << status;
	SendMsg(pSeg, CLOSE_ICE_SESSION_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdateTransIceConnectivityCheckComplete(CSegment* pParam) {
	CSegment* pSeg = new CSegment(*pParam);
	SendMsg(pSeg, TRANS_ICE_CONN_CHECK_COMPLETE_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyIceConnectivityCheckComplete(CSegment* pParam) {
	CSegment* pSeg = new CSegment(*pParam);
	SendMsg(pSeg, ICE_CONN_CHECK_COMPLETE_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyOnVideoPreference(DWORD Width, DWORD Height,
		DWORD BitRate, DWORD FrameRate) {
	CSegment* pSeg = new CSegment;
	*pSeg << Width << Height << BitRate << FrameRate;

	SendMsg(pSeg, SIP_PARTY_UPDATE_VIDEO_PREFERENCE);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdatePartyOnIceBandwidthEventInd(DWORD videoRate) {
	CSegment* pSeg = new CSegment;

	*pSeg << videoRate;
	SendMsg(pSeg, ICE_BANDWIDTH_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::ReqConfNIDConfirmationSIP(const char* numericConfId) {
	CSegment* pSeg = new CSegment;

	WORD strLen = 0;
	if (numericConfId)
		strLen = strlen(numericConfId) + 1;

	*pSeg << strLen;

	if (strLen != 0) {
		pSeg->Put((BYTE*) numericConfId, strLen);
	}

	SendMsg(pSeg, SIP_CONF_NID_CONFIRM_REQ);
}

//--------------------------------------------------------------------------
void CPartyApi::SendBFCPMessageInd(APIS32 opCode, APIU32 Status, BYTE mcuId,
		BYTE terminalId) {
	CSegment* pSeg = new CSegment;

	*pSeg << (DWORD) opCode;
	*pSeg << (DWORD) Status;
	*pSeg << (BYTE) mcuId;
	*pSeg << (BYTE) terminalId;

	SendMsg(pSeg, SIP_PARTY_BFCP_MSG_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::SendBFCPTransportInd(APIU32 Status) {
	CSegment* pSeg = new CSegment;

	*pSeg << (DWORD) Status;

	SendMsg(pSeg, SIP_PARTY_BFCP_TRANSPORT_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::PartySendMuteVideo(BYTE isActive) {
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE) isActive;
	SendMsg(pSeg, SEND_MUTE_VIDEO);
}

//--------------------------------------------------------------------------
void CPartyApi::VideoBridgeUpdatedWithNewResolution(APIU32 Status) {
	CSegment* pSeg = new CSegment;

	*pSeg << (DWORD) Status;
	SendMsg(pSeg, VIDEO_BRIDGE_UPDATED_WITH_NEW_RES);
}

//--------------------------------------------------------------------------
void CPartyApi::SendVideoPreferenceToSipParty() {
	SendMsg(NULL, START_VIDEO_PREFERENCE);
}

//--------------------------------------------------------------------------
void CPartyApi::TipLastAckReceived() {
	SendMsg(NULL, TIP_CNTL_LAST_ACK_RECEIVED);
}

//--------------------------------------------------------------------------
void CPartyApi::TipNegotiationResult(DWORD status, WORD numOfStreams,
		BYTE bIsAudioAux, BYTE bIsVideoAux, DWORD doVideoReInvite) {
	CSegment* pSeg = new CSegment;

	*pSeg << (DWORD) status;
	*pSeg << (WORD) numOfStreams;
	*pSeg << (BYTE) bIsAudioAux;
	*pSeg << (BYTE) bIsVideoAux;
	*pSeg << (DWORD) doVideoReInvite;

	SendMsg(pSeg, TIP_CNTL_NEGOTIATION_RESULT);
}

//--------------------------------------------------------------------------
void CPartyApi::TipContentMessageInd(APIS32 opCode, APIU32 Status, BYTE mcuId,
		BYTE terminalId) {
	CSegment* pSeg = new CSegment;

	*pSeg << (DWORD) opCode;
	*pSeg << (DWORD) Status;
	*pSeg << (BYTE) mcuId;
	*pSeg << (BYTE) terminalId;

	SendMsg(pSeg, TIP_CNTL_CONTENT_MSG_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::SipConfEstablishSlaveCall(CRsrcParams* pMrmpRsrcParams,
		CRsrcParams* pCsRsrcParams, UdpAddresses sUdpAddressesParams,
		CSipNetSetup* pNetSetup, CSipCaps* pLocalCaps, CIpComMode* pInitialMode,
		BYTE bIsAdvancedVideoFeatures, const char* strConfParamInfo,
		BYTE eTransportType, WORD McuNumber, WORD TerminalNumber,
		BYTE bNoVideRsrcForVideoParty, WORD room_Id, DWORD tipPartyType) {
	CSegment* pSeg = new CSegment;

	pNetSetup->Serialize(NATIVE, *pSeg);
	pLocalCaps->Serialize(NATIVE, *pSeg);
	pInitialMode->Serialize(NATIVE, *pSeg);
	*pSeg << bIsAdvancedVideoFeatures;

	DWORD len = strConfParamInfo ? strlen(strConfParamInfo) + 1 : 0;
	*pSeg << len;
	if (len) {
		pSeg->Put((BYTE*) strConfParamInfo, len);
	}

	*pSeg << eTransportType;
	if (pMrmpRsrcParams) {
		pMrmpRsrcParams->Serialize(NATIVE, *pSeg);
	}

	pCsRsrcParams->Serialize(NATIVE, *pSeg);
	int sizeOfUdps = sizeof(UdpAddresses);
	pSeg->Put((BYTE*) (&sUdpAddressesParams), sizeOfUdps);

	*pSeg << McuNumber;
	*pSeg << TerminalNumber;
	*pSeg << bNoVideRsrcForVideoParty;
	*pSeg << room_Id;
	*pSeg << tipPartyType;

	SendMsg(pSeg, SIP_CONF_ESTABLISH_CALL);
}

/*  MSSlave-Flora Yao-2013/10/23- MSSlave Flora Comment: */
//--------------------------------------------------------------------------
void CPartyApi::SipConfEstablishMSSlaveCall(CRsrcParams* pMrmpRsrcParams,
		CRsrcParams* pCsRsrcParams, UdpAddresses sUdpAddressesParams,
		CSipNetSetup* pNetSetup, CSipCaps* pLocalCaps, CIpComMode* pInitialMode,
		BYTE bIsAdvancedVideoFeatures, const char* strConfParamInfo,
		BYTE eTransportType, WORD McuNumber, WORD TerminalNumber,
		BYTE bNoVideRsrcForVideoParty, WORD room_Id, eAvMcuLinkType AvMcuLinkType, DWORD msSlavePartyIdx, CSipCaps* pRemoteCaps,
		CSipCaps* m_MaxLocalCaps/*,CIpComMode* m_pTargetModeMaxAllocation*/) {
	CSegment* pSeg = new CSegment;

	pNetSetup->Serialize(NATIVE, *pSeg);
	pLocalCaps->Serialize(NATIVE, *pSeg);
	pInitialMode->Serialize(NATIVE, *pSeg);
	*pSeg << bIsAdvancedVideoFeatures;

	DWORD len = strConfParamInfo ? strlen(strConfParamInfo) + 1 : 0;
	*pSeg << len;
	if (len) {
		pSeg->Put((BYTE*) strConfParamInfo, len);
	}

	*pSeg << eTransportType;
	if (pMrmpRsrcParams) {
		pMrmpRsrcParams->Serialize(NATIVE, *pSeg);
	}

	pCsRsrcParams->Serialize(NATIVE, *pSeg);
	int sizeOfUdps = sizeof(UdpAddresses);
	pSeg->Put((BYTE*) (&sUdpAddressesParams), sizeOfUdps);

	*pSeg << McuNumber;
	*pSeg << TerminalNumber;
	*pSeg << bNoVideRsrcForVideoParty;
	*pSeg << room_Id;
	*pSeg << AvMcuLinkType;
	*pSeg << msSlavePartyIdx;
	if(pRemoteCaps)
		pRemoteCaps->Serialize(NATIVE, *pSeg);

	m_MaxLocalCaps->Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, SIP_CONF_ESTABLISH_CALL);
}

//--------------------------------------------------------------------------
void CPartyApi::SendMessageFromMasterToSlave(WORD destTipPartyType,
		DWORD opcode, CSegment* pMsg) {
}

//--------------------------------------------------------------------------
void CPartyApi::SendMessageFromSlaveToMaster(WORD srcTipPartyType, DWORD opcode,
		CSegment* pMsg) {
}

//--------------------------------------------------------------------------
void CPartyApi::PartyCntlToPartyMessageMasterToSlave(CSegment* pParam) {
	CSegment* pSeg = new CSegment;

	if (NULL != pParam)
		*pSeg << *pParam;

	SendMsg(pSeg, PARTYCONTROL_PARTY_MASTER_TO_SLAVE);
}

//--------------------------------------------------------------------------
void CPartyApi::PartyCntlToPartyMessageSlaveToMaster(CSegment* pParam) {
	CSegment* pSeg = new CSegment;

	if (NULL != pParam)
		*pSeg << *pParam;

	SendMsg(pSeg, PARTYCONTROL_PARTY_SLAVE_TO_MASTER);
}

//--------------------------------------------------------------------------
void CPartyApi::SendNewNameWithRoomId(char* newName, const char* confName) {
	CSegment* seg = new CSegment;

	*seg << newName << confName;

	SendMsg(seg, NEWNAMEWITHROOMID);
}

//--------------------------------------------------------------------------
void CPartyApi::SetSlavePartyRsrcId(DWORD tipPartyType, DWORD peerRsrcId) {
	CSegment* seg = new CSegment;

	*seg << tipPartyType;
	*seg << peerRsrcId;

	SendMsg(seg, SLAVEPARTY_RSRCID);
}

//--------------------------------------------------------------------------
void CPartyApi::SlaveEndChangeMode() {
	CSegment* seg = new CSegment;

	SendMsg(seg, SLAVE_END_CHANGE_MODE);
}

//--------------------------------------------------------------------------
void CPartyApi::SipBandwidthAllocationStatus(DWORD reqBandwidth,
		DWORD allocBandwidth) {
	CSegment* seg = new CSegment;

	*seg << reqBandwidth << allocBandwidth;

	SendLocalMessage(seg, SIPALLOCATEDBANDWIDTHSTATUS);
}

//--------------------------------------------------------------------------
void CPartyApi::SipBandwidthReInviteNeeded() {
	CSegment* seg = new CSegment;

	SendLocalMessage(seg, SIPBANDWIDTHREINVITENEEDED);
}

//--------------------------------------------------------------------------
void CPartyApi::ICEInsufficientBandwidthEvent() {
	CSegment* seg = new CSegment;

	SendLocalMessage(seg, ICE_INSUFFICIENT_BANDWIDTH);
}

//--------------------------------------------------------------------------
void CPartyApi::BfcpStartReestablishConnection() {
	CSegment* seg = new CSegment;

	SendLocalMessage(seg, BFCP_START_REESTABLISH_CONNECTION);
}

//--------------------------------------------------------------------------
void CPartyApi::BfcpEndReestablishConnection() {
	CSegment* seg = new CSegment;

	SendLocalMessage(seg, BFCP_END_REESTABLISH_CONNECTION);
}

//--------------------------------------------------------------------------
void CPartyApi::SendPartyInviteResultInd(
		eGeneralDisconnectionCause disconnectionCause, BOOL bRedial,
		BOOL bIsGwInvite) {
	CSegment* seg = new CSegment;

	*seg << (BYTE) disconnectionCause;
	*seg << bRedial;
	*seg << bIsGwInvite;

	SendMsg(seg, PARTY_INVITE_RESULT_IND);
}

//--------------------------------------------------------------------------
//LYNC2013_FEC_RED:
void CPartyApi::SendSingleFecOrRedMsgToSipParty(DWORD mediaType,DWORD newFecRedPercent)
{
	TRACEINTO << "LYNC2013_FEC_RED";

	CSegment* pSeg = new CSegment;
	*pSeg << mediaType;
	*pSeg << newFecRedPercent;

	SendMsg(pSeg, SIP_PARTY_SINGLE_FEC_RED_MSG);
}
//--------------------------------------------------------------------------
//LYNC2013_FEC_RED:
void CPartyApi::UpdateRateWithFECorRED(CSegment* seg)
{
	SendMsg(seg, UPDATE_NEW_FEC_RED_RATE);
}

//--------------------------------------------------------------------------
void CPartyApi::AskRelayEndPointForIntra(CSegment* pSeg) {
	SendMsg(pSeg, RELAY_ASK_ENDPOINT_FOR_INTRA);
}

//--------------------------------------------------------------------------
void CPartyApi::HandleMrmpRtcpFirInd(CSegment* pParam) {
	CSegment* seg = new CSegment(*pParam);

	SendMsg(seg, RELAY_ENDPOINT_ASK_FOR_INTRA);
}

//--------------------------------------------------------------------------
void CPartyApi::SendAckForScpReq(unsigned int aSequenceNumber) {
	CSegment* pSeg = new CSegment;
	*pSeg << aSequenceNumber;
	SendMsg(pSeg, SIP_PARTY_ACK_FOR__SCP_REQ);
}

//--------------------------------------------------------------------------
void CPartyApi::SendAckForScpNotificationInd(CSegment* pParam) {
	APIU32 sequenceNum;
	*pParam >> sequenceNum;

	CSegment* pSeg = new CSegment;
	*pSeg << sequenceNum;

	SendMsg(pSeg, SIP_PARTY_ACK_FOR_SCP_NOTIFICATION_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyScpNotificationAckFromEP(APIU32 channelHandle,
		APIU32 remoteSeqNumber, APIUBOOL bIsAck) {
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) channelHandle << (DWORD) remoteSeqNumber << (DWORD) bIsAck;

	SendLocalMessage(pSeg, SIP_PARTY_SCP_NOTIFICATION_ACK_FROM_EP);
}

//--------------------------------------------------------------------------
void CPartyApi::SendScpNotificationToMrmpReq(
		CScpNotificationWrapper* apNotifyStruct) {
	CSegment* pSeg = new CSegment;
	apNotifyStruct->Serialize(NATIVE, *pSeg);
	SendMsg(pSeg, SCP_NOTIFICATION_TO_MRMP_REQ);
}

//--------------------------------------------------------------------------
void CPartyApi::SendScpPipesMappingNotificationToMrmpReq(
		CScpPipeMappingNotification* pPipesMapNotifyStruct) {
	// pPipesMapNotifyStruct->Dump();

	CSegment* pSeg = new CSegment;
	pPipesMapNotifyStruct->Serialize(NATIVE, *pSeg);
	SendMsg(pSeg, SCP_PIPES_MAPPING_NOTIFICATION_TO_MRMP_REQ);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdateScmStreams(CIpComMode* pInitialMode) {
	CSegment* pSeg = new CSegment;

	pInitialMode->Serialize(NATIVE, *pSeg);
	SendMsg(pSeg, UPDATE_SCM_STREAMS);
}

//--------------------------------------------------------------------------
void CPartyApi::SendScpIvrStateNotificationReqToParty(eIvrState ivrState,
		DWORD localSeqNumber) {
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) localSeqNumber << (DWORD) ivrState;
	SendMsg(pSeg, SCP_IVR_STATE_NOTIFICATION_REQ);
}

//--------------------------------------------------------------------------
void CPartyApi::UpdateArtOnTranslateVideoSSRC(DWORD aSsrc) {
	TRACEINTO
			<< "mix_mode: Sending SSRC from SvcToAvcTranslator to ART ssrc = "
			<< aSsrc;

	CSegment* pSeg = new CSegment;
	*pSeg << aSsrc;

	SendMsg(pSeg, UPDATE_ART_WITH_SSRC_REQ);
}

//--------------------------------------------------------------------------
void CPartyApi::SendConfEntryPassword(const char* pwd) {
	CSegment* pSeg = new CSegment;
	DWORD pwdLen = strlen(pwd);
	*pSeg << pwdLen;
	pSeg->Put((BYTE*) pwd, pwdLen);
	SendMsg(pSeg, PARTY_CONF_PWD_MSG_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::SendLocalAuthStatus(BYTE status) {
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE) status;
	SendMsg(pSeg, SIP_PARTY_AUTH_PWD_STATUS);
}

//--------------------------------------------------------------------------
void CPartyApi::IPPartyInternalArtsConnected() {
	CSegment* pSeg = new CSegment;

	SendLocalMessage(pSeg, PARTY_TRANSLATOR_ARTS_CONNECTED);
}

//--------------------------------------------------------------------------
void CPartyApi::IPPartyInternalArtsDisconnected() {
	CSegment* pSeg = new CSegment;

	SendLocalMessage(pSeg, PARTY_TRANSLATOR_ARTS_DISCONNECTED);
}

//--------------------------------------------------------------------------
void CPartyApi::ReceivedAckOnIvrPlayMessageReq() {
	TRACEINTO;
	SendMsg(NULL, ACK_PLAY_MESSAGE);
}

//--------------------------------------------------------------------------
void CPartyApi::ReceivedAckOnIvrPlayRecordReq() //IVR for TIP
{
	TRACEINTO;

	SendMsg(NULL, ACK_RECORD_PLAY_MESSAGE);
}

//--------------------------------------------------------------------------
void CPartyApi::SendDialogStart(DialogState& state) {
	CSegment* pSeg = new CSegment;
	*pSeg << state;

	SendMsg(pSeg, MCCF_IVR_START_DIALOG);
}

//--------------------------------------------------------------------------
void CPartyApi::ReceivedAckOnIvrShowSlideReq() {
	TRACEINTO;
	SendMsg(NULL, ACK_SHOW_SLIDE);
}
//--------------------------------------------------------------------------
void CPartyApi::SipPartyTipEarlyPacketInd()
{
	TRACEINTO;
	SendLocalMessage(NULL, SIP_PARTY_TIP_EARLY_PACKET);
}

//BRIDGE-15745
/////////////////////////////////////////////////////////////////////////////
void CPartyApi::NotifySipPendingTransaction(EPendingTransType ePendTrans)
{
	TRACEINTO;

	CSegment *pParam = new CSegment;
	*pParam << (BYTE)ePendTrans;

	SendLocalMessage(pParam, SIP_PARTY_PENDING_TRANS);
}

//--------------------------------------------------------------------------
void CPartyApi::SipPartyDtlsStatus(APIU32 status) {
	PTRACE2INT(eLevelInfoNormal, "CPartyApi::SipPartyDtlsStatus, status:",
			status);

	CSegment* pSeg = new CSegment;

	*pSeg << status;

	SendLocalMessage(pSeg, SIP_PARTY_DTLS_STATUS);
}

//////////////////////////////////////////////////////////////////////////////
void CPartyApi::SipPartyDtlsChannelsDisconnected(CSipComMode* pCurrentMode) {
	PTRACE(eLevelInfoNormal,
			"CPartyApi::SipPartyDtlsChannelsDisconnected, status:");

	CSegment* pSeg = new CSegment;
	pCurrentMode->Serialize(NATIVE, *pSeg);

	SendLocalMessage(pSeg, SIP_PARTY_DTLS_CHANS_DISCONNECTED);
}

//--------------------------------------------------------------------------
void CPartyApi::SendEndVideoUpgradeToMix() {
	TRACEINTO << "@#@";
	CSegment* seg = new CSegment;

	SendMsg(seg, END_VIDEO_UPGRADE_TO_MIX_AVC_SVC);
}

//--------------------------------------------------------------------------
void CPartyApi::StartAvcToSvcArtTranslator(DWORD ssrc, WORD roomId,
		CRsrcParams &artTranslatorRsrcParams,
		CRsrcParams &mrmpArtTranslatorRsrcParams) {
	TRACEINTO << "START_AVC_TO_SVC_ART_TRANSLATOR , ssrc:" << ssrc;
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD) ssrc;
	*pSeg << (WORD) roomId;
	artTranslatorRsrcParams.Serialize(NATIVE, *pSeg);
	mrmpArtTranslatorRsrcParams.Serialize(NATIVE, *pSeg);
	SendMsg(pSeg, START_AVC_TO_SVC_ART_TRANSLATOR);
}

//--------------------------------------------------------------------------
void CPartyApi::DisconnectAvcToSvcArtTranslator() {
    TRACEINTO << "REMOVE_AVC_TO_SVC_ART_TRANSLATOR";

	CSegment* pSeg = new CSegment;
	SendMsg(pSeg, REMOVE_AVC_TO_SVC_ART_TRANSLATOR);
}
//--------------------------------------------------------------------------
void CPartyApi::DisconnectAvcToSvcArtTranslator(CIpComMode* pInitialMode) {
	TRACEINTO << "REMOVE_AVC_TO_SVC_ART_TRANSLATOR";

	CSegment* pSeg = new CSegment;
	pInitialMode->Serialize(NATIVE, *pSeg);
	SendMsg(pSeg, REMOVE_AVC_TO_SVC_ART_TRANSLATOR);
}
void CPartyApi::MrmpStreamIsMustReq(CSegment* pParam) {
	TRACEINTO << "got req CONF_PARTY_MRMP_STREAM_IS_MUST_REQ";
	CSegment* pSeg = new CSegment(*pParam);

	SendMsg(pSeg, SEND_MRMP_STREAM_IS_MUST_REQ);
}
//--------------------------------------------------------------------------
void CPartyApi::MrmpStreamIsMustAck() {
	TRACEINTO << "got req CONF_PARTY_MRMP_STREAM_IS_MUST_REQ";
	SendMsg(NULL, SEND_MRMP_STREAM_IS_MUST_ACK);
}
//--------------------------------------------------------------------------
void CPartyApi::SendVsrMsgIndToSipParty(ST_VSR_SINGLE_STREAM* vsr) {
	TRACEINTO << "SendVsrMsgIndToSipParty";
	CSegment* pSeg = new CSegment;
	pSeg->Put(reinterpret_cast<BYTE*>(vsr), sizeof(ST_VSR_SINGLE_STREAM));
	SendMsg(pSeg, SIP_PARTY_VSR_MSG_IND);
}

//--------------------------------------------------------------------------
void CPartyApi::SendMsftOutSlaveCreatedToSipParty(CSegment* pParam) {
	TRACEINTO << "SendMsftOutSlaveCreatedToSipParty";
	CSegment* pSeg = new CSegment(*pParam);
	SendMsg(pSeg, SIP_MSFT_OUTSLAVES_CREATED);
}
//--------------------------------------------------------------------------
void CPartyApi::SendMultipeVsrToSipParty(ST_VSR_MUTILPLE_STREAMS* multipleVsr) {
	TRACEINTO << "SendMultipeVsrToSipParty";
	CSegment* pSeg = new CSegment;
	pSeg->Put(reinterpret_cast<BYTE*>(multipleVsr),
			sizeof(ST_VSR_MUTILPLE_STREAMS));
	SendMsg(pSeg, SIP_PARTY_MULTI_VSR_REQ);
}
//--------------------------------------------------------------------------
void CPartyApi::SendSingleVsrMsgIndToSipParty(CSegment* pParam) {
	TRACEINTO << "SendSingleVsrMsgIndToSipParty";
	CSegment* pSeg = new CSegment(*pParam);
//	 pSeg->Put(reinterpret_cast<BYTE*>(vsr), sizeof(ST_VSR_SINGLE_STREAM));
	SendMsg(pSeg, SIP_PARTY_SINGLE_VSR_MSG_IND);
}
//--------------------------------------------------------------------------
void CPartyApi::ActiveMediaForAvMcuLync()
{
	TRACEINTO << "ActiveMediaForAvMcuLync";

	CSegment* pSeg = new CSegment;
	SendMsg(pSeg, ACTIVE_MEDIA_FOR_AVMCU_LYNC);
}

//eFeatureRssDialin
void CPartyApi::SendRecordingControlInfoToParty(CSegment* pParam)
{
	//for both SIP and H323 party
	TRACEINTO<<"SendRecordingControlInfoToParty";
	CSegment* seg = new CSegment(*pParam);

	SendMsg(seg, SRS_RECORDING_CONTROL_IND);
}
void CPartyApi::SendRecordingControlStatus(BYTE status)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)status;
	SendMsg(pSeg, SRS_RECORDING_CONTROL_STATUS);

}
void CPartyApi::SendLayoutControlLocal(BYTE layout)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)layout;
	SendMsg(pSeg, SRS_LAYOUT_CONTROL_PARTY_LOCAL);

}

//WebRtc
void CPartyApi::SendWebRtcPartyEstablishCallIdle()
{
	CSegment* pSeg = new CSegment;
	SendMsg(pSeg, SIP_WEBRTC_PARTY_ESTABLISH_CALL);
}
void CPartyApi::SendWebRtcConnectFailure()
{
	CSegment* pSeg = new CSegment;
	SendMsg(pSeg, WEBRTC_CONNECT_FAILURE);
}
void CPartyApi::SendWebRtcConnectTout()
{
	CSegment* pSeg = new CSegment;
	SendMsg(pSeg, WEBRTC_CONNECT_TOUT);
}
//--------------------------------------------------------------------------
/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
void CPartyApi::SendByeToParty(PartyRsrcID PartyId) 
{
	CSegment* seg = new CSegment;
	*seg <<PartyId;
	SendMsg(seg, VIDEORECOVERYBYE);
}
/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
void CPartyApi::SendMsSlaveVideoInSyncedToSipParty(CSegment* pParam) {
	TRACEINTO << "SendSingleVsrMsgIndToSipParty";
	CSegment* pSeg = new CSegment(*pParam);
	SendMsg(pSeg, MS_SLAVE_VIDEO_IN_SYNCED);
}
//--------------------------------------------------------------------------
void CPartyApi::SendMsSlaveVidRefreshToSipParty(CSegment* pParam) {
	TRACEINTO;
	CSegment* pSeg = new CSegment(*pParam);
	SendMsg(pSeg, MS_SLAVE_VIDREFRESH);
}

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void CPartyApi::SendMediaDisconnectionByCAMToParty(CSegment* pParam) {
	TRACEINTO;
	CSegment* pSeg = new CSegment(*pParam);
	SendMsg(pSeg, CAM_TO_IVR_PARTY_MEDIA_DISCONNECTED);
}

//--------------------------------------------------------------------------
void CPartyApi::SendMediaConnectedByCAMToParty(CSegment* pParam) {
	TRACEINTO;
	CSegment* pSeg = new CSegment(*pParam);
	SendMsg(pSeg, CAM_TO_IVR_PARTY_MEDIA_CONNECTED);
}
