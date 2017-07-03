#include <ostream>
#include "CDRUtils.h"
#include "ConfDef.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyGlobals.h"
#include "ConfPartyManager.h"
#include "DataTypes.h"
#include "IpMfaOpcodes.h"
#include "MuxCntl.h"
#include "MuxHardwareInterface.h"
#include "Party.h"
#include "Segment.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "TraceStream.h"
#include  "UnifiedComMode.h"
#include "IsdnEncryptionCntl.h"
#include "IsdnVideoParty.h"


#define DEBUG_IGNORE_TIMERS 0

const WORD ART_RESPONSE_TIME = 2;

//--------------------------------------------------------------------------
static WORD CalcSyncFlag(WORD syncArray[])
{
	WORD flag = 1;
	for (WORD i = 0; i < NUM_MUX_SYNC_CHANNLS; i++)
		flag = flag && syncArray[i];

	return flag;
}

//--------------------------------------------------------------------------
CH230Repeated::CH230Repeated()
{
	m_audio        = 0;
	m_video        = 0;
	m_loop         = 0;
	m_T120         = 0;
	m_mcu          = 1;
	m_dataSym      = 1;
	m_modeSym      = 1;
	m_chair        = 0; // 1 - because RMX doesn't support chair control
	m_isMmsEnabled = 0; // MMS ControlOFF
}

//--------------------------------------------------------------------------
CH230Repeated::~CH230Repeated()
{ 
}

//--------------------------------------------------------------------------
void CH230Repeated::Serialize(WORD format, CSegment& H221StringSeg)
{
	if (m_mcu)
		H221StringSeg << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(MCC | Attr001);

	if (m_dataSym)
		H221StringSeg << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(MCS | Attr001);

	// Add check
	if (m_isMmsEnabled)
	{
		if (m_modeSym)
			H221StringSeg << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(MMS | Attr001);
		else
			H221StringSeg << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(Cancel_MMS | Attr001);
	}

	m_audio ? H221StringSeg << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(AIA | Attr000) :
	H221StringSeg << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(AIM | Attr000);

	// declare chair capabilities
	if (!m_loop)
		H221StringSeg << (BYTE)(Loop_Off | OTHRCMDATTR);

	if (m_T120)
		H221StringSeg << (BYTE)(Data_Apps_Esc | ESCAPECAPATTR) << (BYTE)(T120_ON | Attr011);
}

//--------------------------------------------------------------------------
void CH230Repeated::SetIsMcu(WORD onOff)
{
	m_mcu = onOff;
}

//--------------------------------------------------------------------------
void CH230Repeated::SetAudio(WORD onOff)
{
	m_audio = onOff;
}

//--------------------------------------------------------------------------
void CH230Repeated::SetVideo(WORD onOff)
{
	m_video = onOff;
}

//--------------------------------------------------------------------------
void CH230Repeated::SetLoop(WORD onOff)
{
	m_loop = onOff;
}
//--------------------------------------------------------------------------
void CH230Repeated::SetT120(WORD onOff)
{
	m_T120 = onOff;
}
//--------------------------------------------------------------------------
void CH230Repeated::SetChair(WORD onOff)
{
	m_chair = onOff;
}

//--------------------------------------------------------------------------
void CH230Repeated::SetModeSym(WORD onOff)
{
	m_modeSym = onOff;
}
//--------------------------------------------------------------------------
WORD CH230Repeated::GetModeSym() const
{
	return m_modeSym;
}
//--------------------------------------------------------------------------
WORD CH230Repeated::GetIsMmsEnabled()
{
	return m_isMmsEnabled;
}
//--------------------------------------------------------------------------
void CH230Repeated::SetIsMmsEnabled(WORD onOff)
{
	m_isMmsEnabled = onOff;
}

//--------------------------------------------------------------------------
#define  RMT230        REMOTE_CI
#define  RMSYNCHREGAIN REMOTE_MFRAME_SYNC
#define  MFSYNCHREGAIN LOCAL_MFRAME_SYNC
#define  RMTCOMMODE    REMOTE_XMIT_MODE
#define  RMTBASCAP     REMOTE_BAS_CAPS
#define  SYNCHLOSS     SYNCH_LOST


PBEGIN_MESSAGE_MAP(CMuxCntl)

	ONEVENT(H320CON,                           MUX_IDLE,           CMuxCntl::OnPartyH320ConIdle)
	ONEVENT(H320CON,                           MUX_SETUP,          CMuxCntl::OnPartyH320ConSetup)
	ONEVENT(SETCOMMODE,                        MUX_CHANGEMODE,     CMuxCntl::OnPartySetCommMode)
	ONEVENT(SETCOMMODE,                        MUX_CONNECT,        CMuxCntl::OnPartySetCommMode)
	ONEVENT(EXNGCAP,                           MUX_CHANGEMODE,     CMuxCntl::OnPartyExchngCapChangeMode)
	ONEVENT(EXNGCAP,                           MUX_CONNECT,        CMuxCntl::OnPartyExchngCapConnect)
	ONEVENT(EXNGCAP,                           MUX_SETUP,          CMuxCntl::OnPartyExchngCapSetup)
	ONEVENT(DISCONNECT,                        MUX_SETUP,          CMuxCntl::OnPartyDisconnectSetup)
	ONEVENT(DISCONNECT,                        MUX_CHANGEMODE,     CMuxCntl::OnPartyDisconnectChangeMode)
	ONEVENT(DISCONNECT,                        MUX_CONNECT,        CMuxCntl::OnPartyDisconnectConnect)
	ONEVENT(DISCONNECT,                        MUX_IDLE,           CMuxCntl::NullActionFunction)

	ONEVENT(MFSYNCHREGAIN,                     MUX_SETUP,          CMuxCntl::OnMuxMFSyncRegainSetup)
	ONEVENT(MFSYNCHREGAIN,                     MUX_CHANGEMODE,     CMuxCntl::OnMuxMFSyncRegainChangeMode)
	ONEVENT(MFSYNCHREGAIN,                     MUX_CONNECT,        CMuxCntl::OnMuxMFSyncRegainConnect)
	ONEVENT(RMSYNCHREGAIN,                     MUX_SETUP,          CMuxCntl::OnMuxSyncRegainSetup)
	ONEVENT(RMSYNCHREGAIN,                     MUX_CHANGEMODE,     CMuxCntl::OnMuxSyncRegainChangeMode)
	ONEVENT(RMSYNCHREGAIN,                     MUX_CONNECT,        CMuxCntl::OnMuxSyncRegainConnect)
	ONEVENT(END_INIT_COMM,                     MUX_SETUP,          CMuxCntl::OnMuxEndInitComSetup)

	ONEVENT(RMTCOMMODE,                        MUX_SETUP,          CMuxCntl::OnMuxRmtXmitMode)
	ONEVENT(RMTCOMMODE,                        MUX_CHANGEMODE,     CMuxCntl::OnMuxRmtXmitMode)
	ONEVENT(RMTCOMMODE,                        MUX_CONNECT,        CMuxCntl::OnMuxRmtXmitMode)
	ONEVENT(RMTBASCAP,                         MUX_SETUP,          CMuxCntl::OnMuxRmtBasSetup)
	ONEVENT(RMTBASCAP,                         MUX_CHANGEMODE,     CMuxCntl::OnMuxRmtBasChangeMode)
	ONEVENT(RMTBASCAP,                         MUX_CONNECT,        CMuxCntl::OnMuxRmtBasConnect)

	ONEVENT(SYNCHLOSS,                         MUX_SETUP,          CMuxCntl::OnMuxSyncLostSetup)
	ONEVENT(SYNCHLOSS,                         MUX_CHANGEMODE,     CMuxCntl::OnMuxSyncLostChangeMode)
	ONEVENT(SYNCHLOSS,                         MUX_CONNECT,        CMuxCntl::OnMuxSyncLostConnect)

	ONEVENT(RMT230,                            MUX_SETUP,          CMuxCntl::OnMuxRmt230Setup)
	ONEVENT(RMT230,                            MUX_CHANGEMODE,     CMuxCntl::OnMuxRmt230ChangeMode)
	ONEVENT(RMT230,                            MUX_CONNECT,        CMuxCntl::OnMuxRmt230Connect)

	ONEVENT(INITCOMTOUT,                       MUX_SETUP,          CMuxCntl::OnTimerInitComSetup)
	ONEVENT(SETCOMMODETOUT,                    MUX_SETUP,          CMuxCntl::OnTimerSetComModeSetup)
	ONEVENT(RMTAUDIOTOUT,                      MUX_SETUP,          CMuxCntl::OnTimerRmtAudioSetup)
	ONEVENT(RMTAUDIOSTABILIZETOUT,             MUX_SETUP,          CMuxCntl::OnTimerRmtAudioStabilizeSetup)
	ONEVENT(RMTCAPSTOUT,                       MUX_SETUP,          CMuxCntl::OnTimerRmtCapsSetup)

	ONEVENT(SYNCTOUT,                          ANYCASE,            CMuxCntl::OnTimerSync)

	ONEVENT(UPDATE_AUD_ALGTOUT,                ANYCASE,            CMuxCntl::OnTimerUpdateAudioAlgorithm)
	ONEVENT(MFARESPONSE_TOUT,                  ANYCASE,            CMuxCntl::OnTimerART)
	ONEVENT(ACK_IND,                           ANYCASE,            CMuxCntl::OnMplAck)
	ONEVENT(SMART_RECOVERY_UPDATE,             ANYCASE,            CMuxCntl::OnSmartRecovery)
	ONEVENT(SYNC_RECOVER_TOUT,                 ANYCASE,            CMuxCntl::OnTimerSyncRecover)

	ONEVENT(H320_RTP_VIDEO_UPDATE_PIC_IND,     MUX_SETUP,          CMuxCntl::NullActionFunction)
	ONEVENT(H320_RTP_VIDEO_UPDATE_PIC_IND,     MUX_CHANGEMODE,     CMuxCntl::OnRtpVideoUpdatePicInd)
	ONEVENT(H320_RTP_VIDEO_UPDATE_PIC_IND,     MUX_CONNECT,        CMuxCntl::OnRtpVideoUpdatePicInd)

	ONEVENT(REMOTE_ECS,                        ANYCASE,            CMuxCntl::OnMuxRemoteECS)

	ONEVENT(INTRA_AFTER_ZERO_MODE_TOUT,        MUX_SETUP,          CMuxCntl::OnTimerIntraAfterZeroMode)
	ONEVENT(INTRA_AFTER_ZERO_MODE_TOUT,        MUX_CHANGEMODE,     CMuxCntl::OnTimerIntraAfterZeroMode)
	ONEVENT(INTRA_AFTER_ZERO_MODE_TOUT,        MUX_CONNECT,        CMuxCntl::OnTimerIntraAfterZeroMode)

	ONEVENT(ZERO_MODE_AFTER_INITIAL_CAPS_TOUT, MUX_SETUP,          CMuxCntl::OnTimetZeroModeAfterInitialCaps)
	ONEVENT(ZERO_MODE_AFTER_INITIAL_CAPS_TOUT, MUX_CHANGEMODE,     CMuxCntl::OnTimetZeroModeAfterInitialCaps)
	ONEVENT(ZERO_MODE_AFTER_INITIAL_CAPS_TOUT, MUX_CONNECT,        CMuxCntl::OnTimetZeroModeAfterInitialCaps)

PEND_MESSAGE_MAP(CMuxCntl, CStateMachine);


//--------------------------------------------------------------------------
bool CMuxCntl::MediaKey::operator<(const CMuxCntl::MediaKey& other) const
{
	if (m_mediaType < other.m_mediaType)
		return true;
	else if (m_mediaType == other.m_mediaType &&
	         m_opcode < other.m_opcode)
		return true;

	return false;
}

//--------------------------------------------------------------------------
const char* CMuxCntl::MediaKey::GetMediaTypeAsString() const
{
	switch (m_mediaType)
	{
		case kIpAudioChnlType  : return "Audio media";
		case kIpVideoChnlType  : return "Video media";
		case kIpContentChnlType: return "Content media";
		case kIpFeccChnlType   : return "Data media";
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}

	return "Unknown media";
}

//--------------------------------------------------------------------------
CComMode::eMediaType CMuxCntl::MediaKey::GetMediaTypeByChnlType(kChanneltype type)
{
	switch (type)
	{
		case kIpAudioChnlType  : return CComMode::eMediaTypeAudio;
		case kIpVideoChnlType  : return CComMode::eMediaTypeVideo;
		case kIpContentChnlType: return CComMode::eMediaTypeContent;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	} // switch

	return CComMode::eMediaTypeLsd;
}

//--------------------------------------------------------------------------
const char* CMuxCntl::MediaKey::GetOpcodeAsString() const
{
	switch (m_opcode)
	{
		case H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ: return "H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ";
		case H320_RTP_UPDATE_CHANNEL_REQ          : return "H320_RTP_UPDATE_CHANNEL_REQ";
		case H320_RTP_UPDATE_CHANNEL_RATE_REQ     : return "H320_RTP_UPDATE_CHANNEL_RATE_REQ";
		case ART_CONTENT_ON_REQ                   : return "ART_CONTENT_ON_REQ";
		case ART_CONTENT_OFF_REQ                  : return "ART_CONTENT_OFF_REQ";
		case ART_EVACUATE_REQ                     : return "ART_EVACUATE_REQ";
	}
	return "Unknown Opcode";
}

