#include "NStream.h"
#include "ConfApi.h"
#include "ConfPartyOpcodes.h"
#include "H323Scm.h"
#include "H323Caps.h"
#undef CIpComMode
#include "IpScm.h"
#include "H323NetSetup.h"
#include "LectureModeParams.h"
#include "VisualEffectsParams.h"
#include "VideoLayout.h"
#include "IVRPlayMessage.h"
#include "ConfAppMngr.h"
#include "IVRCntlLocal.h"
#include "IpAddressDefinitions.h"
#include "IpCommonDefinitions.h"
#include "Party.h"
#include "TaskApi.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "IpNetSetup.h"
#include "SipNetSetup.h"
#include "SipHeadersList.h"
#include "SipCaps.h"
#include "TraceStream.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsAudioCntl.h"
#include "IsdnNetSetup.h"
#include "H320Caps.h"
#include "H320ComMode.h"
#include "IvrApiStructures.h"
#include "AutoScanOrder.h"
#include "ScpHandler.h"
#include "ScpNotificationWrapper.h"
#include "SipVsrControl.h"
#include "MsVsrMsg.h"


////////////////////////////////////////////////////////////////////////////
//                        CConfApi
////////////////////////////////////////////////////////////////////////////
CConfApi::CConfApi(DWORD monitorConfId)
{
#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (monitorConfId != DEFAULT_CONF_ID)
	{
		std::vector<CTaskApp*>* pTasks = CProcessBase::GetProcess()->GetTasks();
		if (pTasks)
		{
			std::vector<CTaskApp*>::const_iterator it;
			for (it = pTasks->begin(); it != pTasks->end(); ++it)
			{
				if (monitorConfId == (*it)->GetMonitorConfId()
					&& (*it)->IsTypeOf("CConf"))
				{
					SetLocalMbx((*it)->GetLocalQueue());
					break;
				}
			}
		}
	}
#endif
}

//--------------------------------------------------------------------------
CConfApi::~CConfApi()
{
}

//--------------------------------------------------------------------------
const char*  CConfApi::NameOf() const
{
	return "CConfApi";
}

//--------------------------------------------------------------------------
void CConfApi::Create(void (* entryPoint)(void*), COsQueue& creatorRcvMbx, DWORD confId, char* name)
{
	CTaskApi::Create(creatorRcvMbx);

	// set specific stack param
	m_appParam << confId
	           << name;

	// load application
	LoadApp(entryPoint);
}

//--------------------------------------------------------------------------
void  CConfApi::ForceKill()
{
	SendOpcodeMsg(FORCE_KILL);
	DestroyOnlyApi();
}

//--------------------------------------------------------------------------
void CConfApi::UpdateDB(CTaskApp* pParty, WORD updType, DWORD param, WORD externFlag, CSegment* pParamSeg)
{
	CSegment* seg = new CSegment;

	*seg << (DWORD)pParty
	     << (WORD)updType
	     << (DWORD)param;

	if (pParamSeg != NULL)
		*seg << *pParamSeg;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(seg, UPDATEDB);
	else
		SendMsg(seg, UPDATEDB);
#else
	if (externFlag == 1)
		SendMsg(seg, UPDATEDB);
	else
		SendLocalMessage(seg, UPDATEDB);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::UpdateChannelsLprHeader(CTaskApp* pParty, BYTE isChannelWithLprHeader)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty
	     << (WORD)CHANNELSWITHLPRPAYLOAD
	     << (DWORD)isChannelWithLprHeader;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(seg, UPDATEDB);
	else
		SendMsg(seg, UPDATEDB);
#else
	SendMsg(seg, UPDATEDB);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::HandleTerminalEvent(WORD req, CSegment* Command)
{
	CSegment* seg = new CSegment;
	*seg << req;

	if (Command)
		*seg << *Command;

	SendMsg(seg, HANDLE_TERMINAL_EVENT);
}

//--------------------------------------------------------------------------
void CConfApi::UpdatePartyStateInCdr(const CTaskApp* pParty)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pParty
	      << (DWORD)EVENT_PARTY_CONNECTED; // just to enter the right case

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(pSeg, UPDATECDR);
	else
		SendMsg(pSeg, UPDATECDR);
#else
	// the same function will apply for IP_PARTY_CONNECTED
	// the CDR will print from pParty
	SendMsg(pSeg, UPDATECDR);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::UpdateGkCallIdInCdr(const CTaskApp* pParty, BYTE* gkCallId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pParty
	      << (DWORD)GK_INFO;

	pSeg->Put((unsigned char*)gkCallId, SIZE_OF_CALL_ID);

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(pSeg, UPDATECDR);
	else
		SendMsg(pSeg, UPDATECDR);
#else
	SendMsg(pSeg, UPDATECDR);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::UpdateNewRateInfoInCdr(const CTaskApp* pParty, DWORD currentRate)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pParty
	      << (DWORD)PARTY_NEW_RATE
	      << (DWORD)currentRate;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(pSeg, UPDATECDR);
	else
		SendMsg(pSeg, UPDATECDR);
#else
	SendMsg(pSeg, UPDATECDR);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::UpdateCallInfoInCdr(const CTaskApp* pParty, DWORD maxBitRate, EFormat format, WORD maxFrameRate)
{
	CSegment* pSeg = new CSegment;

	*pSeg << (DWORD)pParty
	      << (DWORD)PARTICIPANT_MAX_USAGE_INFO
	      << (DWORD)maxBitRate
	      << (DWORD)format
	      << (WORD)maxFrameRate;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(pSeg, UPDATECDR);
	else
		SendMsg(pSeg, UPDATECDR);
#else
	SendMsg(pSeg, UPDATECDR);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::UpdateSvcSipPartyInfoInCdr(const CTaskApp* pParty, const std::list<StreamDesc>* pStreams, ECodecSubType eAudioCodec, DWORD dwBitRateOut, DWORD dwBitRateIn)
{
	CSegment* pSeg = new CSegment;

	*pSeg << (DWORD)pParty
	      << (DWORD)SVC_SIP_PARTY_CONNECTED
	      << (DWORD)(pStreams->size());

	std::ostringstream msg;
	std::list<StreamDesc>::const_iterator it = pStreams->begin();
	for ( ; it != pStreams->end(); ++it)
	{
		*pSeg	<< (DWORD)(it->m_bitRate)
				<< (DWORD)(it->m_frameRate)
				<< (DWORD)(it->m_height)
				<< (DWORD)(it->m_width);
		msg << "\nm_bitRate = " << it->m_bitRate << ",  m_frameRate = " << it->m_frameRate << ",  m_height = " << it->m_height << ",  m_width = " << it->m_width;
	}

	*pSeg << (DWORD)eAudioCodec
			<< dwBitRateOut
			<< dwBitRateIn;
	msg << "\nAudioCodec = " << eAudioCodec
		<< "\nBitRateOut = " << dwBitRateOut
		<< "\nBitRateIn = " << dwBitRateIn;
	TRACEINTO << "BG_CDR  pSeg->GetWrtOffset = " << pSeg->GetWrtOffset() << msg.str().c_str();

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(pSeg, UPDATECDR);
	else
		SendMsg(pSeg, UPDATECDR);
#else
	SendMsg(pSeg, UPDATECDR);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::StartConf(const CCommRes& rsrv)
{
	CSegment* seg   = new CSegment;
	CCommRes* pRsrv = new CCommRes(rsrv);
	pRsrv->Serialize(NATIVE, *seg);
	SendMsg(seg, STARTCONF);
	POBJDELETE(pRsrv);
}

//--------------------------------------------------------------------------
void CConfApi::AddParty(const CRsrvParty& rsrvParty, DWORD undefId, DWORD partyType)
{
	COstrStream str;
	CRsrvParty* pRsrvParty = new CRsrvParty(rsrvParty);
	pRsrvParty->Serialize(NATIVE, str);
	str << (DWORD)undefId << "\n";
	CSegment* seg = new CSegment;
	*seg << (DWORD)partyType;
	*seg << str.str().c_str();
	SendMsg(seg, ADDPARTY);
	POBJDELETE(pRsrvParty);
}

//--------------------------------------------------------------------------
void CConfApi::RejectAddRecordingLinkParty(DWORD rStatus)
{
	CSegment seg;
	seg << rStatus;
	IvrConfNotification(RECORD_CONF_FAILED, &seg);
}

//--------------------------------------------------------------------------
void CConfApi::DisconnectRecordingLink()
{
	SendMsg(NULL, DISCONNECT_RECORDING_LINK_PARTY);
}
//---------------------------------------------------------------------------
void CConfApi::ActivateAvMcuAutoTerminationCheck()
{
	SendMsg(NULL, ACTIVATE_AUTO_TERMINATION_CHECK_FOR_AV_MCU);
}

//--------------------------------------------------------------------------
void CConfApi::DropParty(const char* partyName, WORD mode, WORD discCause)
{
	CSegment* seg = new CSegment;
	*seg << partyName
	     << mode
	     << discCause;

	SendMsg(seg, DELPARTY);
}

//--------------------------------------------------------------------------
void CConfApi::DropPartyViolent(const char* partyName, WORD mode, WORD discCause, DWORD taskId)
{
	CSegment* seg = new CSegment;
	*seg << partyName
	     << mode
	     << discCause
	     << taskId;
	SendMsg(seg, DELPARTYVIOLENT);
}

//--------------------------------------------------------------------------
WORD CConfApi::ImportParty(void* pConfPartyCntl, void* pConfPartyDesc, void* pDestName, CSegment& rspMsg, DWORD tout, OPCODE& rspOpcode)
{
	CSegment* seg = new CSegment;
	*seg << pConfPartyCntl
	     << pConfPartyDesc
	     << pDestName;

	return (SendMessageSync(seg, IMPPARTY, tout, rspOpcode));
}

//--------------------------------------------------------------------------
// bUseDelay:
// = 1 in case of 'blast dial-out'
// = 0 if no delay is needed while connecting, as in manual connection.
void CConfApi::ReconnectParty(const char* partyName, BYTE bUseDelay)
{
	CSegment* seg = new CSegment;
	*seg << partyName
	     << (WORD)bUseDelay;

	SendMsg(seg, RECONPARTY);
}

//--------------------------------------------------------------------------
void CConfApi::SetEndTime(const CStructTm& tm)
{
	CSegment* seg    = new CSegment;
	CStructTm TempTm = tm;

	COstrStream msg;
	TempTm.Serialize(msg);
	*seg << msg.str().c_str();

	SendMsg(seg, SETENDTIME);
}

//--------------------------------------------------------------------------
void CConfApi::SetAutoRedial(const BYTE bAutoRedial)
{
	CSegment* seg = new CSegment;
	*seg << bAutoRedial;

	SendMsg(seg, SETAUTOREDIAL);
}

//--------------------------------------------------------------------------
void CConfApi::ScpRequestFromEp(PartyRsrcID PartyId, CMrmpScpStreamsRequestStructWrap& aScpStreamReq)
{
	CSegment* pSeg = new CSegment;

	*pSeg<< PartyId
	     << SCP_REQUEST_BY_EP;
	aScpStreamReq.Serialize(NATIVE, *pSeg);

  // as far as I understand I need to change this
	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ScpNotificationIndFromEp(PartyRsrcID PartyId, CScpNotificationWrapper& aScpStreamNotificationInd)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << SCP_NOTIFICATION_BY_EP;
	aScpStreamNotificationInd.Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyDisConnect(WORD cause, CTaskApp* pParty, const char* alternativeAddrStr, DWORD MipErrorNumber)
{
	CSegment* pSeg = new CSegment;

	*pSeg << (DWORD)pParty
	      << cause;

	DWORD addrLen = alternativeAddrStr ? strlen(alternativeAddrStr)+1 : 0;
	*pSeg << addrLen;
	if (addrLen)
		pSeg->Put((BYTE*)alternativeAddrStr, addrLen);

	if (MipErrorNumber)
		*pSeg << MipErrorNumber;

	// Messages sent through the local queue can not be processed for a long time if the external queue does not have any messages.
	// D.K. BRIDGE-3389 - Currently decided to send this message via external queue

//#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
//	if (m_pLocalRcvMbx)
//		SendLocalMessage(pSeg, PARTYDISCONNECT);
//	else
//		SendMsg(pSeg, PARTYDISCONNECT);
//#else

	SendMsg(pSeg, PARTYDISCONNECT);

//#endif
}

//--------------------------------------------------------------------------
void CConfApi::PartyChangeVidMode(CTaskApp* pParty, WORD isSetToSecondary)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty
	     << (WORD)isSetToSecondary;

	SendMsg(seg, PARTY_CHANEG_VID_MODE);
}

//--------------------------------------------------------------------------
void CConfApi::InformConfOnPartyReCap(CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty;

	SendMsg(seg, PARTY_RECEIVE_RECAPS);
}

//--------------------------------------------------------------------------
void CConfApi::AddAndConnectRecordingLink(BYTE muteVideo)
{
	CSegment* seg = new CSegment;
	*seg << (BYTE)muteVideo;

	SendMsg(seg, ADD_RECORDING_LINK_PARTY);
}

//--------------------------------------------------------------------------
void CConfApi::EndAddParty(CTaskApp* pParty, WORD status)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty
	     << status;

	SendLocalMessage(seg, ENDADDPARTY);
}

//--------------------------------------------------------------------------
void CConfApi::EndDelParty(CTaskApp* pParty, WORD status)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty
	     << (DWORD)status;

	SendLocalMessage(seg, ENDDELPARTY);
}

//--------------------------------------------------------------------------
void CConfApi::EndExportParty(PartyRsrcID PartyId, WORD status)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << status;

	SendLocalMessage(seg, ENDEXPPARTY);
}

//--------------------------------------------------------------------------
void CConfApi::EndImportParty(PartyRsrcID PartyId, WORD status)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << status;

	SendLocalMessage(seg, ENDIMPPARTY);
}

//--------------------------------------------------------------------------
void CConfApi::RedailParty(CTaskApp* pParty, WORD IsHotBackupRedial)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty
	     << IsHotBackupRedial;

	SendLocalMessage(seg, REDAILPARTY);
}

//--------------------------------------------------------------------------
WORD CConfApi::IsDestConfReadyForMove(CSegment& rspMsg, DWORD tout, OPCODE& rspOpcode)
{
	CSegment* seg = new CSegment;

	return SendMessageSync(seg, ISCONFREADYFORMOVE, tout, rspOpcode);
}

//--------------------------------------------------------------------------
void CConfApi::MoveParty(PartyMonitorID monitorPartyId, ConfMonitorID monitorDestConfId, EMoveType eMoveType)
{
	CSegment* seg = new CSegment;
	*seg << monitorPartyId
	     << monitorDestConfId
	     << (WORD)eMoveType;

	SendMsg(seg, EXPPARTY);
}

//--------------------------------------------------------------------------
WORD CConfApi::AddInH323Party(CH323NetSetup* pH323NetSetup, CTaskApp* pParty, COsQueue& partyRcvMbx, char* name, DWORD tout, CSegment& rspMsg)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty
	     << name;

	pH323NetSetup->Serialize(NATIVE, *seg);
	partyRcvMbx.Serialize(*seg);

	OPCODE rspOpcode;
	return SendMessageSync(seg, H323ADDINPARTY, tout, rspOpcode, rspMsg);
}

//--------------------------------------------------------------------------
void CConfApi::MuteAllButX(DWORD PartyId, BYTE yesNo)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << yesNo;

	SendMsg(seg, MUTE_ALL_BUT_X);
}

//--------------------------------------------------------------------------
void CConfApi::LockConference(BYTE yesNo)
{
	CSegment* seg = new CSegment;
	*seg << yesNo;

	SendMsg(seg, LOCK_CONFERENCE);
}

//--------------------------------------------------------------------------
void CConfApi::ShowParticipantsToConf(DWORD partyRsrcID, DWORD permission, DWORD actionSrc)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)actionSrc
	     << (DWORD)partyRsrcID
	     << (DWORD)permission;

	SendMsg(seg, IVR_SHOW_PARTICIPANTS);
}

//--------------------------------------------------------------------------
void CConfApi::ShowGathering(DWORD partyRsrcID, DWORD permission, DWORD actionSrc)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)actionSrc
	     << (DWORD)partyRsrcID
	     << (DWORD)permission;

	SendMsg(seg, IVR_SHOW_GATHERING);
}

//--------------------------------------------------------------------------
void CConfApi::EnableRollCall(BYTE byteEnable)
{
	UpdateDB((CTaskApp*)0xffff, ROLL_CALL, byteEnable, 1);
}

//--------------------------------------------------------------------------
void CConfApi::PartyPassedIVR_EntranceProcedures(const char* szPartyName)
{
	CSegment* seg = new CSegment;
	*seg <<  szPartyName;

	SendMsg(seg, PARTY_PASSED_IVR);
}

//--------------------------------------------------------------------------
void CConfApi::ConnectDialOutPartyByNumb(BYTE bNumbParty)
{
	CSegment* seg = new CSegment;
	*seg <<  bNumbParty;

	SendMsg(seg, CONNECT_DIAL_OUT);
}

