//+========================================================================+
//                       McmsPCMManager.H                                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       McmsPCMManager.H                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Eitan                                                       |
//-------------------------------------------------------------------------|
// Who  | Date       | Description:                                 	   |
//-------------------------------------------------------------------------|
// Eitan| August 2009| This is the software module that manages the PCM    |
//		|			 | (Personal Conference Messages) 					   |
//+========================================================================+

#ifndef __MCMS_PCM_MNGR_H__
#define __MCMS_PCM_MNGR_H__

#include "StateMachine.h"
#include "ConfPartyDefines.h"
#include "PcmMessage.h"
#include <map>

class 	CMcmsPCMMngrInitParams;
class	CPartyApi;
class	CConfApi;
class	CAudioBridgeInterface;
class	CVideoBridgeInterface;
class   CConfAppMngrInterface;
class   CPCMHardwareInterface;
class   CConf;
class 	CRsrcParams;
class   CPcmMessage;
class   CPcmCommand;
class   CCommConf;
struct  PCM_FUNC_OBJ;
class   CPcmTermListInfoIndication;
class 	CRsrvParty;

#define   INVALID_IMAGE_ID  16

#define PCM_SETUP_TOUT ((WORD)200)
#define PCM_SETUP_TOUT_VALUE 5*SECOND

using namespace std;

typedef map<string,PCM_FUNC_OBJ> PCM_MAP;
typedef map<DWORD,CPartyApi*> CHAIRS_MAP;

class CMcmsPCMManager : public CStateMachine
{
CLASS_TYPE_1(CMcmsPCMManager,CStateMachine)

public:
	
// states
	 enum STATE{IDLE=0, MENU_OFF, ALLOCATE_PCM_RSRC, MENU_SETUP, MENU_ON, HIDING_MENU,  DEALLOCATE_PCM_RSRC };
			
	// Constructors
	 CMcmsPCMManager();
	virtual ~CMcmsPCMManager();
	
	virtual void	Create( const CMcmsPCMMngrInitParams* pMcmsPCMMngrInitParams );
	
