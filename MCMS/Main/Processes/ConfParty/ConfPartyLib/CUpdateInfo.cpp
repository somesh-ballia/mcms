#include "CUpdateInfo.h"
#include "StatusesGeneral.h"
#include "CommResDB.h"
#include "CommConfDB.h"
#include "ConfApi.h"
#include "ConfPartyDefines.h"
#include "Request.h"
#include "CDRLogApi.h"
#include "CDRDetal.h"
#include "OperEvent.h"
#include "SystemFunctions.h"
#include "CdrPersistHelper.h"

extern CCommConfDB*           GetpConfDB();

///////////////////////////////////////////////////////
CUpdateInfo::CUpdateInfo()
{
	m_conferId	= 0xFFFFFFFF;
	m_partyId	= 0xFFFFFFFF;
	m_pRsrvParty = NULL;
	m_connectId  = 0xFFFF;
}

///////////////////////////////////////////////////////
CUpdateInfo::~CUpdateInfo()
{
	POBJDELETE(m_pRsrvParty);
}

/////////////////////////////////////////////////////////////////////////////
WORD operator==(const CUpdateInfo& first,const CUpdateInfo& second)
						{
	WORD    rval = 0;     	
	if ( first.m_conferId ==  second.m_conferId
			&& !strncmp(first.m_pRsrvParty->GetName(), second.m_pRsrvParty->GetName(), H243_NAME_LEN )
			&& first.m_connectId == second.m_connectId
			&& first.m_partyId == second.m_partyId)
		rval = 1;   

	return rval;
						}

