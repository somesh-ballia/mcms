// CertAlarmInfo.h

#ifndef CERTALARMINFO_H_
#define CERTALARMINFO_H_

#include <vector>
#include <string>

#include "PObject.h"
#include "DataTypes.h"
#include "CertMngrDefines.h"

class CCertAlarmInfo : public CPObject
{
CLASS_TYPE_1(CCertAlarmInfo, CPObject)
public:
    CCertAlarmInfo(void);
    CCertAlarmInfo(bool actv,
                   WORD code,
                   DWORD user,
                   const std::string& desc,
                   eCertificateType type);

    virtual const char* NameOf(void) const;
    std::string ToStr(void) const;
    void Fire(void) const;

    bool             m_actv;
    WORD             m_code;
    DWORD            m_user;
    std::string      m_desc;
    eCertificateType m_type;
};

typedef std::vector<CCertAlarmInfo> CERT_ALARM_VECTOR;

////////////////////////////////////////////////////////////////////////////////
// the functor prints out Active Alarm
class CCertAlarmInfoPrinter
{
public:
    CCertAlarmInfoPrinter(std::string& out);
    ~CCertAlarmInfoPrinter(void);
    void operator()(const CCertAlarmInfo& aa);

private:
    std::string& m_out;
};

#endif  // CERTALARMINFO_H_
