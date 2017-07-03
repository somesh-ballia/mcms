#ifndef _MUX_CNTL_H_
#define _MUX_CNTL_H_

#include <map>
#include "CDRDefines.h"
#include "IpRtpReq.h"
#include "IpChannelParams.h"
#include "H320Caps.h"
#include "OpcodesMcmsMux.h"
#include "PartyApi.h"
#include "StateMachine.h"

#define thisIsNotInitAudioSelection 0x02

const WORD  H320CON                           = 100;
const WORD  SETCOMMODE                        = 101;
const WORD  INITCOMTOUT                       = 113;
const WORD  SYNCTOUT                          = 115;
const WORD  SETCOMMODETOUT                    = 116;
const WORD  RMTAUDIOTOUT                      = 122;
const WORD  RMTAUDIOSTABILIZETOUT             = 123;
const WORD  RMTCAPSTOUT                       = 124;
const WORD  UPDATE_AUD_ALGTOUT                = 125;
const WORD  RMT_ENCTOUT                       = 128;
const WORD  RMT_ENC_ALGTOUT                   = 129;
const WORD  EXNGCAP                           = 141;
const WORD  ENCRYPT_ENDTOUT                   = 154;
const WORD  SYNC_RECOVER_TOUT                 = 155;
const WORD  ZERO_MODE_AFTER_INITIAL_CAPS_TOUT = 156;
const WORD  INTRA_AFTER_ZERO_MODE_TOUT        = 157;

const DWORD RMT_AUDIO_TOUT          = 5*SECOND;
const DWORD RMT_MCU_AUDIO_TOUT      = 15*SECOND;  // VNGFE-4652 - use a long timeout when connecting ISDN cascade (for cascade with MGC)
const DWORD RMT_AUDIOSTABILIZE_TOUT = 1*SECOND;
const DWORD RMT_CAPS_TOUT           = 4*SECOND;
const DWORD SET_COMMODE_TOUT        = 60*SECOND;
const DWORD SET_COMMODE_TOUT_10     = 10*SECOND;
const DWORD MUX_CONNECT_TOUT        = 60*SECOND;
const DWORD MUX_SYNC_TOUT           = 30*SECOND;
const DWORD RMT_ENC_TOUT            = 10*SECOND;
const DWORD RMT_ENC_ALG_TOUT        = 15*SECOND;
const DWORD ENCRYPT_END_TOUT        = 20*SECOND;
const DWORD UPDATE_AUD_ALG_TOUT     = 25;

const BYTE  NUM_MUX_SYNC_CHANNLS = 6;

class CMuxHardwareInterface;
class CSegment;
class CParty;
class CIsdnEncryptCntl;

////////////////////////////////////////////////////////////////////////////
//                        CH230Repeated
////////////////////////////////////////////////////////////////////////////
class CH230Repeated : public CPObject
{
	CLASS_TYPE_1(CH230Repeated, CPObject)

public:
	CH230Repeated();
	virtual ~CH230Repeated();

	virtual const char* NameOf() const { return "CH230Repeated"; }
	void           Serialize(WORD format, CSegment& H221StringSeg);
	void           SetIsMcu(WORD onOff);
	void           SetAudio(WORD onOff);
	void           SetVideo(WORD onOff);
	void           SetLoop(WORD onOff);
	void           SetT120(WORD onOff);
	void           SetChair(WORD onOff);
	void           SetModeSym(WORD onOff);
	WORD           GetModeSym() const;
	// MMS control
	WORD           GetIsMmsEnabled();
	void           SetIsMmsEnabled(WORD onOff);

protected:
	WORD           m_audio;
	WORD           m_video;
	WORD           m_loop;
	WORD           m_mcu;
	WORD           m_dataSym;
	WORD           m_modeSym;       // h230 command - MMS
	WORD           m_T120;
	WORD           m_chair;
	WORD           m_isMmsEnabled;
};


////////////////////////////////////////////////////////////////////////////
//                        CMuxCntl
////////////////////////////////////////////////////////////////////////////
class CMuxCntl : public CStateMachine
{
	CLASS_TYPE_1(CMuxCntl, CStateMachine)

public:
	CMuxCntl();
	CMuxCntl(CRsrcParams& rsrcDesc, CParty* pParty);
	virtual ~CMuxCntl();

