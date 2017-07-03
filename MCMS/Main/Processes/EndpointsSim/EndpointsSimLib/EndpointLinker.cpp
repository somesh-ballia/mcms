#include "EndpointLinker.h"
#include "EpSimEndpointsList.h"
#include "H323CsReq.h"
#include "MplMcmsProtocol.h"
#include "HostCommonDefinitions.h"
#include "IpCsOpcodes.h"
#include "Segment.h"
#include "TaskApi.h"
#include "EpSimCapSetsList.h"
#include "TraceStream.h"
#include "CSSimTaskApi.h"
#include "OpcodesMcmsInternal.h"


////////////////////////////////////////////////////////////////////////////////
CEndpointLinker::CEndpointLinker( CTaskApi * /*pCSApi*/, const CCapSet& rCap, const CH323Behavior& rBehav )
	:CEndpointH323( NULL /*pCSApi*/, rCap, rBehav )
	,m_callerConfID(0)
	,m_callerPartyID(0)
	,m_callerConnectionID(LOBBY_CONNECTION_ID)
	,m_callerCsCallIndex(CEndpointsList::GenerateCallIndex())
	,m_callerCsHandle(5)
	,m_callerCsSrcUnit(eCentral_signaling)
	,m_callerTermType(-1)
	,m_calleeTermType(-1)
{
}

////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnCsCallSetupReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::OnCsCallSetupReq - H323_CS_SIG_CALL_SETUP_REQ");

	SetState(eEpStateConnecting);

	// sign the API command
	m_apiArray[SIM_H323_RCV_SETUP]++;

	// gets the req struct
	mcReqCallSetup *req = (mcReqCallSetup*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointLinker::OnCsCallSetupReq - GetData is NULL ");
		return;
	}

	APIU32 lengthStructure = sizeof(mcReqCallSetupBase);
//	lengthStructure += (sizeof(encTokensHeaderStruct) - sizeof(encTokensHeaderBasicStruct));

	// save the req struct
	memcpy( &m_mcReqCallSetup, req, lengthStructure );

	BYTE* ptr = (BYTE*)m_pEncryptionStruct;
	PDELETEA( ptr );
	m_pEncryptionStruct = NULL;
	if( req->encryTokens.numberOfTokens > 0 )
	{
		APIU32 tokensLen = sizeof(encTokensHeaderBasicStruct) + req->encryTokens.dynamicTokensLen;
		m_pEncryptionStruct = (encTokensHeaderStruct*) new BYTE[tokensLen];
		memcpy( m_pEncryptionStruct, &req->encryTokens, tokensLen );
	}

	// checks legal parameters
	IsLeagaParametersCallSetup( req );

	/* v4.1c <--> V6 merge temp comments
	// retrieve the IP and the port
	RetrieveIPFromString( 0 );	// src
	RetrieveIPFromString( 1 );	// dest
	end of merge temp comments*/

	// set H225 Local ip+port (RMX)
	/////////////////////////////////
	m_H225LocalIpAddress.port = 6005 + (10 * m_arrayIndex);
	if (m_mcReqCallSetup.srcIpAddress.transAddr.port != 0)
		m_H225LocalIpAddress.port = m_mcReqCallSetup.srcIpAddress.transAddr.port;
	m_H225LocalIpAddress.transportType = m_mcReqCallSetup.srcIpAddress.transAddr.transportType;
	m_H225LocalIpAddress.distribution = m_mcReqCallSetup.srcIpAddress.transAddr.distribution;
	m_H225LocalIpAddress.ipVersion = m_mcReqCallSetup.srcIpAddress.transAddr.ipVersion;
	m_H225LocalIpAddress.addr = m_mcReqCallSetup.srcIpAddress.transAddr.addr;


	// set H225 Remote ip+port (E.P.)
	/////////////////////////////////
	m_H225RemoteIpAddress.port = 6006 + (10 * m_arrayIndex);
	if (m_mcReqCallSetup.destIpAddress.transAddr.port != 0)
		m_H225RemoteIpAddress.port = m_mcReqCallSetup.destIpAddress.transAddr.port;
	m_H225RemoteIpAddress.transportType = m_mcReqCallSetup.destIpAddress.transAddr.transportType;
	m_H225RemoteIpAddress.distribution = m_mcReqCallSetup.destIpAddress.transAddr.distribution;
	m_H225RemoteIpAddress.ipVersion = m_mcReqCallSetup.destIpAddress.transAddr.ipVersion;
	m_H225RemoteIpAddress.addr = m_mcReqCallSetup.destIpAddress.transAddr.addr;

	SetIpV6Address();


	// send indication
//	SendCsCallRingBackInd( pMplProtocol );

	//trans send indication
	NotifyCallOfferingInd(req);
}

////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::NotifyCallOfferingInd(mcReqCallSetup *setup)
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::NotifyCallOfferingInd - H323_CS_SIG_CALL_OFFERING_IND");
	/* v4.1c <--> V6 merge temp comment send of merge temp comments*/
	// create basic ind
	// ================
	size_t indLen=0;
	BYTE* pIndBytes = CreateIndicationAndEncryToken(sizeof(mcIndCallOffering), setup->encryTokens, indLen);
	mcIndCallOffering *ind = (mcIndCallOffering*)pIndBytes;

	//Translate message
	ind->bH245Establish=1;
	ind->bIsActiveMc=setup->bIsActiveMc;
	ind->bIsOrigin=0;
	memcpy(ind->callId, setup->callId, sizeof(ind->callId));
	ind->conferenceGoal=setup->callGoal;
	memcpy(ind->conferenceId, setup->conferenceId, sizeof(ind->conferenceId));
	memcpy(ind->destPartyAliases, setup->destPartyAliases, sizeof(ind->destPartyAliases));
	FillMcTransportAddress( &ind->h245IpAddress, eIpVersion4, m_H225LocalIpAddress.addr.v4.ip/*m_IpH225Local*/,
						m_H225LocalIpAddress.port/*m_portH225Local*/, eDistributionUnicast, eTransportTypeTcp);
	//ind->indexTblAuth;//zero ?
	ind->localH225Port = m_H225LocalIpAddress.port/*m_portH225Local*/;
	ind->rate=setup->maxRate;
	ind->referenceValue=setup->referenceValue;
	memcpy(ind->sDisplay, setup->callTransient.sDisplay, sizeof(ind->sDisplay));
	ind->srcEndpointType=setup->localEndpointType;
	memcpy(ind->srcPartyAliases, setup->srcPartyAliases, sizeof(ind->srcPartyAliases));
	ind->type=setup->type;
	memcpy(ind->userUser, setup->callTransient.userUser, sizeof(ind->userUser));
	ind->userUserSize=setup->callTransient.userUserSize;

	ind->srcIpAddress=setup->srcIpAddress;
	ind->destIpAddress=setup->destIpAddress;

	// send the struct to CS Api
	// ================================
	CCSSimTaskApi api(GetCSID());
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol;

	//set caller mpl header
	pCSProt->AddPortDescriptionHeader(m_callerPartyID,m_callerConfID,m_callerConnectionID,ePhysical_art_light);
	pCSProt->AddCSHeader(m_callerCsHandle,eMcms,m_callerCsSrcUnit);
	::FillCsProtocol(pCSProt, api.GetCSID(),
			         H323_CS_SIG_CALL_OFFERING_IND,
			         (BYTE*)ind, indLen, m_callerCsCallIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndBytes);
}

