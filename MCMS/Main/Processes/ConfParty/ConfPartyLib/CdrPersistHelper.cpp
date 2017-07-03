#include "CdrPersistHelper.h"
#include "CdrPersistApiEnums.h"
#include "InterfaceType.h"
#include "ConfPartySharedDefines.h"
#include "CDRDefines.h"
#include "CDRUtils.h"
#include "StringsMaps.h"
#include "NStream.h"
#include "BilParty.h"
#include "EventData.h"
#include "PlcmCdrEvent.h"
#include "H221Str.h"
#include "Q931Structs.h"
#include "ConfParty.h"

#include "SystemFunctions.h"
#include <sstream>
#include "TraceStream.h"
#include "Trace.h"


///////////////////////////////////////////////////////////////////////////
//						CCdrPersistConverter							//
///////////////////////////////////////////////////////////////////////////

std::string CCdrPersistConverter::ConvertBitRate(DWORD bitRatelType)
{
	switch(bitRatelType)
	{
		case 0xFFFFFFFF:
		{
			return 	"automatic";
			break;
		}
		default:
		{
			std::ostringstream bitRatelTypeStr;
			bitRatelTypeStr << bitRatelType;
			return bitRatelTypeStr.str();
		}
	}
}

SignallingType CCdrPersistConverter::InterfaceTypeToPlcmSignallingType(BYTE interfaceType) const
{
	if (interfaceType == H323_INTERFACE_TYPE)
	{
		return eSignallingType_H323;
	}
	if (interfaceType == SIP_INTERFACE_TYPE)
	{
		return eSignallingType_SIP;
	}
	if (interfaceType == ISDN_INTERFACE_TYPE)
	{
		return eSignallingType_ISDN;
	}
	FTRACEINTO << "Undefined Interface type " << interfaceType;
	return eSignallingType_DEFAULT;
}

std::string CCdrPersistConverter::PartystateToPlcmString(int type, DWORD partyState) const
{
	const char *pDescription ;
	if(CStringsMaps::GetDescription(type, partyState, &pDescription) == TRUE)
	{
		return pDescription;
	}
	FTRACEINTO << "No such Part State  " << partyState << " type  " << (int)type ;
	return "";
}

void PlcmEventConnectedHelper::SetDataPartyConnected(BYTE interfaceType, const CPartyConnected& partyConnected)
{
	m_cdrEventConnected.m_signallingType = InterfaceTypeToPlcmSignallingType(interfaceType) ;
	m_cdrEventConnected.m_name = partyConnected.GetPartyName();
	m_cdrEventConnected.m_partyId = partyConnected.GetPartyId();
	m_cdrEventConnected.m_ongoingPartyStatus.m_id = partyConnected.GetPartyState();
	m_cdrEventConnected.m_ongoingPartyStatus.m_description =PartystateToPlcmString(ONGOING_PARTY_STATUS_ENUM, partyConnected.GetPartyState());
	m_cdrEventConnected.m_secondaryCause.m_id = partyConnected.GetSecondaryCause();
	m_cdrEventConnected.m_secondaryCause.m_description =PartystateToPlcmString(SECONDARY_CAUSE_ENUM, partyConnected.GetSecondaryCause());

	// m_localCommMode and  m_localCommMode fields are not set in original cdr so They weren't set in the new 1...
	//string localCommMode, remoteCommMode;
	// SetLocalAndRemoteMode(eventType, partyConnected, localCommMode,remoteCommMode);
	// m_cdrEventConnected.m_localCommMode = localCommMode;
	// m_cdrEventConnected.m_remoteCommMode = remoteCommMode;

}

