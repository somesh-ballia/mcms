#include "ConfAppMngrInterface.h"
#include "BridgePartyInitParams.h"
#include "BridgePartyAudioParams.h"
#include "ConfAppMngr.h"
#include "AudioBridgeInterface.h"

////////////////////////////////////////////////////////////////////////////
//                        CConfAppMngrInterface
////////////////////////////////////////////////////////////////////////////
CConfAppMngrInterface::CConfAppMngrInterface()
{
	m_pConfAppMngrImplementation = NULL;
	m_lecturerPartyId            = INVALID;
	m_bMuteIncomingLectureMode   = FALSE;
}

//--------------------------------------------------------------------------
CConfAppMngrInterface::~CConfAppMngrInterface()
{
	delete m_pConfAppMngrImplementation;
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::HandleEvent(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);

	OPCODE opcode;
	*pMsg >> opcode;

	if (GetConfAppMngrImplementation())
		GetConfAppMngrImplementation()->HandleEvent(pMsg, 0, opcode);
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::Create(const CConfAppMngrInitParams* pConfAppMngrInitParams)
{
	CreateImplementation(pConfAppMngrInitParams);
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::CreateImplementation(const CConfAppMngrInitParams* pConfAppMngrInitParams)
{
	if (m_pConfAppMngrImplementation)
		POBJDELETE(m_pConfAppMngrImplementation);

	m_pConfAppMngrImplementation = new CConfAppMngr;

	if (GetConfAppMngrImplementation())
		GetConfAppMngrImplementation()->Create(pConfAppMngrInitParams);
	else
		PASSERT(1);
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::UpdateConfParams()
{
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::ConnectPartyAudio(CBridgePartyInitParams* pBridgePartyInitParams, bool isIvrOn)
{
	if (GetConfAppMngrImplementation() == NULL)
	{
		TRACESTRFUNC(eLevelError) << "Failed, invalid pointer";
		return;
	}

	GetConfAppMngrImplementation()->ConnectPartyAudio(pBridgePartyInitParams, isIvrOn);

	// lecturer party has been set- this scenario is only in conferences where non-lecturer parties are to be muted.
	if (m_bMuteIncomingLectureMode)
	{
		BOOL isLecturerSpecified = m_lecturerPartyId != (DWORD)-1;
		BOOL isLecturerParty     = pBridgePartyInitParams->GetPartyRsrcID() == m_lecturerPartyId;

		TRACEINTO << "OldLecturerPartyId:" << m_lecturerPartyId << ", NewLecturerPartyId:" << pBridgePartyInitParams->GetPartyRsrcID();

		if (!isLecturerParty)
		{
			PASSERT(pBridgePartyInitParams->GetParty() == NULL);
			GetConfAppMngrImplementation()->GetConfAppInfo()->m_pAudBrdgInterface->UpdateMute(pBridgePartyInitParams->GetPartyRsrcID(), eMediaIn, eOn, OPERATOR_REQ_BY_ID);
		}
		else
		{
			BOOL isSpecifiedLecturer = isLecturerSpecified && isLecturerParty;

			// party should be unmuted if it is the lecturer. otherwise keep previous mute mask value.
			BOOL overrideMute = isSpecifiedLecturer;

			const CBridgePartyAudioInParams* pPartyAudioInParams = (const CBridgePartyAudioInParams*)(pBridgePartyInitParams->GetMediaInParams());
			// we don't always get the media params.. in this case will override mute only for a lecturer.
			if (IsValidPObjectPtr(pPartyAudioInParams))
			{
				CDwordBitMask muteMask = pPartyAudioInParams->GetMuteMask();

				TRACEINTO << "MuteMask:" << muteMask.GetDWORD();

				// is previously muted by operator
				overrideMute = muteMask.IsBitSet(OPERATOR) != 0;
			}

			if (overrideMute)
				GetConfAppMngrImplementation()->GetConfAppInfo()->m_pAudBrdgInterface->UpdateMute(pBridgePartyInitParams->GetPartyRsrcID(), eMediaIn, eOff, OPERATOR_REQ_BY_ID);
		}
	}
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::ConnectPartyVideo(CBridgePartyInitParams* pBridgePartyInitParams)
{
	if (GetConfAppMngrImplementation())
		GetConfAppMngrImplementation()->ConnectPartyVideo(pBridgePartyInitParams);
}

//--------------------------------------------------------------------------/
void CConfAppMngrInterface::DisconnectPartyAudio(CBridgePartyDisconnectParams* pBridgePartyDisconnectParams)
{
	if (GetConfAppMngrImplementation())
		GetConfAppMngrImplementation()->DisconnectPartyAudio(pBridgePartyDisconnectParams);
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::DisconnectPartyVideo(CBridgePartyDisconnectParams* pBridgePartyDisconnectParams)
{
	if (GetConfAppMngrImplementation())
		GetConfAppMngrImplementation()->DisconnectPartyVideo(pBridgePartyDisconnectParams);
}


//--------------------------------------------------------------------------
void CConfAppMngrInterface::ExportPartyAudio(CBridgePartyExportParams* pBridgePartyExportParams)
{
	if (GetConfAppMngrImplementation())
		GetConfAppMngrImplementation()->ExportPartyAudio(pBridgePartyExportParams);
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::ExportPartyVideo(CBridgePartyExportParams* pBridgePartyExportParams)
{
	if (GetConfAppMngrImplementation())
		GetConfAppMngrImplementation()->ExportPartyVideo(pBridgePartyExportParams);
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::RemovePartyFromCAM(PartyRsrcID partyId, const char* partyName)
{
	if (GetConfAppMngrImplementation())
		GetConfAppMngrImplementation()->RemovePartyFromCAM(partyId, partyName);
}

//--------------------------------------------------------------------------
DWORD CConfAppMngrInterface::GetPartyToForce(PartyRsrcID partyId)
{
	if (GetConfAppMngrImplementation())
		return GetConfAppMngrImplementation()->GetPartyToForce(partyId);
	return 0;
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::SetLecturerParty(PartyRsrcID lecturerPartyId, BOOL isMuteIncoming)
{
	m_bMuteIncomingLectureMode = isMuteIncoming;
	CAudioBridgeInterface* pAudioBridgeInterface = GetConfAppMngrImplementation()->GetConfAppInfo()->m_pAudBrdgInterface;

	// previous lecturer exists- if replaced, mute him
	BOOL differentLecturer    = lecturerPartyId != m_lecturerPartyId;
	BOOL isOldLecturer        = m_lecturerPartyId != (DWORD)-1;
	BOOL isOldLecturerCascade = GetConfAppMngrImplementation()->GetIsPartyCascadeLink(m_lecturerPartyId);
	BOOL isNewLecturer        = lecturerPartyId != (DWORD)-1;

	if (differentLecturer && isOldLecturer && m_bMuteIncomingLectureMode)
	{
		TRACEINTO << "isOldLecturerCascade:" << isOldLecturerCascade;

		if (!isOldLecturerCascade)
			pAudioBridgeInterface->UpdateMute(m_lecturerPartyId, eMediaIn, eOn, OPERATOR_REQ_BY_ID);
	}

	TRACEINTO << "OldLecturerPartyId:" << m_lecturerPartyId << ", NewLecturerPartyId:" << lecturerPartyId << ", MuteIncomingLectureMode:" << (int)m_bMuteIncomingLectureMode;

	m_lecturerPartyId = lecturerPartyId;

	// if previous lecturer replaced by new lecturer, then unmute the new lecturer
	if (differentLecturer && isNewLecturer)
	{
		pAudioBridgeInterface->UpdateMute(m_lecturerPartyId, eMediaIn, eOff, OPERATOR_REQ_BY_ID);
	}
}

//--------------------------------------------------------------------------
void CConfAppMngrInterface::SetMuteIncomingLectureMode(BOOL yesNo)
{
	BOOL changed = m_bMuteIncomingLectureMode != yesNo;
	m_bMuteIncomingLectureMode = yesNo;
	if (changed)
		GetConfAppMngrImplementation()->MuteAllButLecturer(m_lecturerPartyId, (EOnOff)yesNo);
}
//---------------------------------------------------------------------------
//eFeatureRssDialin
//--------------------------------------------------------------------------
void CConfAppMngrInterface::HandleRecordingControlAck(BYTE status)
{
	if (GetConfAppMngrImplementation())
		GetConfAppMngrImplementation()->HandleRecordingControlStatus(status);
}

