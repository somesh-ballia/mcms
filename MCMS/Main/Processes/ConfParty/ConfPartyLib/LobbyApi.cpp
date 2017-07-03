#include "LobbyApi.h"
#include "TaskApi.h"
#include "ConfPartyOpcodes.h"


////////////////////////////////////////////////////////////////////////////
//                        CLobbyApi
////////////////////////////////////////////////////////////////////////////
void CLobbyApi::PartyDisconnect(CParty* pParty)
{
	CSegment* seg = new CSegment;

	*seg << (DWORD)pParty;

	SendMsg(seg, PARTY_DISCON);
}

//--------------------------------------------------------------------------
void CLobbyApi::PartyIdent(CParty* pParty, WORD type, WORD status) // shiraITP - 55
{
	CSegment* seg = new CSegment;

	*seg << (DWORD)pParty
	     << type
	     << status;

	SendMsg(seg, PARTY_IDENT);
}

//--------------------------------------------------------------------------
void CLobbyApi::PartyTransfer(CParty* pParty, WORD status)
{
	CSegment* seg = new CSegment;

	*seg << (DWORD)pParty
	     << status;

	SendMsg(seg, PARTY_TRANSFER);
}

//--------------------------------------------------------------------------
void CLobbyApi::RejectUnreservedParty(DWORD confId, DWORD undefId, WORD status)
{
	CSegment* seg = new CSegment;

	*seg << confId
	     << undefId
	     << status;

	SendMsg(seg, REJECT_UNRESERVED_PARTY);
}

//--------------------------------------------------------------------------
void CLobbyApi::ReleaseUnreservedParty(DWORD confId, DWORD partyId, DWORD undefId)  // shiraITP - 48
{
	CSegment* seg = new CSegment;

	*seg << confId
	     << partyId
	     << undefId;

	SendMsg(seg, RELEASE_UNRESERVED_PARTY);
}

//--------------------------------------------------------------------------
void CLobbyApi::RejectStartMeetingRoom(DWORD confId, WORD status, BOOL isAdHoc, char* adHocConfName)
{
	DWORD     undefID = 0xFFFFFFFF;
	CSegment* seg     = new CSegment;

	*seg << confId
	     << 0xFFFFFFFF
	     << status
	     << (BYTE)isAdHoc;

	if (isAdHoc)
		*seg << adHocConfName;

	SendMsg(seg, REJECT_START_MEETING_ROOM);
}

//--------------------------------------------------------------------------
void CLobbyApi::SuspendIvrParty(ETargetConfType targetConfType, DWORD dwTargetConfId, DWORD dwSourceConfId, DWORD dwPartyId, CCommRes* pAdHocRsrv)
{
	CSegment* seg = new CSegment;

	*seg << dwSourceConfId << dwPartyId << (DWORD)targetConfType;

	if (targetConfType == eMeetingRoom)
		*seg << dwTargetConfId;

	if (targetConfType == eAdHoc)
		pAdHocRsrv->Serialize(NATIVE, *seg);

	SendMsg(seg, SUSPEND_IVR_PARTY);
}

//--------------------------------------------------------------------------
void CLobbyApi::ConfOnAir(DWORD confMonitorID, char* targetConfName)
{
	CSegment* seg = new CSegment;

	*seg << confMonitorID << targetConfName;

	SendMsg(seg, CONF_ON_AIR);
}

