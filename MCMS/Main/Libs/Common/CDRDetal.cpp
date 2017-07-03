// +========================================================================+
// CDRDETAL.CPP                                                             |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       CDRDETAL.CPP                                                 |
// SUBSYSTEM:  MCMSOPER                                                     |
// PROGRAMMER: Michel                                                       |
// -------------------------------------------------------------------------|
// Who | Date       | Description                                           |
// -------------------------------------------------------------------------|
//     |            |                                                       |
// +========================================================================+
#include "CDRDetal.h"
#include "CDRDefines.h"
#include "ConfStart.h"
#include "NetChannConn.h"
#include "NetChannelDisco.h"
#include "BilParty.h"
#include "OperEvent.h"
#include "NStream.h"
#include "Macros.h"
#include "StatusesGeneral.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "CDRShort.h"
#include "ApiStatuses.h"
#include "InternalProcessStatuses.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        CCdrEvent
////////////////////////////////////////////////////////////////////////////
CCdrEvent::CCdrEvent()
{
  m_operator_name[0]       = '\0';
  m_cdr_event_type         = 0;
  m_event_struct_length    = 0;
  m_cause_conf_end         = 0;
  m_pConfStart             = NULL;
  m_pNetChanlCon           = NULL;
  m_pNetChannelDisco       = NULL;
  m_pPartyConnected        = NULL;
  m_pSvcSipPartyConnected  = NULL;
  m_pPartyDisconect        = NULL;
  m_pRemoteComMode         = NULL;
  m_pPartyErrors           = NULL;
  m_pOperAddParty          = NULL;
  m_pOperDelParty          = NULL;
  m_pOperSetEndTime        = NULL;
  m_pIpChanlCon            = NULL;
  m_pConfStartCont1        = NULL;
  m_pOperAddPartyCont1     = NULL;
  m_pOperAddPartyCont2     = NULL;
  m_pPartyDisconectCont1   = NULL;
  m_pAddPartyDetailed      = NULL;
  m_pConfStartCont2        = NULL;
  m_pConfStartCont3        = NULL;
  m_pPartyBillingCode      = NULL;
  m_pPartyVisualName       = NULL;
  m_pOperMoveParty         = NULL;
  m_pOperMoveToConf        = NULL;
  m_partyCallingNum        = NULL;
  m_pConfStartCont4        = NULL;
  m_pConfStartCont5        = NULL;
  m_pConfStartCont10       = NULL;
  m_pUpdateUserDefinedInfo = NULL;
  m_DTMFfailureInd         = NULL;
  m_partyCalledNum         = NULL;
  m_recording              = NULL;
  m_systemRecording        = NULL;
  m_sipPrivateExtensions   = NULL;
  m_gkInfo                 = NULL;
  m_NewRateInfo            = NULL;
  m_pOperIpV6PartyCont1    = NULL;
  m_pCDRPartyChairPerson   = NULL;
  m_CallInfo			   = NULL;
  m_CPartyCorrelationData  = NULL;
  m_CConfCorrelationData   = NULL;
  m_pMPIChanlCon           = NULL;
}

//--------------------------------------------------------------------------
CCdrEvent::CCdrEvent(const CCdrEvent& other) : CPObject(other)
{
  SAFE_COPY(m_operator_name, other.m_operator_name);

  m_cdr_event_type         = other.m_cdr_event_type;
  m_time_stamp             = other.m_time_stamp;
  m_event_struct_length    = other.m_event_struct_length;
  m_cause_conf_end         = other.m_cause_conf_end;
  m_pConfStart             = NULL;
  m_pNetChanlCon           = NULL;
  m_pNetChannelDisco       = NULL;
  m_pPartyConnected        = NULL;
  m_pSvcSipPartyConnected  = NULL;
  m_pPartyDisconect        = NULL;
  m_pRemoteComMode         = NULL;
  m_pPartyErrors           = NULL;
  m_pOperAddParty          = NULL;
  m_pOperDelParty          = NULL;
  m_pOperSetEndTime        = NULL;
  m_pIpChanlCon            = NULL;
  m_pConfStartCont1        = NULL;
  m_pOperAddPartyCont1     = NULL;
  m_pOperAddPartyCont2     = NULL;
  m_pPartyDisconectCont1   = NULL;
  m_pAddPartyDetailed      = NULL;
  m_pConfStartCont2        = NULL;
  m_pConfStartCont3        = NULL;
  m_pPartyBillingCode      = NULL;
  m_pPartyVisualName       = NULL;
  m_pOperMoveParty         = NULL;
  m_pOperMoveToConf        = NULL;
  m_partyCallingNum        = NULL;
  m_pConfStartCont4        = NULL;
  m_pConfStartCont5        = NULL;
  m_pConfStartCont10       = NULL;
  m_pUpdateUserDefinedInfo = NULL;
  m_DTMFfailureInd         = NULL;
  m_partyCalledNum         = NULL;
  m_recording              = NULL;
  m_systemRecording        = NULL;
  m_sipPrivateExtensions   = NULL;
  m_gkInfo                 = NULL;
  m_NewRateInfo            = NULL;
  m_pOperIpV6PartyCont1    = NULL;
  m_pCDRPartyChairPerson   = NULL;
  m_CallInfo 		       = NULL;
  m_CPartyCorrelationData  = NULL;
  m_CConfCorrelationData   = NULL;
  m_pMPIChanlCon           = NULL;

  *this                    = other;
}

//--------------------------------------------------------------------------
CCdrEvent& CCdrEvent::operator=(const CCdrEvent& other)
{
  if (&other == this) 
    return *this;
	
  SAFE_COPY(m_operator_name, other.m_operator_name);

  m_cdr_event_type      = other.m_cdr_event_type;
  m_event_struct_length = other.m_event_struct_length;
  m_cause_conf_end      = other.m_cause_conf_end;
  m_time_stamp          = other.m_time_stamp;

  switch (other.GetCdrEventType())
  {
    case CONFERENCE_START:
    {
      m_pConfStart = new CConfStart(*other.m_pConfStart);
      break;
    }

    case CONFERENCES_END:
    {
      m_cause_conf_end = other.m_cause_conf_end;
      break;
    }

    case NET_CHANNEL_CONNECTED:
    {
      m_pNetChanlCon = new CNetChanlCon(*other.m_pNetChanlCon);
      break;
    }

    case NET_CHANNEL_DISCONNECTED:
    {
      m_pNetChannelDisco = new CNetChannelDisco(*other.m_pNetChannelDisco);
      break;
    }

    case IP_PARTY_CONNECTED:
    case SIP_PARTY_CONNECTED:
    case EVENT_PARTY_CONNECTED:
    {
      m_pPartyConnected = new CPartyConnected(*other.m_pPartyConnected);
      break;
    }

    case SVC_SIP_PARTY_CONNECTED:
    {
      m_pSvcSipPartyConnected = new CSvcSipPartyConnected(*other.m_pSvcSipPartyConnected);
      break;
    }

    case EVENT_PARTY_DISCONNECTED:
    {
      m_pPartyDisconect = new CPartyDisconnected(*other.m_pPartyDisconect);
      break;
    }

    case REMOTE_COM_MODE:
    {
      m_pRemoteComMode = new CRemoteComMode(*other.m_pRemoteComMode);
      break;
    }

    case PARTY_ERRORS:
    {
      m_pPartyErrors = new CPartyErrors(*other.m_pPartyErrors);
      break;
    }

    case RESERVED_PARTY:
    case OPERATOR_ADD_PARTY:
    case OPERATOR_UPDATE_PARTY:
    {
      m_pOperAddParty = new COperAddParty(*other.m_pOperAddParty);
      break;
    }

    case EVENT_NEW_UNDEFINED_PARTY:
    {
      m_pAddPartyDetailed = new CAddPartyDetailed(*other.m_pAddPartyDetailed);
      break;
    }

    case OPERATOR_TERMINATE:
    {
      strcpy(m_operator_name, other.m_operator_name);
      break;
    }

    case OPERATORS_DELETE_PARTY:
    case OPERATOR_DISCONNECTE_PARTY:
    case OPERATOR_RECONNECT_PARTY:
    {
      m_pOperDelParty = new COperDelParty(*other.m_pOperDelParty);
      break;
    }

    case OPERATOR_SET_END_TIME:
    {
      m_pOperSetEndTime = new COperSetEndTime(*other.m_pOperSetEndTime);
      break;
    }

    case OPERRATOR_ONHOLD_PARTY:
    case OPERRATOR_BACK_TO_CONFERENCE_PARTY:
    case OPERRATOR_ATTEND_PARTY:
    case OPERRATOR_MOVE_PARTY_FROM_CONFERENCE:
    {
      m_pOperMoveParty = new COperMoveParty(*other.m_pOperMoveParty);
      break;
    }

    case OPERRATOR_ATTEND_PARTY_TO_CONFERENCE:
    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE:
    {
      m_pOperMoveToConf = new COperMoveToConf(*other.m_pOperMoveToConf);
      break;
    }

    case MPI_CHANNEL_CONNECTED:
    {
      m_pMPIChanlCon = new CMPIChanlCon(*other.m_pMPIChanlCon);
      break;
    }

    case H323_CALL_SETUP:
    case SIP_CALL_SETUP:
    {
      m_pIpChanlCon = new CIpChanlCon(*other.m_pIpChanlCon);
      break;
    }

    case CONFERENCE_START_CONTINUE_1:
    {
      m_pConfStartCont1 = new CConfStartCont1(*other.m_pConfStartCont1);
      break;
    }

    case CONFERENCE_START_CONTINUE_2:
    {
      m_pConfStartCont2 = new CConfStartCont2(*other.m_pConfStartCont2);
      break;
    }

    case CONFERENCE_START_CONTINUE_3:
    case CONFERENCE_REMARKS:
    {
      m_pConfStartCont3 = new CConfStartCont3(*other.m_pConfStartCont3);
      break;
    }

    case RESERVED_PARTY_CONTINUE_1:
    case OPERATOR_ADD_PARTY_CONTINUE_1:
    case OPERATOR_UPDATE_PARTY_CONTINUE_1:
    {
      m_pOperAddPartyCont1 = new COperAddPartyCont1(*other.m_pOperAddPartyCont1);
      break;
    }

    case RESERVED_PARTY_CONTINUE_2:
    case OPERATOR_UPDATE_PARTY_CONTINUE_2:
    case OPERATOR_ADD_PARTY_CONTINUE_2:
    case NEW_UNDEFINED_PARTY_CONTINUE_1:
    {
      m_pOperAddPartyCont2 = new COperAddPartyCont2(*other.m_pOperAddPartyCont2);
      break;
    }

    case PARTY_DISCONNECTED_CONTINUE_1:
    {
      m_pPartyDisconectCont1 = new CPartyDisconnectedCont1(*other.m_pPartyDisconectCont1);
      break;
    }

    case EVENT_PARTY_ADD_BILLING_CODE:
    {
      m_pPartyBillingCode = new CPartyAddBillingCode(*other.m_pPartyBillingCode);
      break;
    }

    case EVENT_SET_PARTY_VISUAL_NAME:
    {
      m_pPartyVisualName = new CPartySetVisualName(*other.m_pPartyVisualName);
      break;
    }

    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_1:
    {
      m_partyCallingNum = new CCDRPartyCalling_NumMoveToCont1(*other.m_partyCallingNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_4:
    {
      m_pConfStartCont4 = new CConfStartCont4(*other.m_pConfStartCont4);
      break;
    }

    case CONFERENCE_START_CONTINUE_5:
    {
      m_pConfStartCont5 = new CConfStartCont5(*other.m_pConfStartCont5);
      break;
    }

    case CONFERENCE_START_CONTINUE_10:
    {
      m_pConfStartCont10 = new CConfStartCont10(*other.m_pConfStartCont10);
      break;
    }

    case USER_DEFINED_INFORMATION:
    {
      m_pUpdateUserDefinedInfo = new CUpdateUserDefinedInfo(*other.m_pUpdateUserDefinedInfo);
      break;
    }

    case DTMF_CODE_FAILURE:
    {
      m_DTMFfailureInd = new CCDRPartyDTMFfailureIndication(*other.m_DTMFfailureInd);
      break;
    }

    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_2:
    {
      m_partyCalledNum = new CCDRPartyCalled_NumMoveToCont2(*other.m_partyCalledNum);
      break;
    }

    case RECORDING_LINK_EVENT:
    {
      m_recording = new CCDRPartyRecording(*other.m_recording);
      break;
    }

    case RECORDING_SYSTEM_LINK_EVENT:
    {
      m_systemRecording = new CCDRPartySystemRecording(*other.m_systemRecording);
      break;
    }

    case SIP_PRIVATE_EXTENSIONS_EVENT:
    {
      m_sipPrivateExtensions = new CCDRSipPrivateExtensions(*other.m_sipPrivateExtensions);
      break;
    }

    case GK_INFO:
    {
      m_gkInfo = new CGkInfo(*other.m_gkInfo);
      break;
    }

    case PARTY_NEW_RATE:
    {
      m_NewRateInfo = new CNewRateInfo(*other.m_NewRateInfo);
      break;
    }

	case PARTICIPANT_MAX_USAGE_INFO: 
	{
		m_CallInfo = new CCallInfo(*other.m_CallInfo);
		break; 
	}

	case PARTY_CORRELATION_DATA:
	{
		m_CPartyCorrelationData = new CPartyCorrelationData(*other.m_CPartyCorrelationData);
		break;
	}

	case CONF_CORRELATION_DATA:
	{
		m_CConfCorrelationData = new CConfCorrelationData(*other.m_CConfCorrelationData);
		break;
	}

    case  EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1(*other.m_pOperIpV6PartyCont1);
      break;
    }

    case  RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1(*other.m_pOperIpV6PartyCont1);
      break;
    }

    case  USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1(*other.m_pOperIpV6PartyCont1);
      break;
    }

    case  USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1(*other.m_pOperIpV6PartyCont1);
      break;
    }
    case  PARTY_CHAIR_UPDATE:
    if (m_pCDRPartyChairPerson != NULL)
    {
      PDELETE(m_pCDRPartyChairPerson);
    }

    m_pCDRPartyChairPerson = new CCDRPartyChairPerson(*other.m_pCDRPartyChairPerson);
    break;
  } // switch

  return *this;
}

