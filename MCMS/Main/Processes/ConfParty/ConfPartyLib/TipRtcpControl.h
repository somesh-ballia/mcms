/*
 * TipRtcpControl.h
 *
 *  Created on: Feb 15, 2011
 *      Author: shmuell
 */

#ifndef TIPRTCPCONTROL_H_
#define TIPRTCPCONTROL_H_

#include "StateMachine.h"
#include "TaskApp.h"
#include "TipStructs.h"
#include "HardwareInterface.h"
#include "PartyApi.h"

// Tip Rtcp states for state-machine:
const WORD sTIP_START_NEGOTIATION_SENT		 = 110;
const WORD sTIP_NEGOTIATION_RESULT_RECEIVED	 = 111;
const WORD sTIP_NEGOTIATION_COMPLETED		 = 112;
const WORD sTIP_NEGOTIATION_FAILED			 = 113;


class CTipRtcpCntl : public CStateMachine
{
	CLASS_TYPE_1(CTipRtcpCntl, CStateMachine)

public:

	CTipRtcpCntl(CTaskApp * pOwnerTask);
	virtual const char* NameOf() const { return "CTipRtcpCntl";}
	virtual ~CTipRtcpCntl();
	virtual void* GetMessageMap() {return (void*)m_msgEntries;}
	virtual void  HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	void Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, WORD mcuNum, WORD termNum, ETipAuxFPS localAuxFPS);
	DWORD SendMsgToMpl(BYTE* pStructure, int structureSize, DWORD opcode);
	void  OnPartyStartTipNegotiation(CSegment *pParam);
	void  OnNegotiationResultInd(CSegment *pParam);
	void  OnTipLastAckReceivedInd(CSegment *pParam);
	void  OnNegotiationTout(CSegment *pParam);
	void  OnPartyNegotiationHandled(CSegment *pParam);
	void  OnPartyNegotiationHandledIdle(CSegment *pParam);
	void  OnPartyNegotiationHandledNegotiationFailed(CSegment *pParam);
	void  OnNegotiationResultIndNegotiationFailed(CSegment *pParam);
	void  OnPartyCloseTipSessionIdle(CSegment *pParam);
	void  OnPartyCloseTipSessionAnycase(CSegment *pParam);
	void  OnContentMsgInd(CSegment *pParam);
	void  OnContentMsgIndAnycase(CSegment *pParam);
	void  SendStartNegotiationReq(APIU32 isPreferTip);
	void  SendEndNegotiationReq(ETipNegStatus status);
	void  SetLocalNegotiationParams();
	void  SetLocalVideoOptionsParams();
	void  SetLocalAudioOptionsParams();
	void  SetLocalVideoMuxParams();
	void  SetLocalAudioMuxParams();
	void  SetLocalSystemWideOptions();
	WORD  GetNegotiationResultNumOfStreams(APIU32 status);
	BYTE  GetNegotiationResultAudioAux();
	BYTE  IsAuxVideo5FpsNegotiated();
	BYTE  GetNegotiationResultHighProfile();
	void  SendContentMsgReq(DWORD opcode);
	BYTE  GetIsRequestSent() const {return m_bIsRequestSent;}
	void  SetIsRequestSent(BYTE bIsRequestSent);
	BYTE  GetLocalAuxFPS() const {return m_LocalAuxFPS;}
	void  SetLocalAuxFPS(ETipAuxFPS localAuxFPS);
	BYTE  IsLocalSupportVideoAuxiliary() const;
//	StartNegotiation();



protected:

	CHardwareInterface*			m_pMfaInterface;
	CPartyApi*					m_pPartyApi;
    WORD						m_mcuNum;
    WORD						m_termNum;
    BYTE						m_bWaitForWithdrawAck;
    BYTE						m_bIsRequestSent;
    ETipAuxFPS					m_LocalAuxFPS; // Auxiliary FPS supported by local.

	TipNegotiationSt*			m_pLocalNegotiationParams;
	TipNegotiationSt*			m_pNegotiationResultParams;

	BOOL						m_bLastAckReceived;
	BOOL						m_bEndSuccessNegSent;
	PDECLAR_MESSAGE_MAP

};
#endif /* TIPRTCPCONTROL_H_ */
