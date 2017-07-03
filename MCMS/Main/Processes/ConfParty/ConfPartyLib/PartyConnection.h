#ifndef _PARTYCONNECTION_H__
#define _PARTYCONNECTION_H__

#include "PartyCntl.h"
#include "StateMachine.h"
#include "NetSetup.h"
#include "Conf.h"
#include "H323PartyControl.h"
#include "MoveParams.h"
#include "SIPInternals.h"
#include "H320Caps.h"
#include "H320ComMode.h"
#include "ConfPartyDefines.h"
#include "CopVideoTxModes.h"

class  CConf;
class  CPartyCntl;
class  CAudioBridgeInterface;
class  CComModeH323;
class  CCapH323;
class  CH323NetSetup;
class  CH323ExportPartyCntl;
class  CTerminalNumberingManager;
class  CFECCBridge;
class  CContentBridge;

class CPartyConnection : public CStateMachine
{
	CLASS_TYPE_1(CPartyConnection, CStateMachine)
	friend class   CTestPartyList;
	friend WORD    operator==(const CPartyConnection& first, const CPartyConnection& second) { return 0; }

public:
	               CPartyConnection();
	virtual       ~CPartyConnection();

	const char*    NameOf() const             { return "CPartyConnection"; }
	virtual void*  GetMessageMap()            { return (void*)m_msgEntries; }

	virtual void   HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	void           Destroy();

	// PSTN  functions
	void           ConnectPSTN(CConf* pConf, CNetSetup* pNetSetUp, COsQueue* pConfRcvMbx, CAudioBridgeInterface* pAudioBridgeInterface,
	                   CConfAppMngrInterface* pConfAppMngrInterface, COsQueue* pPartyRcvMbx, CTaskApp* pParty, WORD termNum,
	                   const char* telNum, WORD type, const char* partyName,
	                   const char* confName, const char* password, ConfMonitorID monitorConfId,
	                   PartyMonitorID monitorPartyId, ENetworkType networkType, BYTE voice = 0, BYTE audioVolume = 5,
	                   const char* service_provider = NULL, WORD stby = 0, WORD connectDelay = 0,
	                   const char* AV_service_name = NULL, WORD welcome_msg_time = 0,
	                   BYTE IsRecording = NO);
	void           ReconnectPstn(const char* confName, const char* password, COsQueue* pConfRcvMbx,
	                   WORD termNum, WORD WelcomMode, BYTE IsRecording = NO, WORD redialInterval = 0);
	void           ChangePSTNScm();
	void           ImportPSTN(CMoveIPImportParams* pMoveImportParams);
	virtual void   DisconnectPstn(WORD mode = 0, DWORD disconnectionDelay = 0, BOOL isViolent = FALSE, DWORD taskId = 0);

	// ISDN  functions
	void           ConnectIsdn(CConf* pConf, CIsdnNetSetup* pNetSetUp, CCapH320* pCap, CComMode* pScm,
	                   CComMode* pTransmitScm, COsQueue* pConfRcvMbx, CAudioBridgeInterface* pAudioBridgeInterface,
	                   CVideoBridgeInterface* pVideoBridgeInterface, CConfAppMngrInterface* pConfAppMngrInterface,
	                   CFECCBridge* pFECCBridge, CContentBridge* pContentBridge, CTerminalNumberingManager* pTerminalNumberingManager,
	                   COsQueue* pPartyRcvMbx, CTaskApp* pParty, WORD termNum, BYTE chnlWidth, WORD numChnl, WORD type,
	                   const char* partyName, const char* confName, ConfMonitorID monitorConfId, PartyMonitorID monitorPartyId,
	                   const char* serviceName, ENetworkType networkType,  WORD nodeType, BYTE voice, WORD stby, DWORD connectDelay,
	                   eTelePresencePartyType eTelePresenceMode = eTelePresencePartyNone, eSubCPtype bySubCPtype = eSubCPtypeClassic, WORD isUndefParty = NO);

	void           ReconnectIsdn(const char* confName, CCapH320* pCap, CComMode* pScm,
	                   CComMode* pTransmitScm, COsQueue* pConfRcvMbx,
	                   WORD termNum, BYTE isRecording, WORD redialInterval, WORD connectDelay);

	void           ChangeIsdnScm(CComMode* pTargetTransmitScm, CComMode* pTargetReceiveScm);
	void           ImportISDN(CMoveIPImportParams* pMoveImportParams);
	void           DisconnectIsdn(WORD mode = 0, DWORD disconnectionDelay = 0, BOOL isViolent = FALSE, DWORD taskId = 0);
	void           AddPartyChannel(CIsdnNetSetup& netSetUp, WORD channelNum);

