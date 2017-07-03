
//+========================================================================+
//            MplMcmsProtocolSizeValidator.h                               |
//            Copyright 2006 Polycom Israel Ltd.				           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.					   |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MplMcmsProtocolSizeValidator.h                                             |
// PROGRAMMER: Keren                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |13/06/07    |                                                     |
//+========================================================================+
#ifndef MPL_MCMS_PROTOCOL_SIZE_VALIDATOR_H_
#define MPL_MCMS_PROTOCOL_SIZE_VALIDATOR_H_

#include "PObject.h"

#include "MplMcmsProtocol.h"

#include "IvrApiStructures.h"
#include "TBStructs.h"
#include "VideoStructs.h"
#include "IpRtpReq.h"
#include "H323CsReq.h"
#include "H323CsInd.h"
#include "IpCsContentRoleToken.h"
#include "IpRtpFeccRoleToken.h"
#include "IpCmReq.h"
#include "AudRequestStructs.h"
#include "AcRequestStructs.h"
#include "AcIndicationStructs.h"
#include "ConfStructs.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsAudioCntl.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsInternal.h"
#include "SysConfig.h"
#include "RecordingRequestStructs.h"
#include "OpcodesMcmsCardMngrRecording.h"
#include "OpcodesMcmsNetQ931.h"
#include "ArtRequestStructs.h"
#include "Bonding.h"
#include "OpcodesMcmsBonding.h"
#include "OpcodesMcmsMux.h"
#include "muxint.h"
#include "OpcodesMcmsShelfMngr.h"
#include "CardsStructs.h"

#define MAX_VALIDATE_SIZE_OPCODES  80

typedef struct
{
	OPCODE opcode;
	DWORD size;
} MplMcmsProtocolSizeValidatorStruct;

class CMplMcmsProtocolSizeValidator : public CPObject
{
    CLASS_TYPE_1(CMplMcmsProtocolSizeValidator, CPObject)
public:
    virtual const char* NameOf(void) const;
    DWORD GetContentSizeByOpcode(OPCODE opcode,
                                 BYTE logicRsrcType = eLogical_res_none) const;

private:
    static const MplMcmsProtocolSizeValidatorStruct g_MplMcmsProtocolOpcodeContentStuctSize[];
};

#endif  // MPL_MCMS_PROTOCOL_SIZE_VALIDATOR_H_
