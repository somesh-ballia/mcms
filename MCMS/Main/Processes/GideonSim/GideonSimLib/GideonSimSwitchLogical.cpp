// GideonSimLogicalModule.cpp

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
#include "GideonSimSwitchLogical.h"
#include "AuthenticationStructs.h"
#include "IpService.h"

extern DWORD GetBurnRate(eBurnTypes burnRateTypes);

/////////////////////////////////////////////////////////////////////////////
//
//   GideonSimSwitchLogical - logical module of SWITCH
//
/////////////////////////////////////////////////////////////////////////////
//
//	STATES:
//		IDLE      - default state immediately after creation
//		SETUP    (?)    - from Connect command until socket connection will be established
//		STARTUP   - socket connected, start configure switch card
//		CONNECT   - switch card started up
//		RECOVERY (?)
//		RESET    (?)
//

PBEGIN_MESSAGE_MAP(CGideonSimSwitchLogical)
	ONEVENT( CM_STARTUP,   IDLE,     CGideonSimSwitchLogical::OnCmStartupIdle)
	ONEVENT( CM_MCMS_MSG,  STARTUP,  CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup)
	ONEVENT( CM_MCMS_MSG,  CONNECT,  CGideonSimSwitchLogical::OnCmProcessMcmsReqConnect)
	ONEVENT( CARD_NOT_READY_DELAY_TIMER,   ANYCASE,  CGideonSimSwitchLogical::OnTimerCardsDelayTout )
	ONEVENT( SWITCH_CONFIG_TIMER,          STARTUP,  CGideonSimSwitchLogical::OnTimerSwitchConfigTout )
	ONEVENT( SET_UNIT_STATUS_FOR_KEEP_ALIVE_NEW,  ANYCASE,  CGideonSimSwitchLogical::SetUnitStatustSwitchForKeepAliveInd)
	ONEVENT( SOFTWARE_UPGRADE_TIMER,  ANYCASE,  CGideonSimSwitchLogical::OnTimerSoftwareUpgrade)
	ONEVENT( SOFTWARE_IPMC_UPGRADE_TIMER,  ANYCASE,  CGideonSimSwitchLogical::OnTimerIpmcSoftwareUpgrade)
PEND_MESSAGE_MAP(CGideonSimSwitchLogical, CGideonSimLogicalModule);

CGideonSimSwitchLogical::CGideonSimSwitchLogical(CTaskApp* pTask,WORD boardId,WORD subBoardId)
			: CGideonSimLogicalModule(pTask,boardId,subBoardId)
{
	 m_units = new CSimSwitchUnitsList(0);
	 m_software_upgrade_done = -1;

	m_bCMLoadedInd = FALSE;
	m_bSlotNumberingConversionInd = FALSE;

}

CGideonSimSwitchLogical::~CGideonSimSwitchLogical()
{
  POBJDELETE(m_units);
}

void CGideonSimSwitchLogical::HandleEvent(CSegment* pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}

void CGideonSimSwitchLogical::OnCmStartupIdle(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmStartupIdle - START SWITCH STARTUP.");

	m_state = STARTUP;

	SendStartupInds();
}

void CGideonSimSwitchLogical::SendStartupInds()
{
	AuthentificationInd();
	CntlIpConfigInd();
	MngmntIpConfigInd();
	SwLocationInd();

	StartTimer(SWITCH_CONFIG_TIMER,::GetGideonSystemCfg()->GetSwitchConfigTime()*SECOND);
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::SendStartupInds - START SWITCH TIMER.");
}

void CGideonSimSwitchLogical::OnTimerCardsDelayTout(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnTimerCardsDelayTout - send CardManagerLoadedInd again.");

	CardManagerLoadedInd();
}

void CGideonSimSwitchLogical::SendUserLdapLogin(const string& userName, const string& userPassword)
{

	PTRACE(eLevelInfoNormal,"GideonSimSwitchLogical::SendUserLdapLogin.");

    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

    AD_USER_IND_S userLoginLdap;
    memset( &userLoginLdap, 0, sizeof(AD_USER_IND_S) );

    strncpy((char *) userLoginLdap.login,  (const char *)userName.c_str(), OPERATOR_NAME_LEN-1);
    strncpy((char *) userLoginLdap.password,   (const char *)userPassword.c_str(), OPERATOR_NAME_LEN-1);

    userLoginLdap.MsgId = 1;

    FillMplProtocol( pMplProtocol, IS_USER_EXIST_ON_ACTIVE_DIRECTORY_IND, (BYTE*)(&userLoginLdap), sizeof(AD_USER_IND_S));
	SendToCmForMplApi(*pMplProtocol);
	PTRACE(eLevelInfoNormal,"After GideonSimSwitchLogical::SendUserLdapLogin.");
	POBJDELETE(pMplProtocol);

}

void CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup(CSegment* pMsg)
{
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pMsg);

	DWORD  opcode = pMplProtocol->getOpcode();
	switch( opcode )
	{
		case CARDS_NOT_READY_REQ:
		{
			// delete startup timer
			DeleteTimer(SWITCH_CONFIG_TIMER);

			// delay connection
 			StartTimer(CARD_NOT_READY_DELAY_TIMER,SWITCH_CARDS_DELAY_TIME);

			PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup - CARDS_NOT_READY_REQ received, start timer.");
			break;
		}
		case SM_KEEP_ALIVE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup - SM_KEEP_ALIVE_REQ received");
			Ack_keep_alive_Switch(*pMplProtocol,STATUS_OK);
			break;
		}
		case MCUMNGR_UPDATE_CFS_KEYCODE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup - MCUMNGR_UPDATE_CFS_KEYCODE_REQ received");
			UpdateLicenseFile(*pMplProtocol);
			break;
		}
		case ETHERNET_SETTINGS_CONFIG_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup - ETHERNET_SETTINGS_CONFIG_REQ received");
			break;
		}
		case SLOTS_NUMBERING_CONVERSION_REQ:
		{
			if(m_bCMLoadedInd)
			{
				PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup - SLOTS_NUMBERING_CONVERSION_REQ received");
				SlotsNumberingConversion();
			}else
			{
				m_bSlotNumberingConversionInd = true;
			}
		}

	        case DEBUG_MODE_NO_REQ:
		  {
		    PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup - DEBUG_MODE_NO_REQ");
		    break;
		  }


		default:
		{
			TRACESTR(eLevelError) << " CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup - "
				<< "UNEXPECTED MESSAGE IN (STARTUP) STATE, opcode - "
				<< "<" << (int)opcode << ">";
		}
	}
	POBJDELETE(pMplProtocol);
}

