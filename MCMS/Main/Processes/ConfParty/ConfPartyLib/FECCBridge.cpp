#include "FECCBridge.h"
#include "FECCBridgeInitParams.h"
#include "FECCBridgePartyInitParams.h"
#include "BridgePartyDisconnectParams.h"
#include "BridgeInitParams.h"
#include "ConfDef.h"
#include "NStream.h"
#include "Macros.h"
#include <iomanip>
#include "ConfPartyOpcodes.h"
#include "ObjString.h"
#include "ConfApi.h"
#include "Conf.h"
#include "PartyApi.h"
#include "H221.h"
#include "HostCommonDefinitions.h"
#include "Party.h"

const WORD OTHERPARTY     = 1;
const WORD CURPARTY       = 2;
const WORD UNVALID        = 4;
const WORD CLOSCHNLERETRY = 10; // retries to close data channel

PBEGIN_MESSAGE_MAP(CFECCBridge)
  ONEVENT(DISCONNECTCONF,           CONNECTED,     CFECCBridge::OnConfDisConnectConfCONNECTED)
  ONEVENT(TERMINATE,                DISCONNECTING, CFECCBridge::OnConfTerminateDISCONNECTING)

  ONEVENT(ENDRATECHANGEDATACHANNEL, CONNECTED,     CFECCBridge::OnEndRateChangeDataChannel)
  ONEVENT(UPDATELSDBITRATE,         CONNECTED,     CFECCBridge::OnConfUpdateLsdBitRateCONNECTED)

  ONEVENT(CONNECTPARTY,             CONNECTED,     CFECCBridge::OnConfConnectPartyCONNECTED)

  ONEVENT(ENDCONNECTPARTY,          CONNECTED,     CFECCBridge::OnEndPartyConnect)

  ONEVENT(DISCONNECTPARTY,          CONNECTED,     CFECCBridge::OnConfDisconnectPartyCONNECTED)
  ONEVENT(DISCONNECTPARTY,          DISCONNECTING, CFECCBridge::OnConfDisconnectPartyCONNECTED)

  ONEVENT(ENDDISCONNECTPARTY,       CONNECTED,     CFECCBridge::OnEndPartyDisConnect)
  ONEVENT(ENDDISCONNECTPARTY,       DISCONNECTING, CFECCBridge::OnEndPartyDisConnect)

  ONEVENT(DATATOKENREQ,             CONNECTED,     CFECCBridge::OnPartyDataTokenRequestCONNECTED)
  ONEVENT(DATATOKENREQ,             DISCONNECTING, CFECCBridge::OnPartyDataTokenRequestDISCONNECTING)

  ONEVENT(DATATOKENRELEASE,         CONNECTED,     CFECCBridge::OnPartyDataTokenReleaseCONNECTED)
  ONEVENT(DATATOKENRELEASE,         DISCONNECTING, CFECCBridge::OnPartyDataTokenReleaseDISCONNECTING)

  ONEVENT(DATATOKENRELEASEFREE,     CONNECTED,     CFECCBridge::OnPartyDataTokenReleaseAndFreeCONNECTED)
  ONEVENT(DATATOKENRELEASEFREE,     DISCONNECTING, CFECCBridge::NullActionFunction)

  ONEVENT(DATATOKENWITHDRAW,        CONNECTED,     CFECCBridge::OnDataTokenWithdraw)
  ONEVENT(DATATOKENWITHDRAW,        DISCONNECTING, CFECCBridge::NullActionFunction)

  ONEVENT(DATATOKENWITHDRAWANDFREE, CONNECTED,     CFECCBridge::OnDataTokenWithdrawAndFree)
  ONEVENT(DATATOKENWITHDRAWANDFREE, DISCONNECTING, CFECCBridge::NullActionFunction)
PEND_MESSAGE_MAP(CFECCBridge, CBridge);

