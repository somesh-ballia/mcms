//+========================================================================+
//                       MplApiDispatcherTask.cpp                                |
//                     Copyright 2005 Polycom                              |
//                      All Rights Reserved.                               |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MplApiDispatcherTask.cpp                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sagi                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Sagi| 14.2.05    | Interprocess tasks                                   |
//+========================================================================+


#include "MplApiDispatcherTask.h"
#include "DispatcherTask.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsAudioCntl.h"
#include "MplApiOpcodes.h"
#include "Macros.h"
#include "MplMcmsProtocol.h"
#include "TaskApi.h"
#include "MplMcmsStructs.h"
#include "MplApiProcess.h"
#include "ListenSocket.h"
#include "StatusesGeneral.h"
#include "ConfStructs.h"
#include "MplMcmsStructs.h"
#include "MplApiSpecialCommandHandler.h"
#include "StringsLen.h"
#include "ObjString.h"
#include "TraceStream.h"
#include "MplMcmsProtocolTracer.h"
#include "SystemFunctions.h"
#include "VideoStructs.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsAudioCntl.h"
#include "MplApiStatuses.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "LibsCommonHelperFuncs.h"
#include "OpcodesMrmcCardMngrMrc.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsShelfMngr.h"


extern std::vector< DWORD > IgnoredOpcodesList;

PBEGIN_MESSAGE_MAP(CMplApiDispatcherTask)
  ONEVENT(BASIC_MSG_TO_MPL_API    ,ANYCASE , CMplApiDispatcherTask::OnBasicMsgToMplApi)
  ONEVENT(CLOSE_SOCKET_CONNECTION ,ANYCASE , CMplApiDispatcherTask::OnMplApiCloseCardConnection)
  ONEVENT(CLEAN_MASTER_SLAVE_REQUESTS_TOUT ,ANYCASE ,CMplApiDispatcherTask::onCleanMasterSlaveRequestTout)
PEND_MESSAGE_MAP(CMplApiDispatcherTask,CStateMachine);

// 1080_60
eSystemCardsMode GetSystemCardsBasedMode()
{
	eSystemCardsMode systemCardsBasedMode= eSystemCardsMode_illegal;
	CMplApiProcess* pConfPartyProcess = (CMplApiProcess*)CMplApiProcess::GetProcess();
	systemCardsBasedMode = pConfPartyProcess->GetSystemCardsBasedMode();
	return systemCardsBasedMode;

}
////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////