////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnCallAnswerReq( CMplMcmsProtocol *pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::OnCallAnswerReq - H323_CS_SIG_CALL_ANSWER_REQ");

	// update port description details
	m_callerConfID       = pMplProtocol->getPortDescriptionHeaderConf_id();
	m_callerPartyID      = pMplProtocol->getPortDescriptionHeaderParty_id();
	m_callerConnectionID = pMplProtocol->getPortDescriptionHeaderConnection_id();
	m_callerCsHandle    = pMplProtocol->getCentralSignalingHeaderCsId();
	m_callerCsSrcUnit   = pMplProtocol->getCentralSignalingHeaderDestUnitId();
	//m_callerCsCallIndex=pMplProtocol->getCentralSignalingHeaderCallIndex();

	// sign the API command
	m_apiArray[SIM_H323_RCV_CALL_ANSWER]++;

	// gets the req struct
	mcReqCallAnswer *req = (mcReqCallAnswer*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointLinker::OnCallAnswerReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_mcReqCallAnswer, req, sizeof(mcReqCallAnswer) );

	BYTE* ptr = (BYTE*)m_pEncryptionStruct;
	PDELETEA( ptr );
	m_pEncryptionStruct = NULL;
	if( req->encryTokens.numberOfTokens > 0 )
	{
		APIU32 tokensLen = sizeof(encTokensHeaderBasicStruct) + req->encryTokens.dynamicTokensLen;
		m_pEncryptionStruct = (encTokensHeaderStruct*) new BYTE[tokensLen];
		memcpy( m_pEncryptionStruct, &req->encryTokens, tokensLen );
	}
	/*
	cmTransportAddress1			h245Address = local H245 address;		//The new RV stack struct
	*/

	// save some important parameters
	m_conferenceType = req->conferenceType;	// video type: CP, VSW, ...
	m_maxRate = req->maxRate;

	// send indication
	SendCsCallConnectedInd( pMplProtocol );

	// send indication
	NotifyCallConnInd( req );
}

////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::SendCsCallConnectedInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::SendCsCallConnectedInd - H323_CS_SIG_CALL_CONNECTED_IND");

	// create basic ind
	// ================
//	mcIndCallConnected *ind = new mcIndCallConnected;
	WORD indLen = 0;
	if( m_pEncryptionStruct == NULL )
	{
		indLen = sizeof(mcIndCallConnected);
	}
	else
	{
		indLen = sizeof(mcIndCallConnected) + m_pEncryptionStruct->dynamicTokensLen;
	}
	BYTE*  pIndBytes = new BYTE [indLen];
	memset( pIndBytes, 0, indLen);	// zeroing the struct
	mcIndCallConnected *ind = (mcIndCallConnected*)pIndBytes;

	if( m_pEncryptionStruct != NULL )
	{
		memcpy(&(ind->encryTokens),m_pEncryptionStruct,sizeof(encTokensHeaderBasicStruct)+m_pEncryptionStruct->dynamicTokensLen);
		memset(((encryptionToken*)(ind->encryTokens.token))->halfKey,'1',128);
	}
/* v4.1c <--> V6 merge temp comments end of merge temp comments*/
	// fills indication struct
	// =======================
	// fill h225 remote (addrs, ip version, ip, port, distrib, transport type)
	FillMcTransportAddress( &ind->h225remote, eIpVersion4, m_H225RemoteIpAddress.addr.v4.ip/*m_IpH225Remote*/,
						m_H225RemoteIpAddress.port/*m_portH225Remote*/,eDistributionUnicast, eTransportTypeTcp);
	// fill h225 local
	FillMcTransportAddress( &ind->h225local,  eIpVersion4, m_H225LocalIpAddress.addr.v4.ip/*m_IpH225Local*/,
						m_H225LocalIpAddress.port/*m_portH225Local*/, eDistributionUnicast, eTransportTypeTcp);
	// fill h245 remote
	FillMcTransportAddress( &ind->h245remote, eIpVersion4, m_H245RemoteIpAddress.addr.v4.ip/*m_IpH245Remote*/,
						m_H245RemoteIpAddress.port/*m_portH245Remote*/, eDistributionUnicast, eTransportTypeTcp);
	// fill h245 local
	FillMcTransportAddress( &ind->h245local,  eIpVersion4, m_H245LocalIpAddress.addr.v4.ip/*m_IpH245Local*/,
						m_H245LocalIpAddress.port/*m_portH245Local*/,  eDistributionUnicast, eTransportTypeTcp);
	// sDisplay
	char szTemp[] = "-cascaded";
	int nTemp = sizeof(ind->sDisplay)-1 - strlen(szTemp);
	strncpy( ind->sDisplay, m_mcReqCallSetup.callTransient.sDisplay,nTemp);
	ind->sDisplay[nTemp-1] = '\0';
	strcat(ind->sDisplay, "-cascaded");
	// userUser
	strcpy( ind->userUser, "userUser Sim");
	int userUserSize = strlen(ind->userUser);
	// more parameters
	ind->remoteEndpointType = 2;		// MCU
	ind->h225RemoteVersion	= 4;		// version
	ind->endPointNetwork	= 2;		// H323
	ind->bAuthenticated		= 0;		// No
	// remoteVendor - polycom RMX
	ind->remoteVendor.info.t35CountryCode	= 88;//m_rVendor.GetCountryCode();
	ind->remoteVendor.info.t35Extension		= 0;//m_rVendor.GetT35Extension();
	ind->remoteVendor.info.manufacturerCode = 172;//m_rVendor.GetManufacturerCode();

	strcpy( ind->remoteVendor.productID, "Polycom RMX 2000"/*m_rVendor.GetProductId() */);
	ind->remoteVendor.productLen = strlen(ind->remoteVendor.productID);
	strncpy( ind->remoteVendor.versionID, m_rVendor.GetVersionId(), 256 - 1 );
	ind->remoteVendor.versionID[256 - 1] = '\0';

	ind->remoteVendor.versionLen = strlen(ind->remoteVendor.versionID);

	// send the struct to CS simulation
	// ================================
	CCSSimTaskApi api(GetCSID());
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, api.GetCSID(),
			         H323_CS_SIG_CALL_CONNECTED_IND,
			         (BYTE*)ind, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndBytes);
}

