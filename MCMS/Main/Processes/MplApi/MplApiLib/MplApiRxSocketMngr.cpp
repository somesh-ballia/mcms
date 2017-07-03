//+========================================================================+
//                         MplApiRxSocketMngr.cpp                          |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MplApiRxSocketMngr.cpp                                      |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//+========================================================================+
#include "NStream.h"
#include "MplApiRxSocketMngr.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "TaskApi.h"
#include "OpcodesRanges.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesMcmsInternal.h"
#include "IpMfaOpcodes.h"
#include "PairOfSockets.h"
#include "OsQueue.h"
#include "MplMcmsProtocol.h"
#include "MplMcmsProtocolTracer.h"
#include "MplApiProcess.h"
#include "SystemFunctions.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsAudioCntl.h"
#include "OpcodesMcmsMux.h"
#include "muxint.h"

static CMplApiProcess *pMplApiProcess = NULL;
extern std::vector< DWORD > IgnoredOpcodesList;

/////////////////////////////////////////////////////////////////////////////
CMplApiRxSocketMngr::CMplApiRxSocketMngr() // constructor
{
	pMplApiProcess = dynamic_cast<CMplApiProcess*>(CMplApiProcess::GetProcess());
	m_MplMcmsProtocolTracer = new CMplMcmsProtocolTracer;
}

/////////////////////////////////////////////////////////////////////////////
CMplApiRxSocketMngr::~CMplApiRxSocketMngr() // constructor
{
	POBJDELETE(m_MplMcmsProtocolTracer);
}

/////////////////////////////////////////////////////////////////////////////
const char * CMplApiRxSocketMngr::GetTaskName() const
{
	return "CMplApiRxSocketMngr";
}

