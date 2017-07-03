#ifndef _CONF_APP_MNGR_INTERFACE_H_
#define _CONF_APP_MNGR_INTERFACE_H_

#include "Conf.h"
#include "ConfAppMngrInitParams.h"
#include "ConfAppMngr.h"

class CConfAppMngr;
class CBridgePartyInitParams;
class CBridgePartyExportParams;
class CBridgePartyDisconnectParams;

////////////////////////////////////////////////////////////////////////////
//                        CConfAppMngrInterface
////////////////////////////////////////////////////////////////////////////
class CConfAppMngrInterface : public CPObject
{
	CLASS_TYPE_1(CConfAppMngrInterface, CPObject)

public:
	                    CConfAppMngrInterface();
	                   ~CConfAppMngrInterface();

	virtual const char* NameOf() const                 { return "CConfAppMngrInterface";}

	void                HandleEvent(CSegment* pMsg);
	virtual void        UpdateConfParams();

public:
	void                Create(const CConfAppMngrInitParams* pAudioBridgeInitParams);
	void                ConnectPartyAudio(CBridgePartyInitParams* pBridgePartyInitParams, bool isIvrOn = true);
	void                ConnectPartyVideo(CBridgePartyInitParams* pBridgePartyInitParams);
	void                DisconnectPartyAudio(CBridgePartyDisconnectParams* pBridgePartyDisconnectParams);
	void                DisconnectPartyVideo(CBridgePartyDisconnectParams* pBridgePartyDisconnectParams);
	void                ExportPartyAudio(CBridgePartyExportParams* pBridgePartyExportParams);
	void                ExportPartyVideo(CBridgePartyExportParams* pBridgePartyExportParams);
	void                RemovePartyFromCAM(PartyRsrcID partyId, const char* partyName);
	DWORD               GetPartyToForce(PartyRsrcID partyId);
	void                SetMuteIncomingLectureMode(BOOL yesNo);
	void                SetLecturerParty(PartyRsrcID lecturerPartyId, BOOL isMuteIncoming);
	PartyRsrcID         GetLecturerPartyId()           { return m_lecturerPartyId; }
	void                HandleRecordingControlAck(BYTE status);

protected:
	void                CreateImplementation(const CConfAppMngrInitParams* pAudioBridgeInitParams);
	CConfAppMngr*       GetConfAppMngrImplementation() {return m_pConfAppMngrImplementation;}

private:
	CConfAppMngr*       m_pConfAppMngrImplementation;
	PartyRsrcID         m_lecturerPartyId;
	BOOL                m_bMuteIncomingLectureMode;
};

#endif
