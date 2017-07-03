
#include "MsSlavesController.h"
#include "PartyApi.h"
#include "Party.h"
#include "SysConfigKeys.h"
#include "ConfigHelper.h"

////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CMsSlavesController)

ONEVENT(PARTY_CONTROL_MS_SLAVE_TO_MAIN_ACK,				ANYCASE,			CMsSlavesController::OnMsSlaveToMainAckMessage)
ONEVENT(SIP_PARTY_VSR_MSG_IND,							ANYCASE,			CMsSlavesController::OnAvMcuVsrMsgInd)
ONEVENT(RMTH230,										ANYCASE,			CMsSlavesController::OnAvMcuRmtH230)
ONEVENT(STREAMS_INTRA_REQ,								ANYCASE,			CMsSlavesController::OnAvMcuStreamsIntraReq)
ONEVENT(SINGLE_PACSI_INFO_IND,							ANYCASE,			CMsSlavesController::OnMsSlaveSinglePacsiInfoInd)
ONEVENT(FULL_PACSI_INFO_TIMER,							ANYCASE,			CMsSlavesController::OnTimerFullPacsiInfo)
ONEVENT(LYNC_CONF_INFO_UPDATED,							ANYCASE,			CMsSlavesController::HandleEventPackageEvent)
ONEVENT(DELMSSLAVEPARTY,								ANYCASE,			CMsSlavesController::OnDelMsSlaveParty)
ONEVENT(CONNECT_MS_OUT_SLAVES_TIMER,					ANYCASE,			CMsSlavesController::OnConnectMsOutSlavesTout)
ONEVENT(CONNECT_MS_IN_SLAVES_TIMER,						ANYCASE,			CMsSlavesController::OnConnectMsInSlavesTout)
ONEVENT(HANDLE_SINGEL_FEC_OR_RED_WITH_AV_MCU2013,		ANYCASE,			CMsSlavesController::OnAvMcuSendHandleSingleFecMsgToSlave)

PEND_MESSAGE_MAP(CMsSlavesController,CStateMachine);


////////////////////////////////////////////////////////////////////////////////
CMsSlavesController::CMsSlavesController()
{
	m_pMainConfParty = NULL;
	m_pTaskApi = NULL;
	m_pSubscribedQueue = NULL;
	// all slaves lists and counters INCLUDE the main party control
	for (int i = 0; i < MAX_MS_OUT_SLAVES; i++)
		m_msOutSlaves[i] = NULL;
	for (int j = 0; j < MAX_MS_IN_SLAVES; j++)
		m_msInSlaves[j] = NULL;

	m_numOfActiveOutSlaves = 0;
	m_numOfActiveInSlaves = 0;

	m_totalBandwidth = 0;
	m_pSipRemoteCaps  = NULL;
	m_localAudioMsi = (DWORD)(-1);

	m_isVsrInProgress = false;
	m_isWaitingVsr = false;
	m_pWaitingVsrMsg = NULL;
	m_isEncrypt = FALSE;
	m_disableInSlavesDynamicRemoval = FALSE;
	m_maxConfBitRate = 0;
	m_AvMcuCascadeMode = eMsSvcResourceOptimize;
}

////////////////////////////////////////////////////////////////////////////////
CMsSlavesController::~CMsSlavesController()
{
	for (int i = 0; i < MAX_MS_OUT_SLAVES; i++)
		if (m_msOutSlaves[i])
			POBJDELETE(m_msOutSlaves[i]);
	for (int j = 0; j < MAX_MS_IN_SLAVES; j++)
		if (m_msInSlaves[j])
			POBJDELETE(m_msInSlaves[j]);

	if (m_pTaskApi)
	{
		m_pTaskApi->DestroyOnlyApi();
		POBJDELETE(m_pTaskApi);
	}

	POBJDELETE(m_pSipRemoteCaps);
	if(m_pWaitingVsrMsg){
		POBJDELETE(m_pWaitingVsrMsg);
	}

}


////////////////////////////////////////////////////////////////////////////////
CMsSlavesController::CMsSlavesController(const CMsSlavesController& other)
:CStateMachine(other)
{
	*this = other;	// use operator =
}


////////////////////////////////////////////////////////////////////////////////
CMsSlavesController& CMsSlavesController::operator=(const CMsSlavesController& other)
{
	if(this != &other)
	{
		m_pMainConfParty = other.m_pMainConfParty;

		if(other.m_pTaskApi == NULL)
			m_pTaskApi = NULL;
		else
			m_pTaskApi = new CConfApi(*(other.m_pTaskApi));

		m_pSubscribedQueue = other.m_pSubscribedQueue;

		// all slaves lists and counters INCLUDE the main party control
		for (int i = 0; i < MAX_MS_OUT_SLAVES; i++)
		{
			if(other.m_msOutSlaves[i] == NULL)
				m_msOutSlaves[i] = NULL;
			else
				m_msOutSlaves[i] = new CMsPartyCntlInfo(*(other.m_msOutSlaves[i]));
		}

		for (int j = 0; j < MAX_MS_IN_SLAVES; j++)
		{
			if(other.m_msInSlaves[j] == NULL)
				m_msInSlaves[j] = NULL;
			else
				m_msInSlaves[j] = new CMsPartyCntlInfo(*(other.m_msInSlaves[j]));
		}

		m_numOfActiveOutSlaves = other.m_numOfActiveOutSlaves;
		m_numOfActiveInSlaves = other.m_numOfActiveInSlaves;
		m_maxConfBitRate      = other.m_maxConfBitRate;

		m_totalBandwidth = other.m_totalBandwidth;
		m_AvMcuCascadeMode = other.m_AvMcuCascadeMode;

		if(other.m_pSipRemoteCaps == NULL)
			m_pSipRemoteCaps = NULL;
		else
			m_pSipRemoteCaps = new CSipCaps(*(other.m_pSipRemoteCaps));

		m_localAudioMsi = other.m_localAudioMsi;
		// pacsi
		m_fullPacsiInfo = other.m_fullPacsiInfo;
		m_lastFullPacsiInfo = other.m_lastFullPacsiInfo;

		// waiting VSR
		m_isVsrInProgress = other.m_isVsrInProgress;
		m_isWaitingVsr = other.m_isWaitingVsr;
		if(NULL !=other.m_pWaitingVsrMsg){
			m_pWaitingVsrMsg = new CMsVsrMsg(*m_pWaitingVsrMsg);
		}else{
			m_pWaitingVsrMsg = NULL;
		}

		m_isEncrypt = other.m_isEncrypt;
		m_disableInSlavesDynamicRemoval = other.m_disableInSlavesDynamicRemoval;

	}
	return *this;
}



