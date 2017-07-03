/*
 * MSAvMCUMngr.cpp
 *
 *  Created on: Jul 28, 2013
 *      Author: inga
 */
#include "MSAvMCUMngr.h"
#include "IpCsOpcodes.h"


extern CIpServiceListManager* GetIpServiceListMngr();
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CMSAvMCUMngr)

PEND_MESSAGE_MAP(CMSAvMCUMngr,CStateMachine);

CMSAvMCUMngr::CMSAvMCUMngr()
{
	m_pCsRsrcDesc = NULL;
	m_pCsInterface = NULL;
	m_pDestUnitId = eSipBalancer;
	m_pTaskApi 	= NULL ;
	m_FocusUri  = NULL;
	m_ToAddrStr = NULL;

}

CMSAvMCUMngr::CMSAvMCUMngr(const CMSAvMCUMngr &other)
:CStateMachine(other)
{
		m_pDestUnitId = other.m_pDestUnitId;
		m_callIndex = other.m_callIndex;
		m_serviceId = other.m_serviceId;
		m_pCsRsrcDesc = other.m_pCsRsrcDesc;

		//POBJDELETE(m_pCsInterface)

		m_pCsInterface = new CCsInterface (*other.m_pCsInterface);

		m_pTaskApi = other.m_pTaskApi;

		m_PartyId = other.m_PartyId;

		m_FocusUri = other.m_FocusUri;
		m_ToAddrStr = other.m_ToAddrStr;

		strncpy(m_strAVMCUAddress, other.m_strAVMCUAddress, sizeof(m_strAVMCUAddress)-1); m_strAVMCUAddress[sizeof(m_strAVMCUAddress)-1] = '\0';
}


CMSAvMCUMngr::~CMSAvMCUMngr()
{
	POBJDELETE(m_pCsInterface);

//	if (m_pCsRsrcDesc != NULL)
//		POBJDELETE(m_pCsRsrcDesc);

	//POBJDELETE(m_pTaskApi);
}


////////////////////////////////////////////////
void CMSAvMCUMngr::Create(CRsrcParams* pRsrcParams/*, CPartyApi* pPartyApi*/,CConf* pConf,CSipNetSetup* SipNetSetup,DWORD ServiceId,DWORD PartyId)

{
	PTRACE(eLevelInfoNormal, "CMSAvMCUMngr::Create ");

	POBJDELETE(m_pCsRsrcDesc);
	m_pCsRsrcDesc = new CRsrcParams(*pRsrcParams);

	m_pCsInterface = new CCsInterface;
	m_pCsInterface->Create(m_pCsRsrcDesc);

	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if ( pRoutingTbl== NULL )
	{
		PASSERT_AND_RETURN(101);
	}

	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi(pConf->GetRcvMbx(),this);
	m_pTaskApi->SetLocalMbx(pConf->GetLocalQueue());

	WORD status = pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pCsRsrcDesc, m_pTaskApi);
	if (status != STATUS_OK)
	DBGPASSERT(status);

	m_callIndex = 0;//SipNetSetup->GetCallIndex();
	m_serviceId = ServiceId;
	m_PartyId = PartyId;

	m_state    = IDLE;


	//POBJDELETE(m_pCsRsrcDesc);
	//m_pCsRsrcDesc = new CRsrcParams(*pRsrcParams);

	//m_pCsInterface = new CCsInterface;
	//m_pCsInterface->Create(m_pCsRsrcDesc);

	/*
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if ( pRoutingTbl== NULL )
	{
		PASSERT_AND_RETURN(101);
	}

	WORD status = pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pCsRsrcDesc, pPartyApi);
	if (status != STATUS_OK)
	DBGPASSERT(status);
*/
}

