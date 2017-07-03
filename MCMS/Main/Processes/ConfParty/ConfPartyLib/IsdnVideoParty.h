#ifndef _ISDN_VIDEO_PARTY_H
#define _ISDN_VIDEO_PARTY_H

#include "Macros.h"
#include "IsdnParty.h"
#include "BondingCntl.h"
#include "MuxCntl.h"
#include "ConfPartyOpcodes.h"
#include "H320ComMode.h"
#include "TokenMsgMngr.h"
#include "BondingMuxCntl.h"
#include "PartyRsrcDesc.h"
#include "CommConfDB.h"
#include "ConfigManagerApi.h"

typedef enum
{
	eNONE                 = 0,
	eCLASSIC              = 1,
	eSINGLE_PORT          = 2,
	eDUAL_PORT            = 3,
	eLAST_VIDEO_PLUS_TYPE = 4 // for enum checking
} VIDEO_PLUS_TYPE;


struct H230_ENTRY
{
	BYTE  opcode;
	AFUNC actFunc;
	char* opcodeStr;
	char* descStr;
};

#define ONH230(opcode, actFunc, opcodestr, descstr) { opcode, FUNC actFunc, opcodestr, descstr },
#define BEGIN_H230_TBL H230_ENTRY CIsdnVideoParty::m_H230Entries[] = {
#define END__H230_TBL  { 0, 0, "UNKNOWN", "UNKNOWN" } };

class CSegment;
class CBondingMuxCntl;

////////////////////////////////////////////////////////////////////////////
//                        CIsdnVideoParty
////////////////////////////////////////////////////////////////////////////
class CIsdnVideoParty : public CIsdnParty
{
	CLASS_TYPE_1(CIsdnVideoParty, CIsdnParty)

public:
	                    CIsdnVideoParty();
	virtual            ~CIsdnVideoParty();

	virtual const char* NameOf()  const { return "CIsdnVideoParty"; }
	virtual void        PartySpecfiedIvrDelay();

	void                OnH230AudioMute(CSegment* pParam);
	void                OnH230AudioActive(CSegment* pParam);
	void                OnH230AudioEqualize(CSegment* pParam);
	void                OnH230AudioZeroDelay(CSegment* pParam);
	void                OnH230VideoIndSuppressed(CSegment* pParam);
	void                OnH230VideoActive(CSegment* pParam);
	void                OnH230ChairCntlCap(CSegment* pParam);
	void                OnH230ChairCmdAcquire(CSegment* pParam);
	void                OnH230ChairStopToken(CSegment* pParam);
	void                OnH230ChairIndRefuse(CSegment* pParam);
	void                OnH230ChairIndToken(CSegment* pParam);
	void                OnH230ChairReleaseToken(CSegment* pParam);
	void                OnH230ChairDropTerm(CSegment* pParam);
	void                OnH230ChairDropConf(CSegment* pParam);
	void                OnH230FloorReq(CSegment* pParam);
	void                OnH230IndicateEndOfString(CSegment* pParam);
	void                OnH230UpdateTerminalList(CSegment* pParam);
	void                OnH230AssignTerminaNum(CSegment* pParam);
	void                OnH230IndTerminaNum(CSegment* pParam);
	void                OnH230DropTerminaNum(CSegment* pParam);
	void                OnH230ChairVideoBroadcast(CSegment* pParam);
	void                OnH230ChairCancelVideoBroadcast(CSegment* pParam);
	void                OnH230TokenCommandAssociation(CSegment* pParam);
	void                OnH230LsdAcquireToken(CSegment* pParam);
	void                OnH230HsdAcquireToken(CSegment* pParam);
	void                OnH230LsdIndicateToken(CSegment* pParam);
	void                OnH230HsdIndicateToken(CSegment* pParam);
	void                OnH230LsdStopToken(CSegment* pParam);
	void                OnH230HsdStopToken(CSegment* pParam);
	void                OnH230LsdReleaseToken(CSegment* pParam);
	void                OnH230HsdReleaseToken(CSegment* pParam);
	void                OnH230LsdCloseChannel(CSegment* pParam);
	void                OnH230TerminalIdentify(CSegment* pParam);
	void                OnH230TerminalIndIdentity(CSegment* pParam);
	void                OnH230TerminalIndIdentityStop(CSegment* pParam);

