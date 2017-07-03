//+========================================================================+
//                       ConfPartyDispatcherTask.cpp                       |
//                     Copyright 2005 Polycom                              |
//                      All Rights Reserved.                               |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ConfPartyDispatcherTask.cpp                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Talya                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Talya| 1.5.05    | Conference/Party Process Dispatcher in Carmel        |
//+========================================================================+


#include "ConfPartyDispatcherTask.h"
#include "ProcessBase.h"
#include "SystemFunctions.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Segment.h"
#include "Trace.h"
#include "MplMcmsProtocol.h"
#include "MplMcmsStructs.h"
#include "TaskApi.h"
#include "ConfPartyDefines.h"
#include "ConfPartyRoutingTable.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyProcess.h"
#include "SipUtils.h"
#include "IpCsOpcodes.h"
#include "CommConfDB.h"
#include "ConfApi.h"
#include "H323CsInd.h"
#include "IpMfaOpcodes.h"
#include "ConfPartyMplMcmsProtocolTracer.h"
#include "MplMcmsProtocolTracer.h"
#include "HostCommonDefinitions.h"
#include "OpcodesMcmsNetQ931.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "ManagerApi.h"
#include "OpcodesMcmsPCM.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
extern const CLobbyApi*   GetpLobbyApi() ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CConfPartyDispatcherTask::CConfPartyDispatcherTask(BOOL isSync )
        :CDispatcherTask(isSync)
{
    m_Thread_Group = eTaskGroupRegular;
}

