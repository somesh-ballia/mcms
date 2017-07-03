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
#include "GideonSimRtmLogical.h"

#include "GideonSimTbComponentLogical.h"
#include "GideonSimIcComponentLogical.h"
#include "GideonSimBarakIcComponentLogical.h"
#include "GideonSimMfaLogical.h"
#include "GideonSimRtmLogical.h"
#include "GideonSimSwitchLogical.h"
#include "GideonSimLogicalModule.h"
#include "OpcodesMrmcCardMngrMrc.h"
/////////////////////////////////////////////////////////////////////////////
//
//   GideonSimRtmLogical - logical module of RTM
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CGideonSimRtmLogical)
	// Card MAnager: start-up
	ONEVENT( CM_STARTUP,   IDLE,     CGideonSimRtmLogical::OnCmStartupIdle)
	ONEVENT( CM_STARTUP,   CONNECT,     CGideonSimRtmLogical::OnCmStartupConnect)
	// Card Manager: message from MCMS
	ONEVENT( CM_MCMS_MSG,  STARTUP,  CGideonSimRtmLogical::OnCmProcessMcmsReqStartup)
	ONEVENT( CM_MCMS_MSG,  CONNECT,  CGideonSimRtmLogical::OnCmProcessMcmsReqConnect)
	ONEVENT( CARD_NOT_READY_DELAY_TIMER,   STARTUP,  CGideonSimRtmLogical::OnTimerCardsDelayTout)
	ONEVENT( CARD_NOT_READY_DELAY_TIMER,   ANYCASE,  CGideonSimRtmLogical::OnTimerCardsDelayTout)//vb temp
	// Card Manager: message from Endpoints Sim (PSTN)
	ONEVENT( CM_EP_SIM_MSG,  CONNECT,  CGideonSimRtmLogical::OnCmProcessEpSimMsgConnect)

	ONEVENT( SPAN_STATUS_TIMER,  ANYCASE,  CGideonSimRtmLogical::OnTimerSpanStatus )

//	// Timer:
//	ONEVENT( MFA_UNIT_CONFIG_TIMER,   STARTUP,  CGideonSimMfaLogical::OnTimerUnitConfigTout)
//	ONEVENT( MFA_MEDIA_CONFIG_TIMER,  STARTUP,  CGideonSimMfaLogical::OnTimerMediaConfigTout)
PEND_MESSAGE_MAP(CGideonSimRtmLogical,CGideonSimLogicalModule)


/////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CGideonSimRtmLogical::CGideonSimRtmLogical(CTaskApp* pTask,WORD boardId,WORD subBoardId,eCardType RtmCardType)
			: CGideonSimLogicalModule(pTask,boardId,subBoardId)
{
	memset(&m_rIsdnParams,0,sizeof(RTM_ISDN_PARAMETERS_S));
	m_disableEnablePortsSet = 0;

	m_cardType = RtmCardType;

}

/////////////////////////////////////////////////////////////////////////////
CGideonSimRtmLogical::~CGideonSimRtmLogical()
{
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimRtmLogical::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::ProcessEndpointsSimMsg(CSegment* pMsg)
{
	DispatchEvent(CM_EP_SIM_MSG, pMsg);
}


/////////////////////////////////////////////////////////////////////////////
// Events & Action functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::OnCmStartupIdle(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnCmStartupIdle - RTM start-up.");

	SendCardManagerLoadedInd();

//vb TEMP till startup will work - START
	m_state = CONNECT;
//vb TEMP till startup will work - END
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::OnCmStartupConnect(CSegment* pMsg)
{
//	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnCmStartupConnect - nothing is done");
	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnCmStartupConnect");
	SendCardManagerLoadedInd();
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::OnTimerCardsDelayTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnTimerCardsDelayTout - timer awoke.");

	SendCardManagerLoadedInd();
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::OnCmProcessMcmsReqStartup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal, "CGideonSimRtmLogical::OnCmProcessMcmsReqStartup");

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pMsg);

	/* Vasily: TODO - future implementation of RTM startup */
