//+========================================================================+
//                 GideonSimLogicalModule.h                                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimLogicalModule.h                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimLogicalModule.h:
//
//////////////////////////////////////////////////////////////////////

#if !defined(__GIDEONSIM__BARAK_LOGICAL_)
#define __GIDEONSIM__BARAK_LOGICAL_

#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif


class CStateMachine;
class CTaskApi;
class CMplMcmsProtocol;
class CSimMfaUnitsList;
class CIcComponent;
class CBarakIcComponent;
class CTbComponent;
class CSimSwitchUnitsList;



// include section
#include "IpCmInd.h"
#include "GideonSimParties.h"
#include "GideonSimLogicalUnit.h"
#include "GideonSimLogicalParams.h"
#include "GideonSimLogicalModule.h"


#include "IceCmReq.h"
#include "IceCmInd.h"
#ifndef __DISABLE_ICE__
#include "auto_array.h"
#endif 	//__DISABLE_ICE__
#include "TipStructs.h"

// define section
#define IVR_SIM_FOLDER_MAIN			"Cfg/IVR/"
#define IVR_SIM_FOLDER_ROLLCALL		"IVRX/RollCall/"
#define IVR_SIM_ROLLCALL_FILE		"/SimRollCall.wav"

#define MAX_IC_IVR_PLAY_MSGS		50

typedef pair<DWORD,DWORD> PartyConfKey;
typedef pair<TICKS,CSegment*> PartyConfValue;


#ifndef __DISABLE_ICE__
class IceProcessor;
#endif 	//__DISABLE_ICE__

/////////////////////////////////////////////////////////////////////////////
//
//   GideonSimBarakLogical - logical module of BARAK
//

class CGideonSimBarakLogical : public CGideonSimLogicalModule
{
CLASS_TYPE_1(CGideonSimBarakLogical,CGideonSimLogicalModule)
public:
			// Constructors
	CGideonSimBarakLogical(CTaskApp* pTask,WORD boardId,WORD subBoardId);
	virtual ~CGideonSimBarakLogical();

			// Initializations

			// Operations
	const char * NameOf() const {return "CGideonSimBarakLogical";}
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	void ProcessEndpointsSimMsg(CSegment* pMsg);
	void ProcessMMIndicationMsg(CSegment* pMsg);
    void SetUnitStatustSimBarakForKeepAliveInd(WORD UnitNum, STATUS status );
    void EstablishConnectionInd();
    void ReestablishConnectionInd();
	void SetBndLclAlignmentTimer( DWORD bndRmtLclAlignment );
			// Utilities
	BOOL IsValidParty(const CMplMcmsProtocol& rMplProt);// const;
	int IsConfPlayMessage( const CMplMcmsProtocol& rMpl );
	void ForwardMsg2MediaMngr (CMplMcmsProtocol& rMplProt ) const;

protected:
			// Indications to MCMS
	void CardManagerLoadedInd() const;
	void CmUnitLoadedInd() const;
	void CmCSIntConfigCompleteInd() const;
	void CmCSExtConfigCompleteInd() const;
	void CmDnatConfigCompleteInd() const;
	void CmConfigCompleteInd() const;
	void CmMediaIpConfigInd();
	void SendActiveSpeakerInd(const CAudioParty& party) const;
	void SendAudioSpeakerInd(const CAudioParty& party) const;
	void Ack_keep_alive(CMplMcmsProtocol& rMplProt,STATUS status);
	void Send802_1xNewConfigInd(CMplMcmsProtocol& rMplProt,STATUS status);
	void SendSlaveEncoderOpenInd( CMplMcmsProtocol& rMplProt ) const;
	void SendMasterVideoEncoderOpenInd( CMplMcmsProtocol& rMplProt ) const;
	void SendRtpVideoUpdateInd(DWORD conferenceId, DWORD partyId, DWORD connectionId, DWORD unitId) const;

	void FillRtpMonitoringStruct(TCmPartyMonitoringInd* pStruct) const;
	void FillRtpStatisticInfoStruct(TCmPartyInfoStatisticsInd* pStruct) const; //CDR_MCCF
			// Utilities
	CAudioParty* FindParty(const CMplMcmsProtocol& rMpl);
	CAudioParty* FindParty(const CAudioParty& other);
	void ForwardAudioMsg2Endpoints(const DWORD opcode, const CAudioParty* party,
								   BYTE* pData = NULL, const DWORD nDataLen = 0) const;
	void ForwardMuxBndReq2Endpoints(CMplMcmsProtocol& rMplProt) const;

