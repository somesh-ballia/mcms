/*
 * MSSlaveChangeModeSipPartyCntl.h
 *
 *  Created on: Jun 14, 2011
 *      Author: ami
 */

#ifndef MSSLAVECHANGEMODESIPPARTYCNTL_H_
#define MSSLAVECHANGEMODESIPPARTYCNTL_H_

//+========================================================================+
//                            SIPSlavePartyControlChangeMode.h                      |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SIPSlavePartyControlChangeMode.CPP                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

class CMSSlaveChangeModeSipPartyCntl: public CSipChangeModePartyCntl
{
CLASS_TYPE_1(CMSSlaveChangeModeSipPartyCntl, CSipChangeModePartyCntl)
public:
	// Constructors
	CMSSlaveChangeModeSipPartyCntl();
	virtual ~CMSSlaveChangeModeSipPartyCntl();

	// Initializations
    // Operations
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	CMSSlaveChangeModeSipPartyCntl& operator= (const CMSSlaveChangeModeSipPartyCntl& other);
	// messages from conf
	//virtual void ChangeScm(CIpComMode* pScm);
	virtual const char*  NameOf() const;
	//void StartSingleVsr(const TCmRtcpVsrInfo& vsrInfo);
	void OnConfChangeModeIdle(CSegment* pParam);
	void OnSlavesControllerSingleVsrMassageInd(CSegment* pMsg);

	void SendSingleUpdatePacsiInfoToSlavesController(const MsSvcParamsStruct& pacsiInfo,  BYTE isReasonFecOrRed=FALSE);

protected:

	PDECLAR_MESSAGE_MAP;

	eVideoPartyType m_VideoPartyType;
};


#endif /* SIPSLAVEPARTYCONTROLCHANGEMODE_H_ */