//--------------------------------------------------------------------------
CCdrEvent::~CCdrEvent()
{
  PDELETE(m_pConfStart);
  PDELETE(m_pNetChanlCon);
  PDELETE(m_pNetChannelDisco);
  PDELETE(m_pPartyConnected);
  PDELETE(m_pSvcSipPartyConnected);
  PDELETE(m_pPartyDisconect);
  PDELETE(m_pRemoteComMode);
  PDELETE(m_pPartyErrors);
  PDELETE(m_pOperAddParty);
  PDELETE(m_pOperDelParty);
  PDELETE(m_pOperSetEndTime);
  PDELETE(m_pOperDelParty);
  PDELETE(m_pIpChanlCon);
  PDELETE(m_pConfStartCont1);
  PDELETE(m_pOperAddPartyCont1);
  PDELETE(m_pOperAddPartyCont2);
  PDELETE(m_pPartyDisconectCont1);
  PDELETE(m_pAddPartyDetailed);
  PDELETE(m_pConfStartCont2);
  PDELETE(m_pConfStartCont3);
  PDELETE(m_pPartyBillingCode);
  PDELETE(m_pPartyVisualName);
  PDELETE(m_partyCallingNum);
  PDELETE(m_pOperMoveParty);
  PDELETE(m_pOperMoveToConf);
  PDELETE(m_pConfStartCont4);
  PDELETE(m_pConfStartCont5);
  PDELETE(m_pConfStartCont10);
  PDELETE(m_pUpdateUserDefinedInfo);
  PDELETE(m_DTMFfailureInd);
  PDELETE(m_partyCalledNum);
  PDELETE(m_recording);
  PDELETE(m_systemRecording);
  PDELETE(m_sipPrivateExtensions);
  PDELETE(m_gkInfo);
  PDELETE(m_NewRateInfo);
  PDELETE(m_pOperIpV6PartyCont1);
  // calling to this delete cuese exception - remark it
  PDELETE(m_pCDRPartyChairPerson);
  PDELETE(m_CallInfo);
  PDELETE(m_CPartyCorrelationData);
  PDELETE(m_CConfCorrelationData);
  PDELETE(m_pMPIChanlCon);
}

//--------------------------------------------------------------------------
void CCdrEvent::Serialize(WORD format, std::ostream& ostr, DWORD apiNum, BYTE fileformat)
{
//	TRACEINTO << "BG_CDR  format = " << format;
  BYTE bilflag = 1;
  if (fileformat == 1)
  {
    switch (m_cdr_event_type)
    {
      case CONFERENCE_START:
      {
        ostr << "CONFERENCE START"<< "\n";
        break;
      }

      case CONFERENCES_END:
      {
        ostr << "CONFERENCE END" << "\n";
        break;
      }

      case NET_CHANNEL_CONNECTED:
      {
        ostr << "NET CHANNEL CONNECTED" << "\n";
        break;
      }

      case NET_CHANNEL_DISCONNECTED:
      {
        ostr << "NET CHANNEL DISCONNECTED"<< "\n";
        break;
      }

      case EVENT_PARTY_CONNECTED:
      {
        ostr << "PARTY CONNECTED" << "\n";
        break;
      }

      case EVENT_PARTY_DISCONNECTED:
      {
        ostr << "PARTY DISCONNECTED" << "\n";
        break;
      }

      case REMOTE_COM_MODE:
      {
        ostr << "REMOTE COM MODE" << "\n";
        break;
      }

      case PARTY_ERRORS:
      {
        ostr << "PARTY ERRORS" << "\n";
        break;
      }

      case RESERVED_PARTY:
      {
        ostr << "RESERVED PARTY" << "\n";
        break;
      }

      case OPERATOR_ADD_PARTY:
      {
        ostr << "OPERATOR ADD PARTY" << "\n";
        break;
      }

      case OPERATOR_UPDATE_PARTY:
      {
        ostr << "OPERATOR UPDATE PARTY" << "\n";
        break;
      }

      case OPERATOR_TERMINATE:
      {
        ostr << "OPERATOR TERMINATE" <<"\n";
        break;
      }

      case OPERATORS_DELETE_PARTY:
      {
        ostr << "OPERATORS DELETE PARTY" << "\n";
        break;
      }

      case OPERATOR_DISCONNECTE_PARTY:
      {
        ostr << "OPERATOR DISCONNECTED PARTY" << "\n";
        break;
      }

      case OPERATOR_RECONNECT_PARTY:
      {
        ostr << "OPERATOR RECONNECT PARTY" << "\n";
        break;
      }

      case OPERATOR_SET_END_TIME:
      {
        ostr << "OPERATOR SET END TIME" << "\n";
        break;
      }

      case OPERRATOR_MOVE_PARTY_FROM_CONFERENCE:
      {
        ostr << "OPERATOR MOVE PARTY FROM CONFERENCE" << "\n";
        break;
      }

      case OPERRATOR_ATTEND_PARTY:
      {
        ostr << "OPERATOR ATTEND PARTY" << "\n";
        break;
      }

      case OPERRATOR_ONHOLD_PARTY:
      {
        ostr << "OPERATOR ON HOLD PARTY" << "\n";
        break;
      }

      case OPERRATOR_BACK_TO_CONFERENCE_PARTY:
      {
        ostr << "OPERATOR BACK TO CONFERENCE PARTY" << "\n";
        break;
      }

      case UNRECOGNIZED_EVENT:
      {
        ostr << "UNRECOGNIZED_EVENT  " << "\n"<< "\n";
        break;
      }

      case ATM_CHANNEL_CONNECTED:
      {
        ostr << "ATM CHANNEL CONNECTED" << "\n";
        break;
      }

      case ATM_CHANNEL_DISCONNECTED:
      {
        ostr << "ATM CHANNEL DISCONNECTED" << "\n";
        break;
      }

      case MPI_CHANNEL_CONNECTED:
      {
        ostr << "MPI CHANNEL CONNECTED" << "\n";
        break;
      }

      case MPI_CHANNEL_DISCONNECTED:
      {
        ostr << "MPI CHANNEL DISCONNECTED" << "\n";
        break;
      }

      case H323_CALL_SETUP:
      {
        ostr << "H323 CALL SETUP" << "\n";
        break;
      }

      case SIP_CALL_SETUP:
      {
        ostr << "SIP CALL SETUP" << "\n";
        break;
      }

      case H323_CLEAR_INDICATION:
      {
        ostr << "H323 CLEAR INDICATION" << "\n";
        break;
      }

      case SIP_CLEAR_INDICATION:
      {
        ostr << "SIP CLEAR INDICATION" << "\n";
        break;
      }

      case IP_PARTY_CONNECTED:
      {
        ostr << "H323 PARTY CONNECTED" << "\n";
        break;
      }

      case SIP_PARTY_CONNECTED:
      {
        ostr << "SIP PARTY CONNECTED" << "\n";
        break;
      }

      case SVC_SIP_PARTY_CONNECTED:
      {
        ostr << "SVC SIP PARTY CONNECTED" << "\n";
        break;
      }

      case CONFERENCE_START_CONTINUE_1:
      {
        ostr << "CONFERENCE START CONTINUE 1" << "\n";
        break;
      }

      case CONFERENCE_START_CONTINUE_2:
      {
        ostr << "CONFERENCE START CONTINUE 2" << "\n";
        break;
      }

      case CONFERENCE_START_CONTINUE_3:
      {
        ostr << "CONFERENCE START CONTINUE 3" << "\n";
        break;
      }

      case RESERVED_PARTY_CONTINUE_1:
      {
        ostr << "RESERVED PARTY CONTINUE 1" << "\n";
        break;
      }

      case RESERVED_PARTY_CONTINUE_2:
      {
        ostr << "RESERVED PARTY CONTINUE 2" << "\n";
        break;
      }

      case OPERATOR_ADD_PARTY_CONTINUE_1:
      {
        ostr << "OPERATOR ADD PARTY CONTINUE 1" << "\n";
        break;
      }

      case OPERATOR_ADD_PARTY_CONTINUE_2:
      {
        ostr << "OPERATOR ADD PARTY CONTINUE 2" << "\n";
        break;
      }

      case OPERATOR_UPDATE_PARTY_CONTINUE_1:
      {
        ostr << "OPERATOR UPDATE PARTY CONTINUE 1" << "\n";
        break;
      }

      case OPERATOR_UPDATE_PARTY_CONTINUE_2:
      {
        ostr << "OPERATOR UPDATE PARTY CONTINUE 2" << "\n";
        break;
      }

      case PARTY_DISCONNECTED_CONTINUE_1:
      {
        ostr << "EVENT PARTY DISCONNECTED CONTINUE 1" << "\n";
        break;
      }

      case OPERRATOR_ATTEND_PARTY_TO_CONFERENCE:
      {
        ostr << "OPERATOR ATTEND PARTY TO CONFERENCE" << "\n";
        break;
      }

      case OPERRATOR_MOVE_PARTY_TO_CONFERENCE:
      {
        ostr << "OPERATOR MOVE PARTY TO CONFERENCE" << "\n";
        break;
      }

      case EVENT_NEW_UNDEFINED_PARTY:
      {
        ostr << "NEW UNDEFINED PARTY" << "\n";
        break;
      }

      case EVENT_PARTY_ADD_BILLING_CODE:
      {
        ostr << "BILLING CODE" << "\n";
        break;
      }

      case EVENT_SET_PARTY_VISUAL_NAME:
      {
        ostr << "SET PARTY VISUAL NAME" << "\n";
        break;
      }

      case CONFERENCE_REMARKS:
      {
        ostr << "CONFERENCE REMARKS" << "\n";
        break;
      }

      case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_1:
      {
        ostr << "OPERATOR MOVE PARTY TO CONFERENCE CONTINUE 1" << "\n";
        break;
      }

      case CONFERENCE_START_CONTINUE_4:
      {
        ostr << "CONFERENCE START CONTINUE 4" << "\n";
        break;
      }

      case CONFERENCE_START_CONTINUE_5:
      {
        ostr << "CONFERENCE START CONTINUE 5" << "\n";
        break;
      }

      case CONFERENCE_START_CONTINUE_10:
      {
        ostr << "CONFERENCE START CONTINUE 10" << "\n";
        break;
      }

      case USER_DEFINED_INFORMATION:
      {
        ostr << "USER_DEFINED_INFORMATION" << "\n";
        break;
      }

      case DTMF_CODE_FAILURE:
      {
        ostr << "DTMF CODE FAILURE" << "\n";
        break;
      }

      case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_2:
      {
        ostr << "OPERATOR MOVE PARTY TO CONFERENCE CONTINUE 2" << "\n";
        break;
      }

      case RECORDING_LINK_EVENT:
      {
        ostr << "RECORDING LINK" << "\n";
        break;
      }

      case RECORDING_SYSTEM_LINK_EVENT:
      {
        ostr << "RECORDING SYSTEM LINK" << "\n";
        break;
      }

      case SIP_PRIVATE_EXTENSIONS_EVENT:
      {
        ostr << "SIP PRIVATE EXTENSIONS" << "\n";
        break;
      }

      case GK_INFO:
      {
        ostr << "GK INFO" << "\n";
        break;
      }

      case PARTY_NEW_RATE:
      {
        ostr << "PARTICIPANT CONNECTION RATE" << "\n";
        break;
      }
	  
	  case PARTICIPANT_MAX_USAGE_INFO: 
	  {
		ostr << "PARTICIPANT MAX USAGE INFO" << "\n";
		break;
	  }

      case EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS:
      {
        ostr << "NEW UNDEFINED PARTY CONTINUE IPV6 ADDRESS" << "\n";
        break;
      }

      case RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
      {
        ostr << "RESERVED PARTICIPANT CONTINUE IPV6 ADDRESS" << "\n";
        break;
      }

      case USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
      {
        ostr << "USER ADD PARTICIPANT CONTINUE IPV6 ADDRESS" << "\n";
        break;
      }

      case USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
      {
        ostr << "USER UPDATE PARTICIPANT CONTINUE IPV6 ADDRESS" << "\n";
        break;
      }

      case PARTY_CHAIR_UPDATE:
      ostr << "PARTY CHAIR UPDATE" << "\n";
      break;

      default:
      {
        ostr<<"--"<<"\n";
        break;
      }
    }   // endswitch

  }
  else  /**/
  {
    bool bIsValidEvent = IsValidEvent(m_cdr_event_type);
    if (!bIsValidEvent)
    {
      PASSERTMSG(false == bIsValidEvent, "CCdrEvent::Serialize - evfent not valid ");
    }   /**/

    ostr << m_cdr_event_type << ",";
  }

  if ((fileformat == 1) && (m_cdr_event_type != UNRECOGNIZED_EVENT))
  {
    m_time_stamp.LongSerialize(ostr);
  }
  else if (m_cdr_event_type != UNRECOGNIZED_EVENT)
  {
    m_time_stamp.SerializeBilling(ostr);
  }

  if (!fileformat)
    ostr << m_event_struct_length   << ",";

  switch (m_cdr_event_type)
  {
    case CONFERENCE_START:
    {
      if (fileformat == 1)        // Long Serialize
        m_pConfStart->Serialize(format, ostr, fileformat);
      else
        m_pConfStart->Serialize(format, ostr);

      break;
    }

    case CONFERENCES_END:
    {
      if (fileformat == 1)        // Long Serialize
      {
        switch ((int)m_cause_conf_end)
        {
          case ONGOING:
          {
            ostr << "Ongoing Conference" << ";\n\n";
            break;
          }

          case CAUSE_OPERATOR_TERMINATE:
          {
            ostr << "Terminate by Operator" << ";\n\n";
            break;
          }

          case END_TIME_TERMINATE:
          {
            ostr << "Terminate when end time passed" << ";\n\n";
            break;
          }

          case AUTO_TERMINATION:
          {
            ostr << "Auto termination" << ";\n\n";
            break;
          }

          case NEVER_STARTED:
          {
            ostr << "Conference never started " << ";\n\n";
            break;
          }

          case CONF_PROBLEM:
          {
            ostr << "Conference problem" << ";\n\n";
            break;
          }

          case MCU_COMPLETED:
          {
            ostr << "Completed by MCU reset" << ";\n\n";
            break;
          }

          case STATUS_UNKNOWN:
          {
            ostr << "Unknown status" << ";\n\n";
            break;
          }

          default:
          {
            ostr<<"--"<<";\n\n";
            break;
          }
        }                                                                       // endswitch m_cause_conf_end
      }                                                                         // endif fileformat==1
      else
      {
        ostr << (WORD)m_cause_conf_end <<";\n";
      }

      break;
    }                                                                           // endcase conference end

    case NET_CHANNEL_CONNECTED:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pNetChanlCon->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pNetChanlCon->Serialize(format, ostr, apiNum);
      break;
    }

    case NET_CHANNEL_DISCONNECTED:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pNetChannelDisco->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pNetChannelDisco->Serialize(format, ostr, apiNum);
      break;
    }

    case SVC_SIP_PARTY_CONNECTED:
    {
      if (fileformat == 1)                                                      // Long Serialize
        ;//m_pSvcSipPartyConnected->Serialize323(format, ostr, fileformat, apiNum);
      else
        m_pSvcSipPartyConnected->Serialize(format, ostr, apiNum);
      break;
    }

    case IP_PARTY_CONNECTED:
    case SIP_PARTY_CONNECTED:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pPartyConnected->Serialize323(format, ostr, fileformat, apiNum);
      else
        m_pPartyConnected->Serialize(format, ostr, apiNum);
      break;
    }

    case EVENT_PARTY_CONNECTED:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pPartyConnected->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pPartyConnected->Serialize(format, ostr, apiNum);
      break;
    }

    case EVENT_PARTY_DISCONNECTED:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pPartyDisconect->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pPartyDisconect->Serialize(format, ostr, apiNum);
      break;
    }

    case REMOTE_COM_MODE:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pRemoteComMode->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pRemoteComMode->Serialize(format, ostr, apiNum);
      break;
    }

    case PARTY_ERRORS:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pPartyErrors->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pPartyErrors->Serialize(format, ostr, apiNum);
      break;
    }

    case RESERVED_PARTY:
    case OPERATOR_ADD_PARTY:
    case OPERATOR_UPDATE_PARTY:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pOperAddParty->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pOperAddParty->Serialize(format, ostr, apiNum);
      break;
    }

    case EVENT_NEW_UNDEFINED_PARTY:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pAddPartyDetailed->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pAddPartyDetailed->Serialize(format, ostr, apiNum);
      break;
    }

    case OPERATOR_TERMINATE:
    {
      if (fileformat == 1)                                                      // Long Serialize
        ostr << "terminate by:"<<m_operator_name<<";\n\n";
      else
        ostr << m_operator_name <<";\n";
      break;
    }

    case OPERATORS_DELETE_PARTY:
    case OPERATOR_DISCONNECTE_PARTY:
    case OPERATOR_RECONNECT_PARTY:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pOperDelParty->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pOperDelParty->Serialize(format, ostr, apiNum);
      break;
    }

    case OPERATOR_SET_END_TIME:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pOperSetEndTime->Serialize(format, ostr, fileformat);
      else
        m_pOperSetEndTime->Serialize(format, ostr);
      break;
    }

    case OPERRATOR_ATTEND_PARTY:
    case OPERRATOR_MOVE_PARTY_FROM_CONFERENCE:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pOperMoveParty->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pOperMoveParty->Serialize(format, ostr, apiNum);
      break;
    }

    case OPERRATOR_ATTEND_PARTY_TO_CONFERENCE:
    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pOperMoveToConf->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pOperMoveToConf->Serialize(format, ostr, apiNum);
      break;
    }

    case OPERRATOR_ONHOLD_PARTY:
    case OPERRATOR_BACK_TO_CONFERENCE_PARTY:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pOperMoveParty->SerializeShort(format, ostr, fileformat, apiNum);
      else
        m_pOperMoveParty->SerializeShort(format, ostr, apiNum);
      break;
    }

    case H323_CLEAR_INDICATION:
    case H323_CALL_SETUP:
    case SIP_CALL_SETUP:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pIpChanlCon->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pIpChanlCon->Serialize(format, ostr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_1:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pConfStartCont1->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pConfStartCont1->Serialize(format, ostr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_2:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pConfStartCont2->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pConfStartCont2->Serialize(format, ostr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_3:
    case CONFERENCE_REMARKS:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pConfStartCont3->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pConfStartCont3->Serialize(format, ostr, apiNum);
      break;
    }

    case RESERVED_PARTY_CONTINUE_1:
    case OPERATOR_ADD_PARTY_CONTINUE_1:
    case OPERATOR_UPDATE_PARTY_CONTINUE_1:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pOperAddPartyCont1->Serialize(format, ostr, fileformat, apiNum);      // ---TO CHANGE.
      else
        m_pOperAddPartyCont1->Serialize(format, ostr, apiNum);
      break;
    }

    case RESERVED_PARTY_CONTINUE_2:
    case OPERATOR_UPDATE_PARTY_CONTINUE_2:
    case OPERATOR_ADD_PARTY_CONTINUE_2:
    case NEW_UNDEFINED_PARTY_CONTINUE_1:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pOperAddPartyCont2->Serialize(format, ostr, fileformat, apiNum);      // ---TO CHANGE.
      else
        m_pOperAddPartyCont2->Serialize(format, ostr, apiNum);
      break;
    }

    case PARTY_DISCONNECTED_CONTINUE_1:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pPartyDisconectCont1->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pPartyDisconectCont1->Serialize(format, ostr, apiNum);
      break;
    }

    case EVENT_PARTY_ADD_BILLING_CODE:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pPartyBillingCode->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pPartyBillingCode->Serialize(format, ostr, apiNum);
      break;
    }

    case EVENT_SET_PARTY_VISUAL_NAME:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pPartyVisualName->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pPartyVisualName->Serialize(format, ostr, apiNum);
      break;
    }

    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_1:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_partyCallingNum->Serialize(format, ostr, fileformat, apiNum);
      else
        m_partyCallingNum->Serialize(format, ostr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_4:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pConfStartCont4->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pConfStartCont4->Serialize(format, ostr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_5:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pConfStartCont5->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pConfStartCont5->Serialize(format, ostr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_10:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pConfStartCont10->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pConfStartCont10->Serialize(format, ostr, apiNum);
      break;
    }

    case USER_DEFINED_INFORMATION:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_pUpdateUserDefinedInfo->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pUpdateUserDefinedInfo->Serialize(format, ostr, apiNum);
      break;
    }

    case DTMF_CODE_FAILURE:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_DTMFfailureInd->Serialize(format, ostr, fileformat, apiNum);
      else
        m_DTMFfailureInd->Serialize(format, ostr, apiNum);
      break;
    }

    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_2:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_partyCalledNum->Serialize(format, ostr, fileformat, apiNum);
      else
        m_partyCalledNum->Serialize(format, ostr, apiNum);
      break;
    }

    case RECORDING_LINK_EVENT:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_recording->Serialize(format, ostr, fileformat, apiNum);
      else
        m_recording->Serialize(format, ostr, apiNum);
      break;
    }

    case RECORDING_SYSTEM_LINK_EVENT:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_systemRecording->Serialize(format, ostr, fileformat, apiNum);
      else
        m_systemRecording->Serialize(format, ostr, apiNum);
      break;
    }

    case SIP_PRIVATE_EXTENSIONS_EVENT:
    {
      if (fileformat == 1)                                                      // Long Serialize
        m_sipPrivateExtensions->Serialize(format, ostr, fileformat, apiNum);
      else
        m_sipPrivateExtensions->Serialize(format, ostr, apiNum);
      break;
    }

    case GK_INFO:
    {
      m_gkInfo->Serialize(format, ostr, apiNum);
      break;
    }

    case EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1->Serialize(format, ostr, apiNum);
      break;
    }

    case RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1->Serialize(format, ostr, apiNum);
      break;
    }

    case USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1->Serialize(format, ostr, apiNum);
      break;
    }

    case USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1->Serialize(format, ostr, apiNum);
      break;
    }

    case PARTY_NEW_RATE:
    {
      m_NewRateInfo->Serialize(format, ostr, apiNum);
      break;
    }
	
	case PARTICIPANT_MAX_USAGE_INFO:
	{
	  m_CallInfo->Serialize(format, ostr, apiNum);
	  break;
	}
	case PARTY_CORRELATION_DATA:
	{
		m_CPartyCorrelationData->Serialize(format, ostr, apiNum);
		break;
	}
	case CONF_CORRELATION_DATA:
	{
		m_CConfCorrelationData->Serialize(format, ostr, apiNum);
		break;
	}

    case PARTY_CHAIR_UPDATE:
    {
      if (fileformat == 1)  // Long Serialize
        m_pCDRPartyChairPerson->Serialize(format, ostr, fileformat, apiNum);
      else
        m_pCDRPartyChairPerson->Serialize(format, ostr, apiNum);
      break;
    }

    default:
    {
      break;
    }
  } // switch
}

