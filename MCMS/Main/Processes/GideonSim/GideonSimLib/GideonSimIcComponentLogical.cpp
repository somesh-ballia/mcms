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
#include "GideonSimIcComponentLogical.h"



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//   CIcComponent - class treats all IC (Ivr Controller) commands (PlayMessage, etc.)
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

//const OPCODE IC_IVR_PLAY_RESPONSE_TOUT	= 11001;

//const DWORD  IC_IVR_RESPONSE_TIME	= 1 * SECOND;

/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CIcComponent)
	// Timer for sending IVR play message responses
	ONEVENT( IC_IVR_PLAY_RESPONSE_TOUT, ANYCASE, CIcComponent::OnIvrRespToutAnycase)
PEND_MESSAGE_MAP(CIcComponent,CStateMachine)


/////////////////////////////////////////////////////////////////////////////
CIcComponent::CIcComponent(CGideonSimMfaLogical* pMfaModule,
                           CTaskApp *pOwnerTask)
        :CStateMachine(pOwnerTask)
{
	m_pMfaModule = pMfaModule;

	for( int i=0; i<MAX_IC_IVR_PLAY_MSGS; i++ )
	{
		m_aIvrPlayMsg[i].pProtocol   = NULL;
		m_aIvrPlayMsg[i].nTimeRemain = 0;
	}

	// endless loop of timer
	StartTimer(IC_IVR_PLAY_RESPONSE_TOUT,IC_IVR_RESPONSE_TIME);
}

/////////////////////////////////////////////////////////////////////////////
CIcComponent::~CIcComponent()
{
	DeleteTimer(IC_IVR_PLAY_RESPONSE_TOUT);

	m_pMfaModule = NULL;

	for( int i=0; i<MAX_IC_IVR_PLAY_MSGS; i++ )
		POBJDELETE(m_aIvrPlayMsg[i].pProtocol);
}


/////////////////////////////////////////////////////////////////////////////
void* CIcComponent::GetMessageMap()
{
	return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
void CIcComponent::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}


