//+========================================================================+
//                     EndpointsSim.h                                      |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EndpointsSim.h                                              |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __ENDPOINTSSIM_
#define   __ENDPOINTSSIM_


////////////////////////////////////////////////////////////////////////////
///
///     INCLUDES
///
#include "DataTypes.h"
#include "StatusesGeneral.h"

////////////////////////////////////////////////////////////////////////////
///
///     Define
///
#define DONOT_CARE	77

////////////////////////////////////////////////////////////////////////////
///
///     DECLARATIONS
///
class CEndpointsSimSystemCfg;
class CSegment;
class CMplMcmsProtocol;

////////////////////////////////////////////////////////////////////////////
///
///     GLOBALS
///
static CEndpointsSimSystemCfg* g_pEpSystemCfg = NULL;


////////////////////////////////////////////////////////////////////////////
///
///     EXTERNALS
///
//system configuration
extern CEndpointsSimSystemCfg* GetEpSystemCfg();
extern void SetEpSystemCfg(CEndpointsSimSystemCfg* p);

extern void SendIsdnMessageToGideonSimApp(CSegment& pParam);
extern void SendAudioMessageToGideonSimApp(CSegment& pParam);
extern void SendMuxMessageToGideonSimApp(CSegment& pParam);
extern void SendScpMessageToGideonSimApp(CSegment& rParam);
extern void FillCsProtocol(CMplMcmsProtocol* pMplProt,
		                   DWORD csID,
                           DWORD opcode,
                           const BYTE* pData,
                           DWORD nDataLen,
                           DWORD nCsCallIndex = 0,
                           DWORD channelIndexParam = 0xFFFFFFFF,
                           DWORD channelMcIndexParam = 0xFFFFFFFF);

////////////////////////////////////////////////////////////////////////////
///
///     DATA TYPES
///
enum eEndpointsTypes {
	eEndPointH323  = 0,
	eEndPointSip   = 1,
	eEndPointPstn  = 2,
	eEndPointIsdn  = 3,
	eEndPointUnknown // must be last
};

enum eDtmfSourceTypes {
	eDtmfSourceAUDIO  		= 0,
	eDtmfSourceRTP     		= 1,
	eDtmfSourceSIGNALLING  	= 2
};


////////////////////////////////////////////////////////////////////////////
///
///     CONSTANTS
///



#endif /* __ENDPOINTSSIM_ */


