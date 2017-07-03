//+========================================================================+
//                     BoardDetails.h							           |
//				Copyright 2005 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BoardDetails.h											   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                        |
//+========================================================================+

#ifndef __BOARDDETAILS_H__
#define __BOARDDETAILS_H__


////////////////////////////////////////////////////////////////////////////
//  INCLUDES
//
#include "PObject.h"


////////////////////////////////////////////////////////////////////////////
//  DECLARATIONS
//


////////////////////////////////////////////////////////////////////////////
//  CONSTANTS
//




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CBoardDetails : public CPObject
{
	CLASS_TYPE_1(CBoardDetails,CPObject)

public:
	static const WORD MAX_BOARDS = 64;
	static const WORD MAX_SUB_BOARDS = 64;
	virtual const char* NameOf() const { return "CBoardDetails";}
	static const WORD MAX_UNITS = 64;
	static const WORD MAX_PORTS = 64;

public:
		// constructors
	CBoardDetails();
	virtual ~CBoardDetails();

		// overrides

		// utils
	virtual BOOL SetBoardDetails(const WORD boardId,const WORD subBoardId,const WORD unitId,const WORD portId);
	virtual void CleanDetails();

	WORD GetBoardId() const    { return m_boardId; }
	WORD GetSubBoardId() const { return m_subBoardId; }
	WORD GetUnitId() const     { return m_unitId; }
	WORD GetPortId() const     { return m_portId; }

	BOOL SetBoardId(const WORD boardId);
	BOOL SetSubBoardId(const WORD subBoardId);
	BOOL SetUnitId(const WORD unitId);
	BOOL SetPortId(const WORD portId);

protected:
	WORD	m_boardId;
	WORD	m_subBoardId;
	WORD	m_unitId;
	WORD	m_portId;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CPartyBoardDetails : public CBoardDetails
{
	CLASS_TYPE_1(CPartyBoardDetails,CBoardDetails)

public:
		// constructors
	CPartyBoardDetails();
	virtual const char* NameOf() const { return "CPartyBoardDetails";}
	virtual ~CPartyBoardDetails();

		// overrides
	virtual void CleanDetails();

		// utils
	BOOL SetPartyDetails(const DWORD confId,const DWORD partyId);

	DWORD GetConfId() const  { return m_confId;  }
	DWORD GetPartyId() const { return m_partyId; }

	BOOL SetConfId( const DWORD confId );
	BOOL SetPartyId(const DWORD partyId);

protected:
	DWORD	m_confId;
	DWORD	m_partyId;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CRtmBoardDetails : public CBoardDetails
{
	CLASS_TYPE_1(CRtmBoardDetails,CBoardDetails)

public:
		// constructors
	CRtmBoardDetails();
	virtual const char* NameOf() const { return "CRtmBoardDetails";}
	virtual ~CRtmBoardDetails();

		// overrides
	virtual void CleanDetails();

		// utils
	virtual BOOL SetBoardDetails(const WORD boardId,const WORD subBoardId,const WORD spanId,const WORD portId);
//	BOOL SetRtmDetails(const WORD spanId,const WORD portId);

	WORD GetRtmSpanId() const { return m_wRtmSpanId; }
	WORD GetRtmPortId() const { return m_wRtmPortId; }

	BOOL SetRtmSpanId(const DWORD spanId);
	BOOL SetRtmPortId(const DWORD portId);

protected:
	WORD	m_wRtmSpanId;
	WORD	m_wRtmPortId;
};


#endif // __BOARDDETAILS_H__ 