void CGideonSimSwitchLogical::OnCmProcessMcmsReqConnect(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqConnect");

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pMsg);

	DWORD opcode = pMplProtocol->getOpcode();
	switch (opcode)
	{
		case CARDS_NOT_READY_REQ:
		{
			// delete startup timer
			DeleteTimer(SWITCH_CONFIG_TIMER);

			// delay connection
 			StartTimer(CARD_NOT_READY_DELAY_TIMER,SWITCH_CARDS_DELAY_TIME);

			PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqConnect - CARDS_NOT_READY_REQ received, start timer.");
			break;
		}

		case SM_KEEP_ALIVE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqConnect - SM_KEEP_ALIVE_REQ received");
			Ack_keep_alive_Switch(*pMplProtocol,STATUS_OK);
			break;
		}

		case MCUMNGR_UPDATE_CFS_KEYCODE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqConnect - MCUMNGR_UPDATE_CFS_KEYCODE_REQ received");
			UpdateLicenseFile(*pMplProtocol);
			break;
		}

		case ETHERNET_SETTINGS_CONFIG_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqConnect - ETHERNET_SETTINGS_CONFIG_REQ received");
			break;
		}

		case SLOTS_NUMBERING_CONVERSION_REQ:
		{
			PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnCmProcessMcmsReqConnect - SLOTS_NUMBERING_CONVERSION_REQ received");
			SlotsNumberingConversion();
			break;
		}

	    case CM_UPGRADE_NEW_VERSION_READY_REQ:
		{
		    StartVersionUpgrade();
		    // sagi add send ack for this request
		    //		        m_software_upgrade_done = -1;
		    //		        PASSERT(CM_UPGRADE_NEW_VERSION_READY_REQ);
		    break;
		}

	    case CM_UPGRADE_START_WRITE_IPMC_REQ:
	    {
	        m_ipmc_software_upgrade_done = 0;
	        break;
	    }

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
		case IS_USER_EXIST_ON_ACTIVE_DIRECTORY_REQ:
		{
		    TRACESTRFUNC(eLevelInfoNormal) << "CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup - IS_USER_EXIST_ON_ACTIVE_DIRECTORY_REQ " ;
			break;
		}

		case ACTIVE_DIRECTORY_STATUS_REQ:
		{
		    TRACESTRFUNC(eLevelInfoNormal) << "CGideonSimSwitchLogical::OnCmProcessMcmsReqStartup - ACTIVE_DIRECTORY_STATUS_REQ";
		    
			break;
		}


		default:
		{
		    TRACESTRFUNC(eLevelError) << "UNEXPECTED MESSAGE IN (CONNECT) STATE, opcode "
                                         << " (" << opcode << ")";
			break;
		}
	}

	POBJDELETE(pMplProtocol);
}

char * CGideonSimSwitchLogical::ConvertCardTypeToShelfMngrString(DWORD cardType) const
{
	switch (cardType)
	{
	  case eCardSwitch:
		  return "SWITCH";

	  case  eCardBreeze :
		  return "MPMX";

	  case	  eCardMpmRx:
	      return "MPMRX-F";

	  case  eCardBarak :
		  return "MPM_PLUS";

	  case  eCardMfa :
		  return "MFA";

	  default:
		 return NULL;
	}
	return NULL;
}

