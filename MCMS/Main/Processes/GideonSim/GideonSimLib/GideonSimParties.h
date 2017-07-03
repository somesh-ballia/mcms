//+========================================================================+
//                   GideonSimParties.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimParties.h                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+
#if !defined(__GIDEONSIM_PARTIES_)
#define __GIDEONSIM_PARTIES_

class CSegment;
class CMplMcmsProtocol;

#include "PObject.h"
/////////////////////////////////////////////////////////////////////////////
//  CBasicParty - presents abstract party type
//

/////////////////////////////////////////////////////////////////////////////
class CBasicParty : public CPObject
{
public:
CLASS_TYPE_1(CBasicParty,CPObject)
	// constructor / destructor
	CBasicParty();
	~CBasicParty();
	
	// base class overrides
//    virtual const char*  NameOf() const { return "CBasicParty"; }

	void Create(CMplMcmsProtocol& rMpl);
	virtual void Cleanup();
	virtual BOOL IsEmpty() const;
	virtual void Serialize(CSegment& seg) const;
	virtual void DeSerialize(CSegment& seg);

	DWORD GetConfId() const { return m_confId; }
	DWORD GetPartyId() const { return m_partyId; }
	DWORD GetConnectionId() const { return m_connectionId; }
	DWORD GetConnectionIdRtp() const { return m_connectionIdRtp; }

protected:
	DWORD  m_confId;
	DWORD  m_partyId;
	DWORD  m_connectionId;
	DWORD  m_connectionIdRtp;
};


/////////////////////////////////////////////////////////////////////////////
//  CAudioParty - presents audio party
//

/////////////////////////////////////////////////////////////////////////////
class CAudioParty : public CBasicParty
{
public:
CLASS_TYPE_1(CAudioParty,CBasicParty)
	// constructor / destructor
	CAudioParty();
	~CAudioParty();
	
	// base class overrides
    virtual const char*  NameOf() const { return "CAudioParty"; }

//	virtual void Cleanup();
//	virtual BOOL IsEmpty() const;
//	virtual void Serialize(CSegment& seg);
//	virtual void DeSerialize(CSegment& seg);

	void SetConfId(const DWORD confId);
	void SetRtpId(const DWORD rtpId);

protected:
};


/////////////////////////////////////////////////////////////////////////////
//  CPstnConnectionParty - presents PSTN connection
//

enum PartyDialType{
	eDialOutType = 0,
	eDialInType  = 1,
	eUnknownDialType,
};

/////////////////////////////////////////////////////////////////////////////
class CPstnConnectionParty : public CBasicParty
{
CLASS_TYPE_1(CPstnConnectionParty,CBasicParty)
public:
	// constructor / destructor
	CPstnConnectionParty();
	~CPstnConnectionParty();
	
	// base class overrides
    virtual const char*  NameOf() const { return "CPstnConnectionParty"; }

	void Create(CMplMcmsProtocol& rMpl,const PartyDialType dialType,
			const WORD firstPort, const WORD portsNum);
	void Cleanup();
//	BOOL IsEmpty() const;
	void FillPortData(CMplMcmsProtocol& rMpl) const;
	void Serialize(CSegment& seg) const;
	void DeSerialize(CSegment& seg);
	WORD  GetPortId() const { return m_portId; }
	WORD  GetPortsNum() const { return m_numPorts; }

protected:
	WORD   m_portId;
	WORD   m_numPorts;
	PartyDialType  m_enDialType;   
};


#endif // !defined(__GIDEONSIM_PARTIES_)











