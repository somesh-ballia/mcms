#ifndef _CVIDEOBRIDGE_H_
#define _CVIDEOBRIDGE_H_

#include <vector>
#include "Bridge.h"
#include "Layout.h"
#include "VisualEffectsParams.h"
#include "VideoDefines.h"
#include "VideoBridgeLectureModeParams.h"
#include "AutoLayoutSet.h"
#include "LPRData.h"
#include "AutoScanOrder.h"
#include "VideoHardwareInterface.h"
#include "BridgePartyVideoRelayMediaParams.h"
#include "RelayIntraDB.h"
#include "VideoBridgeCopEncoder.h"
#include "EventPackage.h"

// Timers opcodes
#define LECTURE_MODE_TOUT                      ((WORD)200)
#define PRESENTATION_MODE_TOUT                 ((WORD)201)
#define SWITCH_TOUT                            ((WORD)202)
#define BRIDGE_SETUP_TOUT                      ((WORD)203)
#define BRIDGE_DISCONNECT_TOUT                 ((WORD)204)
#define   AUTO_SCAN_TIMER                      ((WORD)206)

// merge LGT changes to 7.2.2
#define AFTER_STOP_CONTENT_TIMER               ((WORD)207)
#define AFTER_START_CONTENT_TIMER              ((WORD)208)

#define SWITCH_TIME_OUT_VALUE                  1*SECOND
#define BRIDGE_SETUP_TIME_OUT_VALUE            3*SECOND
#define BRIDGE_DISCONNECT_TIME_OUT_VALUE       3*SECOND
#define XCODE_BRIDGE_DISCONNECT_TIME_OUT_VALUE 3*SECOND

// merge LGT changes to 7.2.2
#define AFTER_STOP_CONTENT_TOUT                5*SECOND
#define AFTER_START_CONTENT_TOUT               5*SECOND

// Indication on Layout
#define NETWORK_QUALITY_UPDATE_TIMER           209
#define NETWORK_QUALITY_UPDATE_TOUT            (5*SECOND)

// ITP multi-link cascade timers
#define NEW_ITP_SPEAKER_TIMER                  ((WORD)210)
#define NEW_ITP_SPEAKER_TIME_OUT_VALUE         (5*SECOND)

// XCode Encoders connection timer
#define CONNECT_XCODE_VB_TOUT                  210
#define CONNECT_XCODE_VB_TIME_OUT_VALUE        2*SECOND
#define DISCONNECT_XCODE_VB_TOUT               211
#define DISCONNECT_XCODE_VB_TIME_OUT_VALUE     10*SECOND

typedef  struct
{
	BYTE screens[MAX_CASCADED_LINKS_NUMBER];
}LAYOUT_DEST_SCREEN_CONFIG;

// //Change Layout Improvement - Conf Layout
#define LAYOUT_NOT_CHANGED                     0
#define LAYOUT_CHANGED_ATTRIBUTES              1
#define LAYOUT_CHANGED                         2
#define LAYOUT_CHANGED_AUTOSCAN                3
#define	MULTIPLE_CHANGE_LAYOUT_BLOCK_SIZE	   20

class CVideoBridgeInitParams;
class CBridgePartyInitParams;
class CVideoBridgePartyCntl;
class CVideoBridgePartyCntlContent;
class CTextOnScreenMngr;
class CVideoBridgePartyInitParams;
class CVideoBridgeAutoScanParams;
class CGathering;
class CMessageOverlayInfo;
class CSiteNameInfo;
class CMediaList;
class CVideoBridgeCP;

class CRoomInfo;
typedef std::map<RoomID, CRoomInfo > VBRIDGE_ROOM_INFO_MAP;
class CTelepresenceLayoutMngr;
class CAvcToSvcParams;
class CTelepresenceLayoutLogic;


typedef std::map <eXcodeRsrcType, CVideoBridgeXCodeEncoder*> XCODE_ENCODERS_MAP;

typedef std::vector<DWORD> ChangedLayoutIdsList;
typedef std::vector<DWORD> IndicationIconIdsList;


typedef std::map <RoomID, list<DWORD>* > ChangedLayoutIdsOrderedByRoomId;
typedef std::list<DWORD> ChangedLayoutIdsOrderedByRoomIdList;

typedef std::vector<PartyRsrcID> CAVMCULinksVector;

// States definition
enum STATE
{
	IDLE_VB, CONNECTED, DISCONNECTING, INSWITCH
};

/////////////////////////////////////////////////////////////////////////////
///                       CAVMCUMngr
/////////////////////////////////////////////////////////////////////////////
class CAVMCUMngr : public CPObject
{
	CLASS_TYPE_1(CAVMVUMngr, CPObject)
public:
	CAVMCUMngr(CVideoBridgeCP* pBridge = NULL);
	virtual ~CAVMCUMngr();
    virtual const char* NameOf() const                                      {return "CAVMCUMngr";}
    void AddLinkToLinksVector(DWORD linkRsrcId);
    bool RemoveLinkFromLinksVector(DWORD linkRsrcId);
    bool FindLinkInLinksVector(DWORD linkRsrcId);
    CAVMCULinksVector& GetMCULinksVector(){return m_AVMCULinksVector;}
    void SetMasterPartyRsrcID(DWORD MSMasterPartyRsrcID){m_masterRsrcPartyID = MSMasterPartyRsrcID;}
    DWORD GetMasterPartyRsrcID(){return m_masterRsrcPartyID;}
    DWORD GetNumAVMCULinks(){return m_numOfAVMCULinks;}
    void  SetNumAVMCULinks(DWORD numOfMSSlaves){ m_numOfAVMCULinks = numOfMSSlaves;};
    LyncMsi GetMSAVMCULocalAUdioMSI();
    void  SetSpotLightSpeakerMsi(LyncMsi spotLightSpeakerMsi) {m_spotlightSpeakerMSI = spotLightSpeakerMsi;}
    LyncMsi GetSpotLightSpeakerMsi(){return m_spotlightSpeakerMSI;}
    void SetVideoSwitchingModeType(EventPackage::VideoSwitchingModeType videoSwitchingModeType){m_videoSwitchingModeType = videoSwitchingModeType;}
    EventPackage::VideoSwitchingModeType GetVideoSwitchingModeType(){return m_videoSwitchingModeType;}

    void SetMSAVMCULocalVideoMSI(LyncMsi localVideoMSI){m_localVideoMSI = localVideoMSI;}
    LyncMsi GetMSAVMCULocalVideoMSI(){return m_localVideoMSI;}

private:
    DWORD m_numOfAVMCULinks;
    DWORD m_masterRsrcPartyID;
    CAVMCULinksVector m_AVMCULinksVector;
    LyncMsi m_localVideoMSI;
    LyncMsi m_spotlightSpeakerMSI;
    EventPackage::VideoSwitchingModeType m_videoSwitchingModeType;

    CVideoBridgeCP*              m_pBridge;



};


////////////////////////////////////////////////////////////////////////////
//                        CRoomInfo
////////////////////////////////////////////////////////////////////////////
class CRoomInfo : public CPObject
{
	CLASS_TYPE_1(CRoomInfo, CPObject)

public:
	CRoomInfo();
	CRoomInfo(RoomID roomId, BOOL isCascadeLink, BYTE numLinks, eTelePresencePartyType roomEPType, const char* mainLinkName);
	virtual ~CRoomInfo();
	virtual const char*    NameOf() const                                      { return "CRoomInfo"; }

	void                   UpdateRoomType(BYTE newNumLinks, eTelePresencePartyType m_roomEPType);
	RoomID                 GetRoomId() const                                   {return m_roomId;}
	eTelePresencePartyType GetRoomEPType() const                               {return m_roomEPType;}
	void                   SetLinkPartyId(BYTE linkNum, PartyRsrcID partyId);
	PartyRsrcID            GetLinkPartyId(BYTE linkNum) const;
	BOOL                   GetIsCascade() const                                { return m_isCascadeLink;}
	BOOL                   IsRoomAllLinksConnected();
	BOOL                   IsPartyInRoom(PartyRsrcID partyId);
	BYTE                   UpdateHighestActiveLinkNumber();
	BYTE                   GetNumberOfActiveLinks() const                      { return m_activeLinks; }
	void                   SetNumberOfActiveLinks(BYTE numActiveLinks)         { m_activeLinks = numActiveLinks;}
	BYTE                   GetMaxLinks() const                                 { return m_maxLinks; }
	void                   SetIsAckPendingForChangeType(BOOL isPending)        { m_isWaitingForChangeITPTypeAck = isPending; }
	BOOL                   GetIsAckPendingForChangeType() const                { return m_isWaitingForChangeITPTypeAck; }
	void                   SetIsAckPendingForRealTPConnections(BOOL isPending) { m_isWaitingForRealTPConnections = isPending; }
	BOOL                   GetIsAckPendingForRealTPConnections() const         { return m_isWaitingForRealTPConnections; }
	const char*            GetMainLinkPartyName() const;

