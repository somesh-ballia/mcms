#ifndef __SIP_INTERNALS__
#define __SIP_INTERNALS__

#include "SipUtils.h"
#include "AllocateStructs.h"

class CConf;
class CSipNetSetup;
class CAudioBridgeInterface;
class CVideoBridgeInterface;
class CFECCBridge;
class CConfAppMngrInterface;
class CTerminalNumberingManager;
class CCopVideoTxModes;

typedef struct {

	CIpComMode*		pScm;
	DWORD			videoRate;
	CQoS*			pQos;
	DWORD			redialInterval;
	DWORD			connectDelay;
	CSipCaps*		pCap;
	//CCapH263*			pH263Cap;

} stSIPAddPartyReconnectParam;


typedef enum
{
	kNoResponsibility,
	kLobby,
	kConf
}EResponsibility;


typedef struct {
    CConf*			pConf;
    DWORD			sysConfId;
    DWORD			sysPartyId;
    const char*		strPartyName;
    CTaskApp*		pParty;
    CSipNetSetup*	pNetSetup;
    WORD			serviceId;
    ETipPartyTypeAndPosition   partyType;
	CIpComMode*		pScm;
	CSipCaps*		pCap;
    COsQueue*		pPartyRcvMbx;
    DWORD			MstrRoomId;


} stSIPSlavePartyCreateParam;


// ConfParty Sip Util functions
void  SDPRateAlignment(sipSdpAndHeadersSt* sdp);
DWORD SipGetVideoFlowControlRateFix(DWORD wVideoRate);
enMediaDirection GetMediaDirectionAttribute(sipSdpAndHeadersSt *pSdpAndHeaders, cmCapDataType eMediaType, ERoleLabel eRole = kRolePeople);
BYTE SetTheDirectionAttribute(sipSdpAndHeadersSt *pSettingSdpAndHeaders, BYTE eAudioDirection, BYTE eVideoDirection, BYTE eDataDirection, BYTE eContentDirection, BYTE eBfcpDirection, BYTE bForceUnmute = FALSE);
BYTE SetMediaDirectionAttribute(sipSdpAndHeadersSt *pSettingSdpAndHeaders, BYTE eInfoSourceMediaDirection, cmCapDataType eMediaType, BYTE bForceUnmute, ERoleLabel eRole = kRolePeople);
void SetMediaLineDirectionAttribute(sipMediaLineSt *pMediaLine, cmCapDirection eDirection);
void SetDirectionAttributesForAVMCU(sipSdpAndHeadersSt *pSettingSdpAndHeaders);
void SetBfcpInSipPartyScm(CIpComMode* pPartyScm,BOOL IsOfferer = TRUE, enTransportType transportType = eUnknownTransportType);
void DumpMediaIp(CObjString* pMsgStr,const mcTransportAddress & mediaIpSip);

#endif


