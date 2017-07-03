/*
 * MSSlaveSipPartyCntl.cpp
 *
 *  Created on: Sep 23 , 2013
 *      Author: Flora Yao
 */
#if 0
#include "SIPInternals.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SipNetSetup.h"
#include "ConfPartyOpcodes.h"
#include "ConfDef.h"
#include "Conf.h"
#include "ConfApi.h"
#include "CommModeInfo.h"
#include "Capabilities.h"
#include "TaskApi.h"
#include "TaskApp.h"
#include "PartyApi.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "BridgeMoveParams.h"
#include "SIPPartyControl.h"
#endif
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "StateMachine.h"
#include "Segment.h"
#include "NStream.h"
#include "ConfPartyDefines.h"
#include "ConfPartyProcess.h"
//#include "IpCommonTypes.h"
#include "IpCommonDefinitions.h"
#include "IpAddressDefinitions.h"
#include "IpScm.h"
#include "ConfPartyOpcodes.h"
#include "ConfDef.h"
#include "Conf.h"
#include "ConfApi.h"
#include "CommModeInfo.h"
#include "Capabilities.h"
#include "TaskApi.h"
#include "TaskApp.h"
#include "PartyApi.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SipCall.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "IpPartyControl.h"
#include "SipNetSetup.h"
#include "SIPCommon.h"
#include "SIPPartyControl.h"
#include "MSSlaveSipPartyCntlAdd.h"
#include "PartyApi.h"
#include "PartyRsrcDesc.h"
#include "BridgeMoveParams.h"
#include "ApiStatuses.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include  "ConfPartyGlobals.h"
#include  "H263VideoMode.h"
#include "WrappersResource.h"  //to be removed noa
#include "ServiceConfigList.h"
#include "IpServiceListManager.h"
#include "SIPInternals.h"
#include "AudioBridgeInterface.h"
#include "ScpHandler.h"
#include "CDRUtils.h"

#define MFA_CONNECT_TIME	20
#define CONNECT_TIME		100 // all internal MCU units receive 5 seconds to reply, and the network has 50 second -> 5+5+50+5+5 <= 100

extern "C" void MSSlaveSipPartyEntryPoint(void* appParam);
//extern "C" void SipPartyOutCreateEntryPoint(void* appParam);
extern const char* GetStatusAsString(int status);
extern CIpServiceListManager* GetIpServiceListMngr();


////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CMSSlaveSipAddPartyCntl)
//almost done
/* SlaveIn & SlaveOut */
	ONEVENT(ALLOCATE_PARTY_RSRC_IND,		ALLOCATE_RSC,				CMSSlaveSipAddPartyCntl::OnPartyRsrcAllocatingRsp)
	ONEVENT(ACK_IND,						CREATE_MPL,					CMSSlaveSipAddPartyCntl::OnPartyMPLCreatingRsp)
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,		PARTY_SETUP,				CMSSlaveSipAddPartyCntl::OnPartyChannelsConnectedSetup)
/* MSSlave Flora Question & comment: At first, i refered the handing of TipSlavePartyCntl, As i understand here: */
/* It is possible to receive the :PARTY_VIDEO_IVR_MODE_CONNECTED or PARTY_VIDEO_CONNECTED in first */
	/* So we need to handle both of these two msg OPCODE. */
/* But my question here is : we should handle these two msg only on state of CONNECT_BRIDGES, but not on state ANYCASE,  just as CSipAddPartyCntl did. */
	/* otherwise SlaveParty Cntl will send  msg of : VIDBRDGCONNECT to SlaveParty twice. */
/* But i think it is better to receive only the msg : PARTY_VIDEO_CONNECTED for OpenVideoBridge for MSSlaveParty here we need to sync up with Video Bridge */
//noa -I agree about the state -we can verify that the salve will not get PARTY_VIDEO_IVR_MODE_CONNECTED -ask Ron or Anat in order to make sure
/* Since it is  the same handing with the CSipAddPartyCntl for CMSSlaveSipAddPartyCntl, do not overload ONEVENT these two msg here */
#if 1
	ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	CONNECT_BRIDGES,			CMSSlaveSipAddPartyCntl::OnVideoBrdgConnected)
	ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	ANYCASE,					CMSSlaveSipAddPartyCntl::OnVideoBrdgConnected)
	ONEVENT(PARTY_VIDEO_CONNECTED,			CONNECT_BRIDGES,			CMSSlaveSipAddPartyCntl::OnVideoBrdgConnected)
	ONEVENT(PARTY_VIDEO_CONNECTED,			ANYCASE,					CMSSlaveSipAddPartyCntl::OnVideoBrdgConnected)
#endif 
/* MSSlave Flora comment: do nothing if we received this msg on ANYCASE for MSSlaveIn/out here, overload it */
	ONEVENT(PARTY_AUDIO_CONNECTED,			ANYCASE,					CMSSlaveSipAddPartyCntl::OnAudConnectPartyConnectAudio)
	//ONEVENT(PARTY_AUDIO_CONNECTED,			PARTY_IN_CONNECTING_STATE,	CMSSlaveSipAddPartyCntl::OnAudConnectPartyConnectAudio)
	ONEVENT(IPPARTYCONNECTED,				CONNECT_BRIDGES,			CMSSlaveSipAddPartyCntl::OnPartyRemoteConnected)
/* Flora comment: for MSSlaveOut, the video out channel is connected in this state */
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,		PARTY_IN_CONNECTING_STATE,	CMSSlaveSipAddPartyCntl::OnPartyChannelsConnectedConnecting)
/* Flora Question: I refered Tip code below, but i can not understand here, i think the we can receive this SIP_PARTY_CHANS_CONNECTED for OutSlave(VideoChanOut) only on state : PARTY_IN_CONNECTING_STATE */
#if 0
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,		CONNECT_BRIDGES,			CMSSlaveSipAddPartyCntl::OnPartyChannelsConnectedConnecting)
#endif
	ONEVENT(CONNECTDELAY,					IDLE,				CMSSlaveSipAddPartyCntl::OnTimerConnectDelay)
	ONEVENT(CONNECTDELAY,					ALLOCATE_RSC,			CMSSlaveSipAddPartyCntl::OnTimerConnectDelay)
//TBD.
PEND_MESSAGE_MAP(CMSSlaveSipAddPartyCntl,CSipAddPartyCntl);


////////////////////////////////////////////////////////////////////////////////
CMSSlaveSipAddPartyCntl::CMSSlaveSipAddPartyCntl()
{
	m_AvMcuLinkType = eAvMcuLinkNone;
	m_RoomId = 0xFFFF;
	m_SentAckToMaster = FALSE;
	m_MSSlaveIndex = 0;
	m_MSaudioLocalMsi = 0;
}

////////////////////////////////////////////////////////////////////////////////
CMSSlaveSipAddPartyCntl::~CMSSlaveSipAddPartyCntl()
{
}

void CMSSlaveSipAddPartyCntl::OnTimerConnectDelay(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::OnTimerConnectDelay", GetPartyRsrcId());
	
	AllocatePartyResources();
	return;
}

////////////////////////////////////////////////////////////////////////////////
void CMSSlaveSipAddPartyCntl::Create(PartyControlInitParameters&  partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	SetupMSSlaveSlaveSipConParameters(partyControInitParam,partyControlDataParams);
	StartPartyConnection();
}

/////////////////////////////////////////////////////////////////////////////
void  CMSSlaveSipAddPartyCntl::StartPartyConnection()
{
	m_connectingState = IP_CONNECTING;

	StartTimer(MFACONNECTTOUT, MFA_CONNECT_TIME*SECOND);
	AllocatePartyResources();
}

