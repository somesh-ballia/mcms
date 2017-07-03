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

#if !defined(__GIDEONSIM_LOGICAL_MODULE_)
#define __GIDEONSIM_LOGICAL_MODULE_

#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

// include section
#include "StateMachine.h"
#include "IpCmInd.h"
#include "GideonSimParties.h"
#include "GideonSimLogicalUnit.h"
#include "GideonSimLogicalParams.h"
#ifndef __DISABLE_ICE__
#include "Mutex.hxx"
#endif	//__DISABLE_ICE__
#include "TipStructs.h"
#include "BFCPMessageNew.h"
#include "OpcodesMcmsCardMngrBFCP.h"
#include "BfcpStructs.h"
#include "BFCPH239Translator.h"

// define section
#define IVR_SIM_FOLDER_MAIN			"Cfg/IVR/"
#define IVR_SIM_FOLDER_ROLLCALL		"IVRX/RollCall/"
#define IVR_SIM_ROLLCALL_FILE		"/SimRollCall.wav"

#define MAX_IC_IVR_PLAY_MSGS		50


/////////////////////////////////////////////////////////////////////////////
//
//   GideonSimLogicalModule - base class (abstract) for all logical modules
//
class CGideonSimLogicalModule : public CStateMachine
{
CLASS_TYPE_1(CGideonSimLogicalModule,CStateMachine)
public:
			// Constructors
	CGideonSimLogicalModule(CTaskApp* pTask,WORD boardId,WORD subBoardId);
	virtual ~CGideonSimLogicalModule();
	const char * NameOf() const {return "CGideonSimLogicalModule";}
			// Initializations

			// Operations
	void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode) = 0;
	WORD  GetSubBoardId() const { return m_wSubBoardId; }
	void SetVersionBurnRate(DWORD version_burn_rate){m_version_burn_rate = version_burn_rate;}
	void SetIPMCBurnRate(DWORD ipmc_burn_rate){m_IPMC_burn_rate = ipmc_burn_rate;}
	void SetBurnAction(eBurnActionTypes burnActionType,eBurnTypes burnType);
			// API with owner class
	void Startup();
	void ProcessMcmsMsg(CSegment* pMsg);

			// Utilities
	void Ack( CMplMcmsProtocol& rMplProt,const STATUS status) const;
	void AckIp(CMplMcmsProtocol& rMplProt,const STATUS status);
	void SendToCmForMplApi(CMplMcmsProtocol& rMplProtocol) const;
	virtual void OnTimerSoftwareUpgrade(CSegment* pMsg);
	virtual void OnTimerIpmcSoftwareUpgrade(CSegment* pMsg);
	void StartVersionUpgrade();
	void SendVideoRefreshInd(CMplMcmsProtocol& rMplProt, APIU32 mediaType, APIU32 mediaDirection, APIU32 msgType, APIU32 tipPosition = eTipVideoPosLast) const;

	//BFCP
	void SendBfcpTcpTransportInd(CMplMcmsProtocol& rMplProt);

	// DTLS
	void SendDtlsEndInd(CMplMcmsProtocol& rMplProt);

	void ForwardMRMInfo2Endpoints(CMplMcmsProtocol& rMplProt, DWORD channelId) const;

	//APIU32	   m_rtpConID;
protected:
			// Action functions

protected:
			// Utilities
	void TraceMplMcms(const CMplMcmsProtocol* pMplProt) const;
	void FillMplProtocol( CMplMcmsProtocol* pMplProt,
			const DWORD opcode,const BYTE* pData,const DWORD nDataLen ) const;
	void FillMplProtocol( CMplMcmsProtocol* pMplProt,
			const DWORD opcode ) const;

			// Attributes
	CTaskApi*  m_pTaskApi;  // Owner Task Api
	WORD       m_wBoardId;
	WORD       m_wSubBoardId;
#ifndef __DISABLE_ICE__
	mutable resip::Mutex m_TaskApiMutex;//because GideonSim with AFEngine is multi-thread process, m_pTaskApi need be protected
#endif	//__DISABLE_ICE__
	int m_software_upgrade_done;
	int m_ipmc_software_upgrade_done;
	DWORD m_version_burn_rate;
	DWORD m_IPMC_burn_rate;
	bool m_stop_software_upgrade;
	bool m_stop_ipmc_upgrade;
	bool m_pause_software_upgrade;
	bool m_pause_ipmc_upgrade;

	//BFCP
	BFCPH239Translator 	*m_pTranslator;
	enTransportType		m_bfcpTransportType;

	PDECLAR_MESSAGE_MAP
};




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

typedef struct {
	int					nTimeRemain;
	CMplMcmsProtocol*	pProtocol;
} tIcIvrPlayMsg;






#endif // !defined(__GIDEONSIM_LOGICAL_MODULE_)

