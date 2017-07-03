#include "NStream.h"
#include "ConfAppMngr.h"
#include "ConfAppPartiesList.h"
#include "StatusesGeneral.h"
#include "PartyApi.h"
#include "ConfPartyOpcodes.h"
#include "OsFileIF.h"
#include "ObjString.h"
#include "TraceStream.h"
#include "SysConfigKeys.h"
#include "CommConfDB.h"
#include "OpcodesMcmsAudio.h"
#include "Party.h"
#include "PrettyTable.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "BridgePartyDisconnectParams.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////
//                        CConfAppPartyParams
////////////////////////////////////////////////////////////////////////////
CConfAppPartyParams::CConfAppPartyParams(CConfAppInfo* confAppInfo)
	: m_confAppInfo(confAppInfo)
	, m_partyId(0)
	, m_pPartyApi(NULL)
	, m_partyName("")
	, m_rollCallName("")
	, m_rollCallRecDuration(0)
	, m_rollCallCheckSum(0)
	, m_isRollCallRecordingExists(false)
	, m_isRollCallEntryTonePlayed(false)
	, m_rollCallInRecording(false)
	, m_isInStopRecording(false)
	, m_pBridgePartyDisconnectParams(NULL)
	, m_partyAudioState(eAPP_PARTY_STATE_IDLE)
	, m_partyVideoState(eAPP_PARTY_STATE_IDLE)
	, m_isLeader(false)
	, m_lastForceInd(-1)
	, m_isNoisyLine(false)
	, m_noisyLineThresholdLevel(m_confAppInfo->GetDefualtNoisyLineThresholdLevel())
	, m_dtmfIndSource(0)
	, m_forcedDTMFIndSourceEnum(eDTMFSourceNone)
	, m_lastSetDtmfSourceTime(0)
	, m_isSlideNoLongerPermitted(false)
	, m_isVideoBridgeReadyForSlide(false)
	, m_isPartyReadyForSlide(false)
	, m_isSlideOn(false)
	, m_isCascadeLinkParty(false)
	, m_isRecordingLinkParty(false)
	, m_partyStartIvrMode(START_IVR_PARTY_STOPPED)
	, m_GatewayPartyType(eRegularPartyNoGateway)
	, m_isPutOnHoldDuringSlide(false)
{
	m_loggerPartyLastReport = 0;
	m_loggerPartyStarted = GetCurrentLoggerNumber();
}

