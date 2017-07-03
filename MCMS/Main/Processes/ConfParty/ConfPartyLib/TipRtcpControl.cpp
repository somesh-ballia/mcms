/*
 * TipRtcpControl.cpp
 *
 *  Created on: Feb 15, 2011
 *      Author: shmuell
 */
#include "TipRtcpControl.h"
#include "ConfPartyOpcodes.h"
#include "OpcodesRanges.h"
#include "OpcodesMcmsCardMngrTIP.h"
#include "Trace.h"
#include "TraceHeader.h"
#include "ObjString.h"

const WORD TIP_NEGOTIATION_TIME = 18; // TIP Lib (Emb) wait 15 seconds according to standard. MCMS timer is 3 seconds later, in case of problem in Emb timer.

PBEGIN_MESSAGE_MAP(CTipRtcpCntl)
// tip Negotiation
ONEVENT(SIP_PARTY_START_TIP_NEGOTIATION,		IDLE,								CTipRtcpCntl::OnPartyStartTipNegotiation)
ONEVENT(IP_CM_TIP_NEGOTIATION_RESULT_IND,		sTIP_START_NEGOTIATION_SENT,		CTipRtcpCntl::OnNegotiationResultInd)
ONEVENT(IP_CM_TIP_LAST_ACK_RECEIVED_IND,		sTIP_START_NEGOTIATION_SENT,		CTipRtcpCntl::OnTipLastAckReceivedInd)
ONEVENT(SIP_PARTY_NEGOTIATION_HANDLED,		sTIP_START_NEGOTIATION_SENT,			CTipRtcpCntl::OnPartyNegotiationHandled)
ONEVENT(SIP_PARTY_NEGOTIATION_HANDLED,			sTIP_NEGOTIATION_RESULT_RECEIVED,	CTipRtcpCntl::OnPartyNegotiationHandled)
ONEVENT(IP_CM_TIP_LAST_ACK_RECEIVED_IND,		sTIP_NEGOTIATION_RESULT_RECEIVED,	CTipRtcpCntl::OnTipLastAckReceivedInd)


// close tip session
ONEVENT(SIP_PARTY_CLOSE_TIP_SESSION,			IDLE,								CTipRtcpCntl::OnPartyCloseTipSessionIdle)
ONEVENT(SIP_PARTY_CLOSE_TIP_SESSION,			ANYCASE,							CTipRtcpCntl::OnPartyCloseTipSessionAnycase)
// error handling tip negotiation:
ONEVENT(TIP_NEGOTIATION_TOUT,					sTIP_START_NEGOTIATION_SENT,		CTipRtcpCntl::OnNegotiationTout)
ONEVENT(TIP_NEGOTIATION_TOUT,					sTIP_NEGOTIATION_RESULT_RECEIVED,	CTipRtcpCntl::OnNegotiationTout)
ONEVENT(SIP_PARTY_NEGOTIATION_HANDLED,			IDLE,								CTipRtcpCntl::OnPartyNegotiationHandledIdle)
ONEVENT(IP_CM_TIP_NEGOTIATION_RESULT_IND,		sTIP_NEGOTIATION_FAILED,			CTipRtcpCntl::OnNegotiationResultIndNegotiationFailed)
ONEVENT(SIP_PARTY_NEGOTIATION_HANDLED,			sTIP_NEGOTIATION_FAILED,			CTipRtcpCntl::OnPartyNegotiationHandledNegotiationFailed)

// Content messages
ONEVENT(IP_CM_TIP_CONTENT_MSG_IND,				sTIP_NEGOTIATION_COMPLETED,			CTipRtcpCntl::OnContentMsgInd)
ONEVENT(IP_CM_TIP_CONTENT_MSG_IND,				ANYCASE,							CTipRtcpCntl::OnContentMsgIndAnycase)


PEND_MESSAGE_MAP(CTipRtcpCntl,CStateMachine);

