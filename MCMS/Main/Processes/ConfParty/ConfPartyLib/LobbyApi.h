#ifndef _LOBBYAPI
#define _LOBBYAPI

#include "TaskApi.h"
#include "ConfPartyDefines.h"
#include "CommRes.h"

// timer size;
const DWORD TRANSFER_TOUT     = 30*SECOND;
#define PARTY_TRANSFER_TOUT TRANSFER_TOUT - 3*SECOND

// to be moved to globals.h , events to lobby transfer
const WORD PARTY_DISCON       = 4001;
const WORD PARTY_IDENT        = 4002;
const WORD PARTY_TRANSFER     = 4003;
const WORD PARTY_NET_TRANSFER = 4004;

const WORD PARTYTRANSFER_TOUT = 4005;
const WORD PARTYIDENT_TOUT    = 4006;

class CRsrcTbl;
class CParty;
class CHdlcEvent;

////////////////////////////////////////////////////////////////////////////
//                        CLobbyApi
////////////////////////////////////////////////////////////////////////////
class CLobbyApi : public CTaskApi
{
	CLASS_TYPE_1(CLobbyApi, CTaskApi)

public:
	// Constructors
	CLobbyApi() { }
	CLobbyApi(const eProcessType processType, const char* taskName);
	~CLobbyApi() { }

	const char* NameOf() const { return "CLobbyApi";}

	void PartyDisconnect(CParty* pParty);
	void PartyIdent(CParty* pParty, WORD type, WORD status);
	void PartyTransfer(CParty* pParty, WORD status);
	void RejectUnreservedParty(DWORD confId, DWORD undefId, WORD status);
	void ReleaseUnreservedParty(DWORD confId, DWORD partyId, DWORD undefId);
	void RejectStartMeetingRoom(DWORD confId, WORD status, BOOL isAdHoc = FALSE, char* adHocConfName = NULL);
	void ConfOnAir(DWORD confMonitorID, char* targetConfName);
	void SuspendIvrParty(ETargetConfType targetConfType, DWORD dwTargetConfId, DWORD dwSourceConfId, DWORD dwPartyId, CCommRes* pAdHocRsrv);
};

#endif /* _LOBBYAPI */

