//+========================================================================+
//                            LectureModeParams.H                          |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       VideoBridgeLectureModeParams.cpp                            |             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Talya                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  Septemper-2005  | Description                              |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#include "VideoBridgeLectureModeParams.h"
#include "SysConfig.h"
#include "ProcessBase.h"

// ------------------------------------------------------------

CVideoBridgeLectureModeParams::CVideoBridgeLectureModeParams()
{
	m_lectureModeType = eLectureModeNone;
	m_timerInterval = 0;
	strcpy(m_CurrentLecturerName,"");
	m_byTimerOnOff = FALSE;
	m_isAudioActivate = FALSE;

}

/////////////////////////////////////////////////////////////////////////////
CVideoBridgeLectureModeParams::CVideoBridgeLectureModeParams(const CVideoBridgeLectureModeParams &other)
:CPObject(other)
{
    *this=other;
}

/////////////////////////////////////////////////////////////////////////////
CVideoBridgeLectureModeParams::~CVideoBridgeLectureModeParams()
{

}
/////////////////////////////////////////////////////////////////////////////
CVideoBridgeLectureModeParams&  CVideoBridgeLectureModeParams::operator = (const CVideoBridgeLectureModeParams& other)
{
	if ( &other == this ) return *this;

	m_lectureModeType 	= other.m_lectureModeType;
	m_timerInterval 	= other.m_timerInterval;
	m_byTimerOnOff 		= other.m_byTimerOnOff;
	strncpy(m_CurrentLecturerName,other.m_CurrentLecturerName,H243_NAME_LEN);
	m_CurrentLecturerName[H243_NAME_LEN-1]='\0';
	m_isAudioActivate = other.m_isAudioActivate;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CVideoBridgeLectureModeParams&  CVideoBridgeLectureModeParams::operator = (const CLectureModeParams& rReservationLectureMode)
{
	switch( rReservationLectureMode.GetLectureModeType() )
	{
	case 1: { // regular or audio activated lecture
	  if(rReservationLectureMode.GetAudioActivated()){
	    // lecture mode - TODO
		  PTRACE(eLevelInfoNormal,"CVideoBridgeLectureModeParams::operator= AudioActivated is ON");

	  }
		//	PASSERT(101);
		//else
			m_lectureModeType = eLectureModeRegular;
		break;
			}
	case 2: { // lecture show
//		m_video_mode = LECTURE_SHOW_MODE;
//		DBGPASSERT_AND_RETURN(strcmp(pCurrentLecturer,"")==0);
//		break;
		PASSERT(101);
		break;
			}
	case 3: { // Presentation Mode
		m_lectureModeType = eLectureModePresentation;
		//m_byTimerOnOff = YES;
		break;
			}
	case 0:  // none
	default : {
		m_lectureModeType = eLectureModeNone;
		break;
			  }
	}

	m_timerInterval = 15; // default value

   	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if(pSysConfig)
	{
		std::string key = "LECTURE_MODE_TIMER_INTERVAL";
		DWORD sysConfiglectureModeTimerInterval  = 0;
		BYTE isFoundInSystemConfig = FALSE;
		isFoundInSystemConfig = pSysConfig->GetDWORDDataByKey(key, sysConfiglectureModeTimerInterval);
		if(isFoundInSystemConfig)
			m_timerInterval = (WORD)sysConfiglectureModeTimerInterval;
	}

	m_byTimerOnOff = rReservationLectureMode.GetTimerOnOff();


	if( rReservationLectureMode.GetLecturerName() != NULL && ( m_lectureModeType == eLectureModeRegular ) )
	{
		// set lecturer for regular lecture & lecture show
		strncpy(m_CurrentLecturerName,rReservationLectureMode.GetLecturerName(),H243_NAME_LEN - 1);
		m_CurrentLecturerName[H243_NAME_LEN-1]='\0';
	}
    else
		strcpy(m_CurrentLecturerName,"");

	if (m_lectureModeType == eLectureModeRegular && m_CurrentLecturerName[0] == '\0')
		m_isAudioActivate = YES; // new flag for COP

	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
BYTE operator==(const CVideoBridgeLectureModeParams& first,const CVideoBridgeLectureModeParams& second)
{
	if(first.m_lectureModeType != second.m_lectureModeType)
		return FALSE;
	if(first.m_timerInterval != second.m_timerInterval)
		return FALSE;
	if(first.m_byTimerOnOff != second.m_byTimerOnOff)
			return FALSE;
	if(strncmp(first.m_CurrentLecturerName,second.m_CurrentLecturerName,H243_NAME_LEN) != 0)
		return FALSE;
	if((first.m_isAudioActivate != 0 && second.m_isAudioActivate == 0) || (first.m_isAudioActivate == 0 && second.m_isAudioActivate != 0))
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeLectureModeParams::IsLectureModeOn() const
{
	return(m_lectureModeType != eLectureModeNone);
}
/////////////////////////////////////////////////////////////////////////////
eLectureModeType CVideoBridgeLectureModeParams::GetLectureModeType() const
{
	return m_lectureModeType;
}
/////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeLectureModeParams::GetTimerInterval() const
{
	return m_timerInterval;
}
/////////////////////////////////////////////////////////////////////////////
const char* CVideoBridgeLectureModeParams::GetLecturerName() const
{
	return m_CurrentLecturerName;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeLectureModeParams::GetIsTimerOn() const
{
	return m_byTimerOnOff;
}

/////////////////////////////////////////////////////////////////////////////
void CVideoBridgeLectureModeParams::SetLecturerName(const char* pLecturerName)
{
	strncpy(m_CurrentLecturerName,pLecturerName,H243_NAME_LEN - 1);
	m_CurrentLecturerName[H243_NAME_LEN-1]='\0';
}
//=======================================================================================================================================//
BYTE CVideoBridgeLectureModeParams::IsAudioActivatedLectureMode()const
{
  return m_isAudioActivate;
}
//=======================================================================================================================================//
void CVideoBridgeLectureModeParams::SetAudioActivatedLectureMode(BYTE isAudioActive)
{
  m_isAudioActivate = isAudioActive;
}
//=======================================================================================================================================//
void CVideoBridgeLectureModeParams::SetAudioActivatedLectureMode(AudioActivatedType audioActiveType)
{
  m_isAudioActivate = (BYTE)audioActiveType;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeLectureModeParams::Dump( std::ostream& msg ) const
{
  switch (m_lectureModeType)
  {
    case eLectureModeNone        : msg << "Mode:None"; break;
    case eLectureModeRegular     : msg << "Mode:LectureMode"; break;
    case eLectureModePresentation: msg << "Mode:PresentationMode"; break;
    default:
    {
      PASSERT((DWORD)m_lectureModeType);
    }
  }

  msg << ", Lecturer:";

  if (m_CurrentLecturerName[0] == '\0')
  {
    msg <<  "NONE";
  }
  else
  {
    msg << m_CurrentLecturerName;
  }

  msg << ", AudioActivate:" << m_isAudioActivate
      << ", Timer:" << (m_byTimerOnOff ? "ON" : "OFF")
      << ", Interval:" << m_timerInterval << "(sec)";
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeLectureModeParams::Dump(CObjString& msg) const
{
  switch (m_lectureModeType)
  {
    case eLectureModeNone        : msg << "Mode:None"; break;
    case eLectureModeRegular     : msg << "Mode:LectureMode"; break;
    case eLectureModePresentation: msg << "Mode:PresentationMode"; break;
    default:
    {
      PASSERT((DWORD)m_lectureModeType);
    }
  }

  msg << ", Lecturer:";

  if (m_CurrentLecturerName[0] == '\0')
  {
    msg <<  "NONE";
  }
  else
  {
    msg << m_CurrentLecturerName;
  }
  
  msg << ", AudioActivate:" << m_isAudioActivate
      << ", Timer:" << (m_byTimerOnOff ? "ON" : "OFF")
      << ", Interval:" << m_timerInterval << "(sec)";
}


//=======================================================================================================================================//
// class CCopLectureModeCntl
//=======================================================================================================================================//
CCopLectureModeCntl::CCopLectureModeCntl()
{
  // data members
  m_isLectureModeActive = NO;
  m_isAudioActivate = NO;

  // current actions
  m_currentAction = eLmAction_None;
  // bridge action
  m_changeModeToLecturer = 0;
  m_openCodecs = 0;
  m_closeCodecs = 0;
  m_movePartiesToVsw = 0;
  m_movePartiesFromVsw = 0;
  m_updateCascadeLinkAsLecturer = 0;
  m_updateCascadeLinkAsNotLecturer = 0;


  // current parameters
  // CVideoBridgeLectureModeParams m_currentParams;

  // pending action
  m_pendingAction = eLmAction_None;
  // pending action parameters
  //CVideoBridgeLectureModeParams m_newParams;

  m_waiting_to_end_change_layout = NO;
  m_pWaitingToEndChangeLayoutParams = new CLectureModeParams;

  m_waiting_to_lecturer_decoder_sync = NO;
  m_send_change_layout_to_everyone_after_lecturer_decoder_sync = NO;

  strcpy(m_prevCascadeAsLecturerName,"");
  m_isDisconnectedCascadeLecturerParty = NO;


}
//=======================================================================================================================================//
CCopLectureModeCntl::~CCopLectureModeCntl()
{
  POBJDELETE(m_pWaitingToEndChangeLayoutParams);
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::IsLectureModeActive()const
{
  return m_isLectureModeActive;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::SetLectureModeActive(BYTE isActive)
{
  m_isLectureModeActive = isActive;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::StartLectureMode(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams)
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartLectureMode");
  switch(m_currentAction){
    case eLmAction_None:{
      m_currentAction = eLmAction_LecturerConnect;
      m_currentParams = videoBridgeLectureModeParams;
      break;
    }
    case eLmAction_LecturerDisonnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartLectureMode - received while disconnecting");
      break;
    }
    case eLmAction_LecturerConnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartLectureMode - received while lecturer connect");
      break;
    }
    case eLmAction_ChangeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartLectureMode - received while change lecturer");
      break;
    }
    case eLmAction_LecturerConnect_CascadeLecturer:{
    	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartLectureMode - received while start cascade lecturer");
       	break;
    }
    case eLmAction_LecturerDisonnect_CascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartLectureMode - received while end cascade lecturer");
		break;
	}
    case eLmAction_ChangeLecturerToCascadeLecturer:{
    	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartLectureMode - received while end change lecturer to cascade lecturer");
    	break;
    }
    case eLmAction_ChangeLecturerFromCascadeLecturer:{
       	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartLectureMode - received while change lecturer from  cascade lecturer");
      	break;
    }
    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
       	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartLectureMode - received while change lecturer to and from cascade lecturer");
       	break;
     }

    default:{
      break;
    }
  }
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndLectureMode(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams)
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndLectureMode");
  switch(m_currentAction){
    case eLmAction_None:{
      m_currentAction = eLmAction_LecturerDisonnect;
      m_currentParams = videoBridgeLectureModeParams;
      break;
    }
    case eLmAction_LecturerDisonnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndLectureMode - received while disconnecting");
      break;
    }
    case eLmAction_LecturerConnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndLectureMode - received while lecturer connect");
      break;
    }
    case eLmAction_ChangeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndLectureMode - received while change lecturer");
      break;
    }
    case eLmAction_LecturerConnect_CascadeLecturer:{
       	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndLectureMode - received while start cascade lecturer");
       	break;
    }
    case eLmAction_LecturerDisonnect_CascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndLectureMode - received while end cascade lecturer");
		break;
	}
    case eLmAction_ChangeLecturerToCascadeLecturer:{
        PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndLectureMode - received while end change lecturer to cascade lecturer");
        break;
    }
    case eLmAction_ChangeLecturerFromCascadeLecturer:{
     	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndLectureMode - received while change lecturer from  cascade lecturer");
     	break;
     }
     case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
       	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndLectureMode - received while change lecturer to and from cascade lecturer");
       	break;
     }
    default:{
      break;
    }
  }

}
//=======================================================================================================================================//
void CCopLectureModeCntl::ChangeLecturer(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams)
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturer");
  switch(m_currentAction){
    case eLmAction_None:{
      m_currentAction = eLmAction_ChangeLecturer;
      m_currentParams = videoBridgeLectureModeParams;
      break;
    }
    case eLmAction_LecturerDisonnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturer - received while disconnecting - do nothing");
      break;
    }
    case eLmAction_LecturerConnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturer - received while connecting");
      break;
    }
    case eLmAction_ChangeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturer - received while change lecturer");
      break;
    }
    case eLmAction_LecturerConnect_CascadeLecturer:{
        	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturer - received while start cascade lecturer");
        	break;
        }
    case eLmAction_LecturerDisonnect_CascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturer - received while end cascade lecturer");
		break;
	}
    case eLmAction_ChangeLecturerToCascadeLecturer:{
            PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturer - received while end change lecturer to cascade lecturer");
            break;
        }
    case eLmAction_ChangeLecturerFromCascadeLecturer:{
            		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturer - received while change lecturer from  cascade lecturer");
            		break;
        }
    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
        	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturer - received while change lecturer to and from cascade lecturer");
           	break;
    }
    default:{
      break;
    }
  }
}
//=======================================================================================================================================//
void CCopLectureModeCntl::StartCascadeLecturerMode(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams)
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartCascadeLecturerMode");
  switch(m_currentAction){
    case eLmAction_None:{
      m_currentAction = eLmAction_LecturerConnect_CascadeLecturer;
      m_currentParams = videoBridgeLectureModeParams;
      break;
    }
    case eLmAction_LecturerDisonnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartCascadeLecturerMode - received while disconnecting - do nothing");
      break;
    }
    case eLmAction_LecturerConnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartCascadeLecturerMode - received while connecting");
      break;
    }
    case eLmAction_ChangeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartCascadeLecturerMode - received while change lecturer");
      break;
    }

    case eLmAction_LecturerConnect_CascadeLecturer:{
    	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartCascadeLecturerMode - received while start cascade lecturer");
    	break;
    }
    case eLmAction_LecturerDisonnect_CascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartCascadeLecturerMode - received while end cascade lecturer");
		break;
	}
    case eLmAction_ChangeLecturerToCascadeLecturer:{
         PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartCascadeLecturerMode - received while end change lecturer to cascade lecturer");
         break;
    }
    case eLmAction_ChangeLecturerFromCascadeLecturer:{
        		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartCascadeLecturerMode - received while change lecturer from  cascade lecturer");
        		break;
    }
    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
       	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartCascadeLecturerMode - received while change lecturer to and from cascade lecturer");
       	break;
    }

    default:{
      break;
    }
  }
}