/////////////////////////////////////////////////////////////////////////////
DWORD CMplApiRxSocketMngr::SendMplEventToTheAppropriateProcess(CMplMcmsProtocol *mplPrtcl)
{
	STATUS status = STATUS_OK;
	DWORD len = 0xFFFFFFFF;
	OPCODE opcode = mplPrtcl->getOpcode();

	bool isIgnoredOpcode = false;
	// IgnoredOpcodesList is always empty if CFG_KEY_DEBUG_MODE is OFF !!!
	for (std::vector< DWORD >::iterator itr =  IgnoredOpcodesList.begin(); itr != IgnoredOpcodesList.end(); itr++) {
		if (  mplPrtcl->getOpcode() == (*itr) ) {
			isIgnoredOpcode = true;
			break;
		}
	}

	if (true == isIgnoredOpcode)
		return status;


	// ack indication
	if(ACK_IND == opcode)
	{
	  
	  
		ACK_IND_S ackStruct;
		memcpy(&ackStruct, mplPrtcl->GetData(), sizeof(ACK_IND_S));
		opcode = ackStruct.ack_base.ack_opcode;

		// PTRACE2(eLevelInfoNormal,"CMplApiRxSocketMngr::SendMplEventToTheAppropriateProcess - received ACK_IND for ",(pMplApiProcess->GetOpcodeAsString(opcode)).c_str() ); 

                // 1080_60
		BYTE logicRsrcType = mplPrtcl->getPortDescriptionHeaderLogical_resource_type_1();
		ECntrlType encoderType = getVideoEncoderCntlType(mplPrtcl);
		eResourceTypes type = (eResourceTypes)mplPrtcl->getPhysicalInfoHeaderResource_type();

		if(pMplApiProcess->GetSystemCardsBasedMode() == eSystemCardsMode_breeze &&
		   logicRsrcType ==  eLogical_video_encoder &&
		   (encoderType == E_VIDEO_MASTER_SPLIT_ENCODER || encoderType == E_VIDEO_SLAVE_SPLIT_ENCODER))
		{
			switch(opcode)
			{
			case TB_MSG_CONNECT_REQ :
			  {
			    BOOL ret = HandleVideoEncoderConnectAckMessages(mplPrtcl);
			    if(ret)
			      {
				len = 999999999;
				return len;
			      }
			    break;
			  }
			default:
			  {
			    BOOL ret = HandleVideoEncoderAckMessagesForAsyncEncoder(mplPrtcl, encoderType);
			    // PTRACE2INT(eLevelInfoNormal,"1080-60 CMplApiRxSocketMngr::SendMplEventToTheAppropriateProcess  call HandleVideoEncoderAckMessagesForAsyncEnec return ret = ", ret);
			    if(ret)
			      {
				len = 999999999;
				return len;
			      }
			    break;
			  }
			}
		}
		else
		{

			switch(opcode)
			{
				case MOVE_RSRC_REQ :
				{
					switch(type)
					{
						case(ePhysical_art):
						case(ePhysical_video_encoder):
						case(ePhysical_video_decoder):
						case(ePhysical_mrmp):
						{
							status = TransferMoveRsrcAckIndForResourceType(mplPrtcl, type, len);
							break;
						}
						case(ePhysical_rtm):
							break;
						default:
						{
							PASSERTMSG(1, "Illegal resource type");
							break;
						}
					}
					return len;

				} break;
			case TB_MSG_OPEN_PORT_REQ :
			{
				if(IsVideoEncoderMessage(mplPrtcl))
				{
					BOOL ret = HandleVideoEncoderOpenPortAckMessages(mplPrtcl);
					if(ret)
					{
						len = 999999999;
						return len;
					}
				}
			break;}
			case TB_MSG_CLOSE_PORT_REQ :
			{
				if(IsVideoEncoderMessage(mplPrtcl))
				{
					BOOL ret = HandleVideoEncoderClosePortAckMessages(mplPrtcl);
					if(ret)
					{
						len = 999999999;
						return len;
					}
				}
			break;}
			case TB_MSG_CONNECT_REQ :
			{
				if(IsVideoEncoderMessage(mplPrtcl))
				{
					BOOL ret = HandleVideoEncoderConnectAckMessages(mplPrtcl);
					if(ret)
					{
						len = 999999999;
						return len;
					}
				}
				break;
			}
			case AC_OPEN_CONF_REQ:
			case AC_CLOSE_CONF_REQ:
			case AC_UPDATE_CONF_PARAMS_REQ :
			case AC_LAYOUT_CHANGE_COMPLETE_REQ:
				{
				  if(!IsMasterAC(mplPrtcl))
				  {
					 PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::SendMplEventToTheAppropriateProcess Ignore the Acks to the AC_OPEN_CONF_REQ/AC_CLOSE_CONF_REQ/AC_UPDATE_CONF_PARAMS_REQ message from shadow AC");
					 len = 999999999;
					 return len;
				  }
				  break;
				}
			case TERMINAL_COMMAND :
				{
					PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::SendMplEventToTheAppropriateProcess Ignore the Ack TERMINAL_COMMAND");
					len = 999999999;
					return len;
				}
			case ALLOC_STATUS_PER_UNIT_REQ :
			{
				SendAllocStatusAckToConfPartyManager(mplPrtcl);
				len = 999999999;
				return len;
			}
			case KILL_PORT_REQ:
			{
				SendKillPortAckToConfPartyIfNeeded(mplPrtcl);
				break;
			}
			case VIDEO_ENCODER_CHANGE_LAYOUT_REQ:	// Change Layout Improvement - MplApi doesn't forward ACK_IND (CL-ACK)
			{
				if (logicRsrcType !=  eLogical_relay_avc_to_svc_video_encoder_1 &&
					logicRsrcType !=  eLogical_relay_avc_to_svc_video_encoder_2	)	// except for acknowledge for AVC to SVC encoder
				{
					PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::SendMplEventToTheAppropriateProcess Ignore VIDEO_ENCODER_CHANGE_LAYOUT_REQ ACKs");
					len = 999999999;
					return len;
				}
			}

			} // end switch
		}
	} // end if opcode==ACK_IND
	else if (VIDEO_SLAVE_ENCODER_OPEN_IND == opcode)
	{
		ChangeSlaveEncoderOpenIndToMastersAck(mplPrtcl);
	}

	if (VIDEO_MASTER_ENCODER_OPEN_IND == opcode)
	{
		ChangeVideoMasterEncoderOpenIndToOpenEncoderPortAck(mplPrtcl);
	}
	if(AC_ACTIVE_SPEAKER_IND == opcode || AC_AUDIO_SPEAKER_IND == opcode)
	{
		if(!IsMasterAC(mplPrtcl))
		{
			PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::SendMplEventToTheAppropriateProcess Ignore AC_ACTIVE_SPEAKER_IND/AC_AUDIO_SPEAKER_IND from shadow AC");
			len = 999999999;
			return len;
		}
	}
	if (SMART_RECOVERY_UPDATE == opcode)
	{
		PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::SendMplEventToTheAppropriateProcess SMART_RECOVERY_UPDATE");
		int res = AddConfIdPartyIdToSmartRecovery(mplPrtcl);
		if (res)
			return res;
	}
	if (PARTY_CM_DEBUG_INFO_IND == opcode)
	{
		PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::SendMplEventToTheAppropriateProcess Ignore PARTY_CM_DEBUG_INFO_IND");
		len = 999999999;
		return len;
	}

	// PTRACE2INT(eLevelInfoNormal,"1080-60 CMplApiRxSocketMngr::SendMplEventToTheAppropriateProcess  call HandleVideoEncoderAckMessagesForAsyncEnec for debug - SendMessage, opcode = ",(DWORD)opcode);
	status = SendMessage(mplPrtcl, opcode, len);
	if(STATUS_OK == status)
	{
		pMplApiProcess->IncrementCntCsMfaToMcms();
	}

	return len;
}

////////////////////////////////////////////////////////////////////////////////////
STATUS CMplApiRxSocketMngr::TransferMoveRsrcAckInd(CMplMcmsProtocol *mplPrtcl, eLogicalResourceTypes logicType, DWORD &len)
{
	mplPrtcl->SetPortDescriptorHeaderLogicRsrcType1(logicType);
	ACK_IND_S ackStruct;
	const DWORD structSize = sizeof(ACK_IND_S);
	memcpy(&ackStruct, mplPrtcl->GetData(), structSize);
	ackStruct.ack_base.ack_opcode = MOVE_PARTY_RESOURCE_REQ;
	mplPrtcl->AddData(structSize, (const char*)(&ackStruct));

	STATUS status = SendMessage(mplPrtcl, MOVE_PARTY_RESOURCE_REQ, len);
	return status;
}
////////////////////////////////////////////////////////////////////////////////////
STATUS CMplApiRxSocketMngr::TransferMoveRsrcAckIndForResourceType(CMplMcmsProtocol *mplPrtcl, eResourceTypes type, DWORD &len)
{
	//STATUS status = STATUS_OK;
	WORD MAX_ENTRIES_FOR_PHYSICAL_RESOURCE = 3; // 3 entries in shared memory for ePhysical_art
	CMatchResourceByResourceTypeAndPartyId matchResources(type, mplPrtcl->getPortDescriptionHeaderParty_id(), &mplPrtcl->GetPhysicalHeaderConst());
	ConnToCardTableEntry MatchingConnToCardTableEntries[MAX_ENTRIES_FOR_PHYSICAL_RESOURCE];
	CSharedMemMap *pSharedMemoryMap = pMplApiProcess->GetSharedMemoryMap();
	WORD numEntriesForPhysicalResource = 0;
	pSharedMemoryMap->GetEntriesSet(matchResources, MatchingConnToCardTableEntries, MAX_ENTRIES_FOR_PHYSICAL_RESOURCE, numEntriesForPhysicalResource);
	for (int i = 0; i < numEntriesForPhysicalResource; i++)
	{
		eLogicalResourceTypes lrt = MatchingConnToCardTableEntries[i].rsrcType;
		TransferMoveRsrcAckInd(mplPrtcl,lrt,len);
	}

	return STATUS_OK;
}
/*
////////////////////////////////////////////////////////////////////////////////////
STATUS CMplApiRxSocketMngr::FixAckOpcodeSendMessage(CMplMcmsProtocol *mplPrtcl, OPCODE opcode)
{
	const DWORD structSize = sizeof(ACK_IND_S);
	ACK_IND_S ackStruct;
	memcpy(&ackStruct, mplPrtcl->GetData(), structSize);
	ackStruct.ack_base.ack_opcode = opcode;
	mplPrtcl->AddData(structSize, (const char*)&ackStruct);

	STATUS status = SendMessage(mplPrtcl, opcode);

	return status;
}
*/

