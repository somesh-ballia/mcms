#include "BridgePartyCntl.h"
#include "PartyApi.h"
#include "ConfApi.h"
#include "ObjString.h"
#include "Bridge.h"
#include "BridgePartyMediaUniDirection.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsAudio.h"

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyCntl
////////////////////////////////////////////////////////////////////////////
CBridgePartyCntl::CBridgePartyCntl()
{
	m_pParty                             = NULL;
	m_partyRsrcID                        = (PartyRsrcID)0;
	m_confRsrcID                         = (ConfRsrcID)0;
	m_pPartyApi                          = NULL;
	m_pConfApi                           = NULL;
	m_pBridge                            = NULL;
	m_wNetworkInterface                  = AUTO_INTERFACE_TYPE;
	m_pBridgePartyIn                     = NULL;
	m_pBridgePartyOut                    = NULL;
	m_connectionFailureCause             = statOK;
	m_connFailureCauseDirection          = eMipNoneDirction;
	m_connFailureCauseTimerOrStatFailure = eMpiNoTimerAndStatus;
	m_connFailureCauseAction             = eMipNoAction;
	m_eCompareBy                         = eCompareByTaskAppPtr;
	m_DisconnectingDirection             = eNoDirection;
	m_bCascadeLinkMode                   = NONE;
	m_bUseSpeakerSsrcForTx				 = FALSE;
	m_bIsCallGeneratorConf               = FALSE;
	m_isAvMCUConnection                  = FALSE;
	m_RoomId						= 0xFFFF;
	memset(m_partyConfName, '\0', BRIDGE_PARTY_CONF_NAME_SIZE);
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyCntl::CBridgePartyCntl(const void* partyId, const char* partyName, PartyRsrcID partyRsrcID)
{
	m_eCompareBy = eCompareByTaskAppPtr;

	if (NULL != partyName)
	{
		strncpy(m_name, partyName, H243_NAME_LEN-1);
		m_name[H243_NAME_LEN -1] = '\0';
	}
	else
	{
		strncpy(m_name, "TEMP", H243_NAME_LEN-1);
		m_name[H243_NAME_LEN -1] = '\0';
	}

	m_pParty                             = (CTaskApp*)partyId;
	m_partyRsrcID                        = partyRsrcID;
	m_confRsrcID                         = (ConfRsrcID)0;
	m_pPartyApi                          = NULL;
	m_pConfApi                           = NULL;
	m_pBridge                            = NULL;
	m_wNetworkInterface                  = AUTO_INTERFACE_TYPE;
	m_pBridgePartyIn                     = NULL;
	m_pBridgePartyOut                    = NULL;
	m_connectionFailureCause             = statOK;
	m_connFailureCauseDirection          = eMipNoneDirction;
	m_connFailureCauseTimerOrStatFailure = eMpiNoTimerAndStatus;
	m_connFailureCauseAction             = eMipNoAction;
	m_DisconnectingDirection             = eNoDirection;
	m_bCascadeLinkMode                   = NONE;
	m_bUseSpeakerSsrcForTx				 = FALSE;
	m_bIsCallGeneratorConf               = FALSE;
	m_RoomId               				= 0xFFFF;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyCntl::CBridgePartyCntl (const CBridgePartyCntl& rOtherBridgePartyCntl)
	: CStateMachineValidation(rOtherBridgePartyCntl)
{
	m_pParty                             = NULL;
	m_partyRsrcID                        = (PartyRsrcID)0;
	m_confRsrcID                         = (ConfRsrcID)0;
	m_pPartyApi                          = NULL;
	m_pConfApi                           = NULL;
	m_pBridge                            = NULL;
	m_wNetworkInterface                  = AUTO_INTERFACE_TYPE;
	m_pBridgePartyIn                     = NULL;
	m_pBridgePartyOut                    = NULL;
	m_connectionFailureCause             = statOK;
	m_connFailureCauseDirection          = eMipNoneDirction;
	m_connFailureCauseTimerOrStatFailure = eMpiNoTimerAndStatus;
	m_connFailureCauseAction             = eMipNoAction;
	m_eCompareBy                         = eCompareByTaskAppPtr;
	m_DisconnectingDirection             = eNoDirection;
	m_bCascadeLinkMode                   = NONE;
	m_bUseSpeakerSsrcForTx				 = FALSE;
	m_bIsCallGeneratorConf               = FALSE;
	m_RoomId							 = 0xFFFF;
	*this                                = rOtherBridgePartyCntl;
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyCntl::~CBridgePartyCntl ()
{
	if (m_pPartyApi)
	{
		m_pPartyApi->DestroyOnlyApi();
		POBJDELETE(m_pPartyApi);
	}

	if (m_pConfApi)
	{
		m_pConfApi->DestroyOnlyApi();
		POBJDELETE(m_pConfApi);
	}

	POBJDELETE(m_pBridgePartyOut);
	POBJDELETE(m_pBridgePartyIn);
}

// ///////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::UnregisterInTask()
{
	if (m_pBridgePartyIn)
		m_pBridgePartyIn->UnregisterInTask();

	if (m_pBridgePartyOut)
		m_pBridgePartyOut->UnregisterInTask();

	CStateMachine::UnregisterInTask();
}

// ///////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::RegisterInTask(CTaskApp* myNewTask)
{
	CStateMachine::RegisterInTask(myNewTask);

	if (m_pBridgePartyOut)
		m_pBridgePartyOut->RegisterInTask(myNewTask);

	if (m_pBridgePartyIn)
		m_pBridgePartyIn->RegisterInTask(myNewTask);
}

////////////////////////////////////////////////////////////////////////////
CBridgePartyCntl& CBridgePartyCntl::operator=(const CBridgePartyCntl& rOtherBridgePartyCntl)
{
	if (&rOtherBridgePartyCntl == this)
		return *this;

	if (NULL != rOtherBridgePartyCntl.m_name)
	{
		strncpy(m_name, rOtherBridgePartyCntl.m_name, H243_NAME_LEN-1);
		m_name[H243_NAME_LEN -1] = '\0';
	}
	else
	{
		strncpy(m_name, "TEMP", H243_NAME_LEN-1);
		m_name[H243_NAME_LEN -1] = '\0';
	}

	if (NULL != m_pPartyApi)
	{
		m_pPartyApi->DestroyOnlyApi();
		POBJDELETE(m_pPartyApi);
	}

	if (rOtherBridgePartyCntl.m_pPartyApi)
	{
		m_pPartyApi = new CPartyApi;
		m_pPartyApi->CreateOnlyApi(rOtherBridgePartyCntl.m_pPartyApi->GetRcvMbx());
	}

	if (NULL != m_pConfApi)
	{
		m_pConfApi->DestroyOnlyApi();
		POBJDELETE(m_pConfApi);
	}

	if (rOtherBridgePartyCntl.m_pConfApi)
	{
		m_pConfApi = new CConfApi(*(rOtherBridgePartyCntl.m_pConfApi));
	}

	m_pParty                             = rOtherBridgePartyCntl.m_pParty;
	m_pBridge                            = rOtherBridgePartyCntl.m_pBridge;
	m_wNetworkInterface                  = rOtherBridgePartyCntl.m_wNetworkInterface;
	m_eCompareBy                         = rOtherBridgePartyCntl.m_eCompareBy;
	m_partyRsrcID                        = rOtherBridgePartyCntl.m_partyRsrcID;
	m_confRsrcID                         = rOtherBridgePartyCntl.m_confRsrcID;
	m_connectionFailureCause             = rOtherBridgePartyCntl.m_connectionFailureCause;
	m_connFailureCauseDirection          = rOtherBridgePartyCntl.m_connFailureCauseDirection;
	m_connFailureCauseTimerOrStatFailure = rOtherBridgePartyCntl.m_connFailureCauseTimerOrStatFailure;
	m_connFailureCauseAction             = rOtherBridgePartyCntl.m_connFailureCauseAction;
	m_DisconnectingDirection             = rOtherBridgePartyCntl.m_DisconnectingDirection;
	m_RoomId							= rOtherBridgePartyCntl.m_RoomId;

	if ((NULL != m_pBridgePartyIn) && (NULL != rOtherBridgePartyCntl.m_pBridgePartyIn))
		*m_pBridgePartyIn = (CBridgePartyMediaUniDirection&)(*rOtherBridgePartyCntl.m_pBridgePartyIn);

	if ((NULL != m_pBridgePartyOut) && (NULL != rOtherBridgePartyCntl.m_pBridgePartyOut))
		*m_pBridgePartyOut = (CBridgePartyMediaUniDirection&)(*rOtherBridgePartyCntl.m_pBridgePartyOut);

	m_bCascadeLinkMode		= rOtherBridgePartyCntl.m_bCascadeLinkMode;

	m_bUseSpeakerSsrcForTx	= rOtherBridgePartyCntl.m_bUseSpeakerSsrcForTx;

	m_bIsCallGeneratorConf  = rOtherBridgePartyCntl.m_bIsCallGeneratorConf;

	return *this;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::CopyInAndOut(const CBridgePartyCntl& rOtherBridgePartyCntl)
{
	if ((NULL != m_pBridgePartyIn) && (NULL != rOtherBridgePartyCntl.m_pBridgePartyIn))
		*m_pBridgePartyIn = (CBridgePartyMediaUniDirection&)(*rOtherBridgePartyCntl.m_pBridgePartyIn);

	if ((NULL != m_pBridgePartyOut) && (NULL != rOtherBridgePartyCntl.m_pBridgePartyOut))
		*m_pBridgePartyOut = (CBridgePartyMediaUniDirection&)(*rOtherBridgePartyCntl.m_pBridgePartyOut);
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::Create(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	strcpy_safe(m_name, pBridgePartyInitParams->GetPartyName());

	SetFullName(pBridgePartyInitParams->GetPartyName(), pBridgePartyInitParams->GetConfName());

	m_pBridge           = (CBridge*)(pBridgePartyInitParams->GetBridge());
	m_pParty            = (CTaskApp*)(pBridgePartyInitParams->GetParty());
	m_partyRsrcID       = pBridgePartyInitParams->GetPartyRsrcID();
	m_confRsrcID        = pBridgePartyInitParams->GetConfRsrcID();
	m_wNetworkInterface = pBridgePartyInitParams->GetNetworkInterface();
	m_bCascadeLinkMode  = pBridgePartyInitParams->GetCascadeLinkMode();
	m_RoomId			= pBridgePartyInitParams->GetPartyRoomID();
	// speakerIndication
	m_bUseSpeakerSsrcForTx = pBridgePartyInitParams->GetUseSpeakerSsrcForTx();
	TRACEINTO << "speakerIndication - m_bUseSpeakerSsrcForTx: " << ( (TRUE == m_bUseSpeakerSsrcForTx) ? "TRUE" : "FALSE");

	CConf* pConf = (CConf*)pBridgePartyInitParams->GetConf();

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pConfApi = new CConfApi(pConf->GetMonitorConfId());
#else
	m_pConfApi = new CConfApi;
#endif
	m_pConfApi->CreateOnlyApi(pConf->GetRcvMbx(), this);
	m_pConfApi->SetLocalMbx(pConf->GetLocalQueue());

	m_pPartyApi = new CPartyApi;
	m_pPartyApi->CreateOnlyApi(m_pParty->GetRcvMbx(), this);

	CCommConf *pCommConf = (CCommConf*)pConf->GetCommConf();
	if (pCommConf)
		m_bIsCallGeneratorConf = pCommConf->GetIsCallGeneratorConference();
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::SetFullName(const char* partyName,const char* confName)
{
  if (partyName && confName)
    snprintf(m_partyConfName, sizeof(m_partyConfName), "ConfName:%s, PartyName:%s", confName, partyName);
  else if (partyName)
    snprintf(m_partyConfName, sizeof(m_partyConfName), "PartyName:%s", partyName);
  else if (confName)
    snprintf(m_partyConfName, sizeof(m_partyConfName), "ConfName:%s", confName);
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::SetConnectionFailureCauseAdditionalInfo(BYTE newConnFailureCauseDirection, BYTE newConnFailureCauseTimerOrStatFailure, BYTE newConnFailureCauseAction)
{
	m_connFailureCauseDirection          = newConnFailureCauseDirection;
	m_connFailureCauseTimerOrStatFailure = newConnFailureCauseTimerOrStatFailure;
	m_connFailureCauseAction             = newConnFailureCauseAction;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::Destroy()
{
	if (m_pPartyApi)
	{
		m_pPartyApi->DestroyOnlyApi();
		POBJDELETE(m_pPartyApi);
	}

	if (m_pConfApi)
	{
		m_pConfApi->DestroyOnlyApi();
		POBJDELETE(m_pConfApi);
	}

	POBJDELETE(m_pBridgePartyIn);
	POBJDELETE(m_pBridgePartyOut);
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
  DispatchEvent(opCode, pMsg);
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::Connect()
{
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::DisConnect()
{
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::Export()
{
}

////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyCntl::IsUniDirectionConnection(EMediaDirection eMediaDirection)
{
	BOOL resultValue = FALSE;

	switch (eMediaDirection)
	{
		case eMediaIn:
		{
			if (m_pBridgePartyIn && !m_pBridgePartyOut)
				resultValue = TRUE;
			break;
		}

		case eMediaOut:
		{
			if (m_pBridgePartyOut && !m_pBridgePartyIn)
				resultValue = TRUE;
			break;
		}

		case eNoDirection:
		{	// any side - In or Out
			if (!m_pBridgePartyIn && !m_pBridgePartyOut)
			{
				resultValue = FALSE;
			}
			else
			{
				if (m_pBridgePartyIn && m_pBridgePartyOut)
				{
					resultValue = FALSE;
				}
				else
					resultValue = TRUE;
			}
			break;
		}
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}

	return resultValue;
}

////////////////////////////////////////////////////////////////////////////
bool operator<(const CBridgePartyCntl& rFirst, const CBridgePartyCntl& rSecond)
{
	bool rval = false;

	ECompareStyle eCompareBy = eCompareByTaskAppPtr;

	if (rFirst.m_eCompareBy == eCompareByName || rSecond.m_eCompareBy == eCompareByName)
		eCompareBy = eCompareByName;

	switch (eCompareBy)
	{
		case eCompareByName:
		{
			if (strcmp(rFirst.m_name, rSecond.m_name) < 0)
				rval = true;
			break;
		}

		case eCompareByTaskAppPtr:
		{
			if (rFirst.m_pParty < rSecond.m_pParty)
				rval = true;
			break;
		}

		case eCompareByPartyRsrcID:
		{
			if (rFirst.m_partyRsrcID < rSecond.m_partyRsrcID)
				rval = true;
			break;
		}
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}

	return rval;
}

////////////////////////////////////////////////////////////////////////////
bool operator==(const CBridgePartyCntl& rFirst, const CBridgePartyCntl& rSecond)
{
	bool rval = false;

	ECompareStyle eCompareBy = eCompareByTaskAppPtr;

	if (rFirst.m_eCompareBy == eCompareByName || rSecond.m_eCompareBy == eCompareByName)
		eCompareBy = eCompareByName;

	if (rFirst.m_eCompareBy == eCompareByPartyRsrcID || rSecond.m_eCompareBy == eCompareByPartyRsrcID)
		eCompareBy = eCompareByPartyRsrcID;

	switch (eCompareBy)
	{
		case eCompareByName:
		{
			if (!strcmp(rFirst.m_name, rSecond.m_name))
				rval = true;
			break;
		}

		case eCompareByTaskAppPtr:
		{
			if (rFirst.m_pParty == rSecond.m_pParty)
				rval = true;
			break;
		}

		case eCompareByPartyRsrcID:
		{
			if (rFirst.m_partyRsrcID == rSecond.m_partyRsrcID)
				rval = true;
			break;
		}

		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	return rval;
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::ArePortsOpened(BOOL& rIsInPortOpened, BOOL& rIsOutPortOpened) const
{
	rIsInPortOpened  = FALSE;
	rIsOutPortOpened = FALSE;

	if (m_pBridgePartyIn)
	{
		if (m_pBridgePartyIn->IsConnected())
			rIsInPortOpened = TRUE;
	}

	if (m_pBridgePartyOut)
	{
		if (m_pBridgePartyOut->IsConnected())
			rIsOutPortOpened = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::RemoveConfParams()
{
	m_confRsrcID = (ConfRsrcID)0;
	m_pBridge    = NULL;

	POBJDELETE(m_pConfApi);

	strncpy(m_partyConfName, "TEMP", BRIDGE_PARTY_CONF_NAME_SIZE-1);
	m_partyConfName[BRIDGE_PARTY_CONF_NAME_SIZE -1] = '\0';

	if (m_pBridgePartyIn)
		m_pBridgePartyIn->RemoveConfParams();

	if (m_pBridgePartyOut)
		m_pBridgePartyOut->RemoveConfParams();
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::UpdateNewConfParams(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	SetFullName(m_name, pBridgePartyInitParams->GetConfName());

	m_pBridge    = (CBridge*)(pBridgePartyInitParams->GetBridge());
	m_confRsrcID = pBridgePartyInitParams->GetConfRsrcID();
	m_pConfApi   = new CConfApi;
	m_pConfApi->CreateOnlyApi(((CConf*)(pBridgePartyInitParams->GetConf()))->GetRcvMbx(), this);
	m_pConfApi->SetLocalMbx(((CConf*)(pBridgePartyInitParams->GetConf()))->GetLocalQueue());
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::SetDisConnectingDirectionsReq(EMediaDirection eDisConnectedDirection)
{
	if (m_DisconnectingDirection == eNoDirection)
	{
		m_DisconnectingDirection = eDisConnectedDirection;
	}
	else
	{
		if ((m_DisconnectingDirection == eMediaIn && eDisConnectedDirection == eMediaOut) ||
			(m_DisconnectingDirection == eMediaOut && eDisConnectedDirection == eMediaIn))
		{
			m_DisconnectingDirection = eMediaInAndOut;
		}
		else
			m_DisconnectingDirection = eDisConnectedDirection;
	}
}
////////////////////////////////////////////////////////////////////////////
// This function receive connected direction and destroy the other direction
void CBridgePartyCntl::DestroyPartyInOut(EMediaDirection ConnectedDirection)
{
	switch (ConnectedDirection)
	{
		case eMediaIn:
		{
			if (m_pBridgePartyOut)
				POBJDELETE(m_pBridgePartyOut);
			break;
		}

		case eMediaOut:
		{
			if (m_pBridgePartyIn)
				POBJDELETE(m_pBridgePartyIn);
			break;
		}

		case eNoDirection:
		{
			if (m_pBridgePartyOut)
				POBJDELETE(m_pBridgePartyOut);

			if (m_pBridgePartyIn)
			{
				PTRACE2INT(eLevelInfoNormal, "CBridgePartyCntl::DestroyPartyInOut -  m_pBridgePartyIn", (DWORD)m_pBridgePartyIn);
				POBJDELETE(m_pBridgePartyIn);
			}
			break;
		}

		default:
		{
			PTRACE(eLevelInfoNormal, "CBridgePartyCntl::DestroyPartyInOut - No direction to destroy ");
			break;
		}
	} // switch
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::DumpMcuInternalProblemDetailed(BYTE mipDirection, BYTE mipTimerStatus, BYTE mipMedia)
{
	CLargeString cstr;
	CLargeString responsibilityStr;
	DWORD        faultOpcode = 0;

	cstr << "Party:" << m_partyRsrcID << " Conf:" << m_confRsrcID;
	DWORD lastReq            = 0;
	switch (mipTimerStatus)
	{
		case (eMipTimer):
		{
			faultOpcode = ACK_NOT_RECEIVED;
			cstr << " Did not receive ACK  for opcode: ";
			break;
		}

		case (eMipStatusFail):
		{
			faultOpcode = ACK_FAILED;
			cstr << " receives Failure Status for opcode: ";
			break;
		}
	} // switch

	if (mipDirection == eMipOut)
	{
		if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
		{
			CProcessBase* pProcess = CProcessBase::GetProcess();
			lastReq = m_pBridgePartyOut->GetLastReq();
			string opcodeStr = pProcess->GetOpcodeAsString(lastReq);
			cstr << opcodeStr.c_str();
			cstr << " from:";
			AddRsrcStrToMipDetailedStr(cstr, mipDirection, mipMedia);
			cstr << " Req:" << m_pBridgePartyOut->GetLastReqId();
		}
		else
		{
			cstr << "m_pBridgePartyOut is not valid!!!! ????";
		}
	}

	if (mipDirection == eMipIn)
	{
		if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
		{
			CProcessBase* pProcess = CProcessBase::GetProcess();
			lastReq = m_pBridgePartyIn->GetLastReq();
			string opcodeStr       = pProcess->GetOpcodeAsString(lastReq);
			cstr << opcodeStr.c_str();
			cstr << " from:";
			AddRsrcStrToMipDetailedStr(cstr, mipDirection, mipMedia);
			cstr << " Req:" << m_pBridgePartyIn->GetLastReqId();
		}
		else
		{
			cstr << "m_pBridgePartyIn is not valid!!!! ????";
		}
	}

	switch (lastReq)
	{
		case TB_MSG_OPEN_PORT_REQ:
		case TB_MSG_CLOSE_PORT_REQ:
		{
			responsibilityStr << "( Responsibility: algorithms )";
			break;
		}

		case AUDIO_OPEN_DECODER_REQ:
		case AUDIO_OPEN_ENCODER_REQ:
		case AUDIO_CLOSE_ENCODER_REQ:
		case AUDIO_CLOSE_DECODER_REQ:
		case TB_MSG_CONNECT_REQ:
		case TB_MSG_DISCONNECT_REQ:
		{
			responsibilityStr << "( Responsibility: embedded )";
			break;
		}

		default:
		{
			responsibilityStr << "( Responsibility: ConfParty )";
			break;
		}
	} // switch

	cstr <<  responsibilityStr;
	DumpMcuInternalDetailed(cstr, faultOpcode);
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyCntl::AddRsrcStrToMipDetailedStr(CLargeString& cstr, BYTE mipDirection, BYTE mipMedia)
{
	switch (mipMedia)
	{
		case (eMipAudio):
		{
			if (mipDirection == eMipOut)
				cstr << " audio encoder";
			if (mipDirection == eMipIn)
				cstr << " audio decoder";
			break;
		}

		case (eMipVideo):
		{
			if (mipDirection == eMipOut)
				cstr << " video encoder";
			if (mipDirection == eMipIn)
				cstr << " video decoder";
			break;
		}
	} // switch
}

////////////////////////////////////////////////////////////////////////////
const char* CBridgePartyCntl::GetConfName()
{
	return (m_pBridge) ? m_pBridge->GetConfName() : NULL;
}

////////////////////////////////////////////////////////////////////////////
BYTE CBridgePartyCntl::GetIsGateWay()
{
	return (m_pBridge) ? m_pBridge->GetIsGateWay() : 0;
}

////////////////////////////////////////////////////////////////////////////
const CConf* CBridgePartyCntl::GetConf()
{
	return (m_pBridge) ? m_pBridge->GetConf() : NULL;
}

////////////////////////////////////////////////////////////////////////////
DWORD CBridgePartyCntl::GetConfMediaType()
{
	const CConf* pConf = GetConf();
	if (pConf)
	{
		//const CCommConf* pCommConf= pConf->GetCommConf();
		CCommConf* pCommConf= (CCommConf*)pConf->GetCommConf();	// not so good...
		if (pCommConf)
			return pCommConf->GetConfMediaType();
	}
	PTRACE(eLevelInfoNormal, "CBridgePartyCntl::GetConfMediaType - Error: NULL pointer, (return eAvcOnly as default) ");
	return eAvcOnly;
}



