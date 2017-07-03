//+========================================================================+
//                            H323AddPartyControl.CPP                      |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323AddPartyControl.CPP                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Sami| 7/4/95     | This file contains four class implementation for     |
//     |            | party control in the conference layer i.e. control   |
//     |            | of party layer that implements net and mux control,  |
//     |            | and multipoint control i.e. bridges connection and   |
//     |            | disconnections.The four classes are :                |
//     |            | CPartyCntl - This class is used as a base class for  |
//     |            |   all the other classes in is used to control party  |
//     |            |   activities while party is in steady state.         |
//     |            | CAddPartyCntl - This class is used to connect the    |
//     |            |   party to the conference.Its responsiblity to       |
//     |            |   establish h221 connection at full bitrate as       |
//     |            |   defined at the conference initial scm, and to      |
//     |            |   connect the party to the audio controller.         |
//     |            | CChangeModeCntl - This class is used to maintain     |
//     |            |   change mode requestes. Change mode requestes may   |
//     |            |   involve h221 change mode procedure and connection  |
//     |            |   and disconnection from bridge controllers.         |
//     |            |   The class has a role in case of party deletion by  |
//     |            |   "zero" mode change.                                |
//     |            | CDelPartyCntl - This class is used to remove a party |
//     |            |   from the conference.Its responsibility to disconnect|
//     |            |   the party form the audio controller and from all   |
//     |            |   other controllers if connected and to disconnect   |
//     |            |   the h221 party controller i.e. mux and net.        |
//     |            |                                                      |
//+========================================================================+

#include "H323AddPartyControl.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyGlobals.h"
#include "Conf.h"
#include "ConfApi.h"
#include "PartyApi.h"
#include "H323Caps.h"
#include "ConfDef.h"
#include "H323DialOutSimulationParty.h"
#include "StatusesGeneral.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "BridgeMoveParams.h"
#include "OpcodesMcmsCommon.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "ContentBridge.h"
#include "AudioBridgeInterface.h"
#include "VideoBridgeInterface.h"
#include "SysConfigKeys.h"
#include "IpCommon.h"
#include "CdrPersistHelper.h"

#define START_MPL_CONNECTION		20  //(*seconds) timer for error handling till first answer from MFA
#define CONNECT_TIME				210 //(*seconds) timer for error handling - Connection process. Change for homologation to 210 from 130


extern "C" void PartyH323OutEntryPoint(void* appParam);
extern "C" void CH323DialOutSimulationPartyEntryPoint(void* appParam);// for conf level simulation



//IDLE -> ALLOCATE_RSC -> CREATE_MPL -> PARTY_SETUP -> PARTY_CONNECT_AUDIO

PBEGIN_MESSAGE_MAP(CH323AddPartyCntl)
  ONEVENT(ALLOCATE_PARTY_RSRC_IND,		ALLOCATE_RSC,         	CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate)
    //ONEVENT(REALLOCATE_PARTY_RSRC_IND,	REALLOCATE_RSC,         CH323AddPartyCntl::OnRsrcReAllocatePartyRspReAllocate)
  //ECS
  //ONEVENT(REALLOCATE_PARTY_RSRC_IND,	CHANGE_ALL_MEDIA,       CH323AddPartyCntl::OnRsrcReAllocatePartyRspChangeAll)
  ONEVENT(REMOTE_SENT_RE_CAPS,	        CHANGE_ALL_MEDIA,       CH323AddPartyCntl::OnPartyReceivedReCapsChangeAll)
  ONEVENT(ACK_IND,				  CREATE_MPL,  		    CH323AddPartyCntl::OnMplAckCreate)
  // party events
  ONEVENT(H323PARTYCONNECT,		  PARTY_SETUP,			CH323AddPartyCntl::OnPartyH323ConnectSetup)
  ONEVENT(H323PARTYCONNECT,		  CHANGE_ALL_MEDIA,		CH323AddPartyCntl::OnPartyH323ConnectSetup)
  ONEVENT(H323PARTYCONNECTALL,	  PARTY_CONNECT_AUDIO,	CH323AddPartyCntl::OnPartyH323ConnectAllPartyConnectAudio)
  ONEVENT(H323PARTYCONNECTALL,	  REALLOCATE_RSC,	    CH323AddPartyCntl::OnPartyH323ConnectAllPartyReallocateRsrc)
//ONEVENT(FECC_PARTY_BRIDGE_CONNECTED,  PARTY_CONNECT_AUDIO,	CH323AddPartyCntl::OnFeccBridgeConnectedPartyConnectAudio)
  //ONEVENT(H323CONNECTCHAIRCNTL,	  PARTY_SETUP,		CH323AddPartyCntl::OnPartyH323ConnectChairCntlSetup)
//ONEVENT(ADDPROTOCOL,		      		PARTY_SETUP,		    CH323AddPartyCntl::OnPartyAddProtocolToH323Cap)
  ONEVENT(REMOVEPROTOCOL,	      PARTY_SETUP,			CH323AddPartyCntl::OnPartyRemoveProtocolFromH323Cap)
  ONEVENT(UPDATE_CAPS,		      PARTY_SETUP,			CH323AddPartyCntl::OnPartyUpdateLocalCaps)
//ONEVENT(SETVIDEORATE,	          		PARTY_SETUP,			CH323AddPartyCntl::OnPartySetNewRatesForVswCall)
//ONEVENT(CONF_FLOWCONTROL,       		PARTY_SETUP,			CH323AddPartyCntl::OnPartySetFlowControlPartyAndSendToConf)
  ONEVENT(PARTY_AUDIO_CONNECTED,  		PARTY_CONNECT_AUDIO,    CH323AddPartyCntl::OnAudConnectParty)
  ONEVENT(PARTY_AUDIO_CONNECTED,  		REALLOCATE_RSC, 		CH323AddPartyCntl::OnAudConnectParty)


  ONEVENT(SECONDARYCAUSEH323,	  PARTY_SETUP,			CH323AddPartyCntl::OnPartyH323SetSecondaryCause)
  ONEVENT(SECONDARYCAUSEH323,	  PARTY_CONNECT_AUDIO,	CH323AddPartyCntl::OnPartyH323SetSecondaryCause)

  // self timers
  ONEVENT(CONNECTDELAY,			IDLE,					CH323AddPartyCntl::OnPartyH323ConnectDelayIdle)  //SETUP - controdicts the redial
  ONEVENT(CONNECTDELAY,			ALLOCATE_RSC,			CH323AddPartyCntl::OnPartyH323ConnectDelayAllocRsc)

  ONEVENT(MFACONNECTTOUT,					ALLOCATE_RSC,          	CH323AddPartyCntl::OnConnectToutAllocate)
  ONEVENT(CONNECTTOUT,						REALLOCATE_RSC,         CH323AddPartyCntl::OnConnectToutReAllocate)
  ONEVENT(MFACONNECTTOUT,					CREATE_MPL,				CH323AddPartyCntl::OnConnectToutCreate)
  ONEVENT(CONNECTTOUT,						PARTY_SETUP,			CH323AddPartyCntl::OnConnectToutPartySetup)
  ONEVENT(CONNECTTOUT,						PARTY_CONNECT_AUDIO,	CH323AddPartyCntl::OnConnectToutPartyConnectAudio)
  ONEVENT(WAIT_FOR_RSRC_AND_ASK_AGAIN,      ALLOCATE_RSC,           CH323AddPartyCntl::OnTimerWaitForRsrcAndAskAgainAllocate)

  ONEVENT(VIDEOMUTE,			 ANYCASE,				CH323AddPartyCntl::OnPartyMuteVideo)

  ONEVENT(ADDSUBLINKSPARTIES           ,ANYCASE,       CH323AddPartyCntl::OnAddSubLinksParties)
  ONEVENT(END_AVC_TO_SVC_ART_TRANSLATOR_DISCONNECTED,  REALLOCATE_RSC,   CPartyCntl::OnEndAvcToSvcArtTranslatorDisconnected)

  PEND_MESSAGE_MAP(CH323AddPartyCntl,CH323PartyCntl);

/////////////////////////////////////////////////////////////////////////////
CH323AddPartyCntl::CH323AddPartyCntl() // constructor
{
	m_isFaulty      = FALSE;
	m_isRecovery   = FALSE;
	m_interfaceType = H323_INTERFACE_TYPE;
	VALIDATEMESSAGEMAP;
}

/////////////////////////////////////////////////////////////////////////////
CH323AddPartyCntl::~CH323AddPartyCntl() // destructor
{

}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
const char* CH323AddPartyCntl::NameOf () const
{
	return "CH323AddPartyCntl";
}

/////////////////////////////////////////////////////////////////////////////
CH323AddPartyCntl& CH323AddPartyCntl::operator=(const CH323AddPartyCntl& other)
{
	if (this != &other)
	{
		m_udpAddresses.AudioChannelPort   = other.m_udpAddresses.AudioChannelPort;
		m_udpAddresses.VideoChannelPort   = other.m_udpAddresses.VideoChannelPort;
		m_udpAddresses.ContentChannelPort = other.m_udpAddresses.ContentChannelPort;
		m_udpAddresses.FeccChannelPort    = other.m_udpAddresses.FeccChannelPort;
		m_udpAddresses.IpType             = other.m_udpAddresses.IpType;
		m_udpAddresses.IpV4Addr           = other.m_udpAddresses.IpV4Addr;

		for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
		{
			memcpy(m_udpAddresses.IpV6AddrArray[i].ip,other.m_udpAddresses.IpV6AddrArray[i].ip,IPV6_ADDRESS_BYTES_LEN);
			m_udpAddresses.IpV6AddrArray[i].scopeId = other.m_udpAddresses.IpV6AddrArray[i].scopeId;
		}


		(CH323PartyCntl&)*this = (CH323PartyCntl&)other;

	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////////
CH323AddPartyCntl& CH323AddPartyCntl::operator =(const CH323PartyCntl& other)
{
	(CH323PartyCntl&)*this = (CH323PartyCntl&)other;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void*  CH323AddPartyCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::Create(PartyControlInitParameters & partyControInitParam,PartyControlDataParameters & partyControlDataParams)     //shiraITP - 6 //shiraITP - 62
{
	SetupH323ConParameters(partyControInitParam,partyControlDataParams);
	ConnectH323(partyControInitParam,partyControlDataParams);

}
/////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::Reconnect(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	// the m_pMuxDesc was created in order to convey Tss information to the bridges
	// that know only to dill with MuxDesc and not with H323Desc.
	// since each disconnecting delete the MuxDesc we reallocate and init this member variable.
	PTRACE2PARTYID(eLevelInfoNormal,"CH323AddPartyCntl::ReconnectH323 : Name - ",m_partyConfName, GetPartyRsrcId());
	SetupH323ConParameters(partyControInitParam,partyControlDataParams);
	DialOut(partyControInitParam,partyControlDataParams);
}
/////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::DialOut(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::DialOut", GetPartyRsrcId());
	//void (*entryPoint)(void*);
	SetupH323PartyTaskEntryPoint();
	SetupCallParameters(partyControInitParam,partyControlDataParams);

	if(partyControInitParam.redialInterval  || m_connectDelay)
	{
		CSegment *pPartyDetails = new CSegment;
		partyControInitParam.pConfRcvMbx->Serialize(*pPartyDetails);

		*pPartyDetails  << (char*)partyControInitParam.pStrConfName
						<< m_McuNumber
						<< partyControInitParam.bIsRecording;

		if (partyControInitParam.redialInterval)
		{
			PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::DialOut : Redail state", GetPartyRsrcId());
			StartTimer(CONNECTDELAY,partyControInitParam.redialInterval,pPartyDetails);
			m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_REDIALING);
		}
		else
		{
			PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::DialOut : Blast state (Connection Delay)", GetPartyRsrcId());
			StartTimer(CONNECTDELAY,m_connectDelay,pPartyDetails);
		}
	}
	else
	{
        //Temporary ID before we ge a response from the resource allocator
		StartPartyConnection();//shiraITP - 8
	}

}


