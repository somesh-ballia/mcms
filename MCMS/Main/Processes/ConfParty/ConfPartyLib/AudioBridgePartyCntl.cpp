//+========================================================================+
//                   AudioBridgePartyCntl.CPP                              |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       AudioBridgePartyCntl.CPP                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  Aug-2005  | Description                                    |
//-------------------------------------------------------------------------|

#include "AudioBridgePartyCntl.h"
#include "BridgePartyAudioInOut.h"
#include "BridgePartyAudioParams.h"
#include "StatusesGeneral.h"
#include "ConfAppBridgeParams.h"
#include "ConfPartyGlobals.h"

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );


// Timers opcodes
#define		AUDIO_BRDG_PARTY_SETUP_TOUT		((WORD)100)
#define		AUDIO_BRDG_PARTY_DISCONNECT_TOUT	((WORD)101)

// Time-out values
#define		AUDIO_BRDG_PARTY_SETUP_TOUT_VALUE		2500 // BRIDGE-7608 - change to 25 seconds. was 700 , vngfe-4753 dsp send close port after recovery - 10 second
#define		AUDIO_BRDG_PARTY_DISCONNECT_TOUT_VALUE	2500 // BRIDGE-7608 - change to 25 seconds. VNVG-24004 set to 20 to avoid kill port -was 1500 was 500  , vngfe-4753 dsp send close port after recovery - 10 second


PBEGIN_MESSAGE_MAP(CAudioBridgePartyCntl)

	ONEVENT(AUDCONNECT,							IDLE,			CAudioBridgePartyCntl::OnAudioBridgeConnectIDLE)
	ONEVENT(AUDCONNECT,							SETUP,		    CAudioBridgePartyCntl::OnAudioBridgeConnectSETUP)
	ONEVENT(AUDCONNECT,							CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeConnectCONNECTED)
	ONEVENT(AUDCONNECT,							DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeConnectDISCONNECTING)

	ONEVENT(AUDDISCONNECT,						IDLE,			CAudioBridgePartyCntl::NullActionFunction)
	ONEVENT(AUDDISCONNECT,						SETUP,			CAudioBridgePartyCntl::OnAudioBridgeDisConnectSETUP)
	ONEVENT(AUDDISCONNECT,						CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeDisConnectCONNECTED)
	ONEVENT(AUDDISCONNECT,						DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeDisConnectDISCONNECTING)

	ONEVENT(AUDIO_EXPORT,						SETUP,			CAudioBridgePartyCntl::OnAudioBridgeExportSETUP)
	ONEVENT(AUDIO_EXPORT,						CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeExportCONNECTED)
	ONEVENT(AUDIO_EXPORT,						DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeExportDISCONNECTING)

	ONEVENT(AUDIO_IN_CONNECTED,					SETUP,			CAudioBridgePartyCntl::OnAudioInConnectedSETUP)
	ONEVENT(AUDIO_IN_CONNECTED,					CONNECTED,		CAudioBridgePartyCntl::OnAudioInConnectedCONNECTED)
	ONEVENT(AUDIO_IN_CONNECTED,					DISCONNECTING,	CAudioBridgePartyCntl::OnAudioInConnectedDISCONNECTING)

	ONEVENT(AUDIO_IN_DISCONNECTED,				SETUP,			CAudioBridgePartyCntl::OnAudioInDisConnectedSETUP)
	ONEVENT(AUDIO_IN_DISCONNECTED,				DISCONNECTING,	CAudioBridgePartyCntl::OnAudioInDisConnectedDISCONNECTING)
	ONEVENT(AUDIO_IN_DISCONNECTED,				CONNECTED,	    CAudioBridgePartyCntl::OnAudioInDisConnectedCONNECTED)

	ONEVENT(AUDIO_OUT_CONNECTED,				SETUP,			CAudioBridgePartyCntl::OnAudioOutConnectedSETUP)
	ONEVENT(AUDIO_OUT_CONNECTED,				CONNECTED,		CAudioBridgePartyCntl::OnAudioOutConnectedCONNECTED)
	ONEVENT(AUDIO_OUT_CONNECTED,				DISCONNECTING,	CAudioBridgePartyCntl::OnAudioOutConnectedDISCONNECTING)

	ONEVENT(AUDIO_OUT_DISCONNECTED,				SETUP,			CAudioBridgePartyCntl::OnAudioOutDisConnectedSETUP)
	ONEVENT(AUDIO_OUT_DISCONNECTED,				DISCONNECTING,	CAudioBridgePartyCntl::OnAudioOutDisConnectedDISCONNECTING)
	ONEVENT(AUDIO_OUT_DISCONNECTED,				CONNECTED,	    CAudioBridgePartyCntl::OnAudioOutDisConnectedCONNECTED)

	ONEVENT(UPDATE_AUDIO_ALGORITHM,				SETUP,			CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioAlgorithmSETUP)
	ONEVENT(UPDATE_AUDIO_ALGORITHM,				CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioAlgorithmCONNECTED)
	ONEVENT(UPDATE_AUDIO_ALGORITHM,				DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioAlgorithmDISCONNECTING)

	ONEVENT(UPDATE_MEDIA_TYPE,				    SETUP,			CAudioBridgePartyCntl::OnAudioBridgeUpdateMediaTypeSETUP)
	ONEVENT(UPDATE_MEDIA_TYPE,				    CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeUpdateMediaTypeCONNECTED)
	ONEVENT(UPDATE_MEDIA_TYPE,				    DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeUpdateMediaTypeDISCONNECTING)

	ONEVENT(UPDATE_MUTE,						SETUP,			CAudioBridgePartyCntl::OnAudioBridgeUpdateMuteSETUP)
	ONEVENT(UPDATE_MUTE,						CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeUpdateMuteCONNECTED)
	ONEVENT(UPDATE_MUTE,						DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeUpdateMuteDISCONNECTING)

	ONEVENT(UPDATE_AUDIO_VOLUME,				SETUP,			CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioVolumeSETUP)
	ONEVENT(UPDATE_AUDIO_VOLUME,				CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioVolumeCONNECTED)
	ONEVENT(UPDATE_AUDIO_VOLUME,				DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioVolumeDISCONNECTING)

	ONEVENT(INCREASE_AUDIO_VOLUME,				SETUP,			CAudioBridgePartyCntl::OnAudioBridgeIncreaseAudioVolumeSETUP)
	ONEVENT(INCREASE_AUDIO_VOLUME,				CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeIncreaseAudioVolumeCONNECTED)
	ONEVENT(INCREASE_AUDIO_VOLUME,				DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeIncreaseAudioVolumeDISCONNECTING)

	ONEVENT(DECREASE_AUDIO_VOLUME,				SETUP,			CAudioBridgePartyCntl::OnAudioBridgeDecreaseAudioVolumeSETUP)
	ONEVENT(DECREASE_AUDIO_VOLUME,				CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeDecreaseAudioVolumeCONNECTED)
	ONEVENT(DECREASE_AUDIO_VOLUME,				DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeDecreaseAudioVolumeDISCONNECTING)

	ONEVENT(UPDATE_NOISE_DETECTION,				SETUP,			CAudioBridgePartyCntl::OnAudioBridgeUpdateNoiseDetectionSETUP)
	ONEVENT(UPDATE_NOISE_DETECTION,				CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeUpdateNoiseDetectionCONNECTED)
	ONEVENT(UPDATE_NOISE_DETECTION,				DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeUpdateNoiseDetectionDISCONNECTING)

	ONEVENT(UPDATE_AGC_EXEC,					SETUP,			CAudioBridgePartyCntl::OnAudioBridgeUpdateAgcSETUP)
	ONEVENT(UPDATE_AGC_EXEC,					CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeUpdateAgcCONNECTED)
	ONEVENT(UPDATE_AGC_EXEC,					DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeUpdateAgcDISCONNECTING)

	ONEVENT(UPDATE_USE_SPEAKER_SSRC_FOR_TX,		SETUP,			CAudioBridgePartyCntl::OnAudioBridgeUpdateUseSpeakerSsrcForTxSETUP)
	ONEVENT(UPDATE_USE_SPEAKER_SSRC_FOR_TX,		CONNECTED,		CAudioBridgePartyCntl::OnAudioBridgeUpdateUseSpeakerSsrcForTxCONNECTED)
	ONEVENT(UPDATE_USE_SPEAKER_SSRC_FOR_TX,		DISCONNECTING,	CAudioBridgePartyCntl::OnAudioBridgeUpdateUseSpeakerSsrcForTxDISCONNECTING)

	ONEVENT(AUDIO_IN_VOLUME_CHANGED,			SETUP,			CAudioBridgePartyCntl::OnAudioInVolumeChanged)
	ONEVENT(AUDIO_IN_VOLUME_CHANGED,			CONNECTED,		CAudioBridgePartyCntl::OnAudioInVolumeChanged)
	ONEVENT(AUDIO_IN_VOLUME_CHANGED,			DISCONNECTING,	CAudioBridgePartyCntl::NullActionFunction)

	ONEVENT(AUDIO_OUT_VOLUME_CHANGED,			SETUP,			CAudioBridgePartyCntl::OnAudioOutVolumeChanged)
	ONEVENT(AUDIO_OUT_VOLUME_CHANGED,			CONNECTED,		CAudioBridgePartyCntl::OnAudioOutVolumeChanged)
	ONEVENT(AUDIO_OUT_VOLUME_CHANGED,			DISCONNECTING,	CAudioBridgePartyCntl::NullActionFunction)

	// Timers events
	ONEVENT(AUDIO_BRDG_PARTY_SETUP_TOUT,		SETUP,			CAudioBridgePartyCntl::OnTimerPartySetupSETUP)
	ONEVENT(AUDIO_BRDG_PARTY_SETUP_TOUT,		CONNECTED,		CAudioBridgePartyCntl::OnTimerPartySetupCONNECTED)

	ONEVENT(AUDIO_BRDG_PARTY_DISCONNECT_TOUT,	SETUP,		    CAudioBridgePartyCntl::OnTimerPartyDisconnectSETUP)
	ONEVENT(AUDIO_BRDG_PARTY_DISCONNECT_TOUT,	CONNECTED,	    CAudioBridgePartyCntl::OnTimerPartyDisconnectCONNECTED)
	ONEVENT(AUDIO_BRDG_PARTY_DISCONNECT_TOUT,	DISCONNECTING,	CAudioBridgePartyCntl::OnTimerPartyDisconnectDISCONNECTING)

PEND_MESSAGE_MAP(CAudioBridgePartyCntl,CBridgePartyCntl);


