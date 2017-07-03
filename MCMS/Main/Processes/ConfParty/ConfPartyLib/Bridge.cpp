#include "Bridge.h"
#include "BridgeInitParams.h"
#include "BridgePartyDisconnectParams.h"
#include "BridgePartyExportParams.h"
#include "HostCommonDefinitions.h"
#include "Party.h"

////////////////////////////////////////////////////////////////////////////
//                        CBridge
////////////////////////////////////////////////////////////////////////////
CBridge::CBridge ()
{
	m_eBridgeImplementationType = eNoType;
	m_pPartyList                = NULL;
	m_pConfApi                  = NULL;
	m_pConf                     = NULL;
	m_pConfName[0]              = '\0';
	m_confRsrcID                = DUMMY_CONF_ID;
}

//--------------------------------------------------------------------------
CBridge::~CBridge ()
{
	if (m_pConfApi)
	{
		m_pConfApi->DestroyOnlyApi();
		POBJDELETE(m_pConfApi);
	}

	if (m_pPartyList)
	{
		m_pPartyList->ClearAndDestroy();
		POBJDELETE(m_pPartyList);
	}
}

//--------------------------------------------------------------------------
CBridge::CBridge (const CBridge& rOtherBridge)
        :CStateMachineValidation(rOtherBridge)
{
	m_eBridgeImplementationType = eNoType;
	m_pPartyList                = NULL;
	m_pConfApi                  = NULL;
	m_pConf                     = NULL;

	*this = rOtherBridge;
}

//--------------------------------------------------------------------------
CBridge& CBridge::operator=(const CBridge& rOtherBridge)
{
	if (&rOtherBridge == this)
		return *this;

	m_eBridgeImplementationType = rOtherBridge.m_eBridgeImplementationType;
	m_state                     = rOtherBridge.GetState();

	AllocatePartyList();

	/* Yoella - Nov-07 H239 MIH
	 * When i try to use the Copy of ContentBridgeList as implemented
	 * in the function CopyContentBridgeList, for all kind of bridges,( and with that to save it general)
	 * i got a problem - the object lost its private class - when i call the disconnect for example,the disconnect
	 * of the parent BridgePartyCntl was invoked, instead of the ContentBridgePartyCntl function for disconnect.
	 * I left it temporary like that and will try to find another solution....
	 */
	if (NULL != rOtherBridge.m_pPartyList)
	{
		if (eContent_Bridge_V1 == m_eBridgeImplementationType)
			m_pPartyList->CopyContentBridgeList(*rOtherBridge.m_pPartyList, this);
		else
			*m_pPartyList = (*rOtherBridge.m_pPartyList);
	}

	m_pConf      = rOtherBridge.m_pConf;
	m_confRsrcID = rOtherBridge.m_confRsrcID;

	strcpy_safe(m_pConfName, rOtherBridge.m_pConfName);

	if (NULL != m_pConfApi)
	{
		m_pConfApi->DestroyOnlyApi();
		POBJDELETE(m_pConfApi);
	}

	if (rOtherBridge.m_pConfApi)
		m_pConfApi = new CConfApi(*(rOtherBridge.m_pConfApi));

	return *this;
}

//--------------------------------------------------------------------------
void CBridge::Create(const CBridgeInitParams* pBridgeInitParams)
{
	m_eBridgeImplementationType = pBridgeInitParams->GetBridgeImplementationType();

	m_pConf      = (CConf*)pBridgeInitParams->GetConf();
	m_confRsrcID = pBridgeInitParams->GetConfRsrcId();

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pConfApi = new CConfApi(m_pConf->GetMonitorConfId());
#else
	m_pConfApi = new CConfApi;
#endif
	m_pConfApi->CreateOnlyApi(m_pConf->GetRcvMbx(), this);
	m_pConfApi->SetLocalMbx(m_pConf->GetLocalQueue());

	strcpy_safe(m_pConfName, pBridgeInitParams->GetConfName());

	AllocatePartyList();
}

//--------------------------------------------------------------------------
void CBridge::Destroy()
{
	if (m_pConfApi)
	{
		m_pConfApi->DestroyOnlyApi();
		POBJDELETE(m_pConfApi);
	}

	if (m_pPartyList)
	{
		m_pPartyList->ClearAndDestroy();
		POBJDELETE(m_pPartyList);
	}

	DeleteAllTimers();

	m_state = IDLE;
}

