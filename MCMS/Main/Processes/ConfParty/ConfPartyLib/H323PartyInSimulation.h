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

#ifndef _H323PARTYINSIMULATION
#define _H323PARTYINSIMULATION

#include "H323Party.h"
#include "H323NetSetup.h"
#include "H323Control.h"



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
class CPrtMontrBaseParams;
class CH323NetSetup;

extern "C" void H323PartyInSimulationEntryPoint(void* appParam);

class CH323PartyInSimulation : public CParty
{
	CLASS_TYPE_1(CH323PartyInSimulation, CParty)
public:             

	// Constructors
	CH323PartyInSimulation();
	virtual ~CH323PartyInSimulation();  
	virtual const char* NameOf() const { return "CH323PartyInSimulation";}
 
		  // Initializations
	void  Create(CSegment& appParam);	

		  // Operations

		  // Action functions
	void  OnLobbyTransferSetup(CSegment* pParam); 
	void  OnLobbyH323IdentIdle(CSegment* pParam); 
	void  OnLobbyRejectCallIdle(CSegment* pParam); 
	void  OnConfEstablishCallSetup(CSegment* pParam);  

	void  OnNetDisconnectSetUp(CSegment* pParam);
	void  OnEndH323Disconnect(CSegment* pParam);

//	void  SetMcCallStruct();
	void  OnConfDisconnect(CSegment* pParam);
	void  OnNetDisconnectConnect(CSegment* pParam);
	void  OnNetDisconnect(CSegment* pParam);
	void  OnEndLobbyH323Disconnect(CSegment* pParam);
	void  OnH323EndChannelConnectSetupOrConnect();

	virtual void CleanUp();
	void  H323CallDrop();

	WORD  GetIsReject()		{ return m_isReject;	 }
	WORD  GetIsLobbySetup()	{ return m_isLobbySetup; }

protected:        
	void  OnH323LogicalChannelConnect(DWORD channelType);
	void  H323EndChannelConnect();
	void  H323UpdateBaudRate();
//	void  OnEndH323Disconnect();
	void  H323LogicalChannelDisConnect(DWORD channelType, WORD dataType, WORD bTransmitting, BYTE roleLabel, BYTE bUpdateCommMode);
	void  InitAndSetPartyMonitor(CPrtMontrBaseParams *pPrtMonitrParams, DWORD channelType);

		  // Attributes 
	CLobbyApi*     m_pLobbyApi; 
	//CPartyApi*	   m_pPartyInApi;  
//	CTpktTask*	   m_pTpktTask;

	WORD           m_identChnl;
	WORD           m_identTix;
	WORD           m_isIdent;
	WORD           m_isReject;
	WORD		   m_isLobbySetup;
	WORD		   m_isAddPartyOK;
	WORD           m_isEndInitComm; //should be in Cparty  
 	int			   m_rejectCallReason;
//parameter for call forward
	char*		   m_forwardAlias;
	CH323NetSetup* m_pH323NetSetup;
	CH323Cntl*     m_pH323Cntl;
    CComModeH323*  m_pInitialModeH323;
    CCapH323*      m_pLocalCapH323;
	DWORD          m_videoRate;


	PDECLAR_MESSAGE_MAP   
};


#endif /* _H323PARTYINSIMULATION */
