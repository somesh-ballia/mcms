#ifndef _CONTENT_BRIDGE_H_
	#define _CONTENT_BRIDGE_H_

#include "Bridge.h"
#include "BridgeInitParams.h"
#include "ContentBridgeInitParams.h"
#include "ContentToken.h"
#include "ContentBridgePartyCntl.h"
#include "BridgePartyDisconnectParams.h"
#include "Party.h"
#include "ContentBridgePartyInitParams.h"
#include "IntraSuppression.h"

#define DISABLE_CONTENT_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TOUT 12*SECOND
//#define CONTENT_LOAD

class CContentBridgePartyCntl;
class CContentBridgeSlave;

////////////////////////////////////////////////////////////////////////////
//                        CContentBridge
////////////////////////////////////////////////////////////////////////////
class CContentBridge : public CBridge
{
	CLASS_TYPE_1(CContentBridge, CBridge)

public:
	// States definition
	enum STATE { CONNECT_INACTIVE = (IDLE + 1), CONNECT, CHANGERATE, CONTENT, DISCONNECTING };

	virtual const char* NameOf() const { return "CContentBridge"; }

	                    CContentBridge();
	virtual            ~CContentBridge();
	CContentBridge&     operator=(const CContentBridge& rOtherContentBridge);

	virtual void        Destroy();
	virtual void        HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	virtual void*       GetMessageMap();

	void                DisconnectParty(const CTaskApp* pParty);
	void                DisconnectParty(const CBridgePartyDisconnectParams* pBridgePartyDisconnectParams);

	virtual void        CreateInActive(const CContentBridgeInitParams* pContentInitParams);
	void                Create(const CContentBridge* pOtherBridge);
	virtual void        Create(const CContentBridgeInitParams* pContentInitParams);
	virtual void        ContentRate(const BYTE newContentRate);
	virtual void        ContentProtocol(const BYTE newContentProtocol, const BYTE isH264HighProfile = FALSE);  //HP content
	virtual BYTE        AmISlaveBridge()                       { return NO; }
	virtual BYTE        AmIMasterBridge()                      { return NO; }
	BYTE                HaveIAnotherLink(CContentBridgePartyCntl* pLinkParty); // have to create cascade bridge so this function in basic will return always NO
	void                SendMessagesToRVGW(WORD opcode, BYTE McuNum, BYTE TermNum, BYTE sendToSpeaker = FALSE);

	// Utils
	BYTE                IsValidContentRate(const BYTE rate) const;
	BYTE                IsValidContentProtocol(const BYTE protocol) const;
	static char*        GetContentRateAsString(const BYTE contentRate);
	char*               GetContentProtocolAsString(const BYTE contentProtocol) const;
	DWORD               GetContentRate(BYTE contentRate) const;
	BYTE                IsSpeakerHWReadyToContent()            { return m_isSpeakerHWReadyToContent; }
	void                SetSpeakerHWReadyToContent(BYTE yesNo) { m_isSpeakerHWReadyToContent = yesNo; }
	BYTE                IsExclusiveContentMode() const         { return m_isExclusiveContentMode; }
	void                SetExclusiveContentMode(BYTE yesNo)    { m_isExclusiveContentMode = yesNo; }
	BYTE                IsTokenHolder(const CTaskApp* pParty) const;
	const CTaskApp*     GetTokenHolder(BYTE& mcuNum, BYTE& terminalNum);
	BYTE                IsPartyWaitForRateChange(const CTaskApp* pParty);
	CTaskApp*           GetPartyIdFromMcuAndTerminalNum(BYTE mcuNum, BYTE terminalNum);
	void                AskSpeakerForContentIntra();
	BYTE                IsTokenHeld();
	virtual void        UpdateConfLectureMode();
	virtual void        ContentTokenWithdraw(BYTE isImmediate = 0);
	BYTE                IsConfRestrictedContentToken(CTaskApp* pParty);               // Restricted content
	void                OnConfUpdateExclusiveContent();                               // Restricted content
	void                OnConfUpdateExclusiveContentContent(CSegment* pParam);        // Restricted content
	void                OnConfUpdateExclusiveContentChangeRate(CSegment* pParam);     // Restricted content
	void                OnConfUpdateExclusiveContentConnect(CSegment* pParam);        // Restricted content
	void                UpdateExclusiveContent();                                     // Restricted content
	void                OnConfUpdateExclusiveContentMode(CSegment* pParam);           // Restricted content
	void                OnConfUpdateExclusiveContentModeContent(CSegment* pParam);    // Restricted content
	void                OnConfUpdateExclusiveContentModeChangeRate(CSegment* pParam); // Restricted content
	void                OnConfUpdateExclusiveContentModeConnect(CSegment* pParam);    // Restricted content
	void                UpdateExclusiveContentMode(BOOL yesNo);                       // Restricted content
	virtual void        SendMessagesToLinks(WORD opcode, BYTE McuNum, BYTE TermNum, BYTE sendToSpeaker = YES) { }