//--------------------------------------------------------------------------
void CBridge::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------

void* CBridge::GetMessageMap()
{
	return NULL;
}

//--------------------------------------------------------------------------
BOOL CBridge::IsConnected() const
{
	if (IDLE == m_state)
		return FALSE;
	else
		return TRUE;
}

//--------------------------------------------------------------------------
BOOL CBridge::IsPartyConnected(const CTaskApp* pParty) const
{
	if ((NULL == m_pPartyList) || (0 == m_pPartyList->size()))
		return FALSE;

	CBridgePartyCntl* pBridgePartyCntl = m_pPartyList->Find((const CParty*)pParty);
	return (pBridgePartyCntl) ? TRUE : FALSE;
}

//--------------------------------------------------------------------------
void CBridge::ArePortsOpened(const CParty* pParty, BOOL& rIsInPortOpened, BOOL& rIsOutPortOpened) const
{
	rIsInPortOpened  = FALSE;
	rIsOutPortOpened = FALSE;

	if ((NULL == m_pPartyList) || (0 == m_pPartyList->size()))
		return;

	CBridgePartyCntl* pResultBridgePartyCntl = m_pPartyList->Find(pParty);
	if (pResultBridgePartyCntl)
		pResultBridgePartyCntl->ArePortsOpened(rIsInPortOpened, rIsOutPortOpened);
}

//--------------------------------------------------------------------------
void CBridge::ArePortsOpened(PartyRsrcID partyId, BOOL& rIsInPortOpened, BOOL& rIsOutPortOpened)
{
	rIsInPortOpened  = FALSE;
	rIsOutPortOpened = FALSE;

	if ((NULL == m_pPartyList) || (0 == m_pPartyList->size()))
		return;

	CBridgePartyCntl* pResultBridgePartyCntl = GetPartyCntl(partyId);
	if (pResultBridgePartyCntl)
		pResultBridgePartyCntl->ArePortsOpened(rIsInPortOpened, rIsOutPortOpened);
}

//--------------------------------------------------------------------------
void CBridge::ArePortsOpened(PartyRsrcID partyId, std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)
{
	if ((NULL == m_pPartyList) || (0 == m_pPartyList->size()))
		return;

	CBridgePartyCntl* pResultBridgePartyCntl = GetPartyCntl(partyId);
	if (pResultBridgePartyCntl)
		pResultBridgePartyCntl->GetPortsOpened(isOpenedRsrcMap);
}

//--------------------------------------------------------------------------
BOOL CBridge::IsStandaloneConf() const
{
	return (m_confRsrcID == STANDALONE_CONF_ID);
}

//--------------------------------------------------------------------------
WORD CBridge::GetNumParties() const
{
	return m_pPartyList ? m_pPartyList->size() : 0;
}

//--------------------------------------------------------------------------
CBridgePartyCntl* CBridge::GetPartyCntl(const char* name)
{
	return m_pPartyList->Find(name);
}

//--------------------------------------------------------------------------
CBridgePartyCntl* CBridge::GetPartyCntl(const CTaskApp* pParty)
{
	return m_pPartyList->Find((const CParty*)pParty);
}

//--------------------------------------------------------------------------
CBridgePartyCntl* CBridge::GetPartyCntl(const PartyRsrcID partyId)
{
	return m_pPartyList->Find(partyId);
}

//--------------------------------------------------------------------------
CTaskApp* CBridge::GetPartyTaskApp(PartyRsrcID partyId)
{
	CBridgePartyCntl* pPartyCntl = GetPartyCntl(partyId);

	return pPartyCntl ? pPartyCntl->GetPartyTaskApp() : NULL;
}

//--------------------------------------------------------------------------
void CBridge::ConnectParty(CBridgePartyCntl* pBridgePartyCntl)
{
	m_pPartyList->Insert(pBridgePartyCntl);

	pBridgePartyCntl->Connect();
}