////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::NotifyCallConnInd(mcReqCallAnswer *answer)
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::NotifyCallConnInd - H323_CS_SIG_CALL_CONNECTED_IND");
	/* v4.1c <--> V6 merge temp comments	end of merge temp comments*/
	// create basic ind
	// ================
	size_t indLen=0;
	BYTE* pIndBytes = CreateIndicationAndEncryToken(sizeof(mcIndCallConnected), answer->encryTokens, indLen);
	mcIndCallConnected *ind = (mcIndCallConnected*)pIndBytes;

	//translation
	//ind->avfFeVndIdInd;//don't know how to translate it
	ind->bAuthenticated=0;
	ind->endPointNetwork=2;

	FillMcTransportAddress( &ind->h225local,  eIpVersion4, m_H225LocalIpAddress.addr.v4.ip/*m_IpH225Local*/,
						m_H225LocalIpAddress.port/*m_portH225Local*/, eDistributionUnicast, eTransportTypeTcp);
	FillMcTransportAddress( &ind->h225remote, eIpVersion4, m_H225LocalIpAddress.addr.v4.ip/*m_IpH225Local*/,
						m_H225LocalIpAddress.port/*m_portH225Local*/,eDistributionUnicast, eTransportTypeTcp);

	ind->h225RemoteVersion=4;

	ind->h245local=answer->h245Address;
	ind->h245remote=answer->h245Address;

	ind->remoteEndpointType=2;//MCU

	// remoteVendor
	ind->remoteVendor.info.t35CountryCode	= 88;//m_rVendor.GetCountryCode();
	ind->remoteVendor.info.t35Extension		= 0;//m_rVendor.GetT35Extension();
	ind->remoteVendor.info.manufacturerCode = 172;//m_rVendor.GetManufacturerCode();

	strcpy( ind->remoteVendor.productID, "Polycom RMX 2000" /*m_rVendor.GetProductId() */);
	ind->remoteVendor.productLen = strlen(ind->remoteVendor.productID);
	strncpy( ind->remoteVendor.versionID, m_rVendor.GetVersionId(), 256 - 1 );
	ind->remoteVendor.versionID[256 - 1] = '\0';

	ind->remoteVendor.versionLen = strlen(ind->remoteVendor.versionID);

	memcpy(ind->sDisplay, answer->callTransient.sDisplay, sizeof(ind->sDisplay));
	memcpy(ind->userUser, answer->callTransient.userUser, sizeof(ind->userUser));
	ind->userUserSize=answer->callTransient.userUserSize;


	// send the struct to CS simulation
	// ================================
	CCSSimTaskApi api(GetCSID());
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol;
	//pCSProt->AddPortDescriptionHeader(0,0,LOBBY_CONNECTION_ID,ePhysical_art_light);
	//pCSProt->AddCSHeader(5,eMcms,eCentral_signaling);

	//set callee side mpl header
	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
	pCSProt->AddCSHeader(m_wCsHandle, eMcms, m_wCsSrcUnit);
	::FillCsProtocol(pCSProt, api.GetCSID(),
			         H323_CS_SIG_CALL_CONNECTED_IND,
			         (BYTE*) ind, indLen, m_nCsCallIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndBytes);
}

