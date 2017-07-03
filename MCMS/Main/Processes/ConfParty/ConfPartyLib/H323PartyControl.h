//+========================================================================+
//                            H323PartyControl.H                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323PartyControl.H                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sami                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 7/4/95     |                                                      |
//+========================================================================+
#ifndef _H323PartyControl
#define _H323PartyControl


#include "IpPartyControl.h"
#include "RvCommonDefs.h"
#include "IpRtpFeccRoleToken.h"
#include "IpCsContentRoleToken.h"
#include "StateMachine.h"
#include "ConfPartyDefines.h"
#include "ConfAppMngrInterface.h"
#include "H323NetSetup.h"
#include "LPRData.h"


class  CCapH323;
class  CComModeH323;
class  CRoomCntl;

typedef enum {
	eConnect,
		eChangeMode_Must,
		eChangeCpMode_FromCaps,
		eChangeMode_BestEffort,
		eSecondary,
		eDisconnectAll
} ePartyMediaState;

#define RATE_STATUS_INITIAL 0
#define RATE_STATUS_NO_CHANGE 1
#define RATE_STATUS_SECONDERY 2
#define RATE_STATUS_FLOW_CONTROL_OUT_CHANNEL 3
#define RATE_STATUS_FLOW_CONTROL_IN_CHANNEL 4
#define RATE_STATUS_FLOW_CONTROL_OUT_CHANNEL_RESET 5
#define RATE_STATUS_FLOW_CONTROL_IN_CHANNEL_RESET 6


#define MASTER 1
#define SLAVE 2
#define COP_NO_VIDEO_UPDATES_TIME 3*SECOND

class CH323PartyCntl : public CIpPartyCntl
{ //abstract class
CLASS_TYPE_1(CH323PartyCntl,CIpPartyCntl )
public:
	// Constructors
	CH323PartyCntl();
	virtual ~CH323PartyCntl();
	virtual const char* NameOf() const { return "CH323PartyCntl";}

	// Initializations:
	CH323PartyCntl& operator=(const CH323PartyCntl& other);

	virtual DWORD  GetRemoteCapsVideoRate(CapEnum protocol) const;

    // Operations:
	virtual void   SetFlowControlParty(BYTE onOff) {m_bIsFlowControlParty = onOff;}
	virtual BYTE   IsFlowControlParty() const {return m_bIsFlowControlParty;}
	virtual void   SetFlowControlRate(DWORD rate) {m_flowControlRate = rate;}
	virtual DWORD  GetFlowControlRate() const {return m_flowControlRate;}

	virtual void  SetH323Alias(const char* name) {}//we can set the alias only in case of CAdd
	virtual void  SetH323AliasType(const WORD AliasType) {} //we can set the alias only in case of CAdd
	virtual BYTE  IsRemoteCapNotHaveVideo() const;
	virtual BYTE  IsRemoteAndLocalHasHDContent1080() const;
	virtual BYTE  IsRemoteAndLocalHasHDContent720() const;
	virtual BYTE  IsRemoteAndLocalHasHighProfileContent() const;

	virtual BYTE  IsVidModeIncludedInCaps() const;
	virtual BYTE  IsPartyEncrypted() const;
	virtual BOOL  IsDisconnectionBecauseOfNetworkProblems() const;
	virtual	BOOL  IsRedialImmediately() {return FALSE;}
  	void SetDataForImportPartyCntl(CPartyCntl *apOtherPartyCntl);
  	virtual BYTE IsRemoteAndLocalCapSetHasContent(eToPrint toPrint = eToPrintNo)const {return NO;};
  	virtual BYTE IsLegacyContentParty();
  	virtual DWORD GetPossibleContentRate() const;
  	virtual CapEnum GetContentProtocol(BYTE bTakeCurrent) const;
  	virtual BYTE IsOpenContentTxChannel(BYTE bTakeCurrent) const;
  	DWORD GetMinConfPartyRate() const;
  	DWORD GetMinContentPartyRate(DWORD currContentRate=0);
  	virtual BYTE AreLocalCapsSupportMedia(cmCapDataType eDataType);
  	virtual BYTE IsPartyConnectAllWhileMove() const { return m_bIsPartyConnectAllWhileMove;}
  	virtual BYTE IsAdditionalRsrcActivated() const { return m_bAdditionalRsrcActivated;}
  	virtual void SetAddPartyStateBeforeMove(WORD state) {m_AddPartyStateBeforeMove = state;}
  	virtual void ChangeScm(CIpComMode* pH323Scm, EChangeMediaType eChangeMediaType) {PASSERT_AND_RETURN(YES);}
	virtual CCapH323*  GetH323RemoteCap()   { return m_pRmtCapH323; }
	virtual ECascadePartyType GetPartyCascadeTypeAndVendor();
	virtual BYTE IsPartyCapsContainsH264SCM(const CMediaModeH323* H323ContentMode,ERoleLabel role);
	virtual BOOL IsNeedToChangeResAccordingToRemoteRevision();
	virtual void PartySendMuteVideo(BYTE isActive);

