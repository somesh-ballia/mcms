//+========================================================================+
//               SIPTransRTCPVideoUpdateInd.h             	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransRTCPVideoUpdateInd.h                           	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef SIPTRANSRTCPVIDEOUPDATEIND_H_
#define SIPTRANSRTCPVIDEOUPDATEIND_H_

class CSipTransRTCPVideoUpdateInd : public CSipTransaction
{
CLASS_TYPE_1(CSipTransRTCPVideoUpdateInd,CSipTransaction)

public:
	CSipTransRTCPVideoUpdateInd(CTaskApp * pOwnerTask);
	virtual ~CSipTransRTCPVideoUpdateInd();
	virtual const char* NameOf() const {return "CSipTransRTCPVideoUpdateInd";}

	void OnPartyReceivedRTCPVideoChanges(CSegment* pParam);
	void OnConfBridgesUpdatedUpdateBridges(CSegment* pParam);
	void OnUpdateBridgesTout(CSegment* pParam);

protected:
	PDECLAR_MESSAGE_MAP

};

#endif /* SIPTRANSRTCPVIDEOUPDATEIND_H_ */