	void                OnH230TerminalPassword(CSegment* pParam);
	void                OnH230TerminalIdentity(CSegment* pParam);
	void                OnH230TerminalConfIdentify(CSegment* pParam);
	void                OnH230TerminalPersonaldentify(CSegment* pParam);
	void                OnH230TerminalExtensionAddress(CSegment* pParam);

	void                OnH230MultiCmdVisualize(CSegment* pParam);
	void                OnH230MultiCancelCmdVisualize(CSegment* pParam);
	void                OnH230MultiIndVisualize(CSegment* pParam);
	void                OnH230MultiCancelIndVisualize(CSegment* pParam);
	void                OnH230VideoCmdSelect(CSegment* pParam);
	void                OnH230CancelVideoCmdSelect(CSegment* pParam);
	void                OnH230MultiIndLopp(CSegment* pParam);
	void                OnH230MultiIndMaster(CSegment* pParam);
	void                OnH230RandomNum(CSegment* pParam);
	void                OnH230IndVideoSrcNum(CSegment* pParam);
	void                OnH230ZeroCom(CSegment* pParam);
	void                OnH230SecondaryStatus(CSegment* pParam);
	void                OnH230CancelZeroCom(CSegment* pParam);
	void                OnH230CancelSecondaryStatus(CSegment* pParam);
	void                OnH230CancelMultiIndMaster(CSegment* pParam);
	void                OnH230MlpData(CSegment* pParam);
	void                OnH230MultiCmdSymetric(CSegment* pParam);
	void                OnH230MultiCmdConf(CSegment* pParam);
	void                OnH230MultiIndicateHierarchy(CSegment* pParam);
	void                GetMcuTerminalNum(CSegment* pParam, BYTE& mcu, BYTE& terminal);

	void                OnBondAlignedSetup(CSegment* pParam);             // rons
	void                SetNetSetup(CIsdnPartyRsrcDesc& pPartyRsrcDesc);  // rons

	// H239
	void                OnMuxH239Message(BYTE msgLen, CSegment* pParam);
	void                OnMuxAMC_CI(BYTE msgLen, CSegment* pParam);
	void                OnContentMsgFromMaster(CSegment* pParam);
	void                OnContentBrdgAmcVideoMode(CSegment* pParam);
	void 				OnContentBrdgAmcVideoPartySetup(CSegment* pParam);
	void                OnContentBrdgAmcLogicalChannelInactive(CSegment* pParam);
	void                OnContentBrdgAmcMCS(CSegment* pParam);
	void                OnPartyUpdatedPresentationOutStream(CSegment* pParam);
	void                OnPartyCntlUpdatePresentationOutStream(CSegment* pParam);
	void                SpreadAllH239Msgs(CSegment* pParam, EMsgDirection direction, WORD isOldTokenMsg, DWORD Opcode, EMsgStatus msgStat = eMsgFree);
	void                HandleTMMList(CTokenMsgMngr* tokenMsgList);
	void                OnMuxAckForContentOnOff(CSegment* pParam);
	void                OnMuxAckForContentOnOffWhileDisconnecting(CSegment* pParam);
	void                OnMuxAckForEvacuateContentStream(CSegment* pParam);
	void                OnContentBrdgTokenMsg(CSegment* pParam);
	DWORD               TranslateH239OpcodeToPPCOpcode(BYTE H239Opcode, BYTE Ack_Nak);
	EHwStreamState      SetCorrectStreamStateForTMM();
	void                SendContentOnOff();
	void                DisconnectPartyDueToProblemsWithH239Stream();
	void                HandleProblemsDuringClosingContentStream();
	void                OnMuxAckForEvacuateTout(CSegment* pParam);
	BYTE                IsPartyLinkToSlave();
	BYTE                IsPartyLinkToMaster();
	void                SetIsSlaveFinishCM(BYTE bYesNo){m_isSlaveFinishCM = bYesNo;}