//--------------------------------------------------------------------------
CMuxCntl::CMuxCntl()
{
	m_pParty                      = NULL;
	m_pLocalCap                   = new CCapH320;
	m_pTaskApi                    = new CPartyApi;
	m_pRemoteCap                  = new CCapH320;
	m_pCurLocalComMode            = new CComMode;
	m_pTargetLocalComMode         = new CComMode;
	m_pCurRemoteComMode           = new CComMode;
	m_pTargetRemoteComMode        = new CComMode;
	m_pLastRemoteComModeSentToART = new CComMode;

	for (BYTE i = 0; i < NUM_MUX_SYNC_CHANNLS; i++)
	{
		m_remoteSync[i] = 1;
		m_localSync[i]  = 1;
	}

	m_remoteSyncFlag               = 1;
	m_localSyncFlag                = 1;
	m_numChnl                      = 0;
	m_changeModeReq                = 0;
	m_isBitRateChange              = 0;
	m_isAudioChange                = 0;
	m_termNum                      = 0;
	m_mcuNum                       = 0;
	m_isEndInitCom                 = 0;
	m_isCapExchangeEnd             = FALSE;
	m_waitForH263H264Caps          = 0;
	m_isNoCapsAtEndInitCom         = 0;
	m_isZeroMode                   = 0;
	m_isAudioRcvChnlOpened         = FALSE;
	m_isAudioXmitChnlOpened        = FALSE;
	m_isAudioOpened                = FALSE;
	m_isEncryptionNegotiationEnded = FALSE;
	m_isRemoteMcu                  = FALSE;
	m_pAudModeRsrv                 = new CAudMode;
	m_pRepeatedH230                = new CH230Repeated;
	m_state                        = MUX_IDLE;
	m_NumOfAudioChanges            = 0;
	m_isFullAudioCapsReceived      = 0;
	m_zero_mode_intra_req_counter  = 0;
	m_pEncrypt                     = NULL;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CMuxCntl::CMuxCntl(CRsrcParams& rsrcDesc, CParty* pParty)
{
	m_pTaskApi = new CPartyApi;
	m_pTaskApi->CreateOnlyApi(pParty->GetRcvMbx(), this);
	m_pTaskApi->SetLocalMbx(pParty->GetLocalQueue());

	m_pParty                      = NULL;
	m_pLocalCap                   = new CCapH320;
	m_pRemoteCap                  = new CCapH320;
	m_pCurLocalComMode            = new CComMode;
	m_pTargetLocalComMode         = new CComMode;
	m_pCurRemoteComMode           = new CComMode;
	m_pTargetRemoteComMode        = new CComMode;
	m_pLastRemoteComModeSentToART = new CComMode;
	m_pRepeatedH230               = new CH230Repeated;
	m_pParty                      = pParty;
	m_pMux                        = new CMuxHardwareInterface(rsrcDesc);
	m_pMux->SetRoomId(rsrcDesc.GetRoomId());

	for (BYTE i = 0; i < NUM_MUX_SYNC_CHANNLS; i++)
	{
		m_remoteSync[i] = 1;
		m_localSync[i]  = 1;
	}

	m_remoteSyncFlag               = 1;
	m_localSyncFlag                = 1;
	m_numChnl                      = 0;
	m_changeModeReq                = 0;
	m_isBitRateChange              = 0;
	m_isAudioChange                = 0;
	m_termNum                      = 0;
	m_mcuNum                       = 0;
	m_isEndInitCom                 = 0;
	m_isCapExchangeEnd             = FALSE;
	m_waitForH263H264Caps          = 0;
	m_isNoCapsAtEndInitCom         = 0;
	m_isZeroMode                   = 0;
	m_isAudioRcvChnlOpened         = FALSE;
	m_isAudioXmitChnlOpened        = FALSE;
	m_isEncryptionNegotiationEnded = FALSE;
	m_isAudioOpened                = FALSE;
	m_isRemoteMcu                  = FALSE;
	m_pAudModeRsrv                 = new CAudMode;
	m_eContentInState              = eNoChannel;
	m_eContentOutState             = eNoChannel;
	m_state                        = MUX_IDLE;
	m_NumOfAudioChanges            = 0;
	m_isFullAudioCapsReceived      = 0;
	m_zero_mode_intra_req_counter  = 0;
	m_pEncrypt                     = new CIsdnEncryptCntl(m_pMux, m_pTaskApi, m_pParty->GetFullName());

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CMuxCntl::~CMuxCntl()
{
	POBJDELETE(m_pLocalCap);
	POBJDELETE(m_pTaskApi);
	POBJDELETE(m_pRemoteCap);
	POBJDELETE(m_pCurLocalComMode);
	POBJDELETE(m_pTargetLocalComMode);
	POBJDELETE(m_pCurRemoteComMode);
	POBJDELETE(m_pTargetRemoteComMode);
	POBJDELETE(m_pLastRemoteComModeSentToART);
	POBJDELETE(m_pMux);
	POBJDELETE(m_pRepeatedH230);
	POBJDELETE(m_pAudModeRsrv);
	POBJDELETE(m_pEncrypt);
	m_ackStatusTable.clear();
}

//--------------------------------------------------------------------------
const char* CMuxCntl::NameOf() const
{
	return "CMuxCntl";
}

//--------------------------------------------------------------------------
void CMuxCntl::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------
void CMuxCntl::Dump(WORD switchFlag)
{
	std::ostringstream ostr;
	ostr.setf(ios::left, ios::adjustfield);
	ostr.setf(ios::showbase);

	ostr << "\nCMuxCntl::Dump\n"
	     << "-----------\n"
	     << setw(20) << "this"             << (hex) << (DWORD) this                               << "\n"
	     << setw(20) << "m_pParty"         << (hex) << (DWORD)m_pParty                           << "\n"
	     << setw(20) << "m_pCurLocalComMode  "      << (hex) << (DWORD)m_pCurLocalComMode        << "\n"
	     << setw(20) << "m_pTargetLocalComMode  "   << (hex) << (DWORD)m_pTargetLocalComMode     << "\n"
	     << setw(20) << "m_pCurRemoteComMode  "     << (hex) << (DWORD)m_pCurRemoteComMode       << "\n"
	     << setw(20) << "m_pTargetRemoteComMode "   << (hex) << (DWORD)m_pTargetRemoteComMode    << "\n"
	     << setw(20) << "m_pLocalCap  "             << (hex) << (DWORD)m_pLocalCap               << "\n"
	     << setw(20) << "party name"       << (hex) << PARTYNAME                                 << "\n"
	     << setw(20) << "m_state   "       << (hex) << STATEASSTRING                             << "\n";

	PTRACE(eLevelInfoNormal, ostr.str().c_str());
}

//--------------------------------------------------------------------------
void CMuxCntl::Connect(CComMode& targetComMode, BYTE mcuNum, BYTE termNum)
{
	m_changeModeReq           = 0;
	m_isBitRateChange         = 0;
	m_isAudioChange           = 0;
	m_isEndInitCom            = 0;
	m_isCapExchangeEnd        = FALSE;
	m_waitForH263H264Caps     = 0;
	m_isFullAudioCapsReceived = 0;
	m_isNoCapsAtEndInitCom    = 0;
	*m_pTargetLocalComMode    = targetComMode;
	*m_pTargetRemoteComMode   = targetComMode;
	if (m_pTargetLocalComMode->GetOtherEncrypMode() != Encryp_On)
	{
		m_isEncryptionNegotiationEnded = TRUE;
	}
	else
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::Connect set encryption negotiation on [enc_trace] , ", PARTYNAME);
	}

	m_pLocalCap->ResetXferCap(); // reset only Xfer-rate cap and not audio cap
	m_pLocalCap->SetXferRateFromScm(targetComMode);

	m_numChnl       = m_pTargetLocalComMode->m_xferMode.GetNumChnl();
	m_termNum       = termNum;
	m_mcuNum        = mcuNum;
	*m_pAudModeRsrv = m_pTargetLocalComMode->m_audMode;

	/* send H221_INIT_COMM */
	DispatchEvent(H320CON, NULL);
}

//--------------------------------------------------------------------------
void CMuxCntl::Disconnect()
{
	DispatchEvent(DISCONNECT, NULL);
}

//--------------------------------------------------------------------------
void CMuxCntl::SetLocalCaps(CCapH320& localCap)
{
	*m_pLocalCap = localCap;
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyH320ConIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartyH320ConIdle : Name - ", PARTYNAME);
	OnPartyH320Con(pParam);
	m_state = MUX_SETUP;
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyH320ConSetup(CSegment* pParam)
{
	PASSERT(1);
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartyH320ConSetup : Name - ", PARTYNAME);
	OnPartyH320Con(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyH320Con(CSegment* pParam)
{
	CSegment initialXmitMode, initialH230;
	DWORD    H320TimeOut = MUX_CONNECT_TOUT;

	m_pRepeatedH230->Serialize(NATIVE, initialH230);

	SetInitialXmitMode(initialXmitMode);

	if (H320TimeOut)
	{
		// DEBUG TRACE
		char s[ONE_LINE_BUFFER_LEN];
		sprintf(s, "H320 timeout is:  %d ", H320TimeOut/SECOND);
		TRACESTR(eLevelInfoNormal) << " CMuxCntl::OnPartyH320Con : Name - " << PARTYNAME << "\n" << s;
		StartTimer(INITCOMTOUT, H320TimeOut);
	}
	else
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartyH320Con : \'There is no H320 timeout for this connection\' , Name - ", PARTYNAME);

	BOOL oldEpsMode = FALSE;
	if (oldEpsMode) // ::GetpSystemCfg()->GetOldEpMode()==TRUE)
	{
		// connecting old end-points:
		// sending g722.1 to SwiftSite 1 EP caused EP not to connect
		// Remove g722.1 from init com.
		if (m_pLocalCap->OnAudioCap(e_G722_1_24))
			m_pLocalCap->RemoveAudioCap(e_G722_1_24);

		if (m_pLocalCap->OnAudioCap(e_G722_1_32))
			m_pLocalCap->RemoveAudioCap(e_G722_1_32);
	}

	m_pMux->InitComm(m_numChnl, m_pTargetLocalComMode->GetChnlWidth(), // to check the chnl width should be about 1-30
	                 *m_pLocalCap, initialXmitMode, initialH230,
	                 m_pTargetLocalComMode->GetOtherRestrictMode());
}

//--------------------------------------------------------------------------
// isdn_encryption
void CMuxCntl::OnMuxRemoteECS(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRemoteECS - ", PARTYNAME);
	if (!IsValidPObjectPtr(m_pEncrypt))
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRemoteECS m_pEncrypt is not valid- ", PARTYNAME);
		PASSERT(101);
		return;
	}

	// Remote ECS events decoded in ASN1 format
	m_pEncrypt->HandleEcsEvent(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxSyncRegainSetup(CSegment* pParam)
{
	DWORD src;
	*pParam >> src;
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxSyncRegainSetup : Remote Name - ", PARTYNAME);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxSyncRegainChangeMode(CSegment* pParam)
{
	WORD  chnlNum = 0; // in bonding chnl num is always 0
	DWORD src;
	*pParam >> src;

	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxSyncRegainChnageMode : Remote Name - ", PARTYNAME);
	OnMuxSync(REMOTE, 1, chnlNum);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxSyncRegainConnect(CSegment* pParam)
{
	WORD  chnlNum = 0; // in bonding chnl num is always 0
	DWORD src;
	*pParam >> src;

	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxSyncRegainConnect : Remote Name - ", PARTYNAME);
	OnMuxSync(REMOTE, 1, chnlNum);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxMFSyncRegainSetup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CMuxCntl::OnMuxMFSyncRegainSetup : ");
	WORD  chnlNum = 0;  // in bonding chnl num is always 0
	DWORD src;
	*pParam >> src;

	PTRACE2(eLevelInfoNormal, "::OnMuxMFSyncRegainSetup : MFA on I channel, Name - ", PARTYNAME);
	if (IsValidTimer(SYNCTOUT))
		DeleteTimer(SYNCTOUT);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxMFSyncRegainChangeMode(CSegment* pParam)
{
	WORD  chnlNum = 0; // in bonding chnl num is always 0
	DWORD src;
	*pParam >> src;

	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxMFSyncRegainChangeMode : Local Name - ", PARTYNAME);
	OnMuxSync(LOCAL, 1, chnlNum); // The Mframe sync event indicate that a local mux sync was regained
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxMFSyncRegainConnect(CSegment* pParam)
{
	WORD  chnlNum = 0; // in bonding chnl num is always 0
	DWORD src;
	*pParam >> src;

	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxMFSyncRegainConnect : Local Name - ", PARTYNAME);
	OnMuxSync(LOCAL, 1, chnlNum); // The Mframe sync event indicate that a local mux sync was regained
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxSync(WORD localRemote, WORD lostRegain, WORD chnlNum)
{
	if (chnlNum >= NUM_MUX_SYNC_CHANNLS)
	{
		PTRACE2(eLevelError, "CMuxCntl::OnMuxSync illegal channel num, Name - ", PARTYNAME);
		return;
	}

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	DWORD       disconnectAfterToutSyncLoss;
	if (FALSE == sysConfig->GetDWORDDataByKey("DISCONNECT_H320_PARTY_SYNC_LOSS_TIMER_SECONDS", disconnectAfterToutSyncLoss))
	{
		disconnectAfterToutSyncLoss = 0;
	}

	switch (localRemote)
	{
		case LOCAL:
		{
			m_localSync[chnlNum] = lostRegain;
			WORD newFlagValue = ::CalcSyncFlag(m_localSync);
			if (m_localSyncFlag != newFlagValue)
			{
				if (m_localSyncFlag) // mux was sync and lost sync
				{ // timer removed - we will not disconnect call in case of sync lost
					// timer is back for a leased party (SBC) - Romem - 30.03.05
					if (disconnectAfterToutSyncLoss)
					{
						StartTimer(SYNCTOUT, disconnectAfterToutSyncLoss*SECOND);
					}
				}
				else if (!m_localSyncFlag)
				{
					if (disconnectAfterToutSyncLoss)
						DeleteTimer(SYNCTOUT);
				}
				m_pTaskApi->MuxSync(localRemote, lostRegain, chnlNum);
			}
			m_localSyncFlag = newFlagValue;
			break;
		}

		case REMOTE:
		{
			m_remoteSync[chnlNum] = lostRegain;
			WORD newFlagValue = ::CalcSyncFlag(m_remoteSync);
			if (m_remoteSyncFlag != newFlagValue)
			{
				if (newFlagValue)
				{
					// syncRegain - Ignore Caps and comm-mode from remote for 5 seconds
					StartTimer(SYNC_RECOVER_TOUT, 5*SECOND);
				}
				m_pTaskApi->MuxSync(localRemote, lostRegain, chnlNum);
			}
			m_remoteSyncFlag = newFlagValue;
			break;
		}

		default:
		{
			if (localRemote != 0)
				PASSERT(localRemote);
			else
				PASSERT(1001);

			break;
		}
	}

	if (m_remoteSyncFlag && m_localSyncFlag)
	{
		// VNGR-9420
		// when VSX is holding the content token, and suffers from sync_loss
		// it does not release the token, because it don't get our h239 extended caps
		if (m_pLocalCap->IsH239Cap())
		{
			PTRACE(eLevelInfoNormal, "SYNC REGAIN - RESEND H239 CAPS");
			SendH239ExtendedVideoCaps();
		}
	}
}

//--------------------------------------------------------------------------
#define AUDIOCMDATTR  (0x0 << 5)                 // audio commands
#define RATECMDATTR   (0x1 << 5)                 // transfer rate commands
#define LSDMLPCMDATTR (0x3 << 5)                 // lsd/mlp commands

void CMuxCntl::SetInitialXmitMode(CSegment& xmitSeg)
{
	WORD xferRateWhidth = m_pTargetLocalComMode->GetChnlWidth();
	BYTE xferRateMode   = Xfer_64;
	switch (xferRateWhidth)
	{
		case B0   : { xferRateMode = Xfer_64;    break;  }
		case H0   : { xferRateMode = Xfer_384;   break;  }
		case 2*B0 : { xferRateMode = Xfer_128;   break;  }
		case 3*B0 : { xferRateMode = Xfer_192;   break;  }
		case 4*B0 : { xferRateMode = Xfer_256;   break;  }
		case 5*B0 : { xferRateMode = Xfer_320;   break;  }
		case 8*B0 : { xferRateMode = Xfer_512;   break;  }
		case 12*B0: { xferRateMode = Xfer_768;   break;  }
		case 18*B0: { xferRateMode = Xfer_1152;  break;  }
		case 23*B0: { xferRateMode = Xfer_1472;  break;  }
		case 24*B0: { xferRateMode = Xfer_1536;  break;  }
		case 30*B0: { xferRateMode = Xfer_1920;  break;  }
		default:
		{
			PTRACE2(eLevelError, "CMuxCntl::SetInitialXmitMode : EXCEPTION !!!, Name - ", PARTYNAME);
			break;
		}
	} // switch

	xmitSeg << (BYTE)(Au_Off_F      | AUDIOCMDATTR)
	        << (BYTE)(xferRateMode  | RATECMDATTR)
	        << (BYTE)(Video_Off     | OTHRCMDATTR)
	        << (BYTE)(Encryp_Off    | OTHRCMDATTR)
	        << (BYTE)(Not_B6_H0     | OTHRCMDATTR);

	xmitSeg << (BYTE)(m_pTargetLocalComMode->GetOtherRestrictMode() | OTHRCMDATTR);

	xmitSeg << (BYTE)(LSD_Off       | LSDMLPCMDATTR)
	        << (BYTE)(MLP_Off       | LSDMLPCMDATTR)
	        << (BYTE)(Hsd_Esc       | ESCAPECAPATTR) << (BYTE)(Hsd_Com_Off   | Attr011)
	        << (BYTE)(Hsd_Esc       | ESCAPECAPATTR) << (BYTE)(H_Mlp_Off_Com | Attr011);
}

//--------------------------------------------------------------------------
void CMuxCntl::SetXmitComMode(CComMode& targetComMode, WORD bitrateFlag, WORD changeModeType)
{
	// vngfe-5527: save encryption status in current target mode IF we are setting a new mode AFTER encryption negotiation has ended (like in chagemode scenario)
	WORD encryp_val        = m_pTargetLocalComMode->GetOtherEncrypMode();
	BYTE should_disconnect = m_pTargetLocalComMode->GetShouldDisconnectOnEncrypFailure();
	*m_pTargetLocalComMode = targetComMode;
	if (m_isEncryptionNegotiationEnded) // in case we have already checked the remote caps, stay with current values
	{
		m_pTargetLocalComMode->SetOtherEncrypMode(encryp_val);
		m_pTargetLocalComMode->SetShouldDisconnectOnEncrypFailure(should_disconnect);
	}

	m_changeModeType = changeModeType;
	if (bitrateFlag)
		ON(m_isBitRateChange);
	else
		OFF(m_isBitRateChange);

	ON(m_changeModeReq);
	DispatchEvent(SETCOMMODE, NULL);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendXmitComMode(CComMode& targetComMode, WORD bitrateFlag)
{
	/* check com mode changes (per channel type) and send message to ART if it's needed */
	CheckMediaChannelsChanges(cmCapTransmit, &targetComMode, m_pLastRemoteComModeSentToART);

	/* send SET_XMIT_MODE signal to MPL */
	m_pMux->SetXmitRcvMode(targetComMode, SET_XMIT_MODE, bitrateFlag, IsPartyH239());

	/* save the com mode in order later to have a possibility to compare previos and current com mode */
	*m_pLastRemoteComModeSentToART = targetComMode;
}

//--------------------------------------------------------------------------
// if isShortCapSet is YES, we need a capset without H263 and Ns_Cap
// otherwise whole capset (default)
DWORD CMuxCntl::GetCapsetLenghtBytes(CCapH320* pCap, BYTE isShortCapSet)
{
	CCapH320 capSet = *pCap;
	if (isShortCapSet == YES)
	{
		// when we send a first capset to the end-point we don't know if the
		// end-point is able to understand Mbe commands.Therefore we can't
		// send H263 or NonStandard caps in the first capset.
		capSet.RemoveH263Caps();
		capSet.RemoveNSCap();
	}

	CSegment capSeg;
	capSet.Serialize(SERIALEMBD, capSeg);

	return capSeg.GetWrtOffset() - capSeg.GetRdOffset();
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxEndInitComSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup : Name - ", PARTYNAME);
	DWORD    rmt_caps_length, restrictCap;
	CSegment capSeg, restrictSeg;

	*pParam >> rmt_caps_length;

	BYTE* seg_data = new BYTE[rmt_caps_length+32];
	pParam->Get(seg_data, rmt_caps_length);
	capSeg.Put(seg_data, rmt_caps_length);

	BYTE dword_size = sizeof (APIU32);
	restrictSeg.Create(dword_size);
	*pParam >> restrictSeg;
	restrictSeg >> restrictCap;

	m_pRemoteCap->DeSerialize(SERIALEMBD, capSeg);

	delete[] seg_data;

	// If Conf Encrypted and Remote Cap do not Include Encryption ->Disconnect
	if (m_pTargetLocalComMode->GetOtherEncrypMode() == Encryp_On)
	{
		if (m_pRemoteCap->IsEncrypCapOn(Encryp_Cap))
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup:  remote encryption Capabilities Detected - ", PARTYNAME);
			// m_pRemoteCap->CreateCCapECS();
		}
		else if (!m_pRemoteCap->IsEncrypCapOn(Encryp_Cap))
		{
			if (m_pTargetLocalComMode->GetShouldDisconnectOnEncrypFailure())
			{
				PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup: remote caps without Encryption - Disconnecting Party - ", PARTYNAME);
				m_pTaskApi->EncryptionDisConnect(REMOTE_DEVICE_CAPABILITIES_DO_NOT_SUPPORT_ENCRYPTION);
				return;
			}
			else
			{
				PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup: remote caps without Encryption - Connecting Party without encryption - ", PARTYNAME);
				m_pTargetLocalComMode->SetOtherEncrypMode(Encryp_Off);
				m_pTargetLocalComMode->SetShouldDisconnectOnEncrypFailure(FALSE);
				ON(m_isEncryptionNegotiationEnded);
				m_pTaskApi->UpdateEncryptionCurrentStateInDB(NO);
			}
		}
	}


	DeleteTimer(INITCOMTOUT);
	ON(m_isEndInitCom);

	if (!m_pRemoteCap->IsMBECapOn(Mbe_Cap))
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup : \'remote can't accept MBE commands !!!\' , Name - ", PARTYNAME);

	// scm derestrict && party restricted
	if ((m_pTargetLocalComMode->GetOtherRestrictMode() == Derestrict) &&
	    m_pRemoteCap->OnXferCap(e_Xfer_Cap_Restrict))
	{
		PTRACE2(eLevelError, "CMuxCntl::OnMuxEndInitComSetup : \'Restrict Detection !!!\' , Name - ", PARTYNAME);
		// in video switch , in case of restrict detection, the
		// party will become secondary when the setCommonDomminator will try a change mode command

		// RMX dont support restrict - disconnect restrict only party
		// TODO: m_pTaskApi->ResteitDisConnect();
		return;
	}

	m_pTaskApi->MuxSyncInitChnl(m_pRemoteCap); // send

	// wait for initial channel to be connected (mux initial xmit mode)
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup : \'WAIT INITIAL CHANNEL TO BE OPENED !!\' Name - ", PARTYNAME);
	m_pCurLocalComMode->SetXferMode(m_pTargetLocalComMode->m_xferMode.GetChnlWidth(), 1);
	m_pCurLocalComMode->m_otherMode.SetRestrictMode(m_pTargetLocalComMode->GetOtherRestrictMode());
	ON(m_isBitRateChange);

	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup : StartTimer(SETCOMMODETOUT) Name - ", PARTYNAME);
	StartTimer(SETCOMMODETOUT, SET_COMMODE_TOUT /*::GetpConfConfig()->GetMuxSetComModeTout()*/);

	if (m_pRemoteCap->IsMBECapOn(Mbe_Cap))
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup : \'remote can accept MBE commands !!!\' , Name - ", PARTYNAME);
		// if we got Mbe capabilities from the end-point
		// send again local caps WITH h263 and h264 caps, in order to make the end-point able
		// to recieve our H263/H264 caps after recieving our Mbe caps.
		// But first we wait for mux will start to send commans to remote !!!
		if (m_pLocalCap->IsH263() || m_pLocalCap->IsH264() || !m_pLocalCap->GetNSCap()->IsEmpty() ||
		    m_pLocalCap->IsPPCap() || m_pLocalCap->IsH239Cap())
		{
			// remove H.261 fron second set
			if (m_isRemoteMcu == FALSE && (m_pLocalCap->IsH263() || m_pLocalCap->IsH264()))
			{
				PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup Remove H261 from local cap set, Name - ", PARTYNAME);
				m_pLocalCap->RemoveH261Caps();
			}

			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup local cap before exchange cap, Name - ", PARTYNAME);
			m_pLocalCap->Dump();

			// send current cap set
			ExchangeCap(*m_pLocalCap, m_pRemoteCap->IsVideoCapSupported(H263_2000));
		}

		ostringstream str;
		str <<  "Capabilities_exchange: Set Timer for remote capabilities: StartTimer(RMTCAPSTOUT) for " << RMT_CAPS_TOUT/100 << " secondes, Name - " << PARTYNAME << "\n";
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxEndInitComSetup: ", str.str().c_str());
		StartTimer(RMTCAPSTOUT, RMT_CAPS_TOUT);
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::AddH261ToLocalCapsAndSenfReCapIfNeeded()
{
	CMedString cstr;
	cstr << "CMuxCntl::AddH261ToLocalCapsAndSenfReCapIfNeeded";
	if (!m_isRemoteMcu)
	{
		m_isRemoteMcu = TRUE;
		cstr << " Remote identified as MCU (first time),";
		BYTE isH261 = (m_pLocalCap->IsH261VideoCap(V_Cif) || m_pLocalCap->IsH261VideoCap(V_Qcif));

		if (!isH261)
		{
			cstr << " Add H.261 caps to local caps and send EXCHANGE_CAPS";
			PTRACE(eLevelInfoNormal, cstr.GetString());
			m_pLocalCap->SetH261Caps(V_Cif, V_1_29_97, V_1_29_97); // add H.261 to local caps
			ExchangeCap(*m_pLocalCap, m_pRemoteCap->IsVideoCapSupported(H263_2000));
			return;
		}
		else
		{
			cstr << " local caps contains H.261 --> do nothing!";
		}
	}
	else
	{
		cstr << " Remote already identified before as MCU and caps with H.261 has been sent";
	}

	PTRACE(eLevelInfoNormal, cstr.GetString());
}

//--------------------------------------------------------------------------
// this action function is called twice in state of setup :
// 1. rmt commode for for initial channel.
// 2. rmt com mode for audio.
// 3. for any bas command in the state of setup. ( bas command only recorded to currenrScm)
void CMuxCntl::OnMuxRmtXmitModeSetup()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup : Name - ", PARTYNAME);

	if (!m_isEndInitCom)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup - rmtXmitMod received before EndInitConn: Name - ", PARTYNAME);
		return;
	}

	// check initial channel bit rate mode
	if (m_isBitRateChange)         // WE ASSUME THAT THE INITIAL CHANNEL IS CONNECTED AFTER END_INIT_COMM

	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup : \'INITIAL CHANNEL CONNECTED !!\' Name - ", PARTYNAME);
		DeleteTimer(SETCOMMODETOUT);
		OFF(m_isBitRateChange);
		m_pMux->SetXmitRcvMode(*m_pCurLocalComMode, SET_RCV_MODE);

		if (m_pTargetLocalComMode->GetOtherEncrypMode() == Encryp_On)
		{
			OpenEncryption(); // 1. Encryp_On in xmit mode
			OpenAudio();
			m_pEncrypt->HandleEvent(NULL, 0, START_ENC);
		}
		else
		{
			OpenAudio();
		}
	}

	if (m_isAudioChange)
	{
		if (m_pCurRemoteComMode->IsAudioOpen())
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup : \'REMOTE AUDIO IS OPEN\' Name - ", PARTYNAME);

			if (m_pCurLocalComMode->GetAudMode() == m_pCurRemoteComMode->GetAudMode())
			{
				PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup : Remote Audio is same as Local ", PARTYNAME);
				if (!IsValidTimer(RMTAUDIOSTABILIZETOUT))
				{
					PTRACE2(eLevelInfoNormal, "CMuxCntl::ModeSetup: Audio_Negotiation Remote Audio Open for party ", PARTYNAME);
					PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup: DeleteTimer(RMTAUDIOTOUT) Name - ", PARTYNAME);
					DeleteTimer(RMTAUDIOTOUT);

					PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup: StartTimer(RMTAUDIOSTABILIZETOUT) Name - ", PARTYNAME);
					StartTimer(RMTAUDIOSTABILIZETOUT, RMT_AUDIOSTABILIZE_TOUT);
				}
				else
				{
					PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup: wait for RMTAUDIOSTABILIZETOUT timer;  Name - ", PARTYNAME);
				}
			}
			else
			{
				PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup : wait for Remote Audio to open same as Local ", PARTYNAME);
				WORD rmt_audio_alg = m_pCurRemoteComMode->GetAudMode();
				if (IsAdvancedAudioAlg(rmt_audio_alg) && !IsAdvancedAudioAlgSupportedByRemoteCaps(rmt_audio_alg))
				{
					// this is because some polycom EP's declare on advanced audio algorithms caps in last set, long time after open it in commmode
					PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup : Remote openned advanced audio alg that not was not signaled in it's caps - ", PARTYNAME);
					PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup : we assume audio alg support by remote caps, adopt remote audio alg and end audio negotiation - ", PARTYNAME);
					// end audio selection properly
					if (IsValidTimer(RMTAUDIOTOUT))
					{
						DeleteTimer(RMTAUDIOTOUT);
					}

					if (IsValidTimer(RMTAUDIOSTABILIZETOUT))
					{
						DeleteTimer(RMTAUDIOSTABILIZETOUT);
					}

					UpdateAudioAlgorithm(m_pCurRemoteComMode->GetAudMode(), 0);
					// prevent previos changes
					OFF(m_isAudioChange);
					m_isFullAudioCapsReceived = TRUE;
					EndAudioNegotiation();
				}
			}
		}
		else
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup : wait for Remote to open Audio ", PARTYNAME);
		}
	}
	else
	{
		if (m_isAudioOpened)
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup : remote changed audio algorithm after end negotiation - ignored ", PARTYNAME);
			// audio already open - we do not change algorithm
		}
		else
		{
			// we should not be here
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSetup : remote changed audio algorithm ", PARTYNAME);
		}
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmtXmitModeConnect()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeConnect : Name - ", PARTYNAME);
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeConnect : \'Remote initiated mode change !!!\' Name - ", PARTYNAME);

	WORD onOff = 0;
	// simulate party AIM /AIA
	if (m_pTargetLocalComMode->GetAudMode() != m_pCurRemoteComMode->GetAudMode())
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeConnect :"
		        " \'REMOTE AUDIO MODE CHANGE CAUSED AUDIO MUTE \' Name - ", PARTYNAME);
	}
	else
	{
		onOff = 1;
	}

	m_pTaskApi->AudioActive(onOff, MCMS, 1);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmtXmitModeChangeMode()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeChangeMode : Name - ", PARTYNAME);

	if ((m_isBitRateChange && m_pCurRemoteComMode->m_xferMode == m_pTargetLocalComMode->m_xferMode) ||
	    *m_pTargetLocalComMode <= *m_pCurRemoteComMode || IsContentAsymetricModeInIVRMode())
	{
		if (IsContentAsymetricModeInIVRMode())
		{
			m_pCurRemoteComMode->SetContentModeOpcode(0);
		}

		// Added by Matvey 07.99
		// If we are waitnig only for video protocol change and remote didn't reach it yet,
		// we will continue to wait untill communication mode time-out.
		// For example if Target Scm is video_off and Remote Scm is H261.
		if (m_pTargetLocalComMode->GetVidMode() > m_pCurRemoteComMode->GetVidMode())
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeChangeMode : Wait for video protocol changes Name - ", PARTYNAME);
			return;
		}

		// Added by Matvey 07.99
		// If remote has changed its video protocol, we update current video mode
		if (m_pTargetLocalComMode->GetVidMode() == m_pCurRemoteComMode->GetVidMode())
			m_pCurLocalComMode->SetVidMode(m_pTargetLocalComMode->m_vidMode);

		if (m_pTargetLocalComMode->m_xferMode == m_pCurRemoteComMode->m_xferMode)
			m_pCurLocalComMode->m_xferMode = m_pTargetLocalComMode->m_xferMode;

		if (m_pTargetLocalComMode->m_lsdMlpMode == m_pCurRemoteComMode->m_lsdMlpMode)
			m_pCurLocalComMode->m_lsdMlpMode = m_pTargetLocalComMode->m_lsdMlpMode;

		if (m_pTargetLocalComMode->m_otherMode == m_pCurRemoteComMode->m_otherMode)
			m_pCurLocalComMode->m_otherMode = m_pTargetLocalComMode->m_otherMode;

		if (m_pTargetLocalComMode->m_contentMode == m_pCurRemoteComMode->m_contentMode)
			m_pCurLocalComMode->m_contentMode = m_pTargetLocalComMode->m_contentMode;

		OFF(m_isBitRateChange);
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeChangeMode : DeleteTimer(SETCOMMODETOUT) Name - ", PARTYNAME);
		DeleteTimer(SETCOMMODETOUT);
		m_state = MUX_CONNECT;
		m_pMux->SetXmitRcvMode(*m_pTargetLocalComMode, SET_RCV_MODE);
		if (m_changeModeReq)
		{
			ReplyMuxEndSetXmitComModeStatusOK();
			OFF(m_changeModeReq);
		}
	}
	else if (m_changeModeType == VIDEO_ASYMMETRIC_CHANGE_MODE_RMT_RCV)
	{
		if (m_pTargetRemoteComMode->GetVidMode() == m_pCurRemoteComMode->GetVidMode())
		{
			m_pCurLocalComMode->SetVidMode(m_pTargetRemoteComMode->m_vidMode);

			OFF(m_isBitRateChange);
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeChangeMode : "
			        "DeleteTimer(SETCOMMODETOUT) ASYMETRIC Name - ", PARTYNAME);
			DeleteTimer(SETCOMMODETOUT);
			m_state = MUX_CONNECT;
			m_pMux->SetXmitRcvMode(*m_pTargetRemoteComMode, SET_RCV_MODE);
			if (m_changeModeReq)
			{
				ReplyMuxEndSetXmitComModeStatusOK();
				OFF(m_changeModeReq);
			}
		}
		else
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeChangeMode : "
			        "UnExpected protocol is transmitted by the party. Name - ", PARTYNAME);
			// take care when timer SETCOMMODETOUT is up
		}
	}
	else // 14702
	{
		if (IsVideoAsymetricMode())
		{
			OFF(m_isBitRateChange);
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeChangeMode : DeleteTimer(SETCOMMODETOUT) ASYMETRIC Name - ", PARTYNAME);
			DeleteTimer(SETCOMMODETOUT);
			m_state = MUX_CONNECT;
			m_pMux->SetXmitRcvMode(*m_pTargetLocalComMode, SET_RCV_MODE);
			if (m_changeModeReq)
			{
				if (m_changeModeType == VIDEO_ASYMMETRIC_CHANGE_MODE_WAIT_OTHER)
					m_changeModeType = VIDEO_ASYMMETRIC_CHANGE_MODE;
				else
				{
					m_changeModeType = VIDEO_ASYMMETRIC_CHANGE_MODE_RMT_RCV;
				}
				ReplyMuxEndSetXmitComModeStatusOK();
				OFF(m_changeModeReq);
			}
		}
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmtXmitMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode : Name - ", PARTYNAME);

	if (IsMuxOnSyncRecover() == TRUE && !m_isRemoteMcu)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode : We are still in sync recover tout (5 seconds after sync regain) , Or in remote sync loss -> Ignore remote comm mode Name - ", PARTYNAME);
		return;
	}

	DWORD write_offset = pParam->GetWrtOffset();
	DWORD rmtXmode_length;
	*pParam >> rmtXmode_length;

	ostringstream str;
	str << " Name - " << PARTYNAME << " pParam_write_offset = " << write_offset << " , rmtXmode_length = " << rmtXmode_length;
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode: ", str.str().c_str());


	CComMode prevRemoteComMode = *m_pCurRemoteComMode;

	BYTE*    seg_data = new BYTE[rmtXmode_length+32];
	pParam->Get(seg_data, rmtXmode_length);

	CSegment h221_seg;
	h221_seg.Put(seg_data, rmtXmode_length);

	delete[] seg_data;

	m_pCurRemoteComMode->DeSerialize(SERIALEMBD, h221_seg);

	if (m_pCurRemoteComMode->GetOtherEncrypMode() == Encryp_On)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode : Encryp_On found in RmtXmitMode !!! Name - ", PARTYNAME);
	}

	m_pTaskApi->MuxRmtXfrMode(*m_pCurRemoteComMode); // Send Msg to party and from party update DB and set data member in CIsdnPartyCntl

	if (m_pRemoteCap)
	{
		if ((m_pRemoteCap->OnXferCap(e_Xfer_Cap_Restrict)) &&
		    (m_pCurRemoteComMode->m_otherMode.GetRestrictMode() != Restrict))
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode : \'non-restrict Xfr Mode found with Restrict Caps  !!\' , Name - ", PARTYNAME);
			// if the end-point reported restrict caps, we add restrict to the xmit mode as well
			m_pCurRemoteComMode->m_otherMode.SetRestrictMode(Restrict);       // set Xfer mode to restricted
		}

		if (IsValidTimer(RMT_ENCTOUT) && m_pCurRemoteComMode->GetOtherEncrypMode() == Encryp_On)
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode : \'Encryp_On found in RmtXmitMode - Deleting Timer RMT_ENCTOUT' , Name - ", PARTYNAME);
			DeleteTimer(RMT_ENCTOUT);
		}
	}

	/* check if there is some communication mode difference */
	// Link Party connected to the Slave conference and serve as LinkToMaster will update the ART after SetXmit (Start from Slave OR from Master)
	if ((((CIsdnVideoParty*)m_pParty)->IsPartyLinkToMaster()) && (0 != m_pCurRemoteComMode->GetContentModeContentRate()))
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode : Do NOT send to the ART the media changes - wait to SetXmitModeFirst , Name - ", PARTYNAME);

	else                                                          // close content from Slave Or NOT cascade
	{
		if (((CIsdnVideoParty*)m_pParty)->IsPartyLinkToMaster())    // close content
		{
			CComMode targetLocalComMode = *m_pTargetLocalComMode;     // When setXmitMode it will be updated
			*m_pTargetLocalComMode = *m_pCurRemoteComMode;            // the new SCM Since sendMsgToArt is always with m_pTargetLocalComMode
			BOOL     msg_sent = CheckMediaChannelsChanges(cmCapReceive, m_pCurRemoteComMode, &prevRemoteComMode);
			*m_pTargetLocalComMode = targetLocalComMode;
		}
		else
			BOOL msg_sent = CheckMediaChannelsChanges(cmCapReceive, m_pCurRemoteComMode, &prevRemoteComMode);
	}

	switch (m_state)
	{
		case MUX_SETUP:
		OnMuxRmtXmitModeSetup();
		break;
	}

	DeleteTimer(SETCOMMODETOUT);
}