////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnCsCreateCntlReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::OnCsCreateCntlReq - H323_CS_SIG_CREATE_CNTL_REQ");

	// gets the req struct
	mcReqCreateControl *req = (mcReqCreateControl*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointLinker::OnCsCreateCntlReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_mcReqCreateControl, req, sizeof(mcReqCreateControl) );
	// Create conference cap set
	m_pConfCap->CreateFrom323Struct(&req->capabilities);
	//set LPR from conf cap to e.p. cap
	m_pCap->SetLPR(m_pConfCap->IsLPR());
	m_isLPRCap = m_pConfCap->IsLPR();
	// create highest capabilities and create communication mode
	CCapSet  rCommonCap(*m_pCap,*m_pConfCap);
	m_rComMode.Create(rCommonCap);
	// checks legal parameters
	IsLeagaParametersCreateCntl( req );

	//save terminal type
	if(pMplProtocol->getCentralSignalingHeaderCallIndex()==m_callerCsCallIndex)
	{
		m_callerTermType=req->masterSlaveTerminalType;
	}
	else if(pMplProtocol->getCentralSignalingHeaderCallIndex()==m_nCsCallIndex)
	{
		m_calleeTermType=req->masterSlaveTerminalType;
	}

	// Send Cap Response Ind
	SendCsCapResponseInd( pMplProtocol );

	ForwardCapabilityInd(pMplProtocol);

	// Send Connected Ind when negotiation complete
	if(m_calleeTermType>0 && m_callerTermType>0)
	{
		APIU32 callerStat=0;
		APIU32 calleeStat=0;
		if(m_calleeTermType> m_callerTermType)
		{
			callerStat=cmMSSlave;
			calleeStat=cmMSMaster;
		}
		else
		{
			calleeStat=cmMSSlave;
			callerStat=cmMSMaster;
		}

		//send ConnectedInd to caller
		CMplMcmsProtocol  callerInd;
		callerInd.AddPortDescriptionHeader(m_callerPartyID,m_callerConfID,m_callerConnectionID,ePhysical_art_light);
		callerInd.AddCSHeader(m_callerCsHandle,eMcms,m_callerCsSrcUnit, m_callerCsCallIndex);

		SendCsCntlConnectedInd( &callerInd, callerStat);

		//send ConnectedInd to callee
		CMplMcmsProtocol  calleeInd;
		calleeInd.AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
		calleeInd.AddCSHeader(m_wCsHandle,eMcms,m_wCsSrcUnit, m_nCsCallIndex);

		// set one of the cascade side as dial in
		m_dialDirection = DIAL_IN;
		SendCsCntlConnectedInd( &calleeInd, calleeStat);
		m_dialDirection = DIAL_OUT;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::ForwardCapabilityInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::ForwardCapabilityInd - H323_CS_SIG_CAPABILITIES_IND");

	// gets the req struct
	mcReqCreateControl *req = (mcReqCreateControl*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointLinker::ForwardCapabilityInd - GetData is NULL ");
		return;
	}

	//  create cap set for declaration
	CCapSet  rCapset;
	rCapset.CreateFrom323Struct(&req->capabilities);

	BYTE* pIndication = NULL;
	WORD  indLen = rCapset.CreateCapStructH323(&pIndication, cmCapReceive);

	// send the struct to MCMS
	// ================================
	CMplMcmsProtocol*  pCSProt = CreateSwitchMplMcmsMsg(*pMplProtocol, H323_CS_SIG_CAPABILITIES_IND, pIndication, indLen);

	if (pCSProt)
	{
		CSegment *pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

        CCSSimTaskApi api(GetCSID());
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SEND_TO_CSAPI);

	// m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

        POBJDELETE(pCSProt);
	} else
		PTRACE(eLevelError,"CEndpointLinker::ForwardCapabilityInd - pCSProt is NULL ");

	PDELETEA(pIndication);
}

/////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnCsReCapReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::OnCsReCapReq - H323_CS_SIG_RE_CAPABILITIES_REQ");

	// gets the req struct
	mcReqCreateControl *req = (mcReqCreateControl*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointLinker::OnCsReCapReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_mcReqCreateControl, req, sizeof(mcReqCreateControl) );

	// Create conference cap set
	m_pConfCap->CreateFrom323Struct(&req->capabilities);

	//set LPR from conf cap to e.p. cap
	m_pCap->SetLPR(m_pConfCap->IsLPR());
	m_isLPRCap = m_pConfCap->IsLPR();

	// checks legal parameters
	IsLeagaParametersCreateCntl( req );

	// wait a while...

	// Send Cap Response Ind
	SendCsCapResponseInd( pMplProtocol );

	ForwardCapabilityInd(pMplProtocol);
}

