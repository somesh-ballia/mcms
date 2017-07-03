//+========================================================================+
//                  	Endpoint.cpp									   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
//+========================================================================+


#include "Macros.h"
#include "Trace.h"
#include "ConfPartyDefines.h"
#include "OpcodesMcmsAudioCntl.h"
#include "IvrApiStructures.h"

#include "SystemFunctions.h"

#include "Segment.h"
#include "MplMcmsProtocol.h"

#include "EndpointsSim.h"
#include "EndpointsSimConfig.h"
#include "EpSimCapSetsList.h"
#include "EpSimH323BehaviorList.h"

#include "Endpoint.h"
#include "ConfPartySharedDefines.h"
#include "AudRequestStructs.h"
#include "AudIndicationStructs.h"

#include "OpcodesMcmsAudio.h"
#include "IpMfaOpcodes.h"
#include "IpRtpInd.h"


/////////////////////////////////////////////////////////////////////////////
//
//   CEndpoint - base class for all Endpoint element
//
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CEndpoint)

//ONEVENT(1				,1				,CEndpoint::OnStartElement)

PEND_MESSAGE_MAP(CEndpoint,CStateMachine);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CEndpoint::CEndpoint( CTaskApi* /*pCSApi*/, const CCapSet& rCap, const CH323Behavior& rBehav )      // constructor
{
	memset(m_szEpName,0,MAX_EP_NAME);
	m_nEpID         = 0xFFFFFFFF;
	m_isChanged   	= TRUE;
	m_dialDirection = DIAL_IN;
//	m_pCSApi        = pCSApi;
	m_pCap          = new CCapSet(rCap);
	m_pConfCap		= new CCapSet();
	m_pBehavior     = new CH323Behavior(rBehav);
	m_enEpState		= eEpStateIdle;
	m_confID = m_partyID = m_connectionID = 0xFFFFFFFF;
	m_nCsCallIndex = m_wCsHandle = m_wCsSrcUnit = 0;
	m_arrayIndex = 0;
	memset(m_szConfName,0,MAX_CONF_NAME);
	m_isMuted		= FALSE;

	m_isToBeDeleted   = FALSE;
	m_isReadyToDelete = FALSE;
	
	m_nMcuId = m_nTerminalId = 0;
	m_enRoleTokenLastCmd = kUnknownRoleTokenOpcode;
}

