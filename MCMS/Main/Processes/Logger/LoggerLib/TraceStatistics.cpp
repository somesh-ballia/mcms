#include <iomanip>
#include <iostream>
using namespace std;


#include "TraceStatistics.h"
#include "ProcessBase.h"
#include "LoggerProcess.h"
#include "SystemFunctions.h"






const DWORD ProcessNameWidth = 25;
const DWORD EntityNameWidth = 15;
const DWORD NumberWidth = 25;

extern const char* MainEntityToString(APIU32 entityType);


static double ComputePercent(ULONGLONG arg, ULONGLONG percent100)
{
    double percents = ((double)(arg * 100)/ (double)percent100);
    return percents;
}








/*----------------------------------------------------------------
 class CKBSize
----------------------------------------------------------------*/

CKBSize::CKBSize()
{
    m_Size = 0;
}

void CKBSize::AddSize(ULONGLONG size)
{
    m_Size += size;
}

void CKBSize::AddSize(const CKBSize & other)
{
    m_Size += other.m_Size;
}
void CKBSize::ToString(CObjString & str)const
{
    static const ULONGLONG KiloByte = 1024;
    static const ULONGLONG MegaByte = 1048576;
    static const ULONGLONG GigaByte = 1048576 * 1024;

    // *= 2^3
    const ULONGLONG LimitKilo = KiloByte << 3;
    const ULONGLONG LimitMega = MegaByte << 3;
    const ULONGLONG LimitGiga = GigaByte << 3;

    //[0, 2^3 * Kilo]
    if(m_Size < LimitKilo)
    {
        str << m_Size << " B";
    }
    // [2^3 * Kilo, 2^3 * Mega]
    else if(m_Size < LimitMega)
    {
        ULONGLONG kb = m_Size / KiloByte;
        ULONGLONG re = m_Size % KiloByte;
        str << kb << "." << re << " KB";
    }
    // [2^3 * Mega, 2^3 * Giga]
    else if(m_Size < LimitGiga)
    {
        ULONGLONG kb = m_Size / MegaByte;
        ULONGLONG re = m_Size % MegaByte;
        str << kb << "." << re << " MB";
    }
    // [2^3 * Giga, *]
    else
    {
        ULONGLONG kb = m_Size / GigaByte;
        ULONGLONG re = m_Size % GigaByte;
        str << kb << "." << re << " GB";
    }
}






















/*----------------------------------------------------------------
 class CMapProcessCnt
----------------------------------------------------------------*/


////////////////////////////////////////////////////////////////////////////
CMapProcessCnt::CMapProcessCnt()
{
    m_CntAllTraces = 0;
}

////////////////////////////////////////////////////////////////////////////
CMapProcessCnt::~CMapProcessCnt()
{
    
}

// ////////////////////////////////////////////////////////////////////////////
// void CMapProcessCnt::IncrementCnt()
// {
//     m_CntAllTraces++;
// }

////////////////////////////////////////////////////////////////////////////
CKBSize& CMapProcessCnt::GetTraceSize()
{
    return m_Size;
}

////////////////////////////////////////////////////////////////////////////
const CKBSize& CMapProcessCnt::GetTraceSizeConst()const
{
    return m_Size;
}

////////////////////////////////////////////////////////////////////////////
ULONGLONG CMapProcessCnt::GetNumOfAllEntityTraces()const
{
    return m_CntAllTraces;
}

////////////////////////////////////////////////////////////////////////////
void CMapProcessCnt::AddTrace(DWORD process, DWORD size)
{
    m_CntAllTraces++;
    m_Size.AddSize(size);
    ULONGLONG &refCnt = this->operator[](process);
    refCnt++;
}










/*----------------------------------------------------------------
 class CMapEntityProcess
----------------------------------------------------------------*/


////////////////////////////////////////////////////////////////////////////
CMapEntityProcess::CMapEntityProcess()
{
    Reset();
}

////////////////////////////////////////////////////////////////////////////
void CMapEntityProcess::AddTrace(DWORD mainEntity, DWORD process, DWORD size)
{
    m_NumAllTraces++;
    CMapProcessCnt & refMapProcesses = this->operator[](mainEntity);
    refMapProcesses.AddTrace(process, size);
}

////////////////////////////////////////////////////////////////////////////
void CMapEntityProcess::Reset()
{
    m_NumAllTraces = 0;
    clear();
}

////////////////////////////////////////////////////////////////////////////
ULONGLONG CMapEntityProcess::GetNumOfAllTraces()const
{
    return m_NumAllTraces;
}
































/*----------------------------------------------------------------
 class CTraceStatistics
----------------------------------------------------------------*/


////////////////////////////////////////////////////////////////////////////
CTraceStatistics::CTraceStatistics()
{
    Reset();
}

////////////////////////////////////////////////////////////////////////////
CTraceStatistics::~CTraceStatistics()
{
}

////////////////////////////////////////////////////////////////////////////
void CTraceStatistics::Reset()
{
    SystemGetTime(m_StartTime);
    m_MapEntityProcess.Reset();
}

////////////////////////////////////////////////////////////////////////////
void CTraceStatistics::AddTrace(DWORD mainEntity, DWORD process, DWORD size)
{
    m_MapEntityProcess.AddTrace(mainEntity, process, size);
}