//////////////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnOutgoingChnlReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::OnOutgoingChnlReq - H323_CS_SIG_OUTGOING_CHNL_REQ ");

	// Vasily - this message contents static AND dynamic parts
	//     currently we don't use dynamic
	//  length of dynamic - req->sizeOfChannelParams
	//  starts in         - req->channelSpecificParams

	// gets the req struct
	mcReqOutgoingChannel *req = (mcReqOutgoingChannel*)pMplProtocol->GetData();
	if ( NULL == req ) {
		PTRACE(eLevelError,"CEndpointLinker::OnOutgoingChnlReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_outGoingChannelReq, req, sizeof(mcReqOutgoingChannel));


	//update state of channel
	channelSpecificParameters* pChnlSpecific = (channelSpecificParameters*) &(req->channelSpecificParams);
	TChannelDetails*  pChannel = GetChannel(m_outGoingChannelReq.channelType,
									pChnlSpecific->pAudioBase.header.roleLabel,
									cmCapTransmit/*OUT*/);

	if( pChannel == NULL ) {
		DBGPASSERT(1000+m_mcIncomingChannelResponse.dataType);
		DBGPASSERT(1000+m_mcIncomingChannelResponse.channelDirection);
		DBGPASSERT(1000+m_mcIncomingChannelResponse.channelIndex);
		return;
	}

	if( pChannel->channelType == cmCapAudio )
		m_apiArray[SIM_H323_RCV_OUTGOING_A_CHNL]++;
	else if( pChannel->channelType == cmCapVideo )
	{
		if( kRolePeople == pChannel->channelRole )
			m_apiArray[SIM_H323_RCV_OUTGOING_V_CHNL]++;
		else
			m_apiArray[SIM_H323_RCV_OUTGOING_239_CHNL]++;
	}
	else if( pChannel->channelType == cmCapData )
		m_apiArray[SIM_H323_RCV_OUTGOING_F_CHNL]++;

	pChannel->openedInConf = TRUE;
	pChannel->openedInSim=TRUE;

	//forward incoming channel indication
	ForwardIncomingChannelInd(pMplProtocol);

	if( IsConnectionCompleted() == TRUE )
		SetState(eEpStateConnected);
}

//////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::ForwardIncomingChannelInd( CMplMcmsProtocol* pMplProtocol)
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::ForwardIncomingChannelInd - H323_CS_SIG_INCOMING_CHANNEL_IND");
	// gets the req struct
	mcReqOutgoingChannel *req = (mcReqOutgoingChannel*)pMplProtocol->GetData();
	if ( NULL == req ) {
		PTRACE(eLevelError,"CEndpointLinker::ForwardIncomingChannelInd - GetData is NULL ");
		return;
	}

	ALLOCBUFFER(pStr,256);
	channelSpecificParameters* pOutgoingChnlSpecific = (channelSpecificParameters*) &(req->channelSpecificParams);
	TChannelDetails*  pChannel = GetChannel(req->channelType,
									pOutgoingChnlSpecific->pAudioBase.header.roleLabel,
									cmCapReceive/*OUT*/);

	if (pChannel)
	{
		pChannel->openedInConf=TRUE;
		pChannel->openedInSim=TRUE;

	sprintf(pStr,"chType <%d>, chIndex <%d>, chMcIndex <%d>, chDirection <%d>.\
				chRole <%d>",
				(int)pChannel->channelType,(int)pChannel->channelIndex,
				(int)pChannel->channelMcIndex,(int)pChannel->channelDirection,
				(int)pChannel->channelRole);
	} else
		PTRACE(eLevelError,"CCEndpointLinker::ForwardIncomingChannelInd -  pChannel is NULL ");

	PTRACE2(eLevelInfoNormal,"CEndpointLinker::ForwardIncomingChannelInd - ",pStr);
	DEALLOCBUFFER(pStr);

	/* v4.1c <--> V6 merge temp comments	end of merge temp comments*/

	// size of message = static + dynamic
	// ===================================
	WORD indLen = sizeof(mcIndIncomingChannel) + req->xmlDynamicProps.sizeOfAllDynamicParts;

	// allocate memory for indication - static + dynamic
	BYTE* pIndicationBytes = new BYTE [indLen];
	memset( pIndicationBytes, 0, indLen);	// zeroing the struct

	mcIndIncomingChannel* pIncomChannInd = (mcIndIncomingChannel*) pIndicationBytes;
	channelSpecificParameters* pIncomingChnlSpecific = (channelSpecificParameters*) &(pIncomChannInd->channelSpecificParams);

	// fills indication struct
	// =======================

	pIncomChannInd->dataType         =  req->channelType;		// cmCapDataType
	pIncomChannInd->channelIndex     = pMplProtocol->getCentralSignalingHeaderMcChannelIndex();		// same as incoming
	pIncomChannInd->channelDirection = 0;
	// fill RTP remote    ( addrs, ip version, ip, port, distrib, transport type)
	FillMcTransportAddress( &pIncomChannInd->rmtRtpAddress, eIpVersion4, m_rtpRemoteIpAddress.addr.v4.ip/*m_IpRtpRemote*/,
					m_rtpRemoteIpAddress.port/*m_portRtpRemote*/, eDistributionUnicast, eTransportTypeUdp);
	pIncomChannInd->bIsActive			= DONOT_CARE;					// active channel
	pIncomChannInd->sameSessionChannelIndex = DONOT_CARE;			// DONOT_CARE
	pIncomChannInd->sessionId			= DONOT_CARE;				// DONOT_CARE
	pIncomChannInd->dynamicPayloadType	= DONOT_CARE;				// DONOT_CARE

	pIncomChannInd->bIsEncrypted        = req->bIsEncrypted;
	pIncomChannInd->encryptionAlgorithm = req->encryptionAlgorithm;

	pIncomChannInd->bUsedH263Plus = 0;

	pIncomChannInd->xmlDynamicProps.numberOfDynamicParts  = 1;
	pIncomChannInd->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(channelSpecificParameters);
	pIncomChannInd->sizeOfChannelParams = sizeof(channelSpecificParameters);	// according to the channel you indented to open.
	memcpy(pIncomingChnlSpecific, pOutgoingChnlSpecific, req->xmlDynamicProps.sizeOfAllDynamicParts);
	//direction reverse
	pIncomingChnlSpecific->pData.header.direction   = cmCapReceive;

	pIncomChannInd->capTypeCode = req->capTypeCode;
	pIncomChannInd->rate	= 640;//this rate is useless and unnecessary, because MCMS don't use this rate, just for trace
	pIncomChannInd->payloadType	= req->payloadType;
	strcpy( pIncomChannInd->channelName, req->channelName); // just a name of channel

	pIncomChannInd->bIsLPR=req->bIsLPR;


	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol*  pCSProt = CreateSwitchMplMcmsMsg(*pMplProtocol
								,H323_CS_SIG_INCOMING_CHANNEL_IND
								,pIndicationBytes
								,indLen);

	if (pCSProt)
	{
		CSegment *pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

        CCSSimTaskApi api(GetCSID());
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SEND_TO_CSAPI);

	// m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

        POBJDELETE(pCSProt);
	} else
		PTRACE(eLevelError,"CEndpointLinker::ForwardIncomingChannelInd - pCSProt is NULL ");

	PDELETEA(pIndicationBytes);

}

//////////////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnCsIncomingChnlResponse( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::OnCsIncomingChnlResponse - H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ ");

	// gets the req struct
	mcReqIncomingChannelResponse *res = (mcReqIncomingChannelResponse*)pMplProtocol->GetData();
	if (NULL == res) {
		PTRACE(eLevelError,"CEndpointLinker::OnCsIncomingChnlResponse - GetData is NULL ");
		return;
	}

	// checks legal parameters
	IsLegalParametersIncomingChnlResponse( res );

	// save the req struct
	memcpy( &m_mcIncomingChannelResponse, res, sizeof(mcReqIncomingChannelResponse) );

	//forward outgoing channel response indication
	ForwardOutgoingChnlResInd(pMplProtocol);

	TChannelDetails chnlDetail;
	chnlDetail.channelDirection=cmCapReceive;
	chnlDetail.channelIndex=pMplProtocol->getCentralSignalingHeaderChannelIndex();
	chnlDetail.channelMcIndex=pMplProtocol->getCentralSignalingHeaderMcChannelIndex();
	//chnlDetail.channelRole=?;//not needed, lucky
	chnlDetail.channelType=res->dataType;
	chnlDetail.openedInConf=TRUE;
	chnlDetail.openedInSim=TRUE;
	SendIncomingChannelConnectedInd(pMplProtocol, &chnlDetail);

	if( IsConnectionCompleted() == TRUE )
		SetState(eEpStateConnected);
}

