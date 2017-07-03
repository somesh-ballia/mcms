//+========================================================================+
//                        Copyright 2005 Polycom                           |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
//+========================================================================+
// FILE: ConfAppWaitEventsList.cpp                                         |
// SUBSYSTEM:  ConfParty                                                   |
//+========================================================================+

#include "ConfAppMngr.h"
#include "ConfAppWaitEventsList.h"
#include "StatusesGeneral.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////////////
CWaitingEvent::CWaitingEvent(TConfAppEvents eventType, DWORD partyRsrcID, DWORD tMCUproductType)
{
  m_partyRsrcID = partyRsrcID;
  m_eventType = eventType;
  // EE-462 enable IVR for MFW
//  if (tMCUproductType == eProductTypeSoftMCUMfw)
//  {
//	  switch (m_eventType)
//	  {
//	    case eCAM_EVENT_PARTY_END_IVR:
//	    {
//	      m_deque.push_back(eCAM_EVENT_PARTY_IN_CONF_IND); // can be after "wait for chair" event since it only sends an indication to party control
//	    //  m_deque.push_back(eCAM_EVENT_PARTY_END_VIDEO_IVR); // can be after "wait for chair" event since it concerns only the video bridge
//	      m_deque.push_back(eCAM_EVENT_PARTY_LAST_DUMMY_FEATURE);
//	      return;
//	    }
//	    case eCAM_EVENT_PARTY_DELETED:
//	    case eCAM_EVENT_PARTY_AUDIO_MOVE_OUT:
//	    {
//	      m_deque.push_back(eCAM_EVENT_PARTY_LAST_DUMMY_FEATURE);
//	      return;
//	    }
//	    case eCAM_EVENT_SET_AS_LEADER:
//	    	return;
//		default:
//			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
//			break;
//	  }
//	  return;
//  }

  switch (m_eventType)
  {
    case eCAM_EVENT_PARTY_END_IVR:
    {
      // end wait for chair - parties events
      m_deque.push_back(eCAM_EVENT_PARTY_END_ALL_WAIT_FOR_CHAIR);
      // end wait for chair - conf event
      m_deque.push_back(eCAM_EVENT_CONF_END_WAIT_FOR_CHAIR);
      // end single party music
      m_deque.push_back(eCAM_EVENT_PARTY_END_SINGLE_PARTY_MUSIC);
      // stop play ringing tone
      m_deque.push_back(eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE);
      // first to join
      m_deque.push_back(eCAM_EVENT_PARTY_FIRST_TO_JOIN);
      // recording in progress (for every party which enters a recorded conf)
      m_deque.push_back(eCAM_EVENT_PARTY_RECORDING_IN_PROGRESS);
      // recording has failed (for chairperson which enters a conf whose recording has failed)
      m_deque.push_back(eCAM_EVENT_PARTY_RECORDING_FAILED);
      // entry roll-call
      m_deque.push_back(eCAM_EVENT_CONF_ENTRY_TONE);
     // conf wait for chair (the first party activates it)
      m_deque.push_back(eCAM_EVENT_CONF_WAIT_FOR_CHAIR);
      // wait for chair (must be the last event)
      m_deque.push_back(eCAM_EVENT_PARTY_WAIT_FOR_CHAIR); // must be the last event, the audio of party changed to Mix
      // send in conf indication
      m_deque.push_back(eCAM_EVENT_PARTY_IN_CONF_IND); // can be after "wait for chair" event since it only sends an indication to party control
      // end video IVR session
      m_deque.push_back(eCAM_EVENT_PARTY_END_VIDEO_IVR); // can be after "wait for chair" event since it concerns only the video bridge
      // dummy feature that represents the last feature of the event
      m_deque.push_back(eCAM_EVENT_PARTY_LAST_DUMMY_FEATURE);
      return;
    }

    case eCAM_EVENT_PARTY_DELETED:
    case eCAM_EVENT_PARTY_AUDIO_MOVE_OUT:
    {
      // chair dropped
      m_deque.push_back(eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED);
      // chair dropped
      m_deque.push_back(eCAM_EVENT_CONF_CHAIR_DROPPED);
      // entry roll-call
      m_deque.push_back(eCAM_EVENT_CONF_EXIT_TONE);
      // single party music
      m_deque.push_back(eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC);
      // dummy feature that represents the last feature of the event
      m_deque.push_back(eCAM_EVENT_PARTY_LAST_DUMMY_FEATURE);
      return;
    }

    case eCAM_EVENT_SET_AS_LEADER:
    {
      // end wait for chair - parties events
      m_deque.push_back(eCAM_EVENT_PARTY_END_ALL_WAIT_FOR_CHAIR);
      // end wait for chair - conf event
      m_deque.push_back(eCAM_EVENT_CONF_END_WAIT_FOR_CHAIR);
      // dummy feature that represents the last feature of the event
      m_deque.push_back(eCAM_EVENT_PARTY_LAST_DUMMY_FEATURE);
      return;
    }

	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }
  FPASSERTMSG(m_eventType, "Unknown event type due waiting event creation");
}


