#ifndef SIPTRANSINVITEMRCSLAVEWITHSDPREQ_H_
#define SIPTRANSINVITEMRCSLAVEWITHSDPREQ_H_

class CSipTransInviteMrcSlaveWithSdpReq : public CSipTransaction
{
CLASS_TYPE_1(CSipTransInviteMrcSlaveWithSdpReq, CSipTransaction)

public:
	CSipTransInviteMrcSlaveWithSdpReq(CTaskApp * pOwnerTask);
	virtual ~CSipTransInviteMrcSlaveWithSdpReq();
	virtual void* GetMessageMap() { return (void*)m_msgEntries;}
	virtual const char* NameOf() const {return "CSipTransInviteMrcSlaveWithSdpReq";}
	//state wait for conf init call
	virtual void OnPartyEstablishCallIdle(CSegment* pParam);
	virtual void OnPartyReceived200OkConnecting(CSegment* pParam);
	virtual void OnPartyReceivedReInviteWaitForReInvite(CSegment* pParam);
	void OnPartyChannelsConnectedOpenChannels(CSegment * pParam);
	virtual void OnPartyReceivedReInviteAck(CSegment* pParam);
	void HandleBridgeConnectedInd(DWORD status);
//	// open channels state
//	void OnPartyChannelsConnectedOpenChannels(CSegment * pParam);
//	// open bridge state
//	virtual void HandleBridgeConnectedInd(DWORD status);
//	// party connecting state
//	void OnPartyReceivedAckConnecting(CSegment* pParam);
//	virtual void OnConfBridgesConnectionTout(CSegment* pParam);
//
//	void OnIceInviteMakeAnsArrivedFromIceStack(CSegment * pParam);
//	void OnICECompleteTout(CSegment * pParam);
//	void OnICEGeneralToutWaitForCandidates(CSegment * pParam);
//	void IceConnectivityCheckComplete(CSegment * pParam);
//	void OnICEReceiveCloseIceInd(CSegment * pParam);
//	void OnIcePortsRetryTout(CSegment* pParam);
//	void RemoveBfcpForTIPDialInWithSDP();
//	void PartyConnectCall();
//
//	void ContinueToOpenChannels();
//
//	void OnPartySlavesRecapIsFinished(CSegment* pParam);

protected:
	PDECLAR_MESSAGE_MAP
	BYTE m_bIsToCloseVideoChannels;
	BYTE m_bIsToCloseDataChannels;

};

#endif /*SIPTRANSINVITEMRCSLAVEWITHSDPREQ_H_*/
