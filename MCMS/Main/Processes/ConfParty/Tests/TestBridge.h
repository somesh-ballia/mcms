
#ifndef _TEST_BRIDGE_H__
#define _TEST_BRIDGE_H__

#include <cppunit/extensions/HelperMacros.h>
#include "Bridge.h"
#include "BridgeInitParams.h"
#include "ConfPartyProcess.h"

class CTestBridge   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestBridge );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testDestructor );
	CPPUNIT_TEST( testCreate );
	CPPUNIT_TEST( testDestroy );
	CPPUNIT_TEST( testCheckIsConnectedAfterConstructorAndCreate );
	CPPUNIT_TEST( testCheckIsConnectedAfterCreateAndChangingTheState );
	CPPUNIT_TEST( testIsPartyConnectedForEmptyBridge );
	CPPUNIT_TEST( testConnectParty );
	CPPUNIT_TEST( testConnectThreeParties );
	CPPUNIT_TEST( testConnectThreePartiesAndCheckPartyNumber );
	//CPPUNIT_TEST( testConnectPartyNULL ); TO DO
	CPPUNIT_TEST( testDisConnectPartyWhenOnePartyIsConnected );
	CPPUNIT_TEST( testDisConnectTwoPartiesWhenThreePartiesAreConnected );
	CPPUNIT_TEST( testGetPartyCntlByTaskAppPtrWhenThreePartiesAreConnected );
	CPPUNIT_TEST( testGetPartyCntlByNameWhenThreePartiesAreConnected );
	CPPUNIT_TEST( testGetPartyCntlByPartyRsrcIdWhenThreePartiesAreConnected );

	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testDestructor();
	void testCreate();
	void testDestroy();
	void testCheckIsConnectedAfterConstructorAndCreate();
	void testCheckIsConnectedAfterCreateAndChangingTheState();
	void testIsPartyConnectedForEmptyBridge();
	void testConnectParty();
	void testConnectThreeParties();
	void testConnectThreePartiesAndCheckPartyNumber();
	//void testConnectPartyNULL(); TO DO
	void testDisConnectPartyWhenOnePartyIsConnected();
	void testDisConnectTwoPartiesWhenThreePartiesAreConnected();
	void testGetPartyCntlByTaskAppPtrWhenThreePartiesAreConnected();
	void testGetPartyCntlByNameWhenThreePartiesAreConnected();
	void testGetPartyCntlByPartyRsrcIdWhenThreePartiesAreConnected();

private:
	CParty* CreateParty();

private:
	CBridge*	m_pBridge;
	CConf*		m_pConf;
	char		m_pConfName[H243_NAME_LEN];
	EBridgeImplementationTypes	m_eBridgeImplementationType;
};

#endif /* _TEST_BRIDGE_H__ */