////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::Create(PartyRsrcID mainPartyRsrcId, PartyMonitorID mainMonitorPartyId, CConf* pConf, CSipCaps* remoteCaps, BOOL isEncrypt)
{
	PASSERT_AND_RETURN(!pConf);

	TRACEINTO << "MainPartyId:" << mainPartyRsrcId << ", ConfName:" << pConf->GetName() << "isEncrypt " << isEncrypt ;

	const CCommConf* pCommConf = pConf->GetCommConf();
	PASSERT_AND_RETURN(!pCommConf);
	m_pMainConfParty = pCommConf->GetCurrentParty(mainMonitorPartyId);

	m_maxConfBitRate = pConf->GetCommConf()->GetConfTransferRate();
	m_AvMcuCascadeMode =((eMsSvcVideoMode)(pCommConf->GetMsSvcCascadeMode()));


	PASSERT_AND_RETURN(!m_pMainConfParty);

	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi(pConf->GetRcvMbx(), this);
	m_pTaskApi->SetLocalMbx(pConf->GetLocalQueue());
	m_pSipRemoteCaps = new CSipCaps(*remoteCaps);

	CSuperLargeString strCaps1;
	m_pSipRemoteCaps->DumpToString(strCaps1);
	PTRACE2(eLevelInfoNormal, "CMsSlavesController::Create - SipRemoteCaps:", strCaps1.GetString());

	m_isEncrypt = isEncrypt;

	// Disable in slaves dynamic removal (make the in slave inactive instead of deleting it)
	m_disableInSlavesDynamicRemoval = GetSystemCfgFlag<BOOL>(CFG_KEY_DISABLE_MS_IN_SLAVES_DYNAMIC_REMOVAL);

	// Add main party to slaves lists
	m_msOutSlaves[0] = new CMsPartyCntlInfo;
	m_msOutSlaves[0]->SetName(m_pMainConfParty->GetName());
	m_msOutSlaves[0]->SetPartyRsrcId(mainPartyRsrcId);
	m_msOutSlaves[0]->SetConnectState(eMsSlaveConnected);
	m_numOfActiveOutSlaves++;

	m_msInSlaves[0] = new CMsPartyCntlInfo;
	m_msInSlaves[0]->SetName(m_pMainConfParty->GetName());
	m_msInSlaves[0]->SetPartyRsrcId(mainPartyRsrcId);
	m_numOfActiveInSlaves++;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::ConnectOutSlaves(EVideoResolutionType mainVideoResolutionType, EVideoResolutionType maxConfResolutionType, DWORD msSsrcRangeStart, DWORD totalBandwidth, CVidModeH323 *pLocalSdesCap)
{
	PASSERT_AND_RETURN(!m_msOutSlaves[0]);

	m_totalBandwidth = totalBandwidth;

	// Main's params
	m_msOutSlaves[0]->SetMaxResolution(mainVideoResolutionType);
	m_msOutSlaves[0]->SetSsrcRangeStart(msSsrcRangeStart);

	//LYNC_AVMCU_1080p30:
	BOOL isSLynvAvMcu1080p30Enabled = FALSE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("LYNC_AVMCU_1080p30_ENCODE_RESOLUTION", isSLynvAvMcu1080p30Enabled);

	BOOL isAllocateHdEncoderInResourceOptimize = FALSE;
	pSysConfig->GetBOOLDataByKey("ALLOCATE_HD_ENCODER_LYNC_AV_MCU_CASCADE_RESOURCE_OPTIMIZE", isAllocateHdEncoderInResourceOptimize);

	TRACEINTO << "Main PartyRsrcId: " << m_msOutSlaves[0]->GetPartyRsrcId()
				  << ", mainVideoResolutionType: " << eVideoResolutionTypeNames[mainVideoResolutionType]
				  << ", maxConfResolutionType: " << eVideoResolutionTypeNames[maxConfResolutionType]
				  << ", isSLynvAvMcu1080p30Enabled: "<<isSLynvAvMcu1080p30Enabled
				  << ", isAllocateHdEncoderInResourceOptimize: "<<isAllocateHdEncoderInResourceOptimize;




	switch(mainVideoResolutionType)
	{
		case eHD720_Res:
		case eHD1080_Res:// create 2 out slaves: SD & CIF
			// Create SD out slave
			m_msOutSlaves[1] = new CMsPartyCntlInfo;
			m_msOutSlaves[1]->SetMaxResolution(eSD_Res);
			m_msOutSlaves[1]->SetSsrcRangeStart(msSsrcRangeStart+10);
			m_numOfActiveOutSlaves++;
			AddMsSlaveParty(eAvMcuLinkSlaveOut, 1, eSD_Res, msSsrcRangeStart+10, pLocalSdesCap);

			// Create CIF out slave
			m_msOutSlaves[2] = new CMsPartyCntlInfo;
			m_msOutSlaves[2]->SetMaxResolution(eCIF_Res);
			m_msOutSlaves[2]->SetSsrcRangeStart(msSsrcRangeStart+20);
			m_numOfActiveOutSlaves++;
			AddMsSlaveParty(eAvMcuLinkSlaveOut, 2, eCIF_Res, msSsrcRangeStart+20, pLocalSdesCap);

			break;

		case eSD_Res:	// create 1 out slave: CIF
			// Create CIF out slave
			m_msOutSlaves[1] = new CMsPartyCntlInfo;
			m_msOutSlaves[1]->SetMaxResolution(eCIF_Res);
			m_msOutSlaves[1]->SetSsrcRangeStart(msSsrcRangeStart+10);
			m_numOfActiveOutSlaves++;
			AddMsSlaveParty(eAvMcuLinkSlaveOut, 1, eCIF_Res, msSsrcRangeStart+10, pLocalSdesCap);

			if(maxConfResolutionType >= eHD720_Res && ( (!isSLynvAvMcu1080p30Enabled && m_AvMcuCascadeMode == eMsSvcVideoOptimize ) || ( m_AvMcuCascadeMode == eMsSvcResourceOptimize && isAllocateHdEncoderInResourceOptimize ) ))	//in case max conf resolution is hd but main is sd need to also add out hd
			{
				m_msOutSlaves[2] = new CMsPartyCntlInfo;
				m_msOutSlaves[2]->SetMaxResolution(eHD720_Res);
				m_msOutSlaves[2]->SetSsrcRangeStart(msSsrcRangeStart+20);
				m_numOfActiveOutSlaves++;
				AddMsSlaveParty(eAvMcuLinkSlaveOut, 2, eHD720_Res, msSsrcRangeStart+20, pLocalSdesCap);
			}
			else if(maxConfResolutionType == eHD1080_Res && isSLynvAvMcu1080p30Enabled && m_AvMcuCascadeMode == eMsSvcVideoOptimize)
			{
				m_msOutSlaves[2] = new CMsPartyCntlInfo;
				m_msOutSlaves[2]->SetMaxResolution(eHD1080_Res);
				m_msOutSlaves[2]->SetSsrcRangeStart(msSsrcRangeStart+20);
				m_numOfActiveOutSlaves++;
				AddMsSlaveParty(eAvMcuLinkSlaveOut, 2, eHD1080_Res, msSsrcRangeStart+20);
			}

			break;

		case eCIF_Res:	// no out slaves should be created, unless max conf resolution is SD/HD but main's is CIF
			if(maxConfResolutionType >= eSD_Res)
			{
				m_msOutSlaves[1] = new CMsPartyCntlInfo;
				m_msOutSlaves[1]->SetMaxResolution(eSD_Res);
				m_msOutSlaves[1]->SetSsrcRangeStart(msSsrcRangeStart+10);
				m_numOfActiveOutSlaves++;
				AddMsSlaveParty(eAvMcuLinkSlaveOut, 1, eSD_Res, msSsrcRangeStart+10, pLocalSdesCap);

				if(maxConfResolutionType == eHD720_Res && m_AvMcuCascadeMode == eMsSvcVideoOptimize)
				{
					m_msOutSlaves[2] = new CMsPartyCntlInfo;
					m_msOutSlaves[2]->SetMaxResolution(eHD720_Res);
					m_msOutSlaves[2]->SetSsrcRangeStart(msSsrcRangeStart+20);
					m_numOfActiveOutSlaves++;
					AddMsSlaveParty(eAvMcuLinkSlaveOut, 2, eHD720_Res, msSsrcRangeStart+20, pLocalSdesCap);
				}
			}

			break;

		default:
			PASSERT(mainVideoResolutionType);
	}
	if(m_numOfActiveOutSlaves == 1)
	{
		TRACEINTO << "Connect out slaves - only main is connected as out in this conf settings, return all out slaves are connected";
		SendAllOutSlavesConnectedToMain();
	}
	else
	{
		// start timer
		StartTimer(CONNECT_MS_OUT_SLAVES_TIMER, CONNECT_MS_SLAVES_TOUT);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::AddMsSlaveParty(eAvMcuLinkType avMcuLinkType, DWORD slaveIndex, EVideoResolutionType slaveMaxResolution, DWORD msSsrcRangeStart, CVidModeH323 *pLocalSdesCap)  // from CSipPartyCntl::OnAddSlaveParty
{
	CRsrvParty* pSlaveParty = new CRsrvParty; AUTO_DELETE(pSlaveParty);
	pSlaveParty->SetNetInterfaceType(SIP_INTERFACE_TYPE);
	pSlaveParty->SetUndefinedType(UNRESERVED_PARTY);
	pSlaveParty->SetPartyId(0xFFFFFFFF);

	pSlaveParty->SetMaxResolution(slaveMaxResolution);

	if (m_pMainConfParty)
	{
		SetSlavePartyName(pSlaveParty, avMcuLinkType, slaveIndex);

		pSlaveParty->SetServiceProviderName(m_pMainConfParty->GetServiceProviderName());
		pSlaveParty->SetServiceId(m_pMainConfParty->GetServiceId());
		pSlaveParty->SetConnectionType(m_pMainConfParty->GetConnectionTypeOper());
		pSlaveParty->SetIpAddress(m_pMainConfParty->GetIpAddress());
		pSlaveParty->SetH323PartyAliasType(m_pMainConfParty->GetH323PartyAliasType());
		pSlaveParty->SetH323PartyAlias(m_pMainConfParty->GetH323PartyAlias());
		pSlaveParty->SetSipPartyAddress(m_pMainConfParty->GetSipPartyAddress());
		pSlaveParty->SetSipPartyAddressType(m_pMainConfParty->GetSipPartyAddressType());
		pSlaveParty->SetPreDefinedIvrString(m_pMainConfParty->GetPreDefinedIvrString()); // BRIDGE-4486

		PASSERT_AND_RETURN(!m_msOutSlaves[0]);

		if(  m_isEncrypt == FALSE  )
		{
			pSlaveParty->SetIsEncrypted(NO);
		}

		// Add party to the conference
		m_pTaskApi->SendAddMsSlavePartyToConf(m_msOutSlaves[0]->GetPartyRsrcId(), *pSlaveParty, avMcuLinkType, slaveIndex, msSsrcRangeStart, m_pSipRemoteCaps, pLocalSdesCap);

		// Update slave's connect state
		if ((eAvMcuLinkSlaveOut == avMcuLinkType) && (m_msOutSlaves[slaveIndex]))
			m_msOutSlaves[slaveIndex]->SetConnectState(eMsSlaveConnecting);

		if ((eAvMcuLinkSlaveIn == avMcuLinkType) && (m_msInSlaves[slaveIndex]))
			m_msInSlaves[slaveIndex]->SetConnectState(eMsSlaveConnecting);
	}
	else
		PTRACE(eLevelError, "CMsSlavesController::AddMsSlaveParty - pConfParty is NULL - can not SendAddMsSlavePartyToConf");


	PASSERT_AND_RETURN(!m_msOutSlaves[0]);
	TRACEINTO << "Main PartyRsrcId: " << m_msOutSlaves[0]->GetPartyRsrcId()
			  << ", slave added: " << pSlaveParty->GetName() << ", slave max resolution: " << eVideoResolutionTypeNames[slaveMaxResolution];

	POBJDELETE(pSlaveParty);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::SetSlavePartyName(CRsrvParty* pSlaveParty, eAvMcuLinkType avMcuLinkType, DWORD slaveIndex)
{
	PASSERT_AND_RETURN(!pSlaveParty);

	char *p_name = new char[H243_NAME_LEN];
	memset(p_name, '\0', H243_NAME_LEN);
	strncpy(p_name, m_pMainConfParty->GetName(), H243_NAME_LEN - sizeof("_xxx_xx") - 1);

	if (eAvMcuLinkSlaveOut == avMcuLinkType)
	{
		switch(slaveIndex)
		{
			case 1:
				strcat(p_name, "_out_01");
				break;
			case 2:
				strcat(p_name, "_out_02");
				break;
			default:
				PASSERT(slaveIndex);
		}
		if (m_msOutSlaves[slaveIndex])
			m_msOutSlaves[slaveIndex]->SetName(p_name);
	}
    else 	//eMsSlaveDirection_in
    {
		switch(slaveIndex)
		{
			case 1:
				strcat(p_name, "_in_01");
				break;
			case 2:
				strcat(p_name, "_in_02");
				break;
			case 3:
				strcat(p_name, "_in_03");
				break;
			case 4:
				strcat(p_name, "_in_04");
				break;
			default:
				PASSERT(slaveIndex);
		}
		if (m_msInSlaves[slaveIndex])
			m_msInSlaves[slaveIndex]->SetName(p_name);
    }

	pSlaveParty->SetName(p_name);

	PDELETEA(p_name);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::ConnectInSlaves(EVideoResolutionType mainVideoResolutionType)
{
	PASSERT_AND_RETURN(!m_msInSlaves[0]);
	TRACEINTO << "Main PartyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId();

	// Set main party's Max Resolution
	m_msInSlaves[0]->SetMaxResolution(mainVideoResolutionType);


	m_pSubscribedQueue = (COsQueue*)&(m_pTaskApi->GetRcvMbx());

	// Subscribe for add/update media video events
	EventPackage::Manager& lyncEventManager = EventPackage::Manager::Instance();
	lyncEventManager.AddSubscriber(m_msInSlaves[0]->GetPartyRsrcId(),
							     EventPackage::eEventType_MediaAdded,
								   std::make_pair(m_pSubscribedQueue, MS_LYNC_SLAVES_CONTROLLER_MSG));
	lyncEventManager.AddSubscriber(m_msInSlaves[0]->GetPartyRsrcId(),
							     EventPackage::eEventType_MediaStatusUpdated,
								   std::make_pair(m_pSubscribedQueue, MS_LYNC_SLAVES_CONTROLLER_MSG));

	// Calculate number of required in slaves according to the number of video parties in AV MCU conversation
	int numOfInSlaves = CalculateNumOfInSlavesRequired(m_msInSlaves[0]->GetPartyRsrcId());

	// If no in slaves should be created - send all in slaves connected (main is already connected)
	if (0 == numOfInSlaves)
	{
		SendAllInSlavesConnectedToMain();
		TRACEINTO << "Number of in slaves is 0 - send all slaves connected";
		return;
	}

	// Create in slaves parties
	for (int index = 1; index <= numOfInSlaves; index++)
	{
		m_msInSlaves[index] = new CMsPartyCntlInfo;
		m_msInSlaves[index]->SetMaxResolution(mainVideoResolutionType);
		m_numOfActiveInSlaves++;
		AddMsSlaveParty(eAvMcuLinkSlaveIn, index, mainVideoResolutionType);
	}

	// start timer
	StartTimer(CONNECT_MS_IN_SLAVES_TIMER, CONNECT_MS_SLAVES_TOUT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMsSlavesController::CalculateNumOfInSlavesRequired(PartyRsrcID mainPartyRsrcId)
{
	// get number of participants on AV MCU conversation from event package
	int numOfVideoPartiesInAvMcuConversation = GetNumOfVideoPartiesInAvMcuConversation(mainPartyRsrcId);
	int numOfInSlavesIncludingMain = (numOfVideoPartiesInAvMcuConversation < MAX_MS_IN_SLAVES) ? numOfVideoPartiesInAvMcuConversation: MAX_MS_IN_SLAVES;

	TRACEINTO << "num Of required In Slaves (including main): " << numOfInSlavesIncludingMain;
	int numOfInSlaves = (numOfInSlavesIncludingMain > 0) ? (numOfInSlavesIncludingMain - 1) : numOfInSlavesIncludingMain;		// does NOT include main party

	// limit the number of in slaves according to a system flag (does NOT include the main party)
	int maxNumOfInSlaves = (int)GetSystemCfgFlagInt<DWORD>(CFG_KEY_MAX_NUM_OF_MS_IN_SLAVES);
	if(m_maxConfBitRate == Xfer_128 || m_maxConfBitRate == Xfer_192 || m_maxConfBitRate == Xfer_256)
	{
	    TRACEINTO << "conf rate is below or equal to 256 -no in slaves only one decoder which is main ";
		maxNumOfInSlaves = 0;
	}

	if (numOfInSlaves > maxNumOfInSlaves)
		numOfInSlaves = maxNumOfInSlaves;

	return numOfInSlaves;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMsSlavesController::GetNumOfVideoPartiesInAvMcuConversation(PartyRsrcID mainPartyRsrcId)
{
	EventPackage::LyncMsiList videoMsis;
	EventPackage::ApiLync::Instance().GetMsiList(mainPartyRsrcId, videoMsis, EventPackage::eMediaType_Video);

	// check if local (RMX's) MSI is included
	LyncMsi localVideoMsi = EventPackage::ApiLync::Instance().GetCorrelativeMSI(mainPartyRsrcId, m_localAudioMsi, EventPackage::eMediaType_Video);

	TRACEINTO << "Main PartyRsrcId:" << mainPartyRsrcId << ", LocalAudioMsi:" << m_localAudioMsi << ", LocalVideoMsi:" << localVideoMsi;

	int numOfVideoPartiesInAvMcuConversation = 0;
	EventPackage::LyncMsiList::iterator _itr = std::find(videoMsis.begin(), videoMsis.end(), localVideoMsi);
	if (_itr != videoMsis.end()) 	// local (RMX's) Msi was found
	{
		numOfVideoPartiesInAvMcuConversation = videoMsis.size() - 1;
		TRACEINTO << "local (RMX's) Msi was found";
	}
	else 							// local (RMX's) Msi was NOT found
	{
		numOfVideoPartiesInAvMcuConversation = videoMsis.size();
		TRACEINTO << "local (RMX's) Msi was NOT found";
	}
	return numOfVideoPartiesInAvMcuConversation;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::HandleEvent(CSegment* pMsg, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::OnMsSlaveToMainAckMessage(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);

	OPCODE opcode = (DWORD)(-1);
	STATUS status = statIllegal;
	PartyRsrcID msSlavePartyRsrcId = (DWORD)(-1);
	eAvMcuLinkType avMcuLinkType = eAvMcuLinkNone;
	DWORD msSlaveIndex = (DWORD)(-1);

	*pMsg >> opcode;
	*pMsg >> status;
	*pMsg >> msSlavePartyRsrcId;
	*pMsg >> (DWORD&)avMcuLinkType;
	*pMsg >> msSlaveIndex;

	TRACEINTO << "ack's details:"
			  << "\n opcode: " << opcode
			  << "\n msSlavePartyRsrcId: " << msSlavePartyRsrcId
			  << "\n avMcuLinkType: " << eAvMcuLinkTypeNames[avMcuLinkType]
			  << "\n msSlaveIndex: " << msSlaveIndex
			  << "\n status: " << status;

	switch (opcode)
	{
	case ADDMSSLAVEPARTY:
		// Update PartyRsrcId And ConnectState + Send all slaves connected if needed
		HandleAddMsSlavePartyAck(avMcuLinkType, msSlaveIndex, msSlavePartyRsrcId, status);
		break;

	case DELMSSLAVEPARTY:
		// Delete slave from list + Send all slaves disconnected if needed
		HandleDeleteMsSlavePartyAck(avMcuLinkType, msSlaveIndex);
		break;

	default:
		TRACEINTO << "incorrect opcode - ack was received for opcode: " << opcode;
		PASSERT(1);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::HandleDeleteMsSlavePartyAck(eAvMcuLinkType avMcuLinkType, DWORD msSlaveIndex)
{
	// Delete slave from list (no need to check the status - delete is confirmed by partyCntl)
	switch (avMcuLinkType)
	{
	case eAvMcuLinkSlaveOut:
		PASSERT_AND_RETURN(!m_msOutSlaves[msSlaveIndex]);
		POBJDELETE(m_msOutSlaves[msSlaveIndex]);
		break;
	case eAvMcuLinkSlaveIn:
		PASSERT_AND_RETURN(!m_msInSlaves[msSlaveIndex]);
		POBJDELETE(m_msInSlaves[msSlaveIndex]);
		break;
	default:
		TRACEINTO << "incorrect slave party's type - eAvMcuLinkType received is: " << avMcuLinkType;
		PASSERT(1);
	}

	// Send all slaves disconnected if needed
	if (TRUE == AreAllSlavesDisconnected())
		SendAllSlavesDisconnectedToMain();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::HandleAddMsSlavePartyAck(eAvMcuLinkType avMcuLinkType, DWORD msSlaveIndex, PartyRsrcID msSlavePartyRsrcId, STATUS status)
{
	switch (avMcuLinkType)
	{
	case eAvMcuLinkSlaveOut:
		PASSERT_AND_RETURN(!m_msOutSlaves[msSlaveIndex]);
		m_msOutSlaves[msSlaveIndex]->SetPartyRsrcId(msSlavePartyRsrcId);
		///m_msOutSlaves[msSlaveIndex]->SetConnectStateAccordingToAck(status);
		if (statOK == status)
		{
			m_msOutSlaves[msSlaveIndex]->SetConnectState(eMsSlaveConnected);	// TBD: decide if it is critical to check that the state was connecting before ack
		}
		else	// failure status
		{
			POBJDELETE(m_msOutSlaves[msSlaveIndex]);
			m_numOfActiveOutSlaves--;
			//DEBUG
			TRACEINTO << "m_numOfActiveOutSlaves: " << m_numOfActiveOutSlaves << ", msSlaveIndex: " << msSlaveIndex;
			if (m_msOutSlaves[msSlaveIndex] != NULL)
				TRACEINTO << "m_msOutSlaves[msSlaveIndex] != NULL";
			else
				TRACEINTO << "m_msOutSlaves[msSlaveIndex] == NULL";
			//DEBUG
		}

		if (TRUE == AreAllOutSlavesConnected())
		{
			DeleteTimer(CONNECT_MS_OUT_SLAVES_TIMER);
			SendAllOutSlavesConnectedToMain();
		}
		break;

	case eAvMcuLinkSlaveIn:
		PASSERT_AND_RETURN(!m_msInSlaves[msSlaveIndex]);
		m_msInSlaves[msSlaveIndex]->SetPartyRsrcId(msSlavePartyRsrcId);
		///m_msInSlaves[msSlaveIndex]->SetConnectStateAccordingToAck(status);
		if (statOK == status)
		{
			m_msInSlaves[msSlaveIndex]->SetConnectState(eMsSlaveConnected);	// TBD: decide if it is critical to check that the state was connecting before ack
		}
		else
		{
			POBJDELETE(m_msInSlaves[msSlaveIndex]);
			m_numOfActiveInSlaves--;
		}

		if (TRUE == AreAllInSlavesConnected())
		{
			DeleteTimer(CONNECT_MS_IN_SLAVES_TIMER);
			SendAllInSlavesConnectedToMain();
		}
		break;

	default:
		TRACEINTO << "incorrect slave party's type - eAvMcuLinkType received is: " << avMcuLinkType;
		PASSERT(1);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CMsSlavesController::AreAllOutSlavesConnected()
{
	BOOL res = FALSE;
	WORD outSlavesConnectedCounter = 0;

	for (int i = 1; i < MAX_MS_OUT_SLAVES; i++)	// MAX_MS_OUT_SLAVES - includes the main
	{
		if ((m_msOutSlaves[i]) && (eMsSlaveConnected == m_msOutSlaves[i]->GetConnectState()))
			outSlavesConnectedCounter++;
	}

	PASSERT_AND_RETURN_VALUE(!m_msOutSlaves[0], FALSE);
	TRACEINTO << "Number of connected out slaves (not including main party) for main partyRsrcId " << m_msOutSlaves[0]->GetPartyRsrcId() <<" is: " << outSlavesConnectedCounter << " (expected: " << m_numOfActiveOutSlaves - 1 << " )";
	if (outSlavesConnectedCounter == m_numOfActiveOutSlaves - 1)		// outSlavesConnectedCounter - does NOT include the main
		res = TRUE;

	//DEBUG
	TRACEINTO << "res: "<< ((res == TRUE) ? "TRUE" : "FALSE");
	//DEBUG
	return res;
}

////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::SendAllOutSlavesConnectedToMain()
{
	PASSERT_AND_RETURN(!m_msOutSlaves[0]);
	TRACEINTO << "main partyRsrcId: " << m_msOutSlaves[0]->GetPartyRsrcId();

	///TBD - define statuses (???)
	STATUS status = statOK;

	// TBD - accumulated bandwidth (???)
	DWORD accBandwidth = m_totalBandwidth;

	mcMuxLync2013InfoReq msSvcMuxMsg;
	memset(&msSvcMuxMsg, 0, sizeof(mcMuxLync2013InfoReq));

	for (int i = 0; i < MAX_STREAM_MUX_LYNC_CONN; ++i)	///TBD - consider sending the number of active slaves
	{
		if (m_msOutSlaves[i])
		{
			SingleStreamDesc& muxDesc     = msSvcMuxMsg.txConnectedParties[i];
			muxDesc.bIsActive             = (eMsSlaveConnected == m_msOutSlaves[i]->GetConnectState());
			muxDesc.localSSRCRange[0]     = m_msOutSlaves[i]->GetSsrcRangeStart();
			muxDesc.localSSRCRange[1]     = m_msOutSlaves[i]->GetSsrcRangeStart() + 9;
			muxDesc.unPartyId             = m_msOutSlaves[i]->GetPartyRsrcId();
		}
	}

	PASSERT_AND_RETURN(!m_msOutSlaves[0]);
	m_pTaskApi->SendAllMsOutSlavesConnected(m_msOutSlaves[0]->GetPartyRsrcId(), status, accBandwidth, &msSvcMuxMsg);

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CMsSlavesController::AreAllInSlavesConnected()
{
	BOOL res = FALSE;
	WORD inSlavesConnectedCounter = 0;

	for (int i = 1; i < MAX_MS_IN_SLAVES; i++)	// MAX_MS_IN_SLAVES - includes the main
	{
		if ((m_msInSlaves[i]) && (eMsSlaveConnected == m_msInSlaves[i]->GetConnectState()))
			inSlavesConnectedCounter++;
	}

	TRACEINTO << "slavesConnectedCounter: " << inSlavesConnectedCounter;
	if (inSlavesConnectedCounter == m_numOfActiveInSlaves - 1)		// inSlavesConnectedCounter - does NOT include the main
		res = TRUE;

	return res;
}

////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::SendAllInSlavesConnectedToMain()
{
	PASSERT_AND_RETURN(!m_msInSlaves[0]);
	TRACEINTO << "main partyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId();

	///TBD - define statuses (???)
	STATUS status = statOK;

	m_pTaskApi->SendAllMsInSlavesConnected(m_msInSlaves[0]->GetPartyRsrcId(), status, m_numOfActiveInSlaves-1);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Note: this function considers the inactive slaves as exist
BOOL CMsSlavesController::AreAllSlavesDisconnected()
{
	BOOL res = FALSE;
	WORD inSlavesConnectedCounter = 0;
	WORD outSlavesConnectedCounter = 0;

	for (int i = 1; i < MAX_MS_IN_SLAVES; i++)	// MAX_MS_IN_SLAVES - includes the main
	{
		if ((m_msInSlaves[i]))
		{
			TRACEINTO << "in slaves: i = " << i << " connect state: "  << m_msInSlaves[i]->GetConnectState();
			inSlavesConnectedCounter++;
		}
	}

	for (int i = 1; i < MAX_MS_OUT_SLAVES; i++)	// MAX_MS_OUT_SLAVES - includes the main
	{
		if ((m_msOutSlaves[i]))
			outSlavesConnectedCounter++;
	}

	TRACEINTO << "inSlavesConnectedCounter: " << inSlavesConnectedCounter
			  << ", outSlavesConnectedCounter: " << outSlavesConnectedCounter;
	if ((inSlavesConnectedCounter == 0) && (outSlavesConnectedCounter == 0))	// slavesConnectedCounters - does NOT include the main
		res = TRUE;

	return res;

}


////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::SendAllSlavesDisconnectedToMain()
{
	PASSERT_AND_RETURN(!m_msInSlaves[0]);
	TRACEINTO << "main partyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId();

	///TBD - define statuses (???)
	STATUS status = statOK;

	m_pTaskApi->SendAllMsSlavesDeleted(m_msInSlaves[0]->GetPartyRsrcId(), status);

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::OnAvMcuVsrMsgInd(CSegment* pMsg)
{
	TRACEINTO << " VSR_FROM_AV_MCU_DEBUG 01";
	ST_VSR_SINGLE_STREAM vsr;
	pMsg->Get(reinterpret_cast<BYTE*>(&vsr), sizeof(ST_VSR_SINGLE_STREAM));

	CMsVsrMsg vsrMsg(vsr);
	vsrMsg.Dump();

	if(m_isVsrInProgress){
		if(NULL != m_pWaitingVsrMsg){
			POBJDELETE(m_pWaitingVsrMsg);
		}
		TRACEINTO << " m_isVsrInProgress is set - waiting for VSR treatment to end";
		m_isWaitingVsr = true;
		m_pWaitingVsrMsg = new CMsVsrMsg(vsrMsg);
	}else{
		m_isVsrInProgress = true;
		OnAvMcuVsrMsg(vsrMsg);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::OnAvMcuVsrMsg(CMsVsrMsg& vsrMsg)
{

	StartTimer(FULL_PACSI_INFO_TIMER,FULL_PACSI_INFO_TOUT);
	m_isVsrInProgress = true;
	ResolutionsSet encodersResolutionSet;
	for(WORD slavesIndex = 0;slavesIndex<MAX_MS_OUT_SLAVES;slavesIndex++)
	{
		if(NULL != m_msOutSlaves[slavesIndex]){
			encodersResolutionSet.insert(m_msOutSlaves[slavesIndex]->GetMaxResolution());
		}
	}

	ResolutionsEntriesMap selectedEntriesPerEncoders;
	vsrMsg.SelectBestEntries(encodersResolutionSet,selectedEntriesPerEncoders);

	if(0 == selectedEntriesPerEncoders.size()){
		TRACEINTO << " no entries found - update not needed";
	}

	for(WORD slavesIndex = 0;slavesIndex<MAX_MS_OUT_SLAVES;slavesIndex++)
	{
		BuildAndSendSingleVsrMsg(vsrMsg, slavesIndex,selectedEntriesPerEncoders);
	}
	StartPacsiInfoSync();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::BuildAndSendSingleVsrMsg(CMsVsrMsg& vsrMsg, WORD slaveIndex, ResolutionsEntriesMap& selectedEntriesPerEncoders)
{

	if(NULL != m_msOutSlaves[slaveIndex])
	{
		EVideoResolutionType encoder_resolution = m_msOutSlaves[slaveIndex]->GetMaxResolution() ;
		ResolutionsEntriesMap::iterator selectedEntryIterator = selectedEntriesPerEncoders.find(encoder_resolution);
		if(selectedEntryIterator == selectedEntriesPerEncoders.end()){
            CMsVsrMsg emptyVsrMsg(vsrMsg);
            emptyVsrMsg.SetEmptyVsr();
            m_pTaskApi->SendSingleVsrMsgInd(m_msOutSlaves[slaveIndex]->GetPartyRsrcId(),&(emptyVsrMsg.GetVsrSingleStreamStruct()));
            TRACEINTO << " out slave " << slaveIndex <<" no selected entry - send empty VSR";
            emptyVsrMsg.Dump();
            return;


		}


		CMsVsrMsg singleVsrMsg(vsrMsg);
		singleVsrMsg.SetSingleEntry((*selectedEntryIterator).second);

		TRACEINTO;
		singleVsrMsg.Dump();

		m_pTaskApi->SendSingleVsrMsgInd(m_msOutSlaves[slaveIndex]->GetPartyRsrcId(),&(singleVsrMsg.GetVsrSingleStreamStruct()));
	}
}
////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CMsSlavesController::OnAvMcuSendHandleSingleFecMsgToSlave(CSegment* pMsg)
{
	DWORD  mediaType, ssrc, newFecRedPercent;

	*pMsg >> mediaType >> ssrc >> newFecRedPercent;

	TRACEINTO << "LYNC2013_FEC_RED: mediaType:" << mediaType  << ", ssrc:" << ssrc  << ", newFecRedPercent:" << newFecRedPercent;


	for(WORD slavesIndex=0; slavesIndex<MAX_MS_OUT_SLAVES; slavesIndex++)
	{
		if (m_msOutSlaves[slavesIndex])
		{
			//TRACEINTO << "LYNC2013_FEC_RED: ssrc:" << m_msOutSlaves[slavesIndex]->GetSsrcRangeStart()
			//		  << ", rsrcId:" << m_msOutSlaves[slavesIndex]->GetPartyRsrcId() << ", name:" << m_msOutSlaves[slavesIndex]->GetName();
			if ( m_msOutSlaves[slavesIndex]->GetSsrcRangeStart() == ssrc )
				m_pTaskApi->SendSingleFecOrRedMsgFromSlavesControllerToPartyControl(m_msOutSlaves[slavesIndex]->GetPartyRsrcId(), mediaType, newFecRedPercent);
		}
	}
}
////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::OnAvMcuStreamsIntraReq(CSegment* pMsg)
{
	// PTRACE(eLevelInWORD encoders_maskfoNormal,"CSipParty::OnPartyRemoteH230");
	WORD encoders_mask = 0;
	WORD  opcode		= 0;
	WORD  videoSyncLost = 0;
	BYTE  bIsGradualIntra = FALSE;
	ERoleLabel eRole = kRolePeople;
	WORD tempRole;

	if(pMsg){
	 *pMsg >> encoders_mask;
	}

	bool sendToAll = (0xFF == encoders_mask) ? true : false;
	bool sendToCifEncoder = encoders_mask & 0x01;
	bool sendToSDEncoder = encoders_mask & 0x02;
	bool sendToHD720Encoder = encoders_mask & 0x04;

	TRACEINTO << "encoders_mask = " << encoders_mask << ", sendToAll = " << sendToAll << ", sendToCifEncoder = " << sendToCifEncoder << ", sendToSDEncoder = " << sendToSDEncoder << ", sendToHD720Encoder = " << sendToHD720Encoder;

	for(WORD slavesIndex = 0;slavesIndex<MAX_MS_OUT_SLAVES;slavesIndex++)
	{
		if(NULL != m_msOutSlaves[slavesIndex]){
			EVideoResolutionType resType = m_msOutSlaves[slavesIndex]->GetMaxResolution();
			if(sendToAll || (eCIF_Res==resType &&  sendToCifEncoder) || (eSD_Res==resType && sendToSDEncoder) || (eHD720_Res==resType && sendToHD720Encoder))
			{
				TRACEINTO << " forward RmtH230 to slave index: " << slavesIndex << " ,  PartyRsrcId = " << m_msOutSlaves[slavesIndex]->GetPartyRsrcId();

				CParty *pParty = GetLookupTableParty()->Get(m_msOutSlaves[slavesIndex]->GetPartyRsrcId());
				PASSERT_AND_RETURN(!pParty);
				CPartyApi* pPartyApi = new CPartyApi;
				pPartyApi->CreateOnlyApi(pParty->GetRcvMbx(), this);
				pPartyApi->SetLocalMbx(pParty->GetLocalQueue());

				CSegment seg;
				seg.CopySegmentFromReadPosition(*pMsg);

				// pSeg->DumpHex();
				pPartyApi->IpSingleIntraForAvMcu(&seg);
				POBJDELETE(pPartyApi);
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::OnAvMcuRmtH230(CSegment* pMsg)
{
	// since main party handle the fast_update, and forward, slaves controller send only to the slaves
	// TRACEINTO <<  " DEBUG_AVMCU_INTRA 02";

	for(WORD slavesIndex = 1;slavesIndex<MAX_MS_OUT_SLAVES;slavesIndex++)
	{
		if(NULL != m_msOutSlaves[slavesIndex]){
			TRACEINTO << " forward RmtH230 to slave index: " << slavesIndex << " ,  PartyRsrcId = " << m_msOutSlaves[slavesIndex]->GetPartyRsrcId();
			CParty *pParty = GetLookupTableParty()->Get(m_msOutSlaves[slavesIndex]->GetPartyRsrcId());
			PASSERT_AND_RETURN(!pParty);
			CPartyApi* pPartyApi = new CPartyApi;
			pPartyApi->CreateOnlyApi(pParty->GetRcvMbx(), this);
			pPartyApi->SetLocalMbx(pParty->GetLocalQueue());
			CSegment* pSeg = new CSegment(); AUTO_DELETE(pSeg);
			if(NULL != pMsg){
				pSeg->CopySegmentFromReadPosition(*pMsg);
			}
			// pSeg->DumpHex();
			pPartyApi->IpRmtH230(pSeg);
			POBJDELETE(pPartyApi);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::StartPacsiInfoSync()
{
	PASSERT_AND_RETURN(!m_msOutSlaves[0]);
	TRACEINTO << "main partyRsrcId: " << m_msOutSlaves[0]->GetPartyRsrcId();

	for(WORD out_slave_index=0;out_slave_index<MAX_MS_OUT_SLAVES;out_slave_index++)
	{
		if(m_msOutSlaves[out_slave_index]){
			m_msOutSlaves[out_slave_index]->SetWaitForPacsi(true);
		}
	}

	StartTimer(FULL_PACSI_INFO_TIMER,FULL_PACSI_INFO_TOUT);
}
////////////////////////////////////////////////////////////////////////////
void  CMsSlavesController::ResetWailForPacsi()
{
	for(WORD out_slave_index=0;out_slave_index<MAX_MS_OUT_SLAVES;out_slave_index++)
	{
		if(m_msOutSlaves[out_slave_index]){
			m_msOutSlaves[out_slave_index]->SetWaitForPacsi(false);
		}
	}
}
////////////////////////////////////////////////////////////////////////////
bool  CMsSlavesController::IsFullPacsiInfoSychCompleted()const
{
	for(WORD out_slave_index=0;out_slave_index<MAX_MS_OUT_SLAVES;out_slave_index++)
	{
		if(m_msOutSlaves[out_slave_index]){
			if(true == m_msOutSlaves[out_slave_index]->GetWaitForPacsi()){
				// found slave that still waiting for pacsi
				return false;
			}
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::OnMsSlaveSinglePacsiInfoInd(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);
	DWORD rsrcPartyId = 0;
	BYTE isReasonFecOrRed = FALSE;

	MsSvcParamsStruct singlePacsiInfo;
	*pMsg >> rsrcPartyId >> isReasonFecOrRed;
	pMsg->Get(reinterpret_cast<BYTE*>(&singlePacsiInfo), sizeof(MsSvcParamsStruct));

	TRACEINTO << " , rsrcPartyId:" << rsrcPartyId << ", isReasonFecOrRed:" << (DWORD)isReasonFecOrRed << "\n" << singlePacsiInfo;

	UpdateFullPacsiInfo(rsrcPartyId,singlePacsiInfo);

	if (isReasonFecOrRed == TRUE)
		SendFullPacsiInfoToOutSlaves(isReasonFecOrRed);
	else if(IsFullPacsiInfoSychCompleted())
			EndPacsiInfoSync();

}
////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::UpdateFullPacsiInfo(DWORD rsrcPartyId,MsSvcParamsStruct& singlePacsiInfo)
{
	bool slave_found = false;
	WORD foundSlaveIndex = (WORD)(-1);
	EVideoResolutionType slave_resolution = eAuto_Res;
	WORD pacsiInfoIndex = 0;
	for(WORD out_slave_index=0;out_slave_index<MAX_MS_OUT_SLAVES;out_slave_index++)
	{
		if(m_msOutSlaves[out_slave_index]){
			if(rsrcPartyId == m_msOutSlaves[out_slave_index]->GetPartyRsrcId()){
				slave_found = true;
				foundSlaveIndex = out_slave_index;
				slave_resolution = m_msOutSlaves[out_slave_index]->GetMaxResolution();
				switch(slave_resolution){
				case eCIF_Res:{
					pacsiInfoIndex = 0;
					break;
				}
				case eSD_Res:{
					pacsiInfoIndex = 1;
					break;
				}
				case eHD720_Res:{
					pacsiInfoIndex = 2;
					break;
				}
				default:{
					PASSERT((DWORD)(slave_resolution+1));
					slave_found = false;
					break;
				}
			}//switch
		}
		}
	}// for

	if(!slave_found){
		TRACEINTO << " slave not found for rsrcPartyId = " << rsrcPartyId;
		return;
	}
	if((m_msOutSlaves[foundSlaveIndex]) && (true == m_msOutSlaves[foundSlaveIndex]->GetWaitForPacsi()))
	{
		m_msOutSlaves[foundSlaveIndex]->SetWaitForPacsi(false);
	}
	else
	{
		// singlePacsiInfo received after timer jumped - update info , but don't send another update
		TRACEINTO << " singlePacsiInfo received to pacsi info index " << pacsiInfoIndex << " while WaitForPacsi is false";
		m_lastFullPacsiInfo.pacsiInfo[pacsiInfoIndex] = singlePacsiInfo;
		// singlePacsiInfo updated down
	}
	m_fullPacsiInfo.pacsiInfo[pacsiInfoIndex] = singlePacsiInfo;

}
////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::EndPacsiInfoSync()
{
	TRACEINTO << m_fullPacsiInfo;
	DeleteTimer(FULL_PACSI_INFO_TIMER);
	SendFullPacsiInfoToOutSlaves();
	ResetWailForPacsi();
	m_lastFullPacsiInfo = m_fullPacsiInfo;
	EndVsr();
}
////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::EndVsr()
{
	if(m_isWaitingVsr){
		TRACEINTO << " executing waiting VSR";
		m_isWaitingVsr = false;
		OnAvMcuVsrMsg(*m_pWaitingVsrMsg);
		POBJDELETE(m_pWaitingVsrMsg);
	}else{
		TRACEINTO;
		m_isVsrInProgress = false;
	}
}
////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::SendFullPacsiInfoToOutSlaves(BYTE isReasonFecOrRed)
{
	for(WORD out_slave_index=0;out_slave_index<MAX_MS_OUT_SLAVES;out_slave_index++)
	{
		if(m_msOutSlaves[out_slave_index]){
			m_pTaskApi->SendFullPacsiInd(m_msOutSlaves[out_slave_index]->GetPartyRsrcId(),&m_fullPacsiInfo,isReasonFecOrRed);
		}
	}
}
////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::SendFullPacsiInfoToOutSlavesExceptRsrcPartyId(DWORD rsrcPartyId)
{
	for(WORD out_slave_index=0;out_slave_index<MAX_MS_OUT_SLAVES;out_slave_index++)
	{
		if(m_msOutSlaves[out_slave_index] && (m_msOutSlaves[out_slave_index]->GetPartyRsrcId() != rsrcPartyId) ){
			m_pTaskApi->SendFullPacsiInd(m_msOutSlaves[out_slave_index]->GetPartyRsrcId(),&m_fullPacsiInfo);
		}
	}
}
////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::OnTimerFullPacsiInfo(CSegment* pMsg)
{
	// if we did not received pacsi info from one of the slaves - copy from
	for(WORD out_slave_index=0;out_slave_index<MAX_MS_OUT_SLAVES;out_slave_index++)
	{
		if(m_msOutSlaves[out_slave_index] && true == m_msOutSlaves[out_slave_index]->GetWaitForPacsi()){
			TRACEINTO << ": pacsi info from slave index" << out_slave_index << " not received";
			m_fullPacsiInfo.pacsiInfo[out_slave_index] = m_lastFullPacsiInfo.pacsiInfo[out_slave_index];
			m_msOutSlaves[out_slave_index]->SetWaitForPacsi(false);
		}
	}
	EndPacsiInfoSync();
}



////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::HandleEventPackageEvent(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!m_msInSlaves[0]);
	TRACEINTO << "Main PartyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId();
	PASSERT_AND_RETURN(!pMsg);

	// BRIDGE-12019 - handle SpotLight speaker connection when all 5 in slaves are already active
	LyncMsi spotLightSpeakerVideoMsi = GetSpotLightSpeakerMsi();
	BOOL spotLightSpeakerMediaAdded = FALSE;

	// If maximum number of slaves are already connected (+ not in SpotLight mode) - nothing to be done
	int maxNumOfInSlaves = (int)GetSystemCfgFlagInt<DWORD>(CFG_KEY_MAX_NUM_OF_MS_IN_SLAVES);

	if(m_maxConfBitRate == Xfer_128 || m_maxConfBitRate == Xfer_192 || m_maxConfBitRate == Xfer_256)
	{
		TRACEINTO << "conf rate is below or equal to 256 -no in slaves only one decoder which is main ";
		maxNumOfInSlaves = 0;
	}

	if ( (maxNumOfInSlaves != 0) && (m_numOfActiveInSlaves - 1 == maxNumOfInSlaves)		// m_numOfActiveInSlaves includes the main and maxNumOfInSlaves doesn't
			&& (0 == spotLightSpeakerVideoMsi))
	{
		TRACEINTO << "maximum number of slaves are already connected (+ not in SpotLight mode) - nothing to be done";
		return;
	}

	//Get the notify container, check if there are any add/update media video notifications
	EventPackage::Events tempEvents;
	*pMsg  >> tempEvents;

	TRACEINTO << ", PartyId:" <<  tempEvents.m_id << tempEvents;


	PartyRsrcID partyId = tempEvents.m_id;
	PartyRsrcID partyImageRsrcId;
	WORD numOfAddedMediaVideoOnAVMCUSide = 0;

	for (EventPackage::Events::iterator _itr = tempEvents.begin(); _itr != tempEvents.end(); ++_itr)
	{
		EventPackage::EventType event = _itr->first;
		switch (event)
		{
			case EventPackage::eEventType_MediaAdded:
			{
				EventPackage::EventMediaAdded* pLyncEventMediaAdded = (EventPackage::EventMediaAdded*)_itr->second;
				if (pLyncEventMediaAdded->value.type != EventPackage::eMediaType_Video)
					continue;
				numOfAddedMediaVideoOnAVMCUSide++;

				if (0 != spotLightSpeakerVideoMsi)
				{	// check if the current media added is the SpotLight speaker's
					spotLightSpeakerMediaAdded = IsSpotLightSpeakerMediaAdded(pLyncEventMediaAdded->value.msi, spotLightSpeakerVideoMsi);
				}

				break;
			}
			case EventPackage::eEventType_MediaStatusUpdated:
			{
				EventPackage::EventMediaStatusUpdated* pLyncEventMediaUpdatedStatus = (EventPackage::EventMediaStatusUpdated*)_itr->second;
				if (pLyncEventMediaUpdatedStatus->value.type != EventPackage::eMediaType_Video)
				{
					if(pLyncEventMediaUpdatedStatus->value.type == EventPackage::eMediaType_Audio && pLyncEventMediaUpdatedStatus->value.msi == m_localAudioMsi)
					{
						UpdateBlockAudioState(pLyncEventMediaUpdatedStatus->value.new_val);
					}
					continue;
				}
				if(pLyncEventMediaUpdatedStatus->value.new_val != EventPackage::eMediaStatusType_SendOnly && pLyncEventMediaUpdatedStatus->value.new_val != EventPackage::eMediaStatusType_SendRecv)
					continue;
				numOfAddedMediaVideoOnAVMCUSide++;

				if (0 != spotLightSpeakerVideoMsi)
				{	// check if the current media added is the SpotLight speaker's
					spotLightSpeakerMediaAdded = IsSpotLightSpeakerMediaAdded(pLyncEventMediaUpdatedStatus->value.msi, spotLightSpeakerVideoMsi);
				}

				break;
			}
			default:
			{
				TRACEINTO << " Lync event is not supported by Slaves Controller, event is: "
						  << event
						  << " Main partyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId();
				continue;
			}
		}
	}

	// If there are some add media video notifications - add in slaves as (/if) required
	if (0 != numOfAddedMediaVideoOnAVMCUSide)
	{
		// Calculate number of required in slaves according to the number of video parties in AV MCU conversation
		int numOfReqiredInSlaves = CalculateNumOfInSlavesRequired(m_msInSlaves[0]->GetPartyRsrcId());

		// Compare the number of required in slaves with the current number of active in slaves
		int numOfActiveInSlavesWithoutMain = m_numOfActiveInSlaves -1;
		int numOfInSlavesToAdd = numOfReqiredInSlaves - numOfActiveInSlavesWithoutMain;

		// If no in slave should exist (only main) - send all in slaves connected (main is already connected)
		if (0 == numOfReqiredInSlaves)
		{
			SendAllInSlavesConnectedToMain();
			TRACEINTO << "Number of in slaves is 0 - send all slaves connected";
			return;
		}

		// BRIDGE-12019 - handle SpotLight speaker connection when all 5 (max) in slaves are already active
		if ((0 == numOfInSlavesToAdd) && (spotLightSpeakerMediaAdded))
		{
			SendAllInSlavesConnectedToMain();
			TRACEINTO << "SpotLight speaker is being connected while max in slaves are already connected - send all slaves connected";
			return;
		}

		int index = -1;
		EFreeOrInactiveMsSlave isFreeOrInactiveSlave = eFreeMsSlave;
		for (int i = 1; i <= numOfInSlavesToAdd; i++)
		{
			index = FindFirstFreeIndexInSlavesArray(isFreeOrInactiveSlave);
			if (-1 == index)
			{
				TRACEINTO << "no index was found for adding/activating in slave";
			}
			else	// index found
			{
				TRACEINTO << "index found: " << index << ((eFreeMsSlave == isFreeOrInactiveSlave) ? " - free slave" : " - inactive slave");

				if (eFreeMsSlave == isFreeOrInactiveSlave)
				{
					if ((index > 0) && (index < MAX_MS_IN_SLAVES) && (NULL == m_msInSlaves[index]))
					{
						TRACEINTO << "Adding in slave, index #" << index;
						m_msInSlaves[index] = new CMsPartyCntlInfo;
						m_msInSlaves[index]->SetMaxResolution(m_msInSlaves[0]->GetMaxResolution());
						m_numOfActiveInSlaves++;
						AddMsSlaveParty(eAvMcuLinkSlaveIn, index, m_msInSlaves[0]->GetMaxResolution());
						// start timer
						StartTimer(CONNECT_MS_IN_SLAVES_TIMER, CONNECT_MS_SLAVES_TOUT);
					}
					else
					{
						TRACEINTO << "not an appropriate index for adding a new in slave,  index: " << index;
					}
				}
				else 	// eInactiveMsSlave == isFreeOrInactiveSlave
				{
					if (m_msInSlaves[index]->GetConnectState() == eMsSlaveInactive)
					{
						TRACEINTO << "Activating in slave, index #" << index;
						m_msInSlaves[index]->SetConnectState(eMsSlaveConnected);
						m_numOfActiveInSlaves++;
						SendAllInSlavesConnectedToMain();
					}
					else
					{
						TRACEINTO << "not an appropriate index for activating in slave,  index: " << index;
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
LyncMsi CMsSlavesController::GetSpotLightSpeakerMsi()
{
	LyncMsi spotLightSpeakerVideoMsi = 0;
	PASSERT_AND_RETURN_VALUE(!m_msInSlaves[0], spotLightSpeakerVideoMsi);
	std::string userSpotLightID = EventPackage::ApiLync::Instance().GetSpotlightUserId(m_msInSlaves[0]->GetPartyRsrcId());
	if(0 != userSpotLightID.size())
	{
		spotLightSpeakerVideoMsi = EventPackage::ApiLync::Instance().GetUserMSI(m_msInSlaves[0]->GetPartyRsrcId(), userSpotLightID, EventPackage::eMediaType_Video);
		TRACEINTO << "SpotLight Speaker Video Msi is: " << spotLightSpeakerVideoMsi << ", main PartyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId();
		if (!IsValidMSI(spotLightSpeakerVideoMsi))
		{
			TRACEINTO << " SpotLight Speaker does not have a valid Msi, Msi: " << spotLightSpeakerVideoMsi << ", main PartyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId();
			spotLightSpeakerVideoMsi = 0;
		}
	}
	else
	{
		TRACEINTO << " SpotLight User ID is empty, main PartyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId();
		spotLightSpeakerVideoMsi = 0;
	}


	return spotLightSpeakerVideoMsi;
}

////////////////////////////////////////////////////////////////////////////
BOOL CMsSlavesController::IsValidMSI(LyncMsi msi)
{
	if (msi > 0 && msi != SOURCE_NONE_DOMINANT_SPEAKER_MSI && msi != SOURCE_ANY_DOMINANT_SPEAKER_MSI && msi != DUMMY_DOMINANT_SPEAKER_MSI)
		return TRUE;
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CMsSlavesController::IsSpotLightSpeakerMediaAdded(LyncMsi videoMsi, LyncMsi spotLightSpeakerVideoMsi)
{
	BOOL bSpotLightSpeakerMediaAdded = FALSE;
	if (spotLightSpeakerVideoMsi == videoMsi)
		bSpotLightSpeakerMediaAdded = TRUE;
	return bSpotLightSpeakerMediaAdded;
}


////////////////////////////////////////////////////////////////////////////
int CMsSlavesController::FindFirstFreeIndexInSlavesArray(EFreeOrInactiveMsSlave& isFreeOrInactiveSlave)
{
	for (int index = 1; index < MAX_MS_IN_SLAVES; index++)
	{
		if ((m_msInSlaves[index]) && (m_msInSlaves[index]->GetConnectState() == eMsSlaveInactive))	//TBD: what about disconnecting state????
		{
			isFreeOrInactiveSlave = eInactiveMsSlave;
			return index;
		}
		if (NULL == m_msInSlaves[index])
		{
			isFreeOrInactiveSlave = eFreeMsSlave;
			return index;
		}
	}
	// no free or inactive index was found
	return -1;
}


////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::OnDelMsSlaveParty(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);
	PartyRsrcID deletedSlavePartyRsrcId = (DWORD)(-1);
	*pMsg  >> deletedSlavePartyRsrcId;

	// Find slave according to rsrcId
	eAvMcuLinkType avMcuLinkType = eAvMcuLinkNone;
	int slaveIndex = -1;

	GetSlaveByRsrcId(deletedSlavePartyRsrcId, avMcuLinkType, slaveIndex);
	TRACEINTO << "Slave to be deleted:"
			  << "\nPartyRsrcId: " << deletedSlavePartyRsrcId
			  << "\nAvMcuLinkType: " << avMcuLinkType
			  << "\nslaveIndex: " << slaveIndex;

	switch (avMcuLinkType)
	{
	case eAvMcuLinkSlaveIn:
		if (m_disableInSlavesDynamicRemoval)
			InactivateInSlave(slaveIndex);
		else
			DeleteInSlave(slaveIndex);
		break;
	case eAvMcuLinkSlaveOut:
		DeleteOutSlave(slaveIndex);
		break;
	default:
		TRACEINTO << "Slave was not found";
	}

}

////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::GetSlaveByRsrcId(PartyRsrcID partyRsrcId, eAvMcuLinkType& avMcuLinkType, int& slaveIndex)
{
	for (int index = 1; index < MAX_MS_IN_SLAVES; ++index)
	{
		if ((m_msInSlaves[index]) && (partyRsrcId == m_msInSlaves[index]->GetPartyRsrcId()))
		{
			avMcuLinkType = eAvMcuLinkSlaveIn;
			slaveIndex = index;
			return;
		}
	}

	for (int index = 1; index < MAX_MS_OUT_SLAVES; ++index)
	{
		if ((m_msOutSlaves[index]) && (partyRsrcId == m_msOutSlaves[index]->GetPartyRsrcId()))
		{
			avMcuLinkType = eAvMcuLinkSlaveOut;
			slaveIndex = index;
			return;
		}
	}
	// slave was not found
	avMcuLinkType = eAvMcuLinkNone;
	slaveIndex = -1;
}


////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::DeleteInSlave(int slaveIndex)
{
	if ((slaveIndex > 0) && (slaveIndex < MAX_MS_IN_SLAVES) &&
		(m_msInSlaves[slaveIndex]))
	{
		PASSERT_AND_RETURN(!m_msInSlaves[0]);
		TRACEINTO << "main partyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId()
				  << ", in slave index to be deleted: " << slaveIndex;

		const char* slavePartyName = m_msInSlaves[slaveIndex]->GetName();
		m_msInSlaves[slaveIndex]->SetConnectState(eMsSlaveDisconnecting);
		m_numOfActiveInSlaves--;

		// Delete party from conference
	    m_pTaskApi->DropParty(slavePartyName, 0 /*delete*/);
	}
	else
		TRACEINTO << "Fail to delete in slave (wrong slave index), slave index: " << slaveIndex;

}


////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::DeleteOutSlave(int slaveIndex)
{
	if ((slaveIndex > 0) && (slaveIndex < MAX_MS_OUT_SLAVES) &&
		(m_msOutSlaves[slaveIndex]))
	{
		PASSERT_AND_RETURN(!m_msOutSlaves[0]);
		TRACEINTO << "main partyRsrcId: " << m_msOutSlaves[0]->GetPartyRsrcId()
				  << ", out slave index to be deleted: " << slaveIndex;

		const char* slavePartyName = m_msOutSlaves[slaveIndex]->GetName();
		m_msOutSlaves[slaveIndex]->SetConnectState(eMsSlaveDisconnecting);
		m_numOfActiveOutSlaves--;

		// Delete party from conference
	    m_pTaskApi->DropParty(slavePartyName, 0 /*delete*/);
	}
	else
		TRACEINTO << "Fail to delete out slave (wrong slave index), slave index: " << slaveIndex;

}


////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::InactivateInSlave(int slaveIndex)
{
	if ((slaveIndex > 0) && (slaveIndex < MAX_MS_IN_SLAVES) &&
		(m_msInSlaves[slaveIndex]) && (m_msInSlaves[slaveIndex]->GetConnectState() != eMsSlaveInactive))
	{
		PASSERT_AND_RETURN(!m_msInSlaves[0]);
		TRACEINTO << "main partyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId()
				  << ", in slave index to be inactivated: " << slaveIndex;

		m_msInSlaves[slaveIndex]->SetConnectState(eMsSlaveInactive);
		m_numOfActiveInSlaves--;
	}
	else
		TRACEINTO << "Fail to inactivate in slave (wrong slave index or already inactive), slave index: " << slaveIndex;
}


//////////////////////////////////////////////////////////////////////////////
//void CMsSlavesController::InactivateOutSlave(int slaveIndex)
//{
//	if ((slaveIndex > 0) && (slaveIndex < MAX_MS_IN_SLAVES) &&
//		(m_msOutSlaves[slaveIndex]) && (m_msOutSlaves[slaveIndex]->GetConnectState() != eMsSlaveInactive))
//	{
//		PASSERT_AND_RETURN(!m_msOutSlaves[0]);
//		TRACEINTO << "main partyRsrcId: " << m_msOutSlaves[0]->GetPartyRsrcId()
//				  << ", out slave index to be inactivated: " << slaveIndex;
//
//		m_msOutSlaves[slaveIndex]->SetConnectState(eMsSlaveInactive);
//		m_numOfActiveOutSlaves--;
//	}
//	else
//		TRACEINTO << "Fail to inactivate out slave (wrong slave index or already inactive), slave index: " << slaveIndex;
//}


////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::DeleteAllSlaves()
{
	PASSERT_AND_RETURN(!m_msInSlaves[0]);
	TRACEINTO << " main PartyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId();

	// remove subscription to add media video events
	EventPackage::Manager& lyncEventManager = EventPackage::Manager::Instance();
	lyncEventManager.DelSubscriber(m_msInSlaves[0]->GetPartyRsrcId(),
								   EventPackage::eEventType_MediaAdded,
								   std::make_pair(m_pSubscribedQueue, MS_LYNC_SLAVES_CONTROLLER_MSG));
	lyncEventManager.DelSubscriber(m_msInSlaves[0]->GetPartyRsrcId(),
								   EventPackage::eEventType_MediaStatusUpdated,
								   std::make_pair(m_pSubscribedQueue, MS_LYNC_SLAVES_CONTROLLER_MSG));

	// delete in slaves
	for (int index = 1; index < MAX_MS_IN_SLAVES; ++index)
	{
		DeleteInSlave(index);
	}
	// delete out slaves
	for (int index = 1; index < MAX_MS_OUT_SLAVES; ++index)
	{
		DeleteOutSlave(index);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::OnConnectMsOutSlavesTout(CSegment* pParam)
{
	PASSERT_AND_RETURN(!m_msOutSlaves[0]);
	TRACEINTO << "main partyRsrcId: " << m_msOutSlaves[0]->GetPartyRsrcId();

	for (int i = 1; i < MAX_MS_OUT_SLAVES; i++)	// MAX_MS_OUT_SLAVES - includes the main
	{
		if ((m_msOutSlaves[i]) && (eMsSlaveConnecting == m_msOutSlaves[i]->GetConnectState()))
		{
			// disconnect slaves that stuck in connecting state
			DeleteOutSlave(i);
		}
	}

	SendAllOutSlavesConnectedToMain();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::OnConnectMsInSlavesTout(CSegment* pParam)
{
	PASSERT_AND_RETURN(!m_msInSlaves[0]);
	TRACEINTO << "main partyRsrcId: " << m_msInSlaves[0]->GetPartyRsrcId();

	for (int i = 1; i < MAX_MS_IN_SLAVES; i++)	// MAX_MS_IN_SLAVES - includes the main
	{
		if ((m_msInSlaves[i]) && (eMsSlaveConnecting == m_msInSlaves[i]->GetConnectState()))
		{
			// disconnect slaves that stuck in connecting state
			DeleteInSlave(i);
		}
	}

	SendAllInSlavesConnectedToMain();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMsSlavesController::UpdateBlockAudioState(EventPackage::MediaStatusType eMediaTypeStatus)
{
	EOnOff eOnOff = eOff;
	if(eMediaTypeStatus == EventPackage::eMediaStatusType_RecvOnly)
	{
		eOnOff = eOn;
	}
	const char* pPartyName = m_pMainConfParty->GetName();
	DWORD mediaMask = 0x00000001;
	TRACEINTO<< "eOnOff " <<eOnOff;
	m_pTaskApi->BlockMedia(pPartyName,eOnOff,mediaMask);
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
//                        CMsSlaveInfo
////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMsPartyCntlInfo::CMsPartyCntlInfo()
{
	memset(m_name, 0, sizeof(m_name));
	m_partyRsrcId = 0;
	m_maxResolution = eAuto_Res;
	m_ssrcRangeStart = (DWORD)(-1);
	m_connectState = eMsSlaveIdle;
	m_waitForPacsi = false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMsPartyCntlInfo::~CMsPartyCntlInfo()
{
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMsPartyCntlInfo::CMsPartyCntlInfo(const CMsPartyCntlInfo& other): CPObject(other)
{
	strcpy_safe(m_name, other.m_name);
	m_partyRsrcId = other.m_partyRsrcId;
	m_maxResolution = other.m_maxResolution;
	m_ssrcRangeStart = other.m_ssrcRangeStart;
	m_connectState = other.m_connectState;
	m_waitForPacsi = other.m_waitForPacsi;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMsPartyCntlInfo& CMsPartyCntlInfo::operator=(const CMsPartyCntlInfo& other)
{
	if(this != &other)
	{
		strcpy_safe(m_name, other.m_name);
		m_partyRsrcId = other.m_partyRsrcId;
		m_maxResolution = other.m_maxResolution;
		m_ssrcRangeStart = other.m_ssrcRangeStart;
		m_connectState = other.m_connectState;
		m_waitForPacsi = other.m_waitForPacsi;
	}
	return *this;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
DWORD	CMsPartyCntlInfo::GetEncoderMaxVsrResolution()const
{

	DWORD max_encoder_resolution = 0;
	switch(m_maxResolution)
	{
	case eCIF_Res:{
//		max_encoder_resolution = 352*288;
		max_encoder_resolution = 320*320; // to adjust to MS
		break;
	}
	case eSD_Res:{
		max_encoder_resolution = 800*600;
		break;
	}
	case eHD720_Res:{
		max_encoder_resolution = 1280*720;
		break;
	}
	case eHD1080_Res:
	case eHD1080p60_Res:
	{
		max_encoder_resolution = 1920*1088;
		break;
	}
	default:{
		//		max_encoder_resolution = 352*288;
		max_encoder_resolution = 320*320; // to adjust to MS
		break;
	}
	}

	return max_encoder_resolution;
}
*/







