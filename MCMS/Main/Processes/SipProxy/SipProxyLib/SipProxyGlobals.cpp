/*
 * SipProxyGlobals.cpp
 *
 *  Created on: Mar 5, 2012
 *      Author: mvolovik
 */
#include "SipProxyGlobals.h"
#include "ProcessBase.h"
#include "TaskApi.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"

void SendReqToConfParty(OPCODE opcode, DWORD connId, DWORD partyId, CSegment *pSeg)
{
  CProcessBase* pProcess = CProcessBase::GetProcess();

  //YossiG - Klockwok NPD issue
  FPASSERT_AND_RETURN(!pProcess);

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
