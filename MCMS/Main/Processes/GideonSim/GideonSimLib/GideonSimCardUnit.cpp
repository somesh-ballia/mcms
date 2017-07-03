//+========================================================================+
//                   GideonSimCardUnit.cpp                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimCardUnit.cpp                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

//#include "ProcessBase.h"
//#include "GSegment.h"
//#include "Trace.h"
//#include "MplMcmsProtocol.h"
//#include "ClientSocket.h"
//
//#include "SimApi.h"
#include "GideonSim.h"
#include "GideonSimCardUnit.h"


/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//
//const WORD  IDLE        = 0;        // default state -  defined in base class
const WORD  SETUP         = 1;
const WORD  STARTUP       = 2;
const WORD  CONFIG        = 3;
const WORD  CONNECT       = 4;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class


/////////////////////////////////////////////////////////////////////////////
//
//   EXTERNALS:
//


/////////////////////////////////////////////////////////////////////////////
//
//   CSimBasicUnit - base class (abstract) for all simulation card's unit
//
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSimBasicUnit)
//  ONEVENTA( XML_REQUEST,  IDLE,    (AFUNC)CSimBasicCard::NullActionFunction,  "XML_REQUEST", "IDLE" )
PEND_MESSAGE_MAP(CSimBasicUnit,CStateMachine);


/////////////////////////////////////////////////////////////////////////////
//  no task creation function (abstract class)

/////////////////////////////////////////////////////////////////////////////
CSimBasicUnit::CSimBasicUnit()      // constructor
{
	m_wUnitId = 0xFFFF;
}

/////////////////////////////////////////////////////////////////////////////
CSimBasicUnit::~CSimBasicUnit()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CSimBasicUnit::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CSimBasicUnit::MplApiMessage(CSegment* pParam)
{
}




/////////////////////////////////////////////////////////////////////////////
//
//   SimArtUnit - ART unit simulation
//
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSimArtUnit)
	// Manager: connect card
//	ONEVENTA( CONNECT_CARD,  IDLE,    (AFUNC)CSimSocketedCard::OnMngrConnectSocketIdle,  "CONNECT_SHLOMIT_SOCKET", "IDLE" )
PEND_MESSAGE_MAP(CSimArtUnit,CSimBasicUnit);

/////////////////////////////////////////////////////////////////////////////
//  no task creation function (abstract class)

/////////////////////////////////////////////////////////////////////////////
CSimArtUnit::CSimArtUnit()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CSimArtUnit::~CSimArtUnit()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CSimArtUnit::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CSimArtUnit::DoNothing() const
{
}

/////////////////////////////////////////////////////////////////////////////
// App Manager: establish connection with MPL-API

/////////////////////////////////////////////////////////////////////////////
//void CSimSocketedCard::OnMngrConnectSocketIdle(CSegment* pMsg)
//{
//	if( !CPObject::IsValidPObjectPtr(m_pSocketConnection) )
//		return;
//
//	m_state = SETUP;
//
//	m_pSocketConnection->Connect();
//}




/////////////////////////////////////////////////////////////////////////////
//
//   SimRtmUnit - RTM unit simulation
//
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSimRtmUnit)
	// Message received from Socket
//	ONEVENTA( SOCKET_RCV_MSG, IDLE,    (AFUNC)CSimSwitchCard::OnSocketRcvIdle,    "SOCKET_RCV_MSG", "IDLE" )
PEND_MESSAGE_MAP(CSimRtmUnit,CSimBasicUnit);

/////////////////////////////////////////////////////////////////////////////
CSimRtmUnit::CSimRtmUnit()      // constructor
{
}

/////////////////////////////////////////////////////////////////////////////
CSimRtmUnit::~CSimRtmUnit()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CSimRtmUnit::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CSimRtmUnit::DoNothing() const
{
}

/////////////////////////////////////////////////////////////////////////////
// Message received from Socket
//

/////////////////////////////////////////////////////////////////////////////
//void CSimSwitchCard::OnSocketRcvIdle(CSegment* pMsg)
//{
//	PTRACE(eLevelInfoNormal,"CSimSwitchCard::OnSocketRcvIdle");
//	DBGPASSERT(1);
////	OnSocketRcv(pMsg);
//}




/////////////////////////////////////////////////////////////////////////////
//
//   SimVideoUnit - video decoder unit simulation
//
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSimVideoUnit)
	// Socket: connection established
//	ONEVENTA( SOCKET_CONNECTED, SETUP, (AFUNC)CSimMfaCard::OnSocketConnectedSetup,  "SOCKET_CONNECTED", "SETUP" )
PEND_MESSAGE_MAP(CSimVideoUnit,CSimBasicUnit);

/////////////////////////////////////////////////////////////////////////////
CSimVideoUnit::CSimVideoUnit()      // constructor
{

}

/////////////////////////////////////////////////////////////////////////////
CSimVideoUnit::~CSimVideoUnit()     // destructor
{

}

/////////////////////////////////////////////////////////////////////////////
void*  CSimVideoUnit::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CSimVideoUnit::DoNothing() const
{
}

/////////////////////////////////////////////////////////////////////////////
// Socket: connection established

/////////////////////////////////////////////////////////////////////////////
//void CSimMfaCard::OnSocketConnectedSetup(CSegment* pMsg)
//{
//	PTRACE(eLevelInfoNormal,"CSimMfaCard::OnSocketConnectedSetup");
//
//	m_state = STARTUP;
//}