	void                OnTimerCascadeSlaveLinkIntraSuppressedChangeRate(CSegment* pParams);
	void                OnTimerCascadeSlaveLinkIntraSuppressedContent(CSegment* pParams);
	void                OnTimerCascadeSlaveLinkIntraSuppressedNoContent(CSegment* pParams);
	void                OnTimerCascadeSlaveLinkIntraSuppressed(CSegment* pParams);
	CBridgePartyList*   GetCascadeLinkSlavePartyList() const   { return m_pCascadeSlavePartiesList; }
	virtual void        AbortPresentation();

protected:
	// Utils: for internal use
	virtual void        IsRateChangeDone();
	virtual void        ChangePresentationRate();
	virtual void        ChangePresentationSpeaker();
	virtual WORD        GetSpeakerProviderIdentity(BYTE& mcuNum, BYTE& termNum);
	virtual void        BroadcastRoleProviderIdentityMsg(BYTE mcuNum, BYTE terminalNum, BYTE label, BYTE dataSize, BYTE* pData);
	virtual void        BroadcastRoleProviderIdentityForNewTokenHolder(BYTE mcuNum, BYTE terminalNum, BYTE label, BYTE dataSize, BYTE* pData);

	void                OnEndPartyConnect(CContentBridgePartyCntl* pPartyCntl, WORD& status);
	void                BroadcastMediaProducerStatus(BYTE channelID, EMediaProducerStatus status);
	virtual BYTE        IsConfLectureModeAllowTokenAcquire(CTaskApp* pParty);
	virtual void        OnConfUpdateLectureModeConnect(CSegment* pParam);
	virtual void        OnConfUpdateLectureModeChangeRate(CSegment* pParam);
	virtual void        OnConfUpdateLectureModeContent(CSegment* pParam);
	virtual void        OnConfUpdateLectureMode(BYTE yesNo);

	// ACTION FUNCTIONS :
	// disconnect event from CONF
	virtual void        OnConfDisConnectConfCONNECT(CSegment* pParam);
	virtual void        OnConfDisConnectConfCHANGERATE(CSegment* pParam);
	virtual void        OnConfDisConnectConfCONTENT(CSegment* pParam);
	virtual void        OnConfDisConnectConfDISCONNECTING(CSegment* pParam);
	virtual void        OnConfDisConnectConf(CSegment* pParam);

	virtual void        OnConfTerminate(CSegment* pParam);
	virtual void        OnConfTerminateDISCONNECTING(CSegment* pParam);

	// connect party event from CONF
	virtual void        OnConfConnectPartyCONNECT(CSegment* pParam);
	virtual void        OnConfConnectPartyCHANGERATE(CSegment* pParam);
	virtual void        OnConfConnectPartyCONTENT(CSegment* pParam);
	virtual void        OnConfConnectPartyDISCONNECTING(CSegment* pParam);
	virtual void        OnPartyConnect(CSegment* pParam);

