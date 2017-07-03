//+========================================================================+
//                 SIPTransInviteWebRtcWithSdpInd.h               	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteWithSdpInd.h                             	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef SIPTRANSINVITEWEBRTCWITHSDPIND_H_
#define SIPTRANSINVITEWEBRTCWITHSDPIND_H_

class CSipTransInviteWebRtcWithSdpInd : public CSipTransInviteWithSdpInd
{
CLASS_TYPE_1(CSipTransInviteWebRtcWithSdpInd, CSipTransInviteWithSdpInd)

public:
	CSipTransInviteWebRtcWithSdpInd(CTaskApp * pOwnerTask);
	virtual ~CSipTransInviteWebRtcWithSdpInd();
	virtual const char* NameOf() const {return "CSipTransInviteWebRtcWithSdpInd";}

	virtual void OnWebRtcPartyEstablishCallIdle(CSegment* pParam);
	virtual void OnWebRtcConnectFailure(CSegment* pParam);
	virtual void OnWebRtcConnectTout(CSegment* pParam);

protected:
	PDECLAR_MESSAGE_MAP

	virtual BOOL MakeANewCallOnPartyEstablishCallIdle(CSipComMode * pBestMode);
	CSipComMode *m_pBestMode;
};

#endif /*SIPTRANSINVITEWEBRTCWITHSDPIND_H_*/
