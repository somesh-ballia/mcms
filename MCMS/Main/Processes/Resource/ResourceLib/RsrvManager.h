#if !defined(AFX_RSRVMANAGER_H__239264C7_5764_49EE_B9D1_F693CD20F40B__INCLUDED_)
#define AFX_RSRVMANAGER_H__239264C7_5764_49EE_B9D1_F693CD20F40B__INCLUDED_

#include "ProcessBase.h"
#include "TaskApp.h"
#include "TaskApi.h"
#include "SingleToneApi.h"
#include "RequestHandler.h"
#include "CRsrcDetailGet.h"

class CCommResApi;
class CCommResRsrvDBAction;
class CComResRepeatDetails;
class CSetEndTime;

class CReservator;

extern "C" void rsrvMngrEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//                        CRsrvManager
////////////////////////////////////////////////////////////////////////////
class CRsrvManager : public CRequestHandler
{
	CLASS_TYPE_1(CRsrvManager, CRequestHandler)

public:
	                    CRsrvManager();
	virtual            ~CRsrvManager();
	void*               GetMessageMap();
	virtual const char* NameOf() const        { return "CRsrvManager";}
	void                InitTask();
	BOOL                IsSingleton() const   { return YES; }
	const char*         GetTaskName() const   { return "RsrvManager"; }

	CReservator*        GetReservator() const { return m_pReservator; }
	void                DumpToTrace(CCommResApi* pCommResApi, char* strNameOfFunction);
	STATUS              OnCreateRsrvRequest(CRequest* pRequest);
	STATUS              OnCreateRsrvRequestContinue1(CRequest* pRequest);
	void                OnStartSlaveOngRequest(CSegment* pSeg);
	void                OnCreateOrUpdateSlaveRsrvRequest(CSegment* pSeg);
	STATUS              OnCreateRepeatedRsrvRequest(CRequest* pRequest);
	void                OnCreateRsrvAdHocRequest(CSegment* pMsg);
	void                OnDeleteRsrvFromApiRequest(CSegment* pSeg);
	void                OnSyncDeleteRsrvFromApiRequest(CSegment* pSeg);
	void                OnDeleteRsrvFromMrRequest(CSegment* pSeg);
	void                OnSyncDeleteRsrvFromMrRequest(CSegment* pSeg);
	void                OnDeleteMrRequest(CSegment* pSeg);
	void                OnIPResourceRequest(CSegment* pSeg);

	STATUS              OnUpdateEndTimeFromApirequest(CRequest* pRequest);
	void                OnCreateRsrvFromMRRequest(CSegment* pMsg);

	void                OnDeleteSlaveRsrvRequest(CSegment* pSeg);
	STATUS              OnUpdateRequest(CRequest* pRequest);
	void                OnReadResDB(CSegment* pSeg);
	void                OnTimeChangedInd(CSegment* pSeg);
	void                OnDeleteRepeatedInd(CSegment* pSeg);
	void                OnWriteRepeatedInd(CSegment* pSeg);
	void                OnStartConferenceTimerInd(CSegment* pSeg);
	void                OnDeleteConferenceTimerInd(CSegment* pSeg);

	virtual void        ReceiveAdditionalParams(CSegment* pSeg);

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

private:
	void                DumpToTrace(CCommResRsrvDBAction* pCommResRsrvDBAction, char* strNameOfFunction);
	void                DumpToTrace(CComResRepeatDetails* pCommResRepeated, char* strNameOfFunction);
	void                DumpToTrace(CSetEndTime* pStructTmDrv, char* strNameOfFunction);

	STATUS              OnUpdateMRRequest(CCommResApi* pCommResApi);
	STATUS              OnUpdateResRequest(CCommResApi* pCommResApi);

	CReservator*        m_pReservator;

	DWORD               m_dwInternalConfStatus;
};

#endif // !defined(AFX_RSRVMANAGER_H__239264C7_5764_49EE_B9D1_F693CD20F40B__INCLUDED_)