/////////////////////////////////////////////////////////////////////////////
//Send CH323AddPartyCntl event to resource allocator process and changes state to ALLOCATE.
void CMSSlaveSipAddPartyCntl::AllocatePartyResources()
{
	PTRACEPARTYID(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::AllocatePartyResources", GetPartyRsrcId());
	EAllocationPolicy bIsFreeVideoResources = eAllocateAllRequestedResources;

	BYTE portGaugeThresholdExceeded = FALSE;

	/* MSSlave Flora Question: How to set up SlaveTeleInfo for MSSlaveIn & MSSlaveOut */
	//noa-save it as non telepresence no need to call this function just to set teleprense none
	//SetSlaveTelepresenseEPInfo(); //_e_m_
	/* this way? */
	m_telepresenseEPInfo->SetEPtype(eTelePresencePartyNone);
	
	/* get m_VideoPartyType from Conf while creating  CMSSlaveSipAddPartyCntl */
	/* MSSlave Flora Question: which params do we need to send to Rsrc? */
	/* do we need to pass param: 1: MSSlave Direction 2: MSSlave Type to Rsrc? */
	//noa- no we calcaulate m_VideoPartyType according to scm we create .
	/*
	from Anat Mail: 
	Regarding the VideoPartyType - Noa explained that we need to use m_maxResolution and not VideoPartyType:
	The flow should be:
	According to pConfParty->GetMaxResolution() the new SCM will be built inside the slave.
	According to SCM, party is calculating m_VideoPartyType.
	This is the generic dial out flow and we want to be similar to it. 
	When creating the slaves, we need to put the right max resolution inside slave ConfParty (m_maxResolution).
	This is the enum:
	
	typedef enum
	{
	  eAuto_Res,
	  eCIF_Res,
	  eSD_Res,
	  eHD720_Res,
	  eHD1080_Res,
	  eHD1080p60_Res
	} EVideoResolutionType;
	*/
	eVideoPartyType localVideoPartyType = eVideo_party_type_none;
	/* MSSlave Flora comment: Calculate the localVideoPartyType  according to scm, just do the same with SipMainParty */
	localVideoPartyType = GetLocalVideoPartyType();
	eVideoPartyType rtvVideoPartyType = GetVideoPartyTypeForRtvBframe();
	localVideoPartyType = max(localVideoPartyType, rtvVideoPartyType);
	
	CreateAndSendAllocatePartyResources( eIP_network_party_type, localVideoPartyType, bIsFreeVideoResources,
			                             portGaugeThresholdExceeded,FALSE,0/*art capacit*/ ,m_TipPartyType, m_RoomId);
	m_state = ALLOCATE_RSC;
}


//_e_m_
////////////////////////////////////////////////////////////////////////////////
DWORD CMSSlaveSipAddPartyCntl::UpdateCapsWithMsftResources(MS_SSRC_PARTY_IND_PARAMS_S  &msSsrcParams)
{
	if (!m_isMsftEnv || IsFeatureSupportedBySystem(eFeatureMs2013SVC) == FALSE )
	{
//		PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::UpdateScmWithMsftResources Shmulik DBG m_isMsftEnv is FALSE: ");
		return STATUS_OK;
	}
	
	DWORD ssrc = msSsrcParams.m_msSsrcFirst;
	PTRACE2INT(eLevelInfoNormal,"CSipAddPartyCntl::UpdateScmWithMsftResources ssrc audio=: ", ssrc);

	m_pSipLocalCaps->setMsftSsrcVideo(ssrc, ssrc+MSFT_SVC_MAX_VIDEO_SSRC_FOR_SLAVE-1, 0);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
void CMSSlaveSipAddPartyCntl::OnPartyRsrcAllocatingRsp(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::OnPartyRsrcAllocatingRsp", GetPartyRsrcId());

	CPartyRsrcDesc tempPartyAllocatedRsrc;
	tempPartyAllocatedRsrc.DeSerialize(SERIALEMBD, *pParam);

	if (m_pPartyHWInterface)
		POBJDELETE(m_pPartyHWInterface);

	DWORD status = tempPartyAllocatedRsrc.GetStatus();
	eVideoPartyType allocatedVideoPartyType = tempPartyAllocatedRsrc.GetVideoPartyType();

	if (status != STATUS_OK)
	{
		DeleteTimer(MFACONNECTTOUT);
		char buf[300];
		sprintf(buf,"CMSSlaveSipAddPartyCntl::OnPartyRsrcAllocatingRsp: STATUS ( %d ), m_type ( %d )",status, m_type);
	    PTRACEPARTYID(eLevelInfoNormal, buf, GetPartyRsrcId());
	    if( STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN == status)
	    {
	    	///Resource Allocator tries to fulfill the allocation request by reconfigurating the DSP, we will set a timer and retry to allocate the resources
	    	if(m_isWaitingForRsrcAndAskAgain)
	    	{
	    		WaitForRsrcAndAskAgain();
	    	  	return;
	    	}
	    	else
	    	{
	    		///We only retry to allocate resources in case of STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN just once
	    	    PTRACE2PARTYID(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::OnPartyRsrcAllocatingRsp: STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN but the m_isWaitingForRsrcAndAskAgain is set to NO!!!!! party: ", m_partyConfName, GetPartyRsrcId());
	    	    PASSERT(1);
	    	}
	    }

		/* MSSlave Flora Question: do we need to lower video resource? and lower to which one ? */
		/* In What situations, we need to lower video resource ? */
		//noa-no need to lower
#if 0
		if( m_VideoPartyType == eVideo_party_type_none && allocatedVideoPartyType != eVideo_party_type_none && status == STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED )
		{
			/* MSSlave Flora Question: In my opinion, for MSSlave, we can not downgrad to AudioOnly */
			PTRACE(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::OnPartyRsrcAllocatingRsp  : no audio ports saving video resource -reallocating to lowest video resource possible ");
			tempPartyAllocatedRsrc.DumpToTrace();
			CreateAndSendAllocatePartyResources( eIP_network_party_type, eCP_H264_upto_CIF_video_party_type, eAllowDowngradingToAudioOnly,FALSE,FALSE,0/*art cap*/ ,m_TipPartyType, m_RoomId);
			return;
		}
#endif
		if (( (STATUS_INSUFFICIENT_VIDEO_RSRC==status) || (STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED==status) ) &&
				(m_type==DIALOUT) && (m_connectDelayCounter < NUM_OF_CONNECT_DELAY_RETRIES))
		{
			PTRACEPARTYID(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::OnPartyRsrcAllocatingRsp :Temporary Video Resources Deficiency Connection Delay ", GetPartyRsrcId());
//			char buf[300];
//			sprintf(buf,"CH323AddPartyCntl::OnRsrcAllocatePartyRspAllocate: Connect Delay ( %d ) ,ConnectDelayTime (%d)",m_connectDelayCounter+1,m_connectDelay);
//	    	PTRACEPARTYID(eLevelInfoNormal, buf);
			//Invoke Connect Delay to allow the system DeAllocation of resources that reserved by a Party in SD
			//Conf that at last need less Resources.
			//m_state=IDLE;
			COsQueue ConfRcvMbx = m_pConf->GetRcvMbx();
			m_pConfRcvMbx = &ConfRcvMbx;
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
			PTRACE2PARTYID(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::OnPartyRsrcAllocatingRsp : ALLOCATION FAILED!!! do not continue process : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());
			CSegment *pSeg = new CSegment;
			*pSeg << m_name;
			m_pTaskApi->UpdateDB(NULL,DISCAUSE,RESOURCES_DEFICIENCY,1,pSeg); // Disconnnect cause
			m_pTaskApi->EndAddParty(m_pParty,statIllegal);
			m_pTaskApi->SendMsSlaveToMainAck(m_MasterRsrcId, ADDMSSLAVEPARTY, statIllegal, DEFAULT_PARTY_ID, m_AvMcuLinkType, m_MSSlaveIndex);
		}

		tempPartyAllocatedRsrc.DumpToTrace();
		/* MSSlave Flora Question: 1: do we need to use the same msg: PARTY_CONTROL_SLAVE_TO_MASTER_ACK with the Tip, */
		/* if we do, we need to add params for MSSlave, at least MsSlaveType and MsSlaveIndex ?*/
		//noa-please add the new opcode as you decided with Anat
		/* Flora Answer: Anat will provide another API to send ACK to Main , change it later */
		return;
	}

	eNetworkPartyType networkPartyType =  tempPartyAllocatedRsrc.GetNetworkPartyType();
	if(networkPartyType!=eIP_network_party_type)
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::OnPartyRsrcAllocatingRsp ALLOCATION FAILED!!! do not continue process eNetworkPartyType!= eIP_network_party_type, eNetworkPartyType = ",eNetworkPartyTypeNames[networkPartyType], GetPartyRsrcId());
		PASSERT(1);
		CSegment *pSeg = new CSegment;
		*pSeg << m_name;
		m_pTaskApi->UpdateDB(NULL,DISCAUSE,RESOURCES_DEFICIENCY,1,pSeg); // Disconnnect cause
		m_pTaskApi->EndAddParty(m_pParty,statIllegal);
		tempPartyAllocatedRsrc.DumpToTrace();

		/* MSSlave Flora Question: do we need to get another interface to send different OPCODE: PARTY_CONTROL_SLAVE_TO_MASTER_ACK from MSSlave to MSMaster ? */
		//noa-yes
		m_pTaskApi->SendMsSlaveToMainAck(m_MasterRsrcId, ADDMSSLAVEPARTY, statIllegal, DEFAULT_PARTY_ID, m_AvMcuLinkType, m_MSSlaveIndex);
		return;
	}

	//CheckMSSlaveResourceAllocationTowardsRequest();
	eVideoPartyType videoPartyType = tempPartyAllocatedRsrc.GetVideoPartyType();
	eVideoPartyType ReqVideoPartyType = GetLocalVideoPartyType();
	eVideoPartyType rtvVideoPartyType = GetVideoPartyTypeForRtvBframe();
	ReqVideoPartyType = max(ReqVideoPartyType, rtvVideoPartyType);
	/* Flora comment: if the allocated rsrc is lower than requested(downgraded), handle it like failure */
	if(videoPartyType < ReqVideoPartyType /* what MS Slave asked in this Allocate Req */)
	{
		PTRACE2PARTYID(eLevelInfoNormal, "SipAddPartyCntl::OnPartyRsrcAllocatingRsp TIP ALLOCATION FAILED!!! do not continue process , VideoPartyType = ",eVideoPartyTypeNames[videoPartyType], GetPartyRsrcId());
		PASSERT(1);
		CSegment *pSeg = new CSegment;
		*pSeg << m_name;
		m_pTaskApi->UpdateDB(NULL,DISCAUSE,RESOURCES_DEFICIENCY,1,pSeg); // Disconnnect cause
		m_pTaskApi->EndAddParty(m_pParty,statIllegal);
#if 0
		tempPartyAllocatedRsrc.DumpToTrace();

		BYTE 	mipHwConn = (BYTE)eMipNoneHw;
		BYTE	mipMedia = (BYTE)eMipNoneMedia;
		BYTE	mipDirect = eMipNoneDirction;
		BYTE	mipTimerStat = eMpiNoTimerAndStatus;
		BYTE	mipAction = eMipNoAction;

		CSegment* pSeg = new CSegment;
		*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
#endif
		m_pTaskApi->SendMsSlaveToMainAck(m_MasterRsrcId, ADDMSSLAVEPARTY, statIllegal, DEFAULT_PARTY_ID, m_AvMcuLinkType, m_MSSlaveIndex);

		return;
	}


	PTRACEPARTYID(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::OnPartyRsrcAllocatingRsp : Allocation is OK", GetPartyRsrcId());
	m_pPartyAllocatedRsrc = new CPartyRsrcDesc(tempPartyAllocatedRsrc);

	pParam->Get((BYTE*)(&m_udpAddresses), sizeof(UdpAddresses));

	m_pPartyAllocatedRsrc->DumpToTrace();

	
    ISDN_PARTY_IND_PARAMS_S tempIsdnParams;
    SVC_PARTY_IND_PARAMS_S  svcParams;
    MS_SSRC_PARTY_IND_PARAMS_S    msSsrcParams;
	pParam->Get((BYTE*)(&tempIsdnParams), sizeof(ISDN_PARTY_IND_PARAMS_S));
	pParam->Get((BYTE*)(&svcParams), sizeof(SVC_PARTY_IND_PARAMS_S));
	pParam->Get((BYTE*)(&msSsrcParams), sizeof(MS_SSRC_PARTY_IND_PARAMS_S));
	
	/* MSSlave Flora Comment: for Lync2013 MainParty, it get the ssrc from the allocat rsp  */
	/* In my opinion: if it is MSSlave out, get the ssrc from the MainParty, what if it is MSSlave in?  */
	/* pParam->Get((BYTE*)(&msSsrcParams), sizeof(MS_SSRC_PARTY_IND_PARAMS_S)); */	
	
	/* MSSlave Flora Question: do we need this svcParams for MSSlave out & In?	*/
//	UpdateScmWithResources(svcParams,allocatedVideoPartyType);
//	UpdateCapsWithMsftResources(msSsrcParams);
	
	if (IsMsSlaveOut())
	{
		msSsrcParams.m_msSsrcFirst = GetMSSlaveSsrcRangeStart();
		msSsrcParams.m_msSsrcLast = msSsrcParams.m_msSsrcFirst + MSFT_SVC_MAX_VIDEO_SSRC_FOR_SLAVE;

	  //	UpdateCapsWithMsftResources(msSsrcParams);

		m_pSipLocalCaps->setMsftSsrcVideo(msSsrcParams.m_msSsrcFirst, msSsrcParams.m_msSsrcFirst+MSFT_SVC_MAX_VIDEO_SSRC_FOR_SLAVE-1, 1);
	}

	//Flora comment 1127: what if it is SlaveIn, where to get the ssrc-range ? or do we need to set the ssrcRange? 
	
	m_MaxLocalCaps = new  CSipCaps;  //for DPA save max caps and scm before changint it according to alloc
	*m_MaxLocalCaps = *m_pSipLocalCaps;
	

	//CheckResourceAllocationTowardsRequest(1);
	m_eLastAllocatedVideoPartyType = m_pPartyAllocatedRsrc->GetVideoPartyType();
	
	m_RoomId = m_pPartyAllocatedRsrc->GetRoomId();
		TRACEINTO << "RoomId: " << m_RoomId << ", LastAllocatedVideoPartyType: " << (int)m_eLastAllocatedVideoPartyType;

	COsQueue ConfRcvMbx = m_pConf->GetRcvMbx();
	m_confName = m_pConf->GetName();
	// in dial out Now really allocate the party task
	TRACEINTO << "Conf Name: " << m_confName << ", LastAllocatedVideoPartyType: " << (int)m_eLastAllocatedVideoPartyType;

	m_pPartyApi = new CPartyApi;
	m_pPartyApi->Create(m_entryPoint,
	                    ConfRcvMbx,
	                    ConfRcvMbx,
	                    m_pPartyAllocatedRsrc->GetPartyRsrcId(),
	                    m_monitorPartyId,
	                    m_monitorConfId,
	                    "",
	                    m_name,
	                    m_confName,
	                    m_serviceId,
	                    m_termNum,
	                    m_McuNumber,
	                    "",
	                    m_voice,
	                    1,
	                    m_IsGateWay);

	m_pParty = m_pPartyApi->GetTaskAppPtr();

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.K."
			<< "  PartyName:"        << m_name
			<< ", PartyId:"          << m_pParty->GetPartyId() << "(" << m_pPartyAllocatedRsrc->GetPartyRsrcId() << ")"
			<< ", IsDialOut:"        << m_type
			<< ", Pointer:"          << std::hex << m_pParty;
#endif

	InsertPartyResourcesToGlobalRsrcRoutingTbl();

	//we can now send msgs to hw
	m_pPartyHWInterface = new CPartyInterface(m_pPartyAllocatedRsrc->GetPartyRsrcId(), m_pPartyAllocatedRsrc->GetConfRsrcId());
	m_pPartyHWInterface->SetRoomId(m_RoomId);
	SendCreateParty(E_NETWORK_TYPE_IP);
	m_state = CREATE_MPL;

	/* MSSlave Flora Question: i copyed it from SIPSlavePartyControl for Tip and SIPPartyControlAdd, but i think for MSSlaveAdd, we do not need it any more ? */
	//noa-not sure what it deos didn't fully understand can you please elobarte?
#if 0
	// in dial in we use it to send ringing
	CRsrcParams* pCsRsrcParams = new CRsrcParams;
	*pCsRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_signaling);
	m_pPartyApi->SendCsResourceAllocated(pCsRsrcParams);
	POBJDELETE(pCsRsrcParams);
#endif
}

////////////////////////////////////////////////////////////////////////////////
/* MSSlave Flora comment: since we have different handing with Master Sip Party: SendAckFromSlaveToMaster, so we need to reload this interface */
void  CMSSlaveSipAddPartyCntl::OnPartyMPLCreatingRsp(CSegment* pParam)
{
	OPCODE ackOpcode;
	DWORD  ackSeqNum;
	STATUS status;
	*pParam >> ackOpcode >> ackSeqNum >> status;
	CMedString str;

	DeleteTimer(MFACONNECTTOUT);

	if (ackOpcode == CONF_MPL_CREATE_PARTY_REQ)
	{
		if (status == STATUS_OK)
		{
			PTRACE2INT(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::OnPartyMPLCreatingRsp : CreateParty Ack Msg Received - ", m_disconnectionCause);
			BYTE eTransportType = eTransportTypeTls;
			EstablishCall(eTransportType);
		}
		else
		{
			CLargeString cstr;
			cstr << "Party:" << GetPartyRsrcId() << " Conf:" << GetConfRsrcId();
			cstr << " receives Failure Status for opcode: TB_MSG_OPEN_PORT_REQ(CONF_MPL_CREATE_PARTY_REQ) ";
			cstr << " req:" << m_lastReqId;
			cstr << "( Responsibility: embedded )";

			DumpMcuInternalDetailed(cstr,ACK_FAILED);

			PTRACEPARTYID(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::OnPartyMPLCreatingRsp : ACK with fail : ", GetPartyRsrcId());
			str << "CMSSlaveSipAddPartyCntl::OnMplAckCreate : create Party failed, Party Rsrc ID - " << GetPartyRsrcId();
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
			DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
			m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);

			m_pTaskApi->SendMsSlaveToMainAck(m_MasterRsrcId, ADDMSSLAVEPARTY, statIllegal, DEFAULT_PARTY_ID, m_AvMcuLinkType, m_MSSlaveIndex);
			POBJDELETE(pSeg);

		}
	}
	else
	{
		str << "CMSSlaveSipAddPartyCntl::OnMplAckCreate : Invalid Opcode, Response Opcode - " << ackOpcode;
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, INVALID_OPCODE_RESPONSE, MAJOR_ERROR_LEVEL, str.GetString(), TRUE);
		PTRACE2INT(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::OnPartyMPLCreatingRsp : unexpected opcode %d", ackOpcode);
	}
}

////////////////////////////////////////////////////////////////////////////////
/* MSSlave Flora comment: It is better to reloading this interface, cause it is more simple than the master sip partyCntl */
void CMSSlaveSipAddPartyCntl::EstablishCall(BYTE eTransportType)
{
	PTRACEPARTYID(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::EstablishCall", GetPartyRsrcId());
	
	CMedString msg1;
	msg1 << "party Index:" << m_MSSlaveIndex << ", MSSlave Party Type:" << eAvMcuLinkTypeNames[m_AvMcuLinkType] << ", Ssrc Range:" << m_MSSlaveSsrcRangeStart;
	PTRACE2(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::EstablishCall - ", msg1.GetString());

	m_state = PARTY_SETUP;

	StartTimer(CONNECTTOUT, CONNECT_TIME*SECOND);

	const char* alternativeAddrStr;

    alternativeAddrStr = NULL;

	CRsrcParams* pRtpRsrcParams = new CRsrcParams;
	*pRtpRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_rtp);
	CRsrcParams* pCsRsrcParams = new CRsrcParams;
	*pCsRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_signaling);
	BYTE  bIsIpOnlyConf = TRUE;

	WORD refMcuNumber = 1;
	WORD refTerminalNumber = 1;
	if (m_pTerminalNumberingManager)
   	{
   		m_pTerminalNumberingManager->allocatePartyNumber(m_pParty, refMcuNumber, refTerminalNumber);
   		ON(m_isTerminalNumberingConn);
   	}
	else
		PASSERTMSG(GetPartyRsrcId(),"Terminal numbering manager not valid");

	
	m_pIpInitialMode->Dump("Flora degug 1:CMSSlaveSipAddPartyCntl::EstablishCall - m_pIpInitialMode is: ",eLevelInfoNormal);
	/* MSSlave Flora Question: we need another PartyApi for MSSlave different with this api for TipSlave ?*/
	/* I think it is better to get a new api for MSSlave, cause the params sent to MSSlaveParty in the msg:*/
	/* SIP_CONF_ESTABLISH_CALL is different with TIPSlaveParty */
	/* CPartyApi::SipConfEstablishSlaveCall */
	//noa-agreed
	m_pPartyApi->SipConfEstablishMSSlaveCall(pRtpRsrcParams, pCsRsrcParams, m_udpAddresses,(CSipNetSetup *) m_pSIPNetSetup,
										m_pSipLocalCaps, m_pIpInitialMode, m_eConfTypeForAdvancedVideoFeatures,
										m_strConfParamInfo.GetString(), eTransportType, refMcuNumber, refTerminalNumber,
										m_bNoVideRsrcForVideoParty, m_RoomId, m_AvMcuLinkType,m_MSSlaveIndex, m_pSipRemoteCaps,
										m_MaxLocalCaps/*,m_pTargetModeMaxAllocation*/);

	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTING);

	POBJDELETE(pRtpRsrcParams);
	POBJDELETE(pCsRsrcParams);
}

/////////////////////////////////////////////////////////////////////////////
/* MSSlave Flora comment: for MSSlaveOut: Received the dummy SIP_PARTY_CHANS_CONNECTED msg from MSSlaveOutParty, */
/* dummy Video Receive Channel connected  */
/* MSSlave Flora Question: maybe we do not need to overloading it with CSipAddPartyCntl::OnPartyChannelsConnectedSetup */
/* It is almost the same with it, except in the CSipAddPartyCntl, there are more handling not MSSLave related, of course in this overloading function  */
/* Only the key code for MSSLavePartyAddCntl left here */
//-noa-didn't underand the question..let's go over it
void CMSSlaveSipAddPartyCntl::OnPartyChannelsConnectedSetup(CSegment* pParam) // channels connected or updated
{
	PTRACEPARTYID(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::OnPartyChannelsConnectedSetup", GetPartyRsrcId());
	m_pIpCurrentMode->DeSerialize(NATIVE,*pParam);

	m_pIpCurrentMode->Dump("Flora degug 4:CMSSlaveSipAddPartyCntl::OnPartyChannelsConnectedSetup - m_pCurrentMode is: ",eLevelInfoNormal);
	/*  MSSlave-Flora Yao-2013/10/23- MSSlave Flora Comment: do not need to work on Audio Media for MSSlave */
	/* m_pIpCurrentMode->CopyMediaModeToOppositeDirection(cmCapAudio,cmCapReceive); */
	/* for MSSlaveOut :received the SIP_PARTY_CHANS_CONNECTED for the VideoIn, the first PartyCh m_pIpCurrentMode should be the pInitialMode here */
	//do not copy from opposite, otherwise video out will be set off
	//m_pIpCurrentMode->CopyMediaModeToOppositeDirection(cmCapVideo,cmCapReceive);
//	UpdateVideoTxModeForAsymmetricModes();

	m_pIpCurrentMode->Dump("Flora degug 5:CMSSlaveSipAddPartyCntl::OnPartyChannelsConnectedSetup - m_pCurrentMode is: ",eLevelInfoNormal);

	/* for SlaveOut/SlaveIn: Start to connect to Video Bridge */
	ConnectBridgesSetup();

	/* MSSlave Flora comment:  not RTP and Bridge are all ready in this state , do not send Ack from Slave to Master here */
	/*
	if(m_SentAckToMaster == TRUE)
		m_pTaskApi->SendAckFromSlaveToMaster(m_MasterRsrcId, statOK, GetPartyRsrcId(), m_TipPartyType, m_eLastAllocatedVideoPartyType);
	*/
}

/////////////////////////////////////////////////////////////////////////////
// The function connect bridges (Video only) after in channels were connected in dial out calls or
// in and out channels were connected in dial in calls.
/* MSSlave Flora comment: we can overload it or not, just remain the key code while overlaoding. */
void CMSSlaveSipAddPartyCntl::ConnectBridgesSetup() // channels connected or updated
{
	PTRACE(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::ConnectBridgesSetup");

	DWORD initialRate	= m_pIpInitialMode->GetMediaBitRate(cmCapVideo,cmCapReceive);
	DWORD currentRate	= m_pIpCurrentMode->GetMediaBitRate(cmCapVideo,cmCapReceive);
	BYTE  bEqualRates	= (initialRate == currentRate);
	BYTE  bIsCp			= (m_pIpInitialMode->GetConfType() == kCp);// (BYTE) m_pScm->IsFreeVideoRate();
	// CSmallString str;
	// str << m_partyConfName << " conf type - " << m_pIpInitialMode->GetConfType() << " init rate - " << initialRate << " current rate - " << currentRate;

	if (bIsCp || bEqualRates)
	{// since we copy media to opposite direction we have video rate in transmit direction at all scenarios
		DWORD videoRate = 0;
		videoRate = m_pIpCurrentMode->GetMediaBitRate(cmCapVideo,cmCapTransmit);
		SetNewVideoRates(videoRate);
	}

	EConfType confType = m_pIpInitialMode->GetConfType();
	*m_pIpInitialMode = *m_pIpCurrentMode;
	m_pIpInitialMode->SetConfType(confType);

	// connect bridges
	/* Floar comment: for MSSLaveOut&In, we do not need it at all */
#if 0	
	if (m_eAudBridgeConnState == eBridgeDisconnected) //in SIP we connect both directions
		ConnectPartyToAudioBridge(m_pIpCurrentMode);
#endif

	// connect video bridge only on CP or equal mode in VSW
	//if(m_TipPartyType != eTipSlaveAux)
	/* MSSlave Flora Question: it is supposed to be different with the Main SlaveParty ,you told me: */
	/* We should send some special params to Bridge, we should sync up these params with Bridge team */
	//noa-agredd it should be different -need t close it with Romeme/Yoella
	cmCapDirection mediaDirection = cmCapReceiveAndTransmit;
	if (IsMsSlaveIn())
	{
		mediaDirection = cmCapReceive;
	}
	else if (IsMsSlaveOut())
	{
		mediaDirection = cmCapTransmit;
	}
	if (m_pIpCurrentMode->IsMediaOn(cmCapVideo, mediaDirection) /*&& m_eConfTypeForAdvancedVideoFeatures == NO &&
		(bIsVswRelay || bIsCp || bIsCop || bEqualRates) */&& (m_eVidBridgeConnState == eBridgeDisconnected))
	{
		ConnectPartyToVideoBridge(m_pIpCurrentMode);
	}

	m_state = CONNECT_BRIDGES;
}

/////////////////////////////////////////////////////////////////////////////
void  CMSSlaveSipAddPartyCntl::OnAudConnectPartyConnectAudio(CSegment* pParam)
{
	/* MSSlave Flora comment:overload it, do nothing here for MSSlavePartyCntl */	
	PTRACE2PARTYID(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::OnAudConnectPartyConnectAudio : Name - ",m_partyConfName, GetPartyRsrcId());
}

/////////////////////////////////////////////////////////////////////////////
void CMSSlaveSipAddPartyCntl::OnPartyRemoteConnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::OnPartyRemoteConnected", GetPartyRsrcId());
	CSipComMode* pRemoteMode = new CSipComMode;
	pRemoteMode->DeSerialize(NATIVE, *pParam);
	BYTE bIsEndConfChangeMode = FALSE;
	*pParam >> bIsEndConfChangeMode;
	OffererPartyConnected(pRemoteMode);
	POBJDELETE(pRemoteMode);

//	if( m_pTaskApi )// do that for all slaves //&& eTipSlaveAux == m_TipPartyType )
//		m_pTaskApi->SendCAMGeneralNotifyCommand( GetPartyRsrcId(), eCAM_EVENT_PARTY_IS_TIP_SLAVE, (DWORD)0 );
}

/////////////////////////////////////////////////////////////////////////////
/* MSSlave Flora comment: overload it , just remaining the key code for MSSlavePartyCntl */
void CMSSlaveSipAddPartyCntl::OffererPartyConnected(CSipComMode* pRemoteMode)
{
	// meaning we received 200 ok with
	// if guess succeed than remote mode is current mode
	PTRACE(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::OffererPartyConnected: (Bridge mode is OK)");
	EConfType confType = m_pIpInitialMode->GetConfType();
	*m_pIpInitialMode = *pRemoteMode;
	m_pIpInitialMode->SetConfType(confType);

	pRemoteMode->Dump("Flora degug 6:CMSSlaveSipAddPartyCntl::OffererPartyConnected - pRemoteMode is: ",eLevelInfoNormal);
	ON(m_isFullBitRateConnect);
	DeleteTimer(CONNECTTOUT);
	m_state = PARTY_IN_CONNECTING_STATE;

	/*  MSSlave-Flora Yao-2013/10/23- MSSlave Flora Comment: Video Bridge is connected until now */
	if (IsMsSlaveOut())
	{
		// since we update the target mode when party out return now we are updating the current mode
		*m_pIpCurrentMode = *m_pIpInitialMode;
		m_pPartyApi->SipConfConnectCall();
	}
	else if (IsMsSlaveIn())
	{
		/* Send PARTY_CONTROL_SLAVE_TO_MASTER_ACK to SlavesController and ENDADDPARTY */
		m_pTaskApi->EndAddParty(m_pParty,statOK);

		if(m_SentAckToMaster == FALSE)
		{									
			m_pTaskApi->SendMsSlaveToMainAck(m_MasterRsrcId, ADDMSSLAVEPARTY, statOK, GetPartyRsrcId(), m_AvMcuLinkType, m_MSSlaveIndex);
			m_SentAckToMaster = TRUE;
		}
	}
	
}

/////////////////////////////////////////////////////////////////////////////
// The function connect party to conf after out channels of Offerer calls were connected
void CMSSlaveSipAddPartyCntl::OnPartyChannelsConnectedConnecting(CSegment* pParam) // channels connected or updated
{
	PTRACEPARTYID(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::OnPartyChannelsConnectedConnecting", GetPartyRsrcId());
	m_pIpCurrentMode->DeSerialize(NATIVE,*pParam);
	m_pTaskApi->EndAddParty(m_pParty,statOK);

	/* Flora comment: Notify SlavesControl of the ACK, MsSlaveParty is ready now. */
	if(m_SentAckToMaster == FALSE)
	{
		m_pTaskApi->SendMsSlaveToMainAck(m_MasterRsrcId, ADDMSSLAVEPARTY, statOK, GetPartyRsrcId(), m_AvMcuLinkType, m_MSSlaveIndex);
		m_SentAckToMaster = TRUE;
	}
	else
	{
		PTRACEPARTYID(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::OnPartyChannelsConnectedConnecting ack already sent on slave", GetPartyRsrcId());

	}
}

/////////////////////////////////////////////////////////////////////////////
/* MSSlave Flora comment: It is not needed to overload it here, just use the CSipAddPartyCntl::OnVideoBrdgConnected */
/* MSSlave Flora Question here is , for Tip, in function: CSipSlavePartyCntl::OnVideoBrdgConnected, */
/* it calls the function : SetPartyStateUpdateDbAndCdrAfterEndConnected(SECONDARY_CAUSE_NO_VIDEO_CONNECTION) at the last line; why need it ? */
/* do you think for MSSlaveOut, we need to call it here ? it do not call SetPartyStateUpdateDbAndCdrAfterEndConnected in function CSipAddPartyCntl::OnVideoBrdgConnected*/
void CMSSlaveSipAddPartyCntl::OnVideoBrdgConnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::OnVideoBrdgConnected", GetPartyRsrcId());
	if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACEPARTYID(eLevelInfoNormal,"CMSSlaveSipAddPartyCntl::OnVideoBrdgConnected : Connect has received after disconnect", GetPartyRsrcId());
	}
	else
	{
		HandleVideoBridgeConnectedInd(pParam);
		
		CMedString msg1;
		msg1 << "Name -:" << m_partyConfName 
			<< ", IsMsSlaveIn = " << IsMsSlaveIn()
			<< ", m_eVidBridgeConnState = " << GetBridgeConnectionStateStr(m_eVidBridgeConnState)
			<< ", IsInDirectionConnectedToVideoBridge() = " << IsInDirectionConnectedToVideoBridge()
			<< ", m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit) = " << m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit)
			<< ", IsMsSlaveOut(): = " << IsMsSlaveOut()
			<< ", IsOutDirectionConnectedToVideoBridge() = " << IsOutDirectionConnectedToVideoBridge()
			<< ", m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapReceive) = " << m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapReceive);
		PTRACE2(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::OnVideoBrdgConnected: ", msg1.GetString());
		if ((IsMsSlaveIn() && IsInDirectionConnectedToVideoBridge() && m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit)) ||
			(IsMsSlaveOut() && IsOutDirectionConnectedToVideoBridge() && m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapReceive)) )
		{
			m_pPartyApi->SendVideoBridgeConnected();
			
			SetPartyStateUpdateDbAndCdrAfterEndConnected(SECONDARY_CAUSE_NO_VIDEO_CONNECTION);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMSSlaveSipAddPartyCntl::SetupMSSlaveSlaveSipConParameters(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
    CConfParty* pConfParty = partyControInitParam.pConfParty;
	if (!pConfParty)
	{
	    PTRACE(eLevelError, "CMSSlaveSipAddPartyCntl::SetupSIPSlaveConParameters - pConfParty is NULL");
	    DBGPASSERT(1124);
	    return;
	}

	pConfParty->SetRemoteName(pConfParty->GetName());

	/* MSSlave Flora Question: do MSSlave need to do the same thing?  within this interface send :UPDATEVISUALNAME */
	pConfParty->SetVisualPartyName(pConfParty->GetName());//michael

	m_AvMcuLinkType 				= partyControlDataParams.AvMcuLinkType;
	m_MSSlaveIndex				= partyControlDataParams.msSlaveIndex;
	
	m_MSSlaveSsrcRangeStart		= partyControlDataParams.msSlaveSsrcRangeStart;
	AllocMemory();
	m_monitorConfId				= partyControInitParam.monitorConfId;
	m_monitorPartyId			= partyControInitParam.pConfParty->GetPartyId();
	m_TipPartyType 				= partyControlDataParams.tipPartyType;
	m_RoomId 					= partyControlDataParams.roomID;
	m_pConf						= partyControInitParam.pConf;
	m_pParty					= partyControlDataParams.pParty;
	m_voice							= pConfParty->GetVoice();
	m_serviceId					= pConfParty->GetServiceId();
	m_bIsLync						= partyControlDataParams.bIsLync;

	m_isMsftEnv                     = TRUE;
	if (partyControlDataParams.pSipRmtCaps != NULL)
			*m_pSipRemoteCaps			= *partyControlDataParams.pSipRmtCaps;

	pConfParty->SetAvMcuLinkType(m_AvMcuLinkType);
	/* MSSlave Flora Question: */
	/* from Anat mail: */
	/*
	In addition, we need to add the ssrc range that we haven't talked about it yet.
	This parameter is relevant only for the out slave and will be taken from the main party.
	Main party has a range of 100 numbers for ssrc that should be used for all slaves, each slave will use 10.
	A new parameter will hold the first number that will be used by the created slave:
	DWORD				 ssrcRangeStart
	Will be taken from: 		CSipCaps::getMsftSsrcVideoFirst(int LineNum)	
	*/
	/*but as we talked before, we should do like this for MSSlave: */
	/*
	in function: CSipCntl::Rtp_FillMsftSvcParamsOnRtpStruct	
	for out:
	m_pChosenLocalCap->getMsftSsrcVideoFirst(1) + slaveindex*10
	
	for in:
	pStruct->updateSsrcParams.unMsFirstSyncSourceInRange = m_pLastRemoteCaps->getMsftSsrcVideoFirst(slaveinindex+1);
	pStruct->updateSsrcParams.unMsLastSyncSourceInRange = m_pLastRemoteCaps->getMsftSsrcVideoLast(slaveinindex+1);
	*/

	/* so i don't think i need to get the ssrc range from main party here */
	SetSeviceIdForConfParty(pConfParty);
	SetIceForConfParty(pConfParty);
	CSipNetSetup* pIpNetSetup = NewAndSetupSipNetSetup(pConfParty,partyControInitParam,partyControlDataParams);
	CIpComMode*	pPartyScm = NewAndGetPartyCntlScm(partyControInitParam,partyControlDataParams);
	
	//Fix bug for MSSlave Out Create: no Vidoe bitrate
	SetMaxBitRateInNetSetup(pIpNetSetup,pConfParty, pPartyScm, partyControInitParam,partyControlDataParams);	

	DWORD vidBitrate = pPartyScm->GetTotalVideoRate();
	TRACEINTO<<"Flora debug 1: vidBitrate : "<< pPartyScm->GetTotalVideoRate();

	
	CSipCaps* pSIPCaps = NewAndGetLocalSipCaps(pPartyScm,pConfParty,pIpNetSetup,vidBitrate,partyControInitParam,partyControlDataParams);
	TRACEINTO<<"Flora debug 2: vidBitrate : "<< vidBitrate;
	
	*m_pSipLocalCaps			= *pSIPCaps;
	*m_pSIPNetSetup				= *pIpNetSetup;
	*m_pIpInitialMode 			= *pPartyScm;

	if (eAvMcuLinkSlaveIn == m_AvMcuLinkType)
	{
		pPartyScm->SetMediaOff(cmCapVideo, cmCapTransmit, kRolePeople);
	}
	else if (eAvMcuLinkSlaveOut == m_AvMcuLinkType)
	{
		pPartyScm->SetMediaOff(cmCapVideo, cmCapReceive, kRolePeople);
	}



	/* MSSlave Flora Question: do i need it for MSSlave ? for Tip, this param is used to CreateAndSendAllocatePartyResources  */
	m_VideoPartyType 			= partyControlDataParams.masterVideoPartyType;
	
	m_pTerminalNumberingManager = partyControInitParam.pTerminalNumberingManager;
	m_pAudioInterface			= partyControInitParam.pAudioBridgeInterface;
	m_pVideoBridgeInterface		= partyControInitParam.pVideoBridgeInterface;
	m_pFECCBridge				= partyControInitParam.pFECCBridge;
	m_pContentBridge			= partyControInitParam.pContentBridge;
	m_pConfAppMngrInterface		= partyControInitParam.pConfAppMngrInterface;

	/* MSSlave Flora comment: for MSSlave, i need to get the MainParty RsrcId here */
	/* Talk with Anat, we use this param or another new one to send it */
	m_MasterRsrcId              = partyControlDataParams.peerPartyRsrcID;

	const char* strConfName = m_pConf->GetName();
	m_name = new char[H243_NAME_LEN];
	memset(m_name, '\0', H243_NAME_LEN);
	strncpy(m_name, partyControlDataParams.pStrPartyName, H243_NAME_LEN - 1);

	m_originalConfRate 				= m_pIpInitialMode->GetCallRate();
	m_eConfTypeForAdvancedVideoFeatures = kNotAnAdvancedVideoConference;

	//if(m_pIpInitialMode->GetConfType() != kVideoSwitch && kVSW_Fixed != m_pIpInitialMode->GetConfType())
	{
		SetNewVideoRates(vidBitrate);
	}
	m_pIpInitialMode->SetTotalVideoRate(vidBitrate);
	
	m_pIpInitialMode->SetMediaOff(cmCapData, cmCapReceiveAndTransmit);

	m_pIpInitialMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
	m_pIpInitialMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
	m_pIpInitialMode->SetMediaOff(cmCapAudio, cmCapReceiveAndTransmit);
	if (IsMsSlaveIn())
	{
		m_pIpInitialMode->SetMediaOff(cmCapVideo, cmCapTransmit, kRolePeople);
	}
	else if (IsMsSlaveOut())
	{
		m_pIpInitialMode->SetMediaOff(cmCapVideo, cmCapReceive, kRolePeople);
	}
	m_pIpInitialMode->SetTipAuxFPS(eTipAuxNone);
	
	m_pSipLocalCaps->SetTipAuxFPS(eTipAuxNone);
	m_pSipLocalCaps->CleanMedia(cmCapBfcp); // bfcp
	m_pSipLocalCaps->CleanMedia(cmCapVideo, kRolePresentation); // content
	m_pSipLocalCaps->CleanMedia(cmCapData, kRolePresentation); // content
	/* MSSlave Flora comment: for MSSlave, i need to clean Audio here ? */
	m_pSipLocalCaps->CleanMedia(cmCapAudio);

	SetFullName(m_name, strConfName);
	PTRACE2PARTYID(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::Create: Name - ",
			m_partyConfName, GetPartyRsrcId());

	m_pBridgeMoveParams = new CBridgeMoveParams;

	/* MSSlave Flora comment: I think this is the mailbox(partresrc) of the main party control so MSSlave can send messages to MainPartyCntl */
	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi(m_pConf->GetRcvMbx(), this);
	m_pTaskApi->SetLocalMbx(m_pConf->GetLocalQueue());

#if 0
	if (m_TipPartyType == eTipSlaveAux)
		SetPartyToAudioOnly();
#endif
	m_entryPoint = MSSlaveSipPartyEntryPoint;

	DWORD tmp_Ssrc_f = 0;
	DWORD tmp_Ssrc_l = 0;
	if (partyControlDataParams.AvMcuLinkType == eAvMcuLinkSlaveOut)
	{
		tmp_Ssrc_f = partyControlDataParams.msSlaveSsrcRangeStart;
		tmp_Ssrc_l = tmp_Ssrc_l + 10;
	}
	else if (partyControlDataParams.AvMcuLinkType == eAvMcuLinkSlaveIn)
	{
		TRACEINTO<<"Flora debug: SSRC ---0 : "<< vidBitrate;
		if (m_pSipRemoteCaps)
		{
			TRACEINTO<<"Flora debug: SSRC ---1 : "<< vidBitrate;
			DWORD unIndex = 1; 
			unIndex = m_MSSlaveIndex + 1;
			tmp_Ssrc_f =  m_pSipRemoteCaps->getMsftSsrcVideoFirst(unIndex);
			tmp_Ssrc_l = m_pSipRemoteCaps->getMsftSsrcVideoLast(unIndex);
		}
	}
	
	CMedString msg1;
	msg1 << "Name -:" << m_partyConfName 
		<< ", ConfType = " << m_pIpInitialMode->GetConfType()
		<< ", videoRate = " << vidBitrate
		<< ", Video Transmit MediaBitRate = " << (DWORD)m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit)
		<< ", MSSlave Index: = " << partyControlDataParams.msSlaveIndex
		<< ", AvMcuLinkType: = " << eAvMcuLinkTypeNames[partyControlDataParams.AvMcuLinkType]
		<< ", MSSlave SSRC First: = " << tmp_Ssrc_f
		<< ", MSSlave SSRC Last: = " << tmp_Ssrc_l;
	PTRACE2(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::SetupSIPConParameters: ", msg1.GetString());

	m_pIpInitialMode->Dump("Flora degug 0:CMSSlaveSipAddPartyCntl::SetupMSSlaveSlaveSipConParameters - m_pIpInitialMode is: ",eLevelInfoNormal);

	POBJDELETE(pSIPCaps);
	POBJDELETE(pPartyScm);
	POBJDELETE(pIpNetSetup);
}

/////////////////////////////////////////////////////////////////////////////
void CMSSlaveSipAddPartyCntl::SetIceForConfParty(CConfParty* pConfParty)
{
	CConfPartyProcess* pConfPartyProcess =
			(CConfPartyProcess*) CConfPartyProcess::GetProcess();
    BOOL bMsEnviroment = FALSE;
	CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(pConfParty->GetServiceId());
	if( pService != NULL && pService->GetConfigurationOfSipServers() )
	{
		if(pService->GetSipServerType() == eSipServer_ms)
			bMsEnviroment = TRUE;
	}
    TRACEINTO<<"m_bIsMrcCall: "<<(int)m_bIsMrcCall<<"bMsEnviroment: "<<(int)bMsEnviroment;

	if(pConfPartyProcess->m_IceInitializationStatus == eIceStatusON && !(bMsEnviroment && m_bIsMrcCall)  )
	{
		PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::NewAndSetupSipNetSetup WIth ICE!!");
		pConfParty->SetEnableICE(TRUE);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::NewAndSetupSipNetSetup Without ICE!! -",pConfPartyProcess->m_IceInitializationStatus);
		pConfParty->SetEnableICE(FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////
CIpComMode* CMSSlaveSipAddPartyCntl::NewAndGetPartyCntlScm(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams )
{
	/* MSSlave Flora Question: should i new the PartyScm base on the pIpMasterInitialMode as the TipSlave does */
	/* Or base on the partyControInitParam.pConfIpScm as the Main SipParty does */
	//CIpComMode* pPartyScm = new CIpComMode(
		//	*(partyControlDataParams.pIpMasterInitialMode));

	CIpComMode* pPartyScm = CIpPartyCntl::NewAndGetPartyCntlScm(partyControInitParam, partyControlDataParams);
	//pPartyScm->SetTipMode(eTipModePossible);

	//WORD bIsAudioOnly = pConfParty->GetVoice();
	//if (partyControlDataParams.tipPartyType == eTipSlaveAux)
	/* MSSlave Flora comment: for MSSlave, we should set off the people video in for MSSlave Out / set off the people video out for MSSlave In */
	/* set up PartyScm for MSSlave here */


	pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit,
			kRoleContentOrPresentation);
	pPartyScm->SetMediaOff(cmCapAudio, cmCapReceiveAndTransmit);	
	pPartyScm->SetMediaOff(cmCapData, cmCapReceiveAndTransmit);
	pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
	pPartyScm->SetTipAuxFPS(eTipAuxNone);

	pPartyScm->SetIsLpr(FALSE);

	if (eAvMcuLinkSlaveOut == GetAvMcuLinkType())
	{
		//Flora for debug 0102	
		pPartyScm->Dump("CMSSlaveSipAddPartyCntl::NewAndGetPartyCntlScm - Before Sdes Update, Local caps :", eLevelInfoNormal);

		if (partyControlDataParams.pLocalSdesCap != NULL)
		{
			CVidModeH323*	tmp_pSipLocalCaps = new CVidModeH323;
			CSdesCap* pTmp_LocalSdesCap       = NULL;
			
			if (tmp_pSipLocalCaps)
			{
				*tmp_pSipLocalCaps = *partyControlDataParams.pLocalSdesCap;
				pTmp_LocalSdesCap = tmp_pSipLocalCaps->GetSdesCap();
			
			}
			if ( pTmp_LocalSdesCap )
			{
				//remove all the current sdescaps from the remote caps
				pPartyScm->RemoveSipSdes(cmCapVideo,cmCapTransmit,kRolePeople);

				//set the sdescaps in remote caps
				pPartyScm->SetSipSdes(cmCapVideo,cmCapTransmit,kRolePeople,pTmp_LocalSdesCap);
				
			}
			
			POBJDELETE(tmp_pSipLocalCaps);
		}
		pPartyScm->Dump("CMSSlaveSipAddPartyCntl::NewAndGetPartyCntlScm - After Sdes Update, Local caps :", eLevelInfoNormal);
	}
	//Flora add for  In slaves Encryption:AV-MCU sends a different key per m-line.
	//  i: Each slaves will use the key according to its index.
	// ii: Each slave will set in the remote caps the correct sdes caps according to its index.
	if (m_pSipRemoteCaps && (m_AvMcuLinkType == eAvMcuLinkSlaveIn))
	{
		//get the sdescap from the remote caps according  to msslave in index
		//CBaseCap* pTmp_Cap	= m_pSipRemoteCaps->GetSdesCapSet(cmCapVideo,unIndex,kRolePeople);
		CSdesCap* pTmp_Cap	= m_pSipRemoteCaps->GetVideoSdesCapAVMCU(m_MSSlaveIndex);

		if (pTmp_Cap)
		{
			//remove all the current sdescaps from the remote caps
			m_pSipRemoteCaps->RemoveSdesCaps(cmCapVideo,kRolePeople);

			//set the sdescaps in remote caps
			m_pSipRemoteCaps->AddSdesCapSet(cmCapVideo,(CBaseCap*)pTmp_Cap,kRolePeople);

			//remove all the current sdescaps from the remote caps
			pPartyScm->RemoveSipSdes(cmCapVideo,cmCapReceive,kRolePeople);

			//set the sdescaps in remote caps
			pPartyScm->SetSipSdes(cmCapVideo,cmCapReceive,kRolePeople,pTmp_Cap);

			POBJDELETE(pTmp_Cap);
		}
	}
	return pPartyScm;
}

/////////////////////////////////////////////////////////////////////////////
CSipNetSetup* CMSSlaveSipAddPartyCntl::NewAndSetupSipNetSetup(
		CConfParty* pConfParty,
		PartyControlInitParameters& partyControInitParam,
		PartyControlDataParameters &partyControlDataParams) {

	//DWORD confRate = pPartyScm->GetCallRate();
//	DWORD vidBitrate = pPartyScm->GetTotalVideoRate();

	char tempName[64];
	memset(&tempName, '\0', IPV6_ADDRESS_LEN);
	ipToString(pConfParty->GetIpAddress(), tempName, 1);

	CMedString msg1;
	msg1 << ", pConfParty->GetIpAddress():" << tempName;
	PTRACE2(eLevelInfoNormal, "CMSSlaveSipAddPartyCntl::NewAndSetupSipNetSetup - ", msg1.GetString());

	CSipNetSetup* pSipNetSetup = new CSipNetSetup;
	const char* strSipAddress = pConfParty->GetSipPartyAddress();

	WORD type = pConfParty->GetSipPartyAddressType();
	WORD port = pConfParty->GetCallSignallingPort(); //5060 - ema bug.

	pSipNetSetup->SetRemoteDisplayName(pConfParty->GetName());
	pSipNetSetup->SetRemoteSipAddress(strSipAddress);
	pSipNetSetup->SetRemoteSipAddressType(type);
	pSipNetSetup->SetRemoteSignallingPort(port);

	//pSipNetSetup->SetMaxRate(confRate);
	pSipNetSetup->SetMinRate(0);
	pSipNetSetup->SetConfId(partyControInitParam.monitorConfId);

	if ((pConfParty->GetNodeType()) == 0) {
		pSipNetSetup->SetEndpointType(2);
	} 
	else 
	{
		if ((pConfParty->GetNodeType()) == 1) 
		{
			pSipNetSetup->SetEndpointType(0);
		}
	}

	pSipNetSetup->SetEnableSipICE(pConfParty->GetEnableICE());
	
	InitDisplayNameForNetSetup(pSipNetSetup);

	return pSipNetSetup;
}

CSipCaps* CMSSlaveSipAddPartyCntl::NewAndGetLocalSipCaps(CIpComMode* pPartyScm,CConfParty* pConfParty,CSipNetSetup* pIpNetSetup,DWORD& vidBitrate,PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams)
{
	DWORD confRate = 0;
		int val;
		val = ::CalculateRateForIpCalls(pConfParty, pPartyScm, confRate, vidBitrate,partyControInitParam.bIsEncript);
		PASSERT((val == -1));
		vidBitrate = vidBitrate / 100;
		BOOL isPreventOverflow = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_PREVENT_OVERFLOW);
	        if( !isPreventOverflow )
			{
			     DWORD audioRate = pPartyScm->GetMediaBitRate(cmCapAudio);
			     DWORD callRate = partyControlDataParams.callRate;
			     if( audioRate > 48 )
			     	 vidBitrate = (callRate - 48*_K_)/100;
			}
	        if(pPartyScm->GetConfType() != kVideoSwitch && kVSW_Fixed != pPartyScm->GetConfType())
	        	pPartyScm->SetVideoBitRate(vidBitrate, cmCapReceiveAndTransmit);

	     BOOL bDisableAvMcuLowRate = GetSystemCfgFlag<BOOL>("DISABLE_LYNC_AV_MCU_128_192_KBPS");
	     if(!bDisableAvMcuLowRate && confRate == 128000 && eAvMcuLinkSlaveIn == GetAvMcuLinkType() )
	     {
	    	 TRACEINTO<<"patch for av-mcu treating 128k call rate as 192k for in only" ;
	    	 pPartyScm->SetVideoBitRate(1920, cmCapReceiveAndTransmit);
	    	 vidBitrate = 2560;

	     }

		CSipCaps* pSIPCaps = new CSipCaps;
/*
		WORD confPartyConnectionType = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());
		if (confPartyConnectionType == DIALOUT)
		{
			SetSIPPartyCapsAndVideoParam(pPartyScm, pSIPCaps, pConfParty, vidBitrate, ((CSipNetSetup*)pIpNetSetup)->GetEnableSipICE(), pIpNetSetup, 0, FALSE, pConfParty->GetServiceId(),partyControInitParam,partyControlDataParams);
		}
		*/
		DWORD encryptionKeyToUse=eUseBothEncryptionKeys;
		if(m_lyncRedialOutAttempt==1)
		{
			encryptionKeyToUse=eUseMkiKeyOnly;
			TRACEINTO << "LYNC_REDIAL set encryptionKeyToUse=eUseMkiKeyOnly on lync redial";
		}
		BYTE highestframerate= (BYTE)eCopVideoFrameRate_None;
		pSIPCaps->Create(pPartyScm, vidBitrate, pConfParty->GetName(), partyControlDataParams.vidQuality,pConfParty->GetServiceId(),encryptionKeyToUse,((ECopVideoFrameRate)highestframerate ), pConfParty->GetMaxResolution(),FALSE,TRUE/*lync 2013*/); //eyalnmic
		return pSIPCaps;
}
void CMSSlaveSipAddPartyCntl::SendSingleUpdatePacsiInfoToSlavesController(const MsSvcParamsStruct& pacsiInfo, BYTE isReasonFecOrRed)
{
	TRACEINTO << " ConfName:" << m_partyConfName << ", PartyName:" << GetPartyRsrcId()
			  << ", isReasonFecOrRed:" << (DWORD)isReasonFecOrRed << " - single pacsi info will not be sent";
}
#if 0
/////////////////////////////////////////////////////////////////////////////
void CSipSlavePartyCntl::ChangeScm(CIpComMode* pIpScm)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::ChangeScm", GetPartyRsrcId());
	EConfType confType = m_pIpInitialMode->GetConfType();
	*m_pIpInitialMode = *pIpScm;
	m_pIpInitialMode->SetConfType(confType);
	SetPartyStateUpdateDbAndCdrAfterEndConnected(SECONDARY_CAUSE_NO_VIDEO_CONNECTION);

}
#endif