////////////////////////////////////////////////////////////////////////////////////
STATUS CMplApiRxSocketMngr::SendMessage(CMplMcmsProtocol *mplPrtcl, OPCODE opcode, DWORD &len)
{
  STATUS status = STATUS_OK;
  const COsQueue * DestinationProcessQueue = GetDestinationProcessQueueByOpcode(opcode);
  if (NULL == DestinationProcessQueue)
  {
    len = 0xFFFFFFFF;
    if(ESTABLISH_CONNECTION == opcode)
    {
      // ignore this opcode temporary !!!
      // SAGI and YURI
      return STATUS_FAIL;
    }
    else
    {
      PASSERTSTREAM(true,
        "Destination process was not found by opcode: "
        << pMplApiProcess->GetOpcodeAsString(opcode));

      return STATUS_FAIL;
    }
  }

  mplPrtcl->UpdateSendTimes();

  m_MplMcmsProtocolTracer->SetData(mplPrtcl);
  m_MplMcmsProtocolTracer->TraceMplMcmsProtocol("MPL -> MCMS");
  m_MplMcmsProtocolTracer->SetData(NULL);

  CSegment * pSeg = new CSegment;
  mplPrtcl->Serialize(*pSeg);
  len = pSeg->GetLen();

  CTaskApi api;
  api.CreateOnlyApi(*DestinationProcessQueue);
  status = api.SendMsg(pSeg,MPLAPI_MSG);
  api.DestroyOnlyApi();

  return status;
}

////////////////////////////////////////////////////////////////////////////////////
void CMplApiRxSocketMngr::SendSocketsConIdPerMFA_CardNum(WORD card_number,WORD conId,COsQueue *txMailslot)
{
	CPairOfSockets pairOfSockets;
	pairOfSockets.m_conId=conId;
    pairOfSockets.m_TransmitSocketMailSlot=txMailslot;
  //pairOfSockets.m_ReciveSocketMailSlot()

    CSegment * pSeg = new CSegment;
	*pSeg <<(WORD) card_number;
	pairOfSockets.Serialize(*pSeg);

	const COsQueue * MplApiDispatcherQueue = pMplApiProcess->GetOtherProcessQueue(eProcessMplApi,eDispatcher);

	CTaskApi api;
	api.CreateOnlyApi(*MplApiDispatcherQueue);
	STATUS a = api.SendMsg(pSeg,MPLAPI_MSG);
	api.DestroyOnlyApi();


}
/////////////////////////////////////////////////////////////////////////////
//void CMplApiRxSocketMngr::SendMplEventLogger (DWORD BufLen,char* Mainbuf)
//{
//	CSegment *seg = new CSegment;
//	seg->Create(Mainbuf,(WORD)BufLen);
//
//	const COsQueue* LoggerManager =
//		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessLogger,eManager);
//
//	STATUS res = LoggerManager->Send(seg,MPLAPI_MSG);//TO DEFINE :RCV_MPL_MSG_TO_LOGGER
//}


