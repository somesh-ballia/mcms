// SyncedElementConfProfiles.cpp: implementation of the SyncedElementConfProfiles class.
//
//////////////////////////////////////////////////////////////////////


#include "SyncedElementConfProfiles.h"
#include "TraceStream.h"
#include "FailoverProcess.h"
#include "FailoverCommunication.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
CSyncedElementConfProfiles::CSyncedElementConfProfiles()
{
	SetElementFileds();
	SetComplexElementFileds();
	SetBasicReservationElementFileds();

	//PrepareBasicGetTransStr();
}

////////////////////////////////////////////////////////////////////////////
void CSyncedElementConfProfiles::SetElementFileds()
{
	m_actionName = "GET_PROFILE_LIST";
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElementConfProfiles::SetComplexElementFileds()
{
	m_trans2ActionName  = "GET_PROFILE";

	m_addOrUpdateOpcode = FAILOVER_CONFPARTY_ADD_OR_UPDATE_PROFILE_IND;
	m_delOpcode         = FAILOVER_CONFPARTY_TERMINATE_PROFILE_IND;
	m_eDestProcess      = eProcessConfParty;
}

/////////////////////////////////////////////////////////////////////////////
void  CSyncedElementConfProfiles::SetBasicReservationElementFileds()
{
	m_responseSummaryLsNodeName = "PROFILE_SUMMARY_LS";
	m_responseSummaryNodeName = "PROFILE_SUMMARY";
}
