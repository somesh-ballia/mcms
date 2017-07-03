#include "MplMcmsHeaderValidator.h"
#include "ObjString.h"


//----------------------------------------------------------------------------
// Validator for COMMON_HEADER_S
//----------------------------------------------------------------------------

CCommonHeaderValidator::CCommonHeaderValidator(const COMMON_HEADER_S & hdr)
        : m_Hdr(hdr)
{
}

CCommonHeaderValidator::~CCommonHeaderValidator()
{
}    
    
bool CCommonHeaderValidator::Validate(CObjString & errorString)
{
    if(NUM_OF_MAIN_ENTITIES <= m_Hdr.src_id)
	{
        errorString << "Bad main entity : " << m_Hdr.src_id;
        return false;
	}

    if(m_Hdr.next_header_type <= eHeaderNone || eHeaderUnknown <= m_Hdr.next_header_type)
    {
        errorString << "Bad next header type : " << m_Hdr.next_header_type;
        return false;
    }
    return true;
}













//----------------------------------------------------------------------------
// Validator for AUDIT_EVENT_HEADER_S
//----------------------------------------------------------------------------
CAuditHeaderValidator::CAuditHeaderValidator(const AUDIT_EVENT_HEADER_S & hdr)
        : m_Hdr(hdr)
{
}

CAuditHeaderValidator::~CAuditHeaderValidator()
{
}    
    
bool CAuditHeaderValidator::Validate(CObjString & errorString)
{
    if(NUM_OF_MAIN_ENTITIES <= m_Hdr.reportModule)
	{
        errorString << "Bad report module : " << m_Hdr.reportModule;
        return false;
	}
    if(NUM_AUDIT_EVENT_TYPES <= m_Hdr.eventType)
    {
        errorString << "Bad event type : " << m_Hdr.eventType;
        return false;
    }
    
    if(NUM_AUDIT_EVENT_STATUSES <= m_Hdr.status)
    {
        errorString << "Bad event status : " << m_Hdr.status;
        return false;
    } 
    return true;
}
