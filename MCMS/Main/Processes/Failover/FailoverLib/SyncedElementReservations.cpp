// CSyncedElementReservations.cpp: implementation of the CSyncedElementReservations class.
//
//////////////////////////////////////////////////////////////////////


#include "SyncedElementReservations.h"
#include "TraceStream.h"
#include "FailoverProcess.h"
#include "FailoverCommunication.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
CSyncedElementReservations::CSyncedElementReservations()
{
	SetElementFileds();
	SetComplexElementFileds();
	SetBasicReservationElementFileds();

	//PrepareBasicGetTransStr();
}

////////////////////////////////////////////////////////////////////////////
void CSyncedElementReservations::SetElementFileds()
{
	m_actionName = "GET_RES_LIST";
}

/////////////////////////////////////////////////////////////////////////////
void CSyncedElementReservations::SetComplexElementFileds()
{
	m_trans2ActionName = "GET_RES";

	m_addOrUpdateOpcode = SLAVE_ADD_OR_UPDATE_RSRV_REQ;
	m_delOpcode         = SLAVE_DELETE_RSRV_REQ;

	m_eDestProcess      = eProcessResource;
}

/////////////////////////////////////////////////////////////////////////////
void  CSyncedElementReservations::SetBasicReservationElementFileds()
{
	m_responseSummaryLsNodeName = "RES_SUMMARY_LS";
	m_responseSummaryNodeName = "RES_SUMMARY";
}


