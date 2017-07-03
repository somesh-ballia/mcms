//+========================================================================+
//                    GideonSimManager.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimManager.h                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimManager.h: interface for the CGideonSimManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_GideonSimMANAGER_H__)
#define _GideonSimMANAGER_H__


#include "ManagerTask.h"
#include "ClientSocket.h"
#include "GideonSimCardApi.h"



class CListenSocketApi;
class CGideonSimSystemCfg;
class CGideonSimCardAckStatusList;


class CGideonSimManager : public CManagerTask
{
CLASS_TYPE_1(CGideonSimManager,CManagerTask )
public:
	CGideonSimManager();
	virtual ~CGideonSimManager();

	virtual const char* NameOf() const { return "CGideonSimManager";}
	void ManagerPostInitActionsPoint();

	void SelfKill();

	TaskEntryPoint GetMonitorEntryPoint();

	void* GetMessageMap();
	
protected:
	// Action functions
	void OnListenSocketOpenConnection(CSegment* pParam);
	void OnListenSocketCloseConnection(CSegment* pParam);
	void OnTimerWait(CSegment* pParam);
	void OnEpIsdnMsgAnycase(CSegment* pParam);
	void OnEpAudioMsgAnycase(CSegment* pParam);
	void OnEpMuxMsgAnycase(CSegment* pParam);
	void OnEpScpMsgAnycase(CSegment* pParam);
	void OnMMIndicationsMsgAnycase(CSegment* pParam);
#ifndef __DISABLE_ICE__
	void OnMMIceMsgAnycase(CSegment* pParam);
#endif	//__DISABLE_ICE__
	
	void OnGuiMsgAnycase(CSegment* pParam);
	void OnPcmMsgAnycase(CSegment* pParam);
	void OnTimerPcmTransfer(CSegment* pParam);
	void OnEpGuiPcmMsgAnycase(CSegment* pParam);	
//	void OnConfPartyMsgAnycase(CSegment* pParam);

		// Logger socket messages
	void OnLoggerSocketConnectedAnycase(CSegment* pParam);
	void OnLoggerSocketFailedAnycase(CSegment* pParam);
	void OnLoggerSocketDroppedAnycase(CSegment* pParam);
	void OnLoggerSocketRcvMsgAnycase(CSegment* pParam);
		// Forward message to logger socket
	void OnForwardLoggerMsgAnycase(CSegment* pParam);

	//Terminal commands
	STATUS SetVersionBurnRate(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleBurningActions(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleSetMcuInternal(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleSendLdapUserReq(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleSimCardReset(CTerminalCommand & command, std::ostream& answer);

protected:
	// Batch scripts running transactions
	// sockets connect / disconnect
//	STATUS HandleSocketAction(CRequest *pRequest);
	STATUS HandleSocketDisconnectAction(CRequest *pRequest);
	STATUS HandleSocketConnectAction(CRequest *pRequest);
    STATUS HandleSetUnitForKeepConnectAction(CRequest *pRequest);
	STATUS HandleCardAckStatusAction(CRequest* pRequest);
	STATUS HandleIsdnTimersAction(CRequest* pRequest);
	STATUS HandleIsdnEnableDisablePortsAction(CRequest* pRequest);
	STATUS HandleInsertCardEventAction(CRequest* pRequest);
	STATUS HandleRemoveCardEventAction(CRequest* pRequest);


	// Utilities
	void ProcessGetRequest(const COsQueue& rTxSocketMbx, CSegment* pParam);
	void ProcessSetRequest(const COsQueue& rTxSocketMbx, CSegment* pParam);

protected:

//	CSimCardApi*   m_pSimCardApi;
	CSimCardApi**  m_ppSimCardApiArr;
	WORD           m_wMaxCards;

	//  configuration of system
	CGideonSimSystemCfg*  m_pSimConfig;
	//  listen socket for Cards GUI application
	CListenSocketApi*     m_pListenSocketApi;
	//  listen socket for Cards PCM application
	CListenSocketApi*     m_pListenSocketPcmApi;
	//  listen socket for Cards GUI showing PCM application
	CListenSocketApi*     m_pListenSocketEpGuiPcmApi;
	//  Card Ack statuses list
	CGideonSimCardAckStatusList*  m_pAckStatusList;
	//  Socket to logger
	CClientSocket*  m_pLoggerSocketConnection;
	
	// Last board used for ISDN dial-in party allocation
	WORD	m_boadSearchDirection;

	//a pointer to PCM share memroy
	unsigned char * m_pcmData;
	COsQueue m_EpGuiPcmTxMbx;

	PDECLAR_TERMINAL_COMMANDS
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
};

#endif // !defined(_GideonSimMANAGER_H__)