////////////////////////////////////////////////////////////////////////////////////
const COsQueue * CMplApiRxSocketMngr::GetDestinationProcessQueueByOpcode (DWORD opcode)
{
	static struct{
		DWORD start;
		DWORD end;
		eProcessType process;
		eOtherProcessQueueEntry task;
	} RangeTable[] = {
		{	MPLAPI_FIRST_OPCODE_IN_RANGE,			MPLAPI_LAST_OPCODE_IN_RANGE,			eProcessMplApi,			eManager	},
		{	CONF_PARTY_FIRST_OPCODE_IN_RANGE,		CONF_PARTY_LAST_OPCODE_IN_RANGE,		eProcessConfParty,		eDispatcher	},
		{	PARTY_IP_MEDIA_FIRST_OPCODE_IN_RANGE,	PARTY_IP_MEDIA_LAST_OPCODE_IN_RANGE,	eProcessConfParty,		eDispatcher	},
		{	MCUMNGR_FIRST_OPCODE_IN_RANGE,			MCUMNGR_LAST_OPCODE_IN_RANGE,			eProcessMcuMngr,		eManager	},
		{	CARDS_DISPATCHER_FIRST_OPCODE_IN_RANGE,	CARDS_DISPATCHER_LAST_OPCODE_IN_RANGE,	eProcessCards,			eDispatcher	},
		{	CARDS_MNGR_FIRST_OPCODE_IN_RANGE,		CARDS_MNGR_LAST_OPCODE_IN_RANGE,		eProcessCards,			eManager	},
		{	RESOURCE_FIRST_OPCODE_IN_RANGE,			RESOURCE_LAST_OPCODE_IN_RANGE,			eProcessResource,		eManager	},
		{	AUTHENTICATION_FIRST_OPCODE_IN_RANGE,	AUTHENTICATION_LAST_OPCODE_IN_RANGE,	eProcessAuthentication,	eManager	},
		{ 0,0,eProcessTypeInvalid,eDispatcher } }; // leave this line for termination detection

    eProcessType process = eProcessTypeInvalid;
    eOtherProcessQueueEntry task = eDispatcher;

	int i = 0;
	while (RangeTable[i].process != eProcessTypeInvalid)
	{
		if (RangeTable[i].start < opcode && opcode < RangeTable[i].end)
		{
			process = RangeTable[i].process;
			task = RangeTable[i].task;
		}
		i++;
	}
	const COsQueue * res = NULL;
	if (process != eProcessTypeInvalid)
	{
		res = pMplApiProcess->GetOtherProcessQueue(process,task);
	}
	return res;
}
////////////////////////////////////////////////////////////////////////////////////
BOOL CMplApiRxSocketMngr::IsVideoEncoderMessage(CMplMcmsProtocol *mplPrtcl)
{
	BOOL isVideoEncoder = NO;
	ConnToCardTableEntry 	connToCardTableEntry;
	APIU32               	connectionId = 0xffffffff;
	connectionId = mplPrtcl->getPortDescriptionHeaderConnection_id();
	CSharedMemMap *pSharedMemMap = pMplApiProcess->GetSharedMemoryMap();
	STATUS con2CardStatus = pSharedMemMap->Get(connectionId,connToCardTableEntry);
	if (STATUS_OK == con2CardStatus)
	{
		if (GetGenericEncoderDecoderRsrcType(connToCardTableEntry.rsrcType) == eLogical_video_encoder)
		{
			isVideoEncoder = YES;
		}
	}
	else
	{
		PTRACE(eLevelError,"CMplApiRxSocketMngr::IsVideoEncoderMessage didn't find connect to card table entry");
	}
	return isVideoEncoder;
}
////////////////////////////////////////////////////////////////////////////////////
BOOL CMplApiRxSocketMngr::HandleVideoEncoderOpenPortAckMessages(CMplMcmsProtocol *mplMcmsPrtcl)
{
	BOOL ret = FALSE;
	OPCODE pOpcode = mplMcmsPrtcl->getCommonHeaderOpcode();
	ECntrlType videoEncoderCntlType = E_NORMAL;
	BYTE logicRsrcType = mplMcmsPrtcl->getPortDescriptionHeaderLogical_resource_type_1();
	if(GetGenericEncoderDecoderRsrcType(logicRsrcType)==eLogical_video_encoder)
	{
		ConnToCardTableEntry 	connToCardTableEntry;
		APIU32               	connectionId = 0xffffffff;
		connectionId = mplMcmsPrtcl->getPortDescriptionHeaderConnection_id();
		CSharedMemMap *pSharedMemMap = pMplApiProcess->GetSharedMemoryMap();
		STATUS con2CardStatus = pSharedMemMap->Get(connectionId,connToCardTableEntry);
		videoEncoderCntlType = connToCardTableEntry.rsrcCntlType;
		switch(videoEncoderCntlType)
		{
			///////////////////////////////////////////////////////////////////////////////////////////
			/// MASTER-SLAVE Topology in MPM cards (SD/HD720asymmetric calls)
			//  We ignore the ACK on TB_MSG_OPEN_PORT_REQ from both the master and slave
			//  We will receive VIDEO_SLAVE_ENCODER_OPEN_IND after all master-slave open encoder port flow will finish
			///////////////////////////////////////////////////////////////////////////////////////////

			case E_VIDEO_MASTER_LB_ONLY:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderOpenPortAckMessages Ignore the Ack to the TB_MSG_OPEN_PORT_REQ  message from master video encoder in master-slave topology");
				ret = TRUE;

			}
			break;
			case E_VIDEO_SLAVE_FULL_ENCODER:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderOpenPortAckMessages Ignore the Ack to the TB_MSG_OPEN_PORT_REQ  message from slave video encoder in master slave topology in MPM card");
				ret = TRUE;


			}
			break;

			///////////////////////////////////////////////////////////////////////////////////////////
			/// MASTER-SLAVE Topology in MPM+ cards (HD1080)
			//  We ignore the ACK on TB_MSG_OPEN_PORT_REQ from both the master and slave
			//  We will receive VIDEO_MASTER_ENCODER_OPEN_IND after the master slave open encoder port flow will finish
			///////////////////////////////////////////////////////////////////////////////////////////
			case E_VIDEO_MASTER_SPLIT_ENCODER:
			case E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderOpenPortAckMessages Ignore the Ack to the TB_MSG_OPEN_PORT_REQ  message from master video encoder in master slave split encoder topology");
				ret = TRUE;
			}
			break;
			case E_VIDEO_SLAVE_SPLIT_ENCODER:
			case E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderOpenPortAckMessages Ignore the Ack to the TB_MSG_OPEN_PORT_REQ  message from slave1 video encoder in master slave split encoder topology");
				ret = TRUE;
			}
			break;

			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;

		}
	}
	else
	{
		PTRACE(eLevelError,"CMplApiRxSocketMngr::HandleVideoEncoderOpenPortAckMessages not video encoder");

	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////
BOOL CMplApiRxSocketMngr::HandleVideoEncoderClosePortAckMessages(CMplMcmsProtocol *mplMcmsPrtcl)
{
	BOOL ret = FALSE;
	OPCODE pOpcode = mplMcmsPrtcl->getCommonHeaderOpcode();
	ECntrlType videoEncoderCntlType = E_NORMAL;
	BYTE logicRsrcType = mplMcmsPrtcl->getPortDescriptionHeaderLogical_resource_type_1();
	if(GetGenericEncoderDecoderRsrcType(logicRsrcType)==eLogical_video_encoder)
	{
		ConnToCardTableEntry 	connToCardTableEntry;
		APIU32               	connectionId = 0xffffffff;
		connectionId = mplMcmsPrtcl->getPortDescriptionHeaderConnection_id();
		CSharedMemMap *pSharedMemMap = pMplApiProcess->GetSharedMemoryMap();
		STATUS con2CardStatus = pSharedMemMap->Get(connectionId,connToCardTableEntry);
		videoEncoderCntlType = connToCardTableEntry.rsrcCntlType;
		switch(videoEncoderCntlType)
		{
			///////////////////////////////////////////////////////////////////////////////////////////
			/// MASTER-SLAVE Topology in MPM cards (SD/HD720asymmetric calls)
			//  The slave video encoder will send ACK just after the master is closed,
			//  We will update its ACK with the masters conn. id, and ignore the ACK from the master
			///////////////////////////////////////////////////////////////////////////////////////////

			case E_VIDEO_MASTER_LB_ONLY:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderClosePortAckMessages Ignore the Ack to the TB_MSG_CLOSE_PORT_REQ message from master video encoder in master-slave topolgy");
				ret =TRUE;
			}
			break;
			case E_VIDEO_SLAVE_FULL_ENCODER:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderClosePortAckMessages the TB_MSG_CLOSE_PORT_REQ message from slave video encoder in master-slave topology in MPM cards(SD30/HD720asymmetric): update message to the master connection ID");
				ChangeSlaveEncoderACKToMasters(mplMcmsPrtcl, E_VIDEO_MASTER_LB_ONLY);
			}
			break;
			///////////////////////////////////////////////////////////////////////////////////////////
			/// MASTER-SLAVE Topology in MPM+ cards (HD1080)
			//  The slave video encoder will send ACK just after all are closed, we ignore the ACKs from the master
			///////////////////////////////////////////////////////////////////////////////////////////
			case E_VIDEO_MASTER_SPLIT_ENCODER:
			case E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderClosePortAckMessages Ignore the Ack to the TB_MSG_CLOSE_PORT_REQ message from master video encoder in master slave topology in MPM+(split encoder)");
				ret =TRUE;
			}
			break;
			case E_VIDEO_SLAVE_SPLIT_ENCODER:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderClosePortAckMessages the TB_MSG_CLOSE_PORT_REQ message from slave video encoder in master-slave topology in MPM+ cards: update message to the master connection ID");
				ChangeSlaveEncoderACKToMasters(mplMcmsPrtcl, E_VIDEO_MASTER_SPLIT_ENCODER);
			}
			break;
			case E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP:
			{
					PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderClosePortAckMessages the TB_MSG_CLOSE_PORT_REQ message from slave video encoder in master-slave topology in MPM+ cards: update message to the master connection ID in case of E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP ");
					ChangeSlaveEncoderACKToMasters(mplMcmsPrtcl, E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP);
			}
			break;
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}
	}
	else
	{
		PTRACE(eLevelError,"CMplApiRxSocketMngr::HandleVideoEncoderClosePortAckMessages not video encoder");
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////
BOOL CMplApiRxSocketMngr::HandleVideoEncoderConnectAckMessages(CMplMcmsProtocol *mplMcmsPrtcl)
{
	BOOL ret = FALSE;
	OPCODE pOpcode = mplMcmsPrtcl->getCommonHeaderOpcode();
	ECntrlType videoEncoderCntlType = E_NORMAL;
	BYTE logicRsrcType = mplMcmsPrtcl->getPortDescriptionHeaderLogical_resource_type_1();
	if(GetGenericEncoderDecoderRsrcType(logicRsrcType)==eLogical_video_encoder)
	{
		ConnToCardTableEntry 	connToCardTableEntry;
		APIU32               	connectionId = 0xffffffff;
		connectionId = mplMcmsPrtcl->getPortDescriptionHeaderConnection_id();
		CSharedMemMap *pSharedMemMap = pMplApiProcess->GetSharedMemoryMap();
		STATUS con2CardStatus = pSharedMemMap->Get(connectionId,connToCardTableEntry);
		videoEncoderCntlType = connToCardTableEntry.rsrcCntlType;
		switch(videoEncoderCntlType)
		{
			///////////////////////////////////////////////////////////////////////////////////////////
			/// MASTER-SLAVE Topology in MPM cards (SD/HD720asymmetric calls)
			//  1. The connect message with the master connection id,
			//     is for connecting the master and slave in the open port process - we ignore this message the process ends when we will receive VIDEO_SLAVE_ENCODER_OPEN_IND
			//  2. The connect message with the slave connection id, is the ack on connecting the encoder to ART, we will update the ACK with the master id (the one that is known in the ConfParty level)
			///////////////////////////////////////////////////////////////////////////////////////////
			case E_VIDEO_MASTER_LB_ONLY:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderConnectAckMessages Ignore the Ack to the TB_MSG_CONNECT_REQ message from master video encoder in MASTER-SLAVE topology- connect the slave and the master");
				ret = TRUE;
			}
			break;
			case E_VIDEO_SLAVE_FULL_ENCODER:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderConnectAckMessages the TB_MSG_CONNECT_REQ message from slave video encoder in MASTER-SLAVE topology in MPM cards(SD30/HD720Asymmetric)- update message to the master connection ID");
				ChangeSlaveEncoderACKToMasters(mplMcmsPrtcl, E_VIDEO_MASTER_LB_ONLY);
			}
			break;
			///////////////////////////////////////////////////////////////////////////////////////////
			/// MASTER-SLAVE Topology in MPM+ cards (HD1080)
			//  1. The connect message with the master connection id, is the ACK on connecting the encoder to ART we continue usaly
			//  2. The connect message with the slave connection id,  is for connecting the master to it's slave in the open port process - we ignore this messages, at the end of the process we will receive VIDEO_MASTER_ENCODER_OPEN_IND
			///////////////////////////////////////////////////////////////////////////////////////////

			case E_VIDEO_SLAVE_SPLIT_ENCODER:
			case E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP:
			{
				PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::HandleVideoEncoderConnectAckMessages Ignore the Ack to the TB_MSG_CONNECT_REQ message from slave video encoder in MASTER SLAVE split encoder topology");
				ret = TRUE;
			}
			break;

			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}

	}
	else
	{
		PTRACE(eLevelError,"CMplApiRxSocketMngr::HandleVideoEncoderConnectAckMessages not video encoder");
	}
	return ret;

}
////////////////////////////////////////////////////////////////////////////////////
void CMplApiRxSocketMngr::SendKillPortAckToConfPartyIfNeeded(CMplMcmsProtocol *mplPrtcl)
{
	if (mplPrtcl->getPhysicalInfoHeaderResource_type() == ePhysical_video_encoder || mplPrtcl->getPhysicalInfoHeaderResource_type() == ePhysical_video_decoder)
	{
		CSegment * pSeg = new CSegment;
		*pSeg << (DWORD) mplPrtcl->getPhysicalInfoHeaderBoard_id()
			  << (DWORD) mplPrtcl->getPhysicalInfoHeaderUnit_id();

		const COsQueue * ConfPartyManagerQueue = pMplApiProcess->GetOtherProcessQueue(eProcessConfParty,eManager);

		CTaskApi api;
		api.CreateOnlyApi(*ConfPartyManagerQueue);
		STATUS a = api.SendMsg(pSeg,KILL_PORT_REQ);
		api.DestroyOnlyApi();
	}
}
////////////////////////////////////////////////////////////////////////////////////
void CMplApiRxSocketMngr::SendAllocStatusAckToConfPartyManager(CMplMcmsProtocol *mplPrtcl)
{
	CSegment * pSeg = new CSegment;
	*pSeg << (DWORD) mplPrtcl->getPhysicalInfoHeaderBoard_id()
		  << (DWORD) mplPrtcl->getPhysicalInfoHeaderUnit_id();

	const COsQueue * ConfPartyManagerQueue = pMplApiProcess->GetOtherProcessQueue(eProcessConfParty,eManager);

	m_MplMcmsProtocolTracer->SetData(mplPrtcl);
	m_MplMcmsProtocolTracer->TraceMplMcmsProtocol("MPL -> MCMS");
	m_MplMcmsProtocolTracer->SetData(NULL);

	CTaskApi api;
	api.CreateOnlyApi(*ConfPartyManagerQueue);
	STATUS a = api.SendMsg(pSeg,ALLOC_STATUS_PER_UNIT_REQ);
	api.DestroyOnlyApi();
}
////////////////////////////////////////////////////////////////////////////////////
BOOL CMplApiRxSocketMngr::IsMasterAC(CMplMcmsProtocol *mplPrtcl)
{
	BOOL isMasterAC = NO;
	if (mplPrtcl->getPhysicalInfoHeaderResource_type() == ePhysical_mrmp)
		return YES;
	WORD boardId = (WORD)(mplPrtcl->getPhysicalInfoHeaderBoard_id());
	const ConnToCardTableEntry* connToCardTableEntry = GetPhysicalInfoHeaderByRsrcTypeAndBoardId(ePhysical_audio_controller,boardId);
	if(connToCardTableEntry != NULL)
	{
	   if(connToCardTableEntry->rsrcCntlType==E_AC_MASTER)
		   isMasterAC = YES;
	}
	else
	{
		PASSERT(1);
	}

	return isMasterAC;
}
//////////////////////////////////////////////////////////////////////////////////
const ConnToCardTableEntry* CMplApiRxSocketMngr::GetPhysicalInfoHeaderByRsrcTypeAndBoardId(eResourceTypes rsrcType,WORD boardId)
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = pMplApiProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries && passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
            const ConnToCardTableEntry & currentEntry = pSharedMemoryMap->m_pEntries[i];

			if (currentEntry.physicalRsrcType == rsrcType && currentEntry.boardId == boardId)
            {
	      pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiRxSocketMngr::GetPhysicalInfoHeaderByRsrcTypeAndBoardId");
                return &(pSharedMemoryMap->m_pEntries[i]);
            }
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiRxSocketMngr::GetPhysicalInfoHeaderByRsrcTypeAndBoardId"," - enty not found");
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////
void CMplApiRxSocketMngr::ChangeSlaveEncoderACKToMasters(CMplMcmsProtocol *mplPrtcl, ECntrlType masterEncCntlType)
{
	PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::ChangeSlaveEncoderACKToMasters");
	APIU32 connectionId = 0xffffffff;
	connectionId = mplPrtcl->getPortDescriptionHeaderConnection_id();
	ConnToCardTableEntry 	slaveConnToCardTableEntry;
	CSharedMemMap *pSharedMemMap = pMplApiProcess->GetSharedMemoryMap();
	STATUS status = pSharedMemMap->Get(connectionId, slaveConnToCardTableEntry);
	ConnToCardTableEntry* pMasterConnToCardTableEntry = NULL;
	pMasterConnToCardTableEntry = GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(slaveConnToCardTableEntry.rsrc_conf_id,slaveConnToCardTableEntry.rsrc_party_id,masterEncCntlType);
	if(	pMasterConnToCardTableEntry)
	{
		mplPrtcl->UpdatePortDescriptionHeader(pMasterConnToCardTableEntry->rsrc_party_id,
											pMasterConnToCardTableEntry->rsrc_conf_id,
											pMasterConnToCardTableEntry->m_id,
											pMasterConnToCardTableEntry->rsrcType);
	}
	else
	{
		PTRACE(eLevelError,"CUpdateSlaveEncoderConnectMessageACK didn't find master connect to card table entry");
	}


}
/////////////////////////////////////////////////////////////////////////////////////////
int CMplApiRxSocketMngr::AddConfIdPartyIdToSmartRecovery(CMplMcmsProtocol *mplPrtcl)
{
	int ans = 0;
	PTRACE(eLevelInfoNormal,"CMplApiRxSocketMngr::AddConfIdPartyIdToSmartRecovery");

	SMART_RECOVERY_UPDATE_S* smartRecStruct = (SMART_RECOVERY_UPDATE_S*)mplPrtcl->GetData();
	APIU32 dsp_num = smartRecStruct->unDspNum;
	APIU32 port_id = smartRecStruct->unPortNum;

	APIU8 board_id = mplPrtcl->getPhysicalInfoHeaderBoard_id();
//	APIU32 connectionId = 0xffffffff;
//	connectionId = mplPrtcl->getPortDescriptionHeaderConnection_id();
//	ConnToCardTableEntry 	slaveConnToCardTableEntry;
//	CSharedMemMap *pSharedMemMap = pMplApiProcess->GetSharedMemoryMap();
//	STATUS status = pSharedMemMap->Get(connectionId, slaveConnToCardTableEntry);
	ConnToCardTableEntry* pConnToCardTableEntry = NULL;
	pConnToCardTableEntry = GetConfPartyIdFromBoardIdPortIdDspNum(board_id, port_id, dsp_num);
	if(	pConnToCardTableEntry)
	{
		mplPrtcl->AddPortDescriptionHeader(pConnToCardTableEntry->rsrc_party_id,
											  pConnToCardTableEntry->rsrc_conf_id,
											  pConnToCardTableEntry->m_id,
											  pConnToCardTableEntry->rsrcType);
	}
	else
	{
		ans = 999999999;
		PTRACE(eLevelError,"CMplApiRxSocketMngr::AddConfIdPartyIdToSmartRecovery didn't find target party connect to card table entry");
	}

	return ans;
}
//////////////////////////////////////////////////////////////////////////////////
void CMplApiRxSocketMngr::ChangeSlaveEncoderOpenIndToMastersAck(CMplMcmsProtocol* mplPrtcl, ECntrlType masterEncCntlType)
{
	PTRACE(eLevelError,"CMplApiRxSocketMngr::ChangeSlaveEncoderOpenIndToMastersAck");
	mplPrtcl->SetCommonHeaderOpcode(ACK_IND);
	ChangeSlaveEncoderACKToMasters(mplPrtcl,masterEncCntlType);
}
//////////////////////////////////////////////////////////////////////////////////
void CMplApiRxSocketMngr::ChangeVideoMasterEncoderOpenIndToOpenEncoderPortAck(CMplMcmsProtocol* mplPrtcl)
{
	PTRACE(eLevelError,"CMplApiRxSocketMngr::ChangeVideoMasterEncoderOpenIndToOpenEncoderPortAck");
	mplPrtcl->SetCommonHeaderOpcode(ACK_IND);
}
//////////////////////////////////////////////////////////////////////////////////
ConnToCardTableEntry* CMplApiRxSocketMngr::GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(DWORD conf_id,DWORD party_id, ECntrlType rsrc_cntl_type)
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = pMplApiProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries &&
		passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
			if ((pSharedMemoryMap->m_pEntries[i].rsrc_conf_id 	== conf_id)	&&
				(pSharedMemoryMap->m_pEntries[i].rsrc_party_id 	== party_id)&&
				(pSharedMemoryMap->m_pEntries[i].rsrcCntlType == rsrc_cntl_type))
				{
				  pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiRxSocketMngr::GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType");
					return &(pSharedMemoryMap->m_pEntries[i]);
				}
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiRxSocketMngr::GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType"," - enty not found");
	return NULL;
}
////////////////////////////////////////////////////////////////////////////////////
ConnToCardTableEntry* CMplApiRxSocketMngr::GetConfPartyIdFromBoardIdPortIdDspNum(APIU8 board_id,APIU32 port_id, APIU32 dsp_num)
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = pMplApiProcess->GetSharedMemoryMap();

	//ConnToCardTableEntry* ans = NULL;
	CMedString cstr;
	cstr << "CMplApiRxSocketMngr::GetConfPartyIdFromBoardIdPortIdDspNum searching for a table entry with:\n";
	cstr << "board id: " << board_id << "\n";
	cstr << "port id: " << port_id << "\n";
	cstr << "dsp num (unit id?) "<< dsp_num << "\n";
	PTRACE(eLevelInfoNormal,cstr.GetString());

	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries &&
			passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
		{
			if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
			{
				passedEntries++;
				if (pSharedMemoryMap->m_pEntries[i].boardId == board_id)
				{
					COstrStream ostr;
					ostr << "found the same baord id - dump the table entry:\n";
					pSharedMemoryMap->m_pEntries[i].Dump(ostr);
					PTRACE(eLevelInfoNormal, ostr.str().c_str());
					if	((pSharedMemoryMap->m_pEntries[i].portId == port_id) &&
						(pSharedMemoryMap->m_pEntries[i].unitId == dsp_num)  &&
						(pSharedMemoryMap->m_pEntries[i].rsrcType == eLogical_mux))
					{
						PTRACE(eLevelInfoNormal,"MATCH FOUND - return entry");
						return &(pSharedMemoryMap->m_pEntries[i]);
					}
				}
			}
			i++;
		}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////