	void                OnConfDisconnectAnyState(CSegment* pParam); // rons

	WORD                FilterAndSendVideoRefresh();
	void                OnSmartRecovery();
	void                OnVideoReadyForIvr(CSegment* pParam);

	void                OnPartyCntlSendMMSOnOff(CSegment* pParam);
	void                OnPartyCntlSendMCCOnOff(CSegment* pParam);
	std::string         GenerateCorrelationId(const char* initial_number);

protected:
	void                OnConfDelNetCntl(CSegment* pParam);
	void                OnConfChangeMode(CSegment* pParam);
	void                OnConfExchangeCap(CSegment* pParam);
	void                OnConfUpdateLocalCaps(CSegment* pParam);

	void                OnMuxEndH320ConSetUp(CSegment* pParam);
	void                OnMuxRmtXfrMode(CSegment* pParam);
	void                OnMuxSyncInitChnl(CSegment* pParam);
	void                OnMuxStartCommH230Seq(CSegment* pParam);
	void                OnMuxSendSilence(CSegment* pParam);
	void                OnMuxRmtCapSetUp(CSegment* pParam);
	void                OnMuxRmtCapConnect(CSegment* pParam);
	void                OnMuxSyncChangeMode(CSegment* pParam);
	void                OnMuxSyncConnect(CSegment* pParam);
	void                OnMuxSync(CSegment* pParam);
	void                OnMcuMediaErrConnect(CSegment* pParam);

	void                OnMuxH230SetUp(CSegment* pParam);
	void                OnMuxH230ChangeMode(CSegment* pParam);
	void                OnMuxH230Connect(CSegment* pParam);
	void                OnMuxH230(CSegment* pParam);
	void                OnMuxH230Bas(BYTE opcode);
	void                OnMuxH230Mbe(CSegment* pParam);
	void                OnMuxH230NS_Com(CSegment* pParam);
	void                OnMuxSyncTimer(CSegment* pParam);

	void                OnAudBrdgValidation(CSegment* pParam);
	void                OnVidBrdgRefresh(CSegment* pParam);
	void                OnVidBrdgH239ReCap(CSegment* pParam);
	void                OnBridgeNumberingMessage(CSegment* pParam);
	void                OnMuxReceivedPartyFullCapSet(CSegment* pParam);

	void                OnPartyUpdateConfRsrcIdForInterfaceAnycase(CSegment* pParam);
	BYTE                isChangeModeAndForContentOnly() const;

	void                OnContentBrdgFreezePic(CSegment* pParam);
	void                OnContentBrdgRefreshVideo(CSegment* pParam);

	void                OnPartyFreezePic(CSegment* pParam); // send VCF to party
	WORD                GetNumConnectedChannels();

	void                OnContentRateChangeDone(CSegment* pParam);

	static H230_ENTRY   m_H230Entries[];

	CComMode*           m_pTargetComMode;
	CComMode*           m_pCurrentComMode;
	CBondingCntl*       m_pBndCntl;
	CMuxCntl*           m_pMuxCntl;
	CBondingMuxCntl*    m_pBndMuxCntl;
	VIDEO_PLUS_TYPE     m_videoPlusType;
	CTokenMsgMngr*      m_pH239TokenMsgMngr;
	BYTE                m_delayIVR;
	DWORD               m_lastContentRateFromMasterForThisToken; // for the content coming from our side (slave)
	BYTE                m_isContentSpeakerOnSlave;
	BYTE                m_isSlaveFinishCM;

	PDECLAR_MESSAGE_MAP
};

#endif /* _ISDN_VIDEO_PARTY_H */