////////////////////////////////////////////////////////////////////////////
//                        CFECCBridge
////////////////////////////////////////////////////////////////////////////
CFECCBridge::CFECCBridge()
{
  m_bitRate    = 0;
  m_tokenOwner = NULL;

  VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
void CFECCBridge::Create(const CFECCBridgeInitParams* pFECCInitParams)
{
  if (!CPObject::IsValidPObjectPtr(pFECCInitParams))
  {
    m_pConfApi->EndFECCBrdgConnect(statInconsistent);
    PASSERT_AND_RETURN(1);
  }

  CBridge::Create((CBridgeInitParams*)pFECCInitParams);

  BYTE bitRate = pFECCInitParams->GetFECCBitRate();

  switch (bitRate)
  {
    case 0xFF:      // None lsd conf.
    case LSD_Off:   // Dynamic lsd conf.
    case LSD_4800:
    case LSD_6400:
    {
      m_bitRate = bitRate;
      break;
    }

    default:
    {
      PASSERT(bitRate+1000);
      break;
    }
  }

  m_state = CONNECTED;

  m_pConfApi->EndFECCBrdgConnect(statOK);
}

//--------------------------------------------------------------------------
void CFECCBridge::EndRateChange(WORD status)
{
  CSegment* pSeg = new CSegment;
  *pSeg << status;
  DispatchEvent(ENDRATECHANGEDATACHANNEL, pSeg);
  POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CFECCBridge::UpdateLsdBitRate(BYTE bitRate)
{
  CSegment* pSeg = new CSegment;
  *pSeg << bitRate;
  DispatchEvent(UPDATELSDBITRATE, pSeg);
  POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CFECCBridge::OnConfUpdateLsdBitRateCONNECTED(CSegment* pParam)
{
  BYTE bitRate;
  *pParam >>  bitRate;

  TRACEINTO << "CFECCBridge::OnConfUpdateLsdBitRateCONNECTED - ConfName:" <<  m_pConfName << ", bitRateType:" << (int)bitRate;

  switch (bitRate)
  {
    case 0xFF:      // None lsd conf.
    {
      // Only for updating the DB.
      m_bitRate = NO;
      UpdateDataSrcId();
      m_bitRate = bitRate; // Set the proper rate 0xFF.
      break;
    }

    case LSD_Off :  // Dynamic lsd conf.
    case LSD_4800:
    case LSD_6400:
    {
      m_bitRate = bitRate;
      break;
    }

    default:
    {
      PASSERT(bitRate+1000);
      break;
    }
  }
}

//--------------------------------------------------------------------------
void CFECCBridge::OnConfDisConnectConfCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridge::OnConfDisConnectConfCONNECTED - ConfName:", m_pConfName);
  m_state = DISCONNECTING;
}

//--------------------------------------------------------------------------
void CFECCBridge::OnConfTerminateDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridge::OnConfTerminateDISCONNECTING - ConfName:", m_pConfName);

  EStat eStatus = statOK;

  m_state = IDLE;

  if (0 != GetNumParties())
  {
    // Upon receiving TERMINATE event, Bridge should be empty
    eStatus = statBridgeIsNotEmpty;
  }

  m_pConfApi->EndFECCBrdgDisConnect(eStatus);
}

//--------------------------------------------------------------------------
void CFECCBridge::OnConfConnectPartyCONNECTED(CSegment* pParam)
{
	CFECCBridgePartyInitParams partyInitParams;
	partyInitParams.DeSerialize(NATIVE, *pParam);

	if (!partyInitParams.IsValidParams())
	{
		CBridge::RejectPartyConnect(partyInitParams.GetParty(), FECC_PARTY_BRIDGE_DISCONNECTED, statInvalidPartyInitParams);
		return;
	}

	const CParty* pParty = (CParty*)partyInitParams.GetParty();
	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << partyId;

	// Avoid repeated connection of the party to the bridge
	PASSERTSTREAM_AND_RETURN(NULL != GetPartyCntl(partyId), "PartyId:" << partyId << " - Already exist in Bridge");

	CFECCBridgePartyCntl* pFECCBBrdgPartyCntl = new CFECCBridgePartyCntl();

	pFECCBBrdgPartyCntl->Create(&partyInitParams);

	// Insert the party to the PartyCtl List and activate Connect on it
	CBridge::ConnectParty(pFECCBBrdgPartyCntl);
}

//--------------------------------------------------------------------------
void CFECCBridge::OnConfDisconnectPartyCONNECTED(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID partyId = partyDisconnectParams.GetPartyId();
	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();

	TRACEINTO << "PartyId:" << partyId << ", MediaDirection:" << eMediaDirection;

	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
	PASSERT_AND_RETURN(!pParty);

	if (pParty == m_tokenOwner)
	{
		CFECCBridgePartyCntl* pPartyCntl = (CFECCBridgePartyCntl*)GetPartyCntl(partyId);
		PASSERT_AND_RETURN(!pPartyCntl);

		m_tokenOwner = NULL;
		UpdateDataSrcId();
		pPartyCntl->DataTokenReject();
	}

	CBridge::DisconnectParty(partyId);
}

//--------------------------------------------------------------------------
void CFECCBridge::OnEndPartyConnect(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

  CBridge::EndPartyConnect(pParam, FECC_PARTY_BRIDGE_CONNECTED);
}

//--------------------------------------------------------------------------
void CFECCBridge::OnEndPartyDisConnect(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

  CBridge::EndPartyDisConnect(pParam, FECC_PARTY_BRIDGE_DISCONNECTED);
}

//--------------------------------------------------------------------------
void CFECCBridge::OnEndRateChangeDataChannel(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridge::OnEndRateChangeDataChannel - ConfName:", m_pConfName);
  // if master retain token to owner and token owner can transmit data
}

//--------------------------------------------------------------------------
void CFECCBridge:: OnPartyDataTokenRequestCONNECTED(CSegment* pParam)
{
	BOOL isJitcMode = NO;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("ULTRA_SECURE_MODE", isJitcMode);
	if (isJitcMode)
	{
		PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenRequestCONNECTED - Failed, FECC token disabled in JITC mode, ConfName:", m_pConfName);
		return;
	}

	CParty* pParty = NULL;

	if (m_bitRate == 0xFF)
	{
		*pParam >> (void*&)pParty;
		PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenRequestCONNECTED - Failed, this conference does not support LSD channel, ConfName:", m_pConfName);
		SendDataTokenReject(pParty); // Send to the E.P a DCR_L message.
		return;
	}

	PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenRequestCONNECTED - ConfName:", m_pConfName);

	*pParam >> (void*&)pParty;
	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	CFECCBridgePartyCntl* pPartyCntl = (CFECCBridgePartyCntl*)GetPartyCntl(pParty->GetPartyRsrcID());
	if (!pPartyCntl)
	{
		PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenRequestCONNECTED - Failed, party is not connected to data bridge, it is connected now on demand, ConfName:", m_pConfName);
		if (m_bitRate)
			return;
	}

	if (pPartyCntl && pPartyCntl->IsTokenBlocked())
	{
		pPartyCntl->DataTokenReject();
		PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenRequestCONNECTED - Failed, party is blocked by MCU and can not get LSD token, PartyName:", pPartyCntl->GetName());
		return;
	}

	// check bit rate to be in scm of conf.
	// check if token is free and not reserved
	WORD bitRate = 0;
	*pParam >> bitRate;
	WORD isCameraControl = 0;
	*pParam >> isCameraControl;

	if (bitRate != LSD_6400)
	{
		PTRACE2(eLevelError, "CFECCBridge::OnPartyDataTokenRequestCONNECTED - Failed, opening of LSD channel is disabled in non LSD_6400 rate, ConfName:", m_pConfName);
		return;
	}

	// Block FECC token in EQ
	if (IsStandaloneConf())
	{
		if (pPartyCntl)
		{
			PTRACE2(eLevelError, "CFECCBridge::OnPartyDataTokenRequestCONNECTED - Failed, FECC token disabled in EQ, ConfName:", m_pConfName);
			pPartyCntl->DataTokenReject();
			return;
		}
	}

	// set token owner id
	WORD tokenOwner = CASCADE_MODE_MCU;
	if (m_tokenOwner && m_tokenOwner == pParty)
		tokenOwner = CURPARTY;

	if (m_tokenOwner && m_tokenOwner != pParty)
		tokenOwner = OTHERPARTY;

	switch (tokenOwner)
	{
		case CASCADE_MODE_MCU:                     // data token owned by MCU
		{
			// if data channel is opened - Accept party request
			if ((!m_bitRate) || (m_bitRate != bitRate) /* request to change the bit rate.*/)
			{
				PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenRequestCONNECTED - Can not open LSD 6.4 in an Encrypted H320 conference - only 4.8, ConfName:", m_pConfName);
				if (pPartyCntl)
				{
					pPartyCntl->DataTokenReject();
					return;
				}
			}

			if (pPartyCntl)
			{
				pPartyCntl->DataTokenAccept(isCameraControl);
				m_tokenOwner = pParty;    // In cascade case the master MCU gives the
				UpdateDataSrcId();        // token to the party but do not do switch data sources
			}
			else
			{
				PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenRequestCONNECTED - Failed, party is not connected to data bridge, ConfName:", m_pConfName);
			}
			break;
		}

		case CURPARTY:
		{
			// request by data token owner, token withdraw first and party responsibility to initiate a new token request
			if (pPartyCntl)
			{
				if (bitRate != m_bitRate)
				{
					m_tokenOwner = NULL;
					UpdateDataSrcId();
					pPartyCntl->DataTokenWithraw();
				}
				else
				{
					pPartyCntl->DataTokenAccept(isCameraControl);
				}
			}
			break;
		}

		case OTHERPARTY:
		case UNVALID:
		{
			// data token is owned by another party
			if (pPartyCntl)
				pPartyCntl->DataTokenReject();
			else
				PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenRequestCONNECTED - Failed, party is not connected to data bridge, ConfName:", m_pConfName);
			break;
		}

		default:
		{
			PASSERT(tokenOwner + 1000);
			break;
		}
	}
}

//--------------------------------------------------------------------------
void CFECCBridge::OnPartyDataTokenRequestDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenRequestDISCONNECTING - Always rejected while bridge disconnecting, ConfName:", m_pConfName);

  CTaskApp* pParty = NULL;

  *pParam >> (void*&)pParty;

  SendDataTokenReject(pParty);    // Send to the E.P a DCR_L message.
}

