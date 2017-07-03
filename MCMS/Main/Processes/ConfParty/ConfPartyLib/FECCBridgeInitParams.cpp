//+========================================================================+
//                   FECCBridgeInitParams.CPP                             |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       FECCBridgeInitParams.CPP                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Romem                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  Oct-2005  | Description                                    |
//-------------------------------------------------------------------------|


#include "FECCBridgeInitParams.h"


// ------------------------------------------------------------
CFECCBridgeInitParams::CFECCBridgeInitParams():
	CBridgeInitParams(),m_bitRate(0)
{

}

// ------------------------------------------------------------
CFECCBridgeInitParams::CFECCBridgeInitParams (const CConf* pConf, const char*	pConfName,ConfRsrcID confRsrcId,
							const EBridgeImplementationTypes eBridgeImplementationType,
							BYTE FECCBitRate):CBridgeInitParams(pConf, pConfName,confRsrcId, eBridgeImplementationType),
							m_bitRate(FECCBitRate)
{
}
// ------------------------------------------------------------
CFECCBridgeInitParams::~CFECCBridgeInitParams ()
{
	
}