////////////////////////////////////////////////////////////////////////////
CTipRtcpCntl::CTipRtcpCntl(CTaskApp *pOwnerTask) : CStateMachine(pOwnerTask)
{
	m_pMfaInterface = NULL;
	m_pPartyApi = NULL;
	m_pLocalNegotiationParams = NULL;
	m_pNegotiationResultParams = NULL;
    m_mcuNum = 0;
    m_termNum = 0;
    m_bWaitForWithdrawAck = FALSE;
    m_bIsRequestSent = FALSE;
    m_LocalAuxFPS = eTipAuxNone;

	m_bLastAckReceived = FALSE;
	m_bEndSuccessNegSent = FALSE;

	VALIDATEMESSAGEMAP;
}
////////////////////////////////////////////////////////////////////////////
CTipRtcpCntl::~CTipRtcpCntl()
{
	POBJDELETE(m_pLocalNegotiationParams);
	POBJDELETE(m_pNegotiationResultParams);
}
////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, WORD mcuNum, WORD termNum, ETipAuxFPS localAuxFPS)
{
	m_pMfaInterface = pMfaInterface;
	m_pPartyApi = pPartyApi;
    m_mcuNum = mcuNum;
    m_termNum = termNum;
    SetLocalAuxFPS(localAuxFPS);
}
////////////////////////////////////////////////////////////////////////////
void  CTipRtcpCntl::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}
////////////////////////////////////////////////////////////////////////////
DWORD CTipRtcpCntl::SendMsgToMpl(BYTE* pStructure, int structureSize, DWORD opcode)
{
	if (!IsValidPObjectPtr(m_pMfaInterface))
	{
		DBGPASSERT(1);
		return 0;
	}

	CSegment* pMsg = NULL;
	if (pStructure)
	{
		pMsg = new CSegment;
		pMsg->Put(pStructure ,structureSize);
	}

	DWORD seqNum = 0;
	seqNum = m_pMfaInterface->SendMsgToMPL(opcode,pMsg);

	POBJDELETE(pMsg);
	return seqNum;
}
////////////////////////////////////////////////////////////////////////////
void  CTipRtcpCntl::OnPartyStartTipNegotiation(CSegment *pParam)
{
	PTRACE(eLevelInfoNormal, "CTipRtcpCntl::OnPartyStartTipNegotiation");
	m_pLocalNegotiationParams = new TipNegotiationSt;
	memset(m_pLocalNegotiationParams, 0, sizeof(TipNegotiationSt));
	SetLocalNegotiationParams();

	APIU32 isPreferTip = 0;         // TIP call from Polycom EPs feature (CTipRtcpCntl::OnPartyStartTipNegotiation)
	*pParam >> isPreferTip;

	SendStartNegotiationReq(isPreferTip);
}