//--------------------------------------------------------------------------
void CBridge::ConnectParty(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pBridgePartyInitParams));

	//for debug FEEC problem - will remove
	TRACEINTO << "PartyId:" << pBridgePartyInitParams->GetPartyRsrcID() << ", pParty: " << pBridgePartyInitParams->GetParty();

	CSegment* pSeg = new CSegment;

	InitBridgeParams(pBridgePartyInitParams);

	pBridgePartyInitParams->Serialize(NATIVE, *pSeg);

	DispatchEvent(CONNECTPARTY, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridge::DisconnectParty(PartyRsrcID partyId)
{
	CBridgePartyCntl* pPartyCntl = GetPartyCntl(partyId);
	if (pPartyCntl)
		pPartyCntl->DisConnect();
}

//--------------------------------------------------------------------------
void CBridge::ExportParty(PartyRsrcID partyId)
{
	CBridgePartyCntl* pPartyCntl = GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << " - Failed, Party not found";
		return;
	}
	pPartyCntl->Export();
}

//--------------------------------------------------------------------------
void CBridge::Disconnect()
{
	TRACEINTO << "ConfName:" << m_pConfName;

	DispatchEvent(DISCONNECTCONF, NULL);
}

//--------------------------------------------------------------------------
void CBridge::Terminate()
{
	TRACEINTO << "ConfName:" << m_pConfName;

	DispatchEvent(TERMINATE, NULL);
}

//--------------------------------------------------------------------------
void CBridge::InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	((CBridgePartyInitParams*)pBridgePartyInitParams)->SetBridge(this);
	((CBridgePartyInitParams*)pBridgePartyInitParams)->SetConfName(m_pConfName);
	((CBridgePartyInitParams*)pBridgePartyInitParams)->SetConf(m_pConf);
	((CBridgePartyInitParams*)pBridgePartyInitParams)->SetConfRsrcID(m_confRsrcID);
}

