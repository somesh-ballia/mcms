//+========================================================================+
//                  EpSimGKRegistrationsList.cpp						   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
//+========================================================================+

#include <string>

#include "Macros.h"
#include "Trace.h"

#include "Segment.h"
#include "TaskApi.h"
#include "ProcessBase.h"
#include "MplMcmsProtocol.h"
#include "IpCsOpcodes.h"
#include "IpMngrOpcodes.h"
#include "GkCsInd.h"
#include "GkCsReq.h"
#include "ChannelParams.h"

#include "SimApi.h"
#include "EndpointsSim.h"
#include "EndpointsSimConfig.h"
#include "EpSimGKRegistrationsList.h"

#include "EndpointsGuiApi.h"
#include "CSSimTaskApi.h"
#include "OpcodesMcmsInternal.h"

// define
#define TIMER_REGISTRATION_EXPIRED	101
const OPCODE GATEKEEPER_BRQ_TIMER = 11001;
const OPCODE GATEKEEPER_IRQ_TIMER = 11002;


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//   CGKeeperList - List of GK elements
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CGKeeperList::CGKeeperList( )      // constructor
{
	m_rcvMbx = new COsQueue();
	int i=0;
	for (i = 0; i < MAX_REGISTRATIONS_IN_GK_LIST; i++)
		m_GKRegistrations[i] = NULL;
	//m_pCSApi = NULL;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CGKeeperList::~CGKeeperList()     // destructor
{
	int i=0;
	for (i = 0; i < MAX_REGISTRATIONS_IN_GK_LIST; i++)
		POBJDELETE( m_GKRegistrations[i] );
	POBJDELETE( m_rcvMbx );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKeeperList::SetTaskRcvMbx(const COsQueue& rcvMbx)
{
	(*m_rcvMbx) = rcvMbx;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//void CGKeeperList::SetCsApi( CTaskApi* pCSApi )
//{
//	m_pCSApi = pCSApi;
//}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKeeperList::HandleNewEvent( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CGKeeperList::HandleNewEvent - R_GK_EpSim==>Events");

	switch( pMplProtocol->getOpcode() )
	{
		case H323_CS_RAS_GRQ_REQ:	// GK Request
		{
			if (NULL == m_GKRegistrations[0]) {	// currently there is only 1 IP Service registration
				m_GKRegistrations[0] = new CGKRegistration;
				//m_GKRegistrations[0]->SetCsApi( m_pCSApi );
			}
			break;
		}
		case H323_CS_RAS_RRQ_REQ:	// service registration (repeats)
		{
			/*
			 if (NULL == m_GKRegistrations[0]) {	// currently there is only 1 IP Service registration
				PTRACE(eLevelError,"CGKeeperList::HandleNewEvent - R_GK_EpSim==>H323_CS_RAS_RRQ_REQ while NOT registered");
				return;
			}
			 */
			if (NULL == m_GKRegistrations[0]) {	// currently there is only 1 IP Service registration
				m_GKRegistrations[0] = new CGKRegistration;
				// m_GKRegistrations[0]->SetCsApi( m_pCSApi );
			}
			break;
		}
		case H323_CS_RAS_ARQ_REQ:	// party connect REQ
		{
			if (NULL == m_GKRegistrations[0]) {	// currently there is only 1 IP Service registration
				PTRACE(eLevelError,"CGKeeperList::HandleNewEvent - R_GK_EpSim==>H323_CS_RAS_ARQ_REQ while NOT registered");
				return;
			}
			break;
		}
		case H323_CS_RAS_DRQ_REQ:	// party disconnect REQ
		{
			if (NULL == m_GKRegistrations[0]) {	// currently there is only 1 IP Service registration
				PTRACE(eLevelError,"CGKeeperList::HandleNewEvent - R_GK_EpSim==>H323_CS_RAS_DRQ_REQ while NOT registered");
				return;
			}
			break;
		}
		case H323_CS_RAS_RAI_REQ:
		{
			if (NULL == m_GKRegistrations[0]) {	// currently there is only 1 IP Service registration
				PTRACE(eLevelError,"CGKeeperList::HandleNewEvent - R_GK_EpSim==>H323_CS_RAS_RAI_REQ while NOT registered");
				return;
			}
			break;
		}
		default:
			break;
	}

	// forward the request
	if (NULL != m_GKRegistrations[0])
		m_GKRegistrations[0]->HandleNewEvent( pMplProtocol );
}


////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKeeperList::SelfRemoveRegistration( CSegment* pParam )
{
	PTRACE(eLevelError,"CGKeeperList::SelfRemoveRegistration - R_GK_EpSim==>Self Remove Registration");

	POBJDELETE( m_GKRegistrations[0] );

	return;
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKeeperList::UnRegisterMCU( DWORD tmp )
{
	PTRACE(eLevelError,"CGKeeperList::UnRegisterMCU - R_GK_EpSim==>UnRegister MCU");

	POBJDELETE( m_GKRegistrations[0] );

	// fills the Ind struct
	gkIndRasURQ indURQ;
	WORD indLen = sizeof(gkIndRasURQ);
	memset( &indURQ, 0, indLen );

	memset ( &indURQ.rejectInfo, 0, sizeof(rejectInfoSt) );

	indURQ.rejectInfo.bIsReject = 0;
	indURQ.rejectInfo.rejectReason = 18;
	indURQ.rejectInfo.bAltGkPermanent = 0;
	indURQ.rejectInfo.altGkList.numOfAltGks = 0;

	// send the struct to MCMS via CS simulation
	// ================================
	CCSSimTaskApi api;
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol;

	::FillCsProtocol(pCSProt, api.GetCSID(),
                     H323_CS_RAS_URQ_IND, (BYTE*)(&indURQ), indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
    {
        api.SendMsg(pMsg, SEND_TO_CSAPI);
        PTRACE(eLevelInfoNormal,
                "CGKeeperList::UnRegisterMCU  GK_EpSim==>H323_CS_RAS_URQ_IND");
    }

//	if (m_pCSApi)
//	{
//		m_pCSApi->SendMsg(pMsg, SEND_TO_CSAPI);
//		PTRACE(eLevelInfoNormal,"CGKeeperList::UnRegisterMCU  GK_EpSim==>H323_CS_RAS_URQ_IND");
//	}
//	else
//	{
//		PASSERT(1);
//	}

	POBJDELETE(pCSProt);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//###########################################################################
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//
//   CGKRegistration
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//###########################################################################
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////




PBEGIN_MESSAGE_MAP(CGKRegistration)

	ONEVENT(1								,ANYCASE	,CGKRegistration::OnStamTest)
	ONEVENT(TIMER_REGISTRATION_EXPIRED		,ANYCASE	,CGKRegistration::OnRegistrationExpired)
	ONEVENT(GATEKEEPER_BRQ_TIMER			,ANYCASE	,CGKRegistration::OnBrqTimerTout)
	ONEVENT(GATEKEEPER_IRQ_TIMER			,ANYCASE	,CGKRegistration::OnIRQTimerTout)

PEND_MESSAGE_MAP(CGKRegistration,CStateMachine);


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CGKRegistration::CGKRegistration( )      // constructor
{
	m_rcvMbx = new COsQueue();
	int i=0;
	for (i = 0; i < MAX_GK_PARTICIPANTS_LIST; i++)
		m_GKParticipants[i] = NULL;
//	m_pCSApi = NULL;
	m_expireTimer = 0;
	m_port = 7777;	// for party registration
	m_crvNumber = 8888; //
	m_pBrqProtocol = NULL;
	m_pIRRProtocol = NULL;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CGKRegistration::~CGKRegistration()     // destructor
{
	int i=0;
	for (i = 0; i < MAX_GK_PARTICIPANTS_LIST; i++)
		POBJDELETE( m_GKParticipants[i] );
	POBJDELETE( m_rcvMbx );
	POBJDELETE(m_pBrqProtocol);
	POBJDELETE(m_pIRRProtocol);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::OnStamTest( CSegment *pParam )
{
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::OnRegistrationExpired( CSegment *pParam )
{
	PTRACE(eLevelInfoNormal,"CGKRegistration::OnRegistrationExpired - R_GK_EpSim==>");
	m_expireTimer = 0;
}

/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::OnBrqTimerTout( CSegment *pParam )
{
	PTRACE(eLevelInfoNormal,"CGKRegistration::OnBrqTimerTout ");

	if( NULL == m_pBrqProtocol ) {
		DBGPASSERT_AND_RETURN(1);
	}

	CSegment *pMsg = new CSegment;
	m_pBrqProtocol->Serialize(*pMsg,CS_API_TYPE);

	CCSSimTaskApi api;
    if(api.CreateOnlyApi() >= 0)
    {
        api.SendMsg(pMsg, SEND_TO_CSAPI);
        PTRACE(eLevelInfoNormal,
            "CGKRegistration::OnBrqTimerTout - Party_R_GK_EpSim==>H323_CS_RAS_GKBRQ_IND");
    }

//	if ( m_pCSApi )
//	{
//		m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);
//		PTRACE(eLevelInfoNormal,"CGKRegistration::OnBrqTimerTout - Party_R_GK_EpSim==>H323_CS_RAS_GKBRQ_IND");
//	}
//	else
//		PASSERT(1);

	POBJDELETE(m_pBrqProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::OnIRQTimerTout( CSegment *pParam )
{
	PTRACE(eLevelInfoNormal,"CGKRegistration::OnIRQTimerTout ");

	if( NULL == m_pIRRProtocol ) {
		DBGPASSERT_AND_RETURN(1);
	}
	// fills the Ind struct
	gkIndGKIRQ ind;
	WORD indLen = sizeof(gkIndGKIRQ);
	memset( &ind, 0, indLen );

	ind.hsRas = 0;

	// send the struct to MCMS via CS simulation
	// ================================
	CCSSimTaskApi api;

//	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol;
//	::FillCsProtocol(pCSProt, api.GetCSID(), H323_CS_RAS_GKIRQ_IND, (BYTE*)(&ind), indLen);
	::FillCsProtocol(m_pIRRProtocol, api.GetCSID(), H323_CS_RAS_GKIRQ_IND, (BYTE*)(&ind), indLen);

	CSegment *pMsg = new CSegment;
//	pCSProt->Serialize(*pMsg,CS_API_TYPE);
	m_pIRRProtocol->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
    {
        api.SendMsg(pMsg, SEND_TO_CSAPI);
        PTRACE(eLevelInfoNormal,
            "CGKRegistration::OnIRQTimerTout - Party_R_GK_EpSim==>H323_CS_RAS_GKIRQ_IND");
    }

	StartTimer( GATEKEEPER_IRQ_TIMER, 60 * SECOND );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::SetTaskRcvMbx(const COsQueue& rcvMbx)
{
	(*m_rcvMbx) = rcvMbx;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//void CGKRegistration::SetCsApi( CTaskApi* pCSApi )
//{
//	m_pCSApi = pCSApi;
//}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CGKRegistration::HandleEvent( CSegment *pMsg,DWORD msgLen,OPCODE opCode )
{
	switch ( opCode )
	{

	default	:
		{
		DispatchEvent( opCode, pMsg );
		break;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::HandleNewEvent( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CGKRegistration::HandleNewEvent - R_GK_EpSim==>Events");

	switch( pMplProtocol->getOpcode() )
	{
	case H323_CS_RAS_GRQ_REQ:	// GK Request
		DoServiceIpReq( pMplProtocol );
		break;

	case H323_CS_RAS_RRQ_REQ:	// service registration (repeats)
		DoServiceRegistrationReq( pMplProtocol );
		break;

	case H323_CS_RAS_ARQ_REQ:	// party connect REQ
		DoPartyConnectReq( pMplProtocol );
		break;

	case H323_CS_RAS_DRQ_REQ:	// party disconnect REQ
		DoPartyDisconnectReq( pMplProtocol );
		break;

	case H323_CS_RAS_RAI_REQ:	// resource availability indication
		DoResourceAvailabilityReq( pMplProtocol );
		break;

	case H323_CS_RAS_IRR_RESPONSE_REQ:	// IRR
		PTRACE(eLevelInfoNormal,"CGKRegistration::HandleNewEvent - R_GK_EpSim==>H323_CS_RAS_IRR_RESPONSE_REQ");
		break;
	default:
		break;
	}

}


////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::DoServiceIpReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CGKRegistration::DoServiceIpReq - R_GK_EpSim==>H323_CS_RAS_GRQ_REQ");

	// the request struct
	gkReqRasGRQ* req = (gkReqRasGRQ*)pMplProtocol->GetData();

	// fills the Ind struct
//	gkIndRasGRQ ind;
//	WORD indLen = sizeof(gkIndRasGRQ);
	WORD indLen = sizeof(gkIndRasGRQ) + sizeof(alternateGkSt) - sizeof(char);
	gkIndRasGRQ *pInd =  (gkIndRasGRQ*)new BYTE[ indLen];
	gkIndRasGRQ &ind = *pInd;
	memset( &ind, 0, indLen );

	//char test[1000];

	strncpy( ind.gatekeeperIdent, "PN:PLCM", MaxIdentifierSize );	// just a name
	ind.gkIdentLength = strlen( ind.gatekeeperIdent );
	//memcpy( (BYTE*)test, (BYTE*)&req->gatekeeperAddress, sizeof(mcXmlTransportAddress) );
	//memcpy( (BYTE*)&ind.rasAddress, (BYTE*)test, sizeof(mcXmlTransportAddress) );
	memcpy( (BYTE*)&ind.rasAddress, (BYTE*)&req->gatekeeperAddress, sizeof(mcXmlTransportAddress) );
	memset( &ind.fs, 0, sizeof(h460FsSt) );	// Avaya
	gkIfSetUnionXml( &ind.rejectOrConfirmCh.unionProps, eChConfirm );	// copied from Koren
	memset ( &ind.rejectOrConfirmCh.choice, 0, sizeof(rejectOrConfirmChoice) );

	// if Avaya
	if( ::GetEpSystemCfg()->GetIsAvayaGatekeeper() == TRUE )
	{
		ind.fs.desireFs.fsId  = 1;
		ind.fs.desireFs.subFs = 1;
	}

	// send the struct to MCMS via CS simulation
	// ================================
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	//Set alternate GK list
	ind.rejectOrConfirmCh.unionProps.unionType = eChConfirm;
	ind.rejectOrConfirmCh.unionProps.unionSize = sizeof(rejectOrConfirmChoice) + sizeof(alternateGkSt) - sizeof(char);

	alternateGkSt *pAlt =(alternateGkSt*) &ind.rejectOrConfirmCh.choice.altGkList.altGkSt;
	ind.rejectOrConfirmCh.choice.altGkList.bIsReject = 0;
	ind.rejectOrConfirmCh.choice.altGkList.numOfAltGks = 1;
	ind.rejectOrConfirmCh.choice.altGkList.xmlDynamicProps.numberOfDynamicParts = 1;
	ind.rejectOrConfirmCh.choice.altGkList.xmlDynamicProps.sizeOfAllDynamicParts = sizeof(alternateGkSt);
	pAlt->xmlHeader.dynamicType = tblAltGk;
	pAlt->xmlHeader.dynamicLength = sizeof(alternateGkSt);
	pAlt->bNeedToRegister = 1;
	pAlt->rasAddress.unionProps.unionType = 0;
	pAlt->rasAddress.unionProps.unionSize = 20;

	pAlt->rasAddress.transAddr.addr.v4.ip = 0x10203040;
	pAlt->rasAddress.transAddr.ipVersion = eIpVersion4;
	pAlt->rasAddress.transAddr.port = 1719;
	pAlt->rasAddress.transAddr.distribution = eDistributionUnicast;
	pAlt->rasAddress.transAddr.transportType = eTransportTypeUdp;

	pAlt->priority = 0;
	pAlt->gkIdentLength = strlen("TestGrqInd");
	strcpy(pAlt->gkIdent, "TestGrqInd");

	CCSSimTaskApi api;
	::FillCsProtocol(pCSProt, api.GetCSID(),
                     H323_CS_RAS_GRQ_IND, (BYTE*)(&ind), indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
    {
        api.SendMsg(pMsg, SEND_TO_CSAPI);
        PTRACE(eLevelInfoNormal,
            "CGKRegistration::DoServiceIpReq - Party_R_GK_EpSim==>H323_CS_RAS_GRQ_IND");
    }

//	if (m_pCSApi)
//	{
//		m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);
//		PTRACE(eLevelInfoNormal,"CGKRegistration::DoServiceIpReq - Party_R_GK_EpSim==>H323_CS_RAS_GRQ_IND");
//	}
//	else
//		PASSERT(1);

	PDELETEA(pInd);
	POBJDELETE(pCSProt);
}

/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::gkIfSetUnionXml( xmlUnionPropertiesSt *pUnionProp, int type )
{
    pUnionProp->unionType = type;
    pUnionProp->unionSize = sizeof(rejectOrConfirmChoice);
}


////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::DoServiceRegistrationReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CGKRegistration::DoServiceRegistrationReq - R_GK_EpSim==>H323_CS_RAS_RRQ_REQ");

	if (m_expireTimer) {
		DeleteTimer( TIMER_REGISTRATION_EXPIRED );
		m_expireTimer = 0;
	}

	// the request struct
	gkReqRasRRQ* req = (gkReqRasRRQ*)pMplProtocol->GetData();

	// fills the Ind struct
	gkIndRasRRQ ind;
	WORD indLen = sizeof(gkIndRasRRQ);
	memset( &ind, 0, indLen );

	// time of registration
	APIU32 registrationDuration = 150;
	//APIU32 registrationDuration = 15; for quick testing


	ind.timeToLive = registrationDuration;
	strncpy( ind.gatekeeperIdent, "PN:PLCM", MaxIdentifierSize );	// just a name
	ind.gkIdentLength = strlen( ind.gatekeeperIdent );
	strcpy( ind.endpointIdent, "1234" );
	ind.epIdentLength = strlen( ind.endpointIdent );
	memset( &ind.fs, 0, sizeof(h460FsSt) );	// Avaya
	gkIfSetUnionXml( &ind.rejectOrConfirmCh.unionProps, eChConfirm );	// copied from Koren
	memset ( &ind.rejectOrConfirmCh.choice, 0, sizeof(rejectOrConfirmChoice) );

	// if Avaya
	if( ::GetEpSystemCfg()->GetIsAvayaGatekeeper() == TRUE )
	{
		ind.fs.desireFs.fsId  = 1;
		ind.fs.desireFs.subFs = 1;
		ind.fs.supportFs.fsId = 1;
		ind.fsAvayaFeDscpInd.fsId = 1;
		ind.fsAvayaFeDscpInd.audioDscp = 46; // QoS, should be between 0 and 63
		ind.fsAvayaFeDscpInd.videoDscp = 26; // QoS, should be between 0 and 63
	}

	// send the struct to MCMS via CS simulation
	// ================================
	CCSSimTaskApi api;
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, api.GetCSID(),
                     H323_CS_RAS_RRQ_IND, (BYTE*)(&ind), indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
    {
        api.SendMsg(pMsg, SEND_TO_CSAPI);
        PTRACE(eLevelInfoNormal,
            "CGKRegistration::DoServiceRegistrationReq - Party_R_GK_EpSim==>H323_CS_RAS_RRQ_IND");
    }

//	if (m_pCSApi) {
//		m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);
//		PTRACE(eLevelInfoNormal,"CGKRegistration::DoServiceRegistrationReq - Party_R_GK_EpSim==>H323_CS_RAS_RRQ_IND");
//	}
//	else
//		PASSERT(1);

	// timer for end registration
	StartTimer( TIMER_REGISTRATION_EXPIRED, registrationDuration );
	m_expireTimer = 1;


	// save protocol until timer awoke
	POBJDELETE(m_pIRRProtocol);
	m_pIRRProtocol = new CMplMcmsProtocol(*pMplProtocol);

	POBJDELETE(pCSProt);
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::DoPartyConnectReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CGKRegistration::DoPartyConnectReq - Party_R_GK_EpSim==>H323_CS_RAS_ARQ_REQ");

	gkReqRasARQ* req = (gkReqRasARQ*)pMplProtocol->GetData();


 	// indication
 	gkIndRasARQ ind;
	WORD indLen = sizeof(gkIndRasARQ);
	memset( &ind, 0, indLen );

	// destCallSignalAddress = copy if IP exists in REQ, if not then creates)
	//if (req->destCallSignalAddress.transAddr.addr.v4.ip != 0)

	if (!isApiTaNull(&req->destCallSignalAddress.transAddr))
	{
		PTRACE(eLevelInfoNormal,"CGKRegistration::DoPartyConnectReq - Party_R_GK_EpSim==>IP Address Exists in Request ");
		memcpy( (BYTE*)&ind.destCallSignalAddress, (BYTE*)&req->destCallSignalAddress, sizeof(mcXmlTransportAddress) );
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CGKRegistration::DoPartyConnectReq - Party_R_GK_EpSim==>IP Address Not Exists in Request (Alias instead)");

		memcpy( (BYTE*)&ind.destCallSignalAddress, (BYTE*)&req->gatekeeperAddress, sizeof(mcXmlTransportAddress) );

		ind.destCallSignalAddress.transAddr.port 		= m_port;

		// create IP Address struct
		/*ind.destCallSignalAddress.transAddr.addr.v4.ip  = 0x02020304;
		ind.destCallSignalAddress.transAddr.ipVersion 	= eIpVersion4;
		ind.destCallSignalAddress.transAddr.port 		= m_port;
		ind.destCallSignalAddress.transAddr.distribution = eDistributionUnicast;
		ind.destCallSignalAddress.transAddr.transportType = eTransportTypeTcp;

		ind.destCallSignalAddress.unionProps.unionType	= 0;
		ind.destCallSignalAddress.unionProps.unionSize	=20;
		*/

		m_port++;
	}
	// fill crv: 	// creates #;	(e.g. start from 1000 and increment every call)
 	ind.crv = m_crvNumber++;
  	// callType: 						= from REQ
	memcpy( (BYTE*)&ind.callType, (BYTE*)&req->callType, sizeof(cmRASCallType) );
	// callModel: 						= direct=cmCallModelTypeDirect
 	ind.callModel = cmCallModelTypeDirect;
 	// bandwidth: 						= from REQ
 	ind.bandwidth = req->bandwidth;
	// leaves irrFrequency 				= 0
	// leaves destExtraCallInfoTypes 	= 0
	// leaves avfFeVndIdInd 			= 0
	// leaves destExtraCallInfo 		= 0
	// leaves destInfo 					= 0
	// leaves remoteExtensionAddress 	= 0
	// conferenceId: 					= create (to make sure all current req for this party contains this value)
	std::string port = "";
	port = m_port;
	strncpy( ind.conferenceId, "9988", MaxConferenceIdSize - 1 );
	ind.conferenceId[MaxConferenceIdSize - 1] = '\0';
	strncat( ind.conferenceId, port.c_str(), MaxConferenceIdSize - strlen(ind.conferenceId) - 1 );	// 9988+pordID
	// callId:							= copy from REQ
	strncpy( ind.callId, req->callId, Size16 - 1 );
	ind.callId[Size16 - 1] = '\0';
	// leaves destInfo 					= 0

	// if Avaya
	if( ::GetEpSystemCfg()->GetIsAvayaGatekeeper() == TRUE )
	{
		ind.avfFeVndIdInd.fsId = 1;
		ind.avfFeVndIdInd.countryCode = 181;
		ind.avfFeVndIdInd.t35Extension = 0;
		ind.avfFeVndIdInd.manfctrCode = 9009;
		strcpy(ind.avfFeVndIdInd.productId,"ViewStation FX");
		strcpy(ind.avfFeVndIdInd.versionId,"Release 6.0.5");
		strcpy(ind.avfFeVndIdInd.enterpriseNum,"EndpointSim");
	}

	// send the struct to MCMS via CS simulation
	// ================================
	CCSSimTaskApi api;
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, api.GetCSID(),
                     H323_CS_RAS_ARQ_IND, (BYTE*)(&ind), indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
    {
        api.SendMsg(pMsg, SEND_TO_CSAPI);
        PTRACE(eLevelInfoNormal,
            "CGKRegistration::DoPartyConnectReq - Party_R_GK_EpSim==>H323_CS_RAS_ARQ_IND");
    }

//	if (m_pCSApi) {
//		m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);
//		PTRACE(eLevelInfoNormal,"CGKRegistration::DoPartyConnectReq - Party_R_GK_EpSim==>H323_CS_RAS_ARQ_IND");
//	}
//	else
//		PASSERT(1);

	POBJDELETE(pCSProt);

	// if gatekeeper shall send BRQ
	DWORD  brqTime = ::GetEpSystemCfg()->GetGatekeeperBrqTime();
	if( 0 != brqTime )
	{
		// indication
		gkIndBRQFromGk  brqIndFromGk;
		WORD brqIndLen = sizeof(gkIndBRQFromGk);
		memset( &brqIndFromGk, 0, brqIndLen );

		brqIndFromGk.hsRas = 0;
		brqIndFromGk.bandwidth = req->bandwidth / 2;
		brqIndFromGk.avfFeMaxNonAudioBitRateInd.fsId = H460_K_FsId_Avaya;
		brqIndFromGk.avfFeMaxNonAudioBitRateInd.bitRate = brqIndFromGk.bandwidth;

		// save protocol until timer awoke
		POBJDELETE(m_pBrqProtocol);
		m_pBrqProtocol = new CMplMcmsProtocol(*pMplProtocol);

        ::FillCsProtocol(m_pBrqProtocol, api.GetCSID(), H323_CS_RAS_GKBRQ_IND,
                         (BYTE*)&brqIndFromGk, brqIndLen);

		StartTimer( GATEKEEPER_BRQ_TIMER, brqTime * SECOND );
	}

	if (!IsValidTimer(GATEKEEPER_IRQ_TIMER))
	{
			StartTimer( GATEKEEPER_IRQ_TIMER, 60 * SECOND );
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::DoPartyDisconnectReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CGKRegistration::DoPartyDisconnectReq - Party_R_GK_EpSim==>H323_CS_RAS_DRQ_REQ");

	// the request struct
	gkReqRasDRQ* req = (gkReqRasDRQ*)pMplProtocol->GetData();


	// indication
 	gkIndRasDRQ ind;
	WORD indLen = sizeof(gkIndRasDRQ);
	memset( &ind, 0, indLen );


	// send the struct to MCMS via CS simulation
	// ================================
	CCSSimTaskApi api;
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, api.GetCSID(),
                     H323_CS_RAS_DRQ_IND, (BYTE*)(&ind), indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
    {
        api.SendMsg(pMsg, SEND_TO_CSAPI);
        PTRACE(eLevelInfoNormal,
            "CGKRegistration::DoPartyDisconnectReq - Party_R_GK_EpSim==>H323_CS_RAS_DRQ_IND");
    }
//
//
//	if (m_pCSApi) {
//		m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);
//		PTRACE(eLevelInfoNormal,"CGKRegistration::DoPartyDisconnectReq - Party_R_GK_EpSim==>H323_CS_RAS_DRQ_IND");
//	}
//	else
//		PASSERT(1);

	POBJDELETE(pCSProt);


}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKRegistration::DoResourceAvailabilityReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CGKRegistration::DoResourceAvailabilityReq - Party_R_GK_EpSim==>H323_CS_RAS_RAI_REQ");

	// the request struct
	gkReqRasRAI* req = (gkReqRasRAI*)pMplProtocol->GetData();

	// indication
// 	gkIndRasDRQ ind;
//	WORD indLen = sizeof(gkIndRasDRQ);
//	memset( &ind, 0, indLen );

	// send the struct to MCMS via CS simulation
	// ================================
	CCSSimTaskApi api;
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, api.GetCSID(), H323_CS_RAS_RAC_IND, NULL, 0 );

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
    {
        api.SendMsg(pMsg, SEND_TO_CSAPI);
        PTRACE(eLevelInfoNormal,
            "CGKRegistration::DoResourceAvailabilityReq - Party_R_GK_EpSim==>H323_CS_RAS_RAC_IND");
    }

//	if (m_pCSApi) {
//		m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);
//		PTRACE(eLevelInfoNormal,"CGKRegistration::DoResourceAvailabilityReq - Party_R_GK_EpSim==>H323_CS_RAS_RAC_IND");
//	}
//	else
//		PASSERT(1);

	POBJDELETE(pCSProt);
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//###########################################################################
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//
//   CGKParty - Party element
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//###########################################################################
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CGKParty)

ONEVENT(1						,ANYCASE	,CGKParty::OnStartElement)
//ONEVENT(2						,ANYCASE	,CGKParty::OnTimerEndRegistration)
//ONEVENT(3						,ANYCASE	,CGKParty::OnTimerRemoveRegistration)

PEND_MESSAGE_MAP(CGKParty,CStateMachine);





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CGKParty::CGKParty( )      // constructor
{
//	m_pCSApi = NULL;
	m_GKApi = new CTaskApi();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CGKParty::~CGKParty()     // destructor
{
	if(m_GKApi)
		m_GKApi->DestroyOnlyApi();
	POBJDELETE( m_GKApi );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//void CGKParty::SetCsApi( CTaskApi* pCSApi )
//{
//	m_pCSApi = pCSApi;
//}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CGKParty::HandleEvent( CSegment *pMsg,DWORD msgLen,OPCODE opCode )
{
	switch ( opCode )
	{

	default	:
		{
		DispatchEvent( opCode, pMsg );
		break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKParty::HandleNewEvent( CMplMcmsProtocol* pMplProtocol )
{

	switch( pMplProtocol->getOpcode() )
	{
		case H323_CS_RAS_ARQ_REQ:	// party connect REQ
			break;

		case H323_CS_RAS_DRQ_REQ:	// party disconnect REQ
			break;

		default   :  {
			PASSERT(pMplProtocol->getOpcode()+1000);
			break;
			}

	}
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGKParty::OnStartElement( CSegment* pParam )
{
}
