
#include "TestAudioBridge.h"
#include "AudioBridgeInterfaceMock.h"
#include <iostream>
#include "BridgeInitParams.h"
#include "ConfMock.h"
#include "PartyMock.h"
#include "BridgePartyMediaParams.h"
#include "AudioBridgeInterface.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION( CTestAudioBridge );

void CTestAudioBridge::setUp()
{
	/*
	m_pConfPartyProcess = new CConfPartyProcess();
	m_pConfPartyProcess->SetUp();
	m_audInterfaceMock = new CAudioBridgeInterfaceMock();
	m_conf = new CConfMock();
	m_conf->CreateMock();

	m_pParty	= new CPartyMock;

	((CPartyMock*)m_pParty)->CreateMock();

	strncpy(m_pConfName, "Conf_1", H243_NAME_LEN);
	m_pConfName[H243_NAME_LEN-1]='\0';

	strncpy(m_pPartyName, "First", H243_NAME_LEN);
	m_pPartyName[H243_NAME_LEN-1]='\0';
	
	m_partyRsrcID	= 1;
	*/
}

void CTestAudioBridge::tearDown()
{
	/*
	POBJDELETE(m_audInterfaceMock);
	POBJDELETE(m_conf);
	POBJDELETE(m_pParty);
	m_pConfPartyProcess->TearDown();
	POBJDELETE(m_pConfPartyProcess);
	*/

}

void CTestAudioBridge::TestBridgeCreation()
{
	/*
	 *	This Test Init the bridge params and creates an audio interface
	 *  The Audio Interface also allocate a new Bridge by the Bridge id 
	 *  we supply to him, this test checks if we really created an
	 *  Audio Bridge
	 */
	//CBridgeInitParams brdgParams(m_conf,"UdiConf",1, eAudio_Bridge_V1);
 	//m_audInterfaceMock->Create(&brdgParams);
	//CBridge* brdgImpel = m_audInterfaceMock->MockGetBridgeImplementation();
	//CPPUNIT_ASSERT(strcmp("CAudioBridge",brdgImpel->NameOf()) == 0);
	
}
  

//void CTestAudioBridge::TestPartyConnection()
//{
//	/*
//	 *	This Test Will activate ConnectParty on the AudioBridgeInterface
//	 *  we expect to Create in the Bridge an AudioBridgePArtyCntl
//	 *  that will hold 2 AudioUniDirections
//	 */
//	CBridgeInitParams brdgParams(m_conf,"UdiConf",eAudio_Bridge_V1);
	
//	// Create Audio Interface with 1 bridge
 //	m_audInterfaceMock->Create(&brdgParams);

//	//Getting the bridge Impl
//	CBridge* brdgImpel = m_audInterfaceMock->MockGetBridgeImplementation();
	
//	CBridgePartyMediaParams * pInMediaParams= new CBridgePartyMediaParams();
//	CBridgePartyMediaParams * pOutMediaParams= new CBridgePartyMediaParams();
//	CBridgePartyInitParams * partyInitParams= new CBridgePartyInitParams("TestParty",
//		m_pParty,m_partyRsrcID,
//		"UdiConf",m_conf,brdgImpel
//		,pInMediaParams,pOutMediaParams);
	
////	CSegment * cSeg = new CSegment();
////	partyInitParams->Serialize(NATIVE,*cSeg);

////	//CBridgePartyInitParams * partyInitParams2= new CBridgePartyInitParams();
////	CBridgePartyInitParams partyInitParams2;
////	partyInitParams2.DeSerialize(NATIVE,*cSeg);

//	m_audInterfaceMock->ConnectParty(partyInitParams);
	
//	delete pInMediaParams;
//	delete pOutMediaParams;
//}

