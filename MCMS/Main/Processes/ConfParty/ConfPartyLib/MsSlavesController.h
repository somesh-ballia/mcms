#ifndef MSSLAVESCONTROLLER_H_
#define MSSLAVESCONTROLLER_H_

#include "StateMachine.h"
#include "ConfParty.h"
#include "ConfApi.h"
#include "Conf.h"
#include "MsVsrMsg.h"
#include "EventPackage.h"
#include "ConfPartySharedDefines.h"


#define MAX_MS_OUT_SLAVES	3	// including main party
#define	MAX_MS_IN_SLAVES	5	// including main party
#define	FULL_PACSI_INFO_TOUT 	1*SECOND
#define CONNECT_MS_SLAVES_TOUT	20*SECOND


typedef enum
{
	eMsSlaveIdle,
	eMsSlaveConnecting,
	eMsSlaveConnected,
	eMsSlaveDisconnecting,
	eMsSlaveInactive,
} EMsSlaveConnect;

typedef enum
{
	eFreeMsSlave,
	eInactiveMsSlave
}EFreeOrInactiveMsSlave;


////////////////////////////////////////////////////////////////////////////
//                        CMsSlaveInfo
////////////////////////////////////////////////////////////////////////////
class CMsPartyCntlInfo : public CPObject
{
	CLASS_TYPE_1(CMsPartyCntlInfo, CPObject)
public:

							CMsPartyCntlInfo();
							CMsPartyCntlInfo(const CMsPartyCntlInfo& other);
							~CMsPartyCntlInfo();

	CMsPartyCntlInfo& 		operator=(const CMsPartyCntlInfo& other);

	const char*   			NameOf() const { return "CMsPartyCntlInfo"; }

	void                	SetName(const char* name) { strcpy_safe(m_name, name); }
	const char*         	GetName() const { return m_name; }
	void 					SetPartyRsrcId(PartyRsrcID partyRsrcId) { m_partyRsrcId = partyRsrcId; }
	PartyRsrcID				GetPartyRsrcId() { return m_partyRsrcId; }
	void 					SetMaxResolution(EVideoResolutionType maxResolution)  { m_maxResolution = maxResolution; }
	EVideoResolutionType	GetMaxResolution()  { return m_maxResolution; }
	void					SetSsrcRangeStart(DWORD ssrcRangeStart) { m_ssrcRangeStart = ssrcRangeStart; }
	DWORD					GetSsrcRangeStart() { return m_ssrcRangeStart; }
	void 					SetConnectState(EMsSlaveConnect connectState) { m_connectState = connectState; }
	EMsSlaveConnect			GetConnectState() { return m_connectState; }

///	void					SetConnectStateAccordingToAck(/*OPCODE opcode, */STATUS status);

	DWORD					GetEncoderMaxVsrResolution()const;

	void 					SetWaitForPacsi(bool waitForPacsi) { m_waitForPacsi = waitForPacsi; }
	DWORD					GetWaitForPacsi()const { return m_waitForPacsi; }

private:

	char 					m_name[H243_NAME_LEN];
	PartyRsrcID				m_partyRsrcId;
	EVideoResolutionType	m_maxResolution;
	DWORD					m_ssrcRangeStart;
	EMsSlaveConnect			m_connectState;
	bool					m_waitForPacsi;

};



////////////////////////////////////////////////////////////////////////////
//                        CMsSlavesController
////////////////////////////////////////////////////////////////////////////
class CMsSlavesController : public CStateMachine
{
	CLASS_TYPE_1(CMsSlavesController, CStateMachine)


public:
									CMsSlavesController();
	CMsSlavesController(const CMsSlavesController& other);
	CMsSlavesController&           	operator=(const CMsSlavesController& other);
	virtual                        	~CMsSlavesController();



	virtual const char*            	NameOf() const 	{ return "CMsSlavesController"; }
	virtual void*                  	GetMessageMap()	{ return (void*)m_msgEntries; }

	void 						   	Create(PartyRsrcID mainPartyRsrcId, PartyMonitorID mainMonitorPartyId, CConf* pConf,CSipCaps* remoteCaps,BOOL isEncrypt);
	void 						   	ConnectOutSlaves(EVideoResolutionType mainVideoResolutionType,EVideoResolutionType maxConfResolutionType, DWORD msSsrcRangeStart, DWORD totalBandwidth, CVidModeH323 *pLocalSdesCap = NULL);
	void 						   	AddMsSlaveParty(eAvMcuLinkType avMcuLinkType, DWORD slaveIndex, EVideoResolutionType slaveMaxResolution, DWORD msSsrcRangeStart = (DWORD)(-1), CVidModeH323 *pLocalSdesCap = NULL);	//from CSipPartyCntl::OnAddSlaveParty
	void 							SetSlavePartyName(CRsrvParty* pSlaveParty, eAvMcuLinkType avMcuLinkType, DWORD slaveIndex);

	void 							ConnectInSlaves(EVideoResolutionType mainVideoResolutionType);
	int 							CalculateNumOfInSlavesRequired(PartyRsrcID partyRsrcId);
	int 							GetNumOfVideoPartiesInAvMcuConversation(PartyRsrcID partyRsrcId);