	//Multiple links for ITP in cascaded conference feature:
	virtual void             OnAddSubLinkToRoomControl(const char* name, DWORD indexOfLink, eTypeOfLinkParty type,DWORD mailbox);
	virtual void             OnRemoveSubLinkFromRoomControl(const char* name, DWORD indexOfLink);
	virtual BOOL             IsSubPartyConnected(const char* name, DWORD indexOfLink);
	virtual eTypeOfLinkParty GetPartyLinkType () {return m_linkType;}

	// mix mode
	virtual void UpdateLocalCapsForHdVswInMixMode(const VideoOperationPoint *pVideoOperationPoint);

protected:

	// Initializations:
    virtual void Destroy();

  	// Operations:

	virtual char* GetPartyAndClassDetails();
             //disconnect
	virtual void  OnPartyChannelDisconnect(CSegment* pParam);
	virtual void  DisconnectChannel(cmCapDataType dataType, cmCapDirection direction, ERoleLabel role);
	virtual void  OnVidBrdgDisconnect(CSegment* pParam);
	virtual void  OnAudBrdgDisconnect(CSegment* pParam);

			//connect
	virtual void  OnPartyH323Connect(CSegment* pParam);
	virtual void  OnUpdatePartyH323VideoBitRate(CSegment* pParam);
	virtual void  OnCleanVideoRateLimitation();
	virtual void  UpdateBridgeFlowControlRateIfNeeded(CLPRParams* lprParams = NULL);
	virtual void  PartyH323ConnectAllPartyConnectAudioOrChangeAll(CSegment* pParam);
	virtual void  AdditionalRsrcHandling(WORD status, CIpComMode* pScm = NULL);
    virtual void  EndConnectionProcess(WORD status);

   virtual EMediaDirection  HandleAudioBridgeConnectedInd(CSegment* pParam);

	//virtual void  ConnectAudio(CComModeH323* pNewH323Mode);
	virtual void  PartyConnectedAudio(CSegment* pParam, WORD status);
			// FECC:
	BYTE IsFECC();
			//Content
	virtual void  ConnectPartyToContentBridge();

	virtual WORD  SetH323VideoRate(DWORD videoRate, BYTE bIsSpecialCaseForChangeRate = FALSE,BYTE bIsDisconnectParty=TRUE);
	virtual void  ChangeScmAccordingToH323Scm(CIpComMode* pH323Scm);
	BYTE CreateNewModeForCopCascadeLecturerLink(CIpComMode* pH323Scm);
	void CopVideoBridgeChangeLinkLectureModeOut(CIpComMode* pNewScm);
	void SaveLecturerLinkCopLevelAccordingToCurrent();
	virtual CIpComMode*	GetScmForVideoBridgeConnection(cmCapDirection direction);

			//Operations for Content:
	virtual int  OnContentBrdgConnected(CSegment* pParam);
	virtual int  OnContentBrdgDisconnected(CSegment* pParam);
	virtual void OnPartyPresentationOutStreamUpdateAnycase(CSegment* pSeg);

			//highest common
	virtual void  ChangeMode(eChangeModeState changeModeState, cmCapDataType dataType, cmCapDirection direction, ERoleLabel role = kRolePeople,
			ePartyMediaState mediaState = eChangeMode_Must, CRsrcParams** avcToSvcTranslatorRsrcParams = NULL,CRsrcParams* pMrmpRsrcParams = NULL);
	virtual void  PartyReceivedReCapsChangeAll(CSegment* pParam);
	virtual void  OnPartyReceivedReCapsAnycase(CSegment* pParam);
	virtual void  PartyReceivedReCaps(CSegment* pParam);
	virtual	void  HandleRemoteReCap();
	virtual void  SetPartyToSecondaryAndStopChangeMode(BYTE reason,DWORD details = 0,BYTE direction = cmCapTransmit,CSecondaryParams *pSecParams=NULL,BYTE bDisconnectChannels = TRUE);
	virtual void  ImplementUpdateSecondaryInPartyControlLevel();
	virtual void  ImplementSecondaryInPartyLevel();
    virtual BYTE  IsConfHasSameLineRateAsSourceConf();
	//audio
	virtual void  OnPartyMuteAudioAnycase(CSegment* pParam);

	virtual void PartyMuteAudio(CSegment* pParam);

