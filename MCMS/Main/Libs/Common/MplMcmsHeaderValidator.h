#ifndef __MPL_MCMS_HEADER_VALIDATOR_H__
#define __MPL_MCMS_HEADER_VALIDATOR_H__


#include "PObject.h"
#include "MplMcmsStructs.h"
#include "AuditDefines.h"

class CObjString;





//----------------------------------------------------------------------------
// Validator for AUDIT_EVENT_HEADER_S
//----------------------------------------------------------------------------

class CCommonHeaderValidator : CPObject
{
CLASS_TYPE_1(CCommonHeaderValidator, CPObject)    
public:
    CCommonHeaderValidator(const COMMON_HEADER_S & hdr);
    virtual ~CCommonHeaderValidator();
    
    virtual const char*  NameOf() const{return "CCommonHeaderValidator";}
    
    bool Validate(CObjString & errorString);
    
private:
    CCommonHeaderValidator(const CCommonHeaderValidator&);
    CCommonHeaderValidator& operator=(const CCommonHeaderValidator&);

    
    const COMMON_HEADER_S & m_Hdr;
};





//----------------------------------------------------------------------------
// Validator for AUDIT_EVENT_HEADER_S
//----------------------------------------------------------------------------

class CAuditHeaderValidator : CPObject
{
CLASS_TYPE_1(CAuditHeaderValidator, CPObject)        
public:    
    CAuditHeaderValidator(const AUDIT_EVENT_HEADER_S & hdr);
    virtual ~CAuditHeaderValidator();
    
    virtual const char*  NameOf() const{return "CAuditHeaderValidator";}
    
    bool Validate(CObjString & errorString);
    
private:
    CAuditHeaderValidator(const CAuditHeaderValidator&);
    CAuditHeaderValidator& operator=(const CAuditHeaderValidator&);

    
    const AUDIT_EVENT_HEADER_S & m_Hdr;
};




#endif // __MPL_MCMS_HEADER_VALIDATOR_H__