//--------------------------------------------------------------------------
void CMuxCntl::SimOnMuxRmtXmitMode()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::SimOnMuxRmtXmitMode : Name - ", PARTYNAME);

	if (IsMuxOnSyncRecover() == TRUE && !m_isRemoteMcu)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::SimOnMuxRmtXmitMode : We are still in sync recover tout (5 seconds after sync regain) , Or in remote sync loss -> Ignore remote comm mode Name - ", PARTYNAME);
		return;
	}

	CComMode prevRemoteComMode = *m_pCurRemoteComMode;

	*m_pCurRemoteComMode = *m_pTargetLocalComMode;

	if (m_pCurRemoteComMode->GetOtherEncrypMode() == Encryp_On)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode : Encryp_On found in RmtXmitMode !!! Name - ", PARTYNAME);
	}

	m_pTaskApi->MuxRmtXfrMode(*m_pCurRemoteComMode); // Send Msg to party and from party update DB and set data member in CIsdnPartyCntl

	if (m_pRemoteCap)
	{
		if ((m_pRemoteCap->OnXferCap(e_Xfer_Cap_Restrict)) &&
		    (m_pCurRemoteComMode->m_otherMode.GetRestrictMode() != Restrict))
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode : \'non-restrict Xfr Mode found with Restrict Caps  !!\' , Name - ", PARTYNAME);
			// if the end-point reported restrict caps, we add restrict to the xmit mode as well
			m_pCurRemoteComMode->m_otherMode.SetRestrictMode(Restrict);       // set Xfer mode to restricted
		}

		if (IsValidTimer(RMT_ENCTOUT) && m_pCurRemoteComMode->GetOtherEncrypMode() == Encryp_On)
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode : \'Encryp_On found in RmtXmitMode - Deleting Timer RMT_ENCTOUT' , Name - ", PARTYNAME);
			DeleteTimer(RMT_ENCTOUT);
		}
	}

	/* check if there is some communication mode difference */
	// Link Party connected to the Slave conference and serve as LinkToMaster will update the ART after SetXmit (Start from Slave OR from Master)
	if ((((CIsdnVideoParty*)m_pParty)->IsPartyLinkToMaster()) && (0 != m_pCurRemoteComMode->GetContentModeContentRate()))
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitMode : Do NOT send to the ART the media changes - wait to SetXmitModeFirst , Name - ", PARTYNAME);

	else                                                          // close content from Slave Or NOT cascade
	{
		if (((CIsdnVideoParty*)m_pParty)->IsPartyLinkToMaster())    // close content
		{
			CComMode targetLocalComMode = *m_pTargetLocalComMode;     // When setXmitMode it will be updated
			*m_pTargetLocalComMode = *m_pCurRemoteComMode;            // the new SCM Since sendMsgToArt is always with m_pTargetLocalComMode
			BOOL     msg_sent = CheckMediaChannelsChanges(cmCapReceive, m_pCurRemoteComMode, &prevRemoteComMode);
			*m_pTargetLocalComMode = targetLocalComMode;
		}
		else
			BOOL msg_sent = CheckMediaChannelsChanges(cmCapReceive, m_pCurRemoteComMode, &prevRemoteComMode);
	}

	switch (m_state)
	{
		case MUX_SETUP:
		OnMuxRmtXmitModeSetup();
		break;
	}
	DeleteTimer(SETCOMMODETOUT);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmtBasChangeMode(CSegment* pParam)
{
	OnMuxRmtBas(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmtBasConnect(CSegment* pParam)
{
	OnMuxRmtBas(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmtBasSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtBasSetup : Name - ", PARTYNAME);

	OnMuxRmtBas(pParam);

	if (!m_isEndInitCom)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtBasSetup - remote caps received before EndInitConn: Name - ", PARTYNAME);
		return;
	}

	AudioNegotiationRmtCaps();

	TRACESTR(eLevelInfoNormal) << " CMuxCntl::OnMuxRmtBasSetup : Name - " << PARTYNAME << " -  m_isAudioOpened = " << (WORD)m_isAudioOpened;

	if (IsRmtCapsBetter())    /* check if the remote cap is equal or higher of our caps */
	{
		ostringstream str;
		str <<  "Capabilities_exchange: full cap set received - end of capabilities exchange" << ", Name - " << PARTYNAME;
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtBasSetup: ", str.str().c_str());
		DeleteTimer(RMTCAPSTOUT);

		m_isCapExchangeEnd        = TRUE;
		m_isFullAudioCapsReceived = TRUE;

		// end audio negotiation
		if (!m_isAudioOpened)
		{
			EndAudioNegotiation();   // will call EndMuxConnect()
		}
		else
		{
			EndMuxConnect();
		}
	}
	else        /* Otherwise start a timer that will be deleted when new caps received */
	{
		ostringstream str;
		if (m_isCapExchangeEnd && m_isFullAudioCapsReceived)
		{
			// special case - don't start the timer to prevent assert when it pops
			str << "We received rmt cap that is not better from the previous after end of caps exchange - do nothing!";
		}
		else
		{
			str <<  "Capabilities_exchange: Set Timer for remote capabilities: StartTimer(RMTCAPSTOUT) for "
			    << RMT_CAPS_TOUT/100 << " secondes\n";
			StartTimer(RMTCAPSTOUT, RMT_CAPS_TOUT);
		}

		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtBasSetup: ", str.str().c_str());
		StartTimer(RMTCAPSTOUT, RMT_CAPS_TOUT);
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::AudioNegotiationRmtCaps()
{
	if (m_isFullAudioCapsReceived == TRUE)
	{
		// audio already open - don't change it
		PTRACE2(eLevelInfoNormal, "CMuxCntl::AudioNegotiationRmtCaps remote caps after m_isFullAudioCapsReceive - do nothing : Name - ", PARTYNAME);
	}
	else
	{
		if (IsAdvancedAudioAlgReceived())
		{
			m_isFullAudioCapsReceived = TRUE;
			PTRACE2(eLevelInfoNormal, "CMuxCntl::AudioNegotiationRmtCaps full audio caps received : Name - ", PARTYNAME);
		}

		CAudMode initAudMode;
		AudioModeSelection(initAudMode, m_pTargetLocalComMode->GetOtherRestrictMode());
		CAudMode currAudMode;
		currAudMode.SetAudMode(m_pCurLocalComMode->GetAudMode());
		/* Repeat the main flow if new capabilities received and we can select better algorithm,
		   until open audio decision (by algorithm, or timer) */
		if (!(initAudMode == currAudMode))
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::AudioNegotiationRmtCaps set new audio algorithm : Name - ", PARTYNAME);
			DeleteTimer(RMTAUDIOSTABILIZETOUT);
			PTRACE2(eLevelInfoNormal, "CMuxCntl::AudioNegotiationRmtCaps : DeleteTimer(RMTAUDIOSTABILIZETOUT) and "
			        " StartTimer(RMTAUDIOTOUT) Name - ", PARTYNAME);
			// VNGFE-4652
			BOOL bUseLongRmtAudioTimer = FALSE;
			CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ISDN_CASCADE_LONG_RMT_AUDIO_NEGOTIATION, bUseLongRmtAudioTimer);
			if (bUseLongRmtAudioTimer && m_isRemoteMcu)
				StartTimer(RMTAUDIOTOUT, RMT_MCU_AUDIO_TOUT);
			else
				StartTimer(RMTAUDIOTOUT, RMT_AUDIO_TOUT);

			UpdateAudioAlgorithm(initAudMode.GetAudMode(), 0);
		}
		else
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::AudioNegotiationRmtCaps audio algorithm did not change : Name - ", PARTYNAME);
			EndAudioNegotiation();
		}
	}
}

//--------------------------------------------------------------------------
WORD CMuxCntl::IsAdvancedAudioAlgReceived()
{
	WORD isAdvancedAudioAlgReceived = 0;

	/* check which algorithms are supported by EP */
	WORD isG722_1C_48 = m_pRemoteCap->OnAudioCap(e_G722_1_Annex_C_48);
	WORD isG722_1C_32 = m_pRemoteCap->OnAudioCap(e_G722_1_Annex_C_32);
	WORD isG722_1C_24 = m_pRemoteCap->OnAudioCap(e_G722_1_Annex_C_24);
	// add here G722.1 & Siren14
	WORD isG722_1_32  = m_pRemoteCap->OnAudioCap(e_G722_1_32);
	WORD isG722_1_24  = m_pRemoteCap->OnAudioCap(e_G722_1_24);
// WORD isG722_1_16 = m_pRemoteCap->OnAudioCap(e_G722_1_16);
	WORD isSiren14_24 = m_pRemoteCap->GetNSCap()->OnSiren1424();
	WORD isSiren14_32 = m_pRemoteCap->GetNSCap()->OnSiren1432();
	WORD isSiren14_48 = m_pRemoteCap->GetNSCap()->OnSiren1448();

	ostringstream str2;
	str2 << " Name - " << PARTYNAME << "\n";
	str2 << "isG722_1C_48 = " << isG722_1C_48 << "\n";
	str2 << "isG722_1C_32 = " << isG722_1C_32 << "\n";
	str2 << "isG722_1C_24 = " << isG722_1C_24 << "\n";
	str2 << "isG722_1_32 = " << isG722_1_32 << "\n";
	str2 << "isG722_1_24 = " << isG722_1_24 << "\n";
	str2 << "isSiren14_24 = " << isSiren14_24 << "\n";
	str2 << "isSiren14_32 = " << isSiren14_32 << "\n";
	str2 << "isSiren14_48 = " << isSiren14_48 << "\n";

	if (isG722_1C_48 || isG722_1C_32 || isG722_1C_24 || isG722_1_32 || isG722_1_24 || isSiren14_24 || isSiren14_32 || isSiren14_48)
	{
		isAdvancedAudioAlgReceived = 1;
		str2 << "found advanced audio algorithm \n";
	}

	PTRACE2(eLevelInfoNormal, "CMuxCntl::CMuxCntl::IsAdvancedAudioAlgReceived remote caps contains:\n", str2.str().c_str());

	return isAdvancedAudioAlgReceived;
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxSyncLostSetup(CSegment* pParam)
{
	DWORD src;
	*pParam >> src;

	src == 0 ? PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxSyncLostSetup : Local Name - ", PARTYNAME) :
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxSyncLostSetup : Remote Name - ", PARTYNAME);

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	DWORD       disconnectAfterToutSyncLoss;
	if (FALSE == sysConfig->GetDWORDDataByKey("DISCONNECT_H320_PARTY_SYNC_LOSS_TIMER_SECONDS", disconnectAfterToutSyncLoss))
		disconnectAfterToutSyncLoss = 0;

	if (0 == src && disconnectAfterToutSyncLoss)
	{
		StartTimer(SYNCTOUT, disconnectAfterToutSyncLoss*SECOND);
	}
}
//--------------------------------------------------------------------------
void CMuxCntl::OnMuxSyncLostChangeMode(CSegment* pParam)
{
	WORD  chnlNum = 0;
	DWORD src;
	*pParam >> src;


	if (src == 0)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxSyncLostChangeMode : Local Name - ", PARTYNAME);
		OnMuxSync(LOCAL, 0, chnlNum);
	}
	else
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxSyncLostChangeMode : Remote Name - ", PARTYNAME);
		OnMuxSync(REMOTE, 0, chnlNum);
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxSyncLostConnect(CSegment* pParam)
{
	WORD  chnlNum = 0;
	DWORD src;
	*pParam >> src;

	if (src == 0)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxSyncLostConnect : Local Name - ", PARTYNAME);
		OnMuxSync(LOCAL, 0, chnlNum);
	}
	else
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxSyncLostConnect : Remote Name - ", PARTYNAME);
		OnMuxSync(REMOTE, 0, chnlNum);
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyExchngCapChangeMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartyExchngCapChangeMode : Name - ", PARTYNAME);
	OnPartyExchngCap(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyExchngCapConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartyExchngCapConnect : Name - ", PARTYNAME);
	OnPartyExchngCap(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyExchngCapSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartyExchngCapSetup : Name - ", PARTYNAME);
	OnPartyExchngCap(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyExchngCap(CSegment* pParam)
{
	WORD IsH263_2000Cap;
	*pParam >> IsH263_2000Cap;
	m_pMux->ExchangeCap(*m_pLocalCap, IsH263_2000Cap);

	if (m_pLocalCap->IsH239Cap())
	{
		SendH239ExtendedVideoCaps();
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmtBas(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtBas : Name - ", PARTYNAME);

	PASSERT(!pParam);
	if (!pParam)
		return;

	DWORD write_offset = pParam->GetWrtOffset();

	DWORD rmt_caps_length;
	*pParam >> rmt_caps_length;

	ostringstream str;
	str << "pParam_write_offset = " << write_offset << " , rmt_caps_length = " << rmt_caps_length;
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtBas: ", str.str().c_str());

	if (write_offset < rmt_caps_length + sizeof(DWORD))
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtBas : Ignore REMOTE_BAS_CAPS because rmt_caps_length > write_offset of the segment, so capability is probably garbage, Name - ", PARTYNAME);
		return;
	}

	BYTE* seg_data = new BYTE[rmt_caps_length+32];
	pParam->Get(seg_data, rmt_caps_length);

	CSegment newCapSeg;
	newCapSeg.Put(seg_data, rmt_caps_length);

	delete[] seg_data;

	CCapH320 newRmtCap;
	newRmtCap.DeSerialize(SERIALEMBD, newCapSeg);

	BOOL updateRmtCaps = FALSE;
	BOOL isReducedCaps = IsReducedCapabilities(newRmtCap);
	BOOL sysZeroMode   = FALSE;

	CSysConfig* pSysConfig  = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("ISDN_ZERO_MODE", sysZeroMode);
	if (isReducedCaps && sysZeroMode)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtBas: Zero Mode Detected , Name - ", PARTYNAME);
		StartZeroModeFlow();
		return;
	}

	if (IsMuxOnSyncRecover() == TRUE && !m_isRemoteMcu)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtBas : We are still in sync recover tout (5 seconds after sync regain),Or in remote sync loss ->  Ignore REMOTE_BAS_CAPS  Name - ", PARTYNAME);
		return;
	}

	CSmallString cstr;
	cstr << "UpdateRmtCaps = ";
	// Here we recieve second capSet from end-point after exchangeCap.
	// If there is mbe_cap and H263/H264/H239_caps on both sides(MCU <-> end-point) we must
	// update remote caps to H263/H264/H239
	if (newRmtCap.IsMBECapOn(Mbe_Cap) && m_pLocalCap->IsMBECapOn(Mbe_Cap) &&
	    ((m_pLocalCap->IsH263() && newRmtCap.IsH263()) ||
	     (m_pLocalCap->IsH264() && newRmtCap.IsH264()) ||
	     (m_pLocalCap->IsH239Cap() && newRmtCap.IsH239Cap())))
	{
		updateRmtCaps = TRUE;
		cstr << "TRUE (local and remote have Video)";
	}
	else
	{
		// Correction for PictureTel
		// If the new remote capset is different from the current remote
		// capset in Mlp/Hmlp caps, update only the relevant fields and
		// send the updated remote capset to the Party task.
		if ((newRmtCap.GetHsdHmlpBas() != m_pLocalCap->GetHsdHmlpBas()) ||
		    (newRmtCap.GetMlpBas() != m_pLocalCap->GetMlpBas()))
		{
			m_pRemoteCap->SetHsdHmlpCap(newRmtCap);
			m_pRemoteCap->SetMlpCap(newRmtCap);
		}

		// change - add video on condition for updating capabilities.
		// fixed bug: after mux sync lost FX sends caps with zero mode and we update remote caps (Ron)
		if (!newRmtCap.IsDataCap(Dxfer_Cap_6400) && m_pRemoteCap->IsDataCap(Dxfer_Cap_6400) &&
		    (newRmtCap.IsH261VideoCap(V_Qcif) || newRmtCap.IsH261VideoCap(V_Cif) ||
		     (newRmtCap.IsMBECapOn(Mbe_Cap) && (newRmtCap.IsH263() || newRmtCap.IsH264()))))
		{
			updateRmtCaps = TRUE;
			cstr << "TRUE (remote has Video and no Data)";
		}

		// In case e.p. remover video caps in previos set
		if ((newRmtCap.IsH261VideoCap(V_Qcif) || newRmtCap.IsH261VideoCap(V_Cif)) &&
		    (!(m_pRemoteCap->IsH261VideoCap(V_Qcif) && m_pRemoteCap->IsH261VideoCap(V_Cif))))
		{
			updateRmtCaps = TRUE;
			cstr << "TRUE (remote has Video and previous set has no video)";
		}

		if ((newRmtCap.OnAudioCap(e_G722_64) && !m_pRemoteCap->OnAudioCap(e_G722_64)) ||
		    (newRmtCap.OnAudioCap(e_G722_48) && !m_pRemoteCap->OnAudioCap(e_G722_48)))
		{
			updateRmtCaps = TRUE;
			cstr << "TRUE (remote added G722)";
		}
	}

	// MGC sends recap with lower capset (H.261) in order to show slide
	// we must respond this caps otherwise the change mode process in MGC side will get stuck!
	if (!updateRmtCaps && m_isRemoteMcu)
	{
		updateRmtCaps = TRUE;
		cstr << "TRUE (remote is MCU)";
	}

	if (updateRmtCaps)
	{
		/* We don't get ns/encryption/H.239 caps now because they come
		 separately from others caps, so before copying we have to store
		 old non-standard caps, otherwise we'll have wrong audio algorithm */
		CCapNS    oldNsCap;
		CCapPP*   pOldPPCap   = NULL;
		CCapECS*  pOldECSCap  = NULL;
		CCapH239* pOldH239Cap = NULL;

		oldNsCap = *m_pRemoteCap->GetNSCap();

		if (m_pRemoteCap->IsPPCap())
		{
			pOldPPCap  = new CCapPP;
			*pOldPPCap = *(m_pRemoteCap->GetPPCap());
		}

		if (m_pRemoteCap->GetCapECS())
		{
			pOldECSCap  = new CCapECS;
			*pOldECSCap = *(m_pRemoteCap->GetCapECS());
		}

		if (m_pRemoteCap->IsH239Cap())
		{
			pOldH239Cap  = new CCapH239;
			*pOldH239Cap = *(m_pRemoteCap->GetH239Caps());
		}

		/* copy new remote caps */
		*m_pRemoteCap = newRmtCap;

		/* Now copy old nsCap to new rmtCap */
		// m_pRemoteCap->SetNSCap (oldNsCap);
		if (pOldPPCap)
		{
			m_pRemoteCap->CreatePPCap(pOldPPCap);
			POBJDELETE(pOldPPCap);
		}

		if (pOldECSCap)
		{
			m_pRemoteCap->CreateCCapECS(pOldECSCap);
			POBJDELETE(pOldECSCap);
		}

		if (pOldH239Cap)
		{
			m_pRemoteCap->SetOnlyExtendedVideoCaps(pOldH239Cap);
			POBJDELETE(pOldH239Cap);
		}
	}
	else
	{
		cstr << "FALSE";
	}

	PTRACE(eLevelInfoNormal, cstr.GetString());

	m_pTaskApi->MuxRmtCap(*m_pRemoteCap, statOK);
}

//--------------------------------------------------------------------------
void CMuxCntl::ExchangeCap(CCapH320& rLocalCap, APIS16 isH263_2000Cap /* = -1 */)
{
	if (-1 == isH263_2000Cap)
		isH263_2000Cap = m_pRemoteCap->IsVideoCapSupported(H263_2000);

	if (!m_pRemoteCap->IsMBECapOn(Mbe_Cap))
		rLocalCap.RemoveNSCap();

	*m_pLocalCap = rLocalCap;
	CSegment* pParam = new CSegment;
	*pParam << (WORD)isH263_2000Cap;
	DispatchEvent(EXNGCAP, pParam);
	POBJDELETE(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::UpdateG7221(CCapH320& targetCap)
{
	BOOL isSysG722_1_320 = YES; // ::GetpSystemCfg()->GetG722_1_H320() // Ron - ask Assaf, for now assume always true

	if (TRUE == isSysG722_1_320)
	{
		switch (m_pTargetLocalComMode->m_audMode.GetBitRate())
		{
			case 0:
			{               // automatic
				targetCap.SetAudioCap(e_G722_1_24);
				targetCap.SetAudioCap(e_G722_1_32);
				break;
			}

			case 24:
			{
				targetCap.SetAudioCap(e_G722_1_24);
				break;
			}

			case 32:
			{
				targetCap.SetAudioCap(e_G722_1_32);
				break;
			}

			default:
			break;
		} // switch
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::OpenAudio()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OpenAudio: Audio_Negotiation Start for party ", PARTYNAME);

	// automatic alogrithm type selection
	CAudMode initAudMode;
	AudioModeSelection(initAudMode, m_pTargetLocalComMode->GetOtherRestrictMode());

	m_pTargetLocalComMode->SetAudMode(initAudMode);
	m_pCurLocalComMode->SetAudMode(initAudMode);

	// if G711 is selected, we try to establish  A_Law/U_Law during the first 3 seconds of the connection proccess
	if (((m_pCurLocalComMode->GetAudMode() == A_Law_OF) ||
	     (m_pCurLocalComMode->GetAudMode() == U_Law_OF) ||
	     (m_pCurLocalComMode->GetAudMode() == A_Law_48) ||
	     (m_pCurLocalComMode->GetAudMode() == U_Law_48)))
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OpenAudio : \'TRYING TO CONNECT A_Law/U_Law\' Name - ", PARTYNAME);
	}

	// if G722 is selected, we try to establish G722 during the first 3 seconds of the connection proccess
	if (((m_pCurLocalComMode->GetAudMode() == G722_m1) ||
	     (m_pCurLocalComMode->GetAudMode() == G722_m2) ||
	     (m_pCurLocalComMode->GetAudMode() == G722_m3)))
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OpenAudio : \'TRYING TO CONNECT G722\' Name - ", PARTYNAME);
	}

	// if G722.1 is selected, we try to establish G722.1 during the first 3 seconds of the connection proccess
	if (((m_pCurLocalComMode->GetAudMode() == Au_32k) ||
	     (m_pCurLocalComMode->GetAudMode() == Au_24k)))
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OpenAudio : \'TRYING TO CONNECT G722.1\' Name - ", PARTYNAME);
	}

	// if Siren14 is selected, we try to establish Siren14 during the first 3 seconds of the connection proccess
	if (((m_pCurLocalComMode->GetAudMode() == Au_Siren14_24k) ||
	     (m_pCurLocalComMode->GetAudMode() == Au_Siren14_32k) ||
	     (m_pCurLocalComMode->GetAudMode() == Au_Siren14_48k)))
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OpenAudio : \'TRYING TO CONNECT Siren14\' Name - ", PARTYNAME);
	}

	// if Siren14 is selected, we try to establish Siren14 during the first 3 seconds of the connection proccess
	if (((m_pCurLocalComMode->GetAudMode() == G722_1_Annex_C_48) ||
	     (m_pCurLocalComMode->GetAudMode() == G722_1_Annex_C_32) ||
	     (m_pCurLocalComMode->GetAudMode() == G722_1_Annex_C_24)))
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OpenAudio : \'TRYING TO CONNECT G722.1 ANNEX C\' Name - ", PARTYNAME);
	}

	if ((m_pTargetLocalComMode->IsFreeVideoRate() ||
	     m_pCurRemoteComMode->IsSameAudioBitrate(*m_pCurLocalComMode)) && m_pCurRemoteComMode->IsAudioOpen())
	{
		PTRACE2(eLevelInfoNormal,
		        "CMuxCntl::OpenAudio : \'MCU AUDIO MODE SELECTED - REMOTE AUDIO ALREADY OPENED\' !!  Name - ", PARTYNAME);
	}
	else
		PTRACE2(eLevelInfoNormal,
		        "CMuxCntl::OpenAudio : \'MCU AUDIO MODE SELECTED - WAIT FOR REMOTE TO SELECT AUDIO \' Name - ", PARTYNAME);

	// VNGFE-4652
	BOOL  bUseLongRmtAudioTimer = FALSE;
	DWORD uRmtAudTimeout        = 0;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ISDN_CASCADE_LONG_RMT_AUDIO_NEGOTIATION, bUseLongRmtAudioTimer);
	if (bUseLongRmtAudioTimer && m_isRemoteMcu)
		uRmtAudTimeout = RMT_MCU_AUDIO_TOUT;
	else
		uRmtAudTimeout = RMT_AUDIO_TOUT;

	StartTimer(RMTAUDIOTOUT, uRmtAudTimeout);
	ostringstream str;
	str <<  "Audio_Negotiation: Set Timer for remote audio algorithm: StartTimer(RMTAUDIOTOUT) for " << uRmtAudTimeout/100 << " secondes\n";    PTRACE2(eLevelInfoNormal, "CMuxCntl::OpenAudio: ", str.str().c_str());

	StartTimer(SETCOMMODETOUT, SET_COMMODE_TOUT /*::GetpConfConfig()->GetMuxSetComModeTout()*/);
	ostringstream str1;
	str1 <<  "Audio_Negotiation: Set Timer for remote comm mode: StartTimer(SETCOMMODETOUT) for " << SET_COMMODE_TOUT/100 << " secondes\n";     PTRACE2(eLevelInfoNormal, "CMuxCntl::OpenAudio: ", str1.str().c_str());

	UpdateAudioAlgorithm(m_pCurLocalComMode->GetAudMode(), 1);
}

//--------------------------------------------------------------------------
void CMuxCntl::AudioModeSelection(CAudMode& audMode, WORD rstrict)
{
	ostringstream str;
	str << "Remote caps contains: \n";

	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

	BOOL sysISDN_G711_ALAW    = FALSE;
	BOOL sysISDN_G711_ULAW    = FALSE;
	BOOL sysISDN_G722_64k     = FALSE;
	BOOL sysISDN_G722_56k     = FALSE;
	BOOL sysISDN_G722_48k     = FALSE;
	BOOL sysISDN_G722_1_32k   = FALSE;
	BOOL sysISDN_G722_1_24k   = FALSE;
	BOOL sysISDN_G722_1_C_48k = FALSE;
	BOOL sysISDN_G722_1_C_32k = FALSE;
	BOOL sysISDN_G722_1_C_24k = FALSE;
	BOOL sysISDN_SIREN_14_48k = FALSE;
	BOOL sysISDN_SIREN_14_32k = FALSE;
	BOOL sysISDN_SIREN_14_24k = FALSE;
	BOOL sysISDN_G728         = FALSE;


	pSysConfig->GetBOOLDataByKey("ISDN_G711_ALAW", sysISDN_G711_ALAW);
	pSysConfig->GetBOOLDataByKey("ISDN_G711_ULAW", sysISDN_G711_ULAW);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_64k", sysISDN_G722_64k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_56k", sysISDN_G722_56k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_48k", sysISDN_G722_48k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_1_32k", sysISDN_G722_1_32k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_1_24k", sysISDN_G722_1_24k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_1_C_48k", sysISDN_G722_1_C_48k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_1_C_32k", sysISDN_G722_1_C_32k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_1_C_24k", sysISDN_G722_1_C_24k);
	pSysConfig->GetBOOLDataByKey("ISDN_SIREN_14_48k", sysISDN_SIREN_14_48k);
	pSysConfig->GetBOOLDataByKey("ISDN_SIREN_14_32k", sysISDN_SIREN_14_32k);
	pSysConfig->GetBOOLDataByKey("ISDN_SIREN_14_24k", sysISDN_SIREN_14_24k);
	pSysConfig->GetBOOLDataByKey("G728_ISDN", sysISDN_G728);

	/* check which algorithms are supported by EP */
	WORD isAlaw      = m_pRemoteCap->OnAudioCap(e_A_Law); // G711
	WORD isUlaw      = m_pRemoteCap->OnAudioCap(e_U_Law); // G711
	WORD isG722_64   = m_pRemoteCap->OnAudioCap(e_G722_64);
	WORD isG722_56   = m_pRemoteCap->OnAudioCap(e_G722_56);
	WORD isG722_48   = m_pRemoteCap->OnAudioCap(e_G722_48);
	WORD isG728      = FALSE;
	BOOL bIsG728ISDN = FALSE;
	pSysConfig->GetBOOLDataByKey("G728_ISDN", bIsG728ISDN);
	if (bIsG728ISDN)
	{
		isG728 = m_pRemoteCap->OnAudioCap(e_Au_16k);
	}

	WORD isG722_1C_48 = m_pRemoteCap->OnAudioCap(e_G722_1_Annex_C_48);
	WORD isG722_1C_32 = m_pRemoteCap->OnAudioCap(e_G722_1_Annex_C_32);
	WORD isG722_1C_24 = m_pRemoteCap->OnAudioCap(e_G722_1_Annex_C_24);
	// add here G722.1 & Siren14
	WORD isG722_1_32  = m_pRemoteCap->OnAudioCap(e_G722_1_32);
	WORD isG722_1_24  = m_pRemoteCap->OnAudioCap(e_G722_1_24);
//WORD isG722_1_16  = m_pRemoteCap->OnAudioCap(e_G722_1_16);
	WORD isSiren14_24 = m_pRemoteCap->GetNSCap()->OnSiren1424();
	WORD isSiren14_32 = m_pRemoteCap->GetNSCap()->OnSiren1432();
	WORD isSiren14_48 = m_pRemoteCap->GetNSCap()->OnSiren1448();

	ostringstream str2;
	str2 << "isAlaw = " << isAlaw << "\n";
	str2 << "isUlaw = " << isUlaw << "\n";
	str2 << "isG722_64 = " << isG722_64 << "\n";
	str2 << "isG722_56 = " << isG722_56 << "\n";
	str2 << "isG722_48 = " << isG722_48 << "\n";
	str2 << "isG728 = " << isG728 << "\n";
	str2 << "isG722_1C_48 = " << isG722_1C_48 << "\n";
	str2 << "isG722_1C_32 = " << isG722_1C_32 << "\n";
	str2 << "isG722_1C_24 = " << isG722_1C_24 << "\n";
	str2 << "isG722_1_32 = " << isG722_1_32 << "\n";
	str2 << "isG722_1_24 = " << isG722_1_24 << "\n";
	str2 << "isSiren14_24 = " << isSiren14_24 << "\n";
	str2 << "isSiren14_32 = " << isSiren14_32 << "\n";
	str2 << "isSiren14_48 = " << isSiren14_48 << "\n";
	PTRACE2(eLevelInfoNormal, "CMuxCntl::AudioModeSelection remote caps contains:\n", str2.str().c_str());

	const char* pPartyName    = PARTYNAME;
	BOOL        bIsForceG711A = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_FORCE_G711A, bIsForceG711A);

	if ((TRUE == bIsForceG711A) || (strstr(pPartyName, "##FORCE_MEDIA") != NULL))
	{
		ostringstream str1;
		str1 << "party_name = " << pPartyName << " , force media by party name:\n";

		// Audio algorithms
		WORD found_force_algorithm       = 0;
		WORD force_algorithm_in_rmt_caps = 0;
		if (strstr(pPartyName, "G711_A") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(A_Law_OF);
			audMode.SetAudMode(A_Law_OF);
			str1 << "set G711_A\n";
			found_force_algorithm = 1;
			if (isAlaw)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "G711_U") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(U_Law_OF);
			audMode.SetAudMode(U_Law_OF);
			str1 << "set G711_U\n";
			found_force_algorithm = 1;
			if (isUlaw)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "G722_64") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(G722_m1);
			audMode.SetAudMode(G722_m1);
			str1 << "set G722_64\n";
			found_force_algorithm = 1;
			if (isG722_64)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "G722_56") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(G722_m2);
			audMode.SetAudMode(G722_m2);
			str1 << "set G722_56\n";
			found_force_algorithm = 1;
			if (isG722_56 || isG722_48)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "G722_48") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(G722_m3);
			audMode.SetAudMode(G722_m3);
			str1 << "set G722_48\n";
			found_force_algorithm = 1;
			if (isG722_48)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "G728") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(G728);
			audMode.SetAudMode(G728);
			str1 << "set G728\n";
			found_force_algorithm = 1;
			if (isG728)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "G722_1_C_48") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(G7221_AnnexC_48k);
			audMode.SetAudMode(G7221_AnnexC_48k);
			str1 << "set G722_1_C_48\n";
			found_force_algorithm = 1;
			if (isG722_1C_48)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "G722_1_C_32") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(G7221_AnnexC_32k);
			audMode.SetAudMode(G7221_AnnexC_32k);
			str1 << "set G722_1_C_32\n";
			found_force_algorithm = 1;
			if (isG722_1C_32)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "G722_1_C_24") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(G7221_AnnexC_24k);
			audMode.SetAudMode(G7221_AnnexC_24k);
			str1 << "set G722_1_C_24\n";
			found_force_algorithm = 1;
			if (isG722_1C_24)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "G722_1_32") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(Au_32k);
			audMode.SetAudMode(Au_32k);
			str1 << "set G722_1_32\n";
			found_force_algorithm = 1;
			if (isG722_1_32)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "G722_1_24") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(Au_24k);
			audMode.SetAudMode(Au_24k);
			str1 << "set G722_1_24\n";
			found_force_algorithm = 1;
			if (isG722_1_24)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "SIREN14_48") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(Au_Siren14_48k);
			audMode.SetAudMode(Au_Siren14_48k);
			str1 << "set SIREN14_48\n";
			found_force_algorithm = 1;
			if (isSiren14_48)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "SIREN14_32") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(Au_Siren14_32k);
			audMode.SetAudMode(Au_Siren14_32k);
			str1 << "set SIREN14_32\n";
			found_force_algorithm = 1;
			if (isSiren14_32)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (strstr(pPartyName, "SIREN14_24") != NULL)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(Au_Siren14_24k);
			audMode.SetAudMode(Au_Siren14_24k);
			str1 << "set SIREN14_24\n";
			found_force_algorithm = 1;
			if (isSiren14_24)
			{
				force_algorithm_in_rmt_caps = 1;
			}
		}

		if (!found_force_algorithm || !force_algorithm_in_rmt_caps)
		{
			m_pTargetLocalComMode->m_audMode.SetBitRate(U_Law_OF);
			audMode.SetAudMode(U_Law_OF);
			str1 << "force algorithm not found - set G711_U\n";
		}

		WORD forced_bit_rate = m_pTargetLocalComMode->m_audMode.GetBitRate();
		audMode.SetBitRateValue(forced_bit_rate);

		PTRACE2(eLevelInfoNormal, "CMuxCntl::AudioModeSelection:\n", str1.str().c_str());
		return;
	}

	if (sysISDN_G711_ALAW == FALSE)
	{
		str << "System not support: G711_ALAW\n ";
		isAlaw = FALSE;
	}

	if (sysISDN_G711_ULAW == FALSE)
	{
		str << "System not support: G711_ULAW\n ";
		isUlaw = FALSE;
	}

	if (sysISDN_G722_64k == FALSE)
	{
		str << "System not support: G722_64k\n ";
		isG722_64 = FALSE;
	}

	if (sysISDN_G722_56k == FALSE)
	{
		str << "System not support: G722_56k\n ";
		isG722_56 = FALSE;
	}

	if (sysISDN_G722_48k == FALSE)
	{
		str << "System not support: G722_48k\n ";
		isG722_48 = FALSE;
	}

	
	
	if (sysISDN_G722_1_32k == FALSE)
	{
		str << "System not support: G722_1_32k\n ";
		isG722_1_32 = FALSE;
	}

	if (sysISDN_G722_1_24k == FALSE)
	{
		str << "System not support: G722_1_24k\n ";
		isG722_1_24 = FALSE;
	}

	if (sysISDN_G722_1_C_48k == FALSE)
	{
		str << "System not support: G722_1_C_48k\n ";
		isG722_1C_48 = FALSE;
	}

	if (sysISDN_G722_1_C_32k == FALSE)
	{
		str << "System not support: G722_1_C_32k\n ";
		isG722_1C_32 = FALSE;
	}

	if (sysISDN_G722_1_C_24k == FALSE)
	{
		str << "System not support: G722_1_C_24k\n ";
		isG722_1C_24 = FALSE;
	}

	if (sysISDN_SIREN_14_48k == FALSE)
	{
		str << "System not support: SIREN_14_48k\n ";
		isSiren14_48 = FALSE;
	}

	if (sysISDN_SIREN_14_32k == FALSE)
	{
		str << "System not support: SIREN_14_32k\n ";
		isSiren14_32 = FALSE;
	}

	if (sysISDN_SIREN_14_24k == FALSE)
	{
		str << "System not support: SIREN_14_24k\n ";
		isSiren14_24 = FALSE;
	}

	if (sysISDN_G728 == FALSE)
	{
		str << "System not support: G728\n ";
		isG728 = FALSE;
	}

	/* check which algorithms are supported by EP */
	if (isAlaw)
	{
		str << "G711_Alaw";
	}

	if (isUlaw)
	{
		str << ", G711_Ulaw";
	}

	if (isG722_64)
	{
		str << ", G722_64";
	}

	if (isG722_56)
	{
		str << ", G722_56";
	}

	if (isG722_48)
	{
		str << ", G722_48";
	}

	if (isG722_1C_48)
	{
		str << ", G722_1_AnnexC_48";
	}

	if (isG728)
	{
		str << ", isG728";
	}

	if (isG722_1_32)
	{
		str << ", G722_1_32";
	}

	if (isG722_1_24)
	{
		str << ", G722_1_24";
	}