//--------------------------------------------------------------------------
void CCdrEvent::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  BYTE bilflag = 0;
  char unRecognizedEvent [CDR_SIZE_RECORD];

  istr >> m_cdr_event_type;
  istr.ignore(1);

  m_time_stamp.DeSerializeBilling(istr);

  istr >> m_event_struct_length;
  istr.ignore(1);
  switch (m_cdr_event_type)
  {
    case CONFERENCE_START:
    {
      m_pConfStart = new CConfStart;
      m_pConfStart->DeSerialize(format, istr);
      break;
    }

    case CONFERENCES_END:
    {
      istr >> tmp;
      m_cause_conf_end = (BYTE)tmp;
      istr.ignore(1);
      break;
    }

    case NET_CHANNEL_CONNECTED:
    {
      m_pNetChanlCon = new CNetChanlCon;
      m_pNetChanlCon->DeSerialize(format, istr, apiNum);
      break;
    }

    case NET_CHANNEL_DISCONNECTED:
    {
      m_pNetChannelDisco = new CNetChannelDisco;
      m_pNetChannelDisco->DeSerialize(format, istr);
      break;
    }

    case IP_PARTY_CONNECTED:
    case SIP_PARTY_CONNECTED:
    case EVENT_PARTY_CONNECTED:
    {
      m_pPartyConnected = new CPartyConnected;
      m_pPartyConnected->DeSerialize(format, istr, apiNum);
      break;
    }

    case SVC_SIP_PARTY_CONNECTED:
    {
      m_pSvcSipPartyConnected = new CSvcSipPartyConnected;
      m_pSvcSipPartyConnected->DeSerialize(format, istr, apiNum);
      break;
    }

    case EVENT_PARTY_DISCONNECTED:
    {
      m_pPartyDisconect = new CPartyDisconnected;
      m_pPartyDisconect->DeSerialize(format, istr, apiNum);
      break;
    }

    case REMOTE_COM_MODE:
    {
      m_pRemoteComMode = new CRemoteComMode;
      m_pRemoteComMode->DeSerialize(format, istr, apiNum);
      break;
    }

    case PARTY_ERRORS:
    {
      m_pPartyErrors = new CPartyErrors;
      m_pPartyErrors->DeSerialize(format, istr, apiNum);
      break;
    }

    case RESERVED_PARTY:
    case OPERATOR_ADD_PARTY:
    case OPERATOR_UPDATE_PARTY:
    {
      m_pOperAddParty = new COperAddParty;
      m_pOperAddParty->DeSerialize(format, istr, apiNum);
      break;
    }

    case EVENT_NEW_UNDEFINED_PARTY:
    {
      m_pAddPartyDetailed = new CAddPartyDetailed;
      m_pAddPartyDetailed->DeSerialize(format, istr, apiNum);
      break;
    }

    case OPERATOR_TERMINATE:
    {
      istr.getline(m_operator_name, OPERATOR_NAME_LEN+1, ';');
      break;
    }

    case OPERATORS_DELETE_PARTY:
    case OPERATOR_DISCONNECTE_PARTY:
    case OPERATOR_RECONNECT_PARTY:
    {
      m_pOperDelParty = new COperDelParty;
      m_pOperDelParty->DeSerialize(format, istr, apiNum);
      break;
    }

    case OPERATOR_SET_END_TIME:
    {
      m_pOperSetEndTime = new COperSetEndTime;
      m_pOperSetEndTime->DeSerialize(format, istr);
      break;
    }

    case OPERRATOR_ATTEND_PARTY:
    case OPERRATOR_MOVE_PARTY_FROM_CONFERENCE:
    {
      m_pOperMoveParty = new COperMoveParty;
      m_pOperMoveParty->DeSerialize(format, istr, apiNum);
      break;
    }

    case OPERRATOR_BACK_TO_CONFERENCE_PARTY:
    case OPERRATOR_ONHOLD_PARTY:
    {
      m_pOperMoveParty = new COperMoveParty;
      m_pOperMoveParty->DeSerializeShort(format, istr, apiNum);
      break;
    }

    case OPERRATOR_ATTEND_PARTY_TO_CONFERENCE:
    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE:
    {
      m_pOperMoveToConf = new COperMoveToConf;
      m_pOperMoveToConf->DeSerialize(format, istr, apiNum);
      break;
    }

    case MPI_CHANNEL_CONNECTED:
    {
      m_pMPIChanlCon = new CMPIChanlCon;
      m_pMPIChanlCon->DeSerialize(format, istr, apiNum);
      break;
    }

    case H323_CALL_SETUP:
    case SIP_CALL_SETUP:
    {
      m_pIpChanlCon = new CIpChanlCon;
      m_pIpChanlCon->DeSerialize(format, istr, apiNum);
      break;
    }

    case H323_CLEAR_INDICATION:
    case CONFERENCE_START_CONTINUE_1:
    {
      m_pConfStartCont1 = new CConfStartCont1;
      m_pConfStartCont1->DeSerialize(format, istr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_2:
    {
      m_pConfStartCont2 = new CConfStartCont2;
      m_pConfStartCont2->DeSerialize(format, istr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_3:
    case CONFERENCE_REMARKS:
    {
      m_pConfStartCont3 = new CConfStartCont3;
      m_pConfStartCont3->DeSerialize(format, istr, apiNum);
      break;
    }

    case RESERVED_PARTY_CONTINUE_1:
    case OPERATOR_ADD_PARTY_CONTINUE_1:
    case OPERATOR_UPDATE_PARTY_CONTINUE_1:
    {
      m_pOperAddPartyCont1 = new COperAddPartyCont1;
      m_pOperAddPartyCont1->DeSerialize(format, istr, apiNum);
      break;
    }

    case RESERVED_PARTY_CONTINUE_2:
    case OPERATOR_UPDATE_PARTY_CONTINUE_2:
    case OPERATOR_ADD_PARTY_CONTINUE_2:
    case NEW_UNDEFINED_PARTY_CONTINUE_1:
    {
      m_pOperAddPartyCont2 = new COperAddPartyCont2;
      m_pOperAddPartyCont2->DeSerialize(format, istr, apiNum);
      break;
    }

    case PARTY_DISCONNECTED_CONTINUE_1:
    {
      m_pPartyDisconectCont1 = new CPartyDisconnectedCont1;
      m_pPartyDisconectCont1->DeSerialize(format, istr, apiNum);
      break;
    }

    case EVENT_PARTY_ADD_BILLING_CODE:
    {
      m_pPartyBillingCode = new CPartyAddBillingCode;
      m_pPartyBillingCode->DeSerialize(format, istr, apiNum);
      break;
    }

    case EVENT_SET_PARTY_VISUAL_NAME:
    {
      m_pPartyVisualName = new CPartySetVisualName;
      m_pPartyVisualName->DeSerialize(format, istr, apiNum);
      break;
    }

    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_1:
    {
      m_partyCallingNum = new CCDRPartyCalling_NumMoveToCont1;
      m_partyCallingNum->DeSerialize(format, istr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_4:
    {
      m_pConfStartCont4 = new CConfStartCont4;
      m_pConfStartCont4->DeSerialize(format, istr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_5:
    {
      m_pConfStartCont5 = new CConfStartCont5;
      m_pConfStartCont5->DeSerialize(format, istr, apiNum);
      break;
    }

    case CONFERENCE_START_CONTINUE_10:
    {
      m_pConfStartCont10 = new CConfStartCont10;
      m_pConfStartCont10->DeSerialize(format, istr, apiNum);
      break;
    }

    case USER_DEFINED_INFORMATION:
    {
      m_pUpdateUserDefinedInfo = new CUpdateUserDefinedInfo;
      m_pUpdateUserDefinedInfo->DeSerialize(format, istr, apiNum);
      break;
    }

    case DTMF_CODE_FAILURE:
    {
      m_DTMFfailureInd = new CCDRPartyDTMFfailureIndication;
      m_DTMFfailureInd->DeSerialize(format, istr, apiNum);
      break;
    }

    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_2:
    {
      m_partyCalledNum = new CCDRPartyCalled_NumMoveToCont2;
      m_partyCalledNum->DeSerialize(format, istr, apiNum);
      break;
    }

    case RECORDING_LINK_EVENT:
    {
      m_recording = new CCDRPartyRecording;
      m_recording->DeSerialize(format, istr, apiNum);
      break;
    }

    case RECORDING_SYSTEM_LINK_EVENT:
    {
      m_systemRecording = new CCDRPartySystemRecording;
      m_systemRecording->DeSerialize(format, istr, apiNum);
      break;
    }

    case SIP_PRIVATE_EXTENSIONS_EVENT:
    {
      m_sipPrivateExtensions = new CCDRSipPrivateExtensions;
      m_sipPrivateExtensions->DeSerialize(format, istr, apiNum);
      break;
    }

    case GK_INFO:
    {
      m_gkInfo = new CGkInfo;
      m_gkInfo->DeSerialize(format, istr, apiNum);
      break;
    }

    case EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1;
      m_pOperIpV6PartyCont1->DeSerialize(format, istr, apiNum);
      break;
    }

    case RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1;
      m_pOperIpV6PartyCont1->DeSerialize(format, istr, apiNum);
      break;
    }

    case USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1;
      m_pOperIpV6PartyCont1->DeSerialize(format, istr, apiNum);
      break;
    }

    case USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1;
      m_pOperIpV6PartyCont1->DeSerialize(format, istr, apiNum);
      break;
    }

    case PARTY_NEW_RATE:
    {
      m_NewRateInfo = new CNewRateInfo;
      m_NewRateInfo->DeSerialize(format, istr, apiNum);
      break;
    }

	case PARTICIPANT_MAX_USAGE_INFO:
	{
	  m_CallInfo = new CCallInfo;
	  m_CallInfo->DeSerialize(format, istr ,apiNum);
	  break;
	}

	case PARTY_CORRELATION_DATA:
	{
		m_CPartyCorrelationData = new CPartyCorrelationData;
		m_CPartyCorrelationData->DeSerialize(format, istr ,apiNum);
		break;
	}
	case CONF_CORRELATION_DATA:
	{
		m_CConfCorrelationData = new CConfCorrelationData;
		m_CConfCorrelationData->DeSerialize(format, istr ,apiNum);
		break;
	}

    case PARTY_CHAIR_UPDATE:
    {
      if (m_pCDRPartyChairPerson != NULL)
      {
        PDELETE(m_pCDRPartyChairPerson);
      }

      m_pCDRPartyChairPerson = new CCDRPartyChairPerson;
      m_pCDRPartyChairPerson->DeSerialize(format, istr, apiNum);
      break;
    }

    default:
    {
      istr.getline(unRecognizedEvent, CDR_SIZE_RECORD+1, ';');
      m_cdr_event_type      = UNRECOGNIZED_EVENT;
      m_event_struct_length = 0;
      m_time_stamp.m_day    = 0;
      m_time_stamp.m_hour   = 0;
      m_time_stamp.m_min    = 0;
      m_time_stamp.m_mon    = 0;
      m_time_stamp.m_sec    = 0;
      m_time_stamp.m_year   = 0;
    }
  } // switch
}

//--------------------------------------------------------------------------
void CCdrEvent::SerializeXml(CXMLDOMElement* pFatherNode) const
{
  CXMLDOMElement* pEventNode = new CXMLDOMElement("CDR_EVENT");

  switch (m_cdr_event_type)
  {
    case CONFERENCE_START:
    {
      m_pConfStart->SerializeXml(pEventNode);
      break;
    }

    case CONFERENCE_START_CONTINUE_1:
    {
      m_pConfStartCont1->SerializeXml(pEventNode);
      break;
    }

    case CONFERENCE_START_CONTINUE_2:
    {
      m_pConfStartCont2->SerializeXml(pEventNode);
      break;
    }

    case CONFERENCE_START_CONTINUE_3:
    {
      m_pConfStartCont3->SerializeXml(pEventNode);
      break;
    }

    case NET_CHANNEL_CONNECTED:
    {
      m_pNetChanlCon->SerializeXml(pEventNode);
      break;
    }

    case H323_CALL_SETUP:
    case SIP_CALL_SETUP:
    {
      m_pIpChanlCon->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case NET_CHANNEL_DISCONNECTED:
    {
      m_pNetChannelDisco->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case CONFERENCES_END:
    {
      BuildConfEndXml(pEventNode);
      break;
    }

    case EVENT_PARTY_DISCONNECTED:
    {
      m_pPartyDisconect->SerializeXml(pEventNode);
      break;
    }

    case PARTY_DISCONNECTED_CONTINUE_1:
    {
      m_pPartyDisconectCont1->SerializeXml(pEventNode);
      break;
    }

    case REMOTE_COM_MODE:
    {
      m_pRemoteComMode->SerializeXml(pEventNode);
      break;
    }

    case EVENT_PARTY_CONNECTED:
    case IP_PARTY_CONNECTED:
    case SIP_PARTY_CONNECTED:
    {
      m_pPartyConnected->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }
    case SVC_SIP_PARTY_CONNECTED:
    {
		m_pSvcSipPartyConnected->SerializeXml(pEventNode, m_cdr_event_type);
		break;
    }

    case OPERATOR_TERMINATE:
    {
      BuildOperTerminateXml(pEventNode);
      break;
    }

    case OPERATOR_SET_END_TIME:
    {
      m_pOperSetEndTime->SerializeXml(pEventNode);
      break;
    }

    case OPERATORS_DELETE_PARTY:
    case OPERATOR_DISCONNECTE_PARTY:
    case OPERATOR_RECONNECT_PARTY:
    {
      m_pOperDelParty->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case OPERRATOR_ATTEND_PARTY:
    case OPERRATOR_MOVE_PARTY_FROM_CONFERENCE:
    {
      m_pOperMoveParty->SerializeXml(pEventNode, m_cdr_event_type, NO /*isShortSerial*/);
      break;
    }

    case OPERRATOR_BACK_TO_CONFERENCE_PARTY:
    case OPERRATOR_ONHOLD_PARTY:
    {
      m_pOperMoveParty->SerializeXml(pEventNode, m_cdr_event_type, YES /*isShortSerial*/);
      break;
    }

    case OPERRATOR_ATTEND_PARTY_TO_CONFERENCE:
    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE:
    {
      m_pOperMoveToConf->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case OPERATOR_ADD_PARTY:
    case RESERVED_PARTY:
    case OPERATOR_UPDATE_PARTY:
    {
      m_pOperAddParty->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case RESERVED_PARTY_CONTINUE_1:
    case OPERATOR_ADD_PARTY_CONTINUE_1:
    case OPERATOR_UPDATE_PARTY_CONTINUE_1:
    {
      m_pOperAddPartyCont1->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case EVENT_NEW_UNDEFINED_PARTY:
    {
      m_pAddPartyDetailed->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case RESERVED_PARTY_CONTINUE_2:
    case OPERATOR_UPDATE_PARTY_CONTINUE_2:
    case OPERATOR_ADD_PARTY_CONTINUE_2:
    case NEW_UNDEFINED_PARTY_CONTINUE_1:
    {
      m_pOperAddPartyCont2->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case PARTY_ERRORS:
    {
      m_pPartyErrors->SerializeXml(pEventNode);
      break;
    }

    case EVENT_SET_PARTY_VISUAL_NAME:
    {
      m_pPartyVisualName->SerializeXml(pEventNode);
      break;
    }

    case EVENT_PARTY_ADD_BILLING_CODE:
    {
      m_pPartyBillingCode->SerializeXml(pEventNode);
      break;
    }

    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_1:
    {
      m_partyCallingNum->SerializeXml(pEventNode);
      break;
    }

    case CONFERENCE_START_CONTINUE_4:
    {
      m_pConfStartCont4->SerializeXml(pEventNode);
      break;
    }

    case CONFERENCE_START_CONTINUE_5:
    {
      m_pConfStartCont5->SerializeXml(pEventNode);
      break;
    }

    case CONFERENCE_START_CONTINUE_10:
    {
      m_pConfStartCont10->SerializeXml(pEventNode);
//      TRACEINTO << "BG_CDR - m_AvcSvc = " << (int)(m_pConfStartCont10->GetAvcSvc());
      break;
    }

    case USER_DEFINED_INFORMATION:
    {
      m_pUpdateUserDefinedInfo->SerializeXml(pEventNode);
      break;
    }

    case DTMF_CODE_FAILURE:
    {
      m_DTMFfailureInd->SerializeXml(pEventNode);
      break;
    }

    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_2:
    {
      m_partyCalledNum->SerializeXml(pEventNode);
      break;
    }

    case RECORDING_LINK_EVENT:
    {
      m_recording->SerializeXml(pEventNode);
      break;
    }

    case RECORDING_SYSTEM_LINK_EVENT:
    {
      m_systemRecording->SerializeXml(pEventNode);
      break;
    }

    case SIP_PRIVATE_EXTENSIONS_EVENT:
    {
      m_sipPrivateExtensions->SerializeXml(pEventNode);
      break;
    }

    case GK_INFO:
    {
      m_gkInfo->SerializeXml(pEventNode);
      break;
    }

    case PARTY_NEW_RATE:
    {
      m_NewRateInfo->SerializeXml(pEventNode);
      break;
    }

	case PARTICIPANT_MAX_USAGE_INFO:
	{
	  m_CallInfo->SerializeXml(pEventNode);
	  break;
	}
	case PARTY_CORRELATION_DATA:
	{
		m_CPartyCorrelationData->SerializeXml(pEventNode);
		break;
	}

	case CONF_CORRELATION_DATA:
	{
		m_CConfCorrelationData->SerializeXml(pEventNode);
		break;
	}


    case UNRECOGNIZED_EVENT:
    {
      pEventNode->AddChildNode("UNRECOGNIZED_EVENT");
      break;
    }

    case  EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case  RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case  USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case  USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      m_pOperIpV6PartyCont1->SerializeXml(pEventNode, m_cdr_event_type);
      break;
    }

    case PARTY_CHAIR_UPDATE:
    {
      m_pCDRPartyChairPerson->SerializeXml(pEventNode);
      break;
    }

    default:
    {
      delete pEventNode;
      pEventNode = NULL;
    }
  } // switch

  if (pEventNode)
  {
    pEventNode->AddChildNode("TIME_STAMP", m_time_stamp);
    pFatherNode->AddChildNode(pEventNode);
  }
}

//--------------------------------------------------------------------------
void CCdrEvent::BuildConfEndXml(CXMLDOMElement* pFatherNode) const
{
  CXMLDOMElement* pConfEndNode = pFatherNode->AddChildNode("CONF_END");
  pConfEndNode->AddChildNode("CONF_END_CAUSE", m_cause_conf_end, CONF_END_CAUSE_TYPE_ENUM);
}

//--------------------------------------------------------------------------
void CCdrEvent::BuildOperTerminateXml(CXMLDOMElement* pFatherNode) const
{
  CXMLDOMElement* pOperTerminateNode = pFatherNode->AddChildNode("OPERATOR_TERMINATE");
  pOperTerminateNode->AddChildNode("OPERATOR_NAME", m_operator_name);
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CCdrEvent::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;
  CXMLDOMElement* pChildNode = NULL;

  pActionNode->nextChildNode(&pChildNode);

  char* pszChildName = NULL;
  pChildNode->get_nodeName(&pszChildName);

  if (!strcmp(pszChildName, "CONF_START"))
  {
    m_cdr_event_type = CONFERENCE_START;
    m_pConfStart     = new CConfStart;
    nStatus          = m_pConfStart->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "CONF_START_1"))
  {
    m_cdr_event_type  = CONFERENCE_START_CONTINUE_1;
    m_pConfStartCont1 = new CConfStartCont1;
    nStatus           = m_pConfStartCont1->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "CONF_START_2"))
  {
    m_cdr_event_type  = CONFERENCE_START_CONTINUE_2;
    m_pConfStartCont2 = new CConfStartCont2;
    nStatus           = m_pConfStartCont2->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "CONF_START_3"))
  {
    m_cdr_event_type  = CONFERENCE_START_CONTINUE_3;
    m_pConfStartCont3 = new CConfStartCont3;
    nStatus           = m_pConfStartCont3->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "NET_CHANNEL_CONNECT"))
  {
    m_cdr_event_type = NET_CHANNEL_CONNECTED;
    m_pNetChanlCon   = new CNetChanlCon;
    nStatus          = m_pNetChanlCon->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "MPI_CHANNEL_CONNECT"))
  {
    m_cdr_event_type = MPI_CHANNEL_CONNECTED;
    m_pMPIChanlCon   = new CMPIChanlCon;
    nStatus          = m_pMPIChanlCon->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "H323_CALL_SETUP"))
  {
    m_cdr_event_type = H323_CALL_SETUP;
    m_pIpChanlCon    = new CIpChanlCon;
    nStatus          = m_pIpChanlCon->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "SIP_CALL_SETUP"))
  {
    m_cdr_event_type = SIP_CALL_SETUP;
    m_pIpChanlCon    = new CIpChanlCon;
    nStatus          = m_pIpChanlCon->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "NET_CHANNEL_DISCONNECTED"))
  {
    m_cdr_event_type   = NET_CHANNEL_DISCONNECTED;
    m_pNetChannelDisco = new CNetChannelDisco;
    nStatus            = m_pNetChannelDisco->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "CONF_END"))
  {
    m_cdr_event_type = CONFERENCES_END;
    GET_VALIDATE_CHILD(pChildNode, "CONF_END_CAUSE", &m_cause_conf_end, CONF_END_CAUSE_TYPE_ENUM);
  }
  else if (!strcmp(pszChildName, "PARTY_CONNECTED"))
  {
    m_cdr_event_type  = EVENT_PARTY_CONNECTED;
    m_pPartyConnected = new CPartyConnected;
    nStatus           = m_pPartyConnected->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "PARTY_DISCONNECTED"))
  {
    m_cdr_event_type  = EVENT_PARTY_DISCONNECTED;
    m_pPartyDisconect = new CPartyDisconnected;
    nStatus           = m_pPartyDisconect->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "PARTY_DISCONNECTED_1"))
  {
    m_cdr_event_type       = PARTY_DISCONNECTED_CONTINUE_1;
    m_pPartyDisconectCont1 = new CPartyDisconnectedCont1;
    nStatus                = m_pPartyDisconectCont1->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "REMOTE_COMMUNICATION_MODE"))
  {
    m_cdr_event_type = REMOTE_COM_MODE;
    m_pRemoteComMode = new CRemoteComMode;
    nStatus          = m_pRemoteComMode->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "H323_PARTY_CONNECTED"))
  {
    m_cdr_event_type  = IP_PARTY_CONNECTED;
    m_pPartyConnected = new CPartyConnected;
    nStatus           = m_pPartyConnected->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "SIP_PARTY_CONNECTED"))
  {
    m_cdr_event_type  = SIP_PARTY_CONNECTED;
    m_pPartyConnected = new CPartyConnected;
    nStatus           = m_pPartyConnected->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_TERMINATE"))
  {
    m_cdr_event_type = OPERATOR_TERMINATE;
    GET_VALIDATE_CHILD(pChildNode, "OPERATOR_NAME", m_operator_name, _1_TO_OPERATOR_NAME_LENGTH);
  }
  else if (!strcmp(pszChildName, "OPERATOR_SET_END_TIME"))
  {
    m_cdr_event_type  = OPERATOR_SET_END_TIME;
    m_pOperSetEndTime = new COperSetEndTime;
    nStatus           = m_pOperSetEndTime->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_DELETE_PARTY"))
  {
    m_cdr_event_type = OPERATORS_DELETE_PARTY;
    m_pOperDelParty  = new COperDelParty;
    nStatus          = m_pOperDelParty->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_DISCONNECT_PARTY"))
  {
    m_cdr_event_type = OPERATOR_DISCONNECTE_PARTY;
    m_pOperDelParty  = new COperDelParty;
    nStatus          = m_pOperDelParty->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_RECONNECT_PARTY"))
  {
    m_cdr_event_type = OPERATOR_RECONNECT_PARTY;
    m_pOperDelParty  = new COperDelParty;
    nStatus          = m_pOperDelParty->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERRATOR_ATTEND_PARTY"))
  {
    m_cdr_event_type = OPERRATOR_ATTEND_PARTY;
    m_pOperMoveParty = new COperMoveParty;
    nStatus          = m_pOperMoveParty->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERRATOR_MOVE_PARTY_FROM_CONFERENCE"))
  {
    m_cdr_event_type = OPERRATOR_MOVE_PARTY_FROM_CONFERENCE;
    m_pOperMoveParty = new COperMoveParty;
    nStatus          = m_pOperMoveParty->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERRATOR_BACK_TO_CONFERENCE_PARTY"))
  {
    m_cdr_event_type = OPERRATOR_BACK_TO_CONFERENCE_PARTY;
    m_pOperMoveParty = new COperMoveParty;
    nStatus          = m_pOperMoveParty->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERRATOR_ONHOLD_PARTY"))
  {
    m_cdr_event_type = OPERRATOR_ONHOLD_PARTY;
    m_pOperMoveParty = new COperMoveParty;
    nStatus          = m_pOperMoveParty->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERRATOR_ATTEND_PARTY_TO_CONFERENCE"))
  {
    m_cdr_event_type  = OPERRATOR_ATTEND_PARTY_TO_CONFERENCE;
    m_pOperMoveToConf = new COperMoveToConf;
    nStatus           = m_pOperMoveToConf->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERRATOR_MOVE_PARTY_TO_CONFERENCE"))
  {
    m_cdr_event_type  = OPERRATOR_MOVE_PARTY_TO_CONFERENCE;
    m_pOperMoveToConf = new COperMoveToConf;
    nStatus           = m_pOperMoveToConf->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_ADD_PARTY"))
  {
    m_cdr_event_type = OPERATOR_ADD_PARTY;
    m_pOperAddParty  = new COperAddParty;
    nStatus          = m_pOperAddParty->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_ADD_PARTY_1"))
  {
    m_cdr_event_type     = OPERATOR_ADD_PARTY_CONTINUE_1;
    m_pOperAddPartyCont1 = new COperAddPartyCont1;
    nStatus              = m_pOperAddPartyCont1->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_ADD_PARTY_2"))
  {
    m_cdr_event_type     = OPERATOR_ADD_PARTY_CONTINUE_2;
    m_pOperAddPartyCont2 = new COperAddPartyCont2;
    nStatus              = m_pOperAddPartyCont2->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "RESERVED_PARTY"))
  {
    m_cdr_event_type = RESERVED_PARTY;
    m_pOperAddParty  = new COperAddParty;
    nStatus          = m_pOperAddParty->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "RESERVED_PARTY_1"))
  {
    m_cdr_event_type     = RESERVED_PARTY_CONTINUE_1;
    m_pOperAddPartyCont1 = new COperAddPartyCont1;
    nStatus              = m_pOperAddPartyCont1->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "RESERVED_PARTY_2"))
  {
    m_cdr_event_type     = RESERVED_PARTY_CONTINUE_2;
    m_pOperAddPartyCont2 = new COperAddPartyCont2;
    nStatus              = m_pOperAddPartyCont2->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_UPDATE_PARTY"))
  {
    m_cdr_event_type = OPERATOR_UPDATE_PARTY;
    m_pOperAddParty  = new COperAddParty;
    nStatus          = m_pOperAddParty->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_UPDATE_PARTY_1"))
  {
    m_cdr_event_type     = OPERATOR_UPDATE_PARTY_CONTINUE_1;
    m_pOperAddPartyCont1 = new COperAddPartyCont1;
    nStatus              = m_pOperAddPartyCont1->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_UPDATE_PARTY_CONTINUE_2"))
  {
    m_cdr_event_type     = OPERATOR_UPDATE_PARTY_CONTINUE_2;
    m_pOperAddPartyCont2 = new COperAddPartyCont2;
    nStatus              = m_pOperAddPartyCont2->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "NEW_ISDN_UNDEFINED_PARTY"))
  {
    m_cdr_event_type    = EVENT_NEW_UNDEFINED_PARTY;
    m_pAddPartyDetailed = new CAddPartyDetailed;
    nStatus             = m_pAddPartyDetailed->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "NEW_UNDEFINED_PARTY_CONTINUE_1"))
  {
    m_cdr_event_type     = NEW_UNDEFINED_PARTY_CONTINUE_1;
    m_pOperAddPartyCont2 = new COperAddPartyCont2;
    nStatus              = m_pOperAddPartyCont2->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "PARTY_ERRORS"))
  {
    m_cdr_event_type = PARTY_ERRORS;
    m_pPartyErrors   = new CPartyErrors;
    nStatus          = m_pPartyErrors->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "VISUAL_NAME_CHANGED"))
  {
    m_cdr_event_type   = EVENT_SET_PARTY_VISUAL_NAME;
    m_pPartyVisualName = new CPartySetVisualName;
    nStatus            = m_pPartyVisualName->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "BILLING"))
  {
    m_cdr_event_type    = EVENT_PARTY_ADD_BILLING_CODE;
    m_pPartyBillingCode = new CPartyAddBillingCode;
    nStatus             = m_pPartyBillingCode->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERRATOR_MOVE_PARTY_TO_CONFERENCE_1"))
  {
    m_cdr_event_type  = OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_1;
    m_partyCallingNum = new CCDRPartyCalling_NumMoveToCont1;
    nStatus           = m_partyCallingNum->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "CONF_START_4"))
  {
    m_cdr_event_type  = CONFERENCE_START_CONTINUE_4;
    m_pConfStartCont4 = new CConfStartCont4;
    nStatus           = m_pConfStartCont4->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "CONF_START_5"))
  {
    m_cdr_event_type  = CONFERENCE_START_CONTINUE_5;
    m_pConfStartCont5 = new CConfStartCont5;
    nStatus           = m_pConfStartCont5->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "CONF_START_10"))
  {
    m_cdr_event_type   = CONFERENCE_START_CONTINUE_10;
    m_pConfStartCont10 = new CConfStartCont10;
    nStatus            = m_pConfStartCont10->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "USER_DEFINED_INFORMATION"))
  {
    m_cdr_event_type         = USER_DEFINED_INFORMATION;
    m_pUpdateUserDefinedInfo = new CUpdateUserDefinedInfo;
    nStatus                  = m_pUpdateUserDefinedInfo->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "DTMF_CODE_FAILURE"))
  {
    m_cdr_event_type = DTMF_CODE_FAILURE;
    m_DTMFfailureInd = new CCDRPartyDTMFfailureIndication;
    nStatus          = m_DTMFfailureInd->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "OPERATOR_MOVE_PARTY_TO_CONFERENCE_2"))
  {
    m_cdr_event_type = OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_2;
    m_partyCalledNum = new CCDRPartyCalled_NumMoveToCont2;
    nStatus          = m_partyCalledNum->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "RECORDING_LINK"))
  {
    m_cdr_event_type = RECORDING_LINK_EVENT;
    m_recording      = new CCDRPartyRecording;
    nStatus          = m_recording->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "RECORDING_SYSTEM_LINK"))
  {
    m_cdr_event_type  = RECORDING_SYSTEM_LINK_EVENT;
    m_systemRecording = new CCDRPartySystemRecording;
    nStatus           = m_systemRecording->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "SIP_PRIVATE_EXTENSIONS"))
  {
    m_cdr_event_type       = SIP_PRIVATE_EXTENSIONS_EVENT;
    m_sipPrivateExtensions = new CCDRSipPrivateExtensions;
    nStatus                = m_sipPrivateExtensions->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "GK_INFO"))
  {
    m_cdr_event_type = GK_INFO;
    m_gkInfo         = new CGkInfo;
    nStatus          = m_gkInfo->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "PARTICIPANT CONNECTION RATE"))
  {
    m_cdr_event_type = PARTY_NEW_RATE;
    m_NewRateInfo    = new CNewRateInfo;
    nStatus          = m_NewRateInfo->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if( !strcmp(pszChildName,"PARTICIPANT MAX USAGE INFO"))
  {
	m_cdr_event_type = PARTICIPANT_MAX_USAGE_INFO;
	m_CallInfo = new CCallInfo;
	nStatus = m_CallInfo->DeSerializeXml(pChildNode,pszError);
	if (nStatus != STATUS_OK)
		return nStatus;
  }
  else if( !strcmp(pszChildName,"PARTY_CORRELATION_DATA"))
  {
	m_cdr_event_type = PARTY_CORRELATION_DATA;
	m_CPartyCorrelationData = new CPartyCorrelationData;
	nStatus = m_CPartyCorrelationData->DeSerializeXml(pChildNode,pszError);
	if (nStatus != STATUS_OK)
		return nStatus;
  }
  else if( !strcmp(pszChildName,"CONF_CORRELATION_DATA"))
  {
	m_cdr_event_type = CONF_CORRELATION_DATA;
	m_CConfCorrelationData = new CConfCorrelationData;
	nStatus = m_CConfCorrelationData->DeSerializeXml(pChildNode,pszError);
	if (nStatus != STATUS_OK)
		return nStatus;
  }
  else if (!strcmp(pszChildName, "EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS"))
  {
    m_cdr_event_type      = EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS;
    m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1;
    nStatus               = m_pOperIpV6PartyCont1->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS"))
  {
    m_cdr_event_type      = RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS;
    m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1;
    nStatus               = m_pOperIpV6PartyCont1->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS"))
  {
    m_cdr_event_type      = USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS;
    m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1;
    nStatus               = m_pOperIpV6PartyCont1->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS"))
  {
    m_cdr_event_type      = USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS;
    m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1;
    nStatus               = m_pOperIpV6PartyCont1->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "PARTY_CHAIR_UPDATE"))
  {
    m_cdr_event_type = PARTY_CHAIR_UPDATE;
    if (m_pCDRPartyChairPerson != NULL)
    {
      PDELETE(m_pCDRPartyChairPerson);
    }

    m_pCDRPartyChairPerson = new CCDRPartyChairPerson;
    nStatus                = m_pCDRPartyChairPerson->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (!strcmp(pszChildName, "UNRECOGNIZED_EVENT"))
  {
    m_cdr_event_type = UNRECOGNIZED_EVENT;
  }

  GET_VALIDATE_CHILD(pActionNode, "TIME_STAMP", &m_time_stamp, DATE_TIME);

  pszError[0] = '\0';
  return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetConferenceStart(const CConfStart* otherConfStart)
{
  m_pConfStart = new CConfStart(*otherConfStart);
}

//--------------------------------------------------------------------------
CConfStart* CCdrEvent::GetConferenceStart()
{
  if (m_pConfStart == NULL)
    return NULL;

  CConfStart* otherConfStart = new CConfStart(*m_pConfStart);
  return otherConfStart;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetNetChanlConnect(const CNetChanlCon* otherNetChanlCon)
{
  m_pNetChanlCon = new CNetChanlCon(*otherNetChanlCon);
}

//--------------------------------------------------------------------------
CNetChanlCon* CCdrEvent::GetNetChanlConnect()
{
  if (m_pNetChanlCon == NULL)
    return NULL;

  CNetChanlCon* otherNetChanlCon = new CNetChanlCon(*m_pNetChanlCon);
  return otherNetChanlCon;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetIpChanlConnect(const CIpChanlCon* otherIpChanlCon)
{
  m_pIpChanlCon = new CIpChanlCon(*otherIpChanlCon);
}

//--------------------------------------------------------------------------
CIpChanlCon* CCdrEvent::GetIpChanlConnect()
{
  if (m_pIpChanlCon == NULL)
    return NULL;

  CIpChanlCon* otherIpChanlCon = new CIpChanlCon(*m_pIpChanlCon);
  return otherIpChanlCon;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetNetChanlDisconnect(const CNetChannelDisco* otherNetChanlDisco)
{
  m_pNetChannelDisco = new CNetChannelDisco(*otherNetChanlDisco);
}

//--------------------------------------------------------------------------
CNetChannelDisco* CCdrEvent::GetNetChanlDisconnect()
{
  if (m_pNetChannelDisco == NULL)
    return NULL;

  CNetChannelDisco* otherNetChanlDisco = new CNetChannelDisco(*m_pNetChannelDisco);
  return otherNetChanlDisco;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetPartyConnect(const CPartyConnected* otherPartyConnect)
{
  m_pPartyConnected = new CPartyConnected(*otherPartyConnect);
}

//--------------------------------------------------------------------------
CPartyConnected* CCdrEvent::GetPartyConnect()
{
  if (m_pPartyConnected == NULL)
    return NULL;

  CPartyConnected* otherPartyConnect = new CPartyConnected(*m_pPartyConnected);
  return otherPartyConnect;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetSvcSipPartyConnect(const CSvcSipPartyConnected* otherSvcSipPartyConnect)
{
  m_pSvcSipPartyConnected = new CSvcSipPartyConnected(*otherSvcSipPartyConnect);
}

//--------------------------------------------------------------------------
CSvcSipPartyConnected* CCdrEvent::GetSvcSipPartyConnect()
{
  if (m_pSvcSipPartyConnected == NULL)
    return NULL;

  CSvcSipPartyConnected* otherSvcSipPartyConnect = new CSvcSipPartyConnected(*m_pSvcSipPartyConnected);
  return otherSvcSipPartyConnect;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetPartyDisconnect(const CPartyDisconnected* otherPartyDisconnect)
{
  m_pPartyDisconect = new CPartyDisconnected(*otherPartyDisconnect);
}

//--------------------------------------------------------------------------
CPartyDisconnected* CCdrEvent::GetPartyDisconnect()
{
  if (m_pPartyDisconect == NULL)
    return NULL;

  CPartyDisconnected* otherPartyDisconnect = new CPartyDisconnected(*m_pPartyDisconect);
  return otherPartyDisconnect;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetRemoteComMode(const CRemoteComMode* otherRemoteComMode)
{
  m_pRemoteComMode = new CRemoteComMode(*otherRemoteComMode);
}

//--------------------------------------------------------------------------
CRemoteComMode* CCdrEvent::GetRemoteComMode()
{
  if (m_pRemoteComMode == NULL)
    return NULL;

  CRemoteComMode* otherRemoteComMode = new CRemoteComMode(*m_pRemoteComMode);
  return otherRemoteComMode;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetPartyErrors(const CPartyErrors* otherPartyErrors)
{
  m_pPartyErrors = new CPartyErrors(*otherPartyErrors);
}

//--------------------------------------------------------------------------
CPartyErrors* CCdrEvent::GetPartyErrors()
{
  if (m_pPartyErrors == NULL)
    return NULL;

  CPartyErrors* otherPartyErrors = new CPartyErrors(*m_pPartyErrors);
  return otherPartyErrors;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetAddReservUpdatParty(const COperAddParty* otherAddParty)
{
  m_pOperAddParty = new COperAddParty(*otherAddParty);
}

//--------------------------------------------------------------------------
COperAddParty* CCdrEvent::GetAddReservUpdatParty()
{
  if (m_pOperAddParty == NULL)
    return NULL;

  COperAddParty* otherAddParty = new COperAddParty(*m_pOperAddParty);
  return otherAddParty;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetAddUnReservUpdatParty(const CAddPartyDetailed* otherAddParty)
{
  m_pAddPartyDetailed = new CAddPartyDetailed(*otherAddParty);
}

//--------------------------------------------------------------------------
CAddPartyDetailed* CCdrEvent::GetAddUnReservUpdatParty()
{
  if (m_pAddPartyDetailed == NULL)
    return NULL;

  CAddPartyDetailed* otherAddParty = new CAddPartyDetailed(*m_pAddPartyDetailed);
  return otherAddParty;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetDelDisconctReconctParty(const COperDelParty* otherDelParty)
{
  m_pOperDelParty = new COperDelParty(*otherDelParty);
}

//--------------------------------------------------------------------------
COperDelParty* CCdrEvent::GetDelDisconctReconctParty()
{
  if (m_pOperDelParty == NULL)
    return NULL;

  COperDelParty* otherDelParty = new COperDelParty(*m_pOperDelParty);
  return otherDelParty;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetEndTimeEvent(const COperSetEndTime* otherSetEndTime)
{
  m_pOperSetEndTime = new COperSetEndTime(*otherSetEndTime);
}

//--------------------------------------------------------------------------
COperSetEndTime* CCdrEvent::GetEndTimeEvent()
{
  if (m_pOperSetEndTime == NULL)
    return NULL;

  COperSetEndTime* otherSetEndTime = new COperSetEndTime(*m_pOperSetEndTime);
  return otherSetEndTime;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetMPIChanlCon(const CMPIChanlCon* otherSetMPIChanlCon)
{
  m_pMPIChanlCon = new CMPIChanlCon(*otherSetMPIChanlCon);
}

//--------------------------------------------------------------------------
CMPIChanlCon* CCdrEvent::GetMPIChanlCon()
{
  if (m_pMPIChanlCon == NULL)
    return NULL;

  CMPIChanlCon* otherMPIChanlCon = new CMPIChanlCon(*m_pMPIChanlCon);
  return otherMPIChanlCon;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetConfStartCont1(const CConfStartCont1* otherSetConfStartCont1)
{
  m_pConfStartCont1 = new CConfStartCont1(*otherSetConfStartCont1);
}

//--------------------------------------------------------------------------
CConfStartCont1* CCdrEvent::GetConfStartCont1()
{
  if (m_pConfStartCont1 == NULL)
    return NULL;

  CConfStartCont1* otherConfStartCont1 = new CConfStartCont1(*m_pConfStartCont1);
  return otherConfStartCont1;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetConfStartCont2(const CConfStartCont2* otherSetConfStartCont2)
{
  m_pConfStartCont2 = new CConfStartCont2(*otherSetConfStartCont2);
}

//--------------------------------------------------------------------------
CConfStartCont2* CCdrEvent::GetConfStartCont2()
{
  if (m_pConfStartCont2 == NULL)
    return NULL;

  CConfStartCont2* otherConfStartCont2 = new CConfStartCont2(*m_pConfStartCont2);
  return otherConfStartCont2;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetConfStartCont3(const CConfStartCont3* otherSetConfStartCont3)
{
  m_pConfStartCont3 = new CConfStartCont3(*otherSetConfStartCont3);
}

//--------------------------------------------------------------------------
CConfStartCont3* CCdrEvent::GetConfStartCont3()
{
  if (m_pConfStartCont3 == NULL)
    return NULL;

  CConfStartCont3* otherConfStartCont3 = new CConfStartCont3(*m_pConfStartCont3);
  return otherConfStartCont3;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetOperAddPartyCont1(const COperAddPartyCont1* otherOperAddPartyCont1)
{
  m_pOperAddPartyCont1 = new COperAddPartyCont1(*otherOperAddPartyCont1);
}

//--------------------------------------------------------------------------
COperAddPartyCont1* CCdrEvent::GetOperAddPartyCont1()
{
  if (m_pOperAddPartyCont1 == NULL)
    return NULL;

  COperAddPartyCont1* otherOperAddPartyCont1 = new COperAddPartyCont1(*m_pOperAddPartyCont1);
  return otherOperAddPartyCont1;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetOperAddPartyCont2(const COperAddPartyCont2* otherOperAddPartyCont2)
{
  m_pOperAddPartyCont2 = new COperAddPartyCont2(*otherOperAddPartyCont2);
}

//--------------------------------------------------------------------------
COperAddPartyCont2* CCdrEvent::GetOperAddPartyCont2()
{
  if (m_pOperAddPartyCont2 == NULL)
    return NULL;

  COperAddPartyCont2* otherOperAddPartyCont2 = new COperAddPartyCont2(*m_pOperAddPartyCont2);
  return otherOperAddPartyCont2;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetOperMoveParty(const COperMoveParty* otherOperMoveParty)
{
  m_pOperMoveParty = new COperMoveParty(*otherOperMoveParty);
}

//--------------------------------------------------------------------------
COperMoveParty* CCdrEvent::GetOperMoveParty()
{
  if (m_pOperMoveParty == NULL)
    return NULL;

  COperMoveParty* otherOperMoveParty = new COperMoveParty(*m_pOperMoveParty);
  return otherOperMoveParty;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetOperMoveToConf(const COperMoveToConf* otherOperMoveToConf)
{
  m_pOperMoveToConf = new COperMoveToConf(*otherOperMoveToConf);
}

//--------------------------------------------------------------------------
COperMoveToConf* CCdrEvent::GetOperMoveToConf()
{
  if (m_pOperMoveToConf == NULL)
    return NULL;

  COperMoveToConf* otherOperMoveToConf = new COperMoveToConf(*m_pOperMoveToConf);
  return otherOperMoveToConf;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetPartyDisconectCont1(const CPartyDisconnectedCont1* otherPartyDisconectCont1)
{
  m_pPartyDisconectCont1 = new CPartyDisconnectedCont1(*otherPartyDisconectCont1);
}

//--------------------------------------------------------------------------
CPartyDisconnectedCont1* CCdrEvent::GetPartyDisconectCont1()
{
  if (m_pPartyDisconectCont1 == NULL)
    return NULL;

  CPartyDisconnectedCont1* otherPartyDisconectCont1 = new CPartyDisconnectedCont1(*m_pPartyDisconectCont1);
  return otherPartyDisconectCont1;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetPartyBillingCode(const CPartyAddBillingCode* otherPartyBillingCode)
{
  m_pPartyBillingCode = new CPartyAddBillingCode(*otherPartyBillingCode);
}

//--------------------------------------------------------------------------
CPartyAddBillingCode* CCdrEvent::GetPartyBillingCode()
{
  if (m_pPartyBillingCode == NULL)
    return NULL;

  CPartyAddBillingCode* otherBillingCode = new CPartyAddBillingCode(*m_pPartyBillingCode);
  return otherBillingCode;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetPartyVisualName(const CPartySetVisualName* otherPartyVisualName)
{
  m_pPartyVisualName = new CPartySetVisualName(*otherPartyVisualName);
}

//--------------------------------------------------------------------------
CPartySetVisualName* CCdrEvent::GetPartyVisualName()
{
  if (m_pPartyVisualName == NULL)
    return NULL;

  CPartySetVisualName* otherVisualName = new CPartySetVisualName(*m_pPartyVisualName);
  return otherVisualName;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetPartyCalling_Num(const CCDRPartyCalling_NumMoveToCont1* otherPartyCalling_Num)
{
  m_partyCallingNum = new CCDRPartyCalling_NumMoveToCont1(*otherPartyCalling_Num);
}

//--------------------------------------------------------------------------
CCDRPartyCalling_NumMoveToCont1* CCdrEvent::GetPartyPartyCalling_Num()
{
  if (m_partyCallingNum == NULL)
    return NULL;

  CCDRPartyCalling_NumMoveToCont1* otherPartyCalling_Num = new CCDRPartyCalling_NumMoveToCont1(*m_partyCallingNum);
  return otherPartyCalling_Num;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetPartyCalled_Num(const CCDRPartyCalled_NumMoveToCont2* otherPartyCalled_Num)
{
  m_partyCalledNum = new CCDRPartyCalled_NumMoveToCont2(*otherPartyCalled_Num);
}

//--------------------------------------------------------------------------
CCDRPartyCalled_NumMoveToCont2* CCdrEvent::GetPartyPartyCalled_Num()
{
  if (m_partyCalledNum == NULL)
    return NULL;

  CCDRPartyCalled_NumMoveToCont2* otherPartyCalled_Num = new CCDRPartyCalled_NumMoveToCont2(*m_partyCalledNum);
  return otherPartyCalled_Num;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetOperIpV6PartyCont1(const COperIpV6PartyCont1* otherOperIpV6PartyCont1)
{
  m_pOperIpV6PartyCont1 = new COperIpV6PartyCont1(*otherOperIpV6PartyCont1);
}

//--------------------------------------------------------------------------
COperIpV6PartyCont1* CCdrEvent::GetOperIpV6PartyCont1()
{
  if (m_pOperIpV6PartyCont1 == NULL)
    return NULL;

  COperIpV6PartyCont1* otherOperIpV6PartyCont1 = new COperIpV6PartyCont1(*m_pOperIpV6PartyCont1);
  return otherOperIpV6PartyCont1;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetConfStartCont4(const CConfStartCont4* pConfStartCont4)
{
  m_pConfStartCont4 = new CConfStartCont4(*pConfStartCont4);
}

//--------------------------------------------------------------------------
CConfStartCont4* CCdrEvent::GetConfStartCont4()
{
  if (m_pConfStartCont4 == NULL)
    return NULL;

  CConfStartCont4* otherConfContactInfo = new CConfStartCont4(*m_pConfStartCont4);
  return otherConfContactInfo;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetConfStartCont5(const CConfStartCont5* otherSetConfStartCont5)
{
  m_pConfStartCont5 = new CConfStartCont5(*otherSetConfStartCont5);
}

//--------------------------------------------------------------------------
CConfStartCont5* CCdrEvent::GetConfStartCont5()
{
  if (m_pConfStartCont5 == NULL)
    return NULL;

  CConfStartCont5* otherConfStartCont5 = new CConfStartCont5(*m_pConfStartCont5);
  return otherConfStartCont5;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetConfStartCont10(const CConfStartCont10* pConfStartCont10)
{
  m_pConfStartCont10 = new CConfStartCont10(*pConfStartCont10);
}

//--------------------------------------------------------------------------
CConfStartCont10* CCdrEvent::GetConfStartCont10()
{
  if (m_pConfStartCont10 == NULL)
    return NULL;

  CConfStartCont10* otherConfContactInfo = new CConfStartCont10(*m_pConfStartCont10);
  return otherConfContactInfo;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetUserDefinedInfo(const CUpdateUserDefinedInfo* pUpdateUserDefinedInfo)
{
  m_pUpdateUserDefinedInfo = new CUpdateUserDefinedInfo(*pUpdateUserDefinedInfo);
}

//--------------------------------------------------------------------------
CUpdateUserDefinedInfo* CCdrEvent::GetUserDefinedInfo()
{
  if (m_pUpdateUserDefinedInfo == NULL)
    return NULL;

  CUpdateUserDefinedInfo* otherUserDefinedInfo = new CUpdateUserDefinedInfo(*m_pUpdateUserDefinedInfo);
  return otherUserDefinedInfo;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetDTMFfailureInd(const CCDRPartyDTMFfailureIndication* pDTMFfailureInd)
{
  m_DTMFfailureInd = new CCDRPartyDTMFfailureIndication(*pDTMFfailureInd);
}

//--------------------------------------------------------------------------
CCDRPartyDTMFfailureIndication* CCdrEvent::GetDTMFfailureInd()
{
  if (m_DTMFfailureInd == NULL)
    return NULL;

  CCDRPartyDTMFfailureIndication* otherDTMFfailureInd = new CCDRPartyDTMFfailureIndication(*m_DTMFfailureInd);
  return otherDTMFfailureInd;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetRecording(const CCDRPartyRecording* pRecording)
{
  m_recording = new CCDRPartyRecording(*pRecording);
}

//--------------------------------------------------------------------------
CCDRPartyRecording* CCdrEvent::GetRecording()
{
  if (m_recording == NULL)
    return NULL;

  CCDRPartyRecording* otherrecording = new CCDRPartyRecording(*m_recording);
  return otherrecording;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetSystemRecording(const CCDRPartySystemRecording* pSystemRecording)
{
  m_systemRecording = new CCDRPartySystemRecording(*pSystemRecording);
}

//--------------------------------------------------------------------------
CCDRPartySystemRecording* CCdrEvent::GetSystemRecording()
{
  if (m_systemRecording == NULL)
    return NULL;

  CCDRPartySystemRecording* otherSystemRecording = new CCDRPartySystemRecording(*m_systemRecording);
  return otherSystemRecording;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetSipPrivateExtensions(const CCDRSipPrivateExtensions* pSipPrivateExt)
{
  m_sipPrivateExtensions = new CCDRSipPrivateExtensions(*pSipPrivateExt);
}

//--------------------------------------------------------------------------
CCDRSipPrivateExtensions* CCdrEvent::GetSipPrivateExtensions()
{
  if (m_sipPrivateExtensions == NULL)
    return NULL;

  CCDRSipPrivateExtensions* otherSipPrivateExt = new CCDRSipPrivateExtensions(*m_sipPrivateExtensions);
  return otherSipPrivateExt;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetGkInfo(const CGkInfo* otherGkInfo)
{
  m_gkInfo = new CGkInfo(*otherGkInfo);
}

//--------------------------------------------------------------------------
CGkInfo* CCdrEvent::GetGkInfo()
{
  if (m_gkInfo == NULL)
    return NULL;

  CGkInfo* otherGkInfo = new CGkInfo(*m_gkInfo);
  return otherGkInfo;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetNewRateInfo(const CNewRateInfo* otherNewRateInfo)
{
  m_NewRateInfo = new CNewRateInfo(*otherNewRateInfo);
}

//--------------------------------------------------------------------------
CNewRateInfo* CCdrEvent::GetNewRateInfo()
{
  if (m_NewRateInfo == NULL)
    return NULL;

  CNewRateInfo* otherNewRateInfo = new CNewRateInfo(*m_NewRateInfo);
  return otherNewRateInfo;
}

//--------------------------------------------------------------------------
void CCdrEvent::SetPartyChairPerson(const CCDRPartyChairPerson* otherPartyChairPerson)
{
  if (m_pCDRPartyChairPerson != NULL)
  {
    PDELETE(m_pCDRPartyChairPerson);
  }

  m_pCDRPartyChairPerson = new CCDRPartyChairPerson(*otherPartyChairPerson);
}

//--------------------------------------------------------------------------
CCDRPartyChairPerson* CCdrEvent::GetPartyChairPerson()
{
  if (m_pCDRPartyChairPerson == NULL)
    return NULL;

  CCDRPartyChairPerson* otherPartyChairPerson = new CCDRPartyChairPerson(*m_pCDRPartyChairPerson);
  return otherPartyChairPerson;
}

////////////////////////////////////////////////////////////////////////////
//m_CallInfo
////////////////////////////////////////////////////////////////////////////////
void  CCdrEvent::SetCallInfo(const CCallInfo* otherCallInfo)
{
	m_CallInfo = new CCallInfo(*otherCallInfo);
}

////////////////////////////////////////////////////////////////////////////////
CCallInfo*  CCdrEvent::GetCallInfo()
{
	if (m_CallInfo == NULL)
    	return  NULL;

	CCallInfo* otherCallInfo = new CCallInfo(*m_CallInfo);
	return otherCallInfo;
}

//CPartyCorrelationData
////////////////////////////////////////////////////////////////////////////////
CPartyCorrelationData*  CCdrEvent::GetCPartyCorrelationData()
{
	if (m_CPartyCorrelationData == NULL)
    	return  NULL;

	CPartyCorrelationData* otherCPartyCorrelationData= new CPartyCorrelationData(*m_CPartyCorrelationData);
	return otherCPartyCorrelationData;
}
////////////////////////////////////////////////////////////////////////////////
void  CCdrEvent::SetCPartyCorrelationData(const CPartyCorrelationData* otherCPartyCorrelationData)
{
	m_CPartyCorrelationData = new CPartyCorrelationData(*otherCPartyCorrelationData);
}

//CConfCorrelationData
////////////////////////////////////////////////////////////////////////////////
CConfCorrelationData*  CCdrEvent::GetCConfCorrelationData()
{
	if (m_CConfCorrelationData == NULL)
    	return  NULL;

	CConfCorrelationData* otherCConfCorrelationData= new CConfCorrelationData(*m_CConfCorrelationData);
	return otherCConfCorrelationData;
}
////////////////////////////////////////////////////////////////////////////////
void  CCdrEvent::SetCConfCorrelationData(const CConfCorrelationData* otherCConfCorrelationData)
{
	m_CConfCorrelationData = new CConfCorrelationData(*otherCConfCorrelationData);
}
//--------------------------------------------------------------------------
bool CCdrEvent::IsValidEvent(WORD cdrEventType)
{
  switch (cdrEventType)
  {
    case CONFERENCE_START:
    case CONFERENCE_START_CONTINUE_1:
    case CONFERENCE_START_CONTINUE_2:
    case CONFERENCE_START_CONTINUE_3:
    case CONFERENCE_START_CONTINUE_10:
    case NET_CHANNEL_CONNECTED:
    case H323_CALL_SETUP:
    case SIP_CALL_SETUP:
    case NET_CHANNEL_DISCONNECTED:
    case CONFERENCES_END:
    case EVENT_PARTY_DISCONNECTED:
    case PARTY_DISCONNECTED_CONTINUE_1:
    case REMOTE_COM_MODE:
    case EVENT_PARTY_CONNECTED:
    case IP_PARTY_CONNECTED:
    case SIP_PARTY_CONNECTED:
    case SVC_SIP_PARTY_CONNECTED:
    case OPERATOR_TERMINATE:
    case OPERATOR_SET_END_TIME:
    case OPERATORS_DELETE_PARTY:
    case OPERATOR_DISCONNECTE_PARTY:
    case OPERATOR_RECONNECT_PARTY:
    case OPERRATOR_ATTEND_PARTY:
    case OPERRATOR_MOVE_PARTY_FROM_CONFERENCE:
    case OPERRATOR_BACK_TO_CONFERENCE_PARTY:
    case OPERRATOR_ONHOLD_PARTY:
    case OPERRATOR_ATTEND_PARTY_TO_CONFERENCE:
    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE:
    case OPERATOR_ADD_PARTY:
    case RESERVED_PARTY:
    case OPERATOR_UPDATE_PARTY:
    case RESERVED_PARTY_CONTINUE_1:
    case RESERVED_PARTY_CONTINUE_2:
    case OPERATOR_ADD_PARTY_CONTINUE_1:
    case OPERATOR_ADD_PARTY_CONTINUE_2:
    case OPERATOR_UPDATE_PARTY_CONTINUE_1:
    case OPERATOR_UPDATE_PARTY_CONTINUE_2:
    case EVENT_NEW_UNDEFINED_PARTY:
    case NEW_UNDEFINED_PARTY_CONTINUE_1:
    case PARTY_ERRORS:
    case EVENT_SET_PARTY_VISUAL_NAME:
    case EVENT_PARTY_ADD_BILLING_CODE:
    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_1:
    case CONFERENCE_START_CONTINUE_4:
    case CONFERENCE_START_CONTINUE_5:
    case USER_DEFINED_INFORMATION:
    case DTMF_CODE_FAILURE:
    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_2:
    case RECORDING_LINK_EVENT:
    case RECORDING_SYSTEM_LINK_EVENT:
    case SIP_PRIVATE_EXTENSIONS_EVENT:
    case GK_INFO:
    case PARTY_NEW_RATE:
    case EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS:
    case RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    case USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    case USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    case PARTY_CHAIR_UPDATE:
	case PARTICIPANT_MAX_USAGE_INFO:
	case PARTY_CORRELATION_DATA:
	case CONF_CORRELATION_DATA:
    case UNRECOGNIZED_EVENT:
    {
      return true;
    }

    default:
    {
      FPTRACE2INT(eLevelInfoNormal, "CCdrLogApi::IsValidEvent - event not valid, event =  ", cdrEventType);
      return false;
    }
  } // switch
}


////////////////////////////////////////////////////////////////////////////
//                        CCdrLongStruct
////////////////////////////////////////////////////////////////////////////
CCdrLongStruct::CCdrLongStruct()
{
  memset(m_pCdrEvent, 0, sizeof(CCdrEvent*)*MAX_CDR_EVENT_IN_LIST);

  m_pCdrShort              = new CCdrShort;
  m_numof_cdr_event_struct = 0;
  m_length                 = 0;
  m_string                 = NULL;
  m_pArrayOfString         = NULL;
  m_array_of_string_size   = 0;
}

//--------------------------------------------------------------------------
CCdrLongStruct::CCdrLongStruct(const CCdrLongStruct& other) : CPObject(other)
{
  memset(m_pCdrEvent, 0, sizeof(CCdrEvent*)*MAX_CDR_EVENT_IN_LIST);

  m_pCdrShort              = NULL;
  m_numof_cdr_event_struct = 0;

  *this = other;
}

//--------------------------------------------------------------------------
CCdrLongStruct::~CCdrLongStruct()
{
  POBJDELETE(m_pCdrShort);
  for (int i = 0; i < MAX_CDR_EVENT_IN_LIST; i++)
  {
    if (m_pCdrEvent[i] != NULL)
      PDELETE(m_pCdrEvent[i]);
  }

  PDELETE(m_string);

  if (NULL != m_pArrayOfString)
  {
    for (DWORD j = 0; j < m_array_of_string_size; j++)
    {
      delete [] m_pArrayOfString[j];
    }
    delete [] m_pArrayOfString;
  }
}

//--------------------------------------------------------------------------
CCdrLongStruct& CCdrLongStruct::operator=(const CCdrLongStruct& other)
{
  if (this == &other)
  {
    return *this;
  }

  if (m_pCdrShort == NULL && other.m_pCdrShort == NULL)
  {
  }
  else if (m_pCdrShort != NULL && other.m_pCdrShort == NULL)
  {
    POBJDELETE(m_pCdrShort);
    m_pCdrShort = NULL;
  }
  else if (m_pCdrShort == NULL && other.m_pCdrShort != NULL)
  {
    m_pCdrShort = new CCdrShort(*other.m_pCdrShort);
  }
  else
  {
    *m_pCdrShort =  *other.m_pCdrShort;
  }

  m_numof_cdr_event_struct = other.m_numof_cdr_event_struct;
  for (int i = 0; i < MAX_CDR_EVENT_IN_LIST; i++)
  {
    if (m_pCdrEvent[i] == NULL && other.m_pCdrEvent[i] == NULL)
    {
    }
    else if (m_pCdrEvent[i] != NULL && other.m_pCdrEvent[i] == NULL)
    {
      delete m_pCdrEvent[i];
      m_pCdrEvent[i] = NULL;
    }
    else if (m_pCdrEvent[i] == NULL && other.m_pCdrEvent[i] != NULL)
    {
      m_pCdrEvent[i] = new CCdrEvent(*other.m_pCdrEvent[i]);
    }
    else
    {
      *m_pCdrEvent[i] =  *other.m_pCdrEvent[i];
    }
  }

  m_length = other.m_length;

/* The Operator can allocate m_string of any size of m_length
   The mcms will allocate m_string only for old cases - in those cases
   mcms won`t let m_length to increase 65KB.
   For new cases (API_NUM_CDR_HUGE)m_string is not allocated. An array of pointers is allocated
   instade of it.
*/
  if ((other.m_pArrayOfString == NULL) && (other.m_string != NULL))
  {
    m_string = new char[(WORD)m_length + 1];
    strcpy(m_string, other.m_string);
    m_array_of_string_size = other.m_array_of_string_size;
  }
  else // new case
  {
    m_array_of_string_size = CalcArrOfStringLength();
    m_pArrayOfString       = new char*[m_array_of_string_size];
    for (DWORD j = 0; j < m_array_of_string_size; j++)
    {
      m_pArrayOfString[j] = new char[CDR_BUF_SIZE];
      memset(m_pArrayOfString[j], ' ', CDR_BUF_SIZE);

      if (other.m_pArrayOfString != NULL && other.m_pArrayOfString[j] != NULL)
      {
        memcpy(m_pArrayOfString[j], other.m_pArrayOfString[j], CDR_BUF_SIZE);
      }
      else
      {
        PASSERT(1);
      }
    }
  }
  return *this;
}

//--------------------------------------------------------------------------
void CCdrLongStruct::SerializeString(WORD format, std::ostream& ostr, DWORD apiNum)
{
  if ((apiNum < API_NUM_CDR_HUGE_3) || ((apiNum > MAX_API_NUM_VER_3) && (apiNum < API_NUM_CDR_HUGE)))
    ostr << (WORD)m_length << "\n";     // casting to WORD since from API_NUM_CDR_HUGE m_length is DWORD
  else
    ostr << m_length << "\n";

  char* ancher = new char[m_length+1];
  memcpy(ancher, m_string, m_length);
  ancher[m_length] = '\0';
  ostr << ancher << "~";
  delete [] ancher;
}

//--------------------------------------------------------------------------
// NOT IN USE ???
void CCdrLongStruct::DeSerializeToString(WORD format, std::istream& istr)
{
  istr >> m_length;
  istr.ignore(1);
  m_string = new char[m_length+1];
  istr.get(m_string, m_length, '~');
  istr.ignore(1);
}

//--------------------------------------------------------------------------
// this function in use only from api API_NUM_CDR_HUGE
void CCdrLongStruct::SerializePtrStringArray(WORD format, std::ostream& ostr)
{
  ostr << m_length <<"\n";
  ostr << (DWORD)(m_pArrayOfString) <<"\n";
  ostr << "~";
}

//--------------------------------------------------------------------------
// this function in use only from api API_NUM_CDR_HUGE.
void CCdrLongStruct::DeSerializePtrStringArray(WORD format, std::istream& istr)
{
  istr >> m_length;  // here m_length is the file length

  DWORD   array_of_string_size = CalcArrOfStringLength();
  m_pArrayOfString = new char*[array_of_string_size];

  istr >> (DWORD&)(m_pArrayOfString);

}

//--------------------------------------------------------------------------
// NOT IN USE ????
void CCdrLongStruct::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  WORD  semi_count             = 0;
  WORD  numof_cdr_event_struct = 0;
  char* str                    = m_string;
  while (str != NULL)
  {
    str = strchr(str, ';');
    if (str != NULL)
    {
      str += 1;
      semi_count++;
    }
  }

  m_numof_cdr_event_struct = (semi_count-1); // because of cdrshort
  m_pCdrShort->DeSerialize(format, istr, apiNum);
  istr.ignore(1);

  // 18.03.99 , for offline formating.
  if (apiNum == 0)
    apiNum = m_pCdrShort->GetFileVersion();

  numof_cdr_event_struct = m_numof_cdr_event_struct;
  if (MAX_CDR_EVENT_IN_LIST < m_numof_cdr_event_struct)
    numof_cdr_event_struct = MAX_CDR_EVENT_IN_LIST;

  for (int i = 0; i < numof_cdr_event_struct; i++)
  {
    m_pCdrEvent[i] = new CCdrEvent;
    m_pCdrEvent[i]->DeSerialize(format, istr, apiNum);
  }
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
void CCdrLongStruct::SerializeXml(CXMLDOMElement* pFatherNode, bool no_partID)
{
  CXMLDOMElement* pFullNode = pFatherNode->AddChildNode("CDR_FULL");

  if (no_partID)
  {
    m_pCdrShort->SerializeXmlWithoutPartID(pFullNode);
    TRACEINTO << "CCdrLongStruct::SerializeXml - "
              << "Serialize CDR for confID " << m_pCdrShort->GetConfId()
              << ", partID " << m_pCdrShort->GetFilePartIndex()
              << " without partID";
  }
  else
  {
	  m_pCdrShort->SerializeXml(pFullNode);
  }

  DWORD       nStringArrays = CalcArrOfStringLength();
  DWORD       nNextArray    = 0, nArrayOffset = 0, nBufferOffset = 0;
  static char pszEventBuffer[CDR_BUF_SIZE];
  bool        isFirstLine = true;


  memset(pszEventBuffer, '\0', CDR_BUF_SIZE);
  while (nNextArray < nStringArrays && m_length > nNextArray*CDR_BUF_SIZE+nArrayOffset)
  {
    while (nArrayOffset < CDR_BUF_SIZE && m_length > nNextArray*CDR_BUF_SIZE+nArrayOffset)
    {
      pszEventBuffer[nBufferOffset++] = m_pArrayOfString[nNextArray][nArrayOffset++];

      if (pszEventBuffer[nBufferOffset-1] == ';')
      {
        if (isFirstLine)
        {
          isFirstLine = false;
        }
        else
        {
          CCdrEvent   objCdrEvent;
          CIstrStream istr(pszEventBuffer);
          objCdrEvent.DeSerialize(OPERATOR_MCMS, istr, API_NUMBER);
          objCdrEvent.SerializeXml(pFullNode);
        }

        nBufferOffset = 0;
        memset(pszEventBuffer, '\0', CDR_BUF_SIZE);
      }
    }
    nNextArray++;
    nArrayOffset = 0;
  }
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
void CCdrLongStruct::SerializeXmlUnformat(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pFullNode = pFatherNode->AddChildNode("CDR_FULL_UNFORMATTED");

  int   nStringArrays = CalcArrOfStringLength();
  char* pszBuffer     = new char [CDR_BUF_SIZE+1];
  DWORD chars2sendAll = m_length;
  DWORD chars2sendNow = 0;

  for (int i = 0; i < nStringArrays; i++)
  {
    memset(pszBuffer, '\0', CDR_BUF_SIZE+1);
    chars2sendNow = MIN_(chars2sendAll, CDR_BUF_SIZE);
    strncpy(pszBuffer, m_pArrayOfString[i], chars2sendNow);
    pFullNode->AddChildNode("CDR_LINE", pszBuffer);
    chars2sendAll -= chars2sendNow;
  }
  PDELETEA(pszBuffer);
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CCdrLongStruct::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;
  pszError[0] = '\0';
  return STATUS_OK;
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CCdrLongStruct::DeSerializeXmlUnformat(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;
  return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCdrLongStruct::AddSerializedEvent(char* pString, DWORD strLen)
{
}

//--------------------------------------------------------------------------
int CCdrLongStruct::FindEvent(const WORD cdr_event_type)
{
  for (int i = 0; i < (int)m_numof_cdr_event_struct; i++)
    if (m_pCdrEvent[i] && m_pCdrEvent[i]->GetCdrEventType() == cdr_event_type)
      return i;
  return NOT_FIND;
}

//--------------------------------------------------------------------------
int CCdrLongStruct::FindEvent(const CCdrEvent& other)
{
  WORD cdr_event_type = other.GetCdrEventType();
  return FindEvent(cdr_event_type);
}

//--------------------------------------------------------------------------
int CCdrLongStruct::Add(const CCdrEvent& other)
{
  if (m_numof_cdr_event_struct >= MAX_CDR_EVENT_IN_LIST)
    return STATUS_ILLEGAL;

  m_pCdrEvent[m_numof_cdr_event_struct] = new CCdrEvent(other);
  m_numof_cdr_event_struct++;
  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCdrLongStruct::Update(const CCdrEvent& other)
{
  int ind = FindEvent(other);
  if (ind == NOT_FIND || ind > MAX_CDR_EVENT_IN_LIST-1)
    return CDR_EVENT_NOT_EXISTS;

  PDELETE(m_pCdrEvent[ind]);
  m_pCdrEvent[ind] = new CCdrEvent(other);
  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCdrLongStruct::Cancel(const WORD cdr_event_type)
{
  int ind = FindEvent(cdr_event_type);
  if (ind == NOT_FIND || ind > MAX_CDR_EVENT_IN_LIST-1)
    return CDR_EVENT_NOT_EXISTS;

  if (m_numof_cdr_event_struct > MAX_CDR_EVENT_IN_LIST)
    return STATUS_ILLEGAL;

  POBJDELETE(m_pCdrEvent[ind]);

  int i = 0;
  for (; i < (int)m_numof_cdr_event_struct; i++)
  {
    if (m_pCdrEvent[i] == NULL)
      break;
  }

  for (int j = i; j < (int)m_numof_cdr_event_struct-1; j++)
    m_pCdrEvent[j] = m_pCdrEvent[j+1];

  m_pCdrEvent[m_numof_cdr_event_struct-1] = NULL;
  m_numof_cdr_event_struct--;
  return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCdrLongStruct::SetShortCdrStruct(const CCdrShort* otherCdrShort)
{
  delete m_pCdrShort;
  m_pCdrShort = new CCdrShort(*otherCdrShort);
}

//--------------------------------------------------------------------------
CCdrShort* CCdrLongStruct::GetShortCdrStruct()
{
  return m_pCdrShort;
}

//--------------------------------------------------------------------------
void CCdrLongStruct::SetArrayOfStringPtr(char** ptrStringArr, long fileLength)
{
  m_length = fileLength;
  m_array_of_string_size = CalcArrOfStringLength();
  m_pArrayOfString = new char*[m_array_of_string_size];
  for (DWORD i = 0; i < m_array_of_string_size; i++)
  {
    m_pArrayOfString[i] = new char[CDR_BUF_SIZE];
    memset(m_pArrayOfString[i], ' ', CDR_BUF_SIZE);
    memcpy(m_pArrayOfString[i], ptrStringArr[i], CDR_BUF_SIZE);
  }
}

//--------------------------------------------------------------------------
char** CCdrLongStruct::GetArrayOfStringPtr()
{
  return m_pArrayOfString;
}

//--------------------------------------------------------------------------
DWORD CCdrLongStruct::CalcArrOfStringLength()
{
  DWORD num_of_pointers = 0;

  if (m_length%CDR_BUF_SIZE == 0)
    num_of_pointers = m_length/CDR_BUF_SIZE;
  else
    num_of_pointers = m_length/CDR_BUF_SIZE+1;

  return num_of_pointers;
}

//--------------------------------------------------------------------------
BYTE CCdrLongStruct::CalcLengthOfFileLength()
{
  char*        msg_info = new char[SIZE_STREAM];
  COstrStream* pOstr    = new COstrStream(msg_info);

  *pOstr << m_length  << "\n";

  BYTE b = sizeof(DWORD) + sizeof("\n");

  POBJDELETE(pOstr);

  delete [] msg_info;
  return b;
}


////////////////////////////////////////////////////////////////////////////
//                        CCdrDetailRequest
////////////////////////////////////////////////////////////////////////////
CCdrDetailRequest::CCdrDetailRequest()
{
  m_connection_id = 0;
  m_conference_id = 0;
}

//--------------------------------------------------------------------------
CCdrDetailRequest::CCdrDetailRequest(const CCdrDetailRequest& other) : CPObject(other)
{
  m_connection_id = other.m_connection_id;
  m_conference_id = other.m_conference_id;
}

//--------------------------------------------------------------------------
CCdrDetailRequest::~CCdrDetailRequest()
{
}

//--------------------------------------------------------------------------
void CCdrDetailRequest::Serialize(WORD format, std::ostream& ostr)
{
  ostr << m_connection_id << ",";
  ostr << m_conference_id   << ";\n";
}

//--------------------------------------------------------------------------
void CCdrDetailRequest::DeSerialize(WORD format, std::istream& istr)
{
  // assuming format = OPERATOR_MCMS

  istr >> m_connection_id;
  istr >> m_conference_id;
}