////////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::ForwardOutgoingChnlResInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::ForwardOutgoingChnlResInd - H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND ");
/* v4.1c <--> V6 merge temp comments	end of merge temp comments*/
	// gets the req struct
	mcReqIncomingChannelResponse *res = (mcReqIncomingChannelResponse*)pMplProtocol->GetData();
	if (NULL == res) {
		PTRACE(eLevelError,"CEndpointLinker::ForwardOutgoingChnlResInd - GetData is NULL ");
		return;
	}

	// create basic ind
	// ================
	mcIndOutgoingChannelResponse *ind = new mcIndOutgoingChannelResponse;
	WORD indLen = sizeof(mcIndOutgoingChannelResponse);
	memset( ind, 0, indLen);	// zeroing the struct

	// fills indication struct
	// =======================
	ind->channelType		= res->dataType;				// audio || video || data || non standard ...
	ind->channelIndex		= pMplProtocol->getCentralSignalingHeaderMcChannelIndex();
	ind->channelDirection	= 1;			// 0 - In / 1 - Out
	ind->sessionId			= DONOT_CARE;							// Session Id of channel
	ind->payloadType		= m_outGoingChannelReq.payloadType;		// Confirmation from the EP. This payloadType may be wrong, but MCMS never use it
	ind->sameSessionChannelIndex	= DONOT_CARE;					// ?? - RTP associated
	ind->associatedChannelIndex		= DONOT_CARE;					// ?? - RTP associated
	ind->bIsEncrypted =	res->bIsEncrypted;
	ind->dynamicPayloadType= res->dynamicPayloadType;
	ind->encryptionAlgorithm = res->encryptionAlgorithm;
	memcpy(ind->EncryptedSession235Key, res->EncryptedSession235Key, sizeof(ind->EncryptedSession235Key));

	// fill RTP remote    ( addrs,  ip version, ip, port, distrib, transport type)
	FillMcTransportAddress( &ind->destRtpAddress, eIpVersion4, m_rtpRemoteIpAddress.addr.v4.ip/*m_IpRtpRemote*/,
						m_rtpRemoteIpAddress.port/*m_portRtpRemote*/, eDistributionUnicast, eTransportTypeUdp);
	//ind->destRtpAddress = res->localRtpAddressIp;

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol*  pCSProt = CreateSwitchMplMcmsMsg(*pMplProtocol
									,H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND
									,(BYTE*)(ind)
									,indLen);


	if (pCSProt)
	{
		CSegment *pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

	CCSSimTaskApi api(GetCSID());
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SEND_TO_CSAPI);

	// m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

		POBJDELETE(pCSProt);
	} else
		PTRACE(eLevelError,"CEndpointLinker::ForwardOutgoingChnlResInd - pCSProt is NULL ");

	//	SendCommandToCS( H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND, (BYTE*)ind, indLen);

	delete ind;
}

///////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnChnlDropReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::OnChnlDropReq - H323_CS_SIG_CHNL_DROP_REQ ");

//	SetState(eEpStateDisconnecting);

	// gets the req struct
	mcReqChannelDrop *req = (mcReqChannelDrop*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointLinker::OnChnlDropReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_mcReqChannelDrop, req, sizeof(mcReqChannelDrop));

	TChannelDetails chnlDetail;
	chnlDetail.channelDirection=req->channelDirection;// ? cmCapTransmit : cmCapReceive;
	chnlDetail.channelIndex=pMplProtocol->getCentralSignalingHeaderChannelIndex();
	chnlDetail.channelMcIndex=pMplProtocol->getCentralSignalingHeaderMcChannelIndex();
	chnlDetail.channelType=req->channelType;
	//chnlDetail.channelRole=?;//not needed, lucky
	chnlDetail.openedInConf = FALSE;

	// send the struct to CS simulation
	// ================================
	SendChannelCloseInd(pMplProtocol, &chnlDetail);

	//because the StartChnlCloseInd will cause extra ChnlDropReq, don't forward the extra ChnlDropReq
	if(req->channelDirection==cmCapTransmit)
		ForwardStartChnlCloseInd(pMplProtocol);
}

///////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::ForwardStartChnlCloseInd( CMplMcmsProtocol* pMplProtocol )
{
	if(!pMplProtocol )
		return;
	PTRACE(eLevelInfoNormal,"CEndpointLinker::ForwardStartChnlCloseInd - H323_CS_SIG_START_CHANNEL_CLOSE_IND ");

	// gets the req struct
	mcReqChannelDrop *req = (mcReqChannelDrop*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointLinker::ForwardStartChnlCloseInd - GetData is NULL ");
		return;
	}

	WORD indLen = sizeof(mcIndStartChannelClose);

	mcIndStartChannelClose*  ind = new mcIndStartChannelClose;
	ind->channelType       = req->channelType;
	ind->channelIndex      = pMplProtocol->getCentralSignalingHeaderMcChannelIndex();
	ind->channelDirection  = (req->channelDirection==cmCapTransmit) ? cmCapReceive : cmCapTransmit;//revert direction

	CMplMcmsProtocol*  pCSProt = CreateSwitchMplMcmsMsg(*pMplProtocol
								,H323_CS_SIG_START_CHANNEL_CLOSE_IND
								,(BYTE*)(ind)
								,indLen);

	if (pCSProt)
	{
		CSegment *pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

	CCSSimTaskApi api(GetCSID());
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SEND_TO_CSAPI);

	// m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

		POBJDELETE(pCSProt);
	} else
		PTRACE(eLevelError,"CEndpointLinker::ForwardStartChnlCloseInd - pCSProt is NULL ");

	delete ind;

}

//////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnCallDropReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::OnCallDropReq - H323_CS_SIG_CALL_DROP_REQ ");

	m_enDiscoInitiator = eInitiatorConf;

	// gets the req struct
	mcReqCallDrop *req = (mcReqCallDrop*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointLinker::OnCallDropReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_mcReqCallDrop, req, sizeof(mcReqCallDrop));
	m_apiArray[SIM_H323_RCV_CALL_DROP]++;

	SetState(eEpStateDisconnecting);

	SendCallIdleInd(pMplProtocol);

	ForwardCallIdleInd(pMplProtocol);
}

///////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::ForwardCallIdleInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::ForwardCallIdleInd - H323_CS_SIG_CALL_IDLE_IND ");

	// send the Call Idle struct to CS simulation
	// ================================
	WORD indLen = sizeof(mcReqCallDrop);
	mcReqCallDrop  ind;

	CMplMcmsProtocol*  pCSProt = CreateSwitchMplMcmsMsg(*pMplProtocol
								,H323_CS_SIG_CALL_IDLE_IND
								,(BYTE*)&ind
								,indLen);

	// if received call_drop_req
	if( m_apiArray[SIM_H323_RCV_CALL_DROP] > 0 )
		ind.rejectCallReason = m_mcReqCallDrop.rejectCallReason;
	else
		ind.rejectCallReason = 0;

	if (pCSProt)
	{
		CSegment *pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

	CCSSimTaskApi api(GetCSID());
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SEND_TO_CSAPI);

	// m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

        POBJDELETE(pCSProt);
	} else
		PTRACE(eLevelError,"CEndpointLinker::ForwardStartChnlCloseInd - pCSProt is NULL ");
}