	CRoomInfo& operator    =(const CRoomInfo& other);

	void                   Dump(std::ostringstream& msg, bool fullInfo = false, bool print_to_log = false) const;
	void                   UpdateRoomTypeAndMaxScreens(eTelePresencePartyType roomEPType, BYTE maxScreens);
	WORD				   GetRoomPartyIds(std::set<DWORD>& roomPartiesIds);

private:
	RoomID                 m_roomId;
	eTelePresencePartyType m_roomEPType;
	BYTE                   m_maxLinks;
	BYTE                   m_activeLinks;
	PartyRsrcID            m_roomLinkPartyIDs[MAX_CASCADED_LINKS_NUMBER];
	BOOL                   m_isCascadeLink;
	BOOL                   m_isWaitingForChangeITPTypeAck;
	BOOL                   m_isWaitingForRealTPConnections;
	char                   m_mainLinkName[H243_NAME_LEN];
};


////////////////////////////////////////////////////////////////////////////
//                        CTelepresenceLayoutMngr
////////////////////////////////////////////////////////////////////////////
class CTelepresenceLayoutMngr : public CPObject
{
	CLASS_TYPE_1(CTelepresenceLayoutMngr, CPObject)

public:
	CTelepresenceLayoutMngr(CConfApi* pConfApi = NULL, CVideoBridgeCP* pBridge = NULL);
	virtual ~CTelepresenceLayoutMngr();
	virtual const char*          NameOf() const { return "CTelepresenceLayoutMngr"; }

	void                         AddNewRoom(RoomID roomId, BOOL isCascade, BYTE numLinks, eTelePresencePartyType roomEPType, const char* mainLinkName, BYTE linkNum = (BYTE)-1);
	void                         RemoveRoom(RoomID roomId);
	CRoomInfo                    UpdateRoomInfo(RoomID roomId, BYTE numLinks, eTelePresencePartyType newRoomEPType, BOOL isAckPending = FALSE);
	CRoomInfo                    GetRoomInfo(RoomID roomId);
	CRoomInfo                    GetRoomInfo(const char* linkPartyName, eTelePresencePartyType epType = eTelePresencePartyNone);
	BOOL                         UpdateRoomSubLink(RoomID roomId, BYTE linkNum, PartyRsrcID partyId);
	BOOL                         UpdateRoomSubLinkTIPEP(RoomID partyInfoRoomId, eTelePresencePartyType epType, const char* partyName, PartyRsrcID partyId, BYTE linkNum = (BYTE)-1);
	void                         OnSpeakerChanged(RoomID newSpeakerRoomId);
	void                         OnSpeakerChanged(RoomID viewerRoomId, RoomID newSpeakerRoomId);
	void                         SetViewerRoomLinksLayout(CRoomInfo& viewer, CRoomInfo& speaker);
	void                         SendBlankPrivateLayoutToVideoBridge(const char* pViewerName);
	void                         SetRoomPendingForRealTPConnections(RoomID roomId, BOOL isPending);
	BOOL                         GetRoomPendingForRealTPConnections(RoomID roomId);
	const VBRIDGE_ROOM_INFO_MAP& GetRoomList() const          { return m_RoomList; }
	BYTE                         GetLinkPartyIndex(const char* mainLinkName);
	void                         SetLinkPartyName(char* h243Dest, const char* mainLinkName, BYTE LinkNum);
	void                         SetConfLayoutToLegacyLayout(bool isSet);
	bool                         IsDisplayContentToLegacyEP() { return m_isDisplayContentToLegacyEP; }
	CTelepresenceLayoutLogic*    GetTelepresenceLayoutLogic(){ return m_pTelepresenceLayoutLogic; }
	CTelepresenceSpeakerModeLayoutLogic* GetTelepresenceSpeakerModeLayoutLogic(){ return m_pSpeakerModeLayoutLogic; }
	CTelepresencePartyModeLayoutLogic*   GetTelepresencePartyModeLayoutLogic()  { return m_pPartyModeLayoutLogic; }

	void                         DumpRoomList(bool fullInfo = false);
	void                         UpdateRoomTypeAndMaxScreens(RoomID roomId, eTelePresencePartyType roomEPType,BYTE maxScreens);

	WORD 						 GetRoomPartiesIds(DWORD partyRsrcId,std::set<DWORD>& roomPartiesIds);

private:
	void                         SendLayoutToVideoBridge(const char* pViewerName, CVideoLayout& resolved1x1Layout);
	void                         Set1X1MatrixCell(BYTE viewerLinkNum, BYTE speakerLinkNum, BYTE link1, BYTE link2, BYTE link3, BYTE link4);
	void                         Init1x1LayoutMatrix();

private:
	CConfApi*                    m_pConfApi;
	CVideoBridgeCP*              m_pBridge;
	VBRIDGE_ROOM_INFO_MAP        m_RoomList;
	LAYOUT_DEST_SCREEN_CONFIG    m_layoutConfFor1x1[MAX_CASCADED_LINKS_NUMBER][MAX_CASCADED_LINKS_NUMBER];
	BOOL                         m_isDisplayContentToLegacyEP;

	CTelepresenceRoomSwitchLayoutLogic*    m_pTelepresenceLayoutLogic;
	CTelepresenceSpeakerModeLayoutLogic*   m_pSpeakerModeLayoutLogic;
	CTelepresencePartyModeLayoutLogic*     m_pPartyModeLayoutLogic;
};



#define TRACE_DERIVED1 TRACEINTO << "Should be implemented in derived class";
#define TRACE_DERIVED2 TRACEINTO << "PartyId:" << partyId << " – Should be implemented in derived class";

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridge
////////////////////////////////////////////////////////////////////////////
class CVideoBridge : public CBridge
{
	CLASS_TYPE_1(CVideoBridge, CBridge)

public:
	CVideoBridge();
	virtual ~CVideoBridge();
	virtual const char*                   NameOf() const     { return "CVideoBridge"; }