/////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::StartPartyConnection() //shiraITP - 63
{
	//new use for timer in Carmel - for all CH323AddPartyCntl flow - there is now one timer

	StartTimer(MFACONNECTTOUT, START_MPL_CONNECTION*SECOND);
	AllocatePartyResources();  //shiraITP - 9
}

/////////////////////////////////////////////////////////////////////////////
//Send CH323AddPartyCntl event to resource allocator process and changes state to ALLOCATE.
void CH323AddPartyCntl::AllocatePartyResources()   //shiraITP - 9
{
	//PTRACEPARTYID(eLevelInfoNormal, "CH323AddPartyCntl::AllocatePartyResources", GetPartyRsrcId());
	TRACEINTO << " CH323AddPartyCntl::AllocatePartyResources :" << "monitor_conf_id: " << m_monitorConfId << ", monitor_party_id: " << m_monitorPartyId << ", party name: " << m_partyConfName << ", roomID:" << m_RoomId;
	BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_H323_FREE_VIDEO_RESOURCES);
	EAllocationPolicy eIsFreeVideoResources = (bEnableFreeVideoResources && IsUndefinedParty())? eAllowDowngradingToAudioOnly : eNoAllocationPolicy;

	eVideoPartyType videoPartyType = eVideo_party_type_none;
	m_pIpInitialMode->Dump("CH323AddPartyCntl::AllocatePartyResources Initial:", eLevelInfoNormal);

	videoPartyType = GetLocalVideoPartyType();
	DWORD artCapacity = 0;
	artCapacity = CalculateArtCapacityAccordingToScm(m_pIpInitialMode, FALSE);
	m_artCapacity = artCapacity;

 	CreateAndSendAllocatePartyResources(eIP_network_party_type,videoPartyType, eIsFreeVideoResources,FALSE/*isPortGaugeFlagOn*/,FALSE/*IsEnableSipICE*/,artCapacity,
	                                    eTipNone, m_RoomId , FALSE/*isBfcpEnabled*/);

	m_bAdditionalRsrcActivated = FALSE;
	m_state = ALLOCATE_RSC;
}

/////////////////////////////////////////////////////////////////////////////
//change state to setup
//deserialize response
//add info to routing table
//Send Create Party to MFA
void CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate(CSegment* pParam) //shiraITP - 10 //shiraITP - 64
{
	PTRACE(eLevelInfoNormal, "CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate");
	PTRACEPARTYID(eLevelInfoNormal, "CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate", GetPartyRsrcId());
	CPartyRsrcDesc* pTempPartyAllocatedRsrc = new CPartyRsrcDesc;
	pTempPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);

	if (m_pPartyHWInterface)
		POBJDELETE(m_pPartyHWInterface);

	DWORD status = pTempPartyAllocatedRsrc->GetStatus();

	//============================================================================================
	// BRIDGE-10439
	// PartyCntl must know the rsrcId unconditionally, as even if the resource-allocation fails,
	// the party will try to remove itself from the lists using its resource ID.
	//============================================================================================
	POBJDELETE(m_pPartyAllocatedRsrc);
	m_pPartyAllocatedRsrc = new CPartyRsrcDesc(*pTempPartyAllocatedRsrc);

	if (status != STATUS_OK)
	{

		char buf[300];
		sprintf(buf,"CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate: STATUS ( %d )",status);
	    PTRACEPARTYID(eLevelInfoNormal, buf, GetPartyRsrcId());

	    //=====================================================================================================
	    // The following line got commented. On self kill the party will clear itself from the flag lists,
	    // so its a guaranteed assert to do the following line that will appear when the party will redo it:
	    // ClearUnusedPartyId(pTempPartyAllocatedRsrc->GetPartyRsrcId());
	    //=====================================================================================================

	    if( STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN == status)
	    {
	    	///Resource Allocator tries to fulfill the allocation request by reconfigurating the DSP, we will set a timer and retry to allocate the resources
	    	if(m_isWaitingForRsrcAndAskAgain)
	    	{
	  		  WaitForRsrcAndAskAgain();
	  		  POBJDELETE(pTempPartyAllocatedRsrc);
	  		  return;
	    	}
	    	else
	    	{
	    		///We only retry to allocate resources in case of STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN just once
	    		PTRACE2PARTYID(eLevelInfoNormal, "CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate: STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN but the m_isWaitingForRsrcAndAskAgain is set to NO!!!!! party: ", m_partyConfName, GetPartyRsrcId());
	    		PASSERT(1);
	    	}
	    }
		if (( (STATUS_INSUFFICIENT_VIDEO_RSRC==status) || (STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED==status) ) &&
									 (m_type==DIALOUT) && (m_connectDelayCounter < NUM_OF_CONNECT_DELAY_RETRIES))
		{
			PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate :Temporary Video Resources Deficiency Connection Delay ", GetPartyRsrcId());
			//char buf[300];
			//sprintf(buf,"CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate: Connect Delay ( %d ) ,ConnectDelayTime (%d)",m_connectDelayCounter+1,m_connectDelay);
	    	//PTRACEPARTYID(eLevelInfoNormal, buf);
			//Invoke Connect Delay to allow the system DeAllocation of resources that reserved by a Party in SD
			//Conf that at last need less Resources.
			//m_state=IDLE;
			CSegment *pPartyDetails = new CSegment;
			m_pConfRcvMbx->Serialize(*pPartyDetails);

			*pPartyDetails  << (char*)m_confName
							<< m_McuNumber
							<< m_isRecording;

			m_connectDelayCounter++;
			StartTimer(CONNECTDELAY,400,pPartyDetails); //It tooks 3sec to deAlloc unNecesary resource(worst case)
	    }

		else
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate : ALLOCATION FAILED!!! do not continue process : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());
			m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,RESOURCES_DEFICIENCY,1); // Disconnnect cause
			EndConnectionProcess(statIllegal);

		}

		pTempPartyAllocatedRsrc->DumpToTrace();
		POBJDELETE(pTempPartyAllocatedRsrc);
		return;
	}

	eNetworkPartyType networkPartyType =  pTempPartyAllocatedRsrc->GetNetworkPartyType();
	if(networkPartyType!=eIP_network_party_type)
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate ALLOCATION FAILED!!! do not continue process eNetworkPartyType!= eIP_network_party_type, eNetworkPartyType = ",eNetworkPartyTypeNames[networkPartyType], GetPartyRsrcId());
		PASSERT(1);
		m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,RESOURCES_DEFICIENCY,1); // Disconnnect cause
		EndConnectionProcess(statIllegal);
		pTempPartyAllocatedRsrc->DumpToTrace();
		POBJDELETE(pTempPartyAllocatedRsrc);
		return;
	}

	POBJDELETE(pTempPartyAllocatedRsrc);

	InitUdpAddresses(pParam);

	if(m_pPartyAllocatedRsrc)
	{
		m_pPartyAllocatedRsrc->DumpToTrace();
		DumpUdpAddresses();

		CheckResourceAllocationTowardsRequest();
		m_eLastAllocatedVideoPartyType = m_pPartyAllocatedRsrc->GetVideoPartyType();
	}
	else
	{
		PASSERT(1);
	}

	//Multiple links for ITP in cascaded conference feature: CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate (set telepresenseEPInfo   Main: set roomID, mailbox)
	if (m_pPartyAllocatedRsrc)
	    m_RoomId = m_pPartyAllocatedRsrc->GetRoomId();
	else
	{
	    PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate ERROR - m_pPartyAllocatedRsrc is NULL, set RoomID=0");
	    m_RoomId = 0;
	}

	if (m_telepresenseEPInfo)
	{
	    m_telepresenseEPInfo->SetRoomID(m_RoomId);
	    m_telepresenseEPInfo->SetPartyMonitorID(m_monitorPartyId);
	}
	else
	{
	    PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate ERROR - m_telepresenseEPInfo is NULL");
	}

	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	if (pConfParty)
	    pConfParty->SetRoomId(m_RoomId);

	TRACESTR(eLevelError) << "ITP_CASCADE: CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate m_RoomId:" << m_RoomId  << " partyName:" << GetName();

	if (m_linkType == eMainLinkParty)
	{
	    if (m_roomControl)
	    {
	        m_roomControl->SetRoomID((DWORD)m_RoomId);
	        if(m_pPartyAllocatedRsrc)
	        	m_roomControl->SetMailBoxAccordingToIndex(0,m_pPartyAllocatedRsrc->GetPartyRsrcId());
	        else
	        	PASSERT(1);
	    }
	    else
	    {
	        PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate ERROR - m_roomControl is NULL");
	    }
	}

	// in Dial out, only Now allocate the real PartyID
	if (DIALOUT == m_type)
	{
		m_pPartyApi = new CPartyApi;
		m_pPartyApi->Create(m_entryPoint,
		                    *m_pConfRcvMbx,
		                    *m_pConfRcvMbx,
		                    GetPartyRsrcId(),
		                    m_monitorPartyId,
		                    m_monitorConfId,
		                    m_pConf->GetCommConf()->GetNumericConfId(),
		                    m_name,
		                    m_confName,
		                    m_serviceId,
		                    m_termNum,
		                    m_McuNumber,
		                    m_password,
		                    m_voice,
		                    1,
		                    m_IsGateWay,
		                    m_isRecording,
		                    m_isAutoVidBitRate,
		                    m_bNoVideRsrcForVideoParty);

		m_pParty = m_pPartyApi->GetTaskAppPtr();
	}

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << " PartyName:" << m_name << ", PartyId:" << m_pParty->GetPartyId() << "(" << GetPartyRsrcId() << ")" << ", IsDialOut:" << m_type << ", Pointer:" << std::hex << m_pParty;
#endif

	InsertPartyResourcesToGlobalRsrcRoutingTbl();

	ISDN_PARTY_IND_PARAMS_S tempIsdnParams;
	SVC_PARTY_IND_PARAMS_S  svcParams;
	pParam->Get((BYTE*)(&tempIsdnParams), sizeof(ISDN_PARTY_IND_PARAMS_S));
	pParam->Get((BYTE*)(&svcParams), sizeof(SVC_PARTY_IND_PARAMS_S));

	eConfMediaType confMediaType = eAvcOnly;
	if (m_pPartyAllocatedRsrc)
		confMediaType = m_pPartyAllocatedRsrc->GetConfMediaType();
	else
		PASSERT(1);

	m_pIpInitialMode->SetConfMediaType(confMediaType);

	TRACEINTO << "ConfMediaType:" << m_pIpInitialMode->GetConfMediaType();
    if (CheckResourceAllocationForMixMode() == FALSE)
        return;

	UpdateScmWithResources(svcParams,m_eLastAllocatedVideoPartyType, m_pPartyAllocatedRsrc->GetIsAvcVswInMixedMode());

	if(m_pPartyAllocatedRsrc)
	{
		m_pPartyHWInterface = new CPartyInterface(m_pPartyAllocatedRsrc->GetPartyRsrcId(), m_pPartyAllocatedRsrc->GetConfRsrcId());
		m_pPartyHWInterface->SetRoomId(m_RoomId);
	}
	else
		PASSERT(1);

	m_state = CREATE_MPL;
	m_lastReqId = SendCreateParty(E_NETWORK_TYPE_IP);
}