ULONGLONG CTraceStatistics::GetNumOfAllTraces() const
{
	return m_MapEntityProcess.GetNumOfAllTraces();
}
////////////////////////////////////////////////////////////////////////////
void CTraceStatistics::Dump(std::ostream &ostr) const
{
    CStructTm now;
    SystemGetTime(now);
    
    ostr << "Statistics of traces"
         << endl;
    ostr << "Times : ";
    m_StartTime.Dump(ostr);
    ostr << "-- ";
    now.Dump(ostr);
    ostr << endl
         << "--------------------------------------------" << endl;
    
    DumpInsideEntities(ostr);
    ostr << endl;
    DumpPerEntity(ostr);
    ostr << endl;
}

////////////////////////////////////////////////////////////////////////////
void CTraceStatistics::DumpInsideEntities(std::ostream &ostr)const
{
    ostr.setf(std::ios::left,std::ios::adjustfield);

    ostr << "1) Inside entities:" << endl
         << "--------------------------------------------" << endl;

    CLargeString strTraceSize;
    
    CMapEntityProcess::const_iterator iEntityCurrent = m_MapEntityProcess.begin();
    CMapEntityProcess::const_iterator iEntityEnd = m_MapEntityProcess.end();
    for(; iEntityCurrent != iEntityEnd ; iEntityCurrent++)
    {
        DWORD mainEntityType = iEntityCurrent->first;
        const char *strMainEntity = ::MainEntityToString(mainEntityType);

        const CMapProcessCnt & processesMap = iEntityCurrent->second;
        const ULONGLONG llAllEntityTraces = processesMap.GetNumOfAllEntityTraces();
        
        strTraceSize.Clear();
        const CKBSize & traceSize = processesMap.GetTraceSizeConst();
        traceSize.ToString(strTraceSize);
        
        ostr << strMainEntity << " [All Data Size : "
             << strTraceSize.GetString()
             << "]"
             << endl;

        double dPersents100 = 0;
        
        CMapProcessCnt::const_iterator iProcCurrent = processesMap.begin();
        CMapProcessCnt::const_iterator iProcEnd = processesMap.end();
        for(; iProcCurrent != iProcEnd ; iProcCurrent++)
        {
            DWORD processType = iProcCurrent->first;
            ULONGLONG llCnt = iProcCurrent->second;

            const char *strProcess = CLoggerProcess::GetProcessNameByMainEntity((eMainEntities)mainEntityType,
                                                                                (eProcessType)processType
                                                                                );
            double dPercents = ComputePercent(llCnt, llAllEntityTraces);
            dPersents100 += dPercents;
            
            ostr << "\t"
                 << std::setw(ProcessNameWidth) << strProcess
                 << std::setw(NumberWidth) << llCnt
                 << dPercents << " %"
                 << endl;
        }
        
        ostr << "\t"
             << "--------------------------------------------" << endl
             << "\t"
             << std::setw(ProcessNameWidth) << "All"
             << std::setw(NumberWidth) << llAllEntityTraces
             << dPersents100 << " %"
             << endl
             << endl;
    }
}


////////////////////////////////////////////////////////////////////////////
void CTraceStatistics::DumpPerEntity(std::ostream &ostr)const
{
    ostr.setf(std::ios::left,std::ios::adjustfield);

    ostr << "2) Per entity:" << endl
         << "--------------------------------------------" << endl;

    double persents100 = 0;
    CKBSize allTraceSize;
    CLargeString strTraceSize;
    const ULONGLONG numAllTraces = m_MapEntityProcess.GetNumOfAllTraces();
    
    CMapEntityProcess::const_iterator iEntityCurrent = m_MapEntityProcess.begin();
    CMapEntityProcess::const_iterator iEntityEnd = m_MapEntityProcess.end();
    for(; iEntityCurrent != iEntityEnd ; iEntityCurrent++)
    {
        DWORD mainEntityType = iEntityCurrent->first;
        const char *strMainEntity = ::MainEntityToString(mainEntityType);
        
        const CMapProcessCnt & processesMap = iEntityCurrent->second;
        ULONGLONG llAllEntityTraces = processesMap.GetNumOfAllEntityTraces();

        double dPercents = ComputePercent(llAllEntityTraces, numAllTraces);
        persents100 += dPercents;

        const CKBSize & traceSize = processesMap.GetTraceSizeConst();
        allTraceSize.AddSize(traceSize);
        
        strTraceSize.Clear();
        traceSize.ToString(strTraceSize);
    
        ostr << std::setw(EntityNameWidth) << strMainEntity
             << std::setw(NumberWidth) << llAllEntityTraces
             << std::setw(NumberWidth) << strTraceSize.GetString()
             << dPercents << " %"
             << endl;
    }

    strTraceSize.Clear();
    allTraceSize.ToString(strTraceSize);
    
    ostr << "--------------------------------------------" << endl
         << std::setw(ProcessNameWidth) << "All"
         << std::setw(NumberWidth) << numAllTraces
         << std::setw(NumberWidth) << strTraceSize.GetString()
         << persents100 << " %"
         << endl;
}