	virtual const char* NameOf() const;
	virtual void   Dump(WORD swithcFlag = 0);
	virtual void   HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	BOOL           IsMuxEvent(OPCODE opCode, CSegment* pParam);

	// mux API for party manager
	void           Connect(CComMode& targetComMode, BYTE mcuNum, BYTE termNum);
	void           Disconnect();

	void           SetXmitComMode(CComMode& targetComMode, WORD bitrateFlag = 0, WORD changeModeType = REGULAR_CHANGE_MODE);

	void           OnPartyH320ConIdle(CSegment* pParam);
	void           OnPartyH320ConSetup(CSegment* pParam);
	void           OnPartyH320Con(CSegment* pParam);

	void           OnPartyExchngCapSetup(CSegment* pParam);
	void           OnPartyExchngCapChangeMode(CSegment* pParam);
	void           OnPartyExchngCapConnect(CSegment* pParam);
	void           OnPartyExchngCap(CSegment* pParam);

	void           OnPartySetComModeChangeMode();
	void           OnPartySetComModeConnect();
	void           OnPartySetComMode();
	void           OnPartySetCommMode(CSegment* pParam);

	void           OnPartyDisconnectSetup(CSegment* pParam);
	void           OnPartyDisconnectChangeMode(CSegment* pParam);
	void           OnPartyDisconnectConnect(CSegment* pParam);
	void           OnPartyDisconnect(CSegment* pParam = NULL);

	void           OnMuxSyncRegainSetup(CSegment* pParam);
	void           OnMuxSyncRegainChangeMode(CSegment* pParam);
	void           OnMuxSyncRegainConnect(CSegment* pParam);
	void           OnMuxSyncLostSetup(CSegment* pParam);
	void           OnMuxSyncLostChangeMode(CSegment* pParam);
	void           OnMuxSyncLostConnect(CSegment* pParam);
	void           OnMuxSync(WORD localRemote, WORD lostRegain, WORD chnlNum);

	void           OnMuxMFSyncRegainSetup(CSegment* pParam);
	void           OnMuxMFSyncRegainChangeMode(CSegment* pParam);
	void           OnMuxMFSyncRegainConnect(CSegment* pParam);

	void           OnMuxEndInitComSetup(CSegment* pParam);

	void           OnMuxRmtBasSetup(CSegment* pParam);
	void           OnMuxRmtBasChangeMode(CSegment* pParam);
	void           OnMuxRmtBasConnect(CSegment* pParam);
	void           OnMuxRmtBas(CSegment* pParam);

	void           OnMuxRmtXmitModeSetup();
	void           OnMuxRmtXmitModeChangeMode();
	void           OnMuxRmtXmitModeConnect();
	void           OnMuxRmtXmitMode(CSegment* pParam);
	void           OnMuxRmtXmitModeSpecial(DWORD chnlDirection, CComMode* pNewScm);

	void           OnMuxRmt230Setup(CSegment* pParam);
	void           OnMuxRmt230ChangeMode(CSegment* pParam);
	void           OnMuxRmt230Connect(CSegment* pParam);
	void           OnMuxRmt230(CSegment* pParam);

	void           OnRtpVideoUpdatePicInd(CSegment* pParam);

	void           OnMuxRemoteECS(CSegment* pParam);

	void           OnTimerSetComModeSetup(CSegment* pParam);
	void           OnTimerRmtAudioSetup(CSegment* pParam);
	void           OnTimerRmtAudioStabilizeSetup(CSegment* pParam);
	void           OnTimerRmtCapsSetup(CSegment* pParam);
	void           OnTimerUpdateAudioAlgorithm(CSegment* pParam);
	void           OnTimerART(CSegment* pParams);
	void           OnTimerInitComSetup(CSegment* pParam);
	void           OnTimerSync(CSegment* pParam);

	void           OnMplAck(CSegment* pParam);
	void           OnMplAckEncKeysInfoReq(CSegment* pParam);

	void           SetInitialXmitMode(CSegment& xmitSeg);

	void           SetLocalCaps(CCapH320& localCap);

	void           SetIsMmsEnabled(WORD onOff);
	void           SetModeSym(WORD onOff);