/////////////////////////////////////////////////////////////////////
CConfPartyDispatcherTask::~CConfPartyDispatcherTask()
{

}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::HandleOtherIdTypes(CMessageHeader & header)
{
	switch ( header.m_addressee.m_idType )
	{
		case ePartyId:
		{
			//PartyMsg (via conf)
			CSegment* pMsg1 = new CSegment;
			*pMsg1 << (DWORD)  header.m_addressee.m_id;
			*pMsg1 << (OPCODE) header.m_opcode;
			if(header.m_bufferLen)
				*pMsg1 << *(header.m_segment);

			CTaskApi* pTaskApi = NULL;
			pTaskApi = (CTaskApi*) (::GetpConfPartyRoutingTable()->GetRsrcMngrPtrByPartyId(header.m_addressee.m_id));

			if (CPObject::IsValidPObjectPtr(pTaskApi))
			{
				pTaskApi->SendMsg(pMsg1, EXTERNAL_PARTY_MSG);
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::HandleOtherIdTypes : EVENT IS DISCARDED \n");
				POBJDELETE(pMsg1);
			}

			break;
		}
		default:
		{
			PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::HandleOtherIdTypes : invalid m_idType");
			break;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
//ALL SPECIFIC HANDLING OF EVENTS IN CONF/PARTY PROCESS SHOULD BE ADDED IN THIS FUNCTION
BOOL CConfPartyDispatcherTask::TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode)
{
	void* ptr = NULL;
	switch ( opCode )
	{
	case MPLAPI_MSG:
		{
			HandleMPLEvent(pMsg);
			break;
		}
	case CSAPI_MSG:
		{
			HandleCsEvent(pMsg);
			break;
		}
	case SIGNALING_MANAGER_MSG:
		{
			HandleSignalingManagerEvent(pMsg);
			break;
		}
	default:
		{
			PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::HandleEvent : invalid opcode");
			return FALSE;
			break;
		}
	}
	return TRUE;

}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::HandleMPLEvent(CSegment *pMPLMsgSeg)
{
	CMplMcmsProtocol* pMplprtcl = new CMplMcmsProtocol;
	pMplprtcl->DeSerialize(*pMPLMsgSeg);
	CConfPartyMplMcmsProtocolTracer(*pMplprtcl).TraceMplMcmsProtocol("***CConfPartyDispatcherTask::HandleMPLEvent");

	ConnectionID ConnId = pMplprtcl->getPortDescriptionHeaderConnection_id();
	PartyRsrcID PartyId = pMplprtcl->getPortDescriptionHeaderParty_id();
	ConfRsrcID ConfId = pMplprtcl->getPortDescriptionHeaderConf_id();
	eLogicalResourceTypes lrt = (eLogicalResourceTypes)pMplprtcl->getPortDescriptionHeaderLogical_resource_type_1();
	OPCODE opcode = pMplprtcl->getOpcode();

	CRsrcParams* pKeyForLookup = new CRsrcParams(ConnId, PartyId, ConfId, lrt);

	DWORD encoderConnectionID;
	if (lrt == eLogical_relay_avc_to_svc_video_encoder_1 || lrt == eLogical_relay_avc_to_svc_video_encoder_2)
	{
		if (opcode == ACK_IND)
		{
			encoderConnectionID = (DWORD)ConnId;

			PTRACE2INT(eLevelInfoNormal, "CConfPartyDispatcherTask::HandleMPLEvent - Encoder-ConnId: ", encoderConnectionID);
		}
	}

	eDispatchingMethods dispatchingMethod = GetSuitedDispatchingMethod(pKeyForLookup);

	CSegment* pMsg = new CSegment;
	if(pMplprtcl->getDataLen())
	{

		if (opcode == ACK_IND)
		{
	        ACK_IND_S* ackIndStruct = (ACK_IND_S*)pMplprtcl->GetData();
	        if (ackIndStruct && ackIndStruct->ack_base.ack_opcode != IVR_PLAY_MESSAGE_REQ)
	        {
                ackIndStruct->ack_base.ack_seq_num = pMplprtcl->getMsgDescriptionHeaderRequest_id();
                TRACEINTO << "Override the seq_num with request ID=" << ackIndStruct->ack_base.ack_seq_num << " opcode=" << ackIndStruct->ack_base.ack_opcode;
	        }
			pMsg->Put((unsigned char*)pMplprtcl->GetData(),sizeof(ACK_IND_S)/*pMplprtcl->getDataLen()*/);
//		 pMsg->Put((unsigned char*)pMplprtcl->GetData(),pMplprtcl->getDataLen());
		}
		else
		{
			pMsg->Put((unsigned char*)pMplprtcl->GetData(),pMplprtcl->getDataLen());
//			pMsg->Put((unsigned char*)pMplprtcl->GetData(),sizeof(ACK_IND_S))/*pMplprtcl->getDataLen()*/);
		}


//		pMsg->Put((unsigned char*)pMplprtcl->GetData(),pMplprtcl->getDataLen());
	}

	if (opcode == ACK_IND)
	{
		TRACEINTO<<"!@# ConnId:"<<ConnId<<" dispatchingMethod:"<<(int)dispatchingMethod<<" sizeof(ACK_IND_S):"<<(int)sizeof(ACK_IND_S)<<" pMplprtcl->getDataLen():"<<pMplprtcl->getDataLen();
		
		ACK_IND_S* ackIndStruct = (ACK_IND_S*)pMplprtcl->GetData();
		APIU32	ack_opcode = ackIndStruct->ack_base.ack_opcode;
		if (ack_opcode != MOVE_PARTY_RESOURCE_REQ)
			*pMsg << ConnId;
	}

	switch(dispatchingMethod){
	case	eDispatching_By_ConnectionId:{
		DispatchMplEventByPartyIdAndConnectionId(opcode, pKeyForLookup, pMsg);
		break;
										 }
	case	eDispatching_By_PartyId:{
		DispatchMplEventByPartyId(opcode, pKeyForLookup, pMsg);
		break;
									}
	case	eDispatching_By_ConfId:{
		DispatchMplEventByConfId(opcode, pKeyForLookup, pMsg);
		break;
								   }
	case	eDispatching_Lobby :{

	        //Add the relevant fields in PSTN dial-in calls
	        if ( NET_SETUP_IND == opcode || NET_DISCONNECT_IND == opcode )
		  {
		    (*pMsg) << ((pMplprtcl->GetPhysicalHeaderConst()).box_id) ;
		    (*pMsg) << ((pMplprtcl->GetPhysicalHeaderConst()).board_id ) ;
		    (*pMsg) << ((pMplprtcl->GetPhysicalHeaderConst()).sub_board_id) ;
		  }

	        DispatchToLobby(opcode, pMsg);
		break;
								   }
	default:
		{
			if(opcode != ICE_ERR_IND && opcode != PCM_COMMAND) //PCM_COMMAND: no need to handle register request on RMX.  
			{
				PASSERTMSG(opcode,"(error=opcode) No Relevant Dispatching Method");
				//PASSERTMSG(opcode,"Opcode");
				//PASSERTMSG(1,"No Relevant Dispatching Method");
			}

			POBJDELETE(pMsg);
		}
	}
	POBJDELETE(pMplprtcl);
	POBJDELETE(pKeyForLookup);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::DispatchMplEventByPartyIdAndConnectionId(OPCODE opcode, CRsrcParams *pKeyForLookup, CSegment* pMsg) const
{
	PASSERTMSG_AND_RETURN(!::GetpConfPartyRoutingTable(), "pConfPartyRoutingTable is NULL");

	CTaskApi* pTaskApi = (CTaskApi*)::GetpConfPartyRoutingTable()->GetRsrcMngrPtrByPartyIdAndConnectionId(pKeyForLookup->GetPartyRsrcId(), pKeyForLookup->GetConnectionId());
	if (CPObject::IsValidPObjectPtr(pTaskApi))
	{
		pTaskApi->SendMsgWithStateMachine(pMsg, opcode);
	}
	else
	{
		TRACEINTO << "PartyId:" << pKeyForLookup->GetPartyRsrcId() << ", ConnectionId:" << pKeyForLookup->GetConnectionId() << " - EVENT IS DISCARDED";
		POBJDELETE(pMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::DispatchMplEventByPartyId(OPCODE opcode, CRsrcParams* pKeyForLookup, CSegment* pMsg) const
{
	//PartyCntlMsg (via conf)
	TRACEINTO<<"!@# 2";
	CSegment* pMsg1 = new CSegment;
	*pMsg1<< pKeyForLookup->GetPartyRsrcId();
	*pMsg1<< opcode;

	//Special Treatment for MOVE_PARTY_RESOURCE_REQ Acknowledge - add the LRT in the end of content
	if(ACK_IND == opcode)
	{
		OPCODE ackOpcode;
		*pMsg >> ackOpcode;
		if(MOVE_PARTY_RESOURCE_REQ == ackOpcode)
		{
			*pMsg << (BYTE)pKeyForLookup->GetLogicalRsrcType();
		}
	}

	opcode = PARTY_RSRC_ID_MSG;
	*pMsg1 << *pMsg;//content
	*pMsg =  *pMsg1;
	POBJDELETE(pMsg1);

	CTaskApi* pTaskApi = NULL;
	pTaskApi = (CTaskApi*) (::GetpConfPartyRoutingTable()->GetRsrcMngrPtrByPartyId(pKeyForLookup->GetPartyRsrcId()));

	if (CPObject::IsValidPObjectPtr(pTaskApi))
	{
		pTaskApi->SendMsg(pMsg, opcode);
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::DispatchMplEventByPartyId : EVENT IS DISCARDED \n");
		POBJDELETE(pMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::DispatchMplEventByConfId(OPCODE opcode, CRsrcParams* pKeyForLookup, CSegment* pMsg) const
{
	CSegment* pMsg1 = new CSegment;
	*pMsg1 << opcode;
	opcode = CONF_MPL_MSG;
	*pMsg1<< *pMsg;//content
	*pMsg =  *pMsg1;
	POBJDELETE(pMsg1);

	CTaskApi* pTaskApi = NULL;
	pTaskApi = (CTaskApi*) (::GetpConfPartyRoutingTable()->GetRsrcMngrPtrByConfId(pKeyForLookup->GetConfRsrcId()));

	if (CPObject::IsValidPObjectPtr(pTaskApi))
	{
		pTaskApi->SendMsg(pMsg, opcode);
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::DispatchMplEventByConfId : EVENT IS DISCARDED \n");
		POBJDELETE(pMsg);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::DispatchToLobby(OPCODE opcode, CSegment* pMsg) const
{
	CProcessBase* pProcess = CProcessBase::GetProcess();
	eMcuState systemState = pProcess->GetSystemState();
	if(eMcuState_Startup == systemState)
	{
		if( H323_CS_SIG_CALL_OFFERING_IND == opcode )
		{
			SendRejectAnswer(pMsg);
		}
		/*else if( SIP_CS_SIG_INVITE_IND == opcode )
		{
			Send503Reject(pMsg);
		}*/
		else
		{
			PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::DispatchToLobby : System state is startup! EVENT IS DISCARDED \n");
			POBJDELETE(pMsg);
		}
		return;
	}

	CTaskApi* pTaskApi = (CTaskApi*)(::GetpLobbyApi());

	if (CPObject::IsValidPObjectPtr(pTaskApi))
	{
		pTaskApi->SendMsg(pMsg, opcode);
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::DispatchToLobby : EVENT IS DISCARDED \n");
		POBJDELETE(pMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::DispatchToConfPartyManager(OPCODE opcode, CSegment* pMsg) const
{
	TRACEINTO << "Opcode:" << opcode;

	CConfPartyProcess* pProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	if (eMcuState_Startup == pProcess->GetSystemState())
	{
		TRACEINTO << "Opcode:" << opcode << " - Failed, System in startup state";
	}
	else
	{
		const CTaskApi* pTaskApi = pProcess->GetManagerApi();
		if (pTaskApi)
		{
			pTaskApi->SendMsg(pMsg, opcode);
			return;
		}
		else
		{
			TRACEINTO << "Opcode:" << opcode << " - Failed, Task API does not exist";
		}
	}
	POBJDELETE(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::DispatchReferEvent(DWORD data_len, char* pData, OPCODE opcode, CSegment* pMsg) const
{
	CTaskApi* pTaskApi = NULL;
	mcIndRefer * pReferMsg = (mcIndRefer *)new char[data_len];
	memcpy(pReferMsg, pData, data_len);

	CSipHeaderList * pTemp = new CSipHeaderList(pReferMsg->sipHeaders);

	const CSipHeader* pTo = pTemp ? pTemp->GetNextHeader(kTo) : NULL;
	const char* pToStr = pTo ? pTo->GetHeaderStr() : "";
	char *pConfName = new char [H243_NAME_LEN];
	memset(pConfName, 0, H243_NAME_LEN);

	if(strcmp(pToStr, ""))
	{
		char *temp = (char*)strstr(pToStr, "@");
		if(temp)
		{
			strncpy(pConfName, pToStr, temp - pToStr);
			pConfName[temp-pToStr] = '\0';
		}
	}
	PTRACE2(eLevelInfoNormal,"CConfPartyDispatcherTask::DispatchReferEvent SIP Refer, To:",pConfName);
	BYTE bRejectRequest = TRUE;
	//verify conf name is not empty
	if(strcmp(pConfName, ""))
	{
		//CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(pConfName);
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(pConfName);
		//conf found, handle it
		if(IsValidPObjectPtr(pCommConf))
		{
			bRejectRequest = FALSE;
			pTaskApi = new CConfApi;
			pTaskApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()));

			if (CPObject::IsValidPObjectPtr(pTaskApi))
			{
				pTaskApi->SendMsg(pMsg, opcode);
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::DispatchReferEvent : EVENT IS DISCARDED \n");
				POBJDELETE(pMsg);
			}

			POBJDELETE(pTaskApi);
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::DispatchReferEvent, SIP REFER dest conf is not found.");
			APIU32 callIndex = 0;
			APIU32 channelIndex = 0;
			APIU32 mcChannelIndex = 0;
			APIU32 stat1 = 0;
			APIS32 status = 0;
			APIU16 srcUnitId = 0;
			APIU32 csServiceId = 0;

			*pMsg >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId >> csServiceId;

			mcReqReferResp* pReferResp = new mcReqReferResp;
			pReferResp->pCallObj = pReferMsg->pCallObj;
			pReferResp->status = SipCodesNotFound;
			pReferResp->remoteCseq = pReferMsg->remoteCseq;

			CSegment* pSeg = new CSegment;
			*pSeg << callIndex
				  << srcUnitId;
			pSeg->Put((BYTE*)pReferResp, sizeof(mcReqReferResp));
			POBJDELETE(pReferResp);

			SendMsgToCS(csServiceId, SIP_CS_SIG_REFER_RESP_REQ, pSeg);
			POBJDELETE(pSeg);
			POBJDELETE(pMsg);
		}
	}

	POBJDELETE(pTemp);
	PDELETEA(pReferMsg);
	PDELETEA(pConfName);
}
/*IBM - EVENT PACKAGE */
/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::DispatchSubscribeEvent(DWORD data_len, char* pData, OPCODE opcode, CSegment* pMsg) const
{
	CTaskApi* pTaskApi = NULL;
	mcIndSubscribe * pSubscribeMsg = (mcIndSubscribe *)new char[data_len];
	memcpy(pSubscribeMsg, pData, data_len);

	CSipHeaderList * pTemp = new CSipHeaderList(pSubscribeMsg->sipHeaders);
	if (DispatchSubscribeEventAccordingToTargetDialog(pTemp,opcode,pMsg) != STATUS_OK)
	{
		if (DispatchSubscribeEventAccordingToTag(pTemp,opcode,pMsg) != STATUS_OK)
		{
			PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::DispatchSubscribeEvent, SIP SUBSCRIBE dest conf is not found.");
			APIU32 callIndex = 0;
			APIU32 channelIndex = 0;
			APIU32 mcChannelIndex = 0;
			APIU32 stat1 = 0;
			APIS32 status = 0;
			APIU16 srcUnitId = 0;
			APIU32 csServiceId = 0;

			*pMsg >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId >> csServiceId;
			POBJDELETE(pMsg);

//			mcReqSubscribeResp* pSubscribeResp = new mcReqSubscribeResp;
//			pSubscribeResp->pCallObj = pSubscribeMsg->pCallObj;
//			pSubscribeResp->status = SipCodesNotFound;
//			pSubscribeResp->remoteCseq = pSubscribeMsg->remoteCseq;
//
//			CSegment* pSeg = new CSegment;
//			*pSeg << callIndex
//				  << srcUnitId;
//			pSeg->Put((BYTE*)pSubscribeResp, sizeof(mcReqSubscribeResp));
//
//			SendMsgToCS(csServiceId, SIP_CS_SIG_SUBSCRIBE_RESP_REQ, pSeg);
//			POBJDELETE(pSeg);

			//POBJDELETE(pMsg);
		}
	}
//	else	// STATUS == OK, already deleted inside!!
//	{
//		POBJDELETE(pMsg);
//	}

	POBJDELETE(pTemp);
	PDELETEA(pSubscribeMsg);
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyDispatcherTask::DispatchSubscribeEventAccordingToTargetDialog(CSipHeaderList* pTempHeaders, OPCODE opcode, CSegment* pMsg) const
{
	STATUS status = STATUS_FAIL;
	const CSipHeader* pTDialog = pTempHeaders ? pTempHeaders->GetNextHeader(kTargetDialog) : NULL;
	const char* pTDialogStr = pTDialog ? pTDialog->GetHeaderStr() : "";

	if (pTDialog)
	{
		TRACEINTOFUNC << "found target dialog header: " << pTDialogStr;
		//<sip:1001@10.227.2.179>;tag=rmx2k_554517939-8573-RMX-0000000003-0080871424
		char* rmxFromSipTag = (char*)strstr(pTDialogStr, "-RMX-");
		if (rmxFromSipTag != NULL)
		{
			char* partyIdFromSipTag = rmxFromSipTag + 5;
			if (partyIdFromSipTag != NULL)
			{
				partyIdFromSipTag = strtok (partyIdFromSipTag,";-");
				if (partyIdFromSipTag != NULL)
				{
					PartyRsrcID partyRsrcID = strtol(partyIdFromSipTag, NULL, 16);
					TRACEINTOFUNC << "partyRsrcID = " << partyRsrcID;
					//DWORD connection_id =
					CTaskApi* pTaskApi = (CTaskApi*) (::GetpConfPartyRoutingTable()->GetRsrcMngrPtrByPartyId(partyRsrcID));

					if (CPObject::IsValidPObjectPtr(pTaskApi))
					{
						pTaskApi->SendMsg(pMsg, opcode);
						status = STATUS_OK;
					}
					else
					{
						TRACEINTOFUNC << ": EVENT IS DISCARDED - Could not find Conf Mbx (party id: " << partyRsrcID << ")";
					}
				}
			}
		}
	}

	return status;

}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyDispatcherTask::DispatchSubscribeEventAccordingToTag(CSipHeaderList* pTempHeaders, OPCODE opcode, CSegment* pMsg) const
{
	STATUS status = STATUS_FAIL;
	const CSipHeader* pTo = pTempHeaders ? pTempHeaders->GetNextHeader(kTo) : NULL;
	const char* pToStr = pTo ? pTo->GetHeaderStr() : "";
	char *pConfName = new char [H243_NAME_LEN];
	memset(pConfName, 0, H243_NAME_LEN);

	if(strcmp(pToStr, ""))
	{
		char *temp = (char*)strstr(pToStr, "@");
		if(temp)
		{
			strncpy(pConfName, pToStr, temp - pToStr);
			pConfName[temp-pToStr] = '\0';
		}
	}

	TRACEINTOFUNC << "SIP Subscribe, To conf: " <<  pConfName;

	BYTE bRejectRequest = TRUE;
	//verify conf name is not empty
	if(strcmp(pConfName, ""))
	{
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(pConfName);
		//conf found, handle it
		if(IsValidPObjectPtr(pCommConf))
		{
			bRejectRequest = FALSE;
			CTaskApi* pTaskApi = new CConfApi;
			pTaskApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()));

			if (CPObject::IsValidPObjectPtr(pTaskApi))
			{
				pTaskApi->SendMsg(pMsg, opcode);
				status = STATUS_OK;
			}
			else
			{
				TRACEINTOFUNC << ": EVENT IS DISCARDED";
			}
			POBJDELETE(pTaskApi);
		}
	}
	PDELETEA(pConfName);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::DispatchNotifyAck(ConfRsrcID dwConfId, PartyRsrcID dwPartyId, DWORD data_len, char* pData, OPCODE opcode, CSegment* pMsg) const
{
	CTaskApi* pTaskApi = NULL;
	mcIndNotifyResp* pNotifyAckMsg = (mcIndNotifyResp*)new char[data_len];
	memcpy(pNotifyAckMsg, pData, data_len);
	TRACEINTO << "CConfPartyDispatcherTask::DispatchNotifyAck \nConfRsrcID = " << dwConfId << "\nPartyRsrcID = " << dwPartyId;
	BYTE bRejectRequest = TRUE;
	//verify conf name is not empty
	if(dwConfId > 0)
	{
		CTaskApi* pTaskApi = NULL;
		// Find dest task according to party id (party cntl mbx)
		pTaskApi = (CTaskApi*) (::GetpConfPartyRoutingTable()->GetRsrcMngrPtrByPartyId(dwPartyId));
		if (CPObject::IsValidPObjectPtr(pTaskApi))
		{
			bRejectRequest = FALSE;
			pTaskApi->SendMsg(pMsg, opcode);
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::DispatchNotifyAck : EVENT IS DISCARDED \n");
			POBJDELETE(pMsg);
		}
//		POBJDELETE(pTaskApi);
	}
	else
	{
		POBJDELETE(pMsg);
	}
	PDELETEA(pNotifyAckMsg);
}

/*IBM - EVENT PACKAGE */
/////////////////////////////////////////////////////////////////////////////
void  CConfPartyDispatcherTask::SendMsgToCS(DWORD cs_Id, OPCODE opcode, CSegment* pseg1) const
{
	CMplMcmsProtocol *pMplMcmsProtocol = new CMplMcmsProtocol;

	if(!pMplMcmsProtocol)
	{
		PASSERT(101);
		return;
	}

	pMplMcmsProtocol->AddCommonHeader(opcode);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPortDescriptionHeader(0, 0, 1023);

	if(pseg1)
	{
		DWORD	callIndex = 0;
		WORD	srcUnitId = 0;
		*pseg1 >> callIndex
			   >> srcUnitId;
		pMplMcmsProtocol->AddCSHeader(cs_Id, 0, srcUnitId, callIndex, 0, 0, 0, -1);

		DWORD nMsgLen = pseg1->GetWrtOffset() - pseg1->GetRdOffset();
		BYTE* pMessage = new BYTE[nMsgLen];
		pseg1->Get(pMessage,nMsgLen);
		pMplMcmsProtocol->AddData(nMsgLen,(const char*)pMessage);
		PDELETEA(pMessage);
	}
	pMplMcmsProtocol->AddPayload_len(CS_API_TYPE);
	CMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("CConfPartyDispatcherTask::SendMsgToCS ",CS_API_TYPE);
	pMplMcmsProtocol->SendMsgToCSApiCommandDispatcher();
	PDELETE(pMplMcmsProtocol);
}

/////////////////////////////////////////////////////////////////////////////
eDispatchingMethods CConfPartyDispatcherTask::GetSuitedDispatchingMethod(CRsrcParams* pKey) const
{
	eDispatchingMethods dispatchingMethod = eDispatching_none;

	if (pKey->GetConnectionId() == LOBBY_CONNECTION_ID)
		dispatchingMethod = eDispatching_Lobby;

	else if (pKey->GetConnectionId() == CONF_PARTY_CONNECTION_ID)
		dispatchingMethod = eDispatching_ConfPartyManager;

	else if (pKey->GetConnectionId() != DUMMY_CONNECTION_ID)
		dispatchingMethod = eDispatching_By_ConnectionId;

	else if (pKey->GetPartyRsrcId() != DUMMY_PARTY_ID)
		dispatchingMethod = eDispatching_By_PartyId;

	else if (pKey->GetConfRsrcId() != DUMMY_CONF_ID)
		dispatchingMethod = eDispatching_By_ConfId;

	return dispatchingMethod;
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::HandleCsEvent(CSegment* pCsMsgSeg)
{
	CMplMcmsProtocol*   pMplCsprtcl = new CMplMcmsProtocol;
	pMplCsprtcl->DeSerialize(*pCsMsgSeg, CS_API_TYPE);
	CConfPartyMplMcmsProtocolTracer(*pMplCsprtcl).TraceMplMcmsProtocol("***CConfPartyDispatcherTask::HandleCsEvent", CS_API_TYPE);

	APIU32       csServiceId    = pMplCsprtcl->getCentralSignalingHeaderCsId();
	ConnectionID ConnId         = pMplCsprtcl->getPortDescriptionHeaderConnection_id();
	PartyRsrcID  PartyId        = pMplCsprtcl->getPortDescriptionHeaderParty_id();
	ConfRsrcID   ConfId         = pMplCsprtcl->getPortDescriptionHeaderConf_id();
	OPCODE       opcode         = pMplCsprtcl->getOpcode();
	APIU32       callIndex      = pMplCsprtcl->getCentralSignalingHeaderCallIndex();
	APIU32       channelIndex   = pMplCsprtcl->getCentralSignalingHeaderChannelIndex();
	APIU32       mcChannelIndex = pMplCsprtcl->getCentralSignalingHeaderMcChannelIndex();
	APIS32       status         = pMplCsprtcl->getCentralSignalingHeaderStatus();
	APIU16       srcUnitId      = pMplCsprtcl->getCentralSignalingHeaderSrcUnitId();
	APIU32       serviceId      = pMplCsprtcl->getCentralSignalingHeaderServiceId();

	CRsrcParams* pKeyForLookup = new CRsrcParams(ConnId, PartyId, ConfId, eLogical_ip_signaling);

	eDispatchingMethods dispatchingMethod = GetSuitedDispatchingMethod(pKeyForLookup);

	if (SIP_CS_SIG_REFER_IND == opcode)
		dispatchingMethod = eDispatching_Refer;
	else if (SIP_CS_SIG_SUBSCRIBE_IND == opcode)
		dispatchingMethod = eDispatching_Sebscribe;
	else if (SIP_CS_SIG_NOTIFY_RESPONSE_IND == opcode)
		dispatchingMethod = eDispatching_NotifyAck;

	CSegment* pMsg = new CSegment;
	pMsg->Put(callIndex);
	pMsg->Put(channelIndex);
	pMsg->Put(mcChannelIndex);
	pMsg->Put((APIU32)status);
	pMsg->Put(srcUnitId);

	switch (opcode)
	{
		case SIP_CS_SIG_INVITE_IND:
		case SIP_CS_CCCP_SIG_INVITE_IND:
		case H323_CS_SIG_CALL_OFFERING_IND:
		case SIP_CS_SIG_OPTIONS_IND:
		case SIP_CS_SIG_REFER_IND:
		case SIP_CS_SIG_SUBSCRIBE_IND:
		case SIP_CS_SIG_NOTIFY_RESPONSE_IND:
		{
			pMsg->Put(csServiceId);
			break;
		}
		default:
		{
			break;
		}
	}

	if (pMplCsprtcl->getDataLen())
	{
		pMsg->Put((unsigned char*)pMplCsprtcl->GetData(), pMplCsprtcl->getDataLen());
	}

	switch (dispatchingMethod)
	{
		case eDispatching_Lobby:
		{
			DispatchToLobby(opcode, pMsg);
			break;
		}

		case eDispatching_ConfPartyManager:
		{
			DispatchToConfPartyManager(opcode, pMsg);
			break;
		}

		case eDispatching_Refer:
		{
			DispatchReferEvent(pMplCsprtcl->getDataLen(), pMplCsprtcl->GetData(), opcode, pMsg);
			break;
		}

		/*IBM - EVENT PACKAGE */
		case eDispatching_Sebscribe:
		{
			DispatchSubscribeEvent(pMplCsprtcl->getDataLen(), pMplCsprtcl->GetData(), opcode, pMsg);
			break;
		}

		case eDispatching_NotifyAck:
		{
			DispatchNotifyAck(ConfId, PartyId, pMplCsprtcl->getDataLen(), pMplCsprtcl->GetData(), opcode, pMsg);
			break;
		}

		/*IBM - EVENT PACKAGE */
		case eDispatching_By_ConnectionId:
		{
			DispatchMplEventByPartyIdAndConnectionId(opcode, pKeyForLookup, pMsg);
			break;
		}

		default:
		{
			PASSERTMSG(1, "No Relevant Dispatching Method for CS Event");
			POBJDELETE(pMsg);
			break;
		}
	} // switch

	POBJDELETE(pMplCsprtcl);
	POBJDELETE(pKeyForLookup);
}


/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::HandleSignalingManagerEvent(CSegment *pGkManagerMsgSeg)
{
	ConnectionID connId;
	*pGkManagerMsgSeg >> connId;
	PartyRsrcID partyId;
	*pGkManagerMsgSeg >> partyId;
	OPCODE opcode;
	*pGkManagerMsgSeg >> opcode;

	CRsrcParams* pKeyForLookup = new CRsrcParams(connId, partyId, 0xFFFFFFFF, eLogical_ip_signaling );
	eDispatchingMethods dispatchingMethod = GetSuitedDispatchingMethod(pKeyForLookup);

	CSegment* pMsg = new CSegment;
	DWORD msgLen =  pGkManagerMsgSeg->GetWrtOffset() - pGkManagerMsgSeg->GetRdOffset();
	if (msgLen)
		pMsg->Put(pGkManagerMsgSeg->GetPtr(1), msgLen);

	switch (dispatchingMethod)
	{
		case eDispatching_Lobby:
		{
			DispatchToLobby(opcode, pMsg);
			break;
		}
		case eDispatching_By_ConnectionId:
		{
			DispatchMplEventByPartyIdAndConnectionId(opcode, pKeyForLookup, pMsg);
			break;
		}
		default:
		{
			PASSERTMSG(1,"No Relevant Dispatching Method for signaling Manager Event");
			POBJDELETE(pMsg);
		}
	}
	POBJDELETE(pKeyForLookup);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::Send503Reject(CSegment* pMsg) const
{
	PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::Send503Reject");
	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIS32 status = 0;
	APIU16 srcUnitId = 0;
	APIU32 csServiceId = 0;

	*pMsg >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId >> csServiceId;

	mcReqInviteResponse* pInviteResp = new mcReqInviteResponse;
	memset(pInviteResp, 0, sizeof(mcReqInviteResponse));
	pInviteResp->status = SipCodesServiceUnavail;

	CSegment* pSeg = new CSegment;
	*pSeg << callIndex
		  << srcUnitId;
	pSeg->Put((BYTE*)pInviteResp, sizeof(mcReqInviteResponse));

	SendMsgToCS(csServiceId, SIP_CS_SIG_INVITE_RESPONSE_REQ, pSeg);
	POBJDELETE(pSeg);
	PDELETE(pInviteResp);

}


/////////////////////////////////////////////////////////////////////////////
void CConfPartyDispatcherTask::SendRejectAnswer(CSegment* pMsg) const
{
	PTRACE(eLevelInfoNormal,"CConfPartyDispatcherTask::SendRejectAnswer");
	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIS32 status = 0;
	APIU16 srcUnitId = 0;
	APIU32 csServiceId = 0;

	*pMsg >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId >> csServiceId;

	APIU32 lengthStructure = sizeof(mcReqCallAnswer);
	mcReqCallAnswer* pCallAnswerReq = (mcReqCallAnswer *)new BYTE[lengthStructure];
	memset(pCallAnswerReq, '\0', lengthStructure);
	pCallAnswerReq->rejectCallReason = -1;

	CSegment* pSeg = new CSegment;
	*pSeg << callIndex
		  << srcUnitId;
	pSeg->Put((BYTE*)pCallAnswerReq, sizeof(mcReqCallAnswer));

	SendMsgToCS(csServiceId, H323_CS_SIG_CALL_ANSWER_REQ, pSeg);
	POBJDELETE(pSeg);
	PDELETEA(pCallAnswerReq);
	POBJDELETE(pMsg);
}
