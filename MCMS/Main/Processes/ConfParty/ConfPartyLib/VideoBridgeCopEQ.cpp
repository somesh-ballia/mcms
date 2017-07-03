//+========================================================================+
//                     VideoBridgeCop.cpp                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       VideoBridgeCopEQ.cpp                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: 		                                                       |
//-------------------------------------------------------------------------|
// Who  | Date    | Description			                                   |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#include "VideoBridgeCopEQ.h"


extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
//~~~~~~~~~~~~~~~~~~~~~~~~~~ global function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//LayoutType GetNewLayoutType(const BYTE oldLayoutType);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~ state machine ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CVideoBridgeCOPEq)

  // from VideoBridge - added INSWITCH state



  //ONEVENT(CONNECTPARTY,			    CONNECTED,		CVideoBridgeCOP::OnConfConnectPartyCONNECTED)

  //ONEVENT(DELETED_PARTY_FROM_CONF,	CONNECTED,		CVideoBridgeCOP::OnConfDeletePartyFromConfCONNECTED)
  ONEVENT(DELETED_PARTY_FROM_CONF,	DISCONNECTING,	CVideoBridgeCOPEq::NullActionFunction)

  ONEVENT(SPEAKERS_CHANGED,		        CONNECTED,		CVideoBridgeCOPEq::NullActionFunction)
  ONEVENT(SPEAKERS_CHANGED,		        DISCONNECTING,	CVideoBridgeCOPEq::NullActionFunction)

  //ONEVENT(DISCONNECTCONF,		    IDLE,			CVideoBridgeCOP::OnConfDisConnectConfIDLE)
  //ONEVENT(DISCONNECTCONF,		    CONNECTED,		CVideoBridgeCOP::OnConfDisConnectConfCONNECTED)
  //ONEVENT(DISCONNECTCONF,		    DISCONNECTING,	CVideoBridgeCOP::OnConfDisConnectConfDISCONNECTING)

//  ONEVENT(ENDCONNECTPARTY_IVR_MODE,	CONNECTED,		CVideoBridgeCOP::OnEndPartyConnectIVRModeCONNECTED)
//  ONEVENT(ENDCONNECTPARTY_IVR_MODE,	DISCONNECTING,		CVideoBridgeCOP::OnEndPartyConnectIVRModeDISCONNECTING)
//
  ONEVENT(PARTY_VIDEO_OUT_UPDATED,	CONNECTED,			CVideoBridgeCOPEq::NullActionFunction)
  ONEVENT(PARTY_VIDEO_OUT_UPDATED,	DISCONNECTING,		CVideoBridgeCOPEq::NullActionFunction)


PEND_MESSAGE_MAP(CVideoBridgeCOPEq, CVideoBridgeCOP);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~ constructors & destructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVideoBridgeCOPEq::CVideoBridgeCOPEq()
{
	 m_layoutType = CP_LAYOUT_1X1;
	 for(int i=(int)CP_LAYOUT_1X1;i<(int)CP_NO_LAYOUT;i++)
	 {
	   m_pReservation[i]= NULL;
	 }

}
// ------------------------------------------------------------------------------------------------------------------------
CVideoBridgeCOPEq::~CVideoBridgeCOPEq()
{
	for(int i=0; i<(int)CP_NO_LAYOUT; i++)
	{
	   if(IsValidPObjectPtr(m_pReservation[i]))
		   POBJDELETE(m_pReservation[i]);
	}
}
//================================================================================================================================================//
const char* CVideoBridgeCOPEq::NameOf () const
{
	return "CVideoBridgeCOPEq";
}
//================================================================================================================================================//
void*  CVideoBridgeCOPEq::GetMessageMap()
{
	return (void*)m_msgEntries;
}
//================================================================================================================================================//