	void           ExchangeCap(CCapH320& rLocalCap, APIS16 isH263_2000Cap = -1);

	void           SetRmtH239ExtendedVidCaps(CSegment& seg, BYTE len);
	void           SetRmtAMCCaps(CSegment& seg, BYTE len);
	BYTE           IsPartyH239();

	eContentState  GetContentInStreamState()                                   { return m_eContentInState;}
	eContentState  GetContentOutStreamState()                                  { return m_eContentOutState;}
	void           SetContentInStreamState(eContentState contentInStreamState) { m_eContentInState = contentInStreamState;}

	void           UpdatePresentationOutStream();
	void           SendContentOnOffReqToMux();
	void           SendEvacuateReqForMuxOnH239Stream();

	BYTE           GetContentModeControlID() const {return m_pTargetLocalComMode->GetContentModeControlID();}
	void           SendH230(WORD attrib, WORD value);
	void           SendH230(CSegment& h230String);
	void           SendH230Mbe(CSegment* pSeg);
	void           SendH230Sbe(CSegment* pSeg);
	void           SendAudioValid(WORD onOff);

	void           EndMuxConnect();
	void           AskEpForIntra();
	void           SendH239VideoCaps();
	void           FreezePic();
	void           SendMMS(WORD onOff);
	void           SendMCC(WORD onOff);

	void           SetNewConfRsrcId(DWORD confRsrcId); // Move update confId

	void           OnSmartRecovery(CSegment* pParam);
	void           OnTimerSyncRecover(CSegment* pParam);
	BYTE           IsMuxOnSyncRecover();

	void           AddH261ToLocalCapsAndSenfReCapIfNeeded();
	void           SimOnMuxRmtXmitMode();

	// states
	enum STATE {MUX_IDLE, MUX_SETUP, MUX_CONNECT, MUX_CHANGEMODE};

	CCapH320*      m_pLocalCap;

protected:
	// Audio negotiation functions
	void           OpenAudio();   // start of audio negotiation

	void           UpdateAudioAlgorithm(WORD new_audio_algorithm, BYTE isInitialAudioAlgorithmSelection);
	void           UpdateG7221(CCapH320& targetCap);
	void           AudioModeSelection(CAudMode& audMode, WORD rstrict);

	void           OpenEncryption();
	void           ReplyMuxEndSetXmitComModeStatusOK();
	void           SendXmitComMode(CComMode& targetComMode, WORD bitrateFlag);
	BOOL           SendMsgToART(DWORD opcode, kChanneltype type, DWORD direction);
	void           SendEncP8P0();
	void           SendH239ExtendedVideoCaps();

	BYTE           CheckEncryptionSetupProcessCompleted();
	BYTE           IsInitAudioSelection();
	BYTE           IsContentAsymetricModeInIVRMode();
	WORD           IsVideoAsymetricMode();
	BOOL           IsRmtCapsBetter();
	DWORD          GetCapsetLenghtBytes(CCapH320* pCap, BYTE isShortCapSet);

	void           EndAudioNegotiation();
	WORD           IsAdvancedAudioAlgReceived();
	bool           IsAdvancedAudioAlg(WORD commModeAudioAlg);
	bool           IsAdvancedAudioAlgSupportedByRemoteCaps(WORD commModeAudioAlg);

	/* this struct is designed for saving info about requests are sent to ART.
	 * Than it will allow to differentiate ACK_IND and to check if all ACKs are got */
	struct MediaKey
	{
		             MediaKey(kChanneltype type, DWORD opcode) : m_mediaType(type), m_opcode(opcode) { }

		bool         operator<(const MediaKey& other) const;

		const char*  GetMediaTypeAsString() const;
		const char*  GetOpcodeAsString() const;
		static
			CComMode::eMediaType GetMediaTypeByChnlType(kChanneltype type);

		kChanneltype m_mediaType;
		DWORD        m_opcode;
	};

	/* this struct is used for keepeng com mode changes (per channel type) that should be
	 * sent to ART (each member can be equal either NONE or H320_RTP_UPDATE_..._REQ */
	class MediaChangesTable
	{
		public:
			           MediaChangesTable() : m_AudioKey(NONE), m_VideoKey(NONE), m_ContentKey(NONE), m_DataKey(NONE) { }

