//+========================================================================+
//                   FECCBridgeInitParams.H                               |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       FECCBridgeInitParams.H                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Romem                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  OCT-2005  | Description                                    |
//-------------------------------------------------------------------------|

#ifndef _FECC_BRIDGE_INIT_PARAMS_
#define _FECC_BRIDGE_INIT_PARAMS_

#include "BridgeInitParams.h"

class CConf;
  
class CFECCBridgeInitParams : public CBridgeInitParams 
{
CLASS_TYPE_1(CFECCBridgeInitParams,CBridgeInitParams)
public:
	CFECCBridgeInitParams (const CConf* pConf, const char*	pConfName,ConfRsrcID confRsrcId,
							const EBridgeImplementationTypes eBridgeImplementationType,
							BYTE FECCBitRate);
	virtual const char* NameOf() const { return "CFECCBridgeInitParams";}
	CFECCBridgeInitParams ();
	virtual ~CFECCBridgeInitParams ();

	void SetFECCBitRate(WORD bitRate)		{ m_bitRate = bitRate; }

	WORD GetFECCBitRate()	const	{ return m_bitRate; }

private:
	CFECCBridgeInitParams (const CFECCBridgeInitParams&);
	CFECCBridgeInitParams& operator = (const CFECCBridgeInitParams);

	WORD m_bitRate;
};

#endif //_FECC_BRIDGE_INIT_PARAMS_


