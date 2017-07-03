/*
 * SIPSlavePartyControlChangeMode.h
 *
 *  Created on: Jun 14, 2011
 *      Author: ami
 */

#ifndef SIPSLAVEPARTYCONTROLCHANGEMODE_H_
#define SIPSLAVEPARTYCONTROLCHANGEMODE_H_

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

class CSipSlaveChangeModePartyCntl: public CSipChangeModePartyCntl
{
CLASS_TYPE_1(CSipSlaveChangeModePartyCntl, CSipChangeModePartyCntl)
public:
	// Constructors
	CSipSlaveChangeModePartyCntl();
	virtual ~CSipSlaveChangeModePartyCntl();

	// Initializations

    // Operations
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	CSipSlaveChangeModePartyCntl& operator= (const CSipSlaveChangeModePartyCntl& other);
	// messages from conf
	//virtual void ChangeScm(CIpComMode* pScm);
	virtual void EndChangeMode();
	void OnPartyRemoteConnectedSecondTime(CSegment* pParam);
	virtual const char*  NameOf() const;


protected:

	PDECLAR_MESSAGE_MAP;

	eVideoPartyType m_VideoPartyType;
};


#endif /* SIPSLAVEPARTYCONTROLCHANGEMODE_H_ */

