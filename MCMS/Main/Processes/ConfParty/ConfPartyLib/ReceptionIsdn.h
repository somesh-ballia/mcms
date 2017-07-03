#ifndef _RECEPTION_ISDN_H_
#define _RECEPTION_ISDN_H_

#include "Reception.h"

class CIsdnNetSetup;

////////////////////////////////////////////////////////////////////////////
//                        CReceptionIsdn
////////////////////////////////////////////////////////////////////////////
class CReceptionIsdn : public CReception
{
	CLASS_TYPE_1(CReceptionIsdn, CReception)

public:
	explicit            CReceptionIsdn(CTaskApp* pOwnerTask);
	virtual            ~CReceptionIsdn();

	// Initializations
	void                Create(DWORD monitorConfId, DWORD monitorPartyId, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx,
	                           CIsdnNetSetup* pNetSetUp, CLobby* pLobby, char* pPartyName,
	                           const char* confName, CConfParty* pConfParty, CCommConf* pComConf);

	void                CreateSuspend(CIsdnNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* confName);
	void                CreateSuspendConfExist(CIsdnNetSetup* pNetSetUp, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD ConfId, char* confName);
	void                CreateMeetingRoom(CNetSetup* pNetSetUp, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD confId, char* mrName);
	void                CreateGateWayConf(CNetSetup* pNetSetup, COsQueue* pLobbyRcvMbx, CLobby* pLobby, CTaskApp* ListId, DWORD gwId, char* gwName, char* targetNumber);

	void                GetCallParams(CIsdnNetSetup** netSetUp);
	WORD                GetVoiceType(CConfParty* pConfParty);

	// Operations
	virtual void*       GetMessageMap();
	virtual const char* NameOf() const;
	void                OnLobbyReleasePartySuspend(CSegment* pParam);
	void                OnConfMngrRejectPartySuspend(CSegment* pParam);
	void                ReplayToNet(DWORD opcode, CIsdnNetSetup& netSetup) const;

protected:
	virtual void        OnException();

	PDECLAR_MESSAGE_MAP
};

#endif /* _RECEPTION_ISDN_H_ */