void PlcmEventConnectedHelper::SetDataSvcPartyConnected(BYTE interfaceType, const CSvcSipPartyConnected& svcSipPartyConnected)
{
	// SVC_SIP_PARTY_CONNECTED

	m_cdrEventConnected.m_signallingType = InterfaceTypeToPlcmSignallingType(interfaceType) ;
	m_cdrEventConnected.m_name = svcSipPartyConnected.GetPartyName();
	m_cdrEventConnected.m_partyId = svcSipPartyConnected.GetPartyId();

	m_cdrEventConnected.m_svcConnectInfo.m_svcPartyStatus = PartyStateToSvcPartyStatusType(svcSipPartyConnected.GetPartyState());
	m_cdrEventConnected.m_svcConnectInfo.m_receiveLineRate.m_negotiated = svcSipPartyConnected.GetBitRateIn();
	m_cdrEventConnected.m_svcConnectInfo.m_transmitLineRate.m_negotiated = svcSipPartyConnected.GetBitRateOut();

	const std::list<SvcStreamDesc> listStreams = *svcSipPartyConnected.GetStreams();

	std::list<SvcStreamDesc>::const_iterator it = listStreams.begin();
	int i = 0;

	// TODO Rather change schma to list
	for ( ; it != listStreams.end() &&  i < 3; ++it, ++i)
	{
		StreamContent *pStream = NULL;
		if (i == 0)
		{
			pStream = &m_cdrEventConnected.m_svcConnectInfo.m_uplinkVideoCapabilites.m_stream1;

		}
		else if (i == 1)
		{
			pStream = &m_cdrEventConnected.m_svcConnectInfo.m_uplinkVideoCapabilites.m_stream2;
		}
		else
		{
			pStream = &m_cdrEventConnected.m_svcConnectInfo.m_uplinkVideoCapabilites.m_stream3;
		}
		pStream->m_resolutionWidth = it->m_width;
		pStream->m_resolutionHight = it->m_height;
		DWORD dw = it->m_frameRate / 256;
		if (dw * 256 == it->m_frameRate)
		{
			pStream->m_relayMaxFrameRate = dw;
		}
		else // TODO  schema should be changed to float (or string as in the original), to enable float variable
		{
			/*char buff[10] = "";
			float tmp = ((float)(it->m_frameRate)) / 256.0;
			sprintf(buff, "%.1f", tmp);*/
			pStream->m_relayMaxFrameRate = dw;
		}
		pStream->m_relayMaxBitRate = it->m_bitRate;

	}
	m_cdrEventConnected.m_svcConnectInfo.m_audioCodec = AudiCodecToRelayCodecType(svcSipPartyConnected.GetAudioCodec());
}


SvcPartyStatusType PlcmEventConnectedHelper::PartyStateToSvcPartyStatusType(DWORD partyState) const
{
	//  actually same values ...
	if (partyState == PARTY_IDLE) { return eSvcPartyStatusType_Idle;}
	if (partyState == PARTY_CONNECTED) { return eSvcPartyStatusType_Connected;}
	if (partyState == PARTY_DISCONNECTED) { return eSvcPartyStatusType_Disconnected;}

	if (partyState == PARTY_DISCONNECTED) { return eSvcPartyStatusType_Disconnected;}
	if (partyState == PARTY_WAITING_FOR_DIAL_IN) { return eSvcPartyStatusType_WaitingForDialIn;}
	if (partyState == PARTY_CONNECTING) { return eSvcPartyStatusType_Connecting;}

	if (partyState == PARTY_DISCONNECTING) { return eSvcPartyStatusType_Disconnecting;}
	if (partyState == PARTY_CONNECTED_PARTIALY) { return eSvcPartyStatusType_PartiallyConnected;}

	if (partyState == PARTY_DELETED_BY_OPERATOR) { return eSvcPartyStatusType_DeletedByUser;}

	if (partyState == PARTY_SECONDARY) { return eSvcPartyStatusType_Secondary;}
	// yaela missing
	//if (partyState == PARTY_STAND_BY) { return ???;}

	if (partyState == PARTY_CONNECTED_WITH_PROBLEM) { return eSvcPartyStatusType_ConnectedWithProblem;}

	if (partyState == PARTY_REDIALING) { return eSvcPartyStatusType_Redialing;}

	FTRACEINTO << "No such Svc Party State  " << partyState;


	return eSvcPartyStatusType_DEFAULT;

}


