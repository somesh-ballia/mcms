#ifndef _TEST_BRIDGE_INTERFACE_H__
#define _TEST_BRIDGE_INTERFACE_H__

// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

#include "Bridge.h"
#include "BridgeInitParams.h"
#include "ConfPartyProcess.h"

class CBridgeInterface;
class CParty;

class CTestBridgeInterface   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestBridgeInterface );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testCreate );
	//CPPUNIT_TEST( testCreateWithZeroBridgeImplementations );
	CPPUNIT_TEST( testIsBridgeConnected );
	CPPUNIT_TEST( testIsPartyConnectedForEmptyBridge );
	CPPUNIT_TEST( testConnectPartyAndCheckIsPartyConnectedAndNumOfParties );
	CPPUNIT_TEST( testDisConnectPartyAndCheckIsPartyConnectedAndNumOfParties );

	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testCreate();
	//void testCreateWithZeroBridgeImplementations();
	void testIsBridgeConnected();
	void testIsPartyConnectedForEmptyBridge();
	void testConnectPartyAndCheckIsPartyConnectedAndNumOfParties();
	void testDisConnectPartyAndCheckIsPartyConnectedAndNumOfParties();

protected:
	CBridgeInterface* m_pBridgeInterface;
	CConf*		m_pConf;
	CParty*		m_pParty;
	char		m_pConfName[H243_NAME_LEN];
	char		m_pPartyName[H243_NAME_LEN];
	EBridgeImplementationTypes	m_eBridgeImplementationType;
	PartyRsrcID m_partyRsrcID;
	ConfRsrcID	m_confRsrcID;
	WORD		m_wNetworkInterface;
	WORD		m_partyRoomId;
};


#endif /* _TEST_BRIDGE_INTERFACE_H__ */

