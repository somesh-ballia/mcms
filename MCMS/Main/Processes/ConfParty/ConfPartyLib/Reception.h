#if !defined(_Reception_H__)
#define _Reception_H__

#include  "TaskApp.h"
#include  "LobbyApi.h"
#include  "PartyApi.h"
#include  "PObject.h"
#include  "StatusesGeneral.h"
#include  "ConfParty.h"
#include  "ConfPartyOpcodes.h"
#include  "CommConf.h"

class CRsrcParams;
class CPartyApi;
class CLobby;

////////////////////////////////////////////////////////////////////////////
//                        CReception
////////////////////////////////////////////////////////////////////////////
class CReception : public CStateMachine
{
	CLASS_TYPE_1(CReception, CStateMachine)

public:
	                CReception(CTaskApp* pOwnerTask);
	               ~CReception();

	const char*     NameOf() const { return "CReception";}

	// Initializations
	virtual void    Create(DWORD monitorConfId, DWORD monitorPartyId, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx,
	                       CNetSetup* pNetSetUp, CLobby* pLobby, char* pPartyName,
	                       const char* confName, WORD identMode, WORD partyType, CConfParty* pConfParty,
	                       WORD SubCPtype, WORD isChairEnabled = 1, CConfParty* pBackUpPartyReservation = NULL, WORD BackUpScenario = 0);

	virtual void    Create(CNetSetup* pNetSetUp, COsQueue* pLobbyRcvMbx, CLobby* pLobby);
	virtual void    Create(DWORD monitorConfId, DWORD monitorPartyId, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx,
	                       CIsdnNetSetup* pNetSetUp, CLobby* pLobby, char* pPartyName,
	                       const char* confName, CConfParty* pConfParty, CCommConf* pComConf);

	virtual void    CreateSuspendConfExist(CNetSetup* pNetSetUp, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* confName);
	virtual void    CreateSuspendConfExist(CIsdnNetSetup* pNetSetUp, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* confName);
	virtual void    CreateSuspend(CNetSetup* pNetSetUp, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* confName);
	virtual void    CreateMeetingRoom(CNetSetup* pNetSetUp, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD confId, char* confName);

	void            Destroy(bool isInTransferList = true);

	// Operations
	virtual void*   GetMessageMap();
	virtual void    HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	friend bool     operator<(const CReception& first, const CReception& second);
	friend bool     operator==(const CReception& first, const CReception& second);

	// Action functions
	void            OnPartyDisconIdent(CSegment* pParam);
	void            OnPartyDisconTransfer(CSegment* pParam);
	void            OnPartyIdentIdent(CSegment* pParam);
	void            OnPartyTransferTransfer(CSegment* pParam);
	virtual void    OnConfMngrRejectPartySuspend(CSegment* pParam);
	virtual void    OnLobbyReleasePartySuspend(CSegment* pParam);


	void            OnTimerPartySuspend(CSegment* pParam);
	void            OnTimerPartyIdent(CSegment* pParam);
	void            OnTimerPartyTransfer(CSegment* pParam);

	virtual void    GetCallParams(CNetSetup** pNetSetUp);
	//virtual char*   GetCalledNumber();
	//virtual char*   GetCallingNumber();
	void            SetParty(CTaskApp* pParty)                  { m_pParty = pParty; }
	virtual DWORD   GetPartyId()                                { return ((DWORD)m_pParty); }
	CConfParty*     GetConfParty()                              { return m_pConfParty; }
	//void            SetConfParty(CConfParty* confParty);
	CNetSetup*      GetNetSetUp()                               { return m_pSuspended_NetSetUp; }
	//WORD            BackUpPartyReservation();

	void            SetConfOnAirFlag(BYTE yesNo)                { m_confOnAirFlag = yesNo; }
	BYTE            GetConfOnAirFlag()                          { return m_confOnAirFlag; }

	void            SetTargetConfName(const char* name)         { strcpy_safe(m_targetConfName, name); }
	const char*     GetTargetConfName() const                   { return m_targetConfName; }

	void            SetTargetConfType(ETargetConfType type)     { m_targetConfType = type; }
	ETargetConfType GetTargetConfType()                         { return m_targetConfType; }

	virtual void    AcceptParty(CNetSetup* pNetSetUp, CCommConf* pCommConf, CConfParty* pConfParty);

	virtual void    ReplayToNet(DWORD opcode, CIsdnNetSetup& netSetup) const;

	virtual BYTE    IsPartyDefined(DWORD confMonitorId);

	void            SetDisconnected()                           { m_isDisconnected = 1; }
	WORD            IsDisconnected() const                      { return m_isDisconnected; }

	DWORD           GetConfId()                                 { return m_confMonitorId; }
	void            SetMonitorMRId(DWORD mrID)                  { m_confMonitorId = mrID; }
	const char*     GetPartyName();

protected:
	// Operations
	virtual void    OnException();

	// Attributes
	CPartyApi*      m_pPartyApi;
	CTaskApp*       m_pParty;
	CLobby*         m_pLobby;
	CConfParty*     m_pConfParty;
	WORD            m_channel;

	DWORD           m_confMonitorId;
	WORD            m_isDisconnected;

	CNetSetup*      m_pSuspended_NetSetUp;
	CRsrcParams*    m_pSuspended_NetDesc;
	CRsrcParams*    m_pNetDesc;

	CConfParty*     m_pBackUpPartyReservation;
	WORD            m_BackUpScenario;

	WORD            m_status; // to set new field
	BYTE            m_confOnAirFlag; // YES NO
	char            m_targetConfName[H243_NAME_LEN];
	ETargetConfType m_targetConfType;


	PDECLAR_MESSAGE_MAP
};

#endif // !defined(_Reception_H__)