//////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnCallCloseConfirmReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::OnCallCloseConfirmReq - H323_CS_SIG_CALL_CLOSE_CONFIRM_REQ ");

	if( m_enEpState != eEpStateDisconnecting )
	{
		PASSERTMSG(1000+m_enEpState," CEndpointLinker::OnCallCloseConfirmReq - Wrong state ");
		return;
	}

	CleanAfterDisconnect();

	SetState(eEpStateDisconnected);

	if( TRUE == m_isToBeDeleted )
		m_isReadyToDelete = TRUE;
}

//////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnRoleTokenReq(CMplMcmsProtocol* pMplProtocol)
{
	PTRACE2(eLevelInfoNormal,"CEndpointLinker::OnRoleTokenReq, Name - ", m_szEpName );

	mcReqRoleTokenMessage* pReq = (mcReqRoleTokenMessage*)pMplProtocol->GetData();

	DBGPASSERT(pReq->mcuID != m_nMcuId);
	TRACESTR(eLevelError) << " CEndpointLinker::OnRoleTokenReq - IsAck <" << (int)pReq->bIsAck << ">";

	ForwardRoleTokenInd(pMplProtocol);
}

//////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::ForwardRoleTokenInd(CMplMcmsProtocol * pMplProtocol)
{
	PTRACE2(eLevelError,"CEndpointLinker::ForwardRoleTokenInd, Name - ", m_szEpName );

	mcReqRoleTokenMessage* pReq = (mcReqRoleTokenMessage*)pMplProtocol->GetData();
	if (NULL == pReq) {
		PTRACE(eLevelError,"CEndpointLinker::ForwardRoleTokenInd - GetData is NULL ");
		return;
	}

	mcIndRoleToken  ind;
	DWORD  indLen = sizeof(mcIndRoleToken);
	memset(&ind,0,indLen);

	ind.bIsAck=pReq->bIsAck;
	ind.bitRate=pReq->bitRate;
	ind.contentProviderInfo=pReq->contentProviderInfo;
	ind.label=pReq->label;
	ind.mcuID=pReq->mcuID;
	ind.randNumber=pReq->randNumber;
	ind.subOpcode=pReq->subOpcode;
	ind.terminalID=pReq->terminalID;

	CMplMcmsProtocol*  pCSProt = CreateSwitchMplMcmsMsg(*pMplProtocol
								,H323_CS_SIG_CALL_ROLE_TOKEN_IND
								,(BYTE*)&ind
								,indLen);

	if (pCSProt)
	{
		CSegment* pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

 	CCSSimTaskApi api(GetCSID());
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SEND_TO_CSAPI);

	// m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

        POBJDELETE(pCSProt);
	} else
		PTRACE(eLevelError,"CEndpointLinker::ForwardRoleTokenInd - pCSProt is NULL ");

	m_enRoleTokenLastCmd = kPresentationTokenRequest;
}

//////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnChannelOnReq(CMplMcmsProtocol* pMplProtocol)
{
	PTRACE2(eLevelInfoNormal,"CEndpointLinker::OnChannelOnReq, Name - ", m_szEpName );

	mcReqChannelOn* pReq = (mcReqChannelOn*)pMplProtocol->GetData();

//	DBGPASSERT(pReq->channelType < 0);

	ForwardChannelOnInd(pMplProtocol);
}

//////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::ForwardChannelOnInd(CMplMcmsProtocol * pMplProtocol)
{
	PTRACE2(eLevelError,"CEndpointLinker::ForwardChannelOnInd, Name - ", m_szEpName );

/*
	mcReqChannelOn* pReq = (mcReqChannelOn*)pMplProtocol->GetData();
	if (NULL == pReq)
	{
		PTRACE(eLevelError,"CEndpointLinker::ForwardChannelOnInd - GetData is NULL ");
		return;
	}

 	mcReqChannelOn  ind;
	DWORD  indLen = sizeof(mcReqChannelOn);
	memset(&ind,0,indLen);

	CMplMcmsProtocol*  pCSProt = CreateSwitchMplMcmsMsg(*pMplProtocol
								,H323_CS_CHANNEL_ON_IND
								,(BYTE*)&ind
								,indLen);
*/

	CMplMcmsProtocol*  pCSProt = CreateSwitchMplMcmsMsg(*pMplProtocol
								,H323_CS_CHANNEL_ON_IND
								,NULL
								,0);

	if (pCSProt)
	{
		CSegment* pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

 	CCSSimTaskApi api(GetCSID());
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SEND_TO_CSAPI);

	// m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

        POBJDELETE(pCSProt);
	}
	else
		PTRACE(eLevelError,"CEndpointLinker::ForwardChannelOnInd - pCSProt is NULL ");
}

//////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::OnChnlNewRateReq(CMplMcmsProtocol* pMplProtocol)
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::OnChnlNewRateReq - H323_CS_SIG_CHNL_NEW_RATE_REQ ");

	// gets the req struct
	mcReqChannelNewRate *req = (mcReqChannelNewRate*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointLinker::OnChnlNewRateReq - GetData is NULL ");
		return;
	}

	if(req->channelDirection==0)
	{
		ForwardChanNewRateInd(pMplProtocol);
	}
	else
	{
		ForwardFlowControIndInd(pMplProtocol);
	}
}

//////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::ForwardChanNewRateInd(CMplMcmsProtocol* pMplProtocol)
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::ForwardChanNewRateInd - H323_CS_SIG_CHAN_NEW_RATE_IND ");

	// send the Call Idle struct to CS simulation
	// ================================

	mcReqChannelNewRate* pReq = (mcReqChannelNewRate*)pMplProtocol->GetData();
	if (NULL == pReq) {
		PTRACE(eLevelError,"CEndpointLinker::ForwardChanNewRateInd - GetData is NULL ");
		return;
	}

	mcIndChannelNewRate chnlNewRateInd;
	WORD indLen = sizeof(chnlNewRateInd);

	chnlNewRateInd.channelDirection=1;//outgoing
	chnlNewRateInd.channelIndex=pMplProtocol->getCentralSignalingHeaderMcChannelIndex();
	chnlNewRateInd.channelType=pReq->channelType;
	chnlNewRateInd.rate=pReq->rate;

	CMplMcmsProtocol*  pCSProt = CreateSwitchMplMcmsMsg(*pMplProtocol
								,H323_CS_SIG_CHAN_NEW_RATE_IND
								,(BYTE*)&chnlNewRateInd
								,indLen);


	if (pCSProt)
	{
		CSegment *pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

	CCSSimTaskApi api(GetCSID());
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SEND_TO_CSAPI);

	// m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

        POBJDELETE(pCSProt);
	} else
		PTRACE(eLevelError,"CEndpointLinker::ForwardChanNewRateInd - pCSProt is NULL ");
}