//--------------------------------------------------------------------------
void CFECCBridge::OnPartyDataTokenReleaseCONNECTED(CSegment* pParam)
{
	CParty* pParty = NULL;

	if (m_bitRate == 0xFF)
	{
		*pParam >> (void*&)pParty;
		PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenReleaseCONNECTED - This conference does not support LSD channel, ConfName:", m_pConfName);
		SendDataTokenReject(pParty);  // Send to the E.P a DCR_L message.
		return;
	}

	PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenReleaseCONNECTED - ConfName:", m_pConfName);

	*pParam >> (void*&)pParty;
	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	CFECCBridgePartyCntl* pPartyCntl = (CFECCBridgePartyCntl*)GetPartyCntl(pParty->GetPartyRsrcID());
	DBGPASSERT_AND_RETURN(!pPartyCntl);

	// if DIS_L was not sent by the Data Token Holder
	if (pParty != m_tokenOwner)
	{
		pPartyCntl->DataTokenRelease();
		return;
	}

	WORD isCameraControl;
	*pParam >> isCameraControl;

	// In all other cases, also release FECC token
	pPartyCntl->DataTokenRelease();
	m_tokenOwner = NULL;
	UpdateDataSrcId();

	if (isCameraControl)
		m_pConfApi->PCMPartyNotification(pPartyCntl->GetPartyRsrcID(), DATATOKENRELEASE);
}

