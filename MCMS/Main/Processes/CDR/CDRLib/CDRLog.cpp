// CDRLog.cpp

#include "CDRLog.h"

#include <map>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "Trace.h"
#include "CDRDefines.h"
#include "CDRShort.h"
#include "Macros.h"
#include "Segment.h"
#include "OsFileIF.h"
#include "SystemFunctions.h"
#include "StatusesGeneral.h"
#include "Native.h"
#include "psosxml.h"
#include "CDRProcess.h"
#include "SocketApi.h"
#include "TraceStream.h"
#include "CdrListGet.h"
#include "CDRDetal.h"
#include "ApiStatuses.h"
#include "ObjString.h"
#include "ConfPartySharedDefines.h"
#include "ConfStart.h"
#include "UnicodeDefines.h"
#include "FaultsDefines.h"
#include "HlogApi.h"

#define CDR_DIR_NAME "CdrFiles/"
#define CDR_FILE_EXT ".cdr"

#define CDR_FILE_NAME "c"
#define CDR_SHORT_INDEX_FILE "CdrFiles/cdrindex.log"
#define CDR_OLD_INDEX_FILE   "CdrFSHORT_INDEX_FILE_NAMEiles/cdrshort.log"
#define SHORT_INDEX_FILE_NAME "cdrindex.log"

static const DWORD CDR_FILE_NAME_LEN = strlen(CDR_FILE_NAME);
static const DWORD MAX_CDR_FILE_NAME = CDR_FILE_NAME_LEN + 10;
static const DWORD MAX_CDR_FILE_SIZE = 1024 * 1024;

static bool IsConfAssertedAlready(DWORD confId)
{
  static std::map<DWORD, bool> ConfAssertTable;
  bool isFound = (ConfAssertTable.end() != ConfAssertTable.find(confId));
  if (!isFound)
    ConfAssertTable[confId] = true;

  return isFound;
}

static void AssertMsgDesc(int index,
                          const char* message,
                          const char* description)
{
  static CSmallString fullMessage;
  fullMessage.Clear();

  fullMessage << "Line:" << index << " : " << message << " : " << description;

  FPASSERTMSG(TRUE, fullMessage.GetString());
}

#define ASSERT_MSG_DESC(cond, message, desc) \
  { \
    if (cond) \
    { \
      AssertMsgDesc(__LINE__, (message), (desc)); \
    } \
  }

CCdrLog::CCdrLog() :
  m_IsReady(false),
  m_cdrindexfile(NULL),
   m_cdrlogfile(NULL),
  m_pCDRList(new CCdrList)

{}

CCdrLog::~CCdrLog(void)
{
  delete m_pCDRList;
}

// Virtual
const char* CCdrLog::NameOf() const
{
  return GetCompileType();
}

DWORD CCdrLog::GetBiggestConfId() const
{
  return m_pCDRList->FindBiggestConfId();
}