// if(isG722_1_16){
// str << ", G722_1_16";
// }
	if (isSiren14_24)
	{
		str << ", Siren14_24";
	}

	if (isSiren14_32)
	{
		str << ", Siren14_32";
	}

	if (isSiren14_48)
	{
		str << ", Siren14_48";
	}

	str << "\n";

	if (isG722_48)
	{
		isG722_56 = 1;
		str << "G722_48 in caps set G722_56 supported\n ";
	}

	/* if no audio capability so by default A_Law & U_Law supported */
	if (!(isAlaw || isUlaw || isG722_64 || isG722_48 || isG722_1_32 || isG722_1_24))
	{
		if (sysISDN_G711_ALAW)
		{
			ON(isAlaw);
		}

		if (sysISDN_G711_ULAW)
		{
			ON(isUlaw);
		}

		str << "System support: G711_Alaw, G711_Ulaw - by default (no other audio algorithm supported)\n ";
	}

	if ((m_pAudModeRsrv->GetBitRate() == 0) &&
	    m_pTargetLocalComMode->IsFreeVideoRate())      // in transcode aud bitrate set by remote capability

	{
		WORD callWidth = m_pTargetLocalComMode->m_xferMode.GetNumChnl() * m_pTargetLocalComMode->m_xferMode.GetChnlWidth() * 64;

		str << "Audio Bitrate: CP select by remote caps, call width = " << callWidth << "\n";

		if (callWidth <= 96)
		{
			str << "Algorithms priority by rate:G728, G7221_AnnexC_24k, Au_Siren14_24k, G722_m3/m2/m1, G711_ U_Law/A_Law\n";
			if (isG728)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G728);
			else if (isG722_1C_24)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G7221_AnnexC_24k);
			else if (isSiren14_24)
				m_pTargetLocalComMode->m_audMode.SetBitRate(Au_Siren14_24k);
			else if (isG722_48)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G722_m3);
			else if (isG722_56)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G722_m2);
			else if (isG722_64)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G722_m1);
			else if (isUlaw)
				m_pTargetLocalComMode->m_audMode.SetBitRate(U_Law_OF);
			else if (isAlaw)
				m_pTargetLocalComMode->m_audMode.SetBitRate(A_Law_OF);
		}
		else if (callWidth <= 192)
		{
			str << "Algorithms priority by rate:G7221_AnnexC_32k/24k, Au_Siren14_32k/24k, G722_1_32k/24k ,G722_m2/m1, G711_ U_Law/A_Law\n";
			if (isG722_1C_32)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G7221_AnnexC_32k);
			else if (isG722_1C_24)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G7221_AnnexC_24k);
			else if (isSiren14_32)
				m_pTargetLocalComMode->m_audMode.SetBitRate(Au_Siren14_32k);
			else if (isSiren14_24)
				m_pTargetLocalComMode->m_audMode.SetBitRate(Au_Siren14_24k);
			else if (isG722_1_32)
				m_pTargetLocalComMode->m_audMode.SetBitRate(Au_32k);
			else if (isG722_1_24)
				m_pTargetLocalComMode->m_audMode.SetBitRate(Au_24k);
			else if (isG728)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G728);
			else if (isG722_56)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G722_m2);
			else if (isG722_64)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G722_m1);
			else if (isUlaw)
				m_pTargetLocalComMode->m_audMode.SetBitRate(U_Law_OF);
			else if (isAlaw)
				m_pTargetLocalComMode->m_audMode.SetBitRate(A_Law_OF);
		}
		else         // call width > 192
		{
			str << "Algorithms priority by rate:G7221_AnnexC_48k/32k/24k, Au_Siren14_48k/32k/24k, G722_1_32k/24k ,G722_m2/m1, G711_ U_Law/A_Law\n";
			if (isG722_1C_48)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G7221_AnnexC_48k);
			else if (isG722_1C_32)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G7221_AnnexC_32k);
			else if (isG722_1C_24)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G7221_AnnexC_24k);
			else if (isSiren14_48)
				m_pTargetLocalComMode->m_audMode.SetBitRate(Au_Siren14_48k);
			else if (isSiren14_32)
				m_pTargetLocalComMode->m_audMode.SetBitRate(Au_Siren14_32k);
			else if (isSiren14_24)
				m_pTargetLocalComMode->m_audMode.SetBitRate(Au_Siren14_24k);
			else if (isG722_1_32)
				m_pTargetLocalComMode->m_audMode.SetBitRate(Au_32k);
			else if (isG722_1_24)
				m_pTargetLocalComMode->m_audMode.SetBitRate(Au_24k);
			else if (isG728)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G728);
			else if (isG722_56)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G722_m2);
			else if (isG722_64)
				m_pTargetLocalComMode->m_audMode.SetBitRate(G722_m1);
			else if (isUlaw)
				m_pTargetLocalComMode->m_audMode.SetBitRate(U_Law_OF);
				//m_pTargetLocalComMode->m_audMode.SetBitRate(U_Law_OU);
            
			else if (isAlaw)
				m_pTargetLocalComMode->m_audMode.SetBitRate(A_Law_OF);
                //m_pTargetLocalComMode->m_audMode.SetBitRate(A_Law_OU);
            
		}
	}
	else
	{
		str << "Audio Bitrate: VSW select by reservation\n";
	}

	WORD bit_rate = m_pTargetLocalComMode->m_audMode.GetBitRate();
	audMode.SetBitRateValue(bit_rate);
	str << "Selected bitrate = " << bit_rate << "\n";