//--------------------------------------------------------------------------
void CBridge::DisconnectParty(const CBridgePartyDisconnectParams* pBridgePartyDisconnectParams)
{
	CSegment* pSeg = new CSegment;

	pBridgePartyDisconnectParams->Serialize(NATIVE, *pSeg);

	DispatchEvent(DISCONNECTPARTY, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridge::ExportParty(const CBridgePartyExportParams* pBridgePartyExportParams)
{
	CSegment* pSeg = new CSegment;

	pBridgePartyExportParams->Serialize(NATIVE, *pSeg);

	DispatchEvent(EXPORTPARTY, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridge::EndPartyConnect(CSegment* pParam, WORD responseOpcode)
{
	BYTE responseStatus = statIllegal;

	PartyRsrcID PartyId;
	WORD status;
	EMediaDirection eMediaDirection = eNoDirection;
	*pParam >> PartyId >> status;

	if (IsConnectionDirectionNeeded(responseOpcode))	// Check if this is opcode of audio_connected or video_connected
		*pParam >> (WORD&)eMediaDirection;

	TRACEINTO
		<< "PartyId:" << PartyId
		<< ", MediaDirection:" << MediaDirectionToString(eMediaDirection)
		<< ", status:" << status
		<< ", responseOpcode:" << responseOpcode;

	CParty* pParty = GetLookupTableParty()->Get(PartyId);
	PASSERT_AND_RETURN(!pParty);

	CBridgePartyCntl* pPartyCntl = GetPartyCntl(PartyId);
	DBGPASSERT(!pPartyCntl);

	if (pPartyCntl)
	{
		responseStatus = status;

		if (statOK == responseStatus)
		{
			TRACEINTO << "PartyId: " << PartyId << " - Party connected successfully";
		}
		else
		{
			// do not return response to PartyCntl on Connection Failure - The PartyCntl will receive DisconnectInd instead after the end of disconnection with the connection error status
			PASSERTSTREAM(responseStatus, "PartyId:" << PartyId << " - Party connection problem... Party will be disconnected");

			pPartyCntl->SetConnectionFailureCause(responseStatus);

			// for debug info in case of the "MCU internal problem"
			if ((responseStatus == statAudioInOutResourceProblem) || (responseStatus == statVideoInOutResourceProblem))
			{
				BYTE tempStat;
				BYTE newConnFailureCauseDirection          = eMipNoneDirction;
				BYTE newConnFailureCauseTimerOrStatFailure = eMpiNoTimerAndStatus;
				BYTE newConnFailureCauseAction             = eMipNoAction;

				*pParam >>  tempStat >> newConnFailureCauseDirection >> newConnFailureCauseTimerOrStatFailure >> newConnFailureCauseAction;

				pPartyCntl->SetConnectionFailureCauseAdditionalInfo(newConnFailureCauseDirection, newConnFailureCauseTimerOrStatFailure, newConnFailureCauseAction);
			}

			TRACEINTO << " Connection Error - Video Disconnect Party: " << (DWORD)PartyId;
			CBridgePartyDisconnectParams bridgePartyDisconnectParams(PartyId);
			DisconnectParty(&bridgePartyDisconnectParams);

			return;
		}
	}

	// Return response to PartyCntl
	m_pConfApi->PartyBridgeResponseMsg(pParty, PARTY_MSG,
		responseOpcode, responseStatus, TRUE, eMediaDirection);

	// Indication to Conf Application Manager Party Finished to connect to bridge - Only when connection succeeded
	if (pPartyCntl)
	{
		CSegment* pSeg = new CSegment;
		(pParty->GetRcvMbx()).Serialize(*pSeg);
		m_pConfApi->IvrPartyNotification(PartyId, pParty, pPartyCntl->GetName(), responseOpcode, pSeg, eMediaDirection);
		POBJDELETE(pSeg);
	}
}

//--------------------------------------------------------------------------
BOOL CBridge::IsConnectionDirectionNeeded(WORD responseOpcode)
{
	if ((responseOpcode == PARTY_AUDIO_CONNECTED) || (responseOpcode == PARTY_VIDEO_CONNECTED) || (responseOpcode == PARTY_VIDEO_IVR_MODE_CONNECTED))
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------------
BOOL CBridge::IsDisConnectionDirectionNeeded(WORD responseOpcode)
{
	if ((responseOpcode == PARTY_AUDIO_DISCONNECTED) || (responseOpcode == PARTY_VIDEO_DISCONNECTED))
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------------
void CBridge::EndPartyDisConnect(CSegment* pParam, WORD responseOpcode)
{
	CSegment* pSeg = NULL;

	// for debug info in case of the "MCU internal problem"
	BYTE responseStatus = statIllegal;
	BYTE isStatusFailAdditionalInfoNeeded = NO;
	BYTE failureCauseStat;
	BYTE failureCauseDirection;
	BYTE failureCauseTimerOrStatFailure;
	BYTE failureCauseAction;

	PartyRsrcID PartyId;
	WORD status;

	*pParam >> PartyId >> status;

	DBGPASSERT(status);

	EMediaDirection eMediaDirectionConnected = eNoDirection;
	if (IsDisConnectionDirectionNeeded(responseOpcode))
	{
		// We need to send to the party control what is the state of the bridge - which direction remain connected if any
		*pParam >> (WORD&)eMediaDirectionConnected;
	}

	CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(PartyId);
	PASSERT_AND_RETURN(!pParty);

	CBridgePartyCntl* pPartyCntl = NULL;
	if (eMediaDirectionConnected == eNoDirection)
	{
		// If both direction were disconnected we will remove party from bridge.
		TRACEINTO << "PartyId:" << PartyId << ", MediaDirection:" << eMediaDirectionConnected << ", status:" << status << ", responseOpcode:" << responseOpcode << " - Party disconnected both directions";
		pPartyCntl = (CBridgePartyCntl*)m_pPartyList->Remove(PartyId);
	}
	else if (eMediaDirectionConnected == eMediaIn || eMediaDirectionConnected == eMediaOut)
	{
		// As long as one direction is still connected we won't remove party from bridge
		TRACEINTO << "PartyId:" << PartyId << ", MediaDirection:" << eMediaDirectionConnected << ", status:" << status << ", responseOpcode:" << responseOpcode << " - Party disconnected only one direction";
		pPartyCntl = (CBridgePartyCntl*)m_pPartyList->Find(PartyId);
	}

	TRACECOND(!pPartyCntl, "PartyId:" << PartyId << " - Failed, Party does not found in party list");

	if (pPartyCntl)
	{
		BYTE connectionFailureCause = pPartyCntl->GetConnectionFailureCause();
		// Problem in the connection process
		if (statOK != connectionFailureCause)
		{
			responseStatus = connectionFailureCause;

			if ((responseStatus == statAudioInOutResourceProblem) || (responseStatus == statVideoInOutResourceProblem))
			{
				isStatusFailAdditionalInfoNeeded = YES;
				failureCauseDirection            = pPartyCntl->GetConnectionFailureCauseDirection();
				failureCauseTimerOrStatFailure   = pPartyCntl->GetConnectionFailureCauseTimerOrStatFailure();
				failureCauseAction               = pPartyCntl->GetConnectionFailureCausAction();
			}
		}
		else
		{
			responseStatus = status;
			// Problem in the Disconnection process
			if ((responseStatus == statAudioInOutResourceProblem) || (responseStatus == statVideoInOutResourceProblem))
			{
				isStatusFailAdditionalInfoNeeded = YES;
				*pParam >> failureCauseStat >> failureCauseDirection >> failureCauseTimerOrStatFailure >> failureCauseAction;

				PASSERTSTREAM(1, "PartyId:" << PartyId <<
						", ResponseStatus:" << (int)responseStatus <<
						", FailureCauseStat:" << (int)failureCauseStat <<
						", FailureCauseDirection:" << (int)failureCauseDirection <<
						", FailureCauseTimerOrStatFailure:" << (int)failureCauseTimerOrStatFailure <<
						", FailureCauseAction:" << (int)failureCauseAction);
			}
		}
		// Indication to Conf Application Manager Party Finished to disconnect to bridge
		m_pConfApi->IvrPartyNotification(PartyId, pParty, pPartyCntl->GetName(), responseOpcode, NULL, eMediaDirectionConnected);

		if (eMediaDirectionConnected == eNoDirection)
			POBJDELETE(pPartyCntl);
	}

	// Return response to PartyCntl
	if (isStatusFailAdditionalInfoNeeded)
	{
		pSeg = new CSegment;
		*pSeg << (BYTE)failureCauseDirection << (BYTE)failureCauseTimerOrStatFailure << (BYTE)failureCauseAction;
	}

	m_pConfApi->PartyBridgeResponseMsg(pParty, PARTY_MSG, responseOpcode, responseStatus, TRUE, eMediaDirectionConnected, pSeg);

	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridge::EndPartyExport(CSegment* pParam, WORD responseOpcode)
{
	CParty* pParty = NULL;
	WORD status = statIllegal;
	BYTE responseStatus = statIllegal;
	BOOL isPartyCntlMsg = TRUE;

	*pParam >> (void*&)pParty >> status;

	PASSERT_AND_RETURN(!pParty);

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	if (status)
	{
		// Return response to PartyCntl
		TRACEINTO << "PartyId" << partyId << ", Status:" << status;
		m_pConfApi->SendResponseMsg(pParty, PARTY_MSG, responseOpcode, status, isPartyCntlMsg, NULL);
		return;
	}

	CBridgePartyCntl* pPartyCntl = (CBridgePartyCntl*)m_pPartyList->Remove(partyId);
	PASSERTSTREAM_AND_RETURN(!pPartyCntl, "PartyId:" << partyId);

	TRACEINTO << "PartyId:" << partyId << " - Party exported";

	responseStatus = status;

	pPartyCntl->RemoveConfParams();

	CSegment seg;
	seg << (DWORD)pPartyCntl;

	// Return response to PartyCntl
	m_pConfApi->SendResponseMsg(pParty, PARTY_MSG, responseOpcode, responseStatus, isPartyCntlMsg, &seg);
	m_pConfApi->IvrPartyNotification(partyId, pParty, pPartyCntl->GetName(), responseOpcode, NULL);
}

//--------------------------------------------------------------------------
void CBridge::RejectPartyConnect(const CTaskApp* pParty, WORD responseOpcode, BYTE rejectReason)
{
	if (pParty)
		TRACEINTO << "PartyId:" << pParty->GetPartyId() << ", responseOpcode:" << responseOpcode << ", rejectReason:" << (int)rejectReason;

	// Return response to PartyCntl
	m_pConfApi->PartyBridgeResponseMsg(pParty, PARTY_MSG, responseOpcode, rejectReason, TRUE);
}

//--------------------------------------------------------------------------
void CBridge::AllocatePartyList()
{
	if (m_pPartyList)
	{
		m_pPartyList->ClearAndDestroy();
		POBJDELETE(m_pPartyList);
	}

	m_pPartyList = new CBridgePartyList;
}

//--------------------------------------------------------------------------
void CBridge::DumpParties()
{
	TRACEINTO << *m_pPartyList;
}