	// disconnect party event from CONF
	virtual void        OnConfDisConnectPartyCONNECT_INACTIVE(CSegment* pParam);
	virtual void        OnConfDisConnectPartyCONNECT(CSegment* pParam);
	virtual void        OnConfDisConnectPartyCHANGERATE(CSegment* pParam);
	virtual void        OnConfDisConnectPartyCONTENT(CSegment* pParam);
	virtual void        OnConfDisConnectPartyDISCONNECTING(CSegment* pParam);
	virtual void        OnPartyDisConnect(CSegment* pParam);

	// token withdraw event from CONF
	virtual void        OnConfTokenWithdrawCONNECT(CSegment* pParam);
	virtual void        OnConfTokenWithdrawCHANGERATE(CSegment* pParam);
	virtual void        OnConfTokenWithdrawCONTENT(CSegment* pParam);
	virtual void        OnConfTokenWithdrawDISCONNECTING(CSegment* pParam);
	virtual void        OnConfTokenWithdraw(CSegment* pParam);

	// new content rate from conf
	virtual void        OnConfNewRateCONNECT(CSegment* pParam);
	virtual void        OnConfNewRateCHANGERATE(CSegment* pParam);
	virtual void        OnConfNewRateCONTENT(CSegment* pParam);
	virtual void        OnConfNewRateDISCONNECTING(CSegment* pParam);

	// new content scm (Rate & Protocol) from conf
	virtual void        OnConfNewScmCONNECT(CSegment* pParam);
	virtual void        OnConfNewScmCHANGERATE(CSegment* pParam);
	virtual void        OnConfNewScmCONTENT(CSegment* pParam);
	virtual void        OnConfNewScmDISCONNECTING(CSegment* pParam);
	virtual void        OnConfNewSCM(CSegment* pParam);

	// /////////       party control         ////////////
	// end connect party from party
	virtual void        OnEndPartyConnectCONNECT_INACTIVE(CSegment* pParam);
	virtual void        OnEndPartyConnectCONNECT(CSegment* pParam);
	virtual void        OnEndPartyConnectCHANGERATE(CSegment* pParam);
	virtual void        OnEndPartyConnectCONTENT(CSegment* pParam);
	virtual void        OnEndPartyConnectDISCONNECTING(CSegment* pParam);

	// end disconnect party from party
	virtual void        OnEndPartyDisConnectCONNECT(CSegment* pParam);
	virtual void        OnEndPartyDisConnectCHANGERATE(CSegment* pParam);
	virtual void        OnEndPartyDisConnectCONTENT(CSegment* pParam);
	virtual void        OnEndPartyDisConnectDISCONNECTING(CSegment* pParam);
	virtual void        OnEndPartyDisConnect(CSegment* pParam);

	// party changed it's content rate
	virtual void        OnPartyRateChangedCONNECT(CSegment* pParam);
	virtual void        OnPartyRateChangedCHANGERATE(CSegment* pParam);
	virtual void        OnPartyRateChangedCONTENT(CSegment* pParam);
	virtual void        OnPartyRateChangedDISCONNECTING(CSegment* pParam);

	// party asks for intra
	virtual void        OnPartyRefreshContentCONNECT(CSegment* pParam);
	virtual void        OnPartyRefreshContentCHANGERATE(CSegment* pParam);
	virtual void        OnPartyRefreshContentCONTENT(CSegment* pParam);
	virtual void        OnPartyRefreshContentDISCONNECTING(CSegment* pParam);

	// HW content Stream is ready
	virtual void        OnPartyHWContentOnOffAckCONNECT(CSegment* pParam);
	virtual void        OnPartyHWContentOnOffAckCHANGERATE(CSegment* pParam);
	virtual void        OnPartyHWContentOnOffAckCONTENT(CSegment* pParam);
	virtual void        OnPartyHWContentOnOffAckDISCONNECTING(CSegment* pParam);
	virtual void        OnPartyHWContentOnOffAck(CSegment* pParam);