BYTE CMplApiRxSocketMngr::GetGenericEncoderDecoderRsrcType(BYTE logicRsrcType)const
{
	BYTE retType = logicRsrcType;

	if((logicRsrcType==eLogical_COP_HD1080_encoder)
		|| (logicRsrcType==eLogical_COP_HD720_encoder)
		|| (logicRsrcType==eLogical_COP_CIF_encoder)
		|| (logicRsrcType==eLogical_COP_4CIF_encoder)
		|| (logicRsrcType==eLogical_COP_PCM_encoder)
		|| (logicRsrcType==eLogical_COP_VSW_encoder)
		|| (logicRsrcType==eLogical_COP_dummy_encoder)
		|| (logicRsrcType==eLogical_VSW_dummy_encoder))
		retType = eLogical_video_encoder;
	else if ((logicRsrcType==eLogical_COP_Dynamic_decoder)
    		|| (logicRsrcType==eLogical_COP_VSW_decoder)
    		|| (logicRsrcType==eLogical_VSW_dummy_decoder))
		retType = eLogical_video_decoder;

	return retType;
}
// 1080_60
ECntrlType CMplApiRxSocketMngr::getVideoEncoderCntlType(CMplMcmsProtocol *mplMcmsPrtcl)
{
	BOOL ret = FALSE;
	OPCODE pOpcode = mplMcmsPrtcl->getCommonHeaderOpcode();
	ECntrlType videoEncoderCntlType = E_NORMAL;
	BYTE logicRsrcType = mplMcmsPrtcl->getPortDescriptionHeaderLogical_resource_type_1();
	if(GetGenericEncoderDecoderRsrcType(logicRsrcType)==eLogical_video_encoder)
	{
		ConnToCardTableEntry 	connToCardTableEntry;
		APIU32               	connectionId = 0xffffffff;
		connectionId = mplMcmsPrtcl->getPortDescriptionHeaderConnection_id();
		CSharedMemMap *pSharedMemMap = pMplApiProcess->GetSharedMemoryMap();
		STATUS con2CardStatus = pSharedMemMap->Get(connectionId,connToCardTableEntry);
		videoEncoderCntlType = connToCardTableEntry.rsrcCntlType;
	}

	// PTRACE2INT(eLevelInfoNormal,"1080-60 CMplApiRxSocketMngr::getVideoEncoderCntlType  videoEncoderCntlType: ", videoEncoderCntlType);

	return videoEncoderCntlType;
}
BOOL CMplApiRxSocketMngr::HandleVideoEncoderAckMessagesForAsyncEncoder(CMplMcmsProtocol *mplMcmsPrtcl, ECntrlType videoEncoderCntlType)
{
	BOOL ret = FALSE;
	BOOL isFinishedReqHandler = FALSE;


	const ACK_IND_S *pAckStruct = (const ACK_IND_S *)mplMcmsPrtcl->getpData();

	DWORD sub_req_id = mplMcmsPrtcl->getMsgDescriptionHeaderRequest_id();
	//	pMplApiProcess->getMasterSlaveReqHandler()->setMasterSlaveAckToHandlerSubId(pAckStruct->ack_base.ack_seq_num, isFinishedReqHandler);
	pMplApiProcess->getMasterSlaveReqHandler()->setMasterSlaveAckToHandlerSubId(sub_req_id, isFinishedReqHandler);

	PTRACE2INT(eLevelInfoNormal,"1080_60: CMplApiRxSocketMngr::HandleVideoEncoderAckMessagesForAsyncDec  isFinishedReqHandler: ", isFinishedReqHandler);

	if(isFinishedReqHandler == TRUE)
	{
		if(videoEncoderCntlType == E_VIDEO_SLAVE_SPLIT_ENCODER)
		{
			ChangeSlaveEncoderACKToMasters(mplMcmsPrtcl, E_VIDEO_MASTER_SPLIT_ENCODER);

			DWORD handler_id = 0;
			bool get_handler_id = false;
			get_handler_id = (pMplApiProcess->getMasterSlaveReqHandler())->GetHandlerIdBySubReqId(handler_id,sub_req_id);
			if(get_handler_id){
			  PTRACE2INT(eLevelInfoNormal,"1080_60: CMplApiRxSocketMngr::HandleVideoEncoderAckMessagesForAsyncDec  UpdateCommonHeaderToMPL: handler_id = ", handler_id);
			  //mplMcmsPrtcl->UpdateCommonHeaderToMPL(handler_id);
			  mplMcmsPrtcl->UpdateMessageDescriptionHeaderReqId(handler_id);
			}
		}
		pMplApiProcess->getMasterSlaveReqHandler()->deleteMasterSlaveHandlerAccordingToSubId(sub_req_id);
	}
	else
	{
		ret = TRUE;
	}
	return ret;
}
