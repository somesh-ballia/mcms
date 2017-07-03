//+========================================================================+
//                            MplApiRxSocketMngr.h                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MplApiRxSocketMngr.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 04/04/2005 |                                                      |
//+========================================================================+



/*#include "SocketTask.h"
#include "MplMcmsProtocol.h"

*/
#include "PObject.h"
#include "MplMcmsProtocol.h"
#include "AllocateStructs.h"
#include "SharedMemoryMap.h"

class CMplMcmsProtocol;
class COsQueue;
class CMplMcmsProtocolTracer;

class CMatchResourceByResourceTypeAndPartyId : public CSharedMemoryMap<ConnToCardTableEntry>::CSharedMemEntryCondition
{
public:
                CMatchResourceByResourceTypeAndPartyId(eResourceTypes type, DWORD partyId, const PHYSICAL_INFO_HEADER_S* physHeader)
                  : m_type(type), m_partyId(partyId), m_physHeader(physHeader)
                  {}

                virtual ~CMatchResourceByResourceTypeAndPartyId(){;}

                virtual BOOL IsMatch(const ConnToCardTableEntry& candidate)
                {
                                return ((candidate.m_id != EMPTY_ENTRY) && (m_type == candidate.physicalRsrcType) && (m_partyId == candidate.rsrc_party_id) &&
                                                                (m_physHeader->board_id == candidate.boardId && m_physHeader->unit_id == candidate.unitId &&
                                                                m_physHeader->accelerator_id == candidate.acceleratorId && m_physHeader->port_id == candidate.portId));
                }
private:
                eResourceTypes m_type;
                DWORD m_partyId;
                const PHYSICAL_INFO_HEADER_S* m_physHeader;
};


class CMplApiRxSocketMngr : public CPObject
{
CLASS_TYPE_1(CMplApiRxSocketMngr,CPObject)
public:

	// Constructors
	CMplApiRxSocketMngr();
	virtual const char* NameOf() const { return "CMplApiRxSocketMngr";}
	virtual ~CMplApiRxSocketMngr();

	// Initializations
	const char * GetTaskName() const ;

	DWORD SendMplEventToTheAppropriateProcess(CMplMcmsProtocol* mplPrtcl);
	void SendSocketsConIdPerMFA_CardNum(WORD card_number,WORD conId,COsQueue* txMailslot);
//	void SendMplEventLogger (DWORD BufLen,char* Mainbuf);

	BOOL IsVideoEncoderMessage(CMplMcmsProtocol *mplPrtcl);
	BOOL HandleVideoEncoderOpenPortAckMessages(CMplMcmsProtocol *mplPrtcl);
	BOOL HandleVideoEncoderClosePortAckMessages(CMplMcmsProtocol *mplPrtcl);
	BOOL HandleVideoEncoderConnectAckMessages(CMplMcmsProtocol *mplPrtcl);
	void SendKillPortAckToConfPartyIfNeeded(CMplMcmsProtocol *mplPrtcl);
	void SendAllocStatusAckToConfPartyManager(CMplMcmsProtocol *mplPrtcl);
// 1080_60
	ECntrlType getVideoEncoderCntlType(CMplMcmsProtocol *mplMcmsPrtcl);
	BOOL HandleVideoEncoderAckMessagesForAsyncEncoder(CMplMcmsProtocol *mplMcmsPrtcl, ECntrlType videoEncoderCntlType);


	BOOL IsMasterAC(CMplMcmsProtocol *mplPrtcl);
	const ConnToCardTableEntry* GetPhysicalInfoHeaderByRsrcTypeAndBoardId(eResourceTypes rsrcType,WORD boardId);
	void ChangeSlaveEncoderACKToMasters(CMplMcmsProtocol *mplPrtcl,ECntrlType masterEncCntlType);
	void ChangeSlaveEncoderOpenIndToMastersAck(CMplMcmsProtocol* mplPrtcl, ECntrlType masterEncCntlType = E_VIDEO_MASTER_LB_ONLY);
	void ChangeVideoMasterEncoderOpenIndToOpenEncoderPortAck(CMplMcmsProtocol* mplPrtcl);
	int AddConfIdPartyIdToSmartRecovery(CMplMcmsProtocol *mplPrtcl);
	ConnToCardTableEntry* GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(DWORD conf_id,DWORD party_id, ECntrlType rsrc_cntl_type);
	ConnToCardTableEntry* GetConfPartyIdFromBoardIdPortIdDspNum(APIU8 board_id,APIU32 port_id, APIU32 dsp_num);
	const COsQueue * GetDestinationProcessQueueByOpcode (DWORD opcode);
	BYTE GetGenericEncoderDecoderRsrcType(BYTE logicRsrcType)const;

protected:
	// Operations


	// Attributes

private:
	STATUS TransferMoveRsrcAckIndForResourceType(CMplMcmsProtocol *mplPrtcl, eResourceTypes type, DWORD &len);
	STATUS TransferMoveRsrcAckInd(CMplMcmsProtocol *mplPrtcl, eLogicalResourceTypes logicType, DWORD &len);
//	STATUS FixAckOpcodeSendMessage(CMplMcmsProtocol *mplPrtcl, OPCODE opcode);
	STATUS SendMessage(CMplMcmsProtocol *mplPrtcl, OPCODE opcode, DWORD &len);

	CMplMcmsProtocolTracer *m_MplMcmsProtocolTracer;
};