	// /////   PARTY task messages for TOKEN   /////////
	// ** token_acquire message
	virtual void        OnPartyTokenAcquireIDLE(CSegment* pParam);
	virtual void        OnPartyTokenAcquireCONNECT(CSegment* pParam);
	virtual void        OnPartyTokenAcquireCHANGERATE(CSegment* pParam);
	virtual void        OnPartyTokenAcquireCONTENT(CSegment* pParam);
	virtual void        OnPartyTokenAcquireDISCONNECTING(CSegment* pParam);
	virtual void        OnPartyTokenAcquire(CSegment* pParam);

	// ** token_release message
	virtual void        OnPartyTokenReleaseCONNECT(CSegment* pParam);
	virtual void        OnPartyTokenReleaseCHANGERATE(CSegment* pParam);
	virtual void        OnPartyTokenReleaseCONTENT(CSegment* pParam);
	virtual void        OnPartyTokenReleaseDISCONNECTING(CSegment* pParam);
	virtual void        OnPartyTokenRelease(CSegment* pParam);

	// ** token_withdraw_ack message
	virtual void        OnPartyTokenWithdrawAckCONNECT(CSegment* pParam);
	virtual void        OnPartyTokenWithdrawAckCHANGERATE(CSegment* pParam);
	virtual void        OnPartyTokenWithdrawAckCONTENT(CSegment* pParam);
	virtual void        OnPartyTokenWithdrawAckDISCONNECTING(CSegment* pParam);
	virtual void        OnPartyTokenWithdrawAck(CSegment* pParam);

	// role_provider_identity  message
	virtual void        OnPartyRoleProviderIdentityCONNECT(CSegment* pParam);
	virtual void        OnPartyRoleProviderIdentityCHANGERATE(CSegment* pParam);
	virtual void        OnPartyRoleProviderIdentityCONTENT(CSegment* pParam);
	virtual void        OnPartyRoleProviderIdentityDISCONNECTING(CSegment* pParam);
	virtual void        OnPartyRoleProviderIdentity(CSegment* pParam);

	// media_producer_status  message
	virtual void        OnPartyMediaProducerStatusCONNECT(CSegment* pParam);
	virtual void        OnPartyMediaProducerStatusCHANGERATE(CSegment* pParam);
	virtual void        OnPartyMediaProducerStatusCONTENT(CSegment* pParam);
	virtual void        OnPartyMediaProducerStatusDISCONNECTING(CSegment* pParam);
	virtual void        OnPartyMediaProducerStatus(CSegment* pParam);

	// ** token_query message
	virtual void        OnPartyBfcpTokenQueryIDLE(CSegment* pParam);
	virtual void        OnPartyBfcpTokenQueryCONNECT(CSegment* pParam);
	virtual void        OnPartyBfcpTokenQueryCHANGERATE(CSegment* pParam);
	virtual void        OnPartyBfcpTokenQueryCONTENT(CSegment* pParam);
	virtual void        OnPartyBfcpTokenQueryDISCONNECTING(CSegment* pParam);
	virtual void        OnPartyBfcpTokenQuery(CSegment* pParam);

	// /////    TOKEN messages for BRIDGE    /////////
	// new_token_holder message
	virtual void        OnTokenNewTokenHolderCONNECT(CSegment* pParam);
	virtual void        OnTokenNewTokenHolderCHANGERATE(CSegment* pParam);
	virtual void        OnTokenNewTokenHolderCONTENT(CSegment* pParam);
	virtual void        OnTokenNewTokenHolderDISCONNECTING(CSegment* pParam);
	// no_token_holder  message
	virtual void        OnTokenNoTokenHolderCONNECT(CSegment* pParam);
	virtual void        OnTokenNoTokenHolderCHANGERATE(CSegment* pParam);
	virtual void        OnTokenNoTokenHolderCONTENT(CSegment* pParam);
	virtual void        OnTokenNoTokenHolderDISCONNECTING(CSegment* pParam);
	void                OnTokenNoTokenHolderEnd(CSegment* pParam);

