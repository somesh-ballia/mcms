//+========================================================================+
//                            SIPPartyControlAdd.h                         |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyControlAdd.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPPARTYCONTORLADD__
#define __SIPPARTYCONTORLADD__

#include "IpPartyControl.h"

class CSipAddPartyCntl: public CSipPartyCntl
{
CLASS_TYPE_1(CSipAddPartyCntl, CSipPartyCntl)

public:
	CSipAddPartyCntl();
	virtual ~CSipAddPartyCntl();

	virtual void Create(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	void Reconnect(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	virtual const char* NameOf() const {return "CSipAddPartyCntl";}                
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	void ConnectBridgesSetup();
	virtual void HandleReAllocationForNotOfferer();
	void OnPartyChannelsConnectedSetup(CSegment* pParam);
	void OnPartyChannelsConnectedConnecting(CSegment* pParam);
	void OnPartyRemoteConnected(CSegment* pParam);
	
	void ChangeScm(CIpComMode* pScm,BYTE IsAsSipContentEnable);
	void OnPartyRemoteCapsRecieved(CSegment* pParam);
	void OnTimerConnectDelay(CSegment* pParam);
	void OnPartyRsrcAllocatingRsp(CSegment* pParam);

	void OnPartyMPLCreatingRsp(CSegment* pParam);
//	void  OnPartyUpdateVisualName(CSegment* pParam);
	void OnAudConnectPartyConnectAudio(CSegment* pParam);
	void OnVideoBrdgConnected(CSegment* pParam);
	void OnVideoBrdgDisconnectedConnectBridges(CSegment* pParam);
	void OnVideoBrdgDisconnectedNeedToRealloc(CSegment* pParam);
	void OnRsrcReAllocatePartyRspReAllocate(CSegment* pParam);
	void OnVideoBrdgConnectedAfterRealloc(CSegment* pParam);
	void OnConnectToutConnectBridgesAfterRealloc(CSegment* pParam);
	void OnFeccBrdgCon(CSegment* pParam);
	
	virtual void HandleVideoBridgeUpdate(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges);

	void  OnConnectToutAllocate(CSegment* pParam);
	void  OnConnectToutCreate(CSegment* pParam);
	void  OnConnectToutUpdateVideo(CSegment* pParam);
	void  OnConnectToutConnectBridges(CSegment* pParam);
	void  OnConnectToutPartySetup(CSegment* pParam);
	void  OnConnectToutConnectingState(CSegment* pParam);
	void  OnConnectToutNeedToRealloc(CSegment* pParam);
	void  OnConnectToutReAllocate(CSegment* pParam);
	void  OnTimerWaitForRsrcAndAskAgainAllocate(CSegment* pParam);

	void  OnVideoBrdgDisconnectedUpdateLegacyStatus(CSegment* pParam);
	void  OnConnectToutUpdateLegacyStatus(CSegment* pParam);
	void  OnVideoBrdgConnectedUpdateLegacyStatus(CSegment* pParam);
	DWORD UpdateScmWithResources(SVC_PARTY_IND_PARAMS_S  &aSvcParams,eVideoPartyType allocatedVideoPartyType, BOOL isAvcVswInMixedMode);
	DWORD UpdateCapsWithMsftResources(MS_SSRC_PARTY_IND_PARAMS_S  &msSsrcParams);
	virtual void HandleVideoBridgeUpdateForChannelHandle(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges);
	void CheckResourceAllocationForTipAndUpdateRate(eVideoPartyType allocatedVideoPartyType);

	void SetMsConfInviteReq(sipSdpAndHeaders* pSdpAndHeaders,CSipNetSetup * pNetSetup);
	void SetFocusUri();
	void CreateAVMCUMngrs();
	void OnMSOrganizerEndConnection(CSegment* pParam);
	void OnMSFocusEndConnection(CSegment* pParam);
	void OnMSSubscriberEndConnection(CSegment* pParam);
	void ContinueWithDialOutFlow();
	void CreateScmAccordingToAvMcuOptimizeMode();
	void UpdateToStrForSDP(char* ToAddrStr);
	void SetLocalSipHostAddress();


	void ActiveMedia();

protected:

	PDECLAR_MESSAGE_MAP;

	void DialOut(DWORD redialInterval, BYTE eTransportType = 0);
	BYTE CheckNameValidityAndDisconnectIfNeeded();
	void StartPartyConnection();
	virtual void AllocatePartyResources();
	virtual void EstablishCall(BYTE eTransportType = 0);
	void OffererPartyConnected(CSipComMode* pRemoteMode);
	void EndOffererConnected();
	void AnswererPartyConnected();
//	void UpdateLocalCapsAndScmAccordingToRemote();
	BYTE HandleReallocateForSipResponse(CSegment* pParam);
	void UpdateVideoTxModeForAsymmetricModes();
//	void AddSlaveParty(WORD tipPartyType, DWORD room_Id);
//	void OnSlaveToMaster(CSegment* pParam);
	void SetTelepresenseEPInfo(); //_e_m_

	BOOL GetIsNeedToChangeTipResAccordingToConfVidQuality(CConfParty* pConfParty);
	void SetupSipEntryPoint();
	void UpdateBfcpTransportTypeIfNeeded(CIpComMode* pPartyScm, CSipCaps* pRmtCaps, CSipCaps* pLocalCaps);
	void ResetRedialParameter();
	void SetupFeccMode(const char* strPartyName);
	void SetupTIPMode(PartyControlInitParameters&  partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	void ConnectSIP(PartyControlInitParameters& partyControInitParam);
	void SetupSIPConParameters(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);	
	void BuildNewSrcPartyAddress();


	CIpComMode*				m_pTargetModeMaxAllocation;//For DPA
	CSipCaps*               m_MaxLocalCaps;


};


#endif



