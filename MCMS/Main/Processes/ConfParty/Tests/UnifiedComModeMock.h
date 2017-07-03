//+========================================================================+
//                      UnifiedComModeMock.h                                       |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       UnifiedComModeMock.h	                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Romem                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |  7 July-2005  |                                                      |
//+========================================================================+


#ifndef _UNIFIEDCOMMODEMOCK_H__
#define _UNIFIEDCOMMODEMOCK_H__

#include "UnifiedComMode.h"

class CUnifiedComModeMock : public CUnifiedComMode
{
public:
	virtual ~CUnifiedComModeMock () {}
	CUnifiedComModeMock (BYTE amcFixedContentRate = 0) : CUnifiedComMode(amcFixedContentRate,0) {}
	CUnifiedComModeMock (const CUnifiedComModeMock& rBridgePartyCntl): CUnifiedComMode(rBridgePartyCntl) {}
	CUnifiedComModeMock& operator= (const CUnifiedComModeMock& rOther) { (CUnifiedComMode&)(*this) = (CUnifiedComMode&)rOther; return *this; }

	CIpComMode* GetIPSCM() const {return m_pIPScm;}
};

#endif /* _UNIFIEDCOMMODEMOCK_H__ */
