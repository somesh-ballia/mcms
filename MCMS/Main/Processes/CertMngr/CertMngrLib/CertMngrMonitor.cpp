// CertMngrMonitor.cpp: implementation of the CertMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "CertMngrMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "TraceStream.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "CertificateListGet.h"
#include "Request.h"
#include "CertificateGet.h"
#include "ApiStatuses.h"
#include "DummyEntry.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CCertMngrMonitor)
    ONEVENT(XML_REQUEST, IDLE, CCertMngrMonitor::HandlePostRequest)
PEND_MESSAGE_MAP(CCertMngrMonitor, CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CCertMngrMonitor)
	ON_TRANS("TRANS_CERTIFICATE_LIST", "GET",     CCertificateListGet, CCertMngrMonitor::HandleGetList)
	ON_TRANS("TRANS_CERTIFICATE",      "GET_CA",  CCertificateGet,     CCertMngrMonitor::HandleGetCTL)
	ON_TRANS("TRANS_CERTIFICATE",      "GET",     CCertificateGet,     CCertMngrMonitor::HandleGetPersonal)
	ON_TRANS("TRANS_CERTIFICATE",      "GET_CRL", CCertificateGet,     CCertMngrMonitor::HandleGetCRL)
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void CertMngrMonitorEntryPoint(void* appParam)
{  
	CCertMngrMonitor* monitorTask = new CCertMngrMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
STATUS CCertMngrMonitor::HandleListRequest(CRequest* req,
                                           eCertificateType type)
{
    PASSERTMSG_AND_RETURN_VALUE(!req, "Invalid parameter", STATUS_OK);

    // check type of received object
    CSerializeObject* obj = req->GetRequestObject();
    if (!obj)
    {
        req->SetStatus(STATUS_NOT_FOUND);
        PASSERTMSG(true, "Unable to get request object");
        return STATUS_OK;
    }

    if (!obj->IsTypeOf(CCertificateGet::GetCompileType()))
    {
        req->SetStatus(STATUS_OBJECT_NOT_RECOGNIZED);
        PASSERTSTREAM(true,
            "Invalid class type " << obj->GetRTType()
                << ", should be " << CCertificateGet::GetCompileType());
        return STATUS_OK;
    }

    CCertificateGet* cert = (CCertificateGet*)obj->Clone();

    // set appropriate list type
    cert->SetListType(type);
    bool found = cert->IsCertificateExist();
    if (!found && type == eCertificatePersonal)
    {
    	cert->SetListType(eOCS);
    	found = cert->IsCertificateExist();
    }
    if (!found)
    {  
			req->SetStatus(CertificateStatusNotExist(type));
			PASSERTSTREAM(true,
				"Unable to find certificate by issuer " << cert->GetIssuerName()
					<< ", serial " << cert->GetSerialNumber()
					<< " at " << CertificateTypeToStr(type));
			delete cert;
			return STATUS_OK;
    }

    req->SetConfirmObject(cert);

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CCertMngrMonitor::HandleGetList(CRequest* req)
{
	STATUS nRetStatus = STATUS_OK;
	
	CCertificateListGet *pListResponse = NULL;
	
	pListResponse = new CCertificateListGet();

	req->SetConfirmObject(pListResponse);
	
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CCertMngrMonitor::HandleGetCTL(CRequest* req)
{
    return HandleListRequest(req, eCertificateTrust);
}

//////////////////////////////////////////////////////////////////////
STATUS CCertMngrMonitor::HandleGetPersonal(CRequest *req)
{
    return HandleListRequest(req, eCertificatePersonal);
}

//////////////////////////////////////////////////////////////////////
STATUS CCertMngrMonitor::HandleGetCRL(CRequest *req)
{
    return HandleListRequest(req, eCertificateRevocation);
}