//=======================================================================================================================================//

void CCopLectureModeCntl::EndCascadeLecturerMode(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams)
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndCascadeLecturerMode");
  switch(m_currentAction){
    case eLmAction_None:{
      m_currentAction = eLmAction_LecturerDisonnect_CascadeLecturer;
      const char* currentLecturerName = m_currentParams.GetLecturerName();
      SetPrevCascadeAsLecturerName(currentLecturerName);
      m_currentParams = videoBridgeLectureModeParams;
      break;
    }
    case eLmAction_LecturerDisonnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndCascadeLecturerMode - received while disconnecting");
      break;
    }
    case eLmAction_LecturerConnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndCascadeLecturerMode - received while lecturer connect");
      break;
    }
    case eLmAction_ChangeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndCascadeLecturerMode - received while change lecturer");
      break;
    }
    case eLmAction_LecturerConnect_CascadeLecturer:{
       	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndCascadeLecturerMode - received while start cascade lecturer");
       	break;
    }
    case eLmAction_LecturerDisonnect_CascadeLecturer:{
       	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndCascadeLecturerMode - received while end cascade lecturer");
      	break;
    }
    case eLmAction_ChangeLecturerToCascadeLecturer:{
         PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndCascadeLecturerMode - received while end change lecturer to cascade lecturer");
         break;
    }
    case eLmAction_ChangeLecturerFromCascadeLecturer:{
    		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndCascadeLecturerMode - received while change lecturer from  cascade lecturer");
    		break;
    }
    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
    	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndCascadeLecturerMode - received while change lecturer to and from cascade lecturer");
      	break;
    }

    default:{
      break;
    }
  }

}

