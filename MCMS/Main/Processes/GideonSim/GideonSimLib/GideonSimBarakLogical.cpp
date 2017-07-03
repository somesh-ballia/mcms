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
#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#include <assert.h>

#include "ManagerApi.h"

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
#ifndef __DISABLE_ICE__
#include "OpcodesMcmsCardMngrICE.h"
#endif	//__DISABLE_ICE__
#include "CardsStructs.h"
#include "IpMfaOpcodes.h"
#include "IpRtpReq.h"
#include "IpRtpInd.h"
#include "IpCmReq.h"
#include "IpCmInd.h"
#include "IpWebRtcReq.h"
#include "IpWebRtcInd.h"

#ifndef __DISABLE_ICE__
#include "IceCmReq.h"
#include "IceCmInd.h"
#include "ICEApiDefinitions.h"
#endif	//__DISABLE_ICE__

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
#include "GideonSimBarakLogical.h"
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

#ifndef __DISABLE_ICE__
#include "auto_array.h"
#include "Lock.hxx"
#include "IceManager.h"
#include "IceDummyProcessor.h"
#include "GideonSimIceMplMsgSender.h"
#include "IceMMTransferProcessor.h"
#include "MMIceMplMsgTransferer.h"
#endif 	//__DISABLE_ICE__

#include "802_1xApiInd.h"

#include "GideonSimLogicalParams.h"
#define	LICENSE_FILE_NAME  "Simulation/License.cfs"
//#define VASILY_MAX_LICENSE_LEN  1024

extern char* CardUnitConfiguredTypeToString(APIU32 unitConfigType);
extern const char* UnitReconfigStatusToString(APIU32 unitReconfigStatus);


#include "GideonSimBarakLogical.h"
#include "GideonSimBarakIcComponentLogical.h"
#include "GideonSimTbComponentLogical.h"
#include "OpcodesMcmsPCM.h"

#include "GideonSimLogicalParams.h"
#include "OpcodesMcmsCardMngrTIP.h"
#include "TipStructs.h"
#include "OpcodesMrmcCardMngrMrc.h"
#include "MrcStructs.h"

#include <math.h>

const OPCODE PCM_MENU_TIMER	    		= 10230; // olga for testing
const OPCODE ICE_SEND_STAT_TIMER	    = 10231; // N.A. DEBUG testing timer

extern DWORD GetBurnRate(eBurnTypes burnRateTypes);
extern DWORD GetMcuInternalOpcode();
extern eLogicalResourceTypes GetMcuInternalResType();
extern BYTE GetStatusOrTimer();
/////BARAK/////

/////////////////////////////////////////////////////////////////////////////
//
//   GideonSimBarakLogical - logical module of BARAK
//
/////////////////////////////////////////////////////////////////////////////
//               Action table
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CGideonSimBarakLogical)
	// Card Manager: start-up
	ONEVENT( CM_STARTUP,   IDLE,     CGideonSimBarakLogical::OnCmStartupIdle)
	//ONEVENT( CM_STARTUP,   CONNECT,     CGideonSimBarakLogical::OnCmStartupIdle)
	ONEVENT( CM_STARTUP,   CONNECT,     CGideonSimBarakLogical::OnCmStartupConnect)
	// Card Manager: message from MCMS
	ONEVENT( CM_MCMS_MSG,  STARTUP,  CGideonSimBarakLogical::OnCmProcessMcmsReqStartup)
	ONEVENT( CM_MCMS_MSG,  CONNECT,  CGideonSimBarakLogical::OnCmProcessMcmsReqConnect)
	// Timers:
	ONEVENT( BARAK_UNIT_CONFIG_TIMER,   STARTUP,  CGideonSimBarakLogical::OnTimerUnitConfigTout)




    ONEVENT( BARAK_CS_INTERNAL_CONFIG_COMPLETE,   STARTUP,  CGideonSimBarakLogical::CmCSIntConfigCompleteInd)
    ONEVENT( BARAK_CS_EXTERNAL_CONFIG_COMPLETE,   STARTUP,  CGideonSimBarakLogical::CmCSExtConfigCompleteInd)
    ONEVENT( BARAK_DNAT_CONFIG_COMPLETE,   STARTUP,  CGideonSimBarakLogical::CmDnatConfigCompleteInd)

ONEVENT( BARAK_CS_INTERNAL_CONFIG_COMPLETE,   CONNECT,  CGideonSimBarakLogical::CmCSIntConfigCompleteInd)
 ONEVENT( BARAK_CS_EXTERNAL_CONFIG_COMPLETE,   CONNECT,  CGideonSimBarakLogical::CmCSExtConfigCompleteInd)
 ONEVENT( BARAK_DNAT_CONFIG_COMPLETE,   CONNECT,  CGideonSimBarakLogical::CmDnatConfigCompleteInd)


	ONEVENT( BARAK_MEDIA_CONFIG_TIMER,  STARTUP,  CGideonSimBarakLogical::OnTimerMediaConfigTout)
	ONEVENT( BARAK_MEDIA_CONFIG_TIMER,  CONNECT,  CGideonSimBarakLogical::OnTimerMediaConfigTout)
	ONEVENT( CARD_NOT_READY_DELAY_TIMER,   STARTUP,  CGideonSimBarakLogical::OnTimerCardsDelayTout)
	ONEVENT( BARAK_SPEAKER_CHANGE_TIMER,	CONNECT,  CGideonSimBarakLogical::OnTimerSpeakerChangeTout)
	// Card Manager: message from Endpoints Sim (DTMF)
	ONEVENT( CM_EP_SIM_MSG,  CONNECT,  CGideonSimBarakLogical::OnCmProcessEpSimMsgConnect)
	// XMl file for Keep alive unit
	ONEVENT( SET_UNIT_STATUS_FOR_KEEP_ALIVE_NEW,  CONNECT,  CGideonSimBarakLogical::SetUnitStatustSimBarakForKeepAliveInd)

	ONEVENT( CM_LOADED_TEST_TIMER, ANYCASE, CGideonSimBarakLogical::OnCmLoaded_test)

        ONEVENT(ISDN_CFG_TIMER,    ANYCASE,  CGideonSimBarakLogical::OnTimerIsdnCfg)//olga
#ifndef __DISABLE_ICE__
        ONEVENT(CHECK_COMPLETE_TIMER,    ANYCASE,  CGideonSimBarakLogical::OnTimerCheckComplete)//Lior
#endif	//__DISABLE_ICE__
        ONEVENT( PACKET_LOSS_TIMER,          ANYCASE,  CGideonSimBarakLogical::OnTimerPacketLossTout )
        ONEVENT( SOFTWARE_UPGRADE_TIMER,  ANYCASE,  CGideonSimBarakLogical::OnTimerSoftwareUpgrade)
        ONEVENT( SOFTWARE_IPMC_UPGRADE_TIMER,  ANYCASE,  CGideonSimBarakLogical::OnTimerIpmcSoftwareUpgrade)
	// Card Manager: indication message from MM
	ONEVENT( CM_MM_MSG,  			CONNECT,	CGideonSimBarakLogical::OnCmProcessMMMsgConnect)
    ONEVENT(PCM_MENU_TIMER,    ANYCASE,  CGideonSimBarakLogical::OnPcmMenuTimer)//eitan

    //BFCP
    ONEVENT(BFCP_HELLO_TIMER, 	ANYCASE, CGideonSimBarakLogical::OnBfcpSendHelloMsgAnycase)
    ONEVENT(BFCP_HELLO_ACK_TIMER, ANYCASE, CGideonSimBarakLogical::OnBfcpCheckReceivedHelloAckTout)

    //ICE timer
    ONEVENT(ICE_SEND_STAT_TIMER,    ANYCASE,  CGideonSimBarakLogical::OnIceStatusIndSend) // N.A. DEBUG


PEND_MESSAGE_MAP(CGideonSimBarakLogical,/*CStateMachine*/CGideonSimLogicalModule)


/////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CGideonSimBarakLogical::CGideonSimBarakLogical(CTaskApp* pTask,WORD boardId,WORD subBoardId)
			: CGideonSimLogicalModule(pTask,boardId,subBoardId)
			, m_isIceInited(false)
			, m_iceChanId(1)
{
	m_nSpeakerIndex = MAX_AUDIO_PARTIES;
//vb    m_GideonSimBarakUnitsForKeepAliveInd.status=eNormal;
//vb     for (int i=0;i< MAX_NUM_OF_UNITS;i++)
//vb          m_GideonSimBarakUnitsForKeepAliveInd.statusOfUnitsList[i]=eOk;

	m_cardType = eMpmPlus_40;
	CCardCfgBarak* pCurrCardCfg = NULL;
	pCurrCardCfg = (CCardCfgBarak*)::GetGideonSystemCfg()->GetCardCfg( boardId );
	if (pCurrCardCfg)
		m_cardType = pCurrCardCfg->GetDetailedCardType();
	else
	{
		PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::CGideonSimBarakLogical - pCurrCardCfg is NULL!!");
		return;
	}
	// m_cardType = eMpmPlus_40;

	m_units = new CSimBarakUnitsList( boardId, subBoardId );
	m_units->SetCardType( m_cardType );

	m_IC = new CBarakIcComponent(this,pTask);
	m_TB = new CTbComponent(pTask);

	m_IsAudioControllerMaster = false;

	UNIT_S currUnit;
	for (int i=0; i<MAX_NUM_OF_UNITS; i++)
	{
		if (pCurrCardCfg)
		{
		    currUnit = pCurrCardCfg->GetUnit(i);
		    if (currUnit.unitId != 65535) //current unit is set
		    {
		        m_units->UpdateUnitStatus(i, currUnit.unitStatus);
		        TRACEINTO << "UpdateUnitStatus() BoardId: " << boardId << " currUnit: " << i << " status: " << currUnit.unitStatus;
		    }
		}
		else
		    TRACEINTO << "CGideonSimBarakLogical::CGideonSimBarakLogical - pCurrCardCfg is NULL - can not UpdateUnitStatus";
	}

	m_software_upgrade_done = FALSE;
	//memset(m_IcePartiesRemoteArr, 0, sizeof(m_IcePartiesRemoteArr));

#ifndef __DISABLE_ICE__
	eProductFamily curProductFamily  = CProcessBase::GetProcess()->GetProductFamily();
	if(curProductFamily==eProductFamilyCallGenerator)
	{
		m_iceProcessor=new IceMMTransferProcessor(*(new  MMIceMplMsgTransferer(*this)));
	}
	else {
		int BarakCardMode=::GetGideonSystemCfg()->GetBarakCardMode();
		if(BarakCardMode==BarakCard_REAL_RTP_MODE) {//work with CS and real SIP endpoint(HDX, MOC)
			m_iceProcessor=new IceManager(*(new  GideonSimIceMplMsgSender(*this)));
		}
		else {//work with EpSim
			m_iceProcessor=new IceDummyProcessor(*(new  GideonSimIceMplMsgSender(*this)));
		}
	}
#endif	//__DISABLE_ICE__

}

/////////////////////////////////////////////////////////////////////////////
CGideonSimBarakLogical::~CGideonSimBarakLogical()
{
	POBJDELETE( m_units );
	POBJDELETE( m_IC );
	POBJDELETE( m_TB );

	map<PartyConfKey,PartyConfValue>::iterator iter = m_waitForHelloMap.begin();

	while(iter != m_waitForHelloMap.end())
	{
	    PartyConfValue value = (*iter).second;
	    POBJDELETE(value.second);
	    m_waitForHelloMap.erase(iter++);
	}
}

/////////////////////////////////////////////////////////////////////////////
void* CGideonSimBarakLogical::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::ProcessEndpointsSimMsg(CSegment* pMsg)
{
	DispatchEvent(CM_EP_SIM_MSG,pMsg);
}


