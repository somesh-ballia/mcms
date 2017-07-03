//+========================================================================+
//                   BondingMuxCntl.H                                      |
//		     Copyright 2005 Polycom, Inc                                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       BondingMuxCntl.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Olga                                                        |
//-------------------------------------------------------------------------|
// Who  | Date  12-2007  | Description                                     |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _BONDING_MUX_CNTL_H_
#define _BONDING_MUX_CNTL_H_

//==============================================================================================================//
#include "BondingCntl.h"
#include "MuxCntl.h"
#include "PartyApi.h"
#include "StateMachine.h"




class CBondingMuxCntl : public CStateMachine
{
CLASS_TYPE_1(CBondingMuxCntl, CStateMachine)	
public: 
  // Constructors
  CBondingMuxCntl(CBondingCntl* pBnd, CMuxCntl* pMux, CRsrcParams& rsrcDesc, CParty* pParty);
  virtual ~CBondingMuxCntl();
  

  // Operations
  virtual const char*  NameOf() const;
  virtual void   HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);
  virtual BOOL   DispatchEvent(OPCODE event,CSegment* pParam = NULL);

 protected:

  CMuxCntl*      m_pMuxCntl;
  CBondingCntl*  m_pBndCntl;

  // api to party 
  CPartyApi*          m_pTaskApi;
};

//==============================================================================================================//

#endif // _BONDING_MUX_CNTL_H_