///////////////////////////////////////////////////////	
int	CUpdateInfo::SendDropParty()
{
	int  status = STATUS_OK;

	/*** VALIDITY of update info ***/
	if (!m_pRsrvParty)
		status = STATUS_ILLEGAL;

	if (status == STATUS_OK)
		status = ::GetpConfDB()->SearchPartyName(m_conferId, m_partyId);

	/*** SEND DropParty to conference ***/
	if (status == STATUS_OK)
	{
		const char* partyName = ::GetpConfDB()->GetPartyName(m_conferId, m_partyId);
		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(m_conferId);

		if( pCurConf )
		{
			CConfApi confApi;
			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			confApi.DropParty(partyName);
			confApi.DestroyOnlyApi();
		}
		else
			PTRACE(eLevelError,"CUpdateInfo::SendDropParty - pCurConf is NULL !!!!");

		// Write to CDR
		CStructTm curTime;
		PASSERT(SystemGetTime(curTime));

		CCdrLogApi cdrApi;

		COperAddParty* rsrvUpdate = new COperAddParty;
		CCdrEvent* pCdrEventParty = new CCdrEvent;
		//		rsrvStart->SetOperatorName(operName);
		rsrvUpdate->SetPartyName(m_pRsrvParty->GetName());
		rsrvUpdate->SetPartyId(m_partyId);
		rsrvUpdate->SetConnectionType(m_pRsrvParty->GetConnectionTypeOper());
		//rsrvStart->SetBondingMode(pConfParty->GetBondingMode1());
		rsrvUpdate->SetNetNumberChannel(m_pRsrvParty->GetNetChannelNumber());
		//rsrvStart->SetNetChannelWidth(pConfParty->GetNetChannelWidth());
		rsrvUpdate->SetNetServiceName((char*)(m_pRsrvParty->GetServiceProviderName()));
		//		rsrvStart->SetRestrict(pConfParty->GetRestrict());
		rsrvUpdate->SetVoice(m_pRsrvParty->GetVoice());
		//		rsrvStart->SetNumType(pConfParty->GetNumType());
		rsrvUpdate->SetNetSubServiceName((char*)(m_pRsrvParty->GetSubServiceName()));
		rsrvUpdate->SetIdentMethod(m_pRsrvParty->GetIdentificationMethod());
		rsrvUpdate->SetMeetMeMethod(m_pRsrvParty->GetMeet_me_method());
		Phone* pPhoneNum = m_pRsrvParty->GetFirstCallingPhoneNumber();						  
		while (pPhoneNum != NULL) {
			rsrvUpdate->AddPartyPhoneNumber(pPhoneNum->phone_number);
			pPhoneNum = m_pRsrvParty->GetNextCallingPhoneNumberOper();
		}
		pPhoneNum = m_pRsrvParty->GetFirstCalledPhoneNumber();
		while (pPhoneNum != NULL) {
			rsrvUpdate->AddMcuPhoneNumber(pPhoneNum->phone_number);
			pPhoneNum = m_pRsrvParty->GetNextCalledPhoneNumber();
		}

		pCdrEventParty->SetCdrEventType(OPERATOR_UPDATE_PARTY);
		pCdrEventParty->SetTimeStamp(curTime);
		pCdrEventParty->SetAddReservUpdatParty( rsrvUpdate );
		/*status = */cdrApi.ConferenceEvent(m_conferId, *pCdrEventParty);
		POBJDELETE(pCdrEventParty);
		POBJDELETE(rsrvUpdate); 

		CCdrEvent cdrEvent;
		COperAddPartyCont1* rsrvUpdateCont1 = new COperAddPartyCont1;
		rsrvUpdateCont1->SetNodeType(m_pRsrvParty->GetNodeType ());               
		rsrvUpdateCont1->SetPassword(m_pRsrvParty->GetPassword ());                 
		// IpV6
		if ((m_pRsrvParty->GetIpAddress ()).ipVersion == eIpVersion4)
			rsrvUpdateCont1->SetIpAddress((m_pRsrvParty->GetIpAddress ()).addr.v4.ip);
		else
			rsrvUpdateCont1->SetIpAddress(0xFFFFFFFF);
		//		rsrvUpdateCont1->SetIpAddress(m_pRsrvParty->GetIpAddress ());
		rsrvUpdateCont1->SetCallSignallingPort(m_pRsrvParty->GetCallSignallingPort ());
		rsrvUpdateCont1->SetVideoProtocol(m_pRsrvParty->GetVideoProtocol ());
		rsrvUpdateCont1->SetVideoRate(m_pRsrvParty->GetVideoRate ());

		BYTE bIpType = m_pRsrvParty->GetNetInterfaceType();
		if (H323_INTERFACE_TYPE == bIpType)
		{
			rsrvUpdateCont1->SetIpPartyAliasType(m_pRsrvParty->GetH323PartyAliasType ());
			rsrvUpdateCont1->SetIpPartyAlias(m_pRsrvParty->GetH323PartyAlias ());                 
		}
		else if (SIP_INTERFACE_TYPE == bIpType)
		{
			rsrvUpdateCont1->SetSipPartyAddressType(m_pRsrvParty->GetSipPartyAddressType());
			rsrvUpdateCont1->SetSipPartyAddress(m_pRsrvParty->GetSipPartyAddress()); 
		}

		rsrvUpdateCont1->SetAudioVolume(m_pRsrvParty->GetAudioVolume ());
		rsrvUpdateCont1->SetUndefinedType(m_pRsrvParty->GetUndefinedType ());
		rsrvUpdateCont1->SetNetInterfaceType(m_pRsrvParty->GetNetInterfaceType ());

		cdrEvent.SetCdrEventType(OPERATOR_UPDATE_PARTY_CONTINUE_1);
		cdrEvent.SetTimeStamp(curTime);
		cdrEvent.SetOperAddPartyCont1(rsrvUpdateCont1);

		/*status = */cdrApi.ConferenceEvent(m_conferId, cdrEvent);
		PTRACE(eLevelInfoNormal, "CUpdateInfo::SendDropParty: CDR failure");
		POBJDELETE(rsrvUpdateCont1);

		CCdrEvent cdrEvent1;
		COperIpV6PartyCont1* rsrvUpdateCont2 = new COperIpV6PartyCont1;
		mcTransportAddress trAddr; 
		memset(&trAddr,0,sizeof(mcTransportAddress));
		if ((m_pRsrvParty->GetIpAddress().ipVersion) == eIpVersion6)
		{
			memcpy(&(trAddr.addr.v6),(void*)&m_pRsrvParty->GetIpAddress().addr.v6,sizeof(ipAddressV6If));
			trAddr.ipVersion = eIpVersion6;
		}
		char tempName[64];
		memset (&tempName,'\0',64);
		ipToString(trAddr,tempName,1);
		rsrvUpdateCont2->SetIpV6Address(tempName);

		cdrEvent1.SetCdrEventType(USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS);
		cdrEvent1.SetTimeStamp(curTime);
		cdrEvent1.SetOperIpV6PartyCont1(rsrvUpdateCont2);

		/*status = */cdrApi.ConferenceEvent(m_conferId, cdrEvent);
		PTRACE(eLevelInfoNormal, "CUpdateInfo::SendDropParty: CDR failure - 1");
		if(trAddr.ipVersion == eIpVersion6 && pCurConf)
			pCurConf->OperatorIpV6PartyCont1(m_pRsrvParty,tempName,USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS);//noa add
		POBJDELETE(rsrvUpdateCont2);

		if (pCurConf)
			// YossiG Klockwork NPD issue
			pCurConf->OperatorAddPartyCont2(m_pRsrvParty, OPERATOR_UPDATE_PARTY_CONTINUE_2);
	}

	SendUpdatePartyEventToCdr();
	return status;
}


