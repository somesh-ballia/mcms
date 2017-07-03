//+========================================================================+
//                    BridgePartyMediaParams.H                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyMediaParams.H                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#ifndef _CBridgePartyMediaParams_H_
#define _CBridgePartyMediaParams_H_

#include "PObject.h"
#include "TelepresenseEPInfo.h"

class CBridgePartyMediaParams : public CPObject {
CLASS_TYPE_1(CBridgePartyMediaParams,CPObject)
public:
	CBridgePartyMediaParams ();
	virtual ~CBridgePartyMediaParams ();
	virtual BOOL  IsValidParams() const { return FALSE; }
	virtual BOOL IsValidCopParams() const { return TRUE; }
	virtual BOOL IsValidXCodeParams() const { return TRUE; }
	virtual const char* NameOf() const { return "CBridgePartyMediaParams";}

	void SetTelePresenceMode(eTelePresencePartyType eTelePresenceMode);
	eTelePresencePartyType GetTelePresenceMode() const;
	void SetTelePresenceEPInfo(CTelepresenseEPInfo* tpInfo);
	CTelepresenseEPInfo* GetTelePresenceEPInfo() const;

protected:
	CBridgePartyMediaParams& operator = (const CBridgePartyMediaParams& rOtherBridgePartyMediaParams);
	CBridgePartyMediaParams (const CBridgePartyMediaParams& rOtherBridgePartyMediaParams);

	eTelePresencePartyType m_eTelePresenceMode;   // PCI bug patch (to be removed in V3.x)
	CTelepresenseEPInfo*   m_telepresenseEPInfo;
};


#endif