//////////////////////////////////////////////////////////////////////////////////
void CEndpointLinker::ForwardFlowControIndInd(CMplMcmsProtocol* pMplProtocol)
{
	PTRACE(eLevelInfoNormal,"CEndpointLinker::ForwardFlowControIndInd - H323_CS_SIG_FLOW_CONTROL_IND_IND ");

	// send the Call Idle struct to CS simulation
	// ================================

	mcReqChannelNewRate* pReq = (mcReqChannelNewRate*)pMplProtocol->GetData();
	if (NULL == pReq) {
		PTRACE(eLevelError,"CEndpointLinker::ForwardFlowControIndInd - GetData is NULL ");
		return;
	}

	mcIndFlowControlIndication flowControlIndication;
	WORD indLen = sizeof(flowControlIndication);

	flowControlIndication.channelDirection=0;//incoming
	flowControlIndication.channelIndex=pMplProtocol->getCentralSignalingHeaderMcChannelIndex();
	flowControlIndication.channelType=pReq->channelType;
	flowControlIndication.rate=pReq->rate;

	CMplMcmsProtocol*  pCSProt = CreateSwitchMplMcmsMsg(*pMplProtocol
								,H323_CS_SIG_FLOW_CONTROL_IND_IND
								,(BYTE*)&flowControlIndication
								,indLen);

	if (pCSProt)
	{
		CSegment *pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

	CCSSimTaskApi api(GetCSID());
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SEND_TO_CSAPI);

	// m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

        POBJDELETE(pCSProt);
	} else
		PTRACE(eLevelError,"CEndpointLinker::ForwardFlowControIndInd - pCSProt is NULL ");
}

//////////////////////////////////////////////////////////////////////////////////
CMplMcmsProtocol* CEndpointLinker::CreateSwitchMplMcmsMsg(const CMplMcmsProtocol &MplProtocol, DWORD opcode, BYTE *pIndication, DWORD nDataLen)
{
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(MplProtocol);
	pCSProt->m_CS_McmsHeader.dst_unit_id=eMcms;
	if(MplProtocol.getCentralSignalingHeaderCallIndex()==m_callerCsCallIndex)
	{
		pCSProt->m_MplMcmsPortDescriptionHeader[0].conf_id=m_confID;
		pCSProt->m_MplMcmsPortDescriptionHeader[0].party_id=m_partyID;
		pCSProt->m_MplMcmsPortDescriptionHeader[0].connection_id=m_connectionID;
		pCSProt->m_CS_McmsHeader.cs_id=m_wCsHandle;
		pCSProt->m_CS_McmsHeader.dst_unit_id=m_wCsSrcUnit;
		pCSProt->m_CS_McmsHeader.call_index=m_nCsCallIndex;
	}
	else if(MplProtocol.getCentralSignalingHeaderCallIndex()==m_nCsCallIndex)
	{
		pCSProt->m_MplMcmsPortDescriptionHeader[0].conf_id=m_callerConfID;
		pCSProt->m_MplMcmsPortDescriptionHeader[0].party_id=m_callerPartyID;
		pCSProt->m_MplMcmsPortDescriptionHeader[0].connection_id=m_callerConnectionID;
		pCSProt->m_CS_McmsHeader.cs_id=m_callerCsHandle;
		pCSProt->m_CS_McmsHeader.dst_unit_id=m_callerCsSrcUnit;
		pCSProt->m_CS_McmsHeader.call_index=m_callerCsCallIndex;
	}
	else
	{
		PTRACE(eLevelError,"The CsCallIndex in CMplMcmsProtocol is invalid. There must be a bug.");
		return 0;
	}

	//NOTE: here's a trap in this FillCsProtocol function. this function will reverse the src and dst unit id
	::FillCsProtocol(pCSProt,pCSProt->getCentralSignalingHeaderCsId(), opcode,pIndication,nDataLen);

	//revert channel index
	pCSProt->m_CS_McmsHeader.channel_index = MplProtocol.getCentralSignalingHeaderMcChannelIndex();
	pCSProt->m_CS_McmsHeader.mc_channel_index =MplProtocol.getCentralSignalingHeaderChannelIndex();

	return pCSProt;
}

//////////////////////////////////////////////////////////////////////////////////
BYTE* CEndpointLinker::CreateIndicationAndEncryToken(size_t ind_size, const encTokensHeaderStruct &encryTokens, size_t &newSize)
{
	newSize = 0;
	if(ind_size<=0)
		return 0;

	// create basic ind
	// ================
	WORD size = sizeof(encTokensHeaderBasicStruct) + encryTokens.dynamicTokensLen;
	if( encryTokens.numberOfTokens )
	{
		newSize = ind_size + encryTokens.dynamicTokensLen;
		BYTE* ptr = (BYTE*)m_pEncryptionStruct;
		PDELETEA( ptr );

		m_pEncryptionStruct = (encTokensHeaderStruct*) new BYTE[size];
		memcpy(m_pEncryptionStruct, &encryTokens, size);
	}
	else
	{
		BYTE* ptr = (BYTE*)m_pEncryptionStruct;
		PDELETEA( ptr );
		m_pEncryptionStruct = NULL;
		newSize = ind_size;
	}

	BYTE* pIndBytes = new BYTE [newSize];
	memset( pIndBytes, 0, newSize);	// zeroing the struct

	void *destEncry=pIndBytes+newSize-sizeof(encTokensHeaderStruct);
	if( m_pEncryptionStruct != NULL )
	{
		WORD size = sizeof(encTokensHeaderBasicStruct) + m_pEncryptionStruct->dynamicTokensLen;
		memcpy(destEncry,m_pEncryptionStruct,size);
	}

	return pIndBytes;
}