// TRACESTR(eLevelInfoNormal) << " CMuxCntl::AudioModeSelection : bit rate = " << bit_rate << "\n";
	str << "Selected Algorithm = ";
	switch (bit_rate)
	{
		case 0:
		{                 // automatic selection method to be reviewed.
			str << "Rate 0 no audio selected";
			break;
		}

		case 16:
		{                 // olga - ask Ron!
			if (isG728)
			{
				audMode.SetAudMode(G728);
				str << "G728\n";
			}
			else
			{
				str << "Rate 16k no audio algorithm supported for H320 , ";
				// party will have no video , lsd/mlp up to 6400 , hsd/hmlp
				PTRACE2(eLevelError,
				        "CMuxCntl::AudioModeSelection : \'Remote Cap - bit rate = 16 , ZERO !!\' ",
				        PARTYNAME);
				switch (m_pCurRemoteComMode->GetAudMode())
				{
					case A_Law_OF:
					case U_Law_OF:
					case A_Law_48:
					case U_Law_48:
					{
						audMode.SetAudMode(m_pCurRemoteComMode->GetAudMode());
						str << "G711_ U_Law/A_Law\n";
						break;
					}

					default:
					{                              // selection by remote cap
						if (rstrict == Restrict)     // same as 56 restrict
							isUlaw ? audMode.SetAudMode(U_Law_48) : audMode.SetAudMode(A_Law_48);
						else                         // non restrict
							isUlaw ? audMode.SetAudMode(U_Law_OF) : audMode.SetAudMode(A_Law_OF);

						str << "G711_ U_Law/A_Law\n";
						break;
					}
				} // switch
			}

			break;
		}

		case 24:
		{
			/* priorities for 24k: G.722.1 AnnexC / Siren14 / G.722.1 / G.722 / G.711  */
			if (isG722_1C_24)
			{
				audMode.SetAudMode(G7221_AnnexC_24k);
				str << "G7221_AnnexC_24k\n";
			}

			else if (isSiren14_24)
			{
				audMode.SetAudMode(Au_Siren14_24k);
				str << "Siren14_24k\n";
			}

			else if (isG722_1_24)
			{
				audMode.SetAudMode(Au_24k);
				str << "G722_1_24k\n";
			}

			else          // party will have no video , lsd/mlp up to 6400 , hsd/hmlp
			{
				PTRACE2(eLevelError,
				        "CMuxCntl::AudioModeSelection : \'Remote Cap Not Includes audio 24k , ZERO !!\' ",
				        PARTYNAME);
				switch (m_pCurRemoteComMode->GetAudMode())
				{
					case A_Law_OF:
					case U_Law_OF:
					case A_Law_48:
					case U_Law_48:
					{
						audMode.SetAudMode(m_pCurRemoteComMode->GetAudMode());
						str << "G711_ U_Law/A_Law\n";
						break;
					}

					default:
					{                                          // selection by remote cap
						if (rstrict == Restrict)                 // same as 56 restrict
							isUlaw ? audMode.SetAudMode(U_Law_48) : audMode.SetAudMode(A_Law_48);
						else                                     // non restrict
							isUlaw ? audMode.SetAudMode(U_Law_OF) : audMode.SetAudMode(A_Law_OF);

						str << "G711_ U_Law/A_Law\n";
						break;
					}
				} // switch
			}

			break;
		}

		case 32:
		{
			/* priorities for 32k: G.722.1 AnnexC / Siren14 / G.722.1 / G.722 / G.711  */
			if (isG722_1C_32)
			{
				audMode.SetAudMode(G7221_AnnexC_32k);
				str << "G7221_AnnexC_32k\n";
			}
			else if (isSiren14_32)
			{
				audMode.SetAudMode(G7221_AnnexC_32k);
				str << "G7221_AnnexC_32k\n";
			}
			else if (isSiren14_32)
			{
				audMode.SetAudMode(Au_Siren14_32k);
				str << "Au_Siren14_32k\n";
			}
			else if (isG722_1_32)
			{
				audMode.SetAudMode(Au_32k);
				str << "G722_1_32k\n";
			}
			else           // party will have no video , lsd/mlp up to 6400 , hsd/hmlp
			{
				PTRACE2(eLevelError,
				        "CMuxCntl::AudioModeSelection : \'Remote Cap Not Includes audio 32k , ZERO !!\' ",
				        PARTYNAME);
				switch (m_pCurRemoteComMode->GetAudMode())
				{
					case A_Law_OF:
					case U_Law_OF:
					case A_Law_48:
					case U_Law_48:
					{
						audMode.SetAudMode(m_pCurRemoteComMode->GetAudMode());
						str << "G711_ U_Law/A_Law\n";
						break;
					}

					default:
					{                                          // selection by remote cap
						if (rstrict == Restrict)                 // same as 56 restrict
							isUlaw ? audMode.SetAudMode(U_Law_48) : audMode.SetAudMode(A_Law_48);
						else                                     // non restrict
							isUlaw ? audMode.SetAudMode(U_Law_OF) : audMode.SetAudMode(A_Law_OF);

						str << "G711_ U_Law/A_Law\n";
						break;
					}
				} // switch
			}

			break;
		}

		case 48:
		{
			/* priorities for 48k: G.722.1 AnnexC / Siren14 / G.722 / G.711  */
			if (isG722_1C_48)
			{
				audMode.SetAudMode(G7221_AnnexC_48k);
				str << "G7221_AnnexC_48k\n";
			}
			else if (isSiren14_48)
			{
				audMode.SetAudMode(Au_Siren14_48k);
				str << "Siren14_48k\n";
			}
			else if (rstrict == Restrict)
			{
				if (isG722_48 && m_pLocalCap->OnAudioCap(e_G722_48))
				{
					audMode.SetAudMode(G722_m3);
					str << "G722_m3\n";
				}
				else
				{
					isUlaw ? audMode.SetAudMode(U_Law_48) : audMode.SetAudMode(A_Law_48);
					str << "G711_ U_Law/A_Law_48k\n";
				}
			}
			else
			{
				// non restrict
				if (isG722_48 && m_pLocalCap->OnAudioCap(e_G722_48))
				{
					audMode.SetAudMode(G722_m3);
					str << "G722_m3\n";
				}
				else
				{
					isUlaw ? audMode.SetAudMode(U_Law_48) : audMode.SetAudMode(A_Law_48);
					str << "G711_ U_Law/A_Law_48k\n";
				}
			}

			break;
		}

		case 56:
		{
			if (rstrict == Restrict)             // same as 48
			{
				if (isG722_48 && m_pLocalCap->OnAudioCap(e_G722_48))
				{
					audMode.SetAudMode(G722_m3);
					str << "G722_m3\n";
				}
				else
				{
					isUlaw ? audMode.SetAudMode(U_Law_48) : audMode.SetAudMode(A_Law_48);
					str << "G711_ U_Law/A_Law_48k\n";
				}

				break;
			}

			// non restrict
			if (isG722_48 && m_pLocalCap->OnAudioCap(e_G722_48))
			{
				audMode.SetAudMode(G722_m2);
				str << "G722_m2\n";
			}
			else
			{
				isUlaw ? audMode.SetAudMode(U_Law_OF) : audMode.SetAudMode(A_Law_OF);
				str << "G711_ U_Law/A_Law\n";
			}

			break;
		}

		case 64:
		{
			isUlaw ? audMode.SetAudMode(U_Law_OU) : audMode.SetAudMode(A_Law_OU);
			str << "G711_ U_Law/A_Law\n";
			break;
		}

		default:
		{
			PASSERT(bit_rate);
			break;
		}
	} // switch

	PTRACE2(eLevelInfoNormal, "CMuxCntl::AudioModeSelection:\n", str.str().c_str());
}