////////////////////////////////////////////////
void CMSAvMCUMngr::SendSIPMsgToCS(DWORD opcode, void* pMsg, size_t size)
{
	PTRACE2INT(eLevelInfoNormal, "CMSAvMCUMngr::SendSIPMsgToCS ",m_callIndex);
	DWORD subServiceId = 0;
	CSegment* pSegment = new CSegment;
	pSegment->Put((BYTE*)pMsg, size);
	m_pCsInterface->SendMsgToCS(opcode, pSegment, m_serviceId,m_serviceId, m_pDestUnitId,m_callIndex, 0, 0, 0);

	delete pSegment;
}
/*
////////////////////////////////////////////////////////////////
void CMSAvMCUMngr::SendMessageToPartyCntl(OPCODE event, CSegment *pSeg)
{
	m_pTaskApi->SendAVMCUMngrMsg(event, pSeg);
	POBJDELETE(pSeg);
}
*/
////////////////////////////////////////////////////////////////
DWORD CMSAvMCUMngr::SetDialOutSessionTimerHeaders(CSipHeaderList& headerList)
{
	DWORD sessionTimer	= GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_SESSION_EXPIRES);
	DWORD MinSeconds 	= GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_MIN_SEC);
	sessionTimer =1200;
	MinSeconds =1200;
	PTRACE(eLevelInfoNormal, "CMSAvMCUMngr::SetDialOutSessionTimerHeaders - noa temp hardcoded timer 1200 if 90 as default call disconnect");
	CMedString strSessionExpire;
	CMedString strMinSec;
	strMinSec << MinSeconds;

	strSessionExpire << sessionTimer;

	if(sessionTimer > 0)
	{
		headerList.AddHeader(kSessionExpires,strSessionExpire.GetStringLength(),strSessionExpire.GetString());
		headerList.AddHeader(kMinSe,strMinSec.GetStringLength(),strMinSec.GetString());
	}
	else
		PTRACE(eLevelInfoNormal, "CMSAvMCUMngr::SetDialOutSessionTimerHeaders - No support of session timer");
	return OK_VAL;
}
////////////////////////////////////////////////////////////////////////////////
STATUS CMSAvMCUMngr::BuildToAddress(DWORD partyId)
{


	int len = strlen(m_FocusUri);

	ALLOCBUFFER(FocusUriStr,len+1); AUTO_DELETE_ARRAY(FocusUriStr);
	strncpy(FocusUriStr,m_FocusUri,len);
	FocusUriStr[len] = '\0';

	const char* pReadPtr1 = strstr(FocusUriStr, "sip:");
	 PASSERTSTREAM_AND_RETURN_VALUE(!pReadPtr1,"PartyId:" << partyId,STATUS_INCONSISTENT_PARAMETERS);

	     const char* pReadPtr2 = pReadPtr1 + 4; // should point after sip:

    	int ToAddrsStrLen = strlen(pReadPtr2);//len - pReadPtr3;

		 m_ToAddrStr = new char[ToAddrsStrLen+1];
	     memset(m_ToAddrStr, 0, ToAddrsStrLen);
	     strncpy(m_ToAddrStr,pReadPtr2,ToAddrsStrLen);
	     m_ToAddrStr[ToAddrsStrLen] = '\0';

	     TRACEINTO << "ToAddrsStrLen :" << ToAddrsStrLen;
	     TRACEINTO << "m_ToAddrStr :" << m_ToAddrStr;

	     return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
void CMSAvMCUMngr::BuildAVMCUAddress(const char* SrcPartyAddress)
{
	TRACEINTO << "SrcPartyAddress:" << SrcPartyAddress ;

	DWORD Srclen = strlen(SrcPartyAddress);
	DWORD Urilen = strlen(";gruu;opaque=app:conf:focus:id:");
	DWORD Addlen;
	DWORD FocusUriLen;

	strncpy(m_strAVMCUAddress,SrcPartyAddress,MaxAddressListSize-1);
	m_strAVMCUAddress[MaxAddressListSize-1] = '\0';
	Addlen = strlen(m_strAVMCUAddress);

	TRACEINTO << "Srclen:" << Srclen << ", Urilen:"<< Urilen << ", AddLen:" << Addlen;

	if(Urilen + Addlen < MaxAddressListSize)
	{
		strncat(m_strAVMCUAddress,";gruu;opaque=app:conf:focus:id:",Urilen);
		m_strAVMCUAddress[Addlen+Urilen] = '\0';

		TRACEINTO << "m_strAVMCUAddress1 :" << m_strAVMCUAddress;

		Addlen = strlen(m_strAVMCUAddress);

		TRACEINTO << "Addlen :" << Addlen;

//		if(m_FocusUri)
//		{
			FocusUriLen = strlen(m_FocusUri);
			TRACEINTO << "m_FocusUri :" << m_FocusUri ;
			TRACEINTO <<" FocusUriLen :" << FocusUriLen ;

			if(FocusUriLen + Addlen < MaxAddressListSize)
			{
				strncat(m_strAVMCUAddress,m_FocusUri,FocusUriLen);
				m_strAVMCUAddress[FocusUriLen + Addlen] = '\0';
			}
//		}

		TRACEINTO << "m_strAVMCUAddress2 :" << m_strAVMCUAddress;
	}


}
//////////////////////////////////////////////////////////////////////////////////
void CMSAvMCUMngr::GetOutboundSipProxy(char* pProxyAddress)
{
	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
	PASSERTSTREAM_AND_RETURN(!pServiceParams, "ServiceId:" << m_serviceId << " - IP service does not exist");

	WORD sipState = pServiceParams->GetConfigurationOfSipServers();

	if (pServiceParams->GetSipProxyStatus() != eServerStatusOff)
	{
		if (sipState == eConfSipServerManually)
		{
			CSmallString outboundProxyName = pServiceParams->GetSipProxyName();
			if (outboundProxyName.IsEmpty() == NO)
				strcpy_safe(pProxyAddress, MaxLengthOfSingleUrl, outboundProxyName.GetString());
			return;
		}

		if (sipState == eConfSipServerAuto)
		{
			CSmallString domainName = pServiceParams->GetLocalDomainName();
			if (domainName.IsEmpty() == NO)
				strcpy_safe(pProxyAddress, MaxLengthOfSingleUrl, domainName.GetString());
			return;
		}
	}
}

// --------------------------------------------------------------------------
void CMSAvMCUMngr::RemoveFromRsrcTbl()
{
	if (m_pCsRsrcDesc)
	{
		CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
		PASSERT_AND_RETURN(!pRoutingTbl);

		if (STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*m_pCsRsrcDesc))
		{
			PASSERTSTREAM_AND_RETURN(1, "ConnectionId:" << m_pCsRsrcDesc->GetConnectionId() << ", PartyId:" << m_pCsRsrcDesc->GetPartyRsrcId());
		}
	}
}

