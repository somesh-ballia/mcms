//+========================================================================+
//                   ContentBridgePartyInitParams.h                              |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       ContentBridgePartyInitParams.h                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Yoella                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  June-2006  | Description                                    |
//-------------------------------------------------------------------------|

#ifndef _CONTENT_BRIDGE_PARTY_INIT_PARAMS_
#define _CONTENT_BRIDGE_PARTY_INIT_PARAMS_

#include "BridgePartyInitParams.h"
#include "ConfPartyGlobals.h"
#include "ContentBridge.h"
#include "Trace.h"


class CContentBridgePartyInitParams : public CBridgePartyInitParams
{
CLASS_TYPE_1(CContentBridgeInitParams,CBridgePartyInitParams)
public:

    CContentBridgePartyInitParams ();
	CContentBridgePartyInitParams (const char* pPartyName, const CTaskApp* pParty,  const PartyRsrcID partyRsrcID,
							const WORD wNetworkInterface,DWORD partyContentRate,WORD partyContentProtocol, BYTE PartyContentH264HProfile = FALSE,
							const CBridgePartyMediaParams * pMediaInParams = NULL ,
							const CBridgePartyMediaParams * pMediaOutParams = NULL,
							const CBridgePartyCntl* pBridgePartyCntl = NULL ,
							const char* pSiteName = "",
							BYTE isCascade = NO,
							BYTE cascadeLinkMode = NONE );
    virtual ~CContentBridgePartyInitParams ();
	void  Serialize(WORD format,CSegment& seg) const;
	void  DeSerialize(WORD format,CSegment& seg);
	virtual const char* NameOf() const { return "CContentBridgePartyInitParams";}
	
	void SetByCurrentContentRate(BYTE byCurrentContentRate);
	BYTE GetByCurrentContentRate()	const	{ return m_byCurrentContentRate;}

	void SetByCurrentContentProtocol(BYTE byCurrentContentProtocol);
	BYTE GetByCurrentContentProtocol()	const	{ return m_byCurrentContentProtocol;}

	//HP content
	BYTE GetByCurrentContentH264HighProfile() const {return m_byCurrentContentH264HighProfile;}  
	void SetByCurrentContentH264HighProfile(BYTE byCurrentContentProfile);


	BOOL IsValidParams() const;
	BOOL IsMyContentProtocolValid() const;
	BOOL IsMyContentRateValid()const;
    void SetMyPartyNumbers(BYTE& m_mcuNumber,BYTE& m_terminalNumber) ;

private:

	CContentBridgePartyInitParams (const CContentBridgePartyInitParams& rOtherContentBridgePartyInitParams);
	CContentBridgePartyInitParams& operator = (const CContentBridgePartyInitParams& rOtherContentBridgePartyInitParams);
	BYTE m_byCurrentContentRate;
	BYTE m_byCurrentContentProtocol;
	BYTE m_byCurrentContentH264HighProfile; //HP content
};

#endif //_CONTENT_BRIDGE_PARTY_INIT_PARAMS_


