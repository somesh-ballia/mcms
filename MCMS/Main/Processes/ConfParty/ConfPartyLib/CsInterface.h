//+========================================================================+
//                       CsInterface.h	                                   |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       CsInterface.h	                                           |
// SUBSYSTEM:  CS/ConfParty                                                |
// PROGRAMMER: Guy D,													   |
// Date : 15/6/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        
/*

This header file contains the interface  for requests between CS and ConfParty 
within the Carmel.
*/

#ifndef __CSINTERFACE_H__
#define __CSINTERFACE_H__

#include "Interface.h"
#include "RsrcParams.h"
#include "MplMcmsProtocol.h"

// An abstract class for all HARDWARE resource objects.
class CCsInterface : public CInterface
{
	CLASS_TYPE_1(CCsInterface, CInterface)

public: 
	// Constructors
	CCsInterface();
	virtual const char* NameOf() const { return "CCsInterface";}
	virtual ~CCsInterface();  
	CCsInterface(const CCsInterface&);
	
	// Initializations  
	void  Create(CRsrcParams* pRsrcParams);
	
	// Operations
	void				UpdateRsrcParams(CRsrcParams* pRsrcParams);
	CRsrcParams*		GetRsrcParams() const { return m_pRsrcParams;};
	DWORD				GetPartyRsrcId() const;
	DWORD				GetConfRsrcId() const;
//	CMplMcmsProtocol*	GetMplMcmsProtocol() const { return m_pMplMcmsProtocol;};
	void				SetPartyRsrcId(DWORD partyRsrcId);
	void				SetConfRsrcId(DWORD confRsrcId);
	void				SendMsgToCS(OPCODE opcode, CSegment* pseg1, WORD csId = 0, DWORD csServiceId = 0, WORD csDestUnitId = 0, DWORD callIndex = 0, DWORD channelIndex = 0, DWORD mcChannelIndex = 0, APIS32 status = 0 );

	// TDD Additions
	void				SetTddMockInterface(CMplMcmsProtocol* pMockMplMcmsProtocol);
protected:
	
	// Attributes
	CRsrcParams*		m_pRsrcParams;
//	CMplMcmsProtocol*	m_pMplMcmsProtocol;
	
	// Operations	
};

#endif /* __CSINTERFACE_H__  */