bool CCdrLog::InitDB()
{
  char     fname[9];
  DWORD    filenum;
  long int record_offset = 0;
  long int recCount = 1;

  BOOL ret = CreateDirectory(CDR_DIR_NAME);
  if (FALSE == ret)
  {
    ASSERT_MSG_DESC(true, "CreateDirectory failure", CDR_DIR_NAME);
    return false;
  }

  // Go with old file id new file does not exist
  if (!IsFileExists(CDR_SHORT_INDEX_FILE))
  {
    m_cdrindexfile = fopen(CDR_OLD_INDEX_FILE, "r");

    if (m_cdrindexfile == NULL && IsFileExists(CDR_OLD_INDEX_FILE))
    {
      TRACEINTOFUNC <<  "Failed to open " << CDR_OLD_INDEX_FILE
                    << " although it exists. Try to change permission and open again";

      chmod(CDR_OLD_INDEX_FILE, 0664);
      m_cdrindexfile = fopen(CDR_OLD_INDEX_FILE, "r");
    }

    if (m_cdrindexfile != NULL)
      m_cdrindexfile = From_SHORT_to_LONG(m_cdrindexfile);
    else
      m_cdrindexfile = fopen(CDR_SHORT_INDEX_FILE, "a+");

    PASSERTSTREAM_AND_RETURN_VALUE(NULL == m_cdrindexfile,
        "fopen: " << CDR_OLD_INDEX_FILE << ": " << strerror(errno),
        false);
  }
  else
  {
    m_cdrindexfile = fopen(CDR_SHORT_INDEX_FILE, "a+");

    if (m_cdrindexfile == NULL && IsFileExists(CDR_SHORT_INDEX_FILE))
    {
      TRACEINTOFUNC <<  "Failed to open " << CDR_SHORT_INDEX_FILE
                    << " although it exists. Try to change permission and open again";

      chmod(CDR_SHORT_INDEX_FILE, 0664);
      m_cdrindexfile = fopen(CDR_SHORT_INDEX_FILE, "r");
    }

    PASSERTSTREAM_AND_RETURN_VALUE(NULL == m_cdrindexfile,
        "fopen: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno),
        false);
  }

  // The function returns no value
  rewind(m_cdrindexfile);

  CCdrShort cdrShort;
  WORD      Last_file_id = 0;
  CStructTm last_start_date_time;
  CStructTm curTime;
  SystemGetTime(curTime);

  ALLOCBUFFER(buf, CDR_SIZE_RECORD);
  WORD NumOfRecords = 0;
  while (0 != fread(buf, 1, CDR_SIZE_RECORD - 1, m_cdrindexfile))
  {
    // Ignores empty lines
    if (0 == (strcmp(buf, "\n")))
      continue;

    // Cuts the string by the end of record
    const char tail[] = ";\n";
    char*      ptr = strstr(buf, tail);

    // Keeps semicolon at the record
    if (NULL != ptr)
      ptr[1] = '\0';

    CIstrStream istr(buf);
    bool        isDiplayName = true;
    cdrShort.DeSerialize(CONFIG, istr, 0, isDiplayName);

    const char* file_name = cdrShort.GetFileName();
    NumOfRecords = filenum = (*file_name == '\0' ?
        0 : atoi(file_name + CDR_FILE_NAME_LEN));

    if ((*cdrShort.GetActualStrtTime()).IsValidForCdr())
    {
      if ((*cdrShort.GetActualStrtTime()) <= curTime)
        if ((last_start_date_time) <= (*cdrShort.GetActualStrtTime()))
        {
          last_start_date_time = *cdrShort.GetActualStrtTime();
          Last_file_id = filenum;
        }

    }
    else
    {
      TRACEWARN << "CDR record not valid:" << cdrShort;
    }

    m_pCDRList->SetFileNumber(filenum);
    record_offset = (recCount - 1) * (CDR_SIZE_RECORD - 1);
    cdrShort.SetOffset(record_offset);

    int stat = m_pCDRList->Add(cdrShort);

    /*********************************************************/
    /* 21.4.10 VNGR 14769 changed by Rachel Cohen            */
    /* if Add(cdrShort) return err do not increase recCount  */
    /*********************************************************/
    if (stat == STATUS_OK)
      recCount++;
  }

  BYTE rval = fclose(m_cdrindexfile);
  if (0 != rval)
  {
    DEALLOCBUFFER(buf);
    PASSERTSTREAM(true,
        "fclose: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno));
    return false;
  }

  int          max_cdrs = MAX_CDR_SHORT_IN_LIST;
  eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
  if (eProductTypeRMX4000 == curProductType)
    max_cdrs = MAX_CDR_SHORT_IN_LIST_FOR_AMOS;

  if (NumOfRecords < max_cdrs)
    Last_file_id = NumOfRecords;

  /**********************************************/
  /* 21.4.10 VNGR 14769 changed by Rachel Cohen */
  /* m_pCDRList->SetIndexShort((Last_file_id)); */
  /**********************************************/
  m_pCDRList->SetFileNumber((DWORD)Last_file_id);
  // VNGFE-4891 shimon
  m_pCDRList->SetIndexShort(Last_file_id);
  DEALLOCBUFFER(buf);
  m_IsReady = true;

  return true;
}

void CCdrLog::ConfStart(CSegment* pSeg)
{
  CCdrShortDrv cdr;
  cdr.DeSerialize(NATIVE, *pSeg);

  DWORD confid = cdr.GetConfId();

  TRACEINTO << "A new conference arrived: " << cdr;

  CCdrShort* pCdrShortOld = m_pCDRList->GetCurrentShort(confid);
  if (pCdrShortOld != NULL)
  {
    CMedString message = "conference exist; ";

    const char* oldconf_name, * newconf_name;
    oldconf_name = pCdrShortOld->GetH243ConfName();
    newconf_name = cdr.GetH243ConfName();
    if (strcmp(oldconf_name, newconf_name) != 0)
    {
      message << "conference have a new name : " << oldconf_name
              << " -> " << newconf_name;

      PASSERTMSG(true, message.GetString());
      return;
    }
    else
    {
      message << "offset updated";
      TRACEINTO << "\n" << message.GetString();

      int   index = m_pCDRList->FindId(confid);
      DWORD record_offset = index * (CDR_SIZE_RECORD - 1);
      pCdrShortOld->SetOffset(record_offset);
    }
  }
  else
  {
    CreateCDRFile(cdr, true);
  }
}

STATUS CCdrLog::CreateCDRFile(const CCdrShort& src, bool start_event)
{
  CCdrShort cdr = src;

  m_pCDRList->IncrementFileNumber();

  DWORD filenum = m_pCDRList->GetFileNumber();
  DWORD record_offset =
    (filenum != 0 ? (filenum - 1) * (CDR_SIZE_RECORD - 1) : 0);

  cdr.SetOffset(record_offset);
  cdr.SetFileVersion(API_NUMBER);
  cdr.SetFileMarked(NO);

  std::ostringstream fname;
  fname << CDR_FILE_NAME << filenum;
  cdr.SetFileName(fname.str().c_str());

  // Does not increment file part index for start event, the index = 1
  if (!start_event)
    cdr.IncrementFilePartIndex();

  STATUS stat = m_pCDRList->Add(cdr);
  if (STATUS_OK != stat)
    return stat;

  COstrStream ostr1;
  cdr.Serialize(CONFIG, ostr1, 0, true);
  string strShortForIndex = ostr1.str();

  COstrStream ostr2;
  cdr.Serialize(CONFIG, ostr2, 0, false);
  string strShortForData = ostr2.str();

  std::ostringstream fpath;
  fpath << CDR_DIR_NAME << fname.str() << CDR_FILE_EXT;

  m_cdrlogfile = fopen(fpath.str().c_str(), "w");
  PASSERTSTREAM_AND_RETURN_VALUE(NULL == m_cdrlogfile,
      "fopen: " << fpath.str() << ": " << strerror(errno),
      STATUS_FAIL);

  m_cdrindexfile = fopen(CDR_SHORT_INDEX_FILE, "r+");
  if (m_cdrindexfile == NULL)
  {
    fclose(m_cdrlogfile);
    PASSERTSTREAM(true,
        "fopen: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno));
    return STATUS_FAIL;
  }

  // The function returns no value
  rewind(m_cdrindexfile);

  int rs = fflush(m_cdrindexfile);
  PASSERTSTREAM(rs < 0,
      "fflush: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno));

  rs = fseek(m_cdrindexfile, record_offset, SEEK_SET);
  PASSERTSTREAM(rs < 0,
      "fseek: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno));

  rs = fputs(strShortForIndex.c_str(), m_cdrindexfile);
  PASSERTSTREAM(rs < 0,
      "fputs: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno));

  rs = fputs(strShortForData.c_str(), m_cdrlogfile);
  PASSERTSTREAM(rs < 0,
      "fputs: " << fpath.str() << ": " << strerror(errno));

  rs = fclose(m_cdrindexfile);
  PASSERTSTREAM(rs < 0,
      "fclose: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno));

  rs = fclose(m_cdrlogfile);
  PASSERTSTREAM(rs < 0,
      "fclose: " << fpath.str() << ": " << strerror(errno));

  return STATUS_OK;
}

void CCdrLog::ConfEvent(CSegment* pSeg)
{
  DWORD confID;
  *pSeg >> confID;

  const CCdrShort* pCdrShortOld = m_pCDRList->GetCurrentShort(confID);
  PASSERTSTREAM_AND_RETURN(NULL == pCdrShortOld,
        "CDR file for ConfID " << confID << " doesn't not exist");

  CCdrEventDrv  cdrEventDrv;
  CCdrEventDrv& refCdrEventDrv = cdrEventDrv;

  refCdrEventDrv.DeSerialize(NATIVE, *pSeg);

  eConfCdrStatus status = pCdrShortOld->GetStatus();
  if (ONGOING_CONFERENCE != status)
  {
    PASSERTSTREAM(true,
        "CDR file status for ConfID " << confID
        << ", PartID " << pCdrShortOld->GetFilePartIndex()
        << " should be " << GetConfCdrStatusName(ONGOING_CONFERENCE)
        << "(" << ONGOING_CONFERENCE << ")"
        << ", current status " << GetConfCdrStatusName(status)
        << "(" << status << ")");

    PrintCdrEventInfo(pCdrShortOld, cdrEventDrv);
    return;
  }

  char filename[256];
  snprintf(filename, sizeof(filename), "%s%s%s", CDR_DIR_NAME,
           pCdrShortOld->GetFileName(), CDR_FILE_EXT);

  int res = GetFileSize(filename);
  if (-1 == res)
  {
    PASSERTSTREAM(true, "stat: " << filename << ": " << strerror(errno));
    return;
  }

  if (MAX_CDR_FILE_SIZE < (DWORD)res)
  {
    if (CCdrShort::IsEnableCDRFileParts())
    {
      STATUS stat = CreateCDRFile(*pCdrShortOld, false);
      if (STATUS_FAIL == stat)
        return;
    }
    else
    {
      bool isAsserted = IsConfAssertedAlready(confID);
      if (true == isAsserted)
        return;

      CMedString description;
      description << "CDR File for conference "
                  << pCdrShortOld->GetH243ConfName() << ":" << confID
                  << " reached its maximum size, "
                  << MAX_CDR_FILE_SIZE
                  << " bytes";

      CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
                          CDR_FILE_REACH_MAX_CAPACITY,
                          MAJOR_ERROR_LEVEL,
                          description.GetString(),
                          FALSE);
      return;
    }
  }

  COstrStream ostr;
  ((CCdrEvent&)refCdrEventDrv).Serialize(CONFIG, ostr, 0);

  string str = ostr.str();

  m_cdrlogfile = fopen(filename, "a");
  if (m_cdrlogfile == NULL)
  {
    PASSERTSTREAM(true,
                  "fopen: " << filename << ": " << strerror(errno));
    return;
  }

  fputs(str.c_str(), m_cdrlogfile);

  BYTE rval = fclose(m_cdrlogfile);
  ASSERT_MSG_DESC(0 != rval, "Failed to close file", filename);

  WORD eventType = refCdrEventDrv.GetCdrEventType();
  if (CONFERENCE_START_CONTINUE_10 == eventType)
  {
    CConfStartCont10* confStart10 = refCdrEventDrv.GetConfStartCont10();
    const char*       displayName = confStart10->GetConfDisplayName();
    if (NULL != displayName)
    {
      TRACEINTO << "\nConference Display Name : " << displayName;
      UpdateDisplayName(confID, displayName);
    }

    PDELETE(confStart10);
  }
}

void CCdrLog::UpdateDisplayName(DWORD confID, const char* displayName)
{
  // Updates display name in index
  CCdrShort* pCdrShortOld = m_pCDRList->GetCurrentShort(confID);
  PASSERTSTREAM_AND_RETURN(NULL == pCdrShortOld,
      "Conference doesn't not exist " << confID);

  eConfCdrStatus status = pCdrShortOld->GetStatus();
  if (status != ONGOING_CONFERENCE)
  {
    PASSERTSTREAM(true,
                  "CDR file status for ConfID " << confID
                  << ", PartID " << pCdrShortOld->GetFilePartIndex()
                  << " should be " << GetConfCdrStatusName(ONGOING_CONFERENCE)
                  << "(" << ONGOING_CONFERENCE << ")"
                  << ", current status " << GetConfCdrStatusName(status)
                  << "(" << status << ")");

    return;
  }

  pCdrShortOld->SetDisplayName(displayName);

  const char* file_name = pCdrShortOld->GetFileName();
  long int    record_offset = pCdrShortOld->GetOffset();

  COstrStream ostr;
  ((CCdrShort*)pCdrShortOld)->Serialize(NATIVE, ostr, 0, true);
  string strShort = ostr.str();

  m_cdrindexfile = fopen(CDR_SHORT_INDEX_FILE, "r+");
  if (m_cdrindexfile == NULL)
  {
    PASSERTSTREAM(true,
        "fopen: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno));
    return;
  }

  rewind(m_cdrindexfile);
  fflush(m_cdrindexfile);
  fseek(m_cdrindexfile, record_offset, SEEK_SET);
  fputs(strShort.c_str(), m_cdrindexfile);

  BYTE rval = fclose(m_cdrindexfile);
  PASSERT(0 != rval);
}

void CCdrLog::ConfEnd(CSegment* pSeg)
{
  long int    record_offset = 0;
  const char* file_name;

  m_cdrindexfile = NULL;
  m_cdrlogfile = NULL;

  DWORD confID;
  *pSeg >> confID;

  int            i = 0;
  eConfCdrStatus segCause;
  WORD           tmp;
  *pSeg >> tmp;
  segCause = (eConfCdrStatus)tmp;

  CCdrShort* pCdrShortOld = m_pCDRList->GetCurrentShort(confID);
  if (pCdrShortOld == NULL)
  {
    PASSERTSTREAM(true, "Conference doesn't exist, ConfID " << confID);
    return;
  }

  CStructTmDrv startTimeDrv;
  startTimeDrv.DeSerialize(NATIVE, *pSeg);

  CStructTmDrv actualDurationDrv;
  actualDurationDrv.DeSerialize(NATIVE, *pSeg);

  CCdrEventDrv cdrEventDrv;
  cdrEventDrv.DeSerialize(NATIVE, *pSeg);

  eConfCdrStatus status = pCdrShortOld->GetStatus();
  if (status != ONGOING_CONFERENCE)
  {
    PASSERTSTREAM(true,
        "CDR file status for ConfID " << confID
        << ", PartID " << pCdrShortOld->GetFilePartIndex()
        << " should be " << GetConfCdrStatusName(ONGOING_CONFERENCE)
        << "(" << ONGOING_CONFERENCE << ")"
        << ", current status " << GetConfCdrStatusName(status)
        << "(" << status << ")");

    PrintCdrEventInfo(pCdrShortOld, cdrEventDrv);
    return;
  }

  file_name = pCdrShortOld->GetFileName();
  record_offset = pCdrShortOld->GetOffset();
  pCdrShortOld->SetActualStrtTime((CStructTm&)startTimeDrv);
  pCdrShortOld->SetActualDuration((CStructTm&)actualDurationDrv);

  // Updates status for the CDR and all parts
  m_pCDRList->SetStatus(confID, segCause);

  COstrStream ostr1;
  bool        isDisplayName = true;
  ((CCdrShort*)pCdrShortOld)->Serialize(CONFIG, ostr1, 0, isDisplayName);
  string strShortForIndex = ostr1.str();

  COstrStream ostr2;
  isDisplayName = false;
  ((CCdrShort*)pCdrShortOld)->Serialize(CONFIG, ostr2, 0, isDisplayName);
  string strShortForData = ostr2.str();

  m_cdrindexfile = fopen(CDR_SHORT_INDEX_FILE, "r+");
  PASSERTSTREAM_AND_RETURN(NULL == m_cdrindexfile,
      "fopen: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno));

  rewind(m_cdrindexfile);
  fflush(m_cdrindexfile);
  fseek(m_cdrindexfile, record_offset, SEEK_SET);
  fputs(strShortForIndex.c_str(), m_cdrindexfile);

  BYTE rval = fclose(m_cdrindexfile);
  PASSERT(0 != rval);

  // add end conference event to conference events file
  char filename[256];
  snprintf(filename, sizeof(filename), "%s%s%s",
           CDR_DIR_NAME, file_name, CDR_FILE_EXT);

  COstrStream ostr3;
  ((CCdrEvent&)cdrEventDrv).Serialize(CONFIG, ostr3, 0);
  string strEvent = ostr3.str();

  m_cdrlogfile = fopen(filename, "r+");
  PASSERTSTREAM_AND_RETURN(NULL == m_cdrlogfile,
      "fopen: " << filename << ": " << strerror(errno));

  fputs(strShortForData.c_str(), m_cdrlogfile);

  fseek(m_cdrlogfile, 0, 2);
  fputs(strEvent.c_str(), m_cdrlogfile);

  rval = fclose(m_cdrlogfile);
  ASSERT_MSG_DESC(0 != rval, "Failed to close file", filename);
}

void CCdrLog::PrintCdrEventInfo(const CCdrShort* pCdrShortOld,
                                const CCdrEventDrv& cdrEventDrv)
{
  // ===== 1. prepare conf info for printing
  COstrStream ostr;
  pCdrShortOld->Dump(ostr);

  // ===== 2. prepare event info for printing
  CXMLDOMElement dummyFather("Dummy");
  cdrEventDrv.SerializeXml(&dummyFather);
  char* eventBuf = NULL;

  dummyFather.DumpDataAsLongStringEx(&eventBuf);

  // ===== 3. print to trace
  TRACESTR(eLevelInfoNormal) << "\nCCdrLog::PrintCdrEventInfo\n"
                         << "\nConf Info:"
                         << "\n==========\n"
                         << ostr.str().c_str() << "\n"
                         << "\nEvent Info:"
                         << "\n===========\n"
                         << eventBuf;

  PDELETEA(eventBuf);
}

FILE* CCdrLog::From_SHORT_to_LONG(FILE* input_file)
{
  FILE* output_file = fopen(CDR_SHORT_INDEX_FILE, "a+");
  PASSERTSTREAM_AND_RETURN_VALUE( NULL == output_file,
    "fopen: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno),
    NULL);

  BYTE      rval;
  char      buf[CDR_SIZE_RECORD_OLD];
  WORD      format = CONFIG;
  CCdrShort cdrShort;

  while ((fgets(buf, CDR_SIZE_RECORD_OLD, input_file)) != NULL)
  {
    if ((strcmp(buf, "\n")) != 0)
    {
      CIstrStream istr(buf);
      cdrShort.DeSerialize(format, istr, 0);

      COstrStream ostr;
      cdrShort.Serialize(format, ostr, 0);
      fputs(ostr.str().c_str(), output_file);
    }
  }

  rval = fclose(input_file);
  PASSERTSTREAM(rval != 0,
      "fclose: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno));

  return output_file;
}

CCdrList* CCdrLog::GetCdrList() const
{
  return m_pCDRList;
}

bool CCdrLog::IsReady() const
{
  return m_IsReady;
}

void CCdrLog::SetCdrMarked(const char* fname)
{
  char file_name_s[64];
  strcpy_safe(file_name_s, fname);

  char* base = basename(file_name_s);
  int   index = m_pCDRList->FindId(base);
  if (index != NOT_FIND)
    m_pCDRList->FileMarked(index);
  else
    PASSERTSTREAM(0 != strcmp(base, SHORT_INDEX_FILE_NAME),
                  "Unable to find CDR filename " << base);
}

STATUS CCdrLog::GetLongStructPtrArray(DWORD confID,
                                      DWORD partID,
                                      CCdrLongStruct& cdrLongStruct)
{
  CCdrShort* pCdrShort = m_pCDRList->GetShort(confID, partID);
  PASSERTSTREAM_AND_RETURN_VALUE(pCdrShort == NULL,
    "Conference doesn't exist, ConfID " << confID << ", PartID " << partID,
    STATUS_FILE_NOT_EXISTS);

  pCdrShort->SetFileMarked(YES);

  DWORD       recoffset = pCdrShort->GetOffset();
  const char* fileName = pCdrShort->GetFileName();
  PASSERTMSG_AND_RETURN_VALUE(NULL == fileName,
    "CDR doesn't have a file name",
    STATUS_FILE_NOT_EXISTS);

  cdrLongStruct.SetShortCdrStruct(pCdrShort);

  // Puts the updated short to index file
  FILE* cdrindexfile = fopen(CDR_SHORT_INDEX_FILE, "r+");
  PASSERTSTREAM_AND_RETURN_VALUE(NULL == cdrindexfile,
    "fopen: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno),
    STATUS_FILE_NOT_EXISTS);

  COstrStream ostr1;
  bool        isDiplayName = true;
  pCdrShort->Serialize(CONFIG, ostr1, 0, isDiplayName);
  std::string strShortForIndex = ostr1.str();

  fseek(cdrindexfile, recoffset, SEEK_SET);
  fputs(strShortForIndex.c_str(), cdrindexfile);

  BYTE rval = fclose(cdrindexfile);
  PASSERTSTREAM(0 != rval,
      "fclose: " << CDR_SHORT_INDEX_FILE << ": " << strerror(errno));

  // put the updated short to data file
  std::string fulLogFileName = CDR_DIR_NAME;
  fulLogFileName += fileName;
  fulLogFileName += CDR_FILE_EXT;

  FILE* cdrlogfile = fopen(fulLogFileName.c_str(), "r+");
  PASSERTSTREAM_AND_RETURN_VALUE(NULL == cdrlogfile,
      "fopen: " << fulLogFileName.c_str() << ": " << strerror(errno),
      STATUS_FILE_NOT_EXISTS);

  COstrStream ostr2;
  isDiplayName = false;
  pCdrShort->Serialize(CONFIG, ostr2, 0, isDiplayName);
  string strShortForData = ostr2.str();

  rewind(cdrlogfile);
  fputs(strShortForData.c_str(), cdrlogfile);
  fflush(cdrlogfile);

  int fileLen = GetFileSize(fulLogFileName);
  if (-1 == fileLen)
  {
    PASSERTSTREAM(0 != rval,
        "GetFileSize: " << fulLogFileName.c_str() << ": " << strerror(errno));

    rval = fclose(cdrlogfile);
    PASSERTSTREAM(0 != rval,
        "fclose: " << fulLogFileName.c_str() << ": " << strerror(errno));

    return STATUS_FILE_NOT_EXISTS;
  }

  rewind(cdrlogfile);

  // Calc the estimated numbers of buffer we need to allocate for the file,
  // based on lseek.
  // The actual number will be calculate in the object according to the actual numread
  // in the function SetArrayOfStringPtr.

  int tmp_num_of_buf =
    (fileLen % CDR_BUF_SIZE > 0 ? fileLen / CDR_BUF_SIZE + 1 : fileLen / CDR_BUF_SIZE);

  char** bufstring = new char*[tmp_num_of_buf];

  int k;
  memset(bufstring, 0, tmp_num_of_buf);

  int total_numread = 0;
  int num_of_used_buffers = 0;
  for (k = 0; k < tmp_num_of_buf; k++)
  {
    bufstring[k] = new char[CDR_BUF_SIZE];
    memset(bufstring[k], ' ', CDR_BUF_SIZE);

    // divide the file data into small buffers.
    int numread = fread(bufstring[k], 1, CDR_BUF_SIZE, cdrlogfile);
    total_numread += numread;

    num_of_used_buffers = (numread > 0 ? k + 1 : num_of_used_buffers);

    if (numread < CDR_BUF_SIZE)
      break;
  }

  rval = fclose(cdrlogfile);
  PASSERTSTREAM(0 != rval, "fclose: " << fulLogFileName.c_str()
                                      << ": " << strerror(errno));

  // notice, total_numread and tmp_num_of_buf might by not equal,
  // since fread converts the '0d0a' to '0a' in the end of each line
  if (k != tmp_num_of_buf)
    tmp_num_of_buf = k + 1;

  // Set the pointers to the strings in the Object
  cdrLongStruct.SetArrayOfStringPtr(bufstring, total_numread);

  for (k = 0; k < tmp_num_of_buf; k++)
    delete [] bufstring[k];

  delete [] bufstring;

  return STATUS_OK;
}