	virtual void                          Create(const CVideoBridgeInitParams* pVideoBridgeInitParams);
	virtual void                          Destroy();
	virtual void                          InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams);
	virtual void*                         GetMessageMap()    { return (void*)m_msgEntries; }

	// Api from Conf
	virtual void                          DeletePartyFromConf(const char* partyName);

	// Api from PartyCntl
	virtual void                          UpdateMute(PartyRsrcID partyId, EOnOff eOnOff, WORD srcRequest);
	virtual void                          UpdateVideoInParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams)                           { TRACE_DERIVED1 }
	virtual void                          UpdateVideoOutParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams)                          { TRACE_DERIVED1 }
	virtual void                          UpdateVideoRelayInParams(CTaskApp* pParty, CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayMediaParams)  { TRACE_DERIVED1 }
	virtual void                          UpdateVideoOutParams(eXcodeRsrcType eEncoderPartyType, CBridgePartyVideoOutParams* pBridgePartyVideoOutParams)    { TRACE_DERIVED1 }
	virtual void                          UpdateVideoRelayOutParams(CTaskApp* pParty, CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayMediaParams) { TRACE_DERIVED1 }
	virtual void                          VideoRefresh(PartyRsrcID partyId);
	virtual void                          EventModeIntraPreviewReq(PartyRsrcID partyId)                                                                     { TRACE_DERIVED2 }
	virtual void                          UpdateEMPartyStartCascadeLinkAsLecturerPendingMode(CTaskApp* pParty, WORD cascadeLecturerCopEncoderIndex)         { TRACE_DERIVED1 }
	virtual void                          RelayEpAskForIntra(CSegment* pParam)                                                                              { TRACE_DERIVED1 }
	virtual void                          UpdateNewSpeakerIndReceivedFromRemoteMCU(PartyRsrcID partyId, DWORD numOfActiveLinks, BYTE itpType)               { TRACE_DERIVED2 }
	virtual void                          UpdateNewSpeakerAckIndReceivedFromRemoteMCU(PartyRsrcID partyId)                                                  { TRACE_DERIVED2 }
	virtual void                          AckOnIvrScpShowSlide(PartyRsrcID partyId, BYTE bIsAck)                                                            { TRACE_DERIVED2 }
	virtual void                          AckOnIvrScpStopShowSlide(PartyRsrcID partyId, BYTE bIsAck)                                                        { TRACE_DERIVED2 }

	// TelePresence
	virtual void                          TurnOnOffTelePresence(BYTE onOff, CVisualEffectsParams* pVisualEffects)                                           { TRACE_DERIVED1 }
	virtual void                          UpdatePartyTelePresenceMode(CTaskApp* pParty, eTelePresencePartyType partyNewTelePresenceMode)                    { TRACE_DERIVED1 }
	virtual void           				  SetTelepresenceLayoutMode(ETelePresenceLayoutMode newLayoutMode)        								            { TRACE_DERIVED1 } // TELEPRESENCE_LAYOUTS
	virtual void                          GetCroppingValues(DWORD& croppingHorr, DWORD& croppingVer);
	virtual void                          SetSiteName(const char* partyName, const char* visualName);

	// Api from CAM
	virtual void                          IvrPartyCommand(PartyRsrcID partyId, OPCODE opcode, CSegment* pDataSeg);
	virtual void                          PLC_SetPartyPrivateLayout(PartyRsrcID partyId, LayoutType newPrivateLayoutType)                                   { TRACE_DERIVED2 }
	virtual void                          PLC_PartyReturnToConfLayout(PartyRsrcID partyId)                                                                  { TRACE_DERIVED2 }
	virtual void                          PLC_ForceToCell(PartyRsrcID partyId, PartyRsrcID partyImageToSee, BYTE cellToForce)                               { TRACE_DERIVED2 }
	virtual void                          PLC_CancelAllPrivateLayoutForces(PartyRsrcID partyId)                                                             { TRACE_DERIVED2 }
	virtual void                          DisplayPartyListOnScreen(PartyRsrcID partyId, BYTE IsConfSecure, char** AudioArr, WORD numAudioParties, char** VideoArr, WORD numVideoParties, BOOL IsChairOnly) { TRACE_DERIVED2 }
	virtual void                          DisplayPartyTextOnScreen(PartyRsrcID partyId, char** displayStringArr, WORD displayStringArrNo)                   { TRACE_DERIVED2 }
	virtual void                          DisplayPartyTextListOnScreen(PartyRsrcID partyId, CTextOnScreenMngr* TextMsgList, DWORD timeout)                  { TRACE_DERIVED2 }
	virtual void                          StopDisplayPartyTextOnScreen(PartyRsrcID partyId)                                                                 { TRACE_DERIVED2 }
	virtual void                          DisplayGatheringOnScreen(CGatheringManager* pGatheringManager)                                                    { TRACE_DERIVED1 }
	virtual bool                          IsBridgePartyVideoOutStateIsConnected(const char* partyName)                                                      { TRACE_DERIVED1 return false; }
	virtual void                          DisplayGatheringOnScreen(const char* partyName, CGatheringManager* pGatheringManager)                             { TRACE_DERIVED1 }
	virtual void                          SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects)                                                      { TRACE_DERIVED1 }
	virtual void                          SetVisualEffectsParams(const std::string& partyName, CVisualEffectsParams* pVisualEffects)                        { TRACE_DERIVED1 }

	// Images and speaker management
	virtual void                          ConnectPartyToPCMEncoder(PartyRsrcID partyId)                                                                     { TRACE_DERIVED2 }
	virtual void                          DisconnectPartyFromPCMEncoder(PartyRsrcID partyId)                                                                { TRACE_DERIVED2 }
	virtual DWORD                         GetPartyResolutionInPCMTerms(PartyRsrcID partyId)                                                                 { TRACE_DERIVED2 return (DWORD)-1; }
	virtual void                          ChangeSpeakerNotation(PartyRsrcID partyId, DWORD imageId)                                                         { TRACE_DERIVED2 }
	virtual CLayout*                      GetPartyLayout(const char* partyName)              { return NULL; }
	virtual void                          UpdateMessageOverlayForParty(PartyRsrcID partyId, CMessageOverlayInfo* pMessageOverlayInfo)                       { TRACE_DERIVED2 }
	virtual void                          UpdateMessageOverlayStopForParty(PartyRsrcID partyId)                                                             { TRACE_DERIVED2 }

	// Mix Avc Svc
	virtual void                          UpgradeToMixAvcSvcForRelayParty(PartyRsrcID partyId)                                                              { TRACE_DERIVED2 }
	virtual void                          UpgradeToMixAvcSvcForNonRelayParty(PartyRsrcID partyId, CAvcToSvcParams* pAvcToSvcParams)                         { TRACE_DERIVED2 }
	virtual void                          DowngradeMixAvcSvcRelayParty(PartyRsrcID partyId)                                                                 { TRACE_DERIVED2 }
	virtual void                          DowngradeMixAvcSvcNonRelayParty(PartyRsrcID partyId)                                                              { TRACE_DERIVED2 }

	virtual CVideoBridgePartyCntlContent* GetContentDecoder()                          { return NULL; }

	void                                  AddPartyImage(PartyRsrcID partyId);
	bool                                  DelPartyImage(PartyRsrcID partyId);
	bool                                  IsPartyImageExistInImageVector(PartyRsrcID partyId);
	CImage*                               GetPartyImage(CLayout& layout, WORD position);

	void                                  SetPartyImageSpeakerId(PartyRsrcID partyId);
	PartyRsrcID                           GetPartyImageSpeakerId() const;
	PartyRsrcID                           GetPartyImageIdByPosition(WORD position) const;
	WORD                                  GetPartyImageVectorSize() const              { return m_pPartyImageVector->size(); }
	WORD                                  GetPartyImageVectorSizeUnmuted() const;
    // AV MCU
	PartyRsrcID                           GetOldestAVMCUSpeaker(PartyRsrcID deletedPartyId = SOURCE_NONE_DOMINANT_SPEAKER_MSI);
	bool                                  IsValidMSI(LyncMsi msi, bool isIncldueLocalMsi = true);
	WORD                                  GetNumOfAVMCUBranches();

	CTaskApp*                             GetLastActiveAudioSpeakerRequest(void) const { return m_pLastActiveAudioSpeakerRequest; }
	PartyRsrcID                           GetLastActiveAudioSpeakerId() const;
	// Applications interface
	const char*                           GetLecturerName(void) const;
	BYTE                                  IsSameLayout(void) const                     { return m_IsSameLayout; }
	BOOL                                  SetAutoLayout(BOOL isAutoLayout);
	virtual void                          StartAutoLayout();

	LayoutType                            GetCurrentLayoutType()                       { return m_layoutType; }
	EIconType                             GetRecordingIcon(void) const                 { return m_eRecordingIcon; }
	void                                  SetRecordingIcon(EIconType eRecordingIcon)   { m_eRecordingIcon = eRecordingIcon; }

	WORD								  GetNumAudioParticipants(void) const				  { return m_numAudioParticipants; }
	void								  SetNumAudioParticipants(WORD numAudioParticipants)   { m_numAudioParticipants = numAudioParticipants; }

	// API to layout handler and party control
	virtual CLayout*                      GetReservationLayout(void) const;
	CLayout*                              GetAnyPartyLayout() const;
	virtual CLayout*                      GetConfLayout()                              { return NIL(CLayout); }

	virtual void                          SendMessageOverlayToNewConnectedPartyIfNeeded(CSegment* pParam) { TRACE_DERIVED1 }
	CVideoBridgeAutoScanParams*           GetVideoBridgeAutoScanParams() const         { return m_pAutoScanParams; }
	CTelepresenceLayoutMngr*              GetTelepresenceLayoutMngr() const            { return m_pTelepresenceLayoutMngr; }
	CAVMCUMngr*                           GetAVMCUMngr()const                          { return m_pAVMCUMngr;}

	PartyMonitorID                        GetMonitorPartyId(const CTaskApp* pParty) const;

	virtual LayoutType                    GetLegacyContentDefaultLayout()              { return CP_LAYOUT_1P4VER; }
	BOOL                                  IsXCodeConf();
	BOOL                                  IsASSIPConf();
	CPartyRsrcDesc*                       GetXCodeContentDecoderPartyRsrc()            { return m_pConf->GetXCodeContentDecoderPartyRsrc(); }
	PartyMonitorID                        GetContentDecoderXCodeMonitorPartyId()       { return m_pConf->GetContentDecoderXCodeMonitorPartyId(); }
	void                                  AddChangedLayoutIdToVector(DWORD layoutId);
	void                                  EmptyChangedLayoutIdsVector();
	void                                  VerifyIdVectorIsEmpty();

	BOOL                                  IsTipCompatibilityMode() const;

	void                                  AddIndicationIconIdToVector(DWORD indicationIconId);
	void                                  EmptyIndicationIconIdsVector();
	void                                  VerifyIndicationIconIdVectorIsEmpty();

	virtual void                          UpdateVidBrdgTelepresenseEPInfo(PartyRsrcID partyId, CTelepresenseEPInfo* pTelepresenseEPInfo, const char* pSiteName) { TRACE_DERIVED2 }
	CTelepresenceLayoutMngr*              GetTelepresenceLayoutMngr() { return 	m_pTelepresenceLayoutMngr; }
	BOOL                                  GetTelepresenceOnOff() const { return m_telepresenceOnOff; }
	BOOL                                  GetManageTelepresenceLayoutsInternally() const { return m_bManageTelepresenceLayoutsInternally; }
	virtual BOOL                          GetIsAutoLayout() const {return m_isAutoLayout;}
	virtual void                          SetIsAutoLayout(BOOL isAutoLayout){m_isAutoLayout = isAutoLayout;}

