//+========================================================================+
//                         ConfPartyApi.cpp                                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                    All Rights Reserved.                                 |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ConfPartyApi.cpp                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:		                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//+========================================================================+

#include "ConfPartyManagerApi.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
CConfPartyManagerApi::CConfPartyManagerApi() // constructor
        :CManagerApi(eProcessConfParty)
{
}

/////////////////////////////////////////////////////////////////////////////
CConfPartyManagerApi::~CConfPartyManagerApi() // destructor
{
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
STATUS  CConfPartyManagerApi::SipProxyDBRequest()
{
  	CSegment*  seg = new CSegment;
  	CProcessBase *process = CProcessBase::GetProcess();
	const COsQueue* pCsMngrMbx = process->GetOtherProcessQueue(eProcessConfParty, eManager);

	STATUS status = pCsMngrMbx->Send(seg, SIP_PROXY_TO_CONF_PARTY_DB_REQ);
	PASSERTMSG(STATUS_OK != status, "FAILED to send message to ConfParty");

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS  CConfPartyManagerApi::GkResourceQuery(DWORD serviceId)
{
  	CSegment*  seg = new CSegment;
	seg->Put(serviceId);
  	CProcessBase *process = CProcessBase::GetProcess();
	const COsQueue* pCsMngrMbx = process->GetOtherProcessQueue(eProcessConfParty, eManager);

	STATUS status = pCsMngrMbx->Send(seg, GKMNGR_RESOURCE_INFO_REQ);
	PASSERTMSG(STATUS_OK != status, "FAILED to send GKMNGR_RESOURCE_INFO_REQ to ConfParty");

	return status;
}

STATUS  CConfPartyManagerApi::SipProxyServerTypeRequest(DWORD serviceId, DWORD serverType)
{
	CSegment *pSeg = new CSegment;

	*pSeg << serviceId;
	*pSeg << serverType;

	CProcessBase *process = CProcessBase::GetProcess();

	const COsQueue* pCsMngrMbx = process->GetOtherProcessQueue(eProcessConfParty, eManager);

	STATUS status = pCsMngrMbx->Send(pSeg, SIP_PROXY_TO_CONF_PARTY_SERVER_TYPE_REQ);

	PASSERTMSG(STATUS_OK != status, "FAILED to send SIP_PROXY_TO_CONF_PARTY_SERVER_TYPE_REQ to ConfParty");

	return status;
}