/////////////////////////////////////////////////////////////////////////////
void CIcComponent::OnMcmsReq( CMplMcmsProtocol* pMplProtocol )
{
	DWORD  opcode = pMplProtocol->getOpcode();
	switch( opcode ) {
		case IVR_RECORD_ROLL_CALL_REQ:
		{
			BYTE* pData   = (BYTE*)(pMplProtocol->GetData());
			DWORD dataLen = pMplProtocol->getDataLen();
			CSegment  seg;
			seg.Put( pData, dataLen );

			CIVRPlayMessage* pIVRPlayMessage = new CIVRPlayMessage; AUTO_DELETE(pIVRPlayMessage);
			pIVRPlayMessage->DeSerialize(&seg);

			// copy roll-call file
			string sourceRollCallFile = IVR_SIM_FOLDER_ROLLCALL;
			sourceRollCallFile += 		IVR_SIM_ROLLCALL_FILE;

//			string destRollCallFile = 	IVR_SIM_FOLDER_MAIN;
//			destRollCallFile += 		IVR_SIM_FOLDER_ROLLCALL;
//			destRollCallFile += 		"/testtest.wav";


			SIVRPlayMessageStruct*  pReqStruct = &(pIVRPlayMessage->play);

			for( int i=0; i<(int)pReqStruct->numOfMediaFiles; i++ )
			{
				// send indication to GUI
				SIVRMediaFileParamsStruct	*mediaF = &pReqStruct->mediaFiles[i];
				DWORD action = mediaF->actionType;
				//if (pReqStruct->mediaFiles[i].actionType == IVR_ACTION_TYPE_RECORD)
				if (action == IVR_ACTION_TYPE_RECORD)
				{
					char* pszFileName = pReqStruct->mediaFiles[i].fileName;
					string destRollCallFile = "IVRX/";
					destRollCallFile += pszFileName;	// RollCall/<filename>

					PTRACE2(eLevelInfoNormal,"CIcComponent::OnMcmsReq - IVR_PLAY_MESSAGE_REQ. Path: ",pszFileName);

					link( sourceRollCallFile.c_str(), destRollCallFile.c_str() );

					break;
				}
			}

			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIcComponent::OnIvrRespToutAnycase(CSegment* pParam)
{
	// endless loop of timer
	StartTimer(IC_IVR_PLAY_RESPONSE_TOUT,IC_IVR_RESPONSE_TIME);

	// run over all msg's an check which should be ACKed
	for( int i=0; i<MAX_IC_IVR_PLAY_MSGS; i++ )
	{
		if( NULL != m_aIvrPlayMsg[i].pProtocol )
		{
			if( 0 >= m_aIvrPlayMsg[i].nTimeRemain )
			{
				// send ACK for this msg here
				if( TRUE == m_pMfaModule->IsValidParty(*(m_aIvrPlayMsg[i].pProtocol)) ||
					   1 == m_pMfaModule->IsConfPlayMessage(*m_aIvrPlayMsg[i].pProtocol) )
				{
					switch( m_aIvrPlayMsg[i].pProtocol->getOpcode() )
					{
						case IVR_RECORD_ROLL_CALL_REQ:
						{
							SIVRRecordMessageIndStruct   rollCallStructInd;
							memset( &rollCallStructInd, 0, sizeof(SIVRRecordMessageIndStruct) );
							rollCallStructInd.status = STATUS_OK;
							rollCallStructInd.recordingLength = 3;
							m_aIvrPlayMsg[i].pProtocol->AddCommonHeader( IVR_RECORD_ROLL_CALL_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );
							m_aIvrPlayMsg[i].pProtocol->AddData( sizeof(SIVRRecordMessageIndStruct), (char*)(&rollCallStructInd) );
							m_pMfaModule->SendToCmForMplApi(*(m_aIvrPlayMsg[i].pProtocol));	// send to MCMS
							break;
						}
						case IVR_PLAY_MESSAGE_REQ:
						default:
						{
							m_pMfaModule->Ack(*(m_aIvrPlayMsg[i].pProtocol),STATUS_OK);
							break;
						}
					}
				}
				else
					TRACESTR(eLevelInfoNormal) << " CIcComponent::OnIvrRespToutAnycase - party not valid (disconnected?) ";
				// delete protocol
				POBJDELETE(m_aIvrPlayMsg[i].pProtocol);
			}
			else
			{
				// decrease remain time
				m_aIvrPlayMsg[i].nTimeRemain--;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIcComponent::AddIvrPlayMsgToQ(CMplMcmsProtocol& rMplProt)
{
	// check opcode
	if( IVR_PLAY_MESSAGE_REQ != rMplProt.getOpcode() )
		return;

	CSegment  seg;
	seg.Put((BYTE*)rMplProt.GetData(),rMplProt.getDataLen());

	CIVRPlayMessage* pIVRPlayMessage = new CIVRPlayMessage;
	pIVRPlayMessage->DeSerialize(&seg);

	SIVRPlayMessageStruct*  pReqStruct = &(pIVRPlayMessage->play);

		// if loop - send ACK immediately
	if( 1000 < pReqStruct->numOfRepetition )
	{
		m_pMfaModule->Ack(rMplProt,STATUS_OK);
	}
	else
	{
		// place msg to queue
		int i = 0;
		for( i=0; i<MAX_IC_IVR_PLAY_MSGS; i++ )
		{
			// find first free place in array
			if( NULL == m_aIvrPlayMsg[i].pProtocol )
			{
				m_aIvrPlayMsg[i].nTimeRemain = IC_REMAIN_TIMES_DEFAULT;
				m_aIvrPlayMsg[i].pProtocol   = new CMplMcmsProtocol(rMplProt);
				break;
			}
		}
		if( MAX_IC_IVR_PLAY_MSGS == i)
			PTRACE(eLevelInfoNormal,"CIcComponent::AddIvrPlayMsgToQ - no free place");
	}

	PDELETE(pIVRPlayMessage);
}

/////////////////////////////////////////////////////////////////////////////
void CIcComponent::AddIvrRollcallRecordToQ(CMplMcmsProtocol& rMplProt)
{
	// check opcode
	if( IVR_RECORD_ROLL_CALL_REQ != rMplProt.getOpcode() )
		return;

	CSegment  seg;
	seg.Put((BYTE*)rMplProt.GetData(),rMplProt.getDataLen());

	// place msg to queue
	int i = 0;
	for( i=0; i<MAX_IC_IVR_PLAY_MSGS; i++ )
	{
		// find first free place in array
		if( NULL == m_aIvrPlayMsg[i].pProtocol )
		{
			m_aIvrPlayMsg[i].nTimeRemain = IC_REMAIN_TIMES_ROLLCALL_RECORD;
			m_aIvrPlayMsg[i].pProtocol   = new CMplMcmsProtocol(rMplProt);
			break;
		}
	}
	if( MAX_IC_IVR_PLAY_MSGS == i)
		PTRACE(eLevelInfoNormal,"CIcComponent::AddIvrRollcallRecordToQ - no free place");
}