extern "C" void MplApiDispatcherEntryPoint(void* appParam)
{
	CMplApiDispatcherTask * MplApidispatcherTask = new CMplApiDispatcherTask;
	MplApidispatcherTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CMplApiDispatcherTask::CMplApiDispatcherTask()
        :CDispatcherTask(FALSE)
{
	m_pProcess = dynamic_cast<CMplApiProcess*>(CMplApiProcess::GetProcess());
	m_MplMcmsProtocolTracer = new CMplMcmsProtocolTracer;
    m_Thread_Group = eTaskGroupRegular;
}


/////////////////////////////////////////////////////////////////////
CMplApiDispatcherTask::~CMplApiDispatcherTask()
{
	POBJDELETE(m_MplMcmsProtocolTracer);
}

///////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::InitTask()
{
}

///////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::AddFilterOpcodePoint()
{
	AddFilterOpcodeToQueue(BASIC_MSG_TO_MPL_API);
}

///////////////////////////////////////////////////////////////////////////////
void*  CMplApiDispatcherTask::GetMessageMap()
{
	return (void*)m_msgEntries;
}
///////////////////////////////////////////////////////////////////////////////
void  CMplApiDispatcherTask::OnBasicMsgToMplApi(CSegment* pMsg)
{
	CMplMcmsProtocol  		mplPrtcl;
	ConnToCardTableEntry 	connToCardTableEntry;
	APIU32               	connectionId = 0xffffffff;
	eLogicalResourceTypes   lrt;

	mplPrtcl.DeSerialize(*pMsg);

	//Change Layout Improvement - Layout Shared Memory (CL-SM)
	if(MULTIPLE_PARTIES_CHANGE_LAYOUT_REQ == mplPrtcl.getCommonHeaderOpcode())
	{
		HandleMultiplePartiesChangeLayoutRequest(mplPrtcl);
		return;
	}

	//Indication Icon Change Improvement - Indication Icon Shared Memory (CL-SM)
	if(MULTIPLE_PARTIES_INDICATION_ICON_CHANGE_REQ == mplPrtcl.getCommonHeaderOpcode())
	{
		HandleMultiplePartiesIndicationIconChangeRequest(mplPrtcl);
		return;
	}

	connectionId = mplPrtcl.getPortDescriptionHeaderConnection_id();
	lrt = (eLogicalResourceTypes)mplPrtcl.getPortDescriptionHeaderLogical_resource_type_1();

	if(0xffffffff != connectionId)
	{
		CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
		STATUS con2CardStatus = pSharedMemMap->Get(connectionId,connToCardTableEntry);
		if ((STATUS_OK == con2CardStatus)||(lrt==eLogical_COP_dummy_encoder)||(lrt==eLogical_VSW_dummy_encoder)||(lrt==eLogical_VSW_dummy_decoder))
		{
			if(VIDEO_ENCODER_CHANGE_LAYOUT_REQ == mplPrtcl.getCommonHeaderOpcode())		//Change Layout Improvement - Layout Shared Memory (CL-SM)
				mplPrtcl.SetPortDescriptorHeaderRoomId(connToCardTableEntry.room_id);

			CMplApiSpecialCommandHandler handler(mplPrtcl);
            handler.SetPhysicalInfoHeader(&connToCardTableEntry);
            connToCardTableEntry.DumpRaw("CMplApiDispatcherTask::OnBasicMsgToMplApi");
		}else if(STATUS_OK != con2CardStatus){
		  PASSERT((DWORD)con2CardStatus);
		}
	}
	else
		TRACEINTO << "Opcode: "<< mplPrtcl.getCommonHeaderOpcode() << " Invalid Connection ID!!!";


    // moved to one step before NORMAL sending

// 	OPCODE opcodeBefore = mplPrtcl.getCommonHeaderOpcode();
// 	CMplApiSpecialCommandHandler handler(mplPrtcl);
// 	STATUS status = handler.HandeSpecialCommand();
// 	if(STATUS_OK != status && FALSE == IsAudioCntrlOpcode(&mplPrtcl))
// 	{
// 		m_pProcess->OnSpecialCommanderFailure(mplPrtcl, opcodeBefore, status);
// 		return;
// 	}

	mplPrtcl.UpdateCommonHeaderToMPL();
	mplPrtcl.AddPayload_len(MPL_API_TYPE);

	SendMessageToTxSocket(&mplPrtcl);

	m_pProcess->IncrementCntMcmsToCsMfa();
}

///////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::SendMessageToTxSocket(CMplMcmsProtocol *mplPrtcl)
{
	if(MOVE_PARTY_RESOURCE_REQ == mplPrtcl->getCommonHeaderOpcode())
	{
		SendByLogicResources(mplPrtcl);
		return;
	}
	BYTE ret = NO;
	if(IsVideoEncoderMessage(mplPrtcl))
	{
		ret = HandleVideoEncoderMessages(mplPrtcl);
		if(ret)
			return;
	}


	if(IsAudioCntrlOpcode(mplPrtcl))
    {
        SendByAudioCntrRulls(mplPrtcl);
        return;
    }

	if(AUDIO_UPDATE_MUTE_REQ == mplPrtcl->getCommonHeaderOpcode())
	{
		DWORD confId    = mplPrtcl->getPortDescriptionHeaderConf_id();
		DWORD partyId   = mplPrtcl->getPortDescriptionHeaderParty_id();
		TRACEINTO << "PartyId:" << partyId << ", ConfId:" << confId;
		if(IsAudioRelayDecoderOnRmx(mplPrtcl))
		{
			SendUpdateMuteToMrmpAndMpl(mplPrtcl);
			return;
		}
	}



    // NORMAL sending

    OPCODE opcodeBefore = mplPrtcl->getCommonHeaderOpcode();
	CMplApiSpecialCommandHandler handler(*mplPrtcl);
	STATUS status = handler.HandeSpecialCommand();
	if(STATUS_OK != status)
	{
		m_pProcess->OnSpecialCommanderFailure(*mplPrtcl, opcodeBefore, status);
		return;
	}
    mplPrtcl->AddPayload_len(MPL_API_TYPE);

	int boardId = mplPrtcl->getPhysicalInfoHeaderBoard_id();
	if(INVALID_BOARD_ID == boardId)
	{
		TRACEINTO << "Opcode: "<< mplPrtcl->getCommonHeaderOpcode() <<" Invalid board ID!!!";
		//SendToAllActiveCards(mplPrtcl);
	}
	else
	{
		SendToCard(boardId, mplPrtcl);
	}
}

///////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::SendByLogicResources(CMplMcmsProtocol *mplPrtcl)
{
	MOVE_RESOURCES_REQ_S reqParam;
	memcpy(&reqParam, mplPrtcl->getpData(), sizeof(MOVE_RESOURCES_REQ_S));

	MOVE_RESOURCES_PARAMS_S confType = reqParam.moveRsrcParams;
	mplPrtcl->AddData(sizeof(MOVE_RESOURCES_PARAMS_S), (const char*)&confType);

	if (eRsrcActive == reqParam.openLogicalResources[eLogical_audio_encoder] ||
		eRsrcActive == reqParam.openLogicalResources[eLogical_audio_decoder] ||
		eRsrcActive == reqParam.openLogicalResources[eLogical_rtp]           ||
		eRsrcActive == reqParam.openLogicalResources[eLogical_mux])
	{
		eLogicalResourceTypes lrt = (eRsrcActive == reqParam.openLogicalResources[eLogical_audio_encoder]) ? eLogical_audio_encoder :
									((eRsrcActive == reqParam.openLogicalResources[eLogical_audio_decoder]) ? eLogical_audio_decoder : eLogical_rtp);
		SendMoveRsrcReq(mplPrtcl, MOVE_RSRC_ART_REQ, lrt);
	}

	if (eRsrcActive == reqParam.openLogicalResources[eLogical_relay_avc_to_svc_rtp_with_audio_encoder] ||
		eRsrcActive == reqParam.openLogicalResources[eLogical_legacy_to_SAC_audio_encoder])
	{
		SendMoveRsrcReq(mplPrtcl, MOVE_RSRC_ART_REQ, eLogical_relay_avc_to_svc_rtp_with_audio_encoder);
	}

	if (eRsrcActive == reqParam.openLogicalResources[eLogical_relay_avc_to_svc_rtp])
	{
		SendMoveRsrcReq(mplPrtcl, MOVE_RSRC_ART_REQ, eLogical_relay_avc_to_svc_rtp);
	}

	if (eRsrcActive == reqParam.openLogicalResources[eLogical_net])
	{
		SendMoveRsrcReq(mplPrtcl, MOVE_RSRC_NET_REQ);
	}

	if (eRsrcActive == reqParam.openLogicalResources[eLogical_video_encoder])
	{
		SendMoveRsrcReq(mplPrtcl, MOVE_RSRC_VIDEO_ENC_REQ, eLogical_video_encoder);
	}

	if (eRsrcActive == reqParam.openLogicalResources[eLogical_relay_avc_to_svc_video_encoder_1])
	{
		SendMoveRsrcReq(mplPrtcl, MOVE_RSRC_VIDEO_ENC_REQ, eLogical_relay_avc_to_svc_video_encoder_1);
	}

	if (eRsrcActive == reqParam.openLogicalResources[eLogical_relay_avc_to_svc_video_encoder_2])
	{
		SendMoveRsrcReq(mplPrtcl, MOVE_RSRC_VIDEO_ENC_REQ, eLogical_relay_avc_to_svc_video_encoder_2);
	}

	if (eRsrcActive == reqParam.openLogicalResources[eLogical_video_decoder])
	{
		SendMoveRsrcReq(mplPrtcl, MOVE_RSRC_VIDEO_DEC_REQ);
	}

	if (eRsrcActive == reqParam.openLogicalResources[eLogical_relay_rtp])
	{
		SendMoveRsrcReq(mplPrtcl, MOVE_RSRC_MRMP_REQ);
	}
}

///////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::SendMoveRsrcReq(CMplMcmsProtocol *mplPrtcl, OPCODE opcode, eLogicalResourceTypes lrt)
{
	OPCODE opcodeBefore = mplPrtcl->getCommonHeaderOpcode();
	mplPrtcl->SetCommonHeaderOpcode(opcode);

	CMplApiSpecialCommandHandler handler(*mplPrtcl);
	STATUS status = handler.HandeSpecialCommand((void*)lrt);
	if(STATUS_OK != status)
	{
		m_pProcess->OnSpecialCommanderFailure(*mplPrtcl, opcodeBefore, status);
		return;
	}

	mplPrtcl->SetCommonHeaderOpcode(MOVE_RSRC_REQ);
	mplPrtcl->AddPayload_len(MPL_API_TYPE);

	int boardId = mplPrtcl->getPhysicalInfoHeaderBoard_id();
	SendToCard(boardId, mplPrtcl);
}

///////////////////////////////////////////////////////////////////////////////
//Change Layout Improvement - Layout Shared Memory (CL-SM)
void  CMplApiDispatcherTask::HandleMultiplePartiesChangeLayoutRequest(CMplMcmsProtocol& mplPrtcl)
{
	DWORD numOfLayoutIds;
	memcpy(&numOfLayoutIds, mplPrtcl.getpData(),sizeof(DWORD));

	CLayoutSharedMemoryMap* pLayoutSharedMemoryMap = m_pProcess->GetLayoutSharedMemory();
	if (NULL == pLayoutSharedMemoryMap)
	{
		PASSERT_AND_RETURN(1);
	}

	CLayoutEntry layoutEntry;
	int status = STATUS_OK;

	DWORD layoutIdsArr[numOfLayoutIds];
	memcpy(layoutIdsArr, mplPrtcl.getpData()+sizeof(DWORD),numOfLayoutIds*sizeof(DWORD));

    std::ostringstream str;
    for (DWORD i = 0; i < numOfLayoutIds; i++) {
    	str <<  layoutIdsArr[i] << " ";
    	if ((i % 30) == 0)
    		str << "\n ";
    }
    TRACEINTO << "CL-SM: numOfLayoutIds: " << numOfLayoutIds << ", Layout ids': " << str.str().c_str();

	for (WORD i = 0; i < numOfLayoutIds; i++)
	{
		status = pLayoutSharedMemoryMap->Get(layoutIdsArr[i], layoutEntry);
		if ((STATUS_OK == status) && (layoutEntry.IsChanged()))
		{
			mplPrtcl.SetCommonHeaderOpcode(VIDEO_ENCODER_CHANGE_LAYOUT_REQ);
			mplPrtcl.UpdatePortDescriptionHeader( layoutEntry.GetPartyRsrcId(),
												  layoutEntry.GetConfRsrcId(),
												  layoutEntry.GetConnectionId(),
												  eLogical_video_encoder,
												  0, 0, 0, 0, 0, 0, 0xffff );

			MCMS_CM_CHANGE_LAYOUT_S changeLayoutParams = layoutEntry.GetChangeLayoutParams();
			WORD numOfImages = CLibsCommonHelperFuncs::GetNumbSubImg(changeLayoutParams.nLayoutType);

			int sizeOfChangeLayoutWithoutImages = sizeof(MCMS_CM_CHANGE_LAYOUT_S)-sizeof(MCMS_CM_IMAGE_PARAM_S*);
			int sizeOfImageParamArray = numOfImages*(sizeof(MCMS_CM_IMAGE_PARAM_S));
			int changeLayoutStructSize = sizeOfChangeLayoutWithoutImages + sizeOfImageParamArray;
			BYTE* pMessage = new BYTE[changeLayoutStructSize];
			memcpy(pMessage, (BYTE*)(&changeLayoutParams), sizeOfChangeLayoutWithoutImages);
			memcpy(pMessage+sizeOfChangeLayoutWithoutImages, (BYTE*)(layoutEntry.GetImageParam()), sizeOfImageParamArray);

			mplPrtcl.AddData(changeLayoutStructSize,(const char*)pMessage);
			PDELETEA(pMessage);

			CSegment *seg = new CSegment;
			mplPrtcl.Serialize(*seg, MPL_API_TYPE);

			OnBasicMsgToMplApi(seg);
			POBJDELETE(seg);
		}
		else
		{
			// not found
			TRACEINTO << " Not found, id: " << layoutIdsArr[i];
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//Indication Icon Change Improvement - Indication Icon Shared Memory (CL-SM)
void  CMplApiDispatcherTask::HandleMultiplePartiesIndicationIconChangeRequest(CMplMcmsProtocol& mplPrtcl)
{
	DWORD numOfIndicationIconChangeIds;
	memcpy(&numOfIndicationIconChangeIds, mplPrtcl.getpData(),sizeof(DWORD));

	CIndicationIconSharedMemoryMap* pIndicationIconSharedMemoryMap = m_pProcess->GetIndicationIconSharedMemory();
	if (NULL == pIndicationIconSharedMemoryMap)
	{
		PASSERT_AND_RETURN(1);
	}

	CIndicationIconEntry indicationIconEntry;
	int status = STATUS_OK;

	DWORD indicationIconIdsArr[numOfIndicationIconChangeIds];
	memcpy(indicationIconIdsArr, mplPrtcl.getpData()+sizeof(DWORD),numOfIndicationIconChangeIds*sizeof(DWORD));

    std::ostringstream str;
    for (DWORD i = 0; i < numOfIndicationIconChangeIds; i++) {
    	str <<  indicationIconIdsArr[i] << " ";
    	if ((i % 30) == 0)
    		str << "\n ";
    }
    TRACEINTO << "CL-SM: numOfIndicationIconIds: " << numOfIndicationIconChangeIds << ", Indication Icon ids': " << str.str().c_str();

	for (WORD i = 0; i < numOfIndicationIconChangeIds; i++)
	{
		status = pIndicationIconSharedMemoryMap->Get(indicationIconIdsArr[i], indicationIconEntry);
		if ((STATUS_OK == status) && (indicationIconEntry.IsChanged()))
		{
			mplPrtcl.SetCommonHeaderOpcode(VIDEO_ENCODER_ICONS_DISPLAY_REQ);
			mplPrtcl.UpdatePortDescriptionHeader( indicationIconEntry.GetPartyRsrcId(),
												  indicationIconEntry.GetConfRsrcId(),
												  indicationIconEntry.GetConnectionId(),
												  eLogical_video_encoder,
												  0, 0, 0, 0, 0, 0, 0xffff );

			ICONS_DISPLAY_S indicationIconParams = indicationIconEntry.GetIndicationIconParams();
			int indicationIconStructSize = sizeof(ICONS_DISPLAY_S);
			
			BYTE* pMessage = new BYTE[indicationIconStructSize];
			memcpy(pMessage, (BYTE*)(&indicationIconParams), indicationIconStructSize);
			
			mplPrtcl.AddData(indicationIconStructSize,(const char*)pMessage);
			PDELETEA(pMessage);

			CSegment *seg = new CSegment;
			mplPrtcl.Serialize(*seg, MPL_API_TYPE);

			OnBasicMsgToMplApi(seg);
			POBJDELETE(seg);
		}
		else
		{
			// not found
			TRACEINTO << " Not found, id: " << indicationIconIdsArr[i];
		}
	}
}



///////////////////////////////////////////////////////////////////////////////
void  CMplApiDispatcherTask::OnMplApiCloseCardConnection(CSegment* pMsg)
{
	 TRACEINTO << ">>>><<<<CMplApiDispatcherTask::OnMplApiCloseCardConnection. NOT IMPLEMENTED";

/*
TO BE DEALT WITH LATER

	WORD conId=0xFF;
	*pMsg >> conId;
	if (conId!=0xFF)

		for (int board_num=0;board_num<MAX_NUM_OF_BOARDS;board_num++)
			if (m_Mpl_Api_Card2SocketTable[board_num]->m_conId==conId)
			{
	            POBJDELETE(m_Mpl_Api_Card2SocketTable[board_num]) ;
				 return;
			}
*/
}

///////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::SendToAllActiveCards(CMplMcmsProtocol *mplPrtcl)
{
	for(WORD i = 0 ; i < MAX_NUM_OF_BOARDS ; i++)
	{
		WORD conId = m_pProcess->GetConnectionId(i);
		if((0xFF != conId)&&(0xFE != conId)&& (0xFD != conId)&&(0xFC != conId))
		{
			SendToCard(i, mplPrtcl);
		}
	}
}

void CMplApiDispatcherTask::SendToCard(WORD boardId, CMplMcmsProtocol *mplPrtcl)
{
	bool isIgnoredOpcode = false;
	// IgnoredOpcodesList is always empty if CFG_KEY_DEBUG_MODE is OFF !!!
	for (std::vector< DWORD >::iterator itr =  IgnoredOpcodesList.begin(); itr != IgnoredOpcodesList.end(); itr++) {
		if (  mplPrtcl->getOpcode() == (*itr) ) {
			isIgnoredOpcode = true;
			break;
		}
	}

	if (true == isIgnoredOpcode)
		return;

	bool showAssert=true;
    if (((mplPrtcl->getCommonHeaderOpcode() == ETHERNET_SETTINGS_CONFIG_REQ) || (mplPrtcl->getCommonHeaderOpcode() == Op802_1x_NEW_CONFIG_REQ))
    	            && (m_pProcess->IsStartupFinished() == FALSE))
        showAssert=false;

	COsQueue* TxtaskMailSlot = m_pProcess->GetTxQueue(boardId,showAssert);
	if (NULL == TxtaskMailSlot)
	{
		std::string buff = "Failed to get TxQueue, message canceled, opcode: ";
		buff += m_pProcess->GetOpcodeAsString(mplPrtcl->getCommonHeaderOpcode());
		TRACEINTO << "boardID:" << boardId
		          << ", subBoardID:" << (DWORD)mplPrtcl->getPhysicalInfoHeaderSub_board_id()
		          << ", unitID:" << (DWORD)mplPrtcl->getPhysicalInfoHeaderUnit_id() << "\n"
		          << buff <<"\n";
		return;
	}

	//VNGR-17161
	WORD UnitId = mplPrtcl->getPhysicalInfoHeaderUnit_id();
	WORD mpl_brdid = mplPrtcl->getPhysicalInfoHeaderBoard_id();
	WORD PortId = mplPrtcl->getPhysicalInfoHeaderPort_id();

	if( ( 0xFF == mpl_brdid ) &&
		( 0xFF == UnitId ) &&
		( 0xFF == PortId ) )
	{
		TRACEINTO << "Invalid boardId unitId and portId";
	}

	mplPrtcl->UpdateSendTimes();

	CSegment* pSeg = new CSegment;
	mplPrtcl->Serialize(*pSeg);

  CTaskApi api;
  api.CreateOnlyApi(*TxtaskMailSlot);
  STATUS status = api.SendMsg(pSeg, MPLAPI_MSG_TO_MPL);

  TRACECOND(STATUS_OK != status,
    "Failed to send MPLAPI_MSG_TO_MPL, boardID:" << boardId
     << ", subBoardID:" << (DWORD)mplPrtcl->getPhysicalInfoHeaderSub_board_id()
     << ", unitID:" << (DWORD)mplPrtcl->getPhysicalInfoHeaderUnit_id());

  const char* msg = (STATUS_OK == status) ?
    "MplApi SUCCESS to send to Tx Socket" :
    "MplApi FAILED to send to Tx Socket";

  m_MplMcmsProtocolTracer->SetData(mplPrtcl);
  m_MplMcmsProtocolTracer->TraceMplMcmsProtocol(msg);
  m_MplMcmsProtocolTracer->SetData(NULL);
}

void CMplApiDispatcherTask::SendByAudioCntrRulls(CMplMcmsProtocol *mplPrtcl)
{

    OPCODE opcode = mplPrtcl->getCommonHeaderOpcode();
    switch(opcode)
    {
        case AC_OPEN_CONF_REQ:
        case AC_CLOSE_CONF_REQ:
        case AC_LAYOUT_CHANGE_COMPLETE_REQ:
        case AC_UPDATE_CONF_PARAMS_REQ:
	  SpreadACMessageToAllCards(mplPrtcl);
	  break;
        case AC_OPEN_CONF_RESEND:
	  mplPrtcl->SetCommonHeaderOpcode(AC_OPEN_CONF_REQ);
	  SendACMessageToNewCard(mplPrtcl);
	  break;
        default:
	  PASSERT(opcode);
          break;
    }

  // Remarked code: changed in V4.2 from: sending to 1 master and 1 slave to: send to all audio controllers
    // 1) send to master
    //mplPrtcl->GetPhysicalHeader().future_use1 = (APIU8)E_AC_MASTER;
    //HandleMessageSendToCard(mplPrtcl, TRUE);

    // 2) send to slave
    //mplPrtcl->GetPhysicalHeader().future_use1 = (APIU8)E_AC_SLAVE;
    //HandleMessageSendToCard(mplPrtcl, FALSE);
}

//////////////////////////////////////////////////////////////////////////////
// Amos AC

// to check: m_pProcess->OnSpecialCommanderFailure(*mplPrtcl, mplPrtcl->getCommonHeaderOpcode(), status);
void CMplApiDispatcherTask::SpreadACMessageToAllCards(CMplMcmsProtocol *mplPrtcl)
{
  // TRACEINTO << "AMOS_AC_DEBUG: CMplApiDispatcherTask::SpreadACMessageToAllCards";
  STATUS status = STATUS_OK;

  // we use copy of mplPrtcl in order not to change mplPrtcl inside CMplApiSpecialCommandHandler
  CMplMcmsProtocol entry_search_mplPrtcl(*mplPrtcl);
  CMplApiSpecialCommandHandler search_handler(entry_search_mplPrtcl);

  // search for all entries of the same type (ePhysical_audio_controller, E_AC_MASTER/E_AC_SLAVE/E_AC_RESERVED )
  DWORD max_num_of_ac = 8; // for Amos only 4 possible, 8 for future and testing
  ConnToCardTableEntry** entriesFoundArray = new ConnToCardTableEntry*[max_num_of_ac];
  for(DWORD init_index=0;init_index<max_num_of_ac;init_index++){
    entriesFoundArray[init_index]=NULL;
  }
  DWORD entries_found = 0;

  entries_found = search_handler.GetPhysicalInfoHeaderByRsrcTypeCntrltype(ePhysical_audio_controller,0,E_NORMAL,entriesFoundArray,max_num_of_ac);

  // TRACEINTO << "AMOS_AC_DEBUG: CMplApiDispatcherTask::SpreadACMessageToAllCard, entries_found = " << entries_found;

  if(entries_found < 1 || entries_found > max_num_of_ac)
  {
    status = STATUS_PROBLEMS_IN_SHARED_MEMORY;
    delete entriesFoundArray; // ron: use delete and not delete[] to delete the pointers array and not the shared memory entries
    PASSERT(status);
    return;
  }

  // send to all cards
  for(DWORD found_entry_index = 0; found_entry_index < entries_found; found_entry_index++){
    // we use copy of mplPrtcl in order not to change mplPrtcl inside CMplApiSpecialCommandHandler
    CMplMcmsProtocol entry_mplPrtcl(*mplPrtcl);
    CMplApiSpecialCommandHandler handler_entry(entry_mplPrtcl);

    if(entriesFoundArray[found_entry_index] != NULL){

       handler_entry.SetPhysicalInfoHeader(entriesFoundArray[found_entry_index]);
       entry_mplPrtcl.GetPhysicalHeader().future_use1 = (APIU8)(entriesFoundArray[found_entry_index]->rsrcCntlType);
       entry_mplPrtcl.SetPortDescriptorHeaderLogicRsrcType1(eLogical_audio_controller);

       WORD boardId = entry_mplPrtcl.getPhysicalInfoHeaderBoard_id();
       // TRACEINTO << "AMOS_AC_DEBUG: CMplApiDispatcherTask::SpreadACMessageToAllCards entry found:  boardId = " << boardId << " , rsrcCntlType = "<< (DWORD)(entriesFoundArray[found_entry_index]->rsrcCntlType);
       SendToCard(boardId, &entry_mplPrtcl);

    }else{
      // we should not be here
      PTRACE2INT(eLevelError,"CMplApiDispatcherTask::HandleACMessageSendToAllCards , wrong entry = NULL ", entries_found);
    }
  }

  delete[] entriesFoundArray;
  return;
}
//////////////////////////////////////////////////////////////////////////////

void CMplApiDispatcherTask::SendACMessageToNewCard(CMplMcmsProtocol *mplPrtcl)
{
  CMplApiSpecialCommandHandler handler(*mplPrtcl);
  WORD boardId = mplPrtcl->getPhysicalInfoHeaderBoard_id();
  TRACEINTO << "AMOS_AC_DEBUG: CMplApiDispatcherTask::SendACMessageToNewCard send to boardId = " << boardId;
  const ConnToCardTableEntry* entry_found = handler.GetPhysicalInfoHeaderByRsrcTypeAndBoardId(ePhysical_audio_controller);
  if(entry_found != NULL){
    handler.SetPhysicalInfoHeader(entry_found);
    mplPrtcl->GetPhysicalHeader().future_use1 = (APIU8)(entry_found->rsrcCntlType);
    mplPrtcl->SetPortDescriptorHeaderLogicRsrcType1(eLogical_audio_controller);
    SendToCard(boardId, mplPrtcl);
  }else{
    TRACEINTO << "CMplApiDispatcherTask::SendACMessageToNewCard failed to find boardId = " << boardId;
    PASSERT(boardId);
  }
}

//////////////////////////////////////////////////////////////////////////////
  // Remarked code: changed in V4.2 from: sending to 1 master and 1 slave to: send to all audio controllers
// void CMplApiDispatcherTask::HandleMessageSendToCard(CMplMcmsProtocol *mplPrtcl, BOOL isShoudBeFoundInDB)
// {
//     CMplApiSpecialCommandHandler handler(*mplPrtcl);
// 	STATUS status = handler.HandeSpecialCommand();
// 	if(STATUS_OK != status)
// 	{
//         if(TRUE == isShoudBeFoundInDB)
//         {
//             m_pProcess->OnSpecialCommanderFailure(*mplPrtcl, mplPrtcl->getCommonHeaderOpcode(), status);
//         }
//         return;
// 	}

//     int boardId = mplPrtcl->getPhysicalInfoHeaderBoard_id();
//     SendToCard(boardId, mplPrtcl);
// }
//////////////////////////////////////////////////////////////////////////////
BOOL CMplApiDispatcherTask::HandleVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl)
{

  //TRACEINTO << "(VNGR-26979) " << "CMplApiDispatcherTask::HandleVideoEncoderMessages";
	BOOL ret = FALSE;
	OPCODE pOpcode = mplMcmsPrtcl->getCommonHeaderOpcode();

	//	PTRACE2(eLevelInfoNormal,"CMplApiDispatcherTask::HandleVideoEncoderMessages - ", m_pProcess->GetOpcodeAsString(pOpcode).c_str() );

	ECntrlType videoEncoderCntlType = E_NORMAL;
	BYTE logicRsrcType = mplMcmsPrtcl->getPortDescriptionHeaderLogical_resource_type_1();
	if(GetGenericEncoderDecoderRsrcType(logicRsrcType) == eLogical_video_encoder)
	{
		ConnToCardTableEntry 	connToCardTableEntry;
		APIU32               	connectionId = 0xffffffff;
		connectionId = mplMcmsPrtcl->getPortDescriptionHeaderConnection_id();
		CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
		STATUS con2CardStatus = pSharedMemMap->Get(connectionId,connToCardTableEntry);
		connToCardTableEntry.DumpRaw("1080_60: CMplApiDispatcherTask::HandleVideoEncoderMessages: connToCardTableEntry ");
		videoEncoderCntlType = connToCardTableEntry.rsrcCntlType;
		//temp Eitan PCM
		if (eLogical_COP_PCM_encoder == logicRsrcType/* && pOpcode != VIDEO_ENCODER_UPDATE_PARAM_REQ*/)
			videoEncoderCntlType = E_ENCODER_PCM;
		switch(pOpcode)
		{
			case(TB_MSG_OPEN_PORT_REQ):
			{
				ret = HandleOpenVideoEncoderMessages(mplMcmsPrtcl, videoEncoderCntlType);
				break;

			}
			case(VIDEO_ENCODER_UPDATE_PARAM_REQ):
			{
			  //			  TRACEINTO << "(VNGR-26979) " << "CMplApiDispatcherTask::HandleVideoEncoderMessages VIDEO_ENCODER_UPDATE_PARAM_REQ";
				if(eSystemCardsMode_breeze == GetSystemCardsBasedMode()) // 1080_60
				{
				  // PTRACE2(eLevelInfoNormal,"1080_60:CMplApiDispatcherTask::HandleVideoEncoderMessages - VIDEO_ENCODER_UPDATE_PARAM_REQ is currently blocked", m_pProcess->GetOpcodeAsString(pOpcode).c_str() );
					// ret = HandleGenericVideoEncoderMessages(mplMcmsPrtcl, videoEncoderCntlType);
					ret = HandleGenericUpdateVideoEncoderMessages(mplMcmsPrtcl, videoEncoderCntlType);
				}
				else
				{
					HandleUpdateVideoEncoderMessages(mplMcmsPrtcl, videoEncoderCntlType);
				}
				break;
			}
			case(TB_MSG_CONNECT_REQ):
			{
				ret = HandleConnectVideoEncoderMessages(mplMcmsPrtcl, videoEncoderCntlType);
				break;
			}
// 			case(TB_MSG_CLOSE_PORT_REQ):
// 			{
// 				ret = HandleCloseVideoEncoderMessages(mplMcmsPrtcl, videoEncoderCntlType);
// 				break;
// 			}
			default:
// 1080_60
				ret = HandleGenericVideoEncoderMessages(mplMcmsPrtcl, videoEncoderCntlType);
				break;

		}
	}
	else
	{
		PTRACE(eLevelError,"CMplApiDispatcherTask::HandleVideoEncoderMessages not video encoder!!!!");

	}
	return ret;

}

// 1080_60
BOOL CMplApiDispatcherTask::HandleGenericVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType)
{
	BOOL ret = FALSE;
	BOOL in = FALSE;
	if(videoEncoderCntlType == E_VIDEO_MASTER_SPLIT_ENCODER &&
			eSystemCardsMode_breeze == GetSystemCardsBasedMode())
	{

	  // PTRACE2(eLevelInfoNormal,"1080_60: CMplApiDispatcherTask::HandleGenericVideoEncoderMessages E_VIDEO_MASTER_SPLIT_ENCODER - ", m_pProcess->GetOpcodeAsString(mplMcmsPrtcl->getCommonHeaderOpcode()).c_str() );
		SendGenericVideoEncoderMsgsForSplitEnc(mplMcmsPrtcl, videoEncoderCntlType);
		ret = TRUE;
	}else{
	  if(videoEncoderCntlType != E_VIDEO_MASTER_SPLIT_ENCODER){
	    //	    PTRACE2(eLevelInfoNormal,"1080_60: CMplApiDispatcherTask::HandleGenericVideoEncoderMessages not E_VIDEO_MASTER_SPLIT_ENCODER  videoEncoderCntlType = ", RsrcCntlTypeToString(videoEncoderCntlType) );
	  }
	}
	return ret;
}

BOOL CMplApiDispatcherTask::HandleGenericUpdateVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType)
{
  //  TRACEINTO << "(VNGR-26979) " << "CMplApiDispatcherTask::HandleGenericUpdateVideoEncoderMessages, videoEncoderCntlType = " << RsrcCntlTypeToString(videoEncoderCntlType);
	BOOL ret = FALSE;
	BOOL in = FALSE;
	if(videoEncoderCntlType == E_VIDEO_MASTER_SPLIT_ENCODER &&
			eSystemCardsMode_breeze == GetSystemCardsBasedMode())
	{

	  // PTRACE2(eLevelInfoNormal,"1080_60: CMplApiDispatcherTask::HandleGenericVideoEncoderMessages E_VIDEO_MASTER_SPLIT_ENCODER - ", m_pProcess->GetOpcodeAsString(mplMcmsPrtcl->getCommonHeaderOpcode()).c_str() );
		SendGenericUpdateVideoEncoderMsgsForSplitEnc(mplMcmsPrtcl, videoEncoderCntlType);
		ret = TRUE;
	}else{
	  if(videoEncoderCntlType == E_VIDEO_SLAVE_SPLIT_ENCODER){
	    // do nothing - SendGenericUpdateVideoEncoderMsgsForSplitEnc on E_VIDEO_MASTER_SPLIT_ENCODER already sent to both
	    TRACEINTO << "1080_60: CMplApiDispatcherTask::HandleGenericVideoEncoderMessages videoEncoderCntlType = " << RsrcCntlTypeToString(videoEncoderCntlType);
	  }else{
	     EVideoEncoderType apiVideoEncoderType = E_VIDEO_ENCODER_NORMAL;
	     apiVideoEncoderType = TranslateVideoEncoderCntlTypeToApi(videoEncoderCntlType);
	     UpdateVideoEncoderType(mplMcmsPrtcl, apiVideoEncoderType);
	  }
	    
	}
	return ret;
}



void CMplApiDispatcherTask::SendGenericVideoEncoderMsgsForSplitEnc(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderMasterCntlType)
{

  //   TRACEINTO << "(VNGR-26979) " << "CMplApiDispatcherTask::SendGenericVideoEncoderMsgsForSplitEn, videoEncoderCntlType = " << RsrcCntlTypeToString(videoEncoderMasterCntlType);
	//ENCODER_PARAM_S *pEncoderParamsStruct = (ENCODER_PARAM_S *)mplMcmsPrtcl->getpData();
	APIU32 connectionId = 0xffffffff;
	connectionId = mplMcmsPrtcl->getPortDescriptionHeaderConnection_id();
	ConnToCardTableEntry 	masterConnToCardTableEntry;
	CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
	STATUS status = pSharedMemMap->Get(connectionId, masterConnToCardTableEntry);
	ECntrlType videoEncoderSlaveCntlType = GetSlaveCntlTypeFromMasterCntlType(videoEncoderMasterCntlType);
	ConnToCardTableEntry* pSlaveConnToCardTableEntry = NULL;
	pSlaveConnToCardTableEntry = GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(masterConnToCardTableEntry.rsrc_conf_id,masterConnToCardTableEntry.rsrc_party_id,videoEncoderSlaveCntlType);

	if(	pSlaveConnToCardTableEntry )
	{
		EVideoEncoderType masterVideoEncoderApiType = TranslateVideoEncoderCntlTypeToApi(videoEncoderMasterCntlType);
		EVideoEncoderType slaveVideoEncoderApiType = TranslateVideoEncoderCntlTypeToApi(videoEncoderSlaveCntlType);

	    masterConnToCardTableEntry.DumpRaw("1080_60: CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForSplitEnc (Master)");
		pSlaveConnToCardTableEntry->DumpRaw("1080_60: CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForSplitEnc (Slave)");

		CreateAndSendTBGenericReq(mplMcmsPrtcl,/* pEncoderParamsStruct,*/ E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER, masterConnToCardTableEntry);
		CreateAndSendTBGenericReq(mplMcmsPrtcl, /*pEncoderParamsStruct,*/ E_VIDEO_ENCODER_SLAVE_SPLIT_ENCODER, *pSlaveConnToCardTableEntry);



	}
}

void CMplApiDispatcherTask::SendGenericUpdateVideoEncoderMsgsForSplitEnc(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderMasterCntlType)
{

  //  TRACEINTO << "(VNGR-26979) " << "CMplApiDispatcherTask::SendGenericUpdateVideoEncoderMsgsForSplitEnc, videoEncoderCntlType = " << RsrcCntlTypeToString(videoEncoderMasterCntlType);			

	//ENCODER_PARAM_S *pEncoderParamsStruct = (ENCODER_PARAM_S *)mplMcmsPrtcl->getpData();
	APIU32 connectionId = 0xffffffff;
	connectionId = mplMcmsPrtcl->getPortDescriptionHeaderConnection_id();
	ConnToCardTableEntry 	masterConnToCardTableEntry;
	CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
	STATUS status = pSharedMemMap->Get(connectionId, masterConnToCardTableEntry);
	ECntrlType videoEncoderSlaveCntlType = GetSlaveCntlTypeFromMasterCntlType(videoEncoderMasterCntlType);
	ConnToCardTableEntry* pSlaveConnToCardTableEntry = NULL;
	pSlaveConnToCardTableEntry = GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(masterConnToCardTableEntry.rsrc_conf_id,masterConnToCardTableEntry.rsrc_party_id,videoEncoderSlaveCntlType);

	if(	pSlaveConnToCardTableEntry )
	{
		EVideoEncoderType masterVideoEncoderApiType = TranslateVideoEncoderCntlTypeToApi(videoEncoderMasterCntlType);
		EVideoEncoderType slaveVideoEncoderApiType = TranslateVideoEncoderCntlTypeToApi(videoEncoderSlaveCntlType);

	    masterConnToCardTableEntry.DumpRaw("1080_60: CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForSplitEnc (Master)");
		pSlaveConnToCardTableEntry->DumpRaw("1080_60: CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForSplitEnc (Slave)");

		UpdateVideoEncoderType(mplMcmsPrtcl, masterVideoEncoderApiType);
		CreateAndSendTBGenericReq(mplMcmsPrtcl,/* pEncoderParamsStruct,*/ E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER, masterConnToCardTableEntry);
		UpdateVideoEncoderType(mplMcmsPrtcl, slaveVideoEncoderApiType);
		CreateAndSendTBGenericReq(mplMcmsPrtcl, /*pEncoderParamsStruct,*/ E_VIDEO_ENCODER_SLAVE_SPLIT_ENCODER, *pSlaveConnToCardTableEntry);



	}
}

void CMplApiDispatcherTask::CreateAndSendTBGenericReq(CMplMcmsProtocol *mplMcmsPrtcl, EVideoEncoderType rsrcCntlType,ConnToCardTableEntry& pConnToCardTableEntry)
{
		OPCODE opcode = mplMcmsPrtcl->getCommonHeaderOpcode();;
		//pEncoderParamStruct->nVideoEncoderType          = rsrcCntlType;
		DWORD reqId = mplMcmsPrtcl->getMsgDescriptionHeaderRequest_id();
		CMplMcmsProtocol *pLocalMplMcmsProtocol = new CMplMcmsProtocol;
		pLocalMplMcmsProtocol->AddCommonHeader(mplMcmsPrtcl->getCommonHeaderOpcode());
		pLocalMplMcmsProtocol->AddMessageDescriptionHeader();
		pLocalMplMcmsProtocol->AddPortDescriptionHeader(pConnToCardTableEntry.rsrc_party_id,
												   pConnToCardTableEntry.rsrc_conf_id,
												   pConnToCardTableEntry.m_id,
												   pConnToCardTableEntry.rsrcType);
		pLocalMplMcmsProtocol->AddData(mplMcmsPrtcl->getDataLen(), (const char*)mplMcmsPrtcl->getpData());
		//pMplMcmsProtocol->AddData(sizeof(ENCODER_PARAM_S), (const char*)(pEncoderParamStruct));

		pLocalMplMcmsProtocol->GetPhysicalInfoHeaderConnToCardTable(&pConnToCardTableEntry);
		pLocalMplMcmsProtocol->UpdateCommonHeaderToMPL(mplMcmsPrtcl->getCommonHeaderSequence_num());


		SetMasterSlaveRequestHandlerSubIdData(opcode, reqId, pLocalMplMcmsProtocol->getMsgDescriptionHeaderRequest_id());

		CMplApiSpecialCommandHandler handler(*pLocalMplMcmsProtocol);
		STATUS status = handler.HandeSpecialCommand((void *)&pConnToCardTableEntry);
		if(STATUS_OK != status)
		{
			m_pProcess->OnSpecialCommanderFailure(*pLocalMplMcmsProtocol, opcode, status);
			POBJDELETE(pLocalMplMcmsProtocol);
			return;
		}
		pLocalMplMcmsProtocol->AddPayload_len(MPL_API_TYPE);

		int boardId = pLocalMplMcmsProtocol->getPhysicalInfoHeaderBoard_id();
		if(INVALID_BOARD_ID == boardId)
		{
			//SendToAllActiveCards(pLocalMplMcmsProtocol);
		}
		else
		{
			SendToCard(boardId, pLocalMplMcmsProtocol);
		}
		POBJDELETE(pLocalMplMcmsProtocol);
		return;

		/*pMplMcmsProtocol->AddPayload_len(MPL_API_TYPE);


		int boardId = pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id();
		SendToCard(boardId, pMplMcmsProtocol);

		POBJDELETE(pMplMcmsProtocol);*/








	/*mplMcmsPrtcl->AddPortDescriptionHeader(pConnToCardTableEntry.rsrc_party_id,
											   pConnToCardTableEntry.rsrc_conf_id,
											   pConnToCardTableEntry.m_id,
											   pConnToCardTableEntry.rsrcType);



	opcodeBefore = mplMcmsPrtcl->getCommonHeaderOpcode();
	CMplApiSpecialCommandHandler handler(*mplMcmsPrtcl);
	STATUS status = handler.HandeSpecialCommand((void *)&pConnToCardTableEntry);
	if(STATUS_OK != status)
	{
		m_pProcess->OnSpecialCommanderFailure(*mplMcmsPrtcl, opcodeBefore, status);
		return;
	}
	mplMcmsPrtcl->AddPayload_len(MPL_API_TYPE);

	int boardId = mplMcmsPrtcl->getPhysicalInfoHeaderBoard_id();
	if(INVALID_BOARD_ID == boardId)
	{
		SendToAllActiveCards(mplMcmsPrtcl);
	}
	else
	{
		SendToCard(boardId, mplMcmsPrtcl);
	}


*/




	/*ENCODER_PARAM_S nEncoderStruct;
	memset(&nEncoderStruct,0,sizeof(ENCODER_PARAM_S));

	nEncoderStruct.nVideoConfType                       = pEncoderStruct->nVideoConfType;
	nEncoderStruct.nVideoEncoderType             		= rsrcCntlType;
	nEncoderStruct.nBitRate    		             		= pEncoderStruct->nBitRate;
	nEncoderStruct.nProtocol   		             		= pEncoderStruct->nProtocol;

	nEncoderStruct.tH263_H261VideoParams.nQcifFrameRate = pEncoderStruct->tH263_H261VideoParams.nQcifFrameRate;
	nEncoderStruct.tH263_H261VideoParams.nCifFrameRate  = pEncoderStruct->tH263_H261VideoParams.nCifFrameRate;
	nEncoderStruct.tH263_H261VideoParams.n4CifFrameRate = pEncoderStruct->tH263_H261VideoParams.n4CifFrameRate;
	nEncoderStruct.tH263_H261VideoParams.b263HighBbIntra = pEncoderStruct->tH263_H261VideoParams.b263HighBbIntra; //Default should be NO !! H263_HIGH_BIT_BUDGET_INTRA when set to YES (1) the H263 intra that sent will be bigger.(the intra will be SOFTER) China request for soft intra

	nEncoderStruct.tH264VideoParams.nMBPS        		= pEncoderStruct->tH264VideoParams.nMBPS;
	nEncoderStruct.tH264VideoParams.nFS          		= pEncoderStruct->tH264VideoParams.nFS;
	nEncoderStruct.tH264VideoParams.nStaticMB			= pEncoderStruct->tH264VideoParams.nStaticMB;
	nEncoderStruct.tH264VideoParams.nResolutionWidth    = pEncoderStruct->tH264VideoParams.nResolutionWidth;
	nEncoderStruct.tH264VideoParams.nResolutionHeight   = pEncoderStruct->tH264VideoParams.nResolutionHeight;
	nEncoderStruct.tH264VideoParams.nResolutionFrameRate= pEncoderStruct->tH264VideoParams.nResolutionFrameRate;
	nEncoderStruct.tH264VideoParams.nProfile   = pEncoderStruct->tH264VideoParams.nProfile;
	nEncoderStruct.tH264VideoParams.unPacketPayloadFormat= pEncoderStruct->tH264VideoParams.unPacketPayloadFormat;
	nEncoderStruct.tH264VideoParams.unMaxDPB 			 =pEncoderStruct->tH264VideoParams.unMaxDPB;
	nEncoderStruct.nSampleAspectRatio = pEncoderStruct->nSampleAspectRatio;
	nEncoderStruct.tDecoderDetectedMode.nDecoderDetectedModeWidth  = pEncoderStruct->tDecoderDetectedMode.nDecoderDetectedModeWidth;
	nEncoderStruct.tDecoderDetectedMode.nDecoderDetectedModeHeight = pEncoderStruct->tDecoderDetectedMode.nDecoderDetectedModeHeight;
	nEncoderStruct.tDecoderDetectedMode.nDecoderDetectedSampleAspectRatioWidth = pEncoderStruct->tDecoderDetectedMode.nDecoderDetectedSampleAspectRatioWidth;
	nEncoderStruct.tDecoderDetectedMode.nDecoderDetectedSampleAspectRatioHeight = pEncoderStruct->tDecoderDetectedMode.nDecoderDetectedSampleAspectRatioHeight;


	nEncoderStruct.nResolutionTableType         = pEncoderStruct->nResolutionTableType;
	nEncoderStruct.nParsingMode                 = pEncoderStruct->nParsingMode;
	nEncoderStruct.nMTUSize                     = pEncoderStruct->nMTUSize;
	nEncoderStruct.nTelePresenceMode            = pEncoderStruct->nTelePresenceMode;
	nEncoderStruct.nVideoQualityType            = pEncoderStruct->nVideoQualityType;
	nEncoderStruct.bIsVideoClarityEnabled       = pEncoderStruct->bIsVideoClarityEnabled;

	nEncoderStruct.tCroppingParams.nHorizontalCroppingPercentage = pEncoderStruct->tCroppingParams.nHorizontalCroppingPercentage;
    nEncoderStruct.tCroppingParams.nVerticalCroppingPercentage =  pEncoderStruct->tCroppingParams.nVerticalCroppingPercentage;
    nEncoderStruct.bEnableMbRefresh =  pEncoderStruct->bEnableMbRefresh;
    nEncoderStruct.nEncoderResolutionRatio = pEncoderStruct->nEncoderResolutionRatio;
    nEncoderStruct.nMaxSingleTransferFrameSize = pEncoderStruct->nMaxSingleTransferFrameSize;

    // TIP
    nEncoderStruct.bIsTipMode = pEncoderStruct->bIsTipMode;
*/
/*	pEncoderParamStruct->nVideoEncoderType          = rsrcCntlType;
	CMplMcmsProtocol *pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->AddCommonHeader(pOpcode);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPortDescriptionHeader(pConnToCardTableEntry.rsrc_party_id,
											   pConnToCardTableEntry.rsrc_conf_id,
											   pConnToCardTableEntry.m_id,
											   pConnToCardTableEntry.rsrcType);
	//pMplMcmsProtocol->AddData(sizeof(ENCODER_PARAM_S), (const char*)(&nEncoderStruct));
	pMplMcmsProtocol->AddData(sizeof(ENCODER_PARAM_S), (const char*)(pEncoderParamStruct));

	pMplMcmsProtocol->GetPhysicalInfoHeaderConnToCardTable(&pConnToCardTableEntry);
	pMplMcmsProtocol->UpdateCommonHeaderToMPL();
	pMplMcmsProtocol->AddPayload_len(MPL_API_TYPE);


	int boardId = pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id();
	SendToCard(boardId, pMplMcmsProtocol);

	POBJDELETE(pMplMcmsProtocol);*/
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This function handles all the special treatments for the open video encoder
BOOL CMplApiDispatcherTask::HandleOpenVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType)
{
	BOOL ret = FALSE;
	switch(videoEncoderCntlType)
	{
		/// MASTER-SLAVE Layout Builder and Full Encoder topology in MPM cards - for SD/HD asymmetric calls
		case E_VIDEO_MASTER_LB_ONLY:
		{
			SendOpenMasterSlaveVideoEncoderMsgsForLBandFullEnc(mplMcmsPrtcl);
			ret = TRUE;
		}
		break;
		/// MASTER-SLAVE split video encoder topology in MPM+ cards - for HD1080 asymmetric calls
		// from 03.09 Due to real time problem in HD720 decoders, the HD720 encoder was split too to master-slave
		// With the same topology but with new types due to difference in the CM code
		case E_VIDEO_MASTER_SPLIT_ENCODER:
		case E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP:
		{
			SendOpenMasterSlaveVideoEncoderMsgsForSplitEnc(mplMcmsPrtcl, videoEncoderCntlType);
			ret = TRUE;
		}
		break;
		case E_ENCODER_PCM:
		{
			UpdateVideoEncoderType(mplMcmsPrtcl, E_VIDEO_ENCODER_PCM);
			break;
		}
		default:
		{
			UpdateVideoEncoderType(mplMcmsPrtcl, E_VIDEO_ENCODER_NORMAL);
		}
	}
	return ret;
}
//////////////////////////////////////////////////////////////////////////////
//This function handles all the special treatments for the update video encoder
void CMplApiDispatcherTask::HandleUpdateVideoEncoderMessages(CMplMcmsProtocol * mplMcmsPrtcl, ECntrlType videoEncoderCntlType)
{
  //  TRACEINTO << "(VNGR-26979) " << "CMplApiDispatcherTask::HandleUpdateVideoEncoderMessages, videoEncoderCntlType = " << RsrcCntlTypeToString(videoEncoderCntlType);
	switch((WORD)videoEncoderCntlType)
	{
// 1080_60
		case 1800:

		break;
		default:
		{
		  //		  TRACEINTO << "(VNGR-26979) " << "CMplApiDispatcherTask::HandleUpdateVideoEncoderMessages, set videoEncoderCntlType = " << RsrcCntlTypeToString(E_VIDEO_ENCODER_NORMAL);
			EVideoEncoderType apiVideoEncoderType = E_VIDEO_ENCODER_NORMAL;
			apiVideoEncoderType = TranslateVideoEncoderCntlTypeToApi(videoEncoderCntlType);
			UpdateVideoEncoderType(mplMcmsPrtcl, apiVideoEncoderType);
			break;
		}
	}
}
// 1080_60
void CMplApiDispatcherTask::SendUpdateMasterSlaveVideoEncoderMsgsForSplitEnc(CMplMcmsProtocol * mplMcmsPrtcl, ECntrlType videoEncoderCntlType)
{

	/*Incase of Close Master Video Encoder with Split Encoder topology(In MPM+ for HD1080 calls, and for HD720 symmetric calls from 03.09)
	 * we send only close port to the master:
	 * This function is in the use of simulation only!!!
			 * 1. TB_MSG_CLOSE_PORT_REQ to the master Encoder with the Master's physical info
			 * 2. TB_MSG_CLOSE_PORT_REQ to the  slave Encoder with the Slaves's physical info
*/
	/*PTRACE(eLevelError,"CMplApiDispatcherTask::SendCloseMasterSlaveVideoEncoderMsgsForSplitEnc");
	ConnToCardTableEntry 	masterConnToCardTableEntry;
	APIU32               	masterConnectionId = 0xffffffff;
	masterConnectionId = mplPrtcl->getPortDescriptionHeaderConnection_id();
	if(masterConnectionId!=0xffffffff)
	{
		CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
		STATUS con2CardStatus = pSharedMemMap->Get(masterConnectionId,masterConnToCardTableEntry);
		if (con2CardStatus == STATUS_OK)
		{
			ECntrlType slaveEncoderCntlType =  GetSlaveCntlTypeFromMasterCntlType(masterEncoderCntlType);
			ConnToCardTableEntry* pSlaveConnToCardTableEntry = NULL;
			pSlaveConnToCardTableEntry = GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(masterConnToCardTableEntry.rsrc_conf_id,masterConnToCardTableEntry.rsrc_party_id,slaveEncoderCntlType);

			if(	pSlaveConnToCardTableEntry )
			{
				CreateAndSendTBClosePortReq(masterConnToCardTableEntry);
				CreateAndSendTBClosePortReq(*pSlaveConnToCardTableEntry);
			}
		}
		else
		{
			PTRACE(eLevelError,"CMplApiDispatcherTask::SendCloseMasterSlaveVideoEncoderMsgsForSplitEnc didn't find connect to card table entry");
		}
	}*/
}

//////////////////////////////////////////////////////////////////////////////
BOOL CMplApiDispatcherTask::HandleConnectVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType)
{
	PTRACE2INT(eLevelError,"CMplApiDispatcherTask::HandleConnectVideoEncoderMessages videoEncoderCntlType ", videoEncoderCntlType);

	BOOL ret = FALSE;
	if(videoEncoderCntlType == E_VIDEO_MASTER_LB_ONLY)
	{
		/// MASTER-SLAVE video encoder topology in MPM cards (SD/HD720asymmetric calls) we connect the slave to art
		SendConnectMasterSlaveVideoEncoderMsgForLBandFullEnc(mplMcmsPrtcl);
		ret = TRUE;
	}
	return ret;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CMplApiDispatcherTask::HandleCloseVideoEncoderMessages(CMplMcmsProtocol *mplMcmsPrtcl,ECntrlType videoEncoderCntlType)
{
	BOOL ret = FALSE;
	switch(videoEncoderCntlType)
	{
		/// MASTER-SLAVE video encoder in MPM cards (SD/HD720asymmetric calls)
		case E_VIDEO_MASTER_LB_ONLY:
		{
			SendCloseMasterSlaveVideoEncoderMsgsForLBandFullEnc(mplMcmsPrtcl);
			ret = TRUE;
		}
		break;
		/// MASTER-SLAVE video encoder in MPM+ cards (HD1080asymmetric calls) and for HD720 symmetric calls from 03.09
		case E_VIDEO_MASTER_SPLIT_ENCODER:
		case E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP:
		{
			PTRACE(eLevelInfoNormal,"CMplApiDispatcherTask::HandleCloseVideoEncoderMessages send just close port to master in case of E_VIDEO_MASTER_SPLIT_ENCODER and E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP");
			/////JUST FOR SIMULATION!!!!!!

			if(eSystemCardsMode_breeze == GetSystemCardsBasedMode() ||  FALSE == IsTarget())
			{
			  if(FALSE == IsTarget()){
				///JUST FOR SIMULATION
				PTRACE(eLevelInfoNormal,"CMplApiDispatcherTask::HandleCloseVideoEncoderMessages JUST FOR SIMULATION send close port to both master and slave in case of E_VIDEO_MASTER_SPLIT_ENCODER and E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP, so we can get the ack on close port from slave");
				ret = TRUE;
			  }else{

				PTRACE(eLevelInfoNormal,"CMplApiDispatcherTask::HandleCloseVideoEncoderMessages 1080_60: send close port to both master and slave encoders");
			  }
			  SendCloseMasterSlaveVideoEncoderMsgsForSplitEnc(mplMcmsPrtcl,videoEncoderCntlType );

			}
		
		}
		break;

		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;

	}
	return ret;

}

//////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForLBandFullEnc(CMplMcmsProtocol *mplPrtcl)
{
	/*Incase of Open Video Encoder of Master-Slave video encoder,when the Layout builder is in one the master and the encoder is on the slave(in MPM for SD and HD720 asymmetric), we create instead of the current message 3 new messages:
			 * 1. TB_MSG_OPEN_PORT_REQ to the master Encoder with the Master's physical info and with nVideoEncoderType = E_VIDEO_ENCODER_MASTER_LB_ONLY
			 * 2. TB_MSG_OPEN_PORT_REQ to the slave  Encoder with the Slaves's physical info and with nVideoEncoderType = E_VIDEO_ENCODER_SLAVE_FULL_ENCODER
			 * 3. TB_MSG_CONNECT_REQ to connect the master's encoder to the slave'sencoder
	*/
	PTRACE(eLevelInfoNormal,"CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForLBandFullEnc");

	const ENCODER_PARAM_S *pEncoderParamsStruct = (const ENCODER_PARAM_S *)mplPrtcl->getpData();
	APIU32 connectionId = 0xffffffff;
	connectionId = mplPrtcl->getPortDescriptionHeaderConnection_id();
	ConnToCardTableEntry 	masterConnToCardTableEntry;
	CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
	STATUS status = pSharedMemMap->Get(connectionId, masterConnToCardTableEntry);
	ConnToCardTableEntry* pSlaveConnToCardTableEntry = NULL;
	pSlaveConnToCardTableEntry = GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(masterConnToCardTableEntry.rsrc_conf_id,masterConnToCardTableEntry.rsrc_party_id,E_VIDEO_SLAVE_FULL_ENCODER);
	if(	pSlaveConnToCardTableEntry)
	{
	  masterConnToCardTableEntry.DumpRaw("CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForLBandFullEnc (Master)");
	  pSlaveConnToCardTableEntry->DumpRaw("CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForLBandFullEnc (Slave)");
		CreateAndSendTBOpenPortReq(pEncoderParamsStruct,E_VIDEO_ENCODER_MASTER_LB_ONLY,masterConnToCardTableEntry);
		CreateAndSendTBOpenPortReq(pEncoderParamsStruct,E_VIDEO_ENCODER_SLAVE_FULL_ENCODER,*pSlaveConnToCardTableEntry);
		CreateAndSendTBConnentReq(pSlaveConnToCardTableEntry->m_id, pSlaveConnToCardTableEntry->rsrc_party_id,masterConnToCardTableEntry.m_id,masterConnToCardTableEntry.rsrc_party_id,masterConnToCardTableEntry);
	}

}
//////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForSplitEnc(CMplMcmsProtocol *mplPrtcl, ECntrlType videoEncoderMasterCntlType)
{
	/*Incase of Open Video Encoder of Master Split Encoder topology(In MPM+ for HD1080 calls), we create instead of the current message 3 new messages:
			 * 1. TB_MSG_OPEN_PORT_REQ to the master Encoder with the Master's physical info and with nVideoEncoderType = E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER
			 * 2. TB_MSG_OPEN_PORT_REQ to the slave Encoder with the Slaves's physical info and with nVideoEncoderType = E_VIDEO_ENCODER_SLAVE_SPLIT_ENCODER
			 * 3. TB_MSG_CONNECT_REQ to connect the master's encoder to the slave's encoder - CM will create from this request two streams for both directions
	*/
	PTRACE(eLevelInfoNormal,"CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForSplitEnc");

	const ENCODER_PARAM_S *pEncoderParamsStruct = (const ENCODER_PARAM_S *)mplPrtcl->getpData();
	APIU32 connectionId = 0xffffffff;
	connectionId = mplPrtcl->getPortDescriptionHeaderConnection_id();
	ConnToCardTableEntry 	masterConnToCardTableEntry;
	CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
	STATUS status = pSharedMemMap->Get(connectionId, masterConnToCardTableEntry);
	ECntrlType videoEncoderSlaveCntlType = GetSlaveCntlTypeFromMasterCntlType(videoEncoderMasterCntlType);
	ConnToCardTableEntry* pSlaveConnToCardTableEntry = NULL;
	pSlaveConnToCardTableEntry = GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(masterConnToCardTableEntry.rsrc_conf_id,masterConnToCardTableEntry.rsrc_party_id,videoEncoderSlaveCntlType);

	if(	pSlaveConnToCardTableEntry )
	{
		EVideoEncoderType masterVideoEncoderApiType = TranslateVideoEncoderCntlTypeToApi(videoEncoderMasterCntlType);
		EVideoEncoderType slaveVideoEncoderApiType = TranslateVideoEncoderCntlTypeToApi(videoEncoderSlaveCntlType);

	  masterConnToCardTableEntry.DumpRaw("CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForSplitEnc (Master)");
	  pSlaveConnToCardTableEntry->DumpRaw("CMplApiDispatcherTask::SendOpenMasterSlaveVideoEncoderMsgsForSplitEnc (Slave)");

	  
	  DWORD reqId = mplPrtcl->getMsgDescriptionHeaderRequest_id();
	  DWORD req_num = CreateAndSendTBOpenPortReq(pEncoderParamsStruct,masterVideoEncoderApiType,masterConnToCardTableEntry);

		SetMasterSlaveRequestHandlerSubIdData(TB_MSG_OPEN_PORT_REQ, reqId, req_num);

		bool update_seq_num = false;
		//		if(eSystemCardsMode_breeze == GetSystemCardsBasedMode()){ // 1080_60
		//		  update_seq_num = true;
		//		}

	        req_num = CreateAndSendTBOpenPortReq(pEncoderParamsStruct,slaveVideoEncoderApiType,*pSlaveConnToCardTableEntry);
		SetMasterSlaveRequestHandlerSubIdData(TB_MSG_OPEN_PORT_REQ, reqId, req_num);
		CreateAndSendTBConnentReq(masterConnToCardTableEntry.m_id,masterConnToCardTableEntry.rsrc_party_id, pSlaveConnToCardTableEntry->m_id, pSlaveConnToCardTableEntry->rsrc_party_id,*pSlaveConnToCardTableEntry);//The order of connection is important to distinguish between cases when we get the ack indication
	}
}
//////////////////////////////////////////////////////////////////////////////
DWORD CMplApiDispatcherTask::CreateAndSendTBOpenPortReq(const ENCODER_PARAM_S *pEncoderStruct, EVideoEncoderType rsrcCntlType,ConnToCardTableEntry &pConnToCardTableEntry, bool forceSeqNum, DWORD seqNum )
{
    ENCODER_PARAM_S nEncoderStruct   = *pEncoderStruct;
    nEncoderStruct.nVideoEncoderType = rsrcCntlType;

    CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
    pMplMcmsProtocol->AddCommonHeader(TB_MSG_OPEN_PORT_REQ);
    pMplMcmsProtocol->AddMessageDescriptionHeader();
    pMplMcmsProtocol->AddPortDescriptionHeader(pConnToCardTableEntry.rsrc_party_id, pConnToCardTableEntry.rsrc_conf_id, pConnToCardTableEntry.m_id, pConnToCardTableEntry.rsrcType);
    pMplMcmsProtocol->AddData(sizeof(ENCODER_PARAM_S), (const char*)(&nEncoderStruct));
    pMplMcmsProtocol->GetPhysicalInfoHeaderConnToCardTable(&pConnToCardTableEntry);
    pMplMcmsProtocol->UpdateCommonHeaderToMPL();
    pMplMcmsProtocol->AddPayload_len(MPL_API_TYPE);

    if(forceSeqNum){
      PTRACE2INT(eLevelInfoNormal,"1080_60: CMplApiDispatcherTask::CreateAndSendTBOpenPortReq: updating seq_num to ",seqNum);
      pMplMcmsProtocol->UpdateCommonHeaderToMPL(seqNum);
    }

    int boardId = pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id();
    SendToCard(boardId, pMplMcmsProtocol);

    DWORD seq_num = (DWORD)(-1);
    seq_num = pMplMcmsProtocol->getCommonHeaderSequence_num();

    DWORD req_num = pMplMcmsProtocol->getMsgDescriptionHeaderRequest_id();

    POBJDELETE(pMplMcmsProtocol);

    return req_num;
}
//////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::CreateAndSendTBConnentReq(APIU32 connectionID1,APIU32 partyID1, APIU32 connectionID2,APIU32 partyID2,ConnToCardTableEntry& pConnToCardTableEntry)
{
	TB_MSG_CONNECT_S tConnecttStruct;
	memset(&tConnecttStruct,0,sizeof(TB_MSG_CONNECT_S));

	tConnecttStruct.physical_port1.connection_id = connectionID1;
	tConnecttStruct.physical_port1.party_id= partyID1;
	tConnecttStruct.physical_port2.connection_id = connectionID2;
	tConnecttStruct.physical_port2.party_id= partyID2;

	CMplMcmsProtocol *pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->AddCommonHeader(TB_MSG_CONNECT_REQ);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPortDescriptionHeader(pConnToCardTableEntry.rsrc_party_id,
											   pConnToCardTableEntry.rsrc_conf_id,
											   pConnToCardTableEntry.m_id,
											   pConnToCardTableEntry.rsrcType);
	pMplMcmsProtocol->AddData(sizeof(TB_MSG_CONNECT_S), (const char*)(&tConnecttStruct));

	pMplMcmsProtocol->GetPhysicalInfoHeaderConnToCardTable(&pConnToCardTableEntry);

	OPCODE opcodeBefore = pMplMcmsProtocol->getCommonHeaderOpcode();
	CMplApiSpecialCommandHandler handler(*pMplMcmsProtocol);
	STATUS status = handler.HandeSpecialCommand();
	if(STATUS_OK != status)
	{
		m_pProcess->OnSpecialCommanderFailure(*pMplMcmsProtocol, opcodeBefore, status);
		return;
	}
	pMplMcmsProtocol->UpdateCommonHeaderToMPL();
	pMplMcmsProtocol->AddPayload_len(MPL_API_TYPE);

	int boardId = pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id();
	SendToCard(boardId, pMplMcmsProtocol);


	POBJDELETE(pMplMcmsProtocol);

}
//////////////////////////////////////////////////////////////////////////////
BOOL CMplApiDispatcherTask::IsVideoEncoderMessage(const CMplMcmsProtocol *mplMcmsPrtcl)const
{
	BOOL ans = FALSE;
	BYTE logicRsrcType = mplMcmsPrtcl->getPortDescriptionHeaderLogical_resource_type_1();
	if(GetGenericEncoderDecoderRsrcType(logicRsrcType) == eLogical_video_encoder)
		ans = TRUE;
	return ans;
}
//////////////////////////////////////////////////////////////////////////////
BYTE CMplApiDispatcherTask::GetGenericEncoderDecoderRsrcType(BYTE logicRsrcType)const
{
	BYTE retType = logicRsrcType;

	if((logicRsrcType==eLogical_COP_HD1080_encoder)
		|| (logicRsrcType==eLogical_COP_HD720_encoder)
		|| (logicRsrcType==eLogical_COP_CIF_encoder)
		|| (logicRsrcType==eLogical_COP_4CIF_encoder)
		|| (logicRsrcType==eLogical_COP_PCM_encoder)
		|| (logicRsrcType==eLogical_COP_VSW_encoder)
		|| (logicRsrcType==eLogical_COP_dummy_encoder)
		|| (logicRsrcType==eLogical_VSW_dummy_encoder)
		|| (logicRsrcType==eLogical_video_encoder_content))
		retType = eLogical_video_encoder;
	else if ((logicRsrcType==eLogical_COP_Dynamic_decoder)
    		|| (logicRsrcType==eLogical_COP_VSW_decoder)
    		|| (logicRsrcType==eLogical_VSW_dummy_decoder))
		retType = eLogical_video_decoder;

	return retType;
}
//////////////////////////////////////////////////////////////////////////////
BOOL CMplApiDispatcherTask::IsAudioCntrlOpcode(const CMplMcmsProtocol *mplMcmsPrtcl)const
{
    OPCODE opcode = mplMcmsPrtcl->getCommonHeaderOpcode();
    switch(opcode)
    {
        case AC_OPEN_CONF_REQ:
        case AC_CLOSE_CONF_REQ:
        case AC_UPDATE_CONF_PARAMS_REQ:
        case AC_LAYOUT_CHANGE_COMPLETE_REQ:
        case AC_OPEN_CONF_RESEND:
            return TRUE;
            break;

        default:
            return FALSE;
            break;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::UpdateVideoEncoderType(CMplMcmsProtocol *mplPrtcl, EVideoEncoderType encoderType)
{
    ENCODER_PARAM_S tEncoderStruct   = *(const ENCODER_PARAM_S*)mplPrtcl->getpData();
    tEncoderStruct.nVideoEncoderType = encoderType;
    mplPrtcl->AddData(sizeof(ENCODER_PARAM_S), (const char*)(&tEncoderStruct));
}

//////////////////////////////////////////////////////////////////////////////////////////////
ConnToCardTableEntry* CMplApiDispatcherTask::GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(DWORD conf_id,DWORD party_id, ECntrlType rsrcCntlType)
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = m_pProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries &&
		passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
			if ((pSharedMemoryMap->m_pEntries[i].rsrc_conf_id 	== conf_id)	&&
				(pSharedMemoryMap->m_pEntries[i].rsrc_party_id 	== party_id)&&
				(pSharedMemoryMap->m_pEntries[i].rsrcCntlType == rsrcCntlType))
				{
				  pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiDispatcherTask::GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType");
					return &(pSharedMemoryMap->m_pEntries[i]);
				}
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiDispatcherTask::GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType"," - enty not found");
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::SendConnectMasterSlaveVideoEncoderMsgForLBandFullEnc(CMplMcmsProtocol *mplPrtcl)
{
	/*Incase of Connect message to connect Video Encoder with ART in MASTER-SLAVE topology in MPM(for SD and HD720 asymmetric)
	 * We will send a Connect message to connect the slave Encoder to ART
	*/
	PTRACE(eLevelInfoNormal,"CMplApiDispatcherTask::SendConnectMasterSlaveVideoEncoderMsgForLBandFullEnc");

	TB_MSG_CONNECT_S* tbMsgConnect = (TB_MSG_CONNECT_S *) mplPrtcl->getpData();
	APIU32 ConnectionId1 = tbMsgConnect->physical_port1.connection_id; //the art is the first physical port in the connect(art,encoder) message
	APIU32 PartyId1 = tbMsgConnect->physical_port1.party_id;

	APIU32 ConnectionId2 = tbMsgConnect->physical_port2.connection_id; //the encoder is the second physical port in the connect(art,encoder) message
	ConnToCardTableEntry masterConnToCardTableEntry;
	if(ConnectionId2 != 0xffffffff)
	{
		CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
		STATUS status = pSharedMemMap->Get(ConnectionId2,masterConnToCardTableEntry);
		if (status == STATUS_OK)
		{
			ConnToCardTableEntry* pSlaveConnToCardTableEntry = NULL;
			pSlaveConnToCardTableEntry = GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(masterConnToCardTableEntry.rsrc_conf_id,masterConnToCardTableEntry.rsrc_party_id,E_VIDEO_SLAVE_FULL_ENCODER);
			if(	pSlaveConnToCardTableEntry)
			{
				CreateAndSendTBConnentReq(ConnectionId1,PartyId1, pSlaveConnToCardTableEntry->m_id,pSlaveConnToCardTableEntry->rsrc_party_id, *pSlaveConnToCardTableEntry);
			}
		}
		else
		{
			PTRACE(eLevelError,"CMplApiDispatcherTask::SendConnectMasterSlaveVideoEncoderMsgForLBandFullEnc didn't find connect to card table entry");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::SendCloseMasterSlaveVideoEncoderMsgsForLBandFullEnc(CMplMcmsProtocol *mplPrtcl)
{
	/*Incase of Close Master-Slave Video Encoder we create instead of the current message 2 new messages:
			 * 1. TB_MSG_CLOSE_PORT_REQ to the master Encoder with the Master's physical info
			 * 2. TB_MSG_CLOSE_PORT_REQ to the slave  Encoder with the Slaves's physical info
	*/
	PTRACE(eLevelError,"CMplApiDispatcherTask::SendCloseMasterSlaveVideoEncoderMsgsForLBandFullEnc");
	ConnToCardTableEntry 	masterConnToCardTableEntry;
	APIU32               	masterConnectionId = 0xffffffff;
	masterConnectionId = mplPrtcl->getPortDescriptionHeaderConnection_id();
	if(masterConnectionId!=0xffffffff)
	{
		CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
		STATUS con2CardStatus = pSharedMemMap->Get(masterConnectionId,masterConnToCardTableEntry);
		if (con2CardStatus == STATUS_OK)
		{
			ConnToCardTableEntry* pSlaveConnToCardTableEntry = NULL;
			pSlaveConnToCardTableEntry = GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(masterConnToCardTableEntry.rsrc_conf_id,masterConnToCardTableEntry.rsrc_party_id,E_VIDEO_SLAVE_FULL_ENCODER);
			if(	pSlaveConnToCardTableEntry)
			{
				CreateAndSendTBClosePortReq(masterConnToCardTableEntry);
				CreateAndSendTBClosePortReq(*pSlaveConnToCardTableEntry);
			}
		}
		else
		{
			PTRACE(eLevelError,"CMplApiDispatcherTask::SendCloseMasterSlaveVideoEncoderMsgsForLBandFullEnc didn't find connect to card table entry");
		}
	}
}
//////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::SendCloseMasterSlaveVideoEncoderMsgsForSplitEnc(CMplMcmsProtocol *mplPrtcl, ECntrlType masterEncoderCntlType)
{
	/*Incase of Close Master Video Encoder with Split Encoder topology(In MPM+ for HD1080 calls, and for HD720 symmetric calls from 03.09)
	 * we send only close port to the master:
	 * This function is in the use of simulation only!!!
			 * 1. TB_MSG_CLOSE_PORT_REQ to the master Encoder with the Master's physical info
			 * 2. TB_MSG_CLOSE_PORT_REQ to the  slave Encoder with the Slaves's physical info
*/
	PTRACE(eLevelError,"CMplApiDispatcherTask::SendCloseMasterSlaveVideoEncoderMsgsForSplitEnc");
	ConnToCardTableEntry 	masterConnToCardTableEntry;
	APIU32               	masterConnectionId = 0xffffffff;
	masterConnectionId = mplPrtcl->getPortDescriptionHeaderConnection_id();
	if(masterConnectionId!=0xffffffff)
	{
		CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
		STATUS con2CardStatus = pSharedMemMap->Get(masterConnectionId,masterConnToCardTableEntry);
		if (con2CardStatus == STATUS_OK)
		{
			ECntrlType slaveEncoderCntlType =  GetSlaveCntlTypeFromMasterCntlType(masterEncoderCntlType);
			ConnToCardTableEntry* pSlaveConnToCardTableEntry = NULL;
			pSlaveConnToCardTableEntry = GetPhysicalInfoHeaderByConfPartyIDAndRsrcCntlType(masterConnToCardTableEntry.rsrc_conf_id,masterConnToCardTableEntry.rsrc_party_id,slaveEncoderCntlType);

			if(	pSlaveConnToCardTableEntry )
			{
				CreateAndSendTBClosePortReq(masterConnToCardTableEntry);
				CreateAndSendTBClosePortReq(*pSlaveConnToCardTableEntry);
			}
		}
		else
		{
			PTRACE(eLevelError,"CMplApiDispatcherTask::SendCloseMasterSlaveVideoEncoderMsgsForSplitEnc didn't find connect to card table entry");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
void CMplApiDispatcherTask::CreateAndSendTBClosePortReq(ConnToCardTableEntry& pConnToCardTableEntry)
{

  PTRACE(eLevelError,"CMplApiDispatcherTask::CreateAndSendTBClosePortReq");
	CMplMcmsProtocol *pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->AddCommonHeader(TB_MSG_CLOSE_PORT_REQ);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPortDescriptionHeader(pConnToCardTableEntry.rsrc_party_id,
											   pConnToCardTableEntry.rsrc_conf_id,
											   pConnToCardTableEntry.m_id,
											   pConnToCardTableEntry.rsrcType);
	pMplMcmsProtocol->GetPhysicalInfoHeaderConnToCardTable(&pConnToCardTableEntry);
	pMplMcmsProtocol->UpdateCommonHeaderToMPL();
	pMplMcmsProtocol->AddPayload_len(MPL_API_TYPE);
	//CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("*CMplApiDispatcherTask::CreateAndSendTBClosePortReq*");

	int boardId = pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id();
	SendToCard(boardId, pMplMcmsProtocol);

	POBJDELETE(pMplMcmsProtocol);
}
//////////////////////////////////////////////////////////////////////////////
EVideoEncoderType CMplApiDispatcherTask::TranslateVideoEncoderCntlTypeToApi(ECntrlType eVideoEncoderCntlType)
{
	switch(eVideoEncoderCntlType)
	{
		case E_NORMAL:
		{
			return E_VIDEO_ENCODER_NORMAL;
		}
		case E_AC_MASTER:
		case E_AC_SLAVE:
		{
			PTRACE(eLevelError,"CMplApiDispatcherTask::TranslateVideoEncoderCntlTypeToApi the ECntrlType isn't video encoder type");
			PASSERTMSG(1, "Illegal video encoder resource cntl type");
			return E_VIDEO_ENCODER_NORMAL;
		}
		case E_VIDEO_MASTER_LB_ONLY:
		{
			return E_VIDEO_ENCODER_MASTER_LB_ONLY;
		}
		case E_VIDEO_SLAVE_FULL_ENCODER:
		{
			return E_VIDEO_ENCODER_SLAVE_FULL_ENCODER;
		}
		case E_VIDEO_MASTER_SPLIT_ENCODER:
		{
			return E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER;
		}
		case E_VIDEO_SLAVE_SPLIT_ENCODER:
		{
			return E_VIDEO_ENCODER_SLAVE_SPLIT_ENCODER;
		}
		case E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP:
		{
			return E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER_HALF_DSP;
		}
		case E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP:
		{
			return E_VIDEO_ENCODER_SLAVE_SPLIT_ENCODER_HALF_DSP;
		}
		case E_ENCODER_PCM:
		{
			return E_VIDEO_ENCODER_PCM;
		}
		default:
		{
			PTRACE(eLevelError,"CMplApiDispatcherTask::TranslateVideoEncoderCntlTypeToApi the ECntrlType isn't valid");
			PASSERTMSG(1, "Illegal video encoder resource cntl type");
			return E_VIDEO_ENCODER_NORMAL;
		}
	}

	return E_VIDEO_ENCODER_NORMAL;
}
//////////////////////////////////////////////////////////////////////////////
ECntrlType CMplApiDispatcherTask::GetSlaveCntlTypeFromMasterCntlType(ECntrlType eMasterCntlType)
{
	switch(eMasterCntlType)
	{
		case E_NORMAL:
		case E_AC_SLAVE:
		case E_VIDEO_SLAVE_FULL_ENCODER:
		case E_VIDEO_SLAVE_SPLIT_ENCODER:
		case E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP:
		{
			PTRACE(eLevelError,"CMplApiDispatcherTask::GetSlaveCntlTypeFromMasterCntlType the ECntrlType isn't a master type");
			PASSERTMSG(1, "Illegal master cntl type");
			return eMasterCntlType;
		}
		case E_AC_MASTER:
		{
			return E_AC_SLAVE;
		}
		case E_VIDEO_MASTER_LB_ONLY:
		{
			return E_VIDEO_SLAVE_FULL_ENCODER;
		}
		case E_VIDEO_MASTER_SPLIT_ENCODER:
		{
			return E_VIDEO_SLAVE_SPLIT_ENCODER;
		}
		case E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP:
		{
			return E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP;
		}
		default:
		{
			PTRACE(eLevelError,"CMplApiDispatcherTask::GetSlaveCntlTypeFromMasterCntlType the ECntrlType isn't valid");
			PASSERTMSG(1, "Illegal cntl type");
			return eMasterCntlType;
		}
	}

	return eMasterCntlType;
}

//==========================================================================================================================================================//
CMplApiMasterSlaveReqAsyncHandler* CMplApiDispatcherTask::GetMasterSlaveReqHandler()
{
  CMplApiMasterSlaveReqAsyncHandler* pMplApiMasterSlaveReqAsyncHandler=NULL;
  if(m_pProcess){
    pMplApiMasterSlaveReqAsyncHandler = m_pProcess->getMasterSlaveReqHandler();
  }
  return pMplApiMasterSlaveReqAsyncHandler;
}
//==========================================================================================================================================================//
void   CMplApiDispatcherTask::onCleanMasterSlaveRequestTout(CSegment* pMsg)
{
  PTRACE(eLevelInfoNormal,"CMplApiDispatcherTask::onCleanMasterSlaveRequestTout");
  if(m_pProcess){
    m_pProcess->getMasterSlaveReqHandler()->onCleanMasterSlaveRequestTout();
  }
  StartTimer(CLEAN_MASTER_SLAVE_REQUESTS_TOUT,20*SECOND);
}
//==========================================================================================================================================================//
void   CMplApiDispatcherTask::SetMasterSlaveRequestHandlerSubIdData(DWORD opcode, DWORD reqId, DWORD subReqId)
{
  CMplApiMasterSlaveReqAsyncHandler* pMplApiMasterSlaveReqAsyncHandler = GetMasterSlaveReqHandler();
  if(pMplApiMasterSlaveReqAsyncHandler){
    if(FALSE == pMplApiMasterSlaveReqAsyncHandler->IsInUse()){ // start timer on first use
      PTRACE(eLevelInfoNormal,"CMplApiDispatcherTask::SetMasterSlaveRequestHandlerSubIdData - start timer on first use for 5 seconds");
      StartTimer(CLEAN_MASTER_SLAVE_REQUESTS_TOUT,20*SECOND);
    }
    pMplApiMasterSlaveReqAsyncHandler->setMasterSlaveRequestHandlerSubIdData(opcode, reqId,subReqId);
  }
}
//==========================================================================================================================================================//
const ConnToCardTableEntry* CMplApiDispatcherTask::GetPhysicalInfoHeaderByConfIdPartyIdAndLogicalRsrcType(DWORD confId, DWORD partyId, eLogicalResourceTypes logicalResourceType) // eLogical_relay_audio_encoder
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	CSharedMemMap *pSharedMemoryMap = m_pProcess->GetSharedMemoryMap();
	while (i < pSharedMemoryMap->m_pHeader->m_maxEntries && passedEntries < pSharedMemoryMap->m_pHeader->m_numEntries)
	{
		if (pSharedMemoryMap->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			passedEntries++;
			const ConnToCardTableEntry& currentEntry = pSharedMemoryMap->m_pEntries[i];

			if ((currentEntry.rsrc_conf_id== confId )&& (currentEntry.rsrcType== logicalResourceType) && partyId == currentEntry. rsrc_party_id  )
			{
				pSharedMemoryMap->m_pEntries[i].DumpRaw("CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByLogicalRsrcType");
				return &(pSharedMemoryMap->m_pEntries[i]);
			}
		}
		i++;
	}
	PTRACE2COND(SHARED_MEMORY_DEBUG_PRINTS,eLevelInfoNormal,"CMplApiSpecialCommandHandler::GetPhysicalInfoHeaderByConfAndLogicalRsrcType"," - enty not found");
	return NULL;
}
//==========================================================================================================================================================//
bool CMplApiDispatcherTask::IsAudioRelayDecoderOnRmx(CMplMcmsProtocol *pMplMcmsProtocol)
{
	DWORD confId    = pMplMcmsProtocol->getPortDescriptionHeaderConf_id();
	DWORD partyId   = pMplMcmsProtocol->getPortDescriptionHeaderParty_id();

	TRACEINTO << "PartyId:" << partyId << ", ConfId:" << confId;
	ConnToCardTableEntry connToCardTableEntry;
	DWORD connectionId = pMplMcmsProtocol->getPortDescriptionHeaderConnection_id();
	CSharedMemMap *pSharedMemMap = m_pProcess->GetSharedMemoryMap();
	STATUS con2CardStatus = pSharedMemMap->Get(connectionId,connToCardTableEntry);
//	const ConnToCardTableEntry* pConnToCardTableEntry = GetPhysicalInfoHeaderByConfIdPartyIdAndLogicalRsrcType(confId,partyId,eLogical_relay_audio_decoder);
	if(STATUS_OK == con2CardStatus)
	{
		if((connToCardTableEntry.rsrcType == eLogical_relay_audio_decoder) && (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily()))//==GetProductFamily
		{
			TRACEINTO << "PartyId:" << partyId << ", ConfId:" << confId << "TRUE";
			return true;
		}
	}
	return false;
}
//==========================================================================================================================================================//
void CMplApiDispatcherTask::SendUpdateMuteToMrmpAndMpl(CMplMcmsProtocol *pMplMcmsProtocol)
{
	if(!pMplMcmsProtocol)
	{
		return;
	}
	DWORD confId    = pMplMcmsProtocol->getPortDescriptionHeaderConf_id();
	DWORD partyId   = pMplMcmsProtocol->getPortDescriptionHeaderParty_id();
	CMplMcmsProtocol mplPrtcl(*pMplMcmsProtocol);
	int boardId = pMplMcmsProtocol->getPhysicalInfoHeaderBoard_id();
	mplPrtcl.GetPhysicalHeader().resource_type = ePhysical_mrmp;
	TRACEINTO << "PartyId:" << partyId << ", ConfId:" << confId << "resource_type: " << mplPrtcl.GetPhysicalHeader().resource_type;
    SendToCard(boardId, pMplMcmsProtocol);
    SendToCard(boardId, &mplPrtcl);
}