/////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::InitUdpAddresses(CSegment* pParam)
{
	int sizeOfUdps = sizeof(UdpAddresses);
	UdpAddresses  sUdpAddressesParams;
	memset(&sUdpAddressesParams,0,sizeOfUdps);
	pParam->Get((BYTE*)(&sUdpAddressesParams),sizeOfUdps);

	m_udpAddresses.IpType = sUdpAddressesParams.IpType;
	m_udpAddresses.AudioChannelPort   = sUdpAddressesParams.AudioChannelPort;
	m_udpAddresses.VideoChannelPort   = sUdpAddressesParams.VideoChannelPort;
	m_udpAddresses.ContentChannelPort = sUdpAddressesParams.ContentChannelPort;
	m_udpAddresses.FeccChannelPort    = sUdpAddressesParams.FeccChannelPort;
	m_udpAddresses.IpV4Addr.ip   	  = sUdpAddressesParams.IpV4Addr.ip;

	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memcpy(m_udpAddresses.IpV6AddrArray[i].ip,sUdpAddressesParams.IpV6AddrArray[i].ip,IPV6_ADDRESS_BYTES_LEN);
		m_udpAddresses.IpV6AddrArray[i].scopeId = sUdpAddressesParams.IpV6AddrArray[i].scopeId;
	}



}

///////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::DumpUdpAddresses()
{
	CLargeString *pStr = new CLargeString;
	*pStr	<< "AudioChannelPort	 = " << m_udpAddresses.AudioChannelPort  <<'\n'
			<< "VideoChannelPort	 = " << m_udpAddresses.VideoChannelPort  <<'\n'
			<< "ContentChannelPort 	 = " << m_udpAddresses.ContentChannelPort  <<'\n'
			<< "FeccChannelPort		 = " << m_udpAddresses.FeccChannelPort  <<'\n'
			<< "UdpAddressType		 = " << m_udpAddresses.IpType  <<'\n';

	char str[128];
	*pStr << "IP v4	  = " << ::ipV4ToString(m_udpAddresses.IpV4Addr.ip, str) <<'\n';

	*pStr << "Ip V6:";
	mcTransportAddress tempAddr;
	char szIP[IPV6_ADDRESS_LEN];
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(&tempAddr,0,sizeof(mcTransportAddress));
		memset(szIP,0,IPV6_ADDRESS_LEN);

		tempAddr.ipVersion = (DWORD)eIpVersion6;
		tempAddr.addr.v6.scopeId = m_udpAddresses.IpV6AddrArray[i].scopeId;
		memcpy(tempAddr.addr.v6.ip,m_udpAddresses.IpV6AddrArray[i].ip,IPV6_ADDRESS_BYTES_LEN);
		::ipToString(tempAddr,szIP,1);

		*pStr << "\nIp V6 " << i << " ="
		      << "\nScope Id: " << m_udpAddresses.IpV6AddrArray[i].scopeId
		      << "\nIp: " << szIP << "\n";
	}


	//		*pStr << "IP v6	  = " << (WORD)m_udpAddresses.IpAddr.v6.ip  <<'\n';
	PTRACEPARTYID(eLevelInfoNormal, pStr->GetString(), GetPartyRsrcId());
	POBJDELETE(pStr);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnMplAckCreate(CSegment* pParam) //shiraITP - 11 - party task was created
{
	OPCODE ackOpcode;
	DWORD  ackSeqNum;
	STATUS status;
	*pParam >> ackOpcode >> ackSeqNum >> status;
	CMedString str;

	if (ackOpcode == CONF_MPL_CREATE_PARTY_REQ)
	{

		DeleteTimer(MFACONNECTTOUT);

		if (status == STATUS_OK)
		{
			PTRACEPARTYID(eLevelInfoNormal, "CH323AddPartyCntl::OnMplAckCreate", GetPartyRsrcId());
			EstablishH323Call(); //shiraITP - 12
		}
		else
		{
			PTRACE2PARTYID(eLevelError, "CH323AddPartyCntl::OnMplAckCreate : CREATION FAILED!!! do not continue process : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());

			CLargeString cstr;
			cstr << "Party:" << GetPartyRsrcId() << " Conf:" << GetConfRsrcId();
			cstr << " receives Failure Status for opcode: TB_MSG_OPEN_PORT_REQ(CONF_MPL_CREATE_PARTY_REQ) ";
			cstr << " req:" << m_lastReqId;
			cstr << "( Responsibility: embedded )";

			DumpMcuInternalDetailed(cstr,ACK_FAILED);
			//str << "CH323AddPartyCntl::OnMplAckCreate : create Party failed, Party Rsrc ID - " << GetPartyRsrcId();
			//CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, ACK_FAILED, MAJOR_ERROR_LEVEL, str.GetString(), TRUE);
			DBGPASSERT(status);
			m_isFaulty = 1; // Invoking KillPort process in RA.

			BYTE 	mipHwConn = (BYTE)eMipCardManager;
			BYTE	mipMedia = (BYTE)eMipArt;
			BYTE	mipDirect = eMipNoneDirction;
			BYTE	mipTimerStat = eMipStatusFail;
			BYTE	mipAction = eMipOpen;

			CSegment* pSeg = new CSegment;
			*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
			DWORD MpiErrorNumber = 1;//GetMpiErrorNumber(pSeg);
			m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
			POBJDELETE(pSeg);

		}
	}
	else
	{
		PTRACEPARTYID(eLevelError,"CH323AddPartyCntl::OnMplAckCreate : invalid opcode", GetPartyRsrcId());
		str << "CH323AddPartyCntl::OnMplAckCreate : Invalid Opcode, Response Opcode - " << ackOpcode;
	    CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, INVALID_OPCODE_RESPONSE, MAJOR_ERROR_LEVEL, str.GetString(), TRUE);
		m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
		if(ackOpcode)
			DBGPASSERT(ackOpcode);
		else
			PASSERTMSG(GetPartyRsrcId(), "No Opcode");
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::EstablishH323Call() //shiraITP - 12 //shiraITP - 65
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::EstablishH323Call", GetPartyRsrcId());

	m_state = PARTY_SETUP;

	StartTimer(CONNECTTOUT, CONNECT_TIME*SECOND);
	//update FECC
	WORD feccStatus = TRUE;
	if (!IsFECC())
		feccStatus = FALSE;
	m_pTaskApi->UpdateDB(m_pParty, IPFECCSTATUS, feccStatus);

	//encryption
	WORD  encAlg		= kUnKnownMediaType;
	WORD  halfKeyType	= kHalfKeyUnKnownType;
	if (m_pLocalCapH323->IsPartyEncrypted())
	{
		encAlg		= (WORD)m_pLocalCapH323->GetEncryptionAlg();
		halfKeyType = GetHalfKeyType();
	}

//	BYTE  bIsIpOnlyConf = (m_pConf->GetCommConf()->GetNetwork() == NETWORK_H323);
	if(m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc)
	{
		TRACEINTO << "mix_mode - already in mix mode, reset flags";
		m_deferUpgrade = false;
		m_bPartyInUpgradeProcess = false;
	}

	CRsrcParams* pRtpRsrcParams = new CRsrcParams;
	*pRtpRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_rtp);

	CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
		for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
		{
			avcToSvcTranslatorRsrcParams[i]=NULL;
		}

    CRsrcParams* pMrmpRsrcParams = NULL;
    CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
    eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpInitialMode,TRUE);
//    if ((eCurrentVideoType != eVideo_party_type_none && eCurrentVideoType != eCOP_party_type  &&	// KW amirk/Angelina
//        (pCommConf->GetConfMediaType() == eMixAvcSvc)))
    if (m_bIsMrcCall || (eCurrentVideoType != eVideo_party_type_none && eCurrentVideoType!= eCOP_party_type  &&
    			       m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc )
    			       || (m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc && eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily()))
    {
        pMrmpRsrcParams = new CRsrcParams;
        *pMrmpRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_relay_rtp);
        if (pMrmpRsrcParams->GetRsrcDesc())
        {
            if (pMrmpRsrcParams->GetRsrcDesc()->GetConnectionId() == 0)
            {
                TRACEINTOFUNC << "mix_mode: eLogical_relay_rtp wasn't allocated!";
                POBJDELETE(pMrmpRsrcParams);
                pMrmpRsrcParams=NULL;
                PASSERT(101);
//              SystemCoreDump(FALSE);
            }
        }
        else
        {
            TRACEINTOFUNC << "mix_mode: eLogical_relay_rtp wasn't allocated!";
            PASSERT(101);
//          SystemCoreDump(FALSE);
        }
    }

    CRsrcParams* pCsRsrcParams = new CRsrcParams;
	*pCsRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_signaling);

	BYTE  bIsIpOnlyConf = TRUE;
	eLogicalResourceTypes logicalType[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS]={eLogical_relay_avc_to_svc_rtp_with_audio_encoder,eLogical_relay_avc_to_svc_rtp};

	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		WORD itemNum=1;
		WORD found;
		avcToSvcTranslatorRsrcParams[i]=new CRsrcParams;
		found=m_pPartyAllocatedRsrc->GetRsrcParams(*avcToSvcTranslatorRsrcParams[i],logicalType[i],itemNum);

