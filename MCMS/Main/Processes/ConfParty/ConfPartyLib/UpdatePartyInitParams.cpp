//+========================================================================+
//                    UpdatePartyInitParams.cpp                            |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       UpdatePartyInitParams.cpp                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+


#include "UpdatePartyInitParams.h"

// ------------------------------------------------------------
CUpdatePartyInitParams::CUpdatePartyInitParams ()
{
	m_UpdateInParams = NULL;
	m_pConfAppBridgeParams = NULL;
	m_UpdateOutParams = NULL;
	m_isIVR = FALSE;
	
}
// ------------------------------------------------------------
CUpdatePartyInitParams::CUpdatePartyInitParams (const CBridgePartyInitParams& rOtherUpdatePartyInitParams)
        :CPObject(rOtherUpdatePartyInitParams)
{
	m_UpdateInParams = NULL;
	m_UpdateOutParams = NULL;
	
	m_pConfAppBridgeParams = new CConfAppBridgeParams();
	InitConfAppParams(rOtherUpdatePartyInitParams.GetConfAppParams());
		
	m_pConfName = rOtherUpdatePartyInitParams.GetConfName() ;
	m_pPartyName = rOtherUpdatePartyInitParams.GetPartyName();
}

// ------------------------------------------------------------
CUpdatePartyInitParams::~CUpdatePartyInitParams ()
{
	POBJDELETE(m_UpdateInParams);
	POBJDELETE(m_UpdateOutParams);
	POBJDELETE(m_pConfAppBridgeParams);
}
// ------------------------------------------------------------
CUpdatePartyInitParams& CUpdatePartyInitParams::operator = (const CUpdatePartyInitParams& rOtherUpdatePartyInitParams)
{
	if ( &rOtherUpdatePartyInitParams == this ) return *this;
	// PRESERVE:BEGIN
	// Insert your preservable code here...
	return *this;
	// PRESERVE:END
}
// ------------------------------------------------------------
void CUpdatePartyInitParams::AllocateInParams()
{
	m_UpdateInParams = new CBridgePartyMediaParams;
}
// ------------------------------------------------------------
void CUpdatePartyInitParams::AllocateOutParams()
{
	m_UpdateOutParams = new CBridgePartyMediaParams;
}
// ------------------------------------------------------------
void CUpdatePartyInitParams::InitConfAppParams(const CConfAppBridgeParams* pConfAppParams)
{
	m_pConfAppBridgeParams->SetNoiseDetection((pConfAppParams->IsNoiseDetection()) ? TRUE : FALSE);
	m_pConfAppBridgeParams->SetNoiseDetectionThreshold(pConfAppParams->GetNoiseDetectionThreshold());
	m_pConfAppBridgeParams->SetMuteIncoming(pConfAppParams->IsMuteIncoming());
	m_pConfAppBridgeParams->SetIvrInConf((pConfAppParams->IsIvrInConf()) ? TRUE : FALSE);
}
