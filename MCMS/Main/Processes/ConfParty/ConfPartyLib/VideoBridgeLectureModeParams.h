// +========================================================================+
// LectureModeParams.CPP                                                    |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       VideoBridgeLectureModeParams.cpp                             |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Talya                                                        |
// -------------------------------------------------------------------------|
// Who  | Date  September-2005  | Description                               |
// -------------------------------------------------------------------------|
// +========================================================================+

#ifndef _CVideoBridgeLectureModeParams_H_
#define _CVideoBridgeLectureModeParams_H_

#include "PObject.h"
#include "VideoDefines.h"
#include "LectureModeParams.h"
#include "ObjString.h"

typedef enum
{
  eLmAction_None = 0,
  eLmAction_LecturerConnect,
  eLmAction_LecturerConnect_CascadeLecturer,
  eLmAction_LecturerDisonnect,
  eLmAction_LecturerDisonnect_CascadeLecturer,
  eLmAction_ChangeLecturer,
  eLmAction_ChangeLecturerToCascadeLecturer,
  eLmAction_ChangeLecturerFromCascadeLecturer,
  eLmAction_ChangeLecturerToAndFromCascadeLecturer,
  eLmAction_LecturerStartAudioActivated,
  eLmAction_ChangeLecturerToAudioActivated,
  eLmAction_ChangeLecturerFromAudioActivated,
  eLmAction_LecturerStopAudioActivated,
  eLmAction_SameLecturerForcedFromAudioActivated,
}LectureModeAction;

typedef enum
{
  eLmSmartState_None = 0,
  eLmSmartState_SameLevel1Lecturer,
  eLmSmartState_SameLevel2Lecturer,
  eLmSmartState_DiffLevel1Lecturer,
  eLmSmartState_DiffLevel2Lecturer,
  eLmSmartState_Current,
} LectureSmartState;

#define WAIT_API    1
#define WAIT_BRIDGE 2

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeLectureModeParams
////////////////////////////////////////////////////////////////////////////
class CVideoBridgeLectureModeParams : public CPObject
{
  CLASS_TYPE_1(CVideoBridgeLectureModeParams, CPObject)

public:
                                 CVideoBridgeLectureModeParams();
                                 CVideoBridgeLectureModeParams(const CVideoBridgeLectureModeParams& other);
  virtual                       ~CVideoBridgeLectureModeParams();
  virtual const char*            NameOf() const { return "CVideoBridgeLectureModeParams";}
  CVideoBridgeLectureModeParams& operator=(const CVideoBridgeLectureModeParams& other);
  CVideoBridgeLectureModeParams& operator=(const CLectureModeParams& other);
  friend BYTE                    operator==(const CVideoBridgeLectureModeParams& first, const CVideoBridgeLectureModeParams& second);

  BYTE                           IsLectureModeOn() const;
  eLectureModeType               GetLectureModeType() const;
  WORD                           GetTimerInterval() const;
  const char*                    GetLecturerName() const;
  BYTE                           GetIsTimerOn() const;

  void                           SetLecturerName(const char* pLecturerName);
  void                           SetTimerInterval(WORD timerInterval)                { m_timerInterval = timerInterval;}
  void                           SetLectureModeType(eLectureModeType lectureModeType){m_lectureModeType = lectureModeType;}

  BYTE                           IsAudioActivatedLectureMode() const;
  void                           SetAudioActivatedLectureMode(BYTE isAudioActive);
  void                           SetAudioActivatedLectureMode(AudioActivatedType audioActiveType);

  void                           Dump(std::ostream& msg) const;
  void                           Dump(CObjString& msg) const;

protected:
  eLectureModeType              m_lectureModeType;
  WORD                          m_timerInterval; // interval the lecture see switch of listeners on his screen.Lets say that minimal value is 5 SECOND
  char                          m_CurrentLecturerName[H243_NAME_LEN];
  // the flag says if the timer is turned ON
  // m_byTimerOnOff==1 => timer exist
  // m_byTimerOnOff==0 => timer deleted
  BYTE                          m_byTimerOnOff;
  BYTE                          m_isAudioActivate;
};