	// H323  functions
	void           ConnectIP(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams);
	void           ReconnectH323(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams);
	void           ChangeH323Scm(CComModeH323* pH323Scm, EChangeMediaType eChangeMediaType);
	void           ImportIp(CMoveIPImportParams* pMoveImportParams, BOOL  isCSSPlugin = FALSE);
	virtual void   DisconnectH323(WORD mode = 0, WORD isSetRedailToZero = 0, DWORD disconnectionDelay = 0, BOOL isViolent = FALSE, DWORD taskId = 0);
	BYTE           ContinueAddPartyAfterMoveIfNeeded();
	BYTE           ProceedToChangeContentMode();

	// SIP  functions
	void           ReconnectSip(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams);
	void           ChangeSipScm(CIpComMode* pScm, BYTE IsAsSipContentEnable);
	virtual void   DisconnectSip(WORD mode = 0, WORD cause = 0, const char* alternativeAddrStr = NULL, DWORD disconnectionDelay = 0, BOOL isViolent = FALSE, DWORD taskId = 0);

	void           ConnectAVMCUIP(CSipNetSetup* pNetSetup, PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams, sipSdpAndHeaders* pSdpAndHeaders);
	void           ConnectDMAAVMCUIP(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams, const char* FocusUri);
	void           Export(COsQueue* pDestRcvMbx, void* pConfParty, ConfMonitorID destConfId, PartyMonitorID destPartyId, EMoveType eCurMoveType = eMoveDefault);

	void           SetParty(CTaskApp* pParty)          { m_pPartyCntl->SetParty(pParty); }
	void           SetPartyRsrcId(PartyRsrcID partyId) { m_pPartyCntl->SetPartyRsrcId(partyId); }
	void           SetPartyName(char* name)            { m_pPartyCntl->SetPartyName(name); }
	CTaskApp*      GetPartyTaskApp() const             { return m_pPartyCntl->GetPartyTaskApp(); }
	WORD           IsDisconnect() const                { return m_pPartyCntl->IsDisconnect(); }
	BYTE           IsTaskAppIsValid()                  { return ((m_pPartyCntl->GetMonitorPartyId() + 100) != (DWORD)m_pPartyCntl->GetPartyTaskApp()) ? TRUE : FALSE; }
	PartyRsrcID    GetPartyRsrcId() const              { return m_pPartyCntl->GetPartyRsrcId(); }
	WORD           ArePartyResourcesAllocated() const  { return m_pPartyCntl->ArePartyResourcesAllocated(); }
	WORD           GetDisconnectMode()                 { return m_pPartyCntl->GetDisconnectMode(); }
	WORD           GetDialType()                       { return m_pPartyCntl->GetDialType(); }
	WORD           IsDisconnectState() const           { return m_pPartyCntl->IsDisconnectState(); }
	WORD           GetVoice() const                    { return m_pPartyCntl->GetVoice(); }
	CPartyCntl*    GetPartyCntl()                      { return m_pPartyCntl; }
	CPartyApi*     GetPartyApi()                       { return m_pPartyCntl->GetPartyApi(); }

	CIpComMode*    GetCurrentIpScm();
	CIpComMode*    GetInitialIpScm();
	void           StartCGContent();          // for Call Generator
	void           StopCGContent();           // for Call Generator

	CComMode*      GetIsdnTargetTransmitScm();
	CComMode*      GetIsdnTargetReceiveScm();

	CNetSetup*     GetNetSetUp()                       { return m_pPartyCntl->GetNetSetUp(); }
	PartyMonitorID GetMonitorPartyId()                 { return m_pPartyCntl->GetMonitorPartyId(); }
	ConfMonitorID  GetMonitorConfId()                  { return m_pPartyCntl->GetMonitorConfId(); }
	const char*    GetName() const                     { return m_pPartyCntl->GetName(); }
	const char*    GetFullName() const                 { return m_pPartyCntl->GetFullName(); }
	WORD           GetInterfaceType() const            { return m_pPartyCntl->GetInterfaceType(); }
	DWORD          GetConnectionDelayTime() const      { return m_pPartyCntl->GetConnetDelay(); }
	DWORD          GetPartyControlState() const        { return m_pPartyCntl->GetState(); }
	WORD           IsDisconnectDelay() const           { return m_pPartyCntl->GetIsDisconnectDelay(); }

	void           ChangeSipfromIceToNoneIce(PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams);
	void           ChangeSipfromTipToNonTip(CIpComMode* pScm, PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams);
	void           SetTimerForAsSipAddContentIfNeeded();
	void           AddAsSipContenttout(CSegment* pParam);
	void           UpdateAVMCUPartyToStartMedia();

protected:
	CPartyCntl*    m_pPartyCntl;

	PDECLAR_MESSAGE_MAP
};


#endif //_PARTYCONNECTION_H__
