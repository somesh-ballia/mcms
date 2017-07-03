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
// FILE: ConfAppWaitEventsList.h                                           |
// SUBSYSTEM:  ConfParty                                                   |
//+========================================================================+

#ifndef __CONF_APPLICATIONS_WAIT_EVENTS_LIST_H__
#define __CONF_APPLICATIONS_WAIT_EVENTS_LIST_H__

#include "ConfAppMngr.h"
#include <map>
#include <deque>

class CWaitingEvent;

typedef deque<TConfAppEvents>                     ConfAppEventsDeq;
typedef deque<TConfAppEvents>::iterator           ConfAppEventsDeqItr;
typedef deque<TConfAppEvents>::const_iterator     ConfAppEventsDeqItr_const;

typedef map<DWORD,CWaitingEvent*>                 WaitingEventMap;
typedef map<DWORD,CWaitingEvent*>::iterator       WaitingEventMapItr;
typedef map<DWORD,CWaitingEvent*>::const_iterator WaitingEventMapItr_const;

////////////////////////////////////////////////////////////////////////////
struct CWaitingEvent
{
  DWORD                   m_partyRsrcID;
  TConfAppEvents          m_eventType;
  ConfAppEventsDeq        m_deque;

                          CWaitingEvent(TConfAppEvents eventType, DWORD partyRsrcID, DWORD tMCUproductType);
};


////////////////////////////////////////////////////////////////////////////
class CConfAppWaitEventsList : public CPObject
{
  CLASS_TYPE_1(CConfAppWaitEventsList, CPObject)

  WaitingEventMap         m_map;

public:
                          CConfAppWaitEventsList();
                          CConfAppWaitEventsList(const CConfAppWaitEventsList& other);
                         ~CConfAppWaitEventsList();

  virtual const char*     NameOf() const { return "CConfAppWaitEventsList"; }
  CConfAppWaitEventsList& operator=(const CConfAppWaitEventsList& other);


  DWORD                   AddEvent(TConfAppEvents eventType, DWORD partyRsrcID, DWORD tMCUproductType);
  void                    DelEvent(DWORD partyRsrcID);
  void                    DelAllEvents();

  int                     GetNextFeature(DWORD key, TConfAppEvents& feature);
  TConfAppEvents          GetEventType(DWORD key);

  void                    AddFeature(DWORD key, DWORD opcode);

private:
  void                    DelEvent(WaitingEventMapItr itr);
};

#endif  // __CONF_APPLICATIONS_WAIT_EVENTS_LIST_H__
