	#ifndef _CDR_PERSIST_HELPER
#define _CDR_PERSIST_HELPER

//#include "PObject.h"
//#include "CDREvent.h"

//#include "BilParty.h"
#include "CdrPersistApiEnums.h"
#include "PlcmCdrEventConnected.h"
#include "PlcmCdrEventDisconnectedExtended.h"
#include "PlcmCdrEventCallStartExtended.h"
#include "DataTypes.h"
#include "OperEvent.h"
#include "CommConf.h"


class CPartyConnected;
class CSvcSipPartyConnected;
class CPartyDisconnected;
class CPartyDisconnectedCont1;
class CConfParty;


class CCdrPersistConverter
{
public:
	virtual std::string GetTypeOf() = 0;

	// TODO: when all objects from CommConf will remove to here - remove the statis
	static std::string ConvertBitRate(DWORD bitRatelType);

	protected:

		std::string PartystateToPlcmString(int type, DWORD partyState) const;

		SignallingType InterfaceTypeToPlcmSignallingType(BYTE interfaceType) const;

		virtual ~CCdrPersistConverter() {}

	private:

};


class PlcmEventConnectedHelper : public CCdrPersistConverter
{
	public:

	virtual std::string GetTypeOf() { return "PlcmEventConnectedHelper"; }

	void SetDataPartyConnected(BYTE interfaceType, const CPartyConnected& partyConnected) ;
	void SetDataSvcPartyConnected(BYTE interfaceType, const CSvcSipPartyConnected& svcSipPartyConnected);

	virtual ~PlcmEventConnectedHelper() {}

	PlcmCdrEventConnected& GetCdrObject() { return m_cdrEventConnected; }



	private:

	PlcmCdrEventConnected m_cdrEventConnected;

	SvcPartyStatusType PartyStateToSvcPartyStatusType(DWORD partyState) const;
	RelayCodecType AudiCodecToRelayCodecType(DWORD audiCodec) const;
	void SetLocalAndRemoteMode(WORD eventType, const CPartyConnected& partyConnected, std::string &localCommMode, std::string &remoteCommMode) const;

};

class PlcmCdrEventDisconnectedExtendedHelper  : public CCdrPersistConverter
{
	public:

		virtual std::string GetTypeOf() { return "PlcmCdrEventDisconnectedExtendedHelper"; }

		void SetDataFromPartyDisconnected(const CPartyDisconnected& partyDisconnected);

		void SetDataFromPartyDisconnected1(const CPartyDisconnectedCont1& partyDisconnectedCont1);

		PlcmCdrEventDisconnectedExtended& GetCdrObject() { return m_cdrEventDisconnectedExtended; }


		virtual ~PlcmCdrEventDisconnectedExtendedHelper() {}

		void SetInterfaceType(BYTE interfaceType);

	private:

		PlcmCdrEventDisconnectedExtended m_cdrEventDisconnectedExtended;

};


class PlcmCdrEventCallStartExtendedHelper  : public CCdrPersistConverter
{
public:

	virtual std::string GetTypeOf() { return "PlcmCdrEventCallStartExtendedHelper"; }


	void SetNewIsdnUndefinedParty_BasicAndContinue(CConfParty& confParty, BYTE interfaceType, CCommConf& pCommConf);

	void SetNewIsdnUndefinedParty(CAddPartyDetailed& addPartyDetailed, BYTE interfaceType);


	void SetNewIsdnUndefinedPartyContinue1(COperAddPartyCont2& operAddPartyCont2);


	PlcmCdrEventCallStartExtended& GetCdrObject() { return m_cdrEventCallStartExtended; }


	virtual ~PlcmCdrEventCallStartExtendedHelper() {}

private:

	NetworkLineType NumTypeToPlcmNetworkLineType(BYTE numType) const;

	Connection ConnectionTypeToPlcmConnection(BYTE connectionType);

	MeetMeMethodeType MeetingMethodToPlcmMeetMeMethodeType(BYTE meetingMethod) const;
	IdentMethodType IdentMethodToPlcmIdentMethodType(BYTE identMethod) const;

	NodeType NodeTypeToPlcmNodeType(BYTE nodeType) const;
	VideoProtocolType VideoProtocolToPlcmVideoProtocolType(BYTE videoProtocolType) const;
	AliasType AliasTypeToPlcmAliasType(WORD aliasType) const;
	SipAddressType SipAddressToPlcmSipAddressType(WORD sipAddressType) const;

	BoolAutoType AutoEnumBoolToBoolAutoType(BYTE autoEnumVal);

	CallContentType CallContentToPlcmCallContent(BYTE callContent);

	PlcmCdrEventCallStartExtended m_cdrEventCallStartExtended;


};


// Usage example:
// PlcmEventConnectedHelper cdrEventConnectedHelper;
// cdrEventConnectedHelper.SetDataPartyConnected(IP_PARTY_CONNECTED, partyConnected);
// SendCdrEvendToCdrManager((ApiBaseObjectPtr)&cdrEventConnectedHelper.GetCdrObject(), false, cdrEventConnectedHelper.GetCdrObject().m_partyId);
// (SendCdrEvendToCdrManager is currently defined in CommConf.h)


#endif //_CDR_PERSIST_ENUM_MAPPER
