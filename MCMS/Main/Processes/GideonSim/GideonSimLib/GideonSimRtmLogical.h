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

#if !defined(__GIDEONSIM_RTM_LOGICAL_)
#define __GIDEONSIM_RTM_LOGICAL_


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
#include "GideonSimLogicalModule.h"

// define section
#define IVR_SIM_FOLDER_MAIN			"Cfg/IVR/"
#define IVR_SIM_FOLDER_ROLLCALL		"IVRX/RollCall/"
#define IVR_SIM_ROLLCALL_FILE		"/SimRollCall.wav"

#define MAX_IC_IVR_PLAY_MSGS		50



/////////////////////////////////////////////////////////////////////////////
//
//   GideonSimRtmLogical - logical module of RTM
//
const BYTE SIM_ISDN_NUM_SPANS_IN_BOARD = 12;
//const BYTE MAX_PSTN_PARTIES = 30;

class CGideonSimRtmLogical : public CGideonSimLogicalModule
{
CLASS_TYPE_1(CGideonSimRtmLogical,CGideonSimLogicalModule)
public:
			// Constructors
	CGideonSimRtmLogical(CTaskApp* pTask,WORD boardId,WORD subBoardId,eCardType RtmCardType = eRtmIsdn);
	virtual ~CGideonSimRtmLogical();
	const char * NameOf() const {return "CGideonSimRtmLogical";}
			// Initializations

			// Operations
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	void ProcessEndpointsSimMsg(CSegment* pMsg);

	void SetRtmEnableDisablePorts( DWORD span, DWORD firstPort, DWORD numPorts, DWORD action );

protected:
			// Indications to MCMS
//	void ProgressInd2Conf(const CPstnConnectionParty& party) const;
//	void DisconnectInd2Conf(const CPstnConnectionParty& party) const;
//	void ClearInd2Conf(const CPstnConnectionParty& party) const;

	void SendCardManagerLoadedInd() const;
	void SendCmMediaIpConfigInd() const;
	void SendRtmIsdnSpanStatusInd(const DWORD spanId) const;
	void SendRtmIsdnSpanStatusInd_ClockSourceAlert(const bool isSingle) const;

//	void ForwardNetReq2Endpoints(const DWORD opcode,
//		const CPstnConnectionParty& party, char* pData, DWORD nDataLen) const;
	void ForwardNetReq2Endpoints(const DWORD spanId, const DWORD portId, CMplMcmsProtocol& rMplProt) const;
			// Utilities
//	CPstnConnectionParty* FindParty(const CMplMcmsProtocol& rMpl);
//	CPstnConnectionParty* FindParty(const CPstnConnectionParty& other);
	void SendNetSetupReqFailure( const DWORD confId, const DWORD partyId, const DWORD connectionId, const DWORD virtPortId );

			// Action functions
	void OnCmStartupIdle(CSegment* pMsg);
	void OnCmStartupConnect(CSegment* pMsg);
	void OnCmProcessMcmsReqStartup(CSegment* pMsg);
	void OnCmProcessMcmsReqConnect(CSegment* pMsg);
	void OnCmProcessEpSimMsgConnect(CSegment* pMsg);
	void OnTimerCardsDelayTout(CSegment* pMsg);
	void Ack_keep_alive(CMplMcmsProtocol& rMplProt,STATUS status);
//	void OnTimerUnitConfigTout(CSegment* pMsg);
//	void OnTimerMediaConfigTout(CSegment* pMsg);
	void OnTimerSpanStatus(CSegment* pMsg);

protected:
			// Utilities

			// Attributes
//	CPstnConnectionParty  m_raPstnPartiesArr[MAX_PSTN_PARTIES];
	CSimNetSpan				m_arSpans[SIM_ISDN_NUM_SPANS_IN_BOARD+1];	// 1<->MAX, 0 is not valid
	RTM_ISDN_PARAMETERS_S	m_rIsdnParams;
	DWORD					m_disableEnablePortsSet; // debug

	eCardType				    m_cardType;

	PDECLAR_MESSAGE_MAP
};




#endif // !defined(__GIDEONSIM_RTM_LOGICAL_)