//--------------------------------------------------------------------------
void CFECCBridge::OnPartyDataTokenReleaseDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenReleaseDISCONNECTING - Ignored, ConfName:", m_pConfName);
}

//--------------------------------------------------------------------------
void CFECCBridge::OnPartyDataTokenReleaseAndFreeCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CFECCBridge::OnPartyDataTokenReleaseAndFreeCONNECTED - ConfName:", m_pConfName);

	CParty* pParty = NULL;
	*pParam >> (void*&)pParty;

	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	CFECCBridgePartyCntl* pPartyCntl = (CFECCBridgePartyCntl*)GetPartyCntl(pParty->GetPartyRsrcID());
	DBGPASSERT_AND_RETURN(!pPartyCntl);

	pPartyCntl->DataTokenRelease();
	m_tokenOwner = NULL;
	UpdateDataSrcId();
}

//--------------------------------------------------------------------------
void CFECCBridge::OnDataTokenWithdraw(CSegment* pParam)
{
	if (m_bitRate == 0xFF)
	{
		PTRACE2(eLevelInfoNormal, "CFECCBridge::OnDataTokenWithdraw - This conference does not support LSD channel, ConfName:", m_pConfName);
		return;
	}

	if (!CPObject::IsValidPObjectPtr(m_tokenOwner))
	{
		// if GetCascadeMode() = CASCADE_MODE_SLAVE and m_tokenOwner = NULL means it
		// free or it belongs to one of the other master's parties.
		PTRACE2(eLevelInfoNormal, "CFECCBridge::OnDataTokenWithdraw - The MCU is already the token owner, ConfName:", m_pConfName);
		return;
	}

	PTRACE2(eLevelInfoNormal, "CFECCBridge::OnDataTokenWithdraw, ConfName:", m_pConfName);

	DWORD srcReq;
	*pParam >> srcReq;

	CFECCBridgePartyCntl* pPartyCntl = (CFECCBridgePartyCntl*)GetPartyCntl(((CParty*)m_tokenOwner)->GetPartyRsrcID());
	PASSERT_AND_RETURN(!pPartyCntl);

	switch (srcReq)
	{
		case CHAIRMAN:
		{
			PTRACE(eLevelInfoNormal, "CFECCBridge::OnDataTokenWithdraw : CHAIR WHITHDRAW THE TOKEN ");
			pPartyCntl->DataTokenWithraw();
			m_tokenOwner = NULL;
			UpdateDataSrcId();
			break;
		}

		case OPERATOR:
		{
			pPartyCntl->DataTokenWithraw();
			m_tokenOwner = NULL;
			UpdateDataSrcId();
			break;
		}

		default:
		{
			PASSERT(srcReq + 1000);
			break;
		}
	}
}