/////////////////////////////////////////////////////////////////
CConfAppPartyParams::~CConfAppPartyParams()
{
	if (m_pPartyApi)
		m_pPartyApi->DestroyOnlyApi();
	POBJDELETE(m_pPartyApi);

///	if (IsRollCallRecordingExists())
	if (0 != GetPartyRollCallName().length())	// check if record Roll Call was requested for this party and not yet deleted
	{
		string recordingName = GetRollCallRecFullPath();
		if (IsFileExists(recordingName))
			if (!DeleteFile(recordingName))
				TRACEINTO
					<< "Failed to delete Roll Call Recording file:"
					<< recordingName;
	}
	if(m_pBridgePartyDisconnectParams){
		POBJDELETE(m_pBridgePartyDisconnectParams);
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppPartyParams::SetPartyAudioState(TAppPartyState state)
{
	if (state < eAPP_PARTY_STATE_MAX)
	{
		m_partyAudioState = state;
		TRACEINTO << "setting partyId " << m_partyId << " to state " << state;
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppPartyParams::SetPartyVideoState(TAppPartyState state)
{
	if (state < eAPP_PARTY_STATE_MAX)
		m_partyVideoState = state;
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartyParams::IsSuitableForForce()
{
	return (eAPP_PARTY_STATE_MIX == GetPartyVideoState()) ? true : false;
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartyParams::isValidDtmfIndSource(DWORD sourceOpcode) const
{
	bool returnCode = false;

	switch (m_forcedDTMFIndSourceEnum)
	{
		case eDTMFSourceAuto:
			switch (m_dtmfIndSource)
			{
				case SIGNALLING_DTMF_INPUT_IND:
				case RTP_DTMF_INPUT_IND:
					returnCode = (sourceOpcode == SIGNALLING_DTMF_INPUT_IND || sourceOpcode == RTP_DTMF_INPUT_IND);
					break;

				case AUD_DTMF_IND_VAL:
					returnCode = (sourceOpcode == AUD_DTMF_IND_VAL);
					break;
			}
			break;

		//now check the dtmf source that we got if it is as the last one
		case eDTMFSourceOutband:
			returnCode = (sourceOpcode == SIGNALLING_DTMF_INPUT_IND || sourceOpcode == RTP_DTMF_INPUT_IND);
			break;

		case eDTMFSourceInband:
			returnCode = (sourceOpcode == AUD_DTMF_IND_VAL);
			break;

		default:
			break;
	}

	TRACEINTO
		<< "ForcedDTMFIndSourceEnum:" << m_forcedDTMFIndSourceEnum
		<< ", DtmfIndSource:" << m_dtmfIndSource
		<< ", SourceOpcode:" << sourceOpcode
		<< ", ReturnCode:" << (int)returnCode;

	return returnCode;
}

/////////////////////////////////////////////////////////////////
bool isVoIpParty(const CConfParty* pParty)
{
	switch (pParty->GetNetInterfaceType())
	{
	case PSTN_INTERFACE_TYPE:
	case ISDN_INTERFACE_TYPE:
		return false;

	default:
		return pParty->GetVoice();
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppPartyParams::SetDtmfIndSource(DWORD sourceOpcode)
{
	//VNGFR-4884 - 2011-11-20 - Participant connected for1 hour in a conference couldn't send DTMF commands.
	//set DTMF source - if it is different from the previous one && `CFG_KEY_SET_DTMF_SOURCE_DIFF_IN_SEC` seconds passes

	//VNGFE-4884: 2012-08-05 - provide separate default for DTMF source for Audio-Only (VOIP) EPs

	time_t currentTime = 0;
	GetCurrentTime(currentTime);

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	struct ForcedDtmfType
	{
		const char* dtmfSourceKey;
		const char* dtmfAutoTimerKey;
	} src[] = {
		{ CFG_KEY_ACCEPT_H323_DTMF_TYPE, CFG_KEY_SET_DTMF_SOURCE_DIFF_IN_SEC },
		{ CFG_KEY_ACCEPT_VOIP_DTMF_TYPE, CFG_KEY_SET_DTMF_SOURCE_DIFF_IN_SEC_VOIP },
	};

	const CCommConf* pConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	const CConfParty* pParty = IsValidPObjectPtr(pConf) ? pConf->GetCurrentParty(GetPartyName().c_str()) : NULL;
	PASSERT_AND_RETURN(!pParty);

	const bool audioOnly = isVoIpParty(pParty);
	const size_t iSrc = audioOnly ? 1 : 0;

	if (eDTMFSourceNone == m_forcedDTMFIndSourceEnum)
	{
		DWORD forcedDtmfSource = 0;
		sysConfig->GetDWORDDataByKey(src[iSrc].dtmfSourceKey, forcedDtmfSource);
		m_forcedDTMFIndSourceEnum = static_cast<EDTMFSourceEnum>(forcedDtmfSource);
	}

	TRACEINTO
		<< "AudioOnly:" << audioOnly
		<< ", NetworkInterface:" << (int)pParty->GetNetInterfaceType()
		<< ", ForcedDTMFIndSourceEnum:" << m_forcedDTMFIndSourceEnum
		<< ", DtmfIndSource:" << m_dtmfIndSource
		<< ", SourceOpcode:" << sourceOpcode;

	if (eDTMFSourceAuto == m_forcedDTMFIndSourceEnum)
	{
		DWORD setDtmfSourceDiffInSec = 120;
		sysConfig->GetDWORDDataByKey(src[iSrc].dtmfAutoTimerKey, setDtmfSourceDiffInSec);

		if (m_dtmfIndSource != sourceOpcode &&
			(DWORD)(currentTime - m_lastSetDtmfSourceTime) > setDtmfSourceDiffInSec)
		{
			TRACEINTO
				<< "SourceOpcode:" << sourceOpcode << ", sec pass from last set:"
				<< (currentTime - m_lastSetDtmfSourceTime) << ", limit is:" << setDtmfSourceDiffInSec;

			m_dtmfIndSource = sourceOpcode;
		}

		m_lastSetDtmfSourceTime = currentTime;
	}
}

/////////////////////////////////////////////////////////////////
const string CConfAppPartyParams::GetRollCallRecFullPath() const
{
	string recordingName = IVR_FOLDER_ROLLCALL + m_rollCallName;
	return recordingName;
}

/////////////////////////////////////////////////////////////////
void CConfAppPartyParams::ComposeAndSetRollCallName()
{
	CSmallString tempRollCallName;
	tempRollCallName << "RC_" << m_partyId <<  '_' << m_confAppInfo->GetConfRsrcID() << ".WAV";
	SetPartyRollCallName(tempRollCallName.GetString());
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartyParams::IsPartyAudioStateConnected()
{
	switch (GetPartyAudioState())
	{
		case eAPP_PARTY_STATE_IVR_ENTRY:
		case eAPP_PARTY_STATE_IVR_FEATURE:
		case eAPP_PARTY_STATE_CONNECTING_TO_MIX:
		case eAPP_PARTY_STATE_MIX:
			return true;

		default:
			return false;
	}
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartyParams::IsPartyVideoStateConnected()
{
	switch (GetPartyVideoState())
	{
		case eAPP_PARTY_STATE_IVR_ENTRY:
		case eAPP_PARTY_STATE_IVR_FEATURE:
		case eAPP_PARTY_STATE_CONNECTING_TO_MIX:
		case eAPP_PARTY_STATE_MIX:
			return true;

		default:
			return false;
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppPartyParams::StopRollCallRecording(CBridgePartyDisconnectParams* pBridgePartyDisconnectParams)
{
	m_pPartyApi->SendOpcodeToIvrSubFeature(STOP_ROLL_CALL_RECORDING);
	if(NULL != m_pBridgePartyDisconnectParams){
		POBJDELETE(m_pBridgePartyDisconnectParams);
	}
	if(NULL != pBridgePartyDisconnectParams){
		m_pBridgePartyDisconnectParams = new CBridgePartyDisconnectParams(*pBridgePartyDisconnectParams);
	}else{
		TRACEINTO << "pBridgePartyDisconnectParams is NULL";
	}
}
/////////////////////////////////////////////////////////////////

template <typename _T> void delete_element(_T& T) { delete T; }


/////////////////////////////////////////////////////////////////
// ConfAppPartiesList
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
CConfAppPartiesList::CConfAppPartiesList(CConfAppInfo *confAppInfo)
{
	m_leaderInConf = 0;
	m_confAppInfo = confAppInfo;
	m_chairPartyId = DUMMY_PARTY_ID;
}

/////////////////////////////////////////////////////////////////
CConfAppPartiesList::~CConfAppPartiesList()
{
	std::for_each(m_partyList.begin(), m_partyList.end(), &delete_element<PartyVector::value_type>);

	std::for_each(m_partyDeletedList.begin(), m_partyDeletedList.end(), &delete_element<PartyVector::value_type>);
}

/////////////////////////////////////////////////////////////////
int CConfAppPartiesList::AddOrUpdateParty(DWORD opcode, DWORD confWithLocalIVR, BYTE confWithExternalIVR, CSegment* pParam)
{
	PartyRsrcID partyId = 0;
	CTaskApp* pParty = NULL;
	char partyName[H243_NAME_LEN];
	WORD mediaDirection;
	COsQueue partyAppRcvMbx;
	DWORD audioOrVideo = (DWORD)-1;
	*pParam >> partyId >> (void*&)pParty >> partyName >> mediaDirection;

	std::string externalIvrFile;
	if (eCAM_EVENT_PARTY_VIDEO_IVR_MODE_CONNECTED == opcode)
		*pParam >> externalIvrFile;

	partyAppRcvMbx.DeSerialize(*pParam);

	int partyIndex = FindPartyIndex(partyId);

	if (-1 == partyIndex)		// The party is not in the list yet
	{
		TRACEINTO << "PartyId:" << partyId << ", PartyName:" << partyName << " - New party was added";

		// Insert party to the list
		int numOfParties = m_partyList.size();	// number of parties before adding the new one
		CConfAppPartyParams* partyParams = new CConfAppPartyParams(m_confAppInfo);
		partyParams->m_partyId   = partyId;
		partyParams->m_partyName = partyName;

		// Create party API for each party which is added to the list
		partyParams->m_pPartyApi = new CPartyApi;
		partyParams->m_pPartyApi->CreateOnlyApi(partyAppRcvMbx);

		// Set isLeader value
		partyParams->m_isLeader = 0; // no

		m_partyList.push_back(partyParams);

		partyIndex = numOfParties;
	}

	switch (opcode)
	{
		// In case of an Audio Connection indication
		case eCAM_EVENT_PARTY_AUDIO_CONNECTED:
		{
			TRACEINTO << "PartyId:" << partyId << ", PartyName:" << partyName << ", LocalIVR:" << confWithLocalIVR << ", ExternalIVR:" << (int)confWithExternalIVR << ", AudioState:" << m_partyList[partyIndex]->m_partyAudioState << " - Party AUDIO connected";

			if (confWithLocalIVR)
			{
				if (eAPP_PARTY_STATE_IDLE == m_partyList[partyIndex]->m_partyAudioState)
				{
					// Start IVR entrance:
					SendSetAsLeaderIfNeeded(partyId, partyIndex);
					// Send message to party - CAM is ready to start IVR

					(m_partyList[partyIndex]->m_pPartyApi)->SendMsg(NULL, CAM_READY_FOR_IVR);
					// Set audio state to IVR_ENTRY
					m_partyList[partyIndex]->m_partyAudioState = eAPP_PARTY_STATE_IVR_ENTRY;
					TRACEINTO << "PartyId:" << partyId << ", NewAudioState:eAPP_PARTY_STATE_IVR_ENTRY";
				}
				audioOrVideo = CAM_AUDIO;
				// If the state is eAPP_PARTY_STATE_DISCONNECTED - we will start the wait event that follows the IVR entrance
				// from the ConfAppMngr (eCAM_EVENT_PARTY_END_IVR) >> we will change the state there
			}
			else if (confWithExternalIVR)
			{
				audioOrVideo = CAM_AUDIO;
				SendSetAsLeaderIfNeeded(partyId, partyIndex);
				(m_partyList[partyIndex]->m_pPartyApi)->SendMsg(NULL, CAM_READY_FOR_IVR);

				m_partyList[partyIndex]->m_partyAudioState = eAPP_PARTY_STATE_IVR_ENTRY;
				TRACEINTO << "PartyId:" << partyId << ", NewAudioState:eAPP_PARTY_STATE_IVR_ENTRY";
			}
			else	// confWithIVR == 0
			{
				m_partyList[partyIndex]->m_partyAudioState = eAPP_PARTY_STATE_MIX;
				TRACEINTO << "PartyId:" << partyId << ", NewAudioState:eAPP_PARTY_STATE_MIX";
			}

			break;
		}

		// In case of a Video Connection indication
		case eCAM_EVENT_PARTY_VIDEO_CONNECTED:
		{
			TRACEINTO << "PartyId:" << partyId << ", PartyName:" << partyName << " - Party VIDEO connected";

			// Set video state to in mix
			m_partyList[partyIndex]->m_partyVideoState = eAPP_PARTY_STATE_MIX;

			audioOrVideo = CAM_VIDEO;
			break;
		}

		// In case of a Video IVR Mode Connection indication (video connected for show slide)
		case eCAM_EVENT_PARTY_VIDEO_IVR_MODE_CONNECTED:
		{
			TRACEINTO << "PartyId:" << partyId << ", PartyName:" << partyName << " - Party VIDEO-IVR connected";

			// VNGFE-7937 (Cloned to Bridge-12005)
			if (m_partyList[partyIndex]->m_partyVideoState != eAPP_PARTY_STATE_MOVING)
			{
				// Set video state to IVR
				m_partyList[partyIndex]->m_partyVideoState = eAPP_PARTY_STATE_IVR_ENTRY;
			}
			else
			{
				TRACEINTO << "PartyId:" << partyId << ", PartyName:" << partyName << " - Move has been started so not change state";
			}
			audioOrVideo = CAM_VIDEO;
			break;
		}

		case eCAM_EVENT_PARTY_ON_HOLD:
		{
			WORD isHoldInIVR, isHoldWhileWaitingForChair;
			*pParam >> isHoldInIVR >> isHoldWhileWaitingForChair;
			bool isPutOnHoldDuringSlide = isHoldInIVR || isHoldWhileWaitingForChair;
			TRACEINTO << "PartyId:" << partyId << ", IsHoldInIVR:" << isHoldInIVR << ", IsHoldWhileWaitingForChair:" << isHoldWhileWaitingForChair;
			// Set flag in party info
			m_partyList[partyIndex]->m_isPutOnHoldDuringSlide = isPutOnHoldDuringSlide;
			break;
		}

		default:
		{
			return -1;
			break;
		}
	}
	if (audioOrVideo != (DWORD)-1)
	{
		CSegment partySeg;
		partySeg << (DWORD)audioOrVideo;
		m_partyList[partyIndex]->m_pPartyApi->SendMediaConnectedByCAMToParty(&partySeg);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////
int	CConfAppPartiesList::DelParty(PartyRsrcID partyId)
{
	return DelPartyIndex(FindPartyIndex(partyId));
}

/////////////////////////////////////////////////////////////////
int CConfAppPartiesList::DelParty(string name)
{
	return DelPartyIndex(FindPartyIndex(name));
}

/////////////////////////////////////////////////////////////////
int CConfAppPartiesList::DelPartyIndex(WORD ind)
{
	if ((ind == (WORD)(-1)) || (ind >= m_partyList.size()))
		return STATUS_FAIL;     // illegal index

	POBJDELETE(m_partyList[ind]);

	m_partyList.erase(m_partyList.begin() + ind);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////
int CConfAppPartiesList::FindPartyIndex(PartyRsrcID partyId)
{
	size_t size = m_partyList.size();
	for (size_t i = 0; i < size; ++i)
		if (m_partyList[i]->GetPartyRsrcId() == partyId)
			return i;
	return -1;      // not found
}

/////////////////////////////////////////////////////////////////
CConfAppPartyParams* CConfAppPartiesList::GetParty(PartyRsrcID partyId)
{
	int ind = FindPartyIndex(partyId);
	if (-1 == ind)
		return NULL;
	return m_partyList[ind];
}

/////////////////////////////////////////////////////////////////
int CConfAppPartiesList::FindPartyIndex(string name)
{
	size_t size = m_partyList.size();
	for (size_t i = 0; i < size; ++i)
		if (m_partyList[i]->GetPartyName() == name)
			return i;
	return -1;      // not found
}

/////////////////////////////////////////////////////////////////
int CConfAppPartiesList::FindPartyDelIndex(PartyRsrcID partyId)
{
	size_t size = m_partyDeletedList.size();
	for (size_t i = 0; i < size; ++i)
		if (m_partyDeletedList[i]->GetPartyRsrcId() == partyId)
			return i;
	return -1;      // not found
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::ChangePartyState(PartyRsrcID partyId, DWORD opcode)
{
	int ind = FindPartyIndex(partyId);
	if (-1 == ind)
	{
		TRACEINTO << "PartyId:" << partyId << "- Failed, Party not found";
		PASSERT_AND_RETURN(1);
	}

	const CConfAppPartyParams* pPartyParams = GetParty(partyId);
	PASSERT_AND_RETURN(!pPartyParams);

	TRACEINTO << "PartyName:" << pPartyParams->m_partyName << ", PartyId:" << pPartyParams->m_partyId << ", Opcode:" << opcode;
	switch (opcode)
	{
		case eCAM_EVENT_PARTY_DELETED:
		{
			if (m_confAppInfo->GetIsEntryQueue())
			{
				DelPartyIndex(ind);
			}
			else                                          // Not EQ and (or Roll Call recording exists or is leader)
			{
				// Set party state to DELETED and move it to partyDeletedList (party in DELETED state is always in the deleted list)
				m_partyList[ind]->SetPartyAudioState(eAPP_PARTY_STATE_DELETED);
				m_partyList[ind]->SetPartyVideoState(eAPP_PARTY_STATE_DELETED);
				MovePartyToDeletedList(ind);
			}
			break;
		}

		case eCAM_EVENT_PARTY_AUDIO_MOVE_OUT:
			DelPartyIndex(ind);
			break;

		case eCAM_EVENT_PARTY_VIDEO_MOVE_OUT:
			m_partyList[ind]->SetPartyVideoState(eAPP_PARTY_STATE_DISCONNECTED);
			break;

		case eCAM_EVENT_PARTY_END_IVR:
			m_partyList[ind]->SetPartyAudioState(eAPP_PARTY_STATE_IVR_FEATURE);
			if (m_partyList[ind]->GetPartyVideoState() == eAPP_PARTY_STATE_IVR_ENTRY)
				m_partyList[ind]->SetPartyVideoState(eAPP_PARTY_STATE_IVR_FEATURE);
			break;

		case PARTY_VIDEO_IVR_MODE_CONNECTED:
			m_partyList[ind]->SetPartyVideoState(eAPP_PARTY_STATE_IVR_ENTRY);
			break;

		case PARTY_VIDEO_CONNECTED:
			m_partyList[ind]->SetPartyVideoState(eAPP_PARTY_STATE_MIX);
			break;
	}
}

/////////////////////////////////////////////////////////////////
TAppPartyState CConfAppPartiesList::GetPartyAudioState(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", eAPP_PARTY_STATE_MAX);

	return m_partyList[index]->GetPartyAudioState();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetPartyAudioState(PartyRsrcID partyId, TAppPartyState state)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == index, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[index]->SetPartyAudioState(state);
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::UpdateStateUponDisconnecting(PartyRsrcID partyId, DWORD audioOrVideo)
{
	int partyInd = FindPartyIndex(partyId);
	if (-1 == partyInd)
		return;
	// BRIDGE-14196 - party's external IVR cntl was not notified of disconnection
	CSegment partySeg;
	partySeg << (DWORD)audioOrVideo;
	if (m_partyList[partyInd]->GetPartyApi() != NULL)
	{
		TRACEINTO << "PartyId:" << partyId << ", AudioState:eAPP_PARTY_STATE_IDLE";
		m_partyList[partyInd]->GetPartyApi()->SendMediaDisconnectionByCAMToParty(&partySeg);
	}
	TAppPartyState audio_state = m_partyList[partyInd]->GetPartyAudioState();

	if (CAM_AUDIO == audioOrVideo)
	{
		if ((eAPP_PARTY_STATE_IVR_ENTRY == audio_state) || (eAPP_PARTY_STATE_IDLE == audio_state))
		{
			TRACEINTO << "PartyId:" << partyId << ", AudioState:eAPP_PARTY_STATE_IDLE";
			m_partyList[partyInd]->SetPartyAudioState(eAPP_PARTY_STATE_IDLE);
		}
		else
		{
			TRACEINTO << "PartyId:" << partyId << ", AudioState:eAPP_PARTY_STATE_DISCONNECTED";
			m_partyList[partyInd]->SetPartyAudioState(eAPP_PARTY_STATE_DISCONNECTED);
		}
	}
	else	// CAM_VIDEO
	{
		// BRIDGE-14692 recap caused v.bridge disconnect-reconnect during IVR.
		// if this occurs during slide display we reset the isSlideNoLongerPermitted flag to enable the slide to be displayed upon reconnection.
		if ((eAPP_PARTY_STATE_IVR_ENTRY == audio_state) || (m_confAppInfo->IsWaitForChair()))
		{
			COstrStream msg;
			msg << "video disconnected during ";
			if (eAPP_PARTY_STATE_IVR_ENTRY == audio_state)
			{
				m_partyList[partyInd]->SetSlideIsNoLongerPermitted(false);
				msg << "IVR entry stage. isSlideNoLongerPermitted flag will be reset.";
			}
			else
				msg << "\"waiting for chair\" stage";
			TRACEINTO << msg.str();
			m_partyList[partyInd]->SetSlideIsNoLongerPermitted(false);
		}
		if ((eAPP_PARTY_STATE_IVR_ENTRY == m_partyList[partyInd]->GetPartyVideoState()) || (eAPP_PARTY_STATE_IDLE == m_partyList[partyInd]->GetPartyVideoState()))
		{
			TRACEINTO << "PartyId:" << partyId << ", VideoState:eAPP_PARTY_STATE_IDLE";
			m_partyList[partyInd]->SetPartyVideoState(eAPP_PARTY_STATE_IDLE);
		}
		else
		{
			TRACEINTO << "PartyId:" << partyId << ", VideoState:eAPP_PARTY_STATE_DISCONNECTED";
			m_partyList[partyInd]->SetPartyVideoState(eAPP_PARTY_STATE_DISCONNECTED);
		}
	}

	// If audio + video states are IDLE - delete the party from parties list
	if ((eAPP_PARTY_STATE_IDLE == m_partyList[partyInd]->GetPartyAudioState()) && (eAPP_PARTY_STATE_IDLE == m_partyList[partyInd]->GetPartyVideoState()))
	{
		if (m_partyList[partyInd]->GetIsPutOnHoldDuringSlide())
			TRACEINTO << "party " << partyId << ((CAM_AUDIO == audioOrVideo) ? " CAM_AUDIO" : " CAM_VIDEO") << " is being disconnected due to \"hold\" action. will not delete from party list";
		else
		{
			// Delete the party from parties list
			DelPartyIndex(partyInd);
			TRACEINTO << " DelPartyIndex " << partyInd << " (party rsrc id " << partyId << ")";
		}
	}
}

/////////////////////////////////////////////////////////////////
TAppPartyState CConfAppPartiesList::GetPartyVideoState(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", eAPP_PARTY_STATE_MAX);

	return m_partyList[index]->GetPartyVideoState();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetPartyVideoState(PartyRsrcID partyId, TAppPartyState state)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == index, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[index]->SetPartyVideoState(state);
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetPartyAsLeader(PartyRsrcID partyId, bool isLeader)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == index, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[index]->SetPartyAsLeader(isLeader);

	m_chairPartyId = (isLeader) ? partyId : DUMMY_PARTY_ID;
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::GetIsPartyLeader(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", false);

	return m_partyList[index]->GetIsPartyLeader();
}

/////////////////////////////////////////////////////////////////
WORD CConfAppPartiesList::GetIsDeletedPartyLeader(PartyRsrcID partyId)
{
	DWORD i = 0;
	for (i = 0; i < m_partyDeletedList.size(); i++)
		if (m_partyDeletedList[i])
			if (partyId == m_partyDeletedList[i]->GetPartyRsrcId())
			{
				if (m_partyDeletedList[i]->GetIsPartyLeader())
					return 1;
				else
					return 0;
			}

	return 0;
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::IsLeaderInConf()
{
	size_t size = m_partyList.size();
	for (size_t i = 0; i < size; ++i)
		if (m_partyList[i]->GetIsPartyLeader())
			return true;
	return false; // no leader
}

/////////////////////////////////////////////////////////////////
WORD CConfAppPartiesList::GetNumOfParticipants()
{
	TRACEINTO << "NumOfParticipants: " << m_partyList.size();
	return m_partyList.size();
}

/////////////////////////////////////////////////////////////////
WORD CConfAppPartiesList::GetNumOfInMixParticipants( DWORD *fileIdArray )
{
	const CCommConf* pConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	const CConfParty* pConfParty = NULL;

	WORD numOfInMixParticipants = 0;
	DWORD i;
	for (i = 0; i < m_partyList.size(); i++)
		if (m_partyList[i]) {
			WORD partyAudioState = m_partyList[i]->GetPartyAudioState();
			//TRACEINTO << "partyName: " << m_partyList[i]->GetPartyName().c_str() << ", partyState: " << partyAudioState;
			if ((eAPP_PARTY_STATE_MIX 		  == partyAudioState) ||
				(eAPP_PARTY_STATE_IVR_FEATURE == partyAudioState) ||
				(eAPP_PARTY_STATE_DISCONNECTED == partyAudioState))
			{
				if (pConf)
					pConfParty = pConf->GetCurrentParty(m_partyList[i]->GetPartyName().c_str());

				if (fileIdArray)
				{
					if (numOfInMixParticipants < MAX_ROLL_CALL_PARTY_LIST)
					{
						fileIdArray[numOfInMixParticipants] = m_partyList[i]->GetPartyRsrcId();

						if (pConfParty && !pConfParty->IsTIPSlaveParty())
							numOfInMixParticipants++;
					}
				}
				else
				{
					if (pConfParty && !pConfParty->IsTIPSlaveParty())
							numOfInMixParticipants++;
				}
			}
		}

	TRACEINTO << "numOfInMixParticipants: " << numOfInMixParticipants;
	return numOfInMixParticipants;
}

/////////////////////////////////////////////////////////////////
DWORD CConfAppPartiesList::GetPartyInMix()
{
	//in TIP call we return the aux ID

	CCommConf* pConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	PASSERT_AND_RETURN_VALUE(!pConf, (DWORD)(-1));

	size_t size = m_partyList.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (m_partyList[i])
		{
			TAppPartyState partyAudioState = m_partyList[i]->GetPartyAudioState();

			if (eAPP_PARTY_STATE_MIX == partyAudioState)
			{
				const char* partyName = m_partyList[i]->GetPartyName().c_str();
				CConfParty* pConfParty =  pConf->GetCurrentParty(partyName);
				PASSERT_AND_RETURN_VALUE(!pConfParty, (DWORD)(-1));

				BYTE isTipAux = pConfParty->IsTIPAuxParty();
				bool isTipCall = IsTipParty(partyName);

				TRACEINTO << "PartyId:" << m_partyList[i]->GetPartyRsrcId() << ", IsTipAux:" << (int)isTipAux << ", IsTipCall:" << (int)isTipCall;

				if (isTipCall == FALSE)    //not Tip call
					return m_partyList[i]->GetPartyRsrcId();
				else if (isTipAux == TRUE) //TIP call
					return m_partyList[i]->GetPartyRsrcId();
			}
		}
	}

	return (DWORD)(-1);			// not found
}

/////////////////////////////////////////////////////////////////
const char* CConfAppPartiesList::GetPartyRollCallParams(PartyRsrcID partyId, WORD& checkSum, WORD& duration, DWORD opcode)
{
	int ind = -1;
	const char* tmp = NULL;

	if (eCAM_EVENT_CONF_EXIT_TONE == opcode)
	{
		ind = FindPartyDelIndex(partyId);
		if (-1 == ind)
			return NULL;

		bool isRollCallExists = m_partyDeletedList[ind]->IsRollCallRecordingExists();
		if (!isRollCallExists)
			return NULL;

		// get file name
		tmp = m_partyDeletedList[ind]->GetPartyRollCallNamePtr();
		if (NULL == tmp)
			return NULL;

		if (0 == strlen(tmp))
			return NULL;

		// get checkSum
		checkSum = m_partyDeletedList[ind]->GetPartyRollCallCheckSum();
		// get duration
		duration = m_partyDeletedList[ind]->GetRollCallRecDuration();
	}
	else
	{
		ind = FindPartyIndex(partyId);
		if (-1 == ind)
			return NULL;

		WORD isRollCallExists = m_partyList[ind]->IsRollCallRecordingExists();
		if (0 == isRollCallExists)
			return NULL;

		// get file name
		tmp = m_partyList[ind]->GetPartyRollCallNamePtr();
		if (NULL == tmp)
			return NULL;

		if (0 == strlen(tmp))
			return NULL;

		// get checkSum
		checkSum = m_partyList[ind]->GetPartyRollCallCheckSum();
		// get duration
		duration = m_partyList[ind]->GetRollCallRecDuration();
	}

	return tmp;
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::IsRollCallRecordingExists(PartyRsrcID partyId, WORD whereToSearch)
{
	// search party in active parties list
	if (whereToSearch == ACTIVE_LIST || whereToSearch == ALL_LISTS)
	{
		int index = FindPartyIndex(partyId);
		if (-1 != index)
			return m_partyList[index]->IsRollCallRecordingExists();
	}

	// search party in disconnected parties list
	if (whereToSearch == DISCONNECTED_LIST || whereToSearch == ALL_LISTS)
	{
		int index = FindPartyDelIndex(partyId);
		if (-1 != index)
			return m_partyDeletedList[index]->IsRollCallRecordingExists();
	}

	return false; // not found
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::MovePartyToDeletedList(WORD ind)
{
	m_partyDeletedList.push_back(m_partyList[ind]);
	m_partyList[ind] = NULL;
	DelPartyIndex(ind);
}

/////////////////////////////////////////////////////////////////
DWORD CConfAppPartiesList::GetPartyToForce(PartyRsrcID partyId)
{
	if (m_partyList.size() < 3)
	{
		PTRACE(eLevelInfoNormal, "CConfAppPartiesList::GetPartyToForce - Less than 3 participants");
		return 0;
	}

	// get the requester index
	int requesterPartyInd = FindPartyIndex(partyId);
	if (-1 == requesterPartyInd)
	{
		PTRACE(eLevelError, "CConfAppPartiesList::GetPartyToForce - Party not found");
		return 0; // error
	}

	// get the first ind to check
	int firstInd = GetFirstIndexToSearch(requesterPartyInd);

	// save first search index (and it is not the requester)
	int currentToSearch = firstInd;

	while (TRUE)
	{
		// check if suitable to be forced
		if (m_partyList[currentToSearch]->IsSuitableForForce())
		{
			m_partyList[requesterPartyInd]->SetLastForceInd(currentToSearch);
			return m_partyList[currentToSearch]->GetPartyRsrcId();
		}

		// forward the index
		currentToSearch++;

		// check if itself, and if so forward the index
		if (currentToSearch == requesterPartyInd)
			currentToSearch++;

		if ((DWORD)currentToSearch >= m_partyList.size())
		{
			currentToSearch = 0;
			if (currentToSearch == requesterPartyInd)
				currentToSearch++;  // there are at least 3 parties
		}

		// check if we end the search
		if (currentToSearch == firstInd)
			break;
	}

	PTRACE(eLevelInfoNormal, "CConfAppPartiesList::GetPartyToForce - Suitable Party not found");
	return 0;
}

/////////////////////////////////////////////////////////////////
int CConfAppPartiesList::GetFirstIndexToSearch( int requesterPartyInd )
{
	if (1 == m_partyList.size())
		return 0;	// should not happened

	int lastForceInd = m_partyList[requesterPartyInd]->GetLastForceInd();
	lastForceInd++;
	if (lastForceInd == requesterPartyInd)	// equell to the requester
		lastForceInd++;
	if ((DWORD)lastForceInd >= m_partyList.size())
	{
		lastForceInd = 0;
		if (lastForceInd == requesterPartyInd)	// equell to the requester
			lastForceInd++;
	}
	return lastForceInd;
}

/////////////////////////////////////////////////////////////////
DWORD CConfAppPartiesList::GetPartyByIndex( WORD ind )
{
	if (ind >= m_partyList.size())
		return (DWORD)-1;	// illegal index

	return m_partyList[ind]->GetPartyRsrcId();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::OnPartyExitToneEnded(PartyRsrcID partyId)
{
	int ind = FindPartyDelIndex(partyId);
	if (-1 != ind)
		OnPartyExitToneEnded(ind);
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::OnPartyExitToneEnded(int partyIndex)
{
	if (!m_partyDeletedList[partyIndex])
	{
		TRACEINTO << "PartyIndex:" << partyIndex;
		return;
	}
	else
	{
		TRACEINTO << "PartyIndex:" << partyIndex << ", PartyName:" << m_partyDeletedList[partyIndex]->m_partyName;

		// Delete Roll Call recording file if exists
		if (m_partyDeletedList[partyIndex]->IsRollCallRecordingExists())
		{
			string recordingName = m_partyDeletedList[partyIndex]->GetRollCallRecFullPath();
			if (IsFileExists(recordingName))
			{
				if (!DeleteFile(recordingName))
				{
					TRACEINTO << "Failed to delete Roll Call Recording file: " << recordingName;
				}
				else
					m_partyDeletedList[partyIndex]->SetPartyRollCallName("");
			}
		}
		// Remove Party from Deleted parties List
		RemovePartyFromDeletedList(partyIndex);
	}
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::IsVideoBridgeReadyForSlide(DWORD ind)
{
	if (ind >= m_partyList.size())
		return (WORD)-1;	// illegal index
	return m_partyList[ind]->IsVideoBridgeReadyForSlide();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetVideoBridgeReadyForSlide(DWORD ind, bool videoBridgeReadyForSlide)
{
	if (ind >= m_partyList.size())
		return;	// illegal index
	m_partyList[ind]->SetVideoBridgeReadyForSlide(videoBridgeReadyForSlide);
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::IsPartyReadyForSlide(DWORD ind)
{
	if (ind >= m_partyList.size())
		return false; // illegal index
	return m_partyList[ind]->IsPartyReadyForSlide();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetPartyReadyForSlide(DWORD ind, bool isPartyReadyForSlide)
{
	if (ind >= m_partyList.size())
		return;	// illegal index
	m_partyList[ind]->SetPartyReadyForSlide(isPartyReadyForSlide);
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetIsCascadeLinkParty(PartyRsrcID partyId, bool isCascadeLinkParty)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == index, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[index]->SetIsCascadeLinkParty(isCascadeLinkParty);
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::GetIsCascadeLinkParty(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", false);

	return m_partyList[index]->GetIsCascadeLinkParty();
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::GetRollCallEntryTonePlayed(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", false);

	return m_partyList[index]->GetRollCallEntryTonePlayed();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetRollCallEntryTonePlayed(PartyRsrcID partyId, bool isRollCallEntryTonePlayed)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == index, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[index]->SetRollCallEntryTonePlayed(isRollCallEntryTonePlayed);
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::IsPartyAudioStateConnected(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", false);

	return m_partyList[index]->IsPartyAudioStateConnected();
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::IsPartyVideoStateConnected(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", false);

	return m_partyList[index]->IsPartyVideoStateConnected();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetSlideIsOn(PartyRsrcID partyId, bool isSlideOn)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == index, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[index]->SetSlideIsOn(isSlideOn);
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::GetSlideIsOn(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", false);

	return m_partyList[index]->GetSlideIsOn();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetIsRecordingLinkParty(PartyRsrcID partyId, bool isRecordingLinkParty)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == index, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[index]->SetIsRecordingLinkParty(isRecordingLinkParty);
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::GetIsRecordingLinkParty(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", false);

	return m_partyList[index]->GetIsRecordingLinkParty();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::DeleteAllRollCallFiles()
{
	// Remove all parties from Deleted Parties List and delete the related Roll Call recording files
	int ind;
	if (m_partyDeletedList.size() > 0)
	{
		for (ind = m_partyDeletedList.size()-1; ind >= 0; ind--)
		{
			OnPartyExitToneEnded(ind);
		}
	}
}

/////////////////////////////////////////////////////////////////
int CConfAppPartiesList::RemovePartyFromDeletedList(WORD ind)
{
	if ((ind == (WORD)(-1)) || (ind >= m_partyDeletedList.size()))
		return STATUS_FAIL;     // illegal index

	POBJDELETE(m_partyDeletedList[ind]);
	m_partyDeletedList.erase(m_partyDeletedList.begin() + ind);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetPartyStartIvrMode(PartyRsrcID partyId, WORD startIvrMode)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == index, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[index]->SetPartyStartIvrMode(startIvrMode);
}

/////////////////////////////////////////////////////////////////
WORD CConfAppPartiesList::GetPartyStartIvrMode(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", (WORD)(-1));

	return m_partyList[index]->GetPartyStartIvrMode();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetGatewayPartyType(PartyRsrcID partyId, eGatewayPartyType type)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == index, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[index]->SetGatewayPartyType(type);
}

/////////////////////////////////////////////////////////////////
eGatewayPartyType CConfAppPartiesList::GetGatewayPartyType(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", eInvalidPartyType);

	return m_partyList[index]->GetGatewayPartyType();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetSlideIsNoLongerPermitted(PartyRsrcID partyId, bool isSlideNoLongerPermitted)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == index, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[index]->SetSlideIsNoLongerPermitted(isSlideNoLongerPermitted);
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::GetSlideIsNoLongerPermitted(PartyRsrcID partyId)
{
	int index = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", false);

	return m_partyList[index]->GetSlideIsNoLongerPermitted();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetChairPartyRsrcId(PartyRsrcID partyId)
{
	m_chairPartyId = partyId;
}

/////////////////////////////////////////////////////////////////
PartyRsrcID CConfAppPartiesList::GetChairPartyRsrcId()
{
	return m_chairPartyId;
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SendSetAsLeaderIfNeeded(PartyRsrcID partyId, int partyIndex)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	PASSERT_AND_RETURN(!pCommConf);

	CConfParty* pConfParty  = pCommConf->GetCurrentParty(m_partyList[partyIndex]->GetPartyName().c_str());
	PASSERT_AND_RETURN(!pConfParty);

	WORD isLeader = pConfParty->GetIsLeader();
	TRACEINTO << "PartyId:" << partyId << ", isLeader:" << isLeader;
	if (isLeader)
	{
		(m_partyList[partyIndex]->m_pPartyApi)->SendLeaderStatus((BYTE)isLeader);
	}
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::IsTipParty(PartyRsrcID partyId)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	PASSERTSTREAM_AND_RETURN_VALUE(!pCommConf, "ConfName:" << m_confAppInfo->GetConfName(), false);

	CParty* pParty = GetLookupTableParty()->Get(partyId);
	PASSERTSTREAM_AND_RETURN_VALUE(!pParty, "PartyId:" << partyId, false)

	CConfParty* pConfParty = pCommConf->GetCurrentParty(pParty->GetMonitorPartyId());
	PASSERTSTREAM_AND_RETURN_VALUE(!pConfParty, "pConfParty is NULL for partyId:" << partyId, false);

	BOOL isConfTIP = pCommConf->GetIsTipCompatibleVideo();
	if (isConfTIP && (pConfParty->IsTIPSlaveParty() || pConfParty->IsTIPMasterParty()))
	{
		TRACEINTO << "partyId:" << partyId << ", IsTipParty:1";
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////
BOOL CConfAppPartiesList::IsPartyOnHoldDuringSlide(PartyRsrcID partyId)
{
	int partyIndex = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == partyIndex, "PartyId:" << partyId << " - Failed, Party not found", FALSE);

	return m_partyList[partyIndex]->GetIsPutOnHoldDuringSlide();
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::SetPartyOnHoldDuringSlide(PartyRsrcID partyId, WORD onHoldDuringSlide )
{
	int partyIndex = FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == partyIndex, "PartyId:" << partyId << " - Failed, Party not found");

	m_partyList[partyIndex]->SetIsPutOnHoldDuringSlide(onHoldDuringSlide);
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::IsTipParty(const char* partyName)
{
	PASSERT_AND_RETURN_VALUE(!partyName, false);

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	PASSERTSTREAM_AND_RETURN_VALUE(!pCommConf, "ConfName:" << m_confAppInfo->GetConfName(), false);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(partyName);
	PASSERTSTREAM_AND_RETURN_VALUE(!pConfParty, "PartyName:" << partyName, false);

	BOOL isConfTIP = pCommConf->GetIsTipCompatibleVideo();
	if (isConfTIP && (pConfParty->IsTIPSlaveParty() || pConfParty->IsTIPMasterParty()))
	{
		TRACEINTO << "PartyName:" << partyName << ", IsTipParty:1";
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::IsTipMaster(PartyRsrcID partyId)
{
	CConfParty* pConfParty= GetConfParty(partyId);
	PASSERTSTREAM_AND_RETURN_VALUE(!pConfParty, "pConfParty is NULL for partyId:" << partyId, false);

	BOOL isTipParty = IsTipParty(partyId);
	BOOL isTipMaster = pConfParty->IsTIPMasterParty();

	TRACEINTO << "PartyId:" << partyId << ", IsTipParty:" << (int)isTipParty << ", IsTipMaster:" << (int)isTipMaster;

	if (!isTipParty)
		return false;

	if (isTipMaster)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////
void CConfAppPartiesList::OnTimerPrintPartiesLogDetailsTbl(DWORD currentLogNumber)
{
	DWORD listSize = m_partyList.size();
	if (0 == listSize)
		return;
	if (listSize > 100)
		listSize = 100;	// just not to print too much...

	CPrettyTable<DWORD, DWORD> tbl("Party RsrcID", "Connected in Log#");

	for (DWORD i = 0; i < listSize; i++)
		if (m_partyList[i])
			if (currentLogNumber != m_partyList[i]->GetLoggerPartyLastReport())
			{
				DWORD partyLogStarted = m_partyList[i]->GetLoggerPartyStarted();
				m_partyList[i]->SetLoggerPartyLastReport(currentLogNumber);
				tbl.Add( m_partyList[i]->GetPartyRsrcId(), partyLogStarted );
			}

	if (tbl.Size() > 0)
	{
		tbl.Sort(0);
		TRACEINTO << tbl.Get();
	}

}

/////////////////////////////////////////////////////////////////
CConfParty* CConfAppPartiesList::GetConfParty(PartyRsrcID partyId)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	PASSERTSTREAM_AND_RETURN_VALUE(!pCommConf, "ConfName:" << m_confAppInfo->GetConfName(), false);

	CParty* pParty = GetLookupTableParty()->Get(partyId);
	PASSERTSTREAM_AND_RETURN_VALUE(!pParty, "PartyId:" << partyId, false)

	CConfParty* pConfParty = pCommConf->GetCurrentParty(pParty->GetMonitorPartyId());
	PASSERTSTREAM_AND_RETURN_VALUE(!pConfParty, "pConfParty is NULL for partyId:" << partyId, false);

	return pConfParty;
}

/////////////////////////////////////////////////////////////////
bool CConfAppPartiesList::isMainITPEPOrSingleEP(PartyRsrcID partyId)
{
	CConfParty* pConfParty= GetConfParty(partyId);
	PASSERTSTREAM_AND_RETURN_VALUE(!pConfParty, "pConfParty is NULL for partyId:" << partyId, false);

	if(IsTipParty(partyId))
	{
		TRACEINTO << "partyId: " << partyId <<" is TIP party skip";
		return false;
	}

	if (pConfParty->GetTelePresenceMode() == eTelePresencePartyNone)
	{
		TRACEINTO << "Single EP partyId: " << partyId;
		return true;
	}
	else
	{
		const char* partyname = pConfParty->GetVisualPartyName();

		size_t len = strlen(partyname);
		--len;

		if (partyname[len]== '1')
		{
			TRACEINTO << "Multiple screens EP partyId: " <<  partyId;
			return true;
		}
	}
	return false;
}
