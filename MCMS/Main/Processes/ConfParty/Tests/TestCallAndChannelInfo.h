
#if !defined(TEST_CALLANDCHANNELINFO_H)
#define TEST_CALLANDCHANNELINFO_H

#include <cppunit/extensions/HelperMacros.h>

#include "CallAndChannelInfo.h"
#include "ConfPartyProcess.h"



// private tests (example)

class CTestCall   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestCall );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testDestructor );
 	CPPUNIT_TEST( testSetCallIndex );
 	CPPUNIT_TEST( testGetCallIndex );
	CPPUNIT_TEST( testSetCallStatus );
 	CPPUNIT_TEST( testGetCallStatus );
 	CPPUNIT_TEST( testSetConnectionId );
 	CPPUNIT_TEST( testGetConnectionId );
 	CPPUNIT_TEST( testIncreaseChannelsCounter );
 	CPPUNIT_TEST( testDecreaseChannelsCounter );
 	CPPUNIT_TEST( testGetChannelsCounter );
 	CPPUNIT_TEST( testGetSrcTerminalParams );
 	CPPUNIT_TEST( testSetSrcTerminalParams );
 	CPPUNIT_TEST( testGetDestTerminalParams );
 	CPPUNIT_TEST( testSetDestTerminalParams );
 	CPPUNIT_TEST( testGetSourceInfoAlias );
 	CPPUNIT_TEST( testSetSourceInfoAlias );
 	CPPUNIT_TEST( testGetSrcInfoAliasType );
 	CPPUNIT_TEST( testSetSrcInfoAliasType );
 	CPPUNIT_TEST( testGetDestinationInfoAlias );
 	CPPUNIT_TEST( testSetDestinationInfoAlias );
 	CPPUNIT_TEST( testGetDestInfoAliasType );
 	CPPUNIT_TEST( testSetDestInfoAliasType );
 	CPPUNIT_TEST( testGetCanMapAlias );
 	CPPUNIT_TEST( testSetCanMapAlias );
 	CPPUNIT_TEST( testGetCallId );
 	CPPUNIT_TEST( testSetCallId );
 	CPPUNIT_TEST( testGetCallType );
 	CPPUNIT_TEST( testSetCallType );
// 	CPPUNIT_TEST( testGetCallModelType );
// 	CPPUNIT_TEST( testSetCallModelType );
 	CPPUNIT_TEST( testGetIsActiveMc );
 	CPPUNIT_TEST( testSetIsActiveMc );
 	CPPUNIT_TEST( testGetIsOrigin );
 	CPPUNIT_TEST( testSetIsOrigin );
 	CPPUNIT_TEST( testGetIsClosingProcess );
 	CPPUNIT_TEST( testSetIsClosingProcess );
 	CPPUNIT_TEST( testGetCallCloseInitiator );
 	CPPUNIT_TEST( testSetCallCloseInitiator );
 	CPPUNIT_TEST( testGetSetupH245Address );
 	CPPUNIT_TEST( testSetSetupH245Address );
 	CPPUNIT_TEST( testGetAnswerH245Address );
 	CPPUNIT_TEST( testSetAnswerH245Address );
 	CPPUNIT_TEST( testGetControlH245Address );
 	CPPUNIT_TEST( testSetControlH245Address );
 	CPPUNIT_TEST( testGetSpecificChannel );
 //	CPPUNIT_TEST( testSetCurrentChannel );
 	CPPUNIT_TEST( testRemoveChannel );

	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 

	void testConstructor();
	void testDestructor();
	
	void testSetCallIndex();
 	void testGetCallIndex();
	void testSetCallStatus();
 	void testGetCallStatus();
 	void testSetConnectionId();
 	void testGetConnectionId();
 	void testIncreaseChannelsCounter();
 	void testDecreaseChannelsCounter();
 	void testGetChannelsCounter();
 	void testGetSrcTerminalParams();
 	void testSetSrcTerminalParams();
 	void testGetDestTerminalParams();
 	void testSetDestTerminalParams();
 	void testGetSourceInfoAlias();
 	void testSetSourceInfoAlias();
 	void testGetSrcInfoAliasType();
 	void testSetSrcInfoAliasType();
 	void testGetDestinationInfoAlias();
 	void testSetDestinationInfoAlias();
 	void testGetDestInfoAliasType();
 	void testSetDestInfoAliasType();
 	void testGetCanMapAlias();
 	void testSetCanMapAlias();
 	void testGetCallId();
 	void testSetCallId();
 	void testGetCallType();
 	void testSetCallType();
// 	void testGetCallModelType();
// 	void testSetCallModelType();
 	void testGetIsActiveMc();
 	void testSetIsActiveMc();
 	void testGetIsOrigin();
 	void testSetIsOrigin();
 	void testGetIsClosingProcess();
 	void testSetIsClosingProcess();
 	void testGetCallCloseInitiator();
 	void testSetCallCloseInitiator();
 	void testGetSetupH245Address();
 	void testSetSetupH245Address();
 	void testGetAnswerH245Address();
 	void testSetAnswerH245Address();
 	void testGetControlH245Address();
 	void testSetControlH245Address();
 	void testGetSpecificChannel();
    void testSetCurrentChannel();
	void testRemoveChannel();

	CCall*			   m_pCall;
	
};

class CTestChannel   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestChannel );
	CPPUNIT_TEST( testConstructor );
 	CPPUNIT_TEST( testGetIndex );
 	CPPUNIT_TEST( testSetIndex );
 	CPPUNIT_TEST( testGetChannelDirection );
 	CPPUNIT_TEST( testSetChannelDirection );
 	CPPUNIT_TEST( testGetIsActive );
 	CPPUNIT_TEST( testSetIsActive );
 	CPPUNIT_TEST( testUpdateChannelParams );
 	CPPUNIT_TEST( testGetChannelCloseInitiator );
 	CPPUNIT_TEST( testSetChannelCloseInitiator );
 	CPPUNIT_TEST( testGetPayloadType );
 	CPPUNIT_TEST( testSetPayloadType );
 	CPPUNIT_TEST( testGetCapNameEnum );
 	CPPUNIT_TEST( testSetCapNameEnum );
 	CPPUNIT_TEST( testGetType );
 	CPPUNIT_TEST( testSetType );
 	CPPUNIT_TEST( testGetRoleLabel );
 	CPPUNIT_TEST( testSetRoleLabel );
 	CPPUNIT_TEST( testGetChannelParams );
 	CPPUNIT_TEST( testSetChannelParams );

	//...
	CPPUNIT_TEST_SUITE_END();

public:
    // Constructors 
	void setUp();
	void tearDown(); 

	void testConstructor();

	// Operations

	void		testUpdateChannelParams();

	void		testGetIndex();
	void		testSetIndex();

	void		testGetStatus();
	void		testSetStatus();

	void		testGetChannelDirection();
	void		testSetChannelDirection();

	void		testGetIsActive();
	void		testSetIsActive();

	void		testGetChannelCloseInitiator();
	void		testSetChannelCloseInitiator();

	void		testGetPayloadType();
	void		testSetPayloadType();

	void		testGetCapNameEnum();
	void		testSetCapNameEnum();

	void		testGetType();
	void		 testSetType();

	void		testGetRoleLabel();
	void		testSetRoleLabel();

	void		testGetChannelParams();
	void		testSetChannelParams();


	CChannel*		   m_pChannel;

};


#endif // !defined(TEST_CALLANDCHANNELINFO_H)
