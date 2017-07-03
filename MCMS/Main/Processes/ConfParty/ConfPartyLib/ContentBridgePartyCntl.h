//+========================================================================+
//                   ContentBridgePartyCntl.h                              |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       ContentBridgePartyCntl.h                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Yoella                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  June-2006  | Description                                   |
//-------------------------------------------------------------------------|

#ifndef _CONTENT_BRIDGE_PARTY_CNTL_
#define _CONTENT_BRIDGE_PARTY_CNTL_

#include "BridgePartyCntl.h"
#include "PartyApi.h"
#include "ConfPartyDefines.h"
#include "ContentBridgePartyInitParams.h"

typedef std::vector< DWORD> DWORD_VECTOR;
#define CONTENT_BRDG_PARTY_INTRA_SUPPRESS_TOUT  ((WORD)201)
class CContentBridgePartyCntl : public CBridgePartyCntl {
CLASS_TYPE_1(CContentBridgePartyCntl,CBridgePartyCntl)
public:

	// States definition
	enum STATE{ST_CONNECT = (IDLE+1), ST_CONTENT, ST_DISCONNECTING};

	virtual const char* NameOf() const { return "CContentBridgePartyCntl";}
	// Constructors
	CContentBridgePartyCntl(const void* partyId, const char* partyName = NULL, PartyRsrcID partyRsrcID = 0);
	CContentBridgePartyCntl();
	virtual ~CContentBridgePartyCntl ();
	
	virtual void	Create (const CBridgePartyInitParams* pBridgePartyInitParams);
	

	virtual void*	GetMessageMap(); 
	virtual void	HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);
		
	CContentBridgePartyCntl (const CContentBridgePartyCntl& );
	CContentBridgePartyCntl&  operator= (const CContentBridgePartyCntl&);
		
	virtual void  Connect();
	virtual void  DisConnect();
	virtual void  Destroy();
	//virtual void  Dump() const;
	
	virtual BOOL	IsDisConnecting() { return ((m_state == ST_DISCONNECTING) ? TRUE : FALSE ); }

	void ChangePresentationRate();
	void ChangePresentationSpeaker();
	void SetRate(const BYTE newPartyRate);
	void SetMyNewPointerToBridge(CBridge* pNewBridge) {m_pBridge = pNewBridge;} 
	//void UpdateMuxDesc(CMuxRsrcDesc* pMuxDesc);
	void SendFreeze(void);
	void SendRefresh(void);
	void NoRoleProvider(void);
	void ProviderIdentity(const BYTE mcuNum,const BYTE terminalNum,
		                   const BYTE label,const BYTE dataSize,const BYTE* pData);
	void FlowControlReleaseResponse(const BYTE isAck,const WORD bitRate);
	void MediaProducerStatus(const BYTE channelId,const EMediaProducerStatus status);

	BYTE GetPartyContentRate() const { return m_byCurrentContentRate; }
	BYTE GetPartyContentProtocol() const {return m_byCurrentContentProtocol; }
	BYTE GetPartyContentH264HighProfile() const {return m_byH264HighProfile; }  //HP content

    BYTE GetMcuNum() const {return m_mcuNumber;}
    BYTE GetTermNum() const {return m_terminalNumber;}
    void SetMcuNum(BYTE mcuNum);
    void SetTermNum(BYTE termNum);
    
    
    //MIH
   // BYTE IsPartyLinkToMaster () {return m_bIsLinkToMaster;}
    //BYTE CascadeLinkMode () {return m_bCascadeLinkMode;}
    void SendRateChangeDoneToMaster();
    void ForwardContentTokenMsgToMaster(CSegment* pParam);
    void OnContentBridgeForwardTokenMsgToMaster(CSegment* pParam);
    void ForwardToParty(WORD opcode,BYTE mcuNum,BYTE terminalNum);

    BYTE CheckIsPartyIntraSuppressed(BOOL insertNew);
    void UpdateIntraRequestSupressionOnSend();

    BYTE IsPartyIntraSuppressed();
    void ResetPartyIntraSuppression();

    bool IsIntraSuppressEnabled(WORD intra_suppression_type)const;
	void EnableIntraSuppress(WORD intra_suppression_type);
	void DisableIntraSuppress(WORD intra_suppression_type);

