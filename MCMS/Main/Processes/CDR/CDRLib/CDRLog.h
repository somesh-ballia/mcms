// CDRLog.h

#ifndef CDR_LOG_H_
#define CDR_LOG_H_

#include "Macros.h"
#include "PObject.h"
#include "DataTypes.h"
#include "XmlApi.h"
#include "InternalProcessStatuses.h"

class CCdrShort;
class CCdrShortDrv;
class CCdrEventDrv;
class CSegment;
class COsQueue;
class CCdrLongStruct;
class CCdrList;

class CCdrLog : public CPObject, CNonCopyable
{
  CLASS_TYPE_1(CCdrLog, CPObject)

 public:
                      CCdrLog();
  virtual            ~CCdrLog();
  virtual const char* NameOf() const;

  void                ConfStart(CSegment* pSeg);
  void                ConfEvent(CSegment* pSeg);
  void                ConfEnd(CSegment* pSeg);
  void                PrintCdrEventInfo(const CCdrShort* pCdrShortOld,
                                        const CCdrEventDrv& cdrEventDrv);
  STATUS              GetLongStructPtrArray(DWORD confID,
                                            DWORD partID,
                                            CCdrLongStruct& cdrLongStruct);

  CCdrList*           GetCdrList() const;
  DWORD               GetBiggestConfId() const;
  bool                IsReady() const;
  bool                InitDB();
  void                SetCdrMarked(const char* fname);
  BOOL                RenameExistingCDRDirectory();

 private:
  STATUS              CreateCDRFile(const CCdrShort& src, bool start_event);

  FILE*               From_SHORT_to_LONG(FILE* input_file);
  void                UpdateDisplayName(DWORD confID, const char* displayName);

  bool      m_IsReady;
  FILE*     m_cdrindexfile;
  FILE*     m_cdrlogfile;
  CCdrList* m_pCDRList;
};

#endif