protected:
	CVideoBridge(const CVideoBridge&);                                // not implemented ??
	CVideoBridge& operator                =(const CVideoBridge&);     // not implemented ??

	// lecture mode
	virtual void                          StartLectureMode();
	virtual void                          EndLectureMode(BYTE removeLecturer);
	ePartyLectureModeRole                 GetLectureModeRoleForParty(const char* pPartyName);
	void                                  UpdateConfDBLectureMode() const;

	// presentation mode
	virtual void                          StartPresentationMode();

	// update applications
	void                                  ApplicationActionsOnAudioSpeakerChange();
	virtual void                          ApplicationActionsOnAddPartyToMix(CVideoBridgePartyCntl* pPartyCntl);
	virtual void                          ApplicationActionsOnRemovePartyFromMix(CVideoBridgePartyCntl* pPartyCntl);
	virtual void                          ApplicationActionsOnDeletePartyFromConf(const char* pDeletedPartyName);

	// connect party
	virtual void                          ConnectParty(CVideoBridgePartyCntl* pPartyCntl, BOOL isIVR);
	virtual void                          AddPartyToConfMixAfterVideoInSynced(const CTaskApp* pParty);
	void                                  ResendLastActiveVideoSpeakerRequest();
	virtual void                          NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl);
	virtual void                          CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl, CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
	virtual WORD                          CorrectPartyInitParams(CBridgePartyInitParams& partyInitParams);
	virtual void                          VENUS_SetPartyPrivateLayout(PartyRsrcID partyId, LayoutType newPrivateLayoutType) { TRACE_DERIVED2 }

	// disconnect / delete party
	virtual void                          RemovePartyFromConfMixBeforeDisconnecting(const CTaskApp* pParty);
	virtual void                          RemovePartyFromAnyConfSettingsWhenDeletedFromConf(const char* pDeletedPartyName);
	void                                  RemovePartyFromConfMixBeforeDisconnectingDISCONNECTING(const CTaskApp* pParty);

	// general functions
	virtual void                          GetForcesFromReservation(CCommConf* pCommConf);
	virtual void                          UpdateDB_ConfLayout();
	RequestPriority                       GetRequestPriority(WORD srcReq);
	void                                  SendToPartyVIN(const CTaskApp* pParty, const CTaskApp* pImagePartySee);

	// State machine action functions
	void                                  OnConfDisConnectConfCONNECTED(CSegment* pParam);
	void                                  OnConfDisConnectConfDISCONNECTING(CSegment* pParam);
	void                                  OnConfTerminateCONNECTED(CSegment* pParam);
	virtual void                          OnConfTerminateDISCONNECTING(CSegment* pParam);
	virtual void                          OnConfConnectPartyCONNECTED(CSegment* pParam);
	void                                  OnConfConnectPartyDISCONNECTING(CSegment* pParam);
	virtual void                          OnConfDisConnectPartyCONNECTED(CSegment* pParam);
	virtual void                          OnConfDisConnectPartyDISCONNECTING(CSegment* pParam);
	void                                  OnConfDisConnectPartyWithTelepresence(CVideoBridgePartyCntl* pPartyCntl);
	void                                  OnConfExportPartyCONNECTED(CSegment* pParam);
	virtual void                          OnEndPartyExportCONNECTED(CSegment* pParam);
	virtual void                          OnVideoInSyncedCONNECTED(CSegment* pParam);
	void                                  OnVideoInSyncedDISCONNECTING(CSegment* pParam);
	virtual void                          OnEndPartyConnectCONNECTED(CSegment* pParam);
	void                                  OnEndPartyConnectDISCONNECTING(CSegment* pParam);
	virtual void                          OnEndPartyConnectIVRModeCONNECTED(CSegment* pParam);
	void                                  OnEndPartyConnectIVRModeDISCONNECTING(CSegment* pParam);
	virtual void                          OnEndPartyDisConnect(CSegment* pParam);
	void                                  OnEndPartyUpdateVideoInCONNECTED(CSegment* pParam);
	void                                  OnEndPartyUpdateVideoInDISCONNECTING(CSegment* pParam);
	void                                  OnEndPartyUpdateVideoOutCONNECTED(CSegment* pParam);
	void                                  OnEndPartyUpdateVideoOutDISCONNECTING(CSegment* pParam);
	void                                  OnPartyImageInCellZeroChangedCONNECTED(CSegment* pParam);
	void                                  OnConfAudioSpeakerChangedCONNECTED(CSegment* pParam);
	void                                  OnConfUpdateAutoScanIntervalCONNECTED(CSegment* pParam);
	void                                  OnConfSetLectureModeCONNECTED(CSegment* pParam);
	void                                  OnConfDeletePartyFromConfCONNECTED(CSegment* pParam);
	virtual void                          OnDisplayIndicationIconCONNECTED(CSegment* pParam);
	void                                  OnSetSiteNameCONNECTED(CSegment* pParams);
	void                                  OnPartyUpdateMessageOverlayCONNECTED(CSegment* pParam);
	// BRIDGE-8051
	virtual void                          OnEndPartyIvrModeAfterResumeCall(CSegment* pParam);

	void                                  OnFreezePic(CSegment* pParams);
	BOOL                                  IsLegacyBridge();

	CTaskApp*                             GetPartyTaskApp(PartyRsrcID partyId);

	// VNGR-26449 - unencrypted conference message
	BYTE                                  GetIsPermanentSecureMessage()                                { return m_isPermanentSecureMessage; }
	void                                  SetIsPermanentSecureMessage(BYTE isPermanentSecureMessage)   { m_isPermanentSecureMessage = isPermanentSecureMessage; }
	BYTE                                  GetIsPermanentMessageOverlay()                               { return m_isPermanentMessageOverlay; }
	void                                  SetIsPermanentMessageOverlay(BYTE isPermanentMessageOverlay) { m_isPermanentMessageOverlay = isPermanentMessageOverlay; }
	string                                GetMessageOverlayText()                                      { return m_messageOverlayText; }
	void                                  SetMessageOverlayText(string messageOverlayText)             { m_messageOverlayText = messageOverlayText; }

	void                                  SendChangeLayoutToMultipleParties();
	void                                  SendBlockOfLayoutIds(DWORD numberOfLayoutIdsForMultipleSend, WORD LayoutIdsBlockNumber);
	void                                  SendIndicationIconChangeToMultipleParties();
	void                                  SendBlockOfIndicationIconIds(DWORD numberOfIndicationIconIdsForMultipleSend, WORD IndicationIconIdsBlockNumber);
	void                                  SendMsgToMPL(OPCODE opcode, CSegment* pParams);
	ChangedLayoutIdsOrderedByRoomId*      BuildMapOfChangedLayoutIdListOrderedByRoomId();

	// //Change Layout Improvement - Conf Layout
	DWORD                                 IsLayoutChange(CLayout* pNewLayout, CLayout* pOldLayout);
	void                                  UpdateConfLayout(CLayout* pNewLayout);
	void                                  BuildLayout(CLayout& rResultLayout, LayoutType newLayoutType, BOOL isCop = TRUE);         // isCop=TRUE because originally this is a COP function
	void                                  RemoveOldConfForcesAndBlankes(CLayout& rResultLayout, BOOL isCop = TRUE) const;           // isCop=TRUE because originally this is a COP function
	void                                  SetConfForces(CLayout& rResultLayout, LayoutType newLayoutType, BOOL isCop = TRUE) const; // isCop=TRUE because originally this is a COP function
	void                                  FillLayout(CLayout& rResultLayout, BYTE remove_lecturer, BOOL isCop = TRUE);              // isCop=TRUE because originally this is a COP function
	void                                  CreateVectorOfImagesReadyForSetting(CPartyImageVector& rImagesReadyForSetting) const;
	void                                  UpdateDecodersParamsInImages(CLayout& rResultLayout) const;
	void                                  RemoveLecturerImageFromImagesVector(CPartyImageVector& rImagesReadyForSetting) const;
	void                                  RemoveLecturerForce(CLayout& rResultLayout) const;
	void                                  SetForcedPartiesInLayout(CLayout& rResultLayout, CPartyImageVector& rImagesReadyForSetting, WORD& rNumCellsLeftToFill) const;
	BYTE                                  FillAutoScanImageInLayout(CLayout& rResultLayout, BYTE needToStartTimer = 1);
	void                                  SetSpeakerImageToBigCellInLayout2Plus8IfNeeded(CPartyImageVector& rImagesReadyForSetting, CLayout& rResultLayout, CLayout* pPreviouselySeenLayout, WORD& rNumCellsLeftToFill /*, BOOL isCop*/) const;
	BYTE                                  IsLayoutWithSpeakerPicture(const LayoutType layoutType) const;
	void                                  SetSpeakerImageToBigCellInLayout(CLayout& rResultLayout, CPartyImageVector& rImagesReadyForSetting, WORD& rNumCellsLeftToFill) const;
	void                                  SetImagesToPreviouseSeenCellsInLayout(CLayout& rResultLayout, CPartyImageVector& rImagesReadyForSetting, CLayout* pPreviouselySeenLayout, WORD& rNumCellsLeftToFill) const;
	void                                  SetImagesToAnyUnusedCell(CLayout& rResultLayout, CPartyImageVector& rImagesReadyForSetting) const;
	ECopEncoderMaxResolution              GetCopEncoderMaxResolution() const;
	CVideoBridgePartyCntl*                GetLecturer(BOOL bNewLecturer = FALSE) const;
	CVideoBridgePartyCntl*                GetLecturer(CVideoBridgeLectureModeParams& rVideoBridgeLectureModeParams) const;
	BYTE                                  IsLecturer(CVideoBridgePartyCntl* pVideoBridgePartyCntl, BOOL bNewLecturer = FALSE) const;
	BYTE                                  IsLecturer(const char* partyName, BOOL bNewLecturer = FALSE) const;
	bool                                  IsEQ();

	BOOL                                  m_isAutoLayout;
	CLayout*                              m_ConfLayout;                 // active layout
	CLayout*                              m_pReservation[CP_NO_LAYOUT]; // reservation layout
	LayoutType                            m_layoutType;                 // current layout
	CTaskApp*                             m_pLastSpeakerNotationParty;  // Save the last speakerNotation party to avoid redundant msgs to VideoCard
	CCopLectureModeCntl*                  m_pCopLectureModeCntl;
	eVideoConfType                        m_VideoConfType;              // initiate to  eVideoConfTypeCopHD108025fps / eVideoConfTypeCopHD72050fps by first level encoder
	CopRsrcsParams*                       m_pCopRsrcs;                  // we keep the CopRsrcsParams to update the decoders partyRsrc Id on create

	// images and speaker management
	CPartyImageVector*                    m_pPartyImageVector;
	CTaskApp*                             m_pLastActiveVideoSpeakerRequest;
	CTaskApp*                             m_pLastActiveAudioSpeakerRequest;
	CVideoBridgeLectureModeParams*        m_pLectureModeParams;
	CAutoLayoutSet*                       m_pAutoLayoutSet;
	CVideoBridgeAutoScanParams*           m_pAutoScanParams;
	CMessageOverlayInfo*                  m_pMessageOverlayInfo;
	BYTE                                  m_IsSameLayout;
	eVideoQuality                         m_videoQuality;   // Conference video quality sharpness/motion
	EIconType                             m_eRecordingIcon; // Current recording icon
	WORD                                  m_numAudioParticipants;
	BYTE                                  m_isSiteNamesEnabled;
	CSiteNameInfo*                        m_pSiteNameInfo;
	CTelepresenceLayoutMngr*              m_pTelepresenceLayoutMngr;
	CAVMCUMngr*                           m_pAVMCUMngr;
	BOOL                                  m_telepresenceOnOff;
	BOOL                                  m_bManageTelepresenceLayoutsInternally;

	// VNGR-26449 - unencrypted conference message
	BYTE                                  m_isPermanentSecureMessage;
	BYTE                                  m_isPermanentMessageOverlay;
	string                                m_messageOverlayText;

	// Change Layout Improvement - Layout Shared Memory
	// List of layout IDs (currently partyRsrcIds) that is updated by CBridgePartyVideoOut during BuildLayout.
	// This list holds layout IDs of parties that h ad a layout change that requires sending a request to media card.
	// Sending a message to MplApi (MULTIPLE_PARTIES_CHANGE_LAYOUT_REQ) is done from CVideoBridge per action (e.g. Add Image),
	// with this list of parties IDs that were influenced by this action.
	ChangedLayoutIdsList*                 m_changedLayoutIdsList;
	IndicationIconIdsList*                m_indicationIconIdsList;
	BYTE                                  m_isMuteAllVideoButLeader;
	BYTE                                  m_isMuteAllVideoButLeaderForJustIncoming;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeCP
////////////////////////////////////////////////////////////////////////////
class CVideoBridgeCP : public CVideoBridge
{
	CLASS_TYPE_1(CVideoBridgeCP, CVideoBridge)

public:
	CVideoBridgeCP();
	virtual ~CVideoBridgeCP();
	virtual void              Create(const CVideoBridgeInitParams* pVideoBridgeInitParams);
	virtual void              CreateBase(const CVideoBridgeInitParams* pVideoBridgeInitParams);
	virtual void              InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams);
	const char*               NameOf() const  {return "CVideoBridgeCP";}
	virtual void*             GetMessageMap() {return (void*)m_msgEntries;}

	// CVideoBridgeInterface functions
	// Api from PartyCntl
	virtual void              UpdateVideoInParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void              UpdateVideoOutParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void              UpdateVideoRelayInParams(CTaskApp* pParty, CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams);
	virtual void              UpdateVideoRelayOutParams(CTaskApp* pParty, CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams);
	virtual void              RelayEpAskForIntra(CSegment* pParam);
	virtual void              AckOnIvrScpShowSlide(PartyRsrcID partyRsrcID, BYTE bIsAck);
	virtual void              AckOnIvrScpStopShowSlide(PartyRsrcID partyRsrcID, BYTE bIsAck);

	// Api from CAM
	virtual void              PLC_SetPartyPrivateLayout(PartyRsrcID partyID, LayoutType newPrivateLayoutType);
	virtual void              PLC_PartyReturnToConfLayout(PartyRsrcID partyID);
	virtual void              PLC_ForceToCell(PartyRsrcID partyID, PartyRsrcID partyImageToSee, BYTE cellToForce);
	virtual void              PLC_CancelAllPrivateLayoutForces(PartyRsrcID partyID);
	// TelePresence
	virtual void              TurnOnOffTelePresence(BYTE onOff, CVisualEffectsParams* pVisualEffects);
	virtual void              UpdatePartyTelePresenceMode(CTaskApp* pParty, eTelePresencePartyType partyNewTelePresenceMode);
	virtual void           	  SetTelepresenceLayoutMode(ETelePresenceLayoutMode newLayoutMode);// TELEPRESENCE_LAYOUTS

	// API to layout handler and party control
	virtual CLayout*          GetReservationLayout(void) const;
	virtual void              DisplayPartyListOnScreen(PartyRsrcID partyId, BYTE IsConfSecure, char** AudioArr, WORD numAudioParties, char** VideoArr, WORD numVideoParties, BOOL IsChairOnly);
	virtual void              DisplayPartyTextOnScreen(PartyRsrcID partyId, char** displayStringArr, WORD displayStringArrNo);
	virtual void              DisplayPartyTextListOnScreen(PartyRsrcID partyId, CTextOnScreenMngr* TextMsgList, DWORD timeout);
	virtual void              StopDisplayPartyTextOnScreen(PartyRsrcID partyId);
	virtual void              OnEndImportParty(CSegment* pParam);
	virtual void              DisplayGatheringOnScreen(CGatheringManager* pGatheringManager);
	virtual void              DisplayGatheringOnScreen(const char* partyName, CGatheringManager* pGatheringManager);
	virtual bool              IsBridgePartyVideoOutStateIsConnected(const char* partyName);
	virtual void              SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects);
	virtual void              SetVisualEffectsParams(const std::string& partyName, CVisualEffectsParams* pVisualEffects);

	virtual void              OnEndPartyConnectCONNECTED(CSegment* pParam);
	virtual void              OnMainAllMSInSlavesConnected(CSegment* pParam);
	virtual void              OnEventPackageLyncConfInfoUpdatedConnected(CSegment* pParam);
	virtual void              OnMainLcalRMXAVMCUMSIInd(CSegment* pParam);
	void                      OnEndPartyConnectWithTelepresence(CVideoBridgePartyCntl* pPartyCntl);
	void                      LoadTelepresenceInfoFromExistingParties();
	void                      OnConfSpeakersChangedWithTelepresence(CTaskApp* pFormerVideoSpeaker, CTaskApp* pNewVideoSpeaker);
	virtual void              UpdateNewSpeakerIndReceivedFromRemoteMCU(PartyRsrcID partyId, DWORD numOfActiveLinks, BYTE itpType);
	virtual void              UpdateNewSpeakerAckIndReceivedFromRemoteMCU(PartyRsrcID partyId);
	virtual void              OnTimerITPSpeakerChange(CSegment* pParam);
	void                      OnConfSpeakerPartyITPTypeChanged(BOOL updateLinksOnly = TRUE);
	void                      OnTimerITPTelepresencePartyConnection(CSegment* pParam);
	void                      OnStopPendingForLinksConnections(PartyRsrcID linkPartyId);
	BOOL                      CheckIfRoomContainsTheCurrentSpeakerEp(CRoomInfo& room_info);
	void                      UpdateAllCascadeRoomPartiesSpeakerITPTypeChanged(const CRoomInfo& updated_room);
	void                      UpdateRoomPartiesITPTypeChanged(const CRoomInfo& target_room, DWORD numOfActiveLinks, eTelePresencePartyType itpType, BOOL isAckPending = FALSE);

	virtual void              UpdateVideoClarity(WORD isVideoClarity);
	virtual DWORD             GetPartyResolutionInPCMTerms(PartyRsrcID partyId);
	virtual void              UpdateMessageOverlayForParty(PartyRsrcID partyId, CMessageOverlayInfo* pMessageOverlayInfo);
	virtual void              UpdateMessageOverlayStopForParty(PartyRsrcID partyId);
	virtual void              ChangeSpeakerNotation(PartyRsrcID partyId, DWORD imageId);
	virtual CLayout*          GetPartyLayout(const char* partyName);
	DWORD                     TranslateToPcmResolution(CBridgePartyVideoOutParams& bridgePartyVideoOutParams);
	virtual void              SendMessageOverlayToNewConnectedPartyIfNeeded(CSegment* pParam);

	CVideoOperationPointsSet* GetConfVideoOperationPointsSet() const;
	bool                      GetOperationPointFromList(int layerId, VideoOperationPoint& videoOperationPoint) const;
	void                      GetValidRelayImagesForParty(PartyRsrcID partyRsrcId, std::list<const CImage*>& listImages,bool isCascadeLink) const;
	const CImage*             GetImageBySsrc(unsigned int ssrc) const;

	void                      AskRelayEpsForIntra(AskForRelayIntra& epIntraParams);
	DWORD                     GetNumberOfNonRelayImages() const {return m_NumberOfNonRelayImages;}
	void                      UpdateRelayImageParams(PartyRsrcID partyId);

	// VNGR-26449 - unencrypted conference message
	void                      OnPartySendSecureMessageCONNECTED(CSegment* pParam);
	void                      NotifyOnRemovedVideoInStreams(DWORD partyRscId, DWORD specificSsrc = 0xFFFFFFFF);
	int                       GetMaxAllowedLayerId()            { return m_maxAllowedLayerIdForMixSvcToAvc; }
	int                       GetMaxAllowedLayerIdHighRes()         { return m_maxAllowedLayerIdForMixSvcToAvcHighRes; }
	void                      ChangeConfLayoutType(LayoutType newLayoutType);
	void                      OnMrmpStreamIsMustAckCONNECTED(CSegment* pParam);

	void                      DumpTelepresenceInfo();
	void                      ChangeRoomLayout(const CRoomInfo& room_info);
	void                      SendChangeLayoutAfterRoomConnected(CRoomInfo& room_info, bool isSpeakerInRoom);
	virtual void              UpdateVidBrdgTelepresenseEPInfo(DWORD partyRsrcId, CTelepresenseEPInfo* pTelepresenseEPInfo, const char* pSiteName); //_e_m_
	void                      OnEndWaitingForTelepresenseEPInfo(DWORD partyRsrcId, RoomID roomId, bool epUpdatedToTelepresence = FALSE, bool isSiteNameChange = false); //_e_m_
	void                      EndPartyConnectTelepresenceNon(CVideoBridgePartyCntl* pPartyCntl);
	void                      EndPartyConnectTelepresenceEP(CVideoBridgePartyCntl* pPartyCntl);
	void                      EndPartyConnectTelepresenceMultiLink(CVideoBridgePartyCntl* pPartyCntl);

	// AV-MCU
	void                      PrepareMultiVSRMessage(ST_VSR_MUTILPLE_STREAMS* multipleVsr);
	void                      AssignVideoMSIForEachMSPartyCntl();
	void 					  UnmuteMsInSlave(PartyRsrcID partyRsrcId);
	void                      FetchVideoMSIForLastMSPartyCntlsFromeEventPackage();
	void                      AddAVMCUPartyCntlToBridge(CVideoBridgePartyCntl* pPartyCntl);
	void                      replaceMSIOrDisconnectSlave(LyncMsi videoMsi, PartyRsrcID partyRsrcIDOfReplacedMSI, EventPackage::LyncMsiList& arrVideoMsi);
	void 					  DeleteAvMcuSlave(PartyRsrcID slavePartyRsrcId);
	void                      ActionsOnAVMCUSpotLightOnOff();
	void                      ActionsToResumeDisconnectedSpotLightSpeaker();
	void                      PutSpotLightSourceOnMasterAVMCU();
	bool                      BuildAVMCUSiteName(LyncMsi videoMsi, char* siteName);
	bool                      BuildAVMCUSiteName(const std::string& userName, const std::string& endpointName, char* siteName);
	void                      SendPrivateLayoutEventsToAllAVMCUOutSlavesandToMain(WORD event, CVideoBridgePartyCntl* initiotorVideoPartyControl,CVideoLayout& layout, WORD isPrivateButtonOnly);
	void                      SendVSRToAVMCUMain();

	BYTE                      IsConfInActiveContentPresentation()              { return m_isConfInActiveContentPresentation; }