//--------------------------------------------------------------------------
// this action function is called upon operator or chairman request only
void CFECCBridge::OnDataTokenWithdrawAndFree(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CFECCBridge::OnDataTokenWithdrawAndFree - ConfName:", m_pConfName);

	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_tokenOwner));

	CFECCBridgePartyCntl* pPartyCntl = (CFECCBridgePartyCntl*)GetPartyCntl(((CParty*)m_tokenOwner)->GetPartyRsrcID());
	DBGPASSERT_AND_RETURN(!pPartyCntl);

	pPartyCntl->DataTokenWithraw();
	m_tokenOwner = NULL;
	UpdateDataSrcId();
}

//--------------------------------------------------------------------------
void CFECCBridge::UpdateDataSrcId()
{
  CTaskApp* localTokenOwner = (m_tokenOwner) ? m_tokenOwner : (CTaskApp*)0xFFFF;

  m_pConfApi->UpdateDB(localTokenOwner, LSDSRC, (DWORD)m_bitRate);
}

//--------------------------------------------------------------------------
void CFECCBridge::SendDataTokenReject(CTaskApp* pParty)
{
  DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

  CPartyApi pPartyApi;
  pPartyApi.CreateOnlyApi(pParty->GetRcvMbx());
  pPartyApi.DataTokenReject();
  pPartyApi.DestroyOnlyApi();
}



PBEGIN_MESSAGE_MAP(CFECCBridgePartyCntl)
  ONEVENT(DATCONNECT,                     IDLE,      CFECCBridgePartyCntl::OnDatCntlConnectIDLE)
  ONEVENT(FECC_PARTY_BRIDGE_DISCONNECTED, CONNECTED, CFECCBridgePartyCntl::OnDatCntlDisConnectCONNECTED)
  ONEVENT(DATATOKENACCEPT,                CONNECTED, CFECCBridgePartyCntl::OnDataCntlTokenAccept)
  ONEVENT(DATATOKENREJECT,                CONNECTED, CFECCBridgePartyCntl::OnDataCntlTokenReject)
  ONEVENT(DATATOKENRELEASE,               CONNECTED, CFECCBridgePartyCntl::OnDataCntlTokenRelease)
  ONEVENT(DATATOKENRELEASEFREE,           CONNECTED, CFECCBridgePartyCntl::OnDataCntlTokenReleaseAndFree)
  ONEVENT(DATATOKENREQ,                   CONNECTED, CFECCBridgePartyCntl::OnDataCntlTokenRequest)
  ONEVENT(DATATOKENWITHDRAW,              CONNECTED, CFECCBridgePartyCntl::OnDataCntlTokenWithdraw)
  ONEVENT(DATATOKENRELEASEREQ,            CONNECTED, CFECCBridgePartyCntl::OnDataCntlTokenReleaseRequest)
  ONEVENT(BLOCKLSDTOKENTOUT,              CONNECTED, CFECCBridgePartyCntl::OnTimerBlockTokenRequest)
