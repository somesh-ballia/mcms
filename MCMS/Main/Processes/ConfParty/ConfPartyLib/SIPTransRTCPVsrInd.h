//+========================================================================+
//               SIPTransRTCPVsrInd.h             	                       |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransRTCPVsrInd.h                           	           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef SIPTRANSRTCPVSRIND_H_
#define SIPTRANSRTCPVSRIND_H_

#include "SIPTransaction.h"

class CSIPTransRTCPVsrInd : public CSipTransaction
{
CLASS_TYPE_1(CSIPTransRTCPVsrInd,CSipTransaction)

public:
	CSIPTransRTCPVsrInd(CTaskApp * pOwnerTask);
	virtual ~CSIPTransRTCPVsrInd();
	virtual const char* NameOf() const {return "CSIPTransRTCPVsrInd";}

	void OnPartyReceivedRTCPVsrInd(CSegment* pParam);
	void OnConfBridgesUpdatedUpdateBridges(CSegment* pParam);
	void OnUpdateBridgesTout(CSegment* pParam);
	void OnPartyChannelsUpdated(CSegment* pParam);
	void OnPartyReceivedFullPacsiInfo(CSegment* pParam);

	void SendSingleUpdatePacsiInfoToParty(BYTE isMute);
	ERtvVideoModeType GetRtvVideoModeTypeByRes(DWORD height, DWORD width);

	void UpdateVideoBitRateCausedByFecIfNeeded(CapEnum algorithm, DWORD frameRate, DWORD vidBitRate,
			                                   DWORD tempMaxRateForSvcOrRtv,APIU16 (&qualityReportHistogram)[VSR_NUM_OF_QUALITY_LEVELS]);
	void UpdateFecParams(APIU16 (&qualityReportHistogram)[VSR_NUM_OF_QUALITY_LEVELS]);

protected:
	BYTE	m_isKbitOn;
	BYTE    m_isNeedToUpdateArtRegardingFEC;

	PDECLAR_MESSAGE_MAP

};

#endif /* SIPTRANSRTCPVSRIND_H_ */