protected:
	// auto layout
	virtual void              StartAutoLayout();
	virtual void              StartPresentationMode();

	// connect party
	virtual void              ConnectParty(CVideoBridgePartyCntl* pPartyCntl, BOOL isIVR);
	virtual void              AddPartyToConfMixAfterVideoInSynced(const CTaskApp* pParty);
	virtual void              AddPartyToConfMix(const CTaskApp* pParty);
	virtual void              NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl);
	virtual void              NewVideoRelayPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl);
	virtual void              CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl, CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);

	WORD                      GetSizeOfImageVectorWithoutMute();     // BRIDGE-4043

	// disconnect / delete party
	virtual void              RemovePartyFromConfMixBeforeDisconnecting(const CTaskApp* pParty);
	virtual void              RemovePartyFromAnyConfSettingsWhenDeletedFromConf(const char* pDeletedPartyName);

	// general functions
	virtual void              UpdateDB_ConfLayout();
	virtual void              GetForcesFromReservation(CCommConf* pCommConf);
//	void                          ChangeConfLayoutType(const LayoutType newLayoutType);
	virtual void              AudioSpeakerChanged(CTaskApp* pNewAudioSpeaker);
	virtual void              VENUS_SetPartyPrivateLayout(PartyRsrcID partyID, LayoutType newPrivateLayoutType);
	void                      CreateVideoHardwareInterface();
	void                      SendAddVideoOperationPointSet();
	void                      SendRemoveVideoOperationPointSet();
	BOOL                      IsRelaySupported();

	// Notify on media streams
	void                      NotifyOnAddedVideoInStreams(DWORD partyRscId);
	bool                      GetAPIVideoInNotifyParamsFromOperationPointList(int layerId, int& resolutionWidth, int& resolutionHeight, int& frameRate, int& maxBitRate);

	void                      AddImageToIntraDB(CImage* pImage);
	void                      RemoveImageFromIntraDB(DWORD partyRscId);

	// Svc Avc translation
	void                      UpdateNumberOfNonRelayImagesIfNeeded(CVideoBridgePartyCntl* pPartyCntl, bool imageAdded);
	void                      UpdateDBOnPartyMuteStateWhenThereIsNoInstaceInVB(char name[H243_NAME_LEN] ,EOnOff eOnOff,RequestPriority reqPrio);

	// State machine action functions
	void                      OnConfUpdateVideoMuteCONNECTED(CSegment* pParam);
	void                      OnConfVideoRefreshCONNECTED(CSegment* pParam);            // common assert + derived
	void                      OnConfAudioSpeakerChangedCONNECTED(CSegment* pParam);
	void                      OnConfSpeakersChangedCONNECTED(CSegment* pParam);
	void                      OnConfSetConfVideoLayoutSeeMeAllCONNECTED(CSegment* pParam);
	virtual void              OnConfSetConfVideoLayoutSeeMePartyCONNECTED(CSegment* pParam);
	void                      OnConfSetPrivateVideoLayoutCONNECTED(CSegment* pParam);
	void                      OnConfSetPrivateVideoLayoutOnOffCONNECTED(CSegment* pParam);
	void                      OnTimerLectureModeCONNECTED(CSegment* pParam);
	void                      OnTimerPresentationModeCONNECTED(CSegment* pParam);
	void                      OnConfUpdateAutoLayoutCONNECTED(CSegment* pParam);
	void                      OnConfUpdateVideoClarityCONNECTED(CSegment* pParam);
	void                      OnConfTurnOnOffTelePresence(CSegment* pParams);
	void                      OnPartyImageUpdated(CSegment* pParam);
	void                      OnConfSetAutoScanOrderCONNECTED(CSegment* pParam);
	void                      OnConfUpdateMessageOverlayCONNECTED(CSegment* pParam);
	void                      OnPartyUpdateMessageOverlayCONNECTED(CSegment* pParam);
	void                      OnConfUpdateRefreshLayoutCONNECTED(CSegment* pParam);
	void                      OnConfUpdateSiteNameInfoCONNECTED(CSegment* pParam);
	virtual void              OnConfContentBridgeStartPresentationCONNECTED(CSegment* pParam);
	virtual void              OnConfContentBridgeStopPresentationCONNECTED(CSegment* pParam);
	void                      OnTimerAfterStartStopContentCONNECTED(CSegment* pParam);
	void                      OnPartyNetworkQualityChangedCONNECTED(CSegment* pParam);
	void                      OnTimerNetworkQualityChangedCONNECTED(CSegment* pParam);
	void                      GetPartyVideoDataReq(CSegment* pParam);
	void                      Dump(VB_PARTY_VIDEO_LIST_S* pPartyList);
	virtual void              OnConfTerminateDISCONNECTING(CSegment* pParam);
	virtual void              OnEndPartyDisConnect(CSegment* pParam);
	virtual void              OnEndPartyExportCONNECTED(CSegment* pParam);
	// BRIDGE-8051
	virtual void              OnEndPartyIvrModeAfterResumeCall(CSegment* pParam);

	virtual void              OnAddVideoRelayImageToMixCONNECTED(CSegment* pParam);
	virtual void              OnAddVideoRelayImageToMixDISCONNECTING(CSegment* pParam);

	void                      OnRelayEpAskForIntraConnected(CSegment* pParam);
	void                      OnRelayAskEpsForIntraConnected(CSegment* pParam);

	virtual void              OnVideoRelayImageSvcAvcTranslateUpdateCONNECTED(CSegment* pParam);
	virtual void              OnVideoRelayImageSvcAvcTranslateUpdateDISCONNECTING(CSegment* pParam);

	virtual void              OnVideoNonRelayImageAvcSvcTranslateUpdateCONNECTED(CSegment* pParam);
	virtual void              OnVideoNonRelayImageAvcSvcTranslateUpdateDISCONNECTING(CSegment* pParam);

	virtual void              OnArtUpdateSsrcAckCONNECTED(CSegment* pParam);
	virtual void              UpgradeToMixAvcSvcForRelayParty(PartyRsrcID partyId);
	virtual void              UpgradeToMixAvcSvcForNonRelayParty(PartyRsrcID partyId, CAvcToSvcParams* pAvcToSvcParams);
	virtual void              DowngradeMixAvcSvcRelayParty(PartyRsrcID partyId);
	virtual void              DowngradeMixAvcSvcNonRelayParty(PartyRsrcID partyId);

	void                      ReplyUpgradeToMixAvcSvcUponError(PartyRsrcID partyRsrcID);
	void                      ReplyDowngradeMixAvcSvcUponError(PartyRsrcID partyRsrcID);

	// VNGR-26449 - unencrypted conference message
	void                      OnConfSendSecureMessageCONNECTED(CSegment* pParam);
	void                      OnConfSendSecureMessage(CSegment* pParam);
	void                      OnTimerSecureMessageCONNECTED(CSegment* pParam);
	void                      OnTimerMessageOverlayCONNECTED(CSegment* pParam);
	void                      StopSecureMessageTimer();
	void                      StopMessageOverlayTimer();
	void                      StartSecureMessageTimer(int numOfUnencrypted, CVideoBridgePartyCntl* pCurrParty);
	WORD                      calcMessageOverlayTime();
	eConfMediaType            GetConfMediaType();

	virtual void                  OnConfUpdateMuteAllVideoExceptLeaderCONNECTED(CSegment* pParam);
	virtual void                  OnConfUpdateMuteAllVideoExceptLeaderForJustIncomingCONNECTED(CSegment* pParam);
	virtual void                  OnSetPartyAsLeaderCONNECTED(CSegment* pParam);
	virtual void                  OnConfUpdateMuteAllVideoExceptLeaderDISCONNECTING(CSegment* pParam);
	virtual void                  OnConfUpdateMuteAllVideoExceptLeaderForJustIncomingDISCONNECTING(CSegment* pParam);
	// //Change Layout Improvement - Conf Layout
	BOOL                      BuildConfLayout();
	BOOL                      IsPartyLayoutIdenticalToConfLayoutAfterBuild(CVideoBridgePartyCntl* pCurrParty);
	void                      InitBridgeParamsForMuteAllVideoButLeader(const CBridgePartyInitParams* pBridgePartyInitParams);
	void                      InitBridgeInParamsForMuteAllVideoButLeaderForNonRelay(CBridgePartyVideoInParams* pBridgePartyVideoInParams);
	void                      InitBridgeInParamsForMuteAllVideoButLeaderForRelay(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayMediaParams);
	bool                      IsNeedToMutePartyVideoInBridgeLevel(const char*  partyName);



  	bool                      IsPartyLeader(const char*  name);
	void                      PrepareParamsForMixConf();
	void                      FindAllowedMixSvcToAvcLayerId();

	void                      SetIsConfInActiveContentPresentation(BYTE yesNo) { m_isConfInActiveContentPresentation = yesNo; }

	CImage*					  GetPartyImageByVideoMSI(DWORD newVideoMSI,DWORD& partyRsrcId);

	void 						OnConfSetTelepresenceLayoutModeConnected(CSegment* pParam);// TELEPRESENCE_LAYOUTS

