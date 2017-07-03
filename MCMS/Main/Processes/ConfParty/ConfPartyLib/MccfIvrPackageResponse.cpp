#include "MccfIvrPackageResponse.h"

#include "MscIvr.h"
#include "MscIvrResponse.h"
#include "Event.h"

#include "DialogPrepare.h"
#include "DialogTerminate.h"

#include "MccfIvrDialogManager.h"

#include "OpcodesMcmsInternal.h"

#include "Segment.h"
#include "TaskApi.h"

#include "TraceStream.h"

#include <memory>

//////////////////////////////////////////////////////////////////////
MccfIvrErrorCodesEnum CMccfIvrPackageResponse::ResponseReportMsg(const DialogState& state, MccfIvrErrorCodesEnum status, bool isUpdateReport)
{
	FTRACEINTO << "state:" << state;

	IvrControlTypeEnum dialogType = ict_None;
	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	const char* dialogName = mscIvr.m_pResponseType->objectCodeName();

	if (strcmp(dialogName, DialogPrepare::classType()) == 0)
		dialogType = ict_Prepare;

	else if (strcmp(dialogName, DialogTerminate::classType()) == 0)
		dialogType = ict_Terminate;


	MscIvrResponse ivrResponse;
	ivrResponse.m_status = status;
	ivrResponse.m_dialogId = state.dialogID;


	MscIvr response(mscIvr);
	response.m_pResponseType = &ivrResponse;


	CTaskApi api; // api to tx
	api.CreateOnlyApi(state.clientRspMbx);


	size_t timeout = 30; // TODO: put here the actual time-out value
	if(isUpdateReport)
		timeout = 15;

	CSegment* pSeg = CreateMccfResponse(state.hMccfMsg, response, timeout,isUpdateReport,state.seqNum);


	api.SendMsg(pSeg, MCCF_IVR_PACKAGE_RESPONSE);


	// delete when there is no need to send CONTROL message later
	if ((!isUpdateReport) && (mccf_ivr_OK != status || ict_Terminate == dialogType || ict_Prepare == dialogType))
		delete state.baseObject;

	return status;
}

//////////////////////////////////////////////////////////////////////
void CMccfIvrPackageResponse::ResponseControlMsg(const DialogState& state)
{
	FTRACEINTO << "state:" << state ;

	CTaskApi api; // api to MccfTx
	api.CreateOnlyApi(state.clientRspMbx);

	CSegment* seg = CreateMccfResponse(NULL, *state.baseObject);
	api.SendMsg(seg, MCCF_IVR_PACKAGE_RESPONSE);

	delete state.baseObject;
}

//////////////////////////////////////////////////////////////////////
Event* CMccfIvrPackageResponse::BuildControlMsg(MscIvr* mscIvr, const std::string& dialogID, unsigned int status)
{
	//delete mscIvr->m_pResponseType;
	mscIvr->m_pResponseType = NULL;
	Event* event = new Event;
	mscIvr->m_pResponseType = event;

	event->m_dialogId = dialogID;
	event->m_dialogExit.m_status = status;

	return event;
}

//////////////////////////////////////////////////////////////////////
CSegment* CMccfIvrPackageResponse::CreateMccfResponse(HANDLE appServerID, const ApiBaseObject& response, size_t timeout/* = 0*/, bool isUpdateReport /*=false*/, DWORD seqNum)
{
	CSegment* seg = new CSegment;

	*seg << appServerID << response;

	if (appServerID)
		*seg << timeout << isUpdateReport << seqNum;

	return seg;
}

//////////////////////////////////////////////////////////////////////
