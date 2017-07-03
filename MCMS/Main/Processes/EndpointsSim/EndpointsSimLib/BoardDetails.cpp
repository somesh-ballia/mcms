//+========================================================================+
//                    BoardDetails.cpp									   |
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

#include <string>


#include "BoardDetails.h"



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CBoardDetails::CBoardDetails()
{
	CleanDetails();
}

/////////////////////////////////////////////////////////////////////////////
CBoardDetails::~CBoardDetails()
{
}

/////////////////////////////////////////////////////////////////////////////
BOOL CBoardDetails::SetBoardDetails(const WORD boardId,const WORD subBoardId,const WORD unitId,const WORD portId)
{
	return (SetBoardId(boardId) && SetSubBoardId(subBoardId) && SetUnitId(unitId) && SetPortId(portId));
}

/////////////////////////////////////////////////////////////////////////////
void CBoardDetails::CleanDetails()
{
	m_boardId = m_subBoardId = m_unitId = m_portId = 0xFFFF;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CBoardDetails::SetBoardId(const WORD boardId)
{
	if( boardId >= CBoardDetails::MAX_BOARDS )
		return FALSE;

	m_boardId = boardId;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CBoardDetails::SetSubBoardId(const WORD subBoardId)
{
	if( subBoardId >= CBoardDetails::MAX_SUB_BOARDS )
		return FALSE;

	m_subBoardId = subBoardId;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CBoardDetails::SetUnitId(const WORD unitId)
{
	if( unitId >= CBoardDetails::MAX_UNITS )
		return FALSE;

	m_unitId = unitId;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CBoardDetails::SetPortId(const WORD portId)
{
	if( portId >= CBoardDetails::MAX_PORTS )
		return FALSE;

	m_portId = portId;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CPartyBoardDetails::CPartyBoardDetails()
{
	CleanDetails();
}

/////////////////////////////////////////////////////////////////////////////
CPartyBoardDetails::~CPartyBoardDetails()
{
}

/////////////////////////////////////////////////////////////////////////////
void CPartyBoardDetails::CleanDetails()
{
	CBoardDetails::CleanDetails();
	m_confId = m_partyId = 0xFFFFFFFF;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPartyBoardDetails::SetPartyDetails(const DWORD confId,const DWORD partyId)
{
	return ( SetConfId(confId) && SetPartyId(partyId) );
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPartyBoardDetails::SetConfId( const DWORD confId )
{
	m_confId = confId;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPartyBoardDetails::SetPartyId( const DWORD partyId )
{
	m_partyId = partyId;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CRtmBoardDetails::CRtmBoardDetails()
{
	CleanDetails();
}

/////////////////////////////////////////////////////////////////////////////
CRtmBoardDetails::~CRtmBoardDetails()
{
}

/////////////////////////////////////////////////////////////////////////////
void CRtmBoardDetails::CleanDetails()
{
	CBoardDetails::CleanDetails();
	m_wRtmSpanId = m_wRtmPortId = 0xFFFF;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CRtmBoardDetails::SetBoardDetails(const WORD boardId,const WORD subBoardId,const WORD unitId,const WORD portId)
{
	return (CBoardDetails::SetBoardDetails(boardId,subBoardId,0,0) && SetRtmSpanId(unitId) && SetRtmPortId(portId));
}

/////////////////////////////////////////////////////////////////////////////
BOOL CRtmBoardDetails::SetRtmSpanId( const DWORD spanId )
{
	m_wRtmSpanId = spanId;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CRtmBoardDetails::SetRtmPortId( const DWORD portId )
{
	m_wRtmPortId = portId;
	return TRUE;
}