/////////////////////////////////////////////////////////////////////////////
CAudioBridgePartyCntl::CAudioBridgePartyCntl()
{
	m_pUpdatePartyInitParams = NULL;
	m_bIsVideoRelay = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CAudioBridgePartyCntl::~CAudioBridgePartyCntl()
{

	POBJDELETE(m_pUpdatePartyInitParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::Destroy()
{

	//MOTI - TODO: Remove from the Rsrc Table the DEc/Enc ?

	CBridgePartyCntl::Destroy ();
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::Create(const CBridgePartyInitParams* pBridgePartyInitParams)
{	
	if (NULL == pBridgePartyInitParams)
	{
		PASSERT_AND_RETURN(101);
	}

	// Create base params
	CBridgePartyCntl::Create(pBridgePartyInitParams);
	
	if (!pBridgePartyInitParams->GetMediaInParams() && !pBridgePartyInitParams->GetMediaOutParams())
	{
		PASSERT_AND_RETURN(102);
	}

	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

	if (NULL == pRoutingTbl)
	{
		PASSERT_AND_RETURN(103);
	}

	m_bIsVideoRelay = pBridgePartyInitParams->GetIsVideoRelay();
	m_pUpdatePartyInitParams = NULL;

	BOOL bAudioInFailure = FALSE, bAudioOutFailure = FALSE;
	CRsrcParams *pRsrcParams = new CRsrcParams;
	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);
	pRsrcParams->SetRoomId(pBridgePartyInitParams->GetPartyRoomID());

	CRsrcDesc *pRsrcDesc = NULL;

	if (pBridgePartyInitParams->GetMediaInParams())
	{
		POBJDELETE(m_pBridgePartyIn);
		m_pBridgePartyIn = new CBridgePartyAudioIn();

		CTaskApi *pTaskApiAudioIn = new CTaskApi(*m_pConfApi);

		pTaskApiAudioIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(),m_pBridgePartyIn);

		pRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, GetLogicalResourceTypeDec(), pTaskApiAudioIn);

		if (!pRsrcDesc) 
		{ // Entry not found in Routing Table
			bAudioInFailure = TRUE;

			POBJDELETE(m_pBridgePartyIn);

			PASSERT(104);
		}
		else 
		{
			pRsrcParams->SetRsrcDesc(*pRsrcDesc);

			const CConfAppBridgeParams* pConfAppBridgeParams = pBridgePartyInitParams->GetConfAppParams();
			const CBridgePartyMediaParams* pMediaInParams = pBridgePartyInitParams->GetMediaInParams();
			TRACEINTO << "BRIDGE-12931 - m_pBridgePartyOut resource params initiated with connectionId - ", pRsrcParams->GetConnectionId();
			((CBridgePartyAudioIn *)(m_pBridgePartyIn))->Create(this, pRsrcParams, pMediaInParams, pConfAppBridgeParams);

			//Update DB when connecting muted party
			CheckIsMutedAudioIn();

			// update DB for mute incomming
			if( pConfAppBridgeParams->IsMuteIncoming() && (DIALIN==((CBridgePartyAudioInParams*)pMediaInParams)->GetCallDirection()) )
			{
				UpdateDBMuteAudioIn(eOn, MCMS);
			}
			if (pConfAppBridgeParams->GetIsMuteIncomingByOperator())
			{
				UpdateDBMuteAudioIn(eOn, OPERATOR);
			}
		}

		POBJDELETE(pTaskApiAudioIn);
	}

	if (pBridgePartyInitParams->GetMediaOutParams())
	{
		m_pBridgePartyOut= new CBridgePartyAudioOut();

		CTaskApi *pTaskApiAudioOut = new CTaskApi(*m_pConfApi);

		pTaskApiAudioOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(),m_pBridgePartyOut);

		pRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, GetLogicalResourceTypeEnc(), pTaskApiAudioOut);

		if (!pRsrcDesc) 
		{ // Entry not found in Routing Table
			bAudioOutFailure = TRUE;

			POBJDELETE(m_pBridgePartyOut);

			PASSERT(105);
		}
		else 
		{
			pRsrcParams->SetRsrcDesc(*pRsrcDesc);

			TRACEINTO << "BRIDGE-12931 - m_pBridgePartyOut resource params initiated with connectionId - ", pRsrcParams->GetConnectionId();
			((CBridgePartyAudioOut *)(m_pBridgePartyOut))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaOutParams());
		}

		POBJDELETE(pTaskApiAudioOut);
	}

	if ( TRUE == bAudioInFailure)
		PTRACE2(eLevelError,"CAudioBridgePartyCntl::Create - Audio-In creation failure : Name - ", GetFullName());

	if ( TRUE == bAudioOutFailure)
		PTRACE2(eLevelError,"CAudioBridgePartyCntl::Create - Audio-Out creation failure : Name - ", GetFullName());

	POBJDELETE(pRsrcParams);
}
/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::Update(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	if(NULL == pBridgePartyInitParams)
	{
		PASSERT_AND_RETURN(101);
	}

	if( !pBridgePartyInitParams->GetMediaInParams() && !pBridgePartyInitParams->GetMediaOutParams() )
	{
		PASSERT_AND_RETURN(102);
	}

	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

	if( NULL == pRoutingTbl )
	{
		PASSERT_AND_RETURN(103);
	}

	BOOL bAudioInFailure = FALSE, bAudioOutFailure = FALSE;
	CRsrcParams *pRsrcParams = new CRsrcParams;
	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);
	pRsrcParams->SetRoomId(m_RoomId);

	CRsrcDesc *pRsrcDesc = NULL;

	if ( pBridgePartyInitParams->GetMediaInParams() )
	{
		const CBridgePartyMediaParams* pMediaInParams = pBridgePartyInitParams->GetMediaInParams();
		const CConfAppBridgeParams* pConfAppBridgeParams = pBridgePartyInitParams->GetConfAppParams();

		//If audio IN direction wasn't created already
		if(!m_pBridgePartyIn)
		{
			m_pBridgePartyIn = new CBridgePartyAudioIn();

			CTaskApi *pTaskApiAudioIn = new CTaskApi(*m_pConfApi);

			pTaskApiAudioIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(),m_pBridgePartyIn);

			pRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, GetLogicalResourceTypeDec(), pTaskApiAudioIn);

			if(!pRsrcDesc) { // Entry not found in Routing Table

				bAudioInFailure = TRUE;

				POBJDELETE(m_pBridgePartyIn);

				PASSERT(104);
			}
			else
			{
				pRsrcParams->SetRsrcDesc(*pRsrcDesc);

				TRACEINTO << "BRIDGE-12931 - m_pBridgePartyIn resource params initiated with connectionId - ", pRsrcParams->GetConnectionId();
				((CBridgePartyAudioIn *)(m_pBridgePartyIn))->Create(this, pRsrcParams, pMediaInParams, pConfAppBridgeParams);

				// update DB for mute incomming
				if( pConfAppBridgeParams->IsMuteIncoming() && (DIALIN==((CBridgePartyAudioInParams*)pMediaInParams)->GetCallDirection()) )
				{
					UpdateDBMuteAudioIn(eOn, MCMS);
				}
				if (pConfAppBridgeParams->GetIsMuteIncomingByOperator())
				{
					UpdateDBMuteAudioIn(eOn, OPERATOR);
				}

			}

			POBJDELETE(pTaskApiAudioIn);
		}
		//If we are in disconnecting state we need to save the In Init params
		if(m_state == DISCONNECTING ||(m_state == CONNECTED && m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING))
		{
			if(m_pUpdatePartyInitParams)
			{
				if(!m_pUpdatePartyInitParams->GetMediaInParams())
				{
					m_pUpdatePartyInitParams->InitiateMediaInParams((CBridgePartyAudioParams*)pBridgePartyInitParams->GetMediaInParams());
				}
			}
			else
			{
				InitiateUpdatePartyParams(pBridgePartyInitParams);
			}
		}
	}

	if ( pBridgePartyInitParams->GetMediaOutParams() )
	{
		//If audio out wasn't created already...
		if(!m_pBridgePartyOut)
		{
			m_pBridgePartyOut= new CBridgePartyAudioOut();

			CTaskApi *pTaskApiAudioOut = new CTaskApi(*m_pConfApi);

			pTaskApiAudioOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(),m_pBridgePartyOut);

			pRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, GetLogicalResourceTypeEnc(), pTaskApiAudioOut);

			if(!pRsrcDesc) { // Entry not found in Routing Table

				bAudioOutFailure = TRUE;

				POBJDELETE(m_pBridgePartyOut);

				PASSERT(105);
			}
			else {
				pRsrcParams->SetRsrcDesc(*pRsrcDesc);

				TRACEINTO << "BRIDGE-12931 - m_pBridgePartyIn resource params initiated with connectionId - ", pRsrcParams->GetConnectionId();
				((CBridgePartyAudioOut *)(m_pBridgePartyOut))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaOutParams());
			}

			POBJDELETE(pTaskApiAudioOut);
		}
		//If we are in disconnecting state we need to save the Out Init params
		if (m_state == DISCONNECTING || (m_state == CONNECTED && m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING))
		{
			if (m_pUpdatePartyInitParams)
			{
				if (!m_pUpdatePartyInitParams->GetMediaOutParams())
				{
					m_pUpdatePartyInitParams->InitiateMediaOutParams((CBridgePartyAudioParams*)pBridgePartyInitParams->GetMediaOutParams());
				}
			}
			else
			{
				InitiateUpdatePartyParams(pBridgePartyInitParams);
			}
		}
	}

	if ( TRUE == bAudioInFailure)
		PTRACE2(eLevelError,"CAudioBridgePartyCntl::Update - Audio-In creation failure : Name - ", GetFullName());

	if ( TRUE == bAudioOutFailure)
		PTRACE2(eLevelError,"CAudioBridgePartyCntl::Update - Audio-Out creation failure : Name - ", GetFullName());

	POBJDELETE(pRsrcParams);

}
/////////////////////////////////////////////////////////////////////////////
void	CAudioBridgePartyCntl::Import(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	UpdateNewConfParams(pBridgePartyInitParams);

	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

	if( NULL == pRoutingTbl )
	{
		PASSERT_AND_RETURN(103);
	}

	BOOL bAudioInFailure = FALSE, bAudioOutFailure = FALSE;
	CRsrcParams *pRsrcParams = new CRsrcParams;
	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);

	CRsrcDesc *pRsrcDesc = NULL;

	if ( m_pBridgePartyIn )
	{
		((CBridgePartyAudioIn*)m_pBridgePartyIn)->Import();
		CTaskApi *pTaskApiAudioIn = new CTaskApi(*m_pConfApi);

		pTaskApiAudioIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(),m_pBridgePartyIn);


		pRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, GetLogicalResourceTypeDec(), pTaskApiAudioIn);

		if(!pRsrcDesc) { // Entry not found in Routing Table

			bAudioInFailure = TRUE;

			POBJDELETE(m_pBridgePartyIn);

			PASSERT(104);
		}
		else {

			((CBridgePartyAudioIn *)(m_pBridgePartyIn))->UpdateNewConfParams(m_confRsrcID);

			const CConfAppBridgeParams* pConfAppBridgeParams = pBridgePartyInitParams->GetConfAppParams();
			BYTE direction = ((CBridgePartyAudioIn *)(m_pBridgePartyIn))->GetCallDirection();
			if( pConfAppBridgeParams->IsMuteIncoming() && (DIALIN==direction) ) //VNGFE-1500
			{
				MuteAudioIn(eOn, MCMS);
			}

		}


		POBJDELETE(pTaskApiAudioIn);
	}

	if ( m_pBridgePartyOut )
		{
			CTaskApi *pTaskApiAudioOut = new CTaskApi(*m_pConfApi);

			pTaskApiAudioOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(),m_pBridgePartyOut);

			pRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, GetLogicalResourceTypeEnc(), pTaskApiAudioOut);

			if(!pRsrcDesc) { // Entry not found in Routing Table

				bAudioOutFailure = TRUE;

				POBJDELETE(m_pBridgePartyOut);

				PASSERT(105);
			}
			else {

				((CBridgePartyAudioOut *)(m_pBridgePartyOut))->UpdateNewConfParams(m_confRsrcID);
			}
			POBJDELETE(pTaskApiAudioOut);
		}

	if ( TRUE == bAudioInFailure)
		PTRACE2(eLevelError,"CAudioBridgePartyCntl::Import - Audio-In creation failure : Name - ", GetFullName());

	if ( TRUE == bAudioOutFailure)
		PTRACE2(eLevelError,"CAudioBridgePartyCntl::Import - Audio-Out creation failure : Name - ", GetFullName());

	POBJDELETE(pRsrcParams);
}
/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::InitiateUpdatePartyParams(const CBridgePartyInitParams* pBridgePartyInitParams)
{
		POBJDELETE(m_pUpdatePartyInitParams);
		m_pUpdatePartyInitParams = new CUpdatePartyAudioInitParams(*pBridgePartyInitParams);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::Connect (BYTE isIVR)
{
	CSegment *pSeg = new CSegment;
	*pSeg << isIVR;
	DispatchEvent(AUDCONNECT, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::DisConnect()
{
	DispatchEvent(AUDDISCONNECT, NULL);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::Export()
{
	DispatchEvent(AUDIO_EXPORT, NULL);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CAudioBridgePartyCntl::IsDirectionMuteSrcSet(EMediaDirection eMediaDirection, WORD srcRequest)
{
	BOOL isDirectionMusteSrcSet = FALSE;

	switch (eMediaDirection) {
	case eMediaIn:
		isDirectionMusteSrcSet = ((CBridgePartyAudioIn *)m_pBridgePartyIn)->IsMuteSrc(srcRequest);
		break;

	case eMediaOut:
		isDirectionMusteSrcSet = ((CBridgePartyAudioOut *)m_pBridgePartyOut)->IsMuteSrc(srcRequest);
		break;

	case eMediaInAndOut:
		// only unique direction can be handle here!
		break;
	default:
		DBGPASSERT (1);
		break;

	}

	return isDirectionMusteSrcSet;
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateAudioAlgorithm(EMediaDirection eMediaDirection, DWORD newAudioAlgorithm, DWORD maxAverageBitrate)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)eMediaDirection << newAudioAlgorithm << maxAverageBitrate ;

	DispatchEvent(UPDATE_AUDIO_ALGORITHM, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateMediaType(EMediaTypeUpdate eMediaTypeUpdate)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)eMediaTypeUpdate;

	DispatchEvent(UPDATE_MEDIA_TYPE, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
// speakerIndication - update
void CAudioBridgePartyCntl::UpdateUseSpeakerSsrcForTx(BOOL updatedUseSpeakerSsrcForTx)
{
	CSegment* pSeg = new CSegment;

	*pSeg << (BYTE)updatedUseSpeakerSsrcForTx;

	DispatchEvent(UPDATE_USE_SPEAKER_SSRC_FOR_TX, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateMute(EMediaDirection eMediaDirection, EOnOff eOnOff, WORD srcRequest)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)eMediaDirection << (BYTE)eOnOff << srcRequest;

	DispatchEvent(UPDATE_MUTE, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateAudioVolume(EMediaDirection eMediaDirection, DWORD newAudioVolume)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)eMediaDirection << newAudioVolume;

	DispatchEvent(UPDATE_AUDIO_VOLUME, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::IncreaseAudioVolume (EMediaDirection eMediaDirection, BYTE increaseRate)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)eMediaDirection << increaseRate;

	DispatchEvent(INCREASE_AUDIO_VOLUME, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::DecreaseAudioVolume (EMediaDirection eMediaDirection, BYTE decreaseRate)
{
	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)eMediaDirection << decreaseRate;

	DispatchEvent(DECREASE_AUDIO_VOLUME, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateNoiseDetection(EOnOff eOnOff, BYTE newNoiseDetectionThreshold)
{
	CSegment* pSeg = new CSegment;

	*pSeg  	<< (BYTE)eOnOff
			<< (BYTE)newNoiseDetectionThreshold;

	DispatchEvent(UPDATE_NOISE_DETECTION, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateAGC(EOnOff eOnOff)
{
	CSegment* pSeg = new CSegment;

	*pSeg  	<< (BYTE)eOnOff;

	DispatchEvent(UPDATE_AGC_EXEC, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void*  CAudioBridgePartyCntl::GetMessageMap()
{
	return m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CAudioBridgePartyCntl::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeConnectIDLE(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioBridgeConnectIDLE : Name - ",m_partyConfName);

	if (!m_pBridgePartyIn  &&  !m_pBridgePartyOut) 
	{
		if (!CPObject::IsValidPObjectPtr(m_pConfApi))
		{
			PASSERT_AND_RETURN(1);
		}
		
		m_pConfApi->PartyBridgeResponseMsgById(m_pParty->GetPartyId(), AUDIO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE);
		
		return;
	}

	m_state= SETUP;

	StartTimer(AUDIO_BRDG_PARTY_SETUP_TOUT, AUDIO_BRDG_PARTY_SETUP_TOUT_VALUE);

	BYTE isIVR;
	*pParams >> isIVR;

	if (m_pBridgePartyIn)
		((CBridgePartyAudioIn*)m_pBridgePartyIn)->Connect(isIVR);

	if (m_pBridgePartyOut)
		((CBridgePartyAudioOut*)m_pBridgePartyOut)->Connect(isIVR);
}
/////////////////////////////////////////////////////////////////////////////
//Incase one of the video channels (In/Out) is in setup state and now connects the second direction.
void CAudioBridgePartyCntl::OnAudioBridgeConnectSETUP(CSegment* pParams)
{
	 PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioBridgeConnectSETUP : Name - ",m_partyConfName);

	 OnAudioBridgeConnect(pParams);

}
/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeConnectCONNECTED(CSegment* pParams)
{
    PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioBridgeConnectCONNECTED : Name - ",m_partyConfName);

	if(m_pBridgePartyIn && m_pBridgePartyOut && m_pBridgePartyOut->IsConnected() && m_pBridgePartyIn->IsConnected())
	{
		// Inform Audio Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_pParty->GetPartyId(), AUDIO_BRIDGE_MSG, ENDCONNECTPARTY, statOK, FALSE);
	}
	else
		OnAudioBridgeConnect(pParams);
}
/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeConnect(CSegment* pParams)
{
		if ( !m_pBridgePartyIn  &&  !m_pBridgePartyOut )
		{
			m_pConfApi->PartyBridgeResponseMsgById(m_pParty->GetPartyId(), AUDIO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE);
			return;
		}

		StartTimer(AUDIO_BRDG_PARTY_SETUP_TOUT, AUDIO_BRDG_PARTY_SETUP_TOUT_VALUE);

		BYTE isIVR;
		*pParams >> isIVR;

		//If Audio IN is not connected or in Setup state...
		if (m_pBridgePartyIn && !m_pBridgePartyIn->IsConnected() && !m_pBridgePartyIn->IsConnecting())
			((CBridgePartyAudioIn*)m_pBridgePartyIn)->Connect(isIVR);

		//If Audio Out is not connected or in Setup state...
		if (m_pBridgePartyOut && !m_pBridgePartyOut->IsConnected() && !m_pBridgePartyOut->IsConnecting())
			((CBridgePartyAudioOut*)m_pBridgePartyOut)->Connect(isIVR);
}
/////////////////////////////////////////////////////////////////////////////
//Incase both audio direction are now disconnecing and we received connect command
//we will save the isIVR parameter but wait till the disconnecting process ends before we
// start connect process again.
void CAudioBridgePartyCntl::OnAudioBridgeConnectDISCONNECTING(CSegment* pParams)
{
	if(!m_pUpdatePartyInitParams)
	{
		m_pConfApi->PartyBridgeResponseMsgById(m_pParty->GetPartyId(), AUDIO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE);
		return;
	}
	PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioBridgeConnectDISCONNECTING - Wait to end of disconnect before we start connect again... : Name - ",m_partyConfName);

	BYTE isIVR;
	*pParams >>isIVR;
	m_pUpdatePartyInitParams->SetIsIVR(isIVR);

}
/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeDisConnectSETUP(CSegment* pParams)
{
//	DeleteTimer(AUDIO_BRDG_PARTY_SETUP_TOUT);

	OnAudioBridgeDisConnect(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeDisConnectCONNECTED(CSegment* pParams)
{
	OnAudioBridgeDisConnect(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeDisConnectDISCONNECTING(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioBridgeDisConnectDISCONNECTING - Party is already disconnecting : Name - ", GetFullName());
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeDisConnect(CSegment* pParams)
{
    PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioBridgeDisConnect : Name - ",m_partyConfName);

	BOOL IsAudioFullyDisconnect = FALSE;

	StartTimer(AUDIO_BRDG_PARTY_DISCONNECT_TOUT, AUDIO_BRDG_PARTY_DISCONNECT_TOUT_VALUE);

	switch(GetDisconnectingDirectionsReq())
	{
		case eMediaIn:
		{//Disconnect Audio In
			if (m_pBridgePartyIn)
			{	//If party out is connecting(SETUP) or connected(CONNECTED), this is not full disconnection of party cntl
				if(!m_pBridgePartyOut || (m_pBridgePartyOut && !m_pBridgePartyOut->IsConnecting() && !m_pBridgePartyOut->IsConnected()))
					IsAudioFullyDisconnect = TRUE;

				// Incase we are in setup state and out direction is already connected
				// but the in direction is still in setup--> we will change the state to CONNECTED,
				//stop the setup timer and won't send ack on connect to partyCntl only ack on disconnect
				if(m_pBridgePartyOut && m_pBridgePartyOut->IsConnected() && m_state == SETUP)
				{
					PTRACE2(eLevelInfoNormal,"Party out is connected - Change state to CONNECT : Name - ",m_partyConfName);
					m_state = CONNECTED;
					DeleteTimer(AUDIO_BRDG_PARTY_SETUP_TOUT);
				}
				m_pBridgePartyIn->DisConnect();
			}
			break;
		}
		case eMediaOut:
		{//Disconnect Audio Out
			if (m_pBridgePartyOut)
			{	//If party In in state of CONNECTED or SETUP , this is not full disconnection of party cntl
				if(!m_pBridgePartyIn || (m_pBridgePartyIn && !m_pBridgePartyIn->IsConnecting() && !m_pBridgePartyIn->IsConnected()))
					IsAudioFullyDisconnect = TRUE;

				// Incase we are in setup state and IN direction is already connected
				// but the OUT direction is still in setup--> we will change the state to CONNECTED,
				// stop the setup timer and won't send ack on connect to partyCntl only ack on disconnect
				if(m_pBridgePartyOut && m_pBridgePartyOut->IsConnected() && m_state == SETUP)
				{
					PTRACE2(eLevelInfoNormal,"Party In is connected - Change state to CONNECT : Name - ",m_partyConfName);
					m_state = CONNECTED;
					DeleteTimer(AUDIO_BRDG_PARTY_SETUP_TOUT);
				}
				m_pBridgePartyOut->DisConnect();
			}
			break;
		}
		case eMediaInAndOut:
		{//Disconnect Audio In&Out
			if(m_pBridgePartyIn)
				m_pBridgePartyIn->DisConnect();
			if(m_pBridgePartyOut)
				m_pBridgePartyOut->DisConnect();

			IsAudioFullyDisconnect = TRUE;

			break;
		}
		default:
		{
			DBGPASSERT(1);
		}
	}

	if(IsAudioFullyDisconnect)
	{
		if(m_state == SETUP)
			DeleteTimer(AUDIO_BRDG_PARTY_SETUP_TOUT);
		m_state = DISCONNECTING;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CAudioBridgePartyCntl::OnAudioBridgeExportSETUP(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioBridgeExportSETUP : Name - ",m_partyConfName);

	PASSERTMSG(1,"Illegal Move not Party In Setup");

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, AUDIO_BRIDGE_MSG, ENDEXPORTPARTY, statIllegal, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
void  CAudioBridgePartyCntl::OnAudioBridgeExportDISCONNECTING(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioBridgeExportDISCONNECTING : Name - ",m_partyConfName);

	PASSERTMSG(1,"Illegal Move PartyDisconnecting");

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, AUDIO_BRIDGE_MSG, ENDEXPORTPARTY, statIllegal, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeExportCONNECTED(CSegment* pParams)
{
    PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioBridgeExportCONNECTED : Name - ",m_partyConfName);

    CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
    CRsrcParams* pRsrcParams = NULL;

	if(pRoutingTbl== NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	if( m_pBridgePartyIn )
	{
		((CBridgePartyAudioIn*)m_pBridgePartyIn)->Export();
		pRsrcParams = m_pBridgePartyIn->GetRsrcParams();

		if( !pRsrcParams )
		{
			PASSERT_AND_RETURN(104);
		}

		if ( STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams) )
		{
			DBGPASSERT(105);
		}
	}
	if( m_pBridgePartyOut )
	{
		pRsrcParams = m_pBridgePartyOut->GetRsrcParams();

		if( !pRsrcParams )
		{
			PASSERT_AND_RETURN(106);
		}

		if ( STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams) )
		{
			DBGPASSERT(107);
		}
	}

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, AUDIO_BRIDGE_MSG, ENDEXPORTPARTY, statOK, FALSE);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnTimerPartySetupSETUP(CSegment* pParams)
{
   PTRACE(eLevelError,"CAudioBridgePartyCntl::OnTimerPartySetupSETUP");
   OnTimerPartySetup(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnTimerPartySetupCONNECTED(CSegment* pParams)
{
   PTRACE(eLevelError,"CAudioBridgePartyCntl::OnTimerPartySetupCONNECTED");
   OnTimerPartySetup(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnTimerPartySetup(CSegment* pParams)
{
	CMedString logStr;
	logStr << "CAudioBridgePartyCntl::OnTimerPartySetupSETUP: Name - " << m_partyConfName ;
	logStr << "\nm_partyRsrcID = " << m_partyRsrcID << " , m_confRsrcID = " << m_confRsrcID;
	PTRACE(eLevelError, logStr.GetString());

    //Add Fault to EMA
    //CBridgePartyMediaUniDirection::AddFaultAlarm("Did not receive all acks in Audio Connection",m_partyRsrcID,STATUS_OK,true);


    //for debug info in case of the "MCU internal problem"
    BYTE failureCauseDirection = eMipIn;
    if((CPObject::IsValidPObjectPtr(m_pBridgePartyIn)) &&  m_pBridgePartyIn->IsConnected())
    	failureCauseDirection = eMipOut;

    if(!(CPObject::IsValidPObjectPtr(m_pBridgePartyIn)) && !(CPObject::IsValidPObjectPtr(m_pBridgePartyOut)))
    {
    	PTRACE2(eLevelError, "CAudioBridgePartyCntl::OnTimerPartySetupSETUP both m_pBridgePartyIn and m_pBridgePartyOut aren't valid",m_partyConfName );
       	DBGPASSERT(1);
    }
    CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)statAudioInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer <<(BYTE) eMipOpen;

	DumpMcuInternalProblemDetailed(failureCauseDirection,eMipTimer,eMipAudio);

    //BRIDGE-12388 - check if party's CTaskApp* is still relevant and was not already deleted (according to party lookup table)
	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(m_partyRsrcID);
	if (pParty)
	{
		// Inform Audio Bridge of statAudioInOutResourceProblem -> Inorder to initiate Kill port on AudioEncoder+Decoder
		m_pConfApi->PartyBridgeResponseMsgById(m_pParty->GetPartyId(), AUDIO_BRIDGE_MSG, ENDCONNECTPARTY, statAudioInOutResourceProblem, FALSE,eNoDirection,pSeg);
	}

    POBJDELETE(pSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnTimerPartyDisconnectSETUP(CSegment* pParams)
{
	PTRACE(eLevelInfoNormal,"CAudioBridgePartyCntl::OnTimerPartyDisconnectSETUP" );
	OnTimerPartyDisconnect(pParams);
}
/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnTimerPartyDisconnectCONNECTED(CSegment* pParams)
{
	PTRACE(eLevelInfoNormal,"CAudioBridgePartyCntl::OnTimerPartyDisconnectCONNECTED" );
	OnTimerPartyDisconnect(pParams);
}
/////////////////////////////////////////////////////////////////////////////
// In case of unidirection - one direction is still connected and one direction is disconnecting.
void CAudioBridgePartyCntl::OnTimerPartyDisconnect(CSegment* pParams)
{
	EMediaDirection eConnectedDirection = eNoDirection;
	BYTE failureCauseDirection = eMipNoneDirction;

	CMedString logStr;
	logStr << "CAudioBridgePartyCntl::OnTimerPartyDisconnect: Name - " << m_partyConfName ;
	logStr << "\nm_partyRsrcID = " << m_partyRsrcID << " , m_confRsrcID = " << m_confRsrcID;
	PTRACE(eLevelError, logStr.GetString());

	//Add Fault to EMA
    //CBridgePartyMediaUniDirection::AddFaultAlarm("Did not receive all acks in Audio Disconnection",m_partyRsrcID,STATUS_OK,true);

    //We know that in this case one direction must be connected or in setup state.
	if(GetDisconnectingDirectionsReq()==eMediaOut)
	{
		eConnectedDirection = eMediaIn;

		//for debug info in case of the "MCU internal problem"
		failureCauseDirection = eMipOut;
	}
	if(GetDisconnectingDirectionsReq()==eMediaIn)
	{
		eConnectedDirection = eMediaOut;

		//for debug info in case of the "MCU internal problem"
		failureCauseDirection = eMipOut;
	}
	 CSegment* pSeg = new CSegment;
	 *pSeg << (BYTE)statAudioInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer <<(BYTE) eMipClose;

	 if (GetConnectionFailureCause() == statOK)
		 DumpMcuInternalProblemDetailed(failureCauseDirection,eMipTimer,eMipAudio);
	 //We need to delete partyIn/partyOut incase of disconnect timeout failure
	 DestroyPartyInOut(eConnectedDirection);

    //BRIDGE-12388 - check if party's CTaskApp* is still relevant and was not already deleted (according to party lookup table)
	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(m_partyRsrcID);
	if (pParty)
	{
		// Inform Audio Bridge of statAudioInOutResourceProblem -> Inorder to initiate Kill port on AudioEncoder+Decoder
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, AUDIO_BRIDGE_MSG, ENDDISCONNECTPARTY, statAudioInOutResourceProblem, FALSE,eConnectedDirection,pSeg);
	}

	POBJDELETE(pSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams)
{
    CMedString logStr;
	logStr << "CAudioBridgePartyCntl::OnTimerPartyDisconnectDISCONNECTING: Name - " << m_partyConfName ;
	logStr << "\nm_partyRsrcID = " << m_partyRsrcID << " , m_confRsrcID = " << m_confRsrcID;
	PTRACE(eLevelError, logStr.GetString());

    m_state = IDLE;

    //Add Fault to EMA
    //CBridgePartyMediaUniDirection::AddFaultAlarm("Did not receive all acks in Audio Disconnection",m_partyRsrcID,STATUS_OK,true);

    //for debug info in case of the "MCU internal problem"
    BYTE failureCauseDirection = eMipIn;
    if((CPObject::IsValidPObjectPtr(m_pBridgePartyIn)) && m_pBridgePartyIn->IsDisConnected())
    	failureCauseDirection = eMipOut;
    if(!(CPObject::IsValidPObjectPtr(m_pBridgePartyIn)) && !(CPObject::IsValidPObjectPtr(m_pBridgePartyOut)))
    {
    	PTRACE2(eLevelError, "CAudioBridgePartyCntl::OnTimerPartyDisconnectDISCONNECTING both m_pBridgePartyIn and m_pBridgePartyOut aren't valid",m_partyConfName );
       	DBGPASSERT(1);
    }

    CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)statAudioInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer <<(BYTE) eMipClose;

	if (GetConnectionFailureCause() == statOK)
		DumpMcuInternalProblemDetailed(failureCauseDirection,eMipTimer,eMipAudio);

    //BRIDGE-12388 - check if party's CTaskApp* is still relevant and was not already deleted (according to party lookup table)
	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(m_partyRsrcID);
	if (pParty)
	{
		// Inform Audio Bridge of statAudioInOutResourceProblem -> Inorder to initiate Kill port on AudioEncoder+Decoder
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, AUDIO_BRIDGE_MSG, ENDDISCONNECTPARTY, statAudioInOutResourceProblem, FALSE,eNoDirection,pSeg);
	}

    POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioInConnectedSETUP(CSegment* pParams)
{
	AudioConnectionCompletion(pParams, eMediaIn);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioInConnectedCONNECTED(CSegment* pParams)
{
	AudioConnectionCompletion(pParams, eMediaIn);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioInConnectedDISCONNECTING(CSegment* pParams)
{
	PTRACE(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioInConnectedDISCONNECTING - AUDIO_IN_CONNECTED Ignored!");
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioOutConnectedSETUP(CSegment* pParams)
{
	AudioConnectionCompletion(pParams, eMediaOut);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioOutConnectedCONNECTED(CSegment* pParams)
{
	AudioConnectionCompletion(pParams, eMediaOut);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioOutConnectedDISCONNECTING(CSegment* pParams)
{
	PTRACE(eLevelInfoNormal,"CAudioBridgePartyCntl::OnAudioOutConnectedDISCONNECTING - AUDIO_OUT_CONNECTED Ignored!");
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::AudioConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection)
{
	if (!m_pBridgePartyIn && !m_pBridgePartyOut) 
	{
		DBGPASSERT_AND_RETURN(1);
	}

	BOOL isAudioConnectionCompleted = FALSE;
	EStat	receivedStatus = statOK;
	EMediaDirection ConnectedDirection = eNoDirection;

	*pParams >> (BYTE&)receivedStatus;

	if (statOK != receivedStatus) 
	{
		DeleteTimer(AUDIO_BRDG_PARTY_SETUP_TOUT);
		DumpMcuInternalProblemDetailed(eConnectedMediaDirection,eMipStatusFail,eMipAudio);
		// Inform Audio Bridge about connection failure
		m_pConfApi->PartyBridgeResponseMsgById(m_pParty->GetPartyId(), AUDIO_BRIDGE_MSG, ENDCONNECTPARTY,
			receivedStatus, FALSE, eNoDirection, pParams);

		return;
	}

	// Audio-in is connected
	if (eMediaIn == eConnectedMediaDirection) 
	{
		if (IsUniDirectionConnection(eMediaIn)) 
		{
			isAudioConnectionCompleted = TRUE;
			ConnectedDirection = eMediaIn;
		}
		else 
		{
			if (m_pBridgePartyOut && m_pBridgePartyOut->IsConnected())
			{
				isAudioConnectionCompleted = TRUE;
				ConnectedDirection = eMediaInAndOut;
			}
		}
	}

	// Audio-out is connected
	if (eMediaOut == eConnectedMediaDirection) 
	{
		if (IsUniDirectionConnection(eMediaOut)) 
		{
			isAudioConnectionCompleted = TRUE;
			ConnectedDirection = eMediaOut;
		}
		else 
		{
			if (m_pBridgePartyIn && m_pBridgePartyIn->IsConnected())
			{
				isAudioConnectionCompleted = TRUE;
				ConnectedDirection = eMediaInAndOut;
			}
		}
	}

	if (TRUE == isAudioConnectionCompleted) 
	{
		DeleteTimer(AUDIO_BRDG_PARTY_SETUP_TOUT);

		m_state = CONNECTED;

		CMedString logStr;
		logStr << "CAudioBridgePartyCntl::AudioConnectionCompletion: Connected direction : "<< ConnectedDirection;
		PTRACE(eLevelInfoNormal, logStr.GetString());

		// Inform Audio Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_pParty->GetPartyId(), AUDIO_BRIDGE_MSG,
			ENDCONNECTPARTY, statOK, FALSE, ConnectedDirection);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioInDisConnectedSETUP(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::OnAudioInDisConnectedSETUP : ", m_partyConfName);

    OnAudioInDisConnected(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioInDisConnectedDISCONNECTING(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::OnAudioInDisConnectedDISCONNECTING : ", m_partyConfName);

    OnAudioInDisConnected(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioInDisConnectedCONNECTED(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::OnAudioInDisConnectedCONNECTED : ", m_partyConfName);

    OnAudioInDisConnected(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioInDisConnected(CSegment* pParams)
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

	if(pRoutingTbl== NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	if( !m_pBridgePartyIn )
	{
		PASSERT_AND_RETURN(102);
	}

	if ( !m_pBridgePartyIn->IsDisConnected() )
	{
		PASSERT_AND_RETURN(103);
	}

	CRsrcParams* pRsrcParams = m_pBridgePartyIn->GetRsrcParams();

	if( !pRsrcParams )
	{
		PASSERT_AND_RETURN(104);
	}

	if ( STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams) )
		DBGPASSERT(105);

	AudioDisConnectionCompletion(pParams, eMediaIn);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioOutDisConnectedSETUP(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::OnAudioOutDisConnectedSETUP : ", m_partyConfName);

	OnAudioOutDisConnected(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioOutDisConnectedDISCONNECTING(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::OnAudioOutDisConnectedDISCONNECTING : ", m_partyConfName);

	OnAudioOutDisConnected(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioOutDisConnectedCONNECTED(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::OnAudioOutDisConnectedCONNECTED : ", m_partyConfName);

	OnAudioOutDisConnected(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioOutDisConnected(CSegment* pParams)
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

	if(pRoutingTbl== NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	if( !m_pBridgePartyOut )
	{
		PASSERT_AND_RETURN(102);
	}

	if( !m_pBridgePartyOut->IsDisConnected() )
	{
		PASSERT_AND_RETURN(103);
	}

	CRsrcParams* pRsrcParams = m_pBridgePartyOut->GetRsrcParams();

	if( !pRsrcParams )
	{
		PASSERT_AND_RETURN(104);
	}

	if ( STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams) )
		DBGPASSERT(105);

	AudioDisConnectionCompletion(pParams, eMediaOut);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::AudioDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection)
{

	if ( !m_pBridgePartyIn && !m_pBridgePartyOut) {
		DBGPASSERT_AND_RETURN(1);
	}

	BOOL isAudioDisConnectionCompleted = FALSE;
	BOOL isHalfDisconnection = FALSE;
	EStat	receivedStatus = statOK;
	EMediaDirection ConnectedDirection = eNoDirection;

	BYTE audioInClosePortStatus = statOK;
	BYTE audioOutClosePortStatus = statOK;

	*pParams >> (BYTE&)receivedStatus;

	//*** 1. Check if this is FULL disconnection or not - Full disconnection is when all connected directions disconnects!!! ****

	// Audio-in is disconnected
	if ( eMediaIn == eDisConnectedMediaDirection )
	{
		// Check if only audio in was connected
		if ( IsUniDirectionConnection(eMediaIn) )
		{
			isAudioDisConnectionCompleted = TRUE;
		}
		else // if both directions were connected but audio out was already disconnected.
			if ( m_pBridgePartyOut && m_pBridgePartyOut->IsDisConnected() )
			{
				isAudioDisConnectionCompleted = TRUE;
			}
	}

	// Audio-out is disconnected
	if ( eMediaOut == eDisConnectedMediaDirection )
	{
		// Check if only audio out was connected
		if ( IsUniDirectionConnection(eMediaOut) )
		{
			isAudioDisConnectionCompleted = TRUE;
		}
		else // if both directions were connected but audio in was already disconnected.
			if ( m_pBridgePartyIn && m_pBridgePartyIn->IsDisConnected() )
			{
				isAudioDisConnectionCompleted = TRUE;
			}
	}

	//******* 2. If this is full disconnection  ********
	//Check if all ports closed properly and inform audio bridge with the directions that remain connected

	if ( TRUE == isAudioDisConnectionCompleted )
	{
		ConnectedDirection = eNoDirection;
		DeleteTimer(AUDIO_BRDG_PARTY_DISCONNECT_TOUT);
		SetDisConnectingDirectionsReq(eNoDirection);
		m_state = IDLE;

		//For debug in case of MCU internal problem
		if(m_pBridgePartyIn)
			audioInClosePortStatus = m_pBridgePartyIn->GetClosePortAckStatus();
		if(m_pBridgePartyOut)
		 	audioOutClosePortStatus = m_pBridgePartyOut->GetClosePortAckStatus();

		if(audioInClosePortStatus!=STATUS_OK)
		{
			PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::AudioDisConnectionCompletion  the status to the close port from the from decoder wasn't with STATUS_OK : ", m_partyConfName);
			receivedStatus = (EStat)audioInClosePortStatus;
		}
		else
			receivedStatus = (EStat)audioOutClosePortStatus;

		//In case of problem...
		if(statAudioInOutResourceProblem==receivedStatus)
		{
			if (GetConnectionFailureCause() == statOK)
				DumpMcuInternalProblemDetailed(eDisConnectedMediaDirection,eMipStatusFail,eMipAudio);

			//BRIDGE-13016 - check if party's CTaskApp* is still relevant and was not already deleted (according to party lookup table)
			const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(m_partyRsrcID);
			if (pParty)
			{
				// Inform Audio Bridge - Add the direction connected state
				m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, AUDIO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE,ConnectedDirection,pParams);
			}

		}
		else
		{
			// if received connect req while disconnecting
			if(m_pUpdatePartyInitParams)
			{
				PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::AudioDisConnectionCompletion - Start Connect process - state is IDLE : Name - ",m_partyConfName);
				DestroyPartyInOut(ConnectedDirection);
				StartConnectProcess();

			}
			else
			{
				PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::AudioDisConnectionCompletion - Both Direction were disconnected - state is IDLE : Name - ",m_partyConfName);
				DestroyPartyInOut(ConnectedDirection);

				//BRIDGE-13016 - check if party's CTaskApp* is still relevant and was not already deleted (according to party lookup table)
				const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(m_partyRsrcID);
				if (pParty)
				{
					// Inform Audio Bridge - Add the direction connected state
					m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, AUDIO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE,ConnectedDirection);
				}
			}
		}
	}

	//*** 3. If this is not full disconnection we need to check if half disconnection
	//       (according to the disconnection req we received from partyCntl)   ****

	else
	{	//Incase both directio were connected or in setup and only one direction was disconnected ,according to the disconnect direction we recieved from the partyCntl

		//Incase AudioOut disconnected
		if(m_pBridgePartyOut && (eDisConnectedMediaDirection == eMediaOut) && (m_pBridgePartyIn && ((m_pBridgePartyIn->IsConnected())||(m_pBridgePartyIn->IsConnecting()))) && GetDisconnectingDirectionsReq()==eMediaOut)
		{
			isHalfDisconnection = TRUE;
			ConnectedDirection = eMediaIn;

			//For debug in case of MCU internal problem
			audioOutClosePortStatus = m_pBridgePartyOut->GetClosePortAckStatus();
			if(audioOutClosePortStatus!=STATUS_OK)
			{
				PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::AudioDisConnectionCompletion  the status to the close port from the from encoder wasn't with STATUS_OK : ", m_partyConfName);
				receivedStatus = (EStat)audioOutClosePortStatus;
			}
		}
		//Incase AudioIn disconnected
		else if (m_pBridgePartyIn && (eDisConnectedMediaDirection == eMediaIn) && (m_pBridgePartyOut && ((m_pBridgePartyOut->IsConnected())||(m_pBridgePartyOut->IsConnecting()))) && GetDisconnectingDirectionsReq()==eMediaIn)
		{
			isHalfDisconnection = TRUE;
			ConnectedDirection = eMediaOut;

			//For debug in case of MCU internal problem
			audioInClosePortStatus = m_pBridgePartyIn->GetClosePortAckStatus();
			if(audioInClosePortStatus!=STATUS_OK)
			{
				PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::AudioDisConnectionCompletion  the status to the close port from the from decoder wasn't with STATUS_OK : ", m_partyConfName);
				receivedStatus = (EStat)audioInClosePortStatus;
			}
		}

		if(isHalfDisconnection)
		{
			DeleteTimer(AUDIO_BRDG_PARTY_DISCONNECT_TOUT);
			SetDisConnectingDirectionsReq(eNoDirection);

			//Incase of problem...
			if(statAudioInOutResourceProblem==receivedStatus)
			{
				if (GetConnectionFailureCause() == statOK)
					DumpMcuInternalProblemDetailed(eDisConnectedMediaDirection,eMipStatusFail,eMipAudio);

				//BRIDGE-13016 - check if party's CTaskApp* is still relevant and was not already deleted (according to party lookup table)
				const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(m_partyRsrcID);
				if (pParty)
				{
					// Inform Audio Bridge - Add param
					m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, AUDIO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE,ConnectedDirection,pParams);
				}
			}
			else
			{	// if received connect req while disconnecting
				if(m_pUpdatePartyInitParams)
				{
					PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::AudioDisConnectionCompletion - Start Connect process : Name - ",m_partyConfName);
					DestroyPartyInOut(ConnectedDirection);
					StartConnectProcess();
				}
				else
				{
					PTRACE2(eLevelInfoNormal,"CAudioBridgePartyCntl::AudioDisConnectionCompletion - Disconnect only one direction, state remain the same : Name - ",m_partyConfName);
					DestroyPartyInOut(ConnectedDirection);

					//BRIDGE-13016 - check if party's CTaskApp* is still relevant and was not already deleted (according to party lookup table)
					const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(m_partyRsrcID);
					if (pParty)
					{
						// Inform Audio Bridge - Add the direction connected state
						m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, AUDIO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE,ConnectedDirection);
					}
				}
			}
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::StartConnectProcess()
{
	if((m_pUpdatePartyInitParams->GetMediaInParams()))
	{
		if(!m_pBridgePartyIn)
			CreatePartyIn();
		else
			((CBridgePartyAudioIn *)m_pUpdatePartyInitParams->GetMediaInParams())->UpdatePartyInParams(m_pUpdatePartyInitParams);

	}
	if((m_pUpdatePartyInitParams->GetMediaOutParams()))
	{
		if(!m_pBridgePartyOut)
			CreatePartyOut();
		else
			((CBridgePartyAudioOut *)m_pUpdatePartyInitParams->GetMediaOutParams())->UpdatePartyOutParams(m_pUpdatePartyInitParams);
	}

	Connect(m_pUpdatePartyInitParams->IsIVR());

	POBJDELETE(m_pUpdatePartyInitParams);



}

////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::CreatePartyIn()
{
	BOOL bAudioInFailure = FALSE;
	CRsrcParams *pRsrcParams = new CRsrcParams;
	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

	const CBridgePartyMediaParams* pMediaInParams = m_pUpdatePartyInitParams->GetMediaInParams();
	const CConfAppBridgeParams* pConfAppBridgeParams = m_pUpdatePartyInitParams->GetConfAppParams();

	CRsrcDesc *pRsrcDesc = NULL;

	m_pBridgePartyIn = new CBridgePartyAudioIn();

	CTaskApi *pTaskApiAudioIn = new CTaskApi(*m_pConfApi);

	pTaskApiAudioIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(),m_pBridgePartyIn);

	pRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, GetLogicalResourceTypeDec(), pTaskApiAudioIn);

	if(!pRsrcDesc) { // Entry not found in Routing Table

		bAudioInFailure = TRUE;

		POBJDELETE(m_pBridgePartyIn);

		PASSERT(104);
	}
	else
	{
		pRsrcParams->SetRsrcDesc(*pRsrcDesc);

		TRACEINTO << "BRIDGE-12931 - m_pBridgePartyIn resource params initiated with connectionId - ", pRsrcParams->GetConnectionId();
		((CBridgePartyAudioIn *)(m_pBridgePartyIn))->Create(this, pRsrcParams, pMediaInParams, pConfAppBridgeParams);

		// update DB for mute incomming
		if( pConfAppBridgeParams->IsMuteIncoming() && (DIALIN==((CBridgePartyAudioInParams*)pMediaInParams)->GetCallDirection()) )
		{
			UpdateDBMuteAudioIn(eOn, MCMS);
		}
		if (pConfAppBridgeParams->GetIsMuteIncomingByOperator())
		{
			UpdateDBMuteAudioIn(eOn, OPERATOR);
		}
	}

	POBJDELETE(pRsrcParams);
	POBJDELETE(pTaskApiAudioIn);
}

////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::CreatePartyOut()
{
	BOOL bAudioOutFailure = FALSE;
	CRsrcParams *pRsrcParams = new CRsrcParams;
	pRsrcParams->SetConfRsrcId(m_confRsrcID);
	pRsrcParams->SetPartyRsrcId(m_partyRsrcID);
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

	CRsrcDesc *pRsrcDesc = NULL;

	m_pBridgePartyOut= new CBridgePartyAudioOut();

	CTaskApi *pTaskApiAudioOut = new CTaskApi(*m_pConfApi);

	pTaskApiAudioOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(),m_pBridgePartyOut);

	pRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, GetLogicalResourceTypeEnc(), pTaskApiAudioOut);

	if(!pRsrcDesc) { // Entry not found in Routing Table

		bAudioOutFailure = TRUE;

		POBJDELETE(m_pBridgePartyOut);

		PASSERT(105);
	}
	else {
		pRsrcParams->SetRsrcDesc(*pRsrcDesc);
		TRACEINTO << "BRIDGE-12931 - m_pBridgePartyOut resource params initiated with connectionId - ", pRsrcParams->GetConnectionId();
		((CBridgePartyAudioOut *)(m_pBridgePartyOut))->Create(this, pRsrcParams, m_pUpdatePartyInitParams->GetMediaOutParams());
	}

	POBJDELETE(pRsrcParams);
	POBJDELETE(pTaskApiAudioOut);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateMuteSETUP(CSegment* pParams)
{
	OnAudioBridgeUpdateMute(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateMuteCONNECTED(CSegment* pParams)
{
	OnAudioBridgeUpdateMute(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateMuteDISCONNECTING(CSegment* pParams)
{
	EOnOff eOnOff = eOff;
	EMediaDirection eMediaDirection = eNoDirection;
	WORD srcRequest = 0;

	*pParams >> (BYTE&)eMediaDirection >> (BYTE&)eOnOff >> srcRequest;

	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
		m_pUpdatePartyInitParams->UpdateInitMute(eMediaDirection,eOnOff,srcRequest);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateMute(CSegment* pParams)
{
	EOnOff eOnOff = eOff;
	EMediaDirection eMediaDirection = eNoDirection;
	WORD srcRequest = 0;

	*pParams >> (BYTE&)eMediaDirection >> (BYTE&)eOnOff >> srcRequest;

	CMedString str = "mute on: ";
	str << eOnOff << " direction no/in/out: " << eMediaDirection << " mute src is: " << srcRequest;
	PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::OnAudioBridgeUpdateMute : ", str.GetString());

	switch (eMediaDirection) 
	{
		case eMediaIn: 
		{
			if(m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
			{
				if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
					m_pUpdatePartyInitParams->UpdateInitMute(eMediaIn,eOnOff,srcRequest);
			}
			else
				MuteAudioIn(eOnOff, srcRequest);
			
			break;
		}
		case eMediaOut: 
		{
			if (m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING)
			{
				if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
					m_pUpdatePartyInitParams->UpdateInitMute(eMediaOut,eOnOff,srcRequest);
			}
			else
				MuteAudioOut(eOnOff, srcRequest);
			
			break;
		}
		case eMediaInAndOut: 
		{
			if (m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
			{
				if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
					m_pUpdatePartyInitParams->UpdateInitMute(eMediaIn,eOnOff,srcRequest);
			}
			else
				MuteAudioIn(eOnOff, srcRequest);

			if (m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING)
			{
				if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
					m_pUpdatePartyInitParams->UpdateInitMute(eMediaOut,eOnOff,srcRequest);
			}
			else
				MuteAudioOut(eOnOff, srcRequest);

			break;
		}
		default: 
		{
			DBGPASSERT (1);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::CheckIsMutedAudioIn()
{

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		BYTE Muted = ((CBridgePartyAudioIn *)m_pBridgePartyIn)->isMuted();

		if(Muted)
		{

			if( ((CBridgePartyAudioIn *)m_pBridgePartyIn)->IsMuteSrc(PARTY))
			{
				PTRACE2(eLevelError, "CAudioBridgePartyCntl::CheckIsMutedAudioIn : mute src is PARTY - ", m_partyConfName);
				UpdateDBMuteAudioIn(eOn,PARTY);
			}
			if(((CBridgePartyAudioIn *)m_pBridgePartyIn)->IsMuteSrc(MCMS))
			{
				PTRACE2(eLevelError, "CAudioBridgePartyCntl::CheckIsMutedAudioIn : mute src is MCMS - ", m_partyConfName);
				UpdateDBMuteAudioIn(eOn,MCMS);
			}
			if(((CBridgePartyAudioIn *)m_pBridgePartyIn)->IsMuteSrc(OPERATOR))
			{
				PTRACE2(eLevelError, "CAudioBridgePartyCntl::CheckIsMutedAudioIn : mute src is OPERATOR", m_partyConfName);
				UpdateDBMuteAudioIn(eOn,OPERATOR);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::MuteAudioIn(EOnOff eOnOff, WORD srcRequest)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		((CBridgePartyAudioIn *)m_pBridgePartyIn)->UpdateMute(eOnOff, srcRequest);

		//Update DB Routine
		UpdateDBMuteAudioIn(eOnOff,srcRequest);
	}
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::MuteAudioIn : FAILED!!! m_pBridgePartyIn is invalid : ", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateDBMuteAudioIn(EOnOff eOnOff, WORD srcRequest)
{
	if (CPObject::IsValidPObjectPtr(m_pConfApi))
	{
		//Update DB Routine
		switch (srcRequest)
		{
			case MCMS:
			{
				m_pConfApi->UpdateDB(m_pParty,MUTE_STATE,eOnOff |0x0F000000);
				
				break;
			 }
			case OPERATOR:
			{
				m_pConfApi->UpdateDB(m_pParty,MUTE_STATE,eOnOff & 0x1);
				
				break;
			}
			case PARTY:
			{
				m_pConfApi->UpdateDB(m_pParty,MUTE_STATE,eOnOff |0xF0000000); // mute/unmute in DB for operator
				
				break;
			}
			case ALL:
			{
				m_pConfApi->UpdateDB(m_pParty,MUTE_STATE,eOnOff |0x0F000000); //MCMS
				m_pConfApi->UpdateDB(m_pParty,MUTE_STATE,eOnOff & 0x1);		  //OPERATOR
				m_pConfApi->UpdateDB(m_pParty,MUTE_STATE,eOnOff |0xF0000000); //PARTY
				
				break;
			}
		}
	}
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::UpdateDBMuteAudioIn : "
			"FAILED!!! m_pConfApi is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::MuteAudioOut(EOnOff eOnOff, WORD srcRequest)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
	{
		((CBridgePartyAudioOut *)m_pBridgePartyOut)->UpdateMute(eOnOff, srcRequest);

		//Update DB Routine
		m_pConfApi->UpdateDB(m_pParty,BLOCK_STATE,eOnOff);	// block/unblock in DB for operator
	}
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::MuteAudioOut : FAILED!!! m_pBridgePartyOut is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioAlgorithmSETUP(CSegment* pParams)
{
	OnAudioBridgeUpdateAudioAlgorithm(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioAlgorithmCONNECTED(CSegment* pParams)
{
	OnAudioBridgeUpdateAudioAlgorithm(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioAlgorithmDISCONNECTING(CSegment* pParams)
{
	EMediaDirection eMediaDirection = eNoDirection;
	DWORD newAudioAlgorithm = 0;
	DWORD maxAverageBitrate = 0;

	*pParams >> (BYTE&)eMediaDirection >> newAudioAlgorithm >> maxAverageBitrate;

	TRACEINTO << "Alg: " << newAudioAlgorithm << ", BitRate: " << maxAverageBitrate;

	if ( ! CBridgePartyAudioParams::IsValidAudioAlgorithm(newAudioAlgorithm) ) {
		DBGPASSERT_AND_RETURN(1);
	}

	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
		m_pUpdatePartyInitParams->UpdateInitAudioAlgorithm(eMediaDirection, newAudioAlgorithm, maxAverageBitrate);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioAlgorithm(CSegment* pParams)
{
	EMediaDirection eMediaDirection = eNoDirection;
	DWORD newAudioAlgorithm = 0;
	DWORD maxAverageBitrate = 0;

	*pParams >> (BYTE&)eMediaDirection >> newAudioAlgorithm >> maxAverageBitrate;

	TRACEINTO << "Alg: " << newAudioAlgorithm << ", BitRate: " << maxAverageBitrate;

	switch (eMediaDirection) {
	case eMediaIn: {
		if(m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateInitAudioAlgorithm(eMediaIn, newAudioAlgorithm, maxAverageBitrate);
		}
		else
			UpdateAudioInAlgorithm(newAudioAlgorithm, maxAverageBitrate);
		break;
				   }
	case eMediaOut: {
		if(m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateInitAudioAlgorithm(eMediaOut, newAudioAlgorithm, maxAverageBitrate);
		}
		else
			UpdateAudioOutAlgorithm(newAudioAlgorithm, maxAverageBitrate);
		break;
					}
	case eMediaInAndOut: {
		if(m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateInitAudioAlgorithm(eMediaIn, newAudioAlgorithm, maxAverageBitrate);
		}
		else
			UpdateAudioInAlgorithm(newAudioAlgorithm, maxAverageBitrate);

		if(m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateInitAudioAlgorithm(eMediaOut, newAudioAlgorithm, maxAverageBitrate);
		}
		else
			UpdateAudioOutAlgorithm(newAudioAlgorithm, maxAverageBitrate);

		break;
						 }
	default: {
		DBGPASSERT (1);
			 }
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateAudioInAlgorithm(DWORD newAudioAlgorithm, DWORD maxAverageBitrate)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
		((CBridgePartyAudioIn *)m_pBridgePartyIn)->UpdateAudioAlgorithm(newAudioAlgorithm, maxAverageBitrate);
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::UpdateAudioInAlgorithm( : FAILED!!! m_pBridgePartyIn is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateAudioOutAlgorithm(DWORD newAudioAlgorithm, DWORD maxAverageBitrate)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
		((CBridgePartyAudioOut *)m_pBridgePartyOut)->UpdateAudioAlgorithm(newAudioAlgorithm, maxAverageBitrate);
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::UpdateAudioOutAlgorithm : FAILED!!! m_pBridgePartyOut is invalid : ", m_partyConfName);
}


/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateMediaTypeSETUP(CSegment* pParams)
{
	OnAudioBridgeUpdateMediaType(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateMediaTypeCONNECTED(CSegment* pParams)
{
	OnAudioBridgeUpdateMediaType(pParams);
}
/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateMediaTypeDISCONNECTING(CSegment* pParams)
{
	EMediaTypeUpdate eMediaTypeUpdate = eMediaTypeUpdateNotNeeded;

	*pParams >> (BYTE&)eMediaTypeUpdate ;

	if ( ! CBridgePartyAudioParams::IsValidMediaTypeUpdate(eMediaTypeUpdate) )
	{
		DBGPASSERT_AND_RETURN(1);
	}

	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
		m_pUpdatePartyInitParams->UpdateMediaInType(eMediaTypeUpdate);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateMediaType(CSegment* pParams)
{
	EMediaTypeUpdate eMediaTypeUpdate = eMediaTypeUpdateNotNeeded;

	*pParams >> (BYTE&)eMediaTypeUpdate ;

	if(m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
	{
		if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
			m_pUpdatePartyInitParams->UpdateMediaInType(eMediaTypeUpdate);
	}

	else
		UpdateMediaInType(eMediaTypeUpdate);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateMediaInType(EMediaTypeUpdate eMediaTypeUpdate)
{
//	if (eMediaTypeUpdateNotNeeded == eMediaTypeUpdate)
//		return;
//
//	BOOL isVideoParticipant = FALSE;
//
//	if (eMediaTypeUpdateAudioToVideo == eMediaTypeUpdate)
//		isVideoParticipant = TRUE;
//
//	else if (eMediaTypeUpdateVideoToAudio == eMediaTypeUpdate)
//		isVideoParticipant = FALSE;


	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
		((CBridgePartyAudioIn *)m_pBridgePartyIn)->UpdateMediaType(eMediaTypeUpdate);
	else
		TRACEINTO << "FAILED!!! m_pBridgePartyIn is invalid " << "  for party " <<  m_partyConfName << "   NewType is " <<  EMediaTypeUpdateNames[eMediaTypeUpdate];
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioVolumeSETUP(CSegment* pParams)
{
	OnAudioBridgeUpdateAudioVolume(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioVolumeCONNECTED(CSegment* pParams)
{
	OnAudioBridgeUpdateAudioVolume(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioVolumeDISCONNECTING(CSegment* pParams)
{
	EMediaDirection eMediaDirection = eNoDirection;
	DWORD newAudioVolume;

	*pParams >> (BYTE&)eMediaDirection >> newAudioVolume;

	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
		m_pUpdatePartyInitParams->UpdateInitVolume(eMediaDirection,newAudioVolume);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAudioVolume(CSegment* pParams)
{
	EMediaDirection eMediaDirection = eNoDirection;
	DWORD newAudioVolume;

	*pParams >> (BYTE&)eMediaDirection >> newAudioVolume;

	switch (eMediaDirection) {
	case eMediaIn: {
		if(m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateInitVolume(eMediaDirection,eMediaIn);
		}
		else
			UpdateAudioInVolume(newAudioVolume);
		break;
				   }
	case eMediaOut: {
		if(m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateInitVolume(eMediaDirection,eMediaOut);
		}
			UpdateAudioOutVolume(newAudioVolume);
		break;
					}
	case eMediaInAndOut: {
		if(m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateInitVolume(eMediaDirection,eMediaIn);
		}
		else
			UpdateAudioInVolume(newAudioVolume);

		if(m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateInitVolume(eMediaDirection,eMediaOut);
		}
		else
		UpdateAudioOutVolume(newAudioVolume);
		break;
						 }
	default: {
		DBGPASSERT (1);
			 }
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateAudioInVolume(DWORD newAudioVolume)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
		((CBridgePartyAudioIn *)m_pBridgePartyIn)->UpdateAudioVolume(newAudioVolume);
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::UpdateAudioInVolume( : FAILED!!! m_pBridgePartyIn is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpdateAudioOutVolume(DWORD newAudioVolume)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
		((CBridgePartyAudioOut *)m_pBridgePartyOut)->UpdateAudioVolume(newAudioVolume);
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::UpdateAudioOutVolume : FAILED!!! m_pBridgePartyOut is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeIncreaseAudioVolumeSETUP(CSegment* pParams)
{
	OnAudioBridgeIncreaseAudioVolume(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeIncreaseAudioVolumeCONNECTED(CSegment* pParams)
{
	OnAudioBridgeIncreaseAudioVolume(pParams);
}

//////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeIncreaseAudioVolumeDISCONNECTING(CSegment* pParams)
{
	EMediaDirection eMediaDirection = eNoDirection;
	BYTE increaseRate =0;
	WORD newAudioVolume;

	*pParams >> (BYTE&)eMediaDirection >> increaseRate;

	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
		m_pUpdatePartyInitParams->IncreaseInitVolume(eMediaDirection,increaseRate);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeIncreaseAudioVolume(CSegment* pParams)
{
	EMediaDirection eMediaDirection = eNoDirection;
	BYTE increaseRate =0;

	*pParams >> (BYTE&)eMediaDirection >> increaseRate;

	switch (eMediaDirection) {
	case eMediaIn: {
		if(m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->IncreaseInitVolume(eMediaIn,increaseRate);
		}
		else
			IncreaseAudioInVolume(increaseRate);
		break;
				   }
	case eMediaOut: {
		if(m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->IncreaseInitVolume(eMediaOut,increaseRate);
		}
		else
			IncreaseAudioOutVolume(increaseRate);
		break;
					}
	case eMediaInAndOut: {
		if(m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->IncreaseInitVolume(eMediaIn,increaseRate);
		}
		else
			IncreaseAudioInVolume(increaseRate);

		if(m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->IncreaseInitVolume(eMediaOut,increaseRate);
		}
		else
			IncreaseAudioOutVolume(increaseRate);

		break;
						 }
	default: {
		DBGPASSERT (1);
			 }
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::IncreaseAudioInVolume(BYTE increaseRate)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
		((CBridgePartyAudioIn *)m_pBridgePartyIn)->IncreaseAudioVolume(increaseRate);
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::IncreaseAudioInVolume( : FAILED!!! m_pBridgePartyIn is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::IncreaseAudioOutVolume(BYTE increaseRate)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
		((CBridgePartyAudioOut *)m_pBridgePartyOut)->IncreaseAudioVolume(increaseRate);
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::IncreaseAudioOutVolume : FAILED!!! m_pBridgePartyOut is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeDecreaseAudioVolumeSETUP(CSegment* pParams)
{
	OnAudioBridgeDecreaseAudioVolume(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeDecreaseAudioVolumeCONNECTED(CSegment* pParams)
{
	OnAudioBridgeDecreaseAudioVolume(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeDecreaseAudioVolumeDISCONNECTING(CSegment* pParams)
{
	EMediaDirection eMediaDirection = eNoDirection;
	BYTE decreaseRate = 0;
	WORD newAudioVolume;

	*pParams >> (BYTE&)eMediaDirection >> decreaseRate;

	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
		m_pUpdatePartyInitParams->DecreaseInitVolume(eMediaDirection,decreaseRate);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeDecreaseAudioVolume(CSegment* pParams)
{
	EMediaDirection eMediaDirection = eNoDirection;
	BYTE decreaseRate = 0;

	*pParams >> (BYTE&)eMediaDirection >> decreaseRate;

	switch (eMediaDirection) {
	case eMediaIn: {
		if(m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->DecreaseInitVolume(eMediaIn,decreaseRate);
		}
		else
			DecreaseAudioInVolume(decreaseRate);
		break;
				   }
	case eMediaOut: {
		if(m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->DecreaseInitVolume(eMediaOut,decreaseRate);
		}
		else
			DecreaseAudioOutVolume(decreaseRate);
		break;
					}
	case eMediaInAndOut: {
		if(m_pBridgePartyIn && m_pBridgePartyIn->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->DecreaseInitVolume(eMediaIn,decreaseRate);
		}
		else
			DecreaseAudioInVolume(decreaseRate);

		if(m_pBridgePartyOut && m_pBridgePartyOut->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->DecreaseInitVolume(eMediaOut,decreaseRate);
		}
		else
			DecreaseAudioOutVolume(decreaseRate);
		break;
						 }
	default: {
		DBGPASSERT (1);
			 }
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::DecreaseAudioInVolume(BYTE decreaseRate)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
		((CBridgePartyAudioIn *)m_pBridgePartyIn)->DecreaseAudioVolume(decreaseRate);
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::DecreaseAudioInVolume( : FAILED!!! m_pBridgePartyIn is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::DecreaseAudioOutVolume(BYTE decreaseRate)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
		((CBridgePartyAudioOut *)m_pBridgePartyOut)->DecreaseAudioVolume(decreaseRate);
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::DecreaseAudioOutVolume : FAILED!!! m_pBridgePartyOut is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateNoiseDetectionSETUP(CSegment* pParams)
{
	OnAudioBridgeUpdateNoiseDetection(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateNoiseDetectionCONNECTED(CSegment* pParams)
{
	OnAudioBridgeUpdateNoiseDetection(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateNoiseDetectionDISCONNECTING(CSegment* pParams)
{
	EOnOff eOnOff = eOff;
	BYTE newNoiseDetectionThreshold = 0;

	*pParams >> (BYTE&)eOnOff >> newNoiseDetectionThreshold;

	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
		m_pUpdatePartyInitParams->UpdateInitNoiseDetection(eOnOff,newNoiseDetectionThreshold);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateNoiseDetection(CSegment* pParams)
{
	EOnOff eOnOff = eOff;
	BYTE newNoiseDetectionThreshold = 0;

	*pParams >> (BYTE&)eOnOff >> newNoiseDetectionThreshold;

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		if(m_pBridgePartyIn->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateInitNoiseDetection(eOnOff,newNoiseDetectionThreshold);
		}
		else
			((CBridgePartyAudioIn *)m_pBridgePartyIn)->UpdateNoiseDetection(eOnOff, newNoiseDetectionThreshold);
	}
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::OnAudioBridgeUpdateNoiseDetectionCONNECTED : FAILED!!! m_pBridgePartyIn is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAgcSETUP(CSegment* pParams)
{
	OnAudioBridgeUpdateAgc(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAgcCONNECTED(CSegment* pParams)
{
	OnAudioBridgeUpdateAgc(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAgcDISCONNECTING(CSegment* pParams)
{
	EOnOff eOnOff = eOff;

	*pParams >> (BYTE&)eOnOff;

	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
		m_pUpdatePartyInitParams->UpdateInitAGC(eOnOff);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateAgc(CSegment* pParams)
{
	EOnOff eOnOff = eOff;

	*pParams >> (BYTE&)eOnOff;

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		if(m_pBridgePartyIn->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateInitAGC(eOnOff);
		}
		else
			((CBridgePartyAudioIn *)m_pBridgePartyIn)->UpdateAGC(eOnOff);
	}
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::OnAudioBridgeUpdateAgcCONNECTED : FAILED!!! m_pBridgePartyIn is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioInVolumeChanged(CSegment* pParams)
{
	DWORD newAudioInVolume = 0;

	*pParams >> newAudioInVolume;

	m_pConfApi->UpdateDB(m_pParty,AUDIOVOLUME,newAudioInVolume);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioOutVolumeChanged(CSegment* pParams)
{
	DWORD newAudioOutVolume = 0;

	*pParams >> newAudioOutVolume;

	m_pConfApi->UpdateDB(m_pParty,LISTENINGAUDIOVOLUME,newAudioOutVolume);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::IvrCommand(OPCODE opcode, DWORD seqNumToken, CSegment *pDataSeg)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
		((CBridgePartyAudioOut *)m_pBridgePartyOut)->IvrCommand(opcode, seqNumToken, pDataSeg);
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::IvrCommand : FAILED!!! m_pBridgePartyOut is invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::IvrUpdateStandalone(BOOL isStandalone)
{
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
	{
		((CBridgePartyAudioOut *)m_pBridgePartyOut)->IvrUpdateStandalone(isStandalone);
	}
	if(CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		((CBridgePartyAudioIn *)m_pBridgePartyIn)->IvrUpdateStandalone(isStandalone);
	}
	else
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::IvrUpdateStandalone : FAILED!!! m_pBridgePartyOut and m_pBridgePartyOut are invalid : ", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::IvrNotification(OPCODE opcode, CSegment *pParam)
{
	m_pConfApi->IvrPartyNotification(m_partyRsrcID, m_pParty, m_name, opcode, pParam);
}

/////////////////////////////////////////////////////////////////////////////
bool CAudioBridgePartyCntl::UpdateAudioDelay(TAudioUpdateCompressedAudioDelayReq* pstAudioDelay)
{
	if(CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		return ((CBridgePartyAudioIn*)m_pBridgePartyIn)->UpdateAudioDelay(pstAudioDelay);
	}
	else
	{
		PTRACE2(eLevelInfoNormal, "CAudioBridgePartyCntl::IvrUpdateStandalone : FAILED!!! m_pBridgePartyOut and m_pBridgePartyOut are invalid : ", m_partyConfName);
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////
eLogicalResourceTypes CAudioBridgePartyCntl::GetLogicalResourceTypeDec()
{
    return (m_bIsVideoRelay ? eLogical_relay_audio_decoder : eLogical_audio_decoder);
}

/////////////////////////////////////////////////////////////////////////////
eLogicalResourceTypes CAudioBridgePartyCntl::GetLogicalResourceTypeEnc()
{
    return (m_bIsVideoRelay ? eLogical_relay_audio_encoder : eLogical_audio_encoder);
}

/////////////////////////////////////////////////////////////////////////////
bool CAudioBridgePartyCntl::UpdateAudioRelayParamsIn(DWORD ssrc)
{
	if(CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		((CBridgePartyAudioIn*)m_pBridgePartyIn)->UpdateAudioRelayParams(ssrc);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool CAudioBridgePartyCntl::UpdateAudioRelayParamsOut(DWORD numOfSsrcIds, DWORD* ssrc_array ,DWORD ivrSsrc)
{
	if(CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
	{
		((CBridgePartyAudioOut *)m_pBridgePartyOut)->UpdateAudioRelayParams(numOfSsrcIds, ssrc_array,ivrSsrc );
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::UpgradeToMixAvcSvc()
{
	if(CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		TRACEINTO << " m_pBridgePartyIn is valid, keep in Upgrade ";
		((CBridgePartyAudioIn*)m_pBridgePartyIn)->UpgradeToMixAvcSvc();
	}
	else
	{
		TRACEINTO << " Error: m_pBridgePartyIn not valid ";
		SendEndUpgradeToMixAvcSvcToPartyCntl(statIllegal);
	}
}

//////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::SendEndUpgradeToMixAvcSvcToPartyCntl(EStat responseStatus)
{
	TRACEINTO << responseStatus;
	m_pConfApi->ReplayUpgradeSvcAvcTranslate( m_partyRsrcID, AUDIO, responseStatus);
}

//////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)const
{
	if (m_pBridgePartyIn)
		m_pBridgePartyIn->GetPortsOpened(isOpenedRsrcMap);

	if (m_pBridgePartyOut)
		m_pBridgePartyOut->GetPortsOpened(isOpenedRsrcMap);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateUseSpeakerSsrcForTxSETUP(CSegment* pParams)
{
	OnAudioBridgeUpdateUseSpeakerSsrcForTx(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateUseSpeakerSsrcForTxCONNECTED(CSegment* pParams)
{
	OnAudioBridgeUpdateUseSpeakerSsrcForTx(pParams);
}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateUseSpeakerSsrcForTxDISCONNECTING(CSegment* pParams)
{
	BYTE useSpeakerSsrcForTx = FALSE;

	*pParams >> (BYTE&)useSpeakerSsrcForTx;

	if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
		m_pUpdatePartyInitParams->UpdateUseSpeakerSsrcForTx(useSpeakerSsrcForTx);

}

/////////////////////////////////////////////////////////////////////////////
void CAudioBridgePartyCntl::OnAudioBridgeUpdateUseSpeakerSsrcForTx(CSegment* pParams)
{
	BYTE useSpeakerSsrcForTx = FALSE;

	*pParams >> (BYTE&)useSpeakerSsrcForTx;

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
	{
		if(m_pBridgePartyOut->GetState() == DISCONNECTING)
		{
			if (CPObject::IsValidPObjectPtr(m_pUpdatePartyInitParams))
				m_pUpdatePartyInitParams->UpdateUseSpeakerSsrcForTx(useSpeakerSsrcForTx);
		}
		else
			((CBridgePartyAudioOut *)m_pBridgePartyOut)->UpdateUseSpeakerSsrcForTx((BOOL)useSpeakerSsrcForTx);
	}
	else
		TRACEINTO << "FAILED!!! m_pBridgePartyOut is invalid : " << m_partyConfName;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CAudioBridgePartyCntl::GetSSRCOfIn()
{
	DWORD ssrc = INVALID;
	if(m_pBridgePartyIn)
	{
		ssrc =((CBridgePartyAudioIn*)m_pBridgePartyIn)->GetSSRC();

	}
	else
	{
		PASSERT(m_partyRsrcID);
	}
	return ssrc;

}
/////////////////////////////////////////////////////////////////////////////

void CAudioBridgePartyCntl::SendUpdateEncoderOnSeenImageSSRC(DWORD audioUplinkSSRCOfSeenImage)
{
	TRACEINTO << "audioUplinkSSRCOfSeenImage : " <<  audioUplinkSSRCOfSeenImage;
	if(m_pBridgePartyOut)
	{
		TRACEINTO << "party id:" << m_partyRsrcID;
		((CBridgePartyAudioOut *)m_pBridgePartyOut)->UpdateAudioOutOnSeenImageSSRC(audioUplinkSSRCOfSeenImage);
	}
	else
	{
		PASSERT(m_partyRsrcID);
	}
}
