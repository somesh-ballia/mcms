/*
 * SIPSlaveParty.h
 *
 *  Created on: Feb 15, 2011
 *      Author: mvolovik
 */

#ifndef SIPSLAVEPARTY_H_
#define SIPSLAVEPARTY_H_

extern "C" void SipSlavePartyEntryPoint(void* appParam);

class CSipSlaveParty: public CSipParty
{
CLASS_TYPE_1(CSipSlaveParty, CSipParty)
	public:
	CSipSlaveParty();
	virtual ~CSipSlaveParty();
	void  Create(CSegment& appParam);

	virtual const char* NameOf() const {return "CSipSlaveParty";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	virtual void   CleanUp();
	virtual void OnConfCloseCall(CSegment * pParam);// maybe it should be part of the transaction class?
	virtual void OnPartyBadStatusConnecting(CSegment* pParam);
	void OnConfEstablishCallIdle(CSegment* pParam);
	void OnPartyChannelsConnected(CSegment* pParam);
	void UpdateDbOnChannelsConnected();
	void UpdateDbScm();
//	void SetPartyMonitorBaseParamsAndConnectChannel(DWORD channelType,DWORD rate,
//													mcTransportAddress* partyAdd,mcTransportAddress* mcuAdd,DWORD protocol,
//													DWORD pmIndex,DWORD vendorType);
	void LogicalChannelConnect(CPrtMontrBaseParams *pPrtMonitor, DWORD channelType, DWORD vendorType);
//	void LogicalChannelUpdate(DWORD channelType, DWORD vendorType);
	void OnConfPartyReceiveAudBridgeConnected();
	void OnConfPartyReceiveVidBridgeConnected();
	void OnPartyReceivedAck(DWORD status);
	void LogicalChannelDisconnect(DWORD eChannelType);
//	void TellConfOnDisconnecting(int reason,const char* alternativeAddrStr);
	void OnConfInformResourceAllocatedIdle(CSegment* pParam);
	void HandleBridgeConnectedInd(DWORD status);
	void OnConfConnectCallRmtConnected(CSegment* pParam);
	void OpenOutChannels();
	virtual void SendFastUpdateReq(ERoleLabel eRole, DWORD remoteSSRC = NON_SSRC, DWORD priorityID = INVALID, DWORD msSlavePartyIndex = 0);
	void OnConfReadyToCloseCall(CSegment* pParam);
	void OnMcuMngrPartyMonitoringReq(CSegment* pParam);
	void SendMonitoringReq();
	void OnConfBridgesUpdatedUpdateBridges();
	void OnSlavePartyRecapAck();
	void OnSlavePartyCloseChannels(CSegment* pParam);
	void OnSlavePartyOpenChannels(CSegment* pParam);
	virtual void OnAckPlayMessage(CSegment* pParam); //AT&T
	virtual void OnAckRecordPlayMessage(CSegment* pParam); //IVR for TIP
	virtual void UpdateTipParamForIvr(); //IVR for TIP

	protected:
		PDECLAR_MESSAGE_MAP;
		CSipNetSetup* m_pNetSetup; //remove it after done
		WORD m_isAudioBridgeConnected;//remove it after done
		WORD m_isVideoBridgeConnected;//remove it after done

};

#endif /* SIPSLAVEPARTY_H_ */
