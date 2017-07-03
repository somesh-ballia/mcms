//=================================================================================================
//
//Copyright (C) 2000 ACCORD Networks Ltd.
//This file contains confidential information proprietary to ACCORD Networks Ltd. The use or 
//disclosure of any information contained in this file without the written consent of an officer of
//ACCORD Networks Ltd. is expressly forbidden.
//
//=================================================================================================

//=================================================================================================
//
//Module Name:  Partyov.h
//
//General Description:  
//
//    1. 
//    2.
//
//Generated By: Natalie                            Date: 14.05.00
//
//Revisions and Updates: 
//
//Date         Updated By         Description
//========   ==============   =====================================================================

//=================================================================================================

#ifndef _ISDN_PARTY_VOICE_IN_H_
#define _ISDN_PARTY_VOICE_IN_H_


#include "IsdnVoiceParty.h"
#include "LobbyApi.h"

extern "C" void PartyVoiceInEntryPoint(void* appParam);

class CIsdnVoicePartyIn : public CIsdnVoiceParty
{
  CLASS_TYPE_1(CIsdnVoicePartyIn, CIsdnVoiceParty)
 public:             
  // Constructors
  CIsdnVoicePartyIn();
  virtual ~CIsdnVoicePartyIn();
	
  void  Create(CSegment& appParam);	
  const char* NameOf()  const { return "CIsdnVoicePartyIn"; }
	
  // Operations
  // action functions
  void  OnLobbyTransferSetup(CSegment* pParam); 
  void  OnConfEstablishCallSetup(CSegment* pParam);  
  virtual void  OnLobbyNetIdentIdle(CSegment* pParam); 
  void  OnEndNetDisconnect(CSegment* pParam);
  void  IlligaleActionFuntion(CSegment* pParam);
  void  OnAudioPlusVTXStatusIndVTXSetup(CSegment* pParam);
  void  OnVTXSetupTimeOut(CSegment* pParam);
  void  OnConfEstablishCall(CSegment* pParam);
  void  OnNetConnectSetUp(CSegment* pParam); 

 public:             
  void  PropgateNetEvents(CSegment* Msg,WORD chanNum = 0 );             

 protected: 
  //void  H221ConnectVoice(BYTE EndPointVtxSupport = 0, WORD VtxVoiceAlg = 0);
  //void  MuxEndH221ConSetUp(CCapH221 *pRmtCap,CComMode *pRmtComMode);

  // Attributes 
  CLobbyApi*           m_pLobbyApi;   
  //WORD                 m_identChnl;
  //WORD                 m_identTix;
  //WORD                 m_isEndInitComm; //should be in Cparty  
	  
  //WORD                 m_isReject;

  PDECLAR_MESSAGE_MAP 

};

#endif