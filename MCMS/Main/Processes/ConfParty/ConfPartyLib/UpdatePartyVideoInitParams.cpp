//+========================================================================+
//                    UpdatePartyVideoInitParams.cpp                            |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       UpdatePartyVideoInitParams.cpp                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+


#include "UpdatePartyVideoInitParams.h"

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
LayoutType GetNewLayoutType(const BYTE oldLayoutType);

// ------------------------------------------------------------
CUpdatePartyVideoInitParams::CUpdatePartyVideoInitParams()
{
	
	int i;
	for(i=(int)CP_LAYOUT_1X1;i<(int)CP_NO_LAYOUT;i++)
	{
		m_pReservation[i] = NULL;
	}
	
	for(i=(int)CP_LAYOUT_1X1;i<(int)CP_NO_LAYOUT;i=i+1)
	{
		m_pPrivateReservation[i] = NULL;
	}
}
// ------------------------------------------------------------
CUpdatePartyVideoInitParams::CUpdatePartyVideoInitParams(const CBridgePartyInitParams& PartyInitParams)
:CUpdatePartyInitParams(PartyInitParams)
{
	for(int i=(int)CP_LAYOUT_1X1;i<(int)CP_NO_LAYOUT;i++)
	{
		m_pReservation[i] = NULL;
		m_pPrivateReservation[i] = NULL;
	}
	
	if(PartyInitParams.GetMediaInParams())	
	{
		m_UpdateInParams = new CBridgePartyVideoInParams();
		((CBridgePartyVideoInParams*)m_UpdateInParams)->InitPartyVideoInParams((CBridgePartyVideoInParams*)PartyInitParams.GetMediaInParams());
	}	

	if(PartyInitParams.GetMediaOutParams())
	{
		m_UpdateOutParams = new CBridgePartyVideoOutParams();
		((CBridgePartyVideoOutParams*)m_UpdateOutParams)->InitPartyVideoOutParams((CBridgePartyVideoOutParams*)PartyInitParams.GetMediaOutParams());
	}
		
}
// ------------------------------------------------------------
CUpdatePartyVideoInitParams::~CUpdatePartyVideoInitParams ()
{

	int i;
	for(i=(int)CP_LAYOUT_1X1;i<(int)CP_NO_LAYOUT;i++)
	{
		POBJDELETE(m_pReservation[i]);
	}
	for(i=(int)CP_LAYOUT_1X1;i<(int)CP_NO_LAYOUT;i++)
	{
		POBJDELETE(m_pPrivateReservation[i]);
	}
}
// ------------------------------------------------------------
void CUpdatePartyVideoInitParams::AllocateInParams()
{
	if(m_UpdateInParams == NULL)
		m_UpdateInParams = new CBridgePartyVideoInParams;
}
// ------------------------------------------------------------
void CUpdatePartyVideoInitParams::AllocateOutParams()
{
	if(m_UpdateOutParams == NULL)
		m_UpdateOutParams = new CBridgePartyVideoOutParams;
}
// ------------------------------------------------------------
void CUpdatePartyVideoInitParams::InitiateMediaOutParams(const CBridgePartyVideoOutParams* pBridgePartyMediaParams)
{
	AllocateOutParams();
	((CBridgePartyVideoOutParams*)m_UpdateOutParams)->InitPartyVideoOutParams(pBridgePartyMediaParams);
	InitOtherVideoOutParams();	
}
// ------------------------------------------------------------
void CUpdatePartyVideoInitParams::InitiateMediaInParams(const CBridgePartyVideoInParams* pBridgePartyMediaParams)
{
	AllocateInParams();
	((CBridgePartyVideoInParams*)m_UpdateInParams)->InitPartyVideoInParams(pBridgePartyMediaParams);
}

// ------------------------------------------------------------
void CUpdatePartyVideoInitParams::InitOtherVideoOutParams()
{
	int i;
	for(i=(int)CP_LAYOUT_1X1;i<(int)CP_NO_LAYOUT;i++)
	{
		m_pReservation[i]	= new CLayout((LayoutType)i,m_pConfName);
	}
	for(i=(int)CP_LAYOUT_1X1;i<(int)CP_NO_LAYOUT;i++)
	{
		m_pPrivateReservation[i] = new CLayout((LayoutType)i,m_pConfName);
	}
}

// ------------------------------------------------------------
EStat CUpdatePartyVideoInitParams::UpdateVideoInParams(const CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	PTRACE2(eLevelInfoNormal,"CUpdatePartyVideoInitParams::UpdateVideoInParams : Name - ",m_pPartyName);
	
	EStat responseStatus = statOK;
		
	if(!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "CUpdatePartyVideoInitParams::UpdateVideoInParams : Internal Error receive invalid params");
		responseStatus = statIllegal;
	}
	else
	{
		*((CBridgePartyVideoParams*)m_UpdateInParams) = *pBridgePartyVideoParams;
	}
	
	return responseStatus;
	
}
//--------------------------------------------------------------
EStat CUpdatePartyVideoInitParams::UpdateVideoOutParams(const CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	PTRACE2(eLevelInfoNormal,"CUpdatePartyVideoInitParams::UpdateVideoOutParams : Name - ",m_pPartyName);
	
	EStat responseStatus = statOK;
		
	if(!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "CUpdatePartyVideoInitParams::UpdateVideoOutParams : Internal Error receive invalid params");
		responseStatus = statIllegal;
	}
	else
	{
		*((CBridgePartyVideoParams*)m_UpdateOutParams) = *pBridgePartyVideoParams;
	}
	
	return responseStatus;
	

}
//--------------------------------------------------------------
void CUpdatePartyVideoInitParams::UpdatePartyLayout(CVideoLayout newLayout)
{
	if(GetLectureModeRole()==eLISTENER || GetLectureModeRole()==eCOLD_LISTENER)
		PTRACE(eLevelInfoNormal,"CUpdatePartyVideoInitParams::UpdatePartyLayout : RejectChangeLayoutRequestBecauseOfApplications");
	else
	{
		LayoutType newLayoutType=GetNewLayoutType(newLayout.GetScreenLayout());
		m_pReservation[newLayoutType]->SetLayout(newLayout,PARTY_lev);
	}
}
//--------------------------------------------------------------
void CUpdatePartyVideoInitParams::UpdateLectureModeRole(ePartyLectureModeRole partyLectureModeRole)
{	
	((CBridgePartyVideoOutParams*)m_UpdateOutParams)->SetPartyLectureModeRole(partyLectureModeRole);
}
//--------------------------------------------------------------
ePartyLectureModeRole CUpdatePartyVideoInitParams::GetLectureModeRole()
{
	return ((CBridgePartyVideoOutParams*)m_UpdateOutParams)->GetPartyLectureModeRole();

}
//--------------------------------------------------------------
void CUpdatePartyVideoInitParams::UpdateSiteName(const char* visualName)
{
	((CBridgePartyVideoInParams*)m_UpdateInParams)->SetSiteName(visualName);
}
//--------------------------------------------------------------
CLayout* CUpdatePartyVideoInitParams::GetReservationLayout(int index)
{
	CLayout* layout = m_pReservation[index];
	return layout;
}
//--------------------------------------------------------------
CLayout* CUpdatePartyVideoInitParams::GetPrivateReservationLayout(int index)
{
	CLayout* layout = m_pPrivateReservation[index];
	return layout;
}







