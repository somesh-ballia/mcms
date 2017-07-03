// GideonSimLogicalModule

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
#include "OpcodesMrmcCardMngrMrc.h"

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
#include "GideonSimMfaLogical.h"
#include "GideonSimIcComponentLogical.h"
#include "GideonSimTbComponentLogical.h"
#include "OpcodesMrmcCardMngrMrc.h"

PBEGIN_MESSAGE_MAP(CGideonSimMfaLogical)
	ONEVENT( CM_STARTUP,   IDLE,     CGideonSimMfaLogical::OnCmStartupIdle)
	ONEVENT( CM_STARTUP,   CONNECT,     CGideonSimMfaLogical::OnCmStartupConnect)
	ONEVENT( CM_MCMS_MSG,  STARTUP,  CGideonSimMfaLogical::OnCmProcessMcmsReqStartup)
	ONEVENT( CM_MCMS_MSG,  CONNECT,  CGideonSimMfaLogical::OnCmProcessMcmsReqConnect)
	ONEVENT( MFA_UNIT_CONFIG_TIMER,   STARTUP,  CGideonSimMfaLogical::OnTimerUnitConfigTout)
ONEVENT( MFA_MEDIA_CONFIG_COMPLETE_TIMER,  STARTUP,  CGideonSimMfaLogical::OnTimerMediaConfigCompletedTout)
ONEVENT( MFA_MEDIA_CONFIG_COMPLETE_TIMER,  CONNECT,  CGideonSimMfaLogical::OnTimerMediaConfigCompletedTout)

	ONEVENT( MFA_MEDIA_CONFIG_TIMER,  STARTUP,  CGideonSimMfaLogical::OnTimerMediaConfigTout)


	ONEVENT( MFA_MEDIA_CONFIG_TIMER,  CONNECT,  CGideonSimMfaLogical::OnTimerMediaConfigTout)
	ONEVENT( CARD_NOT_READY_DELAY_TIMER,   STARTUP,  CGideonSimMfaLogical::OnTimerCardsDelayTout)
	ONEVENT( MFA_SPEAKER_CHANGE_TIMER,CONNECT,  CGideonSimMfaLogical::OnTimerSpeakerChangeTout)
	ONEVENT( CM_EP_SIM_MSG,  CONNECT,  CGideonSimMfaLogical::OnCmProcessEpSimMsgConnect)
	ONEVENT( SET_UNIT_STATUS_FOR_KEEP_ALIVE_NEW,  CONNECT,  CGideonSimMfaLogical::SetUnitStatustSimMfaForKeepAliveInd)
	ONEVENT( CM_LOADED_TEST_TIMER, ANYCASE, CGideonSimMfaLogical::OnCmLoaded_test)
  ONEVENT(ISDN_CFG_TIMER,    ANYCASE,  CGideonSimMfaLogical::OnTimerIsdnCfg)//olga
  ONEVENT( SOFTWARE_UPGRADE_TIMER,  ANYCASE,  CGideonSimMfaLogical::OnTimerSoftwareUpgrade)
  ONEVENT( SOFTWARE_IPMC_UPGRADE_TIMER,  ANYCASE,  CGideonSimMfaLogical::OnTimerIpmcSoftwareUpgrade)
PEND_MESSAGE_MAP(CGideonSimMfaLogical, CGideonSimLogicalModule);


CGideonSimMfaLogical::CGideonSimMfaLogical(CTaskApp* pTask,WORD boardId,WORD subBoardId)
			: CGideonSimLogicalModule(pTask,boardId,subBoardId)
{
	m_nSpeakerIndex = MAX_AUDIO_PARTIES;

	m_cardType = eMfa_26;
	CCardCfgMfa* pCurrCardCfg = NULL;
	pCurrCardCfg = (CCardCfgMfa*)::GetGideonSystemCfg()->GetCardCfg( boardId );
	if (pCurrCardCfg)
		m_cardType = pCurrCardCfg->GetCardType();
	else
	{
		PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::CGideonSimMfaLogical - pCurrCardCfg is NULL!!");
		return;
	}
	// m_cardType = eMfa_26;

	m_units = new CSimMfaUnitsList( boardId, subBoardId );
	m_units->SetCardType( m_cardType );

	m_IC = new CIcComponent(this,pTask);
	m_TB = new CTbComponent(pTask);

	m_IsAudioControllerMaster = false;

	UNIT_S currUnit;
	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		currUnit = pCurrCardCfg->GetUnit(i);

		if (currUnit.unitId != 65535) //current unit is set
		{
			m_units->UpdateUnitStatus(i, currUnit.unitStatus);

			TRACEINTO << "UpdateUnitStatus() BoardId: " << boardId << " currUnit: " << i << " status: " << currUnit.unitStatus;
		}
	}

	m_software_upgrade_done = FALSE;
}

CGideonSimMfaLogical::~CGideonSimMfaLogical()
{
	POBJDELETE( m_units );
	POBJDELETE( m_IC );
	POBJDELETE( m_TB );
}

void CGideonSimMfaLogical::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}

void CGideonSimMfaLogical::ProcessEndpointsSimMsg(CSegment* pMsg)
{
	DispatchEvent(CM_EP_SIM_MSG,pMsg);
}

void CGideonSimMfaLogical::OnCmStartupIdle(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmStartupIdle");

	m_state = STARTUP;

	CardManagerLoadedInd();

	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmStartupIdle - Wait for CM_UNIT_CONFIG_REQ.");
}

void CGideonSimMfaLogical::OnCmStartupConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmStartupConnect!!!!!!!!!!!!!!!!");
	ReestablishConnectionInd();
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmStartupConnect - Wait for CM_UNIT_CONFIG_REQ.");
}

void CGideonSimMfaLogical::OnCmLoaded_test()
{
	CardManagerLoadedInd();
}

