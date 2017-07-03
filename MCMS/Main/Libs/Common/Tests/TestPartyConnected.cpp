#include "TestPartyConnected.h"
#include "BilParty.h"
#include "H221Str.h"
#include "NStream.h"
#include "OperEvent.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestPartyConnected );


TestPartyConnected::TestPartyConnected()
{
}

TestPartyConnected::~TestPartyConnected()
{
}

void TestPartyConnected::setUp()
{
}

void TestPartyConnected::tearDown()
{
}

void TestPartyConnected::testSerializeDeSerialize()
{
	CPartyConnected party1;
	
	party1.SetPartyName("Cucu-Lulu");
	party1.SetPartyId(42);
	party1.SetPartyState(PARTY_CONNECTED);
	party1.SetSecondaryCause(CAUSE_RESTRICT);

	CH221Str capabilities;
	const char *cap = "12345";
	capabilities.SetH221FromString(strlen(cap), cap);
	
	CH221Str remoteCommMode;
	const char *mode = "6789";
	remoteCommMode.SetH221FromString(strlen(mode), mode);
	
//	party1.SetCapabilities(capabilities);
//	party1.SetRemoteCommMode(remoteCommMode);

	COstrStream ostr;
    party1.Serialize(0, ostr, 0);
    
    const string tmpStr = ostr.str();
    
    CIstrStream istr(ostr.str());
	CPartyConnected party2;
	party2.DeSerialize(0, istr, 0);

    CPPUNIT_ASSERT(party1 == party2);
}

void TestPartyConnected::testPartyDetailedSerializeDeSerialize()
{
	CAddPartyDetailed party1;
	
	party1.SetPartyName("Cucu-Lulu");
	party1.SetPartyId(42);

		party1.SetConnectionType(DIAL_IN);
//		rsrvStart->SetBondingMode(pConfParty->GetBondingMode1());
		party1.SetNetNumberChannel(1);
//		rsrvStart->SetNetChannelWidth(pConfParty->GetNetChannelWidth());
		party1.SetNetServiceName("Cucu-Lulu1");
//		rsrvStart->SetRestrict(pConfParty->GetRestrict());
		party1.SetVoice(2);
		//rsrvStart->SetNumType(pConfParty->GetNumType());
		party1.SetNetSubServiceName("Cucu-Lulu2");
		party1.SetIdentMethod(3);
		party1.SetMeetMeMethod(4);
/*		Phone* pPhoneNum = pConfParty->GetFirstCallingPhoneNumber();						  
		while (pPhoneNum != NULL) {
			rsrvStart->AddPartyPhoneNumber(pPhoneNum->phone_number);   
			pPhoneNum = pConfParty->GetNextCallingPhoneNumberOper();
		}
		pPhoneNum = pConfParty->GetFirstCalledPhoneNumber();						  
		while (pPhoneNum != NULL) {
			rsrvStart->AddMcuPhoneNumber(pPhoneNum->phone_number);   
			pPhoneNum = pConfParty->GetNextCalledPhoneNumber();
		}*/
		party1.SetNodeType(5);                 
//		rsrvStart->SetChair(pConfParty->GetChair ());
//		rsrvStart->SetPassword(pConfParty->GetPassword ());                 
		party1.SetIpAddress(6);
		party1.SetCallSignallingPort(7);
		party1.SetVideoProtocol(8);
		party1.SetVideoRate(9);
		//rsrvStartCont1.SetBondingPhoneNumber(pConfParty->GetBondingPhoneNumber ());
	//	rsrvStart->SetBondingPhoneNumber((const ACCCdrPhone *)pConfParty->GetBondingPhoneNumber ());
		
//		BYTE interfaceType = pConfParty->GetNetInterfaceType();
//		if (H323_INTERFACE_TYPE == interfaceType)
//		{
//			party1.SetIpPartyAliasType(pConfParty->GetH323PartyAliasType ());
//			party1.SetIpPartyAlias(pConfParty->GetH323PartyAlias ());                 
//		}
//		else if (SIP_INTERFACE_TYPE == interfaceType)
//		{
			party1.SetSipPartyAddressType(PARTY_SIP_SIPURI_ID_TYPE);
			party1.SetSipPartyAddress("Cucu-Lulu@1.2.3.4"); 
//		}                 

		party1.SetAudioVolume(10);
		party1.SetUndefinedType(11);
		party1.SetNetInterfaceType(5);



	COstrStream ostr;
    party1.Serialize(0, ostr, 0);
    
    const string tmpStr = ostr.str();
    
    CIstrStream istr(ostr.str());
	CAddPartyDetailed party2;
	party2.DeSerialize(0, istr, 0);

    CPPUNIT_ASSERT(party1 == party2);
}