void CUpdateInfo::SendUpdatePartyEventToCdr()
{
	int  status = STATUS_OK;



	if (!m_pRsrvParty)
		status = STATUS_ILLEGAL;

	if (status == STATUS_OK)
		status = ::GetpConfDB()->SearchPartyName(m_conferId, m_partyId);

	if (status != STATUS_OK)
		return;


	CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(m_conferId);
	if (pCurConf == NULL)
		return;
	PlcmCdrEventConfOperatorUpdateParty eveUpdateParty;
	std::ostringstream confIdStr;
	confIdStr << m_conferId;
	eveUpdateParty.m_confId = confIdStr.str();

	eveUpdateParty.m_name = m_pRsrvParty->GetName();
	eveUpdateParty.m_id = m_partyId;
	eveUpdateParty.m_connection=(ConnectionType)m_pRsrvParty->GetConnectionTypeOper();
	eveUpdateParty.m_netChannelNumber=  (NetChannelNumType)m_pRsrvParty->GetNetChannelNumber();
	eveUpdateParty.m_subServiceName =  (char*)m_pRsrvParty->GetServiceProviderName();
	eveUpdateParty.m_identificationMethod = (IdentMethodType)m_pRsrvParty->GetIdentificationMethod();
	eveUpdateParty.m_meetMeMethod = pCurConf->ConvertMeetMeMethod(m_pRsrvParty->GetMeet_me_method());
	Phone* pPhoneNum = m_pRsrvParty->GetFirstCallingPhoneNumber();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_phoneList.m_phone1 = pPhoneNum->phone_number;

	pPhoneNum = m_pRsrvParty->GetNextCallingPhoneNumberOper();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_phoneList.m_phone2 = pPhoneNum->phone_number;

	pPhoneNum = m_pRsrvParty->GetNextCallingPhoneNumberOper();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_phoneList.m_phone3 = pPhoneNum->phone_number;

	pPhoneNum = m_pRsrvParty->GetNextCallingPhoneNumberOper();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_phoneList.m_phone4 = pPhoneNum->phone_number;

	pPhoneNum = m_pRsrvParty->GetNextCallingPhoneNumberOper();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_phoneList.m_phone5 = pPhoneNum->phone_number;

	pPhoneNum = m_pRsrvParty->GetFirstCalledPhoneNumber();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_phoneList.m_phone6 = pPhoneNum->phone_number;

	/////////////
	pPhoneNum = m_pRsrvParty->GetNextCalledPhoneNumber();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_mcuPhoneList.m_phone1 = pPhoneNum->phone_number;

	pPhoneNum = m_pRsrvParty->GetNextCalledPhoneNumber();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_mcuPhoneList.m_phone2 = pPhoneNum->phone_number;

	pPhoneNum = m_pRsrvParty->GetNextCalledPhoneNumber();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_mcuPhoneList.m_phone3 = pPhoneNum->phone_number;

	pPhoneNum = m_pRsrvParty->GetNextCalledPhoneNumber();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_mcuPhoneList.m_phone4 = pPhoneNum->phone_number;

	pPhoneNum = m_pRsrvParty->GetNextCalledPhoneNumber();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_mcuPhoneList.m_phone5 = pPhoneNum->phone_number;

	pPhoneNum =m_pRsrvParty->GetNextCalledPhoneNumber();
	if (pPhoneNum != NULL)
		eveUpdateParty.m_mcuPhoneList.m_phone6 = pPhoneNum->phone_number;


	eveUpdateParty.m_nodeType = (NodeType)m_pRsrvParty->GetNodeType ();
	eveUpdateParty.m_password = m_pRsrvParty->GetPassword ();
	char tempName[64];
	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	ipToString(m_pRsrvParty->GetIpAddress(),tempName,1);
	eveUpdateParty.m_ipAddress = tempName;


	eveUpdateParty.m_signallingPort = m_pRsrvParty->GetCallSignallingPort ();
	eveUpdateParty.m_videoProtocol = pCurConf->ConvertVideoProtocolType(m_pRsrvParty->GetVideoProtocol ());
	eveUpdateParty.m_videoBitRate = CCdrPersistConverter::ConvertBitRate(m_pRsrvParty->GetVideoRate ());
	eveUpdateParty.m_bondingPhone = m_pRsrvParty->GetPhoneNumber();

	BYTE interfaceType = m_pRsrvParty->GetNetInterfaceType();

	if (H323_INTERFACE_TYPE == interfaceType)
	{
		eveUpdateParty.m_signallingType = eSignallingType_H323;
		eveUpdateParty.m_alias.m_aliasType =  pCurConf->ConvertAliasType(m_pRsrvParty->GetH323PartyAliasType ());
		eveUpdateParty.m_alias.m_name  = m_pRsrvParty->GetH323PartyAlias ();

	}
	else if (SIP_INTERFACE_TYPE == interfaceType)
	{
		eveUpdateParty.m_signallingType = eSignallingType_SIP;
		eveUpdateParty.m_alias.m_aliasType = pCurConf->ConvertAliasType(m_pRsrvParty->GetSipPartyAddressType());
		eveUpdateParty.m_alias.m_name = m_pRsrvParty->GetSipPartyAddress();
	}

	eveUpdateParty.m_volume = m_pRsrvParty->GetAudioVolume ();
	eveUpdateParty.m_undefined = m_pRsrvParty->GetUndefinedType ();
	eveUpdateParty.m_encryption = (BoolAutoType)pCurConf->GetIsEncryption();

	if (m_pRsrvParty->GetIpAddress().ipVersion == eIpVersion6)
	{
		mcTransportAddress trAddr;
		memset(&trAddr,0,sizeof(mcTransportAddress));
		memcpy(&(trAddr.addr.v6),(void*)&m_pRsrvParty->GetIpAddress().addr.v6,sizeof(ipAddressV6If));
		trAddr.ipVersion = eIpVersion6;
		char tempName[64];
		memset (&tempName,'\0',64);
		ipToString(trAddr,tempName,1);
		eveUpdateParty.m_ipv6Address = tempName;

	}

	pCurConf->SendCdrEvendToCdrManager((ApiBaseObjectPtr)&eveUpdateParty, false, m_partyId);


}