//================================================================================================================================================//
// create of video bridge COPEq
//================================================================================================================================================//
void CVideoBridgeCOPEq::Create (const CVideoBridgeInitParams* pVideoBridgeInitParams)
{

  PTRACE(eLevelInfoNormal,"CVideoBridgeCOPEq::Create ");
  // validate params
  DWORD validation_status = ValidateInitParams(pVideoBridgeInitParams);
  if(STATUS_OK!=validation_status){
    PTRACE2INT(eLevelInfoNormal,"CVideoBridgeCOPEq::Create illegal params, status = ",validation_status);
    PASSERT(validation_status);
    //m_pConfApi->EndVidBrdgConnect(validation_status); m_pConfApi not created yet - will create exception
    return;
  }
  CBridge::Create((CBridgeInitParams*)pVideoBridgeInitParams);
  int i;
  for(i=(int)CP_LAYOUT_1X1;i<(int)CP_NO_LAYOUT;i++)
  {
     m_pReservation[i]= new CLayout((LayoutType)i,GetConfName());
  }

  m_state = CONNECTED;
  m_pConfApi->EndVidBrdgConnect(statOK);

}

//================================================================================================================================================//
void CVideoBridgeCOPEq::InitBridgeParams(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	CBridge::InitBridgeParams(pBridgePartyInitParams);
	//IF WE WILL INHERIT FROM VBCP we can call it instead of the code below!!!
	if(pBridgePartyInitParams->GetMediaOutParams())
	{
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetVisualEffects(m_pVisualEffects);
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetLayoutType(m_layoutType);

		ePartyLectureModeRole partyLectureModeRole = GetLectureModeRoleForParty(pBridgePartyInitParams->GetPartyName());
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetPartyLectureModeRole(partyLectureModeRole);
		((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetVideoQualityType((eVideoQuality)m_videoQuality);
        ((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetIsVideoClarityEnabled(m_isVideoClarityEnabled);
        ((CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams())->SetIsSiteNamesEnabled(m_isSiteNamesEnabled);
	}
	if(pBridgePartyInitParams->GetMediaInParams())
	{
		((CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams())->SetBackgroundImageID(m_pVisualEffects->GetBackgroundImageID());
                ((CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams())->SetIsVideoClarityEnabled(m_isVideoClarityEnabled);

	}

}
//================================================================================================================================================//
void CVideoBridgeCOPEq::OnConfTerminateDISCONNECTING(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CVideoBridgeCOPEq::OnConfTerminateDISCONNECTING");
	CVideoBridge::OnConfTerminateDISCONNECTING(pParam);
}
//================================================================================================================================================//
WORD CVideoBridgeCOPEq::IsValidState(WORD state)const
{
	WORD valid_state = TRUE;
	switch(state)
	{
		case INSWITCH:
		{
			valid_state = FALSE;
			PTRACE(eLevelInfoNormal,"CVideoBridgeCOPEq::IsValidState NOT VALID STATE!!!");
			PASSERT(state);
			break;
		 }
	}
	return valid_state;
}
//================================================================================================================================================//
WORD CVideoBridgeCOPEq::IsValidEvent(OPCODE event)const
{
	WORD valid_event = TRUE;
	switch(event)
	{
		case END_CONNECT_COP_ENCODER:
		case END_DISCONNECT_COP_ENCODER:
		case END_CLOSE_COP_DECODER:
		case END_DISCONNECT_COP_DECODER:
		case END_CONNECT_COP_DECODER:
		case VIDEO_REFRESH_COP_DECODER:
		case VIDEO_IN_SYNCED_COP_DECODER:
		case ENDCONNECTPARTY://should be only ENDCONNECTPARTY_IVR_MODE
		case SETCONFVIDLAYOUT_SEEMEALL:
		case SETCONFVIDLAYOUT_SEEMEPARTY:
		case SETPRIVATEVIDLAYOUT:
		case SETPRIVATEVIDLAYOUTONOFF:
		case PCM_CONNECTED:
		case PCM_DISCONNECTED:
		case PCM_UPDATED:
		case SWITCH_TOUT:
		case SMART_SWITCH_TOUT:
		case CONNECT_COP_VB_TOUT:
		case SET_MESSAGE_OVERLAY:
		{
	    	valid_event = FALSE;
	    	PTRACE(eLevelInfoNormal,"CVideoBridgeCOPEq::IsValidEvent NOT VALID EVENTS!!!");
	    	PASSERT(event);
	    	break;
	    }
	}
	return valid_event;
}
//================================================================================================================================================//
void CVideoBridgeCOPEq::OnConfDeletePartyFromConfCONNECTED(CSegment * pParam)
{
	PTRACE(eLevelInfoNormal,"CVideoBridgeCOPEq::OnConfDeletePartyFromConfCONNECTED!!!");

}
//================================================================================================================================================//

void  CVideoBridgeCOPEq::OnConfUpdateVideoMute(CSegment* pParam)
{
	WORD  srcReq = 0;
	EOnOff eOnOff = eOff;
	EMediaDirection eMediaDirection = eNoDirection;
	CVideoBridgePartyCntl* pPartyCntl = NULL;

	*pParam >> srcReq;

	if(srcReq==OPERATOR)
	{
		char name[H243_NAME_LEN];
		*pParam >> name >> (BYTE&)eOnOff >> (BYTE&)eMediaDirection;
		name[H243_NAME_LEN-1]='\0';
		pPartyCntl = (CVideoBridgePartyCntl*)GetPartyCntl(name);
	}
	else
	{
		PartyRsrcID partyRsrcID = INVALID;
		*pParam >> partyRsrcID >> (BYTE&)eOnOff >> (BYTE&)eMediaDirection;
		pPartyCntl = ( CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	}

	if(eMediaDirection != eMediaIn)
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeCOPEq::OnConfUpdateVideoMute Mute Video Out Not supported: Name - ",m_pConfName);
		DBGPASSERT_AND_RETURN(101);
	}

	if(NIL(CVideoBridgePartyCntl) == pPartyCntl)
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeCOPEq::OnConfUpdateVideoMute Party not connected to bridge: Name - ",m_pConfName);
		return;
	}

	RequestPriority reqPrio=AUTO_Prior;
	reqPrio = GetRequestPriority(srcReq);

	pPartyCntl->UpdateSelfMute(reqPrio, eOnOff);
}
//================================================================================================================================================//
void  CVideoBridgeCOPEq::OnConfVideoRefresh(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeCOPEq::OnConfVideoRefresh, Name - ",m_pConfName);

	PartyRsrcID partyRsrcID = INVALID;
	*pParam >> partyRsrcID;
	CVideoBridgePartyCntl*  pPartyCntl = ( CVideoBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	if ( !pPartyCntl )
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeCOPEq::OnConfVideoRefresh :the party is not connected to brdg, Name - ",m_pConfName);
		return;
	}
	if(pPartyCntl->IsConnectedStandalone())
	{
			PTRACE2(eLevelInfoNormal,"CVideoBridgeCOPEq::OnConfVideoRefresh : Party is in slide state, ask MFA for intra. request from party ",pPartyCntl->GetName());
			pPartyCntl->FastUpdate();
			return;
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CVideoBridgeCOPEq::OnConfVideoRefresh :the party should be in stand alone when connected to EQ Name - ",m_pConfName);
	}
}
//================================================================================================================================================//
void CVideoBridgeCOPEq::RemovePartyFromConfMixBeforeDisconnecting(const CTaskApp *pParty)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeCOPEq::RemovePartyFromConfMixBeforeDisconnecting Ignored - ",m_pConfName);
}

//================================================================================================================================================//
void CVideoBridgeCOPEq::OnConfUpdateFlowControlRate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CVideoBridgeCOPEq::OnConfUpdateFlowControlRate Ignored - ", m_pConfName);
}