	// ** NOROLEPROVIDER token message
	virtual void        OnTokenNoProviderCONNECT(CSegment* pParam);
	virtual void        OnTokenNoProviderCHANGERATE(CSegment* pParam);
	virtual void        OnTokenNoProviderCONTENT(CSegment* pParam);
	virtual void        OnTokenNoProviderDISCONNECTING(CSegment* pParam);

	// ** Freeze content req ***
	virtual void        OnTokenFreezContentCONNECT(CSegment* pParam);
	virtual void        OnTokenFreezContentCHANGERATE(CSegment* pParam);
	virtual void        OnTokenFreezContentCONTENT(CSegment* pParam);
	virtual void        OnTokenFreezContentDISCONNECTING(CSegment* pParam);

	// /////    TIMER messages for BRIDGE    /////////
	// ** CHANGERATE timer message
	virtual void        OnTimerChangeRateCONNECT(CSegment* pParam);
	virtual void        OnTimerChangeRateCHANGERATE(CSegment* pParam);
	virtual void        OnTimerChangeRateCONTENT(CSegment* pParam);
	virtual void        OnTimerChangeRateDISCONNECTING(CSegment* pParam);

	// Cascade MIH
	virtual void        CascadePartyTokenAcquire(CSegment* pParam);
	virtual void        ResetLastContentRateFromMaster();

	void                AllocateLinkPartyList();
	BYTE                IsXCodeConf();

	// ignore intra filtering after start content
	void                IgnoreContentIntraFilteringAfterStartContent();
	void                OnTimerDisableIntraAfterStartContent(CSegment* pParam);
	void                EnableIntraSuppress();
	void                DisableIntraSuppress();

	void 				OnTokenNewTokenHolderContinue01(CSegment* pParam);
	void 				OnTokenNewTokenHolderContinue02(CSegment* pParam);
	void 				OnTokenNewTokenHolderContinue03(CSegment* pParam);

public:
	CContentToken*      m_pToken; // temporary - if protected,from some reason derived class (slave)cannot update it
	WORD                m_byCurrentContentRate;
	BYTE                m_byCurrentContentProtocol;
	BYTE			m_byH264HighProfile;  //HP content

protected:
	BYTE                m_isSpeakerHWReadyToContent;
	BYTE                m_isExclusiveContentMode;
	bool                m_isIntraSupressionEnabled;
	CBridgePartyList*   m_pCascadeSlavePartiesList;
	BOOL                m_bCascadeSlaveIntraRequestsPending;

	BOOL                m_bIsRcvArtContentOn;


	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CContentBridgeSlave
////////////////////////////////////////////////////////////////////////////
class CContentBridgeSlave : public CContentBridge
{
	CLASS_TYPE_1(CContentBridgeSlave, CContentBridge)

public:
	                    CContentBridgeSlave();
	virtual            ~CContentBridgeSlave ();

	virtual const char* NameOf() const    { return "CContentBridgeSlave"; }
	virtual void        Create(const CContentBridgeInitParams* pContentInitParams);
	virtual void        Create(const CContentBridge* pContentBridge);
	virtual BYTE        AmISlaveBridge()  { return YES; }
	virtual BYTE        AmIMasterBridge() { return NO; }

	virtual void        IsRateChangeDone();
	void                MasterRateChange(CSegment* pParam);
	void                InformMasterOnRateChangeDone();

