//+========================================================================+
//                GideonSimLogicalModule.cpp                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimLogicalModule.cpp                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimLogicalModule.cpp:
//
/////////////////////////////////////////////////////////////////////////////

#include "Macros.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "InternalProcessStatuses.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesMcmsCardMngrMaintenance.h"
#include "OpcodesMcmsCardMngrRecording.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsAudioCntl.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsRtmIsdnMaintenance.h"
#include "OpcodesMcmsNetQ931.h"  // for NET_ opcodes (until implemented by other sides)
#include "CardsStructs.h"
#include "IpMfaOpcodes.h"
#include "IpRtpReq.h"
#include "IpRtpInd.h"
#include "IpCmReq.h"
#include "IpCmInd.h"
#include "AcIndicationStructs.h"
#include "VideoStructs.h"
#include "IvrApiStructures.h"
#include "IVRPlayMessage.h"
#include "AudRequestStructs.h"
#include "AudIndicationStructs.h"
#include "McuMngrStructs.h"
#include "Q931Structs.h"

#include "SystemFunctions.h"
#include "StateMachine.h"
#include "TaskApi.h"
#include "MplMcmsProtocol.h"
#include "MplMcmsProtocolTracer.h"

#include "GideonSim.h"
#include "GideonSimConfig.h"
#include "GideonSimLogicalModule.h"
#include "GideonSimLogicalUnit.h"
#include "GideonSimCardAckStatusList.h"
#include "ObjString.h"
#include "VideoApiDefinitions.h"
#include "ArtDefinitions.h"

#include "Bonding.h"
#include "OpcodesMcmsBonding.h"
#include "muxint.h"
#include "OpcodesMcmsMux.h"
#include "AcRequestStructs.h"
#include "AcDefinitions.h"


#include "GideonSimLogicalParams.h"
#include "GideonSimTbComponentLogical.h"

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//   CTbComponent - class treats all Party / Conf commands
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CTbComponent)
	//
PEND_MESSAGE_MAP(CTbComponent,CStateMachine)


/////////////////////////////////////////////////////////////////////////////
CTbComponent::CTbComponent( CTaskApp *pOwnerTask)
    :CStateMachine(pOwnerTask)
{
}

/////////////////////////////////////////////////////////////////////////////
CTbComponent::~CTbComponent()
{
}


/////////////////////////////////////////////////////////////////////////////
void* CTbComponent::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//////////////////////////////////////////////////////////////////////////////
void CTbComponent::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}

