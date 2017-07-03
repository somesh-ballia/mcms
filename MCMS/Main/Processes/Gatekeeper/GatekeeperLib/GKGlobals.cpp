
#include "GKGlobals.h"
#include "ProcessBase.h"
#include "TaskApi.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"

void SendReqToCSMngr(CSegment *pSeg, OPCODE opcode)
{
	CProcessBase* pProcess = CProcessBase::GetProcess();
	if (!pProcess)
	{
		FPASSERT(opcode);
		return;
	}
	
	const COsQueue* pCSManager = pProcess->GetOtherProcessQueue(eProcessCSMngr, eManager);
	CTaskApi api;
	api.CreateOnlyApi(*pCSManager);
	STATUS res = api.SendMsg(pSeg,opcode);
	if (res != STATUS_OK)
		FPASSERT(opcode);
}

/////////////////////////////////////////////////////////////////////
void SendReqToConfParty(OPCODE opcode, DWORD connId, DWORD partyId, CSegment *pSeg)
{
	CProcessBase* pProcess = CProcessBase::GetProcess();
	if (pProcess == NULL)
	{
		FPASSERT(opcode);
		return;
	}
	
	const COsQueue* pConfPartyDispatcher = pProcess->GetOtherProcessQueue(eProcessConfParty, eDispatcher);
	CTaskApi api;
	api.CreateOnlyApi(*pConfPartyDispatcher);

	CSegment* pMsg = new CSegment;
	*pMsg << connId;
	*pMsg << partyId; 
	*pMsg << opcode;
	
	if (pSeg)
	{
		pMsg->Put(*pSeg);
		PDELETE(pSeg);
	}

	STATUS res = api.SendMsg(pMsg, SIGNALING_MANAGER_MSG);
	if (res != STATUS_OK)
		FPASSERT(opcode);
}