//		ConnectionID connectionId=1;
//		m_pPartyAllocatedRsrc->GetRsrcDesc(*(avcToSvcTranslatorRsrcParams[i]),connectionId);
//		m_pPartyAllocatedRsrc->GetRsrcDesc(*(avcToSvcTranslatorRsrcParams[i]),logicalType[i],itemNum);
		//WORD CPartyRsrcDesc::GetRsrcDesc(CRsrcDesc& rsrcDesc,eLogicalResourceTypes lrt,WORD item_number)

		if(found)
		{
			TRACEINTO<<"!@# good:  translator:"<<i<<" will be allocated";
		}
		else
		{
			POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
			avcToSvcTranslatorRsrcParams[i]=NULL;
			TRACEINTO<<"!@# bad:  translator:"<<i<<" will not be allocated";
		}
	}

	WORD refMcuNumber = 1;
	WORD refTerminalNumber = 1;
	if (m_pTerminalNumberingManager)
   	{
   		m_pTerminalNumberingManager->allocatePartyNumber(m_pParty, refMcuNumber, refTerminalNumber);
   		ON(m_isTerminalNumberingConn);
   	}
	else
		PASSERTMSG(GetPartyRsrcId(),"Terminal numbering manager not valid");

	m_pPartyApi->H323EstablishCall(GetPartyRsrcId(), m_pLocalCapH323, m_pH323NetSetup,    //shiraITP - 13 - we don't need the numOfLinks, (we can use confParty)
					m_cascadeMode, m_nodeType, m_videoRate, m_pQos,
					/*bIsIpOnlyConf,*/ m_pIpInitialMode, encAlg, halfKeyType,
					pRtpRsrcParams, pMrmpRsrcParams, pCsRsrcParams,avcToSvcTranslatorRsrcParams, m_udpAddresses, refMcuNumber, refTerminalNumber,
					m_pCopVideoTxModes,m_bNoVideRsrcForVideoParty,m_pTerminalNumberingManager,m_RoomId,m_linkType,
					m_SsrcIdsForAvcParty);

	POBJDELETE(pRtpRsrcParams);
    POBJDELETE(pMrmpRsrcParams);
	POBJDELETE(pCsRsrcParams);
	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
     {
		POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
	 }

	if(DIALOUT == m_type)
	{
    	if(m_isRecording)// if its recording link send the exchange ID and if streaming status.
    	{
    		if(m_isStreaming)
    			PTRACE2PARTYID(eLevelInfoNormal,"CH323AddPartyCntl::EstablishH323Call, pass streaming plus exchange conf ID info - ", (const char*)m_pExchangeServerConfId, GetPartyRsrcId());
    		else
    			PTRACE2PARTYID(eLevelInfoNormal,"CH323AddPartyCntl::EstablishH323Call, pass only exchange conf ID info (without streaming) - ", (const char*)m_pExchangeServerConfId, GetPartyRsrcId());

    		m_pPartyApi->SendInfoToRss(m_isStreaming, (char*)m_pExchangeServerConfId);
    	}

    /*
    	// for RSS testing
    	if(1)// if its recording link send the exchange ID and if streaming status.
    	{
    		m_pPartyApi->SendInfoToRss(1, "872kLO1ndk98u");
    	}
    */
	}

	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTING);
    m_pPartyApi->UpdatePartyCapabilitiesAndAudioRate(m_pConf->GetCommConf()->GetAudioRate());
}

/////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::OnPartyH323ConnectSetup(CSegment* pParam)
{
	WORD status;
	WORD masterSlaveStatus;

	TRACECOND_AND_RETURN(!m_pConf, "CH323AddPartyCntl::OnPartyH323ConnectSetup: m_pConf is NULL!!");

	*pParam >> status >> m_bIsCascade >> masterSlaveStatus >> m_incomingVideoChannelHandle;

	m_masterSlaveStatus = (int)masterSlaveStatus;

	TRACEINTO << "ConfId:" << GetConfRsrcId() <<" PartyId:" << GetPartyRsrcId() << " status:" << status << " IsCascade: " << (WORD)m_bIsCascade << " m_masterSlaveStatus:" << m_masterSlaveStatus << " incomingVideoChannelHandle:" << m_incomingVideoChannelHandle;

	// Multiple links for ITP in cascaded conference feature: CH323AddPartyCntl::OnPartyH323ConnectSetup Link icon in EMA..

	if (m_bIsCascade == CASCADE_MCU || m_bIsCascade == CASCADE_GW)
	{
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
		DBGPASSERT(!pConfParty);
		if (pConfParty)
		{
			if (m_masterSlaveStatus == CASCADE_MODE_MASTER)
				pConfParty->SetCascadeMode(CASCADE_MODE_MASTER);
			else if (m_masterSlaveStatus == CASCADE_MODE_SLAVE)
				pConfParty->SetCascadeMode(CASCADE_MODE_SLAVE);
		}
	}
	// clear the party cascade type
	else if (m_bIsCascade == CASCADE_MODE_NONE)
	{
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
		if (pConfParty)
		{
			pConfParty->SetCascadeMode(0);
		}
	}

	if ((status == statOK) || (status == statSecondary) || (status == statVideoBeforeAudio))
	{
		m_state = PARTY_CONNECT_AUDIO;
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTED_PARTIALY);
	}

	PartyConnectedAudio(pParam, status);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnPartyH323ConnectAllPartyConnectAudio(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnPartyH323ConnectAllPartyConnectAudio", GetPartyRsrcId());
	PartyH323ConnectAllPartyConnectAudioOrChangeAll(pParam);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnPartyH323ConnectAllPartyReallocateRsrc(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnPartyH323ConnectAllPartyReallocateRsrc", GetPartyRsrcId());
    //only to update the internal members (scm, caps etc.)
    CH323PartyCntl::PartyH323ConnectAllPartyConnectAudioOrChangeAll(pParam);
}

/////////////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::PartyH323ConnectAllPartyConnectAudioOrChangeAll(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::PartyH323ConnectAllPartyConnectAudioOrChangeAll", GetPartyRsrcId());

	CH323PartyCntl::PartyH323ConnectAllPartyConnectAudioOrChangeAll(pParam);
	AdditionalRsrcHandling(statOK);
	ConnectPartyToFECCBridge(m_pIpCurrentMode);

}

/////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::ContinueAddPartyAfterMove()
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::ContinueAddPartyAfterMove", GetPartyRsrcId());
	m_state = m_AddPartyStateBeforeMove;
	if (IsPartyConnectAllWhileMove())
	{
		AdditionalRsrcHandling(statOK);
		ConnectPartyToFECCBridge(m_pIpCurrentMode);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnAudConnectParty(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnAudConnectParty", GetPartyRsrcId());
	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnAudConnectParty : Connect has received after disconnect", GetPartyRsrcId());
	}

	else
	{
		EMediaDirection eMediaDirection = HandleAudioBridgeConnectedInd(pParam);
            m_pIpCurrentMode->Dump("CH323AddPartyCntl::OnAudConnectParty current", eLevelInfoNormal);

			//BOOL bMuteStatus = m_pIpCurrentMode->GetMediaMode(cmCapAudio, cmCapReceive).GetMute();
            CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
            EOnOff eOnOff = eOff;
            if (CPObject::IsValidPObjectPtr(pConfParty))
            {
         	if (eMediaDirection == eMediaIn || eMediaDirection == eMediaInAndOut)
         	{
        		BOOL bMuteStatus = pConfParty->IsAudioMutedByParty();
                eOnOff = bMuteStatus ? eOn : eOff;
				m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eOnOff, PARTY);

				/*VNGR-23140: In case of failover or redial, maybe the audio was muted/blocked by the operator in the master*/
				if (pConfParty->IsAudioMutedByOperator()) {
					PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnAudConnectParty: Audio muted by Operateor", GetPartyRsrcId());	
					eOnOff = eOn;
					m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eOnOff, OPERATOR);
				}
         	}
			if (eMediaDirection == eMediaOut || eMediaDirection == eMediaInAndOut) 
			{
				if (pConfParty->IsAudioBlocked()) {
					PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnAudConnectParty: Audio blocked by Operateor", GetPartyRsrcId());	
					eOnOff = eOn;
					m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaOut, eOnOff, OPERATOR);
				}
            }
        }
		
		if (AreTwoDirectionsConnectedToAudioBridge())
		{

			if (m_isFullBitRateConnect)
				m_pTaskApi->EndAddParty(m_pParty,statOK);
			
			PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnAudConnectParty: TwoDirectionsConnectedToAudioBridge", GetPartyRsrcId()); 
			CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
			if (CPObject::IsValidPObjectPtr(pConfParty))
			{
			WORD bIsAudioOnly = pConfParty->GetVoice();
			if (bIsAudioOnly)
			{
				PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnAudConnectParty: EndConnectionProcess", GetPartyRsrcId()); 
				EndConnectionProcess(statOK);
			}
		}
	}
}
}

/////////////////////////////////////////////////////////////////////////////
WORD CH323AddPartyCntl::GetHalfKeyType() const
{
	return m_pIpInitialMode->GetEncryptionHalfKeyType();
}

//////////////////////////////////////////////////////////////////////////////
/*
void  CH323AddPartyCntl::OnPartyAddProtocolToH323Cap(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnPartyAddProtocolToH323Cap : Name - ",m_partyConfName, GetPartyRsrcId());
	WORD numberOfAddedCodecs = 0;
	WORD codecsEnumCap = 0;
	CapEnum codecInternalConst = eUnknownAlgorithemCapCode;

	*pParam >> numberOfAddedCodecs;
	*pParam >> codecsEnumCap;
	codecInternalConst = (CapEnum)codecsEnumCap;

	m_pLocalCapH323->AddProtocolToCapSet(1, codecInternalConst);
}
*/
//////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnPartyRemoveProtocolFromH323Cap(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnPartyRemoveProtocolFromH323Cap", GetPartyRsrcId());
	WORD NoRemovedProtocols;
	WORD ProtocolPayLoadType;
	WORD bIsCapEnum;

	*pParam >> NoRemovedProtocols
			>> ProtocolPayLoadType
			>> bIsCapEnum;

	if(bIsCapEnum)
		m_pLocalCapH323->RemoveProtocolFromCapSet((CapEnum)ProtocolPayLoadType);
	else
		m_pLocalCapH323->RemoveProtocolFromCapSet((payload_en)ProtocolPayLoadType);
}


//////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnPartyUpdateLocalCaps(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::UpdateLocalCaps", GetPartyRsrcId());
	m_pLocalCapH323->DeSerialize(NATIVE,*pParam);
}

//////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::ChangeScm(CIpComMode* pH323Scm, EChangeMediaType eChangeMediaType)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::ChangeScm", GetPartyRsrcId());

	if (m_state == IDLE)
	{
		PTRACEPARTYID(eLevelError,"CH323AddPartyCntl::ChangeScm - State is IDLE - don't change scm", GetPartyRsrcId());
		return;
	}

	if (pH323Scm)
		ChangeScmAccordingToH323Scm(pH323Scm);
	else
		PTRACEPARTYID(eLevelError,"CH323AddPartyCntl::ChangeScm - No H320 Scm", GetPartyRsrcId());
}

/////////////////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::OnPartyH323SetSecondaryCause(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnPartyH323SetSecondaryCause", GetPartyRsrcId());

	*pParam >> m_SecondaryCause;

	POBJDELETE(m_pSecondaryParams);

	m_pSecondaryParams = new CSecondaryParams;
	m_pSecondaryParams->DeSerialize(NATIVE,*pParam);
}