////////////////////////////////////////////////////////////////////////////
//                        CSmartLecturerState
////////////////////////////////////////////////////////////////////////////
struct CSmartLecturerState
{
  struct CLecturerInfo
  {
    DWORD           m_PartyRsrcID;
    DWORD           m_LevelEncoderEntityId;
    DWORD           m_LevelEncoderIndex;
  };
  LectureSmartState m_eState;
  CLecturerInfo     m_oldLecturer;
  CLecturerInfo     m_newLecturer;

                    CSmartLecturerState() { Clean(); }

  const char*       GetStateName(LectureSmartState state);
  void              Clean()                   { memset(this, 0, sizeof(*this)); m_eState = eLmSmartState_None;}
  BOOL              IsSameLevelState () const { return (m_eState == eLmSmartState_SameLevel2Lecturer || m_eState == eLmSmartState_SameLevel1Lecturer); }
  BOOL              IsDiffLevelState () const { return (m_eState == eLmSmartState_DiffLevel2Lecturer || m_eState == eLmSmartState_DiffLevel1Lecturer); }
  BOOL              IsSmartLevelState() const { return (IsSameLevelState() || IsDiffLevelState()); }
};


////////////////////////////////////////////////////////////////////////////
//                        CCopLectureModeCntl
////////////////////////////////////////////////////////////////////////////
class CCopLectureModeCntl : public CPObject
{
  CLASS_TYPE_1(CCopLectureModeCntl, CPObject);

public:
                                 CCopLectureModeCntl();
  virtual                       ~CCopLectureModeCntl();
  virtual const char*            NameOf() const                               { return "CCopLectureModeCntl";}

  BYTE                           IsLectureModeActive() const;
  void                           SetLectureModeActive(BYTE isActive);
  BYTE                           IsAudioActivatedLectureMode() const;
  void                           SetAudioActivatedLectureMode(BYTE isAudioActive);

  // lecture mode actions
  void                           StartLectureMode(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams);
  void                           EndLectureMode(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams);
  void                           ChangeLecturer(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams);
  void                           StartCascadeLecturerMode(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams);
  void                           EndCascadeLecturerMode(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams);
  void                           ChangeLecturerToCascadeLink(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams);
  void                           ChangeLecturerFromCascadeLink(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams);
  void                           ChangeLecturerToAndFromCascadeLink(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams);

  void                           EndStartLectureMode();
  void                           EndEndLectureMode();
  void                           EndChangeLecturer();
  void                           EndStartCascadeLecturerMode();
  void                           EndEndCascadeLecturerMode();
  void                           EndChangeLecturerToCascadeLecturer();
  void                           EndChangeLecturerFromCascadeLecturer();
  void                           EndChangeLecturerToAndFromCascadeLink();

  LectureModeAction              GetLectureModeAction(CVideoBridgeLectureModeParams& newVideoBridgeLectureMode) const;
  LectureModeAction              GetCurrentLectureModeAction() const;
  LectureModeAction              GetLectureModeActionNew(CVideoBridgeLectureModeParams& rNewLectureModeParams) const;

  CVideoBridgeLectureModeParams* GetCurrentParams();

  // pending lecture mode action
  BYTE                           IsInAction() const;
  BYTE                           IsPendingAction() const;
  LectureModeAction              GetPendingLectureModeAction() const;
  CVideoBridgeLectureModeParams* GetPendingParams();
  void                           SetPendingAction(LectureModeAction action, CVideoBridgeLectureModeParams& rNewLectureModeParams);
  void                           UpdateCurrentActionFromPending();
  void                           UpdateCurrentParams(CVideoBridgeLectureModeParams& rNewLectureModeParams);

  // bridge actions
  void                           StartOpenCodecs();
  void                           EndOpenCodecs();

  void                           StartCloseCodecs();
  void                           EndCloseCodecs();

  void                           StartMovePartiesToVsw();
  void                           EndMovePartiesToVsw();

  void                           StartMovePartiesFromVsw();
  void                           EndMovePartiesFromVsw();

  void                           StartChangeModeToLecturer();
  void                           EndChangeModeToLecturer();

  void                           StartCascadeLinkAsLecturerPendingMode();
  void                           EndStartCascadeLinkAsLecturerPendingMode();