// --------------------------------------------------------------------------
void CMSAvMCUMngr::SetFocusUri(const char* focusUri)
{
	PASSERT_AND_RETURN(!focusUri);

	if (m_FocusUri)
		delete[] m_FocusUri;

	int focusUriLen = strlen(focusUri);
	m_FocusUri = new char[focusUriLen+1];
	strcpy_safe(m_FocusUri, focusUriLen+1, focusUri);
}
//--------------------------------------------------------------------------

void CMSAvMCUMngr::OnSipByeInd(CSegment* pParam)
{

	m_state = sMS_DISCONNECTING;
	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIU16 srcUnitId = 0;
	PTRACE(eLevelInfoNormal,"CMSAvMCUMngr::OnSipByeInd");

	TRACECOND_AND_RETURN(!pParam , "pParam is NULL");
	*pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId;

	mcIndBye* pByeMsg = (mcIndBye *)pParam->GetPtr(1);
	DWORD status = pByeMsg->status;
	DBGPASSERT(status); // only if status != STATUS_OK

	if (IsValidTimer(PARTYCONNECTTOUT))
		DeleteTimer(PARTYCONNECTTOUT);


	//Send BYE ACK

	mcReqBye200Ok* pBye200OkMsg = new mcReqBye200Ok;
	size_t size = sizeof(mcReqBye200Ok);
	memset(pBye200OkMsg, 0, size);
	SendSIPMsgToCS(SIP_CS_SIG_BYE_200_OK_REQ, pBye200OkMsg, size);


	m_state = sMS_DISCONNECTED;

	RemoveFromRsrcTbl();

	PDELETE(pBye200OkMsg);
	PTRACE(eLevelInfoNormal,"CMSAvMCUMngr::OnSipByeInd");

	//No Need to updaet the DB on disconnect

}