//=======================================================================================================================================//

LectureModeAction CCopLectureModeCntl::GetCurrentLectureModeAction()const
{
  return m_currentAction;
}
//=======================================================================================================================================//
LectureModeAction CCopLectureModeCntl::GetPendingLectureModeAction()const
{
  return m_pendingAction;
}
//=======================================================================================================================================//
LectureModeAction CCopLectureModeCntl::GetLectureModeAction(CVideoBridgeLectureModeParams& rNewLectureModeParams)const
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction");
  LectureModeAction ret_val = eLmAction_None;


  eLectureModeType currentLectureModeType = m_currentParams.GetLectureModeType();
  eLectureModeType newLectureModeType = rNewLectureModeParams.GetLectureModeType();

  const char* currentLecturerName = m_currentParams.GetLecturerName();
  const char* newLecturerName = rNewLectureModeParams.GetLecturerName();

  if(currentLectureModeType == newLectureModeType){

    switch(newLectureModeType){
    case eLectureModeRegular:{
	if(currentLecturerName[0]=='\0' && newLecturerName[0]!='\0'){
	  // lecture name set
	  PTRACE2(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction: Lecturer Connect, name - ",newLecturerName);
	  ret_val = eLmAction_LecturerConnect;
	}else if(currentLecturerName[0]!='\0' && newLecturerName[0]=='\0'){
	  // lecture name to none
	  PTRACE2(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction: Lecturer Disonnect, name - ",currentLecturerName);
	  ret_val = eLmAction_LecturerDisonnect;
	}else{
	  if(strncmp(currentLecturerName,newLecturerName,H243_NAME_LEN) != 0){
	    PTRACE2(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction: Lecturer Change, new lecturer name - ",newLecturerName);
	    // lecture name changed
	    ret_val = eLmAction_ChangeLecturer;
	  }else{
	    PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - no change");
	  }
	}
	break;
      }
    case eLectureModeNone:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - eLectureModeNone");
    }
    case eLectureModePresentation:
    default:{
	PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - illegal LectureModeType = ",(DWORD)newLectureModeType);
	break;
      }
    }
  }else{
    // new m_lectureModeType
    if(currentLectureModeType == eLectureModeNone && newLectureModeType != eLectureModeNone && newLecturerName[0]!='\0'){
	PTRACE2(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction: Lecturer type set, new lecturer name - ",newLecturerName);
	ret_val = eLmAction_LecturerConnect;

    }else if(currentLectureModeType != eLectureModeNone && newLectureModeType == eLectureModeNone){
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction: Lecturer type set to None, end lecture mode");
      ret_val = eLmAction_LecturerDisonnect;
    }
  }

  CSmallString sstr;
  switch(ret_val){
  case eLmAction_None:
    sstr << "eLmAction_None";
    break;
  case eLmAction_LecturerConnect:
    sstr << "eLmAction_LecturerConnect";
    break;
  case eLmAction_LecturerDisonnect:
    sstr << "eLmAction_LecturerDisonnect";
    break;
  case eLmAction_ChangeLecturer:
    sstr << "eLmAction_ChangeLecturer";
    break;
  default:
    sstr << "unknwn LmAction";
    break;
  }
  PTRACE2(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction: action = ",sstr.GetString());
  return ret_val;
}
//=======================================================================================================================================//
LectureModeAction CCopLectureModeCntl::GetLectureModeActionNew(CVideoBridgeLectureModeParams& rNewLectureModeParams)const
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction");
  LectureModeAction ret_val = eLmAction_None;

  eLectureModeType currentLectureModeType = m_currentParams.GetLectureModeType();
  eLectureModeType newLectureModeType = rNewLectureModeParams.GetLectureModeType();

  const char* newLecturerName = rNewLectureModeParams.GetLecturerName();
  const char* currentLecturerName = m_currentParams.GetLecturerName();

  switch(newLectureModeType)
  {
  case eLectureModeRegular:
  {
	  switch(currentLectureModeType)
	  {
	  //old_type: eLectureModeRegular ; new_type: eLectureModeRegular
	  case eLectureModeRegular:
	  {
		  if (rNewLectureModeParams.IsAudioActivatedLectureMode())
		  {
			  // new mode: audio activate --> change to audio activate
			  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - new mode: audio activate -->change to audio activated");
			  ret_val = eLmAction_ChangeLecturerToAudioActivated;
		  }
		  else if (m_currentParams.IsAudioActivatedLectureMode() == 0)
		  {
			  //old_mode: fixed , new_mode: fixed
			  if(strncmp(currentLecturerName,newLecturerName,H243_NAME_LEN)==0)
			  {
				  //  old_name: name1 ; new name name2
				  // names are equal
				  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - no change (1)");
				  ret_val = eLmAction_None;
			  }
			  else
			  {
				  // name changed
				  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - new lecturer forced");
				  ret_val = eLmAction_ChangeLecturer;
			  }
		  }
		  else
		  {
			  //old_mode: audio activated , new_mode: fixed
			  if(strncmp(currentLecturerName,newLecturerName,H243_NAME_LEN)==0)
			  {
				  //  old_name: name1 ; new name name2
				  // names are equal
				  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - audio activate mode changed but names are equal --> stop audio activate");
				  ret_val = eLmAction_SameLecturerForcedFromAudioActivated;
			  }
			  else
			  {
				  // name changed
				  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - audio activate mode changed and names changed --> change from audio activate");
				  ret_val = eLmAction_ChangeLecturerFromAudioActivated;
			  }
		  }
		  break;
	  }
	  case eLectureModeNone:
	  {
		  if(newLecturerName[0]=='\0')
		  {
			  // old_type: eLectureModeNone ; new_type: eLectureModeRegular ; new name: empty
			  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - change from eLectureModeNone to eLectureModeRegular no name set: start audio activated");
			  ret_val = eLmAction_LecturerStartAudioActivated;
		  }
		  else
		  {
			  // old_type: eLectureModeNone ; new_type: eLectureModeRegular ; new name: name1
			  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - change from eLectureModeNone to eLectureModeRegular with name set: start lecturer");
			  ret_val = eLmAction_LecturerConnect;
		  }
		  break;
	  }


	  case eLectureModePresentation:
	  default:{
		  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction: newLectureModeType = eLectureModePresentation - illegal do nothing");
		  ret_val = eLmAction_None;
		  break;
	  }

	  break;
	  }// switch(currentLectureModeType)

	  break;
  }

  // new_type: eLectureModeNone
  case eLectureModeNone:{

	  switch(currentLectureModeType){
	  case eLectureModeRegular:{
		  if(currentLecturerName[0]=='\0'){
			  // old_type: currentLectureModeType ; new_type: currentLectureModeType
			  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - change from eLectureModeRegular to eLectureModeNone: stop audio activated");
			  ret_val = eLmAction_LecturerStopAudioActivated;

		  }else{
			  // old_type: currentLectureModeType ; new_type: currentLectureModeType
			  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - change from eLectureModeRegular to eLectureModeNone: stop lecture mode");
			  ret_val = eLmAction_LecturerDisonnect;
		  }

		  break;
	  }
	  case eLectureModeNone:{
		  // old_type: eLectureModeNone ; new_type: eLectureModeNone
		  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - no change (3)");
		  ret_val = eLmAction_None;

		  break;
	  }
	  case eLectureModePresentation:
	  default:{
		  PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - illegal LectureModeType = ",(DWORD)newLectureModeType);
		  ret_val = eLmAction_None;
		  break;
	  }
	  }

	  break;
  }

  // new_type: eLectureModePresentation
  case eLectureModePresentation:
  default:{
	  PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction - illegal LectureModeType = ",(DWORD)newLectureModeType);
	  ret_val = eLmAction_None;
	  break;
  }

  }

  CSmallString sstr;
  DumpLectureModeAction(sstr,ret_val);
  PTRACE2(eLevelInfoNormal,"CCopLectureModeCntl::GetLectureModeAction: action = ",sstr.GetString());

  return ret_val;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndStartLectureMode()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndStartLectureMode");
  EndLectureModeAction();
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndEndLectureMode()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndEndLectureMode");
  EndLectureModeAction();
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndChangeLecturer()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndChangeLecturer");
  EndLectureModeAction();
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndLectureModeAction()
{
  m_currentAction = eLmAction_None;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndStartCascadeLecturerMode()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndStartCascadeLecturerMode");

  EndLectureModeAction();
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndEndCascadeLecturerMode()
{
	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndEndCascadeLecturerMode");
	strcpy(m_prevCascadeAsLecturerName,"");
	m_isDisconnectedCascadeLecturerParty = NO;
	EndLectureModeAction();
}
//=======================================================================================================================================//
  // bridge actions
//=======================================================================================================================================//
void CCopLectureModeCntl::StartOpenCodecs()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartOpenCodecs");
  m_openCodecs = 1;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndOpenCodecs()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndOpenCodecs");
  m_openCodecs = 0;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::StartCloseCodecs()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartCloseCodecs");
  m_closeCodecs = 1;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndCloseCodecs()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndCloseCodecs");
  m_closeCodecs = 0;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::StartMovePartiesToVsw()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartMovePartiesToVsw");
  m_movePartiesToVsw = 1;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndMovePartiesToVsw()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndMovePartiesToVsw");
  m_movePartiesToVsw = 0;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::StartMovePartiesFromVsw()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::StartMovePartiesFromVsw");
  m_movePartiesFromVsw = 1;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndMovePartiesFromVsw()
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndMovePartiesFromVsw");
  m_movePartiesFromVsw = 0;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetOpenCodecsAction()const
{
  return m_openCodecs;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetCloseCodecsAction()const
{
  return m_closeCodecs;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetMovePartiesToVswAction()const
{
  return m_movePartiesToVsw;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetMovePartiesFromVswAction()const
{
  return m_movePartiesFromVsw;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetMoveVswPartiesAction()const
{
  BYTE moveVswPartiesAction = 0;
  if(m_movePartiesToVsw || m_movePartiesFromVsw){
    moveVswPartiesAction = 1;
  }
  return moveVswPartiesAction;
}
//=======================================================================================================================================//
CVideoBridgeLectureModeParams* CCopLectureModeCntl::GetCurrentParams()
{
  return &m_currentParams;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::IsAudioActivatedLectureMode()const
{
  return m_currentParams.IsAudioActivatedLectureMode();
}
//=======================================================================================================================================//
void CCopLectureModeCntl::SetAudioActivatedLectureMode(BYTE isAudioActive)
{
	m_currentParams.SetAudioActivatedLectureMode(isAudioActive);
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::IsInAction()const
{
  BYTE inAction = NO;
  if(m_openCodecs || m_closeCodecs || m_movePartiesToVsw || m_movePartiesFromVsw || m_changeModeToLecturer||m_updateCascadeLinkAsLecturer||m_updateCascadeLinkAsNotLecturer){
    inAction = YES;
  }
  return inAction;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::Dump()const
{
  CMedString mstr;

  mstr << "m_isLectureModeActive = " << (DWORD)m_isLectureModeActive << "\n";
  mstr << "m_isAudioActivate     = " << (DWORD)m_isAudioActivate << "\n";

  mstr << "m_currentAction = ";
  DumpLectureModeAction(mstr,m_currentAction);
  mstr << "\n";

  mstr << "m_currentParams: \n";
  m_currentParams.Dump(mstr);
  mstr << "\n";

  mstr << "m_openCodecs           = " << (DWORD)m_openCodecs << "\n";
  mstr << "m_closeCodecs          = " << (DWORD)m_closeCodecs << "\n";
  mstr << "m_movePartiesToVsw     = " << (DWORD)m_movePartiesToVsw << "\n";
  mstr << "m_movePartiesFromVsw   = " << (DWORD)m_movePartiesFromVsw << "\n";
  mstr << "m_changeModeToLecturer = " << (DWORD)m_changeModeToLecturer << "\n";
  mstr << "m_updateCascadeLinkAsLecturer= " << (DWORD)m_updateCascadeLinkAsLecturer << "\n";
  mstr << "m_updateCascadeLinkAsNotLecturer=" << (DWORD)m_updateCascadeLinkAsNotLecturer << "\n";

  mstr << "m_pendingAction = ";
  DumpLectureModeAction(mstr,m_pendingAction);
  mstr << "\n";

  mstr << "m_newParams: \n";
  m_newParams.Dump(mstr);
  mstr << "\n";

  mstr << "m_waiting_to_end_change_layout = " << (DWORD)m_waiting_to_end_change_layout << "\n";
  mstr << "m_waiting_to_lecturer_decoder_sync = " << (DWORD)m_waiting_to_lecturer_decoder_sync << "\n";



  PTRACE(eLevelInfoNormal,mstr.GetString());
}
//=======================================================================================================================================//
void CCopLectureModeCntl::DumpLectureModeAction(CObjString& sstr,LectureModeAction action)const
{
  switch(action){
  case eLmAction_None:
    sstr << "eLmAction_None";
    break;
  case eLmAction_LecturerConnect:
    sstr << "eLmAction_LecturerConnect";
    break;
  case eLmAction_LecturerDisonnect:
    sstr << "eLmAction_LecturerDisonnect";
    break;
  case eLmAction_ChangeLecturer:
    sstr << "eLmAction_ChangeLecturer";
    break;
  case eLmAction_LecturerStartAudioActivated:
    sstr << "eLmAction_LecturerStartAudioActivated";
    break;
  case eLmAction_ChangeLecturerToAudioActivated:
    sstr << "eLmAction_ChangeLecturerToAudioActivated";
    break;
  case eLmAction_ChangeLecturerFromAudioActivated:
    sstr << "eLmAction_ChangeLecturerFromAudioActivated";
    break;
  case eLmAction_LecturerStopAudioActivated:
    sstr << "eLmAction_LecturerStopAudioActivated";
    break;
  case eLmAction_LecturerConnect_CascadeLecturer:
	  sstr << "eLmAction_LecturerConnect_CascadeLecturer";
	  break;
  case eLmAction_LecturerDisonnect_CascadeLecturer:
	  sstr <<  "eLmAction_LecturerDisonnect_CascadeLecturer";
	  break;
  case  eLmAction_ChangeLecturerToCascadeLecturer:
	  sstr << "eLmAction_ChangeLecturerToCascadeLecturer";
	  break;
  case eLmAction_ChangeLecturerFromCascadeLecturer:
	  sstr << "eLmAction_ChangeLecturerFromCascadeLecturer";
	  break;
  case eLmAction_ChangeLecturerToAndFromCascadeLecturer:
	  sstr << "eLmAction_ChangeLecturerToAndFromCascadeLecturer";
	  break;

  default:
    sstr << "unknwn LmAction";
    break;
  }
}
//=======================================================================================================================================//
void CCopLectureModeCntl::SetPendingAction(LectureModeAction action,CVideoBridgeLectureModeParams& rNewLectureModeParams)
{
  switch(m_pendingAction){

  case eLmAction_None:{
    PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction");
    m_pendingAction = action;
    m_newParams = rNewLectureModeParams;
    break;
  }
  case eLmAction_LecturerDisonnect:{
    PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction old m_pendingAction = eLmAction_LecturerDisonnect");

    switch(action){

    case eLmAction_None:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action = eLmAction_None => do nothing");
      break;
    }
    case eLmAction_LecturerDisonnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action = eLmAction_LecturerDisonnect => do nothing");
      break;
    }
    case eLmAction_LecturerDisonnect_CascadeLecturer:{
	  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect_CascadeLecturer => skip eLmAction_LecturerDisonnect (old pending) end set to eLmAction_LecturerDisonnect_CascadeLecturer(new pending)");
	  m_pendingAction = action;
	  m_newParams =rNewLectureModeParams;
	  break;
	}

    case eLmAction_LecturerConnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect => skip eLmAction_LecturerDisonnect (old pending) end set to eLmAction_ChangeLecturer insted of eLmAction_LecturerConnect (new pending)");
      m_pendingAction = eLmAction_ChangeLecturer;
      m_newParams =rNewLectureModeParams;
      break;
    }
    case eLmAction_LecturerConnect_CascadeLecturer:{
          PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect_CascadeLecturer => skip eLmAction_LecturerDisonnect (old pending) end set to eLmAction_ChangeLecturerToCascadeLecturer instead of eLmAction_LecturerConnect_CascadeLecturer (new pending)");
          m_pendingAction = eLmAction_ChangeLecturerToCascadeLecturer;
          m_newParams =rNewLectureModeParams;
          break;
        }
    case eLmAction_ChangeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturer => skip eLmAction_LecturerDisonnect (old pending)end set to eLmAction_ChangeLecturer (new pending)");
      m_pendingAction = action;
      m_newParams = rNewLectureModeParams;
      break;
    }

    case eLmAction_ChangeLecturerToCascadeLecturer:{
          PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToCascadeLecturer => skip eLmAction_LecturerDisonnect (old pending)end set to eLmAction_ChangeLecturerToCascadeLecturer (new pending)");
          m_pendingAction = action;
          m_newParams = rNewLectureModeParams;
          break;
        }
    case eLmAction_ChangeLecturerFromCascadeLecturer:{
		  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerFromCascadeLecturer => skip eLmAction_LecturerDisonnect (old pending)end set to eLmAction_ChangeLecturerFromCascadeLecturer (new pending)");
		  m_pendingAction = action;
		  m_newParams = rNewLectureModeParams;
		  break;
		}
    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
    	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToAndFromCascadeLecturer => skip eLmAction_LecturerDisonnect (old pending)end set to eLmAction_ChangeLecturerToAndFromCascadeLecturer (new pending)");
    	m_pendingAction = action;
    	m_newParams = rNewLectureModeParams;
    		  break;
    		}

    default:{
      PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction unknown new action = ",(DWORD) action);
      break;
    }
    }

    break;
  }
 case eLmAction_LecturerDisonnect_CascadeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction old m_pendingAction = eLmAction_LecturerDisonnect_CascadeLecturer");
      switch(action){
      case eLmAction_None:{
        PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action = eLmAction_None => do nothing");
        break;
      }
      case eLmAction_LecturerDisonnect:{
        PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action = eLmAction_LecturerDisonnect => skip eLmAction_LecturerDisonnect_CascadeLecturer(old pending) end set to eLmAction_LecturerDisonnect (new pending)");
        m_pendingAction = action;
        m_newParams =rNewLectureModeParams;
        break;
      }
      case eLmAction_LecturerDisonnect_CascadeLecturer:{
  	  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect_CascadeLecturer do nothing");
  	  break;
  	}

      case eLmAction_LecturerConnect:{
        PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect => skip eLmAction_LecturerDisonnect_CascadeLecturer (old pending) end set to eLmAction_ChangeLecturerFromCascadeLecturer instead of eLmAction_LecturerDisonnect_CascadeLecturer (new pending)");
        m_pendingAction = eLmAction_ChangeLecturerFromCascadeLecturer;
        m_newParams =rNewLectureModeParams;
        break;
      }
      case eLmAction_LecturerConnect_CascadeLecturer:{
    	  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect_CascadeLecturer => skip eLmAction_LecturerDisonnect_CascadeLecturer (old pending) end set to eLmAction_ChangeLecturerToAndFromCascadeLecturer instead of eLmAction_LecturerDisonnect_CascadeLecturer (new pending)");
    	  m_pendingAction = eLmAction_ChangeLecturerToAndFromCascadeLecturer;
          m_newParams =rNewLectureModeParams;
          break;
      }
      case eLmAction_ChangeLecturer:{
        PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturer => skip eLmAction_LecturerDisonnect_CascadeLecturer (old pending)end set to eLmAction_ChangeLecturerFromCascadeLecturer (new pending)");
        m_pendingAction = eLmAction_ChangeLecturerFromCascadeLecturer;
        m_newParams = rNewLectureModeParams;
        break;
      }

      case eLmAction_ChangeLecturerToCascadeLecturer:{
            PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToCascadeLecturer => skip eLmAction_LecturerDisonnect_CascadeLecturer (old pending)end set to eLmAction_ChangeLecturerFromCascadeLecturer (new pending)");
            m_pendingAction = eLmAction_ChangeLecturerFromCascadeLecturer;
            m_newParams = rNewLectureModeParams;
            break;
          }
      case eLmAction_ChangeLecturerFromCascadeLecturer:{
  		  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerFromCascadeLecturer => skip eLmAction_LecturerDisonnect_CascadeLecturer (old pending)end set to eLmAction_ChangeLecturerFromCascadeLecturer (new pending)");
  		  m_pendingAction = action;
  		  m_newParams = rNewLectureModeParams;
  		  break;
  		}
      case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
      	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToAndFromCascadeLecturer => skip eLmAction_LecturerDisonnect_CascadeLecturer (old pending)end set to eLmAction_ChangeLecturerToAndFromCascadeLecturer (new pending)");
      	m_pendingAction = action;
      	m_newParams = rNewLectureModeParams;
      		  break;
      		}

      default:{
        PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction unknown new action = ",(DWORD) action);
        break;
      }
      }

      break;
    }
  case eLmAction_LecturerConnect:{
    PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction old m_pendingAction = eLmAction_LecturerConnect");


    switch(action){

    case eLmAction_None:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action = eLmAction_None => do nothing");
      break;
    }
    case eLmAction_LecturerDisonnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect => skip eLmAction_LecturerConnect (old pending) end set to eLmAction_None insted of eLmAction_LecturerConnect (new pending)");
      m_pendingAction = eLmAction_None;
      break;
    }

    case eLmAction_LecturerDisonnect_CascadeLecturer:{
    	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect_CascadeLecturer => skip eLmAction_LecturerConnect (old pending) end set to eLmAction_None insted of eLmAction_LecturerConnect (new pending)");
        m_pendingAction = eLmAction_None;
        break;
    }
    case eLmAction_LecturerConnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect => do nothing");
      break;
    }
    case eLmAction_LecturerConnect_CascadeLecturer:{
    	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect_CascadeLecturer => skip eLmAction_LecturerConnect (old pending)end set to eLmAction_LecturerConnect_CascadeLecturer (new pending)");
       	m_pendingAction = action;
       	m_newParams = rNewLectureModeParams;
       	break;
    }

    case eLmAction_ChangeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturer => set eLmAction_LecturerConnect with new lecturer params");
      m_newParams = rNewLectureModeParams;
      break;
    }
    case eLmAction_ChangeLecturerToCascadeLecturer:{
	  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToCascadeLecturer => set eLmAction_LecturerConnect_CascadeLecturer with new lecturer params");
	  m_pendingAction = eLmAction_LecturerConnect_CascadeLecturer;
	  m_newParams = rNewLectureModeParams;
	  break;
	}
    case eLmAction_ChangeLecturerFromCascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerFromCascadeLecturer => set eLmAction_LecturerConnect with new lecturer params");
		m_newParams = rNewLectureModeParams;
		break;
    }
    case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
		  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToAndFromCascadeLecturer => set eLmAction_LecturerConnect_CascadeLecturer with new lecturer params");
		  m_pendingAction = eLmAction_LecturerConnect_CascadeLecturer;
		  m_newParams = rNewLectureModeParams;
		  break;
		}
    default:{
      PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction unknown new action = ",(DWORD) action);
      break;
    }
    }


    break;
  }
  ///
  case eLmAction_LecturerConnect_CascadeLecturer:{
     PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction old m_pendingAction = eLmAction_LecturerConnect_CascadeLecturer");


     switch(action){

     case eLmAction_None:{
       PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action = eLmAction_None => do nothing");
       break;
     }
     case eLmAction_LecturerDisonnect:{
       PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect => skip eLmAction_LecturerConnect_CascadeLecturer (old pending) end set to eLmAction_None instead of eLmAction_LecturerConnect_CascadeLecturer (new pending)");
       m_pendingAction = eLmAction_None;
       break;
     }

     case eLmAction_LecturerDisonnect_CascadeLecturer:{
     	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect_CascadeLecturer => skip eLmAction_LecturerConnect_CascadeLecturer (old pending) end set to eLmAction_None instead of eLmAction_LecturerConnect (new pending)");
         m_pendingAction = eLmAction_None;
         break;
     }
     case eLmAction_LecturerConnect:{
       PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect => skip eLmAction_LecturerConnect_CascadeLecturer (old pending)end set to eLmAction_LecturerConnect (new pending)");
       m_pendingAction = action;
       m_newParams = rNewLectureModeParams;
       break;
     }
     case eLmAction_LecturerConnect_CascadeLecturer:{
    	 PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect_CascadeLecturer => update params");
         m_newParams = rNewLectureModeParams;
        break;
     }

     case eLmAction_ChangeLecturer:{
       PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturer => set eLmAction_LecturerConnect with new lecturer params");
       m_newParams = rNewLectureModeParams;
       break;
     }
     case eLmAction_ChangeLecturerToCascadeLecturer:{
 	  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToCascadeLecturer => set eLmAction_LecturerConnect_CascadeLecturer with new lecturer params");
 	  m_pendingAction = eLmAction_LecturerConnect_CascadeLecturer;
 	  m_newParams = rNewLectureModeParams;
 	  break;
 	}
     case eLmAction_ChangeLecturerFromCascadeLecturer:{
 		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerFromCascadeLecturer => set eLmAction_LecturerConnect with new lecturer params");
 		action = eLmAction_LecturerConnect;
 		m_newParams = rNewLectureModeParams;
 		break;
     }
     case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
 		  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToAndFromCascadeLecturer => set eLmAction_LecturerConnect_CascadeLecturer with new lecturer params");
 		  m_pendingAction = eLmAction_LecturerConnect_CascadeLecturer;
 		  m_newParams = rNewLectureModeParams;
 		  break;
 		}
     default:{
       PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction unknown new action = ",(DWORD) action);
       break;
     }
     }


     break;
   }
 case eLmAction_ChangeLecturer:{
    PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction old m_pendingAction = eLmAction_ChangeLecturer");

    switch(action){

    case eLmAction_None:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action = eLmAction_None => do nothing");
      break;
    }
    case eLmAction_LecturerDisonnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect => skip eLmAction_ChangeLecturer (old pending) and set instead eLmAction_LecturerDisconnect (new pending)");
      m_pendingAction = action;
      m_newParams = rNewLectureModeParams;
      break;
    }
    case eLmAction_LecturerDisonnect_CascadeLecturer:{
       	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect_CascadeLecturer => skip eLmAction_ChangeLecturer (old pending) and set instead  eLmAction_LecturerDisonnect_CascadeLecturer(new pending)");
        m_pendingAction = action;
        m_newParams = rNewLectureModeParams;
        break;
    }

    case eLmAction_LecturerConnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect => do nothing");
      break;
    }
    case eLmAction_LecturerConnect_CascadeLecturer:{
       	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect_CascadeLecturer => => do nothing");
       	break;
    }
    case eLmAction_ChangeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturer => set eLmAction_ChangeLecture with new params");
      m_newParams = rNewLectureModeParams;
      break;
    }

    case eLmAction_ChangeLecturerToCascadeLecturer:{
	  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToCascadeLecturer => skip eLmAction_ChangeLecturer (old pending) and set instead eLmAction_ChangeLecturerToCascadeLecturer");
	  m_pendingAction = action;
	  m_newParams = rNewLectureModeParams;
	  break;
	}
	case eLmAction_ChangeLecturerFromCascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerFromCascadeLecturer => skip eLmAction_ChangeLecturer (old pending) and set instead eLmAction_ChangeLecturerFromCascadeLecturer");
		m_pendingAction = action;
		m_newParams = rNewLectureModeParams;
		break;
	}
	case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToAndFromCascadeLecturer => skip eLmAction_ChangeLecturer (old pending) and set instead eLmAction_ChangeLecturerToAndFromCascadeLecturer");
		m_pendingAction = action;
		m_newParams = rNewLectureModeParams;
		break;
	}

    default:{
      PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction unknown new action = ",(DWORD) action);
      break;
    }
    }


    break;
  }


  case eLmAction_ChangeLecturerToCascadeLecturer:{
     PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction old m_pendingAction = eLmAction_ChangeLecturerToCascadeLecturer");

     switch(action){

     case eLmAction_None:{
       PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action = eLmAction_None => do nothing");
       break;
     }
     case eLmAction_LecturerDisonnect:{
       PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect => skip eLmAction_ChangeLecturerToCascadeLecturer (old pending) and set instead eLmAction_LecturerDisconnect (new pending)");
       m_pendingAction = action;
       m_newParams = rNewLectureModeParams;
       break;
     }
     case eLmAction_LecturerDisonnect_CascadeLecturer:{
        PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect_CascadeLecturer => skip eLmAction_ChangeLecturerToCascadeLecturer (old pending) and set instead  eLmAction_LecturerDisonnect_CascadeLecturer(new pending)");
        m_pendingAction = action;
        m_newParams = rNewLectureModeParams;
        break;
     }

     case eLmAction_LecturerConnect:{
       PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect => do nothing, WE SHOULDN GET HERE...");
       PASSERT(101);
       break;
     }
     case eLmAction_LecturerConnect_CascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect_CascadeLecturer => => do nothing");
		break;
     }
     case eLmAction_ChangeLecturer:{
       PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturer => set eLmAction_ChangeLecture with new params");
       m_pendingAction = action;
       m_newParams = rNewLectureModeParams;
       break;
     }

     case eLmAction_ChangeLecturerToCascadeLecturer:{
    	 PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToCascadeLecturer => eLmAction_ChangeLecturerToCascadeLecturer update paramsr");
    	 m_newParams = rNewLectureModeParams;
 	  break;
 	}
 	case eLmAction_ChangeLecturerFromCascadeLecturer:{
 		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerFromCascadeLecturer => skip eLmAction_ChangeLecturerToCascadeLecturer (old pending) and set instead eLmAction_ChangeLecturer");
 		m_pendingAction = eLmAction_ChangeLecturer;
 		m_newParams = rNewLectureModeParams;
 		break;
 	}
 	case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
 		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToAndFromCascadeLecturer => update params of eLmAction_ChangeLecturerToCascadeLecturer (old pending)");
 		m_newParams = rNewLectureModeParams;
 		break;
 	}

     default:{
       PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction unknown new action = ",(DWORD) action);
       break;
     }
     }


     break;
   }


  case eLmAction_ChangeLecturerFromCascadeLecturer:{
       PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction old m_pendingAction = eLmAction_ChangeLecturerFromCascadeLecturer");

       switch(action){

       case eLmAction_None:{
         PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action = eLmAction_None => do nothing");
         break;
       }
       case eLmAction_LecturerDisonnect:{
         PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect => skip eLmAction_ChangeLecturerFromCascadeLecturer (old pending) and set instead eLmAction_LecturerDisonnect_CascadeLecturer (new pending)");
         m_pendingAction = eLmAction_LecturerDisonnect_CascadeLecturer;
         m_newParams = rNewLectureModeParams;
         break;
       }
       case eLmAction_LecturerDisonnect_CascadeLecturer:{
          PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerDisonnect_CascadeLecturer => skip eLmAction_ChangeLecturerFromCascadeLecturer (old pending) and set instead  eLmAction_LecturerDisonnect_CascadeLecturer(new pending)");
          m_pendingAction = action;
          m_newParams = rNewLectureModeParams;
          break;
       }

       case eLmAction_LecturerConnect:{
         PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect => do nothing, WE SHOULDN GET HERE...");
         PASSERT(101);
         break;
       }
       case eLmAction_LecturerConnect_CascadeLecturer:{
  		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_LecturerConnect_CascadeLecturer => => do nothing");
  		break;
       }
       case eLmAction_ChangeLecturer:{
         PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturer => set eLmAction_ChangeLecturerFromCascadeLecturer with new params");
         m_newParams = rNewLectureModeParams;
         break;
       }

       case eLmAction_ChangeLecturerToCascadeLecturer:{
      	 PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToCascadeLecturer => skip eLmAction_ChangeLecturerFromCascadeLecturer(old) and set to eLmAction_ChangeLecturerToAndFromCascadeLecturer with updated params");
      	 m_pendingAction = eLmAction_ChangeLecturerToAndFromCascadeLecturer;
      	 m_newParams = rNewLectureModeParams;
   	  break;
   	}
   	case eLmAction_ChangeLecturerFromCascadeLecturer:{
   		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerFromCascadeLecturer => skip the current eLmAction_ChangeLecturerFromCascadeLecturer pending action and set pending as eLmAction_ChangeLecturer");
   		m_pendingAction = eLmAction_ChangeLecturer;
   		m_newParams = rNewLectureModeParams;
   		break;
   	}
   	case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
   		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction new action=eLmAction_ChangeLecturerToAndFromCascadeLecturer => update params of eLmAction_ChangeLecturerFromCascadeLecturer");
   		m_newParams = rNewLectureModeParams;
   		break;
   	}

       default:{
         PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction unknown new action = ",(DWORD) action);
         break;
       }
       }


       break;
     }
  default:{
    PTRACE2INT(eLevelInfoNormal,"CCopLectureModeCntl::SetPendingAction unknown old action = ",(DWORD) action);
    m_pendingAction = action;
    m_newParams = rNewLectureModeParams;
    break;
  }
  }
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::IsPendingAction()const
{
  BYTE isPendingAction = NO;
  if(m_pendingAction != eLmAction_None){
    isPendingAction = YES;
  }
  return isPendingAction;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::UpdateCurrentParams(CVideoBridgeLectureModeParams& rNewLectureModeParams)
{
  // update current current
  if (m_currentAction != eLmAction_None)
  {
	  CMedString cstr;
	  cstr << "CCopLectureModeCntl::UpdateCurrentParams can not update params while other action is in progress (current action: ";
	  DumpLectureModeAction(cstr,m_currentAction);
	  cstr << ")";
	  PTRACE(eLevelInfoNormal,cstr.GetString());
  }

  m_currentParams = rNewLectureModeParams;

}
//=======================================================================================================================================//
void CCopLectureModeCntl::UpdateCurrentActionFromPending()
{
  // update current current
  m_currentAction = m_pendingAction;
  m_currentParams = m_newParams;

  // reset pending
  m_pendingAction = eLmAction_None;
}
//=======================================================================================================================================//
CVideoBridgeLectureModeParams* CCopLectureModeCntl::GetPendingParams()
{
  return &m_newParams;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::StartChangeModeToLecturer()
{
  m_changeModeToLecturer = 1;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndChangeModeToLecturer()
{
  m_changeModeToLecturer = 0;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetChangeModeToLecturer()const
{
  return m_changeModeToLecturer;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetWaitingToEndChangeLayout()const
{
  return m_waiting_to_end_change_layout;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::SetWaitingToEndChangeLayout(BYTE waitingToEndChangeLayout)
{
  m_waiting_to_end_change_layout = waitingToEndChangeLayout;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::SetWaitingToEndChangeLayoutParams(CLectureModeParams& rNewResrvationLectureMode)
{
  *m_pWaitingToEndChangeLayoutParams = rNewResrvationLectureMode;
}
//=======================================================================================================================================//
CLectureModeParams* CCopLectureModeCntl::GetWaitingToEndChangeLayoutParams()const
{
  return m_pWaitingToEndChangeLayoutParams;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::SetWaitingToLecturerDecoderSync(BYTE waitingToLecturerDecoderSync)
{
  m_waiting_to_lecturer_decoder_sync = waitingToLecturerDecoderSync;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetWaitingToLecturerDecoderSync()const
{
  return m_waiting_to_lecturer_decoder_sync;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::SetSendChangeLayoutToEveryOneAfterLecturerDecoderSync(BYTE sendChangeLayoutToEveryOne)
{
  m_send_change_layout_to_everyone_after_lecturer_decoder_sync = sendChangeLayoutToEveryOne;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetSendChangeLayoutToEveryOneAfterLecturerDecoderSync() const
{
  return m_send_change_layout_to_everyone_after_lecturer_decoder_sync;
}
//=======================================================================================================================================/
void CCopLectureModeCntl::StartUpdateCascadeLinkAsLecturer()
{
  m_updateCascadeLinkAsLecturer = 1;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndUpdateCascadeLinkAsLecturer()
{
	m_updateCascadeLinkAsLecturer = 0;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetUpdateCascadeLinkAsLecturer()const
{
  return m_updateCascadeLinkAsLecturer;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::StartCascadeLinkAsLecturerPendingMode()
{
	m_startCascadeLinkAsLecturerPendingMode = 1;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndStartCascadeLinkAsLecturerPendingMode()
{
	m_startCascadeLinkAsLecturerPendingMode = 0;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetStartCascadeLinkAsLecturerPendingMode()const
{
  return m_startCascadeLinkAsLecturerPendingMode;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::StartUpdateCascadeLinkAsNotLecturer()
{
	m_updateCascadeLinkAsNotLecturer = 1;
}
//=======================================================================================================================================//
void CCopLectureModeCntl::EndUpdateCascadeLinkAsNotLecturer()
{
	m_updateCascadeLinkAsNotLecturer = 0;
}
//=======================================================================================================================================//
BYTE CCopLectureModeCntl::GetUpdateCascadeLinkAsNotLecturer()const
{
  return m_updateCascadeLinkAsNotLecturer;
}
//=======================================================================================================================================//
const char* CCopLectureModeCntl::GetPrevCascadeAsLecturerName() const
{
	return m_prevCascadeAsLecturerName;
}
//=======================================================================================================================================//

void CCopLectureModeCntl::SetPrevCascadeAsLecturerName(const char* pLecturerName)
{
	strncpy(m_prevCascadeAsLecturerName,pLecturerName,H243_NAME_LEN - 1);
	m_prevCascadeAsLecturerName[H243_NAME_LEN-1]='\0';
}
//=======================================================================================================================================//
void CCopLectureModeCntl::ChangeLecturerToCascadeLink(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams)
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToCascadeLink");
  switch(m_currentAction){
    case eLmAction_None:{
      m_currentAction = eLmAction_ChangeLecturerToCascadeLecturer;
      m_currentParams = videoBridgeLectureModeParams;
      break;
    }
    case eLmAction_LecturerDisonnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToCascadeLink - received while disconnecting - do nothing");
      break;
    }
    case eLmAction_LecturerConnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToCascadeLink - received while connecting");
      break;
    }
    case eLmAction_ChangeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToCascadeLink - received while change lecturer");
      break;
    }
    case eLmAction_LecturerConnect_CascadeLecturer:{
        	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToCascadeLink - received while start cascade lecturer");
        	break;
        }
    case eLmAction_LecturerDisonnect_CascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToCascadeLink - received while end cascade lecturer");
		break;
	}
    case eLmAction_ChangeLecturerToCascadeLecturer:{
    	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToCascadeLink - received while changelecturer to  cascade lecturer");
    	break;
    }
    default:{
      break;
    }
  }
}

//=======================================================================================================================================//
void CCopLectureModeCntl::EndChangeLecturerToCascadeLecturer()
{
	 PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndChangeLecturerToCascadeLecturer");
	 EndLectureModeAction();
}
//=======================================================================================================================================//
void CCopLectureModeCntl::ChangeLecturerFromCascadeLink(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams)
{
  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerFromCascadeLink");
  switch(m_currentAction){
    case eLmAction_None:{
      m_currentAction = eLmAction_ChangeLecturerFromCascadeLecturer;
      const char* currentLecturerName = m_currentParams.GetLecturerName();
      SetPrevCascadeAsLecturerName(currentLecturerName);
      m_currentParams = videoBridgeLectureModeParams;
      break;
    }
    case eLmAction_LecturerDisonnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerFromCascadeLink - received while disconnecting - do nothing");
      break;
    }
    case eLmAction_LecturerConnect:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerFromCascadeLink - received while connecting");
      break;
    }
    case eLmAction_ChangeLecturer:{
      PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerFromCascadeLink - received while change lecturer");
      break;
    }
    case eLmAction_LecturerConnect_CascadeLecturer:{
        	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerFromCascadeLink - received while start cascade lecturer");
        	break;
        }
    case eLmAction_LecturerDisonnect_CascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerFromCascadeLink - received while end cascade lecturer");
		break;
	}
    case eLmAction_ChangeLecturerToCascadeLecturer:{
    	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerFromCascadeLink - received while change lecturer to  cascade lecturer");
    	break;
    }

   case eLmAction_ChangeLecturerFromCascadeLecturer:{
   		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerFromCascadeLink - received while change lecturer from  cascade lecturer");
   		break;
   }
   case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
   		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerFromCascadeLink - received while change lecturer to and from cascade lecturer");
   	break;
   }
    default:{
      break;
    }
  }
}


//=======================================================================================================================================//
void CCopLectureModeCntl::EndChangeLecturerFromCascadeLecturer()
{
	 PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndChangeLecturerFromCascadeLecturer");
	 strcpy(m_prevCascadeAsLecturerName,"");
	 m_isDisconnectedCascadeLecturerParty = NO;
	 EndLectureModeAction();
}
//=======================================================================================================================================//
void CCopLectureModeCntl::ChangeLecturerToAndFromCascadeLink(CVideoBridgeLectureModeParams& videoBridgeLectureModeParams)
{
	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToAndFromCascadeLink");

	switch(m_currentAction){

	case eLmAction_None:{
	      m_currentAction = eLmAction_ChangeLecturerToAndFromCascadeLecturer;
	      const char* currentLecturerName = m_currentParams.GetLecturerName();
	      SetPrevCascadeAsLecturerName(currentLecturerName);
	      m_currentParams = videoBridgeLectureModeParams;
	break;
	}
	case eLmAction_LecturerDisonnect:{
	     PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToAndFromCascadeLink - received while disconnecting - do nothing");
	break;
	}
	case eLmAction_LecturerConnect:{
	  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToAndFromCascadeLink - received while connecting");
	  break;
	}
	case eLmAction_ChangeLecturer:{
	  PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToAndFromCascadeLink - received while change lecturer");
	  break;
	}
	case eLmAction_LecturerConnect_CascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToAndFromCascadeLink - received while start cascade lecturer");
		break;
	}
	case eLmAction_LecturerDisonnect_CascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToAndFromCascadeLink - received while end cascade lecturer");
		break;
	}
	case eLmAction_ChangeLecturerToCascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToAndFromCascadeLink - received while change lecturer to  cascade lecturer");
		break;
	}
	case eLmAction_ChangeLecturerFromCascadeLecturer:{
		PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToAndFromCascadeLink - received while change lecturer from  cascade lecturer");
		break;
	}
	case eLmAction_ChangeLecturerToAndFromCascadeLecturer:{
			PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::ChangeLecturerToAndFromCascadeLink - received while change lecturer to and from cascade lecturer");
			break;
		}
	default:{
	  break;
	}
  }
}
//=======================================================================================================================================//

void CCopLectureModeCntl::EndChangeLecturerToAndFromCascadeLink()
{
	PTRACE(eLevelInfoNormal,"CCopLectureModeCntl::EndChangeLecturerToAndFromCascadeLink");
	strcpy(m_prevCascadeAsLecturerName,"");
	m_isDisconnectedCascadeLecturerParty = NO;
	EndLectureModeAction();

}
//=======================================================================================================================================/
const char* CSmartLecturerState::GetStateName(LectureSmartState state)
{
  if (state == eLmSmartState_Current)
    state = m_eState;

  switch (state)
  {
    case eLmSmartState_None              : return "eLmSmartState_None";
    case eLmSmartState_SameLevel1Lecturer: return "eLmSmartState_SameLevel1Lecturer";
    case eLmSmartState_SameLevel2Lecturer: return "eLmSmartState_SameLevel2Lecturer";
    case eLmSmartState_DiffLevel1Lecturer: return "eLmSmartState_DiffLevel1Lecturer";
    case eLmSmartState_DiffLevel2Lecturer: return "eLmSmartState_DiffLevel2Lecturer";
    default                              : return "eLmSmartState_Error";
  }
}