	void 						   	HandleEvent(CSegment* pMsg, OPCODE opCode);
	void 						   	OnMsSlaveToMainAckMessage(CSegment* pMsg);
	void 							HandleDeleteMsSlavePartyAck(eAvMcuLinkType avMcuLinkType, DWORD msSlaveIndex);
	void 							HandleAddMsSlavePartyAck(eAvMcuLinkType avMcuLinkType, DWORD msSlaveIndex, PartyRsrcID msSlavePartyRsrcId, STATUS status);

	BOOL 							AreAllOutSlavesConnected();
	void							SendAllOutSlavesConnectedToMain();
	BOOL 							AreAllInSlavesConnected();
	void							SendAllInSlavesConnectedToMain();
	BOOL 							AreAllSlavesDisconnected();
	void 							SendAllSlavesDisconnectedToMain();

	void							OnAvMcuVsrMsgInd(CSegment* pMsg);
	void							OnAvMcuRmtH230(CSegment* pMsg);
	// pacsi
	void							OnMsSlaveSinglePacsiInfoInd(CSegment* pMsg);
	void							OnTimerFullPacsiInfo(CSegment* pMsg);
	void 							SetLocalAudioMsi(LyncMsi localAudioMsi) { m_localAudioMsi = localAudioMsi; }

	void 							DeleteAllSlaves();

	void 							OnAvMcuStreamsIntraReq(CSegment* pMsg);
	void 							OnAvMcuSendHandleSingleFecMsgToSlave(CSegment* pMsg);   //LYNC2013_FEC_RED

	void 							OnConnectMsOutSlavesTout(CSegment* pParam);
	void 							OnConnectMsInSlavesTout(CSegment* pParam);
	void                            UpdateBlockAudioState(EventPackage::MediaStatusType eMediaTypeStatus);

protected:

	// VSR
	void							BuildAndSendSingleVsrMsg(CMsVsrMsg& vsrMsg, WORD slaveIndex, ResolutionsEntriesMap& selectedEntriesPerEncoders);
	void 							OnAvMcuVsrMsg(CMsVsrMsg& vsrMsg);
	void 							EndVsr();
	// pacsi
	void 							StartPacsiInfoSync();
	void 							EndPacsiInfoSync();
	void							ResetWailForPacsi();
	bool							IsFullPacsiInfoSychCompleted()const;
	void 							UpdateFullPacsiInfo(DWORD rsrcPartyId,MsSvcParamsStruct& singlePacsiInfo);
	void  							SendFullPacsiInfoToOutSlaves(BYTE isReasonFecOrRed=FALSE);
	void   							SendFullPacsiInfoToOutSlavesExceptRsrcPartyId(DWORD rsrcPartyId);

	void 							HandleEventPackageEvent(CSegment* pMsg);
	LyncMsi 						GetSpotLightSpeakerMsi();
	BOOL 							IsValidMSI(LyncMsi msi);
	BOOL 							IsSpotLightSpeakerMediaAdded(LyncMsi videoMsi, LyncMsi spotLightSpeakerVideoMsi);
	int 							FindFirstFreeIndexInSlavesArray(EFreeOrInactiveMsSlave& isFreeOrInactiveSlave);
	void 							OnDelMsSlaveParty(CSegment* pMsg);
	void 							GetSlaveByRsrcId(PartyRsrcID partyRsrcId, eAvMcuLinkType& avMcuLinkType, int& slaveIndex);
	void 							DeleteInSlave(int slaveIndex);
	void 							DeleteOutSlave(int slaveIndex);
	void 							InactivateInSlave(int slaveIndex);
//	void 							InactivateOutSlave(int slaveIndex);




private:

	// all slaves lists and counters INCLUDE the main party control
	CMsPartyCntlInfo* 	m_msOutSlaves[MAX_MS_OUT_SLAVES];	// 1st "slave" (index 0) will always be the main party
	CMsPartyCntlInfo* 	m_msInSlaves[MAX_MS_IN_SLAVES];

	WORD 				m_numOfActiveOutSlaves;		// including the main
	WORD 				m_numOfActiveInSlaves;		// including the main

	const CConfParty* 	m_pMainConfParty;
	BYTE                m_maxConfBitRate;
	CConfApi*           m_pTaskApi;
	COsQueue*           m_pSubscribedQueue;		// the original pointer to conf queue, the one with whom we subsribed to the event package, for del subscribe.
												// (it is needed because when changing the partyControl from add to change mode and delete, new pointers are created for this queue)
	DWORD				m_totalBandwidth;
	CSipCaps*	        m_pSipRemoteCaps;

	LyncMsi 			m_localAudioMsi;

	BOOL                m_isEncrypt;

	BOOL 				m_disableInSlavesDynamicRemoval;

	// pacsi
	MsFullPacsiInfoStruct m_fullPacsiInfo;
	MsFullPacsiInfoStruct m_lastFullPacsiInfo;

	// waiting VSR
	bool m_isVsrInProgress;
	bool m_isWaitingVsr;
	CMsVsrMsg* m_pWaitingVsrMsg;
	eMsSvcVideoMode m_AvMcuCascadeMode;

	PDECLAR_MESSAGE_MAP
};





#endif /* MSSLAVESCONTROLLER_H_ */