//--------------------------------------------------------------------------
BYTE CMuxCntl::IsInitAudioSelection(void)
{
	return !(m_isAudioChange & thisIsNotInitAudioSelection);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartySetComModeChangeMode()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeChangeMode : Name - ", PARTYNAME);
	// remote is already at wanted comMode
	if (*m_pTargetLocalComMode == *m_pCurRemoteComMode)
	{
		PTRACE(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeChangeMode : Remote is already in desirable Scm !!!");
		*m_pCurLocalComMode = *m_pTargetLocalComMode;
		this->SendXmitComMode(*m_pTargetLocalComMode, m_isBitRateChange);
		m_state = MUX_CONNECT;
		ReplyMuxEndSetXmitComModeStatusOK();
		return;
	}
	else if (m_pTargetLocalComMode->IsClosureOfVideoOnly(*m_pCurLocalComMode))
	{
		PTRACE(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeChangeMode : Closure Of Video Only !!!");
		*m_pCurLocalComMode = *m_pTargetLocalComMode;
		this->SendXmitComMode(*m_pTargetLocalComMode, m_isBitRateChange);
		m_state = MUX_CONNECT;
		ReplyMuxEndSetXmitComModeStatusOK();
		return;
	}

	OnPartySetComMode();
	// To Support Codian and RadVision behavior when we are trying to open the content in 0k the REMOTE_XMIT_MODE arrived without content caps.
	// the Codian and the RV does NOT open the content in 0k so we are simulate as if they do
	// We are in ChangeMode so the video in 0 and we are in change content (NEED to add a condition for that ,to send Sim only if the Video is 0 and the content is 0)

	// I made a private version with this code but this code never tested in lab.
	// we need to add this code that activate simulation only when the content rate is 0 (in the beginning of the call)
	// if(!m_pTargetLocalComMode->IsContentOn() && !m_pCurRemoteComMode->IsContentOn() && !m_pTargetLocalComMode->VideoOn() && !m_pCurRemoteComMode->VideoOn())//VNGR-20534
	// {
	// PTRACE(eLevelInfoNormal,"CMuxCntl::OnPartySetComModeChangeMode : Open content in 0K (video also 0K) ==> SimOnMuxRmtXmitMode !!!");//VNGR-20534

	// Tsahi - Fix for VNGR-21260 - Conference changed to H264 but EP stays in H263
	// Tsahi - VNGFE-4753 - We allow by default to simulate REMOTE_XMIT_MODE in ISDN in all cases.
	// When the flag SIMULATE_REMOTE_XMIT_MODE_ISDN_H263_TO_H264 set to NO, we disable the simulation of REMOTE_XMIT_MODE when changing between H263 and H264. - (Done for VNGR-21260)
	BOOL bSimulateRmtXmitMode = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SIMULATE_REMOTE_XMIT_MODE_ISDN_H263_TO_H264, bSimulateRmtXmitMode);
	if (!bSimulateRmtXmitMode &&
	    m_pCurRemoteComMode->GetVidMode() == H263 &&
	    m_pTargetRemoteComMode->GetVidMode() == H264)
	{
		PTRACE(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeChangeMode : NO NEED to simulate REMOTE_XMIT_MODE ");
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeChangeMode : Simulate REMOTE_XMIT_MODE ");
		SimOnMuxRmtXmitMode();
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartySetCommMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartySetCommMode : Name - ", PARTYNAME);

	if (MUX_CONNECT == m_state)
		OnPartySetComModeConnect();
	else if (MUX_CHANGEMODE == m_state)
		OnPartySetComModeChangeMode();
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartySetComModeConnect()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeConnect: Name - ", PARTYNAME);
	if (m_isBitRateChange && m_pTargetLocalComMode->m_xferMode <= m_pCurRemoteComMode->m_xferMode)
	{
		m_pCurLocalComMode->m_xferMode = m_pTargetLocalComMode->m_xferMode;
		this->SendXmitComMode(*m_pTargetLocalComMode, m_isBitRateChange);
		ReplyMuxEndSetXmitComModeStatusOK();
		return;
	}

	if (*m_pTargetLocalComMode == *m_pCurRemoteComMode)
	{
		*m_pCurLocalComMode = *m_pTargetLocalComMode;
		if (!m_pTargetLocalComMode->IsDifferInContentRateOnly(*m_pCurLocalComMode))     // / IsLinkToMaster() - Cascade H239
			this->SendXmitComMode(*m_pTargetLocalComMode, m_isBitRateChange);
		else
			PTRACE2(eLevelInfoNormal, "CMuxCntl::Link to Master - we receive Remote_Xmit_mode and the content rate is diff: Name - ", PARTYNAME);

		ReplyMuxEndSetXmitComModeStatusOK();
	}

	else
	{
		// close video
		if (m_pTargetLocalComMode->IsClosureOfVideoOnly(*m_pCurLocalComMode) ||
		    m_pTargetLocalComMode->IsClosureOfVideoAndContentOnly(*m_pCurLocalComMode))
		{
			*m_pCurLocalComMode = *m_pTargetLocalComMode;
			this->SendXmitComMode(*m_pTargetLocalComMode, m_isBitRateChange);
			ReplyMuxEndSetXmitComModeStatusOK();
		}

		else
		{
			// Content mode change from conference when remote already changed its content mode respectively
			if ((m_pTargetLocalComMode->m_contentMode == m_pCurRemoteComMode->m_contentMode) &&
			    !(m_pTargetLocalComMode->m_contentMode == m_pCurLocalComMode->m_contentMode))
			{
				*m_pCurLocalComMode = *m_pTargetLocalComMode;
				if (!m_pTargetLocalComMode->IsDifferInContentRateOnly(*m_pCurLocalComMode)) // IsLinkToMaster()- Cascade H239
					this->SendXmitComMode(*m_pTargetLocalComMode, m_isBitRateChange);

				else                                                                        // In case this is a link to Master we won't send the Xmit_mode, but we need to start the EndChangeMode
					PTRACE2(eLevelInfoNormal, "CMuxCntl::Link to Master - we receive Remote_Xmit_mode and the content rate is diff: Name - ", PARTYNAME);

				ReplyMuxEndSetXmitComModeStatusOK();
				return;
			}

			if (m_changeModeType == VIDEO_ASYMMETRIC_CHANGE_MODE)
			{
				PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeConnect VIDEO ASYMMETRIC CHANGE MODE : Name - ", PARTYNAME);
				ON(m_changeModeReq);
				ReplyMuxEndSetXmitComModeStatusOK();
				this->SendXmitComMode(*m_pTargetLocalComMode, m_isBitRateChange);
				return;
			}

			m_state = MUX_CHANGEMODE;
			if (m_pCurLocalComMode->GetVidMode() != m_pTargetLocalComMode->GetVidMode() && m_pTargetLocalComMode->GetVidMode() == H263)
			{
				BOOL        bModifyLegacyIsdnEPCaps = FALSE;
				std::string key                     = CFG_ISDN_LEGACY_EP_CLOSE_CONTENT_FORCE_H263;
				CSysConfig* sysConfig               = CProcessBase::GetProcess()->GetSysConfig();
				sysConfig->GetBOOLDataByKey(key, bModifyLegacyIsdnEPCaps);
				// if CFG_MODIFY_LEGACY_ISDN_EP_CAPS is set to YES, modify MCU cap set: remove H.261 cap set,
				// and remove H.264 cap set (if remote has no H.264 caps)
				if (bModifyLegacyIsdnEPCaps)
				{
					PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeConnect Video Protocol changed - exchange cap: Name - ", PARTYNAME);
					if (m_pLocalCap->IsH263() || m_pLocalCap->IsH264() || !m_pLocalCap->GetNSCap()->IsEmpty() ||
					    m_pLocalCap->IsPPCap() || m_pLocalCap->IsH239Cap())
					{
						// remove H.261 from second set
						if (m_pLocalCap->IsH263() || m_pLocalCap->IsH264())
						{
							PTRACE(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeConnect Remove H261/H.264  caps before opening the video");
							m_pLocalCap->RemoveH261Caps();
							if (!m_pRemoteCap->IsH264())
								m_pLocalCap->RemoveH264Caps();
						}

						if (!m_pRemoteCap->IsH239Cap())
						{
							PTRACE(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeConnect Remove H.239  caps before opening the video (Remote does not have H.239 caps)");
							m_pLocalCap->RemoveH239Caps();
						}
					}
				}

				// send current cap set
				ExchangeCap(*m_pLocalCap, m_pRemoteCap->IsVideoCapSupported(H263_2000));
			}

			OnPartySetComMode();

			// Tsahi - Fix for VNGR-21260 - Conference changed to H264 but EP stays in H263
			// Tsahi - VNGFE-4753 - We allow by default to simulate REMOTE_XMIT_MODE in ISDN in all cases.
			// When the flag SIMULATE_REMOTE_XMIT_MODE_ISDN_H263_TO_H264 set to NO, we disable the simulation of REMOTE_XMIT_MODE when changing between H263 and H264. - (Done for VNGR-21260)
			BOOL bSimulateRmtXmitMode = TRUE;
			CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SIMULATE_REMOTE_XMIT_MODE_ISDN_H263_TO_H264, bSimulateRmtXmitMode);
			if (!bSimulateRmtXmitMode &&
			    m_pCurRemoteComMode->GetVidMode() == H263 &&
			    m_pTargetRemoteComMode->GetVidMode() == H264)
			{
				PTRACE(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeConnect : NO NEED to simulate REMOTE_XMIT_MODE ");
			}
			else
			{
				PTRACE(eLevelInfoNormal, "CMuxCntl::OnPartySetComModeConnect : Simulate REMOTE_XMIT_MODE ");
				PTRACE2(eLevelInfoNormal, "CMuxCntl::To support Codian and RadVision GW behavior ,simulate as they have opened the Content when the RMX ask to open content.: Name - ", PARTYNAME);
				SimOnMuxRmtXmitMode();
			}
		}
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartySetComMode()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartySetComMode : Name - ", PARTYNAME);

	this->SendXmitComMode(*m_pTargetLocalComMode, m_isBitRateChange);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyDisconnectSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartyDisconnectSetup : Name - ", PARTYNAME);
	OnPartyDisconnect();
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyDisconnectChangeMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartyDisconnectChangeMode : Name - ", PARTYNAME);
	OnPartyDisconnect();
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyDisconnectConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnPartyDisconnectConnect : Name - ", PARTYNAME);
	OnPartyDisconnect();
}

//--------------------------------------------------------------------------
void CMuxCntl::OnPartyDisconnect(CSegment* pParam)
{
	DeleteAllTimers();
}

//--------------------------------------------------------------------------
void CMuxCntl::OnTimerSetComModeSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerSetComModeSetup: Name - ", PARTYNAME);

	if (m_isBitRateChange)
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerSetComModeSetup : \'FAILED TO OPEN INITIAL CHANNEL !!!\' Name - ", PARTYNAME);

	if (m_isAudioChange)
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerSetComModeSetup : \'FAILED TO OPEN AUDIO !!!\' Name - ", PARTYNAME);

	DeleteTimer(RMTAUDIOTOUT);

	m_pTaskApi->TimeOutMuxEndH320Connect(*m_pRemoteCap, *m_pCurRemoteComMode, NOT_RECIVED_END_INIT_COM);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnTimerRmtAudioSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerRmtAudioSetup : Name - ", NameOf());


	if (!(m_pCurLocalComMode->GetAudMode() == m_pCurRemoteComMode->GetAudMode()))
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerRmtAudioSetup : \'REMOTE AUDIO CHANGED\' Name - ", PARTYNAME);
		if (m_pCurRemoteComMode->IsAudioOpen())
		{
			// adopt audio mode to the remote request
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerRmtAudioSetup: Audio_Negotiation Remote Audio Open Other algorithm - adopt remote audiofor party ", PARTYNAME);
			UpdateAudioAlgorithm(m_pCurRemoteComMode->GetAudMode(), 0);
			OFF(m_isAudioChange);
			EndAudioNegotiation();
		}
		else
		{
			// VNGFE-4652
			BOOL  bUseLongRmtAudioTimer = FALSE;
			DWORD uRmtAudTimeout        = 0;
			CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ISDN_CASCADE_LONG_RMT_AUDIO_NEGOTIATION, bUseLongRmtAudioTimer);
			if (bUseLongRmtAudioTimer && m_isRemoteMcu)
				uRmtAudTimeout = RMT_MCU_AUDIO_TOUT;
			else
				uRmtAudTimeout = RMT_AUDIO_TOUT;

			StartTimer(RMTAUDIOTOUT, uRmtAudTimeout);
			ostringstream str;
			str <<  "Audio_Negotiation: remote audio not open, Set Timer for remote audio algorithm: StartTimer(RMTAUDIOTOUT) for " << uRmtAudTimeout/100 << " secondes\n";
			PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerRmtAudioSetup : ", str.str().c_str());
			return;
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerRmtAudioSetup : \'REMOTE AUDIO ACCEPTED BY MCU\'  Name - ", PARTYNAME);
		OFF(m_isAudioChange);
		EndAudioNegotiation();
	}
}
//--------------------------------------------------------------------------
void CMuxCntl::OnTimerUpdateAudioAlgorithm(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "**CMuxCntl::OnTimerUpdateAudioAlgorithm**"); // Eitan tmp

	/* Update MUX and remote side with the new audio algorithm.
	   MUX will send to remote Comfort Noise patterns of new algorithm */
	this->SendXmitComMode(*m_pCurLocalComMode, m_isBitRateChange);

	/* Stay in Comfort Noise state for additional 1/2 sec */
// StartTimer(COMFORT_NOISE_TOUT, 50 /*SECOND/2*/);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnTimerRmtAudioStabilizeSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerRmtAudioStabilizeSetup - ", PARTYNAME);
	m_NumOfAudioChanges++;
	if (2 == m_NumOfAudioChanges)
	{
		// adopt audio mode to the remote request
		UpdateAudioAlgorithm(m_pCurRemoteComMode->GetAudMode(), 0);
	}

	if (!(m_pCurLocalComMode->GetAudMode() == m_pCurRemoteComMode->GetAudMode()))
	{
		ostringstream str;
		str << " local_aud_mode = " << m_pCurLocalComMode->GetAudMode()<< " , Remote_aud_mode = " << m_pCurRemoteComMode->GetAudMode();
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerRmtAudioStabilizeSetup : REMOTE AUDIO CHANGED  - ", str.str().c_str());

		CAudMode initAudMode;
		AudioModeSelection(initAudMode, m_pTargetLocalComMode->GetOtherRestrictMode());

		UpdateAudioAlgorithm(initAudMode.GetAudMode(), 0);
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerRmtAudioStabilizeSetup : StartTimer(RMTAUDIOSTABILIZETOUT) Name - ", PARTYNAME);
		StartTimer(RMTAUDIOSTABILIZETOUT, RMT_AUDIOSTABILIZE_TOUT);
	}
	else
	{
		OFF(m_isAudioChange);
		EndAudioNegotiation();
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::OnTimerRmtCapsSetup(CSegment* pParam)
{
	ostringstream str;
	str <<  "Capabilities_exchange: timer for full cap set received - end of capabilities exchange";
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerRmtCapsSetup: ", str.str().c_str());

	if (m_pCurRemoteComMode->IsAudioOpen())
	{
		// end audio negotiation
		m_isFullAudioCapsReceived = TRUE;
		EndAudioNegotiation();
	}
	else
	{
		ostringstream str;
		str <<  "Remote did not open audio yet - Wait for another audio Alg.";
		PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerRmtCapsSetup: ", str.str().c_str());
		StartTimer(RMTCAPSTOUT, RMT_CAPS_TOUT);
		return;
	}

	// end exchange caps
	if (m_pRemoteCap->IsH264() || m_pRemoteCap->IsH263() || m_waitForH263H264Caps > 0)
	{
		m_isCapExchangeEnd = TRUE;
		EndMuxConnect();
	}
	else
	{
		m_waitForH263H264Caps++;
		ostringstream str;
		// There is a large gap between 2 cap sets from HDX (increase the timer in order not to miss the last cap set)
		DWORD         EXTENDED_RMT_CAPS_TOUT = RMT_CAPS_TOUT + SECOND;
		str <<  "Wait for H263/H264 caps one more time - StartTimer(RMTCAPSTOUT) for " << EXTENDED_RMT_CAPS_TOUT/100 << " secondes\n";
		PTRACE2(eLevelInfoNormal, "OnTimerRmtCapsSetup: ", str.str().c_str());
		StartTimer(RMTCAPSTOUT, EXTENDED_RMT_CAPS_TOUT);
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::OnTimerSync(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerSync : Name - ", PARTYNAME);
	m_pTaskApi->MuxSyncTimer();
}

//--------------------------------------------------------------------------
void CMuxCntl::UpdateAudioAlgorithm(WORD new_audio_algorithm, BYTE isInitialAudioAlgorithmSelection)
{
	PTRACE(eLevelInfoNormal, "CMuxCntl::UpdateAudioAlgorithm");

	ON(m_isAudioChange);

	m_pTargetLocalComMode->m_audMode.SetAudMode(new_audio_algorithm);
	m_pCurLocalComMode->m_audMode.SetAudMode(new_audio_algorithm);

	this->SendXmitComMode(*m_pCurLocalComMode, m_isBitRateChange);
}

//--------------------------------------------------------------------------
BYTE CMuxCntl::IsPartyH239()
{
	if (!(m_pLocalCap->IsH239Cap()))
	{
		PTRACE(eLevelInfoNormal, "CMuxCntl::IsPartyH239 no H239 in local caps");
	}

	return (m_pLocalCap->IsH239Cap() && m_pRemoteCap->IsH239Cap());
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmt230Setup(CSegment* pParam)
{
	OnMuxRmt230(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmt230ChangeMode(CSegment* pParam)
{
	OnMuxRmt230(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmt230Connect(CSegment* pParam)
{
	OnMuxRmt230(pParam);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMuxRmt230(CSegment* pParam)
{
	DWORD write_offset = pParam->GetWrtOffset();

	DWORD rmt_h230_length;
	*pParam >> rmt_h230_length;

	ostringstream str;
	str << "pParam_write_offset = " << write_offset << " , rmt_h230_length = " << rmt_h230_length;
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmt230: ", str.str().c_str());

	BYTE* seg_data = new BYTE[rmt_h230_length+32];
	pParam->Get(seg_data, rmt_h230_length);

	CSegment h230Seg;
	h230Seg.Put(seg_data, rmt_h230_length);

	delete[] seg_data;

	h230Seg.DumpHex();

	m_pTaskApi->MuxRmtH230(&h230Seg); // forward task to party manager
}

//--------------------------------------------------------------------------
BYTE CMuxCntl::CheckEncryptionSetupProcessCompleted()
{
	BYTE rval = TRUE;
	if (!(m_pCurRemoteComMode->m_ECSMode.GetXmitEncrpAlg() == m_pTargetLocalComMode->m_ECSMode.GetXmitEncrpAlg()))
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::CheckEncryptionSetupProcessCompleted :"
		        " \'Failed - P9 Was not Received\'  Name - ", PARTYNAME);
		rval = FALSE;
	}

	if ((m_pCurRemoteComMode->m_ECSMode).GetXmitRcvKey() == NULL || (m_pTargetLocalComMode->m_ECSMode).GetXmitRcvKey() == NULL)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::CheckEncryptionSetupProcessCompleted :"
		        " \'Failed - XmitRcvKey Was not Received\'  Name - ", PARTYNAME);
		rval = FALSE;
	}

	if (rval)
		PTRACE2(eLevelInfoNormal, "CMuxCntl::CheckEncryptionSetupProcessCompleted:"
		        " \'Encryption Process Ended On Time!!!\'  Name - ", PARTYNAME);

	return rval;
}

//--------------------------------------------------------------------------
void CMuxCntl::OpenEncryption()
{
	if (!(m_pRemoteCap->IsEncrypCapOn(Encryp_Cap))) {
		PASSERT_AND_RETURN(101);
	}

	m_pCurLocalComMode->m_otherMode.SetEncrypMode(Encryp_On);
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OpenEncryption : StartTimer(RMT_ENCTOUT) Name - ", PARTYNAME);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendEncP8P0()
{
	if (!(m_pRemoteCap->IsEncrypCapOn(Encryp_Cap))) {
		PASSERT_AND_RETURN(101);
	}

	PASSERT(!m_pLocalCap->GetCapECS());

	CSegment ECSString;
	m_pLocalCap->GetCapECS()->Serialize(SERIALEMBD, ECSString);
	m_pMux->SendECS(ECSString);

	PTRACE2(eLevelInfoNormal, "CMuxCntl::SendEncP8P0 : StartTimer(RMT_ENC_ALG_TOUT) Name - ", PARTYNAME);
	StartTimer(RMT_ENC_ALGTOUT, RMT_ENC_ALG_TOUT);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendH239ExtendedVideoCaps()
{
	if (!IsValidPObjectPtr(m_pLocalCap) || !IsValidPObjectPtr(m_pLocalCap->GetH239Caps()))
	{
		PASSERT_AND_RETURN(101);
	}

	PTRACE(eLevelInfoNormal, "CMuxCntl::SendH239ExtendedVideoCaps");
	CSegment*       segForH230 = new CSegment;
	const CCapH239* capH239    = m_pLocalCap->GetH239Caps();
	((CCapH239*)capH239)->SerializeH239ExtendedVidCaps(SERIALEMBD, *segForH230);

	SendH230(*segForH230);

	POBJDELETE(segForH230);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendH230(CSegment& h230String)
{
	m_pMux->SendH230(h230String);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendH230(WORD attrib, WORD value)
{
	CSegment H221String;
	H221String << (BYTE)(H230_Esc | ESCAPECAPATTR)
	           << (BYTE)(attrib | value);
	m_pMux->SendH230(H221String);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendAudioValid(WORD onOff)
{
	m_pRepeatedH230->SetAudio(onOff);
	CSegment* pSeg = new CSegment;
	m_pRepeatedH230->Serialize(NATIVE, *pSeg);
	m_pMux->SetRepeatedH230(*pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendMMS(WORD onOff)
{
	SetIsMmsEnabled(onOff);
	SetModeSym(onOff);
	CSegment* pSeg = new CSegment;
	m_pRepeatedH230->Serialize(NATIVE, *pSeg);
	m_pMux->SetRepeatedH230(*pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendMCC(WORD onOff)
{
	m_pRepeatedH230->SetIsMcu(onOff);
	CSegment* pSeg = new CSegment;
	m_pRepeatedH230->Serialize(NATIVE, *pSeg);
	m_pMux->SetRepeatedH230(*pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendH230Mbe(CSegment* pSeg)
{
	CSegment H221String;
	BYTE     opcode;
	BYTE     mcu;
	BYTE     lenb;
	BYTE     term;
	WORD     lenw;

	*pSeg>>lenb>>opcode>>mcu;
	H221String<<(BYTE)(Start_Mbe | ESCAPECAPATTR)<<lenb<<opcode<<mcu;
	lenw = lenb;
	for (WORD i = 0; i < (lenw-2); i++)
	{
		*pSeg>>term;
		H221String<<term;
	}

	m_pMux->SendH230(H221String);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendH230Sbe(CSegment* pSeg)
{
	CSegment H221String;
	BYTE     opcode;
	BYTE     mcu;
	BYTE     term;

	*pSeg>>opcode>>mcu>>term;
	H221String << (BYTE)(H230_Esc | ESCAPECAPATTR)
	           <<opcode<<(BYTE)(H230_Sbe_Esc | ESCAPECAPATTR)
	           <<mcu
	           <<(BYTE)(H230_Sbe_Esc | ESCAPECAPATTR)
	           <<term;
	m_pMux->SendH230(H221String);
}

//--------------------------------------------------------------------------
void CMuxCntl::ReplyMuxEndSetXmitComModeStatusOK()
{
	if ((m_changeModeType == VIDEO_ASYMMETRIC_CHANGE_MODE) ||
	    (m_changeModeType == VIDEO_ASYMMETRIC_CHANGE_MODE_RMT_RCV))
	{
		m_pTaskApi->MuXEndSetXmitComMode(statAsymmetricVideoOK);
	}
	else
	{
		m_pTaskApi->MuXEndSetXmitComMode(statOK);
	}
}

//--------------------------------------------------------------------------
WORD CMuxCntl::IsVideoAsymetricMode()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::IsVideoAsymetricMode: Name - ", PARTYNAME);
	WORD retVal = 0;

	// here we will compare the communication mode and make sure that the only
	// variable different is the video protocol + the video mode is Asymetric
	if ((*m_pTargetLocalComMode).IsAsymmetricVideoProtocolOnly(*m_pCurRemoteComMode) &&
	    ((m_changeModeType == VIDEO_ASYMMETRIC_CHANGE_MODE_WAIT_OTHER) ||
	     (m_changeModeType == VIDEO_ASYMMETRIC_CHANGE_MODE_WAIT_OTHER_RMT_RCV)))
		retVal = 1;

	return retVal;
}

//--------------------------------------------------------------------------
BOOL CMuxCntl::IsRmtCapsBetter()
{
	BOOL ret = FALSE;
	// check if we recieved H.264 and H.239 from EP
	if (m_pRemoteCap->IsH264() && m_pRemoteCap->IsH239Cap())
		ret = TRUE;

	TRACESTR(eLevelInfoNormal) << " CMuxCntl::IsRmtCapsBetter returns = " << (WORD)ret;
	return ret;
}

//--------------------------------------------------------------------------/
BYTE CMuxCntl::IsContentAsymetricModeInIVRMode()
{
	WORD retVal = 0;

// if(!m_IsGateWay) {
	CCommConf* pCommConf = NULL;
	pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

	// we will compare the communication mode and make sure that the only
	// variable different is the content mode - VSX send us remote_xmit_mode with content 0k
	// although we didn't open the content yet.

	if (pCommConf)
	{
		if (pCommConf->GetDualVideoMode() == eDualModeEnterprisePC)
		{
			if ((*m_pTargetLocalComMode).IsAsymmetricContentModeOnly(*m_pCurRemoteComMode))
			{
				PTRACE2(eLevelInfoNormal, "CMuxCntl::IsContentAsymetricModeInIVRMode: Name - ", PARTYNAME);
				retVal = 1;
			}
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::IsContentAsymetricModeInIVRMode, Con DB object is NULL, Name: ", m_pParty->GetFullName());
	}
	return retVal;
}

//--------------------------------------------------------------------------/
void CMuxCntl::SetRmtH239ExtendedVidCaps(CSegment& seg, BYTE len)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::SetRmtH239ExtendedVidCaps: Name - ", PARTYNAME);
	if (m_pRemoteCap)
		m_pRemoteCap->SetH239ExtendedVideoCaps(seg, len);

	m_pTaskApi->MuxRmtCap(*m_pRemoteCap, statOK);
}

//--------------------------------------------------------------------------/
void CMuxCntl::SetRmtAMCCaps(CSegment& seg, BYTE len)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::SetRmtAMCCaps: Name - ", PARTYNAME);
	if (m_pRemoteCap)
		m_pRemoteCap->SetAMCCaps(seg, len);
}

//--------------------------------------------------------------------------
void CMuxCntl::SetIsMmsEnabled(WORD onOff)
{
	m_pRepeatedH230->SetIsMmsEnabled(onOff);
}

//--------------------------------------------------------------------------
void CMuxCntl::SetModeSym(WORD onOff)
{
	m_pRepeatedH230->SetModeSym(onOff);
}

//--------------------------------------------------------------------------
void CMuxCntl::EndAudioNegotiation()
{
	if (m_isAudioOpened)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::EndAudioNegotiation - audio already openned, Name - ", PARTYNAME);
		return;
	}

	if (m_isFullAudioCapsReceived && !m_isAudioChange)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::EndAudioNegotiation  Name - ", PARTYNAME);

		m_isAudioOpened = TRUE;
		DeleteTimer(SETCOMMODETOUT);

		m_pTaskApi->StartH230CommandSeq();

		/************ TEMPORARY send message to ART when audio negotiation ends *********************/
		if (!m_isAudioXmitChnlOpened)
		{
			BOOL open_msg_sent = SendMsgToART(H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ, kIpAudioChnlType, cmCapTransmit);
			if (open_msg_sent)
				m_isAudioXmitChnlOpened = TRUE;
		}

		if (!m_isAudioRcvChnlOpened)
		{
			BOOL open_msg_sent = SendMsgToART(H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ, kIpAudioChnlType, cmCapReceive);
			if (open_msg_sent)
				m_isAudioRcvChnlOpened = TRUE;
		}
		/********************************************************************************************/
		m_pTaskApi->MuxEndH320Connect(*m_pRemoteCap, *m_pCurRemoteComMode, statOK, m_isEncryptionNegotiationEnded); // inform party audio connected
		EndMuxConnect();
	}
	else
	{
		if (!m_isFullAudioCapsReceived)
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::EndAudioNegotiation : negotiation not end - m_isFullAudioCapsReceived is false, Name - ", PARTYNAME);
		}

		if (m_isAudioChange)
		{
			PTRACE2(eLevelInfoNormal, "CMuxCntl::EndAudioNegotiation : negotiation not end - m_isAudioChange is ON, Name - ", PARTYNAME);
		}
	}
}

//--------------------------------------------------------------------------
BOOL CMuxCntl::IsMuxEvent(OPCODE event, CSegment* pParam)
{
	const PMSG_MAP*       pMsgMap = GetMessageMap();
	const PMSG_MAP_ENTRY* pEntry  = pMsgMap ? pMsgMap->m_entries : NULL;
	while (pEntry && pEntry->actFunc != 0)
	{
		if (pEntry->event == event)
		{
			if (event != ACK_IND)  /* in case of ACK_IND we should check its opcode because */
				return TRUE;         /* BondingCntl is waiting same ACK event */

			APIU32   opcode;
			CSegment copyParam(*pParam);
			copyParam >> opcode;
			if ((H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ == opcode) ||
			    (H320_RTP_UPDATE_CHANNEL_REQ == opcode) ||
			    (H320_RTP_UPDATE_CHANNEL_RATE_REQ == opcode) ||
			    (ART_CONTENT_ON_REQ == opcode) ||
			    (ART_CONTENT_OFF_REQ == opcode) ||
			    (ART_EVACUATE_REQ == opcode) ||
			    (ENC_KEYS_INFO_REQ == opcode))

				return TRUE;

			return FALSE;
		}

		pEntry++;
	}

	return FALSE;
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMplAck(CSegment* pParam)
{
	ACK_IND_S* pAckIndStruct = new ACK_IND_S;
	*pParam >> pAckIndStruct->ack_base.ack_opcode
	>> pAckIndStruct->ack_base.ack_seq_num
	>> pAckIndStruct->ack_base.status
	>> pAckIndStruct->ack_base.reason
	>> pAckIndStruct->media_type
	>> pAckIndStruct->media_direction;

	APIU32 opcode    = pAckIndStruct->ack_base.ack_opcode;
	APIU32 status    = pAckIndStruct->ack_base.status;
	APIU32 direction = pAckIndStruct->media_direction;

	TRACESTR(eLevelInfoNormal) << " CMuxCntl::OnMplAck - ACK Received:  Media Type = " << pAckIndStruct->media_type
	                           << ", Media direction = " << direction << ", Opcode = " << opcode << ", Status = " << status;

	if ((H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ == opcode) ||
	    (H320_RTP_UPDATE_CHANNEL_REQ == opcode) ||
	    (H320_RTP_UPDATE_CHANNEL_RATE_REQ == opcode))
	{
		MediaKey                           ackKey((kChanneltype)pAckIndStruct->media_type, opcode);
		std::map<MediaKey, bool>::iterator it = m_ackStatusTable.find(ackKey);

		if (STATUS_OK != status || it == m_ackStatusTable.end() || it->second)
		{
			std::string msgStr = " CMuxCntl::OnMplAck :  ";
			if (STATUS_OK != status)
			{
				msgStr += "wrong status";
			}
			else if (it == m_ackStatusTable.end())
			{
				msgStr += "ACK was not found in the ack table";
			}
			else
			{
				msgStr += "we already received this ACK";
			}

			TRACESTR(eLevelError) << msgStr.c_str();
		}
		else
			it->second = true;       /* set True value to a related ACK entry in the table */

		if (IsAllAcksReceived())   /* continue only when all ACKs are got */
		{
			if (IsValidTimer(MFARESPONSE_TOUT))
				DeleteTimer(MFARESPONSE_TOUT);

			m_ackStatusTable.clear();
		}
	}

	else if (opcode == ART_CONTENT_ON_REQ || opcode == ART_CONTENT_OFF_REQ)
	{
		if (status == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << " CMuxCntl::OnMplAck - ACK Received:  Media Type = " << pAckIndStruct->media_type
			                           << ", Media direction = " << direction << ", Opcode = " << opcode << ", Status = " << status;

			if (direction != cmCapReceive)
				PASSERTMSG(direction, "CMuxCntl::OnMplAck : ART_CONTENT_ON_REQ/ART_CONTENT_OFF_REQ - Wrong direction");

			// Here we will update the stream state accordingly and send a message to the party saying the the ack was received

			if (direction == cmCapReceive)
			{
				if (m_eContentInState != eNoChannel)
				{
					if (opcode == ART_CONTENT_ON_REQ)
					{
						m_eContentInState = eStreamOn;
						TRACEINTO << "CMuxCntl::OnMplAck : ART_CONTENT_ON_REQ - Update ContnetInState = " << m_eContentInState;
					}
					else
					{
						m_eContentInState = eStreamOff;
						TRACEINTO << "CMuxCntl::OnMplAck : ART_CONTENT_OFF_REQ - Update ContnetInState = " << m_eContentInState;
					}
				}

				if (opcode == ART_CONTENT_ON_REQ)
					TRACEINTO << "CMuxCntl::OnMplAck : ART_CONTENT_ON_REQ - ContnetInState = " << m_eContentInState;
				else
					TRACEINTO << "CMuxCntl::OnMplAck : ART_CONTENT_OFF_REQ - ContnetInState = " << m_eContentInState;
			}
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << " CMuxCntl::OnMplAck - ACK Received:  Media Type = " << pAckIndStruct->media_type
			                           << ", Media direction = " << direction << ", Opcode = " << opcode << ", Status = " << status;

			DBGPASSERT(opcode);
		}

		m_pTaskApi->SendContentOnOffAck(status, m_eContentInState);
	}
	else if (opcode == ART_EVACUATE_REQ)
	{
		if (status == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << " CMuxCntl::OnMplAck - ACK Received:  Media Type = " << pAckIndStruct->media_type
			                           << ", Media direction = " << direction << ", Opcode = " << opcode << ", Status = " << status;
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << " CMuxCntl::OnMplAck - ACK Received:  Media Type = " << pAckIndStruct->media_type
			                           << ", Media direction = " << direction << ", Opcode = " << opcode << ", Status = " << status;
			DBGPASSERT(opcode);
		}

		if (m_eContentInState == eStreamOff || m_eContentInState == eSendStreamOff)
			m_pTaskApi->SendContentEvacuateAck(status);
	}
	else if (opcode == ENC_KEYS_INFO_REQ)
	{
		OnMplAckEncKeysInfoReq(pParam);
	}

	PDELETE(pAckIndStruct);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnTimerART(CSegment* pParams)
{
	TRACESTR(eLevelError) << " CMuxCntl::OnTimerART - we did not Recieve Ack for: " << ART_RESPONSE_TIME << " seconds";

	IsAllAcksReceived();
}

//--------------------------------------------------------------------------
BOOL CMuxCntl::SendMsgToART(DWORD opcode, kChanneltype chnlType, DWORD direction)
{
	TRACESTR(eLevelInfoNormal) << " CMuxCntl::SendMsgToART: media type = " << chnlType << ", direction = " << direction << ", opcode = " << opcode;
	CSegment* pMsg = new CSegment;

	if (opcode == H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ ||
	    opcode == H320_RTP_UPDATE_CHANNEL_REQ)
	{
		DWORD capType = GetCapTypeCodeByChnlType(chnlType, direction);

		if (eUnknownAlgorithemCapCode == capType)   /* don't send if a current algorithm is not valid */

		{
			TRACESTR(eLevelInfoNormal) << " CMuxCntl::SendMsgToART - CURRENT ALGORITHM IS NOT VALID : media type = "
			                           << chnlType << ", direction = " << direction << ", opcode = " << opcode;
			POBJDELETE(pMsg);
			return FALSE;
		}

		TUpdatePortOpenRtpChannelReq* pStructReq = new TUpdatePortOpenRtpChannelReq;
		memset(pStructReq, 0, sizeof(TUpdatePortOpenRtpChannelReq));
		pStructReq->bunIsRecovery      = 0;
		pStructReq->unChannelType      = chnlType;
		pStructReq->unChannelDirection = direction;
		pStructReq->unCapTypeCode      = capType;
		pStructReq->unEncryptionType   = (APIU32)-1;
		memset(&(pStructReq->aucSessionKey), '0', sizeOf128Key);
		pStructReq->unSequenceNumber = (APIU32)-1;
		pStructReq->unTimeStamp      = (APIU32)-1;
		pStructReq->unSyncSource     = (APIU32)-1;

		FillUpdateRtpSpecificChannelParams(&pStructReq->tUpdateRtpSpecificChannelParams,
		                                   m_pTargetLocalComMode, chnlType, direction);

		pStructReq->tLprSpecificParams.bunLprEnabled         = 0;
		pStructReq->tLprSpecificParams.unVersionID           = 0;
		pStructReq->tLprSpecificParams.unMinProtectionPeriod = 0;
		pStructReq->tLprSpecificParams.unMaxProtectionPeriod = 0;
		pStructReq->tLprSpecificParams.unMaxRecoverySet      = 0;
		pStructReq->tLprSpecificParams.unMaxRecoveryPackets  = 0;
		pStructReq->tLprSpecificParams.unMaxPacketSize       = 0;

		pMsg->Put((BYTE*)(pStructReq), sizeof(TUpdatePortOpenRtpChannelReq));
		POBJDELETE(pStructReq);
	}

	else if (H320_RTP_UPDATE_CHANNEL_RATE_REQ == opcode)
	{
		DWORD                     IPRate     = 0;
		TUpdateRtpChannelRateReq* pStructReq = new TUpdateRtpChannelRateReq;
		pStructReq->unChannelType      = chnlType;
		pStructReq->unChannelDirection = direction;

		// Rate in *100k like in IP.
		if (kIpContentChnlType == chnlType)
			// IPRate = unifiedComMode.TranslateAmscRateToIPRate(m_pTargetLocalComMode->m_contentMode.GetContentRate())*10;
			IPRate = CUnifiedComMode::TranslateAmscRateToIPRate(m_pTargetLocalComMode->m_contentMode.GetContentRate());
		else // Video channel
			IPRate = m_pTargetLocalComMode->GetMediaBitrate(CComMode::eMediaTypeVideo);

		pStructReq->unNewChannelRate = IPRate;

		pMsg->Put((BYTE*)(pStructReq), sizeof(TUpdateRtpChannelRateReq));
		POBJDELETE(pStructReq);
	}

	TRACESTR(eLevelInfoNormal) << " CMuxCntl::SendMsgToART - media type = " << chnlType
	                           << ", direction = " << direction << ", opcode = " << opcode;

	/* save info about the sent message in order to differentiate ACKs later */
	m_ackStatusTable[MediaKey(chnlType, opcode)] = false;
	/* send message to ART */
	m_pMux->SendMsgToMPL(opcode, pMsg);

	POBJDELETE(pMsg);
	return TRUE;
}

//--------------------------------------------------------------------------
BOOL CMuxCntl::IsAllAcksReceived()
{
	std::string  msgStr;
	unsigned int numberOfAcks = 0;
	for (std::map<MediaKey, bool>::iterator it = m_ackStatusTable.begin();
	     it != m_ackStatusTable.end(); ++it)
		if (it->second)    /* print ACK details if it's got */
		{
			msgStr += "(";
			msgStr += it->first.GetMediaTypeAsString();
			msgStr += ",";
			msgStr += it->first.GetOpcodeAsString();
			msgStr += "), ";
			++numberOfAcks;
		}

	TRACESTR(eLevelInfoNormal) << " CMuxCntl::IsAllAcksReceived for partyId: " << m_pMux->GetPartyRsrcId()
	                           <<", Received " << numberOfAcks << " Acks so far: " << msgStr;
	// Is all ack received
	return (m_ackStatusTable.size() == numberOfAcks ? true : false);
}

//--------------------------------------------------------------------------
CMuxCntl::MediaChangesTable CMuxCntl::GetMediaChannelsChanges(CComMode* pNewScm, CComMode* pOldScm)
{
	ostringstream str;
	// some End Points don't send in their comm mode the correct Xfer_Rate
	// so for calculating the video bit rate we should use our Xfer_Rate
	// that based on the actual num of bonding channels that are connected (Eitan 05/2008)
	WORD                        callWidthForCalc = m_pTargetLocalComMode->GetChnlWidth();

	CMuxCntl::MediaChangesTable mediaChangesTbl;
	/* check Audio changes */
	str << "old_audio_mode = " << pOldScm->GetAudMode() << " , new_audio_mode = " << pNewScm->GetAudMode();
	if (!pOldScm->IsAudioOpen() && pNewScm->IsAudioOpen())
	{
		str << " , audio_open";
		mediaChangesTbl.m_AudioKey = H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ;
	}
	else if (pNewScm->GetAudMode() == pOldScm->GetAudMode() || !pNewScm->IsAudioOpen())
	{
		str << " , audio_no_change";
		mediaChangesTbl.m_AudioKey = NONE;
	}
	else
	{
		str <<" , audio_update";
		mediaChangesTbl.m_AudioKey = H320_RTP_UPDATE_CHANNEL_REQ;
	}

	str << "\n";

	/* check Video changes */
	str << "old_video_mode = " << pOldScm->GetVidMode() << " , new_video_mode = " << pNewScm->GetVidMode();
	if (!pOldScm->VideoOn() && pNewScm->VideoOn())
	{
		str << " , video_open";
		mediaChangesTbl.m_VideoKey = H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ;
	}
	else if (pNewScm->GetVidMode() == pOldScm->GetVidMode())
	{
		DWORD oldVideoBitrate = pOldScm->GetMediaBitrate(CComMode::eMediaTypeVideo, callWidthForCalc);
		DWORD newVideoBitrate = pNewScm->GetMediaBitrate(CComMode::eMediaTypeVideo, callWidthForCalc);

		if (pNewScm->VideoOn() && newVideoBitrate != oldVideoBitrate)
		{
			// CComMode operator== misses video rate changes
			// check video rate
			str << "video_rate_changed: oldVideoBitrate = " << oldVideoBitrate << " , newVideoBitrate = " << newVideoBitrate;
			mediaChangesTbl.m_VideoKey = H320_RTP_UPDATE_CHANNEL_REQ;
		}
		else
		{
			str << " , video_no_change";
			mediaChangesTbl.m_VideoKey = NONE;
		}
	}
	else
	{
		str <<" , video_update";
		mediaChangesTbl.m_VideoKey = H320_RTP_UPDATE_CHANNEL_REQ;
	}

	str << "\n";

	/* check Content changes */
	str << "old_content_rate = " << (DWORD)(pOldScm->m_contentMode.GetContentRate()) << " , new_content_rate = "
	    << (DWORD)(pNewScm->m_contentMode.GetContentRate());
	if (pNewScm->m_contentMode == pOldScm->m_contentMode)
	{
		str << " , content_no_change";
		mediaChangesTbl.m_ContentKey = NONE;
	}
	else if (!pOldScm->m_contentMode.IsContentModeOn() && pNewScm->m_contentMode.IsContentModeOn())
	{
		str << " , content_open";
		mediaChangesTbl.m_ContentKey = H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ;
	}
	else if (pNewScm->m_contentMode.GetContentRate() != pOldScm->m_contentMode.GetContentRate())
	{
		str <<" , content_update_rate";
		mediaChangesTbl.m_ContentKey = H320_RTP_UPDATE_CHANNEL_RATE_REQ;

		if (pOldScm->VideoOn() && pNewScm->VideoOn())
			if (pNewScm->GetVideoBitrateInB(callWidthForCalc) != pOldScm->GetVideoBitrateInB(callWidthForCalc))
				mediaChangesTbl.m_VideoKey = H320_RTP_UPDATE_CHANNEL_RATE_REQ;

	}
	else
	{
		str <<" , content_update/close (do nothing)";
		// we don't send H320_RTP_UPDATE_CHANNEL_REQ for now, only OPEN and UPDATE_CHANNEL_RATE (we got in here when contant channel closed)
		// mediaChangesTbl.m_ContentKey = H320_RTP_UPDATE_CHANNEL_REQ;
	}

	str << "\n";

	/* check Data changes */
	if ((!pOldScm->HsdOn() && pNewScm->HsdOn()) || (!pOldScm->LsdOn() && pNewScm->LsdOn()) ||
	    (!pOldScm->MlpOn() && pNewScm->MlpOn()) || (!pOldScm->HmlpOn() && pNewScm->HmlpOn()))
	{
		str << "data_open";
		mediaChangesTbl.m_DataKey = H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ;
	}
	else if (pNewScm->GetLsdMode() == pOldScm->GetLsdMode() && pNewScm->GetMlpMode() == pOldScm->GetMlpMode() &&
	         pNewScm->GetHsdMode() == pOldScm->GetHsdMode() && pNewScm->GetHmlpMode() == pOldScm->GetHmlpMode())
	{
		str << "data_no_change";
		mediaChangesTbl.m_DataKey = NONE;
	}
	else
	{
		str <<"data_update";
		mediaChangesTbl.m_DataKey = H320_RTP_UPDATE_CHANNEL_REQ;
	}

	PTRACE2(eLevelInfoNormal, "CMuxCntl::GetMediaChannelsChanges: \n", str.str().c_str());

	return mediaChangesTbl;
}

//--------------------------------------------------------------------------/////////////////////////////////
void CMuxCntl::OnMuxRmtXmitModeSpecial(DWORD chnlDirection, CComMode* pNewScm)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMuxRmtXmitModeSpecial  Name - ", PARTYNAME);
	CComMode targetLocalComMode = *m_pTargetLocalComMode;
	*m_pTargetLocalComMode = *pNewScm; // Since sendMsgToArt is always with m_pTargetLocalComMode
	CheckMediaChannelsChanges(chnlDirection, pNewScm, m_pLastRemoteComModeSentToART);
	*m_pTargetLocalComMode = targetLocalComMode;
}

//--------------------------------------------------------------------------///////////////////////////
BOOL CMuxCntl::CheckMediaChannelsChanges(DWORD chnlDirection, CComMode* pNewScm, CComMode* pOldScm)
{
	/* check if there is some communication mode difference */
	MediaChangesTable tbl = this->GetMediaChannelsChanges(pNewScm, pOldScm);

	BOOL while_audio_negotiation  = (MUX_SETUP == m_state);
	BOOL need_send_audio_open_msg = ((cmCapTransmit == chnlDirection && !m_isAudioXmitChnlOpened) ||
	                                 (cmCapReceive == chnlDirection && !m_isAudioRcvChnlOpened));
	/* send message to ART if there are audio mode changes */
	if ((tbl.m_AudioKey != NONE || need_send_audio_open_msg) &&
	    while_audio_negotiation) /* sending update for audio should be while audio negotiation only */

	{
		TRACESTR(eLevelInfoNormal) << " CMuxCntl::CheckMediaChannelsChanges : AUDIO MEDIA ART MESSAGES ARE DELAYED"
		" TO BE AFTER AUDIO NEGOTIATION END ";
	}

	/* send message to ART if there are video mode changes */
	if (tbl.m_VideoKey != NONE)
	{
		SendMsgToART(tbl.m_VideoKey, kIpVideoChnlType, chnlDirection);
	}

	if (tbl.m_ContentKey != NONE)
	{
		SendMsgToART(tbl.m_ContentKey, kIpContentChnlType, chnlDirection);
	}

	/* send message to ART if there are data mode changes */
	if (tbl.m_DataKey != NONE)
	{
		TRACESTR(eLevelInfoNormal) << " CMuxCntl::CheckMediaChannelsChanges : FECC is not supported, so art messages will not be sent ";
	}

	if (m_ackStatusTable.size() > 0) /* start a timer if some message was sent */

	{
		TRACESTR(eLevelInfoNormal) << " CMuxCntl::CheckMediaChannelsChanges StartTimer(MFARESPONSE_TOUT) of "
		                           << ART_RESPONSE_TIME << " seconds";
		if (IsValidTimer(MFARESPONSE_TOUT))
			DeleteTimer(MFARESPONSE_TOUT);

		StartTimer(MFARESPONSE_TOUT, ART_RESPONSE_TIME * SECOND);
		return TRUE;
	}

	return FALSE;
}
//--------------------------------------------------------------------------
void CMuxCntl::FillUpdateRtpSpecificChannelParams(TUpdateRtpSpecificChannelParams* pUpdateRtpSpecificChannelParams,
                                                  CComMode* scm, kChanneltype chnlType, DWORD chnlDirection)
{
	DWORD rate = scm->GetMediaBitrate(MediaKey::GetMediaTypeByChnlType(chnlType));
	/* need to reduce a bitrate for 4% in case of video or content transmitted */
	pUpdateRtpSpecificChannelParams->unBitRate                          = rate;
	pUpdateRtpSpecificChannelParams->unPayloadType                      = (APIU32)-1;
	pUpdateRtpSpecificChannelParams->unDtmfPayloadType                  = _UnKnown;
	pUpdateRtpSpecificChannelParams->bunIsH263Plus                      = (APIU32)-1;
	pUpdateRtpSpecificChannelParams->bunIsCCEnabled                     = 0;
	pUpdateRtpSpecificChannelParams->bunIsFlipIntraBit                  = (APIU32)-1;
	pUpdateRtpSpecificChannelParams->unAnnexesMask                      = (APIU32)-1;
	pUpdateRtpSpecificChannelParams->nHcMpiCif                          = -1;
	pUpdateRtpSpecificChannelParams->nHcMpiQCif                         = -1;
	pUpdateRtpSpecificChannelParams->nHcMpi4Cif                         = -1;
	pUpdateRtpSpecificChannelParams->nHcMpi16Cif                        = -1;
	pUpdateRtpSpecificChannelParams->nHcMpiVga                          = -1;
	pUpdateRtpSpecificChannelParams->nHcMpiNtsc                         = -1;
	pUpdateRtpSpecificChannelParams->nHcMpiSvga                         = -1;
	pUpdateRtpSpecificChannelParams->nHcMpiXga                          = -1;
	pUpdateRtpSpecificChannelParams->nHcMpiSif                          = -1;
	pUpdateRtpSpecificChannelParams->nHcMpiQvga                         = -1;
	pUpdateRtpSpecificChannelParams->b32StreamRequiresEndOfFrameParsing = FALSE;
	pUpdateRtpSpecificChannelParams->unCustomMaxMbpsValue               = (APIU32)-1;
	pUpdateRtpSpecificChannelParams->unMaxFramesPerPacket               = 1;                                   // TODO
	pUpdateRtpSpecificChannelParams->unDestMcuId                        = m_pParty->GetMcuNum();
	pUpdateRtpSpecificChannelParams->unDestTerminalId                   = m_pParty->GetTerminalNum();
	pUpdateRtpSpecificChannelParams->bunContentEnabled                  = 0;                                   // TODO
	pUpdateRtpSpecificChannelParams->unPacketPayloadFormat              = E_PACKET_PAYLOAD_FORMAT_SINGLE_UNIT; // TODO

	if (kIpContentChnlType == chnlType)
	{
		// Content In Channel - for writing
		if (chnlDirection != cmCapTransmit) // Before open content in conf
		{
			if (m_eContentInState == eNoChannel)
			{
				m_eContentInState = eStreamOff;                                 // each content channel will set to StreamOff
			}
		}
		else                                                                // Content Out channel - for reading
		if (chnlDirection == cmCapTransmit) // Before IVR ends
		{
			if (m_eContentOutState == eNoChannel)                             // we open content channel before IVR ends.
			{
				m_eContentOutState = eStreamOff;                                // each content channel will set to StreamOff
			}
			// After IVR
			else if (m_eContentOutState == eWaitToSendStreamOn)               // we already receive IN conf.
			{
				pUpdateRtpSpecificChannelParams->bunContentEnabled = (APIU32)eStreamOn;
				m_eContentOutState                                 = eStreamOn; // We don't wait for ack.
				m_pTaskApi->MfaUpdatedPresentationOutStream();
			}
		}
	}
}

//--------------------------------------------------------------------------///////////
void CMuxCntl::UpdatePresentationOutStream()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::UpdatePresentationOutStream  Name - ", PARTYNAME);

	// No need to send updatePortOpen - will be send by set_xmit_mode
	if (m_eContentOutState == eNoChannel)
		m_eContentOutState = eWaitToSendStreamOn;

	// updatePortOpen was already sent once when we send Xmit_Mode(0k)-Now we need to send it again with StreamOn.
	if (m_eContentOutState == eStreamOff)
	{
		m_eContentOutState = eWaitToSendStreamOn;
		DWORD opcode = H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ;
		SendMsgToART(opcode, kIpContentChnlType, cmCapTransmit);
	}
	else
		TRACESTR(eLevelInfoNormal) << " CMuxCntl::UpdatePartyInConf - PresentationOutState: " << m_eContentOutState;
}

//--------------------------------------------------------------------------
DWORD CMuxCntl::GetCapTypeCodeByChnlType(kChanneltype chnlType, DWORD direction)
{
	CComMode* scm = (cmCapTransmit == direction ? m_pTargetLocalComMode : m_pCurRemoteComMode);
	switch (chnlType)
	{
		case kIpAudioChnlType:
		{
			WORD audMode = scm->GetAudMode();
			switch (audMode)
			{
				case A_Law_OU        : return eG711Alaw64kCapCode;
				case U_Law_OU        : return eG711Ulaw64kCapCode;
				case A_Law_OF        : return eG711Alaw56kCapCode;
				case U_Law_OF        : return eG711Ulaw56kCapCode;
				case G722_m1         : return eG722_64kCapCode;
				case G722_m2         : return eG722_56kCapCode;
				case G722_m3         : return eG722_48kCapCode;
				case Au_32k          : return eG7221_32kCapCode;
				case Au_24k          : return eG7221_24kCapCode;
				case Au_Siren14_24k  : return eSiren14_24kCapCode;
				case Au_Siren14_32k  : return eSiren14_32kCapCode;
				case Au_Siren14_48k  : return eSiren14_48kCapCode;
				case G7221_AnnexC_48k: return eG7221C_48kCapCode;
				case G7221_AnnexC_32k: return eG7221C_32kCapCode;
				case G7221_AnnexC_24k: return eG7221C_24kCapCode;
				case G728            : return eG728CapCode;
				default              : return eUnknownAlgorithemCapCode;
			}
		}
		break;

		case kIpVideoChnlType:
		{
			WORD videoMode = scm->GetVidMode();
			switch (videoMode)
			{
				case H264: return eH264CapCode;
				case H263: return eH263CapCode;
				case H261: return eH261CapCode;
				default  : return eUnknownAlgorithemCapCode;
			}
		}
		break;

		case kIpContentChnlType: return eH263CapCode;
		case kIpFeccChnlType   : return eAnnexQCapCode;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}

	return eUnknownAlgorithemCapCode;
}

//--------------------------------------------------------------------------
void CMuxCntl::SendContentOnOffReqToMux()
{
	PTRACE2INT(eLevelInfoNormal, "CMuxCntl::SendContentOnOffReqToMux - Stream state = ", (APIU32)m_eContentInState);

	if (m_eContentInState == eWaitToSendStreamOn)
	{
		PASSERT_AND_RETURN((APIU32)eWaitToSendStreamOn);
	}

	DWORD  dwOpcode   = 0;
	APIU32 bunIsOnOff = 0;

	if (m_eContentInState == eSendStreamOn)
	{
		bunIsOnOff = 1;
		dwOpcode   = ART_CONTENT_ON_REQ;
	}
	else if (m_eContentInState == eSendStreamOff)
	{
		bunIsOnOff = 0;
		dwOpcode   = ART_CONTENT_OFF_REQ;
	}
	else
	{
		PASSERTMSG((DWORD)m_eContentInState, "CMuxCntl::SendContentOnOffReqToMux - Wrong status");
		return;
	}

	CSegment* pMsg = new CSegment;

	TContentOnOffReq* pStruct   = new TContentOnOffReq;
	pStruct->unChannelDirection = cmCapReceive;
	pStruct->bunIsOnOff         = bunIsOnOff;

	pMsg->Put((BYTE*)(pStruct), sizeof(TContentOnOffReq));

	m_pMux->SendMsgToMPL(dwOpcode, pMsg);
	PDELETE(pStruct);
	POBJDELETE(pMsg);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendEvacuateReqForMuxOnH239Stream()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::SendEvacuateReqForMuxOnH239Stream - Name ", PARTYNAME);

	DWORD     dwOpcode = 0;
	CSegment* pMsg     = new CSegment;

	dwOpcode = ART_EVACUATE_REQ;
	TEvacuateReq* pStruct = new TEvacuateReq;

	pStruct->unChannelType      = (DWORD)kIpContentChnlType;
	pStruct->unChannelDirection = cmCapReceive;

	pMsg->Put((BYTE*)(pStruct), sizeof(TEvacuateReq));

	m_pMux->SendMsgToMPL(dwOpcode, pMsg);
	PDELETE(pStruct);
	POBJDELETE(pMsg);
}

//--------------------------------------------------------------------------
void CMuxCntl::EndMuxConnect()
{
	ostringstream str;
	if (m_isAudioOpened && m_isCapExchangeEnd && m_isEncryptionNegotiationEnded)
	{
		str <<  "Audio_Negotiation and Capabilities_exchange completed - end connect";
		m_state = MUX_CONNECT;

		if (!(m_pCurRemoteComMode->m_xferMode == m_pTargetLocalComMode->m_xferMode))
		{
			PTRACE(eLevelInfoNormal, "CMuxCntl::EndMuxConnect Remote Xfer_Mode != Local Xfer_Mode -> Ignore remote and put instead local");
			DBGPASSERT(1);
			m_pCurRemoteComMode->SetXferMode(m_pTargetLocalComMode->GetXferMode());
		}

		/* start the change mode procedure (open content, video) */
		m_pTaskApi->MuxReceivedPartyFullCapSet(*m_pRemoteCap, *m_pCurRemoteComMode);
	}
	else
	{
		if (!m_isAudioOpened)
		{
			str <<  "Audio_Negotiation not completed  ";
		}

		if (!m_isCapExchangeEnd)
		{
			str <<  "Capabilities_exchange not completed";
		}

		if (!m_isEncryptionNegotiationEnded)
		{
			str <<  "Encryption negotiation not completed";
		}
	}

	PTRACE2(eLevelInfoNormal, "CMuxCntl::EndMuxConnect: ", str.str().c_str());
}

//--------------------------------------------------------------------------
void CMuxCntl::OnTimerInitComSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimerInitComSetup : Name - ", PARTYNAME);

	DeleteTimer(INITCOMTOUT);

	if (DEBUG_IGNORE_TIMERS)
	{
		PTRACE2(eLevelError, "***** CMuxCntl::OnTimerInitComSetup, temporary for debug - timer ignored ***** "
		        " Name - ", PARTYNAME);
		return;
	}

	// simulate dummy parameters for end_init_com
	CCapH320* rmtCap     = new CCapH320;
	CComMode* currentScm = new CComMode;

	m_pTaskApi->TimeOutMuxEndH320Connect(*rmtCap, *currentScm, CAP_EXCHANGE_FAILURE_IN_ESTABLISH_CALL);

	POBJDELETE(rmtCap);
	POBJDELETE(currentScm);
}

//--------------------------------------------------------------------------
void CMuxCntl::AskEpForIntra()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::AskEpForIntra: Name - ", PARTYNAME);
	CSegment H221String;
	H221String << (BYTE)(OTHRCMDATTR | Fast_Update);
	m_pMux->SendH230(H221String);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendH239VideoCaps()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::SendH239VideoCaps: Name - ", PARTYNAME);

	if (m_pLocalCap->IsH239Cap())
	{
		SendH239ExtendedVideoCaps();
	}
}

//--------------------------------------------------------------------------
void CMuxCntl::SetNewConfRsrcId(DWORD confRsrcId)  // Move update confId
{
	m_pMux->SetConfRsrcId(confRsrcId);
}

//--------------------------------------------------------------------------
// send VCF to remote
void CMuxCntl::FreezePic()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::FreezePic : Name - ", PARTYNAME);
	CSegment H221String;
	H221String << (BYTE)(OTHRCMDATTR | Freeze_Pic);
	m_pMux->SendH230(H221String);
}

//--------------------------------------------------------------------------
bool CMuxCntl::IsAdvancedAudioAlg(WORD commModeAudioAlg)
{
	bool is_advanced = false;

	BOOL bG7221_C_Advanced = TRUE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("ISDN_AUDIO_G7221_C_ADVANCED", bG7221_C_Advanced);
	if (!bG7221_C_Advanced)
	{
		PTRACE2(eLevelInfoNormal, "CMuxCntl::IsAdvancedAudioAlg bG7221_C_Advanced is set to NO - ", PARTYNAME);
	}

	switch (commModeAudioAlg)
	{
		case G7221_AnnexC_48k:
		case G7221_AnnexC_32k:
		case G7221_AnnexC_24k:
		{
			if (bG7221_C_Advanced)
			{
				is_advanced = true;
			}
			break;
		}

		case Au_32k:
		case Au_24k:
		case Au_Siren14_48k:
		case Au_Siren14_32k:
		case Au_Siren14_24k:
		{
			is_advanced = true;
			break;
		}
	} // switch

	std::ostringstream ostr;
	ostr << "CMuxCntl::IsAdvancedAudioAlg , commModeAudioAlg = " << commModeAudioAlg;
	if (is_advanced)
	{
		ostr << " return true";
	}
	else
	{
		ostr << " return false";
	}

	PTRACE(eLevelInfoNormal, ostr.str().c_str());

	return is_advanced;
}

//--------------------------------------------------------------------------
bool CMuxCntl::IsAdvancedAudioAlgSupportedByRemoteCaps(WORD commModeAudioAlg)
{
	bool is_supported = true;
	switch (commModeAudioAlg)
	{
		case G7221_AnnexC_48k:
		{
			if (m_pRemoteCap->OnAudioCap(e_G722_1_Annex_C_48))
			{
				is_supported = true;
			}
			else
			{
				is_supported = false;
			}
			break;
		}

		case G7221_AnnexC_32k:
		{
			if (m_pRemoteCap->OnAudioCap(e_G722_1_Annex_C_32))
			{
				is_supported = true;
			}
			else
			{
				is_supported = false;
			}
			break;
		}

		case G7221_AnnexC_24k:
		{
			if (m_pRemoteCap->OnAudioCap(e_G722_1_Annex_C_24))
			{
				is_supported = true;
			}
			else
			{
				is_supported = false;
			}
			break;
		}

		case Au_32k:
		{
			if (m_pRemoteCap->OnAudioCap(e_G722_1_32))
			{
				is_supported = true;
			}
			else
			{
				is_supported = false;
			}
			break;
		}

		case Au_24k:
		{
			if (m_pRemoteCap->OnAudioCap(e_G722_1_24))
			{
				is_supported = true;
			}
			else
			{
				is_supported = false;
			}
			break;
		}

		case Au_Siren14_48k:
		{
			if (m_pRemoteCap->GetNSCap()->OnSiren1448())
			{
				is_supported = true;
			}
			else
			{
				is_supported = false;
			}
			break;
		}

		case Au_Siren14_32k:
		{
			if (m_pRemoteCap->GetNSCap()->OnSiren1432())
			{
				is_supported = true;
			}
			else
			{
				is_supported = false;
			}
			break;
		}

		case Au_Siren14_24k:
		{
			if (m_pRemoteCap->GetNSCap()->OnSiren1424())
			{
				is_supported = true;
			}
			else
			{
				is_supported = false;
			}
			break;
		}
	} // switch

	std::ostringstream ostr;
	ostr << "CMuxCntl::IsAdvancedAudioAlgSupportedByRemoteCaps, commModeAudioAlg = " << commModeAudioAlg;
	if (is_supported)
	{
		ostr << " return true";
	}
	else
	{
		ostr << " return false";
	}

	PTRACE(eLevelInfoNormal, ostr.str().c_str());

	return is_supported;
}

//--------------------------------------------------------------------------
void CMuxCntl::OnSmartRecovery(CSegment* pParam)
{
	m_pTaskApi->OnSmartRecovery();
}

//--------------------------------------------------------------------------
void CMuxCntl::OnTimerSyncRecover(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CMuxCntl::OnTimerSyncRecover stop ignoring REMOTE_BAS_CAPS or REMOTE_XMIT_MODE");
}

//--------------------------------------------------------------------------
BYTE CMuxCntl::IsMuxOnSyncRecover()
{
	if (CStateMachine::IsValidTimer(SYNC_RECOVER_TOUT) || !m_remoteSyncFlag)
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------------
void CMuxCntl::OnRtpVideoUpdatePicInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CMuxCntl::OnRtpVideoUpdatePicInd - request an intra from EP (art can't sync BCH)");
	AskEpForIntra();
}

//--------------------------------------------------------------------------
void CMuxCntl::OnMplAckEncKeysInfoReq(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnMplAckEncKeysInfoReq [enc_trace]: Name - ", PARTYNAME);

	m_isEncryptionNegotiationEnded = TRUE;
	m_pTaskApi->UpdateEncryptionCurrentStateInDB(YES);
	EndMuxConnect();
}

//--------------------------------------------------------------------------
BOOL CMuxCntl::IsReducedCapabilities(CCapH320& rCapH320)
{
	BOOL isReduced = FALSE;
	isReduced = rCapH320.IsReducedcapbilities();
	return isReduced;
}

//--------------------------------------------------------------------------
void CMuxCntl::StartZeroModeFlow()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::StartZeroModeFlow , ", PARTYNAME);
	SendInitialCapSet();
	StartTimer(ZERO_MODE_AFTER_INITIAL_CAPS_TOUT, SECOND);
}

//--------------------------------------------------------------------------
void CMuxCntl::SendInitialCapSet()
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::SendInitialCapSet , ", PARTYNAME);
	CCapH320* pInitialCaps = new CCapH320();

	pInitialCaps->OnAudioCap(e_A_Law); // G711
	pInitialCaps->OnAudioCap(e_U_Law);
	pInitialCaps->OnAudioCap(e_G722_48);
	pInitialCaps->SetXferRateFromScm(*m_pCurLocalComMode);
	pInitialCaps->SetH261Caps(V_Cif, V_1_29_97, V_1_29_97);
	m_pMux->ExchangeCap(*pInitialCaps, FALSE);

	POBJDELETE(pInitialCaps);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnTimetZeroModeAfterInitialCaps(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMuxCntl::OnTimetZeroModeAfterInitialCaps - sending full cap set, ", PARTYNAME);
	ExchangeCap(*m_pLocalCap);
	StartTimer(INTRA_AFTER_ZERO_MODE_TOUT, 2*SECOND);
}

//--------------------------------------------------------------------------
void CMuxCntl::OnTimerIntraAfterZeroMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "MuxCntl::OnTimerIntraAfterZeroMode , ", PARTYNAME);
	m_zero_mode_intra_req_counter++;
	// ask EP for video intra
	AskEpForIntra();
	// ask EP for content intra
	BYTE isContentOn = m_pCurRemoteComMode->m_contentMode.IsContentModeOn();
	if (isContentOn)
	{
		CContentMode* pContentMode = new CContentMode(m_pCurLocalComMode->m_contentMode);
		if (pContentMode)
		{
			m_pTaskApi->SendH239ContentVideoMode(pContentMode);
		}

		POBJDELETE(pContentMode);
		m_pTaskApi->SendContentVideoRefresh();
	}

	if (m_zero_mode_intra_req_counter < 4)
	{
		StartTimer(INTRA_AFTER_ZERO_MODE_TOUT, 2*SECOND);
	}
}