//--------------------------------------------------------------------------
void CConfApi::InviteParty(DWORD partyRsrcID, DWORD partyMonitorID, char* invitePartyDailingString, const map<WORD, WORD>& dailingOrder)
{
	CSegment* seg = new CSegment;
	*seg << partyRsrcID
	     << partyMonitorID
	     << invitePartyDailingString
	     << (WORD)dailingOrder.size();

	for (map<WORD, WORD>::const_iterator ii = dailingOrder.begin(); ii != dailingOrder.end(); ++ii)
	{
		*seg << ii->first;   // interfaceType
		*seg << ii->second;  // interfaceOrder
	}

	SendMsg(seg, IVR_INVITE_PARTY_TO_CONF);
}

//--------------------------------------------------------------------------
void CConfApi::DisconnectInvitedParticipant(DWORD partyRsrcID)
{
	CSegment* seg = new CSegment;
	*seg << partyRsrcID;

	SendMsg(seg, DISCONNECT_INVITED_PARTICIPANT_REQ);
}

//--------------------------------------------------------------------------
void CConfApi::InitiateFlowControlForConference(DWORD newVidRate)
{
	CSegment* seg = new CSegment;
	*seg << newVidRate;

	SendMsg(seg, CONF_FLOWCONTROL);
}

//--------------------------------------------------------------------------
void CConfApi::InitiateFlowControlForParty(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty;

	SendMsg(seg, PARTY_FLOWCONTROL);
}

//--------------------------------------------------------------------------
// VNGR-6679 - Solution
WORD CConfApi::AddInSipParty(CSipNetSetup* pNetSetup, CSipCaps* pRemoteCaps, CTaskApp* pParty, COsQueue& partyRcvMbx,
                             char* name, DWORD tout, CSegment& rspMsg, BYTE IsOfferer, BYTE bIsMrcHeader, BYTE bIsWebRtcCall,
                             LyncConnType lyncEpType, RemoteIdent epType, eIsUseOperationPointsPreset isUseOperationPointesPresets, BOOL bIsRemoteSlave, BYTE  initialLayout)
{

	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty
	     << name
	     << IsOfferer
	     << bIsMrcHeader
	     << bIsWebRtcCall
	     << (BYTE)lyncEpType
	     << (BYTE)epType
	     << (BYTE)isUseOperationPointesPresets
	     << (BYTE)bIsRemoteSlave
	     << initialLayout;

	pNetSetup->Serialize(NATIVE, *seg);
	partyRcvMbx.Serialize(*seg);
	if (pRemoteCaps)
		pRemoteCaps->Serialize(NATIVE, *seg);

	OPCODE rspOpcode;
	return SendMessageSync(seg, SIPADDINPARTY, tout, rspOpcode, rspMsg);
}

//--------------------------------------------------------------------------
void CConfApi::ReleaseAVMCUParty(PartyMonitorID partyId, CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD sdpLen)
{
	CSegment* seg = new CSegment;
	*seg << partyId;
	*seg << sdpLen;
	seg->Put((BYTE*)pSdpAndHeaders, sdpLen);

	pNetSetup->Serialize(NATIVE, *seg);

	SendMsg(seg, RELEASE_AVMCU_PARTY);
}

//--------------------------------------------------------------------------
void CConfApi::ReleaseReservedPartyFromLobby(DWORD listId, DWORD dwPartyId)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)listId
	     << (DWORD)dwPartyId;

	SendMsg(seg, RELEASE_RESERVED_PARTY);
}

//--------------------------------------------------------------------------
void CConfApi::SendIpMonitoringEventToParty(DWORD partyID, const char* partyName)
{
	CSegment* seg = new CSegment;
	*seg << partyID
	     << partyName;

	SendMsg(seg, IPPARTYMONITORINGREQ);
}

//--------------------------------------------------------------------------
void CConfApi::EndInitCom(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << PartyId;

	SendMsg(seg, PARTYENDINITCOM);
}

//--------------------------------------------------------------------------
void CConfApi::PartyEndDisConnect(PartyRsrcID PartyId, WORD status)
{
	CSegment* seg = new CSegment;

	*seg<< PartyId
	    << PARTYENDDISCONNECT
	    << status;

//#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
//	if (m_pLocalRcvMbx)
//		SendLocalMessage(seg, PARTY_MSG);
//	else
//		SendMsg(seg, PARTY_MSG);
//#else
	SendMsg(seg, PARTY_MSG);
//#endif
}