char * CGideonSimSwitchLogical::ConvertRtmCardTypeToShelfMngrString(DWORD rtmCardType) const
{
	switch (rtmCardType)
	{
	  case eRtmIsdn:
		  return "RTM_ISDN";

	  case  eRtmIsdn_9PRI :
		  return "RTM_ISDN9";

	  case	  eRtmIsdn_9PRI_10G:
	      return "RTM_ISDN9_10G";

	  default:
		 return NULL;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
//
void CGideonSimSwitchLogical::Ack_keep_alive_Switch( CMplMcmsProtocol& rMplProt, STATUS status ) const
{

	SM_COMPONENT_STATUS_S *keepAliveStructPtr = (SM_COMPONENT_STATUS_S *)&m_units->m_switchSM_units;


	keepAliveStructPtr->unSlotId		= 9;
	keepAliveStructPtr->unSubBoardId	= 1;
	keepAliveStructPtr->unStatus = 0;
	keepAliveStructPtr->unStatusDescriptionBitmask = 0;
	strncpy( (char*)(keepAliveStructPtr->sSmCompName), "CNTL", strlen("CNTL") );

	keepAliveStructPtr++;

	/********************************************************************************************/
	/* 2.7.10  added by rachel Cohen - read configuration from xml and send it to cards process.*/
	/********************************************************************************************/

	DWORD cardType;
	for (int i=0;i<MAX_NUM_OF_BOARDS ;i++)
	{
//		TRACEINTO << "\nCGideonSimSwitchLogical::Ack_keep_alive_Switch  cards CFG \n";
	    CCardCfgMfa* pCurrCardCfg = NULL;
		pCurrCardCfg = (CCardCfgMfa*)::GetGideonSystemCfg()->GetCardCfg( i );

		if (CPObject::IsValidPObjectPtr(pCurrCardCfg))
		{
			cardType = pCurrCardCfg->GetCardType();
			char * cardTypeStr = ConvertCardTypeToShelfMngrString(cardType);

			if(cardTypeStr == NULL)
				continue;
			//TRACEINTO << "\nCGideonSimSwitchLogical::cardType  : " << cardType <<ConvertCardTypeToShelfMngrString(cardType)<<"\n";
			keepAliveStructPtr->unSlotId = i;
			keepAliveStructPtr->unSubBoardId = 1;
			keepAliveStructPtr->unStatus = 0;
			keepAliveStructPtr->unStatusDescriptionBitmask = 0;

			strncpy( (char*)(keepAliveStructPtr->sSmCompName),cardTypeStr,strlen(cardTypeStr)+1);

			if (true == pCurrCardCfg->GetRtmAttached())
			{
				DWORD  RtmCardType = pCurrCardCfg->GetRtmCardType();

				  TRACESTRFUNC(eLevelInfoNormal) << "CGideonSimSwitchLogical::Ack_keep_alive_Switch RtmCardType " << RtmCardType;
				char * cardTypeStr = ConvertRtmCardTypeToShelfMngrString(RtmCardType);

				if(cardTypeStr != NULL)
				{
					keepAliveStructPtr++;
					keepAliveStructPtr->unSlotId = i;
					keepAliveStructPtr->unSubBoardId = 2;
					keepAliveStructPtr->unStatus = 0;
					keepAliveStructPtr->unStatusDescriptionBitmask = 0;
					strncpy( (char*)(keepAliveStructPtr->sSmCompName), cardTypeStr, strlen(cardTypeStr) );
				}

			}


		}


		// sagi - temp patch for soft mcu , gideon sim should not running
		if( eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily() )
                {
                        keepAliveStructPtr->unSlotId = 1;
                        keepAliveStructPtr->unSubBoardId = 1;
                        keepAliveStructPtr->unStatus = 1;
                        keepAliveStructPtr->unStatusDescriptionBitmask = 0;
                        strncpy( (char*)(keepAliveStructPtr->sSmCompName), "MPMX", strlen("MPMX") );
                }



		/****************************************************************************************************/
		/* 8.3.11  added by Ofir Nissel - Simulate voltage problem for different Power supplies (PWR1\PWR2)	*/
		/****************************************************************************************************/

/*
		if(i==11)
		{
			//Component 11-slot id: 9, sub board id: 1,status: Major, bitmask: 0x2, component name: PWR1
			TRACEINTO << "\nCGideonSimSwitchLogical::cardType  : " << cardType <<ConvertCardTypeToShelfMngrString(cardType)<<"\n";
			keepAliveStructPtr->unSlotId = 11;
			keepAliveStructPtr->unSubBoardId = 1;
			keepAliveStructPtr->unStatus = 1;
			keepAliveStructPtr->unStatusDescriptionBitmask = 0x2;
			strncpy( (char*)(keepAliveStructPtr->sSmCompName), "PWR1", strlen("PWR1") );
		}
		if(i==12)
		{

			TRACEINTO << "\nCGideonSimSwitchLogical::cardType  : " << cardType <<ConvertCardTypeToShelfMngrString(cardType)<<"\n";
			keepAliveStructPtr->unSlotId = 12;
			keepAliveStructPtr->unSubBoardId = 1;
			keepAliveStructPtr->unStatus = 1;
			keepAliveStructPtr->unStatusDescriptionBitmask = 0x2;
			strncpy( (char*)(keepAliveStructPtr->sSmCompName), "PWR2", strlen("PWR2") );
		}

		*/
		keepAliveStructPtr++;
	}



//	TRACEINTO << "\nCGideonSimSwitchLogical::Ack_keep_alive_Switch\n";
//
//	TRACEINTO << " 1.Name: " <<  m_units->m_switchSM_units.unSmComp1.sSmCompName
//			  << " SlotId: " <<  m_units->m_switchSM_units.unSmComp1.unSlotId
//			  << " Status: " <<  m_units->m_switchSM_units.unSmComp1.unStatus
//			  << " Desc: " <<  m_units->m_switchSM_units.unSmComp1.unStatusDescriptionBitmask;
//
//	TRACEINTO << " 2.Name: " <<  m_units->m_switchSM_units.unSmComp2.sSmCompName
//			 <<  " SlotId: " <<  m_units->m_switchSM_units.unSmComp2.unSlotId
//			 <<  " Status: " <<  m_units->m_switchSM_units.unSmComp2.unStatus
//			 <<  " Desc: " <<  m_units->m_switchSM_units.unSmComp2.unStatusDescriptionBitmask;
//
//	TRACEINTO << " 3.Name: " <<  m_units->m_switchSM_units.unSmComp3.sSmCompName
//			 <<  " SlotId: " <<  m_units->m_switchSM_units.unSmComp3.unSlotId
//			 <<  " Status: " <<  m_units->m_switchSM_units.unSmComp3.unStatus
//			 <<  " Desc: " <<  m_units->m_switchSM_units.unSmComp3.unStatusDescriptionBitmask;


	rMplProt.AddCommonHeader(SM_KEEP_ALIVE_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	//rMplProt.AddData(sizeof(SWITCH_SM_KEEP_ALIVE_S),(char*)(&rswitch_keep_alive_ind_Struct));
	rMplProt.AddData(sizeof(SWITCH_SM_KEEP_ALIVE_S),(char*)(&m_units->m_switchSM_units));
	SendToCmForMplApi(rMplProt);
  //  PrintSmFatalFailureToTrace(m_units->m_switchSM_units);
   //
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::PrintSmFatalFailureToTrace(SWITCH_SM_KEEP_ALIVE_S fatalFailureStruct)
{
	CLargeString dataStr = "\nCGideonSimSwitchLogical::PrintSmFatalFailureToTrace";
	dataStr             << "\n===========-----====-----=====-----===================\n";

	ALLOCBUFFER(specificCompFailureStr, ONE_LINE_BUFFER_LEN);

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp1, specificCompFailureStr );
	dataStr << "Component 1 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp2, specificCompFailureStr );
	dataStr << "Component 2 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp3, specificCompFailureStr );
	dataStr << "Component 3 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp4, specificCompFailureStr );
	dataStr << "Component 4 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp5, specificCompFailureStr );
	dataStr << "Component 5 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp6, specificCompFailureStr );
	dataStr << "Component 6 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp7, specificCompFailureStr );
	dataStr << "Component 7 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp8, specificCompFailureStr );
	dataStr << "Component 8 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp9, specificCompFailureStr );
	dataStr << "Component 9 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp10, specificCompFailureStr );
	dataStr << "Component 10 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp11, specificCompFailureStr );
	dataStr << "Component 11 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp12, specificCompFailureStr );
	dataStr << "Component 12 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp13, specificCompFailureStr );
	dataStr << "Component 13 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp14, specificCompFailureStr );
	dataStr << "Component 14 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp15, specificCompFailureStr );
	dataStr << "Component 15 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp16, specificCompFailureStr );
	dataStr << "Component 16 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp17, specificCompFailureStr );
	dataStr << "Component 17 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp18, specificCompFailureStr );
	dataStr << "Component 18 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp19, specificCompFailureStr );
	dataStr << "Component 19 - " << specificCompFailureStr;

	PrintSmSpecificCompFailure( fatalFailureStruct.unSmComp20, specificCompFailureStr );
	dataStr << "Component 20 - " << specificCompFailureStr;

	TRACESTR(eLevelInfoNormal) << dataStr.GetString();

	DEALLOCBUFFER(specificCompFailureStr);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::PrintSmSpecificCompFailure(SM_COMPONENT_STATUS_S compFailureStruct, char* resStr)
{
	CSmallString statusStr;
	char* tmpStr = NULL;//= "temp";//::ShelfMngrComponentStatusToString(compFailureStruct.unStatus);
	if (tmpStr)
	{
		statusStr << tmpStr;
	}
	else
	{
		statusStr << "(invalid: " << compFailureStruct.unStatus << ")";
	}

	if (eSmComponentNotExist != compFailureStruct.unStatus)
	{
		sprintf(resStr, "slot id: %d,sub board id: %d, status: %s, bitmask: 0x%x, component name: %s\n",
		                compFailureStruct.unSlotId,
		                compFailureStruct.unSubBoardId,
		                statusStr.GetString(),
		                compFailureStruct.unStatusDescriptionBitmask,
		                (char*)(compFailureStruct.sSmCompName) );
	}

	else
	{
		sprintf(resStr, "slot id: %d,sub board id: %d, status: %s, bitmask: 0x%x\n",
		                compFailureStruct.unSlotId,
		                compFailureStruct.unSubBoardId,
		                statusStr.GetString(),
		                compFailureStruct.unStatusDescriptionBitmask );
	}
}

/*
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::Ack_keep_alive( CMplMcmsProtocol& rMplProt, STATUS status ) const
{
	KEEP_ALIVE_S   rkeep_alive_ind_Struct;
	memset(&rkeep_alive_ind_Struct,0,sizeof(KEEP_ALIVE_S ));
    rkeep_alive_ind_Struct.status=eNormal;
     for (int i=0;i< MAX_NUM_OF_UNITS;i++)
          rkeep_alive_ind_Struct.statusOfUnitsList[i]=eOk;

	rMplProt.AddCommonHeader(CM_KEEP_ALIVE_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	rMplProt.AddData(sizeof(KEEP_ALIVE_S),(char*)(&rkeep_alive_ind_Struct));

	SendToCmForMplApi(rMplProt);
}
*/
/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::OnTimerSwitchConfigTout(CSegment* pMsg)
{

	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::OnTimerSwitchConfigTout - SWITCH TIMER OVER.");

	CSegment*   pSeg = new CSegment;
	*pSeg	<< m_wBoardId
			<< m_wSubBoardId;

	m_state = CONNECT;

	if( CPObject::IsValidPObjectPtr(m_pTaskApi) )
		m_pTaskApi->SendLocalMessage(pSeg,SWITCH_CONNECTED);

	CardManagerLoadedInd();
}


/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::OnTimerSoftwareUpgrade(CSegment* pMsg)
{
	//DWORD switchBurnRate=::GetBurnRate(eSwitchVersionBurnRate);
	CGideonSimLogicalModule::OnTimerSoftwareUpgrade(pMsg);

}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::OnTimerIpmcSoftwareUpgrade(CSegment* pMsg)
{

	//DWORD switchBurnRate=::GetBurnRate(eSwitchIpmcBurnRate);
	CGideonSimLogicalModule::OnTimerIpmcSoftwareUpgrade(pMsg);


}

/////////////////////////////////////////////////////////////////////////////
/*void CGideonSimSwitchLogical::OnTimerSoftwareUpgrade(CSegment* pMsg)
{

	if (m_software_upgrade_done < 100)
	  {
	    m_software_upgrade_done++;

	    UPGRADE_PROGRESS_IND_S rStruct;
	    rStruct.progress_precents = m_software_upgrade_done;
	    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	    FillMplProtocol( pMplProtocol,CM_UPGRADE_PROGRESS_IND,
			(BYTE*)(&rStruct),sizeof(UPGRADE_PROGRESS_IND_S));
	    SendToCmForMplApi(*pMplProtocol);
	    POBJDELETE(pMplProtocol);

	  }



	if (m_software_upgrade_done < 100)
	{
	  StartTimer(SOFTWARE_UPGRADE_TIMER,20);
	}
	else
	  {
	     UPGRADE_IPMC_IND_S rStruct;
	     rStruct.require_ipmc_upgrade = TRUE;
	     CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	     FillMplProtocol( pMplProtocol,CM_UPGRADE_IPMC_IND,
			      (BYTE*)(&rStruct),sizeof(UPGRADE_IPMC_IND_S));
	     SendToCmForMplApi(*pMplProtocol);
	     POBJDELETE(pMplProtocol);

	     m_software_upgrade_done = -1;
	  }

}
*/

///////////////////////////////////////////////////////////////////////////////
//void CGideonSimSwitchLogical::OnTimerIpmcSoftwareUpgrade(CSegment* pMsg)
//{
//
//	if (m_ipmc_software_upgrade_done < 100)
//	  {
//	    m_ipmc_software_upgrade_done++;
//
//	    UPGRADE_PROGRESS_IND_S rStruct;
//	    rStruct.progress_precents = m_ipmc_software_upgrade_done;
//	    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
//	    FillMplProtocol( pMplProtocol,CM_UPGRADE_IPMC_PROGRESS_IND,
//			(BYTE*)(&rStruct),sizeof(UPGRADE_PROGRESS_IND_S));
//	    SendToCmForMplApi(*pMplProtocol);
//	    POBJDELETE(pMplProtocol);
//
//	  }
//
//
//
//	if (m_ipmc_software_upgrade_done < 100)
//	{
//	  StartTimer(SOFTWARE_IPMC_UPGRADE_TIMER,50);
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
//	    //	     SendToCmForMplApi(*pMplProtocol);
//	    //	     POBJDELETE(pMplProtocol);
//
//	     m_ipmc_software_upgrade_done = -1;
//	  }
//
//}




/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::CardManagerLoadedInd()
{
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::CardManagerLoadedInd - SEND to MPL-API.");

	// send CM_CARD_MNGR_LOADED_IND
	CM_CARD_MNGR_LOADED_S  rStruct;
	memset(&rStruct,0,sizeof(CM_CARD_MNGR_LOADED_S));

	string mplSerialNum = ::GetGideonSystemCfg()->GetBoxChassisId();
	int len = mplSerialNum.length();
	memcpy(rStruct.serialNum, ::GetGideonSystemCfg()->GetBoxChassisId(), len);

	rStruct.status    = STATUS_OK;
	rStruct.cardType  = eSwitch;

	rStruct.hardwareVersion.ver_release  = 1;
	rStruct.hardwareVersion.ver_major    = 2;
	rStruct.hardwareVersion.ver_minor    = 3;
	rStruct.hardwareVersion.ver_internal = 4;

	for (int i = 0; i < MAX_NUM_OF_SW_VERSIONS; i++)
  {
    strcpy((char*) rStruct.swVersionsList[i].versionDescriptor, "The Description...");
    strcpy((char*) rStruct.swVersionsList[i].versionNumber, "12345");
  }

	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,CM_CARD_MNGR_LOADED_IND,
			(BYTE*)(&rStruct),sizeof(CM_CARD_MNGR_LOADED_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);

	m_bCMLoadedInd = TRUE;
	if(m_bSlotNumberingConversionInd)
	{
		m_bSlotNumberingConversionInd = FALSE;
		SlotsNumberingConversion();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::SlotsNumberingConversion() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::SlotsNumberingConversion - SEND to MPL-API.");

	// send SLOTS_NUMBERING_CONVERSION_IND
	SLOTS_NUMBERING_CONVERSION_TABLE_S  rStruct;
	memset(&rStruct,0,sizeof(SLOTS_NUMBERING_CONVERSION_TABLE_S));

	// fills the struct for Amos
	FillAmosSlotInConversionTable( &rStruct );

	// sends to MPL-API
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,SLOTS_NUMBERING_CONVERSION_IND,
			(BYTE*)(&rStruct),sizeof(SLOTS_NUMBERING_CONVERSION_TABLE_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);

}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::FillAmosSlotInConversionTable( SLOTS_NUMBERING_CONVERSION_TABLE_S *rStruct ) const
{
/*	if ( CProcessBase::GetProcess()->GetProductType() == eProductTypeRMX2000)
	{
		PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::FillAmosSlotInConversionTable 2000.");

		//  							  index, Board-ID  Sub-Board-ID, Display-ID
	 	FillSlotInConversionTable( rStruct, 0, 		1, 			1, 			1  );	// MPM+
	 	FillSlotInConversionTable( rStruct, 1, 		2, 			1, 			2  );	// MPM+


	 	FillSlotInConversionTable( rStruct, 2, 		1, 			2, 			1 );	// RTM-ISDN
	 	FillSlotInConversionTable( rStruct, 3, 		2, 			2, 			2 );	// RTM-ISDN
	 	FillSlotInConversionTable( rStruct, 4, 		3, 			1, 			3 );	// RTM-ISDN
	 	FillSlotInConversionTable( rStruct, 5, 		4, 			1, 			4 );	// RTM-ISDN

	 	FillSlotInConversionTable( rStruct, 6, 		5, 			1, 			5 );	// Switch

	 	rStruct->numOfBoardsInTable = 7;
	}
	else if ( CProcessBase::GetProcess()->GetProductType() == eProductTypeRMX4000)
	{*/
		PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::FillAmosSlotInConversionTable 4000.");

	//  							  index, Board-ID  Sub-Board-ID, Display-ID
 	FillSlotInConversionTable( rStruct, 0, 		1, 			1, 			1  );	// MPM+
 	FillSlotInConversionTable( rStruct, 1, 		2, 			1, 			2  );	// MPM+
 	FillSlotInConversionTable( rStruct, 2, 		3, 			1, 			3  );	// MPM+
 	FillSlotInConversionTable( rStruct, 3, 		4, 			1, 			4  );	// MPM+

 	FillSlotInConversionTable( rStruct, 4, 		1, 			2, 			13 );	// RTM-ISDN
 	FillSlotInConversionTable( rStruct, 5, 		2, 			2, 			14 );	// RTM-ISDN
 	FillSlotInConversionTable( rStruct, 6, 		3, 			2, 			15 );	// RTM-ISDN
 	FillSlotInConversionTable( rStruct, 7, 		4, 			2, 			16 );	// RTM-ISDN

 	FillSlotInConversionTable( rStruct, 8, 		5, 			1, 			17 );	// Switch
 	FillSlotInConversionTable( rStruct, 9, 		7, 			1, 			7  );	// CPU-1
 	FillSlotInConversionTable( rStruct, 10,		9, 			1, 			9  );	// CPU-2

	rStruct->numOfBoardsInTable = 11;	// for Amos (currently 11)
/*	}
else//rmx1500
{
 	FillSlotInConversionTable( rStruct, 0, 		1, 			1, 			1  );	// MPM+


 	FillSlotInConversionTable( rStruct, 1, 		1, 			2, 			13 );	// RTM-ISDN


 	FillSlotInConversionTable( rStruct, 2, 		5, 			1, 			17 );	// Switch
 	FillSlotInConversionTable( rStruct, 3, 		7, 			1, 			7  );	// CPU-1
 	FillSlotInConversionTable( rStruct, 4,		9, 			1, 			9  );	// CPU-2
 	rStruct->numOfBoardsInTable = 5;
}*/
 }

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::FillSlotInConversionTable(SLOTS_NUMBERING_CONVERSION_TABLE_S *rStruct, DWORD ind, APIU32 boardId, APIU32 subBoardId, APIU32 displayBoardId) const
{
	if (ind >= MAX_NUM_OF_BOARDS)
		return;
	if ((boardId >= MAX_NUM_OF_BOARDS) || (subBoardId >= MAX_NUM_OF_BOARDS) || (displayBoardId >= MAX_NUM_OF_BOARDS))
		return;
	rStruct->conversionTable[ind].boardId 		 = boardId;
	rStruct->conversionTable[ind].subBoardId 	 = subBoardId;
	rStruct->conversionTable[ind].displayBoardId = displayBoardId;
}
/////////////////////////////////////////////////////////////////////////////
/* amir 30-4-09
void CGideonSimSwitchLogical::SlotsNumberingConversion() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::SlotsNumberingConversion - SEND to MPL-API.");

	// send CM_CARD_MNGR_LOADED_IND
	SLOTS_NUMBERING_CONVERSION_TABLE_S  rStruct;
	memset(&rStruct,0,sizeof(SLOTS_NUMBERING_CONVERSION_TABLE_S));

	// fills the struct for Amos
	FillAmosSlotInConversionTable( &rStruct );

	// sends to MPL-API
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	FillMplProtocol( pMplProtocol,SLOTS_NUMBERING_CONVERSION_IND,
			(BYTE*)(&rStruct),sizeof(SLOTS_NUMBERING_CONVERSION_TABLE_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);

}
*/
/* amir 30-4-09
/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::FillAmosSlotInConversionTable( SLOTS_NUMBERING_CONVERSION_TABLE_S *rStruct ) const
{
	rStruct->numOfBoardsInTable = 11;

	//  							  index, Board-ID  Sub-Board-ID, Display-ID
 	FillSlotInConversionTable( rStruct, 0, 		1, 			1, 			2  );	// MPM+
 	FillSlotInConversionTable( rStruct, 1, 		2, 			1, 			3  );	// MPM+
 	FillSlotInConversionTable( rStruct, 2, 		3, 			1, 			4  );	// MPM+
 	FillSlotInConversionTable( rStruct, 3, 		4, 			1, 			5  );	// MPM+

 	FillSlotInConversionTable( rStruct, 4, 		1, 			2, 			13 );	// RTM-ISDN
 	FillSlotInConversionTable( rStruct, 5, 		2, 			2, 			14 );	// RTM-ISDN
 	FillSlotInConversionTable( rStruct, 6, 		3, 			2, 			15 );	// RTM-ISDN
 	FillSlotInConversionTable( rStruct, 7, 		4, 			2, 			16 );	// RTM-ISDN

 	FillSlotInConversionTable( rStruct, 8, 		5, 			1, 			17 );	// Switch
 	FillSlotInConversionTable( rStruct, 9, 		7, 			1, 			7  );	// CPU-1
 	FillSlotInConversionTable( rStruct, 10,		9, 			1, 			9  );	// CPU-2
 }
*/
/* amir 30-4-09
/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::FillSlotInConversionTable(SLOTS_NUMBERING_CONVERSION_TABLE_S *rStruct, DWORD ind, APIU32 boardId, APIU32 subBoardId, APIU32 displayBoardId) const
{
	if (ind >= MAX_NUM_OF_BOARDS)
		return;
	if ((boardId >= MAX_NUM_OF_BOARDS) || (subBoardId >= MAX_NUM_OF_BOARDS) || (displayBoardId >= MAX_NUM_OF_BOARDS))
		return;
	rStruct->conversionTable[ind].boardId 		 = boardId;
	rStruct->conversionTable[ind].subBoardId 	 = subBoardId;
	rStruct->conversionTable[ind].displayBoardId = displayBoardId;
}
*/
/////////////////////////////////////////////////////////////////////////////
// Indications from card to MCMS
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::EstablishConnectionInd()
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
void CGideonSimSwitchLogical::ReestablishConnectionInd()
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

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/*void CGideonSimSwitchLogical::ReestablishConnectionInd()
{
	PTRACE(eLevelInfoNormal,"CSimSwitchCard::ReestablishConnectionInd - SEND to MPL-API.");

	// prepare & send REESTABLISH_CONNECTION
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	REESTABLISH_CONNECTION_S  rStruct;
	rStruct.dummy = 111;
	FillMplProtocol( pMplProtocol,
		REESTABLISH_SetUnitStatustSwitchForKeepAliveIndCONNECTION,(BYTE*)(&rStruct),sizeof(REESTABLISH_CONNECTION_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

*/

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::AuthentificationInd() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::AuthentificationInd - SEND to MPL-API.");

	ALLOCBUFFER(pszLine1,KEYCODE_LENGTH);
	ALLOCBUFFER(pszLine2,KEYCODE_LENGTH);
	memset(pszLine1,0,KEYCODE_LENGTH);
	memset(pszLine2,0,KEYCODE_LENGTH);

	if( STATUS_OK != ReadAutenticationStringsFromFile(pszLine1,pszLine2) )
	{
		pszLine1[0] = '\0';
		pszLine2[0] = '\0';
	}

	// prepare & send MPL_AUTHENTICATION_IND
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	MPL_AUTHENTICATION_S   rStructAut;
	memset(&rStructAut,0,sizeof(MPL_AUTHENTICATION_S));
	rStructAut.mcuVersion.ver_release  = 1;
	rStructAut.mcuVersion.ver_major    = 0;
	rStructAut.mcuVersion.ver_minor    = 0;
	rStructAut.mcuVersion.ver_internal = 1;
	rStructAut.chassisVersion.ver_release	= 1;
	rStructAut.chassisVersion.ver_major		= 0;
	rStructAut.chassisVersion.ver_minor		= 0;
	rStructAut.chassisVersion.ver_internal	= 1;
	rStructAut.platformType = /*(DWORD)::GetGideonSystemCfg()->GetPlatformType();	///	*/(DWORD)eGideonLite;

	rStructAut.isNewCtrlGeneration = 1;


/* merge to 7 ????
	rStructAut.platformType = (DWORD)::GetGideonSystemCfg()->GetPlatformType();

	TRACEINTO << __FUNCTION__ << " : platformType = " << rStructAut.platformType;
*/

	string mplSerialNum = ::GetGideonSystemCfg()->GetBoxChassisId();
	int len = mplSerialNum.length();
	memcpy(rStructAut.serialNum, mplSerialNum.c_str(), len);

	memcpy(rStructAut.cfs_X_KeyCode,pszLine1,KEYCODE_LENGTH);
	memcpy(rStructAut.cfs_U_KeyCode,pszLine2,KEYCODE_LENGTH);

	FillMplProtocol( pMplProtocol,
		MPL_AUTHENTICATION_IND,(BYTE*)(&rStructAut),sizeof(MPL_AUTHENTICATION_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);

	DEALLOCBUFFER(pszLine1);
	DEALLOCBUFFER(pszLine2);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::CntlIpConfigInd() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::CntlIpConfigInd - SEND to MPL-API.");

	// prepare & send MPL_AUTHENTICATION_IND
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	IP_PARAMS_S   rStructIp;
	memset(&rStructIp,0,sizeof(IP_PARAMS_S));

	rStructIp.interfacesList[0].iPv4.isDHCPv4InUse = TRUE;
	rStructIp.interfacesList[0].iPv4.iPv4Address   = ::SystemIpStringToDWORD(::GetGideonSystemCfg()->GetMplApiIpAddress());
	rStructIp.interfacesList[0].ipType             = eIpType_IpV4;

	rStructIp.networkParams.isDhcpInUse   = FALSE; /* NETWORK_PARAMS_S */
	rStructIp.networkParams.subnetMask    = ::SystemIpStringToDWORD("255.255.248.0");
//	rStructIp.networkParams.vLanMode      = 0;
//	rStructIp.networkParams.vLanId        = 0;

//	rStructIp.dnsConfig.dnsServerStatus   = FALSE; /* DNS_CONFIGURATION_S */
	rStructIp.dnsConfig.dnsServerStatus   = eServerStatusOff; /* DNS_CONFIGURATION_S */

	strncpy((char*)rStructIp.dnsConfig.hostName,"Who_are_you",NAME_LEN-1);

	FillMplProtocol( pMplProtocol,
		MPL_CTRL_IP_CONFIG_IND,(BYTE*)(&rStructIp),sizeof(IP_PARAMS_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}
//////////////////////////////////////////////////////////////////////////
//#define MANAGEMENT_NETWORK_CONFIG_PATH  "Cfg/NetworkCfg_Management.xml"
STATUS CGideonSimSwitchLogical::CreateApiMessageFromServiceFile(IP_PARAMS_S& rStructIp)
{
	STATUS stat = STATUS_OK;
	CIPService tmpService;
	char* pFileName = getenv ("GIDEONSIM_MNGMNT_CONFIG_FILE");
	STATUS readFileStatus = tmpService.ReadXmlFile(pFileName, eNoActiveAlarm, eRenameFile);
	if(STATUS_OK ==readFileStatus)
	{
		stat = tmpService.ConvertToIpParamsStruct(rStructIp);
	}
	else
		stat = readFileStatus;
	return stat;
}
void   CGideonSimSwitchLogical::CreateApiMessageDefault(IP_PARAMS_S& rStructIp)
{
	rStructIp.interfacesList[0].iPv4.isDHCPv4InUse = TRUE;
	rStructIp.interfacesList[0].iPv4.iPv4Address   = ::SystemIpStringToDWORD(::GetGideonSystemCfg()->GetMplApiIpAddress());
	TRACEINTO 	<< "CGideonSimSwitchLogical::MngmntIpConfigInd: IpAddress: "
					<< ::GetGideonSystemCfg()->GetMplApiIpAddress();
	rStructIp.interfacesList[0].ipType             = eIpType_Both;
	rStructIp.interfacesList[0].iPv6s[0].configurationType = eV6Configuration_Manual;
	for(int i = 0; i <(int) (sizeof(rStructIp.interfacesList[0].iPv6s[0].iPv6Address[0])/sizeof(APIU8)) ; ++i)
		{
			TRACEINTO << __FUNCTION__ << " : i = " << i;
			rStructIp.interfacesList[0].iPv6s[0].iPv6Address[i] = 'A'+i;
			rStructIp.interfacesList[1].iPv6s[0].iPv6Address[i] = 'B'+i;
		}

	rStructIp.interfacesList[0].isSecured		   = ::GetGideonSystemCfg()->GetMplApiSecureMode();

	rStructIp.networkParams.isDhcpInUse   = FALSE; /* NETWORK_PARAMS_S */
	rStructIp.networkParams.subnetMask    = ::SystemIpStringToDWORD("255.255.248.0");
	//	rStructIp.networkParams.vLanMode      = 0;
	//	rStructIp.networkParams.vLanId        = 0;

	rStructIp.dnsConfig.dnsServerStatus   = eServerStatusOff; /* DNS_CONFIGURATION_S , eServerStatus */
	strncpy((char*)rStructIp.dnsConfig.hostName,"Who_are_you",NAME_LEN-1);

	// ===== Port Speed List
	rStructIp.portSpeedList[0].portNum = 0;
	rStructIp.portSpeedList[0].portSpeed = ePortSpeed_10_FullDuplex;
	rStructIp.portSpeedList[1].portNum = 1;
	rStructIp.portSpeedList[1].portSpeed = ePortSpeed_100_FullDuplex;
	rStructIp.portSpeedList[2].portNum = 2;
	rStructIp.portSpeedList[2].portSpeed = ePortSpeed_1000_FullDuplex;
}
/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::MngmntIpConfigInd()
{
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::MngmntIpConfigInd - SEND to MPL-API.");

	// prepare & send MPL_MNGMNT_IP_CONFIG_IND
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	IP_PARAMS_S   rStructIp;
	memset(&rStructIp,0,sizeof(IP_PARAMS_S));


	if (NULL != getenv ("GIDEONSIM_MNGMNT_CONFIG_FILE"))
	{
		STATUS status = STATUS_OK;
		status = CreateApiMessageFromServiceFile(rStructIp);
		if(status != STATUS_OK)
		{
			memset(&rStructIp,0,sizeof(IP_PARAMS_S));
			CreateApiMessageDefault(rStructIp);
		}
	}
	else
	  CreateApiMessageDefault(rStructIp);

	if (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator)
	{
		rStructIp.interfacesList[0].ipType             = eIpType_IpV4;
	}

	FillMplProtocol( pMplProtocol,
		MPL_MNGMNT_IP_CONFIG_IND,(BYTE*)(&rStructIp),sizeof(IP_PARAMS_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::SwLocationInd() const
{
	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::SwLocationInd - SEND to MPL-API.");

	// prepare & send MPL_SW_LOCATION_IND
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

	MPL_SW_LOCATION_S   rStructSw;
	memset(&rStructSw,0,sizeof(MPL_SW_LOCATION_S));
	rStructSw.hostIp      = ::SystemIpStringToDWORD("127.0.0.1");
	rStructSw.urlType     = eFtp;
	rStructSw.vLanId      = 0;

	FillMplProtocol( pMplProtocol,
		MPL_SW_LOCATION_IND,(BYTE*)(&rStructSw),sizeof(MPL_SW_LOCATION_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CGideonSimSwitchLogical::UpdateLicenseFile(CMplMcmsProtocol& rMplProtocol) const
{
	STATUS status = STATUS_OK;

	ALLOCBUFFER(pszLine1,KEYCODE_LENGTH+1);
	ALLOCBUFFER(pszLine2,KEYCODE_LENGTH+1);
	memset(pszLine1,'\0',KEYCODE_LENGTH+1);
	memset(pszLine2,'\0',KEYCODE_LENGTH+1);

	if( STATUS_OK == ReadAutenticationStringsFromFile(pszLine1,pszLine2) )
	{
		CFS_KEYCODE_S* pKeyCodeStruct = (CFS_KEYCODE_S*)rMplProtocol.GetData();
		char* pszKeyLine = (char*)(pKeyCodeStruct->keycode);

		if( pszKeyLine[0] == 'X' )
			strncpy(pszLine1,pszKeyLine,KEYCODE_LENGTH);
		else if( pszKeyLine[0] == 'U' )
			strncpy(pszLine2,pszKeyLine,KEYCODE_LENGTH);
		else
		{
			PTRACE2(eLevelError,"CGideonSimSwitchLogical::UpdateLicenseFile: illegal keycode - ",pszKeyLine);
			status = STATUS_FAIL;
		}
		if( STATUS_OK != WriteAutenticationStringsToFile(pszLine1,pszLine2) )
		{
			PTRACE(eLevelError,"CGideonSimSwitchLogical::UpdateLicenseFile: failed to write keycodes!");
			status = STATUS_FAIL;
		}
	}
	else
	{
		PTRACE(eLevelError,"CGideonSimSwitchLogical::UpdateLicenseFile: failed to read keycodes!");
		status = STATUS_FAIL;
	}
	DEALLOCBUFFER(pszLine1);
	DEALLOCBUFFER(pszLine2);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CGideonSimSwitchLogical::ReadAutenticationStringsFromFile(char* pszLine1,char* pszLine2) const
{
//	PTRACE(eLevelInfoNormal,"CGideonSimSwitchLogical::SwLocationInd - SEND to MPL-API.");
	STATUS status = STATUS_OK;

	char* licenseFileName = "Simulation/License.cfs";
	if ( CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator )
		licenseFileName = "Cfg/License.cfs";

	FILE* infile = fopen(licenseFileName, "r");

	if( infile )
	{
		if( NULL == fgets(pszLine1,KEYCODE_LENGTH,infile) )
		{
			PTRACE(eLevelError,"CGideonSimSwitchLogical::ReadAutenticationStringsFromFile: failed to read first line!");
			status = STATUS_FAIL;
		}
		else if( NULL == fgets(pszLine2,KEYCODE_LENGTH,infile) )
		{
			PTRACE(eLevelError,"CGideonSimSwitchLogical::ReadAutenticationStringsFromFile: failed to read second line!");
			status = STATUS_FAIL;
		}

		fclose(infile);

		char* pPtr = NULL;
		if( NULL != (pPtr = strchr(pszLine1,'\n')) )
			*pPtr = 0;
		if( NULL != (pPtr = strchr(pszLine2,'\n')) )
			*pPtr = 0;
	}
	else
	{
		PTRACE2(eLevelError,"CGideonSimSwitchLogical::ReadAutenticationStringsFromFile: failed open file - ",licenseFileName);
		status = STATUS_OPEN_FILE_FAILED;
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CGideonSimSwitchLogical::WriteAutenticationStringsToFile(const char* pszLine1,const char* pszLine2) const
{
	// check incoming parameters
	if( pszLine1 == NULL || pszLine2 == NULL )
	{
		PTRACE(eLevelError,"CGideonSimSwitchLogical::WriteAutenticationStringsToFile: illegal parameter!");
		return STATUS_FAIL;
	}

	STATUS status = STATUS_OK;

	size_t  len = strlen(pszLine1)+1;
	ALLOCBUFFER(pszTempLine1,len);
	memset(pszTempLine1,0,len);
	strncpy(pszTempLine1,pszLine1,len-1);
	pszTempLine1[len - 1] = '\0';

	len = strlen(pszLine2)+1;
	ALLOCBUFFER(pszTempLine2,len);
	memset(pszTempLine2,0,len);
	strncpy(pszTempLine2,pszLine2,len-1);
	pszTempLine2[len - 1] = '\0';

	char* pPtr = NULL;
	if( NULL != (pPtr = strchr(pszTempLine1,'\n')) )
		*pPtr = 0;
	if( NULL != (pPtr = strchr(pszTempLine2,'\n')) )
		*pPtr = 0;

	char* licenseFileName = "Simulation/License.cfs";
	if ( CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator )
		licenseFileName = "Cfg/License.cfs";

	FILE*  infile = fopen(licenseFileName,"w+");

	if( infile )
	{
		fputs(pszTempLine1,infile);
		fputc('\n',infile);
		fputs(pszTempLine2,infile);
		fputc('\n',infile);
		fclose(infile);
	}
	else
	{
		PTRACE2(eLevelError,"CGideonSimSwitchLogical::WriteAutenticationStringsToFile: failed open file - ",licenseFileName);
		status = STATUS_OPEN_FILE_FAILED;
	}

	DEALLOCBUFFER(pszTempLine1);
	DEALLOCBUFFER(pszTempLine2);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::SetUnitStatustSwitchForKeepAliveInd(WORD UnitNum, STATUS status )
{
	int boardId = UnitNum - 1; // since boardId is unitNum-1 (e.g. Switch==boardId_0)
	int subBoardId = FIXED_CM_SUBBOARD_ID;

	int unStatus = eSmComponentMajor;
	if (status == 0)
		unStatus = eSmComponentOk;
	if (status == 32)
		unStatus = eSmComponentNotExist;


	switch(UnitNum)
	{
		case 1 :{
			     m_units->m_switchSM_units.unSmComp1.unSlotId=boardId;
			     //m_units->m_switchSM_units.unSmComp2.unSubBoardId=subBoardId;
			     m_units->m_switchSM_units.unSmComp1.unStatus=unStatus;
			     m_units->m_switchSM_units.unSmComp1.unStatusDescriptionBitmask=status;
			     strncpy( (char*)(m_units->m_switchSM_units.unSmComp1.sSmCompName), "SWITCH", strlen("SWITCH")+1 );

		        break;
		        }
		case 2 :{
			     m_units->m_switchSM_units.unSmComp2.unSlotId=boardId;
			     m_units->m_switchSM_units.unSmComp2.unSubBoardId=subBoardId;
			     m_units->m_switchSM_units.unSmComp2.unStatus=unStatus;
			     m_units->m_switchSM_units.unSmComp2.unStatusDescriptionBitmask=status;
			     strncpy( (char*)(m_units->m_switchSM_units.unSmComp2.sSmCompName), "MFA", strlen("MFA")+1 );

		        break;
		        }
		case 3 :{
				/*
				// in comment since currently Mfa_boardId_2 is not implemented in GideonSim's default configuration
			     m_units->m_switchSM_units.unSmComp3.unSlotId=boardId;
			     m_units->m_switchSM_units.unSmComp3.unStatus=unStatus;
			     m_units->m_switchSM_units.unSmComp3.unStatusDescriptionBitmask=status;
			     strncpy( (char*)(m_units->m_switchSM_units.unSmComp3.sSmCompName), "MFA", strlen("MFA")+1 );
		      	*/
		        break;
		        }

		case 4 :{
			     m_units->m_switchSM_units.unSmComp4.unSlotId=boardId;
			     m_units->m_switchSM_units.unSmComp4.unStatus=unStatus;
			     m_units->m_switchSM_units.unSmComp4.unStatusDescriptionBitmask=status;
			     strncpy( (char*)(m_units->m_switchSM_units.unSmComp4.sSmCompName), "FANS", strlen("FANS")+1 );

		        break;
		        }
		case 5 :{
			     m_units->m_switchSM_units.unSmComp5.unSlotId=boardId;
			     m_units->m_switchSM_units.unSmComp5.unStatus=unStatus;
			     m_units->m_switchSM_units.unSmComp5.unStatusDescriptionBitmask=status;
			     strncpy( (char*)(m_units->m_switchSM_units.unSmComp5.sSmCompName), "PWR", strlen("PWR")+1 );

		        break;
		        }
		case 6 :{
			     m_units->m_switchSM_units.unSmComp6.unSlotId=boardId;
			     m_units->m_switchSM_units.unSmComp6.unStatus=unStatus;
			     m_units->m_switchSM_units.unSmComp6.unStatusDescriptionBitmask=status;
			     strncpy( (char*)(m_units->m_switchSM_units.unSmComp6.sSmCompName), "BACKPLANE", strlen("BACKPLANE")+1 );

		        break;
		        }
		case 7 :{
			     m_units->m_switchSM_units.unSmComp7.unSlotId=boardId;
			     m_units->m_switchSM_units.unSmComp7.unStatus=unStatus;
			     m_units->m_switchSM_units.unSmComp7.unStatusDescriptionBitmask=status;
			     strncpy( (char*)(m_units->m_switchSM_units.unSmComp7.sSmCompName), "CNTL", strlen("CNTL")+1 );

		        break;
		        }
	}

	PTRACE(eLevelError,"CGideonSimSwitchLogical::SetUnitStatustSwitchForKeepAliveInd: Shlomit+++++++^^^^^-------");
    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

    FillMplProtocol( pMplProtocol,SM_FATAL_FAILURE_IND,
			(BYTE*)(&m_units->m_switchSM_units),sizeof(SWITCH_SM_KEEP_ALIVE_S));

	SendToCmForMplApi(*pMplProtocol);
	POBJDELETE(pMplProtocol);
}


/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::SetUnitStatusSwitchForMFARemoveInd(BYTE boardId, BYTE subBoardId)
{
	TRACEINTO << "CGideonSimMfaLogical::SetUnitStatusSwitchForMFARemoveInd boardId: " <<  (int)boardId << " subBoardId: " << (int)subBoardId;

	//BOOL bIsRtm = (subBoardId==1) ? false : true;
	BOOL bIsRtm = false;
	SM_COMPONENT_STATUS_S *componentStructPtr = (SM_COMPONENT_STATUS_S *)&m_units->m_switchSM_units;


	switch(boardId)
    {
	  case 1 :
	  case 2 :
	  case 3 :
	  case 4 :
		{
	 	 for (int i=0;i<MAX_NUM_OF_BOARDS ;i++)
		  {
		    CCardCfgMfa* pCurrCardCfg = NULL;
		    pCurrCardCfg = (CCardCfgMfa*)::GetGideonSystemCfg()->GetCardCfg( i );
			if ((pCurrCardCfg) && (componentStructPtr->unSlotId == boardId) &&
					(componentStructPtr->unSubBoardId == subBoardId))
			{

				/*************************************************************************************************/
				/* 15.6.10 VNGR 15414 changed by Rachel Cohen                                                    */
				/* When card is pushed in status field will set to eSmComponentNotExist (= 2 ) with bitmask 0x20 */
				/* When card is pushed out status field will set to eSmComponentOk (= 0 ) with bitmask 0x20      */
				/*************************************************************************************************/
				componentStructPtr->unStatus = eSmComponentOk;
				componentStructPtr->unStatusDescriptionBitmask = 32;//0x20;
				TRACEINTO << "CGideonSimMfaLogical::SetUnitStatusSwitchForMFARemoveInd remove component name "<<(char*)(componentStructPtr->sSmCompName);
				break;


			}
			componentStructPtr++;
		  }
		break;
		}
		default:
		{
			PASSERT(1);
			return;
		}

	}

    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

    FillMplProtocol( pMplProtocol, SM_FATAL_FAILURE_IND, (BYTE*)(&m_units->m_switchSM_units), sizeof(SWITCH_SM_KEEP_ALIVE_S));

	SendToCmForMplApi(*pMplProtocol);

	POBJDELETE(pMplProtocol);
}


/////////////////////////////////////////////////////////////////////////////
void CGideonSimSwitchLogical::SetUnitStatusSwitchForMFAInsertInd(BYTE boardId, BYTE subBoardId)
{
	TRACEINTO << "CGideonSimMfaLogical::SetUnitStatusSwitchForMFARemoveInd boardId: " <<  (int)boardId << " subBoardId: " << (int)subBoardId;

	//BOOL bIsRtm = (subBoardId==1) ? false : true;
	BOOL bIsRtm = false;
	SM_COMPONENT_STATUS_S *componentStructPtr = (SM_COMPONENT_STATUS_S *)&m_units->m_switchSM_units;


	switch(boardId)
    {
		case 1 :
		case 2 :
		case 3 :
		case 4 :
		{

		  for (int i=0;i<MAX_NUM_OF_BOARDS ;i++)
			  {
			    CCardCfgMfa* pCurrCardCfg = NULL;
			    pCurrCardCfg = (CCardCfgMfa*)::GetGideonSystemCfg()->GetCardCfg( i );
				if ((pCurrCardCfg) && (componentStructPtr->unSlotId == boardId) &&
						(componentStructPtr->unSubBoardId == subBoardId))
				{

					/*************************************************************************************************/
					/* 15.6.10 VNGR 15414 changed by Rachel Cohen                                                    */
					/* When card is pushed in status field will set to eSmComponentNotExist (= 2 ) with bitmask 0x20 */
					/* When card is pushed out status field will set to eSmComponentOk (= 0 ) with bitmask 0x20      */
					/*************************************************************************************************/
					componentStructPtr->unStatus = eSmComponentNotExist;
					componentStructPtr->unStatusDescriptionBitmask = 32;//0x20;
					TRACEINTO << "CGideonSimMfaLogical::SetUnitStatusSwitchForMFARemoveInd insert component name "<<(char*)(componentStructPtr->sSmCompName);
					break;


				}
				componentStructPtr++;
			  }
			break;
		}
		default:
		{
			PASSERT(1);
			return;
		}

	}

    CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;

    FillMplProtocol( pMplProtocol, SM_FATAL_FAILURE_IND, (BYTE*)(&m_units->m_switchSM_units), sizeof(SWITCH_SM_KEEP_ALIVE_S));

	SendToCmForMplApi(*pMplProtocol);

	POBJDELETE(pMplProtocol);
}



///////////////////////////////////////////////////////////////////////////////
/*void CGideonSimSwitchLogical::SetStatusStatustSimMfaForKeepAliveInd(STATUS status )

{
	if (status<NUM_OF_CARD_STATES)
	    m_GideonSimMfaUnitsForKeepAliveInd.status=status;
    else
	    PTRACE(eLevelError,"CGideonSimMfaLogical::SetStatusStatustSimMfaForKeepAliveInd: illegal status");

}*/
///////////////////////////////////////////////////////////////////////////////

