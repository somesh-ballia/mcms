#ifndef TRACE_STATISTICS_H_
#define TRACE_STATISTICS_H_


#include <map>
using namespace std;

#include "PObject.h"
#include "StructTm.h"
#include "ObjString.h"







/*----------------------------------------------------------------
 class CKBSize
----------------------------------------------------------------*/

class CKBSize
{
public:
    CKBSize();
    
    void AddSize(ULONGLONG size);
    void AddSize(const CKBSize & other);
    void ToString(CObjString & str)const;

private:
    
    ULONGLONG m_Size;
};








/*----------------------------------------------------------------
   CMapProcessCnt. this data structure represents a table:
   ------------------------------------
 |   process   |  cnt
-----------------------
 |   Art       |  3
 |   Video     |  5
 |   ConfParty |  0xffff
    ...
----------------------------------------------------------------*/ 
class CMapProcessCnt : public map<DWORD, ULONGLONG>
{
public:
    CMapProcessCnt();
    virtual ~CMapProcessCnt();

    //   void IncrementCnt();
	virtual const char* NameOf() const { return "CMapProcessCnt";}
    CKBSize& GetTraceSize();
    const CKBSize& GetTraceSizeConst()const;

    
    ULONGLONG GetNumOfAllEntityTraces()const;
    void AddTrace(DWORD process, DWORD size);
    
    
private:
    // disabled
    CMapProcessCnt& operator = (const CMapProcessCnt&);
    
    ULONGLONG m_CntAllTraces;  // contains cnt for the entire entity.
    CKBSize m_Size;        // contains sum of sizes of all traces.
};








/*----------------------------------------------------------------
   CMapEntityProcess. this data structure represents a table:
 ------------------------------------
 entity,cnt,size |   process   |  cnt
 ------------------------------------
 MPL             |   Art       |  3
                 |   Video     |  5
 MCMS            |   ConfParty |  0xffff
 ...
----------------------------------------------------------------*/

class CMapEntityProcess : public map<DWORD, CMapProcessCnt >
{
public:
    CMapEntityProcess();
    virtual ~CMapEntityProcess(){;}
    void AddTrace(DWORD mainEntity, DWORD process, DWORD size);
    void Reset();
	virtual const char* NameOf() const { return "CMapEntityProcess";}
    ULONGLONG GetNumOfAllTraces()const;
    
private:
    // disabled
    CMapEntityProcess(const CMapEntityProcess&);
    CMapEntityProcess&operator=(const CMapEntityProcess&);

    ULONGLONG m_NumAllTraces;
};









/*----------------------------------------------------------------
 class CTraceStatistics
----------------------------------------------------------------*/
class CTraceStatistics : public CPObject
{
CLASS_TYPE_1(CTraceStatistics, CPObject)
public:
    CTraceStatistics();
    virtual ~CTraceStatistics();
    virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CTraceStatistics";}

    void AddTrace(DWORD mainEntity, DWORD process, DWORD size);
    void Reset();
    ULONGLONG GetNumOfAllTraces() const;
private:
    // disabled
    CTraceStatistics(const CTraceStatistics&);
    CTraceStatistics& operator=(const CTraceStatistics&);

    void DumpInsideEntities(std::ostream &ostr)const;
    void DumpPerEntity(std::ostream &ostr)const;

    CMapEntityProcess m_MapEntityProcess;
    CStructTm m_StartTime;
};






#endif /*TRACE_STATISTICS_H_*/