//	BYTE                      IsConfInActiveContentPresentation()              { return m_isConfInActiveContentPresentation; }

	CVisualEffectsParams*     m_pVisualEffects;                 // borders etc
	BYTE                      m_isVideoClarityEnabled;
	CVideoHardwareInterface*  m_pVideoHardwareInterface;        // for video relay flow

	CRelayIntraDB             m_intraDB;
	DWORD                     m_NumberOfNonRelayImages;
	size_t                    m_indicationsUpdateCycle;         // cycle #: we perform the full update once per several partial updates
	BYTE                      m_isConfInActiveContentPresentation;
	int                       m_maxAllowedLayerIdForMixSvcToAvc;
	int                       m_maxAllowedLayerIdForMixSvcToAvcHighRes;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeCPContent
////////////////////////////////////////////////////////////////////////////
class CVideoBridgeCPContent : public CVideoBridgeCP
{
	CLASS_TYPE_1(CVideoBridgeCPContent, CVideoBridgeCP)

public:
	CVideoBridgeCPContent();
	virtual ~CVideoBridgeCPContent();
	virtual void                  Create(const CVideoBridgeInitParams* pVideoBridgeInitParams);
	virtual const char*           NameOf() const {return "CVideoBridgeCPContent";}
	virtual void                  CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl, CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
	virtual void                  NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl);

	CVideoBridgePartyCntlContent* GetContentDecoder();
	CImage*                       GetContentImage();
	BOOL                          IsContentImageNeedToBeAdded();
	void                          ContentDecoderConditionChanged(EConditionForContentDecoder conditionType);
	void                          ActionOnConditionChange(BYTE YesNoCondition);
	void                          SetIsLegacyPartyConnected(BYTE yesNo);
	BYTE                          IsLegacyPartyConnected();
	void 						  UpdateLayoutHandlerTypeIfNeeded();
	BYTE                          IsLastLegacyPartyLeftConf();
	DWORD                         GetContentRate()                   { return m_ContentRate; }
	void                          SetContentRate(DWORD rate)         { m_ContentRate = rate; }
	DWORD                         GetContentProtocol()               { return m_ContentProtocol; }
	void                          SetContentProtocol(DWORD protocol) { m_ContentProtocol = protocol; }
	void                          AskContentSpeakerForIntra();
	void                          AddContentDecoderToConfMixAfterVideoInSynced();
	void                          RemoveContentDecoderFromConfMixBeforeContentStoped();
	void                          ResetDecoderFailureStatusInConf();

	virtual LayoutType            GetLegacyContentDefaultLayout();

