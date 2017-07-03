//+========================================================================+
//                     MSFocusMngr.h		                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MSFocusMngr.h		                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | Sept-2013  |                                                      |
//+========================================================================+


#ifndef MSFOCUSMNGR_H_
#define MSFOCUSMNGR_H_

#include "MSAvMCUMngr.h"
#include "Response.h"


class CMSFocusMngr : public CMSAvMCUMngr
{
CLASS_TYPE_1(CMSFocusMngr,CMSAvMCUMngr)

public:

	// Constructors:
			CMSFocusMngr();
	virtual ~CMSFocusMngr();
	CMSFocusMngr(const CMSFocusMngr &other);

	// Initializations and settings:
	virtual const char*  NameOf() const  { return "CMSFocusMngr"; }
	virtual void* GetMessageMap() {return (void*)m_msgEntries;}

	void Create(CRsrcParams* RsrcParams,CConf* pConf,DWORD PartyId,CSipNetSetup* SipNetSetup,DWORD ServiceId,char* FocusUri,const char* strConfParamInfo);
	void BuildAddUserMsg(CSipNetSetup* SipNetSetup,const char* strConfParamInfo);
	void BuildXMLBudy(CSipNetSetup* SipNetSetup,std::ostream &ostr);
	STATUS ParseAddUserXML(sipSdpAndHeadersSt * LyncSdp);

	void OnTimerAddUserConnect(CSegment* pParam);
	void OnSipInviteResponseConnecting(CSegment* pParam);
	void OnCsProvisunalResponseInd(CSegment* pParam);

	void SendInviteAckReq();
	void RemoveFocuseConnection();

	void TerminateFocusConnection();
	void OnSipBye200OkInd(CSegment* pParam);
	void OnTimerDisconnectMngr(CSegment* pParam);

	void OnSipByeIndConnecting(CSegment* pParam);
	void OnSipByeInd(CSegment* pParam);


protected:


	EventPackage::Response* 				m_MSAddUserResponse;
	//EventPackage::AddUser*			    m_MsAddUserInvite;


	PDECLAR_MESSAGE_MAP



};


#endif /* MSFOCUSMNGR_H_ */
