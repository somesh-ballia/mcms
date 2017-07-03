// CertAlarmInfo.cpp

#include "CertAlarmInfo.h"

#include <sstream>
#include <iomanip>

#include "ProcessBase.h"
#include "AlarmableTask.h"
#include "AlarmStrTable.h"
#include "FaultsDefines.h"
#include "CertMngrDefines.h"

CCertAlarmInfo::CCertAlarmInfo(void) :
    m_actv(true),
    m_code(-1),
    m_user(-1),
    m_type(eCertificateTypeUnknown)
{}

CCertAlarmInfo::CCertAlarmInfo(bool actv,
                               WORD code,
                               DWORD user,
                               const string& desc,
                               eCertificateType type) :
    m_actv(actv),
    m_code(code),
    m_user(user),
    m_desc(desc),
    m_type(type)
{}

// Virtual
const char* CCertAlarmInfo::NameOf(void) const
{
    return GetCompileType();
}

std::string CCertAlarmInfo::ToStr(void) const
{
    std::ostringstream buf;

    buf << (m_actv ? "+" : "-") << " "
        << CertificateTypeToStr(m_type)
        << " " << std::setw(4) << m_user
        << " " << GetAlarmName(m_code);

    if (m_actv)
        buf << ": " << m_desc;

    return buf.str();
}

void CCertAlarmInfo::Fire(void) const
{
    CProcessBase* proc = CProcessBase::GetProcess();
    CAlarmableTask* task = (CAlarmableTask*)proc->GetCurrentTask();

    if(task == NULL)
    	return;
    bool is_exist = task->IsActiveAlarmExistByErrorCodeUserId(m_code, m_user);
    if (is_exist)
        task->RemoveActiveAlarmByErrorCodeUserId(m_code, m_user);

    if (!m_actv)
        return;

    task->AddActiveAlarm(FAULT_GENERAL_SUBJECT,
                         m_code,
                         MAJOR_ERROR_LEVEL,
                         m_desc,
                         true,
                         true,
                         m_user);
}

CCertAlarmInfoPrinter::CCertAlarmInfoPrinter(std::string& out) :
    m_out(out)
{}

CCertAlarmInfoPrinter::~CCertAlarmInfoPrinter(void)
{
    if (m_out.empty())
        return;

    if (*m_out.rbegin() != '\n')
        return;

    // Removes last end-line if exist
    m_out.erase(m_out.end() - 1);
}

void CCertAlarmInfoPrinter::operator()(const CCertAlarmInfo& aa)
{
    m_out += aa.ToStr() + "\n";
}
