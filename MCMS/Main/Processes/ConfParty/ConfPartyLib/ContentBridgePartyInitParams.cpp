//+========================================================================+
//                   ContentBridgePartyInitParams.cpp                             |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       ContentBridgePartyInitParams.cpp                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Yoella                                                    |
//-------------------------------------------------------------------------|
// Who  | Date  June-2006  | Description                                   |
//-------------------------------------------------------------------------|
//
//+========================================================================+


#include "ContentBridgePartyInitParams.h"



// ------------------------------------------------------------
CContentBridgePartyInitParams::CContentBridgePartyInitParams ()
{
}

// ------------------------------------------------------------
//constructor for partyCntl to fill, CBridge will fill all other params
CContentBridgePartyInitParams::CContentBridgePartyInitParams (const char* pPartyName, const CTaskApp* pParty, const PartyRsrcID partyRsrcID,
												const WORD wNetworkInterface,DWORD partyContentRate,WORD PartycontentProtocol, BYTE PartyContentH264HProfile,
												const CBridgePartyMediaParams * pMediaInParams ,
												const CBridgePartyMediaParams * pMediaOutParams,
												const CBridgePartyCntl* pBridgePartyCntl,
												const char* pSiteName,
												BYTE isCascade,
												BYTE cascadeLinkMode) :
												CBridgePartyInitParams(pPartyName,pParty,partyRsrcID, DUMMY_ROOM_ID, wNetworkInterface,pMediaInParams,
												pMediaOutParams,pBridgePartyCntl)
{
	BYTE newPartyRateAMC = CUnifiedComMode::TranslateRateToAMCRate(partyContentRate);
	m_byCurrentContentRate = newPartyRateAMC;
	m_byCurrentContentProtocol = PartycontentProtocol;
	m_byCurrentContentH264HighProfile = PartyContentH264HProfile;  //HP content

	TRACEINTO << "m_byCurrentContentProtocol=" << (int)m_byCurrentContentProtocol;
	if(isCascade)
		m_bCascadeLinkMode = cascadeLinkMode;
	else
		m_bCascadeLinkMode = NONE;
}
// ------------------------------------------------------------
CContentBridgePartyInitParams::CContentBridgePartyInitParams (const CContentBridgePartyInitParams &rOtherContentBridgePartyInitParams)
        :CBridgePartyInitParams(rOtherContentBridgePartyInitParams)
{
}

// ------------------------------------------------------------
CContentBridgePartyInitParams::~CContentBridgePartyInitParams ()
{
}

// ------------------------------------------------------------
CContentBridgePartyInitParams& CContentBridgePartyInitParams::operator = (const CContentBridgePartyInitParams &rOtherContentBridgePartyInitParams)
{
	// Operator= is not available for this class because all members are const
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyInitParams::SetByCurrentContentRate(BYTE byCurrentContentRate)
{
	m_byCurrentContentRate = byCurrentContentRate;
}

////////////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyInitParams::SetByCurrentContentProtocol(BYTE byCurrentContentProtocol)
{
	m_byCurrentContentProtocol = byCurrentContentProtocol;
    TRACEINTO << "m_byCurrentContentProtocol=" << (int)m_byCurrentContentProtocol;

}

////////////////////////////////////////////////////////////////////////////////////
//HP content
void CContentBridgePartyInitParams::SetByCurrentContentH264HighProfile(BYTE byCurrentContentProfile)
{
	m_byCurrentContentH264HighProfile = byCurrentContentProfile;
}
////////////////////////////////////////////////////////////////////////////////////
void  CContentBridgePartyInitParams::Serialize(WORD format,CSegment& seg) const
{
	CBridgePartyInitParams::Serialize(format,seg) ;

	if(format ==  NATIVE )
    {
		seg << (WORD)m_byCurrentContentRate;
		seg << (WORD)m_byCurrentContentProtocol;
		seg << (WORD)m_byCurrentContentH264HighProfile;   //HP content
        //seg << (WORD)m_statusMasterSlave;
    }

}

////////////////////////////////////////////////////////////////////////////////////
void  CContentBridgePartyInitParams::DeSerialize(WORD format,CSegment& seg)
{
	CBridgePartyInitParams::DeSerialize(format,seg);

	if(format ==  NATIVE )
	{
		WORD tmp;
		seg >> tmp;
		m_byCurrentContentRate=(BYTE)tmp;
		seg >> tmp;
		m_byCurrentContentProtocol=(BYTE)tmp;
	    TRACEINTO << "m_byCurrentContentProtocol=" << (int)m_byCurrentContentProtocol;

		//HP content
		seg >> tmp;
		m_byCurrentContentH264HighProfile=(BYTE)tmp;
//        seg >> tmp;
//        m_statusMasterSlave = (BYTE)tmp;
	}
}
// ------------------------------------------------------------
BOOL CContentBridgePartyInitParams::IsValidParams() const
{

	if(CBridgePartyInitParams::IsValidParams() == FALSE)
		return FALSE;

	if(CContentBridgePartyInitParams::IsMyContentRateValid() == FALSE)
		return FALSE;

	if(CContentBridgePartyInitParams::IsMyContentProtocolValid() == FALSE)
		return FALSE;

	return TRUE;

}
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CContentBridgePartyInitParams::IsMyContentRateValid() const
{
	return ((CContentBridge*)GetBridge())->IsValidContentRate(m_byCurrentContentRate);
}
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CContentBridgePartyInitParams::IsMyContentProtocolValid() const
{
	return ((CContentBridge*)GetBridge())->IsValidContentProtocol(m_byCurrentContentProtocol);
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyInitParams::SetMyPartyNumbers(BYTE& mcuNum,BYTE& terminalNum)
{
	if (( ! CPObject::IsValidPObjectPtr(GetParty()) ) || ( ! CPObject::IsValidPObjectPtr(GetConf()) ) )
	{
		mcuNum = 0xFF;
		terminalNum = 0xFF;
		DBGPASSERT(1);
	}
	else
	{
		// BAD casting to (WORD&) - Romem need to fix the prototype of GetPartyTerminalNumber function !!!!!
		WORD mcuNumTemp = 0xFF;
		WORD terminalNumTemp = 0xFF;
		STATUS status = ((CConf*)GetConf())->GetPartyTerminalNumber((GetParty()),mcuNumTemp,terminalNumTemp);
		PASSERT_AND_RETURN(status);
		mcuNum = (BYTE)mcuNumTemp;
		terminalNum = (BYTE)terminalNumTemp;
  	}

}