	virtual void HandleEvent (CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	virtual const char*	NameOf () const{return "CMcmsPCMManager";}
	void InitMap();
	void DestroyMap();
	const char* StateToString(BYTE state);
	void DumpStrMap(strmap& strMap);

	void AttachEvents();
	void DetachEvents();
	virtual void HandleObserverUpdate(CSegment* pParam, WORD type);
	void OnMuteStateChangedAnycase(CSegment* pParam);
	void OnPartyAddedAnycase(CSegment* pParam);
	void OnPartyDeletedAnycase(CSegment* pParam);
	void OnPartyStateChangedAnycase(CSegment* pParam);
	void OnFeccStoppedAnycase(CSegment* pParam);
	
	void SendPcmIndication(CPcmMessage* pcmMessage);
	void SendPcmConfirm(CPcmMessage* pcmMessage);
	void SendMessageToPCM(DWORD opcode, const char* msgContent);
		
	// Action funcs (state machine)
	void OnAck(CSegment* pParam);
	void OnPCMCommand(CSegment* pParam);
	void OnChairpersonEntered(CSegment* pParam);
	void OnChairpersonEnteredIdle(CSegment* pParam);
	void OnChairpersonEnteredHidingMenu(CSegment* pParam);
	
	void OnChairpersonLeftIdle(CSegment* pParam);
	void OnChairpersonLeftMenuOff(CSegment* pParam);
	void OnChairpersonLeftMenuSetup(CSegment* pParam);
	void OnChairpersonLeftMenuOn(CSegment* pParam);
	void OnChairpersonLeftHidingMenu(CSegment* pParam);

	void OnChairpersonStartMoveFromConfAnycase(CSegment* pParam);

	void OnFeccKeyIdle(CSegment* pParam);
	void OnFeccKeyMenuOff(CSegment* pParam);
	void OnFeccKeyMenuSetup(CSegment* pParam);
	void OnFeccKeyMenuOn(CSegment* pParam);
	void OnFeccKeyHidingMenu(CSegment* pParam);

	virtual void FeccKeyRecievedFromWrongChair(DWORD partyRsrcId){PASSERT_AND_RETURN(1);}
	void OnDtmfIndIdle(CSegment* pParam);
	void OnDtmfIndMenuOff(CSegment* pParam);
	void OnDtmfIndMenuSetup(CSegment* pParam);
	void OnDtmfIndMenuOn(CSegment* pParam);
	void OnDtmfIndHidingMenu(CSegment* pParam);
	
	void OnStartClickAndViewIdle(CSegment* pParam);
	void OnStartClickAndViewMenuOff(CSegment* pParam);
	void OnStartClickAndViewMenuSetup(CSegment* pParam);
	void OnStartClickAndViewMenuOn(CSegment* pParam);
	void OnStartClickAndViewHidingMenu(CSegment* pParam);

	void OnConfInviteResult(bool res);
	
	void OnPartyConnectedToPCMEncoderMenuSetup(CSegment* pParam);
	void OnPartyConnectedToPCMEncoderAnycase(CSegment* pParam);
	
	virtual void OnEndPartyDisconnectFromPCMEncoderHidingMenu(CSegment* pParam);
	virtual void OnEndPartyDisconnectFromPCMEncoderAnycase(CSegment* pParam);
	
	void OnPartyVideoOutDisconnectedAnycase(CSegment* pParam);
	void PartyLayoutChanged(CSegment* pParam);
	void OnPcmEncoderImageSizeChanged(CSegment* pParam);
	
	virtual void Disconnect();
	virtual void OnConfDisconnectAnycase(CSegment* pParam);

	void OnPCMSetupTimeOutMenuSetup(CSegment* pParam);

	// Commands
	virtual STATUS PcmNullActionFunction(CPcmCommand* pcmCommand);
	virtual STATUS OnPopMenuStatus(CPcmCommand* pcmCommand);
	virtual STATUS OnSetAllAudioMute(CPcmCommand* pcmCommand);
	virtual STATUS OnSetFocus(CPcmCommand* pcmCommand);
	virtual STATUS OnSetCpLayout(CPcmCommand* pcmCommand);
	virtual STATUS OnSetAudioMuteIn(CPcmCommand* pcmCommand);
	virtual STATUS OnSetAudioMuteOut(CPcmCommand* pcmCommand);
	virtual STATUS OnSetVideoMuteIn(CPcmCommand* pcmCommand);
	virtual STATUS OnSetInviteTerm(CPcmCommand* pcmCommand);
	virtual STATUS OnSetDropTerm(CPcmCommand* pcmCommand);
	virtual STATUS OnStopConf(CPcmCommand* pcmCommand);
	virtual STATUS OnFeccControl(CPcmCommand* pcmCommand);
	virtual STATUS OnSetConfLayoutType(CPcmCommand* pcmCommand);
	virtual STATUS OnRecord(CPcmCommand* pcmCommand);
	virtual STATUS OnLocalAddrBook(CPcmCommand* pcmCommand);
	virtual STATUS OnSetDisplaySetting(CPcmCommand* pcmCommand){return STATUS_OK;}

	
	// Indications
	void SendDataIndications();
	void SendDirectLoginConf();
	void SendTerminalEndCall();
	void SendConfInfoIndication();
	virtual void SendMenuIndications();
	void SendImageSizeIndication(pcmImageParams imageParams);
	void SendConfLayoutIndication();
	void SendServiceSettingIndication();
	void SendMuteStatusIndications(BYTE sendIfNoChange = FALSE);
	void SendFeccEndIndication();
	void SendRecordingStateIndication(WORD recordState);
	void SendLanguageIndication();
	// Confirms
	
protected:
	DWORD OnChairpersonLeft(CSegment* pParam);
	void  OnFeccKey(CSegment* pParam);
	void  OnDtmfInd(CSegment* pParam);
	
	virtual void ConnectPartyToPCMEncoder(DWORD partyId);
	virtual void DisconnectPartyFromPCMEncoder(DWORD partyId);
	void UpdateHardwareInterfaceAndRoutingTable(DWORD newPartyRsrcId);
	
	BOOL IsPartyOutConnected(DWORD partyId);
	CTaskApp* GetChairPartyTaskApp(DWORD partyRsrcId);
	STATUS ValidateAndAddPartyToConf(CRsrvParty* pRsrvParty);
	
public:
	CPartyApi*				m_pPartyApi;
	CConfApi*				m_pConfApi;
	CAudioBridgeInterface*	m_pAudioBridgeInterface;
	CVideoBridgeInterface*	m_pVideoBridgeInterface;
	CConfAppMngrInterface*  m_pConfAppMngrInterface;
	CPCMHardwareInterface*	m_pPcmHardwareInterface;
	CConf*					m_pConf;
	CCommConf*				m_pCommConf;
	
	CTaskApp*				m_pChair;
		
protected:
	char	m_pConfName[H243_NAME_LEN];
	DWORD	m_confRsrcId;
	CRsrcParams*	m_pRsrcParams;
	DWORD	m_currentChairRsrcId;
	DWORD   m_feccTokenHolder;
	BYTE	m_terminalId; //terminal id is used to identify a specific PCM menu process
	BYTE	m_menuState; // 0 - menu off , 1 - menu on
	string  m_currentChairName;
	pcmImageParams m_imageParams;
	WORD		   m_recordingState;
	PCM_MAP  m_pcmMap;
	BYTE m_wasConnectPcmSent;
	BYTE m_needToFreeRsrcDisconnectPcm;
	BYTE MAX_NUM_OF_MENUS;		// 4 - MPM+ , 6 - MPMx
	BOOL m_bPcmFeccEnable;
	
	CPcmTermListInfoIndication* m_termListInfoIndication;
	
	PDECLAR_MESSAGE_MAP
	
};

typedef  STATUS (CMcmsPCMManager::*ACT_FUNC)(CPcmCommand*);

struct PCM_FUNC_OBJ  
{
	CPcmCommand*    PcmCommand;
	ACT_FUNC        actFunc; // action function
	
} ; 

#define ADD_PCM_ENTRY(a,b,c)\
	m_pcmMap[a]=(struct PCM_FUNC_OBJ){new b,&c};

///////////////////////////////////////////////////////////////////////////////
class CMcmsPCMManagerCOP : public CMcmsPCMManager
{
CLASS_TYPE_1(CMcmsPCMManagerCOP,CMcmsPCMManager)

public:

	// Constructors
	 CMcmsPCMManagerCOP();
	virtual ~CMcmsPCMManagerCOP();

	void	Create( const CMcmsPCMMngrInitParams* pMcmsPCMMngrInitParams );
	virtual STATUS OnPopMenuStatus(CPcmCommand* pcmCommand);
	virtual void OnConfDisconnectAnycase(CSegment* pParam);
	virtual const char*	NameOf () const{return "CMcmsPCMManagerCOP";}


protected:
	protected:
	PDECLAR_MESSAGE_MAP

};
////////////////////////////////////////////////////////////////////////////
class CMcmsPCMManagerCP : public CMcmsPCMManager
{
CLASS_TYPE_1(CMcmsPCMManagerCP,CMcmsPCMManager)

public:

// Constructors
	 CMcmsPCMManagerCP();
	virtual ~CMcmsPCMManagerCP();

	void	Create( const CMcmsPCMMngrInitParams* pMcmsPCMMngrInitParams );
	virtual void HandleObserverUpdate(CSegment* pParam, WORD type);
	//void InitMap();
	void OnChairpersonEntered(CSegment* pParam);
	void OnChairpersonEnteredIdle(CSegment* pParam);
	void OnChairpersonEnteredHidingMenu(CSegment* pParam);
	void OnChairpersonEnteredAnycase(CSegment* pParam);

	void OnChairpersonLeftIdle(CSegment* pParam);
	void OnChairpersonLeftMenuOff(CSegment* pParam);
	void OnChairpersonLeftAllocatePcm(CSegment* pParam);
	void OnChairpersonLeftMenuSetup(CSegment* pParam);
	void OnChairpersonLeftMenuOn(CSegment* pParam);
	void OnChairpersonLeftHidingMenu(CSegment* pParam);

	void OnChairpersonStartMoveFromConfAllocatePcm(CSegment* pParam);
	void OnChairpersonStartMoveFromConfMenuSetup(CSegment* pParam);
	void OnChairpersonStartMoveFromConfMenuOn(CSegment* pParam);


	void OnFeccKeyIdle(CSegment* pParam);
	void OnFeccKeyMenuOff(CSegment* pParam);
	void OnFeccKeyAllocatePcm(CSegment* pParam);
	void OnFeccKeyMenuSetup(CSegment* pParam);
	void OnFeccKeyMenuOn(CSegment* pParam);
	void OnFeccKeyHidingMenu(CSegment* pParam);

	void OnPCMSetupTimeOutAllocatePcm(CSegment* pParam);
	void OnPCMSetupTimeOutMenuSetup(CSegment* pParam);
	void OnPCMSetupTimeOutHidingMenu(CSegment* pParam);

	void SendMenuIndications();

	void AllocatePcmRsrc();
	void DeAllocatePcmRsrc();
	void OnRsrsAllocPcmRspAllocate(CSegment* pParam);
	void OnRsrsDeAllocPcmRspDeAllocate(CSegment* pParam);

	virtual void FeccKeyRecievedFromWrongChair(DWORD partyRsrcId);

	void OnEndPartyDisconnectFromPCMEncoderHidingMenu(CSegment* pParam);
	void OnEndPartyDisconnectFromPCMEncoderAnycase(CSegment* pParam);

	virtual void OnConfDisconnectAnycase(CSegment* pParam);

	virtual STATUS OnPopMenuStatus(CPcmCommand* pcmCommand);
	virtual STATUS OnSetCpLayout(CPcmCommand* pcmCommand);
	virtual STATUS OnSetConfLayoutType(CPcmCommand* pcmCommand){PASSERT(1); return STATUS_FAIL;}
	virtual STATUS OnSetDisplaySetting(CPcmCommand* pcmCommand);

protected:
	DWORD OnChairpersonLeft(CSegment* pParam);

	void ConnectPartyToPCMEncoder(DWORD partyId);
	void DisconnectPartyFromPCMEncoder(DWORD partyId);

	STATUS SendReqToResourceAllocator(CSegment *seg, OPCODE opcode);
	BYTE IsChair(DWORD partyRsrcId);
    void PrintChairsMap();
    void ClearChairsMap();

    WORD m_tmpKey;
    CHAIRS_MAP chairpersonsMap;

    //BYTE m_wasDisconnectPcmSent;

	PDECLAR_MESSAGE_MAP

};
#endif //__MCMS_PCM_MNGR_H__