//	switch( pMplProtocol->getOpcode() ) {
//		case CM_UNIT_CONFIG_REQ: {
//			// TODO : here is a UNITs configuration
//			 CM_UNIT_CONFIG_S*  pUnits = (CM_UNIT_CONFIG_S*)(pMplProtocol->GetData());
// 			StartTimer(MFA_UNIT_CONFIG_TIMER,::GetGideonSystemCfg()->GetMfaUnitConfigTime()*SECOND);
//			break;
//								 }
//		case CM_MEDIA_IP_CONFIG_REQ:
//		case CM_RTM_MEDIA_IP_CONFIG_REQ: {
//			// TODO : here is a MEDIA IP config
//			// MEDIA_IP_PARAMS_S
// 			StartTimer(MFA_MEDIA_CONFIG_TIMER,::GetGideonSystemCfg()->GetMfaMediaConfigTime()*SECOND);
//			break;
//									 }
//	}
	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::OnCmProcessMcmsReqConnect(CSegment* pMsg)
{
 	PTRACE(eLevelInfoNormal, "CGideonSimRtmLogical::OnCmProcessMcmsReqConnect");

 	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
 	pMplProtocol->DeSerialize(*pMsg);

	DWORD opcode = pMplProtocol->getOpcode();

	switch( opcode )
	{
//vb TEMP till startup will work - START
		case CARDS_NOT_READY_REQ:
		{
			// delay connection
 			StartTimer(CARD_NOT_READY_DELAY_TIMER,RTM_CARDS_DELAY_TIME);
			PTRACE(eLevelInfoNormal, "CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - CARDS_NOT_READY_REQ received, start timer.");
			break;
		}

		case MFA_TASK_NOT_CREATED_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - MFA_TASK_NOT_CREATED_REQ opcode received.");
			break;
//			// delay connection
//			StartTimer(CARD_NOT_READY_DELAY_TIMER,RTM_CARDS_DELAY_TIME);
//			PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - MFA_TASK_NOT_CREATED_REQ received, start timer.");
//			break;
		}
		case CM_MEDIA_IP_CONFIG_REQ :
		case CM_RTM_MEDIA_IP_CONFIG_REQ :
		{
			SendCmMediaIpConfigInd();

			if (CM_MEDIA_IP_CONFIG_REQ == opcode)
			{
				PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - CM_MEDIA_IP_CONFIG_REQ opcode received.");
			}
			else if (CM_RTM_MEDIA_IP_CONFIG_REQ == opcode)
			{
				PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - CM_RTM_MEDIA_IP_CONFIG_REQ opcode received.");
			}

			break;
		}
		case RTM_ISDN_PARAMETERS_REQ :
		{
			RTM_ISDN_PARAMETERS_S* pIsdnParamStruct = (RTM_ISDN_PARAMETERS_S*)pMplProtocol->GetData();
			memcpy(&m_rIsdnParams, pIsdnParamStruct, sizeof(RTM_ISDN_PARAMETERS_S));
			break;
		}
		case RTM_ISDN_SPAN_CONFIG_REQ :
		{
			RTM_ISDN_SPAN_CONFIG_REQ_S* pSpanConfigStruct = (RTM_ISDN_SPAN_CONFIG_REQ_S*)pMplProtocol->GetData();
			if( pSpanConfigStruct->span_id <= SIM_ISDN_NUM_SPANS_IN_BOARD )
			{
				m_arSpans[pSpanConfigStruct->span_id].Configure(*pSpanConfigStruct);
				SendRtmIsdnSpanStatusInd(pSpanConfigStruct->span_id);
				//StartTimer(SPAN_STATUS_TIMER, 5*SECOND);
			}
			else
				PASSERT(pSpanConfigStruct->span_id+1000);
			break;
		}
		case RTM_ISDN_SPAN_DISABLE_REQ :
		{
			// TEMP - treating 'Disable' should be added
			break;
		}
		case RTM_ISDN_KEEP_ALIVE_REQ :
		{
			Ack_keep_alive(*pMplProtocol,STATUS_OK);
			break;
		}

//vb TEMP till startup will work - END
		case CONFPARTY_CM_OPEN_UDP_PORT_REQ:
		case CONFPARTY_CM_UPDATE_UDP_ADDR_REQ:
		case CONFPARTY_CM_CLOSE_UDP_PORT_REQ:
		{
//			CAudioParty*  pParty = FindParty(*pMplProtocol);
//			if( pParty != NULL )
//			{
//				pParty->SetRtpId(pMplProtocol->getPortDescriptionHeaderConnection_id());
//			}
//			else
//				DBGPASSERT(pMplProtocol->getPortDescriptionHeaderConnection_id()+1000);
//
			AckIp(*pMplProtocol,STATUS_OK);
			break;
		}
		case CONF_PARTY_MRMP_OPEN_CHANNEL_REQ:
		{
		    PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnCmProcessMcmsReqConnect CONF_PARTY_MRMP_OPEN_CHANNEL_REQ");
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
/*		case CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ:
		{
		    PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnCmProcessMcmsReqConnect CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ");
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}*/


		case NET_CONNECT_REQ :
		case NET_ALERT_REQ :
		case NET_DISCONNECT_ACK_REQ :
		{
			DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();
			DWORD partId = pMplProtocol->getPortDescriptionHeaderParty_id();

			NET_COMMON_PARAM_S*  pNetCommSt = (NET_COMMON_PARAM_S*)pMplProtocol->GetData();
			DWORD spanId = pNetCommSt->span_id;
			DWORD portId = pNetCommSt->physical_port_number;

				// check port for validity
			if( spanId > SIM_ISDN_NUM_SPANS_IN_BOARD )
			{
				TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - ILLEGAL SPAN_ID! <" << (int)spanId << ">";
				PASSERT(spanId);
				break;
			}
				// send ACK from card
//  			CMplMcmsProtocol* pMplProtocolCopy = new CMplMcmsProtocol(*pMplProtocol);
// 			Ack(*pMplProtocolCopy,STATUS_OK);
// 			POBJDELETE(pMplProtocolCopy);

			BYTE physicalPortNumber = 0;
			DWORD virtPortId = pNetCommSt->virtual_port_number;


			if( NET_ALERT_REQ != opcode )
			{
				physicalPortNumber = m_arSpans[spanId].FindPortByConfPartyId(confId, partId, opcode);
				if( 0 == physicalPortNumber )
				{
					TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - PORT with ConfId <" << confId << "> PartyId <" << partId <<"> not found!";
					PASSERT(1000+partId);
					// TODO: send reject
					break;
				}
			}
			else	// opcode = NET_ALERT_REQ
				{
					// in dial-in ConfId & PartyId are temp before NET_ALERT_REQ
						if( m_arSpans[spanId].IsPortValid(portId) )
						{
					m_arSpans[spanId].UpdatePortConfPartyId(portId, confId, partId);
							physicalPortNumber = portId;
					TRACESTR(eLevelInfoNormal) << " CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - in NET_ALERT_REQ port=" << (int)portId;
						}
						else
						{
					TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - in ALERT_REQ PORT port <" << (int)portId << "> not valid!";
							PASSERT(1000+portId);
							// TODO: send reject
							break;
						}
					}

			// forward message to EndpointsSim
			ForwardNetReq2Endpoints(spanId, physicalPortNumber, *pMplProtocol);
			break;
		}


		case NET_SETUP_REQ :
		{
			DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();
			DWORD partId = pMplProtocol->getPortDescriptionHeaderParty_id();
			DWORD channelConnectionId = pMplProtocol->getPortDescriptionHeaderConnection_id();

			// gets the command data
			NET_SETUP_REQ_HEADER_S* pNetSetUpHeaderSt = (NET_SETUP_REQ_HEADER_S*)pMplProtocol->GetData();
			if (NULL == pNetSetUpHeaderSt) {
				PTRACE(eLevelInfoNormal, "CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - illegal Data");
				PASSERT(NET_SETUP_REQ);
				break;
			}

			// gets some parameters from data
			DWORD portId = pNetSetUpHeaderSt->physical_port_number;
			DWORD virtPortId = pNetSetUpHeaderSt->virtual_port_number;


			// loop on spans until we find a place or until we reach to span 0 (end of list)
			int spanNum = 0;
			DWORD spanId = 0;
			DWORD spanError = 0;
			BYTE physicalPortNumber = 0;
			for (spanNum = 0; spanNum < MAX_NUM_SPANS_ORDER; spanNum++)
			{
				spanId = pNetSetUpHeaderSt->spans_order[spanNum];
				if (0 == spanId)
					break; // end of span list

				// checks span range
				if (spanId > SIM_ISDN_NUM_SPANS_IN_BOARD)
				{
					TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - Span ID error <"
						<< (int)spanId << "> out of range";
					PASSERT(spanId);
					spanError = 1;
					break;
				}

				// try to allocate port on span
				physicalPortNumber = m_arSpans[spanId].AllocatePort( virtPortId, confId, partId, channelConnectionId );
				if( 0 == physicalPortNumber )	// error
				{
					TRACESTR(eLevelInfoNormal) << " CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - SPAN is Full <"
						<< (int)spanId << "> cannot allocate a channel on it!";
					continue;	// span is full, skip to the next span
				}
				else	// OK
				{
					break;	// OK, we allocated the channel!
				}
			}

			// reject the call in case of error, it is NOT a logical error
			if (spanError)
			{
				//
				// here we need to reject this call with suitable error status!!!! ==> to be done later!!
				//
				PTRACE(eLevelInfoNormal, "CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - General request error - Reject the call");
				break;
			}

			// span was not found, all are full
			if ((0 == spanId) || (MAX_NUM_SPANS_ORDER == spanNum))
			{
				// here we need to reject this call with suitable status!!!!
				SendNetSetupReqFailure( confId, partId, channelConnectionId, virtPortId );

				PTRACE(eLevelInfoNormal, "CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - Spans are Full - Reject the call");
				break;
			}

			TRACEINTO << " NET_SETUP_REQ: Span and Port Allocated OK, spanId = " << spanId << " Virtual PortId = "
					  << (int)virtPortId << " Channel Connection Id = " << channelConnectionId;

			// forward message to EndpointsSim
			ForwardNetReq2Endpoints(spanId, physicalPortNumber, *pMplProtocol);
			break;
		}


		case NET_CLEAR_REQ :
		{
			DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();
			DWORD partyId = pMplProtocol->getPortDescriptionHeaderParty_id();
			DWORD channelConnectionId = pMplProtocol->getPortDescriptionHeaderConnection_id();

			TRACEINTO << "CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - (NET_CLEAR_REQ) confId=" << confId
					  << " partyId=" << partyId << " ChannelcoonnectionId=" << channelConnectionId;

			NET_COMMON_PARAM_S*  pNetCommSt = (NET_COMMON_PARAM_S*)pMplProtocol->GetData();
			DWORD spanId = pNetCommSt->span_id;
			DWORD portId = pNetCommSt->physical_port_number;

			BYTE physicalPortNumber = 0;
			physicalPortNumber = m_arSpans[spanId].GetPortByConnectionId(channelConnectionId);


			// check port for validity
			if( spanId > SIM_ISDN_NUM_SPANS_IN_BOARD )
			{
				TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - (NET_CLEAR_REQ) ILLEGAL SPAN_ID! <" << (int)spanId << ">";
				PASSERT(spanId);
				break;
			}


			// try to deallocate port on span by channel ConnectionId
			/*STATUS status = m_arSpans[spanId].DeAllocatePortByChannelConnectionId(channelConnectionId);
			if( status == STATUS_FAIL )
			{
				TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - (NET_CLEAR_REQ) cannot deallocate port";
				PASSERT(spanId);
				break;
			}*/

			// forward message to EndpointsSim
			ForwardNetReq2Endpoints(spanId, physicalPortNumber, *pMplProtocol);
			break;
		}

		case MOVE_RSRC_REQ:
		{
			DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();
			DWORD partId = pMplProtocol->getPortDescriptionHeaderParty_id();

			DWORD spanId = 1;

			BYTE physicalPortNumber = 0;
			// find port of party
			for( spanId=1; spanId <= SIM_ISDN_NUM_SPANS_IN_BOARD; spanId++ )
			{
				if( 0 != (physicalPortNumber = m_arSpans[spanId].FindPortByConfPartyId(confId, partId, opcode)) )
					break;
			}
				// check port for validity
			if( spanId > SIM_ISDN_NUM_SPANS_IN_BOARD )
			{
				TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - ILLEGAL SPAN_ID! <" << (int)spanId << ">";
				PASSERT(spanId);
				break;
			}
			if( 0 != physicalPortNumber )
			{
				if( m_arSpans[spanId].IsPortValid(physicalPortNumber) )
				{
						// update ConfId of party
					MOVE_RESOURCES_PARAMS_S*  pRequest = (MOVE_RESOURCES_PARAMS_S*)pMplProtocol->GetData();
					TRACEINTO << "CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - Handle MOVE_RSRC_REQ. New conf ID <" << pRequest->newConfId << ">";
					m_arSpans[spanId].UpdatePortConfPartyId(physicalPortNumber,pRequest->newConfId,partId);
						// forward message to EndpointsSim
					ForwardNetReq2Endpoints(spanId,physicalPortNumber,*pMplProtocol);
				}
				else
					TRACESTR(eLevelError) << "CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - Port not valid. Span <" << spanId << ">, Port <" << physicalPortNumber << ">.";
			}
			else
				TRACESTR(eLevelError) << "CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - No port found. Conf <" << confId << ">, Party <" << partId << ">.";

			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
        case DEBUG_MODE_YES_REQ:
            // do something, just cannot endure the assertion on default case.
            break;

        case SYSCFG_PARAMS_REQ:
        	break;

	case CM_UPGRADE_NEW_VERSION_READY_REQ:
	  {
	    // sagi add send ack for this request
	  }
	case CM_UPGRADE_START_WRITE_IPMC_REQ:
	  {

	  }
	case RESET_CARD_REQ:
	  {

	  }
	       break;


		default:
		{
			Ack(*pMplProtocol,STATUS_FAIL);
			TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessMcmsReqConnect - UNKNOWN opcode received <" << (int)opcode << ">.";
			PASSERT(opcode+1000);
			break;
		}
	}

	POBJDELETE(pMplProtocol);
}

///////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::Ack_keep_alive( CMplMcmsProtocol& rMplProt, STATUS status )
{
	rMplProt.AddCommonHeader(RTM_ISDN_KEEP_ALIVE_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	SendToCmForMplApi(rMplProt);
}


/////////////////////////////////////////////////////////////////////////////

/* for testing */
void CGideonSimRtmLogical::OnTimerSpanStatus(CSegment* pMsg)
{
	static int spanStatusCounter = 0;
	if (5 == spanStatusCounter)
		spanStatusCounter = 0;

	// send RTM_ISDN_SPAN_STATUS_IND
	RTM_ISDN_SPAN_STATUS_IND_S  rStruct;
	memset(&rStruct,0,sizeof(RTM_ISDN_SPAN_STATUS_IND_S));

	rStruct.span_id = 3;

	switch (spanStatusCounter)
	{
		case 0: m_arSpans[3].FillStatusStructBad(&rStruct, eNetSpanStatusRedAlarm, true);
				break;

		case 1: m_arSpans[3].FillStatusStructBad(&rStruct, eNetSpanStatusYellowAlarm, true);
				break;

		case 2: m_arSpans[2].FillStatusStructBad(&rStruct, eNetSpanStatusNormal, true);
				break;

		case 3: m_arSpans[2].FillStatusStructBad(&rStruct, eNetSpanStatusRedAlarm, false);
				break;

		default: m_arSpans[2].FillStatusStructBad(&rStruct, eNetSpanStatusYellowAlarm, true);
				 break;
	}

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,RTM_ISDN_SPAN_STATUS_IND,
			(BYTE*)(&rStruct),sizeof(RTM_ISDN_SPAN_STATUS_IND_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);


	spanStatusCounter++;
	StartTimer(SPAN_STATUS_TIMER, 5*SECOND);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::OnCmProcessEpSimMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::OnCmProcessEpSimMsgConnect");

	DWORD  opcode, confId, partyId, connectionId;
	WORD   boardId, subBoardId, spanId, portId;

	*pMsg	>> opcode
			>> boardId
			>> subBoardId
			>> spanId
			>> portId;

	*pMsg	>> confId
			>> partyId
			>> connectionId;

	TRACEINTO	<< "CGideonSimRtmLogical::OnCmProcessEpSimMsgConnect opcode:" << opcode;

	switch( opcode )
	{
		case NET_SETUP_IND:
		case NET_CONNECT_IND:
		case NET_CLEAR_IND:
		case NET_PROGRESS_IND:
		case NET_ALERT_IND:
		case NET_PROCEED_IND:
		case NET_DISCONNECT_IND:
		case NET_DISCONNECT_ACK_IND:
		{
			break;
		}
		default:
		{
			TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessEpSimMsgConnect - UNKNOWN opcode received <" << (int)opcode << ">.";
			PASSERT(opcode+1000);
			return;
		}
	}

	DWORD  virtPortNum = 0;
	if( NET_SETUP_IND != opcode && NET_CLEAR_IND != opcode)
	{
			// check port for validity
		if( spanId > SIM_ISDN_NUM_SPANS_IN_BOARD )
		{
			TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessEpSimMsgConnect - ILLEGAL SPAN_ID! <" << (int)spanId << ">";
			PASSERT(1000+spanId);
			return;
		}
		if( TRUE != m_arSpans[spanId].IsPortValid(portId) )
		{
			TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessEpSimMsgConnect - ILLEGAL PORT_ID! <" << (int)portId << ">";
			PASSERT(1000+portId);
			return;
		}
		if( TRUE != m_arSpans[spanId].IsPortInUseByConfParty(portId, confId, partyId, opcode) )
		{
			TRACESTR(eLevelError) << " CGideonSimRtmLogical::OnCmProcessEpSimMsgConnect opcode=" << opcode << " - PORT in use by another party! span<"
				<< (int)spanId << ">, port<" << (int)portId << ">, confId<" << confId << ">, partyId<" << partyId << ">.";
			PASSERT(1000+portId);
			return;
		}
		virtPortNum = m_arSpans[spanId].GetVirtualPortNum(portId);

		//TRACEINTO	<< " CGideonSimRtmLogical::OnCmProcessEpSimMsgConnect virtural_port=" << virtPortNum;
	}

		// get content of msg
	DWORD  dataLen = pMsg->GetWrtOffset() - pMsg->GetRdOffset();
	BYTE*  pData = new BYTE[dataLen];

	pMsg->Get(pData, dataLen);


	if( NET_SETUP_IND == opcode )
	{
		PASSERT(    boardId != 0xFFFF );
		PASSERT( subBoardId != 0xFFFF );
		PASSERT(     spanId != 0xFFFF );
		PASSERT(     portId != 0xFFFF );

		// allocate span and port
		int i = SIM_ISDN_NUM_SPANS_IN_BOARD;
		portId = 0;
/*		while( portId == 0  &&  i>=0 )
		{
			portId = m_arSpans[i].AllocatePort(0,0xFFFF0000,0xFFFF0000);
			if( portId != 0 )
			{
				spanId = i;
				break;
			}
			i--;
		}*/

		for(int i = 1; i <= SIM_ISDN_NUM_SPANS_IN_BOARD; i++ )
		{
		  portId =  m_arSpans[i].AllocatePort(0, 0xFFFF0000, 0xFFFF0000, 0);

		  TRACEINTO	<< " CGideonSimRtmLogical::OnCmProcessEpSimMsgConnect loop i =" << i << " portId = " << portId;

		  if( portId != 0 )
		    {
		      spanId = i;
		      break;
		    }
		}

		PASSERT( spanId == 0xFFFF );
		PASSERT( portId == 0xFFFF );
	}

	CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;

	NET_COMMON_PARAM_S*  pNetCommSt = (NET_COMMON_PARAM_S*)pData;
	pNetCommSt->span_id              = spanId;
	pNetCommSt->virtual_port_number  = virtPortNum;
	pNetCommSt->physical_port_number = portId;


	FillMplProtocol(pMplProt, opcode, pData, dataLen);
	pMplProt->AddPortDescriptionHeader(partyId, confId, connectionId, eLogical_net);

	SendToCmForMplApi(*pMplProt);

	POBJDELETE(pMplProt);

	PDELETEA(pData);

	// if ep disconnected - clean port details
	if( NET_DISCONNECT_ACK_IND == opcode )
	{
		m_arSpans[spanId].DeAllocatePort(portId);
	}

	if ( NET_CLEAR_IND == opcode )
	{
///		STATUS status = m_arSpans[spanId].DeAllocatePortByChannelConnectionId(connectionId);
		m_arSpans[spanId].DeAllocatePort(portId);
	}
}


/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::SendCardManagerLoadedInd() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::SendCardManagerLoadedInd - SEND to MPL-API.");

	// send CM_CARD_MNGR_LOADED_IND
	CM_CARD_MNGR_LOADED_S  rStruct;
	memset(&rStruct,0,sizeof(CM_CARD_MNGR_LOADED_S));

	string mplSerialNum = ::GetGideonSystemCfg()->GetBoxChassisId();
	int len = mplSerialNum.length();
	memcpy(rStruct.serialNum, ::GetGideonSystemCfg()->GetBoxChassisId(), len);

	rStruct.status    = STATUS_OK;
	rStruct.cardType  = m_cardType; 	// this important parameter actually tells the MCMS how
									// many units we have on card

	rStruct.hardwareVersion.ver_release  = 1;
	rStruct.hardwareVersion.ver_major    = 2;
	rStruct.hardwareVersion.ver_minor    = 3;
	rStruct.hardwareVersion.ver_internal = 4;

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,CM_CARD_MNGR_LOADED_IND,
			(BYTE*)(&rStruct),sizeof(CM_CARD_MNGR_LOADED_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::SendCmMediaIpConfigInd() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::SendCmMediaIpConfigInd - SEND to MPL-API.");

	// send CM_MEDIA_IP_CONFIG_IND
	MEDIA_IP_CONFIG_S  rStruct;
	memset(&rStruct,0,sizeof(MEDIA_IP_CONFIG_S));

	rStruct.status = STATUS_OK;
	rStruct.serviceId = 111;
	rStruct.pqNumber  = 1;

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,CM_MEDIA_IP_CONFIG_IND,
			(BYTE*)(&rStruct),sizeof(MEDIA_IP_CONFIG_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::SendRtmIsdnSpanStatusInd(const DWORD spanId) const
{
	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::SendRtmIsdnSpanStatusInd - SEND to MPL-API.");

	if( spanId > SIM_ISDN_NUM_SPANS_IN_BOARD ) {
		PASSERT_AND_RETURN(spanId+1000);
	}

	// send RTM_ISDN_SPAN_STATUS_IND
	RTM_ISDN_SPAN_STATUS_IND_S  rStruct;
	memset(&rStruct,0,sizeof(RTM_ISDN_SPAN_STATUS_IND_S));

	rStruct.span_id = spanId;
	m_arSpans[spanId].FillStatusStruct(&rStruct);

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,RTM_ISDN_SPAN_STATUS_IND,
			(BYTE*)(&rStruct),sizeof(RTM_ISDN_SPAN_STATUS_IND_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::SendRtmIsdnSpanStatusInd_ClockSourceAlert(const bool isSingle) const
{
	for (DWORD spanId=1; spanId <= SIM_ISDN_NUM_SPANS_IN_BOARD; spanId++ )
	{
		// send RTM_ISDN_SPAN_STATUS_IND
		RTM_ISDN_SPAN_STATUS_IND_S  rStruct;
		memset(&rStruct,0,sizeof(RTM_ISDN_SPAN_STATUS_IND_S));

		rStruct.span_id = spanId;
		rStruct.alarm_status = NO_ALARM;
		rStruct.d_chnl_status = D_CHANNEL_ESTABLISHED;
		rStruct.clocking_status = CLOCK_NONE;


		CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

		FillMplProtocol( pMplProtocol,RTM_ISDN_SPAN_STATUS_IND,
				(BYTE*)(&rStruct),sizeof(RTM_ISDN_SPAN_STATUS_IND_S));

		SendToCmForMplApi(*pMplProtocol);
		POBJDELETE(pMplProtocol);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Indications from card to MCMS
/////////////////////////////////////////////////////////////////////////////
//void CGideonSimRtmLogical::ProgressInd2Conf(const CPstnConnectionParty& party) const
//{
// 	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::ProgressInd2Conf - SEND to MPL-API.");

// 	// send CM_CARD_MNGR_LOADED_IND
// 	NET_PROGRESS_IND_S  rStruct;
// 	memset(&rStruct,0,sizeof(NET_PROGRESS_IND_S));

// 	rStruct.progress_dscr  = progRETURNED_ISDN_VAL;
// 	rStruct.net_common_header.first_port   = party.GetPortId();
// 	rStruct.net_common_header.num_of_ports = party.GetPortsNum();

// 	CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;//(rMplProtocol);

// 	FillMplProtocol(pMplProt,NET_PROGRESS_IND,(BYTE*)(&rStruct),sizeof(NET_PROGRESS_IND_S));
// 	pMplProt->AddPortDescriptionHeader(party.GetPartyId(),party.GetConfId(),
// 				party.GetConnectionId(),eLogical_net);

// 	SendToCmForMplApi(*pMplProt);
// 	POBJDELETE(pMplProt);
//}

/////////////////////////////////////////////////////////////////////////////
//void CGideonSimRtmLogical::DisconnectInd2Conf(const CPstnConnectionParty& party) const
//{
// 	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::DisconnectInd2Conf - SEND to MPL-API.");

// 	// send CM_CARD_MNGR_LOADED_IND
// 	NET_DISCONNECT_IND_S  rStruct;
// 	memset(&rStruct,0,sizeof(NET_DISCONNECT_IND_S));

// 	rStruct.net_common_header.first_port   = party.GetPortId();
// 	rStruct.net_common_header.num_of_ports = party.GetPortsNum();

// 	rStruct.cause.cause_val = 98;

// 	CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;//(rMplProtocol);

// 	FillMplProtocol(pMplProt,NET_DISCONNECT_IND,(BYTE*)(&rStruct),sizeof(NET_DISCONNECT_IND_S));
// 	pMplProt->AddPortDescriptionHeader(party.GetPartyId(),party.GetConfId(),
// 				party.GetConnectionId(),eLogical_net);

// 	SendToCmForMplApi(*pMplProt);
// 	POBJDELETE(pMplProt);
//}

/////////////////////////////////////////////////////////////////////////////
//void CGideonSimRtmLogical::ClearInd2Conf(const CPstnConnectionParty& party) const
//{
	// PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::ClearInd2Conf - SEND to MPL-API.");

// 	NET_CLEAR_IND_S  rStruct;
// 	memset(&rStruct,0,sizeof(NET_CLEAR_IND_S));

// 	rStruct.net_common_header.first_port   = party.GetPortId();
// 	rStruct.net_common_header.num_of_ports = party.GetPortsNum();

// 	rStruct.cause.cause_val = 98;

// 	CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;//(rMplProtocol);

// 	FillMplProtocol(pMplProt,NET_CLEAR_IND,(BYTE*)(&rStruct),sizeof(NET_CLEAR_IND_S));
// 	pMplProt->AddPortDescriptionHeader(party.GetPartyId(),party.GetConfId(),
// 				party.GetConnectionId(),eLogical_net);

// 	SendToCmForMplApi(*pMplProt);
// 	POBJDELETE(pMplProt);
//}

/////////////////////////////////////////////////////////////////////////////
//CPstnConnectionParty* CGideonSimRtmLogical::FindParty(const CMplMcmsProtocol& rMpl)
//{
//	CPstnConnectionParty* pParty = NULL;
//
//	int i = 0;
//	for( i=0; i<MAX_PSTN_PARTIES && pParty == NULL; i++ )
//		if( m_raPstnPartiesArr[i].GetConfId() == rMpl.getPortDescriptionHeaderConf_id() )
//			if( m_raPstnPartiesArr[i].GetPartyId() == rMpl.getPortDescriptionHeaderParty_id() )
//				pParty = &(m_raPstnPartiesArr[i]);
//
//	return pParty;
//}

/////////////////////////////////////////////////////////////////////////////
//CPstnConnectionParty* CGideonSimRtmLogical::FindParty(const CPstnConnectionParty& other)
//{
//	CPstnConnectionParty* pParty = NULL;
//
//	int i = 0;
//	for( i=0; i<MAX_PSTN_PARTIES && pParty == NULL; i++ )
//		if( m_raPstnPartiesArr[i].GetConfId() == other.GetConfId() )
//			if( m_raPstnPartiesArr[i].GetPartyId() == other.GetPartyId() )
//				pParty = &(m_raPstnPartiesArr[i]);
//
//	return pParty;
//}

/////////////////////////////////////////////////////////////////////////////
//void CGideonSimRtmLogical::ForwardNetReq2Endpoints(const DWORD opcode,
//		const CPstnConnectionParty& party, char* pData, DWORD nDataLen) const
//{
//	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::ForwardNetReq2Endpoints - SEND TO E.P.");
//
//	CSegment msg;
//
//	msg << opcode
//		<< m_wBoardId
//		<< m_wSubBoardId
//		<< (WORD)0; // unit_id
//
//	party.Serialize(msg);
//
//	msg.Put((BYTE*)(&pData),nDataLen);
//
//	::SendIsdnMessageToEndpointsSimApp(msg);
//}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::ForwardNetReq2Endpoints(const DWORD spanId, const DWORD portId, CMplMcmsProtocol& rMplProt) const
{
	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::ForwardNetReq2Endpoints - SEND TO E.P.");

	CSegment msg;

	msg << rMplProt.getOpcode()
		<< m_wBoardId
		<< m_wSubBoardId
		<< (WORD)spanId
		<< (WORD)portId;

	msg << rMplProt.getPortDescriptionHeaderConf_id()
		<< rMplProt.getPortDescriptionHeaderParty_id()
		<< rMplProt.getPortDescriptionHeaderConnection_id();

	NET_COMMON_PARAM_S*  pNetCommSt = (NET_COMMON_PARAM_S*)rMplProt.GetData();
	msg << pNetCommSt->net_connection_id;

	msg.Put((BYTE*)rMplProt.GetData(), rMplProt.getDataLen());//(BYTE*)(&pData),nDataLen);

	::SendIsdnMessageToEndpointsSimApp(msg);
}


/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::SendNetSetupReqFailure( const DWORD confId, const DWORD partyId, const DWORD connectionId, const DWORD virtPortId )
{
	CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;

	// fill disconnect struct
	NET_DISCONNECT_IND_S  rDisConnectStruct;
	memset(&rDisConnectStruct,0,sizeof(NET_DISCONNECT_IND_S));

	rDisConnectStruct.net_common_header.virtual_port_number = virtPortId;
	rDisConnectStruct.cause.cause_val = causNO_CHAN_AVL_VAL;


	FillMplProtocol(pMplProt, NET_DISCONNECT_IND, (BYTE *)&rDisConnectStruct, sizeof(NET_DISCONNECT_IND_S));
	pMplProt->AddPortDescriptionHeader(partyId, confId, connectionId, eLogical_net);

	SendToCmForMplApi(*pMplProt);

	POBJDELETE(pMplProt);
}


/////////////////////////////////////////////////////////////////////////////
void CGideonSimRtmLogical::SetRtmEnableDisablePorts( DWORD spanId, DWORD firstPort, DWORD numPorts, DWORD action )
{
	PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::SetRtmEnableDisablePorts - ");

	if (spanId > SIM_ISDN_NUM_SPANS_IN_BOARD)
	{
		PASSERT(spanId);
		PTRACE(eLevelError,"CGideonSimRtmLogical::SetRtmEnableDisablePorts - Illegal span number");
		return;
	}

	if (numPorts > 30)
	{
		PASSERT(numPorts);
		PTRACE(eLevelError,"CGideonSimRtmLogical::SetRtmEnableDisablePorts - Illegal ports number");
		return;
	}

	// currently just allocate number of ports
	// still need to treat deallocate, first port
	m_disableEnablePortsSet = 1;

	for (DWORD port = 0; port < numPorts; port++)
	{
		BYTE physicalPortNumber = m_arSpans[spanId].AllocatePort( 30, 666, 777, 888 );
		if( 0 == physicalPortNumber )	// error
		{
			PTRACE(eLevelError,"CGideonSimRtmLogical::SetRtmEnableDisablePorts - Illegal span number");
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CGideonSimRtmLogical::SetRtmEnableDisablePorts - force allocated succesfully");
		}
	}
}