/////////////////////////////////////////////////////////////////////////////
CEndpoint::CEndpoint(const CEndpoint& other) : CStateMachine(other)
{
	// illegal use
	PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
CEndpoint::~CEndpoint()     // destructor
{
	POBJDELETE(m_pCap);
	POBJDELETE(m_pConfCap);
	POBJDELETE(m_pBehavior);
}

/////////////////////////////////////////////////////////////////////////////
CEndpoint& CEndpoint::operator= (const CEndpoint& other)
{
	// illegal use
	PASSERT(1);

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetName( const char* pszEpName )
{
	if ( NULL != pszEpName ) {
		strncpy(m_szEpName, pszEpName, MAX_EP_NAME-1);
		m_szEpName[MAX_EP_NAME-1] = '\0';
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetID( const DWORD nId )
{
	m_nEpID = nId;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetDialDirection(const DWORD dir)
{
	m_dialDirection = ( DIAL_OUT == dir ) ? DIAL_OUT : DIAL_IN;

	if( DIAL_OUT == m_dialDirection )
		if( TRUE == ::GetEpSystemCfg()->GetDeleteDialOutAfterDisconnect() )
			m_isToBeDeleted = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetState(const enEndpointState state)
{
	if( m_enEpState != state )
	{
		m_enEpState = state;
		// update changed status to update GUI view
		m_isChanged = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetEpPartyID( const DWORD partyID )
{
	m_partyID = partyID;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetEpConfID( const DWORD confID )
{
	m_confID = confID;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetEpConnectionID( const DWORD connectionID, const DWORD netConnectionId)
{
	m_connectionID = connectionID;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetArrayIndex( const WORD ind )
{
	m_arrayIndex = ind;
}

/////////////////////////////////////////////////////////////////////////////
// sets CS call index for IP parties or net_connection_id for ISDN
void CEndpoint::SetCallIndex(const DWORD nCsCallIndex)
{
	m_nCsCallIndex = nCsCallIndex;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetCsHandle(const WORD wCsHandle)
{
    PASSERTSTREAM_AND_RETURN(
        wCsHandle == 0 || wCsHandle > 8,
        "Set invalid CS ID: " << wCsHandle);

	m_wCsHandle = wCsHandle;
}

DWORD CEndpoint::GetCSID(void) const
{
    PASSERTSTREAM_AND_RETURN_VALUE(
        m_wCsHandle == 0 || m_wCsHandle > 8,
        "Invalid CS ID: " << m_wCsHandle,
        1);

    return m_wCsHandle;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetCsSrcUnit(const WORD wCsSrcUnit)
{
	m_wCsSrcUnit = wCsSrcUnit;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SetConfName( const char* pszConfName )
{
	if ( NULL != pszConfName ) {
		strncpy(m_szConfName, pszConfName, MAX_CONF_NAME-1);
		m_szConfName[MAX_CONF_NAME-1] = '\0';
	}
}

void  CEndpoint::SetMRMInfo(const CBoardDetails &mrmBoard, const DWORD &connectionId, const DWORD &channelId)
{
	m_rMrmBoard = mrmBoard;
	m_nMrmConnectionId = connectionId;
	m_nMrmChannelId = channelId;

}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::OnScpStreamsRequest(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CEndpoint::OnGuiScpStreamsRequest - connect party, Name - ", m_szEpName);

	if(m_enEpState != eEpStateConnected){
		PTRACE(eLevelInfoNormal,"CEndpoint::OnGuiScpStreamsRequest - EP not connected!");
		return;
	}

	WORD numberOfStreams = 0;
	*pParam >> numberOfStreams;

	CSegment  rMsgSeg;

	rMsgSeg << (DWORD)CONF_PARTY_MRMP_SCP_STREAM_REQ_IND;

	rMsgSeg << m_rMrmBoard.GetBoardId()
			<< m_rMrmBoard.GetSubBoardId()
			<< m_rMrmBoard.GetUnitId()
			<< m_rMrmBoard.GetPortId();


	rMsgSeg << m_confID
			<< m_partyID
			<< m_nMrmConnectionId
			<< m_nMrmChannelId
			<< numberOfStreams;

	rMsgSeg.Put(pParam->GetPtr(true), numberOfStreams*sizeof(MrmpStreamDesc));

	::SendScpMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::OnGuiSetCapsForEp(CCapSet *pCap)
{
	if ( NULL != pCap )
	{
		POBJDELETE(m_pCap);
		m_pCap = new CCapSet(*pCap);
	}
}

/*
/////////////////////////////////////////////////////////////////////////////
void CEndpoint::FillCsProtocol( CMplMcmsProtocol* pMplProt,
		const DWORD opcode, const BYTE* pData, const DWORD nDataLen,
		const DWORD channelIndexParam, const DWORD channelMcIndexParam ) const
{
	DWORD  payloadLen   =  sizeof(COMMON_HEADER_S)
						+  sizeof(MESSAGE_DESCRIPTION_HEADER_S)
						+  sizeof(CENTRAL_SIGNALING_HEADER_S)
						+  sizeof(PORT_DESCRIPTION_HEADER_S) * pMplProt->getPortDescriptionHeaderCounter()
						+  nDataLen;
	DWORD payloadOffset =  sizeof(COMMON_HEADER_S);

	pMplProt->AddCommonHeader(opcode,MPL_PROTOCOL_VERSION_NUM,0,
				(BYTE)eCentral_signaling,(BYTE)eMcms,SystemGetTickCount(),
				payloadLen, payloadOffset,
				(DWORD)eHeaderMsgDesc,sizeof(MESSAGE_DESCRIPTION_HEADER_S));

	pMplProt->AddMessageDescriptionHeader(123,(DWORD)eCentral_signaling,
				SystemGetTickCount(),eHeaderCs,sizeof(CENTRAL_SIGNALING_HEADER_S));

	DWORD  channel_index = (channelIndexParam == 0xFFFFFFFF) ? 
				pMplProt->getCentralSignalingHeaderChannelIndex() : channelIndexParam;
	DWORD  channel_mc_index = (channelMcIndexParam == 0xFFFFFFFF) ?
				pMplProt->getCentralSignalingHeaderMcChannelIndex() : channelMcIndexParam;
				
	pMplProt->AddCSHeader(pMplProt->getCentralSignalingHeaderCsId(),
						pMplProt->getCentralSignalingHeaderDestUnitId(),
						pMplProt->getCentralSignalingHeaderSrcUnitId(),
						pMplProt->getCentralSignalingHeaderCallIndex(), // call_index
						pMplProt->getCentralSignalingHeaderServiceId(), // service_id
						channel_index,      // channel_index
						channel_mc_index,   // mc_channel_index
						(APIS32) STATUS_OK  // status
					);

	pMplProt->AddData(nDataLen,(const char*)pData);
}
*/

/////////////////////////////////////////////////////////////////////////////
BYTE CEndpoint::DtmfChar2Byte(const char& cDtmf) const
{
	if( cDtmf >= '0' && cDtmf <= '9' )
		return (BYTE) ( cDtmf - '0' );
	else if( cDtmf == '*' )
		return (BYTE) 10;
	else if( cDtmf == '#' )
		return (BYTE) 11;
	return 11;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::SendDtmf(const char* pszDtmf, WORD wDtmfSource) const
{
	if( NULL == pszDtmf )
		return;

	switch(wDtmfSource)
	{
		case eDtmfSourceAUDIO:
		{	
			// send DTMF characters one by one
			for( WORD i=0; i < strlen(pszDtmf); i++ )
			{
				CSegment  rMsgSeg;
				
				rMsgSeg << (DWORD)AUD_DTMF_IND_VAL;
				
				rMsgSeg << m_rAudioBoard.GetBoardId()
						<< m_rAudioBoard.GetSubBoardId()
						<< m_rAudioBoard.GetUnitId();
			
				rMsgSeg << m_confID
						<< m_partyID
						<< m_connectionID;
			
				TAudioDtmfEventInd  rDtmfStruct;
				memset(&rDtmfStruct,0,sizeof(TAudioDtmfEventInd));
			//	rDtmfStruct.unLength = strlen(pszDtmf);
			//	strncpy((char*)rDtmfStruct.dtmf,pszDtmf,MAX_RCV_DTMF);
			//	strncpy((char*)rDtmfStruct.dtmf,pszDtmf,sizeof(DWORD));
		
			//	strncpy(rDtmfStruct.dtmf,szTemp,4);
				//rDtmfStruct.dtmf[0] = DtmfChar2Byte(pszDtmf[i]);
				
				rDtmfStruct.enDtmfEvent = DtmfChar2Byte(pszDtmf[i]);
				rMsgSeg.Put((BYTE*)(&rDtmfStruct),sizeof(TAudioDtmfEventInd));
			
				::SendAudioMessageToGideonSimApp(rMsgSeg);
			}
			break;
		}
		case eDtmfSourceRTP:
		{	
			// send DTMF characters one by one
			for( WORD i=0; i < strlen(pszDtmf); i++ )
			{			
				CSegment  rMsgSeg;
				rMsgSeg << (DWORD)IP_RTP_DTMF_INPUT_IND;
				
				rMsgSeg << m_rAudioBoard.GetBoardId()
						<< m_rAudioBoard.GetSubBoardId()
						<< m_rAudioBoard.GetUnitId();
			
				rMsgSeg << m_confID
						<< m_partyID
						<< m_connectionID;
			
				TRtpDtmfRequestInd  dtmfStruct;
				memset(&dtmfStruct,0,sizeof(TRtpDtmfRequestInd));
	
				dtmfStruct.unDigit = DtmfChar2Byte(pszDtmf[i]);
				
				rMsgSeg.Put((BYTE*)(&dtmfStruct),sizeof(TRtpDtmfRequestInd));
			
				::SendAudioMessageToGideonSimApp(rMsgSeg);
			}
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::ActiveSpeaker() const
{
	// AC_ACTIVE_SPEAKER_IND
	if( IsConnectionCompleted() == TRUE )
	{
		if( IsMuted() == FALSE ) {
			CSegment  rMsgSeg;

			rMsgSeg << (DWORD)AC_ACTIVE_SPEAKER_IND;
		
			rMsgSeg << m_rAudioBoard.GetBoardId()
					<< m_rAudioBoard.GetSubBoardId()
					<< m_rAudioBoard.GetUnitId();

			rMsgSeg << m_confID
					<< m_partyID
					<< m_connectionID;

			::SendAudioMessageToGideonSimApp(rMsgSeg);
		}
		else
			PTRACE2(eLevelError,"CEndpoint::ActiveSpeaker - Endpoint muted, can't be a speaker, Name - ",m_szEpName);
	}
	else
		PTRACE2(eLevelError,"CEndpoint::ActiveSpeaker - Endpoint not connected completely, can't be a speaker, Name - ",m_szEpName);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::AudioSpeaker() const
{
	// AC_AUDIO_SPEAKER_IND
	if( IsConnectionCompleted() == TRUE )
	{
		if( IsMuted() == FALSE ) {
			CSegment  rMsgSeg;

			rMsgSeg << (DWORD)AC_ACTIVE_SPEAKER_IND;
		
			rMsgSeg << m_rAudioBoard.GetBoardId()
					<< m_rAudioBoard.GetSubBoardId()
					<< m_rAudioBoard.GetUnitId();

			rMsgSeg << m_confID
					<< m_partyID
					<< m_connectionID;

			::SendAudioMessageToGideonSimApp(rMsgSeg);
			
		}
		else
			PTRACE2(eLevelError,"CEndpoint::AudioSpeaker - Endpoint muted, can't be a speaker, Name - ",m_szEpName);
	}
	else
		PTRACE2(eLevelError,"CEndpoint::AudioSpeaker - Endpoint not connected completely, can't be a speaker, Name - ",m_szEpName);
}
/////////////////////////////////////////////////////////////////////////////
void CEndpoint::Mute()
{
	if( IsConnectionCompleted() )
	{
		m_isMuted = TRUE;
		// virtual
		SendMuteIndication();
	}
	else
		PTRACE2(eLevelError,"CEndpoint::Mute - Endpoint not connected completely, can't be muted, Name - ",m_szEpName);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::Unmute()
{
	if( IsConnectionCompleted() )
	{
		m_isMuted = FALSE;
		// virtual
		SendUnmuteIndication();
	}
	else
		PTRACE2(eLevelError,"CEndpoint::Unmute - Endpoint not connected completely, can't be unmuted, Name - ",m_szEpName);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::FeccTokenRequest()
{
	if( IsConnectionCompleted() )
	{
		if( IsFeccCall() )
			// virtual
			SendFeccTokenRequestIndication();
		else
			PTRACE2(eLevelError,"CEndpoint::FeccTokenRequest - Endpoint has no FECC, Name - ",m_szEpName);
	}
	else
		PTRACE2(eLevelError,"CEndpoint::FeccTokenRequest - Endpoint not connected completely, can't ask for FECC token, Name - ",m_szEpName);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::FeccTokenRelease()
{
	if( IsConnectionCompleted() )
	{
		if( IsFeccCall() )
			// virtual
			SendFeccTokenReleaseIndication();
		else
			PTRACE2(eLevelError,"CEndpoint::FeccTokenRelease - Endpoint has no FECC, Name - ",m_szEpName);
	}
	else
		PTRACE2(eLevelError,"CEndpoint::FeccTokenRelease - Endpoint not connected completely, can't ask for FECC token release, Name - ",m_szEpName);
}

/////////////////////////////////////////////////////////////////////////////

void CEndpoint::FeccKeyRequest(const char* pszDtmf)
{
        if( IsConnectionCompleted() )
        {
                if( IsFeccCall() )
                        // virtual
                        SendFeccKeyRequestIndication(pszDtmf);
                else
                        PTRACE2(eLevelError,"CEndpoint::FeccKeyRequest - Endpoint has no FECC, Name - ",m_szEpName);
        }
        else
                PTRACE2(eLevelError,"CEndpoint::FeccKeyRequest - Endpoint not connected completely, can't ask for FECC key, Name - ",m_szEpName);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::H239TokenRequest()
{
	if( IsConnectionCompleted() )
	{
		if( IsH239Call() )
			// virtual
			SendH239TokenRequestIndication();
		else
			PTRACE2(eLevelError,"CEndpoint::H239TokenRequest - Endpoint has no H239, Name - ",m_szEpName);
	}
	else
		PTRACE2(eLevelError,"CEndpoint::H239TokenRequest - Endpoint not connected completely, can't ask for H239 token, Name - ",m_szEpName);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::H239TokenRelease()
{
	if( IsConnectionCompleted() )
	{
		if( IsH239Call() )
			// virtual
			SendH239TokenReleaseIndication();
		else
			PTRACE2(eLevelError,"CEndpoint::H239TokenRelease - Endpoint has no H239, Name - ",m_szEpName);
	}
	else
		PTRACE2(eLevelError,"CEndpoint::H239TokenRelease - Endpoint not connected completely, can't ask for H239 token release, Name - ",m_szEpName);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpoint::UpdateChannels(const bool au,const bool vi,const bool fecc,const bool h239,
						const BYTE recapMode,const char* pszManufacturerName, const CCapSet *pCapSet)
{
	PTRACE2(eLevelError,"CEndpoint::UpdateChannels - NOTHING for this type of Endpoint, Name - ",m_szEpName);
}


/////////////////////////////////////////////////////////////////////////////
void CEndpoint::LprModeChangeReq(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout)
{
	
	if( IsConnectionCompleted() )
	{
		if( IsLprCall() )
			// virtual
			SendLprModeChangeRequestIndication(lossProtection, mtbf, congestionCeiling, fill, modeTimeout);
		else
			PTRACE2(eLevelError,"CEndpoint::LprModeChangeReq - Endpoint has no LPR, Name - ",m_szEpName);
	}
	else
		PTRACE2(eLevelError,"CEndpoint::LprModeChangeReq - Endpoint not connected completely, can't ask for LPR, Name - ",m_szEpName);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CCommonComMode::CCommonComMode()
{// eG711Ulaw64kCapCode
	m_audio = eUnknownAlgorithemCapCode;
	m_video = eUnknownAlgorithemCapCode;
	m_presentVideo=eUnknownAlgorithemCapCode;
	m_isFecc      = FALSE;
	m_isEncrypted = FALSE;
	m_isH239	  = FALSE;
	m_isLPR		  = FALSE;
}

//////////////////////////////////////////////////////////////////////////
void CCommonComMode::Create(const CCapSet& rCapSet)
{
	m_audio = rCapSet.GetAudioAlgType(0);
	m_video = rCapSet.GetVideoProtocolType(0);
	m_presentVideo=rCapSet.GetPresentVideoProtocolType();
	m_isFecc      = rCapSet.IsFecc();
	m_isEncrypted = rCapSet.IsEncryption();
	m_isH239      = rCapSet.IsH239();
	m_isLPR		  = rCapSet.IsLPR();
}

//////////////////////////////////////////////////////////////////////////
CCommonComMode::~CCommonComMode()
{
}

//////////////////////////////////////////////////////////////////////////
BOOL CCommonComMode::IsAudioToOpen() const
{
	if( m_audio != eUnknownAlgorithemCapCode )
		return TRUE;
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CCommonComMode::IsVideoToOpen() const
{
	if( m_video != eUnknownAlgorithemCapCode )
		return TRUE;
	return FALSE;
}

