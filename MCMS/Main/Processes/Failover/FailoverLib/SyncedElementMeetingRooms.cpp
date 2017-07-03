// CSyncedElementMeetingRooms.cpp: implementation of the CSyncedElementMeetingRooms class.
//
//////////////////////////////////////////////////////////////////////


#include "SyncedElementMeetingRooms.h"
#include "TraceStream.h"
#include "FailoverProcess.h"
#include "FailoverCommunication.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
CSyncedElementMeetingRooms::CSyncedElementMeetingRooms()
{
	SetElementFileds();
	SetComplexElementFileds();
	SetBasicReservationElementFileds();

	//PrepareBasicGetTransStr();
}

////////////////////////////////////////////////////////////////////////////
void CSyncedElementMeetingRooms::SetElementFileds()
{
	m_actionName = "GET_MEETING_ROOM_LIST";
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElementMeetingRooms::SetComplexElementFileds()
{
	m_trans2ActionName = "GET_MEETING_ROOM";

	m_addOrUpdateOpcode = FAILOVER_CONFPARTY_ADD_OR_UPDATE_MEETING_ROOM_IND;
	m_delOpcode         = FAILOVER_CONFPARTY_TERMINATE_MEETING_ROOM_IND;
	m_eDestProcess      = eProcessConfParty;
}

/////////////////////////////////////////////////////////////////////////////
void  CSyncedElementMeetingRooms::SetBasicReservationElementFileds()
{
	m_responseSummaryLsNodeName = "MEETING_ROOM_SUMMARY_LS";
	m_responseSummaryNodeName = "MEETING_ROOM_SUMMARY";
}