void CGideonSimMfaLogical::OnCmProcessMcmsReqStartup(CSegment* pMsg)
{
    PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqStartup");

    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
    pMplProtocol->DeSerialize(*pMsg);


    switch( pMplProtocol->getOpcode() )
    {
    case CARDS_NOT_READY_REQ:
    {
        // delay connection
        StartTimer(CARD_NOT_READY_DELAY_TIMER,MFA_CARDS_DELAY_TIME);

        PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqStartup - - CARDS_NOT_READY_REQ received, start timer.");
        break;
    }
    case CM_UNIT_CONFIG_REQ:
    {
        m_units->UnitsConfigReq( pMplProtocol );	// config the units type according to req

        StartTimer(MFA_UNIT_CONFIG_TIMER,::GetGideonSystemCfg()->GetMfaUnitConfigTime()*SECOND);
        PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqStartup - CM_UNIT_CONFIG_REQ received, start timer.");
        break;
    }
    case SYSTEM_CARDS_MODE_REQ:
    {
        PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqStartup - SYSTEM_CARDS_MODE_REQ received");
        break;
    }


    case CM_MEDIA_IP_CONFIG_REQ:
    case CM_RTM_MEDIA_IP_CONFIG_REQ:
    {
        // TODO : here is a MEDIA IP config
        // MEDIA_IP_PARAMS_S
        StartTimer(MFA_MEDIA_CONFIG_TIMER,::GetGideonSystemCfg()->GetMfaMediaConfigTime()*SECOND);

        OPCODE curOp = pMplProtocol->getOpcode();
        if (CM_MEDIA_IP_CONFIG_REQ == curOp)
        {
            PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqStartup - CM_MEDIA_IP_CONFIG_REQ received, start timer.");
        }
        else if (CM_RTM_MEDIA_IP_CONFIG_REQ == curOp)
        {
            PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqStartup - CM_RTM_MEDIA_IP_CONFIG_REQ received, start timer.");
        }

        break;
    }
    case CM_MEDIA_IP_CONFIG_END_REQ:
    {
        PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqStartup - CM_MEDIA_IP_CONFIG_END_REQ, end of config.");
        m_state = CONNECT;

        if( CPObject::IsValidPObjectPtr(m_pTaskApi) ) {
            CSegment*   pSeg = new CSegment;
            *pSeg	<< m_wBoardId
                    << m_wSubBoardId;
            m_pTaskApi->SendLocalMessage(pSeg,MFA_CONNECTED);
        }
        break;
    }
    case CM_KEEP_ALIVE_REQ:
    {
        Ack_keep_alive(*pMplProtocol,STATUS_OK);
        break;
    }
    case ETHERNET_SETTINGS_CONFIG_REQ:
    {
        PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqStartup - ETHERNET_SETTINGS_CONFIG_REQ received");
        break;
    }
    case CM_UPGRADE_NEW_VERSION_READY_REQ:
    {

        PASSERT(CM_UPGRADE_NEW_VERSION_READY_REQ);
        break;
    }
    case CM_UPGRADE_START_WRITE_IPMC_REQ:
    {
        PASSERT(CM_UPGRADE_START_WRITE_IPMC_REQ);
        break;
    }

    default:
    {
        TRACESTRFUNC(eLevelError) << "UNEXPECTED MESSAGE IN (STARTUP) STATE, opcode "
                                     << " (" << pMplProtocol->getOpcode() << ")";
        break;
    }
    }
    POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimMfaLogical::SendSlaveEncoderOpenInd( CMplMcmsProtocol& rMplProt ) const
{
    PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::SendSlaveEncoderOpenInd");
	ACK_IND_S  rAckStruct;
	memset(&rAckStruct,0,sizeof(ACK_IND_S));
	rAckStruct.ack_base.ack_opcode = TB_MSG_OPEN_PORT_REQ;
	rAckStruct.ack_base.status = STATUS_OK;
	rAckStruct.ack_base.ack_seq_num = rMplProt.getMsgDescriptionHeaderRequest_id();

	rMplProt.AddCommonHeader(VIDEO_SLAVE_ENCODER_OPEN_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(sizeof(ACK_IND_S),(char*)(&rAckStruct));

	SendToCmForMplApi(rMplProt);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimMfaLogical::SetUnitStatustSimMfaForKeepAliveInd(WORD UnitNum, STATUS status )
{
	if (status<NUM_OF_CARD_UNIT_STATUSES)
	{
		if (UnitNum < MAX_NUM_OF_UNITS)
		{
			m_units->UpdateUnitStatus( UnitNum, status );
//vb			//m_GideonSimMfaUnitsForKeepAliveInd.statusOfUnitsList[UnitNum]=status;
		}
		else
			PTRACE(eLevelError,"CGideonSimMfaLogical::SetUnitStatustSimMfaForKeepAliveInd: illegal unit #");
	}
	else
	    PTRACE(eLevelError,"CGideonSimMfaLogical::SetUnitStatustSimMfaForKeepAliveInd: illegal status");

	PTRACE(eLevelError,"CGideonSimMfaLogical::SetUnitStatustSimMfaForKeepAliveInd: Shlomit+++++++-------");
}

void CGideonSimMfaLogical::Ack_keep_alive( CMplMcmsProtocol& rMplProt, STATUS status )
{
	KEEP_ALIVE_S  tKeepAliveInd;
	memset(&tKeepAliveInd,0,sizeof(KEEP_ALIVE_S));

	rMplProt.AddCommonHeader(CM_KEEP_ALIVE_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	m_units->GetUnitsStatus( &tKeepAliveInd );	// gets the units status
	rMplProt.AddData(sizeof(KEEP_ALIVE_S),(char*)(&tKeepAliveInd));

	SendToCmForMplApi(rMplProt);
}

void CGideonSimMfaLogical::SetBndLclAlignmentTimer( DWORD bndRmtLclAlignment )
{
	g_bondingAlignmentTime = bndRmtLclAlignment;
}

void CGideonSimMfaLogical::EstablishConnectionInd()
{
	PTRACE(eLevelInfoNormal,"CSimSwitchCard::EstablishConnectionInd - SEND to MPL-API.");

	// prepare & send ESTABLISH_CONNECTION
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	ESTABLISH_CONNECTION_S  rStruct;
	rStruct.dummy = 111;
	FillMplProtocol( pMplProtocol,
		ESTABLISH_CONNECTION,(BYTE*)(&rStruct),sizeof(ESTABLISH_CONNECTION_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimMfaLogical::ReestablishConnectionInd()
{
	PTRACE(eLevelInfoNormal,"CSimSwitchCard::ReestablishConnectionInd - SEND to MPL-API.");

	// prepare & send CM_CARD_MNGR_RECONNECT_IND
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	CM_CARD_MANAGER_RECONNECT_S  rStruct;
	memset(&rStruct,0,sizeof(CM_CARD_MANAGER_RECONNECT_S));

	string mplSerialNum = ::GetGideonSystemCfg()->GetBoxChassisId();
	int len = mplSerialNum.length();
	memcpy(rStruct.serialNum, ::GetGideonSystemCfg()->GetBoxChassisId(), len);


	FillMplProtocol( pMplProtocol,
		CM_CARD_MNGR_RECONNECT_IND,(BYTE*)(&rStruct),sizeof(CM_CARD_MANAGER_RECONNECT_S));
		//CARD_MANAGER_RECONNECT,(BYTE*)(&rStruct),sizeof(CARD_MANAGER_RECONNECT_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

void CGideonSimMfaLogical::OnCmProcessMcmsReqConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect");

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pMsg);

	eProductFamily curProductFamily  = CProcessBase::GetProcess()->GetProductFamily();	// for Call Generator

	/* temp for Talya - start */
	DWORD  opcode = pMplProtocol->getOpcode();
	TRACEINTO	<< " CGideonSimMfaLogical::OnCmProcessMcmsReqConnect opcode:" << opcode;
	eLogicalResourceTypes lrt = (eLogicalResourceTypes)pMplProtocol->getPortDescriptionHeaderLogical_resource_type_1();
	switch( opcode ) {

		// common for ART / video Enc / video Dec
		case TB_MSG_OPEN_PORT_REQ:
		{
			eResourceTypes  resType = (eResourceTypes)pMplProtocol->getPhysicalInfoHeaderResource_type();
			// if port  is audio - send message to Endpoints Sim
			if( resType == ePhysical_art  ||  resType == ePhysical_art_light )
			{

				if(lrt==eLogical_relay_avc_to_svc_rtp_with_audio_encoder ||
						lrt==eLogical_relay_avc_to_svc_rtp )
				{
					TRACEINTO<<"!@# logicalResourceType:"<<lrt;
				}
				else
				{
					CAudioParty*  pParty = FindParty(*pMplProtocol);
					if( pParty == NULL )
					{
						for( int i=0; i<MAX_AUDIO_PARTIES && pParty == NULL; i++ )
							if( m_raAudioPartiesArr[i].IsEmpty() )
								pParty = &(m_raAudioPartiesArr[i]);
					}
					if( pParty != NULL )
					{
						pParty->Create(*pMplProtocol);
						// forward to E.P. app
						ForwardAudioMsg2Endpoints(opcode, pParty);
					    // forward to MM process if the system is CG
						if (eProductFamilyCallGenerator == curProductFamily)
							ForwardMsg2MediaMngr(*pMplProtocol);	// sends message to MM
					}

					m_units->OnMcmsCommandReq( pMplProtocol );
				}



			}
			else if (resType == ePhysical_video_encoder || resType == ePhysical_video_decoder )
			{
				m_units->OnMcmsCommandReq( pMplProtocol );
			    // forward to MM process if the system is CG
				if (eProductFamilyCallGenerator == curProductFamily)
					ForwardMsg2MediaMngr(*pMplProtocol);	// sends message to MM
			}

			// Send ACK to ConfParty
			CMplMcmsProtocol* pMplProtocol2 = new CMplMcmsProtocol(*pMplProtocol);
			Ack(*pMplProtocol, STATUS_OK);

			if( resType == ePhysical_video_encoder )
			{
				ENCODER_PARAM_S* pReq = (ENCODER_PARAM_S*)pMplProtocol2->GetData();
				if(E_VIDEO_ENCODER_SLAVE_FULL_ENCODER == pReq->nVideoEncoderType )
				{
					SendSlaveEncoderOpenInd(*pMplProtocol2);
				}
			}
            POBJDELETE(pMplProtocol2);
			break;
		}
		case TB_MSG_CLOSE_PORT_REQ:
		{
			// if port  is audio - send message to Endpoints Sim
			if( pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art  ||
				pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art_light )
			{

				if(lrt==eLogical_relay_avc_to_svc_rtp_with_audio_encoder ||
						lrt==eLogical_relay_avc_to_svc_rtp )
				{
					TRACEINTO<<"!@# logicalResourceType:"<<lrt;
				}
				else
				{
					CAudioParty*  pParty = FindParty(*pMplProtocol);
					if( pParty != NULL )
					{
						// forward to E.P. app
						ForwardAudioMsg2Endpoints(opcode,pParty);

						// sends message to MM in order to remove party if the system is CG
						if (eProductFamilyCallGenerator == curProductFamily)
							ForwardMsg2MediaMngr(*pMplProtocol);
						// clean party
						pParty->Cleanup();
					}
					m_units->OnMcmsCommandReq( pMplProtocol );
				}


			}
			else if (pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_video_encoder ||
					 pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_video_decoder )
			{
				m_units->OnMcmsCommandReq( pMplProtocol );
			}

			// Send ACK to ConfParty
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
		case TB_MSG_CONNECT_REQ:
		{
			TB_MSG_CONNECT_S  tConnect;
			memcpy(&tConnect,pMplProtocol->GetData(),sizeof(TB_MSG_CONNECT_S));

			Ack(*pMplProtocol,STATUS_OK);
			//  if TB_MSG_CONNECT_REQ to video decoder, send DECODER_SYNC_IND
			if( tConnect.physical_port1.physical_id.resource_type == ePhysical_video_decoder )
			{
				DECODER_SYNC_IND_S   tSyncStruct;
				memset(&tSyncStruct,0,sizeof(DECODER_SYNC_IND_S));
				tSyncStruct.nStatus = STATUS_OK;

				pMplProtocol->AddCommonHeader(VIDEO_DECODER_SYNC_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

				pMplProtocol->AddData(sizeof(DECODER_SYNC_IND_S),(char*)(&tSyncStruct));

				SendToCmForMplApi(*pMplProtocol);
			}
			break;
		}
		///////////////////
		// ART - audio unit
		///////////////////
//		case ACTIVATE_PORT_REQ:
//		case DEACTIVATE_PORT_REQ:

		case AUDIO_OPEN_DECODER_REQ:
		{
			// if port  is audio - send message to Endpoints Sim
			if( pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art  ||
				pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art_light )
			{
				CAudioParty*  pParty = FindParty(*pMplProtocol);
				if( pParty != NULL )
				{
					pParty->Create(*pMplProtocol);
				}
			}
				// start timer for first party only
			//if( FindTimer(MFA_SPEAKER_CHANGE_TIMER,FALSE) == NULL )
			if( ! IsValidTimer(MFA_SPEAKER_CHANGE_TIMER)  &&  0 != ::GetGideonSystemCfg()->GetMfaSpeakerChangeTime() )
				StartTimer(MFA_SPEAKER_CHANGE_TIMER,::GetGideonSystemCfg()->GetMfaSpeakerChangeTime()*SECOND);
				// send ACK to ConfParty
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
		case AUDIO_OPEN_ENCODER_REQ:
		case AUDIO_CLOSE_ENCODER_REQ:
		case AUDIO_CLOSE_DECODER_REQ:
		case AUDIO_UPDATE_ALGORITHM_REQ:
		case AUDIO_UPDATE_DECODER_REQ:
		case AUDIO_UPDATE_GAIN_REQ:
		case AUDIO_UPDATE_MUTE_REQ:
		case AUDIO_UPDATE_CONNECTION_STATUS_REQ:
		case AUDIO_UPDATE_BITRATE_REQ:
		case AC_OPEN_CONF_REQ:
		case AC_CLOSE_CONF_REQ:
		case AC_UPDATE_CONF_PARAMS_REQ:
		case CG_PLAY_AUDIO_REQ:
		{
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}



		////////////////////////////////
		// ART - video decoder / encoder
		////////////////////////////////
		case VIDEO_ENCODER_CHANGE_LAYOUT_REQ:
		case VIDEO_ENCODER_CHANGE_LAYOUT_ATTRIBUTES_REQ:
		case VIDEO_ENCODER_UPDATE_PARAM_REQ:
		case VIDEO_FAST_UPDATE_REQ:
		{
		    // forward to MM process if the system is CG
			if ((eProductFamilyCallGenerator == curProductFamily) &&
				(VIDEO_FAST_UPDATE_REQ == opcode  || VIDEO_ENCODER_UPDATE_PARAM_REQ == opcode))
				ForwardMsg2MediaMngr(*pMplProtocol);	// sends message to MM
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
/*		case CONF_PARTY_MRMP_OPEN_CHANNEL_REQ:
		{
		    PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect CONF_PARTY_MRMP_OPEN_CHANNEL_REQ");
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}*/
/*		case CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ:
		{
		    PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ");
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}*/
		/////////////////
		// ART - RTP unit
		/////////////////
		case CONFPARTY_CM_OPEN_UDP_PORT_REQ:
		{
			CAudioParty*  pParty = FindParty(*pMplProtocol);
			if( pParty != NULL )
			{
				pParty->SetRtpId(pMplProtocol->getPortDescriptionHeaderConnection_id());
			    // forward to MM process if the system is CG
				if (eProductFamilyCallGenerator == curProductFamily)
					ForwardMsg2MediaMngr(*pMplProtocol);	// sends message to MM
			}
			else
				DBGPASSERT(pMplProtocol->getPortDescriptionHeaderConnection_id()+1000);

			AckIp(*pMplProtocol,STATUS_OK);
			break;
		}
		case H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		case CONFPARTY_CM_UPDATE_UDP_ADDR_REQ:
		case CONFPARTY_CM_CLOSE_UDP_PORT_REQ:
		case H323_RTP_UPDATE_CHANNEL_REQ:
		case H323_RTP_STREAM_ON_REQ:
		case H323_RTP_STREAM_OFF_REQ:
		case SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		case SIP_RTP_UPDATE_CHANNEL_REQ:
		case SIP_CM_OPEN_UDP_PORT_REQ:
		case SIP_CM_UPDATE_UDP_ADDR_REQ:
		case SIP_CM_CLOSE_UDP_PORT_REQ:
		case SIP_RTP_STREAM_ON_REQ:
		case SIP_RTP_STREAM_OFF_REQ:
		case ART_CONTENT_ON_REQ:
		case ART_CONTENT_OFF_REQ:
		case ART_EVACUATE_REQ:
		case CONF_PARTY_MRMP_OPEN_CHANNEL_REQ:
		case CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ:
		case IP_CM_DTLS_CLOSE_REQ:
		{
		    // forward to MM process if the system is CG
			if ((eProductFamilyCallGenerator == curProductFamily) &&
				(((H323_RTP_UPDATE_CHANNEL_REQ 			== opcode) ||
				(SIP_RTP_UPDATE_CHANNEL_REQ				== opcode) ||
				(H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ 	== opcode) ||
				(SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ 	== opcode) ||
				(SIP_CM_OPEN_UDP_PORT_REQ				== opcode) ||
				(CONFPARTY_CM_CLOSE_UDP_PORT_REQ		== opcode) ||
				(SIP_CM_CLOSE_UDP_PORT_REQ				== opcode) ||
				(ART_CONTENT_ON_REQ						== opcode) ||
				(ART_CONTENT_OFF_REQ					== opcode) ||
				(IP_CM_DTLS_CLOSE_REQ					== opcode))))
			{
				ForwardMsg2MediaMngr(*pMplProtocol);	// sends message to MM
			}
			AckIp(*pMplProtocol,STATUS_OK);
			break;
		}
		case RTP_PARTY_VIDEO_CHANNELS_STATISTICS_REQ: //CDR_MCCF
		{
		    // sends ack to MCMS
		    AckIp(*pMplProtocol,STATUS_OK);
		    if( pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art  ||
		        pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art_light )
		    {
		        // sends Recording Indication to MCMS
		        TCmPartyInfoStatisticsInd tStatisticInd;
		        memset( &tStatisticInd, 0, sizeof(TCmPartyInfoStatisticsInd) );
		        FillRtpStatisticInfoStruct(&tStatisticInd);
		        pMplProtocol->AddCommonHeader( IP_CM_PARTY_VIDEO_CHANNELS_STATISTICS_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );
		        pMplProtocol->AddData( sizeof(TCmPartyInfoStatisticsInd), (char*)(&tStatisticInd) );
		        SendToCmForMplApi(*pMplProtocol);   // send to MCMS
		    }
		    break;
		}
		case H323_RTP_PARTY_MONITORING_REQ:
		{
			// sends ack to MCMS
			AckIp(*pMplProtocol,STATUS_OK);
			if( pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art  ||
				pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art_light )
			{
				// sends Recording Indication to MCMS
				TCmPartyMonitoringInd   tMonitorInd;
				memset( &tMonitorInd, 0, sizeof(TCmPartyMonitoringInd) );
				FillRtpMonitoringStruct(&tMonitorInd);
				pMplProtocol->AddCommonHeader( IP_CM_PARTY_MONITORING_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );
				pMplProtocol->AddData( sizeof(TCmPartyMonitoringInd), (char*)(&tMonitorInd) );
				SendToCmForMplApi(*pMplProtocol);	// send to MCMS

			}
			break;
		}
		case H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		case H320_RTP_UPDATE_CHANNEL_REQ:
		case H320_RTP_UPDATE_CHANNEL_RATE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle BCH request");
			Ack(*pMplProtocol, STATUS_OK);
			break;
		}
		case IP_CM_RTCP_VIDEO_PREFERENCE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Do nothing....");
		}
		case IVR_PLAY_MESSAGE_REQ:
		{
			{
				WORD isConfMessage = IsConfPlayMessage(*pMplProtocol);
				if (0 == isConfMessage)
				{
					CAudioParty*  pParty = FindParty(*pMplProtocol);
					if( pParty != NULL )
					{
						// forward to E.P. app
						ForwardAudioMsg2Endpoints(opcode, pParty, (BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
						m_IC->OnMcmsReq( pMplProtocol );
					}
					else
						PTRACE(eLevelError,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Party not found for IVR_PLAY_MESSAGE_REQ");
				}
				else
				{
					ForwardAudioMsg2Endpoints(opcode, NULL, (BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
				}
			}
			m_IC->AddIvrPlayMsgToQ(*pMplProtocol);
			break;
		}
		case IVR_STOP_PLAY_MESSAGE_REQ:
		{
			{
				CAudioParty*  pParty = FindParty(*pMplProtocol);
				if( pParty != NULL )
				{
					// forward to E.P. app
					ForwardAudioMsg2Endpoints(opcode,pParty,
							(BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
				}
			}
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
		case IVR_PLAY_MUSIC_REQ:
		{
			CAudioParty*  pParty = FindParty(*pMplProtocol);
			if( pParty != NULL )
			{
				// forward to E.P. app
				ForwardAudioMsg2Endpoints(opcode,pParty,
						(BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
			}
			else if( 1 == IsConfPlayMessage(*pMplProtocol) )
			{
				ForwardAudioMsg2Endpoints(opcode, NULL,
						(BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
			}
			else
			{
				DBGPASSERT(1000000+pMplProtocol->getPortDescriptionHeaderConf_id());
				DBGPASSERT(1000000+pMplProtocol->getPortDescriptionHeaderParty_id());
			}

			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
		case IVR_STOP_PLAY_MUSIC_REQ:
		{
			CAudioParty*  pParty = FindParty(*pMplProtocol);
			if( pParty != NULL )
			{
				// forward to E.P. app
				ForwardAudioMsg2Endpoints(opcode,pParty,
						(BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
			}
			else if( 1 == IsConfPlayMessage(*pMplProtocol) )
			{
				ForwardAudioMsg2Endpoints(opcode, NULL,
						(BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
			}
			else
			{
				DBGPASSERT(1000000+pMplProtocol->getPortDescriptionHeaderConf_id());
				DBGPASSERT(1000000+pMplProtocol->getPortDescriptionHeaderParty_id());
			}

			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
		case IVR_RECORD_ROLL_CALL_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle IVR_RECORD_ROLL_CALL_REQ");

			CMplMcmsProtocol*  pAkcProt = new CMplMcmsProtocol(*pMplProtocol);
			Ack(*pAkcProt,STATUS_OK);
			POBJDELETE(pAkcProt);

			CAudioParty*  pParty = FindParty(*pMplProtocol);
			if( pParty != NULL )
			{
				// forward to E.P. app
				ForwardAudioMsg2Endpoints(opcode, pParty, (BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());

		//		SystemSleep(100*5);

				// creates RollCall file on disk
				m_IC->OnMcmsReq( pMplProtocol );

				// sends Recording Indication to MCMS
///
				SIVRRecordMessageIndStruct   rollCallStructInd;
				memset( &rollCallStructInd, 0, sizeof(SIVRRecordMessageIndStruct) );
				rollCallStructInd.status = STATUS_OK;
				rollCallStructInd.recordingLength = 3;
				pMplProtocol->AddCommonHeader( IVR_RECORD_ROLL_CALL_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );
				pMplProtocol->AddData( sizeof(SIVRRecordMessageIndStruct), (char*)(&rollCallStructInd) );
				SendToCmForMplApi(*pMplProtocol);	// send to MCMS
///
				m_IC->AddIvrRollcallRecordToQ(*pMplProtocol);
			}
			else
				PTRACE(eLevelError,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Party not found for IVR_RECORD_ROLL_CALL_REQ");

			break;
		}
		case IVR_STOP_RECORD_ROLL_CALL_REQ:
		{
			{
				CAudioParty*  pParty = FindParty(*pMplProtocol);
				if( pParty != NULL )
				{
					// forward to E.P. app
					ForwardAudioMsg2Endpoints(opcode,pParty,
							(BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
				}
			}
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
//		case IVR_MUSIC_ADD_SOURCE_REQ:
		case IVR_ADD_MUSIC_SOURCE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle IVR_ADD_MUSIC_SOURCE_REQ (NULL).");
			break;
		}
		
		// handle dtmf requests  
		case AUDIO_PLAY_TONE_REQ:
		{
		    // forward to MM process if the system is CG
			if (eProductFamilyCallGenerator == curProductFamily)
			{
				PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle AUDIO_PLAY_TONE_REQ - forward to Media Manager");

				ForwardMsg2MediaMngr(*pMplProtocol);	// sends message to MM

				AckIp(*pMplProtocol,STATUS_OK);
			}
			
			break;
		}
		case MOVE_RSRC_REQ:
		{
			if( pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art  ||
				pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art_light )
			{
				CAudioParty*  pParty = FindParty(*pMplProtocol);
				if( pParty != NULL )
				{
					// update ConfId of party
					MOVE_RESOURCES_PARAMS_S*  pRequest = (MOVE_RESOURCES_PARAMS_S*)pMplProtocol->GetData();
					// forward to E.P. app
					ForwardAudioMsg2Endpoints(opcode,pParty,
							(BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
					// set new value of destination conf from MOVE fields
					pParty->SetConfId(pRequest->newConfId);
					char  szString[32];
					sprintf(szString,"<%d>",pRequest->newConfId);
					PTRACE2(eLevelInfoNormal,
						"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle MOVE_RSRC_REQ. New conf ID ",
						szString);
				}
			}
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
		case STARTUP_DEBUG_RECORDING_PARAM_REQ: //CARDS_CM_MEDIA_RECORDING_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle STARTUP_DEBUG_RECORDING_PARAM_REQ.");
			break;
		}
		case CM_KEEP_ALIVE_REQ:
		{
			Ack_keep_alive(*pMplProtocol,STATUS_OK);
			break;
		}
		case ETHERNET_SETTINGS_CONFIG_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - ETHERNET_SETTINGS_CONFIG_REQ received");
			break;
		}
                case AC_TYPE_REQ:
		{
			//TRACEINTO << "OnCmProcessMcmsReqConnect " <<  pMplProtocol->getOpcode();
			PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle AC_TYPE_REQ.");

			//save audio controller role master|shadow
			TAcTypeReq*  pRequest = (TAcTypeReq*)pMplProtocol->GetData();
			if (pRequest->eAudioContollerType == E_AUDIO_CONTROLLER_TYPE_MASTER)
			{
				m_IsAudioControllerMaster = true;
				st_AudioControllerBoardId = m_wBoardId;
				st_AudioControllerSubBoardId = m_wSubBoardId;
				TRACEINTO	<< " CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle AC_TYPE_REQ - Master! BoardId="
					<< (int)m_wBoardId
					<< " Sub BoardId="
					<< (int)m_wSubBoardId;
			}

			//Ack to Cards Process
			Ack(*pMplProtocol, STATUS_OK);

			break;
		}

		/////////////////
		// ART - MUX unit
		/////////////////

		case BND_CONNECTION_INIT:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle BND_CONNECTION_INIT.");

			BND_CONNECTION_INIT_REQUEST_S*  pRequest = (BND_CONNECTION_INIT_REQUEST_S*)pMplProtocol->GetData();

			ForwardMuxBndReq2Endpoints(*pMplProtocol);

			break;
		}

		case BND_ADD_CHANNEL:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle BND_ADD_CHANNEL.");

			BYTE numOfChannels = 0;
			BOOL bBndAllChannelsConnected = FALSE;
			int status = m_units->AddBondingChannel( pMplProtocol, &numOfChannels, &bBndAllChannelsConnected );
			if (0 != status)
			{
				TRACESTR(eLevelError) << " CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Add Bonding Channel error";
				break;	// error
			}
			if (bBndAllChannelsConnected)	// all Bonding channels were connected, need to send "BND_REMOTE_LOCAL_ALIGNMENT" command
			{
				TRACESTR(eLevelInfoNormal) << " CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - BND_ADD_CHANNEL: All channels connected";

				SystemSleep( g_bondingAlignmentTime );	// need to be a timer

				///	BND_REMOTE_LOCAL_ALIGNMENT
				DWORD partyId 	= pMplProtocol->getPortDescriptionHeaderParty_id();
				DWORD confId 	= pMplProtocol->getPortDescriptionHeaderConf_id();
				DWORD connId 	= pMplProtocol->getPortDescriptionHeaderConnection_id();

				BND_REMOTE_LOCAL_ALIGNMENT_INDICATION_S pRemoteLocalAlignmentInd;

				pRemoteLocalAlignmentInd.indicator 		= 0;
				pRemoteLocalAlignmentInd.numOfChannels 	= numOfChannels;
				pRemoteLocalAlignmentInd.dummy[0] 		= (BYTE)(-1);
				pRemoteLocalAlignmentInd.dummy[1] 		= (BYTE)(-1);

				CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;

				FillMplProtocol(pMplProt, BND_REMOTE_LOCAL_ALIGNMENT, (BYTE*)(&pRemoteLocalAlignmentInd), sizeof(BND_REMOTE_LOCAL_ALIGNMENT_INDICATION_S));
				pMplProt->AddPortDescriptionHeader( partyId, confId, connId, eLogical_mux);

				SendToCmForMplApi(*pMplProt);

				POBJDELETE( pMplProt );

				return;
			}

			break;

			break;
		}

		case BND_ABORT_CALL:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle BND_ABORT_CALL.");
			break;
		}

		case H221_INIT_COMM:
		{
			///TEMP
			// Send END_INIT_COMM as a response
			TRACEINTO	<< " CGideonSimMfaLogical::OnCmProcessMcmsReqConnect opcode: "
						<< "H221_INIT_COMM";

			if( ::GetGideonSystemCfg()->GetIsdnCapsFlag() ) {

			    CBehaviourIsdn* behaviourIsdn = ::GetGideonSystemCfg()->GetBehaviourIsdn();
				const EP_SGN_S* pSignalStr = behaviourIsdn ? behaviourIsdn->GetFirstSignal() : NULL;
				if(pSignalStr && ( END_INIT_COMM == pSignalStr->opcode )) {//olga

				  CSegment* pParamSeg = new CSegment;
				  pMplProtocol->Serialize(*pParamSeg);
				  StartTimer(ISDN_CFG_TIMER, pSignalStr->timer, pParamSeg);
				  break;
				}
			}
			DWORD connectionId = pMplProtocol->getPortDescriptionHeaderConnection_id();
			DWORD partyId = pMplProtocol->getPortDescriptionHeaderParty_id();
			DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();

			CAPABILITIES_S  receivedCaps;
	  		char* pData = pMplProtocol->GetData();
			memcpy(&receivedCaps, pData, sizeof(CAPABILITIES_S));
			DWORD numOfBytes = receivedCaps.caps_bas.number_of_bytes;

	 	 	pData = pData + sizeof(CAPABILITIES_S);

	 	 	CSegment capsSeg;
  			capsSeg.Put((BYTE*)pData, numOfBytes);

			CSegment* pEndInitComm = new CSegment;
			*pEndInitComm 	<< numOfBytes
							<< capsSeg
							<< (DWORD)0;


			FillMplProtocol(pMplProtocol, END_INIT_COMM, (BYTE*)(pEndInitComm->GetPtr()), pEndInitComm->GetWrtOffset());
			pMplProtocol->AddPortDescriptionHeader(partyId, confId, connectionId, eLogical_mux);

			SendToCmForMplApi(*pMplProtocol);

			POBJDELETE(pEndInitComm);

			//////////////////////
			// Send REMOTE_XMIT_MODE
			TRACEINTO	<< " CGideonSimMfaLogical::OnCmProcessMcmsReqConnect opcode: "
						<< "Send REMOTE_XMIT_MODE - Gideon Sim initiative";

	 	 	DWORD numBytes = 12;
	 	 	CSegment xmitModeSeg;

			CSegment* pRemoteXmitMode = new CSegment;
			*pRemoteXmitMode 	<< numBytes
								<< (BYTE)0x26 << (BYTE)0x13 << (BYTE)0x40 << (BYTE)0x5a << (BYTE)0x47 << (BYTE)0x5c
								<< (BYTE)0x60 << (BYTE)0x70 << (BYTE)0xf0 << (BYTE)0x60 << (BYTE)0xf0 << (BYTE)0x6e;

			FillMplProtocol(pMplProtocol, REMOTE_XMIT_MODE, (BYTE*)(pRemoteXmitMode->GetPtr()), pRemoteXmitMode->GetWrtOffset());
			pMplProtocol->AddPortDescriptionHeader(partyId, confId, connectionId, eLogical_mux);

			SendToCmForMplApi(*pMplProtocol);

			POBJDELETE(pRemoteXmitMode);

			break;
			///TEMP
		}

		case SET_XMIT_MODE:
		{
			///TEMP
			// Send REMOTE_XMIT_MODE as a response
			TRACEINTO	<< " CGideonSimMfaLogical::OnCmProcessMcmsReqConnect opcode: "
						<< "SET_XMIT_MODE";

		    if( ::GetGideonSystemCfg()->GetIsdnXmitModeFlag() ) //olga
			    break;

			DWORD connectionId = pMplProtocol->getPortDescriptionHeaderConnection_id();
			DWORD partyId = pMplProtocol->getPortDescriptionHeaderParty_id();
			DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();

			SET_XMIT_MODE_S  setXmitModeStruct;
	  		char* pData = pMplProtocol->GetData();
			memcpy(&setXmitModeStruct, pData, sizeof(SET_XMIT_MODE_S));
			DWORD numOfBytes = setXmitModeStruct.xmit_mode.comm_mode_bas.number_of_bytes;

	 	 	pData = pData + sizeof(SET_XMIT_MODE_S);

	 	 	CSegment xmitModeSeg;
  			xmitModeSeg.Put((BYTE*)pData, numOfBytes);

			CSegment* pRemoteXmitMode = new CSegment;
			*pRemoteXmitMode 	<< numOfBytes
								<< xmitModeSeg;


			FillMplProtocol(pMplProtocol, REMOTE_XMIT_MODE, (BYTE*)(pRemoteXmitMode->GetPtr()), pRemoteXmitMode->GetWrtOffset());
			pMplProtocol->AddPortDescriptionHeader(partyId, confId, connectionId, eLogical_mux);

			SendToCmForMplApi(*pMplProtocol);

			POBJDELETE(pRemoteXmitMode);
			break;
			///TEMP
		}

		case EXCHANGE_CAPS:
		{
			///TEMP
			// Send REMOTE_BAS_CAPS as a response
			TRACEINTO	<< " CGideonSimMfaLogical::OnCmProcessMcmsReqConnect opcode: "
						<< "EXCHANGE_CAPS";

			if( ::GetGideonSystemCfg()->GetIsdnCapsFlag() ) //olga
			    break;

			DWORD connectionId = pMplProtocol->getPortDescriptionHeaderConnection_id();
			DWORD partyId = pMplProtocol->getPortDescriptionHeaderParty_id();
			DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();

			EXCHANGE_CAPS_S  exchangeCapsStruct;
	  		char* pData = pMplProtocol->GetData();
			memcpy(&exchangeCapsStruct, pData, sizeof(EXCHANGE_CAPS_S));
			DWORD numOfBytes = exchangeCapsStruct.local_caps.caps_bas.number_of_bytes;

	 	 	pData = pData + sizeof(EXCHANGE_CAPS_S);

	 	 	CSegment exchangeCapsSeg;
  			exchangeCapsSeg.Put((BYTE*)pData, numOfBytes);

			CSegment* pRemoteBasCaps = new CSegment;
			*pRemoteBasCaps 	<< numOfBytes
								<< exchangeCapsSeg;


			FillMplProtocol(pMplProtocol, REMOTE_BAS_CAPS, (BYTE*)(pRemoteBasCaps->GetPtr()), pRemoteBasCaps->GetWrtOffset());
			pMplProtocol->AddPortDescriptionHeader(partyId, confId, connectionId, eLogical_mux);

			SendToCmForMplApi(*pMplProtocol);

			POBJDELETE(pRemoteBasCaps);
			break;
			///TEMP
		}
		case SEND_H_230:
		{
			TRACEINTO	<< " CGideonSimMfaLogical::OnCmProcessMcmsReqConnect opcode: "
						<< "SEND_H_230";
			ForwardMuxBndReq2Endpoints(*pMplProtocol);
			break;
		}

		case ART_FIPS_140_REQ:
		{
			DWORD retStatus = E_FIPS_IND_STATUS_OK;
			BYTE* pData   = (BYTE*)(pMplProtocol->GetData());
			if (NULL == pData)
				break;

			switch( *pData )
			{
				case E_ART_FIPS140_SIMULATION_ERR_CODE_INACTIVE:
					break;

				case E_ART_FIPS140_SIMULATION_ERR_CODE_FAIL_DETERMINISTIC_TEST:		// error simulation
				{
					retStatus = E_FIPS_IND_STATUS_ERROR_SHA1_FAILURE;
					break;
				}

 				default: {break;}
			}

			SystemSleep(100);	// 1 second delay

			pMplProtocol->AddCommonHeader( ART_FIPS_140_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );
			pMplProtocol->AddData( 4, (char*)(&retStatus) );
			SendToCmForMplApi(*pMplProtocol);	// send to MCMS

			break;
		}

	case CM_UPGRADE_NEW_VERSION_READY_REQ:
		{
		  // sagi add send ack for this request
      PASSERT(CM_UPGRADE_NEW_VERSION_READY_REQ);
			break;
		}

	case CM_UPGRADE_START_WRITE_IPMC_REQ:
		{
		  PASSERT(CM_UPGRADE_START_WRITE_IPMC_REQ);
			break;
		}

	case H323_RTP_UPDATE_MT_PAIR_REQ:
	{	// we use this opcode for forwarding party's alias name to MM (for "CG audio improvements" feature)
		// forward to MM process if the system is CG
		if (eProductFamilyCallGenerator == curProductFamily)
			ForwardMsg2MediaMngr(*pMplProtocol);	// sends message to MM

		Ack(*pMplProtocol, STATUS_OK);
		break;
	}

	case IP_CM_DTLS_START_REQ:
		PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessMcmsReqConnect - Handle IP_CM_DTLS_START_REQ");
		SendDtlsEndInd(*pMplProtocol);// IP_CM_DTLS_END_IND
		break;


		// status FAIL if request unrecognized
		default:
		{
			Ack(*pMplProtocol,STATUS_OK);
//			Ack(*pMplProtocol,STATUS_FAIL);
			break;
		}
	}
	/* temp for Talya - end */

	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimMfaLogical::OnCmProcessEpSimMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnCmProcessEpSimMsgConnect");

	DWORD	opcode;
	WORD	boardId, subBoardId, unitId, portId;

	*pMsg	>> opcode
			>> boardId
			>> subBoardId
			>> unitId;

	CAudioParty* pTempParty = new CAudioParty;
	pTempParty->DeSerialize(*pMsg);

	CAudioParty* pParty = FindParty(*pTempParty);
	if( pParty != NULL )
	{
		switch( opcode ) {
			case AUD_DTMF_IND_VAL: {
				TAudioDtmfEventInd   tStruct;
				int size = sizeof(TAudioDtmfEventInd);
				pMsg->Get((BYTE*)&tStruct,size);

				CMplMcmsProtocol*  pMplProt = new CMplMcmsProtocol;

				pMplProt->AddCommonHeader(AUD_DTMF_IND_VAL,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
				pMplProt->AddPhysicalHeader(0,m_wBoardId,m_wSubBoardId,0,0,0,ePhysical_art_light);
				pMplProt->AddPortDescriptionHeader(pParty->GetPartyId(),pParty->GetConfId(),pParty->GetConnectionId());

				pMplProt->AddData(size,(char*)&tStruct);

				SendToCmForMplApi(*pMplProt);
				POBJDELETE(pMplProt);
				break;
			}
			case IP_RTP_DTMF_INPUT_IND: {
				TRtpDtmfRequestInd   tRtpDtmfStruct;
				int size = sizeof(TRtpDtmfRequestInd);
				pMsg->Get((BYTE*)&tRtpDtmfStruct,size);

				CMplMcmsProtocol*  pMplProt = new CMplMcmsProtocol;

				pMplProt->AddCommonHeader(IP_RTP_DTMF_INPUT_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
				pMplProt->AddPhysicalHeader(0,m_wBoardId,m_wSubBoardId,0,0,0,ePhysical_art_light);
				pMplProt->AddPortDescriptionHeader(pParty->GetPartyId(),pParty->GetConfId(),pParty->GetConnectionIdRtp());

				pMplProt->AddData(size,(char*)&tRtpDtmfStruct);

				SendToCmForMplApi(*pMplProt);
				POBJDELETE(pMplProt);
				break;
			}
			case AC_ACTIVE_SPEAKER_IND: {
				SendActiveSpeakerInd(*pParty);
				break;
			}
			case AC_AUDIO_SPEAKER_IND: {
				SendAudioSpeakerInd(*pParty);
				break;
			}
			case IP_RTP_FECC_TOKEN_IND:
			{
				TRtpFeccTokenRequestInd   tStruct;
				int size = sizeof(TRtpFeccTokenRequestInd);
				pMsg->Get((BYTE*)&tStruct,size);
				tStruct.unChannelType      = kIpFeccChnlType;
				tStruct.unChannelDirection = cmCapReceive;

				CMplMcmsProtocol*  pMplProt = new CMplMcmsProtocol;

				pMplProt->AddCommonHeader(IP_RTP_FECC_TOKEN_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
				pMplProt->AddPhysicalHeader(0,m_wBoardId,m_wSubBoardId,0,0,0,ePhysical_art_light);
				pMplProt->AddPortDescriptionHeader(pParty->GetPartyId(),pParty->GetConfId(),pParty->GetConnectionIdRtp());

				pMplProt->AddData(size,(char*)&tStruct);

				SendToCmForMplApi(*pMplProt);
				POBJDELETE(pMplProt);
				break;
			}

			case BND_END_NEGOTIATION:
			{
				*pMsg >> portId;	// MUX portID

				DWORD	nDataLen = pMsg->GetWrtOffset() - pMsg->GetRdOffset();
				BYTE*	pData = new BYTE [nDataLen];
				pMsg->Get(pData,nDataLen);
				BND_END_NEGOTIATION_INDICATION_S* pEndNegotiationIndEp = (BND_END_NEGOTIATION_INDICATION_S*)pData;

				DWORD muxConnectionID = pTempParty->GetConnectionId();

				DWORD confId = pParty->GetConfId();
				DWORD partyId = pParty->GetPartyId();
				DWORD numOfChnls = pEndNegotiationIndEp->callParams.NumOfBndChnls;

				int status = UpdateBondingParameters( (DWORD)unitId, (DWORD)portId, muxConnectionID, confId, partyId, numOfChnls);
				if (status != STATUS_OK)
					PASSERT(status);

				CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;

				FillMplProtocol(pMplProt, BND_END_NEGOTIATION, (BYTE*)(pEndNegotiationIndEp), nDataLen );

				pMplProt->AddPortDescriptionHeader(partyId, confId, muxConnectionID, eLogical_mux);

				SendToCmForMplApi(*pMplProt);
				PDELETEA(pData);
				POBJDELETE(pMplProt);

				break;
			}
			case REMOTE_CI:
			{
				*pMsg >> portId;	// MUX portID
				DWORD	nDataLen = pMsg->GetWrtOffset() - pMsg->GetRdOffset();
				BYTE*	pData = new BYTE [nDataLen];
				pMsg->Get(pData,nDataLen);

				CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;
				DWORD muxConnectionID = pTempParty->GetConnectionId();
				DWORD confId = pParty->GetConfId();
				DWORD partyId = pParty->GetPartyId();

				FillMplProtocol(pMplProt, REMOTE_CI, pData,nDataLen);
				pMplProt->AddPortDescriptionHeader(partyId, confId, muxConnectionID, eLogical_mux);

				SendToCmForMplApi(*pMplProt);
				PDELETEA(pData);
				POBJDELETE(pMplProt);
				break;
			}
	                case CM_UPGRADE_NEW_VERSION_READY_REQ:
			{
			  // sagi add send ack for this request




			    PASSERT(CM_UPGRADE_NEW_VERSION_READY_REQ);
			    break;
			}
		        case CM_UPGRADE_START_WRITE_IPMC_REQ:
			{
			    PASSERT(CM_UPGRADE_START_WRITE_IPMC_REQ);
			    break;
			}

			default: {
				PASSERT(opcode+1000);
				break;
			}
		}
	}
	else
		PASSERT(1);

	POBJDELETE(pTempParty);
}

int CGideonSimMfaLogical::UpdateBondingParameters(DWORD unit, DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls)
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::UpdateBondingParameters ");

	if (!m_units)
		return -1;	// should not happen

	// update bonding parameters (number of channels)
	int status = m_units->UpdateBondingParameters( unit, port, muxConnectionID, confId, partyId, numOfChnls);

	return status;
}

void CGideonSimMfaLogical::OnTimerUnitConfigTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnTimerUnitConfigTout - UNIT CONFIG time is over.");
	CmUnitLoadedInd();
}

void CGideonSimMfaLogical::OnTimerMediaConfigTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnTimerMediaConfigTout - MEDIA CONFIG time is over.");

	CmMediaIpConfigInd();
}

void CGideonSimMfaLogical::OnTimerMediaConfigCompletedTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnTimerMediaConfigTout - MEDIA CONFIG time is over.");

	CmMediaIpConfigCompletedInd();
}

void CGideonSimMfaLogical::OnTimerCardsDelayTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnTimerCardsDelayTout - Wait for CM_UNIT_CONFIG_REQ.");

	CardManagerLoadedInd();
}

void CGideonSimMfaLogical::OnTimerSpeakerChangeTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::OnTimerSpeakerChangeTout - Send CHANGE_SPEAKER.");

	int   startIndex = m_nSpeakerIndex + 1;
	if( startIndex >= MAX_AUDIO_PARTIES )
		startIndex = 0;

	CAudioParty* pParty = NULL;

	int   i;
	for( i=startIndex; i<MAX_AUDIO_PARTIES && NULL == pParty; i++ ) {
		if( m_raAudioPartiesArr[i].IsEmpty() != TRUE ) {
			pParty = &(m_raAudioPartiesArr[i]);
			m_nSpeakerIndex = i;
		}
	}

	if( NULL == pParty ) {
		for( i=0; i<startIndex && NULL == pParty; i++ ) {
			if( m_raAudioPartiesArr[i].IsEmpty() != TRUE ) {
				pParty = &(m_raAudioPartiesArr[i]);
				m_nSpeakerIndex = i;
			}
		}
	}

	if( NULL != pParty )
	{
		StartTimer(MFA_SPEAKER_CHANGE_TIMER,::GetGideonSystemCfg()->GetMfaSpeakerChangeTime()*SECOND);
		SendActiveSpeakerInd(*pParty);
	}
}

/////////////////////////////////////////////////////////////////////////////
CAudioParty* CGideonSimMfaLogical::FindParty(const CMplMcmsProtocol& rMpl)
{
	CAudioParty* pParty = NULL;

	int i = 0;
	for( i=0; i<MAX_AUDIO_PARTIES && pParty == NULL; i++ )
		if( m_raAudioPartiesArr[i].GetConfId() == rMpl.getPortDescriptionHeaderConf_id() )
			if( m_raAudioPartiesArr[i].GetPartyId() == rMpl.getPortDescriptionHeaderParty_id() )
				pParty = &(m_raAudioPartiesArr[i]);

	return pParty;
}

CAudioParty* CGideonSimMfaLogical::FindParty(const CAudioParty& other)
{
	CAudioParty* pParty = NULL;

	int i = 0;
	for( i=0; i<MAX_AUDIO_PARTIES && pParty == NULL; i++ )
		if( m_raAudioPartiesArr[i].GetConfId() == other.GetConfId() )
			if( m_raAudioPartiesArr[i].GetPartyId() == other.GetPartyId() )
				pParty = &(m_raAudioPartiesArr[i]);

	return pParty;
}

BOOL CGideonSimMfaLogical::IsValidParty(const CMplMcmsProtocol& rMplProt) //const
{
	CAudioParty* pParty = FindParty(rMplProt);
	if( pParty == NULL )
		return FALSE;
	return TRUE;
}

void CGideonSimMfaLogical::CardManagerLoadedInd() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::CardManagerLoadedInd - SEND to MPL-API.");

	// send CM_CARD_MNGR_LOADED_IND
	CM_CARD_MNGR_LOADED_S  rStruct;
	memset(&rStruct,0,sizeof(CM_CARD_MNGR_LOADED_S));

	string mplSerialNum = ::GetGideonSystemCfg()->GetBoxChassisId();
	int len = mplSerialNum.length();
	memcpy(rStruct.serialNum, ::GetGideonSystemCfg()->GetBoxChassisId(), len);

	rStruct.status    = STATUS_OK;
	rStruct.cardType  = m_cardType; //eMfa_26;	// this important parameter actually tells the MCMS how
									// many units we have on card

	rStruct.hardwareVersion.ver_release  = 1;
	rStruct.hardwareVersion.ver_major    = 2;
	rStruct.hardwareVersion.ver_minor    = 3;
	rStruct.hardwareVersion.ver_internal = 4;

	for (int i = 0; i < MAX_NUM_OF_SW_VERSIONS; i++)
  {
    strcpy((char*) rStruct.swVersionsList[i].versionDescriptor, "The Description...");
    strcpy((char*) rStruct.swVersionsList[i].versionNumber, "12345");
  }

	CCardCfgMfa* pCurrCardCfg = NULL;
	pCurrCardCfg = (CCardCfgMfa*)::GetGideonSystemCfg()->GetCardCfg( m_wBoardId );
	if(!pCurrCardCfg)
	{
		PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::CardManagerLoadedInd - pCurrCardCfg is NULL!!");
		return;
	}

	UNIT_S currUnit;
	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		currUnit = pCurrCardCfg->GetUnit(i);

		if (currUnit.unitId != 65535) //current unit is set
			m_units->UpdateUnitStatus(i, eOk/*currUnit.unitStatus*/);
	}
	m_units->GetUnitsStatus( rStruct.postResultsList );	// config the units status

	m_units->GetUnitsType( rStruct.unitsTypesList );	// config the units type

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,CM_CARD_MNGR_LOADED_IND,
			(BYTE*)(&rStruct),sizeof(CM_CARD_MNGR_LOADED_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

void CGideonSimMfaLogical::CmUnitLoadedInd() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::CmUnitLoadedInd - SEND to MPL-API.");

	// send CM_UNIT_LOADED_IND
	CM_UNIT_LOADED_S  rStruct;
	memset(&rStruct,0,sizeof(CM_UNIT_LOADED_S));

	rStruct.status = STATUS_OK;

	m_units->GetUnitsStatus( (APIU32*)rStruct.statusOfUnitsList );	// gets the units status


	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,CM_UNIT_LOADED_IND,
			(BYTE*)(&rStruct),sizeof(CM_UNIT_LOADED_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

void CGideonSimMfaLogical::CmMediaIpConfigInd()
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::CmMediaIpConfigInd - SEND to MPL-API.");

	// send CM_MEDIA_IP_CONFIG_IND
	MEDIA_IP_CONFIG_S  rStruct;
	memset(&rStruct,0,sizeof(MEDIA_IP_CONFIG_S));

	rStruct.status = STATUS_OK;
	rStruct.serviceId = 111;
	rStruct.pqNumber  = 1;

	rStruct.iPv4.iPv4Address = SystemIpStringToDWORD("4.3.2.1");
    for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(rStruct.iPv6[i].iPv6Address, 0, IPV6_ADDRESS_LEN);
	}
    rStruct.ipType = (APIU32)eIpType_IpV4;

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,CM_MEDIA_IP_CONFIG_IND,
			(BYTE*)(&rStruct),sizeof(MEDIA_IP_CONFIG_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

void CGideonSimMfaLogical::ForwardAudioMsg2Endpoints (
				const DWORD opcode, const CAudioParty* party,
				BYTE* pData, const DWORD nDataLen ) const
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::ForwardAudioMsg2Endpoints - SEND TO E.P.");

	CSegment msg;

	msg << opcode
		<< m_wBoardId
		<< m_wSubBoardId
		<< (WORD)0; // unit_id

	if (party)	// message to party
		party->Serialize(msg);
	else
	{			// message to conf: need to update conf-ID and connection-ID
		msg << (DWORD)0xFFFFFFFF << (DWORD)0xFFFFFFFF << (DWORD)0xFFFFFFFF;
	}

	if( nDataLen != 0  &&  pData != NULL )
		msg.Put(pData,nDataLen);

	::SendAudioMessageToEndpointsSimApp(msg);
}

void CGideonSimMfaLogical::ForwardMuxBndReq2Endpoints(CMplMcmsProtocol& rMplProt) const
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::ForwardMuxBndReq2Endpoints - SEND TO E.P.");

	CSegment msg;

	DWORD unitId = rMplProt.getPhysicalInfoHeaderUnit_id();
	DWORD portId = rMplProt.getPhysicalInfoHeaderPort_id();

	msg << (DWORD)rMplProt.getOpcode()
		<< (WORD)m_wBoardId
		<< (WORD)m_wSubBoardId
		<< (WORD)unitId;

	msg << (DWORD)rMplProt.getPortDescriptionHeaderConf_id()
		<< (DWORD)rMplProt.getPortDescriptionHeaderParty_id()
		<< (DWORD)rMplProt.getPortDescriptionHeaderConnection_id();

	if (rMplProt.getOpcode() == BND_CONNECTION_INIT)
	{
		// for Bonding only
		msg << (DWORD)portId;
		msg << (DWORD)777;	// dummy for debug purpose
	}

	BYTE*	pData = (BYTE*)rMplProt.GetData();
	DWORD 	len = (DWORD)rMplProt.getDataLen();

	msg.Put((BYTE*)pData, len);

	::SendMuxMessageToEndpointsSimApp(msg);
}



/////////////////////////////////////////////////////////////////////////////
void CGideonSimMfaLogical::ForwardMsg2MediaMngr (CMplMcmsProtocol& rMplProt ) const
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    {
    	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::ForwardMsg2MediaMngr - ERROR: tried to send message to MediaMngr process - system is not CG!!");
    	return;
    }
    
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::ForwardMsg2MediaMngr - SEND TO MediaMngr.");

	CSegment pParamSeg;
	rMplProt.Serialize(pParamSeg);

	::SendMessageToMediaMngrApp( pParamSeg );
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimMfaLogical::SendActiveSpeakerInd(const CAudioParty& party) const
{
	TRACEINTO	<< " CGideonSimMfaLogical::SendActiveSpeakerInd - Change to: BoardId="
		<< (int)st_AudioControllerBoardId
		<< " Sub BoardId="
		<< (int)st_AudioControllerSubBoardId;

	TAcActiveSpeakersInd   tStruct;
	tStruct.unVideoPartyID = party.GetPartyId();
	tStruct.unAudioPartyID = party.GetPartyId(); //YOELLA - VASILY

	CMplMcmsProtocol*  pMplProt = new CMplMcmsProtocol;

	pMplProt->AddCommonHeader(AC_ACTIVE_SPEAKER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	//pMplProt->AddPhysicalHeader(0,m_wBoardId,m_wSubBoardId,0,0,0,ePhysical_art_light);
	pMplProt->AddPhysicalHeader(0,st_AudioControllerBoardId,st_AudioControllerSubBoardId,0,0,0,ePhysical_art_light);
	pMplProt->AddPortDescriptionHeader(INVALID/*pParty->GetPartyId()*/,party.GetConfId(),INVALID/*pParty->GetConnectionId()*/);

	pMplProt->AddData(sizeof(TAcActiveSpeakersInd),(char*)&tStruct);

	SendToCmForMplApi(*pMplProt);
	POBJDELETE(pMplProt);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimMfaLogical::SendAudioSpeakerInd(const CAudioParty& party) const
{
	TRACEINTO	<< " CGideonSimMfaLogical::SendAudioSpeakerInd - Change to: BoardId="
		<< (int)st_AudioControllerBoardId
		<< " Sub BoardId="
		<< (int)st_AudioControllerSubBoardId;

	TAcActiveSpeakersInd   tStruct;
	tStruct.unVideoPartyID = party.GetPartyId();
	tStruct.unAudioPartyID = party.GetPartyId(); //YOELLA - VASILY

	CMplMcmsProtocol*  pMplProt = new CMplMcmsProtocol;

	pMplProt->AddCommonHeader(AC_ACTIVE_SPEAKER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
//	pMplProt->AddPhysicalHeader(0,m_wBoardId,m_wSubBoardId,0,0,0,ePhysical_art_light);
	pMplProt->AddPhysicalHeader(0,st_AudioControllerBoardId,st_AudioControllerSubBoardId,0,0,0,ePhysical_art_light);
	pMplProt->AddPortDescriptionHeader(INVALID/*pParty->GetPartyId()*/,party.GetConfId(),INVALID/*pParty->GetConnectionId()*/);

	pMplProt->AddData(sizeof(TAcActiveSpeakersInd),(char*)&tStruct);

	SendToCmForMplApi(*pMplProt);
	POBJDELETE(pMplProt);
}


int CGideonSimMfaLogical::IsConfPlayMessage( const CMplMcmsProtocol& rMpl )
{
	if (0xFFFFFFFF == rMpl.getPortDescriptionHeaderParty_id())
		return 1;
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//CDR_MCCF:
void CGideonSimMfaLogical::FillRtpStatisticInfoStruct(TCmPartyInfoStatisticsInd* pSt) const
{
    DWORD  unStatisticInfoVal = 0;
    //Set memory to the value of ucMonitoringVal
    memset(pSt,unStatisticInfoVal,sizeof(TCmPartyInfoStatisticsInd));
    static DWORD dummyindex = 0;

    pSt->unNumOfChannels = 2;

    TRtpChannelMonitoringInd* pChnlMonitoring  = (TRtpChannelMonitoringInd *)pSt->acMonitoringData;

    int i = 0;

    //Video out
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unFrameRate = 30;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unMaxFrameRate = 60;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unMinFrameRate = 15;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMaxResolution = k1080p;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMinResolution = k720p;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMaxWidth = 1920;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMinWidth = 352;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMaxHeight = 1088;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMinHeight =240;

    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoResolution = kUnknownFormat;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoWidth = 352;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoHeight = 480;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel = TRUE;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType = kIpVideoChnlType;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection = cmCapTransmit;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval = 2000;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes = 100000; //155 * (20 * ((dummyindex % 5)? (dummyindex * dummyindex):(dummyindex*(dummyindex-1))));
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unLatency = 350;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakJitter =2500;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalJitter = 370;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakFractionLoss = 4500;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unAccumulatedPacket   = 554805;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unIntervalPacket      = 3200;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unAccumulated  = 8;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unInterval     = 2;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unPeak         = 3;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unAccumulated  = 9;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unInterval     = 2;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unPeak         = 2;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unAccumulated  = 6;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unInterval     = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unPeak         = 2;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unAccumulated   = 6;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unInterval      = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unPeak          = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unAccumulated  = 10;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unInterval     = 4;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unPeak         = 5;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unAccumulated    = 6;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unInterval       = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unPeak           = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unAccumulated = 6;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unInterval        = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unPeak            = 1;

    //Content out
    i = 1;

    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unFrameRate = 30;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unMaxFrameRate = 60;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unMinFrameRate = 15;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMaxResolution = k1080p;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMinResolution = k720p;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMaxWidth = 1920;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMinWidth = 352;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMaxHeight = 1088;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoMinHeight =240;

    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel = TRUE;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType = kIpContentChnlType;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection = cmCapTransmit;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval = 3003;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes = 100000;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unProtocol = 0;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unLatency = 5;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakJitter =2;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalJitter = 3;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakFractionLoss = 4;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unAccumulatedPacket   = 554805;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unIntervalPacket      = 3200;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unAccumulated  = 8;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unInterval     = 2;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unPeak         = 3;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unAccumulated  = 9;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unInterval     = 2;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unPeak         = 2;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unAccumulated  = 6;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unInterval     = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unPeak         = 2;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unAccumulated   = 6;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unInterval      = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unPeak          = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unAccumulated  = 10;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unInterval     = 4;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unPeak         = 5;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unAccumulated    = 6;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unInterval       = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unPeak           = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unAccumulated = 6;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unInterval        = 1;
    pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unPeak            = 1;
   //end bs
    dummyindex++;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CGideonSimMfaLogical::FillRtpMonitoringStruct(TCmPartyMonitoringInd* pSt) const
{
	DWORD  unMonitoringVal = 0;
	//Set memory to the value of ucMonitoringVal
	memset(pSt,unMonitoringVal,sizeof(TCmPartyMonitoringInd));
	static DWORD dummyindex = 0;

	pSt->unNumOfChannels = 8;

	TRtpChannelMonitoringInd* pChnlMonitoring  = (TRtpChannelMonitoringInd *)pSt->acMonitoringData;

	int i = 0;
	//Audio in - 0
	i = 0;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.unFramesPerPacket = 1;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel = TRUE;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType = kIpAudioChnlType;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection = cmCapReceive;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval = 2000;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes = 175 * (20 * dummyindex * dummyindex);
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unAccumulatedPacket = 0x7FFFFFFF;//0x80000000;//851264;//4123329024ll;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unAccumulatedPacketLoss = 7777;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unLatency = 100;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakJitter =5000;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalJitter = 200;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakFractionLoss = 1000;

	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unAccumulatedPacket	= 100;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unIntervalPacket		= 200;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unAccumulated	= 8;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unInterval		= 2;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unPeak			= 3;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unAccumulated	= 9;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unInterval		= 2;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unPeak			= 2;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unInterval		= 5;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unPeak			= 2;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unInterval		= 4;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unPeak			= 4;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unAccumulated	= 10;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unInterval		= 4;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unPeak			= 5;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unInterval		= 4;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unPeak			= 1;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unInterval	= 9;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unPeak		= 17;

	//Video in - 1
	i = 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoResolution = k525SD;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoWidth = 720;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoHeight = 480;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel = TRUE;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType = kIpVideoChnlType;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection = cmCapReceive;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval = 2000;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes = 3875*(dummyindex);
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unLatency = 500;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakJitter =2000;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalJitter = 300;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakFractionLoss = 4000;

	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unAccumulatedPacket	= 100;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unIntervalPacket		= 200;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unAccumulated	= 8;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unInterval		= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unPeak			= 3;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unAccumulated	= 9;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unInterval		= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unPeak			= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unInterval		= 3;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unPeak			= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unInterval		= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unPeak			= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unAccumulated	= 10;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unInterval		= 4;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unPeak			= 5;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unInterval		= 4;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unPeak			= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unInterval	= 3;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unPeak		= 1;

	//Content in - 2
	i = 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel = TRUE;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType = kIpContentChnlType;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection = cmCapReceive;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval = 3003;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes = 0;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unProtocol = 0;

	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unLatency = 5;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakJitter =2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalJitter = 3;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakFractionLoss = 4;

	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unAccumulatedPacket	= 100;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unIntervalPacket		= 200;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unAccumulated	= 8;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unInterval		= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unPeak			= 3;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unAccumulated	= 9;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unInterval		= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unPeak			= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unInterval		= 9;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unPeak			= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unInterval		= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unPeak			= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unAccumulated	= 10;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unInterval		= 4;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unPeak			= 5;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unInterval		= 23;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unPeak			= 7;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unInterval	= 12;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unPeak		= 14;

	//FECC in - 3
	i = 3;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel = TRUE;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType = kIpFeccChnlType;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection = cmCapReceive;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval = 2000;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes = 155 * (20 * ((dummyindex % 7)? (dummyindex * dummyindex):(dummyindex*(dummyindex-1))));
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unLatency = 250;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakJitter =6500;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalJitter = 400;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakFractionLoss = 2000;

	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unAccumulatedPacket	= 100;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unIntervalPacket		= 200;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unAccumulated	= 8;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unInterval		= 2;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unPeak			= 3;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unAccumulated	= 9;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unInterval		= 2;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unPeak			= 2;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unInterval		= 5;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unPeak			= 2;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unInterval		= 1;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unPeak			= 1;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unAccumulated	= 10;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unInterval		= 4;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unPeak			= 5;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unInterval		= 11;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unPeak			= 4;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unInterval	= 6;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unPeak		= 8;

	//Audio out - 4
	i = 4;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.unFramesPerPacket = 1;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel = TRUE;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType = kIpAudioChnlType;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection = cmCapTransmit;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval = 2000;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes = 165 * (20 * dummyindex * dummyindex);
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unLatency = 100;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakJitter =5000;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalJitter = 300;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakFractionLoss = 1100;

	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unAccumulatedPacket	= 554805;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unIntervalPacket		= 3200;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unAccumulated	= 8;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unInterval		= 2;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unPeak			= 3;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unAccumulated	= 9;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unInterval		= 2;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unPeak			= 2;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unInterval		= 1;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unPeak			= 2;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unInterval		= 1;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unPeak			= 1;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unAccumulated	= 10;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unInterval		= 4;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unPeak			= 5;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unInterval		= 1;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unPeak			= 1;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unInterval		= 1;
	pChnlMonitoring[i].tRtpAudioChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unPeak			= 1;

	//Video out - 5

	i = 5;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoResolution = kUnknownFormat;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoWidth = 352;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.unVideoHeight = 480;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel = TRUE;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType = kIpVideoChnlType;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection = cmCapTransmit;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval = 2000;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes = 155 * (20 * ((dummyindex % 5)? (dummyindex * dummyindex):(dummyindex*(dummyindex-1))));
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unLatency = 350;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakJitter =2500;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalJitter = 370;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakFractionLoss = 4500;

	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unAccumulatedPacket	= 554805;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unIntervalPacket		= 3200;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unAccumulated	= 8;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unInterval		= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unPeak			= 3;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unAccumulated	= 9;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unInterval		= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unPeak			= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unInterval		= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unPeak			= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unInterval		= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unPeak			= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unAccumulated	= 10;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unInterval		= 4;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unPeak			= 5;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unInterval		= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unPeak			= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unInterval		= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unPeak			= 1;

	//Content out - 6
	i = 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel = TRUE;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType = kIpContentChnlType;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection = cmCapTransmit;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval = 3003;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes = 0;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tRtpCommonChannelMonitoring.unProtocol = 0;

	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unLatency = 5;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakJitter =2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalJitter = 3;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakFractionLoss = 4;

	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unAccumulatedPacket	= 554805;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unIntervalPacket		= 3200;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unAccumulated	= 8;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unInterval		= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unPeak			= 3;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unAccumulated	= 9;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unInterval		= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unPeak			= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unInterval		= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unPeak			= 2;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unInterval		= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unPeak			= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unAccumulated	= 10;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unInterval		= 4;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unPeak			= 5;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unInterval		= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unPeak			= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unInterval		= 1;
	pChnlMonitoring[i].tRtpVideoChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unPeak			= 1;

	//FECC out - 7
	i = 7;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.bunValidChannel = TRUE;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.unChannelType = kIpFeccChnlType;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.unChannelDirection = cmCapTransmit;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.unTicksInterval = 2000;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tRtpCommonChannelMonitoring.unMediaBytes = 155 * (20 * ((dummyindex % 5)? (dummyindex * dummyindex):(dummyindex*(dummyindex-1))));
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unLatency = 250;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakJitter =6500;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalJitter = 400;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtcpInfoSt.unIntervalPeakFractionLoss = 2000;

	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unAccumulatedPacket	= 554805;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.unIntervalPacket		= 3200;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unAccumulated	= 8;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unInterval		= 2;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsActualLoss.unPeak			= 3;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unAccumulated	= 9;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unInterval		= 2;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsOutOfOrder.unPeak			= 2;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unInterval		= 1;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tPacketsFragmented.unPeak			= 2;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unInterval		= 1;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterBufferSize.unPeak			= 1;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unAccumulated	= 10;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unInterval		= 4;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterLatePackets.unPeak			= 5;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unInterval		= 1;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterOverFlows.unPeak			= 1;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unAccumulated	= 6;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unInterval		= 1;
	pChnlMonitoring[i].tRtpFeccChannelMonitoring.tAdvanceMonitoringResultsSt.tRtpStatisticSt.tJitterSamplePacketInterval.unPeak			= 1;
	//end bs
	dummyindex++;
}

void CGideonSimMfaLogical::OnTimerSoftwareUpgrade(CSegment* pMsg)
{
  PASSERT(1);
}


void CGideonSimMfaLogical::OnTimerIpmcSoftwareUpgrade(CSegment* pMsg)
{
  PASSERT(1);
}

void CGideonSimMfaLogical::OnTimerIsdnCfg(CSegment* pMsg) //olga
{
	CBehaviourIsdn* behaviourIsdn = ::GetGideonSystemCfg()->GetBehaviourIsdn();
	if( behaviourIsdn ) {

	    const EP_SGN_S* pSignalStr = behaviourIsdn->GetCurrSignal();
		DBGPASSERT_AND_RETURN(!pSignalStr);

		TRACESTR(eLevelInfoNormal) << "CGideonSimMfaLogical::OnTimerIsdnCfg : signal to be sent = " << pSignalStr->opcode;

		CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
		pMplProtocol->DeSerialize(*pMsg);

		if(( END_INIT_COMM    == pSignalStr->opcode) ||
		   ( REMOTE_BAS_CAPS  == pSignalStr->opcode && ::GetGideonSystemCfg()->GetIsdnCapsFlag() ) ||
		   ( REMOTE_XMIT_MODE == pSignalStr->opcode && ::GetGideonSystemCfg()->GetIsdnXmitModeFlag() ) ||
		   ( REMOTE_CI        == pSignalStr->opcode && ::GetGideonSystemCfg()->GetIsdnH230Flag() )) {

		    DWORD connectionId = pMplProtocol->getPortDescriptionHeaderConnection_id();
			DWORD partyId = pMplProtocol->getPortDescriptionHeaderParty_id();
			DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();

			CSegment* pSegm = new CSegment;
			for(WORD i = 0; i < pSignalStr->dataLen; i++)
			    *pSegm << (BYTE)pSignalStr->data[i];

			FillMplProtocol(pMplProtocol, pSignalStr->opcode, (BYTE*)(pSegm->GetPtr()), pSegm->GetWrtOffset());
			pMplProtocol->AddPortDescriptionHeader(partyId, confId, connectionId, eLogical_mux);

			SendToCmForMplApi(*pMplProtocol);

			POBJDELETE(pSegm);
		}

		const EP_SGN_S* pNextSignalStr = behaviourIsdn->GetNextSignal();
		if(pNextSignalStr) {
		    CSegment* pParamSeg = new CSegment;
		    pMplProtocol->Serialize(*pParamSeg);
		    StartTimer(ISDN_CFG_TIMER, pNextSignalStr->timer, pParamSeg);
		}

		POBJDELETE(pMplProtocol);
	}
}

void CGideonSimMfaLogical::CmMediaIpConfigCompletedInd()
{
	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::CmMediaIpConfigCompletedInd - SEND to MPL-API.");

	  CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	    FillMplProtocol( pMplProtocol,CM_MEDIA_CONFIGURATION_COMPLETED_IND);

	    SendToCmForMplApi(*pMplProtocol);
	    POBJDELETE(pMplProtocol);
}
