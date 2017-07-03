/*
 * MSAvMCUMngr.h
 *
 *  Created on: Jul 28, 2013
 *      Author: inga
 */


#ifndef MSAVMCUMNGR_H_
#define MSAVMCUMNGR_H_

#include "StateMachine.h"
#include "IpPartyControl.h"
#include "CsInterface.h"
#include "ConfPartyRoutingTable.h"
#include "MplMcmsStructs.h"
#include "Conf.h"
#include "ConfApi.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "IPUtils.h"
#include "IpServiceListManager.h"
#include "IpCommon.h"

// SIP transaction states for state-machine:
const WORD sMS_CONNECTING    = 100;
const WORD sMS_CONNECTED     = 101;
const WORD sMS_DISCONNECTING = 102;
const WORD sMS_DISCONNECTED  = 103;

////////////////////////////////////////////////////////////////////////////
//                        CMSAvMCUMngr
////////////////////////////////////////////////////////////////////////////

#define PLCM_CALL_ID_HEADER "Plcm-Call-ID"
#define ON_BEHALF_MS_HEADER "p-session-on-behalf-of: sip:6464@dma200200.dev13.std"
#define DMA_HEADER_BEHALF_MS_HEADER "Supported: ms-early-media,replaces,ms-safe-transfer,ms-conf-invite,histinfo,plcm-ivr-service-provider,timer,100rel,resource-priority,ms-sender,tdialog"
class CMSAvMCUMngr : public CStateMachine
{
public:
	CMSAvMCUMngr();
	virtual ~CMSAvMCUMngr();
	CMSAvMCUMngr(const CMSAvMCUMngr& other);

	// Initializations and settings:
	const char*   NameOf() const { return "CMSAvMCUMngr"; }
	virtual void* GetMessageMap() {return (void*)m_msgEntries;}

	virtual void  Create(CRsrcParams* pRsrcParams, CConf* pConf, CSipNetSetup* SipNetSetup, DWORD ServiceId, DWORD PartyId);
	void          SendSIPMsgToCS(DWORD opcode, void* pMsg, size_t size);
	DWORD         SetDialOutSessionTimerHeaders(CSipHeaderList& headerList);
	void          BuildAVMCUAddress(const char* SrcPartyAddress);
	void          GetOutboundSipProxy(char* pProxyAddress);
	STATUS        BuildToAddress(DWORD partyId);
	void          RemoveFromRsrcTbl();
	void 		  OnSipByeInd(CSegment* pParam);

protected:
	void          SetFocusUri(const char* focusUri);

protected:
	APIU32        m_pDestUnitId;
	APIU32        m_callIndex;
	DWORD         m_serviceId;

	CRsrcParams*  m_pCsRsrcDesc;
	CCsInterface* m_pCsInterface;

	CConfApi*     m_pTaskApi;
	DWORD         m_PartyId;

	char*         m_FocusUri;
	char*         m_ToAddrStr;
	char          m_strAVMCUAddress[MaxAddressListSize];

	PDECLAR_MESSAGE_MAP
};


#endif /* MSAVMCUMNGR_H_ */
