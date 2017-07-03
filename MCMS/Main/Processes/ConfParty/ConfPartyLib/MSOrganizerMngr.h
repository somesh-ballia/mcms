//+========================================================================+
//                     MSOrganizerMngr.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MSOrganizerMngr.h	                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | July-2013  |                                                      |
//+========================================================================+


#ifndef MSORGANIZERMNGR_H_
#define MSORGANIZERMNGR_H_

#include "MSAvMCUMngr.h"
#include "EventPackage.h"

class CMSOrganizerMngr : public CMSAvMCUMngr
{
CLASS_TYPE_1(CMSOrganizerMngr,CMSAvMCUMngr)

public:

	// Constructors:
			CMSOrganizerMngr();
	virtual ~CMSOrganizerMngr();
	CMSOrganizerMngr(const CMSOrganizerMngr &other);

	// Initializations and settings:
	virtual const char*  NameOf() const  { return "CMSOrganizerMngr"; }
	virtual void* GetMessageMap() {return (void*)m_msgEntries;}

	void Create(CRsrcParams* RsrcParams/*, CPartyApi* pPartyApi*/,CConf* pConf,sipSdpAndHeadersSt* MsConfReq,DWORD PartyId,CSipNetSetup* SipNetSetup,DWORD ServiceId,BYTE isRejectForEscalation = FALSE);
	void BuildResponse(sipSdpAndHeadersSt* MsConfReq,CSipNetSetup* SipNetSetup,BYTE IsReject);
	//BYTE SipRingingReq(sipSdpAndHeadersSt* MsConfReq,CSipNetSetup* SipNetSetup,BYTE IsReject);
	STATUS ParseXML(DWORD partyId);

	void OnSipInviteAckIndMSConnecting(CSegment* pParam);
	void OnTimerConnectOrganizer(CSegment* pParam);
	//void OnSipByeInd(CSegment* pParam);
	const char* GetPlcmCallId(sipSdpAndHeadersSt* MsConfReq);

protected:

	sipSdpAndHeadersSt * 	m_LyncSdp;
	EventPackage::ConfInvite*			m_MsConfInviteParser;
	char*                   m_MsConversationId;
	BOOL                    m_IsCallThroughDma;//only using DMA we will support simulcast
	BYTE                    m_isRejectForEscalation;



	PDECLAR_MESSAGE_MAP



};
#endif /* MSORGANIZERMNGR_H_ */