/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::ProcessMMIndicationMsg(CSegment* pMsg)
{
   if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::ProcessMMIndicationMsg - ERROR: system is not CG!!");
		return;
	}

	DispatchEvent(CM_MM_MSG,pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// Events & Action functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnCmStartupIdle(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmStartupIdle");

	m_state = STARTUP;

	CardManagerLoadedInd();

	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmStartupIdle - Wait for CM_UNIT_CONFIG_REQ.");

//	StartTimer(CM_LOADED_TEST_TIMER, 60*SECOND);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnCmStartupConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmStartupConnect!!!!!!!!!!!!!!!!");

	//m_state = STARTUP;

	//CardManagerLoadedInd();
	ReestablishConnectionInd();
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmStartupConnect - Wait for CM_UNIT_CONFIG_REQ.");
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnCmLoaded_test()
{
	CardManagerLoadedInd();
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnCmProcessMcmsReqStartup(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup");

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pMsg);


	switch( pMplProtocol->getOpcode() )
	{
		case CARDS_NOT_READY_REQ:
		{
			// delay connection
 			StartTimer(CARD_NOT_READY_DELAY_TIMER,BARAK_CARDS_DELAY_TIME);

			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - - CARDS_NOT_READY_REQ received, start timer.");
			break;
		}
		case CM_UNIT_CONFIG_REQ:
		{
			m_units->UnitsConfigReq( pMplProtocol );	// config the units type according to req

 			StartTimer(BARAK_UNIT_CONFIG_TIMER,::GetGideonSystemCfg()->GetMfaUnitConfigTime()*SECOND);
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - CM_UNIT_CONFIG_REQ received, start timer.");
			break;
		}
		case SYSTEM_CARDS_MODE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - SYSTEM_CARDS_MODE_REQ received");
			break;
		}

		case CM_UNIT_RECONFIG_REQ:
		{
			UNIT_RECONFIG_S* pReconfigStruct = new UNIT_RECONFIG_S;
		    memset( pReconfigStruct, 0, sizeof(UNIT_RECONFIG_S) );
		    pMsg->Get( (BYTE*)pReconfigStruct, sizeof(UNIT_RECONFIG_S) );

		    TRACESTR(eLevelInfoNormal) << "\nCGideonSimBarakLogical::OnCmProcessMcmsReqStartup - CM_UNIT_RECONFIG_REQ"
		                                  << "\nBoardId:  " << pReconfigStruct->boardId
		                                  << "\nUnitId:   " << pReconfigStruct->unitId
		                                  << "\nUnitType: " << ::CardUnitConfiguredTypeToString(pReconfigStruct->unitType)
		                                  << "\nStatus:   " << ::UnitReconfigStatusToString(pReconfigStruct->unitStatus);

		    delete pReconfigStruct;

			break;
		}
		case CM_DNAT_CONFIG_REQ:
		{

			DispatchEvent(BARAK_DNAT_CONFIG_COMPLETE,NULL);
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - - CM_DNAT_CONFIG_REQ received, start timer.");


			break;
		}

		case CM_CS_INTERNAL_IP_CONFIG_REQ:
		{
			DispatchEvent(BARAK_CS_INTERNAL_CONFIG_COMPLETE,NULL);
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - - CM_CS_INTERNAL_IP_CONFIG_REQ received, start timer.");


			break;
		}
		case CM_CS_EXTERNAL_IP_CONFIG_REQ:
		{
			DispatchEvent(BARAK_CS_EXTERNAL_CONFIG_COMPLETE,NULL);
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - - CM_CS_EXTERNAL_IP_CONFIG_REQ received, start timer.");


			break;
		}

		case CM_MEDIA_CONFIGURATION_COMPLETED_REQ:
		{
			CmConfigCompleteInd();
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - - CM_MEDIA_CONFIGURATION_COMPLETED_REQ received, start timer.");


			break;
		}

		case CM_MEDIA_IP_CONFIG_REQ:
		case CM_RTM_MEDIA_IP_CONFIG_REQ:
		{
			// TODO : here is a MEDIA IP config
			// MEDIA_IP_PARAMS_S
 			StartTimer(BARAK_MEDIA_CONFIG_TIMER,::GetGideonSystemCfg()->GetMfaMediaConfigTime()*SECOND);

 			OPCODE curOp = pMplProtocol->getOpcode();
			if (CM_MEDIA_IP_CONFIG_REQ == curOp)
			{
				PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - CM_MEDIA_IP_CONFIG_REQ received, start timer.");
			}
			else if (CM_RTM_MEDIA_IP_CONFIG_REQ == curOp)
			{
				PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - CM_RTM_MEDIA_IP_CONFIG_REQ received, start timer.");
			}
			break;
		}
		case CM_MEDIA_IP_CONFIG_END_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - CM_MEDIA_IP_CONFIG_END_REQ, end of config.");
			m_state = CONNECT;

			{
#ifndef __DISABLE_ICE__
				resip::Lock lock(m_TaskApiMutex);
#endif	//__DISABLE_ICE__

				if( CPObject::IsValidPObjectPtr(m_pTaskApi) ) {
					CSegment*   pSeg = new CSegment;
					*pSeg	<< m_wBoardId
							<< m_wSubBoardId;
					m_pTaskApi->SendLocalMessage(pSeg,BARAK_CONNECTED);
				}
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
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - ETHERNET_SETTINGS_CONFIG_REQ received");
			break;
		}
		case Op802_1x_NEW_CONFIG_REQ:
		{
			Send802_1xNewConfigInd(*pMplProtocol,STATUS_OK);
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


		default:
		{
//			PTRACE(eLevelError,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - UNKNOWN opcode received.");
			TRACESTR(eLevelError) << " CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - "
				<< "UNEXPECTED MESSAGE IN (STARTUP) STATE, opcode - "
				<< "<" << (int)pMplProtocol->getOpcode() << ">";
			break;
		}
	}
	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::SendSlaveEncoderOpenInd( CMplMcmsProtocol& rMplProt ) const
{
    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::SendSlaveEncoderOpenInd");
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
void CGideonSimBarakLogical::SendMasterVideoEncoderOpenInd( CMplMcmsProtocol& rMplProt ) const
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::SendMasterVideoEncoderOpenInd");
	ACK_IND_S  rAckStruct;
	memset(&rAckStruct,0,sizeof(ACK_IND_S));
	rAckStruct.ack_base.ack_opcode = TB_MSG_OPEN_PORT_REQ;
	rAckStruct.ack_base.status = STATUS_OK;
	rAckStruct.ack_base.ack_seq_num = rMplProt.getMsgDescriptionHeaderRequest_id();

	rMplProt.AddCommonHeader(VIDEO_MASTER_ENCODER_OPEN_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(sizeof(ACK_IND_S),(char*)(&rAckStruct));

	SendToCmForMplApi(rMplProt);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::SetUnitStatustSimBarakForKeepAliveInd(WORD UnitNum, STATUS status )
{
	if (status<NUM_OF_CARD_UNIT_STATUSES)
	{
		if (UnitNum < MAX_NUM_OF_UNITS)
		{
			m_units->UpdateUnitStatus( UnitNum, status );
//vb			//m_GideonSimBarakUnitsForKeepAliveInd.statusOfUnitsList[UnitNum]=status;
		}
		else
			PTRACE(eLevelError,"CGideonSimBarakLogical::SetUnitStatustSimBarakForKeepAliveInd: illegal unit #");
	}
	else
	    PTRACE(eLevelError,"CGideonSimBarakLogical::SetUnitStatustSimBarakForKeepAliveInd: illegal status");

	PTRACE(eLevelError,"CGideonSimBarakLogical::SetUnitStatustSimBarakForKeepAliveInd: Shlomit+++++++-------");
}


///////////////////////////////////////////////////////////////////////////////
//vb void CGideonSimBarakLogical::SetStatusStatustSimBarakForKeepAliveInd(STATUS status )
//vb
//vb{
//vb	if (status<NUM_OF_CARD_STATES)
//vb	    m_GideonSimBarakUnitsForKeepAliveInd.status=status;
//vb    else
//vb	    PTRACE(eLevelError,"CGideonSimBarakLogical::SetStatusStatustSimBarakForKeepAliveInd: illegal status");
//vb
//vb}

///////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::Ack_keep_alive( CMplMcmsProtocol& rMplProt, STATUS status )
{
	KEEP_ALIVE_S  tKeepAliveInd;
	memset(&tKeepAliveInd,0,sizeof(KEEP_ALIVE_S));

	rMplProt.AddCommonHeader(CM_KEEP_ALIVE_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	m_units->GetUnitsStatus( &tKeepAliveInd );	// gets the units status
//vb	m_units->GetUnitsStatus( &m_GideonSimBarakUnitsForKeepAliveInd );	// gets the units status

	rMplProt.AddData(sizeof(KEEP_ALIVE_S),(char*)(&tKeepAliveInd));
//vb	rMplProt.AddData(sizeof(KEEP_ALIVE_S),(char*)(&m_GideonSimBarakUnitsForKeepAliveInd));

	SendToCmForMplApi(rMplProt);
}

///////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::Send802_1xNewConfigInd( CMplMcmsProtocol& rMplProt, STATUS status )
{
	s802_1x_NEW_CONFIG_IND  tNewConfigInd;
	memset(&tNewConfigInd,0,sizeof(s802_1x_NEW_CONFIG_IND));

	rMplProt.AddCommonHeader(Op802_1x_NEW_CONFIG_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(sizeof(s802_1x_NEW_CONFIG_IND),(char*)(&tNewConfigInd));

	SendToCmForMplApi(rMplProt);
}


/////////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::SetBndLclAlignmentTimer( DWORD bndRmtLclAlignment )
{
	g_bondingAlignmentTime = bndRmtLclAlignment;
}


///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
/*void CGideonSimBarakLogical::Ack_keep_alive( CMplMcmsProtocol& rMplProt, STATUS status ) const
{
	KEEP_ALIVE_S   rkeep_alive_ind_Struct;
	memset(&rkeep_alive_ind_Struct,0,sizeof(KEEP_ALIVE_S ));
    rkeep_alive_ind_Struct.status=eNormal;
     for (int i=0;i< MAX_NUM_OF_UNITS;i++)
          rkeep_alive_ind_Struct.statusOfUnitsList[i]=eOk;

	rMplProt.AddCommonHeader(CM_KEEP_ALIVE_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(sizeof(KEEP_ALIVE_S),(char*)(&rkeep_alive_ind_Struct));

	SendToCmForMplApi(rMplProt);

}*/

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::EstablishConnectionInd()
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
void CGideonSimBarakLogical::ReestablishConnectionInd()
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

/////////////////////////////////////////////////////////////////////////////
/*void CGideonSimBarakLogical::ReestablishConnectionInd()
{
	PTRACE(eLevelInfoNormal,"CSimSwitchCard::ReestablishConnectionInd - SEND to MPL-API.");

	// prepare & send REESTABLISH_CONNECTION
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	ESTABLISH_CONNECTION_S  rStruct;
	rStruct.dummy = 111;
	FillMplProtocol( pMplProtocol,
		REESTABLISH_CONNECTION,(BYTE*)(&rStruct),sizeof(REESTABLISH_CONNECTION_S));
		//CARD_MANAGER_RECONNECT,(BYTE*)(&rStruct),sizeof(CARD_MANAGER_RECONNECT_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

*/



/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnCmProcessMcmsReqConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect");

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pMsg);

	eProductFamily curProductFamily  = CProcessBase::GetProcess()->GetProductFamily();	// for Call Generator

	/* temp for Talya - start */
	DWORD  opcode = pMplProtocol->getOpcode();
	eLogicalResourceTypes lrt = (eLogicalResourceTypes)pMplProtocol->getPortDescriptionHeaderLogical_resource_type_1();

	if ((GetMcuInternalOpcode() == opcode) && (lrt == GetMcuInternalResType()))
	{
		if(GetStatusOrTimer() == 2)
		{
			Ack(*pMplProtocol,STATUS_FAIL);
		}
		POBJDELETE(pMplProtocol);
		return;

	}
	switch( opcode ) {

	case CM_DNAT_CONFIG_REQ:
	{
		DispatchEvent(BARAK_DNAT_CONFIG_COMPLETE,NULL);
		PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - - CM_DNAT_CONFIG_REQ received, start timer.");

		break;
	}

	case CM_CS_INTERNAL_IP_CONFIG_REQ:
	{
		DispatchEvent(BARAK_CS_INTERNAL_CONFIG_COMPLETE,NULL);
		PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - - CM_CS_INTERNAL_IP_CONFIG_REQ received, start timer.");

		break;
	}
	case CM_CS_EXTERNAL_IP_CONFIG_REQ:
	{
		DispatchEvent(BARAK_CS_EXTERNAL_CONFIG_COMPLETE,NULL);
		PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - - CM_CS_EXTERNAL_IP_CONFIG_REQ received, start timer.");

		break;
	}

	case CM_MEDIA_CONFIGURATION_COMPLETED_REQ:
	{
		CmConfigCompleteInd();
		PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqStartup - - CM_MEDIA_CONFIGURATION_COMPLETED_REQ received, start timer.");


		break;
	}
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
						ForwardAudioMsg2Endpoints(opcode,pParty);
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


			Ack(*pMplProtocol,STATUS_OK);


			if( resType == ePhysical_video_encoder )
			{
				ENCODER_PARAM_S* pReq = (ENCODER_PARAM_S*)pMplProtocol2->GetData();
				if( E_VIDEO_ENCODER_SLAVE_FULL_ENCODER == pReq->nVideoEncoderType )
				{
					SendSlaveEncoderOpenInd(*pMplProtocol2);
				}
				if(E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER == pReq->nVideoEncoderType || E_VIDEO_ENCODER_MASTER_SPLIT_ENCODER_HALF_DSP == pReq->nVideoEncoderType)
				{
					SendMasterVideoEncoderOpenInd(*pMplProtocol2);
				}
			}
			//  if TB_MSG_OPEN_PORT_REQ to video decoder with type E_VIDEO_DECODER_CONTENT , send DECODER_SYNC_IND
			if( resType == ePhysical_video_decoder )
			{
			   	DECODER_PARAM_S*  pReq = (DECODER_PARAM_S*)pMplProtocol2->GetData();
			   	if(E_VIDEO_DECODER_CONTENT == pReq->nVideoDecoderType )
			   	{
			   		DECODER_SYNC_IND_S   tSyncStruct;
			   		memset(&tSyncStruct,0,sizeof(DECODER_SYNC_IND_S));
			   		tSyncStruct.nStatus = STATUS_OK;
			   		pMplProtocol2->AddCommonHeader(VIDEO_DECODER_SYNC_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
            		pMplProtocol2->AddData(sizeof(DECODER_SYNC_IND_S),(char*)(&tSyncStruct));
            		SendToCmForMplApi(*pMplProtocol2);
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

						// sends message to MM in order to remove party (if the system is CG)
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
		// ART - audio unit
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
			//if( FindTimer(BARAK_SPEAKER_CHANGE_TIMER,FALSE) == NULL )
			if( ! IsValidTimer(BARAK_SPEAKER_CHANGE_TIMER)  &&  0 != ::GetGideonSystemCfg()->GetMfaSpeakerChangeTime() )
				StartTimer(BARAK_SPEAKER_CHANGE_TIMER,::GetGideonSystemCfg()->GetMfaSpeakerChangeTime()*SECOND);
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
		//case IP_CM_RTCP_VIDEO_PREFERENCE_REQ:
		{
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
		// ART - video decoder / encoder
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
		case VIDEO_ENCODER_DSP_SMART_SWITCH_CHANGE_LAYOUT_REQ:
		{
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
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
/*		case CONF_PARTY_MRMP_OPEN_CHANNEL_REQ:
		{
		    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect CONF_PARTY_MRMP_OPEN_CHANNEL_REQ");
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}*/
/*		case CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ:
		{
		    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ");
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}*/
		case H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		case CONFPARTY_CM_UPDATE_UDP_ADDR_REQ:
		case CONFPARTY_CM_CLOSE_UDP_PORT_REQ:
		case H323_RTP_UPDATE_CHANNEL_REQ:
		case H323_RTP_STREAM_ON_REQ:
		case H323_RTP_STREAM_OFF_REQ:
		case H323_RTP_LPR_MODE_CHANGE_REQ:
		case H323_RTP_LPR_MODE_RESET_REQ:
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
		case IP_CM_STOP_PREVIEW_CHANNEL:
		case IP_CM_START_PREVIEW_CHANNEL:
		case CONF_PARTY_MRMP_OPEN_CHANNEL_REQ:
		case CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ:
		case IP_CM_DTLS_CLOSE_REQ:
		{
		    // forward to MM process if the system is CG
			if ((eProductFamilyCallGenerator == curProductFamily) &&
				((H323_RTP_UPDATE_CHANNEL_REQ 			== opcode) ||
				(SIP_RTP_UPDATE_CHANNEL_REQ				== opcode) ||
				(H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ 	== opcode) ||
				(SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ 	== opcode) ||
				(SIP_CM_OPEN_UDP_PORT_REQ				== opcode) ||
				(CONFPARTY_CM_CLOSE_UDP_PORT_REQ		== opcode) ||
				(SIP_CM_CLOSE_UDP_PORT_REQ				== opcode) ||
				(ART_CONTENT_ON_REQ						== opcode) ||
				(ART_CONTENT_OFF_REQ					== opcode) ||
				(IP_CM_DTLS_CLOSE_REQ					== opcode)))
			{
				ForwardMsg2MediaMngr(*pMplProtocol);	// sends message to MM
			}
			if(CONFPARTY_CM_CLOSE_UDP_PORT_REQ == opcode || SIP_CM_CLOSE_UDP_PORT_REQ == opcode)
			    RemoveBfcpWaiting(*pMplProtocol);
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
				SendToCmForMplApi(*pMplProtocol);	// send to MCMS
				PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect -RTP_PARTY_VIDEO_CHANNELS_STATISTICS_REQ- PACKET_LOSS_TIMER");
				StartTimer(PACKET_LOSS_TIMER,5*SECOND);
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
		        SendToCmForMplApi(*pMplProtocol);   // send to MCMS
		        PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect -H323_RTP_PARTY_MONITORING_REQ- PACKET_LOSS_TIMER");
		        StartTimer(PACKET_LOSS_TIMER,5*SECOND);
		    }
		    break;
		}

		case CONF_PARTY_MRMP_PARTY_MONITORING_REQ:
		{
		    // sends ack to MCMS
		    AckIp(*pMplProtocol,STATUS_OK);

		    TCmPartyMonitoringInd   tMonitorInd;
		    memset( &tMonitorInd, 0, sizeof(TCmPartyMonitoringInd) );
		    FillRtpMonitoringStruct(&tMonitorInd);
		    pMplProtocol->AddCommonHeader( CONF_PARTY_MRMP_PARTY_MONITORING_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );
		    pMplProtocol->AddData( sizeof(TCmPartyMonitoringInd), (char*)(&tMonitorInd) );
		    SendToCmForMplApi(*pMplProtocol);   // send to MCMS
		    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect -CONF_PARTY_MRMP_PARTY_MONITORING_REQ");
    
		    break;
		}

		case H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		case H320_RTP_UPDATE_CHANNEL_REQ:
		case H320_RTP_UPDATE_CHANNEL_RATE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle BCH request");
			Ack(*pMplProtocol, STATUS_OK);
			break;
		}
		case IP_CM_RTCP_VIDEO_PREFERENCE_REQ:
		{
			AckIp(*pMplProtocol, STATUS_OK);
			break;
		}
		case IVR_PLAY_MESSAGE_REQ:
		{
//			SIVRPlayMessageIndStruct  tPlayMsgInd;
//			tPlayMsgInd.status = STATUS_FAIL;

			// if port is audio - send message to Endpoints Sim
	//		if( pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art  ||
	//			pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art_light )
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

	//					tPlayMsgInd.status = STATUS_OK;
					}
					else
						PTRACE(eLevelError,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Party not found for IVR_PLAY_MESSAGE_REQ");
				}
				else
				{
					ForwardAudioMsg2Endpoints(opcode, NULL, (BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
				}
			}
//			pMplProtocol->AddData(sizeof(SIVRPlayMessageIndStruct),(char*)(&tPlayMsgInd));

//			SendToCmForMplApi(*pMplProtocol);
//			SystemSleep(100*1);
//			Ack(*pMplProtocol,STATUS_OK);
			// IC will send ACK when needed
			m_IC->AddIvrPlayMsgToQ(*pMplProtocol);
			break;
		}
		case IVR_STOP_PLAY_MESSAGE_REQ:
		{
//			SIVRStopIVRStruct  tStopPlayMsgInd;
//			tPlayMsgInd.status = STATUS_FAIL;
//			memcpy(&tStopPlayMsgInd,pMplProtocol->GetData(),sizeof(SIVRStopIVRStruct));

			// if port is audio - send message to Endpoints Sim
	//		if( pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art  ||
	//			pMplProtocol->getPhysicalInfoHeaderResource_type() == ePhysical_art_light )
			{
				CAudioParty*  pParty = FindParty(*pMplProtocol);
				if( pParty != NULL )
				{
					// forward to E.P. app
					ForwardAudioMsg2Endpoints(opcode,pParty,
							(BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());

//					tStopPlayMsgInd.status = STATUS_OK;
				}
			}
//			pMplProtocol->AddCommonHeader(IVR_STOP_PLAY_MESSAGE_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

//			pMplProtocol->AddData(sizeof(SIVRStopIVRStruct),(char*)(&tStopPlayMsgInd));

//			SendToCmForMplApi(*pMplProtocol);
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
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle IVR_RECORD_ROLL_CALL_REQ");

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
				PTRACE(eLevelError,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Party not found for IVR_RECORD_ROLL_CALL_REQ");

			break;
		}
		case IVR_STOP_RECORD_ROLL_CALL_REQ:
		{
			CAudioParty*  pParty = FindParty(*pMplProtocol);
			if( pParty != NULL )
			{
				// forward to E.P. app
				ForwardAudioMsg2Endpoints(opcode,pParty,
						(BYTE*)(pMplProtocol->GetData()),pMplProtocol->getDataLen());
			}
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
//		case IVR_MUSIC_ADD_SOURCE_REQ:
		case IVR_ADD_MUSIC_SOURCE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle IVR_ADD_MUSIC_SOURCE_REQ (NULL).");
			break;
		}

		// handle dtmf requests
		case AUDIO_PLAY_TONE_REQ:
		{
		    // forward to MM process if the system is CG
			if (eProductFamilyCallGenerator == curProductFamily)
			{
				PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle AUDIO_PLAY_TONE_REQ - forward to Media Manager");

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
						"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle MOVE_RSRC_REQ. New conf ID ",
						szString);
				}
			}
			Ack(*pMplProtocol,STATUS_OK);
			break;
		}
		case STARTUP_DEBUG_RECORDING_PARAM_REQ: //CARDS_CM_MEDIA_RECORDING_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle STARTUP_DEBUG_RECORDING_PARAM_REQ.");
			break;
		}
		case CM_KEEP_ALIVE_REQ:
		{
			Ack_keep_alive(*pMplProtocol,STATUS_OK);
			break;
		}
		case ETHERNET_SETTINGS_CONFIG_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - ETHERNET_SETTINGS_CONFIG_REQ received");
			break;
		}
		case Op802_1x_NEW_CONFIG_REQ:
		{
			Send802_1xNewConfigInd(*pMplProtocol,STATUS_OK);
			break;
		}
		case AC_TYPE_REQ:
		{
			//TRACEINTO << "OnCmProcessMcmsReqConnect " <<  pMplProtocol->getOpcode();

			//save audio controller role master|shadow
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

		case CM_UNIT_RECONFIG_REQ:
		{
			UNIT_RECONFIG_S*  pReconfigStruct = (UNIT_RECONFIG_S*)pMplProtocol->GetData();

			// ===== 2. print to trace
		    TRACESTR(eLevelInfoNormal) << "\nCGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle CM_UNIT_RECONFIG_REQ"
		                                  << "\nBoardId:  " << pReconfigStruct->boardId
		                                  << "\nUnitId:   " << pReconfigStruct->unitId
		                                  << "\nUnitType: " << ::CardUnitConfiguredTypeToString(pReconfigStruct->unitType)
		                                  << "\nStatus:   " << ::UnitReconfigStatusToString(pReconfigStruct->unitStatus);

			// changes the Unit type to the requested
			int status = m_units->ChangeUnitType( pReconfigStruct->unitId, pReconfigStruct->unitType );
			if (STATUS_OK == status)
			    pReconfigStruct->unitStatus = STATUS_OK;
			else
			    pReconfigStruct->unitStatus = STATUS_FAIL;

			// sends IND to the MCMS
			CMplMcmsProtocol*  pProt = new CMplMcmsProtocol(*pMplProtocol);
			pProt->AddCommonHeader( CM_UNIT_RECONFIG_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );
			pProt->AddData( sizeof(UNIT_RECONFIG_S), (char*)(pReconfigStruct) );
			SendToCmForMplApi(*pProt);	// send to MCMS

			POBJDELETE(pProt);
		    break;
		}

		/////////////////
		// ART - MUX unit
		/////////////////

		case BND_CONNECTION_INIT:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle BND_CONNECTION_INIT.");

			BND_CONNECTION_INIT_REQUEST_S*  pRequest = (BND_CONNECTION_INIT_REQUEST_S*)pMplProtocol->GetData();

			ForwardMuxBndReq2Endpoints(*pMplProtocol);

			break;
		}

		case BND_ADD_CHANNEL:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle BND_ADD_CHANNEL.");

			BYTE numOfChannels = 0;
			BOOL bBndAllChannelsConnected = FALSE;
			int status = m_units->AddBondingChannel( pMplProtocol, &numOfChannels, &bBndAllChannelsConnected );
			if (0 != status)
			{
				TRACESTR(eLevelError) << " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Add Bonding Channel error";
				break;	// error
			}
			if (bBndAllChannelsConnected)	// all Bonding channels were connected, need to send "BND_REMOTE_LOCAL_ALIGNMENT" command
			{
				TRACESTR(eLevelInfoNormal) << " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - BND_ADD_CHANNEL: All channels connected";

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

				break;
			}

			break;
		}

		case BND_ABORT_CALL:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle BND_ABORT_CALL.");

			//Ack(*pMplProtocol, STATUS_OK); - no need to return Ack in this phase

			break;
		}

		////////////

		case H221_INIT_COMM:
		{
			///TEMP
			// Send END_INIT_COMM as a response
			TRACEINTO	<< " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect opcode: "
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
			TRACEINTO	<< " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect opcode: "
						<< "Send REMOTE_XMIT_MODE - Gideon Sim initiative";

	 	 	DWORD numBytes = 12;
	 	 	CSegment xmitModeSeg;

			CSegment* pRemoteXmitMode = new CSegment;
			*pRemoteXmitMode 	<< numBytes
			///					<< (BYTE)0x1f << (BYTE)0x26 << (BYTE)0x40 << (BYTE)0x47 << (BYTE)0x5a << (BYTE)0x5c
			///					<< (BYTE)0x60 << (BYTE)0x70 << (BYTE)0xf0 << (BYTE)0x60 << (BYTE)0xf0 << (BYTE)0x6e;
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
			TRACEINTO	<< " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect opcode: "
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
			TRACEINTO	<< " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect opcode: "
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
			TRACEINTO	<< " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect opcode: "
						<< "SEND_H_230";
			ForwardMuxBndReq2Endpoints(*pMplProtocol);
			break;
		}
		case PCM_INDICATION:
		{
			///TEMP
			TRACEINTO	<< " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect opcode: PCM_INDICATION";

			//added by huiyu
			char* pData = pMplProtocol->GetData();
			PTRACE2(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect,INDICATION\n", pData);

			// tmp eitan
			BYTE isUsingRealPcm = FALSE;

			if (isUsingRealPcm)
			{
				//added by huiyu
				CSegment* pParamSeg = new CSegment;
				pMplProtocol->Serialize(*pParamSeg);
				::SendMessageToPCMSimApp(*pParamSeg);
				POBJDELETE(pParamSeg);
			}
			else if (strstr(pData,"control_key")!=NULL)
			{
				DWORD connectionId = pMplProtocol->getPortDescriptionHeaderConnection_id();
				DWORD partyId = pMplProtocol->getPortDescriptionHeaderParty_id();
				DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();

				CLargeString cstr;
				cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"pop_menu_status\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<STATUS value=\"1\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
				const char* msgStr = cstr.GetString();
				CSegment* paramSeg = new CSegment;
				DWORD msgLen = strlen(msgStr) + 1;
				paramSeg->Put((BYTE*)msgStr,msgLen);

				FillMplProtocol(pMplProtocol, PCM_COMMAND, (BYTE*)(paramSeg->GetPtr()), paramSeg->GetWrtOffset());
				pMplProtocol->AddPortDescriptionHeader(partyId, confId, connectionId);

				SendToCmForMplApi(*pMplProtocol);

				CSegment* timerSeg = new CSegment;
				*timerSeg << (DWORD)1;
				pMplProtocol->Serialize(*timerSeg);



				StartTimer(PCM_MENU_TIMER,5*SECOND,timerSeg);
				/*
				CLargeString cstr1;
				cstr1 << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"fecc_control\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<PANE_INDEX value=\"0\" />\n\t\t\t<STATUS value=\"1\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
				const char* msgStr1 = cstr1.GetString();
				CSegment* paramSeg1 = new CSegment;
				DWORD msgLen1 = strlen(msgStr1) + 1;
				paramSeg1->Put((BYTE*)msgStr1,msgLen1);

				FillMplProtocol(pMplProtocol, PCM_COMMAND, (BYTE*)(paramSeg1->GetPtr()), paramSeg1->GetWrtOffset());
				pMplProtocol->AddPortDescriptionHeader(partyId, confId, connectionId);

				SendToCmForMplApi(*pMplProtocol);

				*/
				POBJDELETE(paramSeg);
			}
			break;
			///TEMP
		}
		case PCM_CONFIRM:
		{
			///TEMP
			TRACEINTO	<< " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect opcode: PCM_CONFIRM";

			//added by huiyu
			char* pData = pMplProtocol->GetData();
			PTRACE2(eLevelError,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect,CONFIRM\n", pData);

			CSegment* pParamSeg = new CSegment;
			pMplProtocol->Serialize(*pParamSeg);
			::SendMessageToPCMSimApp(*pParamSeg);
			POBJDELETE(pParamSeg);
			break;
		}
//		case SEND_H_230:
//		{
//			// Send REMORE_CI as a response
//			TRACEINTO	<< " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect opcode: "
//						<< "SEND_H_230";
//
//			DWORD connectionId = pMplProtocol->getPortDescriptionHeaderConnection_id();
//			DWORD partyId = pMplProtocol->getPortDescriptionHeaderParty_id();
//			DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();
//
//			SEND_H_230_S  sendH320Struct;
//	  		char* pData = pMplProtocol->GetData();
//			memcpy(&sendH320Struct, pData, sizeof(SEND_H_230_S));
//			DWORD numOfBytes = sendH320Struct.h230_command.h230_bas.number_of_bytes;
//
//	 	 	pData = pData + sizeof(SEND_H_230_S);
//
//	 	 	CSegment sendH320Seg;
//  			sendH320Seg.Put((BYTE*)pData, numOfBytes);
//
//
//			*pParam >> opcode;
//			switch ( opcode ) {
//
//		    case (Start_Mbe | ESCAPECAPATTR)  :  {
//
//
//
//
//			CSegment* pRemoteBasCaps = new CSegment;
//			*pRemoteBasCaps 	<< numOfBytes
//								<< exchangeCapsSeg;
//
//
//			FillMplProtocol(pMplProtocol, REMOTE_BAS_CAPS, (BYTE*)(pRemoteBasCaps->GetPtr()), pRemoteBasCaps->GetWrtOffset());
//			pMplProtocol->AddPortDescriptionHeader(partyId, confId, connectionId, eLogical_mux);
//
//			SendToCmForMplApi(*pMplProtocol);
//
//			POBJDELETE(pRemoteBasCaps);
//			break;
//		}

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



//		case CM_CHECKSUM_REQ:
//		{
//			DWORD retStatus = STATUS_OK;
//			BYTE* pData   = (BYTE*)(pMplProtocol->GetData());
//			if (NULL == pData)
//				break;
//
//			switch( *pData )
//			{
//				case eStatusInactiveSimulation:
//					break;
//
//				case eStatusFailChecksumTest:		// error simulation
//				{
//					retStatus = STATUS_FAIL;
//					break;
//				}
//
//		 		default: {break;}
//			}
//
//			SystemSleep(100);	// 1 second delay
//
//			pMplProtocol->AddCommonHeader( CM_CHECKSUM_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );
//			pMplProtocol->AddData( 4, (char*)(&retStatus) );
//			SendToCmForMplApi(*pMplProtocol);	// send to MCMS
//
//			break;
//		}

		case SET_ECS:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle SET_ECS.");

			ForwardMuxBndReq2Endpoints(*pMplProtocol);

			break;
		}

	        case CM_UPGRADE_NEW_VERSION_READY_REQ:
		{

			StartVersionUpgrade();
		  break;
		}
	        case CM_UPGRADE_START_WRITE_IPMC_REQ:
		{
		  m_ipmc_software_upgrade_done = 0;
		  //StartTimer(SOFTWARE_IPMC_UPGRADE_TIMER,5);
		  break;
		}

		case H323_RTP_UPDATE_MT_PAIR_REQ:
		{	// we use this opcode for forwarding party's alias name to MM (for "CG audio improvements" feature)
			// forward to MM process if the system is CG
			if (eProductFamilyCallGenerator == curProductFamily)
			{
				TRACEINTO	<< " CGideonSimBarakLogical::OnCmProcessMcmsReqConnect opcode: "
							<< "H323_RTP_UPDATE_MT_PAIR_REQ";
				ForwardMsg2MediaMngr(*pMplProtocol);	// sends message to MM
			}

			Ack(*pMplProtocol, STATUS_OK);
			break;
		}


		case ICE_INIT_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle ICE_INIT_REQ.");
			OnIceInitReq(*pMplProtocol);
			break;
		}
#ifndef __DISABLE_ICE__
		case ICE_MAKE_ANSWER_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle ICE_MAKE_ANSWER_REQ.");
			OnIceMakeAnswerReq(*pMplProtocol);
			break;

		case ICE_MODIFY_SESSION_ANSWER_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle ICE_MODIFY_SESSION_ANSWER_REQ.");
			OnIceModifyAnswerReq(*pMplProtocol);
			break;

		case ICE_CLOSE_SESSION_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle ICE_CLOSE_SESSION_REQ.");
			OnIceCloseSessionReq(*pMplProtocol);
			break;

		case ICE_MAKE_OFFER_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle ICE_MAKE_OFFER_REQ.");
			OnIceMakeOfferReq(*pMplProtocol);
			break;

		case ICE_MODIFY_SESSION_OFFER_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle ICE_MODIFY_SESSION_OFFER_REQ.");
			OnIceModifyOfferReq(*pMplProtocol);
			break;

		case ICE_PROCESS_ANSWER_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle ICE_PROCESS_ANSWER_REQ.");
			OnIceProcessAnswerReq(*pMplProtocol);
			break;
#endif	//__DISABLE_ICE__
		case PARTY_DEBUG_INFO_REQ:
		{

			//PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle PARTY_DEBUG_INFO_REQ");

			// Simulate PARTY_DEBUG_INFO_IND and PARTY_CM_DEBUG_INFO_IND messages.
			// currently PARTY_DEBUG_INFO_IND is 256 bytes for video resources, and 5KB for audio resource.
			// PARTY_CM_DEBUG_INFO_IND has a static payload buffer of 20KB for each message.
			{
				// sends IND to the MCMS
				CMplMcmsProtocol*  pProt = new CMplMcmsProtocol(*pMplProtocol);
				pProt->AddCommonHeader( PARTY_DEBUG_INFO_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );

				char* pInfoStr   = new char[5120];
				memset(pInfoStr,'\0',5120);

				strcat(pInfoStr,"Port: Id=3,Type=3,Active=1,PartyId=2,ConfId=3\nStream: Media=3,Status=1,StreamSize=262144,StreamUsage=0,UnprocessedPackets=0,PacketsToConsume=0\nQueue: SizeInUse=0, MaxSize=400");

				PTRACE2(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle PARTY_DEBUG_INFO_REQ , STRING:\n",pInfoStr);
				pProt->AddData( 5120, (char*)pInfoStr );

				//Ack(*pMplProtocol,STATUS_OK);
				SendToCmForMplApi(*pProt);	// send to MCMS

				delete[] pInfoStr;
				POBJDELETE(pProt);


				// sends IND to the MCMS
				CMplMcmsProtocol*  pProt1 = new CMplMcmsProtocol(*pMplProtocol);
				pProt1->AddCommonHeader( PARTY_CM_DEBUG_INFO_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );

				char* pInfoStr1   = new char[DEBUG_INFO_STRING_SIZE];
				memset(pInfoStr1,'\0',DEBUG_INFO_STRING_SIZE);

				int maxcopies = 30;
				int randomize = rand() % 10;
				switch (randomize)
				{
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
						maxcopies = 4;
						break;

					case 7:
					case 8:
					case 9:
						maxcopies = 30;
						break;

					default:
						maxcopies = 30;
						break;
				}

				for (int i = 0; i < maxcopies; i++)
				{
					strcat(pInfoStr1,"***IncomingMultiStream - IncomingPrivateAudioIVRFromLocalIVRController\nStreamValid=1, StreamSizeUsed=0x20000, NumOfSequentialStreams=4, SourceOutLocalStreamNum=1655, LinkNumber=-1, Bandwidth=-1, ChanOpened?-0, MediaType=0, Dircection=0\nSrcPhysicalLocation: Box=1, Board=2, SubBoard=1, UnitID=0, PortID=-1, SrcResourceType=10, SrcOutJunction=11005, SrcOutgPCIAdd=0x33b8000\nRecordingPhysicalLocation: Box=-1, Board=-1, SubBoard=-1, UnitID=-1, PortID=-1, RecordingRemapAddr=0xffffffff\nDestPhysicalLocation: Box=1, Board=2, SubBoard=1, UnitID=1, PortID=-1, DestInJunction=100, DestInRemapAdd= 0x166000\n");
				}

				PTRACE2(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle PARTY_DEBUG_INFO_REQ , STRING:\n",pInfoStr1);
				pProt1->AddData( DEBUG_INFO_STRING_SIZE, (char*)pInfoStr1 );

				//Ack(*pMplProtocol,STATUS_OK);
				SendToCmForMplApi(*pProt1);	// send to MCMS

				delete[] pInfoStr1;
				POBJDELETE(pProt1);
			}
			break;
		}
		case CONF_DEBUG_INFO_REQ:
		  {

		    //PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle CONF_DEBUG_INFO_REQ");

			// sends IND to the MCMS
			CMplMcmsProtocol*  pProt = new CMplMcmsProtocol(*pMplProtocol);
			pProt->AddCommonHeader( CONF_DEBUG_INFO_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms );

			char* pInfoStr   = new char[4096];
			memset(pInfoStr,'\0',4096);

			for(int i=0;i<10;i++){
			  strcat(pInfoStr,"I am the audio controller, click chhhhh tshhhh \n");
			}

			PTRACE2(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle CONF_DEBUG_INFO_REQ , STRING:\n",pInfoStr);
			pProt->AddData( strlen(pInfoStr), (char*)pInfoStr );
			//pProt->AddData(11 , "blu blu blu" );


			//Ack(*pMplProtocol,STATUS_OK);
			SendToCmForMplApi(*pProt);	// send to MCMS

			delete[] pInfoStr;
			POBJDELETE(pProt);

			break;
		  }
		case IP_CM_TIP_START_NEGOTIATION_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle IP_CM_TIP_START_NEGOTIATION_REQ.");
			OnTipStartNegotiationReq(*pMplProtocol);
			break;
		case IP_CM_TIP_END_NEGOTIATION_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle IP_CM_TIP_END_NEGOTIATION_REQ.");
			OnTipEndNegotiationReq(*pMplProtocol);
			break;
		case IP_CM_RTCP_MSG_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle IP_CM_RTCP_MSG_REQ.");
			break;
		case IP_CM_RTCP_VSR_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle IP_CM_RTCP_VSR_REQ.");
			OnMSSendVSRReq(*pMplProtocol);
			break;

//		case IP_CM_TIP_KILL_OBJECT_REQ:
//			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle IP_CM_TIP_KILL_OBJECT_REQ.");
//			break;
		// status FAIL if request unrecognized

		case SET_LOG_LEVEL_REQ:
		{
		    CProcessBase* proc = CProcessBase::GetProcess();
		    PASSERT_AND_RETURN(NULL == proc);

		    TRACESTRFUNC(eLevelError) << "Ping "
                                         << proc->GetOpcodeAsString(opcode)
                                         << " (" << opcode << ")";

		    CMplMcmsProtocol mpl(*pMplProtocol);
		    mpl.AddCommonHeader(SET_LOG_LEVEL_IND,
                                MPL_PROTOCOL_VERSION_NUM,
                                0,
                                (BYTE)eMpl,
                                (BYTE)eMcms);

		    SendToCmForMplApi(mpl);
		    break;
		}
		case IP_CM_BFCP_MESSAGE_REQ:
		{
			//AckIp(*pMplProtocol,STATUS_OK);
			SendBfcpMessageInd(*pMplProtocol);
			break;
		}

		case IP_CM_DTLS_START_REQ:
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle IP_CM_DTLS_START_REQ.");
			SendDtlsEndInd(*pMplProtocol);// IP_CM_DTLS_END_IND
			break;

		case IP_CM_WEBRTC_CONNECT_REQ :
			PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMcmsReqConnect - Handle IP_CM_WEBRTC_CONNECT_REQ.");
			OnWebRtcConnectReq(*pMplProtocol);
			break;

		default:
		{
			Ack(*pMplProtocol, STATUS_OK);
			break;
		}
	}
	/* temp for Talya - end */

	POBJDELETE(pMplProtocol);
}

void CGideonSimBarakLogical::OnTipStartNegotiationReq(CMplMcmsProtocol &rMplProt)
{
	mcTipNegotiationResultInd tipNegotiationResInd;
	memset(&tipNegotiationResInd, 0, sizeof(mcTipNegotiationResultInd));
	SetNegotiationResultParams(tipNegotiationResInd);
	// sends IND to the MCMS
	CMplMcmsProtocol*  pProt = new CMplMcmsProtocol(rMplProt);
	pProt->AddCommonHeader(IP_CM_TIP_NEGOTIATION_RESULT_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms);
	pProt->AddData(sizeof(mcTipNegotiationResultInd), (char*)(&tipNegotiationResInd));
	SendToCmForMplApi(*pProt);	// send to MCMS
	POBJDELETE(pProt);

	SystemSleep(100);          // 1 second delay

	pProt = new CMplMcmsProtocol(rMplProt);
	pProt->AddCommonHeader(IP_CM_TIP_LAST_ACK_RECEIVED_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms);
	SendToCmForMplApi(*pProt);  // send to MCMS
	POBJDELETE(pProt);

}
////////////////////////////////////////////////////////////////////////////
void  CGideonSimBarakLogical::SetNegotiationResultParams(mcTipNegotiationResultInd &tipNegotiationResultInd)
{
	tipNegotiationResultInd.status = STATUS_OK;
	tipNegotiationResultInd.doVideoReInvite = 1;
	TipNegotiationSt *negotiationSt = &(tipNegotiationResultInd.negotiationSt);
	// Set system wide mux params:
	TipSystemWideOptionsSt* pSystemWideOptions = &(negotiationSt->systemwideOptions);
	pSystemWideOptions->rtpProfile = eRtpAvpf;
	pSystemWideOptions->deviceOptions = eTipDeviceTranscoding;
	pSystemWideOptions->confId = 0;

	// Set audio mux params:
	TipAudioMuxCntlSt* pAudioMux = &(negotiationSt->audioMuxCntl);
	pAudioMux->txPositions[eTipAudioPosCenter] = 1;
	pAudioMux->txPositions[eTipAudioPosLeft] = 1;
	pAudioMux->txPositions[eTipAudioPosRight] = 1;
	pAudioMux->txPositions[eTipAudioPosAux] = 1;
	pAudioMux->rxPositions[eTipAudioPosCenter] = 1;
	pAudioMux->rxPositions[eTipAudioPosLeft] = 1;
	pAudioMux->rxPositions[eTipAudioPosRight] = 1;
	pAudioMux->rxPositions[eTipAudioPosAux] = 1;

	// Set video mux params:
	TipVideoMuxCntlSt* pVideoMux = &(negotiationSt->videoMuxCntl);

	pVideoMux->txPositions[eTipVideoPosCenter] = 1;
	pVideoMux->txPositions[eTipVideoPosLeft] = 1;
	pVideoMux->txPositions[eTipVideoPosRight] = 1;
	pVideoMux->rxPositions[eTipVideoPosCenter] = 1;
	pVideoMux->rxPositions[eTipVideoPosLeft] = 1;
	pVideoMux->rxPositions[eTipVideoPosRight] = 1;

	// Set audio options params:
	// Tx:
	TipAudioOptionsSt* pAudioOptionTx = &(negotiationSt->audioTxOptions);
	pAudioOptionTx->IsAudioDynamicOutput = 0;
	pAudioOptionTx->IsAudioActivityMetric = 0;
	pAudioOptionTx->G722Negotiation = 0;
	pAudioOptionTx->genericOptions.IsEKT = 0;
	// Rx:
	TipAudioOptionsSt* pAudioOptionRx = &(negotiationSt->audioRxOptions);
	memcpy(pAudioOptionTx, pAudioOptionRx, sizeof(TipAudioOptionsSt));

	// Set video options params:
	// Tx:
	TipVideoOptionsSt* pVideoOptionTx = &(negotiationSt->videoTxOptions);
	pVideoOptionTx->IsVideoRefreshFlag = 0;
	pVideoOptionTx->IsInbandParameterSets = 1;
	pVideoOptionTx->IsCABAC = 0;
	pVideoOptionTx->IsLTRP = 0;
	pVideoOptionTx->AuxVideoFPS = eTipAux5FPS;
	pVideoOptionTx->IsGDR = 0;
	pVideoOptionTx->IsHighProfile = 1;
	pVideoOptionTx->IsUnrestrictedMediaXGA5or1 = 1;
	pVideoOptionTx->IsUnrestrictedMediaXGA30 = 1;
	pVideoOptionTx->IsUnrestrictedMedia720p = 1;
	pVideoOptionTx->IsUnrestrictedMedia1080p = 1;
	pVideoOptionTx->genericOptions.IsEKT = 0;
	// Rx:
	TipVideoOptionsSt* pVideoOptionRx = &(negotiationSt->videoRxOptions);
	memcpy(pVideoOptionRx, pVideoOptionTx, sizeof(TipVideoOptionsSt));
}
////////////////////////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnTipEndNegotiationReq(CMplMcmsProtocol &rMplProt)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnTipEndNegotiationReq");
//	SendVideoRefreshInd(rMplProt, kIpVideoChnlType, cmCapTransmit, RTCP_TIP_IDR, eTipVideoPosCenter); // Send intra request (for testing)
//	SendVideoRefreshInd(rMplProt, kIpVideoChnlType, cmCapTransmit, RTCP_TIP_IDR, eTipVideoPosLeft); // Send intra request (for testing)
//	SendVideoRefreshInd(rMplProt, kIpVideoChnlType, cmCapTransmit, RTCP_TIP_IDR, eTipVideoPosRight); // Send intra request (for testing)
//	SendVideoRefreshInd(rMplProt, kIpVideoChnlType, cmCapTransmit, RTCP_TIP_GDR, eTipVideoPosCenter); // Send intra request (for testing)
//	SendVideoRefreshInd(rMplProt, kIpVideoChnlType, cmCapTransmit, RTCP_TIP_GDR, eTipVideoPosLeft); // Send intra request (for testing)
//	SendVideoRefreshInd(rMplProt, kIpVideoChnlType, cmCapTransmit, RTCP_TIP_GDR, eTipVideoPosRight); // Send intra request (for testing)

//	// sends IND to the MCMS
//	mcTipContentMsgInd tipContentMsgInd;
//	CMplMcmsProtocol*  pProt1 = new CMplMcmsProtocol(rMplProt);
//	memset(&tipContentMsgInd, 0, sizeof(mcTipContentMsgInd));
//	pProt1->AddCommonHeader(IP_CM_TIP_CONTENT_MSG_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms);
//	pProt1->AddData(sizeof(mcTipContentMsgInd), (char*)(&tipContentMsgInd));
//	SendToCmForMplApi(*pProt1);	// send to MCMS
//	POBJDELETE(pProt1);
}
///////////////////////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnMSSendVSRReq(CMplMcmsProtocol &rMplProt)
{
	TCmRtcpVsrMsg vsrInd;
	memset(&vsrInd, 0, sizeof(TCmRtcpVsrMsg));
	SetVSRParams(vsrInd);
	// sends IND to the MCMS
	CMplMcmsProtocol*  pProt = new CMplMcmsProtocol(rMplProt);
	pProt->AddCommonHeader(IP_CM_RTCP_VSR_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms);
	pProt->AddData(sizeof(TCmRtcpVsrMsg), (char*)(&vsrInd));
	SendToCmForMplApi(*pProt);	// send to MCMS
	POBJDELETE(pProt);



}
///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void  CGideonSimBarakLogical::SetVSRParams(TCmRtcpVsrMsg &msVsrInd)
{

	msVsrInd.cmRtcpVsrInfo.senderSSRC = 0xFFFFFFFE;
	msVsrInd.cmRtcpVsrInfo.MSI = 0xFFFFFFFE;
	msVsrInd.cmRtcpVsrInfo.requestId = 1;
	msVsrInd.cmRtcpVsrInfo.keyFrame = 1;
	msVsrInd.cmRtcpVsrInfo.numberOfEntries = 2;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].minBitRate = 350000;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].bitRatePerLevel = 10000;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].frameRateBitmask = 4;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].maximumNumberOfPixels = 103680;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].numberOfMustInstances = 0;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].numberOfMayInstances = 1;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].maxWidth = 432;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].maxHeight = 432;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].bitRateHistogram[0] = 1;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].payloadType = 122;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].flags = 0;
	msVsrInd.cmRtcpVsrInfo.VSREntry[0].aspectRatioBitMask = 1;


	msVsrInd.cmRtcpVsrInfo.VSREntry[1].minBitRate = 350000;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].bitRatePerLevel = 10000;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].frameRateBitmask = 4;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].maximumNumberOfPixels = 103680;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].numberOfMustInstances = 0;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].numberOfMayInstances = 1;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].maxWidth = 432;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].maxHeight = 432;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].bitRateHistogram[0] = 1;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].payloadType = 121;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].flags = 0;
	msVsrInd.cmRtcpVsrInfo.VSREntry[1].aspectRatioBitMask = 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//void  CGideonSimBarakLogical::SetVSRParams(TCmRtcpVsrMsg &msVsrInd)