	//ECS
	virtual void  OnPartyReceivedECS(CSegment* pParam);
	virtual void PartyReceivedECS();

    virtual void  OnPartyUpdateLocalCaps(CSegment* pParam);

    //Multiple links for ITP in cascaded conference feature:
    virtual void  OnAddSubLinksParties(CSegment* pParam);
    void          OnConnectLinkTout(CSegment* pParam);
    void          DisconnectMainLink(); //error handling
    void          OnMainPartyUpdateITPSpeaker(CSegment* pParam);
    void          OnMainPartyUpdateITPSpeakerAck(CSegment* pParam);
    void          OnMainPartySendNewITPSpeaker(CSegment* pParam);


	// Faulty MFA
	virtual void OnPartyReceivedFaultyRsrc(CSegment* pSeg);

			//DATA brdg
//	virtual void OnPartyConnectDataBrdg();

	virtual BYTE IsCapableOfVideo();
	virtual BYTE IsCapableOfHD720();
	virtual BYTE IsCapableOfHD1080();
	virtual BYTE IsCapableOf4CIF();
	virtual eVideoPartyType GetCPVideoPartyTypeAccordingToCapabilities();

	virtual DWORD GetConfRate() const;
	virtual DWORD GetSetupRate();
	virtual void UpdateH264ModeInLocalCaps(H264VideoModeDetails h264VidModeDetails,ERoleLabel eRole = kRolePeople);
	virtual APIS32 GetLocalCapsMbps(ERoleLabel eRole);
    virtual void Disable4CifInLocalCaps();
    virtual void RemoveH263H261FromLocalCaps();
    virtual DWORD GetMaxFsAccordingtoProfile(APIU16 profile);
    virtual void GetRemoteCapsParams( WORD& maxMBPS, WORD& maxFS, WORD& maxDPB, WORD& maxBRandCPB, WORD& maxSAR, WORD& maxStaticMB, ERoleLabel eRole,DWORD profile = 0);

	virtual void SetPartyToAudioOnly();
	virtual BYTE IsPartyMasterOrSlaveNotLecturer();
	//virtual ECascadePartyType GetPartyCascadeType();
	virtual BYTE IsPartyCascadeWithCopMcu() const;
	virtual BYTE  IsPartyCascade() const;
	virtual BYTE GetPartyCascadeType() const { return m_bIsCascade; }

	//New Resource handling
    void  OnRsrcReAllocatePartyRspReAllocate(CSegment* pParam);
	void  OnRsrcReAllocatePartyRspChangeAll(CSegment* pParam);
    virtual void   CreateAndSendReAllocatePartyResources(eNetworkPartyType networkPartyType,eVideoPartyType videoPartyType, EAllocationPolicy allocationPolicy,WORD reAllocRtm=FALSE,WORD isSyncMessage=0,BYTE IsEnableSipICE=FALSE,DWORD artCapacity = 0);
    virtual BYTE ChangeVideoModeIfNeeded() {DBGPASSERT (1); return FALSE;};
    virtual void  ChangeOther() {DBGPASSERT (1);}
	// LPR
	virtual void OnUpdatePartyH323LprVideoBitRate(CSegment* pParam);
	virtual void OnLprVideoOutBrdgBitRateUpdated(CSegment* pParam);
	// VNGFE-8204
	void OnChangeContentBitRateByLprOrDba(CSegment* pParam);

    void OnPartyCntlFirRequestByEP(CSegment* pParam);

    // Attributes
	CCapH323*      m_pLocalCapH323;
	CCapH323*      m_pRmtCapH323;

    BYTE		   m_bIsFlowControlParty;
    DWORD		   m_flowControlRate;   //different than 0 only if m_isFlowControlParty is YES
    DWORD		   m_DbaContentRate;

			//Attributes for move:
	BYTE		   m_bIsCascade;
//	WORD		   m_lsdRequestRate;
	BYTE		   m_bConfWaitToEndChangeModeForFecc;
	BYTE            m_bPartyEndChangeVideoInformParty;

	WORD			m_AddPartyStateBeforeMove;
	BYTE			m_bIsPartyConnectAllWhileMove;
	BYTE			m_bAdditionalRsrcActivated;
	BYTE            m_bLateReleaseOfResources;
    BYTE            m_bContinueChangeModeAfterReAlloc;

	int            m_masterSlaveStatus;
	CH323NetSetup* m_pH323NetSetup;
	BYTE           m_bSuspendVideoUpdates;
	BYTE           m_bPendingRemoteCaps;

	//Multiple links for ITP in cascaded conference feature:
	CRoomCntl*              m_roomControl;
	eTypeOfLinkParty        m_linkType;

	PDECLAR_MESSAGE_MAP
};


#endif //H323PartyControl