/////////////////////////////////////////////////////////////////////////////
/*void  CH323AddPartyCntl::OpenDataChannel(WORD bitRate,WORD type)
{
	if (m_pIpInitialMode->GetConfType() == kVideoSwitch) //We need to do changes only in VSW
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OpenDataChannel: VSW conference -  Name - ",m_partyConfName, GetPartyRsrcId());
		if (type == LSD)
			m_lsdRequestRate = bitRate;
	}
	else
	{
		m_bConfWaitToEndChangeModeForFecc = TRUE;
		PTRACE2PARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OpenDataChannel: CP conference - Name - ",m_partyConfName, GetPartyRsrcId());
	}
}*/
/////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnPartyH323ConnectDelayIdle(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnPartyH323ConnectDelayIdle", GetPartyRsrcId());
	PartyH323ConnectDelay(pParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnPartyH323ConnectDelayAllocRsc(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnPartyH323ConnectDelayAllocRsc", GetPartyRsrcId());
	PartyH323ConnectDelay(pParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::PartyH323ConnectDelay(CSegment* pParam)
{
	m_connectDelay = 0;
	//To avoid endless loop
//	if(m_connectDelayCounter > NUM_OF_CONNECT_DELAY_RETRIES)
//	{
//		PASSERTMSG(1,"CH323AddPartyCntl::OnPartyH323ConnectDelayIdle enter endless loop state", GetPartyRsrcId());
//		return;
//	}
	//Temporary ID before we ge a response from the resource allocator
#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.M. PartyId:" << GetPartyId() << ", monitorPartyId:" << m_monitorPartyId << ", pParty:" << std::hex << m_pParty;
#endif
	m_pParty = (CTaskApp*)(m_monitorPartyId + 100);  // just to get unique id for party list
	StartPartyConnection();
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnConnectToutAllocate(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323AddPartyCntl::OnConnectToutAllocate, Timer Expired No Rsp from Rsrc Allocator", GetPartyRsrcId());
	TRACESTR (eLevelError) << "CH323AddPartyCntl::OnConnectToutAllocate: Conf Id" << m_monitorConfId
	<< "Party name is "<< m_name <<"Party Monitor Id is " << m_monitorPartyId;
	DBGPASSERT(GetPartyRsrcId());
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnConnectToutReAllocate(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323AddPartyCntl::OnConnectToutReAllocate, Timer Expired No Rsp from Rsrc Allocator", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
	m_isFaulty = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnConnectToutCreate(CSegment* pParam)
{
	CLargeString cstr;
	cstr << "Party:" << GetPartyRsrcId() << " Conf:" << GetConfRsrcId();
	cstr << " Did not receive ACK  for opcode: : TB_MSG_OPEN_PORT_REQ(CONF_MPL_CREATE_PARTY_REQ) ";
	cstr << " req:" << m_lastReqId;
	cstr << "( Responsibility: embedded )";

	DumpMcuInternalDetailed(cstr,ACK_NOT_RECEIVED);



	CMedString str;

	PTRACEPARTYID(eLevelError,"CH323AddPartyCntl::OnConnectToutCreate, Timer Expired - No Ack from MPL on Create Req", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());
	CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, ACK_NOT_RECEIVED, MAJOR_ERROR_LEVEL, str.GetString(), TRUE);
	m_isFaulty = 1;
	m_isRecovery = 1; // Invoking Recovery process in RA.

	BYTE 	mipHwConn = (BYTE)eMipCardManager;
	BYTE	mipMedia = (BYTE)eMipArt;
	BYTE	mipDirect = eMipNoneDirction;
	BYTE	mipTimerStat = eMipTimer;
	BYTE	mipAction = eMipOpen;

	CSegment* pSeg = new CSegment;
	*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
	DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
	m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
	POBJDELETE(pSeg);

}

/////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnConnectToutPartySetup(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323AddPartyCntl::OnConnectToutPartySetup, Timer Expired - No Rsp from Party", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnConnectToutPartyConnectAudio(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323AddPartyCntl::OnConnectToutPartyConnectAudio, Timer Expired", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());
	m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty,statTout);
}

/////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::OnPartyMuteVideo(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323AddPartyCntl::OnPartyMuteVideo -  Message discarded", GetPartyRsrcId());
}

/////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323AddPartyCntl::IsRemoteAndLocalCapSetHasContent(eToPrint toPrint)const
{
	BYTE res = NO;
	if (m_pRmtCapH323 && m_pLocalCapH323)
	{
		if (m_pRmtCapH323->IsH239() && m_pLocalCapH323->IsH239())
			res = YES;
		else if (m_pRmtCapH323->IsEPC() && m_pLocalCapH323->IsEPC())
			res = YES;
	}
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323AddPartyCntl::IsRemoteAndLocalHasEPCContentOnly()
{
	BYTE res = NO;
	if (m_pRmtCapH323 && m_pLocalCapH323)
	{
		if (!m_pRmtCapH323->IsH239() || !m_pLocalCapH323->IsH239())
			if (m_pRmtCapH323->IsEPC() && m_pLocalCapH323->IsEPC())
				res = YES;

	}
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::EndConnectionProcess(WORD status)
{
	PTRACEPARTYID(eLevelInfoNormal, "CH323AddPartyCntl::EndConnectionProcess", GetPartyRsrcId());
	if (IsValidTimer(MFACONNECTTOUT))
		DeleteTimer(MFACONNECTTOUT);
	if (IsValidTimer(CONNECTTOUT))
		DeleteTimer(CONNECTTOUT);

	DWORD artCapacity = 0;
	eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpCurrentMode);
	artCapacity = CalculateArtCapacityAccordingToScm(m_pIpCurrentMode, TRUE /*add audio + video for current*/);
	if(m_artCapacity != artCapacity)
		 CreateAndSendReAllocateArtForParty(eIP_network_party_type ,eCurrentVideoType, eAllocateAllRequestedResources,FALSE/*ICE*/,artCapacity);

	    m_pTaskApi->EndAddParty(m_pParty, status);
	//for CDR event: PARTICIPANT_DISCONNECT_INFORMATION
	UpdateDetailsForParticipantDisconnectInfoCDREvent(m_pIpCurrentMode);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::OnPartyReceivedReCapsChangeAll(CSegment* pParam)
{
 	PTRACE2PARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnPartyReceivedReCapsChangeAll : Name - ",m_partyConfName, GetPartyRsrcId());
 	PartyReceivedReCapsChangeAll(pParam);
 	m_state = PARTY_SETUP;
}
//////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323AddPartyCntl::OnAddSubLinksParties shiraITP - 20
void  CH323AddPartyCntl::OnAddSubLinksParties(CSegment* pParam)
{
    //PTRACE2(eLevelInfoNormal,"ITP_CASCADE: CH323AddPartyCntl::OnAddSubLinksParties - GetName ",GetName());
    PTRACE2(eLevelInfoNormal,"ITP_CASCADE: CH323AddPartyCntl::OnAddSubLinksParties  - m_name(of mainLink):",m_name);

    if (m_pConf)
    {
        CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());

        if (pConfParty && m_pTaskApi)
            m_pTaskApi->SendAddSubLinksPartiesToConf(pConfParty->GetCascadedLinksNumber(),m_RoomId,m_name);//shiraITP - 21
        else
            PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::OnAddSubLinksParties ERROR - pConfParty or m_pTaskApi is NULL");
    }
    else
    {
        PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::OnAddSubLinksParties ERROR - m_pConf is NULL");
    }
}
///////////////////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::OnTimerWaitForRsrcAndAskAgainAllocate(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323AddPartyCntl::OnTimerWaitForRsrcAndAskAgainAllocate, Name - ",m_partyConfName, GetPartyRsrcId());
	if(m_type == DIALOUT)
	{
		//Temporary ID before we ge a response from the resource allocator
#ifdef LOOKUP_TABLE_DEBUG_TRACE
		TRACEINTO << "D.M. PartyId:" << GetPartyId() << ", monitorPartyId:" << m_monitorPartyId << ", pParty:" << std::hex << m_pParty;
#endif
		m_pParty = (CTaskApp*)(m_monitorPartyId + 100);  // just to get unique id for party list
	}
	StartPartyConnection();
}

/////////////////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::SetH323PartyCapsAndVideoParam(CIpComMode* pPartyScm, CCapH323* pH323Cap, CConfParty* pConfParty, DWORD vidBitrate, WORD dialType, DWORD setupRate, DWORD confRate, BYTE isRecordingLink,PartyControlDataParameters&partyControlDataParams)

{
	PTRACE(eLevelInfoNormal,"CH323AddPartyCntl::SetH323PartyCapsAndVideoParam");
//-------------------------------------------------------------
//In this func. we set following parameters for the H323 party:
//1. local caps
//2. Scm videoMode
//3. video protocol
//	---------------------------------------------------------------
	pPartyScm->SetAudioAlg(FALSE, vidBitrate, pConfParty->GetName());// if the audio algorithm is auto, we set it to the preffered one.

	// in case of audio only
	// we build only audio capabilities
	// (we dont set Scm-videoMode and video protocol)
	if (pConfParty->GetVoice())
	{
		pH323Cap->CreateAudioOnlyCap(vidBitrate, const_cast<CComModeH323*>(pPartyScm), pConfParty->GetName());
		return;
	}

	eVideoQuality vidQuality = partyControlDataParams.vidQuality;
	ePresentationProtocol contentProtocolMode = partyControlDataParams.presentationProtocol;
	BYTE LinkCascadeMode = pConfParty->GetCascadeMode();
	if(partyControlDataParams.bContentMultiResolutionEnabled)
	{
		PTRACE(eLevelInfoNormal,"CConf::SetH323PartyCapsAndVideoParam This is XCode Conf need to change content protocol mode");

		//If Link
		if(LinkCascadeMode != CASCADE_MODE_NONE)
		{
			PTRACE(eLevelInfoNormal,"CConf::SetH323PartyCapsAndVideoParam  this is Link - set to eH264Fix ");
		 	contentProtocolMode = eH264Fix;

		}
		else // Not link!
		{
			if (partyControlDataParams.bContentXCodeH263Supported)
			{
				PTRACE(eLevelInfoNormal,"CConf::SetH323PartyCapsAndVideoParam set to ePresentationAuto ");
				contentProtocolMode = ePresentationAuto;
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CConf::SetH323PartyCapsAndVideoParam set to eH264Dynamic ");
				contentProtocolMode = eH264Dynamic;
			}
		}
	}
	
	pPartyScm->SetContentProtocolMode(contentProtocolMode);

	// TR/CP/Cop conference:
	// Set video mode according to videoProtocol of reservation.
	if (pPartyScm->GetConfType() == kCp || pPartyScm->GetConfType() == kCop)
	{
		ChangeScmOfIpPartyInCpOrCopConf(pPartyScm, pConfParty, pConfParty->GetVideoProtocol(), setupRate, 0,confRate,dialType,pConfParty->GetServiceId(),partyControlDataParams);
		// add (and not replace!!) the scm caps to the local capabilities
	}
	else
	{// vsw / hd conf
		// srt h.239 rate from cosetupRatenf rate
		if(partyControlDataParams.bIsEnableH239)
		{
			// Find the Max content rate of the conf
				/*eEnterpriseMode ContRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
	   			BYTE lConfRate = m_pCommConf->GetConfTransferRate();
		   	DWORD H323AMCRate = m_pUnifiedComMode->GetContentModeAMCInIPRate(lConfRate,ContRatelevel, contentProtocolMode, m_pCommConf->GetCascadeOptimizeResolution());
		   	// Romem
		   	BOOL isHDContent1080Supported = FALSE;
		   	BYTE ConfAMCRate = m_pUnifiedComMode->GetContentModeAMC(lConfRate,ContRatelevel, contentProtocolMode, m_pCommConf->GetCascadeOptimizeResolution());
		   	BYTE newContentProtocol = SelectContentProtocol(FALSE);
		   	BYTE HDResMpi = 0;
		   	isHDContent1080Supported = SelectContentHDResolution(ConfAMCRate,newContentProtocol,HDResMpi);*/
			if (partyControlDataParams.h323MaxContentAMCRate == 0)
			{
				pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			}
			else
			{
				 if (!partyControlDataParams.bIsTIPContentEnable)
				 {
					pPartyScm->SetContent(partyControlDataParams.h323MaxContentAMCRate,cmCapReceiveAndTransmit,partyControlDataParams.contentCap,partyControlDataParams.bIsHDContent1080Supported,partyControlDataParams.hdResMpi,partyControlDataParams.bIsHighProfileContent);
				 }
				 else if (partyControlDataParams.bIsPreferTIP)
				{
					pPartyScm->SetTIPContent(partyControlDataParams.h323MaxContentAMCRate,cmCapReceiveAndTransmit,FALSE);
				}
				 else
				 {
					pPartyScm->SetTIPContent(partyControlDataParams.h323MaxContentAMCRate,cmCapReceiveAndTransmit);
				 }
			}
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CH323AddPartyCntl::SetH323PartyCapsAndVideoParam - H.239 is disabled");
			pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
		}
	}

	// check video rate for changing after ChangeScmOfIpPartyInCpOrCopConf
	PTRACE2INT(eLevelInfoNormal,"CConf::SetH323PartyCapsAndVideoParam, video rate ", pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople));
	DWORD changedVideoRate = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);
	if(changedVideoRate != 0 && vidBitrate > changedVideoRate)
		vidBitrate = changedVideoRate;

	// create with default caps and enable H263 4cif according to video parameters
	BYTE highestframerate= (BYTE)eCopVideoFrameRate_None;
	if(pPartyScm->GetConfType() == kCop)
	{
		CCOPConfigurationList* pCOPConfigurationList = partyControlDataParams.pCOPConfigurationList;
		CCopVideoParams* pCopHighestLevelParams = pCOPConfigurationList->GetVideoMode(0);
		highestframerate= pCopHighestLevelParams->GetFrameRate();
	}
	pH323Cap->CreateWithDefaultVideoCaps(vidBitrate, pPartyScm, pConfParty->GetName(), vidQuality, isRecordingLink,0/*service id*/,((ECopVideoFrameRate)highestframerate ));
	if(pPartyScm->GetConfType() == kCp || pPartyScm->GetConfType() == kCop)
	{
		BYTE protocol = pConfParty->GetVideoProtocol();
		pH323Cap->SetSingleVideoProtocolIfNeeded (protocol);
	}
	Eh264VideoModeType vidMode = GetMaxVideoModeBySysCfg();
	Eh264VideoModeType vidModeAccordingtoRate = eHD1080At60Asymmetric;
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	BYTE partyResolution =  pConfParty->GetMaxResolution();
	BYTE maxConfResolution = partyControlDataParams.maxConfResolution;
	DWORD partyBitrate;
	if (pConfParty->GetVideoRate() == 0xFFFFFFFF)
		partyBitrate = pConfParty->GetVideoRate();
	else
		partyBitrate = pConfParty->GetVideoRate() * 1000;
	DWORD confBitrate = partyControlDataParams.callRate;

	if(confBitrate!=0xFFFFFFFF && partyBitrate != confBitrate && partyResolution == eAuto_Res )
	{
		BYTE maxPartyResolution = ( partyResolution == eAuto_Res && maxConfResolution != eAuto_Res) ?
									maxConfResolution : partyResolution;
		Eh264VideoModeType partyMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxPartyResolution,vidQuality );
        if( partyBitrate != 0xFFFFFFFF )
        	vidModeAccordingtoRate = CResRsrcCalculator::GetVideoMode( GetSystemCardsBasedMode(), partyBitrate, vidQuality, partyMaxVideoMode, FALSE); //FALSE=BaseProfile
        else
            vidModeAccordingtoRate = CResRsrcCalculator::GetVideoMode( GetSystemCardsBasedMode(), confBitrate, vidQuality, partyMaxVideoMode, FALSE); //FALSE=BaseProfile
        TRACEINTO << " CH323AddPartyCntl::SetH323PartyCapsAndVideoParam : partyBitrate = " << partyBitrate << ", confBitrate = " << confBitrate << ", videoMode = " << vidMode;
	}

	if(pPartyScm->GetConfType() == kCp && (vidMode == eCIF30 || (maxConfResolution == eCIF_Res && partyResolution == eAuto_Res) || partyResolution ==  eCIF_Res || vidModeAccordingtoRate == eCIF30) )
	{
		PTRACE(eLevelInfoNormal,"CH323AddPartyCntl::SetH323PartyCapsAndVideoParam -  with cif max resources on slider-remove h263 4cif set");
	    pH323Cap->Set4CifMpi((APIS8)-1);
	    pH323Cap->SetH263FormatMpi(k4Cif, -1, kRolePeople);
	    if(pPartyScm->GetMediaType(cmCapVideo,cmCapReceive) == eH263CapCode)
	    {
	    	pPartyScm->SetFormatMpi (k4Cif, -1, cmCapReceive, kRolePeople);
	    }
	    if(pPartyScm->GetMediaType(cmCapVideo,cmCapTransmit) == eH263CapCode)
	    {
	    	pPartyScm->SetFormatMpi (k4Cif, -1, cmCapTransmit, kRolePeople);
	    }
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::SetMaxRateForIpNetSetup(CConfParty* pConfParty,CIpComMode*pPartyScm, PartyControlInitParameters& partyControlInitParam,CH323NetSetup* pIpNetSetup)
{
	DWORD confRate = 0;
	DWORD vidBitrate;
	int val;
    	val = ::CalculateRateForIpCalls(pConfParty, pPartyScm, confRate, vidBitrate, partyControlInitParam.bIsEncript);
	PASSERT((val == -1));
	vidBitrate = vidBitrate / 100;
	if (pConfParty->GetVideoRate() == 0XFFFFFFFF) {
		pIpNetSetup->SetMaxRate(confRate);
		PTRACE2INT(eLevelInfoNormal, "CH323AddPartyCntl::SetMaxRateForIpNetSetup conf rate -",confRate);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CH323AddPartyCntl::SetMaxRateForIpNetSetup pConfParty->GetVideoRate() * 1000 -",pConfParty->GetVideoRate() * 1000);
		pIpNetSetup->SetMaxRate(pConfParty->GetVideoRate() * 1000);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
CH323NetSetup* CH323AddPartyCntl::CreateAndSetupH323NetSetup(CConfParty* pConfParty,CIpComMode*pPartyScm, PartyControlInitParameters& partyControlInitParam,PartyControlDataParameters &partyControlDataParams)
{
	CH323NetSetup* pIpNetSetup = NULL;
	if (pConfParty->GetConnectionType() == DIAL_OUT)
	{
		char* strConfGUID = GetUniqueGuid(partyControlInitParam.pStrConfGuid,pConfParty);
			
		pIpNetSetup = new CH323NetSetup;
		
		const char* strPartyAlias = pConfParty->GetH323PartyAlias();
		WORD type = pConfParty->GetH323PartyAliasType();
		pIpNetSetup->SetH323PartyAlias(strPartyAlias);
		pIpNetSetup->SetH323PartyAliasType(type);
		pIpNetSetup->SetH323ConfIdAsGUID(strConfGUID);	
		pIpNetSetup->SetMinRate(0);
		pIpNetSetup->SetConfId(partyControlInitParam.monitorConfId);
		if((pConfParty->GetNodeType()) == 0)
		{
			pIpNetSetup->SetEndpointType(2);
		}
		else
		{
			if((pConfParty->GetNodeType()) == 1) {
				pIpNetSetup->SetEndpointType(0);
			}
		}
	}
	else
		pIpNetSetup = (CH323NetSetup*)partyControlDataParams.pIpNetSetup;

	SetMaxRateForIpNetSetup(pConfParty,pPartyScm,partyControlInitParam,pIpNetSetup);

	BOOL rateChanged = FixVideoBitRateIfNeeded( pConfParty, pPartyScm , pIpNetSetup, TRUE);
	if( rateChanged )
	{
		partyControlDataParams.callRate = pPartyScm->GetCallRate()*1000;
	}


	InitDisplayNameForNetSetup(pIpNetSetup);
	SetSeviceIdForConfPartyByConnectionType(pConfParty,pIpNetSetup);
	if (pConfParty->GetConnectionType() == DIAL_OUT)
		InitSetupTaAddr(pIpNetSetup,partyControlInitParam.pStrConfName,pConfParty, YES);

	//Set correlation id in case the string is empty
	if (strlen(pIpNetSetup->GetH323ConfIdAsGUID()) == 0)
	{
		char* strConfGUID = GetUniqueGuid(partyControlInitParam.pStrConfGuid,pConfParty);
		pIpNetSetup->SetH323ConfIdAsGUID(strConfGUID);
	}

	if (pConfParty->GetConnectionType() == DIAL_IN)
		pConfParty->SetCorrelationId(GetGuidFormat(pIpNetSetup->GetCallId()));

	return pIpNetSetup;
}
/////////////////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::SetupH323ConParameters(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams )
{
	//stH323AddPartyCreateParam 	stH323ConnectParams;
	CConf* pConf 							= partyControInitParam.pConf;
	CConfParty* pConfParty 					= partyControInitParam.pConfParty;

	if (pConfParty == NULL)
	{
		PTRACE(eLevelError, "CH323AddPartyCntl::SetupH323ConParameters - pConfParty is NULL");
		DBGPASSERT(1124);
		return;
	}

	const char*avServiceNameStr 				= partyControInitParam.pAvServiceNameStr;
	WORD welcomeMsgTime 					= partyControInitParam.welcomeMsgTime;
	DWORD connectDelay 					= partyControInitParam.connectDelay;
	CIpComMode* pPartyScm 					= NewAndGetPartyCntlScm(partyControInitParam,partyControlDataParams);
	const char* strPartyName;
	BYTE interfaceType;
	BYTE bIsH323;
	WORD bIsAudioOnly;	
	CConfIpParameters* pServiceParams;


	pConfParty->SetIsTipCall(FALSE); //no TIP on H323

	strPartyName 							= pConfParty->GetName();
	interfaceType 							= pConfParty->GetNetInterfaceType();
	bIsH323 								= (interfaceType == H323_INTERFACE_TYPE);
	bIsAudioOnly							= pConfParty->GetVoice();	
	pServiceParams							= GetConfIpParameters(pConfParty);

	if (pServiceParams == NULL)
	{
		PASSERTMSG(1,"CH323AddPartyCntl::SetupH323ConParameters - IP ServiceList is empty, can't configure Default service!!!");
		return;
	}
	PTRACE2INT(eLevelInfoNormal,"CH323AddPartyCntl::SetupH323ConParameters TIP compatible: ", partyControInitParam.bIsTipCompatibleVideo);
	
	CH323NetSetup* pIpNetSetup = CreateAndSetupH323NetSetup(pConfParty,pPartyScm,partyControInitParam,partyControlDataParams);
	WORD  terminalNumber = 0;
	DWORD monitorPartyId = pConfParty->GetPartyId();
	
	// Vnrg 1975
	BYTE isAutoBitRate = 0;
	if (pConfParty->GetVideoRate() == 0XFFFFFFFF)
		ON(isAutoBitRate);
	
	CQoS *pQos = new CQoS(pServiceParams->GetQOS());
	
	CCapH323* pH323Caps = NewAndSetupPartyCaps(partyControInitParam,partyControlDataParams,pPartyScm, pIpNetSetup->GetRemoteSetupRate());

	DWORD confRate = 0;
	DWORD vidBitrate;
	int val;

	val = ::CalculateRateForIpCalls(pConfParty, pPartyScm, confRate, vidBitrate, partyControInitParam.bIsEncript);
	PASSERT((val == -1));
	vidBitrate = vidBitrate / 100;
	partyControInitParam.confRate	= confRate;
	TRACEINTO  << "partyControlDataParams.callRate " << (int)partyControlDataParams.callRate << ", vidBitrate: " << vidBitrate << ", confRate name: " << confRate ;
	
	if (CPObject::IsValidPObjectPtr(m_pH323NetSetup))
		POBJDELETE(m_pH323NetSetup);

	if (CPObject::IsValidPObjectPtr(m_pIpInitialMode))
		POBJDELETE(m_pIpInitialMode);

	if (CPObject::IsValidPObjectPtr(m_pIpCurrentMode))
		POBJDELETE(m_pIpCurrentMode);

	if (CPObject::IsValidPObjectPtr(m_pLocalCapH323))
		POBJDELETE(m_pLocalCapH323);

	if (CPObject::IsValidPObjectPtr(m_pRmtCapH323))
		POBJDELETE(m_pRmtCapH323);

	if (CPObject::IsValidPObjectPtr(m_pQos))
		POBJDELETE(m_pQos);
	
	if (CPObject::IsValidPObjectPtr(m_pTaskApi))
		POBJDELETE(m_pTaskApi);

	PDELETEA(m_name);

	m_pH323NetSetup		= new CH323NetSetup;
	m_pIpInitialMode	= new CComModeH323;
	m_pIpCurrentMode	= new CComModeH323;
	m_pLocalCapH323 	= new CCapH323;
	m_pRmtCapH323		= new CCapH323;
	m_isUndefParty      = pConfParty->IsUndefinedParty();
	*m_pH323NetSetup    = *(CH323NetSetup *)pIpNetSetup;
	*m_pLocalCapH323	= *pH323Caps;
	m_pPartyApi			= NULL;
	m_termNum			= partyControInitParam.termNum;
	m_monitorConfId		= partyControInitParam.monitorConfId;
	m_monitorPartyId	= monitorPartyId;
	m_cascadeMode		= pConfParty->GetCascadeMode();
	m_nodeType			= pConfParty->GetCascadeMode();
	m_voice				= partyControInitParam.voiceType;
	m_IsGateWay			= partyControInitParam.bIsGateWay;
	*m_pIpInitialMode	= *(pPartyScm);
	m_videoRate			= vidBitrate; // 100 bits/sec unit.
	m_pQos				= new CQoS;
	*m_pQos				= *pQos;
	m_sourceId          = 0;
	m_serviceId         = pConfParty->GetServiceId();
	m_subServiceId 		= m_pH323NetSetup->GetSubServiceId();
	
	// m_RoomId      = stH323ConnectParams.roomID;
	//strcpy((char*)m_password,pConfParty->GetPassword());
	SetupConnParameters(partyControInitParam,partyControlDataParams);
	
	POBJDELETE(pH323Caps);
	POBJDELETE(pPartyScm);
	POBJDELETE(pIpNetSetup);
	POBJDELETE(pQos);	

	if (m_videoRate != 0)
	{
		m_pIpInitialMode->SetVideoBitRate(m_videoRate, cmCapReceiveAndTransmit);
		m_pIpInitialMode->SetTotalVideoRate(m_videoRate);
		m_pIpCurrentMode->SetTotalVideoRate(m_videoRate);
	}

	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi(m_pConf->GetRcvMbx(),this);
	m_pTaskApi->SetLocalMbx(m_pConf->GetLocalQueue());

	m_name = new char[H243_NAME_LEN];
	memset(m_name,'\0',H243_NAME_LEN);
	strncpy(m_name,strPartyName,H243_NAME_LEN-1);
	SetFullName(strPartyName, partyControInitParam.pStrConfName);

	SetupTIPLinkInfo(partyControInitParam,partyControlDataParams);
	
	PTRACE2PARTYID(eLevelInfoNormal,"CH323AddPartyCntl::SetupH323ConParameters : Name - ",m_partyConfName, GetPartyRsrcId());
	if (m_cascadeMode == CASCADE_MODE_MASTER)
		PTRACE2INT (eLevelInfoNormal, "CH323AddPartyCntl::SetupH323ConParameters - CASCADE_MODE_MASTER Link:", m_cascadeMode);
	else if (m_cascadeMode == CASCADE_MODE_SLAVE)
		PTRACE2INT (eLevelInfoNormal, "CH323AddPartyCntl::SetupH323ConParameters - CASCADE_MODE_SLAVE Link:", m_cascadeMode);
	else
		PTRACE2INT (eLevelInfoNormal, "CH323AddPartyCntl::SetupH323ConParameters - cascade mode:", m_cascadeMode);

	m_pLocalCapH323->BuildSortedCap();

	m_isAutoVidBitRate = isAutoBitRate;
	m_bLateReleaseOfResources = FALSE;
	m_bContinueChangeModeAfterReAlloc = FALSE;

	
	if (partyControInitParam.bIsRedial)
	{
		ResetRedialParameters();
	}

}

/////////////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::ConnectH323(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{

	CConfParty* pConfParty = partyControInitParam.pConfParty;	
	//decide on according to party tyep
	switch (m_type)
	{
		case DIALIN : 
		{			
			
			if (pConfParty)
			{
				if (m_pH323NetSetup->IsItPrimaryNetwork())
				{
					pConfParty->SetSubServiceMode(ePrimaryIpSubService);
					//pConfParty->SetSubServiceName(SUB_SERVICE_NAME_PRIMARY);
				}
				else
				{
					pConfParty->SetSubServiceMode(eSecondaryIpSubService);
					pConfParty->SetSubServiceName(SUB_SERVICE_NAME_SECONDARY);
				}
			}

			m_pPartyApi = new CPartyApi;
			m_pPartyApi->CreateOnlyApi(*(partyControInitParam.pPartyRcvMbx));
			POBJDELETE(partyControInitParam.pPartyRcvMbx);
//			m_pPartyApi->UpdateVidRateState(m_isAutoVidBitRate);
			StartPartyConnection();
			break;
		}

	    case DIALOUT:
		{
			m_bIsFirstConnectionAfterHotBackupRestore = pConfParty->IsFirstConnectionAfterHotBackupRestore();
			if (! partyControInitParam.standByStart)
			{
				//too many
				DialOut(partyControInitParam,partyControlDataParams);
				m_pTaskApi->UpdateDB(m_pParty,NUMRETRY,(DWORD) m_redial,1);
			}
			else
			{  // connect and stand by
				m_pParty = (CTaskApp*)(m_monitorPartyId + 100);  // just to get unique id for party list
				m_disconnectState = DISCONNECTED;
				m_pTaskApi->EndAddParty(m_pParty,PARTY_STAND_BY);
			}
			break;
		}

		default:
		{
			PTRACEPARTYID(eLevelError,"CH323AddPartyCntl::ConnectH323 : UNKNOWN DIAL TYPE", GetPartyRsrcId());
			break;
		}
	}	

}

///////////////////////////////////////////////////////////////////////
void  CH323AddPartyCntl::SetupH323PartyTaskEntryPoint()
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bIsH323PartyLevelSimulation = 0;
	std::string key = "H323_PARTY_LEVEL_SIMULATION";
	pSysConfig->GetBOOLDataByKey(key, bIsH323PartyLevelSimulation);

	if(bIsH323PartyLevelSimulation)
		m_entryPoint =  CH323DialOutSimulationPartyEntryPoint;// for party control level simulation
	else
		m_entryPoint =  PartyH323OutEntryPoint;
	

}
///////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::SetupCallParameters(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	// in EPC mcuNumber / terminalNumber always 0
	m_McuNumber = (m_pIpInitialMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation)) ? 0 : 1;
	m_termNum = partyControInitParam.termNum;
	m_isRecording = partyControInitParam.pConfParty->GetRecordingLinkParty();
	m_isStreaming = partyControInitParam.bIsStreaming;
	m_isSentH239Out = 0;
	if(partyControInitParam.pStrAppointmentId)
	{
		PDELETEA(m_pExchangeServerConfId);
		m_pExchangeServerConfId  = new BYTE[512];
		strncpy((char *)m_pExchangeServerConfId, partyControInitParam.pStrAppointmentId, 510);
	}

	m_password = partyControInitParam.pConfParty->GetPassword();
	m_confName = partyControInitParam.pStrConfName;
	m_pConfRcvMbx = partyControInitParam.pConfRcvMbx;	