PEND_MESSAGE_MAP(CFECCBridgePartyCntl, CBridgePartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CFECCBridgePartyCntl
////////////////////////////////////////////////////////////////////////////
CFECCBridgePartyCntl::CFECCBridgePartyCntl()          // constructor
{
  m_isLsdTokenBlocked = NO;

  VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CFECCBridgePartyCntl::~CFECCBridgePartyCntl()         // destructor
{
  if (m_pPartyApi)
  {
    m_pPartyApi->DestroyOnlyApi();
    POBJDELETE(m_pPartyApi);
  }
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::SetFullName(const char* partyName, const char* confName)
{
  snprintf(m_partyConfName, sizeof(m_partyConfName), "%s,%s,DAT", confName, partyName);
}

//--------------------------------------------------------------------------
WORD CFECCBridgePartyCntl::IsTokenBlocked()
{
  return m_isLsdTokenBlocked;
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::Connect()
{
  DispatchEvent(DATCONNECT, NULL);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::DisConnect()
{
  DispatchEvent(FECC_PARTY_BRIDGE_DISCONNECTED, NULL);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::DataTokenAccept(WORD isCameraControl)
{
  CSegment* pSeg = new CSegment;
  *pSeg << isCameraControl;
  DispatchEvent(DATATOKENACCEPT, pSeg);

  POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::DataTokenReject()
{
  DispatchEvent(DATATOKENREJECT, NULL);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::DataTokenReleaseAndFree()
{
  DispatchEvent(DATATOKENRELEASEFREE, NULL);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::DataTokenRequest(BYTE bitRate)
{
  CSegment* pSeg = new CSegment;
  *pSeg << bitRate;
  DispatchEvent(DATATOKENREQ, pSeg);
  POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::DataTokenWithraw()
{
  m_isLsdTokenBlocked = YES;
  DispatchEvent(DATATOKENWITHDRAW, NULL);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::DataTokenReleaseRequest()
{
  DispatchEvent(DATATOKENRELEASEREQ, NULL);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::DataTokenRelease()
{
  DispatchEvent(DATATOKENRELEASE, NULL);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::OnDatCntlConnectIDLE(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridgePartyCntl::OnDatCntlConnectIdle - Name:", m_partyConfName);
  m_state = CONNECTED;

  // Inform FECC Bridge
  m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, FECC_BRIDGE_MSG, ENDCONNECTPARTY, statOK, FALSE);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl:: OnDatCntlDisConnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridgePartyCntl::OnDatCntlDisConnectCONNECTED - Name:", m_partyConfName);

  // Inform FECC Bridge
  m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, FECC_BRIDGE_MSG, ENDDISCONNECTPARTY, statOK, FALSE);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::OnDataCntlTokenRequest(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridgePartyCntl::OnDataCntlTokenRequest - Name:", m_partyConfName);
  BYTE bitRate;
  *pParam >> bitRate;
  m_pPartyApi->DataTokenRequest(bitRate);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::OnDataCntlTokenRelease(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridgePartyCntl::OnDataCntlTokenRelease - Name:", m_partyConfName);
  m_pPartyApi->DataTokenRelease();
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::OnDataCntlTokenReleaseAndFree(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridgePartyCntl::OnDataCntlTokenReleaseAndFree - Name:", m_partyConfName);
  m_pPartyApi->DataTokenReleaseAndFree();
}
//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::OnDataCntlTokenAccept(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridgePartyCntl::OnDataCntlTokenAccept - Name:", m_partyConfName);
  WORD isCameraControl = 0;
  *pParam >>isCameraControl;
  m_pPartyApi->DataTokenAccept(isCameraControl);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::OnDataCntlTokenReject(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridgePartyCntl::OnDataCntlTokenReject - Name:", m_partyConfName);
  m_pPartyApi->DataTokenReject();
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::OnDataCntlTokenReleaseRequest(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridgePartyCntl::OnDataCntlTokenReleaseRequest - Name:", m_partyConfName);
  m_pPartyApi->DataTokenReleaseRequest();
}
//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::OnDataCntlTokenWithdraw(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CFECCBridgePartyCntl::OnDataCntlTokenWithdraw - Name:", m_partyConfName);
  m_pPartyApi->DataTokenWithdraw();
  StartTimer(BLOCKLSDTOKENTOUT, 30*SECOND);
}

//--------------------------------------------------------------------------
void CFECCBridgePartyCntl::OnTimerBlockTokenRequest(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CDatPartyCntl::OnTimerBlockTokenRequest - Name:", m_partyConfName);
  m_isLsdTokenBlocked = NO;
}
