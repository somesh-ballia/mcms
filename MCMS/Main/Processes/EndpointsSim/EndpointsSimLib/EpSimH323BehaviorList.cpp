//+========================================================================+
//                  EpSimH323BehaviorList.cpp								   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
//+========================================================================+

#include "Macros.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "IpCsOpcodes.h"

#include "Segment.h"
#include "ProcessBase.h"
#include "TaskApi.h"
#include "MplMcmsProtocol.h"
#include "SocketApi.h"

#include "SimApi.h"
#include "EndpointsSim.h"
#include "EndpointsGuiApi.h"
#include "EpSimH323BehaviorList.h"
#include "OpcodesMcmsInternal.h"



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//   CH323BehaviorList - List of EP Behavior elements
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CH323BehaviorList::CH323BehaviorList()      // constructor
{
	m_updateCounter = 1;

	for (int i = 0; i < MAX_H323_BEHAVIORS; i++)
		m_paBehaviorArray[i] = NULL;

	m_nDefaultBehaviorInd = 0;

	m_nNumElements = 0;

	CH323Behavior rBehav(401,"Default Behavior");
	AddBehavior(rBehav);
	//*********-----******** TEMP for TESTS - start
/*	CH323Behavior rBehav(401,"Matkina");
	AddBehavior(rBehav);

	rBehav.SetID(402);
	rBehav.SetName("Chavitush");
	AddBehavior(rBehav);

	rBehav.SetID(403);
	rBehav.SetName("Tikva");
	AddBehavior(rBehav);

	rBehav.SetID(404);
	rBehav.SetName("Pufi");
	AddBehavior(rBehav);

	rBehav.SetID(405);
	rBehav.SetName("satum");
	AddBehavior(rBehav);*/
	//*********-----******** TEMP for TESTS - end

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CH323BehaviorList::~CH323BehaviorList()     // destructor
{
	for (int i = 0; i < MAX_H323_BEHAVIORS; i++)
		POBJDELETE(m_paBehaviorArray[i]);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
const CH323Behavior* CH323BehaviorList::GetCurrentBehavior(const DWORD behaviorID) const
{
	PTRACE(eLevelInfoNormal,"CH323BehaviorList::GetCurrentBehavior");

	for (int i = 0; i < MAX_H323_BEHAVIORS; i++)
		if (behaviorID == m_paBehaviorArray[i]->GetID() )
			return ( m_paBehaviorArray[i] );

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
const CH323Behavior* CH323BehaviorList::GetDefaultBehavior() const
{
	PTRACE(eLevelInfoNormal,"CH323BehaviorList::	GetDefaultBehavior");

	if (0 == m_nNumElements)	// no elements
		return NULL;

	if (m_nDefaultBehaviorInd >= MAX_H323_BEHAVIORS)
		return NULL;		// invalid index

	if ((m_nDefaultBehaviorInd >= m_nNumElements))
		return NULL;		// invalid index

	return ( m_paBehaviorArray[m_nDefaultBehaviorInd] );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CH323BehaviorList::AddBehavior( const CH323Behavior& rNewBehavior )
{
	PTRACE(eLevelInfoNormal,"CH323BehaviorList::AddBehavior");

	if (m_nNumElements >= MAX_H323_BEHAVIORS)
		return STATUS_FAIL;	// list is full

	// checking if legal parameters
	for (int i = 0; i < m_nNumElements; i++)
		if ( 0 == strcmp (m_paBehaviorArray[i]->GetName(), rNewBehavior.GetName()) )
			return STATUS_FAIL;	// name already exists

	// create ID for this capability
	DWORD newBehaviorID = CreateBehaviorID();

	// inserting the capability to the list
	m_paBehaviorArray[m_nNumElements] = new CH323Behavior( rNewBehavior );
	m_paBehaviorArray[m_nNumElements]->SetID( newBehaviorID );
	m_nNumElements++;

	if (1 == m_nNumElements)	// only 1 behavior
		m_nDefaultBehaviorInd = 0;

	m_updateCounter++;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CH323BehaviorList::DelBehavior( char* behaviorName )
{
	PTRACE(eLevelInfoNormal,"CH323BehaviorList::DelBehavior");

	if (NULL == behaviorName)
		return STATUS_FAIL;

	if (m_nNumElements > MAX_H323_BEHAVIORS) {
		PTRACE(eLevelError,"CH323BehaviorList::DelBehavior error in m_nNumElements");
		m_nNumElements = MAX_H323_BEHAVIORS;	// should not happened
	}

	// checking if legal parameters
	int del = 0;
	for (int i = 0; i < m_nNumElements; i++)
	{
		if (0 == del)
		{
			if ( 0 == strcmp (m_paBehaviorArray[i]->GetName(), behaviorName) )
			{
				PTRACE(eLevelInfoNormal,"CH323BehaviorList::DelBehavior found");
				POBJDELETE (m_paBehaviorArray[i]);
				del = 1;
				if (m_nDefaultBehaviorInd == i)	// deleting the current default
					m_nDefaultBehaviorInd = 0;	// set the default to the first behavior;

			}
		}
		else
		{
			// del was done
			m_paBehaviorArray[i - 1] = m_paBehaviorArray[i];
			m_paBehaviorArray[i] = NULL;
		}
	}

	// update the elements number
	if (1 == del) {	// found and delete
		m_nNumElements--;

		// update the default behavior if needed
		if (0 == m_nNumElements)	// deleted the last behavior
			m_nDefaultBehaviorInd = 0xFFFF;

		m_updateCounter++;
	}


	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int	CH323BehaviorList::UpdateBehavior( char* behaviorName, CH323BehaviorUpdate* updateBehavior )
{
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
DWORD CH323BehaviorList::CreateBehaviorID() const
{
	const DWORD INITIAL_ID = 2000;

	DWORD  id = INITIAL_ID;
	for( int i=0; i<m_nNumElements; i++ )
		if( m_paBehaviorArray[i] != NULL && m_paBehaviorArray[i]->GetID() > id )
			id = m_paBehaviorArray[i]->GetID();

	return id + 1;
}


/////////////////////////////////////////////////////////////////////////////
void CH323BehaviorList::GetFullBehaviorListToGui(CSegment* pParam)
{
	DWORD	nGuiUpdateCounter=0;
	*pParam >> nGuiUpdateCounter;

	COsQueue txMbx;
	txMbx.DeSerialize(*pParam);

	// check if there is e.p. changed
	BOOL need2update = ( m_updateCounter > nGuiUpdateCounter ) ? TRUE : FALSE;

	int  i=0;
	for( i=0; i<MAX_H323_BEHAVIORS && need2update == FALSE; i++ )
	{
		if ( NULL != m_paBehaviorArray[i] ) {
			if( m_paBehaviorArray[i]->IsChanged() != FALSE ) {
				m_paBehaviorArray[i]->ClearChanged();
				need2update = TRUE;
				break;
			}
		}
	}
		// all endpoints in list are not changed
	if( FALSE == need2update )
		return;

	CSegment* pMsgSeg = new CSegment;
	*pMsgSeg	<< GUI_TO_ENDPOINTS
				<< GET_BEHAVIOR_LIST_REQ
				<< (DWORD)STATUS_OK;

	*pMsgSeg	<< m_updateCounter
				<< (DWORD)m_nNumElements;

	for (int i = 0; i < MAX_H323_BEHAVIORS; i++)
	{
		if (m_paBehaviorArray[i] != NULL)
			m_paBehaviorArray[i]->Serialize(*pMsgSeg);
	}

	CTaskApi api;
	api.CreateOnlyApi(txMbx);
	api.SendMsg(pMsgSeg,SOCKET_WRITE);
	api.DestroyOnlyApi();

}


/////////////////////////////////////////////////////////////////////////////
DWORD CH323BehaviorList::GetBehaviorListLength() const
{
	DWORD size=0;
	for (int i = 0; i < MAX_H323_BEHAVIORS; i++)
		if (m_paBehaviorArray[i] != NULL)
			size++;
	return size;
}


/////////////////////////////////////////////////////////////////////////////
void CH323BehaviorList::AddNewBehavior(CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CH323BehaviorList::AddNewBehavior");
	int nextemptyindex = GetNextEmptyPlace();
	if( nextemptyindex != -1 ) {
		m_paBehaviorArray[nextemptyindex] = new CH323Behavior(0xFFFFFFFF,"TEMP"/*temp dummy values*/);
		m_paBehaviorArray[nextemptyindex]->OnStartElement( pParam );
	}
}

/////////////////////////////////////////////////////////////////////////////
int CH323BehaviorList::GetNextEmptyPlace() const
{
	for (int i = 0; i < MAX_H323_BEHAVIORS; i++) {
		if (m_paBehaviorArray[i] == NULL)
			return i;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
void CH323BehaviorList::DeleteBehavior(CSegment* pParam )
{

	DWORD behaviorid=0;
	*pParam >> behaviorid;
	COsQueue txMbx;
	txMbx.DeSerialize(*pParam);
	for (int i = 0; i < MAX_H323_BEHAVIORS; i++)
		if (m_paBehaviorArray[i] != NULL)
		{
			if (m_paBehaviorArray[i]->GetID()==behaviorid)
			{
				delete m_paBehaviorArray[i];
				m_paBehaviorArray[i]=NULL;
			}
		}
}


/////////////////////////////////////////////////////////////////////////////
/*CH323Behavior** CH323BehaviorList::Getm_behaviorArrayPtr()
{
	return m_paBehaviorArray;
}*/


/////////////////////////////////////////////////////////////////////////////
//
//   CBehavior - EP Behavior element
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CH323Behavior::CH323Behavior() // constructor
{
	SetName("DUMMY");

	m_nID				= 0xFFFFFFFF;
	m_isChanged   		= TRUE;

	m_overwriteExisting  = 0;
	m_time2RingBack      = 0;
	m_ringBack           = 0;
	m_time2CallConnected = 0;
	m_callConnected      = 0;
	m_time2SendCap       = 0;
	m_sendCap            = 0;
	m_time2CapResponse   = 0;
	m_capResponse        = 0;
	m_time2CntrlConnected   = 0;
	m_cntrlConnected        = 0;
	m_time2IncomingChannel  = 0;
	m_incomingChannel       = 0;
	m_time2OutgoingCannelResponse  = 0;
	m_outgoingCannelResponse       = 0;
}


/////////////////////////////////////////////////////////////////////////////
CH323Behavior::CH323Behavior(const DWORD nId, const char* pszName) // constructor
{
	m_nID  = nId;
	m_isChanged = TRUE;

	strncpy( m_szName,pszName,MAX_BEHAVIOR_NAME-1 );
	m_szName[MAX_BEHAVIOR_NAME-1] = '\0';

	m_overwriteExisting  = 0;
	m_time2RingBack      = 0;
	m_ringBack           = 0;
	m_time2CallConnected = 0;
	m_callConnected      = 0;
	m_time2SendCap       = 0;
	m_sendCap            = 0;
	m_time2CapResponse   = 0;
	m_capResponse        = 0;
	m_time2CntrlConnected   = 0;
	m_cntrlConnected        = 0;
	m_time2IncomingChannel  = 0;
	m_incomingChannel       = 0;
	m_time2OutgoingCannelResponse  = 0;
	m_outgoingCannelResponse       = 0;
}


/////////////////////////////////////////////////////////////////////////////
CH323Behavior::CH323Behavior(const CH323Behavior& other) : CPObject(other)      // constructor
{
	*this = other;
}


/////////////////////////////////////////////////////////////////////////////
CH323Behavior::~CH323Behavior()     // destructor
{
}


/////////////////////////////////////////////////////////////////////////////
CH323Behavior&  CH323Behavior::operator=(const CH323Behavior& other)
{
	if( this == &other )
		return *this;

	m_nID = other.m_nID;
	m_isChanged = TRUE;

	strncpy( m_szName, other.m_szName, MAX_BEHAVIOR_NAME-1 );
	m_szName[MAX_BEHAVIOR_NAME-1] = '\0';

	m_overwriteExisting = other.m_overwriteExisting;

	m_time2RingBack = other.m_time2RingBack;
	m_ringBack = other.m_ringBack;
	m_time2CallConnected = other.m_time2CallConnected;
	m_callConnected = other.m_callConnected;
	m_time2SendCap = other.m_time2SendCap;
	m_sendCap = other.m_sendCap;
	m_time2CapResponse = other.m_time2CapResponse;
	m_capResponse = other.m_capResponse;
	m_time2CntrlConnected = other.m_time2CntrlConnected;
	m_cntrlConnected = other.m_cntrlConnected;
	m_time2IncomingChannel = other.m_time2IncomingChannel;
	m_incomingChannel = other.m_incomingChannel;
	m_time2OutgoingCannelResponse = other.m_time2OutgoingCannelResponse;
	m_outgoingCannelResponse = other.m_outgoingCannelResponse;

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void CH323Behavior::SetName( const char* pszBehaviorName )
{
	if (NULL != pszBehaviorName) {
		strncpy( m_szName, pszBehaviorName, MAX_BEHAVIOR_NAME-1 );
		m_szName[MAX_BEHAVIOR_NAME-1] = '\0';
	}
}


/////////////////////////////////////////////////////////////////////////////
void CH323Behavior::SetID( const DWORD behaviorID )
{
	m_nID = behaviorID;
}


/////////////////////////////////////////////////////////////////////////////
void CH323Behavior::OnStartElement( CSegment* pParam )
{
/*	*pParam >> m_nID;
	DWORD param1;
	*pParam >> param1;*/
	DeSerialize(*pParam);

}


/////////////////////////////////////////////////////////////////////////////
void CH323Behavior::Serialize(CSegment& rSegment) const
{
	rSegment <<	 m_nID;
	rSegment <<	 m_szName;
	rSegment <<	 m_time2RingBack;
	rSegment <<	 m_ringBack;
	rSegment <<	 m_time2CallConnected;
	rSegment <<	 m_callConnected;
	rSegment <<	 m_time2SendCap;
	rSegment <<	 m_sendCap;
	rSegment <<	 m_time2CapResponse;
	rSegment <<	 m_capResponse;
	rSegment <<	 m_time2CntrlConnected;
	rSegment <<	 m_cntrlConnected;
	rSegment <<	 m_time2IncomingChannel;
	rSegment <<	 m_incomingChannel;
	rSegment <<	 m_time2OutgoingCannelResponse;
	rSegment <<	 m_outgoingCannelResponse;
}


/////////////////////////////////////////////////////////////////////////////
void CH323Behavior::DeSerialize(CSegment& rSegParam)
{
	rSegParam >> m_nID;
	rSegParam >> m_szName;
	rSegParam >> m_time2RingBack;
	rSegParam >> m_ringBack;
	rSegParam >> m_time2CallConnected;
	rSegParam >> m_callConnected;
	rSegParam >> m_time2SendCap;
	rSegParam >> m_sendCap;
	rSegParam >> m_time2CapResponse;
	rSegParam >> m_capResponse;
	rSegParam >> m_time2CntrlConnected;
	rSegParam >> m_cntrlConnected;
	rSegParam >> m_time2IncomingChannel;
	rSegParam >> m_incomingChannel;
	rSegParam >> m_time2OutgoingCannelResponse;
	rSegParam >> m_outgoingCannelResponse;
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//							The End
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