//{
//
//	msVsrInd.cmRtcpVsrInfo.senderSSRC = 0xFFFFFFFE;
//	msVsrInd.cmRtcpVsrInfo.MSI = 0xFFFFFFFE;
//	msVsrInd.cmRtcpVsrInfo.requestId = 1;
//	msVsrInd.cmRtcpVsrInfo.keyFrame = 1;
//	msVsrInd.cmRtcpVsrInfo.numberOfEntries = 1;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].minBitRate = 350000;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].bitRatePerLevel = 10000;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].frameRateBitmask = 4;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].maximumNumberOfPixels = 103680;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].numberOfMustInstances = 0;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].numberOfMayInstances = 1;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].maxWidth = 432;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].maxHeight = 432;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].bitRateHistogram[0] = 1;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].payloadType = 121;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].flags = 0;
//	msVsrInd.cmRtcpVsrInfo.VSREntry[0].aspectRatioBitMask = 1;
//}

////////////////////////////////////////////////////////////////////////////////////////////////


void CGideonSimBarakLogical::OnIceInitReq(CMplMcmsProtocol &rMplProt)
{
	ICE_INIT_REQ_S *iceInitREQ=(ICE_INIT_REQ_S*)rMplProt.GetData();
	if(!iceInitREQ) {
		PTRACE(eLevelError,"CGideonSimBarakLogical::OnIceInitReq-- No ICE_INIT_REQ_S in CMplMcmsProtocol message");
		assert(0);
		return;
	}
	//m_isIceInited=m_iceProcessor->Init(rMplProt);

	ICE_INIT_IND_S  iceInitIND;
	memset(&iceInitIND,0,sizeof(iceInitIND));
	iceInitIND.req_id=iceInitREQ->ice_servers.req_id;
	iceInitIND.status=  STATUS_OK;
	iceInitIND.STUN_Pass_status=eIceServerUnavailble;
	iceInitIND.STUN_udp_status=eIceInitOk;//eIceServerUnavailble;
	iceInitIND.STUN_tcp_status=eIceInitOk;//eIceServerUnavailble;
	iceInitIND.Relay_udp_status=eIceInitOk;
	iceInitIND.Relay_tcp_status=eIceInitOk;//eIceServerUnavailble;
	iceInitIND.fw_type=eFwTypeUdp;

	rMplProt.AddCommonHeader(ICE_INIT_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(sizeof(iceInitIND),(char*)(&iceInitIND));

	//N.A. DEBUG testing for ICE_STATUS_IND webrtc
	if( !IsValidTimer(ICE_SEND_STAT_TIMER) )
	{
		PTRACE(eLevelError,"N.A. DEBUG  CGideonSimBarakLogical::OnIceInitReq--set timer ");
		StartTimer(ICE_SEND_STAT_TIMER, 1);
	}


	SendToCmForMplApi(rMplProt);
}

// N.A. DEBUG sim ICE (webrtc status IND)
void CGideonSimBarakLogical::OnIceStatusIndSend()
{
	ICE_STATUS_IND_S				iceStatusIND;
	memset(&iceStatusIND,0,sizeof(iceStatusIND));

	PTRACE(eLevelError,"N.A. DEBUG  CGideonSimBarakLogical::OnIceStatusIndSend-- TIMES UP !!  ");

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	iceStatusIND.STUN_udp_status=eIceInitOk;//eIceServerUnavailble;
	iceStatusIND.STUN_tcp_status=eIceInitOk;//eIceServerUnavailble;
	iceStatusIND.Relay_udp_status=eIceInitOk;
	iceStatusIND.Relay_tcp_status=eIceInitOk;//eIceServerUnavailble;
	iceStatusIND.fw_type=eFwTypeUnknown;
	iceStatusIND.ice_env=eIceEnvironment_WebRtc;

	FillMplProtocol( pMplProtocol,ICE_STATUS_IND,
				(BYTE*)(&iceStatusIND),sizeof(ICE_STATUS_IND_S));

	SendToCmForMplApi(*pMplProtocol);

	POBJDELETE(pMplProtocol);


	/*
	 typedef enum {

	eIceInitOk,
	eIceInitServerFail,
	eIceStunPassServerAuthenticationFailure,// STUN password server authentication failure
	eIceStunPassServerConnectionFailure, 	// Failed to connect to the STUN password server
	eIceTurnServerDnsResolveFailure,		// DNS resolution failure for the TURN server
	eIceTurnServerUnreachable,				// TURN server is not reachable
	eIceTurnServerAuthorizationFailure,		// TURN authorization failure while trying to allocate a port
	eIceServerUnavailble,
	eIceUnknownProblem

} iceServersStatus;
 */

}






#ifndef __DISABLE_ICE__
void CGideonSimBarakLogical::OnIceMakeAnswerReq( CMplMcmsProtocol& rMplProt )
{
	if(!m_isIceInited) {
		PTRACE(eLevelError,"CGideonSimBarakLogical: Can not process ICE request, \
because the ICE stack is not initiated yet");
		assert(0);
		return;
	}

	ICE_GENERAL_REQ_S *makeAnswerReq=(ICE_GENERAL_REQ_S*)rMplProt.GetData();
	if(!makeAnswerReq) {
		PTRACE(eLevelError,"CGideonSimBarakLogical::OnIceMakeAnswerReq-- \
No ICE_MAKE_ANSWER_REQ_S in CMplMcmsProtocol message");
		assert(0);
		return;
	}

	int sessID;
	if(!m_iceProcessor->MakeAnswer(rMplProt, sessID)) {
		//failed? which indication should be replied?
	}

#if 0
	int size;
	auto_array<char> msgbuf=IceProcessor::BuildIceSdpIndByReq(*makeAnswerReq, localSdp, size);

	rMplProt.AddCommonHeader(ICE_MAKE_ANSWER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(size,msgbuf.c_array());

	SendToCmForMplApi(rMplProt);
#endif

    CSegment* pParamSeg = new CSegment;
    *pParamSeg << (DWORD)(makeAnswerReq->ice_session_index);
    rMplProt.Serialize(*pParamSeg);

    StartTimer (CHECK_COMPLETE_TIMER, 2*SECOND, pParamSeg);
}

void CGideonSimBarakLogical::OnIceModifyAnswerReq(CMplMcmsProtocol& rMplProt)
{
	if(!m_isIceInited) {
		PTRACE(eLevelError,"CGideonSimBarakLogical: Can not process ICE request, \
			because the ICE stack is not initiated yet");
			assert(0);
			return;
	}

	ICE_GENERAL_REQ_S *ModifyAnswerReq=(ICE_GENERAL_REQ_S*)rMplProt.GetData();
	if(!ModifyAnswerReq) {
		PTRACE(eLevelError,"CGideonSimBarakLogical::OnIceModifyAnswerReq-- \
		No ICE_MODIFY_SESSION_ANSWER_REQ_S in CMplMcmsProtocol message");
		assert(0);
		return;
	}

	///int sessID;
	if(!m_iceProcessor->MakeAnswerNoCreate(rMplProt)) {
		//failed? which indication should be replied?
	}

#if 0
	int size;
	auto_array<char> msgbuf=IceProcessor::BuildIceSdpIndByReq(*ModifyAnswerReq, localSdp, size);

	rMplProt.AddCommonHeader(ICE_MODIFY_SESSION_ANSWER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(size,msgbuf.c_array());

	SendToCmForMplApi(rMplProt);
#endif

	CSegment* pParamSeg = new CSegment;
	*pParamSeg << (DWORD)(ModifyAnswerReq->ice_session_index);
	rMplProt.Serialize(*pParamSeg);

	StartTimer (CHECK_COMPLETE_TIMER, 2*SECOND, pParamSeg);

}


void CGideonSimBarakLogical::OnIceCloseSessionReq( CMplMcmsProtocol& rMplProt )
{
	if(!m_isIceInited) {
		PTRACE(eLevelError,"CGideonSimBarakLogical: Can not process ICE request, \
because the ICE stack is not initiated yet");
		assert(0);
		return;
	}

	ICE_CLOSE_SESSION_REQ_S *closeReq=(ICE_CLOSE_SESSION_REQ_S*)rMplProt.GetData();
	if(!closeReq) {
		PTRACE(eLevelError,"CGideonSimBarakLogical::OnIceCloseSessionReq-- \
No ICE_CLOSE_SESSION_REQ_S in CMplMcmsProtocol message");
		assert(0);
		return;
	}

	bool status=m_iceProcessor->CloseSession(rMplProt);
}


void CGideonSimBarakLogical::OnIceMakeOfferReq( CMplMcmsProtocol& rMplProt )
{
	if(!m_isIceInited) {
		PTRACE(eLevelError,"CGideonSimBarakLogical: Can not process ICE request, \
because the ICE stack is not initiated yet");
		assert(0);
		return;
	}

	ICE_GENERAL_REQ_S *makeOfferReq=(ICE_GENERAL_REQ_S*)rMplProt.GetData();
	if(!makeOfferReq) {
		PTRACE(eLevelError,"CGideonSimBarakLogical::OnIceMakeOfferReq-- \
No ICE_MAKE_OFFER_REQ_S in CMplMcmsProtocol message");
		assert(0);
		return;
	}


	int sessID;
	if(!m_iceProcessor->MakeOffer(rMplProt, sessID)) {
		//failed? which indication should be replied?
	}


#if 0
	int size;
	auto_array<char> msgbuf=IceProcessor::BuildIceSdpIndByReq(*makeOfferReq, localSdp, size);

	rMplProt.AddCommonHeader(ICE_MAKE_OFFER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(size,msgbuf.c_array());

	SendToCmForMplApi(rMplProt);
#endif
}


void CGideonSimBarakLogical::OnIceModifyOfferReq( CMplMcmsProtocol& rMplProt )
{
	if(!m_isIceInited) {
		PTRACE(eLevelError,"CGideonSimBarakLogical: Can not process ICE request, \
		because the ICE stack is not initiated yet");
		assert(0);
		return;
	}

	ICE_GENERAL_REQ_S *ModifyOfferReq=(ICE_GENERAL_REQ_S*)rMplProt.GetData();
	if(!ModifyOfferReq) {
		PTRACE(eLevelError,"CGideonSimBarakLogical::OnIceModifyOfferReq-- \
		No ICE_MODIFY_SESSION_OFFER_REQ in CMplMcmsProtocol message");
		assert(0);
		return;
	}

	m_iceProcessor->MakeOfferNoCreate(rMplProt);

#if 0
	int size;
	auto_array<char> msgbuf=IceProcessor::BuildIceSdpIndByReq(*ModifyOfferReq, localSdp, size);

	rMplProt.AddCommonHeader(ICE_MODIFY_SESSION_OFFER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(size,msgbuf.c_array());

	SendToCmForMplApi(rMplProt);
#endif
}

void CGideonSimBarakLogical::OnIceProcessAnswerReq( CMplMcmsProtocol& rMplProt )
{
	if(!m_isIceInited) {
		PTRACE(eLevelError,"CGideonSimBarakLogical: Can not process ICE request, \
because the ICE stack is not initiated yet");
		assert(0);
		return;
	}

	ICE_GENERAL_REQ_S *processAnswerReq=(ICE_GENERAL_REQ_S*)rMplProt.GetData();
	if(!processAnswerReq) {
		PTRACE(eLevelError,"CGideonSimBarakLogical::OnIceProcessAnswerReq-- \
No processAnswerReq in CMplMcmsProtocol message");
		assert(0);
		return;
	}

	bool status=m_iceProcessor->ProcessAnswer(rMplProt);

	//int party_id=rMplProt.getPortDescriptionHeaderParty_id();
	//IceParseSdp(m_IcePartiesRemoteArr[party_id]
	//			,processAnswerReq->sdp_size
	//			,processAnswerReq->sdp);

//need be moved into callback function
#if 0
    //Check complete indication
    ICE_CHECK_COMPLETE_IND_S CheckCompleteInd;
    memset (&CheckCompleteInd, 0, sizeof(CheckCompleteInd));
    rMplProt.AddCommonHeader(ICE_CHECK_COMPLETE_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms);
    BuildCheckCompleteInd (&CheckCompleteInd, processAnswerReq->ice_session_index, party_id);

    rMplProt.AddData(sizeof(CheckCompleteInd), (char*)&CheckCompleteInd);
	SendToCmForMplApi(rMplProt);
	//build and send   Re-INVITE  Indication
	int size;
	auto_array<char> msgbuf=BuildIceReInviteInd(*processAnswerReq, m_IcePartiesRemoteArr[party_id], size);

	rMplProt.AddCommonHeader(ICE_REINVITE_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(size,msgbuf.c_array());

	SendToCmForMplApi(rMplProt);
#endif
}

void CGideonSimBarakLogical::OnTimerCheckComplete (CSegment * pParam)
{
	PTRACE (eLevelInfoNormal, "CGideonSimBarakLogical::OnTimerCheckComplete");
	m_iceProcessor->OnTimerCheckComplete(pParam);

#if 0
    DWORD sessionId = 0;
    CMplMcmsProtocol rMplProt;
    *pParam >> sessionId;

    rMplProt.DeSerialize(*pParam);
    int party_id=rMplProt.getPortDescriptionHeaderParty_id();

    //Check complete indication
    ICE_CHECK_COMPLETE_IND_S CheckCompleteInd;
    memset (&CheckCompleteInd, 0, sizeof(CheckCompleteInd));
    rMplProt.AddCommonHeader(ICE_CHECK_COMPLETE_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms);
    BuildCheckCompleteInd (&CheckCompleteInd, sessionId, party_id);

    rMplProt.AddData(sizeof(CheckCompleteInd), (char*)&CheckCompleteInd);
	SendToCmForMplApi(rMplProt);
#endif
}
#endif	//__DISABLE_ICE__

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnCmProcessEpSimMsgConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessEpSimMsgConnect");

	DWORD	opcode;
	WORD	boardId, subBoardId, unitId, portId;

	*pMsg	>> opcode
			>> boardId
			>> subBoardId
			>> unitId;

	if(CONF_PARTY_MRMP_SCP_STREAM_REQ_IND == opcode) {
		DWORD confId = 0;
		DWORD partyId = 0;
		DWORD connId = 0;
		DWORD channelId = 0;
		WORD  numberOfStreams = 0;

		*pMsg >> portId
		>> confId
		>> partyId
		>> connId
		>> channelId
		>> numberOfStreams;

		TRACEINTO << "CGideonSimBarakLogical::OnCmProcessEpSimMsgConnect - CONF_PARTY_MRMP_SCP_STREAM_REQ_IND number of streams = "
				  << numberOfStreams;

		int structLen = sizeof(MrmpScpStreamsRequestStruct) + sizeof(MrmpStreamDesc)*(numberOfStreams - 1);

		MrmpScpStreamsRequestStruct*   tStruct = (MrmpScpStreamsRequestStruct*)new char[structLen]; AUTO_DELETE_ARRAY(tStruct);

		tStruct->nNumberOfMediaStream = numberOfStreams;
		tStruct->unChannelHandle = channelId;
		tStruct->unSequenseNumber = 0;
		memset(&tStruct->mediaStreams[0], 0, sizeof(MrmpStreamDesc)*tStruct->nNumberOfMediaStream);
		memcpy(&tStruct->mediaStreams[0], pMsg->GetPtr(true), sizeof(MrmpStreamDesc)*tStruct->nNumberOfMediaStream);

		CMplMcmsProtocol*  pMplProt = new CMplMcmsProtocol;

		pMplProt->AddCommonHeader(CONF_PARTY_MRMP_SCP_STREAM_REQ_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
		pMplProt->AddPhysicalHeader(0,boardId,subBoardId,unitId,portId,0,ePhysical_mrmp);
		pMplProt->AddPortDescriptionHeader(partyId,confId,connId);

		pMplProt->AddData(structLen,(char*)tStruct);

		SendToCmForMplApi(*pMplProt);
		POBJDELETE(pMplProt);

		return;
	}


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
			case IP_RTP_FECC_KEY_IND:
			{
				TRtpFeccTokenRequestInd   tStruct;
				int size = sizeof(TRtpFeccTokenRequestInd);
				pMsg->Get((BYTE*)&tStruct,size);
				tStruct.unChannelType      = kIpFeccChnlType;
				tStruct.unChannelDirection = cmCapReceive;

				CMplMcmsProtocol*  pMplProt = new CMplMcmsProtocol;

				pMplProt->AddCommonHeader(opcode,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
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
			case REMOTE_ECS:
			{
//				PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessEpSimMsgConnect **** REMOTE_ECS ****");

				*pMsg >> portId;	// MUX portID

				DWORD	nDataLen = pMsg->GetWrtOffset() - pMsg->GetRdOffset();
				BYTE*	pData = new BYTE [nDataLen]; AUTO_DELETE_ARRAY(pData);
				pMsg->Get(pData,nDataLen);
				REMOTE_ECS_SE* pRemoteEcsInd = (REMOTE_ECS_SE*)pData;

//				for (WORD i=0; i<sizeof(SET_ECS_S)+20-sizeof(DWORD); i++)
//					TRACEINTO << "****** ECS - gsim *******"<< (DWORD)pData[i];


				DWORD muxConnectionID = pTempParty->GetConnectionId();
				DWORD confId = pParty->GetConfId();
				DWORD partyId = pParty->GetPartyId();

				CMplMcmsProtocol* pMplProt = new CMplMcmsProtocol;

				FillMplProtocol(pMplProt, REMOTE_ECS, (BYTE*)(pRemoteEcsInd), nDataLen );

				pMplProt->AddPortDescriptionHeader(partyId,confId,muxConnectionID, eLogical_mux);

				SendToCmForMplApi(*pMplProt);

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


/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnCmProcessMMMsgConnect(CSegment* pMsg)
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMMMsgConnect - ERROR: Recieved msg from MediaMngr process ??? - system is not CG!!");
		return;
	}

	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnCmProcessMMMsgConnect");

	// Following DWORD was WORD
	DWORD	opcode;
	DWORD	boardId, subBoardId, unitId, portId;

	*pMsg	>> opcode
			>> boardId
			>> subBoardId
			>> unitId;

	// print to trace
	CLargeString traceStr = "";
		traceStr << "CGideonSimBarakLogical::OnCmProcessMMMsgConnect:\n";
		traceStr << "================================================\n";
		traceStr << "\n indication      : " << opcode;
		traceStr << "\n boardId         : " << boardId;
		traceStr << "\n subBoardId      : " << subBoardId;
		traceStr << "\n unitId          : " << unitId;

	if (opcode == IP_RTP_VIDEO_UPDATE_PIC_IND)
	{
		DWORD	conferenceId;
		DWORD	partyId;
		DWORD	connectionId;

		*pMsg	>> conferenceId
				>> partyId
				>> connectionId;

		traceStr << "\n conferenceId    : " << conferenceId;
		traceStr << "\n partyId         : " << partyId;
		traceStr << "\n connectionId    : " << connectionId;

		TRACESTR(eLevelInfoNormal) << traceStr.GetString();

		SendRtpVideoUpdateInd(conferenceId, partyId, connectionId, unitId);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::SendRtpVideoUpdateInd(DWORD conferenceId, DWORD partyId, DWORD connectionId, DWORD unitId) const
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::SendRtpVideoUpdateInd - ERROR: tried to send message ""from"" MediaMngr process - system is not CG!!");
		return;
	}

	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::SendRtpVideoUpdateInd - SENDS 'Intra Request' to MPL-API.");

	TRtpVideoUpdatePictureInd rtpVideoUpdateInd;

	rtpVideoUpdateInd.unChannelType			= kIpVideoChnlType;
	rtpVideoUpdateInd.unChannelDirection	= cmCapReceive;

	CMplMcmsProtocol*  pMplProt = new CMplMcmsProtocol;

	pMplProt->AddCommonHeader(IP_RTP_VIDEO_UPDATE_PIC_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	pMplProt->AddPhysicalHeader(0,m_wBoardId,m_wSubBoardId,unitId,0,0,ePhysical_art);
	pMplProt->AddPortDescriptionHeader(partyId,conferenceId,connectionId,eLogical_rtp,eLogical_rtp);

	pMplProt->AddData(sizeof(TRtpVideoUpdatePictureInd),(char*)&rtpVideoUpdateInd);

	SendToCmForMplApi(*pMplProt);
	POBJDELETE(pMplProt);
}


/////////////////////////////////////////////////////////////////////////////
int CGideonSimBarakLogical::UpdateBondingParameters(DWORD unit, DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::UpdateBondingParameters ");

	if (!m_units)
		return -1;	// should not happen

	// update bonding parameters (number of channels)
	int status = m_units->UpdateBondingParameters( unit, port, muxConnectionID, confId, partyId, numOfChnls);

	return status;
}


/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnTimerUnitConfigTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnTimerUnitConfigTout - UNIT CONFIG time is over.");

	CmUnitLoadedInd();

	/// Vasily - temp for Hagai - start
	//  When Hagai will implement MEDIA_CONFIG_REQ - I'll remove it
/*	m_state = CONNECT;

	if( CPObject::IsValidPObjectPtr(m_pTaskApi) ) {
		CSegment*   pSeg = new CSegment;
		*pSeg	<< m_wBoardId
				<< m_wSubBoardId;
		m_pTaskApi->SendLocalMessage(pSeg,BARAK_CONNECTED);
	}
	/// Vasily - temp for Hagai - end
*/
}






/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnTimerMediaConfigTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnTimerMediaConfigTout - MEDIA CONFIG time is over.");

	CmMediaIpConfigInd();

/*	m_state = CONNECT;

	if( CPObject::IsValidPObjectPtr(m_pTaskApi) ) {
		CSegment*   pSeg = new CSegment;
		*pSeg	<< m_wBoardId
				<< m_wSubBoardId;
		m_pTaskApi->SendLocalMessage(pSeg,BARAK_CONNECTED);
	}*/
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnTimerCardsDelayTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnTimerCardsDelayTout - Wait for CM_UNIT_CONFIG_REQ.");

	CardManagerLoadedInd();
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnTimerSpeakerChangeTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnTimerSpeakerChangeTout - Send CHANGE_SPEAKER.");

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

	if( NULL != pParty ) {
			// restart timer
		StartTimer(BARAK_SPEAKER_CHANGE_TIMER,::GetGideonSystemCfg()->GetMfaSpeakerChangeTime()*SECOND);
			// send AC_ACTIVE_SPEAKER_IND
/*		TAcAudioSpeakerInd   tStruct;
		tStruct.unPartyID = pParty->GetPartyId();

		CMplMcmsProtocol*  pMplProt = new CMplMcmsProtocol;

		pMplProt->AddCommonHeader(AC_ACTIVE_SPEAKER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
		pMplProt->AddPhysicalHeader(0,m_wBoardId,m_wSubBoardId,0,0,0,ePhysical_art_light);
		*/
//		pMplProt->AddPortDescriptionHeader(INVALID/*pParty->GetPartyId()*/,pParty->GetConfId(),INVALID/*pParty->GetConnectionId()*/);

/*		pMplProt->AddData(sizeof(TAcAudioSpeakerInd),(char*)&tStruct);

		SendToCmForMplApi(*pMplProt);
		POBJDELETE(pMplProt);*/
		SendActiveSpeakerInd(*pParty);
	}
	// else - no live party found, do not restart timer
}

/////////////////////////////////////////////////////////////////////////////
CAudioParty* CGideonSimBarakLogical::FindParty(const CMplMcmsProtocol& rMpl)
{
	CAudioParty* pParty = NULL;

	int i = 0;
	for( i=0; i<MAX_AUDIO_PARTIES && pParty == NULL; i++ )
		if( m_raAudioPartiesArr[i].GetConfId() == rMpl.getPortDescriptionHeaderConf_id() )
			if( m_raAudioPartiesArr[i].GetPartyId() == rMpl.getPortDescriptionHeaderParty_id() )
				pParty = &(m_raAudioPartiesArr[i]);

	return pParty;
}

/////////////////////////////////////////////////////////////////////////////
CAudioParty* CGideonSimBarakLogical::FindParty(const CAudioParty& other)
{
	CAudioParty* pParty = NULL;

	int i = 0;
	for( i=0; i<MAX_AUDIO_PARTIES && pParty == NULL; i++ )
		if( m_raAudioPartiesArr[i].GetConfId() == other.GetConfId() )
			if( m_raAudioPartiesArr[i].GetPartyId() == other.GetPartyId() )
				pParty = &(m_raAudioPartiesArr[i]);

	return pParty;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CGideonSimBarakLogical::IsValidParty(const CMplMcmsProtocol& rMplProt) //const
{

	CAudioParty* pParty = FindParty(rMplProt);

	if( pParty == NULL )
		return FALSE;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// Indications from card to MCMS
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::CardManagerLoadedInd() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::CardManagerLoadedInd - SEND to MPL-API.");

	// send CM_CARD_MNGR_LOADED_IND
	CM_CARD_MNGR_LOADED_S  rStruct;
	memset(&rStruct,0,sizeof(CM_CARD_MNGR_LOADED_S));

	string mplSerialNum = ::GetGideonSystemCfg()->GetBoxChassisId();
	int len = mplSerialNum.length();
	memcpy(rStruct.serialNum, ::GetGideonSystemCfg()->GetBoxChassisId(), len);

	rStruct.status    = STATUS_OK;
	rStruct.cardType  = m_cardType; //eMpmPlus_40;	// this important parameter actually tells the MCMS how
									// many units we have on card

	TRACEINTO  << " CGideonSimBarakLogical::CardManagerLoadedInd -cardtype "
	        << (int)m_cardType ;

	rStruct.hardwareVersion.ver_release  = 1;
	rStruct.hardwareVersion.ver_major    = 2;
	rStruct.hardwareVersion.ver_minor    = 3;
	rStruct.hardwareVersion.ver_internal = 4;

	for (int i = 0; i < MAX_NUM_OF_SW_VERSIONS; i++)
	{
	  strcpy((char*) rStruct.swVersionsList[i].versionDescriptor, "The Description...");
	  strcpy((char*) rStruct.swVersionsList[i].versionNumber, "12345");
	}

	CCardCfgBarak* pCurrCardCfg = NULL;
	pCurrCardCfg = (CCardCfgBarak*)::GetGideonSystemCfg()->GetCardCfg( m_wBoardId );
	UNIT_S currUnit;
	for (int i=0; i<MAX_NUM_OF_UNITS && pCurrCardCfg; i++)
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

//	m_state = CONFIG;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::CmUnitLoadedInd() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::CmUnitLoadedInd - SEND to MPL-API.");

	// send CM_UNIT_LOADED_IND
	CM_UNIT_LOADED_S  rStruct;
	memset(&rStruct,0,sizeof(CM_UNIT_LOADED_S));

	rStruct.status = STATUS_OK;

/*	rStruct.statusOfUnitsList[MAX_NUM_OF_UNITS/2]     = 1;
	rStruct.statusOfUnitsList[MAX_NUM_OF_UNITS/2 + 1] = 2;
	rStruct.statusOfUnitsList[MAX_NUM_OF_UNITS/2 + 2] = 3;
	rStruct.statusOfUnitsList[MAX_NUM_OF_UNITS/2 + 3] = 6;
//	rStruct.statusOfUnitsList[MAX_NUM_OF_UNITS - 1]   = 6;
*/
	m_units->GetUnitsStatus( (APIU32*)rStruct.statusOfUnitsList );	// gets the units status


	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,CM_UNIT_LOADED_IND,
			(BYTE*)(&rStruct),sizeof(CM_UNIT_LOADED_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::CmConfigCompleteInd() const
{
    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::CmConfigCompleteInd - SEND to MPL-API.");


    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

    FillMplProtocol( pMplProtocol,CM_MEDIA_CONFIGURATION_COMPLETED_IND);

    SendToCmForMplApi(*pMplProtocol);
    POBJDELETE(pMplProtocol);
}

void CGideonSimBarakLogical::CmDnatConfigCompleteInd() const
{
    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::CmDnatConfigCompleteInd - SEND to MPL-API.");


    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

    FillMplProtocol( pMplProtocol,CM_DNS_CONFIG_IND);

    SendToCmForMplApi(*pMplProtocol);
    POBJDELETE(pMplProtocol);
}

void CGideonSimBarakLogical::CmCSIntConfigCompleteInd() const
{
    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::CmCSIntConfigCompleteInd - SEND to MPL-API.");


   /* CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

    FillMplProtocol( pMplProtocol,CM_CS_INTERNAL_IP_CONFIG_IND);

    SendToCmForMplApi(*pMplProtocol);
    POBJDELETE(pMplProtocol);*/


	// send CM_MEDIA_IP_CONFIG_IND
    MEDIA_IP_PARAMS_S  rStruct;
	memset(&rStruct,0,sizeof(MEDIA_IP_PARAMS_S));


	rStruct.serviceId = 111;



	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,CM_CS_INTERNAL_IP_CONFIG_IND,
			(BYTE*)(&rStruct),sizeof(MEDIA_IP_PARAMS_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

void CGideonSimBarakLogical::CmCSExtConfigCompleteInd() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::CmCSExtConfigCompleteInd - SEND to MPL-API.");

	MEDIA_IP_PARAMS_S  rStruct;
	memset(&rStruct,0,sizeof(MEDIA_IP_PARAMS_S));


	rStruct.serviceId = 111;




	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,CM_CS_EXTERNAL_IP_CONFIG_IND,
			(BYTE*)(&rStruct),sizeof(MEDIA_IP_PARAMS_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);

}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::CmMediaIpConfigInd()
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::CmMediaIpConfigInd - SEND to MPL-API.");

	// send CM_MEDIA_IP_CONFIG_IND
	MEDIA_IP_CONFIG_S  rStruct;
	memset(&rStruct,0,sizeof(MEDIA_IP_CONFIG_S));

	rStruct.status = STATUS_OK;
	rStruct.serviceId = 111;
	rStruct.pqNumber  = 1;
    rStruct.iPv4.isDHCPv4InUse = false;
    rStruct.iPv4.iPv4Address = SystemIpStringToDWORD("4.3.2.1");
    rStruct.ipType = (APIU32)eIpType_Both;

    char p[15];
    memset(p,0,15);
    strcpy(p, "2001::1/64");

	memcpy(rStruct.iPv6[0].iPv6Address, p, 15);

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,CM_MEDIA_IP_CONFIG_IND,
			(BYTE*)(&rStruct),sizeof(MEDIA_IP_CONFIG_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);

//	m_state = CONNECT;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::ForwardAudioMsg2Endpoints (
				const DWORD opcode, const CAudioParty* party,
				BYTE* pData, const DWORD nDataLen ) const
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::ForwardAudioMsg2Endpoints - SEND TO E.P.");

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


/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::ForwardMuxBndReq2Endpoints(CMplMcmsProtocol& rMplProt) const
{
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::ForwardMuxBndReq2Endpoints - SEND TO E.P.");

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
//		<< (DWORD)1; // net_connection_id

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
void CGideonSimBarakLogical::ForwardMsg2MediaMngr (CMplMcmsProtocol& rMplProt ) const
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    {
    	PTRACE(eLevelInfoNormal,"CGideonSimMfaLogical::ForwardMsg2MediaMngr - ERROR: tried to send message to MediaMngr process - system is not CG!!");
        return;
    }

	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::ForwardMsg2MediaMngr - SEND TO MediaMngr.");

	CSegment pParamSeg;
	rMplProt.Serialize(pParamSeg);

	::SendMessageToMediaMngrApp( pParamSeg );
}


/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::SendActiveSpeakerInd(const CAudioParty& party) const
{
	TRACEINTO	<< " CGideonSimBarakLogical::SendActiveSpeakerInd - Change to: BoardId="
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

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::SendAudioSpeakerInd(const CAudioParty& party) const
{
	TRACEINTO	<< " CGideonSimBarakLogical::SendAudioSpeakerInd - Change to: BoardId="
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


/////////////////////////////////////////////////////////////////////////////
int CGideonSimBarakLogical::IsConfPlayMessage( const CMplMcmsProtocol& rMpl )
{
	if (0xFFFFFFFF == rMpl.getPortDescriptionHeaderParty_id())
		return 1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//CDR_MCCF:
void CGideonSimBarakLogical::FillRtpStatisticInfoStruct(TCmPartyInfoStatisticsInd* pSt) const
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
/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::FillRtpMonitoringStruct(TCmPartyMonitoringInd* pSt) const
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
//	TRtpVideoChannelMonitoring * pVidChannelMonitoring = (TRtpVideoChannelMonitoring *)(&pChnlMonitoring[i]);
//	pVidChannelMonitoring->unStreamVideoSync = 1;
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

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnTimerSoftwareUpgrade(CSegment* pMsg)
{

	//DWORD barakBurnRate=::GetBurnRate(eMediaCardVersionBurnRate);
	CGideonSimLogicalModule::OnTimerSoftwareUpgrade(pMsg);


}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnTimerIpmcSoftwareUpgrade(CSegment* pMsg)
{

	//DWORD barakBurnRate=::GetBurnRate(eMediaCardIpmcBurnRate);
	CGideonSimLogicalModule::OnTimerIpmcSoftwareUpgrade(pMsg);


}


///////////////////////////////////////////////////////////////////////////////
//void CGideonSimBarakLogical::OnTimerIpmcSoftwareUpgrade(CSegment* pMsg)
//{
//
//	if (m_ipmc_software_upgrade_done < 100)
//	  {
//	    m_ipmc_software_upgrade_done++;
//
//	    UPGRADE_PROGRESS_IND_S rStruct;
//	    rStruct.progress_precents =  m_ipmc_software_upgrade_done++;
//	    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
//	    FillMplProtocol( pMplProtocol,CM_UPGRADE_IPMC_PROGRESS_IND,
//			(BYTE*)(&rStruct),sizeof(UPGRADE_PROGRESS_IND_S));
//	    SendToCmForMplApi(*pMplProtocol);
//	    POBJDELETE(pMplProtocol);
//	  }
//
//
//
//	if (m_ipmc_software_upgrade_done < 100)
//	{
//	  StartTimer(SOFTWARE_IPMC_UPGRADE_TIMER,150);
//	}
//	else
//	  {
//	    //	     CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
//	    //	     pMplProtocol->AddCommonHeader(CM_UPGRADE_IPMC_ALMOST_COMPLETE_IND,
//	    //				       MPL_PROTOCOL_VERSION_NUM,
//	    //				       0,(BYTE)eMpl,(BYTE)eMcms);
//
//	    //	     pMplProtocol->AddPhysicalHeader(0,m_wBoardId,m_wSubBoardId);
//
//
//	    //	     SendToCmForMplApi(*pMplProtocol);
//	    //	     POBJDELETE(pMplProtocol);
//
//	     m_ipmc_software_upgrade_done = -1;
//	  }
//
//}



/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnTimerIsdnCfg(CSegment* pMsg) //olga
{
	CBehaviourIsdn* behaviourIsdn = ::GetGideonSystemCfg()->GetBehaviourIsdn();
	if( behaviourIsdn ) {

	    const EP_SGN_S* pSignalStr = behaviourIsdn->GetCurrSignal();
		DBGPASSERT (!pSignalStr);

		if (pSignalStr)
		    TRACESTR(eLevelInfoNormal) << "CGideonSimBarakLogical::OnTimerIsdnCfg : signal to be sent = " << pSignalStr->opcode;

		CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
		pMplProtocol->DeSerialize(*pMsg);

		if (pSignalStr &&
		   (( END_INIT_COMM    == pSignalStr->opcode) ||
		   ( REMOTE_BAS_CAPS  == pSignalStr->opcode && ::GetGideonSystemCfg()->GetIsdnCapsFlag() ) ||
		   ( REMOTE_XMIT_MODE == pSignalStr->opcode && ::GetGideonSystemCfg()->GetIsdnXmitModeFlag() ) ||
		   ( REMOTE_CI        == pSignalStr->opcode && ::GetGideonSystemCfg()->GetIsdnH230Flag() ))) {

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
/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnTimerPacketLossTout(CSegment* pMsg)
{
	if (CProcessBase::GetProcess()->GetProcessStatus()==eProcessTearDown)
		return;
	PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnTimerPacketLossTout");
	
	// colinzuo: what's the purpose of the following code? Why fixed connection id 9 is used?
	// comment out as it breaks SingleStreamTest
	
	/*
	TCmRtcpMsg packetLossStruct;
	(packetLossStruct.tCmRtcpMsgInfo).uMediaType = kIpVideoChnlType;
//	packetLossStruct.media_direction = cmCapTransmit;
	(packetLossStruct.tCmRtcpMsgInfo).uMsgType = RTCP_INTRA_RTV;//RTCP_PLI;

	CMplMcmsProtocol oMplProtocol;
	oMplProtocol.AddPortDescriptionHeader( 1, 0xffffffff, 9);
	FillMplProtocol( &oMplProtocol, IP_CM_RTCP_MSG_IND,(BYTE*)(&packetLossStruct),sizeof(TCmRtcpMsg));
	SendToCmForMplApi(oMplProtocol);*/
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnPcmMenuTimer(CSegment* pParam)
{
	TRACEINTO	<< " CGideonSimBarakLogical::OnPcmMenuTimer";
	DWORD index = 0;
	*pParam >> (DWORD&)index;
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pParam);



	DWORD connectionId = pMplProtocol->getPortDescriptionHeaderConnection_id();
	DWORD partyId = pMplProtocol->getPortDescriptionHeaderParty_id();
	DWORD confId = pMplProtocol->getPortDescriptionHeaderConf_id();

	CLargeString cstr;
	//cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"pop_menu_status\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<STATUS value=\"0\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
	//cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"set_drop_term\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<KICKALL value=\"false\" />\n\t\t\t<TERM_NAME_COUNT value=\"3\" />\n\t\t\t<TERM_NAME_1 value=\"111\" />\n\t\t\t<TERM_NAME_2 value=\"222\" />\n\t\t\t<TERM_NAME_3 value=\"333\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
	//cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"set_cp_layout\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<VALUE value=\"601\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
	//cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"set_invite_term\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<SELECT_LIST_IS_CHECKED value=\"false\" />\n\t\t\t<TERM_NAME_COUNT value=\"1\" />\n\t\t\t<CALL_TYPE_1 value=\"0\" />\n\t\t\t<TERM_NAME_1 value=\"1.2.3.4\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
	if (index == 1)
		cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"local_addr_book\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<COUNT value=\"5000\" />\n\t\t\t<MATCH_STR value=\"m\" />\n\t\t\t<MATCH_STR_END value=\"z\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
		//cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"set_display_setting\">\n\t\t\t<PANE_INDEX value=\"0\" />\n\t\t\t<PANE_TYPE value=\"2\" />\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<VALUE value=\"601\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
		//cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"set_cp_layout\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<VALUE value=\"601\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
		//cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"local_addr_book\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<COUNT value=\"5000\" />\n\t\t\t<MATCH_STR value=\"v\" />\n\t\t\t<MATCH_STR_END value=\"z\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
	else if (index == 2)
		cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"set_cp_layout\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<VALUE value=\"402\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
		//cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"fecc_control\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<PANE_INDEX value=\"0\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
	else if (index == 3)
		//cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"set_cp_layout\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<VALUE value=\"0\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
		cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"set_focus\">\n\t\t\t<SOURCE_GUID value=\"guid\" />\n\t\t\t<TARGET_GUID value=\"guid\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<FOCUS_POS value=\"" << index << "\" />\n\t\t</COMMAND>\n\t</TVUI_API>";
	else
		cstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\t<TVUI_API version=\"1.0.0.0\">\n\t\t<COMMAND id=\"set_invite_term\">\n\t\t\t<CALL_TYPE_1 value=\"0\" />\n\t\t\t<SELECT_LIST_IS_CHECKED value=\"false\" />\n\t\t\t<SOURCE_GUID value=\"\" />\n\t\t\t<TARGET_GUID value=\"\" />\n\t\t\t<TERM_ID value=\"0\" />\n\t\t\t<TERM_NAME_1 value=\"172.22.184.94\" />\n\t\t\t<TERM_NAME_COUNT value=\"1\" />\n\t\t</COMMAND>\n\t</TVUI_API>";

	const char* msgStr = cstr.GetString();
	CSegment* paramSeg = new CSegment;
	DWORD msgLen = strlen(msgStr) + 1;
	paramSeg->Put((BYTE*)msgStr,msgLen);

	FillMplProtocol(pMplProtocol, PCM_COMMAND, (BYTE*)(paramSeg->GetPtr()), paramSeg->GetWrtOffset());
	pMplProtocol->AddPortDescriptionHeader(partyId, confId, connectionId);

	SendToCmForMplApi(*pMplProtocol);

	if(index < 4)
	{
		index++;
		CSegment* timerSeg = new CSegment;
		*timerSeg << (DWORD)index;
		pMplProtocol->Serialize(*timerSeg);

		StartTimer(PCM_MENU_TIMER,1*SECOND,timerSeg);
	}


	POBJDELETE(paramSeg);
}

void CGideonSimBarakLogical::SendBfcpMessageInd(CMplMcmsProtocol& rMplProt)
{
    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::SendBfcpMessageInd");

    if(m_pTranslator != NULL)
    {
        APIU32  status          = STATUS_OK;
        APIS32  opCode          = 0;
        const mcReqBfcpMessage *pMsg = (const mcReqBfcpMessage *)rMplProt.getpData();
        BFCPFloorInfoT  recBFCPmsg;

        memset(&recBFCPmsg, 0, sizeof(BFCPFloorInfoT));

        if (pMsg->length != 0)
        {
            //DECODE the received BFCP message
            if (statusOK != DecodeBFCPMsg (m_pTranslator, (UInt8*)pMsg->buffer, pMsg->length, &recBFCPmsg))
            {
                //Error decoding!
                PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::SendBfcpMessageInd decode returned error status");
                return;
            }

            if(recBFCPmsg.transactionType == kBFCPHelloAck)
            {
                ParticipantReceivedAck(rMplProt.getPortDescriptionHeaderParty_id(), rMplProt.getPortDescriptionHeaderConf_id());
            }
            else if(recBFCPmsg.transactionType == kBFCPFloorStatus)
            {
                //updates values in the bfcp translator
                BFCPSetConferenceID (&(m_pTranslator->BFCPObj), recBFCPmsg.conferenceID);
                BFCPSetUserID (&(m_pTranslator->BFCPObj), recBFCPmsg.userID);
                UInt16 currentTransId = m_pTranslator->BFCPObj.floorInfo.transactionID;
                UInt16 receivedTransactionId = recBFCPmsg.transactionID;
                m_pTranslator->BFCPObj.floorInfo.transactionID = MAX(receivedTransactionId,currentTransId);

                BFCPFloorInfoT  resBFCPmsg;

                memset(&resBFCPmsg, 0, sizeof(BFCPFloorInfoT));

                resBFCPmsg.transactionType = kBFCPFloorStatusAck;
                resBFCPmsg.floorID = recBFCPmsg.floorID;
                resBFCPmsg.conferenceID = recBFCPmsg.conferenceID;
                resBFCPmsg.userID = recBFCPmsg.userID;
                resBFCPmsg.transactionID = recBFCPmsg.transactionID;

                //ENCODES the response
                UInt32  pMsgLen;
                UInt8   pOutBinaryBFCPMsg[256];
                char *pRemoteProduct = NULL; //ADD TEST FOR TANDBERG

                memset(pOutBinaryBFCPMsg,0,256);

                EncodeBFCPMsg (m_pTranslator, &resBFCPmsg, &pMsgLen, pOutBinaryBFCPMsg, pRemoteProduct, m_bfcpTransportType, FALSE);

                size_t size = sizeof(mcIndBfcpMessage) + pMsgLen;

                mcIndBfcpMessage *pIndBFCPMsg = (mcIndBfcpMessage*)new BYTE[size];

                pIndBFCPMsg->status = STATUS_OK;
                pIndBFCPMsg->length = pMsgLen;

                memcpy(&pIndBFCPMsg->buffer[0],  pOutBinaryBFCPMsg, pMsgLen);

                CMplMcmsProtocol oMplProtocol;
                oMplProtocol.AddPortDescriptionHeader(rMplProt.getPortDescriptionHeaderParty_id(), rMplProt.getPortDescriptionHeaderConf_id(), rMplProt.getPortDescriptionHeaderConnection_id());
                FillMplProtocol( &oMplProtocol, IP_CM_BFCP_MESSAGE_IND,(BYTE*)(pIndBFCPMsg),size);
                SendToCmForMplApi(oMplProtocol);

                InsertParticipantForHello(rMplProt.getPortDescriptionHeaderParty_id(), rMplProt.getPortDescriptionHeaderConf_id(), rMplProt.getPortDescriptionHeaderConnection_id(),
                                            resBFCPmsg.floorID, resBFCPmsg.conferenceID, resBFCPmsg.userID, (UInt32)resBFCPmsg.priority);

                PDELETEA(pIndBFCPMsg);
            }
        }

        if(!IsValidTimer(BFCP_HELLO_TIMER))
            StartTimer(BFCP_HELLO_TIMER, BFCP_SEND_HELLO_DELAY_TIME);
        if(!IsValidTimer(BFCP_HELLO_ACK_TIMER))
            StartTimer(BFCP_HELLO_ACK_TIMER,BFCP_RECEIVE_HELLO_ACK_TIMEOUT);
    }

}

///////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnBfcpSendHelloMsgAnycase()
{
    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnBfcpSendHelloMsgAnycase");

    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

    map<PartyConfKey,PartyConfValue>::iterator iter = m_waitForHelloMap.begin();

    while(iter != m_waitForHelloMap.end())
    {
        PartyConfKey key = (*iter).first;
        PartyConfValue value = (*iter).second;
        DWORD  partyId = key.first;
        DWORD  confId = key.second;
        DWORD  connectionId = 0;
        UInt32 floorId;
        UInt32 conferenceId;
        UInt16 userId;
        UInt32 priority;
        CSegment* pMsg = value.second;

        if(value.first < SystemGetTickCount())
        {
            *pMsg >> (DWORD&)connectionId;
            *pMsg >> (UInt32&)floorId;
            *pMsg >> (UInt32&)conferenceId;
            *pMsg >> (UInt16&)userId;
            *pMsg >> (UInt32&)priority;

            pMplProtocol->AddPortDescriptionHeader(partyId, confId, connectionId);

            char *pRemoteProduct = NULL;
            UInt32  pMsgLen;
            UInt8   pOutBinaryBFCPMsg[256];
            BFCPFloorInfoT BFCPmsg;

            memset(pOutBinaryBFCPMsg,0,256);
            memset(&BFCPmsg,0,sizeof(BFCPFloorInfoT));

            BFCPFloorInfoT *pBfcpFloorInfo  = NULL;

            pBfcpFloorInfo = &m_pTranslator->BFCPObj.floorInfo;
            pBfcpFloorInfo->transactionID++;

            if (pBfcpFloorInfo->transactionID == 0xFFFF)
                pBfcpFloorInfo->transactionID = 1;

            BFCPmsg.transactionType = kBFCPHello;
            BFCPmsg.floorID         = floorId;
            BFCPmsg.conferenceID    = conferenceId;
            BFCPmsg.userID          = userId;
            BFCPmsg.transactionID   = pBfcpFloorInfo->transactionID;
            BFCPmsg.floorRequestID  = 1;
            BFCPmsg.priority        = (eBFCPPriority)priority;

            EncodeBFCPMsg (m_pTranslator, &BFCPmsg, &pMsgLen, pOutBinaryBFCPMsg, pRemoteProduct, m_bfcpTransportType, FALSE);

            size_t size = sizeof(mcIndBfcpMessage) + pMsgLen;

            mcIndBfcpMessage *pIndBFCPMsg = (mcIndBfcpMessage*)new BYTE[size];

            pIndBFCPMsg->status = STATUS_OK;
            pIndBFCPMsg->length = pMsgLen;

            memcpy(&pIndBFCPMsg->buffer[0],  pOutBinaryBFCPMsg, pMsgLen);

            FillMplProtocol( pMplProtocol,IP_CM_BFCP_MESSAGE_IND,(BYTE*)(pIndBFCPMsg),size);

            SendToCmForMplApi(*pMplProtocol);

            InsertParticipantForHelloAck(partyId, confId);

            PTRACE2INT(eLevelInfoNormal,"CGideonSimBarakLogical::OnBfcpSendHelloMsgAnycase insert to wait for ack partyId=", partyId);

            PDELETEA(pIndBFCPMsg);

            iter++;

            m_waitForHelloMap.erase(key);

            TRACEINTO<<"CGideonSimBarakLogical::OnBfcpSendHelloMsgAnycase deleted partyId="<<partyId<< " remained" << m_waitForHelloMap.count(make_pair(partyId,confId));
            POBJDELETE(pMsg);

            break;
        }
        else
        {
            ++iter;
            PTRACE2INT(eLevelInfoNormal,"CGideonSimBarakLogical::OnBfcpSendHelloMsgAnycase maybe another time partyId=",partyId);
        }

    }
    StartTimer(BFCP_HELLO_TIMER, BFCP_SEND_HELLO_DELAY_TIME);
    POBJDELETE(pMplProtocol);
}
///////////////////////////////////////////////////////////////////////////////
void CGideonSimBarakLogical::OnBfcpCheckReceivedHelloAckTout()
{
    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnBfcpCheckReceivedHelloAckTout");

    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

    map<PartyConfKey,TICKS>::iterator iter = m_waitForHelloAckMap.begin();

    while(iter != m_waitForHelloAckMap.end())
    {
        PartyConfKey key = (*iter).first;
        TICKS time = (*iter).second;

        pMplProtocol->AddPortDescriptionHeader(key.first, key.second, 0);

        iter++;

        if(time < SystemGetTickCount())
        {
             DBGPASSERT(1);
             m_waitForHelloAckMap.erase(key);
             TRACEINTO<<"CGideonSimBarakLogical::OnBfcpCheckReceivedHelloAckTout timeout for partyId="<< key.first;
        }
    }

     StartTimer(BFCP_HELLO_ACK_TIMER,BFCP_RECEIVE_HELLO_ACK_TIMEOUT);

     POBJDELETE(pMplProtocol);
}

void CGideonSimBarakLogical::InsertParticipantForHello(DWORD partyId, DWORD confId, DWORD  connectionId, UInt32 floorId, UInt32 conferenceId,UInt16 userId,UInt32 priority)
{
    PartyConfKey key = make_pair(partyId, confId);

    if(!m_waitForHelloAckMap.count(key) && !m_waitForHelloMap.count(key))
    {
        CSegment *pClientInfo = new CSegment;

        *pClientInfo << connectionId
                     << floorId
                     << conferenceId
                     << userId
                     << priority;

        m_waitForHelloMap[key] = make_pair(SystemGetTickCount() + BFCP_SEND_HELLO_DELAY_TIME, pClientInfo);
    }
}

void CGideonSimBarakLogical::ParticipantReceivedAck(DWORD partyId, DWORD confId)
{
    PartyConfKey key = make_pair(partyId, confId);
    if(m_waitForHelloAckMap.count(key))
    {
        m_waitForHelloAckMap.erase(key);
    }
}

void CGideonSimBarakLogical::InsertParticipantForHelloAck(DWORD partyId, DWORD confId)
{
    PartyConfKey key = make_pair(partyId, confId);
    if(!m_waitForHelloAckMap.count(key))
    {
        m_waitForHelloAckMap[key] = SystemGetTickCount() + BFCP_RECEIVE_HELLO_ACK_TIMEOUT;
    }
}

void CGideonSimBarakLogical::RemoveParticipantFromMaps(DWORD partyId, DWORD confId)
{
    PartyConfKey key = make_pair(partyId, confId);
    if(m_waitForHelloAckMap.count(key))
    {
        m_waitForHelloAckMap.erase(key);
    }
    if(m_waitForHelloMap.count(key))
    {
        PartyConfValue value = m_waitForHelloMap[key];
        POBJDELETE(value.second);
        m_waitForHelloMap.erase(key);
    }
}

void CGideonSimBarakLogical::RemoveBfcpWaiting(CMplMcmsProtocol& rMplProt)
{
    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::RemoveBfcpWaiting");

    const TCloseUdpPortMessageStruct *pTbCloseStruct = (const TCloseUdpPortMessageStruct *)rMplProt.getpData();
    const mcReqCmCloseUdpPort& pUdpSt  = pTbCloseStruct->tCmCloseUdpPort;

    if(pUdpSt.channelType == kBfcpChnlType)
    {
        RemoveParticipantFromMaps(rMplProt.getPortDescriptionHeaderParty_id(), rMplProt.getPortDescriptionHeaderConf_id());
    }
}

void CGideonSimBarakLogical::OnWebRtcConnectReq(CMplMcmsProtocol &rMplProt)
{
    PTRACE(eLevelInfoNormal,"CGideonSimBarakLogical::OnWebRtcConnectReq");

    //Send Ack
	ACK_IND_S  rAckStruct;
	memset(&rAckStruct,0,sizeof(ACK_IND_S));
	rAckStruct.ack_base.ack_opcode = rMplProt.getCommonHeaderOpcode();;
	rAckStruct.ack_base.status = STATUS_OK;
	rAckStruct.ack_base.ack_seq_num = rMplProt.getMsgDescriptionHeaderRequest_id();
    CMplMcmsProtocol *pMplProt1 = new CMplMcmsProtocol(rMplProt);
    pMplProt1->AddCommonHeader(ACK_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
    pMplProt1->AddData(sizeof(ACK_IND_S),(char*)(&rAckStruct));
	SendToCmForMplApi(*pMplProt1);
	POBJDELETE(pMplProt1);

	//Send Connect Ind
	const mcReqCmWebRtcConnect *pWebRtcConnectReq = (const mcReqCmWebRtcConnect *)rMplProt.getpData();
	char buf[sizeof(mcIndCmWebRtcConnectInd) + WEBRTC_SDP_MAX_SIZE];
	mcIndCmWebRtcConnectInd *pWebRtcConnectInd = (mcIndCmWebRtcConnectInd *)buf;
	memset(buf, 0, sizeof(buf));
	pWebRtcConnectInd->status = STATUS_OK;
	pWebRtcConnectInd->sdpSize = pWebRtcConnectReq->sdpSize;
	memcpy(pWebRtcConnectInd->sdp, pWebRtcConnectReq->sdp, pWebRtcConnectReq->sdpSize);
	int size = sizeof(mcIndCmWebRtcConnectInd) + pWebRtcConnectInd->sdpSize - 1;
	CMplMcmsProtocol *pMplProt2 = new CMplMcmsProtocol(rMplProt);
	pMplProt2->AddCommonHeader(IP_CM_WEBRTC_CONNECT_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms);
	pMplProt2->AddData(size, (char*)(pWebRtcConnectInd));
	SendToCmForMplApi(*pMplProt2);
	POBJDELETE(pMplProt2);

}
