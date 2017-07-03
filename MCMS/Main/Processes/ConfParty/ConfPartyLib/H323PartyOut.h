/*$Header: /MCMS/MAIN/subsys/mcms/PRT323OU.H 18    26/03/01 20:49 Matvey $*/
//+========================================================================+
//                            PRT323OU.CPP                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PRT323OU.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Michel                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 3/17/96     |                                                     |
//+========================================================================+

#ifndef _PRT323OU
#define _PRT323OU

#include "H323Party.h"

//#ifndef _H323PARTY
//#include  <H323part.h>
//#endif
//#ifndef _TPKTTASK
//#include  <tpkttask.h>
//#endif
//#ifndef _PARTY
//#include  <party.h>
//#endif
//
extern "C" void PartyH323OutEntryPoint(void* appParam);


// compile time flags
// __NETSETUP_AT_H323CNTL__ to pass the netsetup to the H323cntl
// __AUTHENTICATION__ until the system will support authentication and encryption



class CCapH323;
class CComModeH323;

class CH323PartyOut : public CH323Party
{
	CLASS_TYPE_1(CH323PartyOut, CH323Party)
public:             
	
	// Constructors
	CH323PartyOut();
	virtual ~CH323PartyOut();  
	virtual const char* NameOf() const { return "CH323PartyOut";}
	
	// Initializations
	void  Create(CSegment& appParam);	
	
	// Operations
	virtual void*	 GetMessageMap(); 
	void SetCallId(CH323NetSetup* pH323NetSetup);
//	virtual TCall* GetMcCallPtr();
	void	H323CallDrop();
//	void  CleanUp();
//	void  DisconnectH323Cntl();
	void  SetVideoRate(DWORD vidRate);
//	// Action functions
//	void  OnConfDisconnectIdle(CSegment* pParam);
	void  OnConfH323EstablishCallPartyIdle(CSegment* pParam);
//	void  OnPartyProceeding(CSegment* pParam);
//	void  OnPartyConnected(CSegment* pParam);
//	void  OnPartyCallConnected(CSegment* pParam);
	void  OnConfDisconnect(CSegment* pParam);
//	void  OnNetDisconnectChangeMode(CSegment* pParam);
//	void  OnNetDisconnectConnect(CSegment* pParam);
//	void  OnNetDisconnect(CSegment* pParam);
	void  OnH323PartyDisconnect(CSegment* pParam);
	virtual void  OnEndH323Disconnect(CSegment* pParam);
	void  OnH323DisconnectIdle(CSegment* pParam);
	void  OnPartyTranslatorArtsConnected(CSegment* pParam);
	void  ContinueEstablishCall();

	
	// Attributes 
protected: 
	//-------------------------------------
//	CTpktTask*									m_pTpktTask;
// 	CRsrcTbl*									m_pRsrcTbl;
//	CPartyApi*									m_pTaskApi;  
//	H323_MCREQ_CALL_SETUP_S*					m_pReqCallSetup;
//	H323_MCREQ_CALL_ANSWER_S*					m_pReqCallAnswer;
//	H323_MCREQ_CREATE_CNTL_S*					m_pReqCreateCntl;
//	H323_MCREQ_OUTGOING_CHNL_S*					m_pReqOutChnl;
//	H323_MCREQ_INCOMING_CHNL_RESPONSE_S*		m_pReqInChnlResponse;
//	H323_MCREQ_RTP_STREAM_ON_S*					m_pReqStreamOn;
//	H323_MCREQ_VIDEO_UPDT_PIC_S*				m_pReqVideoUpdatePic;
	
	WORD										m_disconnectionCause;
	PDECLAR_MESSAGE_MAP   
};

#endif /* _PRT323OU */

