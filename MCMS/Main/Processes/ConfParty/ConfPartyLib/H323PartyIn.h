/*$Header: /MCMS/MAIN/subsys/mcms/H323PAIN.H 10    11/12/01 21:34 Uria $*/
//+========================================================================+
//                            H323PAIN.H                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323PAIN.H                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 6/20/99     |                                                     |
//+========================================================================+

#ifndef _H323PARTYIN
#define _H323PARTYIN

#include "H323Party.h"

//#ifndef _PARTY
//#include  <party.h>
//#endif

//#ifndef _PARTYAPI
//#include  <partyapi.h>
//#endif
//
//#ifndef _TPKTTASK
//#include  <tpkttask.h>
//#endif
//
//#ifndef _H323EMB_H
//#include  <h323emb.h>
//#endif
//

// defined compilation flags in H323party files
// __Resources__
// __AUTHENTICATION__
// __Sinai__
// __NETSETUP_AT_H323CNTL__

class CH323Cntl;
class CCapH323;
class CLobbyApi;

extern "C" void H323PartyInEntryPoint(void* appParam);

class CH323PartyIn : public CH323Party
{
	CLASS_TYPE_1(CH323PartyIn, CH323Party)
public:             

	// Constructors
	CH323PartyIn();
	virtual ~CH323PartyIn();  
	virtual const char* NameOf() const { return "CH323PartyIn";}
 
		  // Initializations
	void  Create(CSegment& appParam);	

		  // Operations

		  // Action functions
	void  OnLobbyTransferSetup(CSegment* pParam); 
	void  OnLobbyH323IdentIdle(CSegment* pParam); 
	void  OnLobbyRejectCallIdle(CSegment* pParam); 
	void  OnConfEstablishCallSetup(CSegment* pParam);  

	void  OnNetDisconnectSetUp(CSegment* pParam);
	virtual void  OnEndH323Disconnect(CSegment* pParam);

//	void  SetMcCallStruct();
	void  OnConfDisconnect(CSegment* pParam);
	void  OnNetDisconnectConnect(CSegment* pParam);
	void  OnNetDisconnect(CSegment* pParam);
	void  OnEndLobbyH323Disconnect(CSegment* pParam);
	void  OnH323DisconnectIdleSetUp(CSegment* pParam);

	virtual void CleanUp();
	void  H323CallDrop();

	WORD  GetIsReject()		{ return m_isReject;	 }
	WORD  GetIsLobbySetup()	{ return m_isLobbySetup; }
	void  OnPartyTranslatorArtsConnected();
	void  ContinueEstablishCall();

protected:        
		  // Attributes 
	CLobbyApi*     m_pLobbyApi; 
	CPartyApi*	   m_pPartyInApi;  
//	CTpktTask*	   m_pTpktTask;

	WORD           m_identChnl;
	WORD           m_identTix;
	WORD           m_isIdent;
	WORD           m_isReject;
	WORD		   m_isLobbySetup;
	WORD		   m_isAddPartyOK;
	WORD           m_isEndInitComm; //should be in Cparty  
 	int			   m_rejectCallReason;
	DWORD		   m_DialInRejectConnectionId;
//parameter for call forward

	PDECLAR_MESSAGE_MAP   
};


#endif /* _H323PARTYIN */