////////////////////////////////////////////////////////////////////////////
CConfAppWaitEventsList::CConfAppWaitEventsList()
{
}

////////////////////////////////////////////////////////////////////////////
CConfAppWaitEventsList::CConfAppWaitEventsList(const CConfAppWaitEventsList& other) : CPObject(other)
{
  *this = other;
}

////////////////////////////////////////////////////////////////////////////
CConfAppWaitEventsList& CConfAppWaitEventsList::operator=(const CConfAppWaitEventsList& other)
{
  FPASSERTMSG(1, "'operator=' is forbidden for the class CConfAppWaitEventsList");
  return *this;
}

////////////////////////////////////////////////////////////////////////////
CConfAppWaitEventsList::~CConfAppWaitEventsList()
{
  DelAllEvents();
}

////////////////////////////////////////////////////////////////////////////
DWORD CConfAppWaitEventsList::AddEvent(TConfAppEvents eventType, DWORD partyRsrcID, DWORD tMCUProductType)
{
  CWaitingEvent* event = new CWaitingEvent(eventType, partyRsrcID, tMCUProductType);
  m_map[(DWORD)event] = event;
  return (DWORD)event;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppWaitEventsList::GetNextFeature(DWORD key, TConfAppEvents& feature)
{
  WaitingEventMapItr itr = m_map.find(key);
  if (itr != m_map.end())
  {
    if (itr->second->m_deque.size() > 0)
    {
      feature = itr->second->m_deque[0];
      itr->second->m_deque.pop_front();
      if (itr->second->m_deque.size() == 0)
        DelEvent(itr); // it was the last feature of this event than remove this event
      return STATUS_OK;
    }
  }
  feature = eCAM_EVENT_MIN;
  return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////
TConfAppEvents CConfAppWaitEventsList::GetEventType(DWORD key)
{
  WaitingEventMapItr itr = m_map.find(key);
  if (itr != m_map.end())
    return itr->second->m_eventType;
  return eCAM_EVENT_MIN; // illegal event
}

////////////////////////////////////////////////////////////////////////////
void CConfAppWaitEventsList::DelEvent(WaitingEventMapItr itr)
{
  delete itr->second;
  m_map.erase(itr);
}

////////////////////////////////////////////////////////////////////////////
void CConfAppWaitEventsList::DelEvent(DWORD partyRsrcID)
{
  for (WaitingEventMapItr itr = m_map.begin(); itr != m_map.end();) {
    if (partyRsrcID == itr->second->m_partyRsrcID) 
	{
		delete itr->second;
		m_map.erase(itr++);
    }
	else
	{	
		itr++;
	}
  }
}

////////////////////////////////////////////////////////////////////////////
void CConfAppWaitEventsList::DelAllEvents()
{
  for (WaitingEventMapItr itr = m_map.begin(); itr != m_map.end(); ++itr)
    if (itr != m_map.end())
      delete itr->second;
  m_map.clear();
}

////////////////////////////////////////////////////////////////////////////
void CConfAppWaitEventsList::AddFeature(DWORD key, DWORD opcode)
{
  WaitingEventMapItr itr = m_map.find(key);
  if (itr != m_map.end())
    itr->second->m_deque.push_front((TConfAppEvents)opcode);
}