#ifdef LOOKUP_TABLE_DEBUG_TRACE
			TRACEINTO << "D.K. m_pParty:" << std::hex << m_pParty;
#endif
	m_pParty = (CTaskApp*)(m_monitorPartyId + 100);
}

////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::ResetRedialParameters()
{
	m_connectDelayCounter	= 0;
	m_disconnectMode  = 0;
	m_disconnectState = 0;
	m_disconnectionCause = 0;
	m_isDisconnect    = 0;
	m_disconnectDelay = 0;
	m_isWaitingForRsrcAndAskAgain = YES;
	m_isFaulty        = FALSE;
	m_isRecovery      = FALSE;
	
	OFF(m_isFullBitRateConnect);	
	m_isFaulty = 0;
	m_isRecovery = 0;
	OFF(m_presentationStreamOutIsUpdated);
	OFF(m_isPartyInConf);
	OFF(m_isCDRPartyConnected);
	m_eLastReAllocRequestVideoPartyType = eVideo_party_type_dummy;
	m_bLateReleaseOfResources = FALSE;
	m_bContinueChangeModeAfterReAlloc = FALSE;
	m_eLastCopChangeModeParam = COP_decoder_resolution_Last;
	m_eLastCopChangeModeType = eCop_DecoderParams;
	m_lastCopForceEncoderLevel = INVALID_COP_LEVEL;
	m_eFirstRxVideoCapCode = eUnknownAlgorithemCapCode;
	m_eFirstRxVideoProfile = H264_Profile_BaseLine;
	m_copResourceIndexOfCascadeLinkLecturer = INVALID_COP_LEVEL;
	m_bCascadeIsLecturer = FALSE;
	m_lastConnectionRate = 0;
	m_artCapacity  = 0;
	m_conferenceContentRate = 0;
	m_NumOfChangeModesInSec = 0;
	m_pIpCurrentMode->SetAllModesOff();

	// VNGFE-6353 : Reset conditions
	OFF(m_bIsMemberInAudBridge);
	OFF(m_bIsMemberInVidBridge);
	OFF(m_isFeccConn);
	OFF(m_isContentConn);
	OFF(m_isChairConn);
}
//////////////////////////////////////////////////////////////////////
CCapH323* CH323AddPartyCntl::NewAndSetupPartyCaps(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams,CIpComMode* pPartyScm,DWORD setupRate)
{
	CCapH323* pH323Caps = new CCapH323;
	DWORD confRate = 0;
	DWORD vidBitrate;
	CConfParty* pConfParty = partyControInitParam.pConfParty;
	CConfIpParameters* pServiceParams = GetConfIpParameters(pConfParty);
	int val;
    	val = ::CalculateRateForIpCalls(pConfParty, pPartyScm, confRate, vidBitrate, partyControInitParam.bIsEncript);
	PASSERT((val == -1));
	vidBitrate = vidBitrate / 100;
	partyControInitParam.confRate	= confRate;
	
  	WORD confPartyConnectionType = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());
  	DWORD orgConfRate = confRate; //BRIDGE-12263

  	//CQoS *pQos = new CQoS(pServiceParams->GetQOS());
	if (confPartyConnectionType == DIALOUT)
	{
  		SetH323PartyCapsAndVideoParam(pPartyScm, pH323Caps, pConfParty, vidBitrate,confPartyConnectionType, 0, 0, pConfParty->GetRecordingLinkParty(),partyControlDataParams);
	}
	else
	{
		//DWORD setupRate = pIpNetSetup->GetRemoteSetupRate();
		BOOL bIsCpRegardToIncomingSetupRate = 0;
		if ( (pPartyScm->GetConfType() == kCp) && (setupRate != 0) && (setupRate < confRate) )
		{
			CSmallString str;
			str << "CH323AddPartyCntl::NewAndSetupPartyCaps : setup rate is less than the conf rate "
				<< " setup rate = " << setupRate
				<< " conf rate = " << confRate;
			PTRACE (eLevelInfoNormal, str.GetString());

			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			std::string key = "CP_REGARD_TO_INCOMING_SETUP_RATE"; //yael!!!
			pSysConfig->GetBOOLDataByKey(key, bIsCpRegardToIncomingSetupRate);

			if (bIsCpRegardToIncomingSetupRate)	//in order not to allocate too high resources:
			{
				PTRACE(eLevelInfoNormal,"CH323AddPartyCntl::NewAndSetupPartyCaps : Recalculating the rate");
				::ReCalculateRateForIpCpDialInCalls (pConfParty, pPartyScm, partyControlDataParams.networkType, setupRate, confRate, vidBitrate);

				//BRIDGE-12089
				TRACEINTO << " Setting new CallRate = " << (int)vidBitrate;
				pPartyScm->SetCallRate(vidBitrate/1000); //BRIDGE-12263
				partyControlDataParams.callRate = vidBitrate;
				if(partyControlDataParams.pIpNetSetup)
					partyControlDataParams.pIpNetSetup->SetMaxRate(vidBitrate);

				vidBitrate = vidBitrate / 100;
			}
		}

		pPartyScm->SetVideoBitRate(vidBitrate, cmCapReceiveAndTransmit);
		pPartyScm->SetTotalVideoRate(vidBitrate);
		PTRACE2INT(eLevelInfoNormal,"CH323AddPartyCntl::NewAndSetupPartyCaps :  conf rate - ",confRate);
		PTRACE2INT(eLevelInfoNormal,"CH323AddPartyCntl::NewAndSetupPartyCaps :  vidBitrate - ",vidBitrate);
		PTRACE2INT(eLevelInfoNormal,"CH323AddPartyCntl::NewAndSetupPartyCaps :  orgConfRate - ",orgConfRate); //BRIDGE-12263
		
		//WORD confPartyConnectionType = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());
		//CCapH323* pH323Caps = new CCapH323;

		if (bIsCpRegardToIncomingSetupRate)
			SetH323PartyCapsAndVideoParam(pPartyScm, pH323Caps, pConfParty, vidBitrate,confPartyConnectionType, setupRate, orgConfRate,pConfParty->GetRecordingLinkParty(),partyControlDataParams); //BRIDGE-12263 use orgConfRate
		else
			SetH323PartyCapsAndVideoParam(pPartyScm, pH323Caps, pConfParty, vidBitrate,confPartyConnectionType, 0, confRate,pConfParty->GetRecordingLinkParty(),partyControlDataParams);
	}
	return pH323Caps;
}
///////////////////////////////////////////////////////////////////////////////////
void CH323AddPartyCntl::SetupTIPLinkInfo(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	CConfParty* pConfParty = partyControInitParam.pConfParty;
	
	if (!partyControInitParam.bIsRedial)
	{	
		eTelePresencePartyType eTelePresenceMode = (eTelePresencePartyType)pConfParty->GetTelePresenceMode();
		DWORD index = 0;
		m_linkType  	= partyControlDataParams.linkType;
		 if (!IsValidPObjectPtr(m_telepresenseEPInfo))
	    		m_telepresenseEPInfo = new CTelepresenseEPInfo();

		if (IsValidPObjectPtr(m_telepresenseEPInfo))
		{
			m_telepresenseEPInfo->SetNumOfLinks((DWORD)-1);
			m_telepresenseEPInfo->SetLinkNum(index);
		}
		
		 if ( (m_linkType != eRegularParty) || (eTelePresenceMode != eTelePresencePartyNone) )
	    	{
		    	if (m_linkType != eRegularParty)
		    		m_telepresenseEPInfo->SetLinkRole(1);
		    	else
				m_telepresenseEPInfo->SetLinkRole(0);

		        char* pch1 = strrchr(m_name,'_'); // get pointer to the last '_' in string
		        if ( pch1 && (pch1+1) )
		        {
		            index = (DWORD) atoi (pch1+1); // try atoi on the char after '_' in string
		            if (index > 0)
		            	m_telepresenseEPInfo->SetLinkNum(index-1); 
		        }
		        else
		            PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo - pch1 == NULL");
		  }

		if (m_linkType == eMainLinkParty)
		{
		    PTRACE2(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo - eMainLinkParty - create roomCntl!!! - roomID is not updated yet.. m_name:",m_name);

		    m_telepresenseEPInfo->SetNumOfLinks(pConfParty->GetCascadedLinksNumber());
		    m_roomControl = new CRoomCntl(pConfParty->GetCascadedLinksNumber(),0/*stH323ConnectParams.roomID*/,m_pConf);

		    StartTimer(CONNECTLINKTOUT, 30*SECOND);

		    if (index > 0)
		    {
		        BOOL fullyConnected = FALSE;
		        /*if (m_pPartyAllocatedRsrc != NULL)
		        {
		            //we don't need this scope..
		            PTRACE2INT(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::Create, mailbox:",m_pPartyAllocatedRsrc->GetPartyRsrcId());
		            fullyConnected =  m_roomControl->AddLink(m_name,index-1,m_linkType, (DWORD)m_pPartyAllocatedRsrc->GetPartyRsrcId());
		        }
		        else
		        {*/
		            //mailbox is not updated yet..
		            PTRACE2(eLevelError,"ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo, m_pPartyAllocatedRsrc is NULL - don't set mailbox m_name:",m_name);
		            fullyConnected =  m_roomControl->AddLink(m_name,index-1,m_linkType,0);
		        //}
		    }
		    else
		    {
		        PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo : index == 0");
		    }
		}

		if (m_linkType == eSubLinkParty)
		{
			m_telepresenseEPInfo->SetNumOfLinks(pConfParty->GetCascadedLinksNumber());
			m_RoomId      = partyControlDataParams.roomID;
			if (m_pIpInitialMode)
			{
			    m_pIpInitialMode->RemoveContent(cmCapTransmit);
			    m_pIpInitialMode->RemoveContent(cmCapReceive);
			}
			else
			{
			    PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo ERROR - m_pIpInitialMode is NULL - content was not removed");
			    //PASSERTMSG(1,"ITP_CASCADE: CH323AddPartyCntl::Create ERROR - m_pIpInitialMode is NULL - content was not removed");
			}
		}
		m_eTelePresenceMode = eTelePresenceMode;
		m_telepresenseEPInfo->SetEPtype(eTelePresenceMode);
	}
	else
	{
		
		//Multiple links for ITP in cascaded conference feature: ReconnectH323
		DWORD index = 0;
		if (m_telepresenseEPInfo)
		{
			m_telepresenseEPInfo->SetLinkNum(index);
			m_telepresenseEPInfo->SetEPtype(m_eTelePresenceMode);
		}
		if (m_linkType != eRegularParty)
		{
			PTRACE2(eLevelInfoNormal, "ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo - m_name:",m_name);
	
			char* pch1 = strrchr(m_name,'_');
			if (pch1 && (pch1+1))
			{
				index = (DWORD) atoi (pch1+1);
			}
			else
				PTRACE(eLevelInfoNormal, "ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo pch1 == NULL");
			if (m_telepresenseEPInfo)
			{
				m_telepresenseEPInfo->SetLinkRole(1);
				m_telepresenseEPInfo->SetLinkNum((index>0)? index-1 : index);
			}
			else
				PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo - ERROR m_telepresenseEPInfo is NULL");
	
		}
		else
		{
			if (m_telepresenseEPInfo)
				m_telepresenseEPInfo->SetLinkRole(0);
			else
				PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo - ERROR for eRegularParty m_telepresenseEPInfo is NULL");
		}
	
		if (m_linkType == eMainLinkParty)
		{
			PTRACE2(eLevelInfoNormal, "ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo - eMainLinkParty - create roomCntl!!! - roomID is not updated yet.. m_name:",m_name);
			if(pConfParty)
				m_roomControl = new CRoomCntl(pConfParty->GetCascadedLinksNumber(),0/*stH323ConnectParams.roomID*/,m_pConf);
			else
				PASSERT(1);
	
			StartTimer(CONNECTLINKTOUT, 30*SECOND);
	
			if (index > 0)
			{
				BOOL fullyConnected;
				if (m_pPartyAllocatedRsrc != NULL)
				{
					//PTRACE2INT(eLevelInfoNormal, "CH323AddPartyCntl::ReconnectH323 mailbox-",m_pPartyAllocatedRsrc->GetPartyRsrcId());
					fullyConnected =  m_roomControl->AddLink(m_name,index-1,m_linkType, (DWORD)m_pPartyAllocatedRsrc->GetPartyRsrcId());
				}
				else
				{
					PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo m_pPartyAllocatedRsrc is NULL - don't set mailbox");
					fullyConnected =  m_roomControl->AddLink(m_name,index-1,m_linkType,0);
				}
			}
			SetFullName(m_name, partyControInitParam.pStrConfName);
		}
		if (m_linkType == eSubLinkParty)
		{
			if (m_pIpInitialMode)
			{
				m_pIpInitialMode->RemoveContent(cmCapTransmit);
				m_pIpInitialMode->RemoveContent(cmCapReceive);
			}
			else
			{
				PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo ERROR - m_pIpInitialMode is NULL - content was not removed");
				//PASSERTMSG(1,"ITP_CASCADE: CH323AddPartyCntl::ReconnectH323 ERROR - m_pIpInitialMode is NULL - content was not removed");
			}
		}
	
		if (m_telepresenseEPInfo == NULL)
		{
			PTRACE(eLevelError, "ITP_CASCADE: CH323AddPartyCntl::SetupTIPLinkInfo ERROR - m_pIpInitialMode is NULL");
		}
	}
}