protected:
	// Utils - for internal use
	// ACTION FUNCTIONS
	// bridge message - connect party cntl
	void OnContentBridgeConnectIDLE(CSegment* pParam);
	// bridge message - disconnect party cntl
	void OnContentBridgeDisConnectCONNECT(CSegment* pParam);
	void OnContentBridgeDisConnectCONTENT(CSegment* pParam);
	void OnContentBridgeDisConnectDISCONNECTING(CSegment* pParam);
	void OnContentBridgeDisConnect(CSegment* pParam);
	// bridge message - change presentation rate
	void OnContentBridgeChangePresentationRateCONNECT(CSegment* pParam);
	void OnContentBridgeChangePresentationRateCONTENT(CSegment* pParam);
	void OnContentBridgeChangePresentationRateDISCONNECTING(CSegment* pParam);
	// bridge message - change presentation speaker
	void OnContentBridgeChangePresentationSpeakerCONNECT(CSegment* pParam);
	void OnContentBridgeChangePresentationSpeakerCONTENT(CSegment* pParam);
	void OnContentBridgeChangePresentationSpeakerDISCONNECTING(CSegment* pParam);
	// bridge message - set rate
	void OnContentBridgeSetRateCONNECT(CSegment* pParam);
	void OnContentBridgeSetRateCONTENT(CSegment* pParam);
	void OnContentBridgeSetRateDISCONNECTING(CSegment* pParam);
	
	void OnContentBridgeSendFreezeCONNECT(CSegment* pParam);
	void OnContentBridgeSendFreezeCONTENT(CSegment* pParam);
	void OnContentBridgeSendFreezeDISCONNECTING(CSegment* pParam);
	
	// bridge message - send REFRESH to party
	void OnContentBridgeSendRefreshCONNECT(CSegment* pParam);
	void OnContentBridgeSendRefreshCONTENT(CSegment* pParam);
	void OnContentBridgeSendRefreshDISCONNECTING(CSegment* pParam);
	void OnContentBridgeSendRefresh(CSegment* pParam);
	// bridge message - send NS-IND/NoRoleProvider to party
	void OnContentBridgeNoProviderCONNECT(CSegment* pParam);
	void OnContentBridgeNoProviderCONTENT(CSegment* pParam);
	void OnContentBridgeNoProviderDISCONNECTING(CSegment* pParam);
	// bridge message - send NS-IND/RoleProviderIdentity to party
	void OnContentBridgeProviderIdentityCONNECT(CSegment* pParam);
	
	void OnContentBridgeProviderIdentityCONTENT(CSegment* pParam);
	void OnContentBridgeProviderIdentity(CSegment* pParam);
	void OnContentBridgeProviderIdentityDISCONNECTING(CSegment* pParam);
	// bridge message - send MediaProducerStatus to party
	void OnContentBridgeMediaProducerStatusCONNECT(CSegment* pParam);
	void OnContentBridgeMediaProducerStatusCONTENT(CSegment* pParam);
	void OnContentBridgeMediaProducerStatusDISCONNECTING(CSegment* pParam);
	void OnContentBridgeMediaProducerStatus(CSegment* pParam);
	void OnContentBridgeFlowControlReleaseResponse(CSegment* pParam);

	
    // H239 MIH
	void OnContentBridgeSendRateChangeDoneCONNECT(CSegment* pParam);
	void OnContentBridgeSendRateChangeDoneCONTENT(CSegment* pParam);
	void OnContentBridgeSendRateChangeDoneDISCONNECTING(CSegment* pParam);
	void OnContentBridgeSendRateChangeDoneToMaster(CSegment* pParam);
	void OnContentBridgeForwardTokenMsgToMasterCONTENT(CSegment* pParam);
	void OnContentBridgeForwardTokenMsgToMasterCONNECT(CSegment* pParam);

	//   ** CONTENT_REFRESH timer message
	void OnTimerContentRefreshCONNECT(CSegment* pParam);
	void OnTimerContentRefreshCONTENT(CSegment* pParam);

protected:
	BYTE		m_byCurrentContentRate;
	BYTE		m_byCurrentContentProtocol;
	BYTE		m_mcuNumber;
	BYTE		m_terminalNumber;
	BYTE		m_byH264HighProfile; //HP content

	// that is not flag, it's a mask !!!!
	//We dont need it since NO ST_SWITCH any more....BYTE		m_byNotificationReceived; // notification about CHANGE_RATE or CHANGE_SPEAKER
	BYTE IsPartyNoiseSuppressed() {return m_IsPartyNoiseSuppressed;}
	void OnTimerPartyIntraSuppressed(CSegment* pParams);

	bool m_isIntraSupressionEnabled;
	BOOL m_IsPartyNoiseSuppressed;
	DWORD_VECTOR* m_PartyIntraRequestsTime;

	PDECLAR_MESSAGE_MAP;
};
	
#endif //_CONTENT_BRIDGE_PARTY_CNTL_