RelayCodecType PlcmEventConnectedHelper::AudiCodecToRelayCodecType(DWORD audiCodec) const
{
	// ECodecSubType to RelayCodecType - actually same values (eH264 == eH264) etc...
	return (RelayCodecType)audiCodec;
}

void PlcmEventConnectedHelper::SetLocalAndRemoteMode(WORD eventType, const CPartyConnected& partyConnected, std::string &localCommMode, std::string &remoteCommMode) const
{
	COstrStream ostr;
	const CH221Str *pCapabilities2;

	const CH221Str* pCapabilities = partyConnected.GetCapabilities();
	const CH221Str* pRemoteCapabilities = partyConnected.GetRemoteCommMode();

	if( eventType == EVENT_PARTY_CONNECTED )
	{
		if (pCapabilities)
		{
			CCDRUtils::FullDumpCap(pCapabilities->GetPtr(),pCapabilities->GetLen(),ostr,0);
			if(pCapabilities->GetLen())
			{
				localCommMode = ostr.str();
			}
			ostr.str("");
			ostr.clear();
		}
		if (pRemoteCapabilities)
		{
			CCDRUtils::CdrDumpH221Stream(ostr,pRemoteCapabilities->GetLen(),pRemoteCapabilities->GetPtr());
			if(pRemoteCapabilities->GetLen())
			{
				remoteCommMode = ostr.str();
			}
			ostr.str("");
			ostr.clear();
		}

	 }
	else if (eventType == IP_PARTY_CONNECTED || eventType == SIP_PARTY_CONNECTED)
	{
		if (pCapabilities)
		{
			CCDRUtils::CdrDumpH323Cap(pCapabilities->GetPtr(), pCapabilities->GetLen(), ostr, 0);

			if(pCapabilities->GetLen())
			{
				localCommMode = ostr.str();
			}
			ostr.str("");
			ostr.clear();
		}
		if (pRemoteCapabilities)
		{

			CCDRUtils::CdrDumpH323Cap(pRemoteCapabilities->GetPtr(), pRemoteCapabilities->GetLen(), ostr, 0);

			if(pRemoteCapabilities->GetLen())
			{
				remoteCommMode = ostr.str();
			}

			ostr.str("");
			ostr.clear();
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//		class PlcmCdrEventDisconnectedExtendedHelper
///////////////////////////////////////////////////////////////////////////////////////////////////////


void PlcmCdrEventDisconnectedExtendedHelper::SetInterfaceType(BYTE interfaceType)
{
	m_cdrEventDisconnectedExtended.m_signallingType = InterfaceTypeToPlcmSignallingType(interfaceType);
}


void PlcmCdrEventDisconnectedExtendedHelper::SetDataFromPartyDisconnected(const CPartyDisconnected&  partyDisconnected)
{
	m_cdrEventDisconnectedExtended.m_name = partyDisconnected.GetPartyName();

	m_cdrEventDisconnectedExtended.m_partyId = partyDisconnected.GetPartyId();

	// Note that ometimes disconnected cause is defined as emum type in the schema
	m_cdrEventDisconnectedExtended.m_disconnectionCause.m_description = PartystateToPlcmString(DISCONNECTION_CAUSE_ENUM, partyDisconnected.GetDisconctCause());

	m_cdrEventDisconnectedExtended.m_q931DisconnectionCause.m_description = CCDRUtils::GetQ931CauseAsString(partyDisconnected.GetQ931DisonctCause());


}

void PlcmCdrEventDisconnectedExtendedHelper::SetDataFromPartyDisconnected1(const CPartyDisconnectedCont1& partyDisconnectedCont1)
{

	m_cdrEventDisconnectedExtended.m_lSyncLoss = partyDisconnectedCont1.GetL_syncLostCounter();
	m_cdrEventDisconnectedExtended.m_rSyncLoss = partyDisconnectedCont1.GetR_syncLostCounter();
	m_cdrEventDisconnectedExtended.m_lVideoSyncLoss = partyDisconnectedCont1.GetL_videoSyncLostCounter();
	m_cdrEventDisconnectedExtended.m_rVideoSyncLoss = partyDisconnectedCont1.GetR_videoSyncLostCounter();
	m_cdrEventDisconnectedExtended.m_rVideoSyncLoss = partyDisconnectedCont1.GetR_videoSyncLostCounter();

	// these fields are not specifically set in the original cdr so they weren't set here...
	// m_audioBoard, m_videoBoard, m_videoBoard,m_dataBoard,  m_dataUnit, m_dataUnit, m_videoUnit,
	// m_audioBridgeBoard, m_audioBridgeUnit, m_audioUnit, m_audioBridgeBoard, m_audioBridgeUnit,
	// m_audioUnit, m_audioUnit, m_audioUnit, m_muxUnit

}

Connection PlcmCdrEventCallStartExtendedHelper::ConnectionTypeToPlcmConnection(BYTE connectionType)
{
	if (connectionType == DIAL_IN)
	{
		return eConnection_DialIn;
	}
	if (connectionType == DIAL_OUT)
	{
		return eConnection_DialOut;
	}
	FTRACEINTO << "No such Connection Type " << (int)connectionType;
	return eConnection_LAST;
}


///////////////////////////////////////////////////////////////
NetworkLineType PlcmCdrEventCallStartExtendedHelper::NumTypeToPlcmNetworkLineType(BYTE numType) const
{
	// mapped from NUM_TYPE_ENUM defined in Q931Structs.h
	if (numType == ACU_NB_TYPE_UNKNOWN)
	{
		return eNetworkLineType_Unknown;
	}
	if (numType == ACU_NB_TYPE_INTERNATIONAL)
	{
		return eNetworkLineType_International;
	}
	if (numType == ACU_NB_TYPE_NATIONAL)
	{
		return eNetworkLineType_National;
	}
	if (numType == ACU_NB_TYPE_NATIONAL)
	{
		return eNetworkLineType_NetworkSpecific;
	}
	if (numType == ACU_NB_TYPE_SUBSCRIBER)
	{
		return eNetworkLineType_Subscriber;
	}
	if (numType == ACU_NB_TYPE_ABBREVIATED)
	{
		return eNetworkLineType_Abbreviated;
	}
	if (numType == NUM_TYPE_DEF )
	{
		return eNetworkLineType_TakenFromService;
	}
	FTRACEINTO << "No such Num Type  " << (int)numType;
	return eNetworkLineType_DEFAULT;
}

/////////////////////////////////////////////////////////////////////
IdentMethodType PlcmCdrEventCallStartExtendedHelper::IdentMethodToPlcmIdentMethodType(BYTE identMethod) const
{
	// mapped from IDENT_METHOD_ENUM
	if (identMethod == PASSWORD_IDENTIFICATION_METHOD)
	{
		return eIdentMethodType_Password;
	}
	if (identMethod == CALLED_PHONE_NUMBER_IDENTIFICATION_METHOD)
	{
		return eIdentMethodType_CalledPhoneNumber;
	}
	if (identMethod == CALLING_PHONE_NUMBER_IDENTIFICATION_METHOD)
	{
		return eIdentMethodType_CallingPhoneNumber;
	}
	FTRACEINTO << "No such Ident Method  " << (int)identMethod;
	return eIdentMethodType_DEFAULT;
}



///////////////////////////////////////////////////////////////////////////

MeetMeMethodeType PlcmCdrEventCallStartExtendedHelper::MeetingMethodToPlcmMeetMeMethodeType(BYTE meetingMethod) const
{
	// mapped from MEET_ME_METHOD_ENUM
	if (meetingMethod == 1)
	{
		return  eMeetMeMethodeType_McuConference;
	}
	if (meetingMethod == 3)
	{
		return eMeetMeMethodeType_Party;
	}
	if (meetingMethod == 4)
	{
		return eMeetMeMethodeType_Channel;
	}
	FTRACEINTO << "No such Meeting Method " << (int)meetingMethod;
	return eMeetMeMethodeType_Unknown;

}


////////////////////////////////////////////////////////////////////

NodeType PlcmCdrEventCallStartExtendedHelper::NodeTypeToPlcmNodeType(BYTE nodeType) const
{
	// mapped from Node_Type_Enum
	if (nodeType == 0)
	{
		return eNodeType_Mcu;
	}
	if (nodeType == 1)
	{
		return eNodeType_Terminal;
	}
	FTRACEINTO << "No such Node Type " << (int)nodeType;
	return eNodeType_DEFAULT;
}


///////////////////////////////////////////////////////////////////////////

VideoProtocolType PlcmCdrEventCallStartExtendedHelper::VideoProtocolToPlcmVideoProtocolType(BYTE videoProtocolType) const
{
	switch(videoProtocolType)
		{
		case VIDEO_PROTOCOL_H261:
		{
			return 	eVideoProtocolType_H261;
			break;
		}
		case VIDEO_PROTOCOL_H263:
		{
			return 	eVideoProtocolType_H263;
			break;
		}
		case VIDEO_PROTOCOL_H26L:
		{
			return 	eVideoProtocolType_H26L;
			break;
		}
		case VIDEO_PROTOCOL_H264:
		{
			return 	eVideoProtocolType_H264;
			break;
		}
		case VIDEO_PROTOCOL_RTV:
		{
			return 	eVideoProtocolType_Rtv;
			break;
		}
		case AUTO:
		{
			return 	eVideoProtocolType_Auto;
			break;
		}
		case VIDEO_PROTOCOL_H264_HIGH_PROFILE:
		{
			return 	eVideoProtocolType_H264HighProfile;
			break;
		}
		default:
		{
			return eVideoProtocolType_DEFAULT;
			break;
		}
		}
}


AliasType PlcmCdrEventCallStartExtendedHelper::AliasTypeToPlcmAliasType(WORD aliasType) const
{
	// mapped from ALIAS_TYPE_ENUM
	if (aliasType == 0 )
	{
		return eAliasType_None;
	}

	if (aliasType == PARTY_H323_ALIAS_H323_ID_TYPE )
	{
		return eAliasType_323Id;
	}
	if (aliasType == PARTY_H323_ALIAS_E164_TYPE)
	{
		return eAliasType_E164;
	}
	if (aliasType == PARTY_H323_ALIAS_URL_ID_TYPE )
	{
		return eAliasType_UrlId;
	}
	if (aliasType == PARTY_H323_ALIAS_EMAIL_ID_TYPE )
	{
		return eAliasType_EmailId;
	}
	if (aliasType == PARTY_H323_ALIAS_TRANSPORT_ID_TYPE )
	{
		return eAliasType_TransportId;
	}
	if (aliasType == PARTY_H323_ALIAS_PARTY_NUMBER_TYPE )
	{
		return eAliasType_PartyNumber;
	}

	FTRACEINTO << "No such Alias Type " << (int)aliasType;
	return eAliasType_DEFAULT;
}

///////////////////////////////////////////////////////////////////////////

SipAddressType PlcmCdrEventCallStartExtendedHelper::SipAddressToPlcmSipAddressType(WORD sipAddressType) const
{
	if (sipAddressType == PARTY_SIP_SIPURI_ID_TYPE)
	{
		return eSipAddressType_UriType;
	}
	if (sipAddressType == PARTY_SIP_TELURL_ID_TYPE)
	{
		return eSipAddressType_TelUrlType;
	}
	FTRACEINTO << "No such Sip Address Type " << (int)sipAddressType;
	return eSipAddressType_DEFAULT;
}


BoolAutoType PlcmCdrEventCallStartExtendedHelper::AutoEnumBoolToBoolAutoType(BYTE autoEnumVal)
{
	if (autoEnumVal == AUTO)
	{
		return eBoolAutoType_Auto;
	}
	if (autoEnumVal == YES)
	{
		return eBoolAutoType_Yes;
	}
	if (autoEnumVal == NO)
	{
		return eBoolAutoType_No;
	}
	return eBoolAutoType_Auto;
}

CallContentType PlcmCdrEventCallStartExtendedHelper::CallContentToPlcmCallContent(BYTE callContent)
{
	if (callContent == 0)
	{
		return eCallContentType_Framed;
	}
	else if (callContent == 1)
	{
		return eCallContentType_Voice;
	}

	FTRACEINTO << "Undefined callContent " << (int)callContent;
	return eCallContentType_DEFAULT;
}


void PlcmCdrEventCallStartExtendedHelper::SetNewIsdnUndefinedParty_BasicAndContinue(CConfParty& confParty, BYTE interfaceType, CCommConf& pCommConf)
{
	FTRACEINTO << "SetNewIsdnUndefinedParty conf party";

	m_cdrEventCallStartExtended.m_partyDetails.m_signallingType = InterfaceTypeToPlcmSignallingType(interfaceType);
	m_cdrEventCallStartExtended.m_partyDetails.m_id = confParty.GetPartyId();
	m_cdrEventCallStartExtended.m_partyDetails.m_name = confParty.GetName();
	m_cdrEventCallStartExtended.m_partyDetails.m_connection = ConnectionTypeToPlcmConnection(confParty.GetConnectionTypeOper());
	m_cdrEventCallStartExtended.m_partyDetails.m_netChannelNumber  = (NetChannelNumType)confParty.GetNetChannelNumber();

	m_cdrEventCallStartExtended.m_partyDetails.m_serviceName = (char*)(confParty.GetServiceProviderName());
	m_cdrEventCallStartExtended.m_partyDetails.m_subServiceName = (char*)(confParty.GetSubServiceName());
	m_cdrEventCallStartExtended.m_partyDetails.m_callContent = CallContentToPlcmCallContent(confParty.GetVoice());

	m_cdrEventCallStartExtended.m_partyDetails.m_numType = NumTypeToPlcmNetworkLineType(confParty.GetNumType());

	m_cdrEventCallStartExtended.m_partyDetails.m_identificationMethod = IdentMethodToPlcmIdentMethodType(confParty.GetIdentificationMethod());
	m_cdrEventCallStartExtended.m_partyDetails.m_meetMeMethod = MeetingMethodToPlcmMeetMeMethodeType(confParty.GetMeet_me_method());

	WORD indPhone = 0;
	Phone* pPhoneNum = confParty.GetActualPartyPhoneNumber(indPhone);
	int nPos = 0;
	std::stringstream otherPhones;

	while (pPhoneNum != NULL) {
		if (nPos == 0)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_phoneList.m_phone1 = pPhoneNum->phone_number;
		}
		else if (nPos == 1)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_phoneList.m_phone2 =pPhoneNum->phone_number;
		}
		else if (nPos == 2)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_phoneList.m_phone3 = pPhoneNum->phone_number;
		}
		else if (nPos == 3)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_phoneList.m_phone4 = pPhoneNum->phone_number;
		}
		else if (nPos == 4)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_phoneList.m_phone5 = pPhoneNum->phone_number;
		}
		else if (nPos == 5)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_phoneList.m_phone6 = pPhoneNum->phone_number;
		}
		else
		{
			otherPhones << pPhoneNum->phone_number << " ";
		}
		++nPos;
		indPhone++;
		pPhoneNum = confParty.GetActualPartyPhoneNumber(indPhone);
	}

	if (otherPhones.str().size() > 0)
	{
		m_cdrEventCallStartExtended.m_partyDetails.m_phoneList.m_phoneListExList = otherPhones.str();
		otherPhones.str("");
		otherPhones.clear();
	}
	nPos = 0;
	indPhone = 0;
	pPhoneNum = confParty.GetActualMCUPhoneNumber(indPhone);
	while (pPhoneNum != NULL)
	{
		if (nPos == 0)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_mcuPhoneList.m_phone1 = pPhoneNum->phone_number;
		}
		else if (nPos == 1)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_mcuPhoneList.m_phone2 = pPhoneNum->phone_number;
		}
		else if (nPos == 2)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_mcuPhoneList.m_phone3 = pPhoneNum->phone_number;
		}
		else if (nPos == 3)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_mcuPhoneList.m_phone4 = pPhoneNum->phone_number;
		}
		else if (nPos == 4)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_mcuPhoneList.m_phone5 = pPhoneNum->phone_number;
		}
		else if (nPos == 5)
		{
			m_cdrEventCallStartExtended.m_partyDetails.m_mcuPhoneList.m_phone6 = pPhoneNum->phone_number;
		}
		else
		{
			otherPhones << pPhoneNum->phone_number << " ";
		}
		++nPos;
	    indPhone++;
		pPhoneNum = confParty.GetActualMCUPhoneNumber(indPhone);

	}
	if (otherPhones.str().size() > 0)
	{
		m_cdrEventCallStartExtended.m_partyDetails.m_mcuPhoneList.m_phoneListExList = otherPhones.str();
		otherPhones.str("");
		otherPhones.clear();
	}

	m_cdrEventCallStartExtended.m_partyDetails.m_nodeType = NodeTypeToPlcmNodeType(confParty.GetNodeType ());
	m_cdrEventCallStartExtended.m_partyDetails.m_chair = (bool) confParty.GetIsLeader();

	char tempName[64];
	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	ipToString(confParty.GetIpAddress(),tempName,1);

	m_cdrEventCallStartExtended.m_partyDetails.m_ipAddress = tempName;
	m_cdrEventCallStartExtended.m_partyDetails.m_signallingPort = confParty.GetCallSignallingPort();
	m_cdrEventCallStartExtended.m_partyDetails.m_videoBitRate = CCdrPersistConverter::ConvertBitRate(confParty.GetVideoRate());
	m_cdrEventCallStartExtended.m_partyDetails.m_videoProtocol = VideoProtocolToPlcmVideoProtocolType(confParty.GetVideoProtocol());
	if (H323_INTERFACE_TYPE == interfaceType)
	{
		m_cdrEventCallStartExtended.m_partyDetails.m_alias.m_name = confParty.GetH323PartyAlias();
		m_cdrEventCallStartExtended.m_partyDetails.m_alias.m_aliasType = AliasTypeToPlcmAliasType(confParty.GetH323PartyAliasType());
	}
	else if (SIP_INTERFACE_TYPE == interfaceType)
	{
		m_cdrEventCallStartExtended.m_partyDetails.m_sipAddress = confParty.GetSipPartyAddress();
		m_cdrEventCallStartExtended.m_partyDetails.m_sipAddressType = SipAddressToPlcmSipAddressType(confParty.GetSipPartyAddressType());
	}
	 m_cdrEventCallStartExtended.m_partyDetails.m_volume = confParty.GetAudioVolume();
	 m_cdrEventCallStartExtended.m_partyDetails.m_undefined = (bool)confParty.GetUndefinedType();
	 m_cdrEventCallStartExtended.m_partyDetails.m_conferenceCorrelationId = pCommConf.GetCorrelationId();
	 //continue1

	 m_cdrEventCallStartExtended.m_encryption = AutoEnumBoolToBoolAutoType(confParty.GetIsEncrypted());

}

void PlcmCdrEventCallStartExtendedHelper::SetNewIsdnUndefinedPartyContinue1(COperAddPartyCont2& operAddPartyCont2)
{
	FTRACEINTO << "SetNewIsdnUndefinedPartyContinue1";

	m_cdrEventCallStartExtended.m_encryption = AutoEnumBoolToBoolAutoType(operAddPartyCont2.GetIsEncryptedParty());
	m_cdrEventCallStartExtended.m_partyDetails.m_id = operAddPartyCont2.GetPartyId();
	m_cdrEventCallStartExtended.m_partyDetails.m_name = operAddPartyCont2.GetPartyName();
}