	void                OnMasterStartContentCONNECT_INACTIVE(CSegment* pParam);
	void                OnMasterStartContentCONNECT(CSegment* pParam);
	void                OnMasterStartContentCHANGERATE(CSegment* pParam);
	void                OnMasterStartContentCONTENT(CSegment* pParam);
	void                OnMasterStartContentDISCONNECTING(CSegment* pParam);
	void                OnMasterRateChangeCONNECT_INACTIVE(CSegment* pParam);
	void                OnMasterRateChangeCONNECT(CSegment* pParam);
	void                OnMasterRateChangeCHANGERATE(CSegment* pParam);
	void                OnMasterRateChangeCONTENT(CSegment* pParam);
	void                OnMasterRateChangeDISCONNECTING(CSegment* pParam);
	void                OnMasterTokenWithdrawCONNECT(CSegment* pParam);
	void                OnMasterTokenWithdrawCHANGERATE(CSegment* pParam);
	void                OnMasterTokenWithdrawCONTENT(CSegment* pParam);
	void                OnMasterTokenWithdrawDISCONNECTING(CSegment* pParam);
	void                OnPartyTokenAcquireAckCONNECT(CSegment* pParam);
	void                OnPartyTokenAcquireAckCONTENT(CSegment* pParam);
	void                OnPartyTokenAcquireAck(CSegment* pParam);
	void                OnPartyTokenAcquireNakCONNECT(CSegment* pParam);
	void                OnConfTokenWithdrawAnyCase(CSegment* pParam);
	virtual void        OnPartyHWContentOnOffAckCONNECT(CSegment* pParam);
	virtual void        OnPartyHWContentOnOffAckCHANGERATE(CSegment* pParam);
	virtual void        OnPartyHWContentOnOffAckCONTECT(CSegment* pParam);
	virtual void        OnTokenNewTokenHolderCONNECT(CSegment* pParam);
	virtual void        OnTokenNewTokenHolderCHANGERATE(CSegment* pParam);
	virtual void        OnPartyTokenAcquireIDLE(CSegment* pParam);
	virtual void        OnPartyTokenAcquireCONNECT(CSegment* pParam);
	virtual void        OnPartyTokenAcquireCHANGERATE(CSegment* pParam);
	virtual void        OnPartyTokenAcquireCONTENT(CSegment* pParam);
	virtual void        OnPartyTokenAcquire(CSegment* pParam);
	virtual void        SendMessagesToLinks(WORD opcode, BYTE McuNum, BYTE TermNum, BYTE sendToSpeaker = YES);

	virtual void        OnConfConnectPartyCONNECT(CSegment* pParam);
	virtual void        OnConfConnectPartyCHANGERATE(CSegment* pParam);
	virtual void        OnConfConnectPartyCONTENT(CSegment* pParam);

	void                OnPartyConnect(CSegment* pParam, BYTE isConnectWhileContent);

	void                OnForwardTokenMsgToMasterCONNECT(CSegment* pParam);
	void                OnForwardTokenMsgToMasterCONTENT(CSegment* pParam);

	void                ResetLastContentRateFromMaster();

protected:
	CContentBridgePartyCntl* m_pLinkToMasterPartyCntl;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CContentBridgeMaster
////////////////////////////////////////////////////////////////////////////
class CContentBridgeMaster : public CContentBridge
{
	CLASS_TYPE_1(CContentBridgeMaster, CContentBridge)

public:
	                    CContentBridgeMaster();
	virtual            ~CContentBridgeMaster ();

	virtual const char* NameOf() const    { return "CContentBridgeMaster"; }

	void                Create(const CContentBridge* pOtherBridge);
	virtual BYTE        AmISlaveBridge()  { return NO; }
	virtual BYTE        AmIMasterBridge() { return YES; }
	virtual void        SendMessagesToLinks(WORD opcode, BYTE McuNum, BYTE TermNum, BYTE sendToSpeaker = YES);

protected:

	PDECLAR_MESSAGE_MAP
};

#endif // ifndef _CONTENT_BRIDGE_H_