////////////////////////////////////////////////////////////////////////////
void  CTipRtcpCntl::OnTipLastAckReceivedInd(CSegment *pParam)
{
	TRACEINTO << "bEndSuccessNegSent : " << (int)m_bEndSuccessNegSent;

	if (!IsValidPObjectPtr(m_pPartyApi))
	{
		DBGPASSERT(1);
		return;
	}

	m_pPartyApi->TipLastAckReceived();

	m_bLastAckReceived = TRUE;

	if(m_bEndSuccessNegSent && m_state == sTIP_NEGOTIATION_RESULT_RECEIVED)
	{
		m_state = sTIP_NEGOTIATION_COMPLETED;
		DeleteTimer(TIP_NEGOTIATION_TOUT);
	}
}
////////////////////////////////////////////////////////////////////////////
void  CTipRtcpCntl::OnNegotiationResultInd(CSegment *pParam)
{
	APIU32 status;                 	// negotiation status
	APIU32 doVideoReInvite;         // Decision about Re-Invite initiator is done by TIP library
	TipNegotiationSt negotiationSt;  // (see structure below)
	*pParam >> status;
	*pParam >> doVideoReInvite;

	TRACEINTO << " status = " << status << ", doVideoReInvite " << doVideoReInvite;
	//DeleteTimer(TIP_NEGOTIATION_TOUT);

	TipNegotiationSt* pTipNegotiationSt = (TipNegotiationSt*)pParam->GetPtr(1);
	POBJDELETE(m_pNegotiationResultParams);
	if (pTipNegotiationSt)
	{
		m_pNegotiationResultParams = new TipNegotiationSt;
		memcpy(m_pNegotiationResultParams, pTipNegotiationSt, sizeof(TipNegotiationSt));
	}
	if (!IsValidPObjectPtr(m_pPartyApi))
	{
		DBGPASSERT(1);
		return;
	}
	WORD numOfStreams 	= GetNegotiationResultNumOfStreams(status);
	BYTE bIsAudioAux 	= GetNegotiationResultAudioAux();
	BYTE bIsVideoAux 	= IsAuxVideo5FpsNegotiated();

	m_pPartyApi->TipNegotiationResult(status, numOfStreams, bIsAudioAux, bIsVideoAux, doVideoReInvite);
	m_state = sTIP_NEGOTIATION_RESULT_RECEIVED;
}
////////////////////////////////////////////////////////////////////////////
void  CTipRtcpCntl::SendStartNegotiationReq(APIU32 isPreferTip)
{
	if ( IsValidTimer(TIP_NEGOTIATION_TOUT) )
	{
		PTRACE(eLevelInfoNormal, "CTipRtcpCntl::SendStartNegotiationReq: timer is active");
		DeleteTimer(TIP_NEGOTIATION_TOUT);
	}
	StartTimer(TIP_NEGOTIATION_TOUT, TIP_NEGOTIATION_TIME * SECOND);
	mcTipStartNegotiationReq* pStruct = new mcTipStartNegotiationReq;
	memset(pStruct,0,sizeof(mcTipStartNegotiationReq));

	//for TIP call from Polycom EPs feature:
	pStruct->isPreferTip = isPreferTip;
	PTRACE2INT(eLevelInfoNormal, "IS_PREFER_TIP_MODE: CTipRtcpCntl::SendStartNegotiationReq isPreferTip:",isPreferTip);

	memcpy(&(pStruct->negotiationSt), m_pLocalNegotiationParams, sizeof(TipNegotiationSt));
	SendMsgToMpl((BYTE*)(pStruct), sizeof(mcTipStartNegotiationReq), IP_CM_TIP_START_NEGOTIATION_REQ);
	PDELETE(pStruct);

	m_state = sTIP_START_NEGOTIATION_SENT;
}
////////////////////////////////////////////////////////////////////////////
void  CTipRtcpCntl::SendEndNegotiationReq(ETipNegStatus status)
{
	mcTipEndNegotiationReq* pStruct = new mcTipEndNegotiationReq;
	memset(pStruct,0,sizeof(mcTipEndNegotiationReq));
	pStruct->status = status;

	SendMsgToMpl((BYTE*)(pStruct), sizeof(mcTipEndNegotiationReq), IP_CM_TIP_END_NEGOTIATION_REQ);
	PDELETE(pStruct);
}
////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::OnPartyNegotiationHandled(CSegment *pParam)
{
	DWORD status = eTipNegSuccess;
	*pParam >> status;

	TRACEINTO << " status : "<< status << " bLastAckReceived : " << (int)m_bLastAckReceived;

	SendEndNegotiationReq((ETipNegStatus)status);

	if (status == eTipNegSuccess)
	{
		m_bEndSuccessNegSent = TRUE;

		if(m_bLastAckReceived)
		{
			m_state = sTIP_NEGOTIATION_COMPLETED;
			DeleteTimer(TIP_NEGOTIATION_TOUT);
		}
	}
	else // error state
	{
		m_state = sTIP_NEGOTIATION_FAILED;
		DeleteTimer(TIP_NEGOTIATION_TOUT);
	}

	//if (status == eTipNegSuccess)
	//	m_state = sTIP_NEGOTIATION_COMPLETED;
	//else // error state
	//	m_state = sTIP_NEGOTIATION_FAILED;
	//DeleteTimer(TIP_NEGOTIATION_TOUT);
}
////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::OnPartyNegotiationHandledIdle(CSegment *pParam)
{
	DWORD status = eTipNegSuccess;
	*pParam >> status;
	PTRACE2INT(eLevelInfoNormal, "CTipRtcpCntl::OnPartyNegotiationHandledIdle : status = ", status);
	DBGPASSERT(1);
}
////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::OnPartyNegotiationHandledNegotiationFailed(CSegment *pParam)
{
	PTRACE(eLevelInfoNormal, "CTipRtcpCntl::OnPartyNegotiationHandledNegotiationFailed");
}
////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::OnNegotiationResultIndNegotiationFailed(CSegment *pParam)
{
	PTRACE(eLevelInfoNormal, "CTipRtcpCntl::OnNegotiationResultIndNegotiationFailed");
}
////////////////////////////////////////////////////////////////////////////
void  CTipRtcpCntl::SetLocalNegotiationParams()
{
	SetLocalSystemWideOptions();
	SetLocalAudioMuxParams();
	SetLocalVideoMuxParams();
	SetLocalAudioOptionsParams();
	SetLocalVideoOptionsParams();
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::SetLocalSystemWideOptions()
{
	TipSystemWideOptionsSt* pSystemWideOptions = &(m_pLocalNegotiationParams->systemwideOptions);
	memset(pSystemWideOptions, 0, sizeof(TipSystemWideOptionsSt));
	pSystemWideOptions->rtpProfile 		= eRtpAvpf; // In audio session should be AVP.
	pSystemWideOptions->deviceOptions 	= eTipDeviceNone; //eTipDeviceTranscoding;
	pSystemWideOptions->confId 			= 0; // tiptmp put conf id
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::SetLocalAudioMuxParams()
{
	TipAudioMuxCntlSt* pAudioMux = &(m_pLocalNegotiationParams->audioMuxCntl);
	memset(pAudioMux, 0, sizeof(TipAudioMuxCntlSt));

	pAudioMux->txPositions[eTipAudioPosCenter] 	= 1;
	pAudioMux->txPositions[eTipAudioPosLeft] 	= 1; // Triple
	pAudioMux->txPositions[eTipAudioPosRight] 	= 1; // Triple
	pAudioMux->txPositions[eTipAudioPosAux] 	= 1; // Auxiliary

	pAudioMux->rxPositions[eTipAudioPosCenter] 	= 1;
	pAudioMux->rxPositions[eTipAudioPosLeft] 	= 1;
	pAudioMux->rxPositions[eTipAudioPosRight] 	= 1;
	pAudioMux->rxPositions[eTipAudioPosAux] 	= 1; // Auxiliary
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::SetLocalVideoMuxParams()
{
	TipVideoMuxCntlSt* pVideoMux = &(m_pLocalNegotiationParams->videoMuxCntl);
	memset(pVideoMux, 0, sizeof(TipVideoMuxCntlSt));

	pVideoMux->txPositions[eTipVideoPosCenter]	= 1;
	pVideoMux->txPositions[eTipVideoPosLeft]	= 1;	// Triple
	pVideoMux->txPositions[eTipVideoPosRight]	= 1;	// Triple
	if (IsLocalSupportVideoAuxiliary())
		pVideoMux->txPositions[eTipVideoPosAux5Fps]	= 1;	// Auxiliary

	pVideoMux->rxPositions[eTipVideoPosCenter]	= 1;
	pVideoMux->rxPositions[eTipVideoPosLeft]	= 1;	// Triple
	pVideoMux->rxPositions[eTipVideoPosRight]	= 1;	// Triple
	if (IsLocalSupportVideoAuxiliary())
		pVideoMux->rxPositions[eTipVideoPosAux5Fps]	= 1;	// Auxiliary
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::SetLocalAudioOptionsParams()
{
	// Tx:
	TipAudioOptionsSt* pAudioOptionTx = &(m_pLocalNegotiationParams->audioTxOptions);
	memset(pAudioOptionTx, 0, sizeof(TipAudioOptionsSt));
	pAudioOptionTx->IsAudioDynamicOutput 	= 0;
	pAudioOptionTx->IsAudioActivityMetric 	= 1;
	pAudioOptionTx->G722Negotiation 		= 0;
	pAudioOptionTx->genericOptions.IsEKT 	= 0;

	// Rx:
	TipAudioOptionsSt* pAudioOptionRx = &(m_pLocalNegotiationParams->audioRxOptions);
	memset(pAudioOptionRx, 0, sizeof(TipAudioOptionsSt));
	pAudioOptionRx->IsAudioDynamicOutput 	= 1;
	pAudioOptionRx->IsAudioActivityMetric 	= 0;
	pAudioOptionRx->G722Negotiation 		= 0;
	pAudioOptionRx->genericOptions.IsEKT 	= 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::SetLocalVideoOptionsParams()
{
	// Tx:
	TipVideoOptionsSt* pVideoOptionTx = &(m_pLocalNegotiationParams->videoTxOptions);
	memset(pVideoOptionTx, 0, sizeof(TipVideoOptionsSt));
	pVideoOptionTx->IsVideoRefreshFlag 			= 1;
	pVideoOptionTx->IsInbandParameterSets 		= 1;
	pVideoOptionTx->IsCABAC 					= 1;
	pVideoOptionTx->IsLTRP 						= 0;
	pVideoOptionTx->AuxVideoFPS					= eTipAuxNone;
	if (IsLocalSupportVideoAuxiliary())
		pVideoOptionTx->AuxVideoFPS = GetLocalAuxFPS();
	pVideoOptionTx->IsGDR 						= 0;
	pVideoOptionTx->IsHighProfile 				= 0;
	pVideoOptionTx->IsUnrestrictedMediaXGA5or1 	= 0;
	pVideoOptionTx->IsUnrestrictedMediaXGA30 	= 0;
	pVideoOptionTx->IsUnrestrictedMedia720p 	= 0;
	pVideoOptionTx->IsUnrestrictedMedia1080p 	= 0;
	pVideoOptionTx->genericOptions.IsEKT 		= 0;

	// Rx:
	TipVideoOptionsSt* pVideoOptionRx = &(m_pLocalNegotiationParams->videoRxOptions);
	memset(pVideoOptionRx, 0, sizeof(TipVideoOptionsSt));
	pVideoOptionRx->IsVideoRefreshFlag 			= 0;
	pVideoOptionRx->IsInbandParameterSets 		= 1;
	pVideoOptionRx->IsCABAC 					= 1;
	pVideoOptionRx->IsLTRP 						= 0;
	pVideoOptionRx->AuxVideoFPS					= eTipAuxNone;
	if (IsLocalSupportVideoAuxiliary())
		pVideoOptionRx->AuxVideoFPS = GetLocalAuxFPS();
	pVideoOptionRx->IsGDR 						= 0;
	pVideoOptionRx->IsHighProfile 				= 0;
	pVideoOptionRx->IsUnrestrictedMediaXGA5or1 	= 0;
	pVideoOptionRx->IsUnrestrictedMediaXGA30 	= 0;
	pVideoOptionRx->IsUnrestrictedMedia720p 	= 1;
	pVideoOptionRx->IsUnrestrictedMedia1080p 	= 0;
	pVideoOptionRx->genericOptions.IsEKT 		= 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////
WORD  CTipRtcpCntl::GetNegotiationResultNumOfStreams(APIU32 status)
{
	WORD numOfStreams = 0;

	if (m_pNegotiationResultParams)
	{
		BYTE * audioTxPos = m_pNegotiationResultParams->audioMuxCntl.txPositions;
		BYTE * audioRxPos = m_pNegotiationResultParams->audioMuxCntl.rxPositions;
		BYTE * videoTxPos = m_pNegotiationResultParams->videoMuxCntl.txPositions;
		BYTE * videoRxPos = m_pNegotiationResultParams->videoMuxCntl.rxPositions;

		if (audioRxPos[eTipVideoPosCenter] && audioTxPos[eTipVideoPosCenter]
		  && videoRxPos[eTipVideoPosCenter] && videoTxPos[eTipVideoPosCenter])
		{
			numOfStreams = 1;
			if ((audioRxPos[eTipAudioPosLeft] && audioRxPos[eTipAudioPosRight])  // 3 audio rx
			 /*|| (audioTxPos[eTipAudioPosLeft] && audioTxPos[eTipAudioPosRight])  // 3 audio tx*/
			 || (videoRxPos[eTipVideoPosLeft] && videoRxPos[eTipVideoPosRight])  // 3 video rx
			 || (videoTxPos[eTipVideoPosLeft] && videoTxPos[eTipVideoPosRight])) // 3 video tx
			{
				numOfStreams = 3;
			}
		}
		else if (status == eTipNegError) // case EP is not Polycom, and TIP negotiation failed.
			DBGPASSERT(1);
	}

	PTRACE2INT(eLevelInfoNormal,"CTipRtcpCntl::GetNegotiationResultNumOfStreams : ", numOfStreams);
	return numOfStreams;
}
////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CTipRtcpCntl::GetNegotiationResultAudioAux()
{
	BYTE audioAux = FALSE;

	if (m_pNegotiationResultParams)
	{
		BYTE * audioTxPos = m_pNegotiationResultParams->audioMuxCntl.txPositions;
		BYTE * audioRxPos = m_pNegotiationResultParams->audioMuxCntl.rxPositions;

		if (audioRxPos[eTipAudioPosAux] || audioTxPos[eTipAudioPosAux])
			audioAux = TRUE;
	}

	PTRACE2INT(eLevelInfoNormal,"CTipRtcpCntl::GetNegotiationResultAudioAux", audioAux);
	return audioAux;
}
////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CTipRtcpCntl::IsAuxVideo5FpsNegotiated()
{
	BYTE videoAux = FALSE;

	if (m_pNegotiationResultParams)
	{
//      cts bug- aux position isn't declared.
//
//		BYTE * videoTxPos = m_pNegotiationResultParams->videoMuxCntl.txPositions;
//		BYTE * videoRxPos = m_pNegotiationResultParams->videoMuxCntl.rxPositions;
//
//		if ((videoRxPos[eTipVideoPosAux5Fps] && videoTxPos[eTipVideoPosAux5Fps])
//		 || (videoRxPos[eTipVideoPosAux30Fps] && videoTxPos[eTipVideoPosAux30Fps]))
//			videoAux = TRUE;
		if (((m_pNegotiationResultParams->videoTxOptions.AuxVideoFPS == eTipAux5FPS) || (m_pNegotiationResultParams->videoTxOptions.AuxVideoFPS == eTipAux30FPS))
			&& ((m_pNegotiationResultParams->videoRxOptions.AuxVideoFPS == eTipAux5FPS) || (m_pNegotiationResultParams->videoRxOptions.AuxVideoFPS == eTipAux30FPS)))
			videoAux = TRUE;
	}

	PTRACE2INT(eLevelInfoNormal,"CTipRtcpCntl::IsAuxVideo5FpsNegotiated: ", videoAux);
	return videoAux;
}
////////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CTipRtcpCntl::GetNegotiationResultHighProfile()
{
	BYTE bIsHighProfile = FALSE;
	if (m_pNegotiationResultParams)
	{
		if (m_pNegotiationResultParams->videoTxOptions.IsHighProfile
		 && m_pNegotiationResultParams->videoRxOptions.IsHighProfile)
			bIsHighProfile = TRUE;
	}
	return bIsHighProfile;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::OnNegotiationTout(CSegment *pParam)
{
	PTRACE(eLevelInfoNormal,"CTipRtcpCntl::OnNegotiationTout");
	if (!IsValidPObjectPtr(m_pPartyApi))
	{
		DBGPASSERT(1);
		return;
	}
	DWORD status 		= eTipNegError;
	WORD numOfStreams 	= 0;
	BYTE bIsAudioAux 	= FALSE;
	BYTE bIsVideoAux 	= FALSE;
	DWORD doVideoReInvite 	= 0;

	m_pPartyApi->TipNegotiationResult(status, numOfStreams, bIsAudioAux, bIsVideoAux, doVideoReInvite);
	m_state = sTIP_NEGOTIATION_FAILED;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::OnPartyCloseTipSessionIdle(CSegment *pParam)
{
	PTRACE(eLevelInfoNormal,"CTipRtcpCntl::OnPartyCloseTipSessionIdle - no need to close session");
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::OnPartyCloseTipSessionAnycase(CSegment *pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CTipRtcpCntl::OnPartyCloseTipSessionAnycase : state=", m_state);
	SendMsgToMpl(NULL, 0, IP_CM_TIP_KILL_TIP_CONTEXT_REQ);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::OnContentMsgInd(CSegment *pParam)
{
	PTRACE(eLevelInfoNormal,"CTipRtcpCntl::OnContentMsgInd");
	TipContentMsgSt* pContentMsgSt = (TipContentMsgSt*)pParam->GetPtr(1);

	if (IsAuxVideo5FpsNegotiated())
	{
		OPCODE opcode = INVALID_OPCODE;
		EAuxCntlSubOpcode eSubOpcode = (EAuxCntlSubOpcode)pContentMsgSt->subOpcode;
		if (pContentMsgSt->auxPositions[eTipAuxPosition1or5FPS] || eSubOpcode==AuxCntlRelease/*release if for all existing positions*/)
		{
			BYTE bIgnoreMsg = FALSE;
			switch (eSubOpcode)
			{
				case AuxCntlRequest:	// EP request token
					opcode = PARTY_TOKEN_ACQUIRE;
					break;
				case AuxCntlRelease:	// EP release token
					opcode = PARTY_TOKEN_RELEASE;
					break;
				case AuxCntlReleaseAck:	// EP ack our release req. Ignore.
					bIgnoreMsg = TRUE;
					break;
				case AuxCntlRequestGranted: // 1. EP ack our request req. Ignore.
											// 2. EP ack our request req when it holded the token (withdraw ack).
					if (m_bWaitForWithdrawAck)
					{
						opcode = PARTY_TOKEN_WITHDRAW_ACK;
						m_bWaitForWithdrawAck = FALSE;
					}
					else
						bIgnoreMsg = TRUE;
					break;
				case AuxCntlRequestDenied:
					DBGPASSERT(1);      // EP reject our request. Should not happen (EP must agree to our token request). Ignore.
					bIgnoreMsg = TRUE;
					break;
				case AuxCntlMsgUnknown:
				default:
					DBGPASSERT(1);
					bIgnoreMsg = TRUE;
					break;

			}
			if (!bIgnoreMsg)
				m_pPartyApi->TipContentMessageInd(opcode, STATUS_OK, m_mcuNum, m_termNum);
			else
				PTRACE(eLevelInfoNormal,"CTipRtcpCntl::OnContentMsgInd : Ignore message");
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CTipRtcpCntl::OnContentMsgInd - Not 5FPS, ignore message");
			DBGPASSERT(1);
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CTipRtcpCntl::OnContentMsgInd - 5FPS wasn't negotiated, ignore message");
		DBGPASSERT(1);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::OnContentMsgIndAnycase(CSegment *pParam)
{
	PTRACE(eLevelInfoNormal,"CTipRtcpCntl::OnContentMsgIndAnycase - negotiation wasn't completed, ignore message");
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::SetIsRequestSent(BYTE bIsRequestSent)
{
	PTRACE2INT(eLevelInfoNormal,"CTipRtcpCntl::SetIsRequestSent - ", bIsRequestSent);
	m_bIsRequestSent = bIsRequestSent;
}
////////////////////////////////////////////////////////////////////////////
void CTipRtcpCntl::SendContentMsgReq(DWORD opcode)
{
	PTRACE2INT(eLevelInfoNormal,"CTipRtcpCntl::SendContentMsgReq : opcode=", opcode);
	EAuxCntlSubOpcode eSubOpcode = AuxCntlMsgUnknown;

	switch (opcode)
	{
		case CONTENT_ROLE_TOKEN_ACQUIRE_ACK:
			eSubOpcode = AuxCntlRequestGranted;
			break;
		case CONTENT_ROLE_TOKEN_ACQUIRE_NAK:
			eSubOpcode = AuxCntlRequestDenied;
			break;
		case CONTENT_ROLE_TOKEN_WITHDRAW:
			eSubOpcode = AuxCntlRequest;
			m_bWaitForWithdrawAck = TRUE;
			break;
		case CONTENT_ROLE_TOKEN_RELEASE_ACK:
			// eSubOpcode = AuxCntlReleaseAck. TipLib sends the Ack by itself and doesn't wait for our Ack.
			break;
		case CONTENT_ROLE_PROVIDER_IDENTITY:
			eSubOpcode = AuxCntlRequest;
			break;
		case CONTENT_MEDIA_PRODUCER_STATUS:
			// This opcode isn't supposed to arrive
			break;
		case CONTENT_NO_ROLE_PROVIDER:
			eSubOpcode = AuxCntlRelease;
			break;
		default:
			DBGPASSERT(1);
			break;

	}
	PTRACE2INT(eLevelInfoNormal,"CTipRtcpCntl::SendContentMsgReq : SubOpcode=", eSubOpcode);

	if (GetIsRequestSent() && eSubOpcode == AuxCntlRequest) // In order to avoid sending it periodically that isn't needed for TIP.
		eSubOpcode = AuxCntlMsgUnknown;

	if (eSubOpcode != AuxCntlMsgUnknown)
	{
		mcTipContentMsgReq* pStruct = new mcTipContentMsgReq;
		memset(pStruct, 0, sizeof(mcTipContentMsgReq));
		TipContentMsgSt* pContentMsgSt = &(pStruct->ContentMsgSt);
		pContentMsgSt->subOpcode = eSubOpcode;
		pContentMsgSt->auxPositions[eTipAuxPosition1or5FPS] = TRUE;

		SendMsgToMpl((BYTE*)(pStruct), sizeof(mcTipContentMsgReq), IP_CM_TIP_CONTENT_MSG_REQ);
		PDELETE(pStruct);

		if (eSubOpcode == AuxCntlRequest)
			SetIsRequestSent(TRUE);
		else
			SetIsRequestSent(FALSE);
	}


}
////////////////////////////////////////////////////////////////////////////
void  CTipRtcpCntl::SetLocalAuxFPS(ETipAuxFPS localAuxFPS)
{
	PTRACE2INT(eLevelInfoNormal, "CTipRtcpCntl::SetLocalAuxFPS : ", localAuxFPS);
	m_LocalAuxFPS = localAuxFPS;
}
////////////////////////////////////////////////////////////////////////////
BYTE CTipRtcpCntl::IsLocalSupportVideoAuxiliary() const
{
	return (GetLocalAuxFPS() != eTipAuxNone);
}
//////////////////////////////////////////////////////////////////////////

