//+========================================================================+
//					      Copyright 2005 Polycom                           |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Networking Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
#ifndef __CONF_APPLICATIONS_ACTIVE_EVENTS_LIST_H__
#define __CONF_APPLICATIONS_ACTIVE_EVENTS_LIST_H__



#include "ConfAppMngr.h"
#include "ConfAppPartiesList.h"
#include "PObject.h"

class CConfAppInfo;
class CConfAppPartiesList;
class CConfAppFeatureConfIVR;
class CConfAppFeaturePartyIVR;


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
typedef enum
{
	F_APP_DUMMY = 0,
	F_APP_FIRST_PARTY = 1,
	F_APP_ROLLCALL,
	F_APP_SINGLE_PARTY,
	F_APP_ONHOLD,
	F_APP_CONF_IVR,
	F_APP_PARTY_IVR,
	F_APP_FEATURES_LAST
} TAppFeatures;




class CConfAppFeatureObject;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class CConfAppActiveEventsList : public CPObject
{
	CLASS_TYPE_1(CConfAppActiveEventsList,CPObject)

public:
	CConfAppActiveEventsList( CConfAppInfo *confAppInfo, CConfAppPartiesList* participants );
	CConfAppActiveEventsList(const CConfAppActiveEventsList& other );
	virtual const char* NameOf() const { return "CConfAppActiveEventsList";}
	CConfAppActiveEventsList& operator=(const CConfAppActiveEventsList& other); 
	
	virtual ~CConfAppActiveEventsList();
	

	int  AskForObjection( TConfAppEvents feature, DWORD partyRsrcID, WORD onOff );
	void StartPartyIVRNewFeature( DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyRsrcID );
	void StopPartyIVRNewFeature(  DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyRsrcID );

	void StartConfIVRNewFeature( DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyRsrcID );
	void StopConfIVRNewFeature(  DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyRsrcID );
	
	void StartNewFeature( DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyRsrcID, WORD onOff, WORD confOrParty );
	int DoSomethingWithEndIVREvent( DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyRsrcID );
	void SetFlagSlideIsNoLongerPermitted( DWORD partyRsrcID, DWORD yesNo );
	int DoSomethingWithEndPartyEvent( DWORD uniqueEventNumber, TConfAppEvents featureOpcode, DWORD partyRsrcID );
///	int DoSomethingWithSetAsChairEvent( DWORD uniqueEventNumber, TConfAppEvents featureOpcode, DWORD partyRsrcID );
	void DoSomethingWithSetAsChairEvent( DWORD partyRsrcID );
	void StopFeature( DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyID, WORD confOrParty );
	
	int AskForObjection( TConfAppEvents feature, DWORD partyRsrcID );
	void AskForPreAction( TConfAppEvents feature, DWORD partyRsrcID );
	void PlayMessage( CSegment* pParam );
	void RecordRollCall( CSegment* pParam );
	void StopMessage( CSegment* pParam );
	void PlayMessageInd( DWORD opcode, DWORD partyRsrcID, DWORD messageID );
	void EnterPartyToMix( DWORD partyRsrcID );
	void DtmfReceived( DWORD opcode, CSegment* pParam );
	int IsEventAllowed( const TConfAppEvents featureOpcode);
	int IsOpcodeAllowed( const TConfAppEvents featureOpcode );
	void RollCallRecorded( CSegment* pParam );
	int  DoPartyAction( DWORD opcode, DWORD partyRsrcID, DWORD action, CSegment* pParam=NULL );
	void StopRollCallRecording( CSegment* pParam );
	void StopRollCallRecordingAck(DWORD PartyId,DWORD status);
	
	//int  DoConfAction( DWORD opcode, DWORD partyRsrcID, DWORD action );
	int  DoConfAction( DWORD opcode, DWORD partyRsrcID, CSegment *pSeg );

	void DoSomethingUponPartyLeavesConf();

	
	
	void RemoveAllIVREventsUponConfTerminating();
	CConfAppFeaturePartyIVR* GetPartyIVR(){ return m_partyIVR; } 
	CConfAppFeatureConfIVR* GetConfIVR(){ return m_confIVR; } 
	void ReplaceTokenWithSequenceNum( CSegment* pParam );
	void HandlePlayMsgAckInd( CSegment* pParam );
	void HandleChangeIC();
	void RemoveFeatureByOpcode(DWORD opcode);
	DWORD FindSingleParty();
	
protected:
	void AddActiveFeature( int ind, TConfAppEvents featureCode );
	int FindFeatureForMessageInd(  DWORD messageID, DWORD& mainInd, DWORD& subInd );
	CConfAppFeatureObject* FindMainFeature( TConfAppEvents feature );
	int IsNeedEndWaitForChair(DWORD partyRsrcID);
	int IsNeedEndSinglePartyMusic();
	int IsNeedSinglePartyMusic();
	int IsNeedFirstToJoin(DWORD partyRsrcID);
	int IsNeedPartyWaitForChair();
	int IsNeedConfWaitForChair(DWORD partyRsrcID);
	int IsNeedEntryTone( DWORD partyRsrcID );
	int IsNeedChairDropped( DWORD partyRsrcID );
	int IsNeedExitTone( DWORD partyRsrcID );
///	int IsLastPartyDisconnecting();
	//DWORD FindSingleParty();
	void UpdateLeaderInConf();
	int IsNeedRecInProg( DWORD partyRsrcID );
	int IsNeedRecFailed( DWORD partyRsrcID );
	int IsNeedPlayRingingTone(DWORD partyRsrcID );
	int IsNeedStopPlayRingingTone(DWORD partyRsrcID );
protected:
	CConfAppFeatureConfIVR*		m_confIVR;
	CConfAppFeaturePartyIVR*	m_partyIVR;
	CConfAppInfo*			m_confAppInfo;
	CConfAppPartiesList* 	m_participants;

};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////



#endif	// __CONF_APPLICATIONS_ACTIVE_EVENTS_LIST_H__