			// Action functions
	void OnCmStartupIdle(CSegment* pMsg);
    void OnCmStartupConnect(CSegment* pMsg);
	void OnCmProcessMcmsReqStartup(CSegment* pMsg);
	void OnCmProcessMcmsReqConnect(CSegment* pMsg);
	void OnTimerUnitConfigTout(CSegment* pMsg);
	void OnTimerMediaConfigTout(CSegment* pMsg);

	void OnTimerCardsDelayTout(CSegment* pMsg);
	void OnTimerSpeakerChangeTout(CSegment* pMsg);
	void OnTimerIsdnCfg(CSegment* pMsg);
	void OnTimerSoftwareUpgrade(CSegment* pMsg);
	void OnTimerIpmcSoftwareUpgrade(CSegment* pMsg);
	void OnCmLoaded_test();

	void OnCmProcessEpSimMsgConnect(CSegment* pMsg);
	void OnCmProcessMMMsgConnect(CSegment* pMsg);
	void OnPcmMenuTimer(CSegment* pParam);
//vb    void SetStatusStatustSimBarakForKeepAliveInd(STATUS status );
   // STATUS HandleSetUnitForKeepConnectAction(CRequest *pRequest);

   // utilities
   int  UpdateBondingParameters(DWORD unit, DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls);
   void OnTimerPacketLossTout(CSegment* pMsg);
   void OnIceInitReq( CMplMcmsProtocol& rMplProt );
   void OnIceStatusIndSend();

#ifndef __DISABLE_ICE__
	void OnIceMakeAnswerReq( CMplMcmsProtocol& rMplProt );
	void OnIceCloseSessionReq( CMplMcmsProtocol& rMplProt );
	void OnIceMakeOfferReq( CMplMcmsProtocol& rMplProt );
	void OnIceModifyOfferReq( CMplMcmsProtocol& rMplProt );
	void OnIceModifyAnswerReq(CMplMcmsProtocol& rMplProt);
	void OnIceProcessAnswerReq( CMplMcmsProtocol& rMplProt );
	void OnTimerCheckComplete (CSegment * pParam);
	void BuildCheckCompleteInd (ICE_CHECK_COMPLETE_IND_S *CheckCompleteInd, int iceSessionIndex, int party_id);
#endif	//__DISABLE_ICE__
	void OnTipStartNegotiationReq(CMplMcmsProtocol &rMplProt);
	void OnTipEndNegotiationReq(CMplMcmsProtocol &rMplProt);
	void SetNegotiationResultParams(mcTipNegotiationResultInd &tipNegotiationResultInd);

	//BFCP
	void SendBfcpMessageInd(CMplMcmsProtocol& rMplProt);
    void OnBfcpSendHelloMsgAnycase();
    void OnBfcpCheckReceivedHelloAckTout();


    //ms - 2013
    void OnMSSendVSRReq(CMplMcmsProtocol &rMplProt);
    void SetVSRParams(TCmRtcpVsrMsg &msVsrInd);

    //WebRtc
    void OnWebRtcConnectReq(CMplMcmsProtocol &rMplProt);

protected:
			// Utilities
    void InsertParticipantForHello(DWORD partyId, DWORD confId, DWORD  connectionId, UInt32 floorId, UInt32 conferenceId,UInt16 userId,UInt32 priority);
    void ParticipantReceivedAck(DWORD partyId, DWORD confId);
    void InsertParticipantForHelloAck(DWORD partyId, DWORD confId);
    void RemoveParticipantFromMaps(DWORD partyId, DWORD confId);
    void RemoveBfcpWaiting(CMplMcmsProtocol& rMplProt);

			// Attributes
	CAudioParty			m_raAudioPartiesArr[MAX_AUDIO_PARTIES];
	int					m_nSpeakerIndex;
	CSimBarakUnitsList*	m_units;
	CBarakIcComponent*	m_IC;
	CTbComponent*		m_TB;
//vb	KEEP_ALIVE_S    	m_GideonSimBarakUnitsForKeepAliveInd;
	DWORD				m_cardType;
	BOOL				m_IsAudioControllerMaster;

///TENP
	DWORD	m_bndAddChannelCounter;
///TEMP
	bool		m_isIceInited;
	int		m_iceChanId;

	map<PartyConfKey, PartyConfValue> m_waitForHelloMap;
	map<PartyConfKey, TICKS> m_waitForHelloAckMap;

#ifndef __DISABLE_ICE__
	//ICE_SessionInfo_S		m_IcePartiesRemoteArr[MAX_AUDIO_PARTIES];
	IceProcessor	*m_iceProcessor;
#endif	//__DISABLE_ICE__

	PDECLAR_MESSAGE_MAP
};

#endif // !defined(__GIDEONSIM__BARAK_LOGICAL_)