protected:
	virtual void                  OnEndPartyDisConnect(CSegment* pParam);
	virtual void                  OnEndPartyExportCONNECTED(CSegment* pParam);
	virtual void                  OnEndPartyConnectCONNECTED(CSegment* pParam);
	virtual void                  OnConfDisConnectPartyCONNECTED(CSegment* pParam);
	virtual void                  OnConfDisConnectPartyDISCONNECTING(CSegment* pParam);
	void                          OnConfContentBridgeStartPresentationCONNECTED(CSegment* pParam);
	void                          OnConfContentBridgeStopPresentationCONNECTED(CSegment* pParam);
	virtual void                  OnConfTerminateDISCONNECTING(CSegment* pParam);
	void                          OnEndContentDecoderPartyConnectCONNECTED(CSegment* pParam);
	void                          OnEndDestroyContentDecoderCONNECTED(CSegment* pParam);
	void                          OnEndDestroyContentDecoderDISCONNECTING(CSegment* pParam);
	void                          OnEndDisconnectContentDecoderCONNECTED(CSegment* pParam);
	virtual void                  OnContentDecoderVideoInSyncedCONNECTED(CSegment* pParam);
	virtual void                  OnContentDecoderVideoInSyncedDISCONNECTING(CSegment* pParam);
	virtual void                  OnContentDecoderSyncLostCONNECTED(CSegment* pParam);
	virtual void                  OnContentDecoderAllocateRsrcFailureCONNECTED(CSegment* pParam);
	virtual void                  OnContentDecoderResetAllocateRsrcFailureCONNECTED(CSegment* pParam);
	virtual void                  OnConfSetConfVideoLayoutSeeMePartyCONNECTED(CSegment* pParam);
	void                          OnConfSetPrivateVideoLayoutOnOffCONNECTED(CSegment* pParam);

	BYTE                          m_isLegacyPartyConnected;
	BYTE                          m_isContentHD1080Supported;
	DWORD                         m_ContentRate;
	DWORD                         m_ContentProtocol;
	CVideoBridgePartyCntlContent* m_pContentDecoder;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CVideoContentXcodeBridge
////////////////////////////////////////////////////////////////////////////
class CVideoContentXcodeBridge : public CVideoBridgeCP
{
	CLASS_TYPE_1(CVideoContentXcodeBridge, CVideoBridgeCP)

public:
	CVideoContentXcodeBridge();
	virtual ~CVideoContentXcodeBridge();
	virtual void                  Create(const CVideoBridgeInitParams* pVideoBridgeInitParams);
	virtual const char*           NameOf() const {return "CVideoContentXcodeBridge";}
	void                          CreateEncoders(const CVideoBridgeInitParams* pVideoBridgeInitParams);
	void                          CreateDecoder(const CVideoBridgeInitParams* pVideoBridgeInitParams);
	void                          ConnectContentEncoders();
	virtual void                  InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams);
	void                          CloseXCodeEncoders();
	virtual void                  CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl, CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
	virtual void                  NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl);

	CVideoBridgePartyCntlContent* GetContentDecoder();
	CImage*                       GetContentImage();
	void                          BulidLayout();
	BOOL                          IsContentImageNeedToBeAdded();
	void                          SendChangeLayoutToAllLevelEncoders();
	void                          SetEmptyLayoutForAllLevelEncoders();
	void                          EndDisconnect();
	void                          AskContentSpeakerForIntra();
	void                          UpdateVideoOutParams(eXcodeRsrcType eXCodeEncoderType, CBridgePartyVideoOutParams* pBridgePartyVideoOutParams);

	virtual LayoutType            GetLegacyContentDefaultLayout();
	eXcodeRsrcType                GetXCodeEncoderIndex(CVideoBridgePartyCntlXCode* pPartyCntl) const;

protected:
	void                          UpdateEncoderResourceParams(CBridgePartyVideoOutParams& rOutVideoParams, PartyRsrcID& partyRsrcID, DWORD encoder_index, eVideoConfType videoConfType, ContentXcodeRsrcDesc encoderXCodeRsrcDesc) const;
	void                          UpdateDecoderResourceParams(CBridgePartyVideoInParams& rInVideoParams, PartyRsrcID& partyRsrcID, eVideoConfType videoConfType, ContentXcodeRsrcDesc decoderXCodeRsrcDesc) const;
	WORD                          CheckAllXCodeEncodersConnected() const;
	virtual void                  OnXCodeEncoderEndConnectIDLE(CSegment* pParam);
	virtual void                  OnXCodeEncoderEndConnectCONNECTED(CSegment* pParam);
	virtual void                  OnXCodeEncoderEndConnectDISCONNECTING(CSegment* pParam);
	virtual void                  OnEndPartyDisConnect(CSegment* pParam);
	virtual void                  OnEndPartyConnectCONNECTED(CSegment* pParam);
	virtual void                  OnTimerConnectXCodeBridgeIDLE(CSegment* pParam);
	virtual void                  OnXCodeEncoderVideoOutUpdated(CSegment* pParam);
	virtual void                  OnTimerDisconnetDISCONNECTING(CSegment* pParam);
	virtual void                  OnConfConnectPartyCONNECTED(CSegment* pParam);
	virtual void                  OnConfDisConnectPartyCONNECTED(CSegment* pParam);
	virtual void                  OnConfDisConnectPartyDISCONNECTING(CSegment* pParam);
	void                          OnConfContentBridgeStartPresentationCONNECTED(CSegment* pParam);
	virtual void                  OnConfTerminateDISCONNECTING(CSegment* pParam);
	virtual void                  OnConfDisConnectConfIDLE(CSegment* pParam);
	void                          OnEndContentDecoderPartyConnectCONNECTED(CSegment* pParam);
	virtual void                  OnXCodeEncoderEndDisconnectDISCONNECTING(CSegment* pParam);
	virtual void                  OnContentBridgeContentVideoRefreshCONNECTED(CSegment* pParam);
	virtual void                  OnContentDecoderVideoInSyncedCONNECTED(CSegment* pParam);
	virtual void                  OnContentDecoderDisconnectedCONNECTED(CSegment* pParam);
	void                          OnEndDestroyContentDecoderCONNECTED(CSegment* pParam);
	void                          OnEndDestroyContentDecoderDISCONNECTING(CSegment* pParam);
	void                          BuildLayout();

	void                          DestroyXCodeEncoder(WORD xcodeEncoderIndex);
	void                          CreateAndSendDeallocateContentEncoders();
	STATUS                        SendReqToResourceAllocator(CSegment* seg, OPCODE opcode, BOOL isSync = TRUE);

	XCODE_ENCODERS_MAP*           m_xcodeEncodesrMap;
	XCODE_RESOURCES_MAP*          m_pXCodeRsrcMap;
	CVideoBridgePartyCntlContent* m_pContentDecoder;
	DWORD                         m_ContentRate;
	eVideoConfType                m_VideoConfType;
	CLayout*                      m_ConfContentLayout1x1;
	WORD                          m_resourcesStatus;
	WORD                          m_closeAfterOpenEncodesFailed;
  WORD                            m_isFirstContentSessionStarted;

	PDECLAR_MESSAGE_MAP
};


#endif // _CVideoBridge_H_
