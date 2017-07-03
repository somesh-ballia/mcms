
#ifndef _TEST_AUDIO_BRIDGE_H_
#define _TEST_AUDIO_BRIDGE_H_


// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>
#include "ConfPartyProcess.h"
#include "ConfPartyDefines.h"
#include "BridgeDefs.h"
#include "RsrcDesc.h"

class CAudioBridgeInterfaceMock;
class CConfMock ;
class CParty ;




class CTestAudioBridge   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestAudioBridge );
	CPPUNIT_TEST(TestBridgeCreation);
	//CPPUNIT_TEST(TestPartyConnection);
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	void TestBridgeCreation();
	//void TestPartyConnection();

private:
	CAudioBridgeInterfaceMock * m_audInterfaceMock ;
	CConfMock * m_conf ;
	CParty*		m_pParty;
	char		m_pConfName[H243_NAME_LEN] ;
	char		m_pPartyName[H243_NAME_LEN];
	EBridgeImplementationTypes	m_eBridgeImplementationType;
	PartyRsrcID m_partyRsrcID;
};


#endif 
