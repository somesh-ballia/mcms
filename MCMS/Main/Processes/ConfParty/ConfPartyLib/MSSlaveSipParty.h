/*
 * MSSlaveSipParty.h
 *
 *  Created on: Feb 15, 2011
 *      Author: mvolovik
 */

#ifndef MSSLAVESIPPARTY_H_
#define MSSLAVESIPPARTY_H_

extern "C" void MSSlaveSipPartyEntryPoint(void* appParam);

class CMSSlaveSipParty: public CSipParty
{
CLASS_TYPE_1(CMSSlaveSipParty, CSipParty)
public:
	CMSSlaveSipParty();
	virtual ~CMSSlaveSipParty();
	void  Create(CSegment& appParam);
	virtual const char* NameOf() const {return "CMSSlaveSipParty";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	virtual void   CleanUp();
	void OnConfInformResourceAllocatedIdle(CSegment* pParam);
	void OnConfEstablishCallIdle(CSegment* pParam);
	void OnPartyInChannelsConnected();
	void OnPartyChannelsConnected(CSegment* pParam);
	void OnConfPartyReceiveAudBridgeConnected();
	void OnConfPartyReceiveVidBridgeConnected();
	void OnPartyReceivedAck(DWORD status);
	void OnConfConnectCallRmtConnected(CSegment* pParam);
	void OpenOutChannels();
	void OnConfCloseCall(CSegment* pParam);
	void OnConfReadyToCloseCall(CSegment* pParam);
	eAvMcuLinkType GetAvMcuLinkType() const { return m_AvMcuLinkType; }
	DWORD GetMSSlavePartyIndex() const { return m_MSSlaveIndex; }
	void  HandleMsftVsrOnEndTransaction(bool isNeedToSendReinvite,ESipTransactionType endedTransType);
	
	virtual void OnPartyBadStatusConnecting(CSegment * pParam);
	virtual void ForwardRemoteH230ToMsSlavesControllerIfNeeded(CSegment* pParam);
	void OnVidBrdgVideoInSyncAnycase(CSegment* pParam);
	void OnVidBrdgRefreshAnycase(CSegment* pParam);
	
#if 0	
	virtual void OnConfCloseCall(CSegment * pParam);// maybe it should be part of the transaction class?
	virtual void OnPartyBadStatusConnecting(CSegment* pParam);
	void UpdateDbOnChannelsConnected();
	void UpdateDbScm();
//	void SetPartyMonitorBaseParamsAndConnectChannel(DWORD channelType,DWORD rate,
//													mcTransportAddress* partyAdd,mcTransportAddress* mcuAdd,DWORD protocol,
//													DWORD pmIndex,DWORD vendorType);
	void LogicalChannelConnect(CPrtMontrBaseParams *pPrtMonitor, DWORD channelType, DWORD vendorType);
//	void LogicalChannelUpdate(DWORD channelType, DWORD vendorType);
	void LogicalChannelDisconnect(DWORD eChannelType);
//	void TellConfOnDisconnecting(int reason,const char* alternativeAddrStr);
	void HandleBridgeConnectedInd(DWORD status);
	virtual void SendFastUpdateReq(ERoleLabel eRole);
	void OnMcuMngrPartyMonitoringReq(CSegment* pParam);
	void SendMonitoringReq();
	void OnConfBridgesUpdatedUpdateBridges();
	void OnSlavePartyRecapAck();
	void OnSlavePartyCloseChannels(CSegment* pParam);
	void OnSlavePartyOpenChannels(CSegment* pParam);
	virtual void OnAckPlayMessage(CSegment* pParam); //AT&T
	virtual void OnAckRecordPlayMessage(CSegment* pParam); //IVR for TIP
	virtual void UpdateTipParamForIvr(); //IVR for TIP

#endif
protected:
	PDECLAR_MESSAGE_MAP;
	CSipNetSetup* 		m_pNetSetup; //remove it after done
	WORD 				m_isVideoBridgeConnected;//remove it after done
	eAvMcuLinkType      m_AvMcuLinkType;
	DWORD             	m_MSSlaveIndex;

};

#endif /* MSSLAVESIPPARTY_H_ */