//--------------------------------------------------------------------------
void CConfApi::PartyIncreaseDisconnctingTimer(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << INCREASE_DISCONNECT_TIMER;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyEndNetChnlDisConnect(PartyRsrcID PartyId, WORD status)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << ENDNETCHNLDISCONNECT
	     << status;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyEndChangeModeIp(PartyRsrcID PartyId, const CIpComMode& pCurrentScm, DWORD status, WORD reason, CSecondaryParams* pSecParamps)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << PARTYENDCHANGEMODE;

	pCurrentScm.Serialize(NATIVE, *seg);
	*seg << status
	     << reason;
	if (pSecParamps)
		pSecParamps->Serialize(NATIVE, *seg);
	else
	{
		CSecondaryParams secParamps;
		secParamps.Serialize(NATIVE, *seg);
	}

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendPartyReceiveReCapsToPartyControl(PartyRsrcID PartyId, CCapH323* remoteCap)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << REMOTE_SENT_RE_CAPS;

	remoteCap->Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendDisconnectBridgesToPartyControl(PartyRsrcID PartyId, WORD isDisconnectAudio, WORD isDisconnectVideo)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << REMOTE_SENT_DISCONNECT_BRIDGES
	      << isDisconnectAudio
	      << isDisconnectVideo;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendConnectBridgesToPartyControl(PartyRsrcID PartyId, WORD isConnectAudio, WORD isConnectVideo,
												 CIpComMode* pNewMode, CSipCaps* pRemoteCaps,
												 unsigned int incomingVideoChannelHandle, unsigned int outgoingVideoChannelHandle )
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << REMOTE_SENT_CONNECT_BRIDGES
	      << isConnectAudio
	      << isConnectVideo;

	pNewMode->Serialize(NATIVE, *pSeg);
	pRemoteCaps->Serialize(NATIVE, *pSeg);

	*pSeg << incomingVideoChannelHandle;
	*pSeg << outgoingVideoChannelHandle;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyExport(PartyRsrcID PartyId, WORD status)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << PARTYEXPORT
	     << status;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
// last parameter added because of MCU internal problem...
void CConfApi::PartyBridgeResponseMsg(const CTaskApp* pParty,
                                      OPCODE brdgOpcode,
                                      WORD responseOpcode,
                                      WORD status,
                                      BOOL isPartyCntlMsg,
                                      EMediaDirection eMediaDirection,
                                      CSegment* pParams)
{
	if (isPartyCntlMsg)       // message to conference party control
	{
		CSegment* seg = new CSegment;

		*seg << (DWORD)pParty
		     << (WORD)responseOpcode;

		// Don't send status for connect.
		if (responseOpcode != PARTY_AUDIO_CONNECTED &&
		    responseOpcode != PARTY_VIDEO_CONNECTED &&
		    responseOpcode != PARTY_VIDEO_IVR_MODE_CONNECTED &&
		    responseOpcode != PARTY_CONTENT_CONNECTED &&
		    responseOpcode != FECC_PARTY_BRIDGE_CONNECTED)
		{
			*seg << status;
		}

		*seg << (WORD)eMediaDirection;

		if (NULL != pParams)
			*seg << *pParams;

		SendLocalMessage(seg, PARTY_MSG); // Send message to conference
	}
	else                                // message to Bridge
	{
		SendBrdgPartyCntlMsg(pParty, brdgOpcode, responseOpcode, status, eMediaDirection, pParams);
	}
}

void CConfApi::PartyBridgeResponseMsgById(PartyRsrcID partyID,
                                      OPCODE brdgOpcode,
                                      WORD responseOpcode,
                                      WORD status,
                                      BOOL isPartyCntlMsg,
                                      EMediaDirection eMediaDirection,
                                      CSegment* pParams)
{
	if (isPartyCntlMsg)       // message to conference party control
	{
		CSegment* seg = new CSegment;

		*seg << partyID
		     << (WORD)responseOpcode;

		// Don't send status for connect.
		if (responseOpcode != PARTY_AUDIO_CONNECTED &&
		    responseOpcode != PARTY_VIDEO_CONNECTED &&
		    responseOpcode != PARTY_VIDEO_IVR_MODE_CONNECTED &&
		    responseOpcode != PARTY_CONTENT_CONNECTED &&
		    responseOpcode != FECC_PARTY_BRIDGE_CONNECTED)
		{
			*seg << status;
		}

		*seg << (WORD)eMediaDirection;

		if (NULL != pParams)
			*seg << *pParams;

		SendLocalMessage(seg, PARTY_MSG); // Send message to conference
	}
	else                                // message to Bridge
	{
		if (pParams)
			SendBrdgPartyCntlMsgById(partyID, brdgOpcode, responseOpcode, status, eMediaDirection, pParams);
		else
			SendBrdgPartyCntlMsgById(partyID, brdgOpcode, responseOpcode, status, eMediaDirection);
	}
}


//--------------------------------------------------------------------------
// For all the response msg without directions....
void CConfApi::SendResponseMsg(const CTaskApp* pParty,
                               OPCODE brdgOpcode,
                               WORD responseOpcode,
                               WORD status,
                               BOOL isPartyCntlMsg,
                               CSegment* pParams)
{
	if (isPartyCntlMsg)       // message to conference party control
	{
		BOOL isExtParams = FALSE;
		if (NULL != pParams)
			isExtParams = TRUE;

		CSegment* seg = new CSegment;
		*seg << (DWORD)pParty
		     << (WORD)responseOpcode
		     << status
		     << isExtParams;

		if (isExtParams)
			*seg << *pParams;

		SendLocalMessage(seg, PARTY_MSG); // Send message to conference
	}

	else                                // message to Bridge
	{
		if (pParams)
			SendBrdgPartyCntlMsg(pParty, brdgOpcode, responseOpcode, status, eMediaInAndOut, pParams);
		else
			SendBrdgPartyCntlMsg(pParty, brdgOpcode, responseOpcode, status);
	}
}

//--------------------------------------------------------------------------
void CConfApi::CopVideoInChangeMode(PartyRsrcID PartyId, BYTE changeModeParam, ECopChangeModeType changeModeType)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << COP_VIDEO_IN_CHANGE_MODE
	     << changeModeParam
	     << (BYTE)changeModeType;

	SendLocalMessage(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::CopVideoOutChangeMode(PartyRsrcID PartyId, BYTE encoderIndex)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << COP_VIDEO_OUT_CHANGE_MODE
	     << encoderIndex;

	SendLocalMessage(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::CopStartCascadeLinkAsLecturerPendingMode(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << COP_START_CASCADE_LINK_LECTURE_MODE;

	SendLocalMessage(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::CopCascadeLinkLectureMode(PartyRsrcID PartyId, BYTE isActive)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << COP_UPDATE_CASCADE_LINK_LECTURE_MODE
	     << isActive;

	SendLocalMessage(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendCloseChannelToConfLevel(PartyRsrcID PartyId, WORD type, WORD direction, WORD roleLabel)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << PARTY_CLOSE_CHANNEL
	      << type
	      << direction
	      << roleLabel;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::VideoMute(PartyRsrcID PartyId, WORD onOff)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << VIDEOMUTE
	      << onOff;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::AudioMute(PartyRsrcID PartyId, WORD onOff)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << AUDIOMUTE
	      << onOff;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::VideoRefresh(PartyRsrcID PartyId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << VIDREFRESH;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::NotifyVbOnNetworkQualityChange(const CTaskApp* pParty,
                                              eRtcpPacketLossStatus networkStatePerCell,
                                              eRtcpPacketLossStatus networkStatePerLayout)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)NOTIFY_VB_ON_NETWORK_QUALITY_CHANGE
	     << (DWORD)pParty
	     << (WORD)networkStatePerCell
	     << (WORD)networkStatePerLayout;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::EventModeIntraPreviewReq(PartyRsrcID PartyId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << REQUEST_PREVIEW_INTRA;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::VideoRefreshBeforeRecording(const char* partyName, WORD direction)
{
	CSegment* seg = new CSegment;
	*seg << partyName
	     << direction;

	SendMsg(seg, VIDEOREFRESHBEFORERECORDING);
}

//--------------------------------------------------------------------------
void CConfApi::PartyConnect(PartyRsrcID PartyId, WORD status)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << PARTYCONNECT
	      << status;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyMoveToSecondery(PartyRsrcID PartyId, WORD reason, CSecondaryParams* pSecParamps)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << IPPARTYMSECONDARY
	      << reason;

	if (pSecParamps)
		pSecParamps->Serialize(NATIVE, *pSeg);
	else
	{
		CSecondaryParams secParamps;
		secParamps.Serialize(NATIVE, *pSeg);
	}

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateMainPartyOnITPSpeaker(PartyRsrcID PartyId, DWORD numOfActiveLinks, eTelePresencePartyType itpType)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << ITPSPEAKERIND
	      << numOfActiveLinks
	      << (BYTE)itpType;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateMainPartyOnITPSpeakerAck(PartyRsrcID PartyId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << ITPSPEAKERACKIND;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetPartySecondaryCause(PartyRsrcID PartyId, WORD cause, CSecondaryParams& secParams)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << SECONDARYCAUSEH323
	      << cause;

	secParams.Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::H323PartyConnect(PartyRsrcID PartyId,
                                WORD status,
                                BYTE cascadeType,
                                int MSDStatus,
                                unsigned int channelHandle,
                                CCapH323* pRemoteCap,
                                CComModeH323* pCurrentScm,
                                BYTE bLateReleaseResourcesInConfCntl)
{
	CSegment* seg = new CSegment;

	BYTE isRmtCap     = pRemoteCap ? 1 : 0;
	BYTE isCurrentScm = pCurrentScm ? 1 : 0;

	*seg << PartyId
	     << H323PARTYCONNECT
	     << status
	     << cascadeType
	     << (WORD)MSDStatus
	     << (DWORD)channelHandle
	     << isRmtCap;


	if (isRmtCap)
		pRemoteCap->Serialize(NATIVE, *seg);

	*seg << isCurrentScm;
	if (isCurrentScm)
		pCurrentScm->Serialize(NATIVE, *seg);

	*seg << bLateReleaseResourcesInConfCntl;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::H323PartyConnectAll(PartyRsrcID PartyId, CComModeH323* pCurrentScm, DWORD videoRate, CCapH323* remoteCap, BYTE isCodianVcr, unsigned int channelHandle)
{
	CSegment* seg = new CSegment;

	*seg << PartyId
	     << H323PARTYCONNECTALL;

	pCurrentScm->Serialize(NATIVE, *seg);
	*seg << videoRate;
	*seg << isCodianVcr;
	*seg << channelHandle;
	remoteCap->Serialize(NATIVE, *seg);

//#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
//	if (m_pLocalRcvMbx)
//		SendLocalMessage(seg, PARTY_MSG);
//	else
//		SendMsg(seg, PARTY_MSG);
//#else
	SendMsg(seg, PARTY_MSG);
//#endif
}

//--------------------------------------------------------------------------
void CConfApi::AddProtocolToH323Party(PartyRsrcID PartyId, CSegment* pParam)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << ADDPROTOCOL
	     << *pParam;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::RemoveProtocolFromH323Party(PartyRsrcID PartyId, CSegment* pParam)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << REMOVEPROTOCOL
	     << *pParam;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateLocalCapsInConfLevel(PartyRsrcID PartyId, CCapH323& localCaps)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << UPDATE_CAPS;

	localCaps.Serialize(NATIVE, *seg);

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdatePartyH323VideoBitRate(PartyRsrcID PartyId, DWORD newBitRate, WORD channelDirection, WORD eRole)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << UPDATE_VIDEO_RATE
	     << newBitRate
	     << channelDirection
	     << eRole;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdatePartyLprVideoBitRate(PartyRsrcID PartyId,
                                          DWORD newPeopleRate,
                                          WORD channelDirection,
                                          DWORD lossProtection,
                                          DWORD mtbf,
                                          DWORD congestionCeiling,
                                          DWORD fill,
                                          DWORD modeTimeout,
                                          DWORD totalVideoRate,
                                          DWORD RemoteIdent)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << LPR_CHANGE_RATE
	     << newPeopleRate
	     << channelDirection
	     << lossProtection
	     << mtbf
	     << congestionCeiling
	     << fill
	     << modeTimeout
	     << totalVideoRate
	     << RemoteIdent;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
//LYNC2013_FEC_RED:
void CConfApi::UpdatePartyControlRegardingChangeOfTxVideoCausedByFecOrRed(CIpComMode* pNewMode, PartyRsrcID PartyId, DWORD newPeopleRate, DWORD type, BYTE isVidoOutMuted)
{
	TRACEINTO << "LYNC2013_FEC_RED";

	CSegment* seg = new CSegment;

	BYTE isThereNewMode = TRUE;

	if (!pNewMode)
		isThereNewMode = FALSE;

	*seg << PartyId
	     << FEC_RED_CHANGE_RATE
	     << newPeopleRate
	     << type
	     << isVidoOutMuted
	     << isThereNewMode;

	if (isThereNewMode)
		pNewMode->Serialize(NATIVE, *seg);

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::CleanBitRateLimitation(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << CLEAN_VIDEO_RATE_LIMIT;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendSiteAndVisualNamePlusProductIdToPartyControl(const CTaskApp* pParty,
                                                                BYTE bIsVisualName,
                                                                DWORD len,
                                                                char* siteName,
                                                                BYTE bIsProductId,
                                                                DWORD len2,
                                                                char* productId,
                                                                BYTE bIsVersionId,
                                                                DWORD len3,
                                                                char* VersionId,
                                                                eTelePresencePartyType eLocalTelePresencePartyType,
                                                                BYTE isRemoteCopMCU/*=FALSE*/,
                                                                RemoteIdent eRemoteVendorIdent/*=Regular*/)
{
	if (bIsVisualName || bIsProductId || bIsVersionId)
	{
		CSegment* pSeg = new CSegment;
		*pSeg << (DWORD)pParty
		      << UPDATEVISUALNAME
		      << (BYTE)isRemoteCopMCU
		      << len
		      << len2
		      << len3;

		if (bIsVisualName && len)
			pSeg->Put((unsigned char*)siteName, len);

		if (bIsProductId && len2)
			pSeg->Put((unsigned char*)productId, len2);

		if (bIsVersionId && len3)
			pSeg->Put((unsigned char*)VersionId, len3);

		// speakerIndication
		*pSeg << (BYTE)eLocalTelePresencePartyType
			  << (BYTE)eRemoteVendorIdent;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
		if (m_pLocalRcvMbx)
			SendLocalMessage(pSeg, PARTY_MSG);
		else
			SendMsg(pSeg, PARTY_MSG);
#else
		SendMsg(pSeg, PARTY_MSG);
#endif
	}
}

//--------------------------------------------------------------------------
void CConfApi::SiteAndVisualName(PartyMonitorID PartyId, char* siteName)
{
	WORD len = strlen(siteName);

	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << len;

	if (len)
		pSeg->Put((unsigned char*)siteName, len);

	SendMsg(pSeg, UPDATEVISUALNAME);
}

//--------------------------------------------------------------------------
// No - means unmute
// YES - means mute.
// AUTO - means unchanged
void CConfApi::IpMuteMedia(PartyRsrcID PartyId,
                           BYTE audioInMute,
                           BYTE audioOutMute,
                           BYTE VideoInMute,
                           BYTE VideoOutMute,
                           BYTE ContentInMute,
                           BYTE ContentOutMute,
                           BYTE FeccInMute,
                           BYTE FeccOutMute)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << IP_MUTE_MEDIA
	      << audioInMute
	      << audioOutMute
	      << VideoInMute
	      << VideoOutMute
	      << ContentInMute
	      << ContentOutMute
	      << FeccInMute
	      << FeccOutMute;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::IpUpdateBridges(PartyRsrcID PartyId, CIpComMode* pIpScm, cmCapDataType mediaType, cmCapDirection mediaDirection)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << IPPARTYUPDATEBRIDGES;

	pIpScm->Serialize(NATIVE, *pSeg);

	*pSeg << (WORD)mediaType
	      << (WORD)mediaDirection;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SipPartyChannelsConnected(PartyRsrcID PartyId, CIpComMode* pCurrentScm, unsigned int* channelHandle)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << SIP_PARTY_CHANS_CONNECTED;

	pCurrentScm->Serialize(NATIVE, *pSeg);
	*pSeg << channelHandle[0];
	*pSeg << channelHandle[1];

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SipPartyRemoteCapsRecieved(PartyRsrcID PartyId, CSipCaps* pRemoteCaps, CIpComMode* pCurrentScm)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << PARTYRMTCAP;

	pCurrentScm->Serialize(NATIVE, *pSeg);
	pRemoteCaps->Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SipPartyRemoteConnected(PartyRsrcID PartyId, CIpComMode* pCurrentScm, BYTE bIsEndConfChangeMode)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << IPPARTYCONNECTED;

	pCurrentScm->Serialize(NATIVE, *pSeg);
	*pSeg << bIsEndConfChangeMode;

	SendMsg(pSeg, PARTY_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::SipPartySendChannelHandle(PartyRsrcID PartyId, unsigned int incomingVideoChannelHandle, unsigned int outgoingVideoChannelHandle)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << SIP_PARTY_SEND_CHANNEL_HANDLE;
	*pSeg << incomingVideoChannelHandle;
	*pSeg << outgoingVideoChannelHandle;

	SendMsg(pSeg, PARTY_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::SipPartySendFallBackToSIP(PartyRsrcID PartyId, CIpComMode* pCurrentScm)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << FALL_BACK_FROM_TIP_TO_REGULAR_SIP;

	pCurrentScm->Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendFallBckToReglarPartyToConf(const char* name, CIpComMode* pCurrentScm)
{
	CSegment* seg = new CSegment;
	*seg << name;

	pCurrentScm->Serialize(NATIVE, *seg);

	SendMsg(seg, FALL_BACK_FROM_TIP_TO_REGULAR_SIP);
}

//--------------------------------------------------------------------------
void CConfApi::SendPartyReceiveEcsToPartyControl(PartyRsrcID PartyId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << PARTY_RECEIVE_ECS;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendFaultyMfaNoticeToPartyCntl(PartyRsrcID PartyId,
                                              DWORD reason,
                                              BYTE mipHwConn,
                                              BYTE mipMedia,
                                              BYTE mipDirect,
                                              BYTE mipTimerStat,
                                              BYTE mipAction)
{
	CSegment* pSeg = new CSegment;

	*pSeg << PartyId
	      << PARTY_FAULTY_RSRC
	      << reason
	      << mipHwConn;

	if (mipHwConn)
	{
		*pSeg << mipMedia
		      << mipDirect
		      << mipTimerStat
		      << mipAction;
	}

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
// SIP EP's Re-Invite Ind or EP's 200OK of Re-Invite Req, or EP's Ack of Reinvite ind no sdp.
void CConfApi::SendPartyReceiveReCapsToPartyControl(PartyRsrcID PartyId, CSipCaps* pCaps, CIpComMode* pBestMode,WORD bisFallBack)
{
	BYTE bIsRmtCap   = (pCaps != NULL);
	BYTE bIsBestMode = (pBestMode != NULL);

	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << REMOTE_SENT_RE_CAPS
	      << bIsRmtCap;

	if (bIsRmtCap)
		pCaps->Serialize(NATIVE, *pSeg);

	*pSeg << bIsBestMode;
	if (bIsBestMode)
		pBestMode->Serialize(NATIVE, *pSeg);
	*pSeg << bisFallBack;

	SendMsg(pSeg, PARTY_MSG);
}

//////////////////////////////////////////////////////////////////////////////////////
void CConfApi::UpdatePartyControlOnRemoteCaps(PartyRsrcID PartyId,CSipCaps* pCaps)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
		  << UPDATE_REMOTE_CAPS_FROM_PARTY;

		  if (pCaps)
		  		pCaps->Serialize(NATIVE, *pSeg);

		  SendMsg(pSeg, PARTY_MSG);

}
//--------------------------------------------------------------------------
void CConfApi::UpdatePartyControlOnNewRate(PartyRsrcID PartyId, DWORD newBitRate, WORD channelDir, WORD roleLabel)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << UPDATE_VIDEO_RATE
	      << newBitRate
	      << channelDir
	      << roleLabel;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendPartyInConfIndToPartyCntl(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << (WORD)PARTY_IN_CONF_IND;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(seg, PARTY_MSG);
	else
		SendMsg(seg, PARTY_MSG);
#else
	SendMsg(seg, PARTY_MSG);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::PartyEndInitCom(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << PARTYENDINITCOM;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::IsdnPartyConnect(PartyRsrcID PartyId, CCapH320& remoteCap, CComMode& currentScm, WORD status, BYTE isEncryptionSetupDone)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << PARTYCONNECT
	     << status;

	remoteCap.Serialize(NATIVE, *seg);
	currentScm.Serialize(NATIVE, *seg);

	*seg << isEncryptionSetupDone;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyRemoteXferMode(PartyRsrcID PartyId, CComMode& remoteScm)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << RMTXFRMODE;

	remoteScm.Serialize(NATIVE, *seg);

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyReceivedFullCapSet(PartyRsrcID PartyId, CCapH320& remoteCap, CComMode& currentRmtScm)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << ALLRMTCAPSRECEIVED;

	remoteCap.Serialize(NATIVE, *seg);
	currentRmtScm.Serialize(NATIVE, *seg);

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::EndAudBrdgConnect(WORD status)
{
	CSegment* seg = new CSegment;
	*seg << status;
	SendLocalMessage(seg, AUDBRDGCONNECT);
}

//--------------------------------------------------------------------------
void CConfApi::EndAudBrdgDisConnect(WORD status)
{
	CSegment* seg = new CSegment;
	*seg << status;
	SendLocalMessage(seg, AUDBRDGDISCONNECT);
}

//--------------------------------------------------------------------------
void CConfApi::SetAudioVolume(const char* partyName, BYTE volume, EMediaDirection bVolumeInOut)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)UPDATE_AUDIO_VOLUME
	     << partyName
	     << (BYTE)bVolumeInOut
	     << (DWORD)volume;

	SendMsg(seg, AUDIO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateAGCExecFlag(const char* partyName, const WORD onOff)
{
	DBGPASSERT_AND_RETURN(!partyName);

	WORD mediaCntl = 0;
	WORD mediaMsg  = 0;

	mediaCntl = AUDIO_BRIDGE_MSG;
	mediaMsg  = UPDATE_AGC_EXEC;

	CSegment* seg = new CSegment;
	*seg<< (OPCODE)mediaMsg
	    << partyName
	    << (BYTE)onOff;

	SendMsg(seg, mediaCntl);
}

//--------------------------------------------------------------------------
void CConfApi::MuteMedia(const char* partyName, const EOnOff eOnOff, DWORD mediaMask)
{
	CSegment* pSeg = new CSegment;

	WORD mediaCntl = 0;
	WORD mediaMsg  = 0;

	if (mediaMask & 0x00000001)
	{
		mediaCntl = AUDIO_BRIDGE_MSG;
		mediaMsg  = AUDIOMUTE;
	}

	else if (mediaMask & 0x00000002)
	{
		mediaCntl = VIDEO_BRIDGE_MSG;
		mediaMsg  = VIDEOMUTE;
	}
	else
	{
		POBJDELETE(pSeg);
		return;
	}

	*pSeg << (OPCODE)mediaMsg
	      << OPERATOR
	      << partyName
	      << (BYTE)eOnOff
	      << (BYTE)eMediaIn;

	SendMsg(pSeg, mediaCntl);
}

//--------------------------------------------------------------------------
void CConfApi::SendCGStartContent(const char* partyName)
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		TRACEINTO << "Failed, The system is not a 'Call Generator'";
		return;
	}

	CSegment* seg = new CSegment;
	*seg << partyName;

	SendMsg(seg, PARTY_CG_START_CONTENT);
}

//--------------------------------------------------------------------------
void CConfApi::SendCGStopContent(const char* partyName)
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		TRACEINTO << "Failed, The system is not a 'Call Generator'";
		return;
	}

	CSegment* seg = new CSegment;
	*seg << partyName;

	SendMsg(seg, PARTY_CG_STOP_CONTENT);
}

//--------------------------------------------------------------------------
void CConfApi::BlockMedia(const char* partyName, const EOnOff eOnOff, DWORD mediaMask)
{
	CSegment* pSeg = new CSegment;

	WORD mediaCntl = 0;
	WORD mediaMsg  = 0;

	if (mediaMask & 0x00000001)
	{
		mediaCntl = AUDIO_BRIDGE_MSG;
		mediaMsg  = AUDIOMUTE;
	}
	else
	{
		POBJDELETE(pSeg);
		return;
	}

	*pSeg<< (OPCODE)mediaMsg
	     << OPERATOR
	     << partyName
	     << (BYTE)eOnOff
	     << (BYTE)eMediaOut;

	SendMsg(pSeg, mediaCntl);
}

//--------------------------------------------------------------------------
void CConfApi::ReSendOpenConf(WORD new_card_board_id)
{
	WORD  mediaCntl = 0;
	DWORD mediaMsg  = 0;

	mediaCntl = AUDIO_BRIDGE_MSG;
	mediaMsg  = AC_OPEN_CONF_RESEND;

	CSegment* seg = new CSegment;

	*seg << (OPCODE)mediaMsg << new_card_board_id;

	SendMsg(seg, mediaCntl);
}

//--------------------------------------------------------------------------
void CConfApi::StartLookForActiveSpeaker()
{
	CSegment* seg = new CSegment;

	*seg<< (OPCODE)START_LOOK_FOR_ACTIVE_SPEAKER;

	SendMsg(seg, AUDIO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SpeakersChanged(CTaskApp* pPartyVideo, CTaskApp* pPartyAudio, DWORD partyDominantpeakerMSI)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SPEAKERS_CHANGED
	     << (DWORD)pPartyVideo
	     << (DWORD)pPartyAudio
			<< (BYTE)NO
			<< partyDominantpeakerMSI;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::EndVidBrdgConnect(WORD status)
{
	CSegment* seg = new CSegment;
	*seg << status;
	SendLocalMessage(seg, VIDBRDGCONNECT);
}

//--------------------------------------------------------------------------
void CConfApi::EndVidBrdgDisConnect(WORD status)
{
	CSegment* seg = new CSegment;
	*seg << status;
	SendLocalMessage(seg, VIDBRDGDISCONNECT);
}

//--------------------------------------------------------------------------
void CConfApi::PartyVideoInSynced(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status)
{
	SendBrdgPartyCntlMsg(pParty, brdgOpcode, VIDEO_IN_SYNCED, status);
}

//--------------------------------------------------------------------------
void CConfApi::SetAutoScanInterval(WORD intervalValue)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SET_AUTOSCAN_INTERVAL
	     << intervalValue;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetAutoScanOrder(CAutoScanOrder* pAutoScanOrder)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SET_AUTOSCAN_ORDER;

	pAutoScanOrder->Serialize(NATIVE, *seg);

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetVideoConfLayoutSeeMeAll(CVideoLayout& layout, BYTE bAnyway /*= 0*/)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SETCONFVIDLAYOUT_SEEMEALL;
	layout.Serialize(NATIVE, *seg);
	*seg << bAnyway;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateMessageOverlay(CMessageOverlayInfo* pMessageOverlayInfo)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SET_MESSAGE_OVERLAY;
	pMessageOverlayInfo->Serialize(NATIVE, *seg);

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateSiteName(CSiteNameInfo* pSiteNameInfo)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SET_SITE_NAME;
	pSiteNameInfo->Serialize(NATIVE, *seg);

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::RefreshLayout()
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)REFRESHLAYOUT;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateMessageOverlayParty(const char* SrcPartyName, CMessageOverlayInfo* pMessageOverlayInfo)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SET_PARTY_MESSAGE_OVERLAY
	     << SrcPartyName;

	pMessageOverlayInfo->Serialize(NATIVE, *seg);

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
// VNGR-26449 - unencrypted conference message
void CConfApi::UpdateSecureMesaageToAllParties()
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SET_ALL_PARTIES_SECURE_MESSAGE;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
// VNGR-26449 - unencrypted conference message
void CConfApi::StopSecureMessageToParty(const char* PartyName)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)STOP_PARTY_SECURE_MESSAGE
	     << PartyName;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetVideoConfLayoutSeeMeParty(const char* SrcPartyName, CVideoLayout& layout)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SETCONFVIDLAYOUT_SEEMEPARTY
	     << SrcPartyName;

	layout.Serialize(NATIVE, *seg);

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetVideoPrivateLayout(const char* SrcPartyName, CVideoLayout& layout, BOOL isMcmsAction)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SETPRIVATEVIDLAYOUT
	     << isMcmsAction
	     << SrcPartyName;

	layout.Serialize(NATIVE, *seg);

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetVideoPrivateLayoutButtonOnly(const char* SrcPartyName, WORD isPrivate)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SETPRIVATEVIDLAYOUTONOFF
	     << (BYTE)isPrivate
	     << SrcPartyName;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateLectureMode(CLectureModeParams* pLectureMode)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (OPCODE)UPDATELECTUREMODE;
	pLectureMode->Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateContentLectureMode(BOOL yesNo)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (OPCODE)UPDATEONLECTUREMODE;
	*pSeg << (BYTE)yesNo;

	SendMsg(pSeg, CONTENTCNTL_MSG);
}


//--------------------------------------------------------------------------
void CConfApi::UpdateAutoLayout(BYTE onOff)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)UPDATEAUTOLAYOUT
	     << (WORD)onOff;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateVideoClarity(BYTE onOff)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)UPDATE_VIDEO_CLARITY
	     << (WORD)onOff;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateVideoBridgeFlowControlRate(PartyRsrcID PartyId, DWORD newBitRate, WORD channelDirection, WORD eRole, BYTE bIsCascade, CSegment* pLprParams)
{
	CSegment* seg = new CSegment;
	*seg << (WORD)UPDATE_VIDEO_FLOW_CNTL_RATE
	     << PartyId
	     << newBitRate
	     << channelDirection
	     << eRole
	     << bIsCascade;

	BOOL isExtParams = FALSE;
	if (NULL != pLprParams)
		isExtParams = TRUE;

	*seg << isExtParams;
	if (isExtParams)
		*seg << *pLprParams;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateRemoteUseSmartSwitchAccordingToVendor(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (WORD)COP_USE_SMART_SWITCH_ACCORDING_TO_VENDOR
	     << (DWORD)pParty;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ContentDecoderVideoInSynced(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status)
{
	SendBrdgPartyCntlMsg(pParty, brdgOpcode, CONTENT_DECODER_VIDEO_IN_SYNCED, status);
}

//--------------------------------------------------------------------------
void CConfApi::ContenetDecoderSyncLost(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status)
{
	SendBrdgPartyCntlMsg(pParty, brdgOpcode, CONTENT_DECODER_SYNC_LOST, status);
}

//--------------------------------------------------------------------------
void CConfApi::ContenetDecoderRsrcAllocFailure(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status)
{
	SendBrdgPartyCntlMsg(pParty, brdgOpcode, CONTENT_DECODER_ALLOC_RSRC_FAIL, status);
}

//--------------------------------------------------------------------------
void CConfApi::ContentDecoderResetFailStatus(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status)
{
	SendBrdgPartyCntlMsg(pParty, brdgOpcode, CONTENT_DECODER_RESET_FAIL_STATUS, status);
}

//--------------------------------------------------------------------------
void CConfApi::ContentDecoderDisconnected(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status)
{
	SendBrdgPartyCntlMsg(pParty, brdgOpcode, CONTENT_DECODER_DISCONNECTED, status);
}

//--------------------------------------------------------------------------
void CConfApi::DataTokenRequest(CTaskApp* pParty, WORD bitRate, WORD isCameraControl)
{
	CSegment* seg = new CSegment;
	*seg << DATATOKENREQ
	     << (DWORD)pParty
	     << bitRate
	     << isCameraControl;

	SendMsg(seg, FECC_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::DataTokenRequestAccept(CTaskApp* pParty)
{
	SendDataCntlMsg(pParty, DATATOKENACCEPT);
}

//--------------------------------------------------------------------------
void CConfApi::DataTokenRequestReject(CTaskApp* pParty)
{
	SendDataCntlMsg(pParty, DATATOKENREJECT);
}

//--------------------------------------------------------------------------
void CConfApi::DataTokenRelease(CTaskApp* pParty, WORD isCameraControl)
{
	SendDataCntlMsg(pParty, DATATOKENRELEASE, isCameraControl);
}

//--------------------------------------------------------------------------
void CConfApi::DataTokenReleaseAndFree(CTaskApp* pParty, WORD isCameraControl)
{
	SendDataCntlMsg(pParty, DATATOKENRELEASEFREE, isCameraControl);
}

//--------------------------------------------------------------------------
void CConfApi::DataTokenWithdrawAndFree()
{
	SendDataCntlMsg(NULL, DATATOKENWITHDRAWANDFREE);
}

//--------------------------------------------------------------------------
void CConfApi::SendDataCntlMsg(CTaskApp* pParty, WORD opCode, WORD isCameraControl)
{
	CSegment* seg = new CSegment;
	*seg << opCode
	     << (DWORD)pParty
	     << isCameraControl;

	SendMsg(seg, FECC_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::EndFECCBrdgConnect(WORD status)
{
	CSegment* seg = new CSegment;
	*seg << status;
	SendLocalMessage(seg, FECCBRDGCONNECT);
}

//--------------------------------------------------------------------------
void CConfApi::EndFECCBrdgDisConnect(WORD status)
{
	CSegment* seg = new CSegment;
	*seg << status;
	SendLocalMessage(seg, FECCBRDGDISCONNECT);
}

//--------------------------------------------------------------------------
void CConfApi::FeccKeyMsg(DWORD partyRsrcId, WORD msg)
{
	CSegment* seg = new CSegment;
	*seg << FECC_KEY_IND
	     << partyRsrcId
	     << msg;

	SendMsg(seg, PCM_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyVideoBridgeImageInCellZeroChanged(OPCODE brdgOpcode, const CTaskApp* pParty, const CTaskApp* pSrcParty)
{
	CSegment* seg = new CSegment;
	*seg << (WORD)IMAGE_IN_CELL_ZERO_CHANGED
	     << (DWORD)pParty
	     << (DWORD)pSrcParty;

	SendLocalBrdgMsg(seg, brdgOpcode);
}

//--------------------------------------------------------------------------
/*
void CConfApi::SendBrdgPartyCntlMsg(const CTaskApp* pParty,
                                    OPCODE brdgOpcode,
                                    WORD opcode,
                                    WORD status,
                                    EMediaDirection eMediaDirection,
                                    CSegment* pParams)
{
	CSegment* seg = NULL;

	switch (opcode)
	{
		case ENDCONNECTPARTY:
		case ENDDISCONNECTPARTY:
		case ENDCONNECTPARTY_IVR_MODE:
		{
			if (!IsValidPObjectPtr(pParty))
				delete seg;
			
			PASSERT_AND_RETURN(!IsValidPObjectPtr(pParty)); // BRIDGE-3413

			seg = new CSegment;
			*seg << opcode
			     << pParty->GetPartyId() // D.K. Send party Id instead of pointer
			     << status
			     << (WORD)eMediaDirection;
			break;
		}
		default:
		{
			seg = new CSegment;
			*seg << opcode
			     << (DWORD)pParty
			     << status
			     << (WORD)eMediaDirection;
			break;
		}
	}

	if (pParams)
		*seg << *pParams;

	SendLocalBrdgMsg(seg, brdgOpcode);
}
*/

//--------------------------------------------------------------------------
void CConfApi::SendBrdgPartyCntlMsg(const CTaskApp* pParty,
                                    OPCODE brdgOpcode,
                                    WORD opcode,
                                    WORD status,
                                    EMediaDirection eMediaDirection,
                                    CSegment* pParams)
{
	CSegment* seg = new CSegment;

	*seg << opcode
			<< (DWORD)pParty
			<< status
			<< (WORD)eMediaDirection;


	if (pParams)
		*seg << *pParams;

	SendLocalBrdgMsg(seg, brdgOpcode);
}

//--------------------------------------------------------------------------
void CConfApi::SendBrdgPartyCntlMsgById(PartyRsrcID partyID,
                                    OPCODE brdgOpcode,
                                    WORD opcode,
                                    WORD status,
                                    EMediaDirection eMediaDirection,
                                    CSegment* pParams)
{
	CSegment* seg = new CSegment;

	*seg << opcode
		<< partyID
		<< status
		<< (WORD)eMediaDirection;

	if (pParams)
		*seg << *pParams;

	SendLocalBrdgMsg(seg, brdgOpcode);
}


//--------------------------------------------------------------------------
void CConfApi::SendLocalBrdgMsg(CSegment* seg, OPCODE brdgOpcode)
{
	switch (brdgOpcode)
	{
		case  VIDEO_BRIDGE_MSG:     // messages to Video Bridge
		{
			SendLocalMessage(seg, VIDEO_BRIDGE_MSG);
				break;
			}
			case XCODE_BRDG_MSG:
			{
				SendLocalMessage(seg,XCODE_BRDG_MSG);
			break;
		}
		case AUDIO_BRIDGE_MSG:
		{
			SendLocalMessage(seg, AUDIO_BRIDGE_MSG);
			break;
		}
		case CONTENTCNTL_MSG:
		{
			SendLocalMessage(seg, CONTENTCNTL_MSG);
			break;
		}
		case FECC_BRIDGE_MSG:
		{
			SendLocalMessage(seg, FECC_BRIDGE_MSG);
			break;
		}
		default:
		{
			PASSERT(1);
			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
void CConfApi::LayoutChangedNotification(const char* pPartyName)
{
	if (pPartyName)
	{
		CSegment* pSeg = new CSegment;
		*pSeg << pPartyName;
		SendLocalMessage(pSeg, PARTYLAYOUTCHANGED);
	}
}

// --------------------------------------------------------------------------
void CConfApi::IvrPartyNotification(
	DWORD partyID,
	CTaskApp* pParty,
	const char* pPartyName,
	OPCODE notifyOpcode,
	CSegment* pParam,
	EMediaDirection eMediaDirection/* = eMediaInAndOut*/,
	const char* externalIvrFile/* = NULL*/)
{
	CSegment* pSeg = new CSegment;

	*pSeg
		<< (DWORD)EVENT_NOTIFY
		<< notifyOpcode
		<< partyID
		<< (void*)pParty
		<< pPartyName
		<< (WORD)eMediaDirection;

	if (notifyOpcode == PARTY_VIDEO_IVR_MODE_CONNECTED)
	{
		std::string external_file = "";
		if (externalIvrFile != NULL)
			external_file = externalIvrFile;
		*pSeg << externalIvrFile;
	}

	if (pParam)
		*pSeg << (*pParam);

	if ((SIGNALLING_DTMF_INPUT_IND == notifyOpcode) || (RTP_DTMF_INPUT_IND == notifyOpcode))
		SendMsg(pSeg, CONFAPP_MNGR_MSG);          // Send message to external conference mailbox
	else
		SendLocalMessage(pSeg, CONFAPP_MNGR_MSG); // Send message to local conference mailbox
}

//--------------------------------------------------------------------------
void CConfApi::IvrConfNotification(OPCODE notifyOpcode, CSegment *pParam)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)EVENT_NOTIFY
	      << (OPCODE)notifyOpcode
	      << (DWORD)INVALID;  //we always send conf msgs to cam with invalid instead of partyRsrcId

	if(pParam)
		*pSeg << (*pParam);

	SendMsg(pSeg,CONFAPP_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdatePartyIvrStatus(PartyRsrcID PartyId, WORD status)
{
	CSegment* pSeg = new CSegment;

	*pSeg << PartyId
	      << status;

	SendLocalMessage(pSeg, IVR_PARTY_UPDATE_STATUS);   // Send message to local conference mailbox
}

// ---------------------------------------------------------------------------
// The function starts play a message for a specific party
// If the party currently is in other mode , then MSG mode
// then go to the MSG mode .Elsewere ( if the party currently
// in mode MSG , just change the message
// Parameters:
// wMessPhysId - name of the message loaded in the card ( not logic, physic)
// wMessMode	- functio
void CConfApi::StartMessage(PartyRsrcID PartyId,
                            const char* msgFullPath,
                            WORD msgDuration,
                            WORD msgCheckSum,
                            WORD wMessMode,
                            WORD cachePriority,
                            WORD repeated_time_interval)
{
	TRACEINTO << "PartyId:" << PartyId << ", wMessMode:" << wMessMode << ", msgFullPath:" << (msgFullPath ? msgFullPath : "NULL");

	CIVRPlayMessage*       pIVRPlayMessage = new CIVRPlayMessage;
	SIVRPlayMessageStruct* playMsg         = &(pIVRPlayMessage->play);

	playMsg->partyOrconfFlag  = IVR_PLAY_MSG_TO_PARTY; // /EVENT_PARTY_REQUEST;
	playMsg->stopPrevOrAppend = IVR_STOP_PREV_MSG;
	playMsg->mediaType        = IVR_MEDIA_TYPE_AUDIO;

	if (wMessMode == IVR_STATUS_PLAY_LOOP)
		playMsg->numOfRepetition = IVR_REPEAT_FOREVER;  // play loop
	else
		playMsg->numOfRepetition = wMessMode;

	if (playMsg->numOfRepetition > 1)
		playMsg->numOfMediaFiles = 2; // message + silence message for repeated_time_interval seconds
	else
		playMsg->numOfMediaFiles = 1;

	playMsg->mediaFiles = new SIVRMediaFileParamsStruct[playMsg->numOfMediaFiles];
	memset((BYTE*)(playMsg->mediaFiles), 0, sizeof(SIVRMediaFileParamsStruct));
	playMsg->mediaFiles[0].actionType   = IVR_ACTION_TYPE_PLAY;
	playMsg->mediaFiles[0].duration     = msgDuration;
	playMsg->mediaFiles[0].checksum     = msgCheckSum;
	playMsg->mediaFiles[0].frequentness = cachePriority;

	if (playMsg->numOfMediaFiles == 2)
	{
		playMsg->mediaFiles[1].actionType     = IVR_ACTION_TYPE_SILENCE;
		playMsg->mediaFiles[1].duration       = repeated_time_interval;
		playMsg->mediaFiles[1].playMode       = 0;
		playMsg->mediaFiles[1].checksum       = 0;
		playMsg->mediaFiles[1].frequentness   = 0;
		playMsg->mediaFiles[1].fileNameLength = 0;
		playMsg->mediaFiles[1].fileName[0]    = '\0';
	}

	if (msgFullPath)
	{
		if (0 == strncmp(msgFullPath, "Cfg/", 4))
		{
			playMsg->mediaFiles[0].fileNameLength = strlen(msgFullPath)-4;
			strncpy(playMsg->mediaFiles[0].fileName, msgFullPath+4, sizeof(playMsg->mediaFiles[0].fileName) - 1);
			playMsg->mediaFiles[0].fileName[sizeof(playMsg->mediaFiles[0].fileName) - 1] = '\0';
		}
		else
		{

			playMsg->mediaFiles[0].fileNameLength = strlen(msgFullPath);
			if (0 != strlen(msgFullPath))
			{
				strncpy(playMsg->mediaFiles[0].fileName, msgFullPath, sizeof(playMsg->mediaFiles[0].fileName) - 1);
				playMsg->mediaFiles[0].fileName[sizeof(playMsg->mediaFiles[0].fileName) - 1] = '\0';
			}
			else
			{
				playMsg->mediaFiles[0].fileName[0] = '\0';
			}
		}
	}

	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PLAY_MSG;
	*seg << PartyId;
	pIVRPlayMessage->Serialize(seg);

	SendMsg(seg, CONFAPP_MNGR_MSG);

	PDELETEA(playMsg->mediaFiles);
	PDELETE(pIVRPlayMessage);
}

void CConfApi::StartMessageTipAux(PartyRsrcID PartyId, CSegment* pParam)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PLAY_MSG;
	*seg << PartyId;
	*seg << *pParam;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}
//---------------------------------------------------------------------------
void CConfApi::StartStopPLC(PartyRsrcID PartyId, WORD onOff)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PARTY_REQUEST;
	*seg << (DWORD)eCAM_EVENT_PARTY_PLC;
	*seg << PartyId;
	*seg << (DWORD)onOff; // on

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//---------------------------------------------------------------------------
void CConfApi::PlcAction(PartyRsrcID PartyId, DWORD opcode, DWORD layoutCode, BYTE cellNumber)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PARTY_REQUEST;
	*seg << opcode;
	*seg << PartyId;
	*seg << layoutCode;
	*seg << cellNumber;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//---------------------------------------------------------------------------
void CConfApi::StartStopVenus(PartyRsrcID PartyId, WORD onOff)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PARTY_REQUEST;
	*seg << (DWORD)eCAM_EVENT_PARTY_VENUS;
	*seg << PartyId;
	*seg << (DWORD)onOff; // on

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//---------------------------------------------------------------------------
void CConfApi::VenusAction(PartyRsrcID PartyId, DWORD opcode, DWORD layoutCode, BYTE cellNumber)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PARTY_REQUEST;
	*seg << opcode;
	*seg << PartyId;
	*seg << layoutCode;
	*seg << cellNumber;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::NotifyCAM(PartyRsrcID PartyId, DWORD opcode, DWORD uniqueNumber)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_NOTIFY;
	*seg << opcode;
	*seg << PartyId;
	*seg << uniqueNumber; // on

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(seg, CONFAPP_MNGR_MSG);
	else
		SendMsg(seg, CONFAPP_MNGR_MSG);
#else
	SendMsg(seg, CONFAPP_MNGR_MSG);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::NotifyCAMTimer(PartyRsrcID PartyId, DWORD opcode, DWORD featureType, DWORD uniqueNumber, DWORD originalOpcode)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_NOTIFY;
	*seg << opcode;
	*seg << featureType;
	*seg << PartyId;
	*seg << uniqueNumber;
	*seg << originalOpcode;

#ifndef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(seg, CONFAPP_MNGR_MSG);
	else
		SendMsg(seg, CONFAPP_MNGR_MSG);
#else
	SendMsg(seg, CONFAPP_MNGR_MSG);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::EndOfIVRSession(PartyRsrcID PartyId, BYTE isLeader)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_NOTIFY;
	*seg << (DWORD)eCAM_EVENT_PARTY_END_IVR;
	*seg << PartyId;
	*seg << isLeader;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(seg, CONFAPP_MNGR_MSG);
	else
		SendMsg(seg, CONFAPP_MNGR_MSG);
#else
	SendMsg(seg, CONFAPP_MNGR_MSG);
#endif
}

//---------------------------------------------------------------------------
void CConfApi::SendCAMGeneralActionCommand(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode, DWORD param)
{
	CSegment*  seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << confOrPartyAction;
	*seg << opcode;
	*seg << PartyId;
	*seg << param;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//---------------------------------------------------------------------------
void CConfApi::SendCAMAddFeatureToList(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode, DWORD param)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << confOrPartyAction;
	*seg << opcode;
	*seg << PartyId;
	*seg << param;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//---------------------------------------------------------------------------
void CConfApi::SendCAMRemoveFeatureFromList(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode, DWORD param)
{
	CSegment*  seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << confOrPartyAction;
	*seg << opcode;
	*seg << PartyId;
	*seg << param;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//---------------------------------------------------------------------------
void CConfApi::SendCAMGeneralActionCommandSeg(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode, CSegment* pSeg)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << confOrPartyAction;
	*seg << opcode;
	*seg << PartyId;
	*seg << *pSeg;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendCAMGeneralNotifyCommand(PartyRsrcID PartyId, DWORD opcode, CSegment* pParam)
{
	CSegment*  seg = new CSegment;
	*seg << (DWORD)EVENT_NOTIFY;
	*seg << opcode;
	*seg << PartyId;
	*seg << *pParam;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(seg, CONFAPP_MNGR_MSG);
	else
		SendMsg(seg, CONFAPP_MNGR_MSG);
#else
	SendMsg(seg, CONFAPP_MNGR_MSG);
#endif
}

//--------------------------------------------------------------------------
void CConfApi::SendCAMGeneralNotifyCommand(PartyRsrcID PartyId, DWORD opcode, DWORD param)
{
	CSegment*  seg = new CSegment;
	*seg << (DWORD)EVENT_NOTIFY;
	*seg << opcode;
	*seg << PartyId;
	*seg << param;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(seg, CONFAPP_MNGR_MSG);
	else
		SendMsg(seg, CONFAPP_MNGR_MSG);
#else
	SendMsg(seg, CONFAPP_MNGR_MSG);
#endif
}

//---------------------------------------------------------------------------
void CConfApi::SetPartyAsLeader(const char* pPartyName, EOnOff eOnOff)
{
	CSegment*  seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PARTY_REQUEST;
	// bridge-1811
	*seg << (DWORD)eCAM_EVENT_SET_AS_LEADER_FROM_API;
	*seg << pPartyName;
	*seg << (DWORD)eOnOff;

#ifndef MESSAGE_QUEUE_BLOCKING_RESERCH
	if (m_pLocalRcvMbx)
		SendLocalMessage(seg, CONFAPP_MNGR_MSG);
	else
		SendMsg(seg, CONFAPP_MNGR_MSG);
#else
	SendMsg(seg, CONFAPP_MNGR_MSG);
#endif
}

//---------------------------------------------------------------------------
void CConfApi::AddPartyFeatureToWaitList(PartyRsrcID PartyId, DWORD uniqueNumber, DWORD opcode)
{
	CSegment*  seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PARTY_REQUEST;
	*seg << (DWORD)eCAM_EVENT_ADD_FEATURE_TO_WAIT_LIST;
	*seg << PartyId;
	*seg << uniqueNumber;
	*seg << opcode;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//---------------------------------------------------------------------------
void CConfApi::SendStartFeature(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode)
{
	SendCAMGeneralActionCommand(PartyId, confOrPartyAction, opcode, 1);
}

//---------------------------------------------------------------------------
void CConfApi::SendStopFeature(PartyRsrcID PartyId, DWORD confOrPartyAction, DWORD opcode)
{
	SendCAMGeneralActionCommand(PartyId, confOrPartyAction, opcode, 0);
}

//---------------------------------------------------------------------------
// The message stops message playing , but leaves the party
// in MSG mode . For passing the party in mode INCONF use
void CConfApi::StopMessage(PartyRsrcID PartyId, DWORD mediaType)
{
	CSegment*  seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_STOP_MSG;
	*seg << PartyId;
	*seg << mediaType;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//---------------------------------------------------------------------------
// This function starts playing a sequence of messages (messages numbers in the array)
// for a specific party.
void CConfApi::StartSequenceMessages(PartyRsrcID PartyId, const IVRMsgDescriptor* ArrayOfMessages, WORD SubMessagesNumber, WORD publicOrPrivate, WORD PlayMode)
{
	CIVRPlayMessage* pIVRPlayMessage = new CIVRPlayMessage;
	SIVRPlayMessageStruct* playMsg   = &(pIVRPlayMessage->play);

	if (PRIVATE_MSG == publicOrPrivate)
		playMsg->partyOrconfFlag = IVR_PLAY_MSG_TO_PARTY;   // /EVENT_PARTY_REQUEST;
	else
		playMsg->partyOrconfFlag = IVR_PLAY_MSG_TO_CONF;    // /EVENT_CONF_REQUEST;

	playMsg->stopPrevOrAppend = IVR_STOP_PREV_MSG;
	playMsg->mediaType        = IVR_MEDIA_TYPE_AUDIO;
	playMsg->numOfRepetition  = PlayMode;

	playMsg->numOfMediaFiles = SubMessagesNumber;
	playMsg->mediaFiles      = new SIVRMediaFileParamsStruct[SubMessagesNumber];
	memset((BYTE*)(playMsg->mediaFiles), 0, SubMessagesNumber*sizeof(SIVRMediaFileParamsStruct));

	for (int i = 0; i < SubMessagesNumber; i++)
	{
		playMsg->mediaFiles[i].actionType = IVR_ACTION_TYPE_PLAY;
		playMsg->mediaFiles[i].duration   = ArrayOfMessages[i].ivrMsgDuration;
		playMsg->mediaFiles[i].checksum   = ArrayOfMessages[i].ivrMsgCheckSum;

		if (NULL == ArrayOfMessages[i].ivrMsgFullPath)
			TRACEINTO << "ivrMsgFullPath is NULL for message #" << i;
		else if (0 == strncmp(ArrayOfMessages[i].ivrMsgFullPath, "Cfg/", 4))
		{
			playMsg->mediaFiles[i].fileNameLength = strlen(ArrayOfMessages[i].ivrMsgFullPath)-4;
			strncpy(playMsg->mediaFiles[i].fileName, (ArrayOfMessages[i].ivrMsgFullPath)+4, sizeof(playMsg->mediaFiles[i].fileName) - 1);
			playMsg->mediaFiles[i].fileName[sizeof(playMsg->mediaFiles[i].fileName) - 1] = '\0';
		}
		else
		{
			playMsg->mediaFiles[i].fileNameLength = strlen(ArrayOfMessages[i].ivrMsgFullPath);
			if (0 != strlen(ArrayOfMessages[i].ivrMsgFullPath))
			{
				strncpy(playMsg->mediaFiles[i].fileName, ArrayOfMessages[i].ivrMsgFullPath, sizeof(playMsg->mediaFiles[i].fileName) - 1);
				playMsg->mediaFiles[i].fileName[sizeof(playMsg->mediaFiles[i].fileName) - 1] = '\0';
			}

			else
			{
				playMsg->mediaFiles[i].fileName[0] = '\0';
				TRACEINTO << "ivrMsgFullPath is empty";
			}
		}
	}

	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PLAY_MSG;
	*seg << PartyId;
	pIVRPlayMessage->Serialize(seg);

	SendMsg(seg, CONFAPP_MNGR_MSG);

	PDELETEA(playMsg->mediaFiles);
	PDELETE(pIVRPlayMessage);
}

// --------------------------------------------------------------------------
void CConfApi::StartRecordMessage(PartyRsrcID PartyId, const IVRMsgDescriptor* ArrayOfMessages, WORD SubMessagesNumber, WORD cachePriority, bool isRecordOnly)
{
	TRACEINTO;

	CIVRPlayMessage* pIVRPlayMessage = new CIVRPlayMessage;
	SIVRPlayMessageStruct* playMsg   = &(pIVRPlayMessage->play);

	playMsg->partyOrconfFlag  = IVR_PLAY_MSG_TO_PARTY; // /EVENT_PARTY_REQUEST;
	playMsg->stopPrevOrAppend = IVR_STOP_PREV_MSG;
	playMsg->mediaType        = IVR_MEDIA_TYPE_AUDIO;
	playMsg->numOfRepetition  = IVR_STATUS_PLAY_ONCE;

	playMsg->numOfMediaFiles = SubMessagesNumber;
	playMsg->mediaFiles      = new SIVRMediaFileParamsStruct[SubMessagesNumber];
	memset((BYTE*)(playMsg->mediaFiles), 0, SubMessagesNumber*sizeof(SIVRMediaFileParamsStruct));

	//IVR for TIP, different messages for record and play_message

	//we always have at least one file to play
	if (isRecordOnly == true) //record message
	{
		playMsg->mediaFiles[0].actionType   = IVR_ACTION_TYPE_RECORD;
		playMsg->mediaFiles[0].frequentness = PRIORITY_0_CACHE_IVR;
	}
	else //play message
	{
		playMsg->mediaFiles[0].actionType   = IVR_ACTION_TYPE_PLAY;
		playMsg->mediaFiles[0].frequentness = cachePriority;
	}

	if(SubMessagesNumber >1) //can be 2 or 4
	{
		playMsg->mediaFiles[1].actionType   = IVR_ACTION_TYPE_PLAY;
		playMsg->mediaFiles[1].frequentness = cachePriority;
	}
	if(SubMessagesNumber >2) //can be 4
	{
		playMsg->mediaFiles[2].actionType   = IVR_ACTION_TYPE_RECORD;
		playMsg->mediaFiles[2].frequentness = PRIORITY_0_CACHE_IVR;
		playMsg->mediaFiles[3].actionType   = IVR_ACTION_TYPE_PLAY;
		playMsg->mediaFiles[3].frequentness = cachePriority;
	}

	for (int i = 0; i < SubMessagesNumber; i++)
	{
		playMsg->mediaFiles[i].duration = ArrayOfMessages[i].ivrMsgDuration;
		playMsg->mediaFiles[i].checksum = ArrayOfMessages[i].ivrMsgCheckSum;

		if (NULL == ArrayOfMessages[i].ivrMsgFullPath)
			TRACEINTO << "ivrMsgFullPath is NULL for message #" << i;
		else if (0 == strncmp(ArrayOfMessages[i].ivrMsgFullPath, "Cfg/", 4))
		{
			playMsg->mediaFiles[i].fileNameLength = strlen(ArrayOfMessages[i].ivrMsgFullPath)-4;
			strncpy(playMsg->mediaFiles[i].fileName, ArrayOfMessages[i].ivrMsgFullPath+4, sizeof(playMsg->mediaFiles[i].fileName) - 1);
			playMsg->mediaFiles[i].fileName[sizeof(playMsg->mediaFiles[i].fileName) - 1] = '\0';
		}
		else
		{
			playMsg->mediaFiles[i].fileNameLength = strlen(ArrayOfMessages[i].ivrMsgFullPath);
			if (0 != strlen(ArrayOfMessages[i].ivrMsgFullPath))
			{
				strncpy(playMsg->mediaFiles[i].fileName, ArrayOfMessages[i].ivrMsgFullPath, sizeof(playMsg->mediaFiles[i].fileName) - 1);
				playMsg->mediaFiles[i].fileName[sizeof(playMsg->mediaFiles[i].fileName) - 1] = '\0';
			}
			else
			{
				playMsg->mediaFiles[i].fileName[0] = '\0';
				TRACEINTO << "ivrMsgFullPath is empty";
			}
		}
	}

	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_RECORD_ROLLCALL;
	*seg << PartyId;
	pIVRPlayMessage->Serialize(seg);

	SendMsg(seg, CONFAPP_MNGR_MSG);

	PDELETEA(playMsg->mediaFiles);
	PDELETE(pIVRPlayMessage);
}
// --------------------------------------------------------------------------
void CConfApi::StopRollCallRecording(PartyRsrcID PartyId,DWORD status)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_STOP_ROLLCALL_RECORDING;
	*seg << PartyId;
	*seg << status;

	SendMsg(seg, CONFAPP_MNGR_MSG);

}
// --------------------------------------------------------------------------
void CConfApi::StopRollCallRecordingAckTimer(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_STOP_ROLLCALL_RECORDING_ACK_TIMER;
	*seg << PartyId;

	SendMsg(seg, CONFAPP_MNGR_MSG);

}
// --------------------------------------------------------------------------
void CConfApi::UpdatePartyNoiseDetection(PartyRsrcID PartyId, WORD noiseDetection, WORD noiseDetectionThreshold, WORD updateAudioDsp)
{
	CSegment* seg = new CSegment;

	*seg << (DWORD)EVENT_ACTION
	     << (DWORD)UPDATE_PARTY_NOISE_DETECTION
	     << PartyId
	     << noiseDetection
	     << noiseDetectionThreshold
	     << updateAudioDsp;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendCamIvrCntlInd()
{
	CSegment*  seg = new CSegment;
	*seg << (DWORD)EVENT_NOTIFY
	     << (DWORD)UPDATE_IVR_CNTR_IND;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendDtmfFromParty(PartyRsrcID PartyId, const char* dtmfString)
{
	CSegment*  seg = new CSegment;
	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PARTY_REQUEST;
	*seg << (DWORD)eCAM_EVENT_PARTY_SEND_DTMF;
	*seg << PartyId;
	*seg << dtmfString;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairpersonEnteredConf(PartyRsrcID PartyId, const COsQueue& pPartyMbx)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRPERSON_ENTERED
	     << PartyId;

	pPartyMbx.Serialize(*seg);

	SendMsg(seg, PCM_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairpersonLeftConf(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRPERSON_LEFT
	     << PartyId;

	SendMsg(seg, PCM_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairpersonStartMoveFromConf(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRPERSON_START_MOVE_FROM_CONF
	     << PartyId;

	SendMsg(seg, PCM_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PCMPartyNotification(PartyRsrcID PartyId, OPCODE notifyOpcode, CSegment* pParam)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)notifyOpcode
	     << PartyId;

	if (pParam)
		*seg << *pParam;

	SendMsg(seg, PCM_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ForwardDtmfToPCM(PartyRsrcID PartyId, CSegment* pParam)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)DTMF_IND_PCM
	     << PartyId
	     << *pParam;

	SendMsg(seg, PCM_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PCMClickAndView(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)PCM_CLICK_AND_VIEW
	     << PartyId;

	SendMsg(seg, PCM_MNGR_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PCMInviteParty(const char* dialString, int networkType)
{
	CSegment* pSeg = new CSegment;

	if (dialString)
	{
		*pSeg << (WORD)strlen(dialString);
		*pSeg << dialString;
	}
	else
		*pSeg << (WORD)0;

	*pSeg << (WORD)networkType;

	SendMsg(pSeg, PCM_INVITE_PARTY);
}

//--------------------------------------------------------------------------
void CConfApi::InformPartyControlOnPcmState(PartyRsrcID PartyId, BYTE isConnected)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << PCM_PARTY_STATE_CHANGED
	      << isConnected;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendDtmfFromClient(CSegment* pParam, const char* partyName)
{
	if ((CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) &&
			(CProcessBase::GetProcess()->GetProductType() != eProductTypeCallGeneratorSoftMCU))
	{
		TRACEINTO << "Failed, The system is not a 'Call Generator'";
		return;
	}

	CSegment* seg = new CSegment;

	*seg << (DWORD)EVENT_ACTION;
	*seg << (DWORD)EVENT_PARTY_REQUEST;
	*seg << (DWORD)eCAM_EVENT_PARTY_CLIENT_SEND_DTMF;
	*seg << partyName;
	*seg << *pParam;

	SendMsg(seg, CONFAPP_MNGR_MSG);
}

//--------------------------------------------------------------------------
void  CConfApi::ContentVideoRefresh( BYTE controlID, BYTE addIntraRequester, const CTaskApp* pParty,WORD destination)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CONTENTVIDREFRESH
	     << (BYTE)controlID
	     << (BYTE)addIntraRequester;

	// addIntraRequester for the intra suppression mechanism
	if (addIntraRequester)
		*seg << (DWORD)pParty;

	//SendMsg(seg, CONTENTCNTL_MSG);
	SendMsg(seg, destination);
}

//--------------------------------------------------------------------------
void CConfApi::StartContent(BYTE currContentProtocol, BYTE isContentSnatching)
{
	CSegment* pSeg = new CSegment;
	*pSeg << currContentProtocol << isContentSnatching;

	SendLocalMessage(pSeg, STARTCONTENT);
}

//--------------------------------------------------------------------------
void CConfApi::StopContent()
{
	SendOpcodeMsg(STOPCONTENT);
}

//--------------------------------------------------------------------------
void CConfApi::LinkTryToConnect(CSegment* pParam)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (*pParam);

	SendLocalMessage(pSeg, LINK_CONNECT);  // Send message to local conference mailbox
}

//--------------------------------------------------------------------------
void CConfApi::LinkTryToDisconnect(CSegment* pParam)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (*pParam);

	SendLocalMessage(pSeg, LINK_DISCONNECT);   // Send message to local conference mailbox
}

//--------------------------------------------------------------------------
void CConfApi::PartyContentRateChanged(const CTaskApp* pParty, const BYTE parameterID, const DWORD newContentRate)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)PARTY_CONTENT_RATE_CHANGED
	     << (DWORD)pParty
	     << parameterID
	     << newContentRate;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::MasterContentMessage(const DWORD opcode, const CTaskApp* pParty, BYTE mcuNum, BYTE termNum, const DWORD newContentRate)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)opcode
	     << (DWORD)pParty
	     << (BYTE)mcuNum
	     << (BYTE)termNum
	     << (DWORD)newContentRate;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyContentBrdgEndSwitch(const CTaskApp* pParty, const CBridge* pBrdg, WORD status)
{
}

//--------------------------------------------------------------------------
void CConfApi::HWConetntOnOffAck(CTaskApp* pParty, eContentState ePState)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)HW_CONTENT_ON_OFF_ACK
	     << (DWORD)pParty
	     << (DWORD)ePState;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendContentTokenMessage(DWORD subOpcode, CTaskApp* pParty, BYTE MCUNumber, BYTE terminalNumber, BYTE label, BYTE randomNum)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)subOpcode
	     << (DWORD)pParty
	     << (BYTE)MCUNumber
	     << (BYTE)terminalNumber
	     << (BYTE)label
	     << (BYTE)randomNum;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ContentMediaProducerStatus(CTaskApp* pParty, BYTE channelID, BYTE status)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)MEDIA_PRODUCER_STATUS
	     << (DWORD)pParty
	     << (BYTE)channelID
	     << (BYTE)status;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ContentFreezePic(BYTE ControlID)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CONTENTFREEZEPIC;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ContentFreezePic()
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CONTENTFREEZEPIC;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ContentTokenRoleProviderMessage(CTaskApp* pParty, BYTE MCUNumber, BYTE terminalNumber, BYTE label, BYTE size, BYTE* pData)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)ROLE_PROVIDER_IDENTITY
	     << (DWORD)pParty
	     << (BYTE)MCUNumber
	     << (BYTE)terminalNumber
	     << (BYTE)label
	     << (BYTE)(size); // size = msgLen of pParam - all obtained fields

	for (WORD i = 0; i < size; i++)
		*seg << pData[i];

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendBfcpContentTokenQueryMessage(CTaskApp* pParty, BYTE MCUNumber, BYTE terminalNumber, BYTE label, BYTE size, BYTE* pData)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)PARTY_BFCP_TOKEN_QUERY
	     << (DWORD)pParty
	     << (BYTE)MCUNumber
	     << (BYTE)terminalNumber
	     << (BYTE)label
	     << (BYTE)(size); // size = msgLen of pParam - all obtained fields

	for (WORD i = 0; i < size; i++)
		*seg << pData[i];

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::H239FlowControlReleaseReq( CTaskApp* pParty, BYTE channelId, WORD bitRate)
{
}

//--------------------------------------------------------------------------
void CConfApi::ContentTokenWithdraw(BYTE DummyIsImmediate)
{
	CSegment* seg = new CSegment;

	*seg << (DWORD)TOKEN_WITHDRAW;
	*seg << (BYTE)DummyIsImmediate; //Dummy - for isImmediate

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::NewContentTokenHolder(BYTE mcuNum, BYTE terminalNum)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)NEWTOKENHOLDER
	     << mcuNum
	     << terminalNum;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ForwardContentTokenMsgToMaster(BYTE mcuNum, BYTE terminalNum, BYTE randomNum, OPCODE subOpcode)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)FORWARD_TOKEN_MSG_TO_MASTER
	     << (DWORD)subOpcode
	     << mcuNum
	     << terminalNum
	     << randomNum;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::NoContentTokenHolder()
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)NOTOKENHOLDER;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ContentNoRoleProvider()
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)TOKENNOROLEPROVIDER;

	SendMsg(seg, CONTENTCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetVideoSrc(const char* partyName, const char* partyName_2)
{
	SetVideoSrcCP(partyName_2, partyName, 0, OPERATOR, 1);
}

//--------------------------------------------------------------------------
void CConfApi::ForwardMCVToChair(CTaskApp* pParty, WORD onOff)
{
	CSegment* seg = new CSegment;   // sent MCV messege to the chair bridge.
	*seg << (DWORD)MCVREQ
	     << (WORD)onOff
	     << pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
WORD CConfApi::AddInParty(CIsdnNetSetup* pNetSetup, CTaskApp* pParty, COsQueue& partyRcvMbx, char* name, DWORD tout, CSegment& rspMsg)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty;
	*seg << name;

	pNetSetup->Serialize(NATIVE, *seg);
	partyRcvMbx.Serialize(*seg);

	OPCODE rspOpcode;

	return (SendMessageSync(seg, ADDINPARTY, tout, rspOpcode, rspMsg));
}

//--------------------------------------------------------------------------
// non-sync call
void CConfApi::FreeTmpPhoneNumber(PartyRsrcID PartyId, char* partyName)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << TMPPHONENUMBER_FREE
	     << partyName;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::AddPartyChannelDesc(CNetSetup* pNetSetup, char* name, WORD channel_num)
{
	CSegment* seg = new CSegment;
	*seg << name
	     << channel_num;

	pNetSetup->Serialize(NATIVE, *seg);

	SendMsg(seg, ADDPARTYCHNLDESC);
}

//--------------------------------------------------------------------------
void CConfApi::PartyRemoteCap(PartyRsrcID PartyId, CCapH320& remoteCap)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << RMTCAP;

	remoteCap.Serialize(NATIVE, *seg);

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetNodeType(PartyRsrcID PartyId, WORD nodeType)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << SETNODETYPE
	     << nodeType;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyContentBrdgConnect(PartyRsrcID partyID, OPCODE brdgOpced, WORD status)
{
	SendBrdgPartyCntlMsgById(partyID, brdgOpced, ENDCONNECTPARTY, status);
}

//--------------------------------------------------------------------------
void CConfApi::PartyContentBrdgDisConnect(PartyRsrcID partyID, OPCODE brdgOpced, WORD status)
{
	SendBrdgPartyCntlMsgById(partyID, brdgOpced, ENDDISCONNECTPARTY, status);
}
//--------------------------------------------------------------------------
void CConfApi::EndContentBrdgConnect(WORD status)
{
	CSegment* seg = new CSegment;
	*seg << status;

	SendLocalMessage(seg, CONTENTBRDGCONNECT);
}

//--------------------------------------------------------------------------
void CConfApi::EndContentBrdgDisConnect(WORD status)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)0 << status;

	SendLocalMessage(seg, CONTENTBRDGDISCONNECT);
}

//--------------------------------------------------------------------------
void CConfApi::UpdatePresentationRes(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg << PartyId
	     << (WORD)PRESENTATION_OUT_STREAM_UPDATED;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::AudioActive(PartyRsrcID PartyId, WORD onOff, WORD srcRequest)
{
	EOnOff eOnOff = onOff ? eOn : eOff;
	CSegment* seg = new CSegment;
	*seg << (OPCODE)AUDIOMUTE
	     << srcRequest
	     << PartyId
	     << (BYTE)eOnOff
	     << (BYTE)eMediaIn;

	SendMsg(seg, AUDIO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::VideoActive(const CTaskApp* pParty, WORD onOff, WORD srcRequest, BYTE localMsg)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)VIDEOMUTE
	     <<  srcRequest
	     << (DWORD)pParty
	     << onOff;

	if (localMsg == 1)
		SendMsg(seg, VIDEO_BRIDGE_MSG);
	else
		SendLocalMessage(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairTokenRequest(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRTOKENREQ
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairTokenRelease(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRTOKENRELEASE
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairTokenRequestAccept(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRTOKENACCEPT
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairTokenRequestReject(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRTOKENREJECT
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairDropTerminal(const CTaskApp* pParty, BYTE mcu, BYTE terminal)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRDROPPARTY
	     << (DWORD)pParty
	     << mcu
	     << terminal;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::TerminalPersonalIdentifier(const CTaskApp* pParty, BYTE mcu, BYTE terminal)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)PARTYPERSONALIDENT
	     << (DWORD)pParty
	     << mcu
	     << terminal;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairDropConf(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRDROPCONF
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyFloorRequest(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRFLOORREQ
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyInfoIndString(const CTaskApp* pParty, const char* identName)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)INDICATESTRING
	     << (DWORD)pParty
	     << identName;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ActiveTerminalUpdateListRequest(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRTCUREQ
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyIndicateEndOfString(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)TERMINLINDENDOFLIST
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairVideoBroadcast(const CTaskApp* pParty, BYTE mcu, BYTE terminal)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRVIDBROADCAST
	     << (DWORD)pParty
	     << mcu
	     << terminal;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ChairCancelVideoBroadcast(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CHAIRCANCELVIDBROADCAST
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::IndicateTerminalNum(const CTaskApp* pParty, BYTE mcu, BYTE terminal)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)INDICATETERMINALNUM // opcode for state machine
	     << (DWORD)pParty
	     << mcu
	     << terminal;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::IndicateDroppedTerminalNum(const CTaskApp* pParty, BYTE mcu, BYTE terminal)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)INDICATEDROPTERMINALNUM  // opcode for state machine
	     << (DWORD)pParty
	     << mcu
	     << terminal;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::McuMultiCmdConf(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)MULTIPOINTCOMMANDCONFERENCE
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::McuCancelMultiIndMaster(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)CANCELMULTIPOINTINDMASTER
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::McuMultiIndMaster(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)MULTIPOINTINDMASTER
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::RandomNumber(const CTaskApp* pParty, BYTE rand)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)RANDOMNUMBER
	     << (DWORD)pParty
	     << rand;

	SendMsg(seg, CHAIRCNTL_MSG);
}

// ///////////////////////////////////////////////////////////////////////////
void CConfApi::AssignTerminalNum(const CTaskApp* pParty, WORD Bridge, WORD mcu, WORD terminal)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)TERMINALINDASSIGNMENT
	     << (DWORD)pParty
	     << mcu
	     << terminal;

	// SendMessage(seg);
	SendMsg(seg, Bridge);
}

//--------------------------------------------------------------------------
void CConfApi::TokenCommandAssociation(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)TOKENCOMMANDASSOCIATION
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::IndicateVideoSrcNum(const CTaskApp* pParty, BYTE mcu, BYTE terminal)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)0                    // msgLen
	     << (DWORD)0                    // msgLen
	     << (DWORD)pParty
	     << (BYTE)mcu
	     << (BYTE)terminal;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::VideoOnAirIndication(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)MULTIINDVISUALIZE
	     << (DWORD)pParty;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::CancelVideoOnAirIndication(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)0                  // msgLen
	     << VIDEO_BRIDGE_MSG
	     << (DWORD)0                  // msgLen
	     << (WORD)CANCELMULTIINDVISUALIZE
	     <<(DWORD)pParty;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::VideoFreeze(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)FREEZPIC
	     << (DWORD)pParty;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetVideoSrcCP(const char* partyName, const char* partyName_2, WORD sub_image, WORD reqSrc, WORD localMsg, BYTE confForceType)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)SETVIDSRCCP
	     << reqSrc;

	if (partyName == NULL)
	{
		*seg << SEEMEALL
		     << partyName_2
		     << confForceType;
		// confForceType : 0 - usual MCV force, 1 - MCV with MVC cap, 2 - PeopleContentsV0 force //
	}
	else
	{
		*seg << SEEMEPARTY
		     << partyName
		     << partyName_2;
	}

	*seg << sub_image;
	if (localMsg == 1)
		SendMsg(seg, VIDEO_BRIDGE_MSG);
	else
		SendLocalMessage(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::VoiceActivateConfForce(const char* partyName, WORD sub_image, WORD reqSource, BYTE localMsg)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)VOICEACTIVATECP
	     << reqSource
	     << VOICEACTIVATEALL
	     << sub_image
	     << partyName;

	if (localMsg == 1)
		SendMsg(seg, VIDEO_BRIDGE_MSG);
	else
		SendLocalMessage(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetMIHCap(const CTaskApp* pParty, WORD onOff)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)SETMIHCAP
	     << (DWORD)pParty
	     << onOff;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendVCSMessageToChair(const CTaskApp* pParty, BYTE mcu, BYTE terminal)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)VCSREQ
	     << (DWORD)pParty
	     << mcu
	     << terminal;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendVCSCancelMessageToChair(const CTaskApp* pParty)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)VCSCANCELREQ
	     << (DWORD)pParty;

	SendMsg(seg, CHAIRCNTL_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::OperatorAssistance(DWORD partyId, BYTE action, BYTE mode, BYTE init_by)
{
	CSegment*  seg = new CSegment;
	*seg << partyId
	     << action
	     << mode
	     << init_by;

	SendMsg(seg, OPERATOR_ASSIST);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateRecordingControl(WORD wRecordingControl, CTaskApp* pPartyToReply)
{
	CSegment* seg = new CSegment;
	*seg << (WORD)wRecordingControl
	     << (DWORD)pPartyToReply;

	SendMsg(seg, UPDATE_RECORDING_CONTROL);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateNetChannel(PartyRsrcID PartyId, BYTE boardId, DWORD spanId, DWORD connectionID, WORD numChnl)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << UPDATENETCHANNEL
	      << boardId
	      << spanId
	      << connectionID
	      << numChnl;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ReallocateBondingChannels(PartyRsrcID PartyId, DWORD number_of_channels)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << REALLOCATERTM
	      << number_of_channels;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ReallocateOnBoardFull(PartyRsrcID PartyId, DWORD conn_id)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << BOARDFULL
	      << conn_id;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::OnSmartRecovery(PartyRsrcID PartyId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << SMART_RECOVERY;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SetDialStringForGW(const char* dialString)
{
	CSegment* pSeg = new CSegment;

	if (dialString)
	{
		*pSeg << (WORD)strlen(dialString);
		*pSeg << dialString;
	}
	else
		*pSeg << (WORD)0;

	SendMsg(pSeg, ADDDIALSTRING);
}

//--------------------------------------------------------------------------
void CConfApi::SetDialOutServiceNameForGW(const char* isdnServiceName)
{
	CSegment* pSeg = new CSegment;

	if (isdnServiceName)
	{
		*pSeg << (WORD)strlen(isdnServiceName);
		*pSeg << isdnServiceName;
	}
	else
		*pSeg << (WORD)0;

	SendMsg(pSeg, DIALOUTSERVICENAME);
}

//--------------------------------------------------------------------------
void CConfApi::EndGWPartySetup(DWORD partyId, DWORD timeout, BYTE allowOverride, const char* msgStr)
{
	CSegment* pSeg = new CSegment;
	*pSeg << partyId
	      << timeout;

	*pSeg << (BYTE)allowOverride;

	if (msgStr)
	{
		*pSeg << (WORD)strlen(msgStr);
		*pSeg << msgStr;
	}
	else
		*pSeg << (WORD)0;

	SendMsg(pSeg, ENDGWPARTYSETUP);
}

//--------------------------------------------------------------------------
void CConfApi::VideoInSinched(CTaskApp* pParty)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pParty;

	SendMsg(pSeg, DECSYNC);
}

//--------------------------------------------------------------------------
void CConfApi::ContentDecoderAllocFail()
{
	SendOpcodeMsg(CONTENT_DECODER_ALLOC_RSRC_FAIL);
}

//--------------------------------------------------------------------------
void CConfApi::ContentDecoderResetFailStatus()
{
	SendOpcodeMsg(CONTENT_DECODER_RESET_FAIL_STATUS);
}

//--------------------------------------------------------------------------
void CConfApi::SendStartPreviewReqToParty(CPartyPreviewDrv* pPartyPreviewDrv)
{
	COstrStream str;
	pPartyPreviewDrv->Serialize(NATIVE, str);

	CSegment* seg = new CSegment;
	*seg << str.str().c_str();

	SendMsg(seg, START_PREVIEW_PARTY);
}

//--------------------------------------------------------------------------
void CConfApi::SendStopPreviewReqToParty(const char* partyName, cmCapDirection VideoDirection)
{
	CSegment* seg = new CSegment;
	*seg << partyName
	     <<(WORD)VideoDirection;

	SendMsg(seg, STOP_PREVIEW_PARTY);
}

// //////////////////////////////////////////////////////////////////////////////////////////
void CConfApi::SendRequestVideoPreviewIntra(const char* partyName, cmCapDirection Direction)
{
	CSegment* seg = new CSegment;
	*seg << partyName
	     << (WORD)Direction;

	SendMsg(seg, REQUEST_PREVIEW_INTRA);
}

//--------------------------------------------------------------------------
void CConfApi::SetExclusiveContentOn(const char* partyName) // Restricted content
{
	// VNGFE-8300 (BRIDGE-13062)
	if(NULL == partyName)	{
		PASSERT_AND_RETURN(1);
	}

	CSegment* seg = new CSegment;
	*seg << partyName;

	SendMsg(seg, SET_EXCLUSIVE_CONTENT);
}

//--------------------------------------------------------------------------
void CConfApi::RemoveExclusiveContent() // Restricted content
{
	SendOpcodeMsg(REMOVE_EXCLUSIVE_CONTENT);
}

//--------------------------------------------------------------------------
void CConfApi::SetExclusiveContentMode(BOOL yesNo) // Restricted content
{
	CSegment* seg = new CSegment;
	*seg << yesNo;

	SendMsg(seg, SET_EXCLUSIVE_CONTENT_MODE);
}

//--------------------------------------------------------------------------
void CConfApi::SetMuteIncomingLectureMode(BOOL yesNo) // Restricted content
{
	CSegment* seg = new CSegment;
	*seg << yesNo;

	SendMsg(seg, SET_MUTE_INCOMING_LECTURE_MODE);
}
void CConfApi::UpdateMuteAllAudioExceptLeader(BYTE muteAllButLeader, BYTE isMuteIncludeExistingUsers)
{
	DWORD isIVR = FALSE;
	DWORD isForMuteAllButLecture = FALSE;
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)EVENT_ACTION
		  << (OPCODE)EVENT_CONF_REQUEST
		  << (OPCODE)eCAM_EVENT_CONF_MUTE_ALL
		  << (PartyRsrcID)DUMMY_PARTY_ID
		  << isIVR
		  << isForMuteAllButLecture
          << muteAllButLeader
		  << isMuteIncludeExistingUsers;

	SendMsg(pSeg, CONFAPP_MNGR_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::UpdateUnMuteAllAudioExceptLeader(BYTE muteAllButLeader, BYTE isMuteIncludeExistingUsers)
{
	DWORD isIVR = FALSE;
	DWORD isForMuteAllButLecture = FALSE;
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)EVENT_ACTION
		  << (OPCODE)EVENT_CONF_REQUEST
		  << (OPCODE)eCAM_EVENT_CONF_UNMUTE_ALL
		  << (PartyRsrcID)DUMMY_PARTY_ID
		  << isIVR
		  << isForMuteAllButLecture
          << muteAllButLeader
          << isMuteIncludeExistingUsers;

	SendMsg(pSeg, CONFAPP_MNGR_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::SetConfAvcSvcMode(eConfMediaState confState, ConfMonitorID confId)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)confState << confId;
	SendMsg(seg, SET_CONF_AVC_SVC_MEDIA_STATE);
}

//--------------------------------------------------------------------------
void CConfApi::SetPartyAvcSvcMode(eConfMediaState confState, ConfMonitorID confId, PartyMonitorID partyId)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)confState << confId << partyId;
	SendMsg(seg, SET_PARTY_AVC_SVC_MEDIA_STATE);
}

//--------------------------------------------------------------------------
void CConfApi::UpdatePartyCntlOnICEParams(PartyRsrcID PartyId, BYTE IsICECall, CIceParams* IceParams)
{
	BYTE IsIceParams = 0;
	if (IceParams)
		IsIceParams = 1;

	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << UPDATEICEPARAMS
	      << IsICECall
	      << IsIceParams;

	if (IceParams)
		IceParams->Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SipPartySendFallbackFromIceToSip(PartyRsrcID PartyId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << FALL_BACK_FROM_ICE_TO_SIP;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendFallbackFromIceToSipPartyToConf(const char* name)
{
	CSegment* seg = new CSegment;
	*seg << name;

	SendMsg(seg, FALL_BACK_FROM_ICE_TO_SIP);
}

//--------------------------------------------------------------------------
void CConfApi::SendVideoPreferencesToPartyControl(PartyRsrcID PartyId, DWORD Width, DWORD Height, DWORD FR, DWORD BitRate)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << UPDATE_VIDEO_RESOLUTION
	      << Width
	      << Height
	      << FR
	      << BitRate;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::AddSlaveParty(PartyRsrcID PartyId, DWORD tipPartyType)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << ADDSLAVEPARTY
	      << tipPartyType;
	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::AddSubLinksAfterMainConnected(PartyRsrcID PartyId)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << ADDSUBLINKSPARTIES;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendAddSlavePartyToConf(const CRsrvParty& rsrvParty,
                                       DWORD undefId,
                                       DWORD tipPartyType,
                                       DWORD room_Id,
                                       DWORD pPartyRsrcID,
                                       DWORD masterVideoPartyType,
                                       CIpComMode* pMasterScm)
{
	CSegment* seg = new CSegment;
	pMasterScm->Serialize(NATIVE, *seg);
	*seg << (DWORD)undefId
	     << (DWORD)tipPartyType
	     << (DWORD)room_Id
	     << (DWORD)pPartyRsrcID
	     << (DWORD)masterVideoPartyType;

	CRsrvParty* pRsrvParty = new CRsrvParty(rsrvParty);

	pRsrvParty->Serialize(NATIVE, *seg);

	SendMsg(seg, ADDSLAVEPARTY);

	POBJDELETE(pRsrvParty);
}

//--------------------------------------------------------------------------
void CConfApi::SendAddMsSlavePartyToConf(PartyRsrcID mainPartyRsrcId,
                                         const CRsrvParty& rsrvSlaveParty,
                                         eAvMcuLinkType AvMcuLinkType,
                                         DWORD msSlaveIndex,
                                         DWORD msSsrcRangeStart,
                                         CSipCaps* remoteCaps,
                                         CVidModeH323 *pLocalSdesCap)
{
	CSegment* seg = new CSegment;

	DWORD isLocalSdesCap = 0;
	if (pLocalSdesCap)
	{
		TRACEINTO << "CConfApi::SendAddMsSlavePartyToConf: set isLocalSdesCap to 1.";
		isLocalSdesCap = 1;
	}

	*seg << mainPartyRsrcId
	     << (DWORD)AvMcuLinkType
	     << msSlaveIndex
	     << msSsrcRangeStart
		 << isLocalSdesCap;

	if (isLocalSdesCap)
	{
		if (pLocalSdesCap)
		{
			pLocalSdesCap->Serialize(NATIVE, *seg);
		}
	}
	CRsrvParty* pRsrvParty = new CRsrvParty(rsrvSlaveParty);

	if (remoteCaps)
	{
		TRACEINTO << "CConfApi::SendAddMsSlavePartyToConf: set remoteCaps.";
		remoteCaps->Serialize(NATIVE, *seg);
	}

	pRsrvParty->Serialize(NATIVE, *seg);

	SendMsg(seg, ADDMSSLAVEPARTY);

	POBJDELETE(pRsrvParty);
}

//--------------------------------------------------------------------------
void CConfApi::SendMsSlaveToMainAck(PartyRsrcID mainPartyRsrcId,
                                    DWORD opcode,
                                    DWORD status,
                                    PartyRsrcID msSlavePartyRsrcId,
                                    eAvMcuLinkType AvMcuLinkType,
                                    DWORD msSlaveIndex)
{
	std::ostringstream msg;

	msg
		<< "\n  mainPartyRsrcId   :" << mainPartyRsrcId
		<< "\n  opcode            :" << opcode
		<< "\n  status            :" << status
		<< "\n  msSlavePartyRsrcId:" << msSlavePartyRsrcId
		<< "\n  msSlaveDirection  :" << eAvMcuLinkTypeNames[AvMcuLinkType]
		<< "\n  msSlaveIndex      :" << msSlaveIndex;

	TRACEINTO << msg.str().c_str();

	CSegment* seg = new CSegment;
	*seg
		<< mainPartyRsrcId
		<< (OPCODE)PARTY_CONTROL_MS_SLAVE_TO_MAIN_ACK
		<< opcode
		<< status
		<< msSlavePartyRsrcId
		<< (DWORD)AvMcuLinkType
		<< msSlaveIndex;

	SendMsg(seg, PARTY_RSRC_ID_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendMsSlaveToMainMsg(PartyRsrcID mainPartyRsrcId,
                                    DWORD opcode,
                                    CSegment* pMsg)
{
	std::ostringstream msg;

	msg << "\n  mainPartyRsrcId   :" << mainPartyRsrcId
		<< "\n  opcode            :" << opcode;

	TRACEINTO << msg.str().c_str();

	CSegment* pSeg = new CSegment;
	*pSeg<< mainPartyRsrcId
		<< (OPCODE)PARTY_CONTROL_MS_SLAVE_TO_MAIN_MSG
		<< opcode;

	if (NULL != pMsg)
		*pSeg << *pMsg;


	SendMsg(pSeg, PARTY_RSRC_ID_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendAllMsOutSlavesConnected(PartyRsrcID mainPartyRsrcId,
										   DWORD status,
										   DWORD accBandwidth,
										   mcMuxLync2013InfoReq* msSvcMuxMsg)
{
	TRACEINTO << "mainPartyRsrcId: " << mainPartyRsrcId << " - status: " << status << " - accBandwidth: " << accBandwidth;

	CSegment* seg = new CSegment;
	*seg << mainPartyRsrcId
		 << (OPCODE)PARTY_CONTROL_ALL_MS_OUT_SLAVES_CONNECTED
		 << status
		 << accBandwidth;

	 seg->Put(reinterpret_cast<const BYTE*>(msSvcMuxMsg), sizeof(mcMuxLync2013InfoReq));

	SendMsg(seg, PARTY_RSRC_ID_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::SendAllMsInSlavesConnected(PartyRsrcID mainPartyRsrcId, DWORD status, DWORD numOfConnectedInSlaves)
{
	TRACEINTO << "mainPartyRsrcId: " << mainPartyRsrcId << " - status: " << status << " - numOfConnectedInSlaves: " << numOfConnectedInSlaves;

	CSegment* seg = new CSegment;
	*seg << mainPartyRsrcId
		 << (OPCODE)PARTY_CONTROL_ALL_MS_IN_SLAVES_CONNECTED
		 << status
		 << numOfConnectedInSlaves;

	SendMsg(seg, PARTY_RSRC_ID_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendAllMsSlavesDeleted(PartyRsrcID mainPartyRsrcId, DWORD status)
{
	TRACEINTO << "mainPartyRsrcId: " << mainPartyRsrcId << " - status: " << status;

	CSegment* seg = new CSegment;
	*seg << mainPartyRsrcId
		 << (OPCODE)PARTY_CONTROL_ALL_MS_SLAVES_DELETED
		 << status;

	SendMsg(seg, PARTY_RSRC_ID_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::DeleteMsSlave(PartyRsrcID mainPartyRsrcId, PartyRsrcID deletedSlavePartyRsrcId)
{
	TRACEINTO << "mainPartyRsrcId: " << mainPartyRsrcId << " , deletedSlavePartyRsrcId: " << deletedSlavePartyRsrcId;

	CSegment* seg = new CSegment;
	*seg << deletedSlavePartyRsrcId;

	SendMsgToSlavesController(mainPartyRsrcId, DELMSSLAVEPARTY, seg);

//	*seg << mainPartyRsrcId
//		 << (OPCODE)DELMSSLAVEPARTY
//		 << deletedSlavePartyRsrcId;
//
//	SendMsg(seg, PARTY_RSRC_ID_MSG);

}

//--------------------------------------------------------------------------
void CConfApi::SendAddSubLinksPartiesToConf(WORD cascadedLinksNumber, WORD room_Id, char* partyName)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)cascadedLinksNumber
	     << (DWORD)room_Id
	     << partyName;

	SendMsg(seg, ADDSUBLINKSPARTIES);
}

//--------------------------------------------------------------------------
void CConfApi::SendAckFromSlaveToMaster(PartyRsrcID PartyId, DWORD rStatus, PartyRsrcID PartyIdPeer, WORD tipPartyType, WORD allocated_resolution)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << (OPCODE)PARTY_CONTROL_SLAVE_TO_MASTER_ACK
	      << rStatus
	      << PartyIdPeer
	      << tipPartyType
	      << allocated_resolution;

	SendMsg(pSeg, PARTY_RSRC_ID_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::SendMessageFromSlaveToMaster(PartyRsrcID PartyId, WORD srcTipPartyType, DWORD opcode, CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << (OPCODE)PARTY_CONTROL_SLAVE_TO_MASTER
	      << (DWORD)srcTipPartyType
	      << opcode;

	if (pMsg != NULL)
		*pSeg << *pMsg;

	SendMsg(pSeg, PARTY_RSRC_ID_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendMessageFromMasterToSlave(PartyRsrcID PartyId, DWORD opcode, CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << (OPCODE)PARTY_CONTROL_MASTER_TO_SLAVE
	      << opcode;

	if (pMsg != NULL)
		*pSeg << *pMsg;

	SendMsg(pSeg, PARTY_RSRC_ID_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyCntlToPartyMsgFromMasterToSlave(PartyRsrcID PartyId, DWORD opcode, CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << (OPCODE)PARTYCONTROL_PARTY_MASTER_TO_SLAVE
	      << opcode;

	if (NULL != pMsg)
		*pSeg << *pMsg;

	SendMsg(pSeg, PARTY_RSRC_ID_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyCntlToPartyCntlMsgFromSlaveToMaster(PartyRsrcID PartyId, DWORD opcode, CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << (OPCODE)PARTYCONTROL_PARTY_SLAVE_TO_MASTER
	      << opcode;

	if (NULL != pMsg)
		*pSeg << *pMsg;

	SendMsg(pSeg, PARTY_RSRC_ID_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartytoPartyCntlMsgFromMasterToSlave(PartyRsrcID PartyId, WORD destTipPartyType, DWORD opcode, CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << PARTY_PARTYCONTROL_MASTER_TO_SLAVE
	      << (DWORD)destTipPartyType
	      << (DWORD)opcode;

	if (NULL != pMsg)
		*pSeg << *pMsg;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyToPartyCntlMsgFromSlaveToMaster(PartyRsrcID PartyId, WORD srcTipPartyType, DWORD opcode, CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << PARTY_PARTYCONTROL_SLAVE_TO_MASTER
	      << (DWORD)srcTipPartyType
	      << (DWORD)opcode;

	if (NULL != pMsg)
		*pSeg << *pMsg;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////////////////
void CConfApi::SendDisconnectMessageFromMasterPartyControlToAllToSlaves(CTaskApp* pParty)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pParty
			 << DISCONNECT_ALL_SLAVES_FROM_MASTER_PARTY_CONTROL;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::AddVideoRelayImageToMix(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status)
{
	SendBrdgPartyCntlMsg(pParty, brdgOpcode, ADD_VIDEORELAY_IMAGE_TO_MIX, status);
}

//--------------------------------------------------------------------------
void CConfApi::HandleMrmpRtcpFirInd(PartyRsrcID PartyId, RelayIntraParam* pIntraParam)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	     << (WORD)RELAY_ENDPOINT_ASK_FOR_INTRA;

	pIntraParam->Serialize(pSeg);

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendScpNotificationReqToPartyCntl(PartyRsrcID PartyId, CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << SCP_NOTIFICATION_REQ;

	if (pMsg)
		*pSeg << *pMsg;

	TRACEINTO << "Sending the request SCP_NOTIFICATION_REQ to party with partyId: " << PartyId;
	SendMsg(pSeg, PARTY_MSG);
	TRACEINTO << "Sending the request SCP_NOTIFICATION_REQ to party with partyId: " << PartyId << "has succeeded";
}

//--------------------------------------------------------------------------
void CConfApi::ScpNotificationAckFromEP(PartyRsrcID PartyId, DWORD channelHandle, DWORD remoteSequenseNumber, DWORD bIsAck)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << CONF_API_SCP_NOTIFICATION_ACK_FROM_EP
	      << channelHandle
	      << remoteSequenseNumber
	      << bIsAck;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendScpPipesMappingNotificationReqToPartyCntl(PartyRsrcID PartyId, CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << SCP_PIPES_MAPPING_NOTIFICATION_REQ;

	if (pMsg)
		*pSeg << *pMsg;

	SendMsg(pSeg, PARTY_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::UpdateOnRelayImageSvcToAvcTranslated(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status)
{
	SendBrdgPartyCntlMsg(pParty, brdgOpcode, RELAY_IMAGE_SVC_AVC_TRANS_UPDATE, status);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateOnNonRelayImageAvcToSvcTranslated(const CTaskApp* pParty, OPCODE brdgOpcode, WORD status)
{
	SendBrdgPartyCntlMsg(pParty, brdgOpcode, NON_RELAY_IMAGE_AVC_SVC_TRANS_UPDATE, status);
}

//--------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////
// XCode brdg
/////////////////////////////////////////////////////////////////////////////
void  CConfApi::EndXCodeBrdgConnect(WORD status)
{
  CSegment*  seg = new CSegment;
  *seg << status;
  SendLocalMessage(seg, XCODEBRDGCONNECT);
}

/////////////////////////////////////////////////////////////////////////////
void  CConfApi::EndXCodeBrdgDisConnect(WORD status)
{
  CSegment*  seg = new CSegment;
  *seg << status;
  SendLocalMessage(seg, XCODEBRDGDISCONNECT);
}
/////////////////////////////////////////////////////////////////////////////

void CConfApi::SendScpIvrShowSlideToPartyCntl(PartyRsrcID PartyId)
{
	CSegment*  pSeg = new CSegment;
	*pSeg << PartyId
	      << SCP_IVR_SHOW_SLIDE_REQ;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendScpIvrStopShowSlideToPartyCntl(PartyRsrcID PartyId)
{
	CSegment*  pSeg = new CSegment;
	*pSeg << PartyId
	      << SCP_IVR_STOP_SHOW_SLIDE_REQ;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::GetPartyVideoDataReq(CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;

	*pSeg << (OPCODE)GET_PARTY_VIDEO_DATA_REQ;

	if (NULL != pMsg)
		*pSeg << *pMsg;

	SendMsg(pSeg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ConfAuthStatusNotify(PartyRsrcID PartyId, BYTE status)
{
	CSegment * pSeg = new CSegment;
	*pSeg << PartyId
	      << SIP_PARTY_CONF_PWD_STAUTS_ACK
	      << status;

	SendMsg(pSeg, PARTY_MSG);
}

//_t_p_
//--------------------------------------------------------------------------
void CConfApi::ChangeVideoOutForTipPolycom(PartyRsrcID PartyId)
{
	PTRACE(eLevelInfoNormal,"CConfApi::ChangeVideoOutForTipPolycom");
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
		  << CHANGE_VIDEO_OUT_TIP_POLYCOM;

	SendMsg(pSeg, PARTY_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::UpdateMuteAllVideoExceptLeader(BYTE onOff)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)UPDATE_MUTE_ALL_VIDEO_EXCEPT_LEADER
	     << (BYTE)onOff;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::UpdateMuteAllIncomingVideoExceptLeader(BYTE onOff)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)UPDATE_MUTE_ALL_INCOMING_VIDEO_EXCEPT_LEADER
	     << (BYTE)onOff;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::SetPartyAsLeaderForVB(const char* pPartyName, EOnOff eOnOff)
{
	CSegment* seg = new CSegment;
	*seg << (OPCODE)SET_PARTY_AS_LEADER_FOR_VB;
	*seg << pPartyName;
	*seg << (BYTE)eOnOff;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}
/////////////////////////////////////////////////////////////////////////////
void CConfApi::UpdateArtOnTranslateVideoSsrcAck(PartyRsrcID PartyId, WORD status)
{
	TRACEINTO << "PartyId:" << PartyId << ", Status:" << status;

	CSegment* seg = new CSegment;
	*seg << (DWORD)UPDATE_ART_WITH_SSRC_ACK
	     << (DWORD)PartyId
	     << (DWORD)status;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateArtOnTranslateVideoSSRC(PartyRsrcID PartyId, DWORD ssrc)
{
	TRACEINTO << "PartyId:" << PartyId << ", Ssrc:" << ssrc;

	CSegment*  pSeg = new CSegment;
	*pSeg	<< (DWORD)PartyId
			<< (WORD)UPDATE_ART_WITH_SSRC_REQ
			<< (DWORD)ssrc;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ReplayUpgradeSvcAvcTranslate(PartyRsrcID PartyId, DWORD mediaType, EStat status)
{
	TRACEINTO << "PartyId:" << PartyId << ", MediaType:" << mediaType << ", Status:" << status;

	WORD opcode;
	if (VIDEO == mediaType)
		opcode = END_VIDEO_UPGRADE_TO_MIX_AVC_SVC;
	else
		opcode = END_AUDIO_UPGRADE_TO_MIX_AVC_SVC;

	CSegment*  pSeg = new CSegment;
	*pSeg	<< (DWORD)PartyId
			<< (WORD)opcode
			<< (DWORD)status;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::ReplayDowngradeSvcAvcTranslate( PartyRsrcID PartyId, DWORD mediaType, EStat status)
{
	TRACEINTO << "PartyId:" << PartyId << ", MediaType:" << mediaType << ", Status:" << status;

	WORD opcode;
	if (VIDEO == mediaType)
		opcode = END_VIDEO_DOWNGRADE_MIX_AVC_SVC;
	else
		opcode = END_AUDIO_DOWNGRADE_MIX_AVC_SVC;

	CSegment*  pSeg = new CSegment;
	*pSeg	<< (DWORD)PartyId
			<< (WORD)opcode
			<< (DWORD)status;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendAvcToSvcArtTranslatorConnectedToPartyControl(const CTaskApp* pParty, STATUS status)
{
	TRACEINTO << "PartyId:" << pParty->GetPartyId() << ", ConfId:" << pParty->GetConfId() << ", Status:" << status;

	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pParty
	      << (WORD)END_AVC_TO_SVC_ART_TRANSLATOR_CONNECT
	      << (DWORD)status;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendAvcToSvcArtTranslatorDisconnectedToPartyControl(const CTaskApp* pParty, STATUS status)
{
	TRACEINTO << "PartyId:" << pParty->GetPartyId() << ", ConfId:" << pParty->GetConfId() << ", Status:" << status;

	CSegment* pSeg        = new CSegment;
	*pSeg << (DWORD)pParty
	      << (WORD)END_AVC_TO_SVC_ART_TRANSLATOR_DISCONNECTED
	      << (DWORD)status;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyUpgradeToMixChannelsUpdated(PartyRsrcID PartyId,unsigned int channelHandle)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)PartyId
	      << (WORD)PARTY_UPGRADE_TO_MIX_CHANNELS_UPDATED;
	*pSeg << channelHandle;
	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateMrmpStreamIsMustAck( PartyRsrcID PartyId, WORD status)
{
	CSegment* seg = new CSegment;
	*seg << (DWORD)SEND_MRMP_STREAM_IS_MUST_ACK
		 << (DWORD)PartyId
		 << (DWORD)status;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::UpdateMrmpStreamIsMust( PartyRsrcID PartyId, DWORD ssrc, DWORD channelID, BOOL bIsMust)
{
	CSegment*  pSeg = new CSegment;
	*pSeg << (DWORD)PartyId
		 << (WORD)SEND_MRMP_STREAM_IS_MUST_REQ
		 << (DWORD)ssrc
		 << (DWORD)channelID
		 << (DWORD)bIsMust;

	SendMsg(pSeg, PARTY_MSG);
}

//_e_m_
//--------------------------------------------------------------------------
void CConfApi::UpdateVidBrdgTelepresenseEPInfo(PartyRsrcID PartyId , CTelepresenseEPInfo* pTelepresenseEPInfo)
{
	PASSERTMSG_AND_RETURN(!pTelepresenseEPInfo, "pTelepresenseEPInfo == NULL");

	TRACEINTO << "PartyId:" << PartyId;

	CSegment* pSeg = new CSegment;
	*pSeg
		<< PartyId
		<< UPDATE_VIDBRDG_TELEPRESENSE_EP_INFO;

	pTelepresenseEPInfo->Serialize(NATIVE, *pSeg);
	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendUpdateToAudioBridgeOnSeenImage(PartyRsrcID idOfPartyToUpdate, PartyRsrcID idOfSeenParty)
{
	CSegment* seg = new CSegment;
	*seg
		<< (OPCODE)UPDATE_AUDIO_ON_SEEN_IMAGE
		<< idOfPartyToUpdate
		<< idOfSeenParty;

	SendMsg(seg, AUDIO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendVideoUpdateAfterVsrMsgToPartyControl(PartyRsrcID PartyId, CIpComMode* pNewMode)
{
	TRACEINTO << "PartyId:" << PartyId;

	CSegment* pSeg = new CSegment;
	*pSeg
		<< PartyId
		<< UPDATE_VIDEO_AFTER_VSR_MSG;

	pNewMode->Serialize(NATIVE, *pSeg);
	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendLastTargetModeMsgToPartyControl(PartyRsrcID PartyId, CIpComMode* pNewMode)
{
	TRACEINTO << "PartyId:" << PartyId;

	CSegment* pSeg = new CSegment;
	*pSeg
		<< PartyId
		<< UPDATE_LAST_TARGET_MODE_MSG;

	pNewMode->Serialize(NATIVE, *pSeg);
	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendSingleUpdatePacsiInfoToPartyControl(PartyRsrcID PartyId, CIpComMode* pTargetMode, BYTE isMute)
{
	// send update pacsi to party from transaction
	PASSERTMSG_AND_RETURN(!pTargetMode, "pTargetMode == NULL");

	TRACEINTO << "PartyId:" << PartyId << ", isMute:" << (int)isMute;

	CSegment* pSeg = new CSegment;
	*pSeg
		<< PartyId
		<< SINGLE_UPDATE_PACSI_INFO
		<< (BYTE)isMute;

	pTargetMode->Serialize(NATIVE, *pSeg);

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::MSOrganizerEndConnection(PartyRsrcID PartyId, DWORD status, char* FocusUri, char* MsConversationId,BOOL isCallThroughDma )
{
	CSegment* seg = new CSegment;

	*seg << PartyId
		<< MSORGANIZERENDCONNECT;

	*seg << (DWORD)status;
	*seg << isCallThroughDma;

	if (FocusUri)
	{
		*seg << (WORD)strlen(FocusUri);
		*seg << FocusUri;
	}
	else
	{
		*seg << (WORD)0;
	}

	if (MsConversationId)
	{
		*seg << (WORD)strlen(MsConversationId);
		*seg << MsConversationId;
	}
	else
	{
		*seg << (WORD)0;
	}


	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::MSFocusEndConnection(PartyRsrcID PartyId, DWORD status, BOOL IsMS2013Server, char* strToAddr)
{
	CSegment* seg = new CSegment;

	*seg << PartyId
		 << MSFOCUSENDCONNECT;

	*seg << IsMS2013Server;
	*seg << (DWORD)status;

	if (strToAddr)
	{
		*seg << (WORD)strlen(strToAddr);
		*seg << strToAddr;
	}
	else
	{
		*seg << (WORD)0;
	}
	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::MSFocusEndDisconnection(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;

	*seg << PartyId
		 << MSFOCUSENDDISCONNECT;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::MSSubscriberEndConnection(PartyRsrcID PartyId, DWORD status, BOOL IsMS2013Server, const char* uri)
{
	CSegment* seg = new CSegment;

	*seg << PartyId
			<< MSSUBSCRIBERENDCONNECT;

		*seg << (DWORD)status;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::MSSubscriberEndDisconnection(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;

	*seg << PartyId
			<< MSSUBSCRIBERENDDISCONNECT;

	SendMsg(seg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendVsrMsgIndToSlavesController(PartyRsrcID PartyId, CSegment* pParam)
{
	CSegment* seg = new CSegment;

	*seg
		<< PartyId
		<< (DWORD)SIP_PARTY_VSR_MSG_IND;

	ST_VSR_SINGLE_STREAM vsr;
	pParam->Get(reinterpret_cast<BYTE*>(&vsr), sizeof(ST_VSR_SINGLE_STREAM));

	CMsVsrMsg vsrMsg(vsr);
	vsrMsg.Dump();

	seg->Put(reinterpret_cast<BYTE*>(&vsr), sizeof(ST_VSR_SINGLE_STREAM));

	SendMsg(seg, PARTY_RSRC_ID_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendMsgToSlavesController(PartyRsrcID PartyId, DWORD opcode, CSegment* pParam)
{
	CSegment* seg = new CSegment();
	*seg
		<< opcode
		<< PartyId;

	if (NULL != pParam)
	{
		*seg << *pParam;
	}

	SendMsg(seg, MS_LYNC_SLAVES_CONTROLLER_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendSingleVsrMsgInd(PartyRsrcID PartyId, const ST_VSR_SINGLE_STREAM* vsr)
{
	TRACEINTO << "PartyId:" << PartyId;

	CSegment* pSeg = new CSegment;
	*pSeg
		<< PartyId
		<< (DWORD)SIP_PARTY_SINGLE_VSR_MSG_IND;

	pSeg->Put(reinterpret_cast<const BYTE*>(vsr), sizeof(ST_VSR_SINGLE_STREAM));
	SendMsg(pSeg, PARTY_RSRC_ID_MSG);
}
//--------------------------------------------------------------------------
//LYNC2013_FEC_RED:
void CConfApi::SendSingleFecOrRedMsgFromSlavesControllerToPartyControl(PartyRsrcID PartyRsrcId, DWORD mediaType, DWORD newFecRedPercent)
{
	TRACEINTO << "LYNC2013_FEC_RED";

	CSegment* seg = new CSegment;

	*seg << PartyRsrcId
		 << (DWORD)SIP_PARTY_CONTROL_SINGLE_FEC_RED_MSG
		 << mediaType
		 << newFecRedPercent;

	SendMsg(seg, PARTY_RSRC_ID_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::SendFullPacsiInd(PartyRsrcID PartyId,const MsFullPacsiInfoStruct* fullPacsiInfo, BYTE isReasonFecOrRed)
{
	TRACEINTO << "PartyRsrcID:" << (DWORD)PartyId << ", isReasonFecOrRed:" << (DWORD)isReasonFecOrRed;
	PASSERTMSG_AND_RETURN(!fullPacsiInfo, "fullPacsiInfo == NULL");

	CSegment* pSeg = new CSegment;
	*pSeg<< PartyId
		 << (DWORD)FULL_PACSI_INFO_IND
		 << isReasonFecOrRed;
	pSeg->Put(reinterpret_cast<const BYTE*>(fullPacsiInfo), sizeof(MsFullPacsiInfoStruct));
	SendMsg(pSeg, PARTY_RSRC_ID_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendAvmcu2013Detected(PartyRsrcID PartyId)
{
	CSegment* seg = new CSegment;
	*seg
		<< PartyId
		<< MSFT_AVMCU2013DETECTED;

	SendMsg(seg, PARTY_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::SendAvMcuAllMsInSlavesConnected(PartyRsrcID PartyId,DWORD numOfConnectedSlaves)//if numOfConnectedSlaves==0 Only main was connected
{
	CSegment* seg = new CSegment;
	*seg
		<< (OPCODE)ALL_MS_IN_SLAVES_CONNECTED
		<< PartyId
		<< numOfConnectedSlaves;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendAvMcuLocalRMXMsi(PartyRsrcID PartyId, DWORD RmxLocalAudioMSI, DWORD RmxLocalVideoMSI)
{
	CSegment* seg = new CSegment;
	*seg
		<< (OPCODE)LOCAL_RMX_AV_MCU_MSI
		<< PartyId
		<< RmxLocalAudioMSI
		<< RmxLocalVideoMSI;

	SendMsg(seg, VIDEO_BRIDGE_MSG);
}
//eFeatureRssDialin
//--------------------------------------------------------------------------
void CConfApi::SendRecordingControlAckToConf(PartyRsrcID PartyId, BYTE status)
{
	CSegment * pSeg = new CSegment;
	*pSeg << PartyId
	      << SRS_RECORDING_CONTROL_ACK
	      << status;

	SendMsg(pSeg, PARTY_MSG);
}
//--------------------------------------------------------------------------
void CConfApi::SendLayoutControlToConf(PartyRsrcID PartyId, BYTE layout)
{
	CSegment * pSeg = new CSegment;
	*pSeg << PartyId
	      << SRS_LAYOUT_CONTROL_PARTY_TO_CONF
	      << layout;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::PartyToPartyCntlMsgFromMSSlaveToMain(PartyRsrcID PartyId, DWORD opcode, CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;
	*pSeg << PartyId
	      << PARTY_PARTYCONTROL_MSSLAVE_TO_MAIN
	      << (DWORD)opcode;

	if (NULL != pMsg)
		*pSeg << *pMsg;

	SendMsg(pSeg, PARTY_MSG);
}

//--------------------------------------------------------------------------
void CConfApi::SendByeToConf(const char* pPartyName)
{
	CSegment * pSeg = new CSegment;
	*pSeg << pPartyName;
	SendMsg(pSeg, VIDEORECOVERYDISCONNECTPARTYCORDING);
}
//--------------------------------------------------------------------------
// TELEPRESENCE_LAYOUTS
void CConfApi::UpdateConfTelepresenceLayoutMode(ETelePresenceLayoutMode newLayoutMode)
{
	CSegment * pSeg = new CSegment;
	*pSeg << (DWORD)newLayoutMode;
	SendMsg(pSeg, SET_TELEPRESENCE_LAYOUT_MODE);
}
//--------------------------------------------------------------------------

//---------------------------// VNGFE-8204 // --------------------------------------
void CConfApi::ChangePartyContentBitRateByLpr(PartyRsrcID PartyId, DWORD newContentRate, DWORD isDba)
{
	CSegment*  pSeg = new CSegment;
	*pSeg << (DWORD)PartyId
		 << (WORD)CHANGE_CONTENT_BIT_RATE_BY_LPR
		 << (DWORD)newContentRate
		 << (DWORD)isDba;

	SendMsg(pSeg, PARTY_MSG);
}