		DWORD        m_AudioKey;
		DWORD        m_VideoKey;
		DWORD        m_ContentKey;
		DWORD        m_DataKey;
	};

	/* The function checks if all ACKs recieved for sent messages kept by m_ackStatusTable */
	BOOL                     IsAllAcksReceived();

	/* The function checks com mode changes (per channel type) and send message to ART if it's needed */
	BOOL                     CheckMediaChannelsChanges(DWORD chnlDirection, CComMode* pNewScm, CComMode* pOldScm);

	/* The function fills parameters of TUpdateRtpSpecificChannelParams struct */
	void                     FillUpdateRtpSpecificChannelParams(TUpdateRtpSpecificChannelParams* pUpdateRtpSpecificChannelParams,
	                                                            CComMode* scm, kChanneltype mediaType, DWORD chnlDirection);

	/* The function returns cap type according to channel type */
	DWORD                    GetCapTypeCodeByChnlType(kChanneltype chnlType, DWORD direction);

	/* The useful function checks and returns com mode changes (per channel type) */
	MediaChangesTable        GetMediaChannelsChanges(CComMode* pNewScm, CComMode* pOldScm);

	void                     AudioNegotiationRmtCaps();

	// zero mode
	BOOL                     IsReducedCapabilities(CCapH320& rCapH320);
	void                     StartZeroModeFlow();
	void                     SendInitialCapSet();
	void                     OnTimetZeroModeAfterInitialCaps(CSegment* pParam);
	void                     OnTimerIntraAfterZeroMode(CSegment* pParam);

protected:
	CMuxHardwareInterface*   m_pMux;
	CParty*                  m_pParty;
	CPartyApi*               m_pTaskApi;
	CH230Repeated*           m_pRepeatedH230;
	CCapH320*                m_pRemoteCap;                       // m_pRmtCap
	CComMode*                m_pCurLocalComMode;                 // m_pCurComMode - current local scm
	CComMode*                m_pTargetLocalComMode;              // m_pTargetComMode- target local scm
	CComMode*                m_pCurRemoteComMode;                // m_pRmtXmitComMode- current remote scm
	CComMode*                m_pTargetRemoteComMode;             // m_pExpectedRcvComMode- target remote scm
	CComMode*                m_pLastRemoteComModeSentToART;      // need to save a last remote com mode in order to compare previos and current com modes while opening/updating ART channels
	CIsdnEncryptCntl*        m_pEncrypt;
	WORD                     m_numChnl;
	WORD                     m_remoteSync[NUM_MUX_SYNC_CHANNLS]; // sync lost
	WORD                     m_localSync[NUM_MUX_SYNC_CHANNLS];  // sync lost
	WORD                     m_remoteSyncFlag;                   // sync lost
	WORD                     m_localSyncFlag;                    // sync lost
	BYTE                     m_termNum;                          // numbering
	BYTE                     m_mcuNum;                           // numbering
	WORD                     m_changeModeReq;
	WORD                     m_isBitRateChange;
	WORD                     m_isAudioChange;
	WORD                     m_isEndInitCom;
	WORD                     m_isCapExchangeEnd;
	WORD                     m_isNoCapsAtEndInitCom;
	WORD                     m_isZeroMode; // need zero mode design from v.9 (mgc) update with eitan
	WORD                     m_zero_mode_intra_req_counter;
	CAudMode*                m_pAudModeRsrv;
	WORD                     m_changeModeType;
	WORD                     m_NumOfAudioChanges;
	WORD                     m_waitForH263H264Caps;

	BOOL                     m_isAudioRcvChnlOpened;
	BOOL                     m_isAudioXmitChnlOpened;
	eContentState            m_eContentInState;
	eContentState            m_eContentOutState;
	// audio negotiation flags
	BOOL                     m_isFullAudioCapsReceived;
	BOOL                     m_isAudioOpened;
	BOOL                     m_isEncryptionNegotiationEnded;
	BYTE                     m_isRemoteMcu;
	std::map<MediaKey, bool> m_ackStatusTable;

	PDECLAR_MESSAGE_MAP
};

#endif // _MUX_CNTL_H_

