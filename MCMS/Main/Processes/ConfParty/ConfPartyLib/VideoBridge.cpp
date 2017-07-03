//                            VideoBridge.CPP                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       VideoBridge.CPP                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Talya                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                   |
//-------------------------------------------------------------------------|
//+========================================================================+

#include <memory>
#include "VideoBridge.h"
#include "VideoBridgeInitParams.h"
#include "TextOnScreenMngr.h"
#include "TaskApi.h"
#include "BridgePartyInitParams.h"
#include "VideoBridgePartyCntl.h"
#include "BridgePartyDisconnectParams.h"
#include "BridgePartyExportParams.h"
#include "BridgePartyVideoParams.h"
#include "TraceStream.h"
#include "ConfPartyGlobals.h"
#include "VideoBridgePartyInitParams.h"
#include "VideoApiDefinitions.h"
#include "BridgePartyVideoIn.h"
#include "BridgePartyVideoOut.h"
#include "VideoBridgeAutoScanParams.h"
#include "Gathering.h"
#include "VideoApiDefinitionsStrings.h"
#include "SiteNameInfo.h"
#include "BridgePartyVideoUniDirection.h"
#include "VideoRelayBridgePartyCntl.h"
#include "RelayIntraData.h"
#include "StlUtils.h"
#include "VideoRelayInOutMediaStream.h"
#include "AllocateStructs.h"
#include "MplMcmsProtocol.h"
#include "OpcodesMcmsInternal.h"
#include "SysConfigKeys.h"
#include "ConfigHelper.h"
#include "Party.h"
#include "PrettyTable.h"
#include "TelepresenseEPInfo.h"
#include "EnumsToStrings.h"

extern const char* TelePresencePartyTypeToString(eTelePresencePartyType telePresencePartyType);

// ------------------------- global function ------------------------------------------------
LayoutType GetNewLayoutType(const BYTE oldLayoutType);
extern char* CascadeModeToString(BYTE cascadeMode);

/////////////////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CVideoBridge)

  ONEVENT(DELETED_PARTY_FROM_CONF,      IDLE_VB,        CVideoBridge::NullActionFunction)
  ONEVENT(DELETED_PARTY_FROM_CONF,      CONNECTED,      CVideoBridge::OnConfDeletePartyFromConfCONNECTED)
  ONEVENT(DELETED_PARTY_FROM_CONF,      DISCONNECTING,  CVideoBridge::NullActionFunction)
  ONEVENT(DELETED_PARTY_FROM_CONF,      IDLE,           CVideoBridge::NullActionFunction)

  ONEVENT(VIDEOMUTE,                    DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(DISCONNECTCONF,               CONNECTED,      CVideoBridge::OnConfDisConnectConfCONNECTED)
  ONEVENT(DISCONNECTCONF,               DISCONNECTING,  CVideoBridge::OnConfDisConnectConfDISCONNECTING)

  ONEVENT(TERMINATE,                    CONNECTED,      CVideoBridge::OnConfTerminateCONNECTED)
  ONEVENT(TERMINATE,                    DISCONNECTING,  CVideoBridge::OnConfTerminateDISCONNECTING)

  ONEVENT(CONNECTPARTY,                 CONNECTED,      CVideoBridge::OnConfConnectPartyCONNECTED)
  ONEVENT(CONNECTPARTY,                 DISCONNECTING,  CVideoBridge::OnConfConnectPartyDISCONNECTING)

  ONEVENT(DISCONNECTPARTY,              CONNECTED,      CVideoBridge::OnConfDisConnectPartyCONNECTED)
  ONEVENT(DISCONNECTPARTY,              DISCONNECTING,  CVideoBridge::OnConfDisConnectPartyDISCONNECTING)

  ONEVENT(EXPORTPARTY,                  CONNECTED,      CVideoBridge::OnConfExportPartyCONNECTED)

  ONEVENT(ENDEXPORTPARTY,               CONNECTED,      CVideoBridge::OnEndPartyExportCONNECTED)

  ONEVENT(VIDEO_IN_SYNCED,              CONNECTED,      CVideoBridge::OnVideoInSyncedCONNECTED)
  ONEVENT(VIDEO_IN_SYNCED,              DISCONNECTING,  CVideoBridge::OnVideoInSyncedDISCONNECTING)

  ONEVENT(ENDCONNECTPARTY,              CONNECTED,      CVideoBridge::OnEndPartyConnectCONNECTED)
  ONEVENT(ENDCONNECTPARTY,              DISCONNECTING,  CVideoBridge::OnEndPartyConnectDISCONNECTING)

  ONEVENT(ENDCONNECTPARTY_IVR_MODE,     CONNECTED,      CVideoBridge::OnEndPartyConnectIVRModeCONNECTED)
  ONEVENT(ENDCONNECTPARTY_IVR_MODE,     DISCONNECTING,  CVideoBridge::OnEndPartyConnectIVRModeDISCONNECTING)

  ONEVENT(ENDDISCONNECTPARTY,           CONNECTED,      CVideoBridge::OnEndPartyDisConnect)
  ONEVENT(ENDDISCONNECTPARTY,           DISCONNECTING,  CVideoBridge::OnEndPartyDisConnect)

  ONEVENT(PARTY_VIDEO_IN_UPDATED,       CONNECTED,      CVideoBridge::OnEndPartyUpdateVideoInCONNECTED)
  ONEVENT(PARTY_VIDEO_IN_UPDATED,       DISCONNECTING,  CVideoBridge::OnEndPartyUpdateVideoInDISCONNECTING)

  ONEVENT(PARTY_IMAGE_UPDATED,          ANYCASE,        CVideoBridge::NullActionFunction) //should be implemented in derived class

  ONEVENT(PARTY_VIDEO_OUT_UPDATED,      CONNECTED,      CVideoBridge::OnEndPartyUpdateVideoOutCONNECTED)
  ONEVENT(PARTY_VIDEO_OUT_UPDATED,      DISCONNECTING,  CVideoBridge::OnEndPartyUpdateVideoOutDISCONNECTING)

  ONEVENT(VIDREFRESH,                   DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(SPEAKERS_CHANGED,             DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(IMAGE_IN_CELL_ZERO_CHANGED,   CONNECTED,      CVideoBridge::OnPartyImageInCellZeroChangedCONNECTED)
  ONEVENT(IMAGE_IN_CELL_ZERO_CHANGED,   INSWITCH,       CVideoBridge::OnPartyImageInCellZeroChangedCONNECTED)
  ONEVENT(IMAGE_IN_CELL_ZERO_CHANGED,   DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(SETCONFVIDLAYOUT_SEEMEALL,    DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(SETCONFVIDLAYOUT_SEEMEPARTY,  DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(SETPRIVATEVIDLAYOUT,          DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(SETPRIVATEVIDLAYOUTONOFF,     DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(UPDATELECTUREMODE,            CONNECTED,      CVideoBridge::OnConfSetLectureModeCONNECTED)
  ONEVENT(UPDATELECTUREMODE,            DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(UPDATEAUTOLAYOUT,             DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(LECTURE_MODE_TOUT,            DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(PRESENTATION_MODE_TOUT,       DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(SET_SITE_AND_VISUAL_NAME,     CONNECTED,      CVideoBridge::OnSetSiteNameCONNECTED)
  ONEVENT(SET_SITE_AND_VISUAL_NAME,     DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(FREEZPIC,                     ANYCASE,        CVideoBridge::OnFreezePic)

  ONEVENT(UPDATE_VIDEO_CLARITY,         DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(SET_AUTOSCAN_INTERVAL,        ANYCASE,        CVideoBridge::OnConfUpdateAutoScanIntervalCONNECTED)
  ONEVENT(SET_AUTOSCAN_INTERVAL,        DISCONNECTING,  CVideoBridge::NullActionFunction)

  ONEVENT(SET_MESSAGE_OVERLAY,          ANYCASE,        CVideoBridge::NullActionFunction)

  ONEVENT(INDICATION_ICONS_CHANGE,		 CONNECTED ,	CVideoBridge::OnDisplayIndicationIconCONNECTED)
  ONEVENT(INDICATION_ICONS_CHANGE,  DISCONNECTING , CVideoBridge::NullActionFunction)


  ONEVENT(NEW_ITP_SPEAKER_TIMER,         ANYCASE,        CVideoBridge::NullActionFunction)
  //ONEVENT(TELEPRESENCE_PARTY_CONNECTION_TIMER,         ANYCASE,        CVideoBridge::NullActionFunction)

  ONEVENT(NOTIFY_VB_ON_NETWORK_QUALITY_CHANGE,ANYCASE,   CVideoBridge::NullActionFunction)

  ONEVENT(GET_PARTY_VIDEO_DATA_REQ,           ANYCASE,   CVideoBridge::NullActionFunction)
  ONEVENT(UPDATE_MUTE_ALL_VIDEO_EXCEPT_LEADER, ANYCASE,  CVideoBridge::NullActionFunction)
  ONEVENT(SET_PARTY_AS_LEADER_FOR_VB,          ANYCASE,  CVideoBridge::NullActionFunction)

PEND_MESSAGE_MAP(CVideoBridge, CBridge);
/////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------- global function ------------------------------------------------
LayoutType GetNewLayoutType(const BYTE oldLayoutType)
{
	LayoutType result = CP_NO_LAYOUT;

  switch (oldLayoutType)
  {
    case ONE_ONE                   : result = CP_LAYOUT_1X1                 ; break;
    case ONE_TWO                   : result = CP_LAYOUT_1X2                 ; break;
    case TWO_ONE                   : result = CP_LAYOUT_2X1                 ; break;
    case TWO_TWO                   : result = CP_LAYOUT_2X2                 ; break;
    case THREE_THREE               : result = CP_LAYOUT_3X3                 ; break;
    case ONE_AND_FIVE              : result = CP_LAYOUT_1P5                 ; break;
    case ONE_SEVEN                 : result = CP_LAYOUT_1P7                 ; break;
    case ONE_TWO_VER               : result = CP_LAYOUT_1x2VER              ; break;
    case ONE_TWO_HOR               : result = CP_LAYOUT_1x2HOR              ; break;
    case ONE_PLUS_TWO_HOR          : result = CP_LAYOUT_1P2HOR              ; break;
    case ONE_PLUS_TWO_VER          : result = CP_LAYOUT_1P2VER              ; break;
    case ONE_PLUS_THREE_HOR        : result = CP_LAYOUT_1P3HOR              ; break;
    case ONE_PLUS_THREE_VER        : result = CP_LAYOUT_1P3VER              ; break;
    case ONE_PLUS_FOUR_VER         : result = CP_LAYOUT_1P4VER;             ; break;
    case ONE_PLUS_FOUR_HOR         : result = CP_LAYOUT_1P4HOR              ; break;
    case ONE_PLUS_EIGHT_CENTRAL    : result = CP_LAYOUT_1P8CENT             ; break;
    case ONE_PLUS_EIGHT_UPPER      : result = CP_LAYOUT_1P8UP               ; break;
    case ONE_PLUS_TWO_HOR_UPPER    : result = CP_LAYOUT_1P2HOR_UP           ; break;
    case ONE_PLUS_THREE_HOR_UPPER  : result = CP_LAYOUT_1P3HOR_UP           ; break;
    case ONE_PLUS_FOUR_HOR_UPPER   : result = CP_LAYOUT_1P4HOR_UP           ; break;
    case ONE_PLUS_EIGTH            : result = CP_LAYOUT_1P8HOR_UP           ; break;
    case FOUR_FOUR                 : result = CP_LAYOUT_4X4                 ; break;
    case TWO_PLUS_EIGHT            : result = CP_LAYOUT_2P8                 ; break;
    case ONE_PLUS_TWELVE           : result = CP_LAYOUT_1P12                ; break;
    case ONE_ONE_QCIF              : result = CP_LAYOUT_1X1                 ; break;
    case ONE_TWO_FLEX              : result = CP_LAYOUT_1X2_FLEX            ; break;
    case ONE_PLUS_TWO_HOR_R_FLEX   : result = CP_LAYOUT_1P2HOR_RIGHT_FLEX   ; break;
    case ONE_PLUS_TWO_HOR_L_FLEX   : result = CP_LAYOUT_1P2HOR_LEFT_FLEX    ; break;
    case ONE_PLUS_TWO_HOR_UP_R_FLEX: result = CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX; break;
    case ONE_PLUS_TWO_HOR_UP_L_FLEX: result = CP_LAYOUT_1P2HOR_UP_LEFT_FLEX ; break;
    case TWO_TWO_UP_R_FLEX         : result = CP_LAYOUT_2X2_UP_RIGHT_FLEX   ; break;
    case TWO_TWO_UP_L_FLEX         : result = CP_LAYOUT_2X2_UP_LEFT_FLEX    ; break;
    case TWO_TWO_DOWN_R_FLEX       : result = CP_LAYOUT_2X2_DOWN_RIGHT_FLEX ; break;
    case TWO_TWO_DOWN_L_FLEX       : result = CP_LAYOUT_2X2_DOWN_LEFT_FLEX  ; break;
    case TWO_TWO_R_FLEX            : result = CP_LAYOUT_2X2_RIGHT_FLEX      ; break;
    case TWO_TWO_L_FLEX            : result = CP_LAYOUT_2X2_LEFT_FLEX       ; break;
    case ONE_PLUS_ONE_OVERLAY      : result = CP_LAYOUT_OVERLAY_1P1         ; break;
    case ONE_PLUS_TWO_OVERLAY      : result = CP_LAYOUT_OVERLAY_1P2         ; break;
    case ONE_PLUS_THREE_OVERLAY    : result = CP_LAYOUT_OVERLAY_1P3         ; break;
    case ONE_PLUS_TWO_OVERLAY_ITP  : result = CP_LAYOUT_OVERLAY_ITP_1P2     ; break;
    case ONE_PLUS_THREE_OVERLAY_ITP: result = CP_LAYOUT_OVERLAY_ITP_1P3     ; break;
    case ONE_PLUS_FOUR_OVERLAY_ITP : result = CP_LAYOUT_OVERLAY_ITP_1P4     ; break;
    case ONE_TOP_LEFT_PLUS_EIGHT   : result = CP_LAYOUT_1TOP_LEFT_P8        ; break;
    case TWO_TOP_PLUS_EIGHT        : result = CP_LAYOUT_2TOP_P8             ; break;

    default:
    {
      DBGFPASSERT(1);
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridge
////////////////////////////////////////////////////////////////////////////
CVideoBridge::CVideoBridge()
{
  m_pLastActiveAudioSpeakerRequest = NULL;
  m_pLastActiveVideoSpeakerRequest = NULL;
  m_IsSameLayout                   = NO;
  m_pLectureModeParams             = new CVideoBridgeLectureModeParams;
  m_pAutoLayoutSet                 = NULL;
  m_videoQuality                   = eVideoQualitySharpness; // Sharpness is the default video quality
  m_pAutoScanParams                = new CVideoBridgeAutoScanParams(this);
  m_pMessageOverlayInfo            = new CMessageOverlayInfo;
  m_eRecordingIcon                 = E_ICON_REC_OFF;
  m_numAudioParticipants           = 0;
  m_isSiteNamesEnabled             = YES;
  m_pSiteNameInfo                  = new CSiteNameInfo;
  m_pTelepresenceLayoutMngr        = NULL;
  m_telepresenceOnOff              = FALSE;			//_e_m_
  m_bManageTelepresenceLayoutsInternally   = FALSE;	//_e_m_
  m_pPartyImageVector              = new CPartyImageVector;
  m_ConfLayout                     = new CLayout(CP_LAYOUT_1X1, ""); // active layout
  m_changedLayoutIdsList           = new ChangedLayoutIdsList;
  m_indicationIconIdsList          = new IndicationIconIdsList;
  m_pLastSpeakerNotationParty      = NULL;
  //VNGR-26449 - unencrypted conference message
  m_isPermanentSecureMessage       = FALSE;
  m_isPermanentMessageOverlay      = FALSE;
  m_messageOverlayText             = "";
  m_isMuteAllVideoButLeader        = NO;
  m_isMuteAllVideoButLeaderForJustIncoming = FALSE;
  m_pAVMCUMngr                     = NULL;
  m_isAutoLayout                   = FALSE;

  VALIDATEMESSAGEMAP;
}

// ------------------------------------------------------------------------------------------
CVideoBridge::~CVideoBridge()
{
  POBJDELETE(m_pLectureModeParams);
  POBJDELETE(m_pAutoLayoutSet);
  POBJDELETE(m_pAutoScanParams);
  POBJDELETE(m_pMessageOverlayInfo);
  POBJDELETE(m_pSiteNameInfo);
  POBJDELETE(m_pPartyImageVector);
  POBJDELETE(m_ConfLayout);
  POBJDELETE(m_changedLayoutIdsList);
  POBJDELETE(m_indicationIconIdsList);
  POBJDELETE(m_pTelepresenceLayoutMngr);
  POBJDELETE(m_pAVMCUMngr);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::Create(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
  if (!CPObject::IsValidPObjectPtr(pVideoBridgeInitParams))
  {
    PASSERTMSG(1,"CVideoBridge::Create - invalid VideoBridgeInitParams");
    m_pConfApi->EndVidBrdgConnect(statInconsistent);
    return;
  }

  CBridge::Create((CBridgeInitParams*)pVideoBridgeInitParams);

  // aplications params
  m_IsSameLayout = (pVideoBridgeInitParams->GetIsSameLayout()) ? YES : NO;
  m_isAutoLayout = (pVideoBridgeInitParams->GetIsAutoLayout()) ? YES : NO;

  *m_pLectureModeParams = *(pVideoBridgeInitParams->GetLectureModeParams());
  UpdateConfDBLectureMode();
  m_videoQuality = pVideoBridgeInitParams->GetVideoQuality();
  m_isSiteNamesEnabled = pVideoBridgeInitParams->GetIsSiteNamesEnabled();

  CCommConf* pCommConf = pVideoBridgeInitParams->GetCommConf();
  if (IsValidPObjectPtr(pCommConf))
  {
    m_pAutoScanParams->SetTimerInterval(pCommConf->GetAutoScanInterval());
    *m_pMessageOverlayInfo = *(pCommConf->GetMessageOverlay());
    m_bManageTelepresenceLayoutsInternally = pCommConf->GetManageTelepresenceLayoutInternaly();		//_e_m_
  }
  m_isMuteAllVideoButLeader = pVideoBridgeInitParams->GetIsMuteAllVideoButLeader();

}
// ------------------------------------------------------------------------------------------
void CVideoBridge::Destroy ()
{
  CBridge::Destroy();
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams)
{
  PTRACE2(eLevelInfoNormal,"CVideoBridge::InitBridgeParams - should be implemented in derived class , ConfName - ", m_pConfName);
  DBGPASSERT_AND_RETURN(1);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::DeletePartyFromConf(const char* partyName)
{
	CSegment seg;
	seg << partyName;
	DispatchEvent(DELETED_PARTY_FROM_CONF, &seg);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::UpdateMute(PartyRsrcID partyId, EOnOff eOnOff, WORD srcRequest)
{
	CSegment seg;
	seg
		<< srcRequest
		<< partyId
		<< (BYTE)eOnOff
		<< (BYTE)eMediaIn;
	DispatchEvent(VIDEOMUTE, &seg);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::VideoRefresh(PartyRsrcID partyId)
{
	CSegment seg;
	seg << partyId;
	DispatchEvent(VIDREFRESH, &seg);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::SetSiteName(const char* partyName, const char* visualName)
{
	CSegment seg;
	seg << partyName << visualName;
	DispatchEvent(SET_SITE_AND_VISUAL_NAME, &seg);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::IvrPartyCommand(PartyRsrcID partyId, OPCODE opcode, CSegment *pDataSeg)
{
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pVideoPartyCntl, "PartyId:" << partyId << ", Opcode:" << opcode);

	pVideoPartyCntl->IvrCommand(opcode, pDataSeg);
}

//--------------------------------------------------------------------------
void CVideoBridge::AddPartyImage(PartyRsrcID partyId)
{
	m_pPartyImageVector->push_back(partyId);
}

//--------------------------------------------------------------------------
bool CVideoBridge::DelPartyImage(PartyRsrcID partyId)
{
	CPartyImageVector::iterator _ii = std::find(m_pPartyImageVector->begin(), m_pPartyImageVector->end(), partyId);
	if (_ii != m_pPartyImageVector->end())
	{
		m_pPartyImageVector->erase(_ii);
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------
bool CVideoBridge::IsPartyImageExistInImageVector(PartyRsrcID partyId)
{
	CPartyImageVector::iterator _ii = std::find(m_pPartyImageVector->begin(), m_pPartyImageVector->end(), partyId);
	return _ii != m_pPartyImageVector->end() ? true : false;
}

//--------------------------------------------------------------------------
CImage* CVideoBridge::GetPartyImage(CLayout& layout, WORD position)
{
	CVidSubImage* pVidSubImage = layout.GetSubImageNum(position);
	if (pVidSubImage)
	{
		DWORD partyRscId = pVidSubImage->GetImageId();
		if (partyRscId)
		{
			CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyRscId);
			PASSERTSTREAM(!pImage, "PartyId:" << partyRscId);

			return pImage;
		}
	}
	return NULL;
}

//--------------------------------------------------------------------------
DWORD CVideoBridge::GetOldestAVMCUSpeaker(PartyRsrcID deletedPartyId)
{
	for (CPartyImageVector::reverse_iterator _ii = m_pPartyImageVector->rbegin(); _ii != m_pPartyImageVector->rend(); ++_ii)
	{
		CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(*_ii);
		if (!pVideoPartyCntl)
			continue;
		if (deletedPartyId != SOURCE_NONE_DOMINANT_SPEAKER_MSI)
		{
			if (deletedPartyId == pVideoPartyCntl->GetPartyRsrcID())
				continue;
		}
		if (pVideoPartyCntl->IsAVMCUParty() && IsValidMSI(pVideoPartyCntl->GetVideoMSI()))
			return pVideoPartyCntl->GetPartyRsrcID();
	}
	return DUMMY_PARTY_ID;
}

//--------------------------------------------------------------------------
bool CVideoBridge::IsValidMSI(LyncMsi msi, bool isIncldueLocalMsi)
{
	bool isMSIValid = false;
	if (!m_pAVMCUMngr)
		return isMSIValid;

	LyncMsi localAudioMSI = m_pAVMCUMngr->GetMSAVMCULocalAUdioMSI();
	LyncMsi localVideoMSI = m_pAVMCUMngr->GetMSAVMCULocalVideoMSI();

	if (msi > 0 && msi != SOURCE_NONE_DOMINANT_SPEAKER_MSI && msi != SOURCE_ANY_DOMINANT_SPEAKER_MSI && msi != DUMMY_DOMINANT_SPEAKER_MSI)
	{
		if (isIncldueLocalMsi)
		{
			if (msi != localAudioMSI && msi != localVideoMSI)
			{
				isMSIValid = true;
			}
		}
		else
			isMSIValid = true;
	}

	TRACEINTO << "LocalVideoMSI:" << localVideoMSI << ", LocalAudioMSI:" << localAudioMSI << ", MSI:" << msi << ", IsValid:" << (int)isMSIValid;
	return isMSIValid;
}


//--------------------------------------------------------------------------
WORD CVideoBridge::GetNumOfAVMCUBranches()
{
	WORD numOfAVMCUBranches = 0;

	for(CPartyImageVector::iterator _ii = m_pPartyImageVector->begin();_ii != m_pPartyImageVector->end(); _ii++)
	{
		CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(*_ii);
		if (!pVideoPartyCntl)
			continue;
		if (pVideoPartyCntl->IsAVMCUConnection())
			numOfAVMCUBranches++;
	}
	return numOfAVMCUBranches;
}

//--------------------------------------------------------------------------
void CVideoBridge::SetPartyImageSpeakerId(PartyRsrcID partyId)
{
	PASSERT_AND_RETURN(!DelPartyImage(partyId));
	m_pPartyImageVector->insert(m_pPartyImageVector->begin(), partyId);
}

//--------------------------------------------------------------------------
PartyRsrcID CVideoBridge::GetPartyImageSpeakerId() const
{
	return GetPartyImageIdByPosition(0);
}

//--------------------------------------------------------------------------
PartyRsrcID CVideoBridge::GetPartyImageIdByPosition(WORD position) const
{
	if (position < GetPartyImageVectorSize())
		return (*m_pPartyImageVector)[position];
	TRACEINTO << "Position:" << position << " - Failed, position exceeded vector size";
	return 0;
}

//--------------------------------------------------------------------------
WORD CVideoBridge::GetPartyImageVectorSizeUnmuted() const
{
	WORD numberOfUnmutedImages = 0;

	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	for (CPartyImageVector::iterator _ii = m_pPartyImageVector->begin(); _ii != m_pPartyImageVector->end(); ++_ii)
	{
		CImage* pImage = pPartyImageLookupTable->GetPartyImage(*_ii);
		PASSERTSTREAM(!pImage, "PartyId:" << *_ii);

		if (pImage && !pImage->isMuted())
			numberOfUnmutedImages++;
	}
	return numberOfUnmutedImages;
}

//--------------------------------------------------------------------------
PartyRsrcID CVideoBridge::GetLastActiveAudioSpeakerId() const
{
	return m_pLastActiveAudioSpeakerRequest ? ((CParty*)m_pLastActiveAudioSpeakerRequest)->GetPartyRsrcID() : INVALID;
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::GetCroppingValues(DWORD& croppingHorr,DWORD& croppingVer)
{
	if(m_pConf->IsHDVSW())
	{
		croppingHorr = INVALID;
		croppingVer  = INVALID;
	}
	else if ((m_pConf->GetCommConf())->GetIsTelePresenceMode())
	{
		string ITPCroppingMode = "";
		BOOL isITPCroppingMode = CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey("ITP_CROPPING", ITPCroppingMode);
		if (isITPCroppingMode)
		{
		    if(ITPCroppingMode.compare("ITP")==0)
			{
		           croppingHorr = INVALID;
		           croppingVer  = 50;
			}
			else if(ITPCroppingMode.compare("CP")==0)
			{
				croppingHorr = 50;
			    croppingVer  = 50;
			}
			else if(ITPCroppingMode.compare("MIXED")==0)
			{
			    croppingHorr = 50;
				croppingVer  = 84;
			}
	    }
		else
		{
			 croppingHorr = INVALID;
			 croppingVer  = 84;
		}
	}
	else if((m_pConf->GetCommConf())->GetIsCropping())
	{
		croppingHorr = 50;
		croppingVer  = 50;
	}
	else
	{
		croppingHorr = INVALID;
		croppingVer  = INVALID;
	}

}

// ------------------------------------------------------------------------------------------
const char* CVideoBridge::GetLecturerName() const
{
	if(m_pLectureModeParams){
		return (m_pLectureModeParams->GetLecturerName());
	}else{
		PASSERTMSG(1, "CVideoBridge::GetLecturerName - Invalid Lecture Mode Params");
		return "Lecture Name Invalid";
	}
}
// ------------------------------------------------------------------------------------------

BOOL CVideoBridge::SetAutoLayout(BOOL isSet)
{
	TRACEINTO << "TurnON:" << (int)isSet << ", IsAlreadyAutoLayout:" << (GetIsAutoLayout() ? "1" : "0");

	// check no change ->return
	if ((GetIsAutoLayout() && isSet) || (!GetIsAutoLayout() && !isSet))
		return FALSE;

	SetIsAutoLayout(isSet);

	if (isSet) // auto layout turned on
	{
		//Conference started with profile unchecked auto Layout and then change the field to checked
		if (m_pAutoLayoutSet == NULL)
			m_pAutoLayoutSet = new CAutoLayoutSet;

		StartAutoLayout();
	}

	m_pConfApi->UpdateDB((CTaskApp*)0xFFFF, UPDATEAUTOLAYOUT, isSet);
	return TRUE;
}
// ------------------------------------------------------------------------------------------
CLayout* CVideoBridge::GetReservationLayout(void)const
{
	PTRACE2(eLevelInfoNormal,"CVideoBridge::GetReservationLayout - should be implemented in derived class , ConfName:", m_pConfName);
	PASSERT(1);
	return NULL;
}
// ------------------------------------------------------------------------------------------
CLayout* CVideoBridge::GetAnyPartyLayout() const
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::GetAnyPartyLayout - ConfName:", m_pConfName);

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			CLayout* pLayout = pPartyCntl->GetCurrentLayout();
		if (pLayout)
			return pLayout;
	}
	}
	return NULL;
}
// ------------------------------------------------------------------------------------------
PartyMonitorID CVideoBridge::GetMonitorPartyId(const CTaskApp* pParty) const
{
	if (!IsValidPObjectPtr(pParty))
		return INVALID;

	CPartyConnection* pPartyConnection = m_pConf->GetPartyConnection(pParty);
	return IsValidPObjectPtr(pPartyConnection) ? pPartyConnection->GetMonitorPartyId() : INVALID;
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::StartAutoLayout()
{
  PTRACE2(eLevelInfoNormal,"CVideoBridge::StartAutoLayout - do nothing in base class, ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::StartLectureMode()
{
  PASSERTMSG_AND_RETURN(!m_pLectureModeParams, "Failed, 'm_pLectureModeParams' is invalid");
  PASSERTMSG_AND_RETURN(!m_pLectureModeParams->GetLecturerName(), "Failed, 'LecturerName' is invalid");
  DBGPASSERT_AND_RETURN(!m_pLectureModeParams->IsLectureModeOn()); // the lecture mode signal must be ON

  if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular && m_pLectureModeParams->GetIsTimerOn())
    StartTimer(LECTURE_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->SetPartyLectureModeRole(GetLectureModeRoleForParty(pPartyCntl->GetName()));
  }

  UpdateConfDBLectureMode();
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::EndLectureMode(BYTE removeLecturer)
{
  // if removeLecturer == 1 this mean that the conference goes to mode regular and LM is over
  // if removeLecturer == 0 this mean that the conference still in LM but temporarly goes to mode transcoding
	PASSERT_AND_RETURN(!m_pLectureModeParams);

  DeleteTimer(LECTURE_MODE_TOUT);

  if (removeLecturer == YES)
    m_pLectureModeParams->SetLecturerName("");

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->SetPartyLectureModeRole(GetLectureModeRoleForParty(pPartyCntl->GetName()));
  }

  UpdateConfDBLectureMode();
}

// ------------------------------------------------------------------------------------------
ePartyLectureModeRole CVideoBridge::GetLectureModeRoleForParty(const char* pPartyName)
{
  ePartyLectureModeRole partyLectureModeRole = eREGULAR;

  PASSERTMSG_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(m_pLectureModeParams), "Failed, 'm_pLectureModeParams' is invalid", partyLectureModeRole);

  if (m_pLectureModeParams->GetLectureModeType() != eLectureModeNone)
  {
    if (strncmp(pPartyName, m_pLectureModeParams->GetLecturerName(), H243_NAME_LEN) == 0)
    {
      partyLectureModeRole = eLECTURER;
    }
    else
    {
      if (strcmp("", m_pLectureModeParams->GetLecturerName()) == 0)
      {
        partyLectureModeRole = eREGULAR;
      }
      else // m_pCurrentLecturerName == some Name
      {
        CBridgePartyCntl* pLecturer = GetPartyCntl(m_pLectureModeParams->GetLecturerName());
        partyLectureModeRole = (pLecturer) ? eLISTENER : eCOLD_LISTENER;
      }
    }
  }
  return partyLectureModeRole;
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::UpdateConfDBLectureMode() const
{
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pLectureModeParams));

  CMedString mstr;
  mstr << "LectureModeParams[";
  m_pLectureModeParams->Dump(mstr);
  mstr << "]";

  PTRACE2(eLevelInfoNormal, "CVideoBridge::UpdateConfDBLectureMode: ", mstr.GetString());

  CLectureModeParams* pApiLectureModeParams = new CLectureModeParams;

  switch (m_pLectureModeParams->GetLectureModeType())
  {
    case eLectureModeNone        : { pApiLectureModeParams->SetLectureModeType(0); break; }
    case eLectureModeRegular     : { pApiLectureModeParams->SetLectureModeType(1); break; }
    case eLectureModePresentation: { pApiLectureModeParams->SetLectureModeType(3); break; }
    default                      : { PASSERTMSG(1, "Invalid lecture mode type"); }
  }

  pApiLectureModeParams->SetLecturerName(m_pLectureModeParams->GetLecturerName());
  pApiLectureModeParams->SetLectureTimeInterval(m_pLectureModeParams->GetTimerInterval());
  pApiLectureModeParams->SetTimerOnOff(m_pLectureModeParams->GetIsTimerOn());
  pApiLectureModeParams->SetAudioActivated(m_pLectureModeParams->IsAudioActivatedLectureMode());

  CSegment* pSeg = new CSegment;
  pApiLectureModeParams->Serialize(NATIVE, *pSeg);

  m_pConfApi->UpdateDB((CTaskApp*)0xffff, UPDATELECTUREMODE, (DWORD)0, 0, pSeg);

  POBJDELETE(pApiLectureModeParams);
  POBJDELETE(pSeg);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::StartPresentationMode()
{
  PTRACE2(eLevelInfoNormal,"CVideoBridge::StartPresentationMode - do nothing in base class implemented only for CP, ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::ApplicationActionsOnAudioSpeakerChange()
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::ApplicationActionsOnAudioSpeakerChange - ConfName:", m_pConfName);

  if (m_pLectureModeParams->GetLectureModeType() != eLectureModePresentation)
    return;

  DeleteTimer(PRESENTATION_MODE_TOUT);
  StartTimer(PRESENTATION_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);

	if (strlen(m_pLectureModeParams->GetLecturerName()) == 0)
    return;

  CBridgePartyCntl* pLecturer = GetPartyCntl(m_pLectureModeParams->GetLecturerName());
	if (!pLecturer)
    return;

  EndLectureMode(YES); // if there is currently an active lecturer -> stop the lecture mode
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::ApplicationActionsOnAddPartyToMix(CVideoBridgePartyCntl* pPartyCntl)
{
  PTRACE2(eLevelError, "CVideoBridge::ApplicationActionsOnAddPartyToMix - ConfName:", m_pConfName);

  if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular)
  {
    if (GetLectureModeRoleForParty(pPartyCntl->GetName()) == eLECTURER)
    {
      StartLectureMode();
      UpdateConfDBLectureMode();
    }
  }
  StartAutoLayout();
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::ApplicationActionsOnRemovePartyFromMix(CVideoBridgePartyCntl* pPartyCntl)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::ApplicationActionsOnRemovePartyFromMix - ConfName:", m_pConfName);

  if (GetLectureModeRoleForParty(pPartyCntl->GetName()) == eLECTURER)
  {
    if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular)
      EndLectureMode(NO);
    else if (m_pLectureModeParams->GetLectureModeType() == eLectureModePresentation)
      EndLectureMode(YES);
  }
  StartAutoLayout();
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::ApplicationActionsOnDeletePartyFromConf(const char* pDeletedPartyName)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::ApplicationActionsOnDeletePartyFromConf - ConfName:", m_pConfName);

  if (eLECTURER == GetLectureModeRoleForParty(pDeletedPartyName))
    EndLectureMode(YES);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::ConnectParty(CVideoBridgePartyCntl*  pBridgePartyCntl, BYTE isIVR)
{
  m_pPartyList->Insert(pBridgePartyCntl);
  if(pBridgePartyCntl->GetAVMCUMasterPartyRsrcID()!=0 )
  {
	  PTRACE2(eLevelInfoNormal, "CVideoBridge::ConnectParty -av-mcu party without ivr!!! - ConfName:", m_pConfName);
	  isIVR=NO;
  }
  pBridgePartyCntl->Connect(isIVR);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::GetForcesFromReservation(CCommConf* pCommConf)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::GetForcesFromReservation - should be implemented in derived class - ConfName:", m_pConfName);
  PASSERT_AND_RETURN(1);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::ResendLastActiveVideoSpeakerRequest()
{
	// in case the participant in not connected yet we save last video active speaker request
	// and resend it after the party connects
	BYTE rShouldResend = YES;

	std::ostringstream msg;
	if (CPObject::IsValidPObjectPtr(m_pLastActiveVideoSpeakerRequest) && !CPObject::IsValidPObjectPtr(m_pLastActiveAudioSpeakerRequest))
	{
		msg << " - Audio speaker not valid, using Video speaker";
		m_pLastActiveAudioSpeakerRequest = m_pLastActiveVideoSpeakerRequest;
	}

	LyncMsi speakerVideoMsi = 0xFFFFFFF0;
	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(((CParty*)m_pLastActiveVideoSpeakerRequest)->GetPartyRsrcID());
	if (pPartyCntl)
	{
		speakerVideoMsi = pPartyCntl->GetVideoMSI();
		TRACEINTO << "ConfName:" << m_pConfName << ", SpeakerVideoMsi:" << speakerVideoMsi << msg.str().c_str();
		if(pPartyCntl->IsAVMCUParty())
		{
			PartyRsrcID masterAVMCUPartyRsrcID = pPartyCntl->GetAVMCUMasterPartyRsrcID();
			CTaskApp* pAVMCUMasterTaskApp = GetPartyTaskApp(masterAVMCUPartyRsrcID);
			if(!pAVMCUMasterTaskApp)
			{
				TRACEINTO << " No Connected Master Party for AV-MCU slave, reject resend speaker indication, Slave Name: " << pPartyCntl->GetFullName();
				return;
			}
			CSegment* pSeg = new CSegment;
			*pSeg <<  pAVMCUMasterTaskApp << pAVMCUMasterTaskApp << (BYTE)rShouldResend << speakerVideoMsi;

			DispatchEvent(SPEAKERS_CHANGED, pSeg);
			POBJDELETE(pSeg);
			return;
		}
	}

	CSegment* pSeg = new CSegment;


	*pSeg << m_pLastActiveVideoSpeakerRequest << m_pLastActiveAudioSpeakerRequest << (BYTE)rShouldResend << speakerVideoMsi;

	DispatchEvent(SPEAKERS_CHANGED, pSeg);
	POBJDELETE(pSeg);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::AddPartyToConfMixAfterVideoInSynced(const CTaskApp *pParty)
{
  PTRACE2(eLevelError, "CVideoBridge::AddPartyToConfMixAfterVideoInSynced - should be implemented in derived class - ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::RemovePartyFromConfMixBeforeDisconnecting(const CTaskApp *pParty)
{
  PTRACE2(eLevelError, "CVideoBridge::RemovePartyFromConfMixBeforeDisconnecting - should be implemented in derived class - ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::RemovePartyFromAnyConfSettingsWhenDeletedFromConf(const char* pDeletedPartyName)
{
  PTRACE2(eLevelError, "CVideoBridge::RemovePartyFromAnyConfSettingsWhenDeletedFromConf - should be implemented in derived class - ConfName:", m_pConfName);
}
//VNGR-21391 in case of disconnecting party we need to remove the party from the vector of seen image even if the VB is in state of disconnecting,
//In case the VB is in disconnecting state  we can avoid the other application actions after disconnecting as updating all the parties to initiate build layout
// ------------------------------------------------------------------------------------------
void CVideoBridge::RemovePartyFromConfMixBeforeDisconnectingDISCONNECTING(const CTaskApp* pParty)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	DelPartyImage(pParty->GetPartyId());
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfDeletePartyFromConfCONNECTED(CSegment* pParam)
{
	char deletedPartyName[H243_NAME_LEN];
	*pParam >> deletedPartyName;
	deletedPartyName[H243_NAME_LEN-1] = '\0';

	TRACEINTO << "PartyName:" << deletedPartyName;
	RemovePartyFromAnyConfSettingsWhenDeletedFromConf(deletedPartyName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfConnectPartyCONNECTED(CSegment* pParam)
{
	CBridgePartyInitParams partyInitParams;
	partyInitParams.DeSerialize(NATIVE, *pParam);

	if (!partyInitParams.IsValidParams())
	{
		if (CorrectPartyInitParams(partyInitParams) == FALSE)
		{
			TRACEINTO << "ConfName:" << m_pConfName << " - Failed, invalid video params";
			CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_VIDEO_DISCONNECTED, statInvalidPartyInitParams);
			return;
		}
	}

	const CParty* pParty = (CParty*)partyInitParams.GetParty();
	PASSERT_AND_RETURN(!pParty);

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CVideoBridgePartyInitParams videoBridgePartyInitParams(partyInitParams);

	videoBridgePartyInitParams.SetSiteNameInfo(m_pSiteNameInfo);

	// If this party exist in this conference...
	// We don't need to create new VideoBridgePartyCntl - only add or update one of the directions -VideoIn or VideoOut
	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	if (pPartyCntl)
	{
		TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << partyId << " - Connect existing party";
		pPartyCntl->Update(&videoBridgePartyInitParams);
		pPartyCntl->Connect(partyInitParams.IsIvrInConf());
	}
	else // If move from other conference....
	{
		pPartyCntl = (CVideoBridgePartyCntl*)partyInitParams.GetPartyCntl(); // old BridgePartyCntl
		if (pPartyCntl)
		{
			// Only if the party is legacy do action else do nothing since no legacy part stay regular even if destination is Legacy!!!
			// VNGR-24445 - Test is party is legacy in Source Conference or should be legacy in current Conference
			if ((videoBridgePartyInitParams.IsLegacyParty() || pPartyCntl->IsLegacyParty()) && !m_pConf->IsHDVSW())                      // Only if the party is legacy do action else do nothing since no legacy part stay regulareven if dest is Legacy!!!
			{
				if (IsLegacyBridge())
				{
					TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Importing of Legacy party to Legacy conference";

					// move Legacy party that might be represented by normal partycntl, to legacy bridge. I don't want to ask about the source to optimize since it might be deleted...
					CVideoBridgePartyCntlLegacy* pPartyCntlNew = new CVideoBridgePartyCntlLegacy();
					*pPartyCntlNew = *pPartyCntl;
					pPartyCntlNew->SetParamsRelatedToMove(pPartyCntl->GetState());
					pPartyCntlNew->ImportLegacy(&videoBridgePartyInitParams, pPartyCntl);
					// Insert the party to the PartyCtl List and activate Connect on it
					ConnectParty(pPartyCntlNew, partyInitParams.IsIvrInConf());
				}
				else
				{
					// move Legacy party to NONE legacy bridge. I don't want to ask about the source to optimize since it might be deleted...
					TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Importing of Legacy party to NONE Legacy conference";
					CVideoBridgePartyCntl* pPartyCntlNew = new CVideoBridgePartyCntl();
					*pPartyCntlNew = *pPartyCntl;
					((CVideoBridgePartyCntlLegacy*)pPartyCntlNew)->SetParamsRelatedToMove(pPartyCntl->GetState()); // didn't put in the base class to save it as Legacy feature.
					pPartyCntlNew->ImportLegacy(&videoBridgePartyInitParams, pPartyCntl);

					// Insert the party to the PartyCtl List and activate Connect on it
					ConnectParty(pPartyCntlNew, partyInitParams.IsIvrInConf());
				}

				// VNGR-26574 delete old pVideoBrdgPartyCntl inside ImportLegacy before creating new CImage
				// POBJDELETE(pVideoBrdgPartyCntl) // DELETE the old
			}
			else // Party with content caps and non-legacy according to Content Conference settings
			{
				TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Importing of Regular party";
				pPartyCntl->Import(&videoBridgePartyInitParams);
				// Insert the party to the PartyCtl List and activate Connect on it
				ConnectParty(pPartyCntl, partyInitParams.IsIvrInConf());
			}
		}
		else // NOT In MOVE
		{
			TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << videoBridgePartyInitParams.GetPartyRsrcID();

			// To be able to create a list of LegacyPartyCntl within RegularPartyCntl for the LegacyContentBridge
			CreateAndNewPartyCntl(pPartyCntl, &videoBridgePartyInitParams);

			// Insert the party to the PartyCtl List and activate Connect on it
			ConnectParty(pPartyCntl, partyInitParams.IsIvrInConf());
		}
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl, CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
	PTRACE2(eLevelError, "CVideoBridge::CreateAndNewPartyCntl - should be implemented in derived class - ConfName:", m_pConfName);
	CVideoBridge::NewPartyCntl(pVideoBrdgPartyCntl);
	pVideoBrdgPartyCntl->Create(pVideoBridgePartyInitParams);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl)
{
	PTRACE2(eLevelError, "CVideoBridge::NewPartyCntl - should be implemented in derived class - ConfName:", m_pConfName);
	// cp as default in case of error
	pVideoBrdgPartyCntl = new CVideoBridgePartyCntl();
}

// ------------------------------------------------------------------------------------------
WORD CVideoBridge::CorrectPartyInitParams(CBridgePartyInitParams& partyInitParams)
{
  return FALSE;
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfConnectPartyDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelError, "CVideoBridge::OnConfConnectPartyDISCONNECTING - Connect Party rejected, ConfName:", m_pConfName);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfDisConnectPartyCONNECTED(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID partyId = partyDisconnectParams.GetPartyId();
	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();

	TRACEINTO << "PartyId:" << partyId << ", MediaDirection:" << eMediaDirection;

	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
	PASSERT_AND_RETURN(!pParty);

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party already disconnected";
		m_pConfApi->PartyBridgeResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_DISCONNECTED, statOK, 1, eNoDirection);
		return;
	}

	pPartyCntl->SetDisConnectingDirectionsReq(eMediaDirection);

	CCommConf* pCommConf = (CCommConf*)(GetConf())->GetCommConf();
	if (NULL != pCommConf)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
		if ((NULL != m_pConf) && (NULL != pConfParty))
        	{
            	m_pConf->OnPartyDisConnected(pConfParty);//added for BRIDGE-13167
        	}
	}

	// if video out is disconnecting we won't remove party from Mix
	if (eMediaDirection == eMediaIn || eMediaDirection == eMediaInAndOut)
	{
		TRACEINTO << "PartyId:" << partyId << " - Remove Party From Mix";
		RemovePartyFromConfMixBeforeDisconnecting(pParty);
		if (m_pTelepresenceLayoutMngr != NULL)
			OnConfDisConnectPartyWithTelepresence(pPartyCntl);
	}
	else
	{
		TRACEINTO << "PartyId:" << partyId << " - Not media Direction IN, keep with disconnecting";
	}

	CBridge::DisconnectParty(partyId);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfDisConnectPartyDISCONNECTING(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID partyId = partyDisconnectParams.GetPartyId();
	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();

	TRACEINTO << "PartyId:" << partyId << ", MediaDirection:" << eMediaDirection;

	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
	PASSERT_AND_RETURN(!pParty);

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party already disconnected";
		m_pConfApi->PartyBridgeResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_DISCONNECTED, statOK, 1, eNoDirection);
		return;
	}

	pPartyCntl->SetDisConnectingDirectionsReq(eMediaDirection);
	// VNGR-21391 in case of disconnecting party we need to remove the party from the vector of seen image even if the VB is in state of disconnecting,
	// In case the VB is in disconnecting state  we can avoid the other application actions after disconnecting as updating all the parties to initate build layout
	if (eMediaDirection == eMediaIn || eMediaDirection == eMediaInAndOut)
	{
		RemovePartyFromConfMixBeforeDisconnectingDISCONNECTING(pParty);
		if (m_pTelepresenceLayoutMngr != NULL)
			OnConfDisConnectPartyWithTelepresence(pPartyCntl);
	}

	CBridge::DisconnectParty(partyId);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfDisConnectPartyWithTelepresence(CVideoBridgePartyCntl* pPartyCntl)
{
	const CTelepresenseEPInfo& tpInfo = pPartyCntl->GetTelepresenceInfo();
	TRACEINTO << "SubLink:" << tpInfo.GetLinkNum() << ",  Roomid:" << tpInfo.GetRoomID();
	// if sublink, reset the link party id, if mainlink- remove room
	if (tpInfo.GetLinkNum() != 0)
	{
		m_pTelepresenceLayoutMngr->UpdateRoomSubLink(tpInfo.GetRoomID(), tpInfo.GetLinkNum(), (PartyRsrcID)-1);
	}
	else
		m_pTelepresenceLayoutMngr->RemoveRoom(tpInfo.GetRoomID());
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfExportPartyCONNECTED(CSegment* pParam)
{
	CBridgePartyExportParams bridgePartyExportParams;
	bridgePartyExportParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID PartyId = bridgePartyExportParams.GetPartyId();

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << PartyId;

	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(PartyId);
	PASSERT_AND_RETURN(!pParty);

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(PartyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << PartyId << " - Party video not connected (cannot export)";
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_EXPORTED, statIllegal, 1);
		return;
	}

	m_pConf->StopGathering(pPartyCntl->GetName());

	RemovePartyFromConfMixBeforeDisconnecting(pParty);                       // we currently support only move from EQ therefore this function will not actually cause any change
	RemovePartyFromAnyConfSettingsWhenDeletedFromConf(pPartyCntl->GetName());// we currently support only move from EQ therefore this function will not actually cause any change

	CBridge::ExportParty(PartyId);
}

//--------------------------------------------------------------------------
void CVideoBridge::OnEndPartyExportCONNECTED(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

  CBridge::EndPartyExport(pParam, PARTY_VIDEO_EXPORTED);
}

//--------------------------------------------------------------------------
void CVideoBridge::OnEndPartyConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	CSegment seg(*pParam);
	SendMessageOverlayToNewConnectedPartyIfNeeded(&seg);

	// this function is called twice when video connected with IVR
	CBridge::EndPartyConnect(pParam, PARTY_VIDEO_CONNECTED);
}

//--------------------------------------------------------------------------
void CVideoBridge::OnEndPartyConnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Do nothing...";
}

//--------------------------------------------------------------------------
void CVideoBridge::OnEndPartyConnectIVRModeCONNECTED(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	// this function is called twice when video connected with IVR
	CBridge::EndPartyConnect(pParam, PARTY_VIDEO_IVR_MODE_CONNECTED);
}

//--------------------------------------------------------------------------
void CVideoBridge::OnEndPartyConnectIVRModeDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Do nothing...";
}

//--------------------------------------------------------------------------
void CVideoBridge::OnEndPartyDisConnect(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

  CBridge::EndPartyDisConnect(pParam, PARTY_VIDEO_DISCONNECTED);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::OnEndPartyUpdateVideoInCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnEndPartyUpdateVideoInCONNECTED - ConfName:", m_pConfName);

  CTaskApp* pParty         = NULL;
  WORD      status         = statIllegal;
  BOOL      isPartyCntlMsg = TRUE;

  *pParam >> (void*&)pParty >> status;

  m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, status, isPartyCntlMsg);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnEndPartyUpdateVideoInDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnEndPartyUpdateVideoInDISCONNECTING - Do nothing..., ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnEndPartyUpdateVideoOutCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnEndPartyUpdateVideoOutCONNECTED - ConfName:", m_pConfName);

  CTaskApp* pParty           = NULL;
  WORD      status           = statIllegal;
  BOOL      isPartyCntlMsg   = TRUE;
  CSegment* pSeg             = NULL;

  CAckParams*     pAckParams = NULL;
  EMediaDirection eMediaDirection;
  BOOL            IsExtParams;

  *pParam >> (void*&)pParty >> status >> (WORD&)eMediaDirection >> (BYTE&)IsExtParams;

  if (status == statOK && IsExtParams)
  {
    // If there is valid ack params we need to send with the response to party the Ack params.
    pAckParams = new CAckParams;
    pAckParams->DeSerialize(NATIVE, *pParam);

    pSeg       = new CSegment;
    pAckParams->Serialize(NATIVE, *pSeg);
  }

  m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, status, isPartyCntlMsg, pSeg);

  POBJDELETE(pSeg);
  POBJDELETE(pAckParams);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnEndPartyUpdateVideoOutDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnEndPartyUpdateVideoOutDISCONNECTING - Do nothing..., ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfDisConnectConfCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnConfDisConnectConfCONNECTED - ConfName:", m_pConfName);

  // Change to disconnecting state - to prevent connecting new parties
  m_state = DISCONNECTING;
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfDisConnectConfDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnConfDisConnectConfDISCONNECTING - Do nothing... (Bridge is already diconnecting), ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfTerminateCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnConfTerminateCONNECTED - ConfName:", m_pConfName);
  PASSERTMSG(1, "CVideoBridge::OnConfTerminateCONNECTED - Failed, the video bridge should be in disconnecting state");
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfTerminateDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnConfTerminateDISCONNECTING - ConfName:", m_pConfName);

  EStat eStatus = statOK;

  m_state = IDLE;

	if (m_pPartyList->size())
  {
    // Upon receiving TERMINATE event, Bridge should be empty
    eStatus = statBridgeIsNotEmpty;

		TRACEINTO << *m_pPartyList << " - Bridge is not empty";
  }

  m_pConfApi->EndVidBrdgDisConnect(eStatus);
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::OnVideoInSyncedCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnVideoInSyncedCONNECTED - ConfName:", m_pConfName);
  CTaskApp* pParty = NULL;
  WORD      status = statIllegal;

  *pParam >> (void*&)pParty >> status;

  PASSERTMSG_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty), "Failed, 'pParty' is invalid");
  if (status != statOK)
  {
    PTRACE2(eLevelError, "CVideoBridge::OnVideoInSyncedCONNECTED - Failed, received bad status - ConfName:", m_pConfName);
    return;
  }

  AddPartyToConfMixAfterVideoInSynced(pParty);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnVideoInSyncedDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridge::OnVideoInSyncedDISCONNECTING - Do nothing..., ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnPartyImageInCellZeroChangedCONNECTED(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal, "CVideoBridge::OnPartyImageInCellZeroChangedCONNECTED");

    CTaskApp* pParty = NULL;
    CTaskApp* pImage = NULL;

    *pParam >> (DWORD&)pParty >> (DWORD&)pImage;

    PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

    SendToPartyVIN(pParty, pImage);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::SendToPartyVIN(const CTaskApp* pParty, const CTaskApp* pImagePartySee)
{
	if (FALSE == ::IsSendVinEnabledInMCU())
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Failed, VIN not supported in MCU";
		return;
	}

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = ((CParty*)pParty)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	if (!CPObject::IsValidPObjectPtr(pImagePartySee))
	{
		// Talya - To Add - Send Cancel Vin
		TRACEINTO << "PartyName:" << pPartyCntl->GetName() << ", PartyId:" << partyId << ", NewVideoSource:NULL, NewVideoSourcePartyId:NULL";
	}
	else
	{
		PartyRsrcID videoSourcePartyId = ((CParty*)pImagePartySee)->GetPartyRsrcID();

		CVideoBridgePartyCntl* pPartyCntlImage = (CVideoBridgePartyCntl*)GetPartyCntl(videoSourcePartyId);
		PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyCntlImage));

		WORD mcuNumber = 0;
		WORD terminalNumber = 0;

		STATUS status = m_pConf->GetPartyTerminalNumber(pImagePartySee, mcuNumber, terminalNumber);
		PASSERT_AND_RETURN(status);

		TRACEINTO
			<< "PartyName:" << pPartyCntl->GetName()
			<< ", PartyId:" << pPartyCntl->GetPartyRsrcID()
			<< ", NewVideoSource:" << pPartyCntlImage->GetName()
			<< ", NewVideoSourcePartyId:" << videoSourcePartyId
			<< ", TerminalNumber:" << terminalNumber;

		pPartyCntl->ForwardVINToParty(mcuNumber, terminalNumber, videoSourcePartyId);
	}
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfSetLectureModeCONNECTED(CSegment* pParam)
{
  CLectureModeParams* pNewResrvationLectureMode = new CLectureModeParams;
  pNewResrvationLectureMode->DeSerialize(NATIVE, *pParam);

  TRACEINTO << "ConfName:" << m_pConfName << *pNewResrvationLectureMode;

  CVideoBridgeLectureModeParams* pNewVideoBridgeLectureMode = new CVideoBridgeLectureModeParams;
  *pNewVideoBridgeLectureMode = *pNewResrvationLectureMode;

  // Same layout - With Lecture Mode or Presentation Mode - rejected
  if (m_IsSameLayout && pNewVideoBridgeLectureMode->GetLectureModeType() != eLectureModeNone)
  {
    PTRACE2(eLevelError, "CVideoBridge::OnConfSetLectureModeCONNECTED - Failed, SameLayout defined and lecture mode is not defined, ConfName:", m_pConfName);
    POBJDELETE(pNewResrvationLectureMode);
    POBJDELETE(pNewVideoBridgeLectureMode);
    return;
  }

  if (*pNewVideoBridgeLectureMode == *m_pLectureModeParams)
  {
    PTRACE2(eLevelError, "CVideoBridge::OnConfSetLectureModeCONNECTED - Failed, lecture mode params does not changed, ConfName:", m_pConfName);
    POBJDELETE(pNewResrvationLectureMode);
    POBJDELETE(pNewVideoBridgeLectureMode);
    return;
  }

  *m_pLectureModeParams = *pNewVideoBridgeLectureMode;

  switch (m_pLectureModeParams->GetLectureModeType())
  {
    case eLectureModeRegular:
    {
      StartLectureMode();
      break;
    }

    case eLectureModePresentation:
    {
      EndLectureMode(YES);
      StartPresentationMode();
      break;
    }

    case eLectureModeNone:
    {
      EndLectureMode(YES);
      break;
    }

    default:
    {
      PASSERTMSG(m_pLectureModeParams->GetLectureModeType(), "CVideoBridge::OnConfSetLectureModeCONNECTED - Failed, invalid lecture mode type");
      break;
    }
  } // switch

  UpdateConfDBLectureMode();
  POBJDELETE(pNewResrvationLectureMode);
  POBJDELETE(pNewVideoBridgeLectureMode);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnSetSiteNameCONNECTED(CSegment* pParam)
{
	// Romem
	char partyName [H243_NAME_LEN]; *pParam >> partyName ; partyName [H243_NAME_LEN-1] = '\0';
	char visualName[H243_NAME_LEN]; *pParam >> visualName; visualName[H243_NAME_LEN-1] = '\0';

	TRACEINTO << "PartyName:" << partyName << ", VisualPartyName:" << visualName;

	CVideoBridgePartyCntl* pCurrParty = (CVideoBridgePartyCntl*)GetPartyCntl(partyName);
	PASSERT_AND_RETURN(!pCurrParty);

	pCurrParty->SetSiteName(visualName);
	PartyRsrcID partyId = pCurrParty->GetPartyRsrcID();

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if (pPartyCntl->IsImageInPartiesLayout(partyId))
				pPartyCntl->UpdateSiteNameInfo(m_pSiteNameInfo);
			}
		}
	}
// ------------------------------------------------------------------------------------------
void CVideoBridge::UpdateDB_ConfLayout()
{
  PTRACE2(eLevelInfoNormal,"CVideoBridge::UpdateDB_ConfLayout - do nothing in base class, ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
RequestPriority CVideoBridge::GetRequestPriority(WORD srcReq)
{
  switch (srcReq)
  {
    case MCMS    : return MCMS_Prior;
    case OPERATOR: return OPERATOR_Prior;
    case PARTY   : return PARTY_Prior;
    default      :
    {
      PTRACE2(eLevelError, "CVideoBridge::GetRequestPriority - Failed, unknown SRC request, ConfName:", m_pConfName);
      DBGPASSERT(srcReq+100);
      break;
    }
  } // switch

  return AUTO_Prior;
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnFreezePic(CSegment* pParams)
{
  PTRACE2(eLevelInfoNormal,"CVideoBridge::OnFreezePic - do nothing in base class, ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnConfUpdateAutoScanIntervalCONNECTED(CSegment* pParam)
{
  WORD newInterval;
  *pParam >> newInterval;

  PTRACE2INT(eLevelInfoNormal, "CVideoBridge::OnConfUpdateAutoScanIntervalCONNECTED - NewInterval:", newInterval);

  if (m_pAutoScanParams)
    m_pAutoScanParams->SetTimerInterval(newInterval);

  if (m_pLectureModeParams)
    m_pLectureModeParams->SetTimerInterval(newInterval);
}
// ------------------------------------------------------------------------------------------
void CVideoBridge::OnDisplayIndicationIconCONNECTED(CSegment* pParam)
{
	WORD iconType = MAX_NUM_TYPES_OF_ICON;
	*pParam >> iconType;
	WORD iconValue = 0;
	*pParam >> iconValue;
	WORD isIconActive = FALSE;

	//Save the icon value so that new party could know the icon status when its video out channel is opened.
	if (eIconRecording == iconType)
	{
		m_eRecordingIcon = (EIconType)iconValue;   //save in the video bridge
		TRACEINTO << "RecordingIcon:" << EIconRecordingTypeNames[m_eRecordingIcon];
	}
	else if (eIconAudioPartiesCount == iconType)
	{
		*pParam >> isIconActive;
		m_numAudioParticipants = iconValue;    //save in the video bridge
		TRACEINTO << "IsIconActive:" << isIconActive << ", IconValue:" << iconValue << " - Audio participants icon";
	}
	else
	{
		TRACEINTO << "IconType:" << iconType << " - Bad indication icon type";
		PASSERT_AND_RETURN(1);
	}

	VerifyIndicationIconIdVectorIsEmpty();

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if (FALSE == pPartyCntl->IsPartyValidForLayoutIndication())
				continue;

			CBridgePartyVideoOut* pBridgePartyVideoOut = (CBridgePartyVideoOut*)pPartyCntl->GetBridgePartyOut();
			if (pBridgePartyVideoOut)
			{
				if (eIconRecording == iconType)
				{
					pBridgePartyVideoOut->SetRecordingType((EIconType)iconValue);
				}
				else if (eIconAudioPartiesCount == iconType)
				{

					pBridgePartyVideoOut->SetNumAudioParticipantsInConf(iconValue);

					//If conf want to hidden the audio icon, but the party just open its video out channel and layout is not built,
					//then party will continue to show the icon by itself.
					if ((!isIconActive) && (!pBridgePartyVideoOut->isAllowConfToHiddenAudioIcon()))
						continue;

					pBridgePartyVideoOut->SetAudioParticipantsIconActive(isIconActive);

					if (pBridgePartyVideoOut->isInGatheringMode())
					{
						TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Doesn't show audio participants icon since it's in gathering mode";

						if (iconValue > 0)  //if conf have audio participants, show the icon after gathering mode ends.
							pBridgePartyVideoOut->SetAudioParticipantsIconActive(YES);

						continue;
					}
				}

				BOOL bUseSharedMemForIndicationIcon = TRUE;   //send INDICATION_ICONS_CHANGE with shared memory
				pBridgePartyVideoOut->UpdateIndicationIcons(bUseSharedMemForIndicationIcon);

				AddIndicationIconIdToVector(pPartyCntl->GetPartyRsrcID()); //currently we use party resource ID for indication icon

				continue;
			}
		}

		DBGPASSERT(1);
		break;
	}

	//send MULTIPLE_PARTIES_INDICATION_ICON_CHANGE_REQ to mpl api
	SendIndicationIconChangeToMultipleParties();
}
// ------------------------------------------------------------------------------------------
BOOL CVideoBridge::IsXCodeConf()
{
  return (m_pConf->GetCommConf())->GetContentMultiResolutionEnabled();
}
// ------------------------------------------------------------------------------------------
BOOL CVideoBridge::IsASSIPConf()
{
  return ((m_pConf->GetCommConf())->GetIsAsSipContent() && IsXCodeConf());
}
// ------------------------------------------------------------------------------------------
BOOL CVideoBridge::IsLegacyBridge()
{
  return (m_pConf->GetCommConf())->IsLegacyShowContentAsVideo();
}

//--------------------------------------------------------------------------
CTaskApp* CVideoBridge::GetPartyTaskApp(PartyRsrcID partyId)
{
	CBridgePartyCntl* pPartyCtrl = GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN_VALUE(!pPartyCtrl, "PartyId:" << partyId, NULL);

	return pPartyCtrl->GetPartyTaskApp();
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::UpdateConfLayout(CLayout* pNewLayout)
{
	*m_ConfLayout = *pNewLayout;
	m_layoutType  = m_ConfLayout->GetLayoutType();
	TRACEINTO << "ConfName:" << m_pConfName << ", LayoutType:" << LayoutTypeAsString[m_layoutType];
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoBridge::IsLayoutChange(CLayout* pNewLayout, CLayout* pOldLayout)
{
  BYTE isNotationChanged = FALSE;

  if (m_pLastSpeakerNotationParty != GetLastActiveAudioSpeakerRequest())
  {
    isNotationChanged           = TRUE;
    m_pLastSpeakerNotationParty = GetLastActiveAudioSpeakerRequest();
  }

  if (*pNewLayout != *pOldLayout)
    return LAYOUT_CHANGED;
  else if (isNotationChanged)
    return LAYOUT_CHANGED_ATTRIBUTES;
  else if (pNewLayout->GetAutoScanCell() != pOldLayout->GetAutoScanCell()) // the layout is with the same images but auto_scan settings has changed
    return LAYOUT_CHANGED_AUTOSCAN;
  else
    return LAYOUT_NOT_CHANGED;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::BuildLayout(CLayout& rResultLayout, LayoutType newLayoutType, BOOL isCop)
{
  PTRACE(eLevelInfoNormal, "CVideoBridge::BuildLayout");

  // set layout type
  rResultLayout.SetLayoutType(newLayoutType);

  // Remove old forces and blanks
  RemoveOldConfForcesAndBlankes(rResultLayout);
  // forces and blanks
  SetConfForces(rResultLayout, newLayoutType);

  // Fill Layout
  BYTE remove_lecturer = NO;
  if (isCop)
  {
	  WORD isLectureMode   = m_pCopLectureModeCntl->IsLectureModeActive();
	  if (isLectureMode)
	  {
		remove_lecturer = YES;
	  }
  }

  FillLayout(rResultLayout, remove_lecturer, isCop);


  if (isCop)
  {
	  // update decoders connection id and entity id in images
	  UpdateDecodersParamsInImages(rResultLayout);
  }

  //***** NOTE: No need to calculate layout indications for conference layout in this stage (when conf layout is used only for comparison).

}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::RemoveOldConfForcesAndBlankes(CLayout& rResultLayout, BOOL isCop) const
{
  WORD numbSubImg = rResultLayout.GetNumberOfSubImages();
  for (WORD i = 0; i < numbSubImg; i++)
  {
	// Romem - klocwork
	  if((rResultLayout)[i] ==  NULL)
		  continue;
    if ((rResultLayout)[i]->isForcedInConfLevel())
    {
      (rResultLayout)[i]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
      (rResultLayout)[i]->RemovePartyForceName();
    }
    else
    {
      if (((rResultLayout)[i]->GetVideoActivities() == BLANK_CONF_Activ)
    	  || (!isCop && (rResultLayout)[i]->GetVideoActivities() == AUTO_SCAN_Active))
      {
        (rResultLayout)[i]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::SetConfForces(CLayout& rResultLayout, LayoutType newLayoutType, BOOL isCop) const
{
  // ConfForcesLayout should take from forces reservation
  const CLayout* pConfResLayout = m_pReservation[newLayoutType]; // ;GetReservationLayout();

  const WORD numSubImg          = rResultLayout.GetNumberOfSubImages();
  PASSERT_AND_RETURN(numSubImg != pConfResLayout->GetNumberOfSubImages());
  const char* partyName         = NIL(char);
  WORD autoScanCell 			= pConfResLayout->GetAutoScanCell();

  for (WORD i = 0; i < numSubImg; i++)
  {
	// Romem klocwork
	if(!(*pConfResLayout)[i] || !(rResultLayout)[i])
       continue;
    if ((*pConfResLayout)[i]->isForcedInConfLevel() || (*pConfResLayout)[i]->isBlanked() || (isCop && (*pConfResLayout)[i]->IsAutoScan()))
    {
      // Here we know that conf is forced here and copy the force into
      // layout intended for building.
      if ((rResultLayout)[i]->CanBeSetTo((*pConfResLayout)[i]->GetRequestPriority(), (*pConfResLayout)[i]->GetVideoActivities()))
      {
        partyName = (*pConfResLayout)[i]->GetPartyForce();
        // If specific cell was forced to itself , make the cell voice activated
        (rResultLayout)[i]->SetPartyForceName(partyName);
        (rResultLayout)[i]->SetForceAttributes((*pConfResLayout)[i]->GetRequestPriority(), (*pConfResLayout)[i]->GetVideoActivities());
      }
    }
    else if (!isCop && (autoScanCell == i))
    {
      if (rResultLayout[i]->CanBeSetTo(OPERATOR_Prior, AUTO_CONF_Activ))
      {
    	  rResultLayout[i]->SetForceAttributes(AUTO_Prior, AUTO_SCAN_Active);
    	  rResultLayout[i]->RemovePartyForceName();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::FillLayout(CLayout& rResultLayout, BYTE remove_lecturer, BOOL isCop)
{
  const WORD numCellsInLayout = rResultLayout.GetNumberOfSubImages();
  PASSERT_AND_RETURN(numCellsInLayout == 0);

  // stage 1
  // ~~ Create copy of current layout before changing it
  CLayout previouseLayout(*m_ConfLayout /**pCurrentLayout*/);

  // stage 2
  // ~~ We always Fill something that is clean
  rResultLayout.ClearAllImageSources();

  const WORD numConnectedImages = GetPartyImageVectorSize();
  if (numConnectedImages == 0) // may be the case if no decoder is connected in conf
    return;

  // stage 3
  // ~~ Images that are not muted are inserted to Vector imagesReadyForSetting
  CPartyImageVector imagesReadyForSetting;
  CreateVectorOfImagesReadyForSetting(imagesReadyForSetting);

  if (isCop && remove_lecturer)
  {
    PTRACE(eLevelInfoNormal, "CVideoBridgeCOP::FillLayout removing lecturer image from images vector");
    RemoveLecturerImageFromImagesVector(imagesReadyForSetting);
    RemoveLecturerForce(rResultLayout);
  }

  // stage 6
  // ~~~ initiate numCellsLeft (= cells in layout that are not blanked)
  WORD numCellsLeft = 0;
  for (DWORD i = 0; i < numCellsInLayout; i++)
  {
	// klocwork - Romem
	  if((rResultLayout)[i] == NULL)
		  continue;
    if (!((rResultLayout)[i]->isBlanked()))
    {
      numCellsLeft++;
    }
  }

  if (numCellsLeft == 0) // all cells in layout are blanked
    return;

  // stage 7
  // ~~~ Set forced parties
  SetForcedPartiesInLayout(rResultLayout, imagesReadyForSetting, numCellsLeft);

  // stage 8???
  // ~~~ AutoScan Remove from imagesReadyForSetting images that will be seen in the auto_scan cell
  if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
  {
    if (!isCop || (isCop && (numCellsLeft < imagesReadyForSetting.size())))
    {
      if (rResultLayout.IsAutoScanSet())
      {
        if (CPObject::IsValidPObjectPtr(m_pAutoScanParams))
        {
          BYTE numAutoScanImages =  m_pAutoScanParams->FillAutoScanImages(imagesReadyForSetting, numCellsLeft);
          if (numAutoScanImages)
            /*((CVideoBridgeCOP*)this)->*/FillAutoScanImageInLayout(rResultLayout, numAutoScanImages-1);
        }
      }
    }
  }

  // stage 9
  // ~~~ Remove from imagesReadyForSetting images that can not enter layout because speaker order
  if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
  {
    if (numCellsLeft < imagesReadyForSetting.size())
    {
      imagesReadyForSetting.erase((imagesReadyForSetting.begin()+numCellsLeft), imagesReadyForSetting.end());
    }
  }

  // stage 10
  // ~~~ In Layout with Speaker Picture try to set speaker in to first cell
  if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
  {
    if (rResultLayout.GetLayoutType() == CP_LAYOUT_2P8 || rResultLayout.GetLayoutType() == CP_LAYOUT_2TOP_P8)
    {
      SetSpeakerImageToBigCellInLayout2Plus8IfNeeded(imagesReadyForSetting, rResultLayout, &previouseLayout, numCellsLeft);
    }
    else if (IsLayoutWithSpeakerPicture(/*previouseLayout*/ rResultLayout.GetLayoutType()))
    {
      SetSpeakerImageToBigCellInLayout(rResultLayout, imagesReadyForSetting, numCellsLeft);
    }
  }

  // stage 11
  // ~~~ Try to set the remaining images in their previous cell
  if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
  {
    SetImagesToPreviouseSeenCellsInLayout(rResultLayout, imagesReadyForSetting, &previouseLayout, numCellsLeft);
  }

  // stage 12
  // ~~~ Put the remaining images in any unused cell
  if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
  {
    SetImagesToAnyUnusedCell(rResultLayout, imagesReadyForSetting);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::CreateVectorOfImagesReadyForSetting(CPartyImageVector& rImagesReadyForSetting) const
{
	// Run over all connected Images to video bridge images and those not muted insert to Vector
	WORD partyImageVectorSize = GetPartyImageVectorSize();
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	for (WORD i = 0; i < partyImageVectorSize; ++i)
	{
		DWORD partyRscId = GetPartyImageIdByPosition(i);
		if (partyRscId)
		{
			CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
			PASSERTSTREAM_AND_RETURN(!pImage, "CVideoBridgeCOP::CreateVectorOfImagesReadyForSetting - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

			if (!pImage->isMuted())
				rImagesReadyForSetting.push_back(partyRscId);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::UpdateDecodersParamsInImages(CLayout& rResultLayout) const
{
	// to go over all the sub images and if there is an image
	// to set the m_source_connection_id and m_source_partyRsrc_id of the decoder according to the global topology table
	LayoutType               layoutType             = rResultLayout.GetLayoutType();
	ECopEncoderMaxResolution encoder_max_resolution = GetCopEncoderMaxResolution();
	CPartyImageLookupTable*  pPartyImageLookupTable = GetPartyImageLookupTable();
	WORD                     numSubImg              = rResultLayout.GetNumberOfSubImages();

	for (WORD i = 0; i < numSubImg; i++)
	{
		CVidSubImage* pVidSubImage = rResultLayout[i];
		if (!pVidSubImage)
			continue;

		PartyRsrcID partyId = pVidSubImage->GetImageId();
		if (!partyId)
			continue;

		CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyId);
		PASSERTSTREAM(!pImage, "PartyId:" << partyId);

		if (pImage)
		{
			DWORD dspSmartSwitchConnectionId = DUMMY_CONNECTION_ID;
			DWORD dspSmartSwitchEntityId     = DUMMY_PARTY_ID;

			CCopLayoutResolutionTable copLayoutResolutionTable;
			ECopDecoderResolution     decoder_resolution;
			WORD                      decoderConnectionIdIndex;

			WORD ret_value = copLayoutResolutionTable.GetCellResolutionDef(encoder_max_resolution, layoutType, i, decoder_resolution, decoderConnectionIdIndex);
			PASSERTSTREAM(ret_value == (WORD)-1, "GetCellResolutionDef failed, i:" << i);
			if (ret_value == (WORD)-1)
			{
				PASSERTSTREAM(true, "PartyId:" << partyId << ", Index:" << i);
			}
			else
			{
				DWORD decoderConnectionId = m_pCopRsrcs->dynamicDecoders[decoderConnectionIdIndex].connectionId;
				DWORD decoderPartyId      = m_pCopRsrcs->dynamicDecoders[decoderConnectionIdIndex].rsrcEntityId;
				DWORD artPartyId          = pImage->GetArtPartyId();
				BOOL  isImageUpdated      = FALSE;

				CVideoBridgePartyCntlCOP* pCurrParty = (CVideoBridgePartyCntlCOP*)m_pPartyList->Find(artPartyId);
				if (pCurrParty)
				{
						dspSmartSwitchConnectionId = pCurrParty->GetDecoderConnectionIdInImage();
						dspSmartSwitchEntityId     = pCurrParty->GetDecoderPartyIdInImage();

						pCurrParty->SetDecoderConnectionIdInImage(decoderConnectionId);
						pCurrParty->SetDecoderPartyIdInImage(decoderPartyId);

						// for the MPMX DSP smart switch solution
						pCurrParty->SetDspSmartSwitchConnectionId(dspSmartSwitchConnectionId);
						pCurrParty->SetDspSmartSwitchEntityId(dspSmartSwitchEntityId);

						isImageUpdated = TRUE;
					}

				PASSERTSTREAM(!isImageUpdated, "CVideoBridgeCOP::UpdateDecodersParamsInImages - Failed, artPartyId:" << artPartyId << ", Index:" << i);

				TRACEINTO
					<< "SubImageIndex:"                << i
					<< ", decoderConnectionId:"        << pImage->GetConnectionId()
					<< ", decoderPartyId:"             << pImage->GetPartyRsrcId()
					<< ", artPartyId:"                 << artPartyId
					<< ", dspSmartSwitchConnectionId:" << dspSmartSwitchConnectionId
					<< ", dspSmartSwitchEntityId:"     << dspSmartSwitchEntityId;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
ECopEncoderMaxResolution CVideoBridge::GetCopEncoderMaxResolution() const
{
  ECopEncoderMaxResolution copEncoderMaxResolution = COP_encoder_max_resolution_HD1080p25;
  if (m_VideoConfType == eVideoConfTypeCopHD72050fps)
    copEncoderMaxResolution = COP_encoder_max_resolution_HD720p50;

  return copEncoderMaxResolution;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::RemoveLecturerImageFromImagesVector(CPartyImageVector& rImagesReadyForSetting) const
{
	if (m_pCopLectureModeCntl->IsLectureModeActive())
	{
		CVideoBridgePartyCntl* pLecturer = GetLecturer();
		if (pLecturer)
		{
			CPartyImageVector::iterator _ii = std::find(rImagesReadyForSetting.begin(), rImagesReadyForSetting.end(), pLecturer->GetPartyRsrcID());
			if (_ii != rImagesReadyForSetting.end())
				rImagesReadyForSetting.erase(_ii);
		}
	}
}
////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntl* CVideoBridge::GetLecturer(CVideoBridgeLectureModeParams& rVideoBridgeLectureModeParams) const
{
  // if removeLecturer == 1 this mean that the conference goes to mode regular and LM is over
	// if removeLecturer == 0 this mean that the conference still in LM but temporarily goes to mode transcoding
	PASSERT_AND_RETURN_VALUE(!m_pLectureModeParams, NULL);

  const char* lecturerName = rVideoBridgeLectureModeParams.GetLecturerName();
  if (NULL == lecturerName)
    return NULL;

	if (0 == strlen(lecturerName))
		return NULL;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl && !strcmp(lecturerName, pPartyCntl->GetName()))
			return pPartyCntl;
      }

	TRACEINTO << "LecturerName:" << lecturerName << " - Failed, lecturer not found in party list";
	return NULL;
  }

////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntl* CVideoBridge::GetLecturer(BOOL bNewLecturer) const
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if (IsLecturer(pPartyCntl->GetName(), bNewLecturer))
				return pPartyCntl;
  }
	}
	TRACEINTO << "Failed, lecturer not found in party list";
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridge::IsLecturer(CVideoBridgePartyCntl* pVideoBridgePartyCntl, BOOL bNewLecturer) const
{
  const char* partyName = (pVideoBridgePartyCntl) ? pVideoBridgePartyCntl->GetName() : NULL;
  return IsLecturer(partyName, bNewLecturer);
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridge::IsLecturer(const char* partyName, BOOL bNewLecturer) const
{
  CVideoBridgeLectureModeParams* pLectureModeParams = bNewLecturer ? m_pCopLectureModeCntl->GetCurrentParams() : m_pLectureModeParams;
  if (NULL == pLectureModeParams)
  {
    PTRACE2(eLevelError, "CVideoBridge::IsLecturer - Invalid lecture mode params, ConfName=", m_pConfName);
    return NO;
  }

  const char* lecturerName = pLectureModeParams->GetLecturerName();

  if (partyName == NULL || lecturerName == NULL)
    return NO;

  DWORD len1 = strlen(lecturerName);
  DWORD len2 = strlen(partyName);
  if (len1 == len2)
    if (!strncmp(lecturerName, partyName, len1))
    {
      PTRACE2(eLevelError, "CVideoBridge::IsLecturer - LecturerName=", lecturerName);
      return YES;
    }

  return NO;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::RemoveLecturerForce(CLayout& rResultLayout) const
{
  const char* partyName = NULL;
  partyName = GetLecturerName();
  if (partyName == NULL)
    return;

  WORD numbSubImg = rResultLayout.GetNumberOfSubImages();
  for (WORD i = 0; i < numbSubImg; i++)
  {

	if((rResultLayout)[i] == NULL)
		continue;
    if ((rResultLayout)[i]->isForcedInConfLevel())
    {
      const char* forcedName = NULL;
      forcedName = ((rResultLayout)[i]->GetPartyForce());
      if (forcedName == NULL)
      {
        continue;
      }

      WORD len1 = strlen(partyName);
      WORD len2 = strlen(forcedName);
      if (len1 == len2 && !strncmp(partyName, forcedName, len1))
      {
        CSmallString sstr;
        sstr << "removing force to lecturer image: cell_index = " << i << " , lecturer_name = " << partyName;
        PTRACE2(eLevelInfoNormal, "CVideoBridgeCOP::RemoveLecturerForce ", sstr.GetString());
        (rResultLayout)[i]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
        (rResultLayout)[i]->RemovePartyForceName();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::SetForcedPartiesInLayout(CLayout& rResultLayout, CPartyImageVector& rImagesReadyForSetting, WORD& rNumCellsLeftToFill) const
{
	WORD numCellsInLayout = rResultLayout.GetNumberOfSubImages();
	for (WORD i = 0; i < numCellsInLayout; ++i)
	{
		if ((rResultLayout)[i] == NULL)
			continue;

		if ((rResultLayout)[i]->isBlanked())
		{
			TRACEINTO << "CellIndex:" << i << " - Cell is blanked";
			continue;
		}

		if ((rResultLayout)[i]->isForced())
		{
			CBridgePartyCntl* pPartyCntl = const_cast<CVideoBridge*>(this)->GetPartyCntl(rResultLayout[i]->GetPartyForce());
			if (IsValidPObjectPtr(pPartyCntl))
			{
				CPartyImageVector::iterator _ii = std::find(rImagesReadyForSetting.begin(), rImagesReadyForSetting.end(), pPartyCntl->GetPartyRsrcID());
				if (_ii != rImagesReadyForSetting.end())
				{
					rResultLayout[i]->SetImageId(*_ii);
					rImagesReadyForSetting.erase(_ii);
					rNumCellsLeftToFill--;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridge::FillAutoScanImageInLayout(CLayout& rResultLayout, BYTE needToStartTimer)
{
	BYTE ans = TRUE;

	std::ostringstream msg;
	msg << "CVideoBridgeCOP::FillAutoScanImageInLayout - ";

	WORD auto_scan_cell = rResultLayout.GetAutoScanCell();
	msg << "auto_scan_cell:" << auto_scan_cell;
	if (auto_scan_cell != AUTO && rResultLayout[auto_scan_cell])
	{
		DWORD partyRscId = m_pAutoScanParams->GetNextPartyImageId();

		if (partyRscId)
		{
			msg << ", PartyId:" << partyRscId;
			rResultLayout[auto_scan_cell]->SetImageId(partyRscId);

			if (needToStartTimer)
			{
				msg << ", start AUTO_SCAN_TIMER timer for " << m_pAutoScanParams->GetTimerInterval() << " seconds";
				StartTimer(AUTO_SCAN_TIMER, m_pAutoScanParams->GetTimerInterval() * SECOND);
			}
			else
			{
				msg << ", no need to start timer, there is only one image to scan";
			}
		}
		else
		{
			msg << ", no next valid image, so stop the timer";
			ans = FALSE;
		}
	}
	TRACEINTO << msg.str().c_str();
	return ans;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::SetSpeakerImageToBigCellInLayout2Plus8IfNeeded(CPartyImageVector& rImagesReadyForSetting, CLayout& rResultLayout, CLayout* pPreviouselySeenLayout, WORD& rNumCellsLeftToFill) const
{
	std::ostringstream msg;
	msg << "CVideoBridge::SetSpeakerImageToBigCellInLayout2Plus8IfNeeded:";

	WORD newSpeakerPlaceInCurrLayout = 0;
	WORD oldSpeakerPlaceInCurrLayout = 0;

	WORD oldSpeakerPlaceInPrevLayout = 0;
	WORD newSpeakerPlaceInPrevLayout = 0;

	CPartyImageVector::iterator _iiNewSpeaker = rImagesReadyForSetting.begin();
	if (_iiNewSpeaker != rImagesReadyForSetting.end())
	{
		msg << "\n  new_speaker_id    :" << *_iiNewSpeaker;
		newSpeakerPlaceInPrevLayout = pPreviouselySeenLayout->FindImagePlaceInLayout(*_iiNewSpeaker);
	}
	else
	{
		PASSERT_AND_RETURN(1);
	}

	// Old Speaker place in previous layout can be in the 2 big cells in the middle or not in layout (audio only)
	CPartyImageVector::iterator _iiOldSpeaker = _iiNewSpeaker+1;
	if (_iiOldSpeaker != rImagesReadyForSetting.end())
	{
		msg << "\n  old_speaker_id    :" << *_iiOldSpeaker;
		oldSpeakerPlaceInPrevLayout = pPreviouselySeenLayout->FindImagePlaceInLayout(*_iiOldSpeaker);
	}

	msg << "\n  new_speaker place :" << newSpeakerPlaceInPrevLayout << " (in previous layout)";
	msg << "\n  old_speaker_place :" << oldSpeakerPlaceInPrevLayout << " (in previous layout)";

	// put the new speaker next to the old speaker (avoid change layout)
	// change layout if the party is in a selfview therefore the only party that is connected to conference
	if (oldSpeakerPlaceInPrevLayout == 0 && _iiOldSpeaker != rImagesReadyForSetting.end())
		newSpeakerPlaceInCurrLayout = 1;

	msg << "\n  new_speaker_place :" << newSpeakerPlaceInCurrLayout;

	if (!rResultLayout[newSpeakerPlaceInCurrLayout])
	{
		msg << "\n  The cell " << newSpeakerPlaceInCurrLayout << " is empty, error???";
		TRACEINTO << msg.str().c_str();
		return;
	}

	if (newSpeakerPlaceInPrevLayout == 0 || newSpeakerPlaceInPrevLayout == 1)
	{
		oldSpeakerPlaceInCurrLayout = newSpeakerPlaceInPrevLayout ^ 1;
		msg << "\n  old_speaker_place :" << oldSpeakerPlaceInCurrLayout;

		CVidSubImage* pVidSubImageOldSpeaker = (*pPreviouselySeenLayout)[oldSpeakerPlaceInCurrLayout];
		if (pVidSubImageOldSpeaker)
		{
			DWORD partyRscId = pVidSubImageOldSpeaker->GetImageId();
			if (partyRscId)
			{
				// check if image that was in big cell in old layout is still available
				CPartyImageVector::iterator _ii = std::find(rImagesReadyForSetting.begin(), rImagesReadyForSetting.end(), partyRscId);

				// image that was in big cell in old layout is not available (disconnected / forced ...)
				// try to set another image at the big cell
				if (_ii == rImagesReadyForSetting.end())
				{
					msg << "\n  new speaker is already in big cell, but old speaker is not available (disconnected or muted)";
					if (_iiOldSpeaker != rImagesReadyForSetting.end())
					{
						if (rResultLayout[oldSpeakerPlaceInCurrLayout] && !rResultLayout[oldSpeakerPlaceInCurrLayout]->isBlanked() && rResultLayout[oldSpeakerPlaceInCurrLayout]->noImgSet())
						{
							msg << "\n  --> Set old_speaker_id:" << *_iiOldSpeaker << " to cell: " << oldSpeakerPlaceInCurrLayout;
							rResultLayout[oldSpeakerPlaceInCurrLayout]->SetImageId(*_iiOldSpeaker);
							rImagesReadyForSetting.erase(_iiOldSpeaker);
							rNumCellsLeftToFill--;
						}
					}
				}
				else
				{
					msg << "\n  Layout is 2+8 and new speaker is already in big cell, so do nothing";
				}
			}
		}
	}
	else if ((rResultLayout[newSpeakerPlaceInCurrLayout]) && !rResultLayout[newSpeakerPlaceInCurrLayout]->isBlanked() && rResultLayout[newSpeakerPlaceInCurrLayout]->noImgSet())
	{
		// first place in rImagesReadyForSetting is either the speaker or the last speaker that has a valid image
		msg << "\n  --> Set new_speaker_id:" << *_iiNewSpeaker << " to cell: " << newSpeakerPlaceInCurrLayout;
		rResultLayout[newSpeakerPlaceInCurrLayout]->SetImageId(*_iiNewSpeaker);
		rImagesReadyForSetting.erase(_iiNewSpeaker);
		rNumCellsLeftToFill--;
	}
	else
	{
		msg << "\n  The cell " << newSpeakerPlaceInCurrLayout << " is blanked or forced, so try to set in cell " << (newSpeakerPlaceInCurrLayout ^ 1);
		newSpeakerPlaceInCurrLayout = newSpeakerPlaceInCurrLayout ^ 1;

		if (rResultLayout[newSpeakerPlaceInCurrLayout] && !rResultLayout[newSpeakerPlaceInCurrLayout]->isBlanked() && rResultLayout[newSpeakerPlaceInCurrLayout]->noImgSet())
		{
			msg << "\n  --> Set new_speaker_id:" << *_iiNewSpeaker << " to cell: " << newSpeakerPlaceInCurrLayout;
			rResultLayout[newSpeakerPlaceInCurrLayout]->SetImageId(*_iiNewSpeaker);
			rImagesReadyForSetting.erase(_iiNewSpeaker);
			rNumCellsLeftToFill--;
		}
		else
		{
			msg << "\n  The cell " << newSpeakerPlaceInCurrLayout << " is blanked or forced too, so do not set speaker image";
		}
	}
	TRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridge::IsLayoutWithSpeakerPicture(const LayoutType layoutType) const
{
  return isLayoutWithSpeakerPicture(layoutType);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::SetSpeakerImageToBigCellInLayout(CLayout& rResultLayout, CPartyImageVector& rImagesReadyForSetting, WORD& rNumCellsLeftToFill) const
{
	// first place in rImagesReadyForSetting is either the speaker or the last speaker that has a valid image
	if (rResultLayout[0] && !rResultLayout[0]->isBlanked() && rResultLayout[0]->noImgSet())
	{
		CPartyImageVector::iterator _ii = rImagesReadyForSetting.begin();
		if (_ii != rImagesReadyForSetting.end())
		{
			rResultLayout[0]->SetImageId(*_ii);
			rImagesReadyForSetting.erase(_ii);
			rNumCellsLeftToFill--;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::SetImagesToPreviouseSeenCellsInLayout(CLayout& rResultLayout, CPartyImageVector& rImagesReadyForSetting, CLayout* pPreviouselySeenLayout, WORD& rNumCellsLeftToFill) const
{
	CPartyImageVector::iterator _ii =  rImagesReadyForSetting.begin();
	while (_ii != rImagesReadyForSetting.end())
	{
		BYTE whereWasSeen = pPreviouselySeenLayout->FindImagePlaceInLayout(*_ii);
		if (whereWasSeen != AUTO)
		{
			if (rResultLayout[whereWasSeen] && rResultLayout[whereWasSeen]->noImgSet())
			{
				rResultLayout[whereWasSeen]->SetImageId(*_ii);
				_ii = rImagesReadyForSetting.erase(_ii);
				rNumCellsLeftToFill--;
				continue;
			}
		}
		_ii++;
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::SetImagesToAnyUnusedCell(CLayout& rResultLayout, CPartyImageVector& rImagesReadyForSetting) const
{
	CPartyImageVector::iterator _ii =  rImagesReadyForSetting.begin();
	while (_ii != rImagesReadyForSetting.end())
	{
		WORD unUsedCell = rResultLayout.FindFirstUnUsedCell();
		if (unUsedCell != AUTO)
		{
			if (rResultLayout[unUsedCell])
			{
				rResultLayout[unUsedCell]->SetImageId(*_ii);
				_ii = rImagesReadyForSetting.erase(_ii);
			}
			else
			{
				_ii++;
			}
		}
		else
		{
			break;
		}
	}
}


////////////////////////////////////////////////////////////////////////////
//Change Layout Improvement - Layout Shared Memory (CL-SM)
// Send Change Layout to multiple parties

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::AddChangedLayoutIdToVector(DWORD layoutId)
{
	m_changedLayoutIdsList->push_back(layoutId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::EmptyChangedLayoutIdsVector()
{
	m_changedLayoutIdsList->erase(m_changedLayoutIdsList->begin(), m_changedLayoutIdsList->end());
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::VerifyIdVectorIsEmpty()
{
	if (m_changedLayoutIdsList->size() != 0)
	{
		EmptyChangedLayoutIdsVector();
	    PASSERTMSG(1, "CVideoBridge::VerifyIdVectorIsEmpty - Error! - m_changedLayoutIdsList was not empty before loop");
	}
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridge::IsTipCompatibilityMode() const
{
	const CCommConf* pCommConf = m_pConf->GetCommConf();
	if (IsValidPObjectPtr(pCommConf))
	{
		if (pCommConf->GetIsTipCompatible() != eTipCompatibleNone)
			return TRUE;
	}

	return FALSE;
}

void CVideoBridge::AddIndicationIconIdToVector(DWORD indicationIconId)
{
	m_indicationIconIdsList->push_back(indicationIconId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::EmptyIndicationIconIdsVector()
{
	m_indicationIconIdsList->erase(m_indicationIconIdsList->begin(), m_indicationIconIdsList->end());
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::VerifyIndicationIconIdVectorIsEmpty()
{
	if (m_indicationIconIdsList->size() != 0)
	{
		EmptyIndicationIconIdsVector();
	    PASSERTMSG(1, "CVideoBridge::VerifyIndicationIconIdVectorIsEmpty - Error! - m_indicationIconIdsList was not empty before loop");
	}
}



////////////////////////////////////////////////////////////////////////////
void CVideoBridge::SendIndicationIconChangeToMultipleParties()
{
	// sending indication icon change to  multiple parties in blocks of 20 participants in one message (with a break between them)
	///TRACEINTO << "**CL-SM: CVideoBridge::SendIndicationIconChangeToMultipleParties";		///DEBUG***
	DWORD numOfIndicationIconIds = m_indicationIconIdsList->size();

	if (numOfIndicationIconIds > 0)
	{
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		DWORD SleepTime = 0;
		if (pSysConfig)
			pSysConfig->GetDWORDDataByKey("SLEEP_VALUE", SleepTime);

		//xifwang, the sleeptime defaut value is 10s, then what's the value for this feature????
		//xifwang, this will also handle the recording, then TIP support the recording or not?????SInce here have some
		//special handle for TIP.

		std::ostringstream str;

		for (IndicationIconIdsList::iterator it = m_indicationIconIdsList->begin(); it != m_indicationIconIdsList->end(); ++it)
			str << *it << " ";

		TRACEINTO << "Indication Icon change vector size: " << numOfIndicationIconIds << ", Indication Icon ids': " << str.str().c_str();

		DWORD sizeOfBlockAfterModule;
		DWORD multipleIndicationIconBlockSize = numOfIndicationIconIds; //We won't send them with  bundles, now directly send them all.
		WORD j = 0;
		for (j = 0; j < numOfIndicationIconIds / multipleIndicationIconBlockSize; j++)
		{
			SendBlockOfIndicationIconIds(multipleIndicationIconBlockSize, j);

			//PTRACE2INT(eLevelInfoNormal, "CVideoBridge::SendIndicationIconChangeToMultipleParties - go to sleep for ", SleepTime);
			//SystemSleep(SleepTime);
		}

		// send the last block with the remaining Indication Icon IDs
		sizeOfBlockAfterModule = numOfIndicationIconIds % multipleIndicationIconBlockSize;
		if (sizeOfBlockAfterModule > 0)
			SendBlockOfIndicationIconIds(sizeOfBlockAfterModule, j);

		EmptyIndicationIconIdsVector();
	}
}


////////////////////////////////////////////////////////////////////////////
void CVideoBridge::SendBlockOfIndicationIconIds(DWORD numberOfIndicationIconIdsForMultipleSend, WORD IndicationIconIdsBlockNumber)
{
	CSegment* pMsg = new CSegment;
	*pMsg << numberOfIndicationIconIdsForMultipleSend;

	//We won't send them with  bundles, now directly send them all, so no need to offset.
	DWORD arrBaseIndex = IndicationIconIdsBlockNumber; //* MULTIPLE_CHANGE_LAYOUT_BLOCK_SIZE;

	for (WORD i = 0; i < numberOfIndicationIconIdsForMultipleSend; i++)
		*pMsg << (DWORD)m_indicationIconIdsList->at(arrBaseIndex + i);

	SendMsgToMPL(MULTIPLE_PARTIES_INDICATION_ICON_CHANGE_REQ, pMsg);
	POBJDELETE(pMsg);

}


// ------------------------------------------------------------------------------------------
void CVideoBridge::SendChangeLayoutToMultipleParties()
{
	// sending change layout to multiple parties in blocks of 20 participants in one message (with a break between them)
	// /TRACEINTO << "**CL-SM: CVideoBridge::SendChangeLayoutToMultipleParties";		///DEBUG***
	DWORD numOfLayoutIds = m_changedLayoutIdsList->size();

	if (numOfLayoutIds > 0)
	{
		// VNGR-9344 - When switching in conf with more then 80 parties last parties does not change layout
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		DWORD SleepTime  = 0;
		if (pSysConfig)
			pSysConfig->GetDWORDDataByKey("SLEEP_VALUE", SleepTime);

		std::ostringstream str;

		for (ChangedLayoutIdsList::iterator it = m_changedLayoutIdsList->begin(); it != m_changedLayoutIdsList->end(); ++it)
			str << *it << " ";

		TRACEINTO << "Changed layout vector size:" << numOfLayoutIds << ", Layout ids':" << str.str().c_str();

		CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

		// If managed internally therefore not by MLA
		if (pCommConf->GetManageTelepresenceLayoutInternaly() == TRUE && pCommConf->GetIsTelePresenceMode() == TRUE)
		{
			ChangedLayoutIdsOrderedByRoomId* orderedMapByRoomId = BuildMapOfChangedLayoutIdListOrderedByRoomId();
			PASSERT_AND_RETURN(!orderedMapByRoomId);

			DWORD totalSentLayoutsToMpl = 0;
			DWORD numOfChangedLayoutsToSendInCurrentRound = 0;
			DWORD totalRoomIdsToSendInCurrentRound = 0;

			ChangedLayoutIdsOrderedByRoomId::iterator tmpIter = orderedMapByRoomId->begin();

			while (totalSentLayoutsToMpl < numOfLayoutIds)
			{
				// Adding room id layouts till we reach the last available cells
				while (tmpIter != orderedMapByRoomId->end() && ((numOfChangedLayoutsToSendInCurrentRound + ((*tmpIter).second)->size()) < MULTIPLE_CHANGE_LAYOUT_BLOCK_SIZE))
				{
					numOfChangedLayoutsToSendInCurrentRound += (tmpIter->second)->size();
					++tmpIter;
					++totalRoomIdsToSendInCurrentRound;
				}

				// Adding the room id layouts for the last available cells in the block because no slicing for telepresence EP
				if (tmpIter != orderedMapByRoomId->end())
				{
					++totalRoomIdsToSendInCurrentRound;
					numOfChangedLayoutsToSendInCurrentRound += (tmpIter->second)->size();
				}

				CSegment* pMsg = new CSegment;
				*pMsg << numOfChangedLayoutsToSendInCurrentRound;

				std::ostringstream str1;
				str1 << "(Room Id,Layout Id) : ";

				for (WORD i = 0; i < totalRoomIdsToSendInCurrentRound; i++)
				{
					ChangedLayoutIdsOrderedByRoomId::iterator BeginIter = orderedMapByRoomId->begin();
					ChangedLayoutIdsOrderedByRoomIdList* List = BeginIter->second;

					str1 << "(" << BeginIter->first << ":";

					while (!List->empty())
					{
						ChangedLayoutIdsOrderedByRoomIdList::iterator ListIter = List->begin();
						str1 << *ListIter << (List->size() > 1 ? "," : "");
						*pMsg << *ListIter;
						List->erase(ListIter);
					}

					delete List;
					orderedMapByRoomId->erase(BeginIter);
					str1 << ") ";
				}

				TRACEINTO << str1.str().c_str();
				SendMsgToMPL(MULTIPLE_PARTIES_CHANGE_LAYOUT_REQ, pMsg);
				POBJDELETE(pMsg);

				totalSentLayoutsToMpl += numOfChangedLayoutsToSendInCurrentRound;

				// Prepare variables for the next round
				numOfChangedLayoutsToSendInCurrentRound = 0;
				totalRoomIdsToSendInCurrentRound  = 0;
				tmpIter = orderedMapByRoomId->begin();

				PTRACE2INT(eLevelInfoNormal, "CVideoBridge::SendChangeLayoutToMultipleParties Tele - go to sleep for ", SleepTime);
				SystemSleep(SleepTime);
			}

			delete orderedMapByRoomId;
		}
		else
		{
			DWORD sizeOfBlockAfterModule;
			WORD  j = 0;
			for (j = 0; j < numOfLayoutIds / MULTIPLE_CHANGE_LAYOUT_BLOCK_SIZE; j++)
			{
				SendBlockOfLayoutIds(MULTIPLE_CHANGE_LAYOUT_BLOCK_SIZE, j);

				PTRACE2INT(eLevelInfoNormal, "CVideoBridge::SendChangeLayoutToMultipleParties - go to sleep for ", SleepTime);
				SystemSleep(SleepTime);
			}

			// send the last block with the remaining layout IDs
			sizeOfBlockAfterModule = numOfLayoutIds % MULTIPLE_CHANGE_LAYOUT_BLOCK_SIZE;
			if (sizeOfBlockAfterModule > 0)
				SendBlockOfLayoutIds(sizeOfBlockAfterModule, j);
		}
		EmptyChangedLayoutIdsVector();
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridge::SendBlockOfLayoutIds(DWORD numberOfLayoutIdsForMultipleSend, WORD LayoutIdsBlockNumber)
{
	CSegment* pMsg = new CSegment;
	*pMsg << numberOfLayoutIdsForMultipleSend;

	DWORD arrBaseIndex = LayoutIdsBlockNumber * MULTIPLE_CHANGE_LAYOUT_BLOCK_SIZE;

	for (WORD i = 0; i < numberOfLayoutIdsForMultipleSend; i++)
		*pMsg << (DWORD)m_changedLayoutIdsList->at(arrBaseIndex + i);

	SendMsgToMPL(MULTIPLE_PARTIES_CHANGE_LAYOUT_REQ, pMsg);
	POBJDELETE(pMsg);

}

// ------------------------------------------------------------------------------------------
ChangedLayoutIdsOrderedByRoomId* CVideoBridge::BuildMapOfChangedLayoutIdListOrderedByRoomId()
{
	ChangedLayoutIdsOrderedByRoomId* orderedMapByRoomId = new ChangedLayoutIdsOrderedByRoomId;

	int numOfLayoutIds = m_changedLayoutIdsList->size();
	for (int i = 0; i < numOfLayoutIds; ++i)
	{
		DWORD layoutId = m_changedLayoutIdsList->at(i);

		CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(layoutId);
		if(!pVideoPartyCntl){
			PDELETE(orderedMapByRoomId);
		}
		PASSERTSTREAM_AND_RETURN_VALUE(!pVideoPartyCntl, "ConfName:" << m_pConfName << ", PartyId:" << layoutId, NULL);

		ChangedLayoutIdsOrderedByRoomIdList* pList = NULL;

		const CTelepresenseEPInfo& telepresenceInfo = pVideoPartyCntl->GetTelepresenceInfo();

		RoomID roomId = telepresenceInfo.GetRoomID();
		ChangedLayoutIdsOrderedByRoomId::iterator listIter = orderedMapByRoomId->find(roomId);
		if (listIter == orderedMapByRoomId->end())
		{
			pList = new ChangedLayoutIdsOrderedByRoomIdList;
			pList->push_back(layoutId);
			orderedMapByRoomId->insert(std::pair<RoomID, list<DWORD>* >(roomId, pList));
		}
		else
		{
			pList = (*listIter).second;
			pList->push_back(layoutId);
		}
	}

	return orderedMapByRoomId;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridge::SendMsgToMPL(OPCODE opcode, CSegment* pParams)
{
    STATUS status = STATUS_OK;

	CMplMcmsProtocol *pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->AddCommonHeader(opcode);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPortDescriptionHeader( 0xffffffff,
												m_confRsrcID,
												0xffffffff,
												eLogical_video_encoder,
												0, 0, 0, 0, 0, 0xffff );
	if(pParams)
	{
		DWORD nMsgLen = pParams->GetWrtOffset() - pParams->GetRdOffset();
		BYTE* pMessage = new BYTE[nMsgLen];
		pParams->Get(pMessage,nMsgLen);
		pMplMcmsProtocol->AddData(nMsgLen,(const char*)pMessage);
		PDELETEA(pMessage);
	}

	CProcessBase *pProcess = CProcessBase::GetProcess();
	PTRACE2(eLevelInfoNormal,"***CVideoBridge::SendMsgToMPL ", pProcess->GetOpcodeAsString(opcode).c_str());

	status = pMplMcmsProtocol->SendMsgToMplApiCommandDispatcher();
	PASSERT(status);

	POBJDELETE(pMplMcmsProtocol);
}

// ------------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridge::OnEndPartyIvrModeAfterResumeCall(CSegment *pParam)
{
	DWORD partyRsrcID;
	*pParam >> partyRsrcID;
	TRACEINTO << "PartyId:" << partyRsrcID << ", ConfId:" << m_pConf->GetConfId() << " – only implemented for VideoBridgeCP";
}


bool CVideoBridge::IsEQ()
{
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	if (pCommConf)
		return (bool)pCommConf->GetEntryQ();
	return false;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CVideoBridgeCP)

  ONEVENT(VIDEOMUTE,                          CONNECTED,      CVideoBridgeCP::OnConfUpdateVideoMuteCONNECTED)
  ONEVENT(VIDREFRESH,                         CONNECTED,      CVideoBridgeCP::OnConfVideoRefreshCONNECTED)
  ONEVENT(SPEAKERS_CHANGED,                   CONNECTED,      CVideoBridgeCP::OnConfSpeakersChangedCONNECTED)
  ONEVENT(SETCONFVIDLAYOUT_SEEMEALL,          CONNECTED,      CVideoBridgeCP::OnConfSetConfVideoLayoutSeeMeAllCONNECTED)
  ONEVENT(SETCONFVIDLAYOUT_SEEMEPARTY,        CONNECTED,      CVideoBridgeCP::OnConfSetConfVideoLayoutSeeMePartyCONNECTED)
  ONEVENT(SETPRIVATEVIDLAYOUT,                CONNECTED,      CVideoBridgeCP::OnConfSetPrivateVideoLayoutCONNECTED)
  ONEVENT(SETPRIVATEVIDLAYOUTONOFF,           CONNECTED,      CVideoBridgeCP::OnConfSetPrivateVideoLayoutOnOffCONNECTED)
  ONEVENT(LECTURE_MODE_TOUT,                  CONNECTED,      CVideoBridgeCP::OnTimerLectureModeCONNECTED)
  ONEVENT(PRESENTATION_MODE_TOUT,             CONNECTED,      CVideoBridgeCP::OnTimerPresentationModeCONNECTED)
  ONEVENT(UPDATEAUTOLAYOUT,                   CONNECTED,      CVideoBridgeCP::OnConfUpdateAutoLayoutCONNECTED)
  ONEVENT(UPDATE_VIDEO_CLARITY,               CONNECTED,      CVideoBridgeCP::OnConfUpdateVideoClarityCONNECTED)
  ONEVENT(ENDIMPPARTY,                        CONNECTED,      CVideoBridgeCP::OnEndImportParty)
  ONEVENT(ENDIMPPARTY,                        DISCONNECTING,  CVideoBridgeCP::NullActionFunction)
  ONEVENT(END_PARTY_IVR_MODE_ON_RESUME,       CONNECTED,      CVideoBridgeCP::OnEndPartyIvrModeAfterResumeCall)
  ONEVENT(TURN_ON_OFF_TELEPRESENCE,           CONNECTED,      CVideoBridgeCP::OnConfTurnOnOffTelePresence)
  ONEVENT(TURN_ON_OFF_TELEPRESENCE,           DISCONNECTING,  CVideoBridgeCP::NullActionFunction)
  ONEVENT(PARTY_IMAGE_UPDATED,                CONNECTED,      CVideoBridgeCP::OnPartyImageUpdated)
  ONEVENT(SET_MESSAGE_OVERLAY,                CONNECTED,      CVideoBridgeCP::OnConfUpdateMessageOverlayCONNECTED)
  ONEVENT(SET_PARTY_MESSAGE_OVERLAY,          CONNECTED,      CVideoBridgeCP::OnPartyUpdateMessageOverlayCONNECTED)

  //VNGR-26449 - unencrypted conference message
  ONEVENT(SET_ALL_PARTIES_SECURE_MESSAGE,     CONNECTED,      CVideoBridgeCP::OnConfSendSecureMessage)
  ONEVENT(CONF_SECURE_MESSAGE_TIMER,		  CONNECTED,	  CVideoBridgeCP::OnTimerSecureMessageCONNECTED)
  ONEVENT(CONF_MESSAGE_OVERLAY_TIMER, 		  CONNECTED, 	  CVideoBridgeCP::OnTimerMessageOverlayCONNECTED)

  ONEVENT(SET_AUTOSCAN_ORDER,                 CONNECTED,      CVideoBridgeCP::OnConfSetAutoScanOrderCONNECTED)
  ONEVENT(SET_AUTOSCAN_ORDER,                 DISCONNECTING,  CVideoBridgeCP::NullActionFunction)

  ONEVENT(CONTENT_BRIDGE_START_PRESENTATION,  CONNECTED,      CVideoBridgeCP::OnConfContentBridgeStartPresentationCONNECTED)
  ONEVENT(CONTENT_BRIDGE_START_PRESENTATION,  DISCONNECTING,  CVideoBridgeCP::NullActionFunction)

  ONEVENT(CONTENT_BRIDGE_STOP_PRESENTATION,   CONNECTED,      CVideoBridgeCP::OnConfContentBridgeStopPresentationCONNECTED)
  ONEVENT(CONTENT_BRIDGE_STOP_PRESENTATION,   DISCONNECTING,  CVideoBridgeCP::NullActionFunction)

  ONEVENT(AFTER_STOP_CONTENT_TIMER,           CONNECTED,      CVideoBridgeCP::OnTimerAfterStartStopContentCONNECTED)
  ONEVENT(AFTER_STOP_CONTENT_TIMER,           DISCONNECTING,  CVideoBridgeCP::NullActionFunction)

  ONEVENT(AFTER_START_CONTENT_TIMER,          CONNECTED,      CVideoBridgeCP::OnTimerAfterStartStopContentCONNECTED)
  ONEVENT(AFTER_START_CONTENT_TIMER,          DISCONNECTING,  CVideoBridgeCP::NullActionFunction)
  ONEVENT(SET_SITE_NAME,                      CONNECTED,      CVideoBridgeCP::OnConfUpdateSiteNameInfoCONNECTED)
  ONEVENT(REFRESHLAYOUT,                      CONNECTED,      CVideoBridgeCP::OnConfUpdateRefreshLayoutCONNECTED)

  ONEVENT(NOTIFY_VB_ON_NETWORK_QUALITY_CHANGE,ANYCASE,        CVideoBridgeCP::NullActionFunction)
  ONEVENT(NOTIFY_VB_ON_NETWORK_QUALITY_CHANGE,CONNECTED,      CVideoBridgeCP::OnPartyNetworkQualityChangedCONNECTED)

  ONEVENT(NETWORK_QUALITY_UPDATE_TIMER,       ANYCASE,        CVideoBridgeCP::NullActionFunction)
  ONEVENT(NETWORK_QUALITY_UPDATE_TIMER,       CONNECTED,      CVideoBridgeCP::OnTimerNetworkQualityChangedCONNECTED)

  ONEVENT(GET_PARTY_VIDEO_DATA_REQ,           ANYCASE,        CVideoBridgeCP::NullActionFunction)
  ONEVENT(GET_PARTY_VIDEO_DATA_REQ,           CONNECTED,      CVideoBridgeCP::GetPartyVideoDataReq)

  ONEVENT(ADD_VIDEORELAY_IMAGE_TO_MIX,        CONNECTED,      CVideoBridgeCP::OnAddVideoRelayImageToMixCONNECTED)
  ONEVENT(ADD_VIDEORELAY_IMAGE_TO_MIX,        DISCONNECTING,  CVideoBridgeCP::OnAddVideoRelayImageToMixDISCONNECTING)

  ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA,       CONNECTED,      CVideoBridgeCP::OnRelayEpAskForIntraConnected)
  ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA,       ANYCASE,        CVideoBridgeCP::NullActionFunction)

  ONEVENT(RELAY_ASK_ENDPOINTS_FOR_INTRA,      CONNECTED,      CVideoBridgeCP::OnRelayAskEpsForIntraConnected)
  ONEVENT(RELAY_ASK_ENDPOINTS_FOR_INTRA,      ANYCASE,        CVideoBridgeCP::NullActionFunction)

  ONEVENT(RELAY_IMAGE_SVC_AVC_TRANS_UPDATE,   CONNECTED,      CVideoBridgeCP::OnVideoRelayImageSvcAvcTranslateUpdateCONNECTED)
  ONEVENT(RELAY_IMAGE_SVC_AVC_TRANS_UPDATE,   DISCONNECTING,  CVideoBridgeCP::OnVideoRelayImageSvcAvcTranslateUpdateDISCONNECTING)

  ONEVENT(NON_RELAY_IMAGE_AVC_SVC_TRANS_UPDATE,   CONNECTED,      CVideoBridgeCP::OnVideoNonRelayImageAvcSvcTranslateUpdateCONNECTED)
  ONEVENT(NON_RELAY_IMAGE_AVC_SVC_TRANS_UPDATE,   DISCONNECTING,  CVideoBridgeCP::OnVideoNonRelayImageAvcSvcTranslateUpdateDISCONNECTING)

  ONEVENT(ENDCONNECTPARTY,                    CONNECTED,      CVideoBridgeCP::OnEndPartyConnectCONNECTED)
  ONEVENT(NEW_ITP_SPEAKER_TIMER,              ANYCASE,        CVideoBridgeCP::OnTimerITPSpeakerChange)
  ONEVENT(TELEPRESENCE_PARTY_CONNECTION_TIMER,CONNECTED,      CVideoBridgeCP::OnTimerITPTelepresencePartyConnection)
  ONEVENT(TELEPRESENCE_PARTY_CONNECTION_TIMER,ANYCASE,        CVideoBridgeCP::NullActionFunction)

  ONEVENT(UPDATE_MUTE_ALL_VIDEO_EXCEPT_LEADER, CONNECTED,       CVideoBridgeCP::OnConfUpdateMuteAllVideoExceptLeaderCONNECTED)
  ONEVENT(UPDATE_MUTE_ALL_VIDEO_EXCEPT_LEADER, DISCONNECTING,   CVideoBridgeCP::OnConfUpdateMuteAllVideoExceptLeaderDISCONNECTING)

  ONEVENT(UPDATE_MUTE_ALL_INCOMING_VIDEO_EXCEPT_LEADER, CONNECTED,   CVideoBridgeCP::OnConfUpdateMuteAllVideoExceptLeaderForJustIncomingCONNECTED)
  ONEVENT(UPDATE_MUTE_ALL_INCOMING_VIDEO_EXCEPT_LEADER, DISCONNECTING,   CVideoBridgeCP::OnConfUpdateMuteAllVideoExceptLeaderForJustIncomingDISCONNECTING)

  ONEVENT(SET_PARTY_AS_LEADER_FOR_VB,                  CONNECTED,       CVideoBridgeCP::OnSetPartyAsLeaderCONNECTED)
  ONEVENT(SET_PARTY_AS_LEADER_FOR_VB,  				DISCONNECTING,   CVideoBridgeCP::NullActionFunction)

  ONEVENT(UPDATE_ART_WITH_SSRC_ACK,   		  CONNECTED,      CVideoBridgeCP::OnArtUpdateSsrcAckCONNECTED)
  ONEVENT(UPDATE_ART_WITH_SSRC_ACK,   		  DISCONNECTING,  CVideoBridgeCP::NullActionFunction)

  ONEVENT(SEND_MRMP_STREAM_IS_MUST_ACK,   		  CONNECTED,      CVideoBridgeCP::OnMrmpStreamIsMustAckCONNECTED)
  ONEVENT(SEND_MRMP_STREAM_IS_MUST_ACK,   		  DISCONNECTING,  CVideoBridgeCP::NullActionFunction)


  ONEVENT(ALL_MS_IN_SLAVES_CONNECTED,             CONNECTED,      CVideoBridgeCP::OnMainAllMSInSlavesConnected)
  ONEVENT(ALL_MS_IN_SLAVES_CONNECTED,             ANYCASE,        CVideoBridgeCP::NullActionFunction)

  ONEVENT(LYNC_CONF_INFO_UPDATED,                 CONNECTED,      CVideoBridgeCP::OnEventPackageLyncConfInfoUpdatedConnected)
  ONEVENT(LYNC_CONF_INFO_UPDATED,                 ANYCASE,        CVideoBridgeCP::NullActionFunction)

  ONEVENT(LOCAL_RMX_AV_MCU_MSI,         CONNECTED,      CVideoBridgeCP::OnMainLcalRMXAVMCUMSIInd)
  ONEVENT(LOCAL_RMX_AV_MCU_MSI,         ANYCASE,        CVideoBridgeCP::NullActionFunction)

  ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,         CONNECTED,      CVideoBridgeCP::OnConfSetTelepresenceLayoutModeConnected)
  ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,         ANYCASE,        CVideoBridgeCP::NullActionFunction)

PEND_MESSAGE_MAP(CVideoBridgeCP, CVideoBridge);

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeCP
////////////////////////////////////////////////////////////////////////////
CVideoBridgeCP::CVideoBridgeCP()
{
  for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; i++)
    m_pReservation[i] = NULL;

  m_layoutType            = CP_LAYOUT_1X1;
  m_pVisualEffects        = new CVisualEffectsParams;
  m_isVideoClarityEnabled = NO;

  m_pVideoHardwareInterface = NULL;
  m_NumberOfNonRelayImages = 0;
  m_maxAllowedLayerIdForMixSvcToAvc = 0;
  m_maxAllowedLayerIdForMixSvcToAvcHighRes = 0;
  m_isConfInActiveContentPresentation = NO;


  VALIDATEMESSAGEMAP;
}
// ------------------------------------------------------------------------------------------
CVideoBridgeCP::~CVideoBridgeCP() // destructor
{
  for (int i = 0; i < (int)CP_NO_LAYOUT; i++)
  {
    if (IsValidPObjectPtr(m_pReservation[i]))
      POBJDELETE(m_pReservation[i]);
  }
  if (m_pVideoHardwareInterface)
  		POBJDELETE(m_pVideoHardwareInterface);

  POBJDELETE(m_pVisualEffects);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::Create(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
  if (!CPObject::IsValidPObjectPtr(pVideoBridgeInitParams))
  {
    PASSERTMSG(1, "CVideoBridgeCP::Create - Failed, invalid VideoBridgeInitParams");
    m_pConfApi->EndVidBrdgConnect(statInconsistent);
    return;
  }

  CreateBase(pVideoBridgeInitParams);

   m_state = CONNECTED;
  m_pConfApi->EndVidBrdgConnect(statOK);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::CreateBase(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
  CVideoBridge::Create(pVideoBridgeInitParams);

  m_intraDB.SetVideoBridge(this);

  for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; ++i)
    m_pReservation[i] = new CLayout((LayoutType)i, GetConfName());

  GetForcesFromReservation(pVideoBridgeInitParams->GetCommConf());

  if (pVideoBridgeInitParams->GetIsAutoLayout())
  {
    m_pAutoLayoutSet = new CAutoLayoutSet;
    LayoutType tempLayoutType = m_pAutoLayoutSet->GetLayoutType(GetPartyImageVectorSizeUnmuted(), IsSameLayout());
    if (tempLayoutType != m_layoutType)
    {
      m_pReservation[m_layoutType]->SetCurrActiveLayout(NO);
      m_pReservation[tempLayoutType]->SetCurrActiveLayout(YES);
      m_layoutType = tempLayoutType;
    }
  }

  UpdateDB_ConfLayout();

  *m_pVisualEffects       = *(pVideoBridgeInitParams->GetVisualEffectsParams());
  m_isVideoClarityEnabled = pVideoBridgeInitParams->GetIsVideoClarityEnabled();
  *m_pSiteNameInfo        = *(pVideoBridgeInitParams->GetCommConf()->GetSiteNameInfo());
  if( IsRelaySupported() )  //need hardware interface to send video operation point set to MRMP
  {
	  CreateVideoHardwareInterface();
  }
  PrepareParamsForMixConf();
}

// ------------------------------------------------------------------------------------------
	void CVideoBridgeCP::InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams)
	{
	CBridge::InitBridgeParams(pBridgePartyInitParams);

	BOOL isVideoRelay = pBridgePartyInitParams->GetIsVideoRelay();

	TRACEINTO << "ConfName:" << m_pConfName << ", IsVideoRelay:" << (int)isVideoRelay;

	if (!isVideoRelay)
	{
		CVisualEffectsParams* pGatheringVisualEffectsParams = m_pConf->GetGatheringVisualEffects();

		CBridgePartyVideoOutParams* pMediaOutParams = (CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams();
		if (pMediaOutParams)
		{
//			BRIDGE-13161
//			if (pGatheringVisualEffectsParams && pBridgePartyInitParams->GetCascadeLinkMode() != CASCADE_MODE_NONE)
//			{
//				PTRACE(eLevelInfoNormal, "CVideoBridgeCP::InitBridgeParams - SetVisualEffects: from gathering");
//				pMediaOutParams->SetVisualEffects(pGatheringVisualEffectsParams);
//			}
//			else
//			{
//				PTRACE(eLevelInfoNormal, "CVideoBridgeCP::InitBridgeParams - SetVisualEffects: from general");
				pMediaOutParams->SetVisualEffects(m_pVisualEffects);
//			}

			ePartyLectureModeRole partyLectureModeRole = GetLectureModeRoleForParty(pBridgePartyInitParams->GetPartyName());
			pMediaOutParams->SetPartyLectureModeRole(partyLectureModeRole);
			pMediaOutParams->SetLayoutType(m_layoutType);
			pMediaOutParams->SetVideoQualityType(m_videoQuality);
			pMediaOutParams->SetIsVideoClarityEnabled(m_isVideoClarityEnabled);
			pMediaOutParams->SetIsSiteNamesEnabled(m_isSiteNamesEnabled);
		}

		CBridgePartyVideoInParams* pMediaInParams = (CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams();
		if (pMediaInParams)
		{
			pMediaInParams->SetBackgroundImageID(m_pVisualEffects->GetBackgroundImageID());
			pMediaInParams->SetIsVideoClarityEnabled(m_isVideoClarityEnabled);

			eConfMediaType confMediaType = GetConfMediaType();

			if (eMixAvcSvc == confMediaType)
			{
//				BRIDGE-13161
//				if (pGatheringVisualEffectsParams)
//					pMediaInParams->SetVisualEffects(pGatheringVisualEffectsParams);
//				else
					pMediaInParams->SetVisualEffects(m_pVisualEffects);
			}
		}
	}

	if (m_isMuteAllVideoButLeader)
	{
		InitBridgeParamsForMuteAllVideoButLeader(pBridgePartyInitParams);
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::InitBridgeParamsForMuteAllVideoButLeader(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	bool isPartyLeader = IsPartyLeader(pBridgePartyInitParams->GetPartyName());

	TRACEINTO << "PartyName:" << pBridgePartyInitParams->GetPartyName() << ", IsPartyLeader:" << (int)isPartyLeader;

	if (!isPartyLeader)
	{
		if (!pBridgePartyInitParams->GetIsVideoRelay())
		{
			CBridgePartyVideoInParams* pBridgePartyVideoInParams = ((CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams());
			InitBridgeInParamsForMuteAllVideoButLeaderForNonRelay(pBridgePartyVideoInParams);
		}
		else
		{
			CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayMediaParams = ((CBridgePartyVideoRelayMediaParams*)pBridgePartyInitParams->GetMediaInParams());
			InitBridgeInParamsForMuteAllVideoButLeaderForRelay(pBridgePartyVideoRelayMediaParams);
		}
	}
}

// ------------------------------------------------------------------------------------------
bool CVideoBridgeCP::IsNeedToMutePartyVideoInBridgeLevel(const char* partyName)
{
	bool ret = false;
	if (partyName == NULL)
	{
		PASSERT(1);
		return ret;
	}

	TRACEINTO << "PartyName:" << partyName;

	if (m_isMuteAllVideoButLeader)
	{
		bool isPartyLeader = IsPartyLeader(partyName);
		if (!isPartyLeader)
		{
			TRACEINTO << "PartyName:" << partyName << ", RC:YES";
			ret = true;
		}
	}
	return ret;
}
void CVideoBridgeCP::InitBridgeInParamsForMuteAllVideoButLeaderForNonRelay(CBridgePartyVideoInParams* pBridgePartyVideoInParams)
{
	CDwordBitMask  muteMask;
	muteMask = 	pBridgePartyVideoInParams->GetMuteMask();
	muteMask.SetBit(OPERATOR_Prior);
	pBridgePartyVideoInParams->SetMuteMask(muteMask);
}
void CVideoBridgeCP::InitBridgeInParamsForMuteAllVideoButLeaderForRelay(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayMediaParams)
{
	CDwordBitMask  muteMask;
	muteMask = pBridgePartyVideoRelayMediaParams->GetMuteMask();
	muteMask.SetBit(OPERATOR_Prior);
	pBridgePartyVideoRelayMediaParams->SetMuteMask(muteMask);
}
bool CVideoBridgeCP::IsPartyLeader(const char*  name)
{
	bool isLeader = false;
	CCommConf* pCommConf = (CCommConf*)(GetConf())->GetCommConf();
	PASSERT_AND_RETURN_VALUE(!pCommConf, false);
	CConfParty* pConfParty   = pCommConf->GetCurrentParty(name);
	PASSERT_AND_RETURN_VALUE(!pConfParty, false);
	if(pConfParty->GetIsLeader())
		isLeader = true;
	return isLeader;
}

// ------------------------------------------------------------
void CVideoBridgeCP::UpdateVideoInParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	PASSERT_AND_RETURN(!pParty);

	PartyRsrcID partyId = ((CParty*)pParty)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pVideoPartyCntl)
  {
		TRACEINTO << "PartyId:" << partyId << " - Failed, party is not connected to Video Bridge";
    m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 1);
    return;
  }
	if (!pBridgePartyVideoParams)
  {
		TRACEINTO << "PartyId:" << partyId << " - Failed, invalid Video Bridge params object";
    m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 1);
    return;
  }
	if (!pBridgePartyVideoParams->IsValidParams())
  {
		TRACEINTO << "PartyId:" << partyId << " - Failed, invalid Video Bridge params object";
    m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 1);
    return;
  }
  if(IsNeedToMutePartyVideoInBridgeLevel( pVideoPartyCntl->GetName()))
  {
	  InitBridgeInParamsForMuteAllVideoButLeaderForNonRelay(((CBridgePartyVideoInParams*)pBridgePartyVideoParams));
  }
  pVideoPartyCntl->UpdateVideoInParams(pBridgePartyVideoParams);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateVideoRelayInParams(CTaskApp* pParty, CBridgePartyVideoRelayMediaParams * pBridgePartyVideoRelayParams)
{
	PASSERT_AND_RETURN(!pParty);

	PartyRsrcID partyId = ((CParty*)pParty)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pVideoPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, party is not connected to Video Bridge";
	    m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 1);
	    return;
	}
	if (!pBridgePartyVideoRelayParams)
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, invalid Video Bridge params object";
	    m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 1);
	    return;
	 }
	if (!pBridgePartyVideoRelayParams->IsValidParams())
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, invalid Video Bridge params object";
	    m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 1);
	    return;
	  }
	if (!pVideoPartyCntl->IsVideoRelayParty())
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, party is not video relay party";
		 m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_IN_UPDATED, statIllegal, 1);
		 return;
	}
	if(IsNeedToMutePartyVideoInBridgeLevel( pVideoPartyCntl->GetName()))
	{
		InitBridgeInParamsForMuteAllVideoButLeaderForRelay(pBridgePartyVideoRelayParams);
	}
	((CVideoRelayBridgePartyCntl*)  pVideoPartyCntl)->UpdateVideoInParams(pBridgePartyVideoRelayParams);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateVideoRelayOutParams(CTaskApp* pParty, CBridgePartyVideoRelayMediaParams * pBridgePartyVideoRelayParams)
{
	PASSERT_AND_RETURN(!pParty);

	PartyRsrcID partyId = ((CParty*)pParty)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	
	if (!pVideoPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, party is not connected to Video Bridge";
	    m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
	    return;
	}
	
	if (!pBridgePartyVideoRelayParams)
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, invalid Video Bridge params object";
	    m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
	    return;
	}
	
	if (!pBridgePartyVideoRelayParams->IsValidParams())
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, invalid Video Bridge params object";
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
		return;
	}
	
	if (!pVideoPartyCntl->IsVideoRelayParty())
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, party is not video relay party";
		 m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
		 return;
	}

	((CVideoRelayBridgePartyCntl*)pVideoPartyCntl)->UpdateVideoOutParams(pBridgePartyVideoRelayParams);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::RelayEpAskForIntra(CSegment* pParam)
{
	DispatchEvent(RELAY_ENDPOINT_ASK_FOR_INTRA, pParam);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateVideoOutParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	PASSERT_AND_RETURN(!pParty);

	RemoteIdent remoteIdent = (pBridgePartyVideoParams) ? (RemoteIdent)pBridgePartyVideoParams->GetRemoteIdent() : Regular;

	PartyRsrcID partyId = ((CParty*)pParty)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);

	if (!pVideoPartyCntl)
	{
		if (MicrosoftEP_Lync_CCS == remoteIdent)
		{
			CSegment* pMsg = NULL;
			CAckParams* pAckParams	= NULL;
			if (pBridgePartyVideoParams->GetAckParams())
			{
				pMsg = new CSegment;

				pAckParams = new CAckParams(*(pBridgePartyVideoParams->GetAckParams()));

				pAckParams->Serialize(NATIVE, *pMsg);

				CLPRParams*	pLPRParams = pAckParams->GetLPRParams();
				if (pLPRParams)
				{
						TRACEINTO
							<< "PartyId:" << partyId
							<< ", LossProtection:" << pLPRParams->GetLossProtection()
							<< ", MTBF:" << (DWORD)pLPRParams->GetMTBF()
							<< ", CongestionCeiling:" << (DWORD)pLPRParams->GetCongestionCeiling()
							<< ", Fill:" << (DWORD)pLPRParams->GetFill()
							<< ", ModeTimeout:" << (DWORD)pLPRParams->GetModeTimeout();
				}
				else
				{
						TRACEINTO << "PartyId:" << partyId << " - Invalid pLPRParams";
				}
			}

			m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statCCSContentLprSupport,TRUE, pMsg);
			POBJDELETE(pMsg);
			POBJDELETE(pAckParams);
			
			return;
		}
		else
		{
			TRACEINTO << "PartyId:" << partyId << " - Failed, party is not connected to Video Bridge";
			m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
			
			return;
		}
	}

	if (!pBridgePartyVideoParams)
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, invalid Video Bridge params object";
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
		
		return;
	}

	if (!pBridgePartyVideoParams->IsValidParams())
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, invalid Video Bridge params object";
		m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
		
		return;
	}

	pVideoPartyCntl->UpdateVideoOutParams(pBridgePartyVideoParams);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::TurnOnOffTelePresence(BYTE onOff, CVisualEffectsParams* pVisualEffects)
{
	WORD isIncludingVisuals = (pVisualEffects != NULL)? TRUE : FALSE;
	CSegment* pSeg = new CSegment;
	*pSeg << (WORD)onOff;
	*pSeg << (WORD)isIncludingVisuals;
	if (isIncludingVisuals && !CPObject::IsValidPObjectPtr(pVisualEffects))
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::TurnOnOffTelePresence - Failed, invalid Visulal Effects object, ConfName:", m_pConfName);
		return;
	}
	if (isIncludingVisuals)
		pVisualEffects->Serialize(NATIVE, *pSeg);
	DispatchEvent(TURN_ON_OFF_TELEPRESENCE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::OnEndPartyConnectCONNECTED(CSegment* pParam)
{
	PartyRsrcID PartyId;
	WORD status;
	CSegment* pTempParam = new CSegment(*pParam);
	*pTempParam >> PartyId >> status;
	POBJDELETE(pTempParam)

	TRACEINTO << "PartyId:" << PartyId << ", Status:" << (DWORD)status;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);
	if (m_pTelepresenceLayoutMngr != NULL && status == STATUS_OK)
	{
		OnEndPartyConnectWithTelepresence(pPartyCntl);
	}

	CVideoBridge::OnEndPartyConnectCONNECTED(pParam);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnEndPartyConnectWithTelepresence(CVideoBridgePartyCntl* pPartyCntl)
{
	PASSERT_AND_RETURN(!pPartyCntl);
	const CTelepresenseEPInfo& partyITPInfo = pPartyCntl->GetTelepresenceInfo();

	if (partyITPInfo.GetLinkRole() == eRegularParty)
	{
		// case: EPs, not cascade links
		if (partyITPInfo.GetEPtype() == eTelePresencePartyNone)
		{
			EndPartyConnectTelepresenceNon(pPartyCntl);
		}
		else
		{
			// case: Telepresence EPs, not cascade links
			EndPartyConnectTelepresenceEP(pPartyCntl);
		}
	}
	else
	{
		// case: cascade multi link party
		EndPartyConnectTelepresenceMultiLink(pPartyCntl);
	}

	DumpTelepresenceInfo();
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::OnMainAllMSInSlavesConnected(CSegment* pParam)
{
	PartyRsrcID PartyId;
	DWORD numOfConnectedSlaves = 0;
	*pParam >> PartyId >> numOfConnectedSlaves;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	TRACEINTO << pPartyCntl->GetFullName() << ", SenderPartyId:" << PartyId << ", ConnectedSlaves:" << numOfConnectedSlaves;

	TRACECOND_AND_RETURN(!pPartyCntl->IsAVMCUMain(), pPartyCntl->GetFullName() << ", PartyId:" << PartyId);

	if(m_pAVMCUMngr)
	{
		m_pAVMCUMngr->SetNumAVMCULinks(numOfConnectedSlaves+1); // Adding Master
	}
	else
		PASSERT_AND_RETURN(1);
	// Compose VSR

	//// Romem  AV-MCU
	//FetchVideoMSIForLastMSPartyCntlsFromeEventPackage();
	// UnMute Master by MCMS - return  Master to video Mix
	UpdateMute(m_pAVMCUMngr->GetMasterPartyRsrcID(), eOff, MCMS);
	AssignVideoMSIForEachMSPartyCntl();
	SendVSRToAVMCUMain();

}

//--------------------------------------------------------------------------
void CVideoBridgeCP::OnEventPackageLyncConfInfoUpdatedConnected(CSegment* pParam)
{
	PASSERT_AND_RETURN(!m_pAVMCUMngr);

	EventPackage::Events tempEvents;
	*pParam  >> tempEvents;

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" <<  tempEvents.m_id << tempEvents;

	PartyRsrcID masterPartyId = m_pAVMCUMngr->GetMasterPartyRsrcID();

	// Notifications are not related to this conference
	PASSERTSTREAM_AND_RETURN(masterPartyId != tempEvents.m_id, "MasterPartyId:" << masterPartyId << ", PartyId:" << tempEvents.m_id);

	WORD numOfDeletedEPOnAVMCUSide = 0;

	EventPackage::Manager& EventPackage = EventPackage::Manager::Instance();
	EventPackage::LyncMsiList arrVideoMsi;
	EventPackage::ApiLync::Instance().GetMsiList(masterPartyId, arrVideoMsi, EventPackage::eMediaType_Video);

	for (EventPackage::Events::iterator _itr = tempEvents.begin(); _itr != tempEvents.end(); ++_itr)
	{
		EventPackage::EventType event = _itr->first;
		switch (event)
		{
			case EventPackage::eEventType_MediaStatusUpdated:
			{
				EventPackage::EventMediaStatusUpdated* pEventMediaStatusUpdated = (EventPackage::EventMediaStatusUpdated*)_itr->second;
				if (pEventMediaStatusUpdated->value.type != EventPackage::eMediaType_Video)
					continue;
				if (pEventMediaStatusUpdated->value.new_val == EventPackage::eMediaStatusType_SendOnly)
					continue;
				if (pEventMediaStatusUpdated->value.new_val == EventPackage::eMediaStatusType_SendRecv)
					continue;

				PartyRsrcID partyId;
				LyncMsi videoMsi = pEventMediaStatusUpdated->value.msi;
				CImage* pImage = GetPartyImageByVideoMSI(videoMsi, partyId);
				if (pImage) // replace MSI or disconnect slave
				{
					TRACEINTO << "VideoMsi:" << videoMsi << ", OldStatus:" << pEventMediaStatusUpdated->value.old_val << ", NewStatus:" << pEventMediaStatusUpdated->value.new_val;
					replaceMSIOrDisconnectSlave(videoMsi, partyId, arrVideoMsi);

					if(m_pAVMCUMngr->GetVideoSwitchingModeType() == EventPackage::eVideoSwitchingModeType_ManualSwitched  &&  m_pAVMCUMngr->GetSpotLightSpeakerMsi() == videoMsi)
					{
						TRACEINTO <<  " eEventType_MediaStatusUpdated, Hold video MSI is identical to SpotLihgt Speaker MSI - mute video of AV_MCU-Master, Conf Name: " << m_pConfName;
						m_pAVMCUMngr->SetSpotLightSpeakerMsi(0);
						UpdateMute(m_pAVMCUMngr->GetMasterPartyRsrcID(), eOn, MCMS);
					}

					numOfDeletedEPOnAVMCUSide++;
				}
				else // do nothing
				{
					TRACEINTO << "VideoMsi:" << videoMsi << " - Media does not belong to any MS Master/Slave, no update";
				}
				break;
			}

			case EventPackage::eEventType_MediaDeleted:
			{
				EventPackage::EventMediaDeleted* pEventMediaDeleted = (EventPackage::EventMediaDeleted*)_itr->second;
				if (pEventMediaDeleted->value.type != EventPackage::eMediaType_Video)
					continue;

				PartyRsrcID partyId;
				LyncMsi videoMsi = pEventMediaDeleted->value.msi;
				CImage* pImage = GetPartyImageByVideoMSI(videoMsi, partyId);
				if (pImage) // replace MSI or disconnect slave
				{
					replaceMSIOrDisconnectSlave(videoMsi, partyId, arrVideoMsi);

					if(m_pAVMCUMngr->GetVideoSwitchingModeType() == EventPackage::eVideoSwitchingModeType_ManualSwitched  &&  m_pAVMCUMngr->GetSpotLightSpeakerMsi() == videoMsi)
					{
						TRACEINTO <<  " eEventType_MediaDeleted, Deleted video MSI is identical to SpotLihgt Speaker MSI - exiting spotLight, Conf Name: " << m_pConfName;
						m_pAVMCUMngr->SetSpotLightSpeakerMsi(0);
						UpdateMute(m_pAVMCUMngr->GetMasterPartyRsrcID(), eOn, MCMS);
					}

					numOfDeletedEPOnAVMCUSide++;
				}
				else // do nothing
				{
					TRACEINTO << "VideoMsi:" << videoMsi << " - Media does not belong to any MS Master/Slave, no update";
				}
				break;
			}

			case EventPackage::eEventType_UserDisplayTextUpdated:
			{
				EventPackage::EventUserDisplayTextUpdated* pEventUserDisplayTextUpdated = (EventPackage::EventUserDisplayTextUpdated*)_itr->second;

				CAVMCULinksVector& AVMCUVector = m_pAVMCUMngr->GetMCULinksVector();

				for (CAVMCULinksVector::iterator _ii = AVMCUVector.begin(); _ii != AVMCUVector.end(); ++_ii)
				{
					PartyRsrcID partyId = *(_ii);
					CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
					if (!pVideoPartyCntl)
					{
						TRACEINTO << "PartyId:" << partyId << " - Video Bridge Party Control not found, no update";
						continue;
					}
					BOOL isInPortOpened  = FALSE;
					BOOL isOutPortOpened = FALSE;
					pVideoPartyCntl->ArePortsOpened(isInPortOpened, isOutPortOpened);
					if (!isInPortOpened)
						continue;

					LyncMsi videoMsi = pVideoPartyCntl->GetVideoMSI();
					if (!IsValidMSI(videoMsi))
						continue;

					EventPackage::MediaInfo info;
					if (!EventPackage.GetMedia(masterPartyId, std::bind2nd(EventPackage::Predicate::Media_Msi(), videoMsi), info))
						continue;

					if (info.user->m_entity != pEventUserDisplayTextUpdated->value.user)
						continue;

					char siteName[MAX_SITE_NAME_ARR_SIZE];
					if (BuildAVMCUSiteName(pEventUserDisplayTextUpdated->value.new_val, info.endpoint->m_displayText, siteName))
					{
						//pVideoPartyCntl->SetSiteName(siteName);
						SetSiteName(pVideoPartyCntl->GetName(), siteName);
					}
				}
				break;
			}

			case EventPackage::eEventType_EndpointDisplayTextUpdated:
			{
				EventPackage::EventEndpointDisplayTextUpdated* pEventEndpointDisplayTextUpdated = (EventPackage::EventEndpointDisplayTextUpdated*)_itr->second;

				std::string endpointId = pEventEndpointDisplayTextUpdated->value.endpoint;

				EventPackage::EndpointInfo info;
				if (!EventPackage.GetEndpoint(masterPartyId, std::bind2nd(EventPackage::Predicate::Endpoint_Entity(), endpointId), info))
					continue;

				const EventPackage::Endpoint::MediasContainer& medias = info.endpoint->m_medias;
				EventPackage::Endpoint::MediasContainer::const_iterator _imedia = std::find_if(medias.begin(), medias.end(), std::bind2nd(EventPackage::Predicate::Media_Type(), EventPackage::eMediaType_Video));
				const EventPackage::Media* pMedia = (_imedia != medias.end()) ? &*_imedia : NULL;
				if (!pMedia)
					continue;

				if (pMedia->m_status != EventPackage::eMediaStatusType_SendOnly && pMedia->m_status != EventPackage::eMediaStatusType_SendRecv)
					continue;

				LyncMsi videoMsi = pMedia->m_msi;
				PartyRsrcID partyId = DUMMY_PARTY_ID;
				CImage* pImage = GetPartyImageByVideoMSI(videoMsi, partyId);
				if (!pImage)
					continue;

				CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
				if (!pVideoPartyCntl)
					continue;

				char siteName[MAX_SITE_NAME_ARR_SIZE];
				if (BuildAVMCUSiteName(info.user->m_displayText, pEventEndpointDisplayTextUpdated->value.new_val, siteName))
				{
					//pVideoPartyCntl->SetSiteName(siteName);
					SetSiteName(pVideoPartyCntl->GetName(), siteName);
				}
				break;
			}

			case EventPackage::eEventType_VideoSwitchingModeUpdated:
			{
				EventPackage::EventVideoSwitchingModeUpdated* pEventVideoSwitchingModeUpdated = (EventPackage::EventVideoSwitchingModeUpdated*)_itr->second;
				EventPackage::VideoSwitchingModeType newLyncVideoSwitchingMode = pEventVideoSwitchingModeUpdated->value.new_val;
				LyncMsi speakerMsi = pEventVideoSwitchingModeUpdated->value.msi;


				if(newLyncVideoSwitchingMode == EventPackage::eVideoSwitchingModeType_ManualSwitched)
				{
					if(!IsValidMSI(speakerMsi,false))
					{
						PASSERT(1);
						TRACEINTO << "ConfName:" << m_pConfName << ", Msi:" << speakerMsi << " - Try to enter spotlight with invalid MSI, ignore it";
						break;
					}

					EventPackage::MediaInfo info;
					if (!EventPackage.GetMedia(masterPartyId, std::bind2nd(EventPackage::Predicate::Media_Msi(), speakerMsi), info))
						continue;

					m_pAVMCUMngr->SetVideoSwitchingModeType(newLyncVideoSwitchingMode);
					m_pAVMCUMngr->SetSpotLightSpeakerMsi(speakerMsi);

				}
				else
				{
					if(newLyncVideoSwitchingMode == EventPackage::eVideoSwitchingModeType_DEFAULT)
						PASSERT(1);
					m_pAVMCUMngr->SetVideoSwitchingModeType(EventPackage::eVideoSwitchingModeType_DominantSpeakerSwitched);
					m_pAVMCUMngr->SetSpotLightSpeakerMsi(0);
				}
				ActionsOnAVMCUSpotLightOnOff();
				break;
			}

			default:
			{
				TRACEINTO << "Event:" << event << " - Event is not supported by Video Bridge";
				continue;
			}
		}
	}

	// After all those changes, send VSR to main
	if (numOfDeletedEPOnAVMCUSide)
		SendVSRToAVMCUMain();
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::OnMainLcalRMXAVMCUMSIInd(CSegment* pParam)
{
	PartyRsrcID PartyId;
	LyncMsi localRMXAudioMSI = 0;
	LyncMsi localRMXVideoMSI = 0;
	*pParam >> PartyId >> localRMXAudioMSI >> localRMXVideoMSI;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	TRACECOND_AND_RETURN(!pPartyCntl->IsAVMCUMain(), "PartyId:" << PartyId);

	PASSERT_AND_RETURN(!m_pAVMCUMngr);

	pPartyCntl->SetMSAVMCUaudioLocalMsi(localRMXAudioMSI);
	m_pAVMCUMngr->SetMSAVMCULocalVideoMSI(localRMXVideoMSI);

	TRACEINTO << "PartyId:" << PartyId << ", LocalAudioMsi:" << localRMXAudioMSI << ", LocalVideoMsi:" << localRMXVideoMSI;
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::EndPartyConnectTelepresenceNon(CVideoBridgePartyCntl* pPartyCntl)
{
	PASSERT_AND_RETURN(!pPartyCntl);

	const CTelepresenseEPInfo& partyITPInfo = pPartyCntl->GetTelepresenceInfo();

	// check if room already exist in m_pTelepresenceLayoutMngr
	CRoomInfo room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(pPartyCntl->GetITPSiteName(), partyITPInfo.GetEPtype());
	if (room_info.GetRoomId() != (RoomID)-1)
	{
		TRACEINTO << "ITPSiteName:" << pPartyCntl->GetITPSiteName() << " - Room already exist";
		DBGPASSERT(room_info.GetRoomId());
	}
	// Add new room
	TRACEINTO << "ITPSiteName:" << pPartyCntl->GetITPSiteName() << ", RoomId:" << partyITPInfo.GetRoomID() << ", IsCascadeRoom:" << (WORD)partyITPInfo.GetLinkRole() << " - Adding new room";

	m_pTelepresenceLayoutMngr->AddNewRoom(partyITPInfo.GetRoomID(), partyITPInfo.GetLinkRole(), partyITPInfo.GetNumOfLinks(), partyITPInfo.GetEPtype(), pPartyCntl->GetITPSiteName());
	m_pTelepresenceLayoutMngr->UpdateRoomSubLink(partyITPInfo.GetRoomID(), 0, pPartyCntl->GetPartyRsrcID());

	// if RMX manages layout - send change layout
	if (GetManageTelepresenceLayoutsInternally())
	{
		if (FALSE == partyITPInfo.GetWaitForUpdate())
		{
			TRACEINTO << "ITPSiteName:" << pPartyCntl->GetITPSiteName() << ", RoomId:" << partyITPInfo.GetRoomID() << " - Changing room layout";
			CRoomInfo roomInfo = m_pTelepresenceLayoutMngr->GetRoomInfo(partyITPInfo.GetRoomID());
			ChangeRoomLayout(roomInfo);	// ****TBD - check if sending change layout to current room or to all rooms if needed
		}
		else
		{
			TRACEINTO << "ITPSiteName:" << pPartyCntl->GetITPSiteName() << " - Party is waiting for update telepresence info";
		}
	}
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::EndPartyConnectTelepresenceEP(CVideoBridgePartyCntl* pPartyCntl)
{
	PASSERT_AND_RETURN(!pPartyCntl);
	TRACEINTO << " ITP_PARTY_IDENT" << " DEBUG 02 " << " ITPSiteName = " << pPartyCntl->GetITPSiteName();
	const CTelepresenseEPInfo& partyITPInfo = pPartyCntl->GetTelepresenceInfo();

	CRoomInfo room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(partyITPInfo.GetRoomID());

	if(room_info.GetRoomId() == (RoomID)-1) 	// not found by room ID
		room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(pPartyCntl->GetITPSiteName(),partyITPInfo.GetEPtype());

    if(room_info.GetRoomId() == (RoomID)-1)	// not found by name
    {
    	BYTE link_num = -1; 
    	//In case the name of TIP is wrong we correct the link num
    	if (m_pTelepresenceLayoutMngr->GetLinkPartyIndex(pPartyCntl->GetITPSiteName()) == (BYTE)-1)
    	{
    		CCommConf* pCommConf = (CCommConf*)(GetConf())->GetCommConf();
    		PASSERT_AND_RETURN(!pCommConf);

    		CConfParty* pConfParty   = pCommConf->GetCurrentParty(partyITPInfo.GetPartyMonitorID());
    		PASSERTSTREAM_AND_RETURN(!pConfParty, "Failed, 'pConfParty' is NULL");
    		
    		if (pConfParty->GetIsTipCall())
    		{
    			TRACEINTO << "EP is ITP bad ITP name, fixing the link_num";
    			link_num = 0;
    		}
    	}
    	
        // room does not exist - first screen - add new room
        TRACEINTO << "ITP_CASCADE:  adding new room for telepresence EP " << pPartyCntl->GetITPSiteName() << ", room id " << partyITPInfo.GetRoomID();
        m_pTelepresenceLayoutMngr->AddNewRoom(partyITPInfo.GetRoomID(),FALSE, (BYTE)-1, partyITPInfo.GetEPtype(), pPartyCntl->GetITPSiteName(),link_num);
        m_pTelepresenceLayoutMngr->UpdateRoomSubLinkTIPEP(partyITPInfo.GetRoomID(), partyITPInfo.GetEPtype(), pPartyCntl->GetITPSiteName(), pPartyCntl->GetPartyRsrcID(),link_num);

        // start timer to wait for more connections. for multi-cascade links, speaker changes to this room will be delayed until the end of the timer
        m_pTelepresenceLayoutMngr->SetRoomPendingForRealTPConnections(partyITPInfo.GetRoomID(), TRUE);
        pPartyCntl->StartTelepresenceConnectionTimer();		// if CTS 1 screen - the timer is not needed but it will be stopped at the end of this function (see below)

    }
    else
    {
    	// found existing room with name prefix matching the new party name according to TP naming format ("xyz1", "xyz3", etc.)
        TRACEINTO << "ITP_CASCADE: Telepresence slave " << pPartyCntl->GetITPSiteName() << " was added to room id " << room_info.GetRoomId() << " belonging to main party " << room_info.GetMainLinkPartyName();

    	// update only CTS EP's room info with RoomType and MaxScreens
    	// in case of CTS EP (with EP type CTS updated) we know the total number of screens
    	// (this would be master/first CTS screen after receiving update or any slave screen that connects after it)
    	if((partyITPInfo.GetEPtype() == eTelePresencePartyCTS) && (room_info.GetRoomEPType()!= eTelePresencePartyCTS) && (partyITPInfo.GetRoomID() == room_info.GetRoomId()))
    	{
    	   TRACEINTO << " CTS num of screens ident: updating room type to eTelePresencePartyCTS";
    	   m_pTelepresenceLayoutMngr->UpdateRoomTypeAndMaxScreens(partyITPInfo.GetRoomID(),partyITPInfo.GetEPtype(),partyITPInfo.GetNumOfLinks());
    	}

		// update om info
        m_pTelepresenceLayoutMngr->UpdateRoomSubLinkTIPEP(partyITPInfo.GetRoomID(), partyITPInfo.GetEPtype(), pPartyCntl->GetITPSiteName(), pPartyCntl->GetPartyRsrcID());

		// check if room id needs to be updated - if we just added a new room + 1st link, this is not necessary.
		if (partyITPInfo.GetRoomID() != room_info.GetRoomId())
		{
			BOOL is_main_link = (partyITPInfo.GetEPtype() != eTelePresencePartyNone) && (m_pTelepresenceLayoutMngr->GetLinkPartyIndex(pPartyCntl->GetITPSiteName()) == 0);
			// main link connected after a sublink. since the room is changed to the main link's room id,
			// we go over the sublinks that were already connected and updated their partycntl's
			if (is_main_link)
			{
				TRACEINTO << "ITP_CASCADE: received late main link connection for telepresence EP " << pPartyCntl->GetName() << ", updating sublinks";
				room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(partyITPInfo.GetRoomID());

				// update all party cntl's of the new room id
				BYTE numlinks = room_info.GetNumberOfActiveLinks();
				if (numlinks > MAX_CASCADED_LINKS_NUMBER)	//anatg-KW
				{
					DBGPASSERT(numlinks);
					numlinks = MAX_CASCADED_LINKS_NUMBER;
				}
				for (int party_cntl_ind = 1; party_cntl_ind < numlinks; party_cntl_ind++)
				{
					PartyRsrcID link_party_id = room_info.GetLinkPartyId(party_cntl_ind);
					if (link_party_id == (PartyRsrcID)-1)
						continue;
					CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(link_party_id);
					PASSERTMSG_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyCntl), "CVideoBridgeCP::OnEndPartyConnectWithTelepresence - party not found for telepresence sublink!");
					//const CTelepresenseEPInfo& link_TP_info = pPartyCntl->GetTelepresenceInfo();
					pPartyCntl->SetTelepresenceInfoRoomID(room_info.GetRoomId());
					pPartyCntl->SetTelepresenceInfoLinkRole(room_info.GetIsCascade());
				}
			}
			else
			{
				// telepresence EP sublink - update with current room info
				pPartyCntl->SetTelepresenceInfoRoomID(room_info.GetRoomId());
				pPartyCntl->SetTelepresenceInfoLinkRole(room_info.GetIsCascade());
			}
		}
    }

	// Special treatment for CTS:

	if(partyITPInfo.GetEPtype() == eTelePresencePartyCTS)
	{
		// check if all screens were  updated room info
		CRoomInfo updated_room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(partyITPInfo.GetRoomID());
		BOOL isRoomAllLinksConnected = updated_room_info.IsRoomAllLinksConnected();
		BYTE maxLinks = updated_room_info.GetMaxLinks();
		BYTE activeLinks = updated_room_info.GetNumberOfActiveLinks();
		TRACEINTO << " CTS num of screens ident: maxLinks = " << (WORD)maxLinks << ", activeLinks = " << (WORD)activeLinks << ", isRoomAllLinksConnected = " << (WORD)isRoomAllLinksConnected;
		if(activeLinks == maxLinks && isRoomAllLinksConnected)	// all screens connected
		{
			TRACEINTO << " CTS num of screens ident - all screens connected";
			// stop timer ('wait for all screens to connect' timer)
			pPartyCntl->StopTelepresenceConnectionTimer();
			// stop pending for remaining screens to connect (send change layout and set off the pending flag)
			OnStopPendingForLinksConnections(pPartyCntl->GetPartyRsrcID());
		}
	}
	else
	{
		if (m_pTelepresenceLayoutMngr->GetRoomPendingForRealTPConnections(room_info.GetRoomId()) == FALSE)
		{
			// stop pending for remaining screens to connect (send change layout and set off the pending flag)
			OnStopPendingForLinksConnections(pPartyCntl->GetPartyRsrcID());
		}
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::EndPartyConnectTelepresenceMultiLink(CVideoBridgePartyCntl* pPartyCntl)
{
	PASSERT_AND_RETURN(!pPartyCntl);

	const CTelepresenseEPInfo& partyITPInfo = pPartyCntl->GetTelepresenceInfo();
	CRoomInfo room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(pPartyCntl->GetITPSiteName(), partyITPInfo.GetEPtype());

	BOOL is_party_in_room = FALSE;
	//   main party for multi-link cascade
	if ((partyITPInfo.GetLinkNum() == 0) && (room_info.GetRoomId() == (RoomID)-1))
	{
		TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << ", Link#:" << partyITPInfo.GetLinkNum() << ", RoomId:" << partyITPInfo.GetRoomID() << ", IsCascadeRoom:" << (WORD)partyITPInfo.GetLinkRole() << " - Adding new room";
	m_pTelepresenceLayoutMngr->AddNewRoom(partyITPInfo.GetRoomID(), partyITPInfo.GetLinkRole(), partyITPInfo.GetNumOfLinks(), partyITPInfo.GetEPtype(), pPartyCntl->GetITPSiteName());
    }

    // dealing with teleprescence sublink- update link's m_telepresenceInfo member
	if (partyITPInfo.GetLinkNum() != 0)
	{
		if (room_info.GetRoomId() == (RoomID)-1)
		{
			TRACEINTO << "SiteName:" << pPartyCntl->GetITPSiteName() << ", Link#:" << partyITPInfo.GetLinkNum() << ", RoomId:" << partyITPInfo.GetRoomID() << ", IsCascadeRoom:" << (WORD)partyITPInfo.GetLinkRole() << " - First multi-cascade connection is from sublink, adding new room";

	    char main_link_party_name[H243_NAME_LEN];
	    main_link_party_name[0] = '\0';

	    m_pTelepresenceLayoutMngr->SetLinkPartyName(main_link_party_name, pPartyCntl->GetITPSiteName(), 0);
	    m_pTelepresenceLayoutMngr->AddNewRoom(partyITPInfo.GetRoomID(), partyITPInfo.GetLinkRole(), partyITPInfo.GetNumOfLinks(), partyITPInfo.GetEPtype(), main_link_party_name);
	}
		else
		{
	    pPartyCntl->SetTelepresenceInfoEPtype(room_info.GetRoomEPType());
	    pPartyCntl->SetTelepresenceInfoRoomID(room_info.GetRoomId());
	    pPartyCntl->SetTelepresenceInfoNumOfLinks(room_info.GetNumberOfActiveLinks());
	    pPartyCntl->SetTelepresenceInfoLinkRole(room_info.GetIsCascade());
	}
    }

    // update the current party room
    BOOL all_links_connected = m_pTelepresenceLayoutMngr->UpdateRoomSubLink(partyITPInfo.GetRoomID(), partyITPInfo.GetLinkNum(), pPartyCntl->GetPartyRsrcID());
    // check if all links updated -if yes, build layout.
	if (all_links_connected)
	{
      // check if no speaker or main link party IS the speaker- if so skip build layout
      BOOL valid_change = FALSE;
      CVideoBridgePartyCntl* pSpeakerPartyCntl = NULL;
      CRoomInfo room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(partyITPInfo.GetRoomID());
      if (CPObject::IsValidPObjectPtr(m_pLastActiveVideoSpeakerRequest))
	{
			PartyRsrcID partyId = ((CParty*)m_pLastActiveVideoSpeakerRequest)->GetPartyRsrcID();
			pSpeakerPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
			if (pSpeakerPartyCntl)
	    {
				is_party_in_room = room_info.IsPartyInRoom(partyId);
	      valid_change =  !is_party_in_room;
	    }
	}
      if (valid_change)
	{
			TRACEINTO << "RoomId:" << partyITPInfo.GetRoomID() << " - All links connected to Room, building layout";
	  m_pTelepresenceLayoutMngr->OnSpeakerChanged(partyITPInfo.GetRoomID(), pSpeakerPartyCntl->GetTelepresenceInfo().GetRoomID());
	}
      else
			TRACEINTO << "RoomId:" << partyITPInfo.GetRoomID() << " - All links connected to Room, no valid video speaker present";
    }
    // if not all connected, blank the layout of currently connecting party.
    else
      {
		TRACEINTO << "Link#:" << partyITPInfo.GetLinkNum() << ", RoomId:" << partyITPInfo.GetRoomID() << "PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Sending blank layout to party";
	m_pTelepresenceLayoutMngr->SendBlankPrivateLayoutToVideoBridge(pPartyCntl->GetName());
      }
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdatePartyTelePresenceMode(CTaskApp* pParty, eTelePresencePartyType partyNewTelePresenceMode)
{
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = ((CParty*)pParty)->GetPartyRsrcID();

	TRACEINTO << "PartyId:" << partyId << ", TelePresenceMode:" << partyNewTelePresenceMode;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);

  pPartyCntl->UpdatePartyTelePresenceMode(partyNewTelePresenceMode);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::PLC_SetPartyPrivateLayout(PartyRsrcID partyId, LayoutType newPrivateLayoutType)
{
	TRACEINTO << "PartyId:" << partyId << ", PrivateLayoutType:" << newPrivateLayoutType;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);
  PASSERT_AND_RETURN(newPrivateLayoutType >= CP_NO_LAYOUT);

	pPartyCntl->PLC_SetPartyPrivateLayout(newPrivateLayoutType);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::VENUS_SetPartyPrivateLayout(PartyRsrcID partyId, LayoutType newPrivateLayoutType)
{
	TRACEINTO << "PartyId:" << partyId << ", PrivateLayoutType:" << newPrivateLayoutType;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);
  PASSERT_AND_RETURN(newPrivateLayoutType >= CP_NO_LAYOUT);

	pPartyCntl->PLC_SetPartyPrivateLayout(newPrivateLayoutType);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::PLC_PartyReturnToConfLayout(PartyRsrcID partyId)
{
	TRACEINTO << "PartyId:" << partyId;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	pPartyCntl->PLC_PartyReturnToConfLayout();
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::PLC_ForceToCell(PartyRsrcID partyID, PartyRsrcID partyImageToSee, BYTE cellToForce)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::PLC_ForceToCell - ConfName:", m_pConfName);

  PASSERT_AND_RETURN(partyID == partyImageToSee);

  CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(partyID));
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVideoPartyCntl));

  CVideoBridgePartyCntl* pVideoPartyCntlImageToSee = (CVideoBridgePartyCntl*)GetPartyCntl(partyImageToSee);
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVideoPartyCntlImageToSee));

  PASSERT_AND_RETURN(cellToForce > 15);

  pVideoPartyCntl->PLC_ForceToCell(pVideoPartyCntlImageToSee->GetName(), cellToForce);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::PLC_CancelAllPrivateLayoutForces(PartyRsrcID partyID)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::PLC_CancelAllPrivateLayoutForces - ConfName:", m_pConfName);

  CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(partyID));
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVideoPartyCntl));

  pVideoPartyCntl->PLC_CancelAllPrivateLayoutForces();
}
// ------------------------------------------------------------------------------------------
CVideoOperationPointsSet* CVideoBridgeCP::GetConfVideoOperationPointsSet()const
{
	return m_pConf->GetVideoOperationPointsSet();

}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::CreateVideoHardwareInterface()
{
	if (m_pVideoHardwareInterface)
		POBJDELETE(m_pVideoHardwareInterface);

	if (DUMMY_CONF_ID == m_confRsrcID)
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeCP::CreateVideoHardwareInterface : confRsrcID is not set yet!!! ConfName - ", m_pConfName);
	}
	else
	{
	// joinMP - set eLogical_relay_video_encoder instead of eLogical_res_none - because message in RMX
	// OperationPointSet sent to the relevant card MRMP through MplApi shared memory - using eLogical_relay_video_encoder
		CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, DUMMY_PARTY_ID, m_confRsrcID, eLogical_relay_video_encoder);
		m_pVideoHardwareInterface = new CVideoHardwareInterface();
		m_pVideoHardwareInterface->Create(&rsrcParams);
	}
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::SendAddVideoOperationPointSet()
{
	CVideoOperationPointsSet* videoOperationPointSet = GetConfVideoOperationPointsSet();
	m_pVideoHardwareInterface->SendAddVideoOperationPointSet(videoOperationPointSet);

}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::SendRemoveVideoOperationPointSet()
{
	CVideoOperationPointsSet* videoOperationPointSet = GetConfVideoOperationPointsSet();
	m_pVideoHardwareInterface->SendRemoveVideoOperationPointSet(videoOperationPointSet);

}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::NotifyOnAddedVideoInStreams(DWORD partyRscId)
{
	TRACECOND_AND_RETURN(!partyRscId, "CVideoBridgeCP::NotifyOnAddedVideoInStreams - Image is does not exist, no need to update on remove");

	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	TRACECOND_AND_RETURN(pPartyImageLookupTable == 0, "CVideoBridgeCP::NotifyOnAddedVideoInStreams - pPartyImageLookupTable is NULL");

	CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
    if (!pImage)
    {
    	TRACESTR(eLevelInfoHigh) << "CVideoBridgeCP::NotifyOnAddedVideoInStreams - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId;
        return;
    }

	CVideoRelayMediaStreamList videoRelayMediaStreamList = pImage->GetVideoRelayMediaStreamsList();
	TRACECOND_AND_RETURN(videoRelayMediaStreamList.size() == 0, "CVideoBridgeCP::NotifyOnAddedVideoInStreams - Not SVC");

	std::list<CMedia> listMedia;
	for (CVideoRelayMediaStreamList::iterator itr = videoRelayMediaStreamList.begin(); itr != videoRelayMediaStreamList.end(); ++itr)
	{
    	int resolutionWidth;
		int resolutionHeight;
		int frameRate;
		int maxBitRate;
		int layerId = (*itr)->GetLayerId();
		bool foundLayerId = GetAPIVideoInNotifyParamsFromOperationPointList(layerId, resolutionWidth,resolutionHeight,frameRate, maxBitRate);
		if(foundLayerId)
		{
			CMedia media;

			media.m_eMediaStatus = eMediaStatusSendOnly;
			media.m_eMediaType = eMediaVideo;
			media.m_dwID = (*itr)->GetSsrc(); //in this version we will use the ssrc as the id

			media.m_eMediaRole = eMediaRolePeople;
			media.m_dwSsrcID = (*itr)->GetSsrc();

			media.m_maxResolution.m_iWidth = resolutionWidth;
			media.m_maxResolution.m_iHeight = resolutionHeight;
			media.m_frameRate.m_iMax= frameRate;
			media.m_bitRate.m_iMax = maxBitRate;
			media.m_eCodec = eH264SVC;
			media.m_eAvailable = eAvailableTrue;
			listMedia.push_back(media);
		}
		else
			PTRACE2INT(eLevelError, "CVideoBridgeCP::NotifyOnAddedVideoInStreams - Didn't find layer id:", layerId);
	}

	if(listMedia.size() > 0)
	{
		CMediaList mediaList;
		bool isUrget=true;//the notification on adding video stream is urgent
		mediaList.ReplaceMediaVideoIn(listMedia, isUrget);
		CTaskApp* pPartyTaskApp = GetPartyTaskApp(partyRscId);
		if (pPartyTaskApp)
		{
			CSegment* pSeg = new CSegment;
			mediaList.Serialize(pSeg);
			m_pConfApi->UpdateDB(pPartyTaskApp, MEDIA, 0, 0, pSeg);
			TRACEINTO << "PartyId:" << pImage->GetPartyRsrcId() << "\nMediaAdded:" << CStlUtils::ContainerToString(listMedia).c_str();
			POBJDELETE(pSeg);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
bool CVideoBridgeCP::GetAPIVideoInNotifyParamsFromOperationPointList(int layerId, int& resolutionWidth,int& resolutionHeight,int& frameRate, int& maxBitRate)
{
	VideoOperationPoint videoOperationPoint;
	bool bFound = false;
	bFound = GetOperationPointFromList(layerId,videoOperationPoint );
	if (bFound)
	{
		resolutionWidth = videoOperationPoint.m_frameWidth;
		resolutionHeight = videoOperationPoint.m_frameHeight;
		frameRate = videoOperationPoint.m_frameRate*10/256;//translate it to API with MCCF
		maxBitRate = videoOperationPoint.m_maxBitRate;

	}
	return bFound;
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::NotifyOnRemovedVideoInStreams(DWORD partyRscId, DWORD specificSsrc)
{
	TRACECOND_AND_RETURN(!partyRscId, "Image is does not exist, no need to update on remove");

	if(IsPartyImageExistInImageVector(partyRscId))
	{
		CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
		CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
		if (!pImage)
		{
			TRACEINTO << "PartyId:" << partyRscId << " - Failed, The lookup table doesn't have an element";
			return;
		}

		std::list<unsigned int> listMediaID;

		if ((DWORD)-1 == specificSsrc)
		{
			CVideoRelayMediaStreamList videoRelayMediaStreamsList = pImage->GetVideoRelayMediaStreamsList();
			for (CVideoRelayMediaStreamList::iterator _ii = videoRelayMediaStreamsList.begin(); _ii != videoRelayMediaStreamsList.end(); ++_ii)
				listMediaID.push_back((*_ii)->GetSsrc()); // for change layout integration we will use the ssrc as the id
		}
		else	// from AvcSvcTranslator, remove 1 translator
			listMediaID.push_back(specificSsrc);

		if(listMediaID.size() > 0)
		{
			CTaskApp* pPartyTaskApp = GetPartyTaskApp(pImage->GetArtPartyId());
			if (pPartyTaskApp && m_pConfApi)
			{
				unsigned int isUrget = 1;//the notification on adding video stream is urgent
				CSegment* pSeg = new CSegment;

				CStlUtils::SerializeListWithFlag(listMediaID, isUrget, pSeg);
				m_pConfApi->UpdateDB(pPartyTaskApp, MEDIA_REMOVE, 0, 0, pSeg);
				TRACEINTO << "PartyId:" << pImage->GetPartyRsrcId() << "\nRemoveMedia:" << CStlUtils::ContainerToString(listMediaID).c_str();
				POBJDELETE(pSeg);
			}
		}
	}
	else
	{
		TRACEINTO << "PartyId:" << partyRscId << " - Image doesn't Exist In ImageVector";
	}
}

//--------------------------------------------------------------------------
bool CVideoBridgeCP::GetOperationPointFromList(int layerId, VideoOperationPoint& videoOperationPoint)const
{
	CVideoOperationPointsSet* videoOperationPointSet = GetConfVideoOperationPointsSet();
	if(IsValidPObjectPtr(videoOperationPointSet))
		return videoOperationPointSet->GetOperationPointFromList(layerId, videoOperationPoint);
	return false;
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::GetValidRelayImagesForParty(PartyRsrcID partyRsrcId, std::list<const CImage*>& listImages, bool isCascadeLink) const
{
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	CImage* pSelfViewImage = NULL;

	for (CPartyImageVector::iterator _ii = m_pPartyImageVector->begin(); _ii != m_pPartyImageVector->end(); ++_ii)
	{
		DWORD partyRscId = *_ii;
		if (partyRscId)
		{
			CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
			PASSERTSTREAM(!pImage, "CVideoBridgeCP::GetValidRelayImagesForParty - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

			if (pImage)
			{
				bool validImage = true;

				if (!pImage->IsVideoRelayImage() || pImage->isMuted())
					validImage = false; // Image is muted

				if (pImage->GetPartyRsrcId() == partyRsrcId && m_pPartyImageVector->size() > 1)
				{
					pSelfViewImage = pImage;
					validImage = false; // Image is self and its not the only image
				}
				if(pImage->GetPartyRsrcId() == partyRsrcId && isCascadeLink)
				{
					//dont't include the party image for cascade SVC
					TRACEINTO<<"For cascade party we dont send its image";
					pSelfViewImage = pImage;
					validImage = false; // Image is self and its not the only image
				}

				if( validImage )// save valid images only
				{
					listImages.push_back(pImage);
				}
			}
        }
	}

	if(listImages.size() == 0 && m_pPartyImageVector->size() > 1 && (pSelfViewImage && (! (pSelfViewImage->isMuted()))) )
	{
		if(!isCascadeLink)
			listImages.push_back(pSelfViewImage);
		else
		{
			TRACEINTO<<"For cascade party we dont send its image";
		}
	}
}

//--------------------------------------------------------------------------
const CImage* CVideoBridgeCP::GetImageBySsrc(unsigned int ssrc) const
{
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();

	for (CPartyImageVector::iterator _ii = m_pPartyImageVector->begin(); _ii != m_pPartyImageVector->end(); ++_ii)
	{
		DWORD partyRscId = *_ii;
		if (partyRscId)
		{
			CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
			PASSERTSTREAM(!pImage, "CVideoBridgeCP::GetImageBySsrc - Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);

			if (pImage && pImage->IsVideoRelayImage())
			{
				CVideoRelayMediaStreamList videoMediaStreamsList = pImage->GetVideoRelayMediaStreamsList();
				for (CVideoRelayMediaStreamList::iterator _ir = videoMediaStreamsList.begin(); _ir != videoMediaStreamsList.end(); ++_ir)
				{
					if ((*_ir)->GetSsrc() == ssrc)
						return pImage;
				}
			}
		}
	}
	TRACEINTO << "Failed, Image with ssrc = "<< ssrc << " not found in image vector";
	return NULL;
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfUpdateVideoMuteCONNECTED(CSegment* pParam)
{
	WORD srcReq = 0;
	EOnOff eOnOff = eOff;
	EMediaDirection eMediaDirection = eNoDirection;
	CVideoBridgePartyCntl* pPartyCntl = NULL;

	*pParam >> srcReq;

	if (srcReq == OPERATOR)
	{
		char name[H243_NAME_LEN];
		*pParam >> name >> (BYTE&)eOnOff >> (BYTE&)eMediaDirection;
		name[H243_NAME_LEN-1] = '\0';

		TRACEINTO << "ConfName:"         << m_pConfName
		          << ", PartyName:"      << name
		          << ", MediaDirection:" << MediaDirectionToString(eMediaDirection)
		          << ", OnOff:"          << (WORD)eOnOff
		          << ", SrcReq:"         << srcReq;

		pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(name);
		// VNGSWIBM-901
		if (!pPartyCntl && (eProductTypeSoftMCUMfw == CProcessBase::GetProcess()->GetProductType()))
		{
			RequestPriority reqPrio = GetRequestPriority(srcReq);
			UpdateDBOnPartyMuteStateWhenThereIsNoInstaceInVB(name, eOnOff, reqPrio);
			return;
		}
	}
	else
	{
		PartyRsrcID partyId = INVALID;
		*pParam >> partyId >> (BYTE&)eOnOff >> (BYTE&)eMediaDirection;

		TRACEINTO << "ConfName:"         << m_pConfName
		          << ", PartyId:"        << partyId
		          << ", MediaDirection:" << MediaDirectionToString(eMediaDirection)
		          << ", OnOff:"          << (WORD)eOnOff
		          << ", SrcReq:"         << srcReq;

		pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	}

	PASSERT_AND_RETURN(eMediaDirection != eMediaIn);
	PASSERT_AND_RETURN(!pPartyCntl);

	const CImage* pImage = pPartyCntl->GetPartyImage();
	PASSERT_AND_RETURN(!pImage);

	RequestPriority reqPrio = GetRequestPriority(srcReq);

	BYTE previouselyMuted = pImage->isMuted(); // before updating we get previous
	pPartyCntl->UpdateSelfMute(reqPrio, eOnOff);
	BYTE currentlyMuted = pImage->isMuted();   // after updating we get current

	if (previouselyMuted == currentlyMuted)
		return;

	//Bridge-13188: Update DB on video mute/unmute from EMA in order to be sent via the NOTIFY message
	if (eOnOff)
		NotifyOnRemovedVideoInStreams(pPartyCntl->GetPartyRsrcID());
	else
		NotifyOnAddedVideoInStreams(pPartyCntl->GetPartyRsrcID());

	// Change Layout Improvement - Conf Layout (CL-CL)
	BuildConfLayout();
	// check that changed layouts' ID vector is empty before loop (CL-SM)
	VerifyIdVectorIsEmpty();

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
		if (eOnOff == eOff)
				pPartyCntl->UnMuteImage();
		else
				pPartyCntl->MuteImage();
	}
	}

	// Change Layout Improvement - Layout Shared Memory (CL-SM)
	SendChangeLayoutToMultipleParties();

	// Send all the required intra request of relay participants
	m_intraDB.Flush();

	if (currentlyMuted)
		ApplicationActionsOnRemovePartyFromMix(pPartyCntl);
	else
		ApplicationActionsOnAddPartyToMix(pPartyCntl);
}

//------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateDBOnPartyMuteStateWhenThereIsNoInstaceInVB(char name[H243_NAME_LEN],EOnOff eOnOff,RequestPriority reqPrio)
{
	TRACEINTO << "PartyName:" << name;

	CCommConf* pCommConf = (CCommConf*)(GetConf())->GetCommConf();
	PASSERT_AND_RETURN(!pCommConf);

	CConfParty* pConfParty   = pCommConf->GetCurrentParty(name);
	PASSERT_AND_RETURN(!pConfParty);

	WORD bIsMute = TRUE;
	bIsMute = (eOnOff == eOff) ? FALSE : TRUE;
	switch (reqPrio)
	{
		case MCMS_Prior:
		{
			pConfParty->SetVideoMuteByMCU(bIsMute);
			break;
		}

		case OPERATOR_Prior:
		{
			pConfParty->SetVideoMuteByOperator(bIsMute);
			break;
		}

		case PARTY_Prior:
		{
			pConfParty->SetVideoMuteByParty(bIsMute);
			break;
		}

		default:
		{
			PASSERT(reqPrio);
			return; /* no need to send updateDB */
		}
	} // switch
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfVideoRefreshCONNECTED(CSegment* pParam)
{
	PartyRsrcID partyId = INVALID;
	*pParam >> partyId;

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << partyId;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	pPartyCntl->FastUpdate();
}

// ------------------------------------------------------------------------------------------
CLayout* CVideoBridgeCP::GetReservationLayout(void) const
{
  return m_pReservation[m_layoutType];
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::RemovePartyFromConfMixBeforeDisconnecting(const CTaskApp* pParty)
{
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(pParty->GetPartyId());
	PASSERT_AND_RETURN(!pPartyCntl);

  for (DWORD j = (int) CP_LAYOUT_1X1; j < (int) CP_NO_LAYOUT; j++)
    m_pReservation[j]->RemovePartyForce(pPartyCntl->GetName(), PARTY_Prior);

  // make that all other parties can`t see the deleted source
	if (m_pAutoScanParams)
		m_pAutoScanParams->RemoveImageFromAutoScanVector(pParty->GetPartyId());

	NotifyOnRemovedVideoInStreams(pParty->GetPartyId());

	RemoveImageFromIntraDB(pParty->GetPartyId());

	BYTE result = DelPartyImage(pParty->GetPartyId());
	if (result == true)
	{
		PTRACE2(eLevelInfoHigh, "CVideoBridgeCP::RemovePartyFromConfMixBeforeDisconnecting  - AVC Notifies Others, ConfName:", pPartyCntl->GetFullName());
		UpdateNumberOfNonRelayImagesIfNeeded(pPartyCntl, false/*isAddImage*/);	// only if succeeded to del party from list we reduce the counter

		ApplicationActionsOnRemovePartyFromMix(pPartyCntl);

		//Change Layout Improvement - Conf Layout (CL-CL)
		BuildConfLayout();

		BOOL isPartyLayoutIdenticalToConfLayout = FALSE;
		// check that changed layouts' ID vector is empty before loop (CL-SM)
		VerifyIdVectorIsEmpty();

		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		{
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntl* pCurrParty = (CVideoBridgePartyCntl*)_itr->second;
			if (pCurrParty)
			{
		  if (pCurrParty != pPartyCntl) // Delete Image From all other parties in conf
		  {
			isPartyLayoutIdenticalToConfLayout = IsPartyLayoutIdenticalToConfLayoutAfterBuild(pCurrParty);	//Change Layout Improvement - Conf Layout (CL-CL)
			if (!isPartyLayoutIdenticalToConfLayout)
			{
				pCurrParty->DelImage(pParty);
			}
		  }
		  else
		  {
			if (pPartyCntl->GetDisconnectingDirectionsReq() == eMediaIn)
			  pCurrParty->DelImage(pParty);
		  }
		}
		}

		//Change Layout Improvement - Layout Shared Memory (CL-SM)
		SendChangeLayoutToMultipleParties();

		//Send all the required intra request of relay participants
		m_intraDB.Flush();
	}

  UpdateDB_ConfLayout();
}

// ------------------------------------------------------------------------------------------
//Change Layout Improvement - Conf Layout (CL-CL):
// Check if party sees Conf Layout - comparative check
// we compare the current party layout (before change) with the conf layout after change.
// In case of a difference - an update should be sent.
BOOL CVideoBridgeCP::IsPartyLayoutIdenticalToConfLayoutAfterBuild(CVideoBridgePartyCntl* pCurrParty)
{
	if (pCurrParty && !pCurrParty->IsVideoRelayParty() )
	{
		CLayout* currentLayout = pCurrParty->GetCurrentLayout();
		if ( currentLayout && (*currentLayout == *m_ConfLayout) )
			return TRUE;
	}
	return FALSE;
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::RemovePartyFromAnyConfSettingsWhenDeletedFromConf(const char* pDeletedPartyName)
{
  for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; i++)
    m_pReservation[i]->RemovePartyForce(pDeletedPartyName);

  UpdateDB_ConfLayout();

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->DeletePartyFromConf(pDeletedPartyName);
  }

  ApplicationActionsOnDeletePartyFromConf(pDeletedPartyName);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::ConnectParty(CVideoBridgePartyCntl*  pBridgePartyCntl, BYTE isIVR)
{
	if (0 == GetNumParties() &&  IsRelaySupported())
	{
		if (IsEQ())	// temporary solution for BRIDGE-5283 (amir)
		{
			TRACEINTO << "ConfName:" << m_pConfName << " - Not adding VideoOperationPointSet to EQ";
		}
		else
		{
			TRACEINTO << "ConfName:" << m_pConfName << " - First party on relay conference need to send add video operation point";
			SendAddVideoOperationPointSet();
		}
	}
	AddAVMCUPartyCntlToBridge(pBridgePartyCntl);
	CVideoBridge::ConnectParty(pBridgePartyCntl, isIVR);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::AddPartyToConfMixAfterVideoInSynced(const CTaskApp* pParty)
{
  AddPartyToConfMix(pParty);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::AddPartyToConfMix(const CTaskApp* pParty)
{
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = ((CParty*)pParty)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	TRACECOND_AND_RETURN(pPartyCntl->GetPartyResumeFromHoldInIVR(), "PartyId:" << partyId << " - Party is resuming the call in IVR mode, will be added after completing IVR");

	const CImage* pImage = pPartyCntl->GetPartyImage();
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pImage));

	TRACEINTO << "PartyId:" << partyId;

	AddPartyImage(pParty->GetPartyId());

//	if( pPartyCntl->IsVideoRelayParty() ) // BRIDGE-6417
	AddImageToIntraDB((CImage*)pImage);	  // adding to intra list, must be called for AVC too, because of MIXED conf

	UpdateNumberOfNonRelayImagesIfNeeded(pPartyCntl, true/*isAddImage*/);

	NotifyOnAddedVideoInStreams(pParty->GetPartyId());

	//eFeatureRssDialin
	//Don't change layout if it's locked for RL
	//if ((!pImage->isMuted()) || pPartyCntl->IsRecordingLinkParty())
	if (((!pImage->isMuted())&&(!pPartyCntl->IsRecordingLinkParty()) )
		|| ((pPartyCntl->IsRecordingLinkParty())&&(!pPartyCntl->IsLastLayoutForRLLocked())))
	{
		ApplicationActionsOnAddPartyToMix(pPartyCntl);

		//Change Layout Improvement - Conf Layout (CL-CL)
		BuildConfLayout();

		// check that changed layouts' ID vector is empty before loop (CL-SM)
		VerifyIdVectorIsEmpty();

		//BRIDGE-4043
		WORD sizeOfVectorWithoutMute = GetSizeOfImageVectorWithoutMute();

		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		{
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
			if (pPartyCntl)
			{
				BOOL isPartyLayoutIdenticalToConfLayout = IsPartyLayoutIdenticalToConfLayoutAfterBuild(pPartyCntl); //Change Layout Improvement - Conf Layout (CL-CL)
				//VNGFE-7016: in case this is the second party in conf - AddImage should be sent to the first one too (because we should change the self view)
				// VNGFE-7072: if party see private layout, and forced (by MLA) to unsynced image isPartyLayoutIdenticalToConfLayout return true, but we must use AddImage
				WORD isPartySeePrivateLayout = pPartyCntl->GetIsPrivateLayout();
				if ((sizeOfVectorWithoutMute <= 2) || (!isPartyLayoutIdenticalToConfLayout) || isPartySeePrivateLayout)
				{
					pPartyCntl->AddImage(pParty);
				}
			}
		}

		//Change Layout Improvement - Layout Shared Memory (CL-SM)
		SendChangeLayoutToMultipleParties();
		//Send all the required intra request of relay participants
		m_intraDB.Flush();
	}

	// if the party that connects is the last video speaker we will send video speaker again
	if (pPartyCntl->GetPartyTaskApp() == m_pLastActiveVideoSpeakerRequest)
		ResendLastActiveVideoSpeakerRequest();
}

// ------------------------------------------------------------------------------------------
WORD CVideoBridgeCP::GetSizeOfImageVectorWithoutMute()
{
	WORD sizeOfVectorWithoutMute = 0;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			const CImage* pImage = pPartyCntl->GetPartyImage();
		if (!pImage)
		{
				TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Image of party is Null";
			continue;
		}
		if (!pImage->isMuted())
			++sizeOfVectorWithoutMute;
		}
	}


	TRACEINTO << "Size:" << sizeOfVectorWithoutMute;
	return sizeOfVectorWithoutMute;
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateDB_ConfLayout()
{
  CSegment vidLayoutSeg;
  if (!m_pReservation[m_layoutType]->Serialize(CONF_lev, &vidLayoutSeg))
    m_pConfApi->UpdateDB((CTaskApp*) 0xffff, CPCONFLAYOUT, (DWORD) 0, 0, &vidLayoutSeg);
  else
    DBGPASSERT(1);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::AudioSpeakerChanged(CTaskApp* pNewAudioSpeaker)
{
  // Set the parameter m_pLastActiveAudioSpeakerRequest to indicate the Speaker Notation before Layout change
  m_pLastActiveAudioSpeakerRequest = pNewAudioSpeaker;

  if (NULL == pNewAudioSpeaker) // Last speaker in conf
    m_pConfApi->UpdateDB(pNewAudioSpeaker, NOAUDIOSRC, 0);
  else
    m_pConfApi->UpdateDB(pNewAudioSpeaker, AUDIOSRC, 0);

  //Change Layout Improvement - Conf Layout (CL-CL)
  BuildConfLayout();

  // If Audio Only party new speaker or party that is not yet connected,
  // the ChangeSpeaker will only cause removal of SpeakerNotation
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->ChangeAudioSpeaker(m_pLastActiveAudioSpeakerRequest);
  }
  //Change Layout Improvement - Layout Shared Memory (CL-SM)--> results in change layout attributes only
  ///TRACEINTO << "**CL-SM: CVideoBridgeCP::AudioSpeakerChanged - SendChangeLayoutToMultipleParties";	///DEBUG***
  ///SendChangeLayoutToMultipleParties();

  //Send all the required intra request of relay participants
  m_intraDB.Flush();

  ApplicationActionsOnAudioSpeakerChange();
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfSpeakersChangedCONNECTED(CSegment* pParam)
{
	CTaskApp* pNewVideoSpeaker = NULL;
	CTaskApp* pFormerVideoSpeaker = m_pLastActiveVideoSpeakerRequest;
	char siteName[MAX_SITE_NAME_ARR_SIZE];
	CTaskApp* pNewAudioSpeaker = NULL;
	BYTE shouldResend = NO;
	bool isMSAudioOnly = false;
	LyncMsi speakerAudioMsi = DUMMY_DOMINANT_SPEAKER_MSI;
	LyncMsi speakerVideoMsi = DUMMY_DOMINANT_SPEAKER_MSI;

	*pParam >> (void*&)pNewVideoSpeaker >> (void*&)pNewAudioSpeaker >> shouldResend >> speakerAudioMsi;

	#undef PARAMS
	#define PARAMS "ConfName:" << m_pConfName << ", ShouldResend:" << (int)shouldResend << ", SpeakerAudioMsi:" << speakerAudioMsi

	if (shouldResend == YES)
	{
		m_pLastActiveVideoSpeakerRequest = NULL;
		m_pLastActiveAudioSpeakerRequest = NULL;
	}

	if ((m_pLastActiveAudioSpeakerRequest == pNewAudioSpeaker) && (m_pLastActiveVideoSpeakerRequest == pNewVideoSpeaker))
	{
		if (!IsValidMSI(speakerAudioMsi))
		{
			TRACEINTO << PARAMS << " - Failed, speaker does not changed";
			return;
		}
	}


	if (m_pAVMCUMngr)
	{
		if (IsValidMSI(speakerAudioMsi, false))
		{
			if (speakerAudioMsi == m_pAVMCUMngr->GetMSAVMCULocalAUdioMSI())
			{
				TRACEINTO << PARAMS << " - Speaker Audio msi is the same as local Audio msi, ignore change speaker request";
				return;
			}
			CVideoBridgePartyCntl* pAVMCUMasterPartyCntl = NULL;
			if (pNewVideoSpeaker)
				pAVMCUMasterPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(((CParty*)pNewVideoSpeaker)->GetPartyRsrcID());
			if(!pAVMCUMasterPartyCntl)
			{
				TRACEINTO << PARAMS << " Valid Audio MSI from party not connected to Video Bridge, ignore and zero Msi";
				OFF(speakerAudioMsi);
			}
			else if (!pAVMCUMasterPartyCntl->IsAVMCUMain())
			{
				PASSERT(speakerAudioMsi);
				TRACEINTO << PARAMS << " Valid Audio MSI not from AV-MCU Main, ignore and zero Msi";
				OFF(speakerAudioMsi);
			}

			else if (m_pAVMCUMngr->GetVideoSwitchingModeType() == EventPackage::eVideoSwitchingModeType_ManualSwitched && IsValidMSI(m_pAVMCUMngr->GetSpotLightSpeakerMsi()))
			{
				TRACEINTO << PARAMS << " Valid Audio MSI during spotlight session with connected spotlight speaker. Make always the Maser as the speaker";
				OFF(speakerAudioMsi);
			}
		}
	}
	else
	{
		if (IsValidMSI(speakerAudioMsi))
		{
			TRACEINTO << PARAMS << " Valid Audio MSI when no AV-MCU call is active, ignore and zero Msi";
			OFF(speakerAudioMsi);
		}
	}

	if (IsValidMSI(speakerAudioMsi))
	{
		if (m_pAVMCUMngr)
		{
			speakerVideoMsi = EventPackage::ApiLync::Instance().GetCorrelativeMSI(m_pAVMCUMngr->GetMasterPartyRsrcID(), speakerAudioMsi, EventPackage::eMediaType_Video);

			EventPackage::MediaInfo info;
			EventPackage::Manager::Instance().GetMedia(m_pAVMCUMngr->GetMasterPartyRsrcID(), std::bind2nd(EventPackage::Predicate::Media_Msi(), speakerVideoMsi), info);
			if (!info.media || !IsValidMSI(speakerVideoMsi))
			{
				PASSERTSTREAM(true, PARAMS << " - Correlative video media is not found");
				isMSAudioOnly = true;
			}
			else
			{
				if (info.media->m_status != EventPackage::eMediaStatusType_SendOnly && info.media->m_status != EventPackage::eMediaStatusType_SendRecv)
				{
					isMSAudioOnly = true;
					TRACEINTO << PARAMS << ", SpeakerVideoMsi:" << speakerVideoMsi << ", MediaStatus:" << info.media->m_status << " - Media status is inactive";
				}
				else
				{
					TRACEINTO << PARAMS << ", SpeakerVideoMsi:" << speakerVideoMsi << ", MediaStatus:" << info.media->m_status;
					BuildAVMCUSiteName(speakerVideoMsi, siteName);
				}
			}
		}
		else
		{
			PASSERTSTREAM(true, PARAMS);
		}
	}

	// The order of check is important
	// 1.Last party in conference and in IVR
	if ((NULL == pNewAudioSpeaker) && (NULL == pNewVideoSpeaker))
	{
		TRACEINTO << PARAMS << " - Failed, no speaker in conference";
		AudioSpeakerChanged(pNewAudioSpeaker); // Remove Notation and UpdateDB (remove EMA icon from last speaker)
	}

	// 2.New VideoParty become the Audio or Video Speaker
	// A.VideoSpeaker changed
	// B.VideoSpeaker didn`t changed but is also the Audio speaker now
	else if ((((m_pLastActiveVideoSpeakerRequest != pNewVideoSpeaker) || ((m_pLastActiveAudioSpeakerRequest != pNewAudioSpeaker) && (pNewVideoSpeaker == pNewAudioSpeaker))) || IsValidMSI(speakerAudioMsi)) && !isMSAudioOnly)
	{
		if (IsValidMSI(speakerAudioMsi))
		{
			CVideoBridgePartyCntl* pAVMCUMasterPartyCntl = NULL;
			if (pNewVideoSpeaker)
				pAVMCUMasterPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(((CParty*)pNewVideoSpeaker)->GetPartyRsrcID());
			PASSERT_AND_RETURN(!pAVMCUMasterPartyCntl);
			if (pAVMCUMasterPartyCntl->IsAVMCUMain())
			{
				if (shouldResend)
					speakerVideoMsi = speakerAudioMsi;

				PartyRsrcID partyImageSpeakerId = 0;
				CTaskApp* pAVMCUParty = NULL;
				CVideoBridgePartyCntl* pNewAVMCUSpkrPartyCntl = NULL;
				CImage* pImage = GetPartyImageByVideoMSI(speakerVideoMsi, partyImageSpeakerId);

				if (pImage == NULL)
				{
					TRACEINTO << PARAMS << ", SpeakerVideoMsi:" << speakerVideoMsi << " - Video msi of new speaker does not belong to any AV-MCU party control, replace one of the msi of them";
					DWORD repalcedMSIPartyRsrcID = GetOldestAVMCUSpeaker();
					pNewAVMCUSpkrPartyCntl = (CVideoBridgePartyCntl*) GetPartyCntl(repalcedMSIPartyRsrcID);
					PASSERT_AND_RETURN(!pNewAVMCUSpkrPartyCntl);

					pNewAVMCUSpkrPartyCntl->SetVideoMSI(speakerVideoMsi);
					//pNewAVMCUSpkrPartyCntl->SetSiteName(siteName);
					SetSiteName(pNewAVMCUSpkrPartyCntl->GetName(), siteName);

					// romem
					partyImageSpeakerId = repalcedMSIPartyRsrcID;
					// Send VSR to Main
					SendVSRToAVMCUMain();
				}
				else
				{
					pNewAVMCUSpkrPartyCntl = (CVideoBridgePartyCntl*) GetPartyCntl(partyImageSpeakerId);
					PASSERT_AND_RETURN(!pNewAVMCUSpkrPartyCntl);

					TRACEINTO << PARAMS << ", SpeakerVideoMsi:" << speakerVideoMsi << ", " << pNewAVMCUSpkrPartyCntl->GetFullName();

					partyImageSpeakerId = pNewAVMCUSpkrPartyCntl->GetPartyRsrcID();
				}
				pAVMCUParty = (CTaskApp*) GetLookupTableParty()->Get(partyImageSpeakerId);
				PASSERT_AND_RETURN(!pAVMCUParty);

				pNewVideoSpeaker = pAVMCUParty;
				pNewAudioSpeaker = pAVMCUParty;
			}
			else
			{
				TRACEINTO << PARAMS << " - Is not AV-MCU Master, treat the speaker indication as a regular one";
				PASSERT(speakerAudioMsi);
			}
		}

		m_pLastActiveVideoSpeakerRequest = pNewVideoSpeaker;

		// Audio Speaker also changed
		if (m_pLastActiveAudioSpeakerRequest != pNewAudioSpeaker)
		{
			m_pLastActiveAudioSpeakerRequest = pNewAudioSpeaker;
			m_pConfApi->UpdateDB(pNewAudioSpeaker, AUDIOSRC, 0);
			ApplicationActionsOnAudioSpeakerChange();
		}

		PartyRsrcID speakerPartyId = m_pLastActiveVideoSpeakerRequest ? ((CParty*)m_pLastActiveVideoSpeakerRequest)->GetPartyRsrcID() : INVALID;

		bool bPartyImageExistInImageVector = (speakerPartyId != INVALID) ? IsPartyImageExistInImageVector(speakerPartyId) : false;

		if (bPartyImageExistInImageVector)  // only when video decoder is fully connected
		{
			TRACEINTO << PARAMS << ", NewSpeakerId:" << speakerPartyId;

			SetPartyImageSpeakerId(speakerPartyId);

			//Change Layout Improvement - Conf Layout (CL-CL)
			BuildConfLayout();

			/*			//VNGR-9344 - When switching in conf with more then 80 parties last parties does not change layout
			 CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			 DWORD       SleepTime  = 0;
			 if (pSysConfig)
			 pSysConfig->GetDWORDDataByKey("SLEEP_VALUE", SleepTime);
			 */
			if (m_pTelepresenceLayoutMngr != NONE)
			{
				OnConfSpeakersChangedWithTelepresence(pFormerVideoSpeaker, m_pLastActiveVideoSpeakerRequest);
			}

			// check that changed layouts' ID vector is empty before loop (CL-SM)
			VerifyIdVectorIsEmpty();

			CBridgePartyList::iterator _end = m_pPartyList->end();
			for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
			{
				// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
				CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
				if (pPartyCntl)
				 {
					// speaker change for cascade links already handled by 'OnConfSpeakersChangedWithTelepresence'
					if ((m_pTelepresenceLayoutMngr != NONE) && (pPartyCntl->GetTelepresenceInfo().GetLinkRole() != 0))
						continue;

					pPartyCntl->ChangeSpeakers(pNewVideoSpeaker, pNewAudioSpeaker);
				}
			}

			//Send all the required intra request of relay participants
			m_intraDB.Flush();

			//Change Layout Improvement - Layout Shared Memory (CL-SM)
			SendChangeLayoutToMultipleParties();
		}
		else
			// in case the participant is not connected + synced yet,we save last active speaker request
			// and resend it after the party connects in function AddPartyToConfMixAfterVideoInSynced
			TRACEINTO << PARAMS << " - Failed, New video speaker is not connected";
	}

	// 3.Only Audio speaker changed
	else
	{
		if (m_pLastActiveAudioSpeakerRequest != pNewAudioSpeaker || isMSAudioOnly)
		{

			if (isMSAudioOnly) // AV-MCU MS Audio Only
			{
				AudioSpeakerChanged(NULL);
				TRACEINTO << PARAMS << " - Video speaker didn`t changed only new Audio speaker was recognized on AV-MCU side";
			}
			else
			{
				AudioSpeakerChanged(pNewAudioSpeaker);
				TRACEINTO << PARAMS << " - Video speaker didn`t changed only new Audio speaker selected";
			}
		}
		else
		{
			TRACEINTO << PARAMS << " - Failed, speaker does not selected";
		}
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfSpeakersChangedWithTelepresence(CTaskApp* pFormerVideoSpeaker, CTaskApp* pNewVideoSpeaker)
{
	PASSERT_AND_RETURN(!m_pTelepresenceLayoutMngr);

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pNewVideoSpeaker));
	CVideoBridgePartyCntl* pNewSpeakerPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(((CParty*)pNewVideoSpeaker)->GetPartyRsrcID());
	PASSERT_AND_RETURN(!pNewSpeakerPartyCntl);

	CVideoBridgePartyCntl* pFormerSpeakerPartyCntl = NULL;
	if (CPObject::IsValidPObjectPtr(pFormerVideoSpeaker))
	{
		pFormerSpeakerPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(((CParty*)pFormerVideoSpeaker)->GetPartyRsrcID());
		PASSERT_AND_RETURN(!pFormerSpeakerPartyCntl);
	}

	//BRIDGE-826
	TRACEINTO << "ITPSiteName:" << pNewSpeakerPartyCntl->GetITPSiteName() << ", FormerITPSiteName:" << (pFormerSpeakerPartyCntl ? pFormerSpeakerPartyCntl->GetITPSiteName() : "<no speaker set>");
	DWORD new_speaker_room_id = pNewSpeakerPartyCntl->GetTelepresenceInfo().GetRoomID();
	DWORD new_num_links, old_num_links;
	eTelePresencePartyType new_speaker_type, old_speaker_type;

	CRoomInfo old_speaker_room = m_pTelepresenceLayoutMngr->GetRoomInfo(pFormerSpeakerPartyCntl ? pFormerSpeakerPartyCntl->GetTelepresenceInfo().GetRoomID() : (DWORD)-1);
	BOOL is_found = old_speaker_room.GetRoomId() != (RoomID)-1;
	old_speaker_type = (is_found) ? old_speaker_room.GetRoomEPType() : eTelePresencePartyInactive;
	old_num_links = (is_found) ? old_speaker_room.GetNumberOfActiveLinks() : 0;
	//TODO
	CRoomInfo new_speaker_room = m_pTelepresenceLayoutMngr->GetRoomInfo(pNewSpeakerPartyCntl ? pNewSpeakerPartyCntl->GetTelepresenceInfo().GetRoomID() : (DWORD)-1);
	BOOL is_new_found = new_speaker_room.GetRoomId() != (RoomID)-1;
	if (!is_new_found && (pNewSpeakerPartyCntl != NULL))
	{
		new_speaker_room = m_pTelepresenceLayoutMngr->GetRoomInfo(pNewSpeakerPartyCntl->GetITPSiteName(), pNewSpeakerPartyCntl->GetTelepresenceInfo().GetEPtype());
		is_new_found = new_speaker_room.GetRoomId() != (RoomID)-1;
	}
	if (new_speaker_room.GetIsAckPendingForRealTPConnections())
	{
		TRACEINTO << "Speaker room id " << new_speaker_room.GetRoomId() << " is pending new sublink connections. speaker change will execute after the timer";
		return;
	}
	new_speaker_type = (is_new_found)? new_speaker_room.GetRoomEPType() : eTelePresencePartyInactive;
	BOOL has_speaker_EP_type_changed = new_speaker_type != old_speaker_type;
	new_num_links =  (is_new_found)? new_speaker_room.GetNumberOfActiveLinks() : 0;

	BOOL has_speaker_num_links_changed = new_num_links != old_num_links;
	BOOL itp_timer_needed_for_any = FALSE;
	if (m_pTelepresenceLayoutMngr != NULL)
		TRACEINTO << "Speaker change-  speaker room id " << new_speaker_room_id << " with " << new_num_links << " links, prev speaker had " << old_num_links << " links.";

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			const CTelepresenseEPInfo &curr_info = pPartyCntl->GetTelepresenceInfo();

			// speaker change for cascade links already handled by 'OnConfSpeakersChangedWithTelepresence'
			if (curr_info.GetLinkRole() != 0)
			{
				BOOL itp_timer_needed = FALSE;
				RoomID viewer_room_id = curr_info.GetRoomID();
				// for link parties, we act only on the main link party.
				if (curr_info.GetLinkNum() != 0)
					continue;

				if (!is_new_found)
				{
					if(pNewSpeakerPartyCntl)
						TRACEINTO << "PartyId:" << pNewSpeakerPartyCntl->GetPartyRsrcID() << ", ITPSiteName:" << pNewSpeakerPartyCntl->GetITPSiteName() << " - Multi-cascade speaker change cannot be done, no speaker room found for party";
					else
						PASSERT(1);
					continue;
				}

				CRoomInfo viewer_room;
				if(m_pTelepresenceLayoutMngr)
					viewer_room = m_pTelepresenceLayoutMngr->GetRoomInfo(viewer_room_id);
				else
					PASSERT(1);

				if (viewer_room.GetRoomId() == (RoomID)-1)
				{
					TRACEINTO << "PartyId:" << pNewSpeakerPartyCntl->GetPartyRsrcID() << ", ITPSiteName:" << pNewSpeakerPartyCntl->GetITPSiteName() << " - Multi-cascade Speaker change cannot be done, no viewer room found for party";
					continue;
				}
				// requires change to cascade multi link setting. actual layout change is delayed until receiving ack from the remote MCU (or after the timer)
				if ((new_speaker_type != curr_info.GetEPtype()) || (new_num_links != viewer_room.GetNumberOfActiveLinks()))
				{
					TRACEINTO << "Multi-cascade Speaker change- Sending new speaker ind to party id " << pPartyCntl->GetPartyRsrcID() << " with " << new_num_links << " links, and EP type " << (WORD)new_speaker_type;
					CSegment* partySeg = new CSegment;
					*partySeg << (DWORD)new_num_links << (BYTE)new_speaker_type;
					m_pConfApi->SendResponseMsg(pPartyCntl->GetPartyTaskApp(), VIDEO_BRIDGE_MSG, NEW_ITP_SPEAKER_IND, STATUS_OK, TRUE, partySeg);
					POBJDELETE(partySeg);

					// found at least one multi-link that requires ITP links changed- timer will be necessary to wait for remote link ACKs
					itp_timer_needed = TRUE;
					// update room info
					UpdateRoomPartiesITPTypeChanged(viewer_room, new_num_links, new_speaker_type,itp_timer_needed);
				}
				else
					{
						// no need to change the link status (active/inactive)- just rebuild the link party's layout.
						if(m_pTelepresenceLayoutMngr)
							m_pTelepresenceLayoutMngr->OnSpeakerChanged(viewer_room_id, new_speaker_room_id);
						else
							PASSERT(1);
					}
				// update the flag so the timer will start after all parties have been updated.
				itp_timer_needed_for_any |= itp_timer_needed;
			}
		}
	}

	if (itp_timer_needed_for_any)
	{
		CSegment* pSeg = new CSegment;
		*pSeg << new_speaker_room_id;
		StartTimer(NEW_ITP_SPEAKER_TIMER, NEW_ITP_SPEAKER_TIME_OUT_VALUE, pSeg);
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateNewSpeakerIndReceivedFromRemoteMCU(PartyRsrcID partyId, DWORD numOfActiveLinks, BYTE itpType)
{
	CVideoBridgePartyCntl* pITPCascadePartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pITPCascadePartyCntl, "PartyId:" << partyId);

	RoomID room_id = pITPCascadePartyCntl->GetTelepresenceInfo().GetRoomID();

	TRACEINTO << "PartyId:" << partyId << ", NumOfActiveLinks:" << numOfActiveLinks << ", ItpType:" << (WORD)itpType << ", RoomId:" << room_id;

	bool link_is_speaker = false;
	if (m_pLastActiveVideoSpeakerRequest && m_pLastActiveVideoSpeakerRequest->GetPartyId() == partyId)
		link_is_speaker = true;

	CRoomInfo room_to_update = m_pTelepresenceLayoutMngr->GetRoomInfo(room_id);

	std::ostringstream msg;
	room_to_update.Dump(msg);
	TRACEINTO << msg.str().c_str();

	UpdateRoomPartiesITPTypeChanged(room_to_update, numOfActiveLinks, (eTelePresencePartyType)itpType);

	CRoomInfo updated_room = m_pTelepresenceLayoutMngr->GetRoomInfo(room_id);

	// if current speaker changed types, propagate change to all other speakers
	if (link_is_speaker)
		UpdateAllCascadeRoomPartiesSpeakerITPTypeChanged(updated_room);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateNewSpeakerAckIndReceivedFromRemoteMCU(PartyRsrcID partyId)
{
	TRACEINTO << "PartyId:" << partyId;
	// check if v. bridge has a current video speaker party to use
	PASSERTSTREAM_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pLastActiveVideoSpeakerRequest), "Speaker party not found");

	CVideoBridgePartyCntl* pITPCascadePartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pITPCascadePartyCntl, "PartyId:" << partyId << " - Viewer party not found");

	PartyRsrcID speakerPartyId = ((CParty*)m_pLastActiveVideoSpeakerRequest)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pVideoSpeakerPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(speakerPartyId);
	PASSERTSTREAM_AND_RETURN(!pVideoSpeakerPartyCntl, "SpeakerPartyId:" << speakerPartyId << " - Speaker party not found");

	m_pTelepresenceLayoutMngr->OnSpeakerChanged(pITPCascadePartyCntl->GetTelepresenceInfo().GetRoomID(), pVideoSpeakerPartyCntl->GetTelepresenceInfo().GetRoomID());
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateAllCascadeRoomPartiesSpeakerITPTypeChanged(const CRoomInfo& updated_room)
{
	PASSERTSTREAM_AND_RETURN(!m_pLastActiveVideoSpeakerRequest, "Video speaker party not found");

	const VBRIDGE_ROOM_INFO_MAP& current_rooms = m_pTelepresenceLayoutMngr->GetRoomList();

	PartyRsrcID speakerPartyId = ((CParty*)m_pLastActiveVideoSpeakerRequest)->GetPartyRsrcID();
	CVideoBridgePartyCntl* pVideoSpeakerPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(speakerPartyId);
	PASSERTSTREAM_AND_RETURN(!pVideoSpeakerPartyCntl, "SpeakerPartyId:" << speakerPartyId << " - Speaker party not found");

	BOOL itp_timer_needed = FALSE;

	std::ostringstream msg;
	updated_room.Dump(msg);
	TRACEINTO << msg.str().c_str();

	BYTE link_num = updated_room.GetNumberOfActiveLinks();
	eTelePresencePartyType epType =  updated_room.GetRoomEPType();
	TRACEINTO << "This check is for assuring that we have number of active links needed: " << (DWORD)link_num ;
	for (VBRIDGE_ROOM_INFO_MAP::const_iterator cit = current_rooms.begin(); cit != current_rooms.end(); cit++)
	{
		const CRoomInfo &viewer_room = cit->second;

		ostringstream msg1;
		viewer_room.Dump(msg1);
		TRACEINTO << msg1.str().c_str();
		// not cascade room or the speaker room
		if (!viewer_room.GetIsCascade() || (viewer_room.GetRoomId() == updated_room.GetRoomId()))
		{
			//This change for manage layout internally
			CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
			if (!CPObject::IsValidPObjectPtr(pCommConf))
			{
				PASSERTMSG(TRUE,"CVideoBridgeCP::UpdateAllCascadeRoomPartiesSpeakerITPTypeChanged: pCommConf is NULL");
				continue;
			}

			if (pCommConf->GetManageTelepresenceLayoutInternaly())
			{
				BYTE rShouldResend = YES;
				CVideoBridgePartyCntl* plinkPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(updated_room.GetLinkPartyId(0)));
				if (!CPObject::IsValidPObjectPtr(plinkPartyCntl))
				{
					PTRACE2INT(eLevelInfoHigh, "CVideoBridgeCP::UpdateAllCascadeRoomPartiesSpeakerITPTypeChanged: viewer party main link not found for room ID ", viewer_room.GetRoomId());
					PASSERTMSG(TRUE, "CVideoBridgeCP::UpdateAllCascadeRoomPartiesSpeakerITPTypeChanged: - viewer party main link not found for ITP type change");
					continue;
				}
				CTaskApp* pToTaskApp = plinkPartyCntl->GetPartyTaskApp();
				if (!pToTaskApp)
				{
					PASSERTMSG(TRUE, "CVideoBridgeCP::UpdateAllCascadeRoomPartiesSpeakerITPTypeChanged: - TaskApp doesn't exist for party");
					continue;
				}
				CSegment* pSeg = new CSegment;
				*pSeg << pToTaskApp << pToTaskApp << (BYTE)rShouldResend << plinkPartyCntl->GetVideoMSI();

				DispatchEvent(SPEAKERS_CHANGED, pSeg);
				POBJDELETE(pSeg);
			}
			else continue;
		}

		TRACEINTO << "LinkNum:" << (DWORD)link_num << ", MaxLinks:" << (DWORD)viewer_room.GetMaxLinks() << ", IsCascade:" << (DWORD)viewer_room.GetIsCascade();

		if (link_num>viewer_room.GetMaxLinks() && viewer_room.GetIsCascade())
		{
			PASSERTMSG(TRUE, "CVideoBridgeCP::UpdateAllCascadeRoomPartiesITPTypeChanged - viewer link room doesn't have enough links!");
			continue;
		}

		if ((link_num != viewer_room.GetNumberOfActiveLinks()) || (epType!= viewer_room.GetRoomEPType()))
			itp_timer_needed = TRUE;

		if (viewer_room.GetIsCascade())
			UpdateRoomPartiesITPTypeChanged(viewer_room,link_num,epType);

		CVideoBridgePartyCntl* plinkPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(viewer_room.GetLinkPartyId(0)));
		if (!CPObject::IsValidPObjectPtr(plinkPartyCntl))
		{
			PTRACE2INT(eLevelInfoHigh, "CVideoBridgeCP::UpdateNewSpeakerAckIndReceivedFromRemoteMCU: viewer party main link not found for room ID ", viewer_room.GetRoomId());
			PASSERTMSG(TRUE, "CVideoBridgeCP::UpdateNewSpeakerAckIndReceivedFromRemoteMCU - viewer party main link not found for ITP type change");
			continue;
		}
		CSegment* partySeg = new CSegment;
		*partySeg << (DWORD)link_num << (BYTE)epType;
		m_pConfApi->SendResponseMsg(plinkPartyCntl->GetPartyTaskApp(), VIDEO_BRIDGE_MSG, NEW_ITP_SPEAKER_IND, STATUS_OK, TRUE, partySeg);
		POBJDELETE(partySeg);

		ostringstream msg2;
		viewer_room.Dump(msg2);
		TRACEINTO << msg2.str().c_str();
	}
	if (itp_timer_needed)
	{
		CSegment* pSeg = new CSegment;
		*pSeg << updated_room.GetRoomId();
		StartTimer(NEW_ITP_SPEAKER_TIMER, NEW_ITP_SPEAKER_TIME_OUT_VALUE, pSeg);
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateRoomPartiesITPTypeChanged(const CRoomInfo& target_room, DWORD numOfActiveLinks, eTelePresencePartyType itpType, BOOL isAckPending)
{
	BYTE old_numActiveLinks = target_room.GetNumberOfActiveLinks();
	BYTE maxLinks = target_room.GetMaxLinks();

	TRACEINTO
		<< "NumActiveLinks: "<< numOfActiveLinks
		<< ", ITPType:" << (DWORD)itpType
		<< ", IsAckPending:" << (DWORD)isAckPending
		<< ", OldNumActiveLinks:" << (DWORD)old_numActiveLinks
		<< ", MaxLinks:" << (DWORD)maxLinks
		<< ", RoomId:" << target_room.GetRoomId()
		<< ", MainLinkPartyName:" << target_room.GetMainLinkPartyName()
		<< ", ITPType:" << (WORD)itpType;

	m_pTelepresenceLayoutMngr->UpdateRoomInfo(target_room.GetRoomId(), numOfActiveLinks, itpType, isAckPending);

	CRoomInfo result = m_pTelepresenceLayoutMngr->GetRoomInfo(target_room.GetRoomId());

	std::ostringstream msg5;
	result.Dump(msg5);
	TRACEINTO << msg5.str().c_str();

	for (BYTE link=0; link < maxLinks; link++)
	{
		DWORD link_party_id = target_room.GetLinkPartyId(link);
		if (link_party_id == (DWORD)-1)
		{
			TRACEINTO << "no party id set for link #" << (WORD)link << " for room id " << target_room.GetRoomId() << "; numOfActiveLinks: " << numOfActiveLinks ;
			continue;
		}
		CVideoBridgePartyCntl* plinkPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(link_party_id));
		if (!CPObject::IsValidPObjectPtr(plinkPartyCntl))
		{
			PTRACE2INT(eLevelInfoHigh, "CVideoBridgeCP::UpdateRoomPartiesITPTypeChanged: viewer party not found for party ID ", link_party_id);
			PASSERTMSG(TRUE, "CVideoBridgeCP::UpdateRoomPartiesITPTypeChanged - viewer party not found");
			continue;
		}

		std::ostringstream msg;
		plinkPartyCntl->GetTelepresenceInfo().Dump(msg);
		TRACEINTO << msg.str().c_str();

		plinkPartyCntl->SetTelepresenceInfoEPtype((link<numOfActiveLinks)? itpType : eTelePresencePartyInactive);
		plinkPartyCntl->SetTelepresenceInfoRoomID(target_room.GetRoomId());
		plinkPartyCntl->SetTelepresenceInfoNumOfLinks(numOfActiveLinks);
		plinkPartyCntl->SetTelepresenceInfoLinkNum(link);

		std::ostringstream msg2;
		plinkPartyCntl->GetTelepresenceInfo().Dump(msg2);
		TRACEINTO << msg2.str().c_str();
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnTimerITPSpeakerChange(CSegment* pParam)
{
	DWORD viewer_room, speaker_room;
	DWORD timer_speaker_room_id;
	*pParam >> timer_speaker_room_id;

	TRACECOND_AND_RETURN(!m_pLastActiveVideoSpeakerRequest, "Video speaker party not exist");

	PartyRsrcID partyId = ((CParty*)m_pLastActiveVideoSpeakerRequest)->GetPartyRsrcID();

	CVideoBridgePartyCntl* pVideoSpeakerPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pVideoSpeakerPartyCntl, "PartyId:" << partyId << " - Video speaker party not found");

	DWORD current_speaker_room_id = pVideoSpeakerPartyCntl->GetTelepresenceInfo().GetRoomID();
	if (current_speaker_room_id != timer_speaker_room_id)
		TRACEINTO << "Video speaker room (id " << current_speaker_room_id << ") has changed since timer was set (previous speaker id was " << timer_speaker_room_id << ")";
	TRACEINTO << "Timer ended for EP type change to speaker " << pVideoSpeakerPartyCntl->GetITPSiteName() ;

	const VBRIDGE_ROOM_INFO_MAP& room_list = m_pTelepresenceLayoutMngr->GetRoomList();
	BOOL all_acks_received = TRUE;
	// iterate over all rooms, check for pending acks
	for (VBRIDGE_ROOM_INFO_MAP::const_iterator cit = room_list.begin(); cit != room_list.end(); cit++)
	{
		BOOL ack_tout = cit->second.GetIsAckPendingForChangeType();
		// if ack prending, change room links layout manually, create log entry
		if (ack_tout)
		{
			TRACEINTO << " - After timer for  ITP type change ACK for party " << cit->second.GetMainLinkPartyName();
			m_pTelepresenceLayoutMngr->OnSpeakerChanged(cit->second.GetRoomId(), pVideoSpeakerPartyCntl->GetTelepresenceInfo().GetRoomID());
		}
		all_acks_received &= !ack_tout;
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnTimerITPTelepresencePartyConnection(CSegment* pParam)
{
	PartyRsrcID linkPartyId;
	*pParam >> linkPartyId;

	TRACEINTO << "PartyId:" << linkPartyId;

	OnStopPendingForLinksConnections(linkPartyId);
}
// This function is called from EndPartyConnectTelepresenceEP or when reaching Telepresence timer connection timeout
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnStopPendingForLinksConnections(PartyRsrcID linkPartyId)
{
	TRACEINTO << "PartyId:" << linkPartyId;

	if (!m_pTelepresenceLayoutMngr)
		return;
	CVideoBridgePartyCntl* linkPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(linkPartyId);
	PASSERTMSG_AND_RETURN(!CPObject::IsValidPObjectPtr(linkPartyCntl), "ITP_CASCADE: CVideoBridgeCP::OnStopPendingForLinksConnections - party not found");

    CRoomInfo room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(linkPartyCntl->GetTelepresenceInfo().GetRoomID());
	if(room_info.GetRoomId() == (RoomID)-1){ // not found by RoomId
	  room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(linkPartyCntl->GetITPSiteName(), linkPartyCntl->GetTelepresenceInfo().GetEPtype());
	}
	PASSERTMSG_AND_RETURN((room_info.GetRoomId() == (RoomID)-1), "ITP_CASCADE: CVideoBridgeCP::OnStopPendingForLinksConnections - party's room not found");

	TRACESTR(eLevelInfoNormal) << "ITP_CASCADE: CVideoBridgeCP::OnStopPendingForLinksConnections- timer ended for telepresence party " << linkPartyCntl->GetITPSiteName() ;

	//set the pending flag to off
	m_pTelepresenceLayoutMngr->SetRoomPendingForRealTPConnections(room_info.GetRoomId(), FALSE);

	// update all telepresence EP links' info.
	UpdateRoomPartiesITPTypeChanged(room_info,room_info.GetNumberOfActiveLinks(), room_info.GetRoomEPType(), FALSE);

	// TODO if speaker is in room, set the flag to off and update all parties for type and numlnks.
	BOOL isSpeakerInRoom = CheckIfRoomContainsTheCurrentSpeakerEp(room_info);
	if ((TRUE == isSpeakerInRoom) && (1 == linkPartyCntl->GetTelepresenceInfo().GetLinkRole())) // linkRole: 0=regular , 1-link
		UpdateAllCascadeRoomPartiesSpeakerITPTypeChanged(room_info);

	// temp for testing
	DumpTelepresenceInfo();

	// send change layout if Telepresence is managed internally
	if(m_bManageTelepresenceLayoutsInternally){
	  SendChangeLayoutAfterRoomConnected(room_info,isSpeakerInRoom);
	}

}
// ------------------------------------------------------------------------------------------
BOOL CVideoBridgeCP::CheckIfRoomContainsTheCurrentSpeakerEp(CRoomInfo& room_info)
{
	if (!IsValidPObjectPtr(m_pLastActiveVideoSpeakerRequest))
	{
		TRACEINTO << "Timer ended, but there is no current speaker party";
		return false;
	}

	const CParty* pParty = (CParty*)m_pLastActiveVideoSpeakerRequest;
	PartyRsrcID partyId = pParty->GetPartyRsrcID();

		// check if speaker is one of the room links
	if (!room_info.IsPartyInRoom(partyId))
		{
		TRACEINTO << "PartyId:" << partyId << " - Timer ended, but current speaker party is not in the same room";
		return false;
		}

	return true;
}


// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::SendChangeLayoutAfterRoomConnected(CRoomInfo& room_info, bool isSpeakerInRoom)
{
  ChangeRoomLayout(room_info);
  if(isSpeakerInRoom)	// if the current connected room is the speaker - send change layout to all other parties ***TBD - check if the speaker sees it
  {
    const VBRIDGE_ROOM_INFO_MAP room_list = m_pTelepresenceLayoutMngr->GetRoomList();
    // iterate over all rooms, send change layout to all parties
    for (VBRIDGE_ROOM_INFO_MAP::const_iterator cit = room_list.begin(); cit != room_list.end(); cit++)
    {
    	if(cit->second.GetRoomId() == room_info.GetRoomId()){
    		continue; // already sent it
    	}
    	ChangeRoomLayout(cit->second);
     }
  }
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::ChangeRoomLayout(const CRoomInfo& room_info)
{
  std::ostringstream msg;
  room_info.Dump(msg);
  TRACEINTO << msg.str().c_str();

  BYTE maxLinks = room_info.GetMaxLinks();
  if (maxLinks > MAX_CASCADED_LINKS_NUMBER)	//anatg-KW
  {
    DBGPASSERT(maxLinks);
    maxLinks = MAX_CASCADED_LINKS_NUMBER;
  }

  // check that changed layouts' ID vector is empty before loop (CL-SM)
  VerifyIdVectorIsEmpty();		// ****TBD - consider to move this to SendChangeLayoutAfterRoomConnected so it will be sent in bigger group of parties
  for (BYTE link=0; link < maxLinks; link++)
  {
      DWORD link_party_id = room_info.GetLinkPartyId(link);
      if (link_party_id == (DWORD)-1)
      {
	  continue;
      }
      CVideoBridgePartyCntl* plinkPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(link_party_id));
      if (!CPObject::IsValidPObjectPtr(plinkPartyCntl))
      {
    	  // we should not be here
    	  TRACEINTO << " cannot find CVideoBridgePartyCntl for party rsrc id = " << link_party_id;
    	  continue;
      }
      // void CVideoBridgePartyCntl::ChangeConfLayout(CLayout* pConfLayout, BYTE bAnyway /*= 0*/)
      plinkPartyCntl->ChangeConfLayout(NULL, 1);		// ****TBD - consider changing to ChangeLayout
  }
  //Change Layout Improvement - Layout Shared Memory (CL-SM)
  SendChangeLayoutToMultipleParties();

}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfSetConfVideoLayoutSeeMeAllCONNECTED(CSegment* pParam)
{
  // set conf video layout requested by operator in Conf Level
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfSetConfVideoLayoutSeeMeAllCONNECTED - ConfName:", m_pConfName);

  CVideoLayout layout;
  layout.DeSerialize(NATIVE, *pParam);

  BYTE bChangeAnyway = 0;
  *pParam >> bChangeAnyway;

  LayoutType newLayoutType = GetNewLayoutType(layout.GetScreenLayout());

  if ((newLayoutType != m_layoutType) && GetIsAutoLayout())
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfSetConfVideoLayoutSeeMeAllCONNECTED - Failed, can`t change layout manually in Auto Layout conference, ConfName:", m_pConfName);
    return;
  }

  if (!bChangeAnyway && (*m_pReservation[m_layoutType]) == layout)
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfSetConfVideoLayoutSeeMeAllCONNECTED - Failed, received same layout, ConfName:", m_pConfName);
    return;
  }

  // case when send layout just for setting reservations
  if (!layout.IsActive())
  {
    m_pReservation[newLayoutType]->SetLayout(layout, CONF_lev);
    UpdateDB_ConfLayout();
    return;
  }

  m_pReservation[newLayoutType]->SetLayout(layout, CONF_lev);

  ChangeConfLayoutType(newLayoutType);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfSetConfVideoLayoutSeeMePartyCONNECTED(CSegment* pParam)
{
  // set conf video layout requested by operator in Party Level
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfSetConfVideoLayoutSeeMePartyCONNECTED - ConfName:", m_pConfName);

  char targetPartyName[H243_NAME_LEN];
  *pParam >> targetPartyName;
  targetPartyName[H243_NAME_LEN-1] = '\0';

  CVideoLayout layout;
  layout.DeSerialize(NATIVE, *pParam);

  string s;
  layout.ToString(s);
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfSetConfVideoLayoutSeeMePartyCONNECTED - layout:\n", s.c_str());

  if (m_IsSameLayout)
  {
    // means no forces in part level
    PTRACE2(eLevelError, "CVideoBridgeCP::OnConfSetConfVideoLayoutSeeMePartyCONNECTED - Failed, SameLayot flag is ON - no forces in part level, ConfName:", m_pConfName);
    return;
  }
/// romem yyyyyyyyy
  CVideoBridgePartyCntl* pCurrParty = (CVideoBridgePartyCntl*)GetPartyCntl(targetPartyName);
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pCurrParty));

  if(pCurrParty->IsAVMCUParty())
  {
	  if(!pCurrParty->IsAVMCUMain())
	  {
		  TRACEINTO <<  " - Setting forces in party level is not allowed for AV-MCU Slave, Party Name: " << pCurrParty->GetFullName();
	  }
	  else
	  {
		  SendPrivateLayoutEventsToAllAVMCUOutSlavesandToMain(SETCONFVIDLAYOUT_SEEMEPARTY, pCurrParty, layout, FALSE);
	  }
	  return;
  }

  pCurrParty->ChangePartyLayout(layout);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfSetPrivateVideoLayoutCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(m_IsSameLayout, "Failed, SameLayot flag is ON, so no private layout");

	BOOL isMCMSAction;
	char partyName[H243_NAME_LEN];

	*pParam >> isMCMSAction;
	*pParam >> partyName;
	partyName[H243_NAME_LEN-1] = '\0';

	CVideoLayout layout;
	layout.DeSerialize(NATIVE, *pParam);

	CVideoBridgePartyCntl* pCurrParty = (CVideoBridgePartyCntl*)GetPartyCntl(partyName);
	PASSERTSTREAM_AND_RETURN(!pCurrParty, "PartyName:" << partyName << " - Party not found");

	std::ostringstream msg;
	msg
		<< "PartyId:"           << pCurrParty->GetPartyRsrcID()
		<< ", PartyName:"       << partyName
		<< ", CascadeLinkMode:" << CascadeModeToString(pCurrParty->GetCascadeLinkMode())
		<< ", IsMCMSAction:"    << (int)isMCMSAction
		<< ", IsLegacyParty:"   << (int)pCurrParty->IsLegacyParty()
		<< ", IsContent:"       << (int)IsConfInActiveContentPresentation()
		<< ", IsTelepresence:"  << (m_pTelepresenceLayoutMngr ? "1" : "0");

	// in ITP cascade mode, link party layout is only set internally by the CTeleprescenceLayoutMngr
	// VNGFE-7438: Added (m_pConf->GetCommConf()->GetCascadedLinksNumber() > 1) in case ISDN EP GW call we want to permit the MLA to change personal layout
	if (m_pTelepresenceLayoutMngr && !isMCMSAction && pCurrParty->GetCascadeLinkMode() && m_pConf->GetCommConf()->GetCascadedLinksNumber() > 1)
	{
		TRACEINTO << msg.str().c_str() << " - Private layout not allowed for cascade link party";
		return;
	}

	if(pCurrParty->IsAVMCUParty() && !pCurrParty->IsAVMCUMain())
	{
		TRACEINTO << msg.str().c_str() << " - Private layout not allowed for AV-MCU Slave";
		return;
	}


	// FEATURE: “Display Content to Legacy EP in Telepresence Conference“
	// RMX ignore personal layout changes for EPs receiving legacy content when:
	// 1. MLA is responsible for the layout.
	// 2. Content is sent
	// 3. The flag “FORCE_LEGACY_EP_CONTENT _LAYOUT_ON_TELEPRESENCE“ is set to YES.
	if (m_pTelepresenceLayoutMngr && IsConfInActiveContentPresentation() && pCurrParty->IsLegacyParty())
	{
		if (m_pTelepresenceLayoutMngr->IsDisplayContentToLegacyEP())
		{
			TRACEINTO << msg.str().c_str() << " - Private layout not allowed for legacy party while content is sent";
			return;
		}
	}
	// from telepresence layout manager- translate resource party ids to monitoring party ids
	if (isMCMSAction)
	{
		// Received telepresence manger's private layout for party
		CVideoCellLayout* curr_cell = layout.GetCurrentCell(1);
		PASSERTSTREAM_AND_RETURN(!CPObject::IsValidPObjectPtr(curr_cell), msg.str().c_str() << " - Telepresence manager's private layout does not contain cell #1");

		// if not blank, resolve rsrc ids to monitor ids for layout.
		if (curr_cell->GetCellStatus() != EMPTY_BY_OPERATOR_THIS_PARTY)
		{
			PartyRsrcID force_party_rsrc_id = curr_cell->GetForcedPartyId();

			CVideoBridgePartyCntl* pCellParty = (CVideoBridgePartyCntl*)GetPartyCntl(force_party_rsrc_id);
			PASSERTSTREAM_AND_RETURN(!CPObject::IsValidPObjectPtr(pCellParty), msg.str().c_str() << ", ForcedPartyId:" << force_party_rsrc_id << " - No force party found for telepresence manager's private layout");

			PartyMonitorID monitor_id = GetMonitorPartyId(pCellParty->GetPartyTaskApp());

			TRACEINTO << msg.str().c_str() << ", ForcedPartyId:" << force_party_rsrc_id << ", ForcedMonitorId:" << monitor_id << " - Setting layout source party for cell #1";

			curr_cell->SetCurrentPartyId(monitor_id);
			curr_cell->SetForcedPartyId(monitor_id);
		}
	}

	string s;
	layout.ToString(s);
	TRACEINTO << msg.str().c_str() << "\n" << s.c_str();
	if(pCurrParty->IsAVMCUMain())
	{
		SendPrivateLayoutEventsToAllAVMCUOutSlavesandToMain(SETPRIVATEVIDLAYOUT, pCurrParty,layout, TRUE);
	}
	else
		pCurrParty->ChangePartyPrivateLayout(layout);
}

// ------------------------------------------------------------------------------------------
// This is the case Operator does not send us a layout because :
// 1. It is the first time ever to initiate private layout and no private forces were set - only the radio  //    button was set and that means we will take the current conf layout and turn it to private.
// 2. The radio button is set to ON but without choosing a specific layout - So if it is not case 1
// We search for the Active Private Reservation layout and set it.
void CVideoBridgeCP::OnConfSetPrivateVideoLayoutOnOffCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(m_IsSameLayout, "Failed, SameLayot flag is ON, so no private layout");

	PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfSetPrivateVideoLayoutOnOffCONNECTED - ConfName:", m_pConfName);
	char targetPartyName[H243_NAME_LEN];
	WORD isPrivate = 0;

	*pParam >> isPrivate;
	*pParam >> targetPartyName;
	targetPartyName[H243_NAME_LEN - 1] = '\0';

	CVideoBridgePartyCntl* pCurrParty = (CVideoBridgePartyCntl*)GetPartyCntl(targetPartyName);
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pCurrParty));
	if (pCurrParty->IsAVMCUParty())
	{
		if (!pCurrParty->IsAVMCUMain())
		{
			TRACEINTO << pCurrParty->GetFullName() << " - Setting Private layout On/Off is not allowed for AV-MCU Slave";
		}
		else
		{
			CVideoLayout dummyLayout;
			SendPrivateLayoutEventsToAllAVMCUOutSlavesandToMain(SETPRIVATEVIDLAYOUTONOFF, pCurrParty, dummyLayout, isPrivate);
		}
		return;
	}

	pCurrParty->ChangeLayoutPrivatePartyButtonOnly(isPrivate);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnTimerLectureModeCONNECTED(CSegment* pParam)
{
  CVideoBridgePartyCntl* pLecturer     = (CVideoBridgePartyCntl*)GetPartyCntl(m_pLectureModeParams->GetLecturerName());
	DWORD partyImageSpeakerId = GetPartyImageSpeakerId();

	if (pLecturer && partyImageSpeakerId && m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular)
  {
		if (pLecturer->GetPartyRsrcID() == partyImageSpeakerId) // if the lecturer is the current speaker, change his Image
		{
			DWORD partyImageChangedId = GetPartyImageIdByPosition(GetPartyImageVectorSize()-1);
			PASSERT_AND_RETURN(!partyImageChangedId);

			CImage* pImage = GetPartyImageLookupTable()->GetPartyImage(partyImageSpeakerId);
			PASSERTSTREAM_AND_RETURN(!pImage, "CVideoBridgeCP::OnTimerLectureModeCONNECTED - Failed, The lookup table doesn't have an element, PartyId:" << partyImageSpeakerId);

			// Cause to set partyImageSpeakerId to the second place in the vector
			SetPartyImageSpeakerId(partyImageChangedId);
			SetPartyImageSpeakerId(partyImageSpeakerId);

			pLecturer->AddImage(pImage->GetVideoSource());

			// VSGNINJA-849
			//Change Layout Improvement - Layout Shared Memory (CL-SM)
			SendChangeLayoutToMultipleParties();
    }

    if (m_pLectureModeParams->GetIsTimerOn())
      StartTimer(LECTURE_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);
  }
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnTimerPresentationModeCONNECTED(CSegment* pParam)
{
  if (m_pLectureModeParams->GetLectureModeType() != eLectureModePresentation)
  {
    PTRACE2(eLevelError, "CVideoBridgeCP::OnTimerPresentationModeCONNECTED - Failed, invalid lecture mode params, ConfName:", m_pConfName);
    return;
  }

	if (m_pLastActiveAudioSpeakerRequest)
	{
		PartyRsrcID partyId = ((CParty*)m_pLastActiveAudioSpeakerRequest)->GetPartyRsrcID();

		CVideoBridgePartyCntl* pNewSpkrPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
  if (pNewSpkrPartyCntl)  // check that party did not disconnect meanwhile
  {
			TRACEINTO << "ConfName:" << m_pConfName << ", NewLecturerId: " << partyId;

    m_pLectureModeParams->SetLecturerName(pNewSpkrPartyCntl->GetName());

    StartLectureMode();
  }
}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfUpdateAutoLayoutCONNECTED(CSegment* pParam)
{
	WORD isAutoLayout = NO;

	*pParam >> isAutoLayout;

	SetAutoLayout(isAutoLayout);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfUpdateVideoClarityCONNECTED(CSegment* pParam)
{
  WORD newIsVideoClarity = NO;
  *pParam >> newIsVideoClarity;

  if (m_isVideoClarityEnabled == newIsVideoClarity)
  {
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCP::OnConfUpdateVideoClarityCONNECTED - The VideoClarity update is the same as the current mode, isVideoClarityEnabled:", m_isVideoClarityEnabled);
    return;
  }
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfUpdateVideoClarityCONNECTED - ConfName:", m_pConfName);
  UpdateVideoClarity(newIsVideoClarity);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateVideoClarity(WORD isVideoClarity)
{
  m_isVideoClarityEnabled = (BYTE)isVideoClarity;

  // update all the participants
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->UpdateVideoClarity(isVideoClarity);
  }

  // update the DB
  m_pConfApi->UpdateDB((CTaskApp*)0xFFFF, UPDATE_VIDEO_CLARITY, isVideoClarity);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnPartyImageUpdated(CSegment* pParam)
{
	CParty* pParty = NULL;
	WORD      status = statIllegal;
	BYTE		  isExtParams = FALSE;
	BYTE		  isResolutionOnlyUpdate = FALSE;

	*pParam >> (void*&)pParty >> status >> isExtParams;

	if (isExtParams)
		*pParam >> isResolutionOnlyUpdate;

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	TRACEINTO << "PartyId:" << partyId;

	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pVideoPartyCntl);

	//Change Layout Improvement - Conf Layout (CL-CL)
	BuildConfLayout();

	// check that changed layouts' ID vector is empty before loop (CL-SM)
	VerifyIdVectorIsEmpty();

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if (pPartyCntl->IsImageInPartiesLayout(partyId) || pPartyCntl->IsVideoRelayParty())
			{
				pPartyCntl->UpdateImage();
			}
		}
		}

	//Change Layout Improvement - Layout Shared Memory (CL-SM)
	SendChangeLayoutToMultipleParties();

	if (pVideoPartyCntl->IsVideoRelayParty())
		m_intraDB.Flush();//Send the intra requests to all required after the change
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnPartyNetworkQualityChangedCONNECTED(CSegment* pParam)
{
	CParty* pParty = NULL;
	WORD networkStatePerCell, networkStatePerLayout;

	*pParam >> (void*&) pParty >> networkStatePerCell >> networkStatePerLayout;

	PASSERT_AND_RETURN(!IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	TRACEINTO << "PartyId:" << partyId << ", NetworkStatePerCell:" << networkStatePerCell << ", NetworkStatePerLayout:" << networkStatePerLayout;

	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pVideoPartyCntl);

	const CImage* partyUpdatedImage = pVideoPartyCntl->GetPartyImage();
	PASSERT_AND_RETURN(!partyUpdatedImage);

	partyUpdatedImage->networkQualityPerCell().update(static_cast<eRtcpPacketLossStatus>(networkStatePerCell));
	partyUpdatedImage->networkQualityPerLayout().update(static_cast<eRtcpPacketLossStatus>(networkStatePerLayout));

	if (!IsValidTimer(NETWORK_QUALITY_UPDATE_TIMER))
	{
		m_indicationsUpdateCycle = 0;
		StartTimer(NETWORK_QUALITY_UPDATE_TIMER, NETWORK_QUALITY_UPDATE_TOUT);
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::Dump(VB_PARTY_VIDEO_LIST_S* pPartyList)
{
	std::ostringstream msg;
	msg << "CVideoBridgeCP::Dump"
		<< "\nVB_PARTY_VIDEO_LIST_S:"
		<< "\n  Number of ports              :" << pPartyList->list_size;

	for(WORD i = 0; i < pPartyList->list_size; i++)
		TRACEINTO << "\n  List index :" << i
				  << ", port_id:" << (DWORD)pPartyList->party_video_list[i].tPortPhysicalId.physical_id.port_id ;
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::GetPartyVideoDataReq(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CVideoBridge::GetPartyVideoDataReq we reached Conf , m_confRsrcID:", m_confRsrcID);

	BYTE boxId = 1;

	COsQueue q;
	q.DeSerialize(*pParam);

	//get data from segment
	CONF_PARTY_LIST_S* pParamData = new CONF_PARTY_LIST_S;

	pParam->Get((BYTE*)( &(pParamData->list_size) ),sizeof(DWORD));
	pParam->Get((BYTE*)( &(pParamData->boardId) ),sizeof(DWORD));
	pParam->Get((BYTE*)( &(pParamData->unitId) ),sizeof(DWORD));

	DWORD listSize = pParamData->list_size;
	DWORD boardId  = pParamData->boardId;
	DWORD unitId   = pParamData->unitId;

	pParamData->conf_party_list = new CONF_PARTY_ELEMENTS_S[listSize];
	pParam->Get( (BYTE*)( pParamData->conf_party_list ),sizeof(CONF_PARTY_ELEMENTS_S)*listSize );

	//create the list to be send to ConfPartyAssist task
	VB_PARTY_VIDEO_LIST_S*   PartyVideoList = new VB_PARTY_VIDEO_LIST_S;
	PartyVideoList->list_size = 0;
	int actualValidActiveParties = 0;

	std::list<ALLOC_STATUS_PER_PORT_S> tmpConfPartyList;

	TRACEINTO <<"BoardId:" <<boardId << ", UnitId:" << unitId;

	for(WORD j=0; j< pParamData->list_size;j++)
	{
		if (m_confRsrcID == pParamData->conf_party_list[j].rsrc_conf_id)
		{
			TRACEINTO
				<< "Index:" << j
				<< ", PartyId:" << pParamData->conf_party_list[j].rsrc_party_id
				<< ", PortId:" << pParamData->conf_party_list[j].port_id
				<< ", LogicalRsrcType:" << pParamData->conf_party_list[j].logicalRsrcType;

			ALLOC_STATUS_PER_PORT_S confIdPartyId;

			CBridgePartyVideoUniDirection* pParty;

			CBridgePartyCntl *pBridgePartyCntl = GetPartyCntl(pParamData->conf_party_list[j].rsrc_party_id);
			/// Romem & Ron - Bridge-4429 - Feature of Unit DB SYnc does not support Content Transcoding - will be inserted later
			if(!CPObject::IsValidPObjectPtr(pBridgePartyCntl))
			{
				DBGPASSERT(1);
				continue;
			}
			if ((pParamData->conf_party_list[j].logicalRsrcType == eLogical_video_encoder ) && (pBridgePartyCntl->GetBridgePartyOut() != NULL))
			{
				pParty = (CBridgePartyVideoUniDirection*)pBridgePartyCntl->GetBridgePartyOut();
			}
			else if ((pParamData->conf_party_list[j].logicalRsrcType == eLogical_video_decoder) && (pBridgePartyCntl->GetBridgePartyIn() != NULL))
			{
				pParty = (CBridgePartyVideoUniDirection*)pBridgePartyCntl->GetBridgePartyIn();
			}
			else
			{
				//print error;
				continue;
			}

			confIdPartyId.tPortPhysicalId.physical_id.resource_type = pParamData->conf_party_list[j].logicalRsrcType;
			confIdPartyId.tPortPhysicalId.physical_id.physical_unit_params.box_id = boxId;
			confIdPartyId.tPortPhysicalId.physical_id.physical_unit_params.board_id = boardId;
			//confIdPartyId.tPortPhysicalId.physical_id.physical_unit_params.sub_board_id = unitParams.m_subBoardId;
			confIdPartyId.tPortPhysicalId.physical_id.physical_unit_params.unit_id = unitId;

			confIdPartyId.tPortPhysicalId.physical_id.port_id= pParamData->conf_party_list[j].port_id;

			pParty->FillAllocStatus(confIdPartyId);

			++actualValidActiveParties;
			tmpConfPartyList.push_back(confIdPartyId);

			//send reply to ConfPartyAssist task
		}
	}

	//prepare the list to send to confpartyAssist
	//Set the size and allocate the list array
	PartyVideoList->list_size = actualValidActiveParties;

	PartyVideoList->party_video_list
	= new ALLOC_STATUS_PER_PORT_S[actualValidActiveParties];
	memset(PartyVideoList->party_video_list, 0, actualValidActiveParties
			* sizeof(ALLOC_STATUS_PER_PORT_S));

	int currentconfIdPartyId = 0;
	for(std::list<ALLOC_STATUS_PER_PORT_S>::iterator confIdPartyId = tmpConfPartyList.begin();
			confIdPartyId != tmpConfPartyList.end();  ++currentconfIdPartyId, ++confIdPartyId)
	{
		PartyVideoList->party_video_list[currentconfIdPartyId] = *confIdPartyId;
	}
	Dump(PartyVideoList);


	//copy the confPartyList to segment and send
	if (PartyVideoList->list_size > 0)
	{
		CSegment * pPartyVideoSeg = new CSegment;
		pPartyVideoSeg->Put((BYTE*) &(PartyVideoList->list_size), sizeof(DWORD));

		if (PartyVideoList->list_size)
			pPartyVideoSeg->Put(
					(BYTE *) (PartyVideoList->party_video_list),
					sizeof(ALLOC_STATUS_PER_PORT_S) * PartyVideoList->list_size);

		CTaskApi api;
		api.CreateOnlyApi(q);
		api.SendMsg(pPartyVideoSeg,GET_PARTY_VIDEO_DATA_IND);

		//Send ASynch Msg to the confparty process
		// CManagerApi confpartyManagerApi(eProcessConfParty);
		// confpartyManagerApi.SendMsg(pConfPartySeg, GET_CONFS_AND_PARTIES_LIST_IND);

		TRACESTR(eLevelInfoNormal)
		<< "\nCVideoBridgeCP::SendVBPartyVideoListToConfPartyAssist: " << PartyVideoList->list_size ;
	}

	if( PartyVideoList->party_video_list )
		delete[] PartyVideoList->party_video_list;
	delete PartyVideoList;

	if( pParamData->conf_party_list )
		delete[] pParamData->conf_party_list;
	delete pParamData;

	DumpParties();
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::OnTimerNetworkQualityChangedCONNECTED(CSegment* pParam)
{
	const size_t CYCLES = (5*SECOND) / (NETWORK_QUALITY_UPDATE_TOUT);

	++m_indicationsUpdateCycle;
	bool bUpdateAny = (m_indicationsUpdateCycle == CYCLES); // every 6 times this timer call-back is invoked we perform a full update

	if (bUpdateAny)
		m_indicationsUpdateCycle = 0;

	bool bDisableSelfNQ = false;
	const bool bDisableCellNQ = GetSystemCfgFlagInt<bool>("DISABLE_CELLS_NETWORK_IND");

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	bDisableSelfNQ = !(pCommConf->GetEnableSelfNetworkQualityIcon());

	if (bDisableCellNQ && bDisableSelfNQ)
	{
		m_indicationsUpdateCycle = 0; // the next time the timer is enabled let's start fresh
		return;
	}

	TRACEINTO << "ConfName: " << m_pConfName << ", Cycle: " << m_indicationsUpdateCycle;

	size_t changesCounter = 0;

	// initially, update all the parties indications to be displayed
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			const CImage* partyImage = pPartyCntl->GetPartyImage();
			if (!partyImage)
			{
				TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Image is invalid";
				continue;
			}

			bool bUpdateSelf = false;
			if(TRUE == pPartyCntl->IsPartyValidForLayoutIndication())
			{
				bUpdateSelf = !bDisableSelfNQ && partyImage->networkQualityPerLayout().updateDisplay(bUpdateAny);
			}

			bool bUpdateCell = !bDisableCellNQ && partyImage->networkQualityPerCell().updateDisplay(bUpdateAny);
			if (bUpdateCell || bUpdateSelf)
			{
				TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << ", UpdateSelf: " << (WORD)bUpdateSelf << ", UpdateCell:" << (WORD)bUpdateCell;
				++changesCounter;
			}
		}
		}

	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl && pPartyCntl->IsPartyValidForLayoutIndication()) //BRIDGE-17523 - not sending to SVC for example
			pPartyCntl->UpdateIndicationIcons();
	}

	TRACEINTO
		<< "ConfName:" << m_pConfName
		<< ", #parties change requests:" << changesCounter << ", #out of:" << (int)m_pPartyList->size();

	if (changesCounter || !bUpdateAny)
		StartTimer(NETWORK_QUALITY_UPDATE_TIMER, NETWORK_QUALITY_UPDATE_TOUT);
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::SendMessageOverlayToNewConnectedPartyIfNeeded(CSegment* pParam)
{
	eProductFamily eProdFamily = CProcessBase::GetProcess()->GetProductFamily();

	bool sendMessageOverlay = true;
	if ((m_pConf->GetCommConf())->GetEncryptionType() == eEncryptWhenAvailable)
	{
		sendMessageOverlay = false;
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

		std::string key;
		if (0 == m_pConf->GetNumOfUnencryptedParty())
			key = "DISPLAY_ENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE";
		else
			key = "DISPLAY_UNENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE";

		int messageDuration = 0;
		sysConfig->GetIntDataByKey(key, messageDuration);

		// check if the message disabled
		if (messageDuration == 0)
			sendMessageOverlay = true;
	}

	PartyRsrcID PartyId;
	*pParam >> PartyId;

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << PartyId << ", sendMessageOverlay:" << (WORD)sendMessageOverlay;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	if (GetMessageOverlayText() != "")
	{
		if (pPartyCntl->GetMessageOverlayText() == "")
		{
			if (GetIsPermanentMessageOverlay() == TRUE)
			{
				pPartyCntl->SetMessageOverlayText(GetMessageOverlayText());
				pPartyCntl->SetIsMessageOverlayPermanent(TRUE);
			}
			else
			{
				pPartyCntl->SetIsMessageOverlayPermanent(FALSE);
			}
		}
	}
	else
	{
		if (GetIsPermanentMessageOverlay() == TRUE)
			pPartyCntl->SetIsMessageOverlayPermanent(TRUE);
		else
			pPartyCntl->SetIsMessageOverlayPermanent(FALSE);
	}

	if (sendMessageOverlay == true)
	{
		// VNGR-27007
		if (GetMessageOverlayText() != "")
			m_pMessageOverlayInfo->SetMessageText(GetMessageOverlayText());

		m_pMessageOverlayInfo->Dump("CVideoBridgeCP::SendMessageOverlayToNewConnectedPartyIfNeeded");
		pPartyCntl->StartMessageOverlay(m_pMessageOverlayInfo);
	}
	else // sendSecureMessage (after IVR)
	{
		CSegment* pSeg = new CSegment;
		*pSeg << pPartyCntl->GetName();
		OnPartySendSecureMessageCONNECTED(pSeg);
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateMessageOverlayForParty(PartyRsrcID partyId, CMessageOverlayInfo* pMessageOverlayInfo)
{
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pVideoPartyCntl, "PartyId:" << partyId);

	pVideoPartyCntl->UpdateMessageOverlay(pMessageOverlayInfo);
	pVideoPartyCntl->SetPcmOvrlyMsgOn(TRUE); // VNGR-15750-fix
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::UpdateMessageOverlayStopForParty(PartyRsrcID partyId) // VNGR-15750
{
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pVideoPartyCntl, "PartyId:" << partyId);

	if (pVideoPartyCntl->IsPcmOvrlyMsgOn())  // VNGR-15750-fix
		pVideoPartyCntl->UpdateMessageOverlayStop();
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfSetAutoScanOrderCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CVideoBridgeCP::OnConfSetAutoScanOrderCONNECTED - ConfName:", m_pConfName);

  auto_ptr<CAutoScanOrder> pAutoScanOrder(new CAutoScanOrder);
  pAutoScanOrder->DeSerialize(NATIVE, *pParam);

  //Change Layout Improvement - Conf Layout (CL-CL)
  BuildConfLayout();

  // check that changed layouts' ID vector is empty before loop (CL-SM)
  VerifyIdVectorIsEmpty();

  // Update all the participants
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->UpdateAutoScanOrder(pAutoScanOrder.get());
    }

  //Change Layout Improvement - Layout Shared Memory (CL-SM)
  SendChangeLayoutToMultipleParties();

  // update bridge auto scan params (this is the default for new parties)
  if (m_pAutoScanParams)
    m_pAutoScanParams->InitScanOrder(pAutoScanOrder.get());
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfUpdateMessageOverlayCONNECTED(CSegment* pParam)
{
	StopMessageOverlayTimer();
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfUpdateMessageOverlayCONNECTED - ConfName:", m_pConfName);
	CSegment* pCopyParam = new CSegment(*pParam);
	m_pMessageOverlayInfo->DeSerialize(NATIVE, *pParam);

	if (m_pMessageOverlayInfo->GetDisplaySpeedType() == eStatic)
	{
		SetIsPermanentMessageOverlay(TRUE);
		SetMessageOverlayText(m_pMessageOverlayInfo->GetMessageText());
	}
	else
	{
		SetIsPermanentMessageOverlay(FALSE);
	}

	// Update all the participants
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (!pPartyCntl)
			continue;

		// VNGR-26449 - unencrypted conference message

		// set message overlay is permanent for all parties
		if (m_pMessageOverlayInfo->GetDisplaySpeedType() == eStatic)
		{
			pPartyCntl->SetIsMessageOverlayPermanent(TRUE);
			pPartyCntl->SetMessageOverlayText(m_pMessageOverlayInfo->GetMessageText());
		}
		else
		{
			pPartyCntl->SetIsMessageOverlayPermanent(FALSE);
		}

		// BRIDGE-2002
		if (pPartyCntl->GetIsPermanentSecureMessageForParty() == TRUE)
			SetIsPermanentSecureMessage(TRUE);

		pPartyCntl->StopSecureMessageTimer();
		pPartyCntl->StopMessageOverlayEndTimer();

		pPartyCntl->UpdateMessageOverlay(m_pMessageOverlayInfo);
		pPartyCntl->SetPcmOvrlyMsgOn(FALSE); // VNGR-15750-fix
	}

	// VNGR-26449 - unencrypted conference message
	StopSecureMessageTimer();
	// check if secure message is permanent
	if (GetIsPermanentSecureMessage() == TRUE)
	{
		if (m_pMessageOverlayInfo->GetMessageOn() == FALSE)
		{
			OnConfSendSecureMessageCONNECTED(pCopyParam);
		}
		else
		{
			// calculate the time of message overlay
			WORD messageOverlayTime = calcMessageOverlayTime();
			// if the message overlay is also permanent. -1 means that message overlay is permanent
			if (messageOverlayTime != 0)
			{
				// startTimer with the clac to know when to return secure message
				StartTimer(CONF_MESSAGE_OVERLAY_TIMER, messageOverlayTime*SECOND);
			}
		}
	}

	POBJDELETE(pCopyParam);
}

// ------------------------------------------------------------------------------------------
WORD CVideoBridgeCP::calcMessageOverlayTime()
{
	WORD time = 0;

	WORD numOfCharsInScreen = 0;
	WORD timeTakesOneCharThroughScreen = 0;
	WORD messageLen = (m_pMessageOverlayInfo->GetMessageText()).length();


	switch (m_pMessageOverlayInfo->GetDisplaySpeedType())
	{
	case eSlow:
		timeTakesOneCharThroughScreen = 13;
		break;

	case eFast:
		timeTakesOneCharThroughScreen = 7;
		break;

		//message overlay is permanent, but also the secure message.
	default:
		return 0;
	}

	switch (m_pMessageOverlayInfo->GetFontSize_Old())
	{
	case eSmall:
		numOfCharsInScreen = 104;
		break;
	case eMedium:
		numOfCharsInScreen = 62;
		break;
	case eLarge:
		numOfCharsInScreen = 40;
		break;
	}

	time = timeTakesOneCharThroughScreen*(1+(messageLen/numOfCharsInScreen));
	time = time*(m_pMessageOverlayInfo->GetNumOfRepetitions());
	return time;
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnPartyUpdateMessageOverlayCONNECTED(CSegment* pParam)
{
  char targetPartyName[H243_NAME_LEN];
  *pParam >> targetPartyName;
  targetPartyName[H243_NAME_LEN-1] = '\0';

  eProductFamily eProdFamily = CProcessBase::GetProcess()->GetProductFamily();

  TRACEINTO << "ConfName:" << m_pConfName << ", PartyName:" << targetPartyName;

  CMessageOverlayInfo MessageOverlayInfo;
  MessageOverlayInfo.DeSerialize(NATIVE, *pParam);

  CVideoBridgePartyCntl* pCurrParty = (CVideoBridgePartyCntl*)GetPartyCntl(targetPartyName);
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pCurrParty));


  if (pCurrParty->IsVideoRelayParty())
  {
	  TRACEINTO << "Message overlay is not supported in SVC relay party";
	  return;
  }

  UpdateMessageOverlayForParty(pCurrParty->GetPartyRsrcID(), &MessageOverlayInfo);

  //VNGR-26449 - unencrypted conference message
  if((m_pConf->GetCommConf())->GetEncryptionType()==eEncryptWhenAvailable)
  {
	  pCurrParty->StopSecureMessageTimer();
	  pCurrParty->StopMessageOverlayEndTimer();
	  pCurrParty->StartMessageOverlayTimer(m_pConf->GetNumOfUnencryptedParty(),&MessageOverlayInfo);
  }
}

// ------------------------------------------------------------------------------------------

//VNGR-26449 - unencrypted conference message
void CVideoBridgeCP::OnPartySendSecureMessageCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeCP::OnPartySendSecureMessageCONNECTED - ConfName:", m_pConfName);

	char targetPartyName[H243_NAME_LEN];
	*pParam >> targetPartyName;
	targetPartyName[H243_NAME_LEN-1] = '\0';

	TRACEINTO << "PartyName:" << targetPartyName;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(targetPartyName);
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyCntl));

	m_pMessageOverlayInfo->Dump("CVideoBridgeCP::OnPartySendSecureMessageCONNECTED");
	m_pMessageOverlayInfo->SetMessageText(GetMessageOverlayText());

	//stop the timer of the previous message if needed
	pPartyCntl->StopSecureMessageTimer();
	pPartyCntl->StopMessageOverlayEndTimer();

	pPartyCntl->StartSecureMessageTimer(m_pConf->GetNumOfUnencryptedParty(),m_pMessageOverlayInfo);

}
// ------------------------------------------------------------------------------------------
//VNGR-26449 - unencrypted conference message
void CVideoBridgeCP::OnConfSendSecureMessage(CSegment* pParam)
{
	m_pMessageOverlayInfo->Dump("CVideoBridgeCP::OnConfSendSecureMessage");

	OnConfSendSecureMessageCONNECTED(pParam);
}

// ------------------------------------------------------------------------------------------
//VNGR-26449 - unencrypted conference message
void CVideoBridgeCP::OnConfSendSecureMessageCONNECTED(CSegment* pParam)
{
	StopMessageOverlayTimer();

	PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfSendSecureMessageCONNECTED - ConfName:", m_pConfName);

	//stop the timer of the previous message if needed
	StopSecureMessageTimer();

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	int messageDuration = 0;
	std::string key;
	string secureMessage;
	if(0 == m_pConf->GetNumOfUnencryptedParty())
	{
		key = "DISPLAY_ENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE";
		secureMessage = "The conference is secured";
	}
	else
	{
		key = "DISPLAY_UNENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE";
		secureMessage = "The conference is not secured";
	}
	sysConfig->GetIntDataByKey(key, messageDuration);

	if (messageDuration==0)
	{
		//VNGR-27007
		if(GetIsPermanentMessageOverlay()==FALSE)
			m_pMessageOverlayInfo->SetMessageOnOff(FALSE);
	}

	// Update all the participants
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
		m_pMessageOverlayInfo->SetMessageText(secureMessage);
		m_pMessageOverlayInfo->SetMessageOnOff(TRUE);
		m_pMessageOverlayInfo->SetMessageDisplaySpeedType(eStatic);

		//stop the timer of the previous message if needed
			pPartyCntl->StopSecureMessageTimer();

			if (pPartyCntl->GetIsMessageOverlayEndTimerWorks() == FALSE)
		{
				StartSecureMessageTimer(m_pConf->GetNumOfUnencryptedParty(), pPartyCntl);
				pPartyCntl->SetPcmOvrlyMsgOn(FALSE);//VNGR-15750-fix
		}
	}
	}
}

// ------------------------------------------------------------------------------------------
//VNGR-26449 - unencrypted conference message
void CVideoBridgeCP::OnTimerSecureMessageCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnTimerSecureMessageCONNECTED - ConfName:", m_pConfName);

	//stop the message
	if (!GetIsPermanentMessageOverlay())
		m_pMessageOverlayInfo->SetMessageOnOff(FALSE);
	else //put message overlay instead of secure message
		m_pMessageOverlayInfo->SetMessageText(GetMessageOverlayText());

	// Update all the participants
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
	  {
			if (pPartyCntl->GetIsSecureMessageTimerWorks() == FALSE)
	  {
				pPartyCntl->UpdateMessageOverlay(m_pMessageOverlayInfo);
				pPartyCntl->SetPcmOvrlyMsgOn(FALSE);//VNGR-15750-fix
	  }
	}
}
}

// ------------------------------------------------------------------------------------------
//VNGR-26449 - unencrypted conference message
void CVideoBridgeCP::OnTimerMessageOverlayCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnTimerMessageOverlayCONNECTED - ConfName:", m_pConfName);

	/*if (IsValidTimer(CONF_MESSAGE_OVERLAY_TIMER))
	{
		DeleteTimer(CONF_MESSAGE_OVERLAY_TIMER);
	}*/

	OnConfSendSecureMessageCONNECTED(pParam);
}

// ------------------------------------------------------------------------------------------
//VNGR-26449 - unencrypted conference message
void CVideoBridgeCP::StartSecureMessageTimer(int numOfUnencrypted, CVideoBridgePartyCntl* pCurrParty)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::StartSecureMessageTimer - ConfName:", m_pConfName);
	//After the timer expires the message disappears
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	int messageDuration = 0;
	std::string key;
	if(0 == numOfUnencrypted)
	{
		key = "DISPLAY_ENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE";
	}
	else
	{
		key = "DISPLAY_UNENCRYPTED_MESSAGE_TIMER_FOR_ENCRYPT_WHEN_POSSIBLE";
	}
	sysConfig->GetIntDataByKey(key, messageDuration);
	//check if the message is permanent
	if(messageDuration==-1)
	{
	  //setFlag
		SetIsPermanentSecureMessage(TRUE);
		pCurrParty->UpdateMessageOverlay(m_pMessageOverlayInfo);

	}
	//check if the message disabled
	else if(messageDuration==0)
	{
		//VNGR-27007
		if(GetIsPermanentMessageOverlay()==FALSE)
		{
			m_pMessageOverlayInfo->SetMessageOnOff(FALSE);
			pCurrParty->UpdateMessageOverlay(m_pMessageOverlayInfo);
		}
	}
	//after the timer expires the message disappears
	else
	{
		StartTimer(CONF_SECURE_MESSAGE_TIMER, (DWORD)messageDuration*SECOND);
		pCurrParty->UpdateMessageOverlay(m_pMessageOverlayInfo);
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//VNGR-26449 - unencrypted conference message
void CVideoBridgeCP::StopSecureMessageTimer()
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::StopSecureMessageTimer - ConfName:", m_pConfName);
	if (IsValidTimer(CONF_SECURE_MESSAGE_TIMER))
	{
		DeleteTimer(CONF_SECURE_MESSAGE_TIMER);
	}

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//VNGR-26449 - unencrypted conference message
void CVideoBridgeCP::StopMessageOverlayTimer()
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::StopMessageOverlayTimer - ConfName:", m_pConfName);
	if (IsValidTimer(CONF_MESSAGE_OVERLAY_TIMER))
	{
		DeleteTimer(CONF_MESSAGE_OVERLAY_TIMER);
	}

}
// ------------------------------------------------------------------------------------------


void CVideoBridgeCP::GetForcesFromReservation(CCommConf* pCommConf)
{
  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr((CPObject*)pCommConf));

  WORD m_wNumVideoLayouts = pCommConf->GetNumRsrvVidLayout();
  if (m_wNumVideoLayouts > (WORD)CP_NO_LAYOUT) {
    PASSERT_AND_RETURN(m_wNumVideoLayouts);
  }

  // get layouts and write it
  if (m_wNumVideoLayouts == 0)
    return;

  CVideoLayout* pCurrentLayout = pCommConf->GetFirstRsrvVidLayout();
  PASSERT_AND_RETURN(!pCurrentLayout);

  LayoutType newLayoutType = GetNewLayoutType(pCurrentLayout->GetScreenLayout());
  if (newLayoutType >= CP_NO_LAYOUT) {
    PASSERT_AND_RETURN(newLayoutType);
  }

  // sometimes first layout in reservation is not active one
  m_pReservation[newLayoutType]->SetLayoutFromRes(*pCurrentLayout, CONF_lev);
  if (pCurrentLayout->IsActive())
  {
    m_layoutType = newLayoutType;
    m_pReservation[m_layoutType]->SetCurrActiveLayout(YES);
  }

  for (WORD i = 1; i < m_wNumVideoLayouts; i++)
  {
    pCurrentLayout = pCommConf->GetNextRsrvVidLayout();
    if (!pCurrentLayout)
    {
      PASSERT(1);
      break;
    }

    newLayoutType = GetNewLayoutType(pCurrentLayout->GetScreenLayout());
    if (newLayoutType >= CP_NO_LAYOUT)
    {
      DBGPASSERT(newLayoutType);
      break;
    }

    m_pReservation[newLayoutType]->SetLayoutFromRes(*pCurrentLayout, CONF_lev);
    if (pCurrentLayout->IsActive())
    {
      m_layoutType = newLayoutType;
      m_pReservation[m_layoutType]->SetCurrActiveLayout(YES);
    }
  }

  CVideoLayout* pVideoLayout = pCommConf->GetNextRsrvVidLayout();
  WORD testValue = (pVideoLayout) ? pVideoLayout->GetScreenLayout() : 0;
  DBGPASSERT(testValue);
}
// ------------------------------------------------------------------------------------------
// enough for changing layout type, but needed as part of force change
void CVideoBridgeCP::ChangeConfLayoutType(LayoutType newLayoutType)
{
	std::ostringstream msg;
	msg << "OldLayoutType:" << LayoutTypeAsString[m_layoutType] << ", NewLayoutType:" << LayoutTypeAsString[newLayoutType];

	if (newLayoutType == m_layoutType)
		msg << " - Same layout as before, but continue there might be forces change";

	TRACEINTO << msg.str().c_str();

	if (newLayoutType != CP_NO_LAYOUT)
		m_pReservation[newLayoutType]->SetCurrActiveLayout(YES);

	if (m_layoutType != CP_NO_LAYOUT && m_layoutType != newLayoutType)
		m_pReservation[m_layoutType]->SetCurrActiveLayout(NO);

	m_layoutType = newLayoutType;

	// Change Layout Improvement - Conf Layout (CL-CL)
	BuildConfLayout();

	std::set<std::string> setExceptionList;
	int nExceptionParts = m_pConf->GetExceptionPartList(setExceptionList);

	// check that changed layouts' ID vector is empty before loop (CL-SM)
	VerifyIdVectorIsEmpty();

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			bool bChange = true;
			if (nExceptionParts > 0)
			{
				std::string sName(pPartyCntl->GetName());
				if (setExceptionList.find(sName) != setExceptionList.end())
					bChange = false;
			}

			TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << ", bChange:" << (int)bChange;
			if (bChange)
				pPartyCntl->ChangeConfLayout(NULL, 1);
		}
	}

	// Change Layout Improvement - Layout Shared Memory (CL-SM)
	SendChangeLayoutToMultipleParties();

	UpdateDB_ConfLayout();
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::StartAutoLayout()
{
  if (GetIsAutoLayout() && m_pAutoLayoutSet != NULL) // Conf set with Auto Layout ON
  {
    DWORD vectorSizeUnmuted = GetPartyImageVectorSizeUnmuted();
    LayoutType tempLayoutType = m_pAutoLayoutSet->GetLayoutType(vectorSizeUnmuted, IsSameLayout());
    std::ostringstream msg;
    msg << "CVideoBridgeCP::StartAutoLayout "
        << "- ConfName:"     << m_pConfName
        << ", ImagesNumber:" << vectorSizeUnmuted
        << ", IsSameLayout:" << (int)IsSameLayout()
        << ", NewLayout:"    << LayoutTypeAsString[tempLayoutType];

    CCommConf*  pCommConf         = (CCommConf*)m_pConf->GetCommConf();
    CConfParty* pRecLinkConfParty = pCommConf->GetRecordLinkCurrentParty();
    if (pRecLinkConfParty)
    {
      msg <<  "\nRecording Link found in conference, ";
      LayoutType privateRecLinkLayoutType = m_pAutoLayoutSet->GetLayoutTypeForRecLink(vectorSizeUnmuted);

     //eFeatureRssDialin
	 enSrsVideoLayoutType  videoLayout = (enSrsVideoLayoutType) pRecLinkConfParty->GetLastLayoutForRL();
          if(eSrsVideoLayout1X1 == videoLayout)
         {
		privateRecLinkLayoutType = CP_LAYOUT_1X1;
          }
	 else if(eSrsVideoLayout1X2 == videoLayout)
	 {
		 privateRecLinkLayoutType = CP_LAYOUT_1X2;
	 }
	 else
	 {
		//Auto mode
	 }

      CVideoBridgePartyCntl* pRecLinkPartyControl = (CVideoBridgePartyCntl*)GetPartyCntl(pRecLinkConfParty->GetName());
      if (CPObject::IsValidPObjectPtr(pRecLinkPartyControl))
      {
        if (!pRecLinkPartyControl->IsConnectedStandalone())
        {
          msg << "that connected to video bridge, so set it's private layout: " << LayoutTypeAsString[privateRecLinkLayoutType];
          WORD apiLayoutType = ::GetOldLayoutType(privateRecLinkLayoutType);
          WORD numSubImages  = GetNumbSubImg(privateRecLinkLayoutType);
          CVideoLayout privateRecLinkLayout;
          privateRecLinkLayout.SetScreenLayout(apiLayoutType);
          for (int i = 0; i < numSubImages; i++)
          {
            CVideoCellLayout cellLayout;
            cellLayout.SetCellId(i+1);
            cellLayout.SetAudioActivated();
            privateRecLinkLayout.AddCell(cellLayout);
          }
          pRecLinkPartyControl->ChangePartyPrivateLayout(privateRecLinkLayout);
        }
        else
        {
          msg << "that connected to video bridge in CONNECTED_STANDALONE state (sees slide), so do nothing";
        }
      }
      else
      {
        msg << "that not connected to video bridge";
      }
    }

    PTRACE(eLevelInfoHigh, msg.str().c_str());

    if (tempLayoutType != m_layoutType)
    {
      TRACEINTO << "ConfName:" << m_pConfName << ", NewLayout:"<< LayoutTypeAsString[tempLayoutType];
      ChangeConfLayoutType(tempLayoutType);
    }
  }
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::StartPresentationMode()
{
  if (m_pLectureModeParams->GetLectureModeType() != eLectureModePresentation)
  {
    PTRACE2(eLevelError, "CVideoBridgeCP::StartPresentationMode - Failed, invalid presentation mode params, ConfName:", m_pConfName);
    return;
  }
  else
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::StartPresentationMode - ConfName:", m_pConfName);
    if (CPObject::IsValidPObjectPtr(m_pLastActiveAudioSpeakerRequest))
      StartTimer(PRESENTATION_MODE_TOUT, m_pLectureModeParams->GetTimerInterval()*SECOND);
  }
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl, CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
	if(pVideoBridgePartyInitParams->GetIsVideoRelay())
	{
		NewVideoRelayPartyCntl(pVideoBrdgPartyCntl);
	}
	else
		CVideoBridgeCP::NewPartyCntl(pVideoBrdgPartyCntl);

	pVideoBrdgPartyCntl->Create(pVideoBridgePartyInitParams);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl)
{
  pVideoBrdgPartyCntl = new CVideoBridgePartyCntl();
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::NewVideoRelayPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl)
{
  pVideoBrdgPartyCntl = new CVideoRelayBridgePartyCntl();
}
// ------------------------------------------------------------------------------------------
// This function build the text msg to screen
void CVideoBridgeCP::DisplayPartyListOnScreen(PartyRsrcID partyId, BYTE IsConfSecure, char** AudioArr, WORD numAudioParties, char** VideoArr, WORD numVideoParties, BOOL IsOneParytOnly)
{
	CVideoBridgePartyCntl* pVideoPartyCntl = NULL;

	if (IsOneParytOnly)
	{
		pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
		PASSERTSTREAM_AND_RETURN(!pVideoPartyCntl, "PartyId:" << partyId);
	}

	// Total num of parties
	WORD NumOfparties = numAudioParties + numVideoParties;

	CTextOnScreenMngr* TextMsgList = new CTextOnScreenMngr();

	// Build Secure title
	if (IsConfSecure)
	{
		CTextOnScreenMsg* Title1Msg = new CTextOnScreenMsg();
		Title1Msg->SetTextLine("SECURE");
		Title1Msg->SetAlignment(E_TEXT_ALIGNMENT_CENTER);
		Title1Msg->SetFontType(TEXT_FONT_TYPE_BOLD);
		TextMsgList->AddMsg(Title1Msg);
		POBJDELETE(Title1Msg);
	}

	// Build Total title
	CSmallString TotalStr;
	TotalStr << "Total:" << NumOfparties;

	CTextOnScreenMsg* TitleTotalMsg = new CTextOnScreenMsg();
	TitleTotalMsg->SetTextLine(TotalStr.GetString());
	TitleTotalMsg->SetAlignment(E_TEXT_ALIGNMENT_CENTER);
	TitleTotalMsg->SetFontType(TEXT_FONT_TYPE_BOLD);
	TextMsgList->AddMsg(TitleTotalMsg);

	// Build Video & Audio title
	CSmallString VidAudStr;
	VidAudStr << "V:" << numVideoParties << " A:" <<numAudioParties;

	CTextOnScreenMsg* TitleVidAudMsg = new CTextOnScreenMsg();
	TitleVidAudMsg->SetTextLine(VidAudStr.GetString());
	TitleVidAudMsg->SetAlignment(E_TEXT_ALIGNMENT_CENTER);
	TitleVidAudMsg->SetFontType(TEXT_FONT_TYPE_BOLD);
	TextMsgList->AddMsg(TitleVidAudMsg);

	// Display only to chair
	if (IsOneParytOnly)
		pVideoPartyCntl->SendMsgToScreenPerParty(TextMsgList);
	else // Display to all video parties
	{
		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		{
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
			if (pPartyCntl)
				pPartyCntl->SendMsgToScreenPerParty(TextMsgList);
		}
	}

	// delete objects
	POBJDELETE(TextMsgList);
	POBJDELETE(TitleTotalMsg);
	POBJDELETE(TitleVidAudMsg);
}
// ------------------------------------------------------------------------------------------
// This function build the text msg to screen
void CVideoBridgeCP::DisplayPartyTextOnScreen(PartyRsrcID partyId, char** displayStringArr, WORD displayStringArrNo)
{
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pVideoPartyCntl, "PartyId:" << partyId);

	CTextOnScreenMngr* TextMsgList   = new CTextOnScreenMngr();
	CTextOnScreenMsg*  TitleTotalMsg = new CTextOnScreenMsg();

	// Enter List of messages
	for (DWORD i = 0; i < displayStringArrNo; i++)
	{
		CSmallString displayStr;
		if (displayStringArr[i] != NULL)
		{
			displayStr << displayStringArr[i];
			TitleTotalMsg->SetTextLine(displayStr.GetString());
			TitleTotalMsg->SetAlignment(E_TEXT_ALIGNMENT_CENTER);
			TitleTotalMsg->SetFontType(TEXT_FONT_TYPE_BOLD);
		}
		TextMsgList->AddMsg(TitleTotalMsg);
	}
	pVideoPartyCntl->SendMsgToScreenPerParty(TextMsgList);

	// delete objects
	POBJDELETE(TextMsgList);
	POBJDELETE(TitleTotalMsg);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnEndImportParty(CSegment* pParam)
{
	DWORD PartyId;
	*pParam >> PartyId;

	TRACEINTO << "PartyId:" << PartyId;

	CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(PartyId);
	if (pParty)
		AddPartyToConfMixAfterVideoInSynced(pParty);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnEndPartyIvrModeAfterResumeCall(CSegment* pParam)
{
	DWORD PartyId;
	*pParam >> PartyId;

	TRACEINTO << "PartyId:" << PartyId;

	CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(PartyId);
	if (pParty)
		AddPartyToConfMixAfterVideoInSynced(pParty);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfTurnOnOffTelePresence(CSegment* pParam)
{
	WORD onOff;
	WORD isIncludingVisuals;
	*pParam >> onOff >> isIncludingVisuals;

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	ETelePresenceLayoutMode telePresenceLayoutMode = (ETelePresenceLayoutMode)(pCommConf->GetTelePresenceLayoutMode());

	TRACEINTO << "TELEPRESENCE_LAYOUTS_DEBUG IsOn:" << onOff << ", ManageTelepresenceLayoutsInternally:" << (WORD)m_bManageTelepresenceLayoutsInternally << ", IsIncludingVisuals:" << isIncludingVisuals << ", telePresenceLayoutMode:" << TelePresenceLayoutModeToString(telePresenceLayoutMode);

	BOOL bTelepresenceModeChanged = FALSE;
	if (onOff != m_telepresenceOnOff)	// Telepresence mode was changed due to Telepresence add or delete parties
		bTelepresenceModeChanged = TRUE;

	if (TRUE == bTelepresenceModeChanged)
	{
		m_telepresenceOnOff = onOff;
		// Update CBridgePartyVideoOut with the relevant layout handler
		if (TRUE == m_bManageTelepresenceLayoutsInternally)
		{
			CBridgePartyList::iterator _end = m_pPartyList->end();
			for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
			{
				// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
				CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
				if (pPartyCntl)
					pPartyCntl->UpdateLayoutHandlerType();
				}
			}
		}

	if (onOff)
	{
		if (m_pTelepresenceLayoutMngr == NULL)
		{
			m_pTelepresenceLayoutMngr = new CTelepresenceLayoutMngr(m_pConfApi, this);
			if (m_pPartyList->size() > 0)
				LoadTelepresenceInfoFromExistingParties();

			// FEATURE: "Display Content to Legacy EP in Telepresence Conference"
			// BRIDGE-8421
			if (IsLegacyBridge())
			{
				m_pTelepresenceLayoutMngr->SetConfLayoutToLegacyLayout(true);
			}
		}
	}
	else
	{
		POBJDELETE(m_pTelepresenceLayoutMngr);
	}

	if (isIncludingVisuals)
	{
		CVisualEffectsParams* pOldVisualEffects = new CVisualEffectsParams;
		*pOldVisualEffects = *m_pVisualEffects;
		m_pVisualEffects->DeSerialize(NATIVE, *pParam);

		BYTE isCropping       = pCommConf->GetIsCropping();
		BYTE isAutoBrightness = pCommConf->GetAutoBrightness();
		BYTE isAutoLayout     = pCommConf->GetIsAutoLayout();

		m_IsSameLayout = pCommConf->GetIsSameLayout();
		m_pConfApi->UpdateAutoLayout(isAutoLayout);
		m_pConfApi->UpdateLectureMode(pCommConf->GetLectureMode());

		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		{
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
			if (pPartyCntl)
			{
				if (!(*m_pVisualEffects == *pOldVisualEffects))
				{
					if ((TRUE == m_bManageTelepresenceLayoutsInternally) && (FALSE == bTelepresenceModeChanged))	// Telepresence mode has NOT changed - ***TBD --> Should we get here???
					{
						pPartyCntl->UpdateVisualEffects(m_pVisualEffects);
					}
					else
					{
						pPartyCntl->UpdateVisualEffects(m_pVisualEffects, TRUE);	// don't send to hardware immediately, hardware will be updated by full change layout request
					}
				}
				pPartyCntl->UpdateAutoBrightness(isAutoBrightness);

				//if telepresence is turned off, remove the private layout
				if (!(pPartyCntl->IsLegacyParty() && m_isConfInActiveContentPresentation))
				{
					if (!onOff)
					{
						pPartyCntl->ActionsAfterTelepresenceModeChanged(onOff);
					}
				}
			}
		}

		PDELETE(pOldVisualEffects);
	}

	// Send change layout to all participants in order to switch to the suitable Telepresence mode's layout
	if  ((TRUE == m_bManageTelepresenceLayoutsInternally) && (TRUE == bTelepresenceModeChanged))	// Telepresence mode HAS changed
	{
		// check that changed layouts' ID vector is empty before loop (CL-SM)
		VerifyIdVectorIsEmpty();

		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		{
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
			if (pPartyCntl)
				pPartyCntl->ChangeConfLayout(NULL, 1);		// ****TBD - consider changing to ChangeLayout
			}

		//Change Layout Improvement - Layout Shared Memory (CL-SM)
		SendChangeLayoutToMultipleParties();
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::LoadTelepresenceInfoFromExistingParties()
{
	TRACEINTO << "ConfName:" << m_pConfName << ", PartyListSize:" << (int)m_pPartyList->size();

	CBridgePartyList sublink_parties;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			const CTelepresenseEPInfo& tpInfo = pPartyCntl->GetTelepresenceInfo();
			// if sublink, habdle later
			if ((tpInfo.GetLinkRole() != 0) && (m_pTelepresenceLayoutMngr->GetLinkPartyIndex(pPartyCntl->GetITPSiteName()) != 0))
			{
				TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << " - Adding as a multi-cacsade sublink";
				sublink_parties.Insert(pPartyCntl);
			}
			else
			{
				OnEndPartyConnectWithTelepresence(pPartyCntl);
			}
		}
	}

	_end = sublink_parties.end();
	for (CBridgePartyList::iterator _itr = sublink_parties.begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			OnEndPartyConnectWithTelepresence(pPartyCntl);
	}
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::DisplayPartyTextListOnScreen(PartyRsrcID partyId, CTextOnScreenMngr* TextMsgList, DWORD timeout)
{
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pVideoPartyCntl, "PartyId:" << partyId);

	pVideoPartyCntl->SendMsgToScreenPerParty(TextMsgList, timeout);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects)
{
  TRACEINTO << "ConfName:" << m_pConfName << ", BkgImageId:" << pVisualEffects->GetBackgroundImageID();

  PDELETE(m_pVisualEffects);
  m_pVisualEffects = new CVisualEffectsParams(pVisualEffects);

  std::set<std::string> setExceptionList;
  int nExceptionParts = m_pConf->GetExceptionPartList(setExceptionList);

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
    {
    bool bChange = true;
    if (nExceptionParts > 0)
    {
				std::string sName(pPartyCntl->GetName());
      if (setExceptionList.find(sName) != setExceptionList.end())
        bChange = false;
    }
    if (bChange)
				pPartyCntl->SetVisualEffectsParams(m_pVisualEffects);
  }
}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::SetVisualEffectsParams(const std::string& partyName, CVisualEffectsParams* pVisualEffects)
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if (partyName == pPartyCntl->GetName())
		{
			TRACEINTO << "PartyName:" << partyName.c_str() << ", BkgImageId:" << pVisualEffects->GetBackgroundImageID();
				pPartyCntl->SetVisualEffectsParams(pVisualEffects);
			return;
		}
	}
	}

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyName:" << partyName.c_str() << " - Failed, party not found";
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::DisplayGatheringOnScreen(CGatheringManager* pGatheringManager)
{
  char buff[20] = "";

  std::map < std::string, CGathering* >* pMapPartGathering = pGatheringManager->GetMapPartGathering();

  CMedString sLog;
  for (std::map < std::string, CGathering* >::iterator itMap = pMapPartGathering->begin(); itMap != pMapPartGathering->end(); ++itMap)
  {
    sprintf(buff, "%p", itMap->second);
    sLog << "\t\tkey = " << itMap->first << "    val = " << buff << (itMap->second->IsEndGathering() ? "    End Gathering" : "    Show Gathering") << "\n";
  }

  if (pMapPartGathering->size() > 0)
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::DisplayGatheringOnScreen -  mapPartGathering:\n", sLog.GetString());

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
    {
			std::map<std::string, CGathering*>::const_iterator itPartGathering = pMapPartGathering->find(pPartyCntl->GetName());
      if (itPartGathering != pMapPartGathering->end())
      {
        if (itPartGathering->second)
        {
					TRACEINTO << "ConfName:" << m_pConfName << ", PartyName:" << pPartyCntl->GetName();
					pPartyCntl->DisplayGatheringOnScreen(itPartGathering->second);
        }
      }
    }
  }
}

// ------------------------------------------------------------------------------------------
bool CVideoBridgeCP::IsBridgePartyVideoOutStateIsConnected(const char* partyName)
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if (strcmp(partyName, pPartyCntl->GetName()) == 0)
		{
				TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID() << ", PartyName:" << partyName;
				return pPartyCntl->IsBridgePartyVideoOutStateIsConnected();
			}
		}
	}
	TRACEINTO << "PartyName:" << partyName << " - Failed, party not found";
	return false;
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::DisplayGatheringOnScreen(const char* partyName, CGatheringManager* pGatheringManager)
{
	std::map < std::string, CGathering* >* pMapPartGathering = pGatheringManager->GetMapPartGathering();

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if (strcmp(partyName, pPartyCntl->GetName()) == 0)
		{
				std::map < std::string, CGathering* >::const_iterator itPartGathering = pMapPartGathering->find(partyName);
				if (itPartGathering != pMapPartGathering->end())
				{
					if (itPartGathering->second)
					{
						TRACEINTO << "ConfName:" << m_pConfName << ", PartyName:" << pPartyCntl->GetName();
						pPartyCntl->DisplayGatheringOnScreen(itPartGathering->second);
					}
				}
				break;
			}
		}
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::StopDisplayPartyTextOnScreen(PartyRsrcID partyId)
{
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pVideoPartyCntl, "PartyId:" << partyId);

	pVideoPartyCntl->StopMsgToScreenPerParty();
}

// ------------------------------------------------------------------------------------------
DWORD CVideoBridgeCP::GetPartyResolutionInPCMTerms(DWORD partyId)
{
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN_VALUE(!pVideoPartyCntl, "PartyId:" << partyId, (DWORD)-1);

	if (pVideoPartyCntl->IsVideoRelayParty())	// currently not valid for Relay party
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, not a valid for Relay Party";
		return (DWORD)-1;
	}

	CBridgePartyVideoOut* pBridgePartyOut = (CBridgePartyVideoOut*)pVideoPartyCntl->GetBridgePartyOut();
	PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pBridgePartyOut), (DWORD)-1);

	CBridgePartyVideoOutParams bridgePartyVideoOutParams;
	pBridgePartyOut->GetBridgePartyVideoOutParams(bridgePartyVideoOutParams);

	return TranslateToPcmResolution(bridgePartyVideoOutParams);
}

// ------------------------------------------------------------------------------------------
CLayout* CVideoBridgeCP::GetPartyLayout(const char* partyName)
{
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyName);
	PASSERTSTREAM_AND_RETURN_VALUE(!pVideoPartyCntl, "PartyName:" << partyName, NULL);
	return pVideoPartyCntl->GetCurrentLayout();
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::ChangeSpeakerNotation(PartyRsrcID partyId, DWORD imageId)
{
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pVideoPartyCntl, "PartyId:" << partyId);
	pVideoPartyCntl->ChangeSpeakerNotationForPcmFecc(imageId);
}

// ------------------------------------------------------------------------------------------
DWORD CVideoBridgeCP::TranslateToPcmResolution(CBridgePartyVideoOutParams& bridgePartyVideoOutParams)
{
  DWORD pcmResolution = ePcmIllegalImageParams;
  if (bridgePartyVideoOutParams.GetVideoAlgorithm() == H264 || bridgePartyVideoOutParams.GetVideoAlgorithm() == RTV)
  {
    DWORD qcifMaxFS    = 1;   // The division operation is heavy, (H264_L1_DEFAULT_FS/CUSTOM_MAX_FS_FACTOR) + (H264_L1_DEFAULT_FS%CUSTOM_MAX_FS_FACTOR? 1 : 0);
    DWORD cifMaxFS     = 2;   // The division operation is heavy, (H264_L1_1_DEFAULT_FS/CUSTOM_MAX_FS_FACTOR) + (H264_L1_1_DEFAULT_FS%CUSTOM_MAX_FS_FACTOR? 1 : 0);
    DWORD cif30MaxMBPS = 24;  // The division operation is heavy,(H264_L1_3_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR) + (H264_L1_3_DEFAULT_MBPS%CUSTOM_MAX_MBPS_FACTOR? 1 : 0);
    DWORD sdMaxFS      = 9;
    DWORD hd720MaxFS   = 15;
    //New added for VNGR-25807
    DWORD vgaMaxFS     = 5;  //640*480/256/256=4.6875

    if (bridgePartyVideoOutParams.GetFS() > sdMaxFS)
    {
      if (bridgePartyVideoOutParams.GetTelePresenceMode() == eTelePresencePartyNone)
        pcmResolution = e1024x576_16x9;
      else
        pcmResolution = e640x480_4x3;
    }
    //else if (bridgePartyVideoOutParams.GetFS() > cifMaxFS)
    else if (bridgePartyVideoOutParams.GetFS() > vgaMaxFS)  //640*480/256/256=4.6875
    {
      pcmResolution = e640x480_4x3;
    }
    else
    {
      pcmResolution = e320x240_4x3;
    }
  }
  else
  {
    switch (bridgePartyVideoOutParams.GetVideoResolution())
    {
      case (eVideoResolutionQCIF):
      case (eVideoResolutionQVGA):
      case (eVideoResolutionCIF):
      case (eVideoResolutionSIF):
      {
        pcmResolution = e320x240_4x3;
        break;
      }

      case (eVideoResolutionVGA):
      case (eVideoResolution4SIF):
      case (eVideoResolution4CIF):
      case (eVideoResolution525SD):
      case (eVideoResolution625SD):
      {
        pcmResolution = e640x480_4x3;
        break;
      }

      case (eVideoResolutionSVGA):
      case (eVideoResolutionXGA):
      case (eVideoResolutionHD720):
      case (eVideoResolution16CIF):
      case (eVideoResolutionHD1080):
      {
        if (bridgePartyVideoOutParams.GetTelePresenceMode() == eTelePresencePartyNone)
          pcmResolution = e1024x576_16x9;
        else
          pcmResolution = e640x480_4x3;

        break;
      }
	  default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
    } // switch
  }

  return pcmResolution;
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfContentBridgeStopPresentationCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CCVideoBridgeCP::OnConfContentBridgeStopPresentationCONNECTED - ConfName:", m_pConfName);
  StartTimer(AFTER_STOP_CONTENT_TIMER, AFTER_STOP_CONTENT_TOUT);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfContentBridgeStartPresentationCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CVideoBridgeCP::OnConfContentBridgeStartPresentationCONNECTED - ConfName:", m_pConfName);
  StartTimer(AFTER_START_CONTENT_TIMER, AFTER_START_CONTENT_TOUT);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnTimerAfterStartStopContentCONNECTED(CSegment* pParam)
{
	TRACEINTO << "ConfName:"<< m_pConfName << " - Send INTRA to all participants of conference";

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
    {
			if (!pPartyCntl->IsVideoRelayParty()) //In relay parties,there is no need for requesting intra since the endpoint handles the video rate changes after the content stops
    {
				pPartyCntl->FastUpdate();                     //send INTRA to EP (from encoder)
				pPartyCntl->HandleEvent(NULL, 0, VIDREFRESH); //ask for INTRA from EP
    	}
    }
  }
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfUpdateSiteNameInfoCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfUpdateSiteNameInfoCONNECTED - ConfName:", m_pConfName);
  m_pSiteNameInfo->DeSerialize(NATIVE, *pParam);

  // update all the participants
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->UpdateSiteNameInfo(m_pSiteNameInfo);
  }
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfUpdateRefreshLayoutCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCP::OnConfUpdateRefreshLayoutCONNECTED - ConfName:", m_pConfName);

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->RefreshLayout();
  }
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfTerminateDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoHigh, "CVideoBridgeCP::OnConfTerminateDISCONNECTING - ConfName:", m_pConfName);
	CVideoBridge::OnConfTerminateDISCONNECTING(pParam);
}

// ------------------------------------------------------------------------------------------
//Change Layout Improvement - Conf Layout (CL-CL)
BOOL CVideoBridgeCP::BuildConfLayout()
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCP::BuildConfLayout");
  BOOL isConfLayoutChanged = FALSE;


  CLayout* pNewLayout  = new CLayout(m_layoutType, GetConfName());
  BOOL isCop = FALSE;
  BuildLayout(*pNewLayout, m_layoutType, isCop);

  DWORD confLayoutChanged = IsLayoutChange(pNewLayout, m_ConfLayout);
  if (LAYOUT_NOT_CHANGED != confLayoutChanged)
  {
	  isConfLayoutChanged = TRUE;
	  UpdateConfLayout(pNewLayout);

  }

  POBJDELETE(pNewLayout);
  return isConfLayoutChanged;
}


// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnEndPartyDisConnect(CSegment* pParam)
{
	CSegment* pCopyOfSegmet = new CSegment;
	pCopyOfSegmet->CopySegmentFromReadPosition(*pParam);

	PartyRsrcID PartyId;
	WORD status;

	*pCopyOfSegmet >> PartyId >> status;
	CVideoBridgePartyCntl* pPartyCntl =(CVideoBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyCntl));

	if (pPartyCntl->IsAVMCUMain())
	{
		TRACEINTO << pPartyCntl->GetFullName() << " - End Disconnect AV-MCU master, delete AV-MCU manager";
		// Detach of  delete events
		EventPackage::Manager& lyncEventManager = EventPackage::Manager::Instance();
		lyncEventManager.DelSubscriber(pPartyCntl->GetPartyRsrcID(), EventPackage::eEventType_MediaDeleted, std::make_pair((COsQueue*)&(m_pConfApi->GetRcvMbx()),VIDEO_BRIDGE_MSG));
		lyncEventManager.DelSubscriber(pPartyCntl->GetPartyRsrcID(), EventPackage::eEventType_MediaStatusUpdated, std::make_pair((COsQueue*)&(m_pConfApi->GetRcvMbx()),VIDEO_BRIDGE_MSG));
		lyncEventManager.DelSubscriber(pPartyCntl->GetPartyRsrcID(), EventPackage::eEventType_UserDisplayTextUpdated, std::make_pair((COsQueue*)&(m_pConfApi->GetRcvMbx()),VIDEO_BRIDGE_MSG));
		lyncEventManager.DelSubscriber(pPartyCntl->GetPartyRsrcID(), EventPackage::eEventType_EndpointDisplayTextUpdated,std::make_pair((COsQueue*)&(m_pConfApi->GetRcvMbx()),VIDEO_BRIDGE_MSG));
		lyncEventManager.DelSubscriber(pPartyCntl->GetPartyRsrcID(), EventPackage::eEventType_VideoSwitchingModeUpdated,std::make_pair((COsQueue*)&(m_pConfApi->GetRcvMbx()),VIDEO_BRIDGE_MSG));
		POBJDELETE(m_pAVMCUMngr);
	}
	else
	{
		if (pPartyCntl->IsAVMCUParty())
		{
			if (m_pAVMCUMngr)
				m_pAVMCUMngr->RemoveLinkFromLinksVector(pPartyCntl->GetPartyRsrcID());
			else
				PASSERTSTREAM(1, "ConfName:" << m_pConfName);
		}
	}

	POBJDELETE(pCopyOfSegmet);

	CVideoBridge::OnEndPartyDisConnect(pParam);

	if ( 0 == GetNumParties() &&  IsRelaySupported() )
	{
		if (IsEQ())	// temporary solution for BRIDGE-5283 (amir)
		{
			TRACEINTO << "ConfName:" << m_pConfName <<  " - Not removing VideoOperationPointSet from EQ as not added";
		}
		else
		{
			TRACEINTO << "ConfName:" << m_pConfName <<  " - Last party on relay supported conference disconnected need to send remove video operation point";
			SendRemoveVideoOperationPointSet();
		}
	}
}

	// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnEndPartyExportCONNECTED(CSegment* pParam)
{
	CVideoBridge::OnEndPartyExportCONNECTED( pParam);
	if ( 0 == GetNumParties() &&  IsRelaySupported() )
	{
		if (IsEQ())	// temporary solution for BRIDGE-5283 (amir)
		{
			TRACEINTO << "ConfName:" << m_pConfName <<  " - Not removing VideoOperationPointSet from EQ as not added";
		}
		else
		{
			TRACEINTO << "ConfName:" << m_pConfName <<  " - Last party on relay supported conference disconnected need to send remove video operation point";
			SendRemoveVideoOperationPointSet();
		}
	}
}


// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnAddVideoRelayImageToMixCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoHigh, "CVideoBridge::OnAddVideoRelayImageToMixCONNECTED - ConfName:", m_pConfName);
	CTaskApp* pParty = NULL;
	*pParam >> (void*&)pParty;

	PASSERTMSG_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty), "Failed, 'pParty' is invalid");

	//We call the same flow when the video in sync for Transcode party and when the VideoIn connected for VideoRelay participants
	AddPartyToConfMix(pParty);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnAddVideoRelayImageToMixDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoHigh, "CVideoBridgeCP::OnAddVideoRelayImageToMixDISCONNECTING Do nothing - ConfName:", m_pConfName);

}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnVideoRelayImageSvcAvcTranslateUpdateCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoHigh, "CVideoBridgeCP::OnVideoRelayImageSvcAvcTranslateUpdateCONNECTED  ConfName:", m_pConfName);

	// request to check (and create if needed) a new Conf Layout
	StartAutoLayout();

	//Send update to all non relay participants so they can initiate change layout
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
	    {
	    	//only for non relay the update on the svc to avc translation may change the layout.
			if (!pPartyCntl->IsVideoRelayParty())
				pPartyCntl->UpdateOnImageSvcToAvcTranslate();
	    	}
	    }

	 //Send all the required intra request of relay participants
	m_intraDB.Flush();
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnVideoNonRelayImageAvcSvcTranslateUpdateCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoHigh, "CVideoBridgeCP::OnVideoNonRelayImageAvcSvcTranslateUpdateCONNECTED  ConfName:", m_pConfName);

	// send notification to event-package
	CTaskApp* pParty = NULL;
	*pParam >> (void*&)pParty;
	if (pParty)
	{
		NotifyOnAddedVideoInStreams(pParty->GetPartyId());

		CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
		CImage* pImage = pPartyImageLookupTable->GetPartyImage(pParty->GetPartyId());

		AddImageToIntraDB((CImage*)pImage);	// adding to intra list
	}

	// request to check (and create if needed) a new Conf Layout
	StartAutoLayout();

	//Send update to all non relay participants so they can initiate change layout
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
	    {
			if (pPartyCntl->IsVideoRelayParty())
	    	{
				((CVideoRelayBridgePartyCntl*)pPartyCntl)->UpdateOnImageAvcToSvcTranslate();
	    	}
	    }
	    }
	  }

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnVideoNonRelayImageAvcSvcTranslateUpdateDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoHigh, "CVideoBridgeCP::OnVideoNonRelayImageAvcSvcTranslateUpdateDISCONNECTING Do nothing - ConfName:", m_pConfName);
}


// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnVideoRelayImageSvcAvcTranslateUpdateDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoHigh, "CVideoBridgeCP::OnVideoRelayImageSvcAvcTranslateUpdateDISCONNECTING Do nothing - ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnRelayEpAskForIntraConnected(CSegment* pParam)
{
	RelayIntraParam intraData;
	intraData.DeSerialize(pParam);

	TRACEINTO << "PartyId:" << intraData.m_partyRsrcId;

	//2. Send to BridgePartyOut to find out what are the sources that are sent on the required pipes
	CVideoRelayBridgePartyCntl* pPartyCntl = (CVideoRelayBridgePartyCntl*)GetPartyCntl(intraData.m_partyRsrcId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << intraData.m_partyRsrcId << " - Failed, can't find party";
	}
	else
	{
		if (pPartyCntl->IsVideoRelayParty())
		{
			((CVideoRelayBridgePartyCntl*)pPartyCntl)->EpAskForIntra(intraData);
		}
		else
		{
			if (pPartyCntl->IsTranslatorAvcSvcExists())
			{
				TRACEINTO << "PartyId:" << intraData.m_partyRsrcId << " - Send request to AvcToSvcTranslator Encoder";
				((CVideoBridgePartyCntl*)pPartyCntl)->SendRelayIntraRequestToAvcToSvcTranslator(intraData.m_listSsrc, intraData.m_bIsGdr);
			}
			else
				TRACEINTO << "PartyId:" << intraData.m_partyRsrcId << " - AVC Party error";
		}
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::AskRelayEpsForIntra(AskForRelayIntra& epIntraParams)
{
	PTRACE(eLevelWarn, "CVideoBridgeCP::AskRelayEpsForIntra ");
	CSegment* pSeg = new CSegment;
	epIntraParams.Serialize(pSeg);
	DispatchEvent(RELAY_ASK_ENDPOINTS_FOR_INTRA, pSeg);
    POBJDELETE(pSeg);
}
// ------------------------------------------------------------------------------------------

void CVideoBridgeCP::OnRelayAskEpsForIntraConnected(CSegment* pParam)
{
	AskForRelayIntra intraData;
	intraData.DeSerialize(pParam);

	RelayIntraParams::const_iterator itParam = intraData.m_relayIntraParameters.begin();

	// temp temp temp - to be removed
//	if (FALSE == intraData.m_isImmediately)
//	{
//		intraData.m_isImmediately = TRUE;
//		TRACEINTO << "hold hold - intraData.m_isImmediately changed to TRUE";
//	}
	// temp temp temp - to be removed


	if (intraData.m_isImmediately)
	{
		PTRACE(eLevelInfoHigh, "CVideoBridge::OnRelayAskEpsForIntraConnected - m_isImmediately = TRUE");

		for (; itParam != intraData.m_relayIntraParameters.end(); ++itParam)
		{
			int iPartyRsrcID = itParam->m_partyRsrcId;
			bool isGDR = itParam->m_bIsGdr;
			CVideoBridgePartyCntl* pPartyCtrl = (CVideoBridgePartyCntl*)(GetPartyCntl(iPartyRsrcID));
			if (pPartyCtrl)
			{
				if (pPartyCtrl->IsVideoRelayParty())
				{
					std::string  strParams =  CStlUtils::ContainerToString(itParam->m_listSsrc);
					PTRACE2(eLevelInfoHigh, "DEBUG_INTRA: CVideoBridgeCP::OnRelayAskEpsForIntraConnected : ", strParams.c_str());
					((CVideoRelayBridgePartyCntl*)pPartyCtrl)->SendIntraRequestToParty(itParam->m_listSsrc, isGDR, intraData.m_isAllowSuppression);
				}
				else
				{
					if (pPartyCtrl->IsTranslatorAvcSvcExists())
					{
						PTRACE2INT(eLevelWarn, "CVideoBridgeCP::OnRelayAskEpsForIntraConnected -  send request to AvcToSvcTranslator Encoder, PartyRsrcID = ", itParam->m_partyRsrcId);
						((CVideoBridgePartyCntl*)pPartyCtrl)->SendRelayIntraRequestToAvcToSvcTranslator(itParam->m_listSsrc, isGDR);
					}
					else
						PTRACE2INT(eLevelWarn, "CVideoBridgeCP::OnRelayAskEpsForIntraConnected -  AVC without Svc Translator, PartyRsrcId = ", itParam->m_partyRsrcId);
				}
			}
			else
				PTRACE2INT(eLevelWarn, "CVideoBridgeCP::OnRelayAskEpsForIntraConnected -  can't find party with PartyRsrcId = ", itParam->m_partyRsrcId);
		}
	}
	else
	{
		PTRACE(eLevelInfoHigh, "CVideoBridge::OnRelayAskEpsForIntraConnected - m_isImmediately = FALSE");
		m_intraDB.SetNeedIntra(intraData);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::AddImageToIntraDB(CImage* pImage)
{
	if( !IsValidPObjectPtr(pImage))
	{
		PASSERT(1);
		return;
	}
	PTRACE2INT(eLevelInfoHigh, "CVideoBridgeCP::AddImageToIntraDB PartyId:", pImage->GetPartyRsrcId());

	std::list<DWORD> listSsrc;
	pImage->GetSsrcList(listSsrc);
	if ( ! listSsrc.empty() )
		m_intraDB.AddParty(pImage->GetPartyRsrcId(), listSsrc);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::RemoveImageFromIntraDB(DWORD partyRscId)
{
	TRACEINTO << "PartyId:" << partyRscId;
	m_intraDB.RemoveParty(partyRscId);
}

// ------------------------------------------------------------------------------------------
// In current version when there is no AVC participants in the call
// The SvcToAvcTranslator will send VideoSoursesRequest with no streams thus there will be no acctual
// translation and we reduce the cpu
void CVideoBridgeCP::UpdateNumberOfNonRelayImagesIfNeeded(CVideoBridgePartyCntl* pPartyCntl, bool imageAdded)
{
	eConfMediaType confMediaType = GetConfMediaType();
	if (eMixAvcSvc != confMediaType)
		return;

	bool isRelayParty = pPartyCntl->IsVideoRelayParty();
	bool needToSendUpdateToAllRelayParties = false;
	if(!isRelayParty)
	{
		if(imageAdded)
		{
			m_NumberOfNonRelayImages++;

			if(m_NumberOfNonRelayImages==1)
			{
				needToSendUpdateToAllRelayParties = true;
				//the first non relay image was added update the relay participants to start translating SVC to Avc
			}
		}
		else	// image removed
		{
			if(m_NumberOfNonRelayImages == 0)
			{
				PASSERT_AND_RETURN(101);
			}
			m_NumberOfNonRelayImages--;

			if(m_NumberOfNonRelayImages==0)
			{
				//last non relay party image was removed update the relay participants to stop translating SVC to Avc
				needToSendUpdateToAllRelayParties = true;
			}
		}
	}

	if(needToSendUpdateToAllRelayParties)
	{
		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		 {
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
			if (pPartyCntl)
			 {
				if (pPartyCntl->IsVideoRelayParty())
				{
				 if(imageAdded)
						((CVideoRelayBridgePartyCntl*)pPartyCntl)->AvcImageAdded();
				 else
						((CVideoRelayBridgePartyCntl*)pPartyCntl)->LastAvcImageRemoved();
				 }
			 }
	    }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
eConfMediaType CVideoBridgeCP::GetConfMediaType()
{
	CCommConf*  pCommConf         = (CCommConf*)m_pConf->GetCommConf();
	eConfMediaType confMediaType = ((eConfMediaType)pCommConf->GetConfMediaType());
	return confMediaType;

}

/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCP::IsRelaySupported()
{
	BOOL ret = ::IsRelaySupported(GetConfMediaType());

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::AckOnIvrScpShowSlide(PartyRsrcID partyRsrcID, BYTE bIsAck)
{
	CVideoBridgePartyCntl* pVideoBridgePartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(partyRsrcID));
	if (!CPObject::IsValidPObjectPtr(pVideoBridgePartyCntl))
	{
		TRACEINTO << "PartyId:" << partyRsrcID << ", ConfId:" << m_pConf->GetConfId() << " - Invalid pVideoBridgePartyCntl";
		DBGPASSERT_AND_RETURN(1);
	}
	((CVideoRelayBridgePartyCntl*) pVideoBridgePartyCntl)->AckOnIvrScpShowSlide(bIsAck);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::AckOnIvrScpStopShowSlide(PartyRsrcID partyRsrcID, BYTE bIsAck)
{
	CVideoBridgePartyCntl* pVideoBridgePartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(partyRsrcID));
	if (!CPObject::IsValidPObjectPtr(pVideoBridgePartyCntl))
	{
		TRACEINTO << "PartyId:" << partyRsrcID << ", ConfId:" << m_pConf->GetConfId() << " - Invalid pVideoBridgePartyCntl";
		DBGPASSERT_AND_RETURN(1);
	}
	((CVideoRelayBridgePartyCntl*) pVideoBridgePartyCntl)->AckOnIvrScpStopShowSlide(bIsAck);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::OnConfUpdateMuteAllVideoExceptLeaderCONNECTED(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	BYTE newIsMuteAllVideoButLeader = NO;
	*pParam >> newIsMuteAllVideoButLeader;

	if (m_isMuteAllVideoButLeaderForJustIncoming)
	{
		m_isMuteAllVideoButLeaderForJustIncoming = FALSE;
	}
	else
	{
		if (m_isMuteAllVideoButLeader == newIsMuteAllVideoButLeader)
		{
			TRACEINTO << "ConfName:" << m_pConfName << ", IsMuteAllVideoButLeader:" << (WORD)m_isMuteAllVideoButLeader << " - No change";
			return;
		}
	}

	m_isMuteAllVideoButLeader = newIsMuteAllVideoButLeader;
	//1. Update the image of all participants with the OPERATOR mute state except for leader.

	//2. Update Conf DB with the new value of the mute all video except leader

	//3. If there was a change in at least one of the participants mute state:

	// a. send muteimage/unmute image to all participants so they will initatate layout/stream change
	// b. SendChangeLayoutToMultipleParties
	// c. intra DB flush
	// d. application actions for mute/unmute (we split them so the changelayouts will be at the end...
	bool isOneOfParticipantsMuteStateChanged = false;
	RequestPriority reqPrio = OPERATOR_Prior;
	EOnOff partyMuteOnOff = eOff;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		if (m_isMuteAllVideoButLeader)
			partyMuteOnOff = eOn;

		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (!pPartyCntl)
			continue;

		const CImage* pImage = pPartyCntl->GetPartyImage();
		if (!pImage)
		{
			PASSERT(1);
			continue;
		}

		WORD isPartyLeader = IsPartyLeader(pPartyCntl->GetName());
		if (isPartyLeader)
		{
			if (!m_isMuteAllVideoButLeader)
			{
				TRACEINTO << "Party is leader skip mute/unmute as a result from mute all but leader change";
				continue;
			}
			else  //need to unmute the party in case its on and it's chair
			{
				partyMuteOnOff = eOff;
			}
		}
		BYTE previouselyMuted = pImage->isMuted();  // before updating we get previouse
		pPartyCntl->UpdateSelfMute(reqPrio, partyMuteOnOff);

		BYTE currentlyMuted = pImage->isMuted();  // after updating we get current
		if (previouselyMuted != currentlyMuted)
		{
			isOneOfParticipantsMuteStateChanged = true;
		}
		if (partyMuteOnOff == eOn)  //we need to mute and its not the leader, if lecture mode is on as well we need to end it
		{
			if (GetLectureModeRoleForParty(pPartyCntl->GetName()) == eLECTURER)
			{
				if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular)
					EndLectureMode(NO);
				else if (m_pLectureModeParams->GetLectureModeType() == eLectureModePresentation)
					EndLectureMode(YES);
			}
		}
		else
		{   //we unmute the party that was lecturer keren to check
			if (m_pLectureModeParams->GetLectureModeType() == eLectureModeRegular)
			{
				if (GetLectureModeRoleForParty(pPartyCntl->GetName()) == eLECTURER)
				{
					StartLectureMode();
					UpdateConfDBLectureMode();
				}
			}
		}
	}
	//udate DB to do after merege with Marina
	m_pConfApi->UpdateDB((CTaskApp*)0xFFFF, UPDATE_MUTE_ALL_VIDEO_EXCEPT_LEADER, m_isMuteAllVideoButLeader);

	if (!isOneOfParticipantsMuteStateChanged)
	{
		return;
	}

	//Change Layout Improvement - Conf Layout (CL-CL)
	BuildConfLayout();
	// check that changed layouts' ID vector is empty before loop (CL-SM)
	VerifyIdVectorIsEmpty();

	_end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
		if (partyMuteOnOff == eOff) //acctualy it isnt important if we send the mute or unmute event both causes change layout/change stream procedure (cause in one conference we might mute all but unmute leader)
				pPartyCntl->UnMuteImage();
		else
				pPartyCntl->MuteImage();
		}
	}

	//Change Layout Improvement - Layout Shared Memory (CL-SM)
	SendChangeLayoutToMultipleParties();

	//Send all the required intra request of relay participants
	m_intraDB.Flush();

	StartAutoLayout();
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfUpdateMuteAllVideoExceptLeaderDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfId:" << m_confRsrcID << " - Do nothing...";
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnSetPartyAsLeaderCONNECTED(CSegment* pParam)
{
	char targetPartyName[H243_NAME_LEN];
	EOnOff eOnOff = eOff;

	*pParam >> targetPartyName >> (BYTE&)eOnOff;
	targetPartyName[H243_NAME_LEN-1] = '\0';

	if (m_isMuteAllVideoButLeader)
	{
		TRACEINTO << "ConfName:" << m_pConfName << ", PartyName:" << targetPartyName << ", OnOff:" << (WORD)eOnOff;
		eOnOff = (eOnOff) ? eOff : eOn; // when we set as leader we unmute(mute with off) and we unset leader we need to mute video(mute with on)

		// If we set the party is leader need to unmute by operator,  Else if we unset from leader need to mute
		CSegment seg;
		seg << OPERATOR
		    << targetPartyName
		    << (BYTE)eOnOff
		    << (BYTE)eMediaIn;
		DispatchEvent(VIDEOMUTE, &seg);
	}
	else
	{
		TRACEINTO << "ConfName:" << m_pConfName << ", PartyName:" << targetPartyName << ", OnOff:" << (WORD)eOnOff << " - Mute all video but leader is off, no change needed";
	}
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfUpdateMuteAllVideoExceptLeaderForJustIncomingCONNECTED(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	BYTE newIsMuteAllVideoButLeader = NO;
	*pParam >> newIsMuteAllVideoButLeader;
	m_isMuteAllVideoButLeaderForJustIncoming = TRUE;
	if (m_isMuteAllVideoButLeader == newIsMuteAllVideoButLeader)
	{
		TRACEINTO << ",ConfName:" << m_pConfName << ", IsMuteAllVideoButLeader:" << (WORD)m_isMuteAllVideoButLeader << " - No change in value";
		return;
	}

	m_isMuteAllVideoButLeader = newIsMuteAllVideoButLeader;

	m_pConfApi->UpdateDB((CTaskApp*)0xFFFF, UPDATE_MUTE_ALL_VIDEO_EXCEPT_LEADER, m_isMuteAllVideoButLeader);
}
// -----------------------------------------------------------------------------------------
void CVideoBridgeCP::OnConfUpdateMuteAllVideoExceptLeaderForJustIncomingDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfId:" << m_confRsrcID << " - Do nothing...";
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::OnArtUpdateSsrcAckCONNECTED(CSegment* pParam)
{
	// get Party
	PartyRsrcID partyRsrcID;
	WORD status = statIllegal;
	*pParam >> (DWORD&)partyRsrcID >> status;

	TRACEINTO << "PartyId:" << partyRsrcID << ", Status:" << status << " - Ack From ART on update SSRC";

	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVideoPartyCntl));

	// send to Translator
	((CVideoRelayBridgePartyCntl*)pVideoPartyCntl)->UpdateArtOnTranslateVideoSSRCAck(status);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::OnMrmpStreamIsMustAckCONNECTED(CSegment* pParam)
{
	// get Party
	PartyRsrcID partyRsrcID;
	WORD status = statIllegal;
	*pParam >> (DWORD&)partyRsrcID >> status;

	TRACEINTO << "PartyId:" << partyRsrcID << ", Status:" << status << " - Ack From MRMP on update Must Stream";

	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVideoPartyCntl));

	// send to Translator
	if (pVideoPartyCntl->IsVideoRelayParty())
		((CVideoRelayBridgePartyCntl*)pVideoPartyCntl)->UpdateMrmpStreamIsMustAck(status);
	else
	{
		TRACEINTO << " Ack From MRMP on 'update Must Stream', not SVC party ??? ";
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::PrepareParamsForMixConf()
{
	TRACEINTO << " ";

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	if (!pCommConf)
		return;
	if (eMixAvcSvc != (eConfMediaType)pCommConf->GetConfMediaType())
		return;

	FindAllowedMixSvcToAvcLayerId();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::FindAllowedMixSvcToAvcLayerId()
{
	CVideoOperationPointsSet* pVideoOperationPointsSet = GetConfVideoOperationPointsSet();
	if (!pVideoOperationPointsSet)
	{
		TRACEINTO << " - SVC-Translator-Error: pVideoOperationPointsSet == NULL ";
		DBGPASSERT_AND_RETURN(101);
	}

	const std::list <VideoOperationPoint>* list = pVideoOperationPointsSet->GetOperationPointsList();
	std::list <VideoOperationPoint>::const_iterator itr =  list->begin();

	int  chosenLayerID = -1;
	int chosenLayerIDVsw = -1;

	for( ;itr != list->end(); itr++)
	{
			unsigned long  fs = ::CalcOperationPointFS( (*itr) );
		// for regular later-Id
		if ((fs < H264_HD720_FS) && ((*itr).m_layerId > chosenLayerID) && ((*itr).m_did <= 1))	// currently must be SD and not HD, This actually should be take somehow from the resources.
				chosenLayerID = (*itr).m_layerId;
		// for Vsw layer-Id
		if ((*itr).m_layerId > chosenLayerIDVsw)
			chosenLayerIDVsw = (*itr).m_layerId;
		}

	// here we need to choose the max layer ID we allowed to set for this conference BitRate
	if (-1 == chosenLayerID)
	{
		TRACEINTO << " SVC-Translator-Error: chosenLayerID = -1, cannot create translators";
		DBGPASSERT_AND_RETURN(101);
	}
	m_maxAllowedLayerIdForMixSvcToAvc = chosenLayerID;
	m_maxAllowedLayerIdForMixSvcToAvcHighRes = chosenLayerIDVsw;

	if (chosenLayerID == chosenLayerIDVsw)
		{TRACEINTO << " SVC-Translator-Warning: chosenLayerID == chosenLayerIDVsw, HD translator will be low-res ";}
	TRACEINTO << " maxAllowedLayerId: " << m_maxAllowedLayerIdForMixSvcToAvc << ", maxAllowedLayerIdVsw: " << m_maxAllowedLayerIdForMixSvcToAvcHighRes;
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::UpgradeToMixAvcSvcForRelayParty(PartyRsrcID partyRsrcID)
{
    CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(partyRsrcID));
    if (NULL == pVideoPartyCntl)
    {
        ReplyUpgradeToMixAvcSvcUponError(partyRsrcID);
        return;
    }

    if (ISDEBUGMODE("DYNAMIC_MIX", 1)) { ReplyUpgradeToMixAvcSvcUponError(partyRsrcID); return;}	// simulate error
    ISDEBUGMODE_RETURN("DYNAMIC_MIX", 2)															// simulate error without reply


	if (pVideoPartyCntl->IsVideoRelayParty())
	{
		TRACEINTO << " Connect Mix SvcToAvcTranslator ";
		((CVideoRelayBridgePartyCntl*)  pVideoPartyCntl)->UpgradeSvcToAvcTranslator();
	}
	else
	{
	      ReplyUpgradeToMixAvcSvcUponError(partyRsrcID);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::UpgradeToMixAvcSvcForNonRelayParty(PartyRsrcID partyRsrcID, CAvcToSvcParams *pAvcToSvcParams)
{
    CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(partyRsrcID));
    if (NULL == pVideoPartyCntl)
    {
       ReplyUpgradeToMixAvcSvcUponError(partyRsrcID);
       return;
    }

    if (ISDEBUGMODE("DYNAMIC_MIX", 3)) { ReplyUpgradeToMixAvcSvcUponError(partyRsrcID); return;}	// simulate error
    ISDEBUGMODE_RETURN("DYNAMIC_MIX", 4)															// simulate error without reply

	if (!pVideoPartyCntl->IsVideoRelayParty())
	{
	    TRACEINTO << " Connect Mix AvcToSvcTranslator ";
		pVideoPartyCntl->UpgradeAvcToSvcTranslator( pAvcToSvcParams );
	}
	else
	{
		ReplyUpgradeToMixAvcSvcUponError(partyRsrcID);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::ReplyUpgradeToMixAvcSvcUponError(PartyRsrcID partyRsrcID)
{
    TRACEINTO << "  Translator-Error - Video Party error, PartyRsrcID:  " << (DWORD)partyRsrcID;
	DBGPASSERT(101);

	m_pConfApi->ReplayUpgradeSvcAvcTranslate( partyRsrcID, VIDEO, statInconsistent );
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::DowngradeMixAvcSvcRelayParty(PartyRsrcID partyRsrcID)
{
    CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(partyRsrcID));
    if (NULL == pVideoPartyCntl)
    {
 		ReplyDowngradeMixAvcSvcUponError(partyRsrcID);
		return;
    }

	if (pVideoPartyCntl->IsVideoRelayParty())
	{
//		TRACEINTO << " Connect Mix SvcToAvcTranslator ";
//		((CVideoRelayBridgePartyCntl*)  pVideoPartyCntl)->DowngradeSvcToAvcTranslator();
	}
	else
	{
		ReplyDowngradeMixAvcSvcUponError(partyRsrcID);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::DowngradeMixAvcSvcNonRelayParty(PartyRsrcID partyRsrcID)
{
    CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)(GetPartyCntl(partyRsrcID));
    if (NULL == pVideoPartyCntl)
    {
		ReplyDowngradeMixAvcSvcUponError(partyRsrcID);
        return;
    }

	if (!pVideoPartyCntl->IsVideoRelayParty())
	{
//		TRACEINTO << " Connect Mix SvcToAvcTranslator ";
//		pVideoPartyCntl)->DowngradeSvcToAvcTranslator();
	}
	else
	{
		ReplyDowngradeMixAvcSvcUponError(partyRsrcID);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::ReplyDowngradeMixAvcSvcUponError(PartyRsrcID partyRsrcID)
{
    TRACEINTO << " Translator-Error - Video Party error, PartyRsrcID: " << (DWORD)partyRsrcID;
	DBGPASSERT(101);

	m_pConfApi->ReplayDowngradeSvcAvcTranslate( partyRsrcID, VIDEO, statInconsistent );
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::UpdateVidBrdgTelepresenseEPInfo(DWORD partyRsrcId, CTelepresenseEPInfo* pTelepresenseEPInfo, const char* pSiteName)
{
	// check params
	if(NULL == pTelepresenseEPInfo)
	{
		TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << " - pTelepresenseEPInfo is NULL, no update";
		return;
	}
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl *)(GetPartyCntl(partyRsrcId));
	if (NULL == pVideoPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << " - pVideoPartyCntl is NULL, no update";
		return;
	}

	// dump old and new info
	const CTelepresenseEPInfo&  rPartyTelepresenceInfo = pVideoPartyCntl->GetTelepresenceInfo();
	TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << ", old info:";
	rPartyTelepresenceInfo.Dump();

	TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << ", new info:";
	pTelepresenseEPInfo->Dump();

	// update VideoBridgePartyCntl m_telepresenceInfo & ITP site name
	bool is_sitename_change = (NULL != pSiteName) && (strcmp(pVideoPartyCntl->GetITPSiteName(), pSiteName) != 0);
	pVideoPartyCntl->SetITPSiteName(pSiteName);
	pVideoPartyCntl->SetTelepresenceInfo(pTelepresenseEPInfo);

	BOOL waitForUpdate = rPartyTelepresenceInfo.GetWaitForUpdate();
	eTelePresencePartyType oldPartyEPType = rPartyTelepresenceInfo.GetEPtype();
	eTelePresencePartyType newPartyEPType = pTelepresenseEPInfo->GetEPtype();
	bool epUpdatedToTelepresence = false;

	if((oldPartyEPType == eTelePresencePartyNone) && (TRUE == waitForUpdate) && ((newPartyEPType != eTelePresencePartyNone) && (newPartyEPType != eTelePresencePartyInactive)))
	{
		TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << " , EP updated to telepresence";
		epUpdatedToTelepresence = true;
	}

	// things to do when update TelepresenseEPInfo is received
	RoomID roomId = pTelepresenseEPInfo->GetRoomID();
	OnEndWaitingForTelepresenseEPInfo(partyRsrcId, roomId, epUpdatedToTelepresence, is_sitename_change);
}

//_e_m_
// This function is called from UpdateVidBrdgTelepresenseEPInfo (when update is received) or when reaching update timeout
/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::OnEndWaitingForTelepresenseEPInfo(DWORD partyRsrcId, RoomID roomId, bool epUpdatedToTelepresence /* = FALSE*/, bool isSiteNameChange /*= false*/)
{
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl *)(GetPartyCntl(partyRsrcId));
	if (NULL == pVideoPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << " - pVideoPartyCntl is NULL, no update";
		return;
	}
	const CTelepresenseEPInfo&  rPartyTelepresenceInfo = pVideoPartyCntl->GetTelepresenceInfo();

	// set off WaitForUpdate flag
	pVideoPartyCntl->SetTelepresenceInfoWaitForUpdate(FALSE);


	// Update room info in TelepresenceLayoutMngr if room exists

	if(NULL == m_pTelepresenceLayoutMngr){	// no telepresence manager
	  TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << ",m_pTelepresenceLayoutMngr is NULL - not updating m_pTelepresenceLayoutMngr room list";
	  return;
	}

	// find room
	CRoomInfo room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(roomId);
	if (room_info.GetRoomId() == (RoomID)-1)	// not found by RoomId
	{

		TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << ",room not found in rooms list - not updating m_pTelepresenceLayoutMngr room list";
		DumpTelepresenceInfo();
		return;
	}
	COstrStream msg;
	// BRIDGE-14660 check for site name update
	if (isSiteNameChange)
	{
		// remove from old room
		msg << "Site name change - removing link " << rPartyTelepresenceInfo.GetLinkNum() << " from room " << rPartyTelepresenceInfo.GetRoomID() << " and updating according to new site name";
		m_pTelepresenceLayoutMngr->UpdateRoomSubLink(rPartyTelepresenceInfo.GetRoomID(), rPartyTelepresenceInfo.GetLinkNum(), (PartyRsrcID)-1);
		// if we just removed the only link in thetoom, remove room
		if (room_info.GetNumberOfActiveLinks() == 1)
		{
			msg << ". no links left in room id " << rPartyTelepresenceInfo.GetRoomID() << ", removing room.";
			m_pTelepresenceLayoutMngr->RemoveRoom(rPartyTelepresenceInfo.GetRoomID());
		}

		// update in correct room
		TRACEINTO << msg.str();
		msg.flush();
		OnEndPartyConnectWithTelepresence(pVideoPartyCntl);

		room_info = m_pTelepresenceLayoutMngr->GetRoomInfo(rPartyTelepresenceInfo.GetRoomID());
		msg << " room id after update is " << room_info.GetRoomId();

	}


	// we are only taking care of the case when regular EP updated to Telepresence type (due to late identification)
	// in that stage links should not be connected yet
	PartyRsrcID room_main_party_id = room_info.GetLinkPartyId(0);
	if(room_main_party_id != partyRsrcId)
	{
		msg << " not the room main party (slave party) - no need to update room ,room_main_party_id = " << room_main_party_id << ", partyRsrcId = " << partyRsrcId;
	}
	else
	{
		if(epUpdatedToTelepresence)	// ****TBD - is this param needed???
		{
			if(room_info.GetRoomEPType() == eTelePresencePartyNone || room_info.GetRoomEPType() == eTelePresencePartyInactive)	// room type before update
			{
				msg << " updating room to telepresence ,roomId = " << roomId;

				// update room info
				room_info.UpdateRoomTypeAndMaxScreens(rPartyTelepresenceInfo.GetEPtype(), rPartyTelepresenceInfo.GetNumOfLinks());

				// check if all links are connected (not pending for other links)
				if (m_bManageTelepresenceLayoutsInternally)
				{
					// send change layout for EP with 1 screen only (none-telepresence or CTS [for other EPs - num of links value is not known yet])
					if (rPartyTelepresenceInfo.GetNumOfLinks() == 1)
					{
						BOOL isSpeakerInRoom = CheckIfRoomContainsTheCurrentSpeakerEp(room_info);
						SendChangeLayoutAfterRoomConnected(room_info, isSpeakerInRoom);
					}
				}
			}
		}
		else
		{
			msg << "\nEP Telepresence type was not changed - no need to update room ,roomId = "	//" room is already in telepresence mode - no need to update room ,roomId = "
					  << roomId;
		}
	}
	if ((long)msg.tellp() > 0)
		TRACEINTO << msg.str();
	DumpTelepresenceInfo();
}


/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::DumpTelepresenceInfo()
{
	if (NULL != m_pTelepresenceLayoutMngr)
    m_pTelepresenceLayoutMngr->DumpRoomList();

  std::ostringstream msg;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
      msg << "\n";
			pPartyCntl->GetTelepresenceInfo().Dump(msg);
    }
	}

	TRACEINTO << msg.str().c_str();
  }

/////////////////////////////////////////////////////////////////////////////////////////////
//   AV-MCU
/////////////////////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCP::PrepareMultiVSRMessage(ST_VSR_MUTILPLE_STREAMS* multipleVsr)
{
	TRACEINTO << " Sending Multi VSR to Main, ConfName: " << m_pConfName <<"\n";
	multipleVsr->num_vsrs_streams = MAX_STREAM_LYNC_2013_CONN;

	PASSERT_AND_RETURN(!m_pAVMCUMngr);
	CAVMCULinksVector& AVMCUVector = m_pAVMCUMngr->GetMCULinksVector();
	//FetchVideoMSIForLastMSPartyCntlsFromeEventPackage();

	for(WORD i = 0; i < MAX_STREAM_LYNC_2013_CONN; i++)
	{
		multipleVsr->st_vsrs_single_stream[i].msi= SOURCE_NONE_DOMINANT_SPEAKER_MSI;
		multipleVsr->st_vsrs_single_stream[i].partyId = 0;
	}

	DWORD partyRsrcId = 0;
	int j = 0;
	CAVMCULinksVector::iterator _ii = AVMCUVector.begin();

	for(_ii; _ii != AVMCUVector.end();_ii++)
	{
		if(j >= MAX_STREAM_LYNC_2013_CONN)break;
		partyRsrcId = *(_ii);
		CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl *)(GetPartyCntl(partyRsrcId));
		if (NULL == pVideoPartyCntl){
			TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << ",pVideoPartyCntl not found - no update";
			continue;
		}
		BOOL rIsInPortOpened = 0;
		BOOL rIsOutPortOpened = 0;
		pVideoPartyCntl->ArePortsOpened(rIsInPortOpened, rIsOutPortOpened);
		//if(pVideoPartyCntl->IsUniDirectionConnection(eMediaIn))
		if(rIsInPortOpened && IsValidMSI(pVideoPartyCntl->GetVideoMSI()))
		{
			DWORD partyIndex = pVideoPartyCntl->GetMSAVMCIndex();
			if (partyIndex >= MAX_STREAM_LYNC_2013_CONN-1)
			{
				PTRACE2INT(eLevelError,"CVideoBridgeCP::PrepareMultiVSRMessage: Invalid party index ", partyIndex);
				DBGPASSERT(partyIndex);
				continue;
			}
			CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
			CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRsrcId);
			if (!pImage)
			{
				PASSERTSTREAM(!pImage, "Failed, The lookup table doesn't have an element, PartyId:" << partyRsrcId); // BRIDGE-10767: just ignore this image and keep on
				continue;
			}

			if (m_pAVMCUMngr->GetVideoSwitchingModeType() == EventPackage::eVideoSwitchingModeType_ManualSwitched	&&	pVideoPartyCntl->GetPartyRsrcID() != m_pAVMCUMngr->GetMasterPartyRsrcID())
			{
				TRACEINTO << "  SpotLight case, Party is not the master, do not include it in VSR,  Party Name: " << pVideoPartyCntl->GetFullName();
				continue;
			}

			if (m_pAVMCUMngr->GetVideoSwitchingModeType() == EventPackage::eVideoSwitchingModeType_ManualSwitched && pVideoPartyCntl->GetPartyRsrcID() == m_pAVMCUMngr->GetMasterPartyRsrcID())
			{
				if(!IsValidMSI(m_pAVMCUMngr->GetSpotLightSpeakerMsi()))
				{
					TRACEINTO << "  SpotLight case, SpotLight Speaker is invalid,  do not include it in VSR,  Masters's Name: " << pVideoPartyCntl->GetFullName();
					continue;
				}
			}

			multipleVsr->st_vsrs_single_stream[partyIndex].msi= pVideoPartyCntl->GetVideoMSI();
			multipleVsr->st_vsrs_single_stream[partyIndex].partyId = pVideoPartyCntl->GetPartyRsrcID();
			TRACEINTO << "PartyId:" << pVideoPartyCntl->GetPartyRsrcID() << ", msi:" <<pVideoPartyCntl->GetVideoMSI();

			j++;
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::AssignVideoMSIForEachMSPartyCntl()
{
	if(!m_pAVMCUMngr) PASSERT_AND_RETURN(1);
	EventPackage::Manager& rLyncEventManager = EventPackage::Manager::Instance();

	EventPackage::LyncDshList lyncDSH;
	EventPackage::ApiLync::Instance().GetDSH(m_pAVMCUMngr->GetMasterPartyRsrcID(), lyncDSH);

	lyncDSH.erase(
				std::remove_if(lyncDSH.begin(), lyncDSH.end(), std::bind2nd(EventPackage::Predicate::DSH_Msi(), m_pAVMCUMngr->GetMSAVMCULocalAUdioMSI())), lyncDSH.end());


	CAVMCULinksVector& rAVMCULinksVector = m_pAVMCUMngr->GetMCULinksVector();
	EventPackage::LyncMsiList LyncVideoMSIVect;
	EventPackage::ApiLync::Instance().GetMsiList(m_pAVMCUMngr->GetMasterPartyRsrcID(), LyncVideoMSIVect, EventPackage::eMediaType_Video);

	LyncVideoMSIVect.erase(std::remove(LyncVideoMSIVect.begin(), LyncVideoMSIVect.end(), m_pAVMCUMngr->GetMSAVMCULocalVideoMSI()), LyncVideoMSIVect.end());

	EventPackage::LyncDshList::iterator  lyncDSHCleanIt = lyncDSH.begin();
	while (lyncDSHCleanIt != lyncDSH.end())
	{
		EventPackage::LyncDsh& rLinkMediaInfo = *lyncDSHCleanIt;

		if (!IsValidMSI(rLinkMediaInfo.video.msi))
		{
			TRACEINTO << "VideoMSI:" << rLinkMediaInfo.video.msi << ", ConfId:" << m_pConf->GetConfId() << " -  Video MSI is invalid, remove it from VB DSH List";
			lyncDSHCleanIt = lyncDSH.erase(lyncDSHCleanIt);
			continue;
		}
		EventPackage::LyncMsiList::iterator LyncVideoMSIVectCleanIt = std::find(LyncVideoMSIVect.begin(), LyncVideoMSIVect.end(), rLinkMediaInfo.video.msi);
		if (LyncVideoMSIVectCleanIt == LyncVideoMSIVect.end())
		{
			lyncDSHCleanIt = lyncDSH.erase(lyncDSHCleanIt);
			continue;
		}
		++lyncDSHCleanIt;
	}
    // If disconnected SpotLight Speaker re-joined the confernce
	ActionsToResumeDisconnectedSpotLightSpeaker();

	// Clean  lists from Video MSI already in use
	EventPackage::LyncMsiList::iterator msiVectCleanIt = LyncVideoMSIVect.begin();
	while(msiVectCleanIt != LyncVideoMSIVect.end())
	{

		PartyRsrcID partyId = (PartyRsrcID)(-1);
		if (GetPartyImageByVideoMSI(*msiVectCleanIt, partyId)  )
		{
			if(NULL != GetPartyCntl(partyId))
			{
				TRACEINTO " Msi is already allocated for one of AV-MCU Slaves/Master, remove it from candidate list, Msi: " << *msiVectCleanIt << " AV_MCU PartyID:" << partyId;

				msiVectCleanIt = LyncVideoMSIVect.erase(msiVectCleanIt);
				lyncDSH.erase(
						std::remove_if(lyncDSH.begin(), lyncDSH.end(), std::bind2nd(EventPackage::Predicate::DSH_Msi(), *msiVectCleanIt)), lyncDSH.end());
				continue;
			} else
			{
				TRACEINTO << "PartyId:" << partyId << " - Found in process image lookup table does not belong to this conf";
			}
		}
		++msiVectCleanIt;
	}


	for(CAVMCULinksVector::iterator AVMngrIt = rAVMCULinksVector.begin(); AVMngrIt != rAVMCULinksVector.end(); ++AVMngrIt )
	{
		PartyRsrcID partyRsrcId = *AVMngrIt;
		CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl *)(GetPartyCntl(partyRsrcId));
		if (NULL == pVideoPartyCntl)
		{

			TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << " - pVideoPartyCntl not found ";
			continue;
		}

		BOOL rIsOutPortOpened=0, rIsInPortOpened = 0;
		pVideoPartyCntl->ArePortsOpened(rIsInPortOpened, rIsOutPortOpened);
		//if(pVideoPartyCntl->IsUniDirectionConnection(eMediaIn))

		if(!rIsInPortOpened)
		{
			TRACEINTO << "PartyId:" << partyRsrcId << " This is not In Slave, Slave Name: " << pVideoPartyCntl->GetFullName() << "\n";
			continue;
		}

		DWORD currentPartyVideoMSI = pVideoPartyCntl->GetVideoMSI();

		if(IsValidMSI(currentPartyVideoMSI))
		{
			TRACEINTO << " Party Has already assigned a valid MSI,remove it from DSH and Video vectors,  msi  is: " <<  currentPartyVideoMSI  << " Slave Name: " << pVideoPartyCntl->GetFullName() << "\n";

			lyncDSH.erase(
					std::remove_if(lyncDSH.begin(), lyncDSH.end(), std::bind2nd(EventPackage::Predicate::DSH_Msi(), currentPartyVideoMSI)), lyncDSH.end());
			LyncVideoMSIVect.erase(std::remove(LyncVideoMSIVect.begin(), LyncVideoMSIVect.end(), currentPartyVideoMSI), LyncVideoMSIVect.end());
			continue;
		}

		EventPackage::LyncDshList::iterator  lyncDSHIt = lyncDSH.begin();
		EventPackage::LyncMsiList::iterator msiVectIt = LyncVideoMSIVect.begin();

		if(lyncDSHIt == lyncDSH.end())
		{
			TRACEINTO << " No available MSI left in DSH, Take MSI from Other source, Conf Name: " << m_pConfName;
			if(msiVectIt == LyncVideoMSIVect.end())
			{
				PASSERT(partyRsrcId);
				if(!pVideoPartyCntl->IsAVMCUMain())
				{
					TRACEINTO << " LyncVideoMSIVect is totaly used, No MSI for Slave, Delete Slave, Slave's Name: " << pVideoPartyCntl->GetFullName();
					DeleteAvMcuSlave(partyRsrcId);
				}
				else
					TRACEINTO << " LyncVideoMSIVect is totaly used, No MSI for AV_MCU Master, Master's Name: " << pVideoPartyCntl->GetFullName();
			}
			else
			{
				LyncMsi videoMSI = *msiVectIt;
				pVideoPartyCntl->SetVideoMSI(videoMSI);
				LyncVideoMSIVect.erase(std::remove(LyncVideoMSIVect.begin(), LyncVideoMSIVect.end(), videoMSI), LyncVideoMSIVect.end());
				TRACEINTO << " Setting Video MSI from LyncMsi vector : " << videoMSI << " To party: " << pVideoPartyCntl->GetFullName();

				char siteName[MAX_SITE_NAME_ARR_SIZE];
				if (BuildAVMCUSiteName(videoMSI, siteName))
				{
					TRACEINTO << pVideoPartyCntl->GetFullName() << ", SiteName:" << siteName << " - Setting Site Name (VideoMSI Vector case)";
					//pVideoPartyCntl->SetSiteName(siteName);
					SetSiteName(pVideoPartyCntl->GetName(),siteName);
				}
				UnmuteMsInSlave(partyRsrcId);
			}
		}
		else
		{
			EventPackage::LyncDsh& rLinkMediaInfo = *lyncDSHIt;
			LyncMsi videoMSI = rLinkMediaInfo.video.msi;
			pVideoPartyCntl->SetVideoMSI(videoMSI);
			TRACEINTO << " Setting Video MSI from DSH: " << videoMSI << " To party: " << pVideoPartyCntl->GetFullName();
			// Setting Site Name

			char siteName[MAX_SITE_NAME_ARR_SIZE];
			if (BuildAVMCUSiteName(videoMSI, siteName))
			{
				TRACEINTO << pVideoPartyCntl->GetFullName() << ", SiteName:" << siteName << " - Setting Site Name (DSH Vector case)";
				//pVideoPartyCntl->SetSiteName(siteName);
				SetSiteName(pVideoPartyCntl->GetName(),siteName);
			}

			UnmuteMsInSlave(partyRsrcId);

			lyncDSH.erase(
					std::remove_if(lyncDSH.begin(), lyncDSH.end(), std::bind2nd(EventPackage::Predicate::DSH_Msi(), rLinkMediaInfo.audio.msi)), lyncDSH.end());
			EventPackage::LyncMsiList::iterator _deletedMSIIt = LyncVideoMSIVect.erase(std::remove(LyncVideoMSIVect.begin(), LyncVideoMSIVect.end(), videoMSI), LyncVideoMSIVect.end());
			if(_deletedMSIIt == LyncVideoMSIVect.end())
			{
				TRACEINTO << " Video MSI is not found in LyncVideo MSI Vector: " << videoMSI << " ConfName: " << m_pConfName;
			}
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::UnmuteMsInSlave(PartyRsrcID partyRsrcId)
{
	if(m_pAVMCUMngr->GetVideoSwitchingModeType() != EventPackage::eVideoSwitchingModeType_ManualSwitched)
	{
		// unmute slave only when not spotlight mode
		EOnOff eMuteState = eOff;
			UpdateMute(partyRsrcId, eMuteState, MCMS);
	}
}


//--------------------------------------------------------------------------
void CVideoBridgeCP::FetchVideoMSIForLastMSPartyCntlsFromeEventPackage()
{
	if (!m_pAVMCUMngr)
		PASSERT_AND_RETURN(1);
	TRACEINTO << "ConfName:" << m_pConfName;
	EventPackage::Manager& EventPackage = EventPackage::Manager::Instance();
	const EventPackage::Conference* pLocalConference = EventPackage.GetConference(m_pAVMCUMngr->GetMasterPartyRsrcID());
	PASSERT_AND_RETURN(!pLocalConference);

	const EventPackage::Users::UsersContainer& users = pLocalConference->m_users.m_users;

	CAVMCULinksVector& rAVMCULinksVector = m_pAVMCUMngr->GetMCULinksVector();
	TRACEINTO " Number of AV-MCU elemnets in AV MCU Manager is: " << rAVMCULinksVector.size();
	CAVMCULinksVector::iterator AVMngrIt = rAVMCULinksVector.begin();
	for (EventPackage::Users::UsersContainer::const_iterator userIt = users.begin(); userIt != users.end(); ++userIt)
	{
		const EventPackage::User::EndpointsContainer& endpoints = userIt->m_endpoints;
		for (EventPackage::User::EndpointsContainer::const_iterator endPointIt = endpoints.begin(); endPointIt != endpoints.end(); ++endPointIt)
		{
			if (AVMngrIt == rAVMCULinksVector.end())
				return;
			const EventPackage::Endpoint::MediasContainer& medias = endPointIt->m_medias;
			EventPackage::Endpoint::MediasContainer::const_iterator _imediaAudio = std::find_if(medias.begin(), medias.end(), std::bind2nd(EventPackage::Predicate::Media_Type(), EventPackage::eMediaType_Audio));
			const EventPackage::Media* pLyncMedia = (_imediaAudio != medias.end()) ? &*_imediaAudio : NULL;
			if (pLyncMedia == NULL)
				continue;

			DWORD audioMsi = pLyncMedia->m_msi;
			if (audioMsi == m_pAVMCUMngr->GetMSAVMCULocalAUdioMSI())
			{
				TRACEINTO << " EP with LOcalAudio mSI was Detected, LocalAUdio MSI: " << m_pAVMCUMngr->GetMSAVMCULocalAUdioMSI() << " EP Audio MSI: " << audioMsi;
				continue;	// TBD - to disable cross MSI with DSH
			}

			EventPackage::Endpoint::MediasContainer::const_iterator _imediaVideo = std::find_if(medias.begin(), medias.end(), std::bind2nd(EventPackage::Predicate::Media_Type(), EventPackage::eMediaType_Video));
			pLyncMedia = (_imediaVideo != medias.end()) ? &*_imediaVideo : NULL;
			if (pLyncMedia == NULL)
				continue;

			DWORD videoMsi = pLyncMedia->m_msi;
			TRACEINTO << " Found Suitable Video MSI In Event Package, Video MSI: " << videoMsi << " Conf Name: " << m_pConfName;

			CVideoBridgePartyCntl* pVideoPartyCntl = NULL;
			while (pVideoPartyCntl == NULL && AVMngrIt != rAVMCULinksVector.end())
			{
				PartyRsrcID partyRsrcId = *AVMngrIt;
				pVideoPartyCntl = (CVideoBridgePartyCntl *) (GetPartyCntl(partyRsrcId));

				if (NULL == pVideoPartyCntl)
				{
					TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << " - pVideoPartyCntl not found";
				}
				else
				{
					BOOL rIsOutPortOpened = 0, rIsInPortOpened = 0;
					pVideoPartyCntl->ArePortsOpened(rIsInPortOpened, rIsOutPortOpened);
					//if(pVideoPartyCntl->IsUniDirectionConnection(eMediaIn))

					if (!rIsInPortOpened)
					{
						TRACEINTO << "PartyId:" << partyRsrcId << ", SlaveName:" << pVideoPartyCntl->GetFullName() << " - This is not In Slave";
						pVideoPartyCntl = NULL;
					}
				}
				++AVMngrIt;
			}
			if (pVideoPartyCntl)
			{
				pVideoPartyCntl->SetVideoMSI(videoMsi);
				TRACEINTO << "VideoMsi:" << videoMsi << ", PartyName:" << pVideoPartyCntl->GetFullName();
			}
		}
	}
	TRACEINTO << " End Of Function,  Conf Name: " << m_pConfName;
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::SendVSRToAVMCUMain()
{
	PASSERT_AND_RETURN(!m_pAVMCUMngr);

	PartyRsrcID partyId = m_pAVMCUMngr->GetMasterPartyRsrcID();
	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pVideoPartyCntl, "PartyId:" << partyId);

	ST_VSR_MUTILPLE_STREAMS multipleVsr;
	PrepareMultiVSRMessage(&multipleVsr);
	pVideoPartyCntl->SendVSRToSipParty(&multipleVsr);
}
//--------------------------------------------------------------------------
void CVideoBridgeCP::AddAVMCUPartyCntlToBridge(CVideoBridgePartyCntl* pPartyCntl)
{
    PASSERT_AND_RETURN(!pPartyCntl);

	if(pPartyCntl->IsAVMCUMain())
	{
		TRACEINTO << " End Connect AV-MCU Master, Create AV-MCU Mngr, Party Name :" << pPartyCntl->GetFullName() << "\n";

		if (NULL == m_pAVMCUMngr)
		{
			m_pAVMCUMngr= new CAVMCUMngr(this);
			// Subscribe for add events
			EventPackage::Manager& lyncEventManager = EventPackage::Manager::Instance();
			lyncEventManager.AddSubscriber(pPartyCntl->GetPartyRsrcID(), EventPackage::eEventType_MediaDeleted,std::make_pair((COsQueue*)&(m_pConfApi->GetRcvMbx()),VIDEO_BRIDGE_MSG));
			lyncEventManager.AddSubscriber(pPartyCntl->GetPartyRsrcID(), EventPackage::eEventType_MediaStatusUpdated,std::make_pair((COsQueue*)&(m_pConfApi->GetRcvMbx()),VIDEO_BRIDGE_MSG));
			lyncEventManager.AddSubscriber(pPartyCntl->GetPartyRsrcID(), EventPackage::eEventType_UserDisplayTextUpdated,std::make_pair((COsQueue*)&(m_pConfApi->GetRcvMbx()),VIDEO_BRIDGE_MSG));
			lyncEventManager.AddSubscriber(pPartyCntl->GetPartyRsrcID(), EventPackage::eEventType_EndpointDisplayTextUpdated,std::make_pair((COsQueue*)&(m_pConfApi->GetRcvMbx()),VIDEO_BRIDGE_MSG));
			lyncEventManager.AddSubscriber(pPartyCntl->GetPartyRsrcID(), EventPackage::eEventType_VideoSwitchingModeUpdated,std::make_pair((COsQueue*) &(m_pConfApi->GetRcvMbx()),VIDEO_BRIDGE_MSG));
		}
		m_pAVMCUMngr->SetMasterPartyRsrcID(pPartyCntl->GetPartyRsrcID());
	}
	if(pPartyCntl->IsAVMCUParty())
	{
		if(m_pAVMCUMngr)
			m_pAVMCUMngr->AddLinkToLinksVector(pPartyCntl->GetPartyRsrcID());
		else
			PASSERT(666);
	}
}
//--------------------------------------------------------------------------
void CVideoBridgeCP::replaceMSIOrDisconnectSlave(LyncMsi videoMsi, PartyRsrcID partyRsrcIDOfReplacedMSI, EventPackage::LyncMsiList& arrVideoMsi)
{
	PASSERT_AND_RETURN(!m_pAVMCUMngr);

	TRACEINTO << "VideoMsi:" << videoMsi << ", PartyId:" << partyRsrcIDOfReplacedMSI;

	CVideoBridgePartyCntl* pReplacedDeletedPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcIDOfReplacedMSI);
	PASSERT_AND_RETURN(!pReplacedDeletedPartyCntl);

	if(m_pLastActiveAudioSpeakerRequest && m_pLastActiveAudioSpeakerRequest == pReplacedDeletedPartyCntl->GetPartyTaskApp())
	{
		TRACEINTO << "Disconnected Msi is also the current audio speaker, make the audio speaker NULL, Video Msi: " << videoMsi << " Current Audio Speaker: " << pReplacedDeletedPartyCntl->GetFullName();
		AudioSpeakerChanged(NULL);
	}

	EventPackage::Manager& EventPackage = EventPackage::Manager::Instance();

	DWORD numberofAVMCUVideoSites = arrVideoMsi.size();
	EventPackage::LyncMsiList::iterator _ii = std::find(arrVideoMsi.begin(), arrVideoMsi.end(), m_pAVMCUMngr->GetMSAVMCULocalVideoMSI());
	if (_ii != arrVideoMsi.end())
		--numberofAVMCUVideoSites;

	if (numberofAVMCUVideoSites < m_pAVMCUMngr->GetNumAVMCULinks())
	{
		// Delete slave
		TRACEINTO << "NumberOfUsers:" << numberofAVMCUVideoSites << ", NumberOfLinks:" << m_pAVMCUMngr->GetNumAVMCULinks() << " - Delete one of the slaves is required";
		if (pReplacedDeletedPartyCntl->IsAVMCUMain())
		{
			PartyRsrcID partyIdOfMasterReplacedMSI = GetOldestAVMCUSpeaker(pReplacedDeletedPartyCntl->GetPartyRsrcID());
			if (partyIdOfMasterReplacedMSI != DUMMY_PARTY_ID)
			{
				CVideoBridgePartyCntl* pMasterReplacedDeletedPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyIdOfMasterReplacedMSI);
				PASSERT_AND_RETURN(!pMasterReplacedDeletedPartyCntl);
				pReplacedDeletedPartyCntl->SetVideoMSI(pMasterReplacedDeletedPartyCntl->GetVideoMSI());

				char siteName[MAX_SITE_NAME_ARR_SIZE];
				if (BuildAVMCUSiteName(pMasterReplacedDeletedPartyCntl->GetVideoMSI(), siteName))
				{
					TRACEINTO << "PartyId:" << pReplacedDeletedPartyCntl->GetPartyRsrcID() << ", SiteName:" << siteName << " - Setting site name to AV-MCU master";
					//pReplacedDeletedPartyCntl->SetSiteName(siteName);
					SetSiteName(pReplacedDeletedPartyCntl->GetName(),siteName);
				}
				// notify Master Party on delete pMastpMasterReplacedDeletedPartyCntl
				TRACEINTO << " Deleted MSI is on Master, Replace it with MSI oo anotehr MS Slave and delete this slave, Deleted Slave: " <<  pMasterReplacedDeletedPartyCntl->GetFullName() << " New Master's Video MSI: " << pMasterReplacedDeletedPartyCntl->GetVideoMSI();
				pMasterReplacedDeletedPartyCntl->SetVideoMSI(SOURCE_NONE_DOMINANT_SPEAKER_MSI);
				DeleteAvMcuSlave(partyIdOfMasterReplacedMSI);
			}
			else
			{
				TRACEINTO << " Deleted MSI is on Master, No slave available to switch with its MSI, setting its MSI to default.  Conf Name: " << m_pConfName;
				pReplacedDeletedPartyCntl->SetVideoMSI(SOURCE_NONE_DOMINANT_SPEAKER_MSI);
				// Mute Master by MCMS - remove from Mix
				UpdateMute(m_pAVMCUMngr->GetMasterPartyRsrcID(), eOn, MCMS);
			}
		}
		else
		{
			pReplacedDeletedPartyCntl->SetVideoMSI(SOURCE_NONE_DOMINANT_SPEAKER_MSI);
			DeleteAvMcuSlave(pReplacedDeletedPartyCntl->GetPartyRsrcID());
			TRACEINTO << " Deleted MSI is on one of MS Slave, delete this slave. Deleted Slave: " <<  pReplacedDeletedPartyCntl->GetFullName();
		}
	}
	else
	{
		// Replace  MSI
		TRACEINTO << "NumberOfUsers:" << numberofAVMCUVideoSites << ", NumberOfLinks:" << m_pAVMCUMngr->GetNumAVMCULinks() << " - Replace deleted MSI is required";

		pReplacedDeletedPartyCntl->SetVideoMSI(SOURCE_NONE_DOMINANT_SPEAKER_MSI);
		for (EventPackage::LyncMsiList::iterator _msi = arrVideoMsi.begin(); _msi != arrVideoMsi.end(); ++_msi)
		{
			PartyRsrcID partyId;

			if (*_msi != videoMsi && !GetPartyImageByVideoMSI(*_msi, partyId) && *_msi != m_pAVMCUMngr->GetMSAVMCULocalVideoMSI())
			{
				LyncMsi newVideoMsi = *_msi;

				// replace MSI
				TRACEINTO << "PartyId:" << pReplacedDeletedPartyCntl->GetPartyRsrcID() << ", New VideoMsi:" << newVideoMsi << " - Setting new MSI";
				pReplacedDeletedPartyCntl->SetVideoMSI(newVideoMsi);

				char siteName[MAX_SITE_NAME_ARR_SIZE];
				if (BuildAVMCUSiteName(newVideoMsi, siteName))
				{
					TRACEINTO << "PartyId:" << pReplacedDeletedPartyCntl->GetPartyRsrcID() << ", SiteName:" << siteName << " - Setting site name";
					//pReplacedDeletedPartyCntl->SetSiteName(siteName);
					SetSiteName(pReplacedDeletedPartyCntl->GetName(),siteName);
				}
				break;
			}
		}
	}
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::DeleteAvMcuSlave(PartyRsrcID slavePartyRsrcId)
{
	TRACEINTO << " Delete slave, partyRsrcId: " <<  slavePartyRsrcId;
	m_pConfApi->DeleteMsSlave(m_pAVMCUMngr->GetMasterPartyRsrcID(), slavePartyRsrcId);
	EOnOff eMuteState = eOn;
	UpdateMute(slavePartyRsrcId, eMuteState, MCMS);
}

//--------------------------------------------------------------------------
void CVideoBridgeCP::ActionsToResumeDisconnectedSpotLightSpeaker()
{
	TRACEINTO << "ConfName:" << m_pConfName;
	if(!m_pAVMCUMngr) PASSERT_AND_RETURN(1);

	EventPackage::Manager& rLyncEventManager = EventPackage::Manager::Instance();
	// Romem 12.2.14
	// In case of SpotLight, put SpotLight SPeaker on Master
	if(m_pAVMCUMngr->GetVideoSwitchingModeType() == EventPackage::eVideoSwitchingModeType_ManualSwitched)
	{
		LyncMsi spotlightSpeakervideoMSI;
		if(!IsValidMSI(m_pAVMCUMngr->GetSpotLightSpeakerMsi(),false))
		{
			std::string userSpotLightID = EventPackage::ApiLync::Instance().GetSpotlightUserId(m_pAVMCUMngr->GetMasterPartyRsrcID());
			if(!userSpotLightID.size())
			{
				TRACEINTO << " SpotLight User ID is empty, Conf Name: "  << m_pConfName;
			}
			else
			{

				LyncMsi reconnectedSpotLightVideoMsi = EventPackage::ApiLync::Instance().GetUserMSI(m_pAVMCUMngr->GetMasterPartyRsrcID(), userSpotLightID, EventPackage::eMediaType_Video);
				if (!IsValidMSI(reconnectedSpotLightVideoMsi))
				{
					TRACEINTO << " SpotLight Speaker does not have a valid Msi, Msi: " << reconnectedSpotLightVideoMsi << " Conf Name: " << m_pConfName;
				}
				else
				{
					TRACEINTO << " New SpotLight Spekaer Video Msi is: " << reconnectedSpotLightVideoMsi << " Conf Name: " << m_pConfName;
					m_pAVMCUMngr->SetSpotLightSpeakerMsi(reconnectedSpotLightVideoMsi);
					PutSpotLightSourceOnMasterAVMCU();
				}
			}
		}
	}
}
//--------------------------------------------------------------------------
void CVideoBridgeCP::ActionsOnAVMCUSpotLightOnOff()
{
	TRACEINTO << "ConfName:" << m_pConfName;
	PASSERT_AND_RETURN(!m_pAVMCUMngr);
	EOnOff eMuteState = eOn;

	switch(m_pAVMCUMngr->GetVideoSwitchingModeType())
	{
	case EventPackage::eVideoSwitchingModeType_ManualSwitched:
	{
		PutSpotLightSourceOnMasterAVMCU();
		break;
	}
	default:
	{
		eMuteState = eOff;
		UpdateMute(m_pAVMCUMngr->GetMasterPartyRsrcID(), eMuteState, MCMS);
	}
	}

	CAVMCULinksVector& AVMCUVector = m_pAVMCUMngr->GetMCULinksVector();


	for (CAVMCULinksVector::iterator _ii = AVMCUVector.begin(); _ii != AVMCUVector.end(); ++_ii)
	{
		PartyRsrcID partyRsrcId = *(_ii);
		CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl *)(GetPartyCntl(partyRsrcId));
		if (NULL == pVideoPartyCntl)
		{
			TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << ",pVideoPartyCntl not found - no update";
			continue;
		}

		if((pVideoPartyCntl->GetPartyRsrcID() == m_pAVMCUMngr->GetMasterPartyRsrcID()) || !pVideoPartyCntl->IsUniDirectionConnection(eMediaIn))
			continue;

		if (partyRsrcId != m_pAVMCUMngr->GetMasterPartyRsrcID())
		{

			UpdateMute(partyRsrcId, eMuteState, MCMS);
		}
	}
	// Send VSR
	SendVSRToAVMCUMain();
}
//--------------------------------------------------------------------------
void CVideoBridgeCP::PutSpotLightSourceOnMasterAVMCU()
{
	PASSERT_AND_RETURN(!m_pAVMCUMngr);
	if(m_pAVMCUMngr->GetVideoSwitchingModeType() != EventPackage::eVideoSwitchingModeType_ManualSwitched)
	{
		TRACEINTO << " Conference is not in Spot Light Mode, return function, Conf Name: " << m_pConfName;
		return;
	}

	EOnOff eMuteState = eOn;

	if(IsValidMSI(m_pAVMCUMngr->GetSpotLightSpeakerMsi(),false))
	{
		PartyRsrcID spotlightpartyId = DUMMY_PARTY_ID;
		CVideoBridgePartyCntl* pAVMCUMasterPartyCntl = (CVideoBridgePartyCntl *)(GetPartyCntl(m_pAVMCUMngr->GetMasterPartyRsrcID()));
		PASSERT_AND_RETURN(!pAVMCUMasterPartyCntl);

		// SpotLight Speaker is RMX
		if(m_pAVMCUMngr->GetSpotLightSpeakerMsi() == m_pAVMCUMngr->GetMSAVMCULocalVideoMSI())
		{
			TRACEINTO << " SpotLight Speaker is with LOcal RMKX VideoMSI, mute master video as well, Conf Name: " << m_pConfName;
			UpdateMute(m_pAVMCUMngr->GetMasterPartyRsrcID(), eMuteState, MCMS);
		}
		else
		{
			eMuteState = eOff;
			CImage* pImage = GetPartyImageByVideoMSI(m_pAVMCUMngr->GetSpotLightSpeakerMsi(), spotlightpartyId);
			if(pImage)
			{

				TRACEINTO << " SpotLight Speaker is on one of AV-MCU-Parties, Conf Name: " << m_pConfName;
				CVideoBridgePartyCntl* pVideoSPPeakerPartyCntl = (CVideoBridgePartyCntl *)(GetPartyCntl(spotlightpartyId));

				PASSERT_AND_RETURN(!pVideoSPPeakerPartyCntl);

				if(spotlightpartyId == m_pAVMCUMngr->GetMasterPartyRsrcID())
				{
					TRACEINTO << " SpotLight Speaker is on Master Party, mute Video of Master, Master Name: " << pVideoSPPeakerPartyCntl->GetFullName();
				}
				else  // switch Master->Party Of Spotlight speaker
				{
					LyncMsi masterCurrentVideoMsi = pAVMCUMasterPartyCntl->GetVideoMSI();
					char masterCurrentSiteName[MAX_SITE_NAME_ARR_SIZE];
					memset(masterCurrentSiteName,'\0',MAX_SITE_NAME_ARR_SIZE);
					strcpy_safe(masterCurrentSiteName, MAX_SITE_NAME_ARR_SIZE, pAVMCUMasterPartyCntl->GetSiteName());

					pAVMCUMasterPartyCntl->SetVideoMSI(pVideoSPPeakerPartyCntl->GetVideoMSI());
					//pAVMCUMasterPartyCntl->SetSiteName(pVideoSPPeakerPartyCntl->GetSiteName());
					SetSiteName(pAVMCUMasterPartyCntl->GetName(),pVideoSPPeakerPartyCntl->GetSiteName());
					pVideoSPPeakerPartyCntl->SetVideoMSI(masterCurrentVideoMsi);
					//pVideoSPPeakerPartyCntl->SetSiteName(masterCurrentSiteName);
					SetSiteName(pVideoSPPeakerPartyCntl->GetName(),masterCurrentSiteName);
				}
			}
			else
			{
				TRACEINTO << " SpotLight Speaker is Not on one of AV-MCU-Parties, Conf Name: " << m_pConfName;
				pAVMCUMasterPartyCntl->SetVideoMSI(m_pAVMCUMngr->GetSpotLightSpeakerMsi());
				char siteName[MAX_SITE_NAME_ARR_SIZE];
				if(BuildAVMCUSiteName(m_pAVMCUMngr->GetSpotLightSpeakerMsi(), siteName))
				{
					TRACEINTO << " SiteName:" << siteName << " - Setting site name to AV-MCU master";
					//pAVMCUMasterPartyCntl->SetSiteName(siteName);
					SetSiteName(pAVMCUMasterPartyCntl->GetName(),siteName);
				}
			}

			UpdateMute(m_pAVMCUMngr->GetMasterPartyRsrcID(), eMuteState, MCMS);
		}

	}
}
//---------------------------------------------------------------------------------------
bool CVideoBridgeCP::BuildAVMCUSiteName(LyncMsi videoMsi, char* siteName)
{
	PASSERTSTREAM_AND_RETURN_VALUE(!m_pAVMCUMngr, "VideoMsi:" << videoMsi, false);

	EventPackage::Manager& EventPackage = EventPackage::Manager::Instance();

	EventPackage::MediaInfo info;
	EventPackage.GetMedia(m_pAVMCUMngr->GetMasterPartyRsrcID(), std::bind2nd(EventPackage::Predicate::Media_Msi(), videoMsi), info);

	PASSERTSTREAM_AND_RETURN_VALUE(!info.user, "VideoMsi:" << videoMsi, false);
	PASSERTSTREAM_AND_RETURN_VALUE(!info.endpoint, "VideoMsi:" << videoMsi, false);

	return BuildAVMCUSiteName(info.user->m_displayText, info.endpoint->m_displayText, siteName);
}

//--------------------------------------------------------------------------
bool CVideoBridgeCP::BuildAVMCUSiteName(const std::string& userName, const std::string& endpointName, char* siteName)
{
	std::string localSiteName;

	if(!userName.empty())
	{
		localSiteName = userName;
	}

	if (!endpointName.empty())

	if (!userName.empty())
	{
		localSiteName = userName;
	}

	if (!endpointName.empty())
	{
		localSiteName += ".";
		localSiteName += endpointName;
	}

	if(!localSiteName.empty())
	{
		strcpy_safe(siteName, MAX_SITE_NAME_ARR_SIZE, localSiteName.c_str());
	}
	else
	{
		TRACEINTO << "Site Name is empty ";
		return false;
	}

	TRACEINTO << "UserName:" << userName << ", EndpointName:" << endpointName << ", SiteName:" << siteName;

	return true;

}

//--------------------------------------------------------------------------
void CVideoBridgeCP::SendPrivateLayoutEventsToAllAVMCUOutSlavesandToMain(WORD event, CVideoBridgePartyCntl* initiotorVideoPartyControl,CVideoLayout& layout, WORD isPrivateButtonOnly)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	PASSERT_AND_RETURN(!initiotorVideoPartyControl);
	if(initiotorVideoPartyControl->IsAVMCUMain())
	{
		if(!m_pAVMCUMngr)
		{
			PASSERT_AND_RETURN(500);
		}
		else
		{
			CAVMCULinksVector& AVMCUVector = m_pAVMCUMngr->GetMCULinksVector();

			CAVMCULinksVector::iterator _ii = AVMCUVector.begin();

			for(_ii; _ii != AVMCUVector.end();_ii++)
			{
				PartyRsrcID partyRsrcId = *(_ii);
				CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl *)(GetPartyCntl(partyRsrcId));
				if (NULL == pVideoPartyCntl){
					TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << ",pVideoPartyCntl not found - no update";
					continue;
				}
				if( pVideoPartyCntl->IsAVMCUMain() || pVideoPartyCntl->IsUniDirectionConnection(eMediaOut))
				{
					switch(event)
					{
					case SETPRIVATEVIDLAYOUT:
					{
						TRACEINTO << " Forward SETPRIVATEVIDLAYOUT event to AV-MCU Msater and Out slaves, Send to Party: " << pVideoPartyCntl->GetFullName();
						pVideoPartyCntl->ChangePartyPrivateLayout(layout);
						break;
					}
					case SETCONFVIDLAYOUT_SEEMEPARTY:
					{
						TRACEINTO << " Forward SETCONFVIDLAYOUT_SEEMEPARTY event to AV-MCU Msater and Out slaves, Send to Party: " << pVideoPartyCntl->GetFullName();
						pVideoPartyCntl->ChangePartyPrivateLayout(layout);
						break;
					}
					case SETPRIVATEVIDLAYOUTONOFF:
					{
						TRACEINTO << " Forward SETPRIVATEVIDLAYOUTONOFF event to AV-MCU Msater and Out slaves, Send to Party: " << pVideoPartyCntl->GetFullName();
						pVideoPartyCntl->ChangeLayoutPrivatePartyButtonOnly(isPrivateButtonOnly);
						break;
					}
					default:
					{
						PASSERT(event);
					}
					}

				}
			}

		}
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------------------------


PBEGIN_MESSAGE_MAP(CVideoBridgeCPContent)

  ONEVENT(ENDDISCONNECTPARTY,                 CONNECTED,      CVideoBridgeCPContent::OnEndPartyDisConnect)
  ONEVENT(ENDDISCONNECTPARTY,                 DISCONNECTING,  CVideoBridgeCPContent::OnEndPartyDisConnect)

  ONEVENT(CONTENT_BRIDGE_START_PRESENTATION,  CONNECTED,      CVideoBridgeCPContent::OnConfContentBridgeStartPresentationCONNECTED)
  ONEVENT(CONTENT_BRIDGE_START_PRESENTATION,  DISCONNECTING,  CVideoBridgeCPContent::NullActionFunction)

  ONEVENT(CONTENT_BRIDGE_STOP_PRESENTATION,   CONNECTED,      CVideoBridgeCPContent::OnConfContentBridgeStopPresentationCONNECTED)
  ONEVENT(CONTENT_BRIDGE_STOP_PRESENTATION,   DISCONNECTING,  CVideoBridgeCPContent::NullActionFunction)

  ONEVENT(ENDCONNECT_CONTENT_DECODER,         CONNECTED,      CVideoBridgeCPContent::OnEndContentDecoderPartyConnectCONNECTED)
  ONEVENT(ENDCONNECT_CONTENT_DECODER,         DISCONNECTING,  CVideoBridgeCPContent::NullActionFunction)

  ONEVENT(CONTENT_DECODER_VIDEO_IN_SYNCED,    CONNECTED,      CVideoBridgeCPContent::OnContentDecoderVideoInSyncedCONNECTED)
  ONEVENT(CONTENT_DECODER_VIDEO_IN_SYNCED,    DISCONNECTING,  CVideoBridgeCPContent::OnContentDecoderVideoInSyncedDISCONNECTING)

  ONEVENT(CONTENT_DECODER_SYNC_LOST,          CONNECTED,      CVideoBridgeCPContent::OnContentDecoderSyncLostCONNECTED)
  ONEVENT(CONTENT_DECODER_SYNC_LOST,          DISCONNECTING,  CVideoBridgeCPContent::NullActionFunction)

  ONEVENT(END_DISCONNECT_CONTENT_DECODER,     CONNECTED,      CVideoBridgeCPContent::OnEndDisconnectContentDecoderCONNECTED)
  ONEVENT(END_DISCONNECT_CONTENT_DECODER,     DISCONNECTING,  CVideoBridgeCPContent::NullActionFunction)//OnDisconnecting it should be also destroied

  ONEVENT(END_DESTROY_CONTENT_DECODER,        CONNECTED,      CVideoBridgeCPContent::OnEndDestroyContentDecoderCONNECTED)
  ONEVENT(END_DESTROY_CONTENT_DECODER,        DISCONNECTING,  CVideoBridgeCPContent::OnEndDestroyContentDecoderDISCONNECTING)

  ONEVENT(CONTENT_DECODER_ALLOC_RSRC_FAIL,    CONNECTED,      CVideoBridgeCPContent::OnContentDecoderAllocateRsrcFailureCONNECTED)
  ONEVENT(CONTENT_DECODER_ALLOC_RSRC_FAIL,    DISCONNECTING,  CVideoBridgeCPContent::NullActionFunction)

  ONEVENT(CONTENT_DECODER_RESET_FAIL_STATUS,  CONNECTED,      CVideoBridgeCPContent::OnContentDecoderResetAllocateRsrcFailureCONNECTED)
  ONEVENT(CONTENT_DECODER_RESET_FAIL_STATUS,  DISCONNECTING,  CVideoBridgeCPContent::NullActionFunction)

  ONEVENT(SETCONFVIDLAYOUT_SEEMEPARTY,        CONNECTED,      CVideoBridgeCPContent::OnConfSetConfVideoLayoutSeeMePartyCONNECTED)

  ONEVENT(SETPRIVATEVIDLAYOUTONOFF,           CONNECTED,      CVideoBridgeCPContent::OnConfSetPrivateVideoLayoutOnOffCONNECTED)

PEND_MESSAGE_MAP(CVideoBridgeCPContent, CVideoBridgeCP);

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeCPContent
////////////////////////////////////////////////////////////////////////////
CVideoBridgeCPContent::CVideoBridgeCPContent()
{
  m_isLegacyPartyConnected            = NO;
  m_ContentProtocol                   = 0xFFFFFFFF;
  m_ContentRate                       = 0;
  m_pContentDecoder                   = NULL;

  VALIDATEMESSAGEMAP;
}
// ------------------------------------------------------------------------------------------
CVideoBridgeCPContent::~CVideoBridgeCPContent()
{
  POBJDELETE(m_pContentDecoder);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::Create(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
  if (!CPObject::IsValidPObjectPtr(pVideoBridgeInitParams))
  {
    PASSERT(1);
    m_pConfApi->EndVidBrdgConnect(statInconsistent);
    return;
  }

  CVideoBridgeCP::CreateBase(pVideoBridgeInitParams);

  m_pContentDecoder = new CVideoBridgePartyCntlContent(this);
  m_state           = CONNECTED;

  m_pConfApi->EndVidBrdgConnect(statOK);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl, CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
  if (YES == pVideoBridgePartyInitParams->IsLegacyParty() && !pVideoBridgePartyInitParams->GetIsVideoRelay() && !pVideoBridgePartyInitParams->GetMsMasterPartyRsrcId() /* AV-MCU*/)
  {
    NewPartyCntl(pVideoBrdgPartyCntl);
    pVideoBrdgPartyCntl->Create(pVideoBridgePartyInitParams);
  }
  else
    CVideoBridgeCP::CreateAndNewPartyCntl(pVideoBrdgPartyCntl, pVideoBridgePartyInitParams);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::NewPartyCntl - ConfName:", m_pConfName);
  pVideoBrdgPartyCntl = new CVideoBridgePartyCntlLegacy();
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnConfTerminateDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnConfTerminateDISCONNECTING - ConfName:", m_pConfName);
  ContentDecoderConditionChanged(eCondition_confTerminate);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnEndDestroyContentDecoderDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnPartyEndDeAllocContentDecoderDISCONNECTING - ConfName:", m_pConfName);
  CVideoBridgeCP::OnConfTerminateDISCONNECTING(pParam);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnEndDestroyContentDecoderCONNECTED(CSegment* pParam)
{
  CTaskApp* pParty = NULL;
  WORD      status = statIllegal;

  *pParam >> (void*&)pParty >> status;

  if (statOK == status)
    TRACEWARN << "CVideoBridgeCPContent::OnEndDestroyContentDecoderCONNECTED - ContentDecoder problem (SETUP Failed), ContentDecoder deallocated, ConfName:" << m_pConfName;
  else
  {
    TRACEWARN << "CVideoBridgeCPContent::OnEndDestroyContentDecoderCONNECTED - ContentDecoder problem (SETUP Failed), ContentDecoder deallocation failed, ConfName:" << m_pConfName;
    PASSERT(1);
  }
}

//--------------------------------------------------------------------------
//Changed for Legacy party only NOT for content decoder
void CVideoBridgeCPContent::OnEndPartyConnectCONNECTED(CSegment* pParam)
{
	PartyRsrcID PartyId;
	CSegment* pTempParam = new CSegment(*pParam);
	*pTempParam >> PartyId;
	POBJDELETE(pTempParam)

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << PartyId;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	// Do action for legacy party
	if (pPartyCntl->IsLegacyParty())
		ContentDecoderConditionChanged(eCondition_LegacyConnected);

	CVideoBridgeCP::OnEndPartyConnectCONNECTED(pParam);
}


// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnConfDisConnectPartyCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnConfDisConnectPartyCONNECTED - ConfName:", m_pConfName);

  CVideoBridgeCP::OnConfDisConnectPartyCONNECTED(pParam);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnConfDisConnectPartyDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnConfDisConnectPartyDISCONNECTING - ConfName:", m_pConfName);

  CVideoBridgeCP::OnConfDisConnectPartyDISCONNECTING(pParam);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnEndPartyDisConnect(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnEndPartyDisConnect - ConfName:", m_pConfName);

  CVideoBridgeCP::OnEndPartyDisConnect(pParam);

  // To avoid checking the list for each party ,we have to set a new opcode for ENDPARTYDISCONNECT for legacy
  // this cause a lot of code duplication.
  if (IsLastLegacyPartyLeftConf())
    ContentDecoderConditionChanged(eCondition_LastLegacyLeftConf);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnConfContentBridgeStartPresentationCONNECTED(CSegment* pParam)
{
	*pParam >> m_ContentProtocol >> m_ContentRate >> m_isContentHD1080Supported;

	TRACEINTO << "ConfName:" << m_pConfName
		<< ", ContentProtocol:" << m_ContentProtocol
		<< ", ContentRate:" << m_ContentRate
		<< ", IsContentHD1080Supported:" << (int)m_isContentHD1080Supported;

	// Support Content Snatching in XCode
	if (IsConfInActiveContentPresentation() && IsXCodeConf())
	{
		ContentDecoderConditionChanged(eCondition_SnatchPresentation);
	}
	else
	{
		ContentDecoderConditionChanged(eCondition_StartPresentation);
	}

	StartTimer(AFTER_START_CONTENT_TIMER, AFTER_START_CONTENT_TOUT);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnConfContentBridgeStopPresentationCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnConfContentBridgeStopPresentationCONNECTED - ConfName:", m_pConfName);

  m_ContentProtocol = 0xFFFFFFFF;
  m_ContentRate     = 0;

  ContentDecoderConditionChanged(eCondition_StopPresentation);
  RemoveContentDecoderFromConfMixBeforeContentStoped();

  StartTimer(AFTER_STOP_CONTENT_TIMER, AFTER_STOP_CONTENT_TOUT);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnEndContentDecoderPartyConnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnEndContentDecoderPartyConnectCONNECTED - ConfName:", m_pConfName);
  AskContentSpeakerForIntra(); // to sync the ContentDecoder
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnEndDisconnectContentDecoderCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnEndDisconnectContentDecoderCONNECTED - Do nothing... ConfName:", m_pConfName);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnContentDecoderSyncLostCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnContentDecoderSyncLostCONNECTED - ConfName:", m_pConfName);
  AskContentSpeakerForIntra(); // to sync the ContentDecoder,to see if to send only if presentation+Legacy connected
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnContentDecoderVideoInSyncedCONNECTED(CSegment* pParam)
{
  AddContentDecoderToConfMixAfterVideoInSynced();
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::AskContentSpeakerForIntra()
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::AskContentSpeakerForIntra - ConfName:", m_pConfName);
  // Ask Conf to ask ContentSpeaker For Intra
  BYTE controlID = 0xFF; // NOT relevant for content bridge but used by Atara for some reason - NEED to be removed !!
  m_pConfApi->ContentVideoRefresh(controlID);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnContentDecoderVideoInSyncedDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnContentDecoderVideoInSyncedDISCONNECTING - Do nothing... ConfName:", m_pConfName);
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::AddContentDecoderToConfMixAfterVideoInSynced()
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::AddContentDecoderToConfMixAfterVideoInSynced - ConfName:", m_pConfName);

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
    {
			if (pPartyCntl->IsLegacyParty())   // IsLegacyParty To make small changes in the code of Normal VideoBridgePartyCntl
				((CVideoBridgePartyCntlLegacy*)pPartyCntl)->AddContentImage();
    }
  }
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::RemoveContentDecoderFromConfMixBeforeContentStoped()
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::RemoveContentDecoderFromConfMixBeforeContentStoped - ConfName:", m_pConfName);

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
    {
			if (pPartyCntl->IsLegacyParty()) // IsLegacyParty To make small changes in the code of Normal VideoBridgePartyCntl
				((CVideoBridgePartyCntlLegacy*)pPartyCntl)->DelContentImage();
    }
  }
}

// ------------------------------------------------------------------------------------------
CVideoBridgePartyCntlContent* CVideoBridgeCPContent::GetContentDecoder()
{
  return m_pContentDecoder;
}
// ------------------------------------------------------------------------------------------
CImage* CVideoBridgeCPContent::GetContentImage()
{
  CBridgePartyVideoIn* pVideoIn = (CBridgePartyVideoIn*)GetContentDecoder()->GetBridgePartyIn();
  PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pVideoIn), NULL);

  return (CImage*)pVideoIn->GetPartyImage();
}
// ------------------------------------------------------------------------------------------
BOOL CVideoBridgeCPContent::IsContentImageNeedToBeAdded()
{
  CVideoBridgePartyCntlContent* pContentDecodr = GetContentDecoder();
  if (pContentDecodr)
    if (IsConfInActiveContentPresentation() && pContentDecodr->IsContentDecoderSynced())
      return YES;
  return NO;
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::ContentDecoderConditionChanged(EConditionForContentDecoder conditionType)
{
  switch (conditionType)
  {
    case (eCondition_confTerminate):
    {
      SetIsConfInActiveContentPresentation(NO);
      if (m_pContentDecoder)
        m_pContentDecoder->Destroy();
      break;
    }

    case (eCondition_StartPresentation):
    {
      SetIsConfInActiveContentPresentation(YES);
      ActionOnConditionChange(YES);
      UpdateLayoutHandlerTypeIfNeeded();
      break;
    }

    case (eCondition_StopPresentation):
    {
      SetIsConfInActiveContentPresentation(NO);
      ActionOnConditionChange(NO);
      UpdateLayoutHandlerTypeIfNeeded();
      break;
    }
    case (eCondition_LegacyConnected):
    {
      if(IsXCodeConf()) break;
      SetIsLegacyPartyConnected(YES);
      ActionOnConditionChange(YES);
      break;
    }

    case (eCondition_LastLegacyLeftConf):
    {
      if(IsXCodeConf()) break;
      SetIsLegacyPartyConnected(NO);
      ActionOnConditionChange(NO); // Do nothing with the decoder in this case only reset  the conf status in the DB.
      break;
    }

    case (eCondition_SnatchPresentation):
    {
      if(!IsXCodeConf()) break;
      m_pContentDecoder->UpdateVideoInParamsOnChange();
      break;
    }

	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::ActionOnConditionChange(BYTE YesNoCondition)
{
  if (m_pContentDecoder)
  {
    if (YES == YesNoCondition)                // First Legacy is now connected or StartContent, One of them is YES in this Condition Change ==> check the other
    {
      if ((YES == IsLegacyPartyConnected() || IsXCodeConf()) && (YES == IsConfInActiveContentPresentation()))
      {
        PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::ActionOnConditionChange - Legacy party connected and presentation mode is ON, so connect content decoder, ConfName:", m_pConfName);
        BYTE isIVR = NO;
        m_pContentDecoder->Connect(isIVR, m_isContentHD1080Supported);
      }
    }
    else                                      // (NO == YesNoCondition) ==> at least one of them is NO Stop the Legacy feature.We are here only on Stop Content since when the last party left, the decoder is still connected
    {
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::ActionOnConditionChange - Legacy party disConnected ot presentation mode is OFF, ConfName:", m_pConfName);
      ResetDecoderFailureStatusInConf(); // Remove CONFERENCE_CONTENT_RESOURCES_DEFICIENCY from DB since it is Not relevant any more
      if (NO == IsConfInActiveContentPresentation())
      {
        m_pContentDecoder->DisConnect();      // Disconnect the content decoder only in Stop Presentation.
        if (IsXCodeConf())
        {
          m_pConfApi->ContentDecoderDisconnected(NULL, XCODE_BRDG_MSG, statOK);
        }
      }
    }
  }
  else
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::ActionOnConditionChange - Failed, content decoder must be already allocated in this stage, ConfName:", m_pConfName);
    PASSERT_AND_RETURN(1);
  }
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::SetIsLegacyPartyConnected(BYTE yesNo)
{
  m_isLegacyPartyConnected = yesNo;
}
// ------------------------------------------------------------------------------------------
BYTE CVideoBridgeCPContent::IsLegacyPartyConnected()
{
  return m_isLegacyPartyConnected;
}


// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::UpdateLayoutHandlerTypeIfNeeded()	// BRIDGE-11211 - lagacy & Telepresence
{
	// Update CBridgePartyVideoOut with the relevant layout handler
	if ((TRUE == m_telepresenceOnOff) && (TRUE == m_bManageTelepresenceLayoutsInternally))
	{
		CBridgePartyList::iterator _end = m_pPartyList->end();
		for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
		{
			// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
			CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
			if (pPartyCntl)
			{
				if (pPartyCntl->IsLegacyParty())
					pPartyCntl->UpdateLayoutHandlerType();
			}
		}
	}
}

// ------------------------------------------------------------------------------------------
BYTE CVideoBridgeCPContent::IsLastLegacyPartyLeftConf()
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
  {
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
    {
			if (pPartyCntl->IsLegacyParty())
      return NO;
  }
	}
  return YES;
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnEndPartyExportCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnEndPartyExportCONNECTED - ConfName:", m_pConfName);

  CVideoBridgeCP::EndPartyExport(pParam, PARTY_VIDEO_EXPORTED);

  // To avoid checking the list for each party ,we have to set a new opcode for ENDPARTYDISCONNECT for legacy
  // this cause a lot of code duplication.
  if (IsLastLegacyPartyLeftConf())
    ContentDecoderConditionChanged(eCondition_LastLegacyLeftConf);
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnContentDecoderAllocateRsrcFailureCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnContentDecoderAllocateRsrcFailureCONNECTED - ConfName:", m_pConfName);

  // Inform Conf on AllocFail
  m_pConfApi->ContentDecoderAllocFail();
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnContentDecoderResetAllocateRsrcFailureCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnContentDecoderResetAllocateRsrcFailureCONNECTED - ConfName:", m_pConfName);

  ResetDecoderFailureStatusInConf();
}
// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::ResetDecoderFailureStatusInConf()
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::ResetDecoderFailureStatusInConf - ConfName:", m_pConfName);

  m_pConfApi->ContentDecoderResetFailStatus();
}
void CVideoBridgeCPContent::OnConfSetConfVideoLayoutSeeMePartyCONNECTED(CSegment* pParam)
{
	char partyName[H243_NAME_LEN];
	*pParam >> partyName;
	partyName[H243_NAME_LEN-1] = '\0';

	CVideoBridgePartyCntl* pCurrParty = (CVideoBridgePartyCntl*)GetPartyCntl(partyName);
	PASSERTSTREAM_AND_RETURN(!pCurrParty, "PartyName:" << partyName << " - Party not found");

	std::ostringstream msg;
	msg
		<< "PartyId:"           << pCurrParty->GetPartyRsrcID()
		<< ", PartyName:"       << partyName
		<< ", CascadeLinkMode:" << CascadeModeToString(pCurrParty->GetCascadeLinkMode())
		<< ", IsLegacyParty:"   << (int)pCurrParty->IsLegacyParty()
		<< ", IsContent:"       << (int)IsConfInActiveContentPresentation()
		<< ", IsTelepresence:"  << (m_pTelepresenceLayoutMngr ? "1" : "0");

	if (pCurrParty->IsLegacyParty() && IsContentImageNeedToBeAdded())
	{
		TRACEINTO << msg.str().c_str() << " - Change to Legacy layout";

		// change layout to Legacy!!!
		LayoutType legacyContentDefaultLayout = GetLegacyContentDefaultLayout();

		CVideoLayout legacyLayout;
		legacyLayout.m_active       = YES;
		legacyLayout.m_numb_of_cell = GetNumbSubImg(legacyContentDefaultLayout);
		legacyLayout.m_screenLayout = ::GetOldLayoutType(legacyContentDefaultLayout);

		char* arrStr = "[Auto Select]";
		for (WORD i = 0; i < legacyLayout.m_numb_of_cell; i++)
		{
			legacyLayout.m_pCellLayout[i]                   = new CVideoCellLayout;
			legacyLayout.m_pCellLayout[i]->m_cellId         = i + 1;
			legacyLayout.m_pCellLayout[i]->m_cellStatus     = 0;
			legacyLayout.m_pCellLayout[i]->m_forcedPartyId  = 0xFFFFFFFF;
			legacyLayout.m_pCellLayout[i]->m_currentPartyId = 0xFFFFFFFF;
			legacyLayout.m_pCellLayout[i]->SetName(arrStr);
		}
		pCurrParty->ChangePartyPrivateLayout(legacyLayout);
	}
	else
	{
		if (m_IsSameLayout)
		{
			// means no forces in party level
			TRACEINTO << msg.str().c_str() << " - SameLayot flag is ON, so no private layout";
			return;
		}

		CVideoLayout layout;
		layout.DeSerialize(NATIVE, *pParam);

		string s;
		layout.ToString(s);
		TRACEINTO << msg.str().c_str() << "\n" << s.c_str();

		pCurrParty->ChangePartyLayout(layout);
	}
}
// ------------------------------------------------------------------------------------------
LayoutType CVideoBridgeCPContent::GetLegacyContentDefaultLayout()
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	PASSERT_AND_RETURN_VALUE(!pSysConfig, CP_LAYOUT_1P4VER);

	std::string legacyContentDefaultLayoutStr;
	pSysConfig->GetDataByKey("LEGACY_EP_CONTENT_DEFAULT_LAYOUT", legacyContentDefaultLayoutStr);
	LayoutType legacyContentDefaultLayout = TranslateSysConfigStringToLayoutType(legacyContentDefaultLayoutStr);
	PASSERT_AND_RETURN_VALUE(CP_NO_LAYOUT == legacyContentDefaultLayout, CP_LAYOUT_1P4VER);

	TRACEINTO << "LayoutType:" << LayoutTypeAsString[legacyContentDefaultLayout];

	return legacyContentDefaultLayout;
}

// ------------------------------------------------------------------------------------------
void CVideoBridgeCPContent::OnConfSetPrivateVideoLayoutOnOffCONNECTED(CSegment* pParam)
{
	TRACECOND_AND_RETURN(m_IsSameLayout, "CVideoBridgeCPContent::OnConfSetPrivateVideoLayoutOnOffCONNECTED - Failed, SameLayot flag is ON, so no private layout");

	PTRACE2(eLevelInfoNormal, "CVideoBridgeCPContent::OnConfSetPrivateVideoLayoutOnOffCONNECTED - ConfName:", m_pConfName);
	char targetPartyName[H243_NAME_LEN];
	WORD isPrivate = 0;

	*pParam >> isPrivate;
	*pParam >> targetPartyName;
	targetPartyName[H243_NAME_LEN-1] = '\0';

	CVideoBridgePartyCntl* pCurrParty = (CVideoBridgePartyCntl*)GetPartyCntl(targetPartyName);
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pCurrParty));

	if (!isPrivate && pCurrParty->IsLegacyParty() && IsContentImageNeedToBeAdded())
	{
		TRACEINTO << "Can't set to private layout off for legacy party when there is content sharing.";
		return;
	}

	pCurrParty->ChangeLayoutPrivatePartyButtonOnly(isPrivate);
}


// ------------------------------------------------------------------------------------------
CRoomInfo::CRoomInfo() :
		m_roomId(-1), m_roomEPType(eTelePresencePartyNone), m_maxLinks(MAX_CASCADED_LINKS_NUMBER), m_activeLinks(0), m_isCascadeLink(FALSE), m_isWaitingForChangeITPTypeAck(FALSE), m_isWaitingForRealTPConnections(FALSE)
{
	m_mainLinkName[0] = '\0';
	for (BYTE linkNum = 0; linkNum < MAX_CASCADED_LINKS_NUMBER; linkNum++)
		m_roomLinkPartyIDs[linkNum] = (PartyRsrcID)-1;
}

CRoomInfo::~CRoomInfo()
{}

CRoomInfo::CRoomInfo(RoomID roomId, BOOL isCascadeLink, BYTE numLinks, eTelePresencePartyType roomEPType, const char *mainLinkName)
: m_roomId(roomId), m_roomEPType(roomEPType), m_activeLinks(numLinks),
  m_isCascadeLink(isCascadeLink), m_isWaitingForChangeITPTypeAck(FALSE), m_isWaitingForRealTPConnections(FALSE)
{
	m_mainLinkName[0] = '\0';
	if (mainLinkName != NULL)
		strcpy_safe(m_mainLinkName, mainLinkName);

	// for real telepresence rooms, we don't know how many links will be connected
	m_maxLinks = (m_roomEPType != eTelePresencePartyNone) ? MAX_CASCADED_LINKS_NUMBER : numLinks;
	for (BYTE linkNum = 0; linkNum < MAX_CASCADED_LINKS_NUMBER; linkNum++)
		m_roomLinkPartyIDs[linkNum] = (PartyRsrcID)-1;
}

// ------------------------------------------------------------------------------------------
CRoomInfo &CRoomInfo::operator=(const CRoomInfo& other)
{
	m_roomId = other.m_roomId;
	m_isCascadeLink = other.m_isCascadeLink;
	m_isWaitingForChangeITPTypeAck = other.m_isWaitingForChangeITPTypeAck;
	m_isWaitingForRealTPConnections = other.m_isWaitingForRealTPConnections;
	m_roomEPType = other.m_roomEPType;
	m_maxLinks = other.m_maxLinks;
	m_activeLinks = other.m_activeLinks;
	for (BYTE linkNum = 0; linkNum < MAX_CASCADED_LINKS_NUMBER; linkNum++)
		m_roomLinkPartyIDs[linkNum] = other.m_roomLinkPartyIDs[linkNum];

	strcpy_safe(m_mainLinkName, other.GetMainLinkPartyName());
	return *this;
}

// ------------------------------------------------------------------------------------------
void CRoomInfo::SetLinkPartyId(BYTE linkNum, PartyRsrcID partyId)
{
	if (linkNum >= m_maxLinks)
	{
		TRACESTR(eLevelError) << "new link " << (int)linkNum << " cannot be set into room " << m_roomId << " which has " << (int)m_maxLinks << " links maximum";
	}
	if (linkNum < m_maxLinks)
	{
		PASSERT_AND_RETURN(linkNum >= MAX_CASCADED_LINKS_NUMBER);
		m_roomLinkPartyIDs[linkNum] = partyId;
	}
}

// ------------------------------------------------------------------------------------------
void CRoomInfo::UpdateRoomType(BYTE newNumLinks, eTelePresencePartyType newRoomEPType)
{
	PASSERTMSG_AND_RETURN(newNumLinks > m_maxLinks, "new TP type requires more links than currently connected!");
	m_activeLinks = newNumLinks;
	m_roomEPType = newRoomEPType;
}

// ------------------------------------------------------------------------------------------
void CRoomInfo::UpdateRoomTypeAndMaxScreens(eTelePresencePartyType roomEPType, BYTE maxScreens)
{
	if (maxScreens > MAX_CASCADED_LINKS_NUMBER)
	{
		DBGPASSERT(maxScreens);
		maxScreens = MAX_CASCADED_LINKS_NUMBER;
	}
	m_maxLinks = maxScreens;
	m_roomEPType = roomEPType;
}

// ------------------------------------------------------------------------------------------
PartyRsrcID CRoomInfo::GetLinkPartyId(BYTE linkNum) const
{
	PASSERT_AND_RETURN_VALUE(linkNum >= MAX_CASCADED_LINKS_NUMBER, (PartyRsrcID)-1);

	if (linkNum < m_maxLinks)
		return m_roomLinkPartyIDs[linkNum];

	return (PartyRsrcID)-1;
}

// ------------------------------------------------------------------------------------------
BOOL CRoomInfo::IsRoomAllLinksConnected()
{
	for (BYTE linkNum = 0; linkNum < m_activeLinks; linkNum++)
		if (m_roomLinkPartyIDs[linkNum] == (PartyRsrcID)-1)
			return FALSE;

	return TRUE;
}

// ------------------------------------------------------------------------------------------
BOOL CRoomInfo::IsPartyInRoom(PartyRsrcID partyId)
{
	if (partyId == (PartyRsrcID)-1)
		return FALSE;

	for (BYTE linkNum = 0; linkNum < MAX_CASCADED_LINKS_NUMBER; linkNum++)
		if (m_roomLinkPartyIDs[linkNum] == partyId)
			return TRUE;

	return FALSE;
}
// TELEPRESENCE_LAYOUT
WORD CRoomInfo::GetRoomPartyIds(std::set<DWORD>& roomPartiesIds)
{
	WORD num_if_ids = 0;
	for (WORD linkNum=0 ; linkNum<MAX_CASCADED_LINKS_NUMBER; linkNum++)
	{
		if (m_roomLinkPartyIDs[linkNum] != (PartyRsrcID)-1)
		{
			roomPartiesIds.insert(m_roomLinkPartyIDs[linkNum]);
			num_if_ids++;
		}
	}
	return num_if_ids;
}
const char *CRoomInfo::GetMainLinkPartyName() const
{
	return m_mainLinkName;
}

// ------------------------------------------------------------------------------------------
BYTE CRoomInfo::UpdateHighestActiveLinkNumber()
{
	BYTE highestlinkNum = 0;
	BOOL haslinks = FALSE;
	for (BYTE linkNum = 0; linkNum < MAX_CASCADED_LINKS_NUMBER; linkNum++)
	{
		if (m_roomLinkPartyIDs[linkNum] != (PartyRsrcID)-1)
		{
			highestlinkNum = linkNum;
			haslinks = TRUE;
		}
	}
	m_activeLinks = (haslinks) ? ++highestlinkNum : 0;
	TRACEINTO << "Highest active link in room id " << m_roomId << " is " << (WORD)highestlinkNum;
	return highestlinkNum;
}

// ------------------------------------------------------------------------------------------
void CRoomInfo::Dump(std::ostringstream& msg, bool fullInfo, bool print_to_log) const
{
	msg << " RoomId:" << m_roomId << ", RoomLinkPartyIDs:[";
	for (int screens_index = 0; screens_index < MAX_CASCADED_LINKS_NUMBER; screens_index++)
	{
		if (m_roomLinkPartyIDs[screens_index] == (DWORD)(-1))
			msg << "-1";
		else
			msg << m_roomLinkPartyIDs[screens_index];

		if (screens_index == MAX_CASCADED_LINKS_NUMBER - 1)
			msg << "]";
		else
			msg << ",";
	}
	msg << ", MaxLinks:" << (WORD)m_maxLinks << ", ActiveLinks:" << (WORD)m_activeLinks << ", MainLinkName:" << m_mainLinkName << ", RoomEPType:" << TelePresencePartyTypeToString(m_roomEPType);

	if (fullInfo)
		msg << ", IsCascadeLink:" << (WORD)m_isCascadeLink << ", IsWaitingForChangeITPTypeAck:" << (WORD)m_isWaitingForChangeITPTypeAck << ", IsWaitingForRealTPConnections:" << (WORD)m_isWaitingForRealTPConnections;

	if (print_to_log)
		TRACEINTO << msg.str().c_str();
}


// ------------------------------------------------------------------------------------------
CTelepresenceLayoutMngr::CTelepresenceLayoutMngr(CConfApi* pConfApi, CVideoBridgeCP *pBridge)
: m_pConfApi(pConfApi), m_pBridge(pBridge)
{
	PASSERT_AND_RETURN(!m_pConfApi);
	PASSERT_AND_RETURN(!m_pBridge);

	m_isDisplayContentToLegacyEP = GetSystemCfgFlagInt<BOOL>(FORCE_LEGACY_EP_CONTENT_LAYOUT_ON_TELEPRESENCE);

	m_RoomList.clear();
	Init1x1LayoutMatrix();

	m_pTelepresenceLayoutLogic = new CTelepresenceRoomSwitchLayoutLogic();
	m_pSpeakerModeLayoutLogic  = new CTelepresenceSpeakerModeLayoutLogic();
	m_pPartyModeLayoutLogic    = new CTelepresencePartyModeLayoutLogic();

	m_pTelepresenceLayoutLogic->Create();
	m_pSpeakerModeLayoutLogic->Create();
	m_pPartyModeLayoutLogic->Create();
}

CTelepresenceLayoutMngr::~CTelepresenceLayoutMngr()
{
	m_RoomList.clear();
	POBJDELETE(m_pTelepresenceLayoutLogic);
	POBJDELETE(m_pSpeakerModeLayoutLogic);
	POBJDELETE(m_pPartyModeLayoutLogic);
}

void CTelepresenceLayoutMngr::AddNewRoom(RoomID roomId, BOOL isCascade, BYTE numLinks, eTelePresencePartyType roomEPType, const char *mainLinkName, BYTE linkNum)
{
	TRACEINTO << "Adding room id: " << roomId << " is cascade room " << (WORD)isCascade << " room type " << (WORD)roomEPType;
	BOOL is_found = m_RoomList.find(roomId) != m_RoomList.end();
	BOOL isSingleEP = !isCascade && (roomEPType == eTelePresencePartyNone) && (numLinks == (BYTE)-1 || numLinks == (BYTE)1);
	if (!is_found)	// adding new room of real telepresence EP- may not start from main link
	{
		if(linkNum == (BYTE)-1 && !isSingleEP)
			linkNum = GetLinkPartyIndex(mainLinkName);
			
		if ((numLinks == (BYTE)-1) && !isSingleEP && linkNum != 0)
		{
			char main_name[H243_NAME_LEN];
			main_name[H243_NAME_LEN-1] = '\0';
			SetLinkPartyName(main_name,mainLinkName,0);
			m_RoomList.insert(std::make_pair(roomId, CRoomInfo(roomId, isCascade, numLinks, roomEPType, main_name)));
		}
		else
			m_RoomList.insert(std::make_pair(roomId, CRoomInfo(roomId, isCascade, numLinks, roomEPType, mainLinkName)));
	}
	else
		PTRACE2INT(eLevelError,"CTelepresencePLayoutMngr::AddNewRoom : attempting to add an existing room id: ",roomId);

}

void CTelepresenceLayoutMngr::RemoveRoom(RoomID roomId)
{
	VBRIDGE_ROOM_INFO_MAP::iterator rit = m_RoomList.find(roomId);
	if (rit != m_RoomList.end())
		m_RoomList.erase(rit);
	else
		PTRACE2INT(eLevelError,"CTelepresencePLayoutMngr::RemoveRoom : attempting to remove unknown room id: ",roomId);
}

CRoomInfo CTelepresenceLayoutMngr::UpdateRoomInfo(RoomID roomId, BYTE numLinks, eTelePresencePartyType newRoomEPType, BOOL isAckPending)
{
	CRoomInfo no_room_found;

	VBRIDGE_ROOM_INFO_MAP::iterator rit = m_RoomList.find(roomId);
	if (rit != m_RoomList.end())
	{
		rit->second.UpdateRoomType(numLinks, newRoomEPType);
		rit->second.SetIsAckPendingForChangeType(isAckPending);
		return rit->second;
	}
	else
	{
		PTRACE2INT(eLevelError,"CTelepresencePLayoutMngr::UpdateRoomInfo : attempting to update unknown room id: ",roomId);
		return no_room_found;
	}
}
void CTelepresenceLayoutMngr::UpdateRoomTypeAndMaxScreens(RoomID roomId,eTelePresencePartyType roomEPType,BYTE maxScreens)
{
	CRoomInfo no_room_found;

	TRACEINTO << "RoomId:" << roomId << ", MaxScreens:" << (WORD)maxScreens << ", RoomEPType:" << roomEPType;

	VBRIDGE_ROOM_INFO_MAP::iterator rit = m_RoomList.find(roomId);
	if (rit != m_RoomList.end())
	{
		rit->second.UpdateRoomTypeAndMaxScreens(roomEPType,maxScreens);

		CRoomInfo roomInfo = GetRoomInfo(roomId);
		PartyRsrcID  partyRsrcID = (DWORD)(-1);
		CVideoBridgePartyCntl* pPartyCntl = NULL;
		for (BYTE slaveIndex=0; slaveIndex<MAX_CASCADED_LINKS_NUMBER; slaveIndex++){
		  partyRsrcID = roomInfo.GetLinkPartyId(slaveIndex);
		  if((DWORD)(-1) != partyRsrcID){
		    pPartyCntl = (CVideoBridgePartyCntl*)(m_pBridge->GetPartyCntl(partyRsrcID));
		    if(NULL != pPartyCntl){
		      TRACEINTO << " updating partyRsrcId " << (DWORD)partyRsrcID;
		      pPartyCntl->SetTelepresenceInfoNumOfLinks(maxScreens);
		      pPartyCntl->SetTelepresenceInfoEPtype(roomEPType);
		    }
		  }
		}
	}
	else
	{
	  TRACEINTO << " attempting to update unknown room id: " << roomId;
	}
}

CRoomInfo CTelepresenceLayoutMngr::GetRoomInfo(RoomID roomId)
{
	CRoomInfo no_room_found;
	VBRIDGE_ROOM_INFO_MAP::iterator rit = m_RoomList.begin();
	// search by room id
	if (roomId != (RoomID)-1)
	{
		rit = m_RoomList.find(roomId);
		if (rit != m_RoomList.end())
			return rit->second;
	}
	else
	{
		PTRACE2INT(eLevelError,"CTelepresencePLayoutMngr::GetRoomInfo : unknown room id: ",roomId);
		return no_room_found;
	}
	return no_room_found;
}

CRoomInfo CTelepresenceLayoutMngr::GetRoomInfo(const char* linkPartyName, eTelePresencePartyType epType)
{
	CRoomInfo no_room_found;
	BYTE name_len = 0;
	VBRIDGE_ROOM_INFO_MAP::iterator rit = m_RoomList.begin();
	// search by party name prefix
	if (linkPartyName != NULL && ((name_len = strlen(linkPartyName)) > 0))
	{
		char name_prefix[H243_NAME_LEN];
		strncpy(name_prefix, linkPartyName, name_len - 1);
		name_prefix[name_len - 1] = '\0';

		for (; rit != m_RoomList.end(); rit++)
		{
			const char *room_name = rit->second.GetMainLinkPartyName();
			if ((strstr(room_name, name_prefix) != NULL) && (epType == rit->second.GetRoomEPType()))
			{
				// for single screen EPs - check full name
				if (!rit->second.GetIsCascade() && (epType == eTelePresencePartyNone) && (name_len == strlen(room_name)) && (linkPartyName[name_len - 1] != room_name[name_len - 1]))
					continue;
				return rit->second;
			}
		}
	}
	else
	{
		PTRACE2(eLevelError, "CTelepresencePLayoutMngr::GetRoomInfo : unknown room id for party: ", linkPartyName);
		return no_room_found;
	}
	return no_room_found;
}

BOOL CTelepresenceLayoutMngr::UpdateRoomSubLink(RoomID roomId, BYTE linkNum, PartyRsrcID partyId)
{
	BOOL all_connected = FALSE;
	VBRIDGE_ROOM_INFO_MAP::iterator rit = m_RoomList.find(roomId);
	if (rit != m_RoomList.end())
	{
		rit->second.SetLinkPartyId(linkNum, partyId);
		if (rit->second.GetIsCascade() == eRegularParty)
			rit->second.UpdateHighestActiveLinkNumber();
		all_connected = rit->second.IsRoomAllLinksConnected();
	}
	else
		PTRACE2INT(eLevelError,"CTelepresencePLayoutMngr::UpdateRoomSubLink : attempting to update unknown room id: ",roomId);

	return all_connected;
}

BOOL CTelepresenceLayoutMngr::UpdateRoomSubLinkTIPEP(RoomID partyInfoRoomId, eTelePresencePartyType epType, const char *partyName, PartyRsrcID partyId, BYTE link_num)
{
	BOOL all_connected = FALSE;
	PASSERTMSG_AND_RETURN_VALUE(partyName == NULL, "CTelepresenceLayoutMngr::UpdateRoomSubLinkTIPEP party name is null",FALSE);
	CRoomInfo matchingRoom = GetRoomInfo(partyInfoRoomId);
	if(matchingRoom.GetRoomId() == (RoomID)-1){
	   matchingRoom = GetRoomInfo(partyName, epType);
	}
	PASSERTMSG_AND_RETURN_VALUE(matchingRoom.GetRoomId() == (RoomID)-1, "CTelepresenceLayoutMngr::UpdateRoomSubLinkTIPEP link name does not match any known rooms", FALSE);
	if (matchingRoom.GetRoomEPType() != epType) {
		PASSERTMSG_AND_RETURN_VALUE(TRUE, "CTelepresenceLayoutMngr::UpdateRoomSubLinkTIPEP party epType does not match room EP type",FALSE);
	}
	
	if (link_num == (BYTE)-1)
		link_num = GetLinkPartyIndex(partyName);
	
	if (matchingRoom.GetRoomId() != partyInfoRoomId)	// room was started by a sublink
	{
		TRACEINTO << "Party's room id " << partyInfoRoomId << " doesn't match the room id according to name: "<< matchingRoom.GetRoomId();
		// if mainlink (link_num == 0) has a different id, that means that the room was created when a TP sublink connected first.
		// in this case we make a copy of the room including the link info (for the links that already connected), and reinsert the into the map with the new id.
		if (link_num == 0)
		{
			TRACEINTO << "Room id " << matchingRoom.GetRoomId() << " was started by a sublink";
			m_RoomList.erase(matchingRoom.GetRoomId());
			matchingRoom.SetLinkPartyId(link_num, partyId);
			WORD numLinksToCopy = matchingRoom.GetNumberOfActiveLinks();
			CRoomInfo new_room(partyInfoRoomId, FALSE,numLinksToCopy, epType, partyName);//matchingRoom);
			for (BYTE linkNum=0; linkNum<numLinksToCopy; linkNum++)
			{
				new_room.SetLinkPartyId(linkNum, matchingRoom.GetLinkPartyId(linkNum));
			}
			m_RoomList.insert(std::make_pair(partyInfoRoomId,new_room));
		}
		else
			UpdateRoomSubLink(matchingRoom.GetRoomId(), link_num, partyId);
	}
	else
		UpdateRoomSubLink(matchingRoom.GetRoomId(), link_num, partyId);
	return FALSE;
}

void CTelepresenceLayoutMngr::OnSpeakerChanged(RoomID newSpeakerRoomId)
{
	TRACEINTO << "New speaker room id " << newSpeakerRoomId;
	VBRIDGE_ROOM_INFO_MAP::iterator rit = m_RoomList.find(newSpeakerRoomId);
	if (rit == m_RoomList.end())
	{
		PTRACE2INT(eLevelError,"CTelepresencePLayoutMngr::OnSpeakerChanged : new speaker room id not found: ",newSpeakerRoomId);
		PASSERTMSG_AND_RETURN(TRUE, "new speaker room id not found");
	}
	if (rit->second.GetIsAckPendingForRealTPConnections())
	{
		TRACEINTO << "Speaker room id " << rit->second.GetRoomId() << " is pending new sublink connections. speaker change will execute after the timer";
		return;
	}
	CRoomInfo speakerRoom = rit->second;

	for (rit = m_RoomList.begin(); rit!=m_RoomList.end();rit++)
	{
		CRoomInfo& viewerRoom = rit->second;
		if (!viewerRoom.GetIsCascade() || (viewerRoom.GetRoomId() == newSpeakerRoomId))
			continue;
		else
			SetViewerRoomLinksLayout(viewerRoom, speakerRoom);
	}
}

void CTelepresenceLayoutMngr::OnSpeakerChanged(RoomID viewerRoomId, RoomID newSpeakerRoomId)
{
	TRACEINTO << "Viewer room id " << viewerRoomId << " speaker room id " << newSpeakerRoomId;
	if (viewerRoomId == newSpeakerRoomId)
	{
		TRACEINTO << "Invalid room change - viewer room IS the speaker room";
		return;
	}
	VBRIDGE_ROOM_INFO_MAP::iterator sit = m_RoomList.find(newSpeakerRoomId);
	if (sit == m_RoomList.end())
	{
		PTRACE2INT(eLevelError,"CTelepresencePLayoutMngr::OnSpeakerChanged : new speaker room id not found: ",newSpeakerRoomId);
		PASSERTMSG_AND_RETURN(TRUE, "new speaker room id not found");
	}
	if (sit->second.GetIsAckPendingForRealTPConnections())
	{
		TRACEINTO << "Speaker room id " << sit->second.GetRoomId() << " is pending new sublink connections. speaker change will execute after the timer";
		return;
	}
	VBRIDGE_ROOM_INFO_MAP::iterator vit = m_RoomList.find(viewerRoomId);
	if (vit == m_RoomList.end())
	{
		PTRACE2INT(eLevelError,"CTelepresencePLayoutMngr::OnSpeakerChanged viewer room id not found: ",viewerRoomId);
		PASSERTMSG_AND_RETURN(TRUE, "new speaker room id not found");
	}
	vit->second.SetIsAckPendingForChangeType(FALSE);
	CRoomInfo speakerRoom = sit->second;
	CRoomInfo viewerRoom = vit->second;
	if (viewerRoom.GetIsCascade() == eRegularParty)
	{
		TRACEINTO << "Invalid room change - viewer not cascade ";
		return;
	}
	else
		SetViewerRoomLinksLayout(viewerRoom, speakerRoom);
}

void CTelepresenceLayoutMngr::SetViewerRoomLinksLayout(CRoomInfo& viewer, CRoomInfo& speaker)
{
	CVideoLayout layout(ONE_ONE,YES);
	layout.m_pCellLayout[0] = new CVideoCellLayout;
	layout.m_pCellLayout[0]->m_cellId = 1;
	BYTE old_numActiveLinks = viewer.GetNumberOfActiveLinks();
	BYTE maxlinks = viewer.GetMaxLinks();
	BYTE new_numActiveLinks = speaker.GetNumberOfActiveLinks();
	PASSERTMSG_AND_RETURN((old_numActiveLinks == 0 || new_numActiveLinks == 0), "No links");
	if (maxlinks < speaker.GetNumberOfActiveLinks())
	{
		TRACEINTO << "Telepresence link number mismatch between viewer (" <<
				old_numActiveLinks << " links) and speaker (" << speaker.GetNumberOfActiveLinks() << " links)!";
		PASSERTMSG(TRUE, "Telepresence link number mismatch");
	}
    if(viewer.GetNumberOfActiveLinks() >= MAX_CASCADED_LINKS_NUMBER || speaker.GetNumberOfActiveLinks() >= MAX_CASCADED_LINKS_NUMBER)
    {
    	PASSERTMSG_AND_RETURN(viewer.GetNumberOfActiveLinks() >= MAX_CASCADED_LINKS_NUMBER , "No. of viewer links exceeded MAX_CASCADED_LINKS_NUMBER");
    	PASSERTMSG_AND_RETURN(speaker.GetNumberOfActiveLinks() >= MAX_CASCADED_LINKS_NUMBER , "No. of speaker links exceeded MAX_CASCADED_LINKS_NUMBER");
    }
	LAYOUT_DEST_SCREEN_CONFIG* viewer_screens_layout = &(m_layoutConfFor1x1[viewer.GetNumberOfActiveLinks()-1][speaker.GetNumberOfActiveLinks()-1]);
	for (BYTE viewer_link=0; viewer_link< maxlinks; viewer_link++)
	{
		PartyRsrcID viewing_party_id = viewer.GetLinkPartyId(viewer_link);
		CBridgePartyCntl* pViewerPartyCntl = NULL;
		BYTE matching_speaker_link = viewer_screens_layout->screens[viewer_link];
		PartyRsrcID speaker_screen_party_id = (PartyRsrcID)-1;
		if (matching_speaker_link == (BYTE)-1)
		{
			TRACEINTO << "Speaker link is inactive for for link " << (WORD)(viewer_link) << " ! #Telepresence";
		}
		else
			speaker_screen_party_id = speaker.GetLinkPartyId(matching_speaker_link);

		if (viewing_party_id == (PartyRsrcID)-1)
		{
			TRACEINTO << "Telepresence missing viewer party id for link " << (WORD)(viewer_link+1) << " ! #Telepresence";
			continue;
		}
		else
		{
			pViewerPartyCntl = m_pBridge->GetPartyCntl(viewing_party_id);
			BOOL isValidParty = IsValidPObjectPtr(pViewerPartyCntl);
			PASSERTSTREAM(!isValidParty, "The viewer room's party for link " << (WORD)viewer_link << " party rsrc id " << viewing_party_id << " is invalid!!");
			if (!isValidParty)
				continue;
		}
		TRACEINTO << "Telepresence setting viewer link "
								 << (WORD)viewer_link << " which is party id " << (int)viewing_party_id
								 << " to speaker link " << (WORD)(matching_speaker_link)  << " which is party id " << (int)speaker_screen_party_id;
		BOOL isBlank = (speaker_screen_party_id == (PartyRsrcID)-1) || (viewer_link >= new_numActiveLinks);
		CBridgePartyCntl* pSpeakerPartyCntl = (isBlank)? NULL : m_pBridge->GetPartyCntl(speaker_screen_party_id);
		BOOL isValidParty = IsValidPObjectPtr(pSpeakerPartyCntl);
		PASSERTSTREAM(!isValidParty, "The speaker room's party for screen " << (WORD)viewer_link << " party rsrc id " << speaker_screen_party_id << " is invalid!!");

		// set to blank the cells that are not in use
		if (isBlank || !isValidParty)
		{
			layout.m_pCellLayout[0]->SetBlank(EMPTY_BY_OPERATOR_THIS_PARTY);
			// remove forced id
			layout.m_pCellLayout[0]->SetForcedPartyId((DWORD)-1, EMPTY_BY_OPERATOR_THIS_PARTY);
		}
		else
		{
			layout.m_pCellLayout[0]->SetForcedPartyId(speaker_screen_party_id, BY_ITP_LAYOUT_MGMT);
			layout.m_pCellLayout[0]->SetCurrentPartyId(speaker_screen_party_id, BY_ITP_LAYOUT_MGMT);
			layout.m_pCellLayout[0]->SetName(pSpeakerPartyCntl->GetName());
		}
		SendLayoutToVideoBridge(pViewerPartyCntl->GetName(), layout);
	}
}

void CTelepresenceLayoutMngr::SendBlankPrivateLayoutToVideoBridge(const char *pViewerName)
{
	CVideoLayout layout(ONE_ONE,YES);
	layout.m_pCellLayout[0] = new CVideoCellLayout;
	layout.m_pCellLayout[0]->m_cellId = 1;
	layout.m_pCellLayout[0]->SetBlank(EMPTY_BY_OPERATOR_THIS_PARTY);
	SendLayoutToVideoBridge(pViewerName, layout);
}

void CTelepresenceLayoutMngr::SendLayoutToVideoBridge(const char *pViewerName, CVideoLayout &resolved1x1Layout)
{
	//TODO_ITP
	//PASSERTMSG_AND_RETURN(m_pConfApi == NULL, "CTelepresenceLayoutMngr::sendLayoutToVideoBridge - Conf API not initialized!");
	m_pConfApi->SetVideoPrivateLayout(pViewerName, resolved1x1Layout, TRUE);

}

void CTelepresenceLayoutMngr::SetRoomPendingForRealTPConnections(RoomID roomId, BOOL isPending)
{
	BOOL all_connected = FALSE;
	VBRIDGE_ROOM_INFO_MAP::iterator rit = m_RoomList.find(roomId);
	if (rit != m_RoomList.end())
		rit->second.SetIsAckPendingForRealTPConnections(isPending);
	else
		PTRACE2INT(eLevelError,"CTelepresencePLayoutMngr::SetRoomPendingForRealTPConnections : attempting to update unknown room id: ",roomId);
}

BOOL CTelepresenceLayoutMngr::GetRoomPendingForRealTPConnections(RoomID roomId)
{
	BOOL isPending = FALSE;
	VBRIDGE_ROOM_INFO_MAP::iterator rit = m_RoomList.find(roomId);	// ****TBD - add search for room by name
	if (rit != m_RoomList.end())
		isPending = rit->second.GetIsAckPendingForRealTPConnections();
	else
		PTRACE2INT(eLevelError,"CTelepresencePLayoutMngr::GetRoomPendingForRealTPConnections : unknown room id: ",roomId);
	return isPending;
}

void CTelepresenceLayoutMngr::SetLinkPartyName(char *h243Dest, const char *mainLinkName, BYTE LinkNum)
{
	strncpy(h243Dest, mainLinkName, H243_NAME_LEN);
	h243Dest[H243_NAME_LEN - 1] = '\0';
	// main link name is enough
	if (LinkNum == 0)
		return;
	WORD ch = strlen(h243Dest)-1;
	if (h243Dest[ch]>='0' && h243Dest[ch]<='9')
		sprintf(&h243Dest[ch], "%u", LinkNum+1);
	else
	{
		CSmallString str;
		str << "CTelepresenceLayoutMngr::SetLinkPartyName: Invalid format of party name " << mainLinkName;
		PASSERTMSG(1,str.GetString());
	}
}
//--------------------------------------------------------------------------
void CTelepresenceLayoutMngr::SetConfLayoutToLegacyLayout(bool isSet)
{
	// FEATURE: "Display Content to Legacy EP in Telepresence Conference"

	if (!IsDisplayContentToLegacyEP())
		return;

	LayoutType legacyContentDefaultLayoutType = m_pBridge->GetLegacyContentDefaultLayout();

	std::ostringstream msg;
	msg << "CTelepresenceLayoutMngr::SetConfLayoutToLegacyLayout - ";
	msg << "LayoutType:" << LayoutTypeAsString[m_pBridge->GetCurrentLayoutType()] << ", SetToLegacyLayoutType:" << (int)isSet;

	if (isSet)
	{
		if (legacyContentDefaultLayoutType != m_pBridge->GetCurrentLayoutType())
		{
			TRACESTR(eLevelError) << msg.str().c_str() << " - Set conference layout to " << LayoutTypeAsString[legacyContentDefaultLayoutType];
			m_pBridge->SetAutoLayout(FALSE); // Turn OFF “auto-layout“. We will restore “auto-layout“ when MLA stops managing the layout
			m_pBridge->ChangeConfLayoutType(legacyContentDefaultLayoutType); // Set conference layout to "legacy content default layout"
			return;
		}
	}
	else
	{
		// When MLA stops managing the layout (no telepresence systems in the conference and telepresence mode is "Auto")
		// the conference layout will be set to auto (administrator cannot control the conference layout).
		if (legacyContentDefaultLayoutType == m_pBridge->GetCurrentLayoutType())
		{
			TRACESTR(eLevelError) << msg.str().c_str() << " - Set conference layout to 'Auto-Layout'";
			m_pBridge->SetAutoLayout(TRUE);
			return;
		}
	}
	TRACESTR(eLevelError) << msg.str().c_str();
}

BYTE CTelepresenceLayoutMngr::GetLinkPartyIndex( const char *mainLinkName)
{
	PASSERTMSG_AND_RETURN_VALUE(mainLinkName == NULL, "CTelepresenceLayoutMngr::GetLinkPartyIndex: null party name", (BYTE)-1);
	WORD ch = strlen(mainLinkName);
	PASSERTMSG_AND_RETURN_VALUE(ch==0, "CTelepresenceLayoutMngr::GetLinkPartyIndex: Empty party name", (BYTE)-1);
	TRACEINTO << "CTelepresenceLayoutMngr::GetLinkPartyIndex : received name " << mainLinkName << "which has length " << ch;	// GuyS REMOVE
	WORD index = 0;
	ch--;
	if (mainLinkName[ch]>='0' && mainLinkName[ch]<='9')
	{
		index = (WORD)atoi(&mainLinkName[ch]);
		if (index == 0 || index > MAX_CASCADED_LINKS_NUMBER)
		{
			CSmallString str;
			str << "CTelepresenceLayoutMngr::GetLinkPartyIndex: Invalid party name index " << mainLinkName;
			PASSERTMSG_AND_RETURN_VALUE(1,str.GetString(),(BYTE)-1);
		}
		else
			return --index;
	}
	else
	{
		CSmallString str;
		str << "CTelepresenceLayoutMngr::GetLinkPartyIndex: Invalid format of party name " << mainLinkName;
		PASSERTMSG_AND_RETURN_VALUE(1,str.GetString(),(BYTE)-1);
	}
}

void CTelepresenceLayoutMngr::Set1X1MatrixCell(BYTE viewerLinkNum, BYTE speakerLinkNum, BYTE link1, BYTE link2, BYTE link3, BYTE link4)
{
	LAYOUT_DEST_SCREEN_CONFIG* screen_cfg = &m_layoutConfFor1x1[viewerLinkNum][speakerLinkNum];
	screen_cfg->screens[0]= link1;
	screen_cfg->screens[1]= link2;
	screen_cfg->screens[2]= link3;
	screen_cfg->screens[3]= link4;
}

void CTelepresenceLayoutMngr::Init1x1LayoutMatrix()
{
	BYTE LINK_1, LINK_2, LINK_3, LINK_4;
	BYTE _1_LINK,_2_LINKS,_3_LINKS,_4_LINKS;
	BYTE BLANK = (BYTE)-1;
	_1_LINK = LINK_1 = 0;
	_2_LINKS = LINK_2 = 1;
	_3_LINKS = LINK_3 = 2;
	_4_LINKS = LINK_4 = 3;

	Set1X1MatrixCell(_1_LINK,	_1_LINK,	LINK_1,BLANK,BLANK,BLANK);
	Set1X1MatrixCell(_1_LINK,	_2_LINKS,	LINK_1,BLANK,BLANK,BLANK);
	Set1X1MatrixCell(_1_LINK,	_3_LINKS,	LINK_1,BLANK,BLANK,BLANK);
	Set1X1MatrixCell(_1_LINK,	_4_LINKS,	LINK_1,BLANK,BLANK,BLANK);
	Set1X1MatrixCell(_2_LINKS,	_1_LINK,	LINK_1,BLANK,BLANK,BLANK);
	Set1X1MatrixCell(_2_LINKS,	_2_LINKS,	LINK_1,LINK_2,BLANK,BLANK);
	Set1X1MatrixCell(_2_LINKS,	_3_LINKS,	LINK_1,LINK_2,BLANK,BLANK);
	Set1X1MatrixCell(_2_LINKS,	_4_LINKS,	LINK_1,LINK_2,BLANK,BLANK);
	Set1X1MatrixCell(_3_LINKS,	_1_LINK,	LINK_1,BLANK,BLANK,BLANK);
	Set1X1MatrixCell(_3_LINKS,	_2_LINKS,	LINK_1,LINK_2,BLANK,BLANK);
	Set1X1MatrixCell(_3_LINKS,	_3_LINKS,	LINK_1,LINK_2,LINK_3,BLANK);
	Set1X1MatrixCell(_3_LINKS,	_4_LINKS,	LINK_1,LINK_2,LINK_3,BLANK);
	Set1X1MatrixCell(_4_LINKS,	_1_LINK,	LINK_1,BLANK,BLANK,BLANK);
	Set1X1MatrixCell(_4_LINKS,	_2_LINKS,	LINK_1,LINK_2,BLANK,BLANK);
	Set1X1MatrixCell(_4_LINKS,	_3_LINKS,	LINK_1,LINK_2,LINK_3,BLANK);
	Set1X1MatrixCell(_4_LINKS,	_4_LINKS,	LINK_1,LINK_2,LINK_3,LINK_4);
}

/*
	CVideoBridge *m_pVidBridge;		//for setforce calls for the cascade links
	VBRIDGE_ROOM_INFO_LIST m_RoomList;
	LAYOUT_DEST_SCREEN_CONFIG m_layoutConfFor1x1[MAX_CASCADED_LINKS_NUMBER][MAX_CASCADED_LINKS_NUMBER];*/


/////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ class CVideoContentXcodeBridge ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/////////////////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CVideoContentXcodeBridge)
ONEVENT(END_CONNECT_COP_ENCODER,           IDLE_VB,        CVideoContentXcodeBridge::OnXCodeEncoderEndConnectIDLE)
ONEVENT(END_CONNECT_COP_ENCODER,           CONNECTED,      CVideoContentXcodeBridge::NullActionFunction)
ONEVENT(END_CONNECT_COP_ENCODER,           DISCONNECTING,  CVideoContentXcodeBridge::NullActionFunction)

ONEVENT(END_DISCONNECT_COP_ENCODER,        IDLE_VB,        CVideoContentXcodeBridge::NullActionFunction)
ONEVENT(END_DISCONNECT_COP_ENCODER,        CONNECTED,      CVideoContentXcodeBridge::NullActionFunction)
ONEVENT(END_DISCONNECT_COP_ENCODER,        DISCONNECTING,  CVideoContentXcodeBridge::OnXCodeEncoderEndDisconnectDISCONNECTING)

ONEVENT(CONTENT_DECODER_VIDEO_IN_SYNCED,    IDLE_VB,        CVideoContentXcodeBridge::OnContentDecoderVideoInSyncedCONNECTED)
ONEVENT(CONTENT_DECODER_VIDEO_IN_SYNCED,    CONNECTED,      CVideoContentXcodeBridge::OnContentDecoderVideoInSyncedCONNECTED)
ONEVENT(CONTENT_DECODER_VIDEO_IN_SYNCED,    DISCONNECTING,   CVideoContentXcodeBridge::NullActionFunction)

ONEVENT(CONTENT_DECODER_DISCONNECTED,       IDLE_VB,        CVideoContentXcodeBridge::OnContentDecoderDisconnectedCONNECTED)
ONEVENT(CONTENT_DECODER_DISCONNECTED,       CONNECTED,      CVideoContentXcodeBridge::OnContentDecoderDisconnectedCONNECTED)
ONEVENT(CONTENT_DECODER_DISCONNECTED,       DISCONNECTING,   CVideoContentXcodeBridge::NullActionFunction)

ONEVENT(CONNECT_XCODE_VB_TOUT,              IDLE_VB,        CVideoContentXcodeBridge::OnTimerConnectXCodeBridgeIDLE)
ONEVENT(CONNECT_XCODE_VB_TOUT,              CONNECTED,      CVideoContentXcodeBridge::NullActionFunction)
ONEVENT(CONNECT_XCODE_VB_TOUT,              DISCONNECTING,  CVideoContentXcodeBridge::NullActionFunction)

ONEVENT(CONTENTVIDREFRESH,                  CONNECTED,      CVideoContentXcodeBridge::OnContentBridgeContentVideoRefreshCONNECTED)
ONEVENT(CONTENTVIDREFRESH,                  CONNECTED,      CVideoContentXcodeBridge::NullActionFunction)
ONEVENT(CONTENTVIDREFRESH,                  CONNECTED,      CVideoContentXcodeBridge::NullActionFunction)

ONEVENT(BRIDGE_DISCONNECT_TOUT,             DISCONNECTING,  CVideoContentXcodeBridge::OnTimerDisconnetDISCONNECTING)
ONEVENT(DISCONNECTCONF,                     IDLE_VB,        CVideoContentXcodeBridge::OnConfDisConnectConfIDLE)

ONEVENT(END_DESTROY_CONTENT_DECODER,        CONNECTED,      CVideoContentXcodeBridge::OnEndDestroyContentDecoderCONNECTED)
ONEVENT(END_DESTROY_CONTENT_DECODER,        DISCONNECTING,  CVideoContentXcodeBridge::OnEndDestroyContentDecoderDISCONNECTING)

ONEVENT(COP_ENCODER_VIDEO_OUT_UPDATED,      ANYCASE,      CVideoContentXcodeBridge::OnXCodeEncoderVideoOutUpdated)

PEND_MESSAGE_MAP(CVideoContentXcodeBridge, CVideoBridgeCP);

// ------------------------------------------------------------
CVideoContentXcodeBridge::CVideoContentXcodeBridge()
{

	m_pContentDecoder = NULL;

   m_xcodeEncodesrMap = NULL;
   m_pXCodeRsrcMap = NULL;
   m_ContentRate = 0;

   m_VideoConfType = eVideoConfTypeCP;
   m_isSiteNamesEnabled = NO;
   //m_ConfContentLayout1x1 = new CLayout(CP_LAYOUT_1X1, "");
  /* const LayoutType sysConfigLayoutType = GetLegacyContentDefaultLayout();
   PTRACE2INT(eLevelInfoNormal,"CVideoContentXcodeBridge::CVideoContentXcodeBridge, layout type is: ",sysConfigLayoutType);
   m_ConfContentLayout1x1 = new CLayout(sysConfigLayoutType, "");*/
   m_ConfContentLayout1x1 = new CLayout(CP_LAYOUT_1X1, "");
   m_resourcesStatus = STATUS_OK;
   m_closeAfterOpenEncodesFailed = FALSE;
   OFF(m_isFirstContentSessionStarted);

   VALIDATEMESSAGEMAP;
}
// ------------------------------------------------------------
CVideoContentXcodeBridge::~CVideoContentXcodeBridge()
{
	//POBJDELETE(m_pContentDecoder); // This is a memnber of Video Bridge and it is deleted there.
	for(XCODE_ENCODERS_MAP::iterator _ii = m_xcodeEncodesrMap->begin(); _ii != m_xcodeEncodesrMap->end();)
	{
		CVideoBridgeXCodeEncoder* pXCodeEncoder = NULL;
		pXCodeEncoder = _ii->second;
		POBJDELETE(pXCodeEncoder);
		m_xcodeEncodesrMap->erase(_ii++);
	}
    delete m_xcodeEncodesrMap;
    for(XCODE_RESOURCES_MAP::iterator _ii = m_pXCodeRsrcMap->begin(); _ii != m_pXCodeRsrcMap->end();)
    {
    	ContentXcodeRsrcDesc* pContentXcodeRsrcDesc = NULL;
    	pContentXcodeRsrcDesc = _ii->second;
    	POBJDELETE(pContentXcodeRsrcDesc);
    	m_pXCodeRsrcMap->erase(_ii++);
    }

    delete m_pXCodeRsrcMap;
    POBJDELETE(m_ConfContentLayout1x1);
}


// ------------------------------------------------------------------------------------------
void CVideoContentXcodeBridge::Create(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
	if ( ! CPObject::IsValidPObjectPtr(pVideoBridgeInitParams) )
	{
		PASSERT(1);
		m_pConfApi->EndXCodeBrdgConnect(statInconsistent);  //Yiye 789789
		return;
	}

	CVideoBridgeCP::CreateBase(pVideoBridgeInitParams);
	m_xcodeEncodesrMap = new XCODE_ENCODERS_MAP;
	m_pXCodeRsrcMap = new XCODE_RESOURCES_MAP;

	CreateEncoders(pVideoBridgeInitParams);
	CreateDecoder(pVideoBridgeInitParams);

	ConnectContentEncoders();

	m_state =  IDLE_VB;
   StartTimer(CONNECT_XCODE_VB_TOUT,CONNECT_XCODE_VB_TIME_OUT_VALUE);
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::CreateEncoders(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
	XCODE_INIT_PARAMS_LIST* pEncoderInitParamsList = ((CVideoBridgeInitParams*)pVideoBridgeInitParams)->GetXCodeInitParamsList();
	PASSERT_AND_RETURN( pEncoderInitParamsList == NULL);

	XCODE_RESOURCES_MAP* pXCodeResourcesMap = ((CVideoBridgeInitParams*)pVideoBridgeInitParams)->GetXCodeResourcesMap();
	PASSERT_AND_RETURN(pXCodeResourcesMap == NULL);

	if(pEncoderInitParamsList->size() == 0) {
		PASSERT_AND_RETURN(1);
	}

	for (XCODE_INIT_PARAMS_LIST::iterator _ii  = pEncoderInitParamsList->begin(); _ii != pEncoderInitParamsList->end(); _ii++)
	{
		if(_ii->first >= eXcodeContentDecoder)continue;
		BYTE encoderTypeLen = strlen(eXcodeRsrcTypeNames[_ii->first]);
		char partyName[128+encoderTypeLen];                      // init encoder name
		memset(partyName, '\0', 128+encoderTypeLen);
		sprintf(partyName, "%s%s", "Content_Xcode_Encoder_#", eXcodeRsrcTypeNames[_ii->first]);

		const CTaskApp*                pParty            = NULL;  // no party task for ContentXcode encoder
		const WORD                     wNetworkInterface = AUTO_INTERFACE_TYPE;
		const CBridgePartyMediaParams* pMediaInParams    = NULL;
		const CBridgePartyCntl*        pBridgePartyCntl  = NULL;
		const char*                    pSiteName         = "Content_Xcode_Encoder";
		BYTE                     bCascadeLinkMode  = NO;
		eXcodeRsrcType eEncoderRsrcType = _ii->first;
		if(eEncoderRsrcType >= eXcodeContentDecoder)continue;

		CBridgePartyVideoOutParams* pOutVideoParams = new CBridgePartyVideoOutParams;
		CBridgePartyVideoParams* pListVideoOutparams = _ii->second;
		*(CBridgePartyVideoParams*)pOutVideoParams = *pListVideoOutparams;
		PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::CreateEncoders -  Arrived video out params of Encoder:",partyName);
		pListVideoOutparams->DumpVideoParams();

		PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::CreateEncoders -  Actual video out params of Encoder:",partyName);
		pOutVideoParams->DumpVideoParams();

		PartyRsrcID partyRsrcID = (PartyRsrcID)(-1);

		XCODE_RESOURCES_MAP* pXCodeResourcesMap = ((CVideoBridgeInitParams*)pVideoBridgeInitParams)->GetXCodeResourcesMap();
		PASSERT_AND_RETURN(pXCodeResourcesMap == NULL);
		XCODE_RESOURCES_MAP::iterator resourceIt = pXCodeResourcesMap->find(eEncoderRsrcType);
		if(resourceIt ==  pXCodeResourcesMap->end()) {
			PASSERT_AND_RETURN(int(eEncoderRsrcType) + 1);
		}


		partyRsrcID = resourceIt->second->rsrcEntityId;
		ContentXcodeRsrcDesc* pContentXcodeRsrcDesc = resourceIt->second;
		ContentXcodeRsrcDesc* pNewContentXcodeRsrcDesc  = new ContentXcodeRsrcDesc;
		*pNewContentXcodeRsrcDesc = *pContentXcodeRsrcDesc;
		m_pXCodeRsrcMap->insert(XCODE_RESOURCES_MAP::value_type(eEncoderRsrcType,pNewContentXcodeRsrcDesc));
		partyRsrcID = pContentXcodeRsrcDesc->rsrcEntityId;
		UpdateEncoderResourceParams(*pOutVideoParams, partyRsrcID, _ii->first, m_VideoConfType,*pContentXcodeRsrcDesc);
		pOutVideoParams->SetIsSiteNamesEnabled(m_isSiteNamesEnabled);
		pOutVideoParams->SetLayoutType(CP_LAYOUT_1X1);
		if(eEncoderRsrcType == eXcodeH264LinksEncoder)
			bCascadeLinkMode = YES;


		CVideoBridgePartyInitParams* pVideoBridgePartyInitParams = new CVideoBridgePartyInitParams(partyName, pParty, partyRsrcID, wNetworkInterface, pMediaInParams, pOutVideoParams, pBridgePartyCntl, pSiteName, bCascadeLinkMode);

		// set bridge parameters
		pVideoBridgePartyInitParams->SetBridge(this);
		pVideoBridgePartyInitParams->SetConf(m_pConf);
		pVideoBridgePartyInitParams->SetConfRsrcID(m_confRsrcID);
		CVideoBridgeXCodeEncoder* pVideoBridgeXCodeEncoder = new CVideoBridgeXCodeEncoder();
		pVideoBridgeXCodeEncoder->Create(pVideoBridgePartyInitParams);
		eXcodeRsrcType encoderRsrcType = _ii->first;

		m_xcodeEncodesrMap->insert(XCODE_ENCODERS_MAP::value_type(eEncoderRsrcType,pVideoBridgeXCodeEncoder));

		POBJDELETE(pOutVideoParams);
		POBJDELETE(pVideoBridgePartyInitParams);
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::CreateDecoder(const CVideoBridgeInitParams* pVideoBridgeInitParams)
{
	XCODE_RESOURCES_MAP* pXCodeResourcesMap = ((CVideoBridgeInitParams*)pVideoBridgeInitParams)->GetXCodeResourcesMap();
	PASSERT_AND_RETURN(pXCodeResourcesMap == NULL);

	if(pXCodeResourcesMap->size() == 0) {
		PASSERT_AND_RETURN(1);
	}

	XCODE_RESOURCES_MAP::iterator resourceIt = pXCodeResourcesMap->find(eXcodeContentDecoder);
	if(resourceIt ==  pXCodeResourcesMap->end()) {
		PASSERT_AND_RETURN(1);
	}

	ContentXcodeRsrcDesc* pContentXcodeRsrcDesc = resourceIt->second;
	ContentXcodeRsrcDesc* pNewContentXcodeRsrcDesc  = new ContentXcodeRsrcDesc;
	*pNewContentXcodeRsrcDesc = *pContentXcodeRsrcDesc;
	m_pXCodeRsrcMap->insert(XCODE_RESOURCES_MAP::value_type(eXcodeContentDecoder,pNewContentXcodeRsrcDesc));

	CVideoBridgePartyCntlContent* pContentDecoder = pVideoBridgeInitParams->GetVideoBridgeContentDecoder();
	if(!pContentDecoder)
	{
		PASSERT(1);
		TRACEINTO <<  "Content Decoder PartyCntl from Video Bridge is Null, fail XCode Bridge Connection"
				<< " Conf Name: " << m_pConfName << "\n";
		m_pConfApi->EndXCodeBrdgConnect(STATUS_FAIL);
		return;
	}
	m_pContentDecoder = pContentDecoder;
}


////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::UpdateEncoderResourceParams(CBridgePartyVideoOutParams& rOutVideoParams,PartyRsrcID& partyRsrcID,
														   DWORD encoder_index, eVideoConfType videoConfType,
														   ContentXcodeRsrcDesc xcodeRsrcDesc) const
{

	partyRsrcID = xcodeRsrcDesc.rsrcEntityId;
    rOutVideoParams.SetCopResourceIndex(encoder_index);
    rOutVideoParams.SetVideConfType(videoConfType);
    rOutVideoParams.SetCopLrt(xcodeRsrcDesc.logicalRsrcType);

    TRACEINTO << "CVideoContentXcodeBridge::UpdateEncoderResourceParams "
            << "- PartyRsrcID:" << partyRsrcID
            << ", EncoderIndex:" << encoder_index
            << ", VideoConfType:" <<VideoConfTypeAsString[videoConfType]
            << ", LogicalResourceType:" << LogicalResourceTypeToString(xcodeRsrcDesc.logicalRsrcType);
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::UpdateDecoderResourceParams(CBridgePartyVideoInParams& rInVideoParams,PartyRsrcID& partyRsrcID,
														   eVideoConfType videoConfType,ContentXcodeRsrcDesc xcodeRsrcDesc) const
{

	partyRsrcID = xcodeRsrcDesc.rsrcEntityId;
    rInVideoParams.SetVideConfType(videoConfType);
   // rInVideoParams.SetCopLrt(xcodeRsrcDesc.logicalRsrcType);

    TRACEINTO << "CVideoContentXcodeBridge::UpdateEncoderResourceParams "
            << "- PartyRsrcID:" << partyRsrcID
            << ", VideoConfType:" <<VideoConfTypeAsString[videoConfType]
            << ", LogicalResourceType:" << LogicalResourceTypeToString(xcodeRsrcDesc.logicalRsrcType);
}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::UpdateVideoOutParams(eXcodeRsrcType eXCodeEncoderType, CBridgePartyVideoOutParams* pBridgePartyVideoOutParams)
{
	PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::UpdateVideoOutParams, ConfName - ", m_pConfName);

	// this call is not through state machine, so I print the state for further debugging
	switch (m_state)
	{
	case DISCONNECTING:
	{
		PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::UpdateVideoOutParams received in DISCONNECTING state - ignored , ConfName - ", m_pConfName);
		return; // dont execute the function
	}

	case CONNECTED:
	case IDLE_VB:
		break;
	default:
		return;
	} // switch

	 if (eXCodeEncoderType >= MAX_CONTENT_XCODE_RSRCS -1 ) {
		 PASSERT_AND_RETURN(eXCodeEncoderType);
	 }

	 XCODE_ENCODERS_MAP::iterator encoderIt;
	 encoderIt = m_xcodeEncodesrMap->find((eXcodeRsrcType)eXCodeEncoderType);
	 if(encoderIt == m_xcodeEncodesrMap->end()) {
		 PASSERT_AND_RETURN(1);
	 }


	  CVideoBridgeXCodeEncoder* pXCodeVideoEncoder = (CVideoBridgeXCodeEncoder*)encoderIt->second;

	  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pXCodeVideoEncoder));


	  if (!(CPObject::IsValidPObjectPtr(pBridgePartyVideoOutParams)))
	  {
	    PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::UpdateVideoOutParams - Failed, invalid Video Bridge params object, ConfName: ", m_pConfName);
	   // m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
	    return;
	  }

	  if (!(pBridgePartyVideoOutParams->IsValidParams()))
	  {
	    PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::UpdateVideoOutParams - Failed, invalid Video Bridge params, ConfName:", m_pConfName);
	    //m_pConfApi->SendResponseMsg(pParty, VIDEO_BRIDGE_MSG, PARTY_VIDEO_OUT_UPDATED, statIllegal, 1);
	    return;
	  }
	  pXCodeVideoEncoder->UpdateVideoOutParams(pBridgePartyVideoOutParams);
}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::BuildLayout()
{
	const CImage*  pContentDecoderImage = GetContentImage();
	// Romem build layout
	DWORD partyDecoderRscId;
	if(CPObject::IsValidPObjectPtr(pContentDecoderImage) && !pContentDecoderImage->isMuted())
	{
		partyDecoderRscId = pContentDecoderImage->GetPartyRsrcId();
		if((*m_ConfContentLayout1x1)[0] != NULL)
		    (*m_ConfContentLayout1x1)[0]->SetImageId(partyDecoderRscId);
	}
}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::ConnectContentEncoders()
{
  BYTE isIVR = NO;
  for (XCODE_ENCODERS_MAP::iterator _ii = m_xcodeEncodesrMap->begin(); _ii != m_xcodeEncodesrMap->end(); _ii++)
	  _ii->second->Connect(isIVR);
}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnXCodeEncoderEndConnectIDLE(CSegment* pParam)
{

	PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnXCodeEncoderEndConnectIDLE, ConfNaFme - ", m_pConfName);

	WORD            receivedStatus = statOK;
	EMediaDirection eMediaDirection;
	DWORD           encoderIndex;
	CTaskApp*       pParty         = NULL;
	*pParam >> (void*&)pParty;
	*pParam >> (WORD&)receivedStatus;
	*pParam >> (WORD&)eMediaDirection;
	*pParam >> (DWORD&)encoderIndex;

	if(encoderIndex > eXcodeH264LinksEncoder) {
		PASSERT_AND_RETURN(encoderIndex);
	}

	if (receivedStatus != statOK)
	{
		PASSERT(receivedStatus);
		ON(m_closeAfterOpenEncodesFailed);
		DeleteTimer(CONNECT_XCODE_VB_TOUT);
		m_pConfApi->EndXCodeBrdgConnect(receivedStatus);
		TRACEINTO <<  "XCode Encoder Failed to connect "
				 << "- Encoder Type:" << eXcodeRsrcTypeNames[encoderIndex]
		         << ", Status:"   << receivedStatus;
		return;
	}

	XCODE_ENCODERS_MAP::iterator encoderIt;
	CVideoBridgeXCodeEncoder* pVideoBridgeXCodeEncoder = NULL;
	encoderIt = m_xcodeEncodesrMap->find((eXcodeRsrcType)encoderIndex);

	if(encoderIt != m_xcodeEncodesrMap->end())
	{
		PTRACE2INT(eLevelInfoNormal, "CVideoContentXcodeBridge::OnXCodeEncoderEndConnectIDLE - XCode Encoder is connected, send empty change layout to encoder, xcodEncoderIndex = ", encoderIndex);
		pVideoBridgeXCodeEncoder = encoderIt->second;
		PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVideoBridgeXCodeEncoder));
		pVideoBridgeXCodeEncoder->ChangeLayout(m_ConfContentLayout1x1, m_pVisualEffects, m_pSiteNameInfo, 0, 1);
	}
	else
	{
		TRACEINTO <<  "XCode Encoder was not found in Encoders list - End XCode session"
						 << "- Encoder Type:" << eXcodeRsrcTypeNames[encoderIndex]
				         << "Conf Name: " << m_pConfName << "\n";
		m_pConfApi->EndXCodeBrdgConnect(STATUS_FAIL);
		return;
	}

	if (CheckAllXCodeEncodersConnected()) // maybe it should be a function that check if all the relevant resources connected
	{
		PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnXCodeEncoderEndConnectIDLE, all encoders connected, ConfName - ", m_pConfName);
		m_state = CONNECTED;
		//SendChangeLayoutToAllLevelEncoders();
		DeleteTimer(CONNECT_XCODE_VB_TOUT);
		m_pConfApi->EndXCodeBrdgConnect(statOK);
	}
}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnXCodeEncoderEndConnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnXCodeEncoderEndConnectCONNECTED, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           encoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)encoderIndex;

  if (receivedStatus != statOK)
    PASSERT(receivedStatus);

  PTRACE2INT(eLevelInfoNormal, "CVideoContentXcodeBridge:OnXCodeEncoderEndConnectCONNECTED, event from encoder index: ", encoderIndex);

}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnXCodeEncoderEndConnectDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnXCodeEncoderEndConnectDISCONNECTING, ConfName - ", m_pConfName);

  CTaskApp*       pParty         = NULL;
  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           encoderIndex;
  *pParam >> (void*&)pParty;
  *pParam >> receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)encoderIndex;

  if (receivedStatus != statOK)
  {
    PASSERT(receivedStatus);
    m_resourcesStatus = receivedStatus;
  }
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoContentXcodeBridge::CheckAllXCodeEncodersConnected() const
{
	WORD ret_value              = FALSE;
	WORD num_xcode_encoders_connected = 0;

	for (XCODE_ENCODERS_MAP::iterator _ii = m_xcodeEncodesrMap->begin();_ii != m_xcodeEncodesrMap->end(); _ii++)
	{
		if (_ii->second->IsCopEncoderInConnectedState())
		{
			num_xcode_encoders_connected++;
		}
	}

	if (num_xcode_encoders_connected == m_xcodeEncodesrMap->size())
	{
		PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::CheckAllLevelEncodersConnected All level encoders connected, ConfName - ", m_pConfName);
		ret_value = TRUE;
	}
	else
		PTRACE2INT(eLevelInfoNormal, "CVideoContentXcodeBridge::CheckAllLevelEncodersConnected , Num Of connected encoders are: ", num_xcode_encoders_connected);

	return ret_value;
 }
///////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntlContent* CVideoContentXcodeBridge::GetContentDecoder()
{
  return m_pContentDecoder;
}
///////////////////////////////////////////////////////////////////////////
CImage* CVideoContentXcodeBridge::GetContentImage()
{
  CBridgePartyVideoIn* pVideoIn = (CBridgePartyVideoIn*)GetContentDecoder()->GetBridgePartyIn();
  PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pVideoIn), NULL);

  return (CImage*)pVideoIn->GetPartyImage();
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnTimerConnectXCodeBridgeIDLE(CSegment* pParam)
{
	PTRACE2(eLevelError, "CVideoContentXcodeBridge::OnTimerConnectXCodeBridgeIDLE - Failed to connect Xcode Encoders/Decoder, Name - ", m_pConfName);
	PASSERT(101);
	m_pConfApi->EndXCodeBrdgConnect(statVideoInOutResourceProblem);
}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnXCodeEncoderVideoOutUpdated(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnXCodeEncoderVideoOutUpdated - Name - ", m_pConfName);
	WORD            receivedStatus = statOK;
	EMediaDirection eMediaDirection;
	DWORD           xcodEncoderIndex;
	CTaskApp*       pParty         = NULL;
	CVideoBridgeXCodeEncoder* pVideoBridgeXCodeEncoder = NULL;
	*pParam >> (void*&)pParty;
	*pParam >> (WORD&)receivedStatus;
	*pParam >> (WORD&)eMediaDirection;
	*pParam >> (DWORD&)xcodEncoderIndex;

	if (receivedStatus != statOK)
	{
		PASSERT(receivedStatus);
	}

	PTRACE2INT(eLevelInfoNormal, "CVideoContentXcodeBridge::OnXCodeEncoderVideoOutUpdated, xcodEncoderIndex = ", xcodEncoderIndex);
	if (xcodEncoderIndex >= MAX_CONTENT_XCODE_RSRCS -1 ) {
		PASSERT_AND_RETURN(xcodEncoderIndex);
	}

	XCODE_ENCODERS_MAP::iterator encoderIt;
	encoderIt = m_xcodeEncodesrMap->find((eXcodeRsrcType)xcodEncoderIndex);
	if(encoderIt != m_xcodeEncodesrMap->end())
	{
		PTRACE2INT(eLevelInfoNormal, "CVideoContentXcodeBridge::OnXCodeEncoderVideoOutUpdated - Update XCode Encoder ended successfully, send intra to encoder, xcodEncoderIndex = ", xcodEncoderIndex);
		pVideoBridgeXCodeEncoder = encoderIt->second;
		PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVideoBridgeXCodeEncoder));
		pVideoBridgeXCodeEncoder->FastUpdate();
	}
}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	CBridge::InitBridgeParams(pBridgePartyInitParams);

	if (pBridgePartyInitParams->GetMediaOutParams())
	{
		CVisualEffectsParams* pGatheringVisualEffectsParams = m_pConf->GetGatheringVisualEffects();
		if (pGatheringVisualEffectsParams)
			((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetVisualEffects(pGatheringVisualEffectsParams);
		else
			((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetVisualEffects(m_pVisualEffects);

		ePartyLectureModeRole partyLectureModeRole = GetLectureModeRoleForParty(pBridgePartyInitParams->GetPartyName());
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetPartyLectureModeRole(partyLectureModeRole);
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetLayoutType(m_layoutType);
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetVideoQualityType(m_videoQuality);
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetIsVideoClarityEnabled(m_isVideoClarityEnabled);
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetIsSiteNamesEnabled(m_isSiteNamesEnabled);
	}

	if (pBridgePartyInitParams->GetMediaInParams())
	{
		PASSERT(1);
	}
}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::SendChangeLayoutToAllLevelEncoders()
{
	PTRACE(eLevelInfoNormal, "CVideoContentXcodeBridge::SendChangeLayoutToAllLevelEncoders");
	BuildLayout();
	for (XCODE_ENCODERS_MAP::iterator _ii = m_xcodeEncodesrMap->begin(); _ii != m_xcodeEncodesrMap->end(); _ii++)
	{
		_ii->second->ChangeLayout(m_ConfContentLayout1x1, m_pVisualEffects, m_pSiteNameInfo, 0, 1);
		_ii->second->FastUpdate();
	}
}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::SetEmptyLayoutForAllLevelEncoders()
{
	TRACEINTO << "Set empty layout for all Content Encoders";
	(*m_ConfContentLayout1x1)[0]->SetImageId(0);
	for (XCODE_ENCODERS_MAP::iterator _ii = m_xcodeEncodesrMap->begin(); _ii != m_xcodeEncodesrMap->end(); _ii++)
	{
		_ii->second->ChangeLayout(m_ConfContentLayout1x1, m_pVisualEffects, m_pSiteNameInfo, 0, 1);
		_ii->second->FastUpdate();
	}
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnConfTerminateDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "State:" << m_state;

  StartTimer(BRIDGE_DISCONNECT_TOUT, XCODE_BRIDGE_DISCONNECT_TIME_OUT_VALUE);

  CloseXCodeEncoders();

	if (m_pPartyList->size())
  {
    // Upon receiving TERMINATE event, Bridge should be empty
		TRACEINTO << *m_pPartyList << " - Bridge not empty";
    PASSERT(101);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnConfDisConnectConfIDLE(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnConfDisConnectConfCONNECTED Changing State to DISCONNECTING: Name - ", m_pConfName);

  // Change to disconnecting state - to prevent connecting new parties
  m_state = DISCONNECTING;
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnTimerDisconnetDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnTimerDisconnetDISCONNECTING, Conf Name: ",m_pConfName);
  DBGPASSERT(XCODE_BRIDGE_DISCONNECT_TIME_OUT_VALUE );
  m_resourcesStatus = STATUS_FAIL;
  EndDisconnect();
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::EndDisconnect()
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::EndDisconnect, Conf Name: ",m_pConfName);

  if (IsValidTimer(BRIDGE_DISCONNECT_TOUT))
    DeleteTimer(BRIDGE_DISCONNECT_TOUT);

  CreateAndSendDeallocateContentEncoders();

  WORD status = m_resourcesStatus;
  m_state = IDLE;
  m_pConfApi->EndXCodeBrdgDisConnect(status);
}
// ------------------------------------------------------------
void CVideoContentXcodeBridge::CreateAndNewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl, CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
	NewPartyCntl(pVideoBrdgPartyCntl);
	pVideoBrdgPartyCntl->Create(pVideoBridgePartyInitParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::NewPartyCntl(CVideoBridgePartyCntl*& pVideoBrdgPartyCntl)
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::NewPartyCntl  ConfName - ", m_pConfName);
  pVideoBrdgPartyCntl = new CVideoBridgePartyCntlXCode();
}
//------------------------------------------------------------------------------------------
void CVideoContentXcodeBridge::OnEndPartyConnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnEndPartyConnectCONNECTED, ConfName:", m_pConfName);

	PartyRsrcID PartyId;
	WORD status;
	eXcodeRsrcType   eXcodeEncoderIndex;
	CVideoBridgeXCodeEncoder* pVideoBridgeXCodeEncoder = NULL;

	CSegment* pTempParam = new CSegment(*pParam);
	*pTempParam >> PartyId >> status;
	POBJDELETE(pTempParam);
	CVideoBridgePartyCntlXCode* pPartyCntl = (CVideoBridgePartyCntlXCode*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	eXcodeEncoderIndex = GetXCodeEncoderIndex(pPartyCntl);

	if (eXcodeEncoderIndex >= MAX_CONTENT_XCODE_RSRCS -1 ) {
		PASSERT_AND_RETURN(eXcodeEncoderIndex);
	}

	XCODE_ENCODERS_MAP::iterator encoderIt;
	encoderIt = m_xcodeEncodesrMap->find(eXcodeEncoderIndex);
	if(encoderIt == m_xcodeEncodesrMap->end()) {
		PASSERT_AND_RETURN(1);
	}

	pVideoBridgeXCodeEncoder = encoderIt->second;
	if(!CPObject::IsValidPObjectPtr(pVideoBridgeXCodeEncoder)) {
		PASSERT_AND_RETURN(2);
	}
	TRACEINTO << " Send Intra Request To Content emcoder with index: " << eXcodeEncoderIndex;
	pVideoBridgeXCodeEncoder->FastUpdate();
  // this function is called twice when video connected with IVR - TBD for xcode
  //CBridge::EndPartyConnect(pParam, PARTY_VIDEO_CONNECTED);

}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::CloseXCodeEncoders()
{
  for (XCODE_ENCODERS_MAP::iterator _ii = m_xcodeEncodesrMap->begin(); _ii != m_xcodeEncodesrMap->end(); _ii++)
 	  _ii->second->DisConnect();
}
////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnXCodeEncoderEndDisconnectDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnXCodeEncoderEndDisconnectDISCONNECTING, ConfName - ", m_pConfName);

  WORD            receivedStatus = statOK;
  EMediaDirection eMediaDirection;
  DWORD           xcodeEncoderIndex;
  CTaskApp*       pParty         = NULL;
  *pParam >> (void*&)pParty;
  *pParam >> (WORD&)receivedStatus;
  *pParam >> (WORD&)eMediaDirection;
  *pParam >> (DWORD&)xcodeEncoderIndex;

  PTRACE2INT(eLevelInfoNormal, "CVideoContentXcodeBridge::OnXCodeEncoderEndDisconnectDISCONNECTING, Encoder Index - ", xcodeEncoderIndex);

  if (receivedStatus != statOK)
  {
	  PASSERT(receivedStatus);
	  XCODE_RESOURCES_MAP::iterator resourceIt = m_pXCodeRsrcMap->begin();
	  resourceIt = m_pXCodeRsrcMap->find((eXcodeRsrcType)xcodeEncoderIndex);

	  if(resourceIt == m_pXCodeRsrcMap->end())
	  {
	    PASSERT(xcodeEncoderIndex);
	  }
	  else
	  {
		  resourceIt->second->isFaulty=TRUE;
	  }
	  m_resourcesStatus = receivedStatus;
  }
  DestroyXCodeEncoder(xcodeEncoderIndex);

  if (m_xcodeEncodesrMap->size()== 0)
  {
	  EndDisconnect();
  }
  else
  {
	  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge:::OnXCodeEncoderEndDisconnectDISCONNECTING not all XCODE the resources are destroyed, ConfName - ", m_pConfName);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnContentBridgeContentVideoRefreshCONNECTED(CSegment* pParam)
{
	CParty* pParty;
	  BYTE controlId;
	  BYTE isPartyAdded;

	  *pParam >> controlId >> isPartyAdded;

	  if (isPartyAdded)
	  {
		  *pParam >> (void*&)pParty;
		PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

		PartyRsrcID partyId = pParty->GetPartyRsrcID();

		CVideoBridgePartyCntlXCode* pPartyCntl = (CVideoBridgePartyCntlXCode*)GetPartyCntl(partyId);
		PASSERTSTREAM_AND_RETURN(!pPartyCntl, "PartyId:" << partyId << " - Party is not connected to the XCode bridge");

		TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << partyId << " - Party asked for intra";

		eXcodeRsrcType eXcodeEncoderIndex = GetXCodeEncoderIndex(pPartyCntl);
		if (eXcodeEncoderIndex >= MAX_CONTENT_XCODE_RSRCS - 1)
		{
			  PASSERT_AND_RETURN(eXcodeEncoderIndex);
		  }

		XCODE_ENCODERS_MAP::iterator encoderIt = m_xcodeEncodesrMap->find(eXcodeEncoderIndex);
		PASSERT_AND_RETURN(encoderIt == m_xcodeEncodesrMap->end());

		CVideoBridgeXCodeEncoder* pVideoBridgeXCodeEncoder = encoderIt->second;
		PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pVideoBridgeXCodeEncoder));

		  pVideoBridgeXCodeEncoder->FastUpdate();
	  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::DestroyXCodeEncoder(WORD xcodEncoderIndex)
{
	PTRACE2INT(eLevelInfoNormal, "CVideoContentXcodeBridge::DestroyXCodeEncoder, xcodEncoderIndex = ", xcodEncoderIndex);
	if (xcodEncoderIndex >= MAX_CONTENT_XCODE_RSRCS -1 ) {
		PASSERT_AND_RETURN(xcodEncoderIndex);
	}

	XCODE_ENCODERS_MAP::iterator encoderIt;
	encoderIt = m_xcodeEncodesrMap->find((eXcodeRsrcType)xcodEncoderIndex);
	if(encoderIt != m_xcodeEncodesrMap->end())
	{
		POBJDELETE(encoderIt->second);
		m_xcodeEncodesrMap->erase(encoderIt->first);
	}
}
////////////////////////////////////////////////////////////////////////////
eXcodeRsrcType CVideoContentXcodeBridge::GetXCodeEncoderIndex(CVideoBridgePartyCntlXCode* pPartyCntl) const
{
	eXcodeRsrcType xcodeEncoderIndex = eXcodeEncoderDummy;
  if (!CPObject::IsValidPObjectPtr(pPartyCntl))
  {
    PTRACE(eLevelError, "CVideoContentXcodeBridge::GetXCodeEncoderIndex, not valid pPartyCntl");
    PASSERT(111);
    return xcodeEncoderIndex;
  }

  xcodeEncoderIndex= (eXcodeRsrcType)pPartyCntl->GetXCodeEncoderIndex();
  return xcodeEncoderIndex;
}

////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::OnConfConnectPartyCONNECTED(CSegment* pParam)
{
  CBridgePartyInitParams partyInitParams;
  partyInitParams.DeSerialize(NATIVE, *pParam);

  // check init params validity - from CVideoBridge
  if (!partyInitParams.IsValidParams())
  {
    if (CorrectPartyInitParams(partyInitParams) == FALSE)
    {
			TRACEINTO << "ConfName:" << m_pConfName << " - Failed, invalid video params";
      CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_XCODE_DISCONNECTED, statInvalidPartyInitParams);
      return;
    }
  }

  if (!partyInitParams.IsValidXCodeParams())
  {
		TRACEINTO << "ConfName:" << m_pConfName << " - Failed, invalid XCODE params";
    CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_XCODE_DISCONNECTED, statInvalidPartyInitParams);
    return;
  }

  partyInitParams.SetSiteNameInfo(m_pSiteNameInfo);

	const CParty* pParty = (CParty*)partyInitParams.GetParty();
	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CVideoBridgePartyCntl* pVideoBrdgPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);

  // If this party exist in this conf...
  // We don't need to create new VideoBridgePartyCntl - only add or update one of the directions -VideoIn or VideoOut
    if( pVideoBrdgPartyCntl )
    {
		TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << partyId << " - Connecting existing party";

      pVideoBrdgPartyCntl->Update((CVideoBridgePartyInitParams*)(&partyInitParams));
      pVideoBrdgPartyCntl->Connect(partyInitParams.IsIvrInConf());
    }
    else
  {
    // If move from other conf....
		pVideoBrdgPartyCntl = (CVideoBridgePartyCntl*)partyInitParams.GetPartyCntl();
		if (pVideoBrdgPartyCntl)
    {
			PASSERTSTREAM(1, "ConfName:" << m_pConfName << ", PartyId:" << partyId << " - XCODE bridge does not support move");
      CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_XCODE_DISCONNECTED, statInvalidPartyInitParams);
      return;
    }
    else // NOT In MOVE
    {
			TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << partyInitParams.GetPartyRsrcID() << " - Creating new VideoBridgePartyCntl";

      CreateAndNewPartyCntl(pVideoBrdgPartyCntl,(CVideoBridgePartyInitParams*)&partyInitParams);

      // Insert the party to the PartyCtl List and activate Connect on it
      ConnectParty(pVideoBrdgPartyCntl, partyInitParams.IsIvrInConf());
    }
  }
}

// ------------------------------------------------------------------------------------------
void CVideoContentXcodeBridge::OnConfDisConnectPartyCONNECTED(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID partyId = partyDisconnectParams.GetPartyId();
	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << partyId << ", MediaDirection:" << eMediaDirection;

	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party already disconnected";

		const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
	PASSERT_AND_RETURN(!pParty);

		m_pConfApi->PartyBridgeResponseMsg(pParty, XCODE_BRDG_MSG, PARTY_XCODE_DISCONNECTED, statOK, 1, eNoDirection);
		return;
	}

	pPartyCntl->SetDisConnectingDirectionsReq(eMediaDirection);

	CBridge::DisconnectParty(partyId);
}
// ------------------------------------------------------------------------------------------
void CVideoContentXcodeBridge::OnConfDisConnectPartyDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnConfDisConnectPartyDISCONNECTING - ConfName:", m_pConfName);

	OnConfDisConnectPartyCONNECTED(pParam);
}
// ------------------------------------------------------------------------------------------
void CVideoContentXcodeBridge::OnEndPartyDisConnect(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnEndPartyDisConnect - ConfName:", m_pConfName);
	CBridge::EndPartyDisConnect(pParam, PARTY_XCODE_DISCONNECTED);
}

// ---------------------------------------------------------------------------
void CVideoContentXcodeBridge::OnConfContentBridgeStartPresentationCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnConfContentBridgeStartPresentationCONNECTED - ConfName:", m_pConfName);
  if(!m_isFirstContentSessionStarted)
  {
	  ON(m_isFirstContentSessionStarted);
	  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnConfContentBridgeStartPresentationCONNECTED - 1st Content Session Started - ConfName:", m_pConfName);
  }
}
/*
// ---------------------------------------------------------------------------
void CVideoContentXcodeBridge::OnConfContentBridgeStopPresentationCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnConfContentBridgeStopPresentationCONNECTED - ConfName:", m_pConfName);

	m_ContentProtocol = 0xFFFFFFFF;
	m_ContentRate = 0;

	ContentDecoderConditionChanged(eCondition_StopPresentation);
	RemoveContentDecoderFromConfMixBeforeContentStoped();

	StartTimer(AFTER_STOP_CONTENT_TIMER,AFTER_STOP_CONTENT_TOUT);
}*/
// ------------------------------------------------------------------------------------------
void CVideoContentXcodeBridge::OnContentDecoderVideoInSyncedCONNECTED(CSegment* pParam)
{
	 PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::OnContentDecoderVideoInSyncedCONNECTED, Send Content to all Content Encoders - ConfName: ", m_pConfName);
	//AddContentDecoderToConfMixAfterVideoInSynced();
	SendChangeLayoutToAllLevelEncoders();
}

void CVideoContentXcodeBridge::OnContentDecoderDisconnectedCONNECTED(CSegment* pParam)
{
	TRACEINTO << "set empty layout for all Content Encoders.";
	SetEmptyLayoutForAllLevelEncoders();
}
// ------------------------------------------------------------------------------------------
void CVideoContentXcodeBridge::OnEndDestroyContentDecoderCONNECTED(CSegment* pParam)
{
  CTaskApp* pParty = NULL;
  WORD      status = statIllegal;

  *pParam >> (void*&)pParty >> status;

  if (statOK == status)
    TRACEWARN << "CVideoContentXcodeBridge::OnEndDestroyContentDecoderCONNECTED - ContentDecoder problem (SETUP Failed), ContentDecoder deallocated - Terminame XCode Bridge, ConfName:" << m_pConfName;
  else
  {
    TRACEWARN << "CVideoContentXcodeBridge::OnEndDestroyContentDecoderCONNECTED - ContentDecoder problem (SETUP Failed), ContentDecoder deallocation failed - Terminame XCode Bridge, ConfName:" << m_pConfName;
    PASSERT(1);
  }

  // close XCode Bridge
  // Error Handling
  Disconnect();
  Terminate();
}
// ------------------------------------------------------------------------------------------
void CVideoContentXcodeBridge::OnEndDestroyContentDecoderDISCONNECTING(CSegment* pParam)
{
   TRACEINTO << "CVideoContentXcodeBridge::OnEndDestroyContentDecoderDISCONNECTING - ContentDecoder problem (SETUP Failed), ContentDecoder deallocated - Not supported in DISCONNECTING state, ConfName:" << m_pConfName;
   PASSERT(1);
}
// ------------------------------------------------------------------------------------------
void CVideoContentXcodeBridge::AskContentSpeakerForIntra()
{
  PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::AskContentSpeakerForIntra - ConfName:", m_pConfName);
	//Ask Conf to ask ContentSpeaker For Intra
	BYTE controlID = 0xFF; //NOT relevant for content bridge but used by Atara for some reason - NEED to be removed !!
	m_pConfApi->ContentVideoRefresh(controlID);
}

// ------------------------------------------------------------------------------------------
 LayoutType CVideoContentXcodeBridge::GetLegacyContentDefaultLayout()
{
	PTRACE(eLevelInfoNormal, "CVideoContentXcodeBridge::GetLegacyContentDefaultLayout");

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string legacyDefaultLayoutTypeStr;
	sysConfig->GetDataByKey("LEGACY_EP_CONTENT_DEFAULT_LAYOUT",	legacyDefaultLayoutTypeStr);
	LayoutType sysConfigLayoutType = TranslateSysConfigStringToLayoutType(legacyDefaultLayoutTypeStr);
	if (CP_NO_LAYOUT == sysConfigLayoutType)
	{
		PTRACE2(eLevelError,"CVideoContentXcodeBridge::GetLegacyContentDefaultLayout : problem translating the layout type,  string: ",legacyDefaultLayoutTypeStr.c_str());
		PASSERT(1);
		sysConfigLayoutType = CP_LAYOUT_1P4VER;
	}

	return sysConfigLayoutType;
}
//////////////////////////////////////////////////////////////////////////////////////////
void CVideoContentXcodeBridge::CreateAndSendDeallocateContentEncoders()
{
	PTRACE2(eLevelInfoNormal, "CVideoContentXcodeBridge::CreateAndSendDeallocateContentEncoders, Conf Name: ",m_pConfName);

	CSegment* seg = new CSegment;
	// Put the number of XCode Resources to deallocate.
	WORD numXcodeEncodersToDealloc = 0;

	if(m_closeAfterOpenEncodesFailed || (!m_isFirstContentSessionStarted && IsASSIPConf()))
		numXcodeEncodersToDealloc = m_pConf->GetNumXCodeRsrc();
	else
		numXcodeEncodersToDealloc = m_pConf->GetNumXCodeRsrc()-1;

	*seg << numXcodeEncodersToDealloc;
	int internalCounter = 0;
	TRACEINTO << "Call Resource Process to de-allocfate XCode Encoders, Num of Resources to deallocate:  " << numXcodeEncodersToDealloc << " Conf Name:  " << m_pConfName  << "\n";
	for(XCODE_RESOURCES_MAP::iterator _ii = m_pXCodeRsrcMap->begin(); _ii !=  m_pXCodeRsrcMap->end(); _ii++)
	{
		internalCounter++;
		if(internalCounter > numXcodeEncodersToDealloc)break;

		DWORD contentEncoderMonitorPartyId = 0xFFFFFFFF;
		DEALLOC_PARTY_REQ_PARAMS_S deallocatePartyParams;
		memset(&deallocatePartyParams, 0, sizeof(DEALLOC_PARTY_REQ_PARAMS_S));
		deallocatePartyParams.monitor_conf_id  = m_pConf->GetMonitorConfId();
		ContentXcodeRsrcDesc* pContentXcodeRsrcDesc = _ii->second;
		contentEncoderMonitorPartyId = pContentXcodeRsrcDesc->monitorRsrcPartyId;
		deallocatePartyParams.monitor_party_id = contentEncoderMonitorPartyId;

		WORD isFaultyXCodeRsrc = pContentXcodeRsrcDesc->isFaulty;
		if(isFaultyXCodeRsrc)
		{
			if(_ii->first != eXcodeContentDecoder)
			{
				deallocatePartyParams.rsrcsWithProblems[eLogical_video_encoder].connectionId = pContentXcodeRsrcDesc->connectionId;
				deallocatePartyParams.rsrcsWithProblems[eLogical_video_encoder].logicalRsrcType = eLogical_video_encoder_content;
				deallocatePartyParams.numOfRsrcsWithProblems = 1;
			}
		}
		deallocatePartyParams.force_kill_all_ports = FALSE;
		TRACEINTO << "DEALLOC_PARTY_REQ_PARAMS_S:\n"
				<< "XCode Resource Type: " << eXcodeRsrcTypeNames[_ii->first] << '\n'
				<< "monitor_conf_id    =   "<< deallocatePartyParams.monitor_conf_id <<'\n'
				<< "monitor_party_id   =   "<< deallocatePartyParams.monitor_party_id << '\n'
				<< "Rsrc_party_id   =   "<< pContentXcodeRsrcDesc->rsrcEntityId << '\n'
				<< "Connection ID      =   " << pContentXcodeRsrcDesc->connectionId << '\n';
		seg->Put((BYTE*)(&deallocatePartyParams), sizeof(DEALLOC_PARTY_REQ_PARAMS_S));
		// Clear Rsrc Party ID
		GetLookupIdParty()->Clear(pContentXcodeRsrcDesc->rsrcEntityId);
	}
	STATUS res = SendReqToResourceAllocator(seg, DEALLOCATE_CONTENT_XCODE_REQ);
	PASSERT(res);
}
///////////////////////////////////////////////////////////////////////////////////
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
STATUS CVideoContentXcodeBridge::SendReqToResourceAllocator(CSegment* seg, OPCODE opcode, BOOL isSync)
{
	STATUS res = STATUS_OK;
	CProcessBase* process = CProcessBase::GetProcess();
	if (!process)
	{
		PASSERT(101);
	}

	//CManagerApi api(eProcessResource);
	CTaskApi resourceManagerApi(eProcessResource,eManager);
	const StateMachineDescriptor stateMachine = GetStateMachineDescriptor();
	if (isSync)
		res = resourceManagerApi.SendMessageSync(seg, opcode, CONF_RSRC_REQ_TOUT);
	else
		res = resourceManagerApi.SendMsg(seg, opcode, &m_pConfApi->GetRcvMbx(), &stateMachine);
	return res;
}

void CTelepresenceLayoutMngr::DumpRoomList(bool fullInfo)
{
	std::ostringstream msg;
	VBRIDGE_ROOM_INFO_MAP::iterator room_end = m_RoomList.end();
	for (VBRIDGE_ROOM_INFO_MAP::iterator room_itr = m_RoomList.begin(); room_itr != room_end; ++room_itr)
	{
		msg << "\n";
		room_itr->second.Dump(msg, fullInfo);
	}
	TRACEINTO << msg.str().c_str();
}


//==========================================================================================================//
// TELEPRESENCE_LAYOUTS
WORD CTelepresenceLayoutMngr::GetRoomPartiesIds(DWORD partyRsrcId,std::set<DWORD>& roomPartiesIds)
{
	WORD num_of_cameras = 0;
	for(VBRIDGE_ROOM_INFO_MAP::iterator rit = m_RoomList.begin();rit != m_RoomList.end();++rit)
	{
		if(rit->second.IsPartyInRoom(partyRsrcId)){
			num_of_cameras = rit->second.GetRoomPartyIds(roomPartiesIds);
		}
	}
	return num_of_cameras;
}
//==========================================================================================================//


////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
///                       CAVMCUMngr
/////////////////////////////////////////////////////////////////////////////
CAVMCUMngr::CAVMCUMngr(CVideoBridgeCP* pBridge):m_pBridge(pBridge)
{

}
/////////////////////////////////////////////////////////////////////////////
CAVMCUMngr::~CAVMCUMngr()
{

}
/////////////////////////////////////////////////////////////////////////////
void CAVMCUMngr::AddLinkToLinksVector(DWORD linkRsrcId)
{
	m_AVMCULinksVector.push_back(linkRsrcId);
}
/////////////////////////////////////////////////////////////////////////////
bool CAVMCUMngr::RemoveLinkFromLinksVector(DWORD linkRsrcId)
{
	CAVMCULinksVector::iterator _ii = std::find(m_AVMCULinksVector.begin(), m_AVMCULinksVector.end(), linkRsrcId);
	if (_ii != m_AVMCULinksVector.end())
	{
		m_AVMCULinksVector.erase(_ii);
		return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////
bool CAVMCUMngr::FindLinkInLinksVector(DWORD linkRsrcId)
{
	CAVMCULinksVector::iterator _ii = std::find(m_AVMCULinksVector.begin(), m_AVMCULinksVector.end(), linkRsrcId);
	return _ii != m_AVMCULinksVector.end() ? true : false;
}
/////////////////////////////////////////////////////////////////////////////
LyncMsi CAVMCUMngr::GetMSAVMCULocalAUdioMSI()
{
	LyncMsi audioLoaclMSI = 0;

	PASSERT_AND_RETURN_VALUE(!m_pBridge, audioLoaclMSI);
	PASSERT_AND_RETURN_VALUE(!m_masterRsrcPartyID, audioLoaclMSI);

	CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl*)m_pBridge->GetPartyCntl(m_masterRsrcPartyID);
	PASSERT_AND_RETURN_VALUE(!pVideoPartyCntl, audioLoaclMSI);

	return pVideoPartyCntl->GetMSAVMCUaudioLocalMsi();
}
/////////////////////////////////////////////////////////////////////////////
/*CImage*  CVideoBridgeCP::GetPartyImageByVideoMSI(DWORD newVideoMSI,DWORD& partyRsrcId)
{
	CImage* partyImage = NULL;
	DWORD tmpMsi = -1;
	TRACEINTO << "DBG newVideoMSI:" << newVideoMSI;
	for(DWORD i=0;i<m_pPartyList->Size() ; i++)
	{
		CVideoBridgePartyCntl* pCurrParty = (CVideoBridgePartyCntl*)m_pPartyList->At(i);
		if(NULL == pCurrParty) continue;

		partyImage = GetPartyImageLookupTable()->GetPartyImage(pCurrParty->GetPartyRsrcID());
		if(partyImage)
		{
			tmpMsi = partyImage->GetVideoMSI();
			TRACEINTO << "DBG tmpMsi:" << tmpMsi << ", partyRsrcId:" << pCurrParty->GetPartyRsrcID()<< "NULL != partyImage:" << (	NULL != partyImage);

			if(tmpMsi == newVideoMSI)
			{
				partyRsrcId = pCurrParty->GetPartyRsrcID();
				return partyImage;
			}
		}
	}

	return partyImage;
}

*/
/////////////////////////////////////////////////////////////////////////////
CImage*  CVideoBridgeCP::GetPartyImageByVideoMSI(DWORD newVideoMSI,DWORD& partyRsrcId)
{
	CImage* partyImage = NULL;
	PASSERT_AND_RETURN_VALUE(!m_pAVMCUMngr,NULL);

	CAVMCULinksVector& rAVMCULinksVector = m_pAVMCUMngr->GetMCULinksVector();

	for(CAVMCULinksVector::iterator AVMngrIt = rAVMCULinksVector.begin(); AVMngrIt != rAVMCULinksVector.end(); ++AVMngrIt )
	{
		partyRsrcId = *AVMngrIt;
		CVideoBridgePartyCntl* pVideoPartyCntl = (CVideoBridgePartyCntl *)(GetPartyCntl(partyRsrcId));
		if (NULL == pVideoPartyCntl)
		{

			TRACEINTO << "PartyId:" << partyRsrcId << ", ConfId:" << m_pConf->GetConfId() << " - pVideoPartyCntl not found ";
			continue;
		}

		BOOL rIsOutPortOpened=0, rIsInPortOpened = 0;
		pVideoPartyCntl->ArePortsOpened(rIsInPortOpened, rIsOutPortOpened);
		if(!rIsInPortOpened)
		{
			TRACEINTO << "PartyId:" << partyRsrcId << " This is not In Slave, Slave Name: " << pVideoPartyCntl->GetFullName() << "\n";
			continue;
		}

		DWORD currentPartyVideoMSI = pVideoPartyCntl->GetVideoMSI();

		if(IsValidMSI(currentPartyVideoMSI) &&  currentPartyVideoMSI == newVideoMSI )
		{
			partyImage = (CImage*)(pVideoPartyCntl->GetPartyImage());
			TRACEINTO << "VideoMsi: " << newVideoMSI << " is Found assigned for party: " << pVideoPartyCntl->GetFullName() << ", partyRsrcId: " << partyRsrcId;
			return partyImage;
		}
	}
	return partyImage;

}
//==========================================================================================================================//
// TELEPRESENCE_LAYOUTS
void CVideoBridgeCP::SetTelepresenceLayoutMode(ETelePresenceLayoutMode newLayoutMode)
{
	CSegment seg;
	seg << (DWORD)newLayoutMode;
	DispatchEvent(SET_TELEPRESENCE_LAYOUT_MODE, &seg);
}
//==========================================================================================================================//
void CVideoBridgeCP::OnConfSetTelepresenceLayoutModeConnected(CSegment* pParam)
{
	if(NULL == pParam){
		DBGPASSERT(1);
		return;
	}
	DWORD tmpLayoutMode = 0;
	*pParam >> tmpLayoutMode;
	ETelePresenceLayoutMode newLayoutMode = (ETelePresenceLayoutMode)(tmpLayoutMode);

	TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG newLayoutMode = " << TelePresenceLayoutModeToString(newLayoutMode);

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Video Bridge are CVideoBridgePartyCntl, so casting is valid
		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->UpdateTelepresenceLayoutMode(newLayoutMode);
	}
}
//==========================================================================================================================//