  void                           StartUpdateCascadeLinkAsLecturer();
  void                           EndUpdateCascadeLinkAsLecturer();

  void                           StartUpdateCascadeLinkAsNotLecturer();
  void                           EndUpdateCascadeLinkAsNotLecturer();

  BYTE                           GetOpenCodecsAction() const;
  BYTE                           GetCloseCodecsAction() const;
  BYTE                           GetMovePartiesToVswAction() const;
  BYTE                           GetMovePartiesFromVswAction() const;
  BYTE                           GetMoveVswPartiesAction() const;
  BYTE                           GetChangeModeToLecturer() const;
  BYTE                           GetStartCascadeLinkAsLecturerPendingMode() const;
  BYTE                           GetUpdateCascadeLinkAsLecturer() const;
  BYTE                           GetUpdateCascadeLinkAsNotLecturer() const;

  void                           SetWaitingToEndChangeLayout(BYTE waitingToEndChangeLayout);
  BYTE                           GetWaitingToEndChangeLayout() const;
  void                           SetWaitingToEndChangeLayoutParams(CLectureModeParams& rNewResrvationLectureMode);
  CLectureModeParams*            GetWaitingToEndChangeLayoutParams() const;

  void                           SetWaitingToLecturerDecoderSync(BYTE waitingToLecturerDecoderSync);
  BYTE                           GetWaitingToLecturerDecoderSync() const;

  void                           SetSendChangeLayoutToEveryOneAfterLecturerDecoderSync(BYTE sendChangeLayoutToEveryOne);
  BYTE                           GetSendChangeLayoutToEveryOneAfterLecturerDecoderSync() const;
  void                           Dump() const;

  const char*                    GetPrevCascadeAsLecturerName() const;
  void                           SetPrevCascadeAsLecturerName(const char* pLecturerName);

  BYTE                           IsDisconnectedCascadeLecturerParty()                                          {return m_isDisconnectedCascadeLecturerParty;}
  void                           SetIsDisconnectedCascadeLecturerParty(BYTE isDisconnectedCascadeLecturerParty){ m_isDisconnectedCascadeLecturerParty = isDisconnectedCascadeLecturerParty;}

protected:

  void                           EndLectureModeAction();
  void                           DumpLectureModeAction(CObjString& sstr, LectureModeAction action) const;

  BYTE                           m_isLectureModeActive;
  BYTE                           m_isAudioActivate;
  LectureModeAction              m_currentAction;
  BYTE                           m_changeModeToLecturer;
  BYTE                           m_openCodecs;
  BYTE                           m_closeCodecs;
  BYTE                           m_movePartiesToVsw;
  BYTE                           m_movePartiesFromVsw;
  BYTE                           m_startCascadeLinkAsLecturerPendingMode;
  BYTE                           m_updateCascadeLinkAsLecturer;
  BYTE                           m_updateCascadeLinkAsNotLecturer;
  CVideoBridgeLectureModeParams  m_currentParams;
  LectureModeAction              m_pendingAction;
  CVideoBridgeLectureModeParams  m_newParams;
  BYTE                           m_waiting_to_end_change_layout;              // wating for end of change layout befor start of lecture mode action
  CLectureModeParams*            m_pWaitingToEndChangeLayoutParams;
  BYTE                           m_waiting_to_lecturer_decoder_sync;          // waiting for lecturer decoder sync before sending change layout to relevant encoders
  BYTE                           m_send_change_layout_to_everyone_after_lecturer_decoder_sync;
  char                           m_prevCascadeAsLecturerName[H243_NAME_LEN];  // cascade as lecturer when we end cascade link as lecturer the previous  cascade lecturer name
  BYTE                           m_isDisconnectedCascadeLecturerParty;        // in case we are in mode of  eLmAction_LecturerDisonnect_CascadeLecturer,
                                                                              // eLmAction_ChangeLecturerFromCascadeLecturer, eLmAction_ChangeLecturerToAndFromCascadeLecturer,
                                                                              // and the prev cascade lecturer is disconnecting/deleted we wont send update as non lecturer mode
                                                                              // to partycntl
};

#endif // _CVideoBridgeLectureModeParams_H_
