// CyclicFileList.cpp

#include "CyclicFileList.h"

#include <stdlib.h>
#include <stdio.h>

#include "psosxml.h"
#include "StatusesGeneral.h"
#include "OsFileIF.h"
#include "ObjString.h"
#include "ObjString.h"
#include "InitCommonStrings.h"
#include "LoggerDefines.h"

CCyclicFile::CCyclicFile()
{
  memset(m_fileName, 0, CYCLIC_FILE_MAX_NAME_LEN);
  m_sequenceNumber = 0;
  m_fileSize = 0;

  m_firstMessageTime.m_sec = 0;
  m_firstMessageTime.m_min = 0;
  m_firstMessageTime.m_hour = 0;
  m_firstMessageTime.m_day = 0;
  m_firstMessageTime.m_mon = 0;
  m_firstMessageTime.m_year = 0;

  m_lastMessageTime.m_sec = 0;
  m_lastMessageTime.m_min = 0;
  m_lastMessageTime.m_hour = 0;
  m_lastMessageTime.m_day = 0;
  m_lastMessageTime.m_mon = 0;
  m_lastMessageTime.m_year = 0;

  m_containsStartup = 0;
  m_compressionFormat = COMPRESSION_CODE_NONE;
  m_nameFormatVersion = 0;

  m_IsRetrieved = FALSE;
}

// Used by list::sort
bool operator<(const CCyclicFile& lhs, const CCyclicFile& rhs)
{
  return lhs.m_sequenceNumber < rhs.m_sequenceNumber;
}

void CCyclicFile::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
  CXMLDOMElement* pLoggerFileNode = pFatherNode->AddChildNode(m_FileHeaders.hdrXmlTag);

  // prepare VisualName (containing only SerialNum and 1stMsgDate)
  char fileVisualName[CYCLIC_FILE_MAX_NAME_LEN];
  memset(fileVisualName, 0, CYCLIC_FILE_MAX_NAME_LEN);
  snprintf(fileVisualName, sizeof(fileVisualName),
           "%s%08d_%02d-%02d-%04d_%02d-%02d-%02d.%s",
           m_FileHeaders.hdrName,
           m_sequenceNumber,
           m_firstMessageTime.m_day,
           m_firstMessageTime.m_mon,
           m_firstMessageTime.m_year,
           m_firstMessageTime.m_hour,
           m_firstMessageTime.m_min,
           m_firstMessageTime.m_sec,
           m_FileHeaders.hdrExtension);

  pLoggerFileNode->AddChildNode("NAME", m_fileName);
  pLoggerFileNode->AddChildNode("SEQUENCE_NUMBER", m_sequenceNumber);
  pLoggerFileNode->AddChildNode("FILE_SIZE", m_fileSize);
  pLoggerFileNode->AddChildNode("FIRST_MESSAGE", m_firstMessageTime);
  pLoggerFileNode->AddChildNode("LAST_MESSAGE", m_lastMessageTime);
  pLoggerFileNode->AddChildNode("CONTAINS_STARTUP", m_containsStartup);
  pLoggerFileNode->AddChildNode("VISUAL_NAME", fileVisualName);
  pLoggerFileNode->AddChildNode("COMPRESSION_FORMAT", m_compressionFormat,
                                COMPRESSION_CODE_ENUM);
  pLoggerFileNode->AddChildNode("NAME_FORMAT_VERSION", m_nameFormatVersion);
  pLoggerFileNode->AddChildNode("IS_RETRIEVED", m_IsRetrieved);
}

int CCyclicFile::DeSerializeXml(CXMLDOMElement* pActionNode,
                                char* pszError,
                                const char* action)
{
  return STATUS_OK;
}

CSerializeObject* CCyclicFile::Clone()
{
  return new CCyclicFile;
}

const char* CCyclicFile::GetFileName() const
{
  return m_fileName;
}

void CCyclicFile::SetFileName(const char* fileName)
{
  strncpy(m_fileName, fileName, sizeof(m_fileName)-1);
  m_fileName[sizeof(m_fileName)-1] = '\0';
}

void CCyclicFile::SetSequenceNumber(DWORD seqNum)
{
  m_sequenceNumber = seqNum;
}

DWORD CCyclicFile::GetSequenceNumber() const
{
  return m_sequenceNumber;
}

BOOL CCyclicFile::IsContainStartup() const
{
  return m_containsStartup;
}

BOOL CCyclicFile::IsRetrieved() const
{
  return m_IsRetrieved;
}

BYTE CCyclicFile::GetCmprFormat() const
{
  return m_compressionFormat;
}

void CCyclicFile::Dump(std::ostream& msg) const
{
  msg.setf(std::ios_base::left);

  msg << m_fileName << "\n";
  msg << "Seq Num : " << m_sequenceNumber << "\n";
  msg << "Size : " << m_fileSize << "\n";
  msg << "Is Contains Startup : " << (WORD)m_containsStartup << "\n";
  msg << "Is already retrieved : " << (WORD)m_IsRetrieved << "\n";
}

void CCyclicFile::SetParams(const string& fileName,
                            DWORD fileSequenceNumber,
                            DWORD fileSize,
                            const CStructTm& firstMessageTime,
                            const CStructTm& lastMessageTime,
                            BOOL containsStartup,
                            BOOL isRetrieved,
                            const BYTE cmprFormat,
                            int nameFormatVersion,
                            const FileNameHeaders_S& fnh)
{
  strncpy(m_fileName, fileName.c_str(), sizeof(m_fileName) - 1);
  m_fileName[sizeof(m_fileName) - 1] = '\0';

  m_compressionFormat = cmprFormat;

  memcpy(&m_FileHeaders, &fnh, sizeof(FileNameHeaders_S));

  m_sequenceNumber = fileSequenceNumber;
  m_fileSize = fileSize;
  m_firstMessageTime = firstMessageTime;
  m_lastMessageTime = lastMessageTime;
  m_containsStartup = containsStartup;
  m_IsRetrieved = isRetrieved;
  m_nameFormatVersion = nameFormatVersion;
}

void CCyclicFile::SetRetrieved()
{
  m_IsRetrieved = TRUE;
}

CCyclicFileList::CCyclicFileList(const std::string& currentDir) :
  m_numOfFiles(0),
  m_numOfUnretrievedFiles(0),
  m_lastFileSequenceNumber(0),
  m_CurrentDir(currentDir)
{}

CSerializeObject* CCyclicFileList::Clone()
{
  return NULL;
}

void CCyclicFileList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
  // Does not check existence of each file
  CYCLIC_FILE_LIST::const_iterator iter;
  for (iter = m_fileList.begin(); iter != m_fileList.end(); iter++)
    iter->SerializeXml(pFatherNode);
}

void  CCyclicFileList::SerializeXmlPartialList(CXMLDOMElement*& pFatherNode,int nLastItems) const
{
    // Does not check existence of each file
     CYCLIC_FILE_LIST::const_iterator iter = m_fileList.begin();
    if ((int)(m_fileList.size() - nLastItems) > 0) //only advances to last 200 Items if there are more then 200 Items
    {
        std::advance(iter, m_fileList.size() - nLastItems);
    }
    for (; iter != m_fileList.end(); iter++)
    {
        iter->SerializeXml(pFatherNode);
    }
}

int CCyclicFileList::DeSerializeXml(CXMLDOMElement* pActionNode,
                                    char* pszError,
                                    const char* action)
{
  return STATUS_OK;
}

CCyclicFile* CCyclicFileList::GetFileBySeqNum(DWORD wantedSeqNum)
{
  for (CYCLIC_FILE_LIST::iterator iTer = m_fileList.begin();
       iTer != m_fileList.end();
       iTer++)
  {
    CCyclicFile& currentFile = *iTer;

    const DWORD currentSeqNum = currentFile.GetSequenceNumber();
    if (currentSeqNum == wantedSeqNum)
      return &currentFile;
  }

  return NULL;
}

void CCyclicFileList::AddFile(const CCyclicFile& cyclicFile)
{
  m_fileList.push_back(cyclicFile);
  m_numOfFiles++;
}

CCyclicFile& CCyclicFileList::Back()
{
  CYCLIC_FILE_LIST::reverse_iterator iTer = m_fileList.rbegin();
  CCyclicFile& lastFile = *iTer;

  return lastFile;
}

CCyclicFile& CCyclicFileList:: Front()
{
  CCyclicFile& file = m_fileList.front();
  return file;
}

const CCyclicFile& CCyclicFileList::FrontConst() const
{
  const CCyclicFile& file = m_fileList.front();
  return file;
}

void CCyclicFileList::PopFront()
{
  m_fileList.pop_front();
  m_numOfFiles--;
}

BOOL CCyclicFileList::DeleteOldFilesCyclic(int maxNumOfFiles,
                                           DELETE_FILE_REPORT_VECTOR& outDeletedFileVector)
{
  BOOL res = TRUE;

  while (GetNumOfFiles() > maxNumOfFiles)
  {
    DeleteFileReport_S deleteFileReport;
    BOOL               deleteRes = DeleteOldestFile(deleteFileReport.fileName);
    deleteFileReport.deleteResult = deleteRes;
    outDeletedFileVector.push_back(deleteFileReport);

    res = res & deleteRes;
  }

  return res;
}

BOOL CCyclicFileList::DeleteOldestFile(std::string& outDeletedFileName)
{
  const CCyclicFile& refOldestFile = FrontConst();
  std::string        fullName = m_CurrentDir;
  fullName += refOldestFile.GetFileName();

  // Remove file entry from list
  PopFront();

  // Delete file from disk
  BOOL res = DeleteFile(fullName);
  outDeletedFileName = fullName;

  return res;
}

const CCyclicFile& CCyclicFileList::GetOldestFile() const
{
  const CCyclicFile& refCurrentFile = FrontConst();
  return refCurrentFile;
}

void CCyclicFileList::Dump(std::ostream& msg) const
{
  msg.setf(std::ios_base::left);

  msg << "Number of files : " << m_numOfFiles << "\n";
  msg << "Real Num of files : " << m_fileList.size() << '\n';
  msg << "Last Seq number : " << m_lastFileSequenceNumber << "\n";

  for (CYCLIC_FILE_LIST::const_iterator iTer = m_fileList.begin();
       iTer != m_fileList.end();
       iTer++)
  {
    msg << *iTer << "\n";
  }
}

void CCyclicFileList::SortList()
{
  m_fileList.sort();

  const DWORD sequenceNum = m_fileList.back().GetSequenceNumber();
  if (sequenceNum > m_lastFileSequenceNumber)
    m_lastFileSequenceNumber = sequenceNum;
}

void CCyclicFileList::SetFileHeaders(const FileNameHeaders_S& fnh)
{
  memcpy(&m_FileHeaders, &fnh, sizeof(FileNameHeaders_S));
}

void CCyclicFileList::CreateFileName(char* outFileName,
                                     const FileNameHeaders_S& fileHeaderNames,
                                     DWORD fileSequenceNumber,
                                     DWORD fileSize,
                                     CStructTm firstMessageTime,
                                     CStructTm lastMessageTime,
                                     BOOL containsStartup,
                                     BOOL isRetrieved,
                                     const BYTE& cmprFormat)
{
  // Logger file name convention:
  // Log_SNxxxxxxxxxx_FMDddmmyyyy_FMThhmmss_LMDddmmyyyy_LMThhmmss_SZxxxxx_SUY_CFxxxx_NFVxxRTY.log
  // SN  = Sequence Number
  // FMD = First Message Date
  // FMT = First Message Time
  // LMD = Last Message Date
  // LMT = Last Message Time
  // SZ  = file Size
  // SUY = file contains Start-Up (instead of SUY can be SUN - file doesn't contain startup)
  // CF  = Compression Format (currently for RMX V2.0 - zlib)
  // NFV = Name Format Version (currently for RMX V2.0 - 01)
  // RTY/RTN = file already retrieved

  const char* strContainsStartup = containsStartup ? "Y" : "N";
  const char* strIsRetrieved = isRetrieved ? "Y" : "N";
  BYTE nameFormatVersion = 2;

  const char* strCompressionFormat =
    (cmprFormat == COMPRESSION_CODE_ZLIB) ? "zlib" : "none";

  sprintf(
    outFileName,
    "%s_%s%010d_%s%02d%02d%04d_%s%02d%02d%02d_%s%02d%02d%04d_%s%02d%02d%02d_%s%d_%s%s_%s%s_%s%02d_%s%s.%s",
    fileHeaderNames.hdrName,
    fileHeaderNames.hdrSeqNum, fileSequenceNumber,
    fileHeaderNames.hdrFirstDate, firstMessageTime.m_day,
    firstMessageTime.m_mon,
    firstMessageTime.m_year,
    fileHeaderNames.hdrFirstTime, firstMessageTime.m_hour,
    firstMessageTime.m_min,
    firstMessageTime.m_sec,
    fileHeaderNames.hdrLastDate, lastMessageTime.m_day,
    lastMessageTime.m_mon,
    lastMessageTime.m_year,
    fileHeaderNames.hdrLastTime, lastMessageTime.m_hour,
    lastMessageTime.m_min,
    lastMessageTime.m_sec,
    fileHeaderNames.hdrFileSize, fileSize,
    fileHeaderNames.hdrIsContainStartup, strContainsStartup,
    fileHeaderNames.hdrCmprFormat, strCompressionFormat,
    fileHeaderNames.hdrNameFormatVersion, nameFormatVersion,
    fileHeaderNames.hdrIsRetrieved, strIsRetrieved,
    fileHeaderNames.hdrExtension);
}

BOOL CCyclicFileList::ParseFileName(const char* fileNameToParse,
                                    const FileNameHeaders_S& fileHeaderNames,
                                    DWORD& outFileSequenceNumber,
                                    DWORD& outFileSize,
                                    CStructTm& outFirstMessageTime,
                                    CStructTm& outLastMessageTime,
                                    BOOL& outContainsStartup,
                                    BOOL& outIsRetrieved,
                                    BYTE& outCmprFormat,
                                    int& outNameFormatVersion)
{
  // Logger file name convention:
  // LOG_SNxxxxxxxxxx_FMDddmmyyyy_FMThhmm_LMDddmmyyyy_LMThhmm_SZxxxxx_SUY_CFxxxx_NFVxx_RTY.log
  // SN  = Sequence Number
  // FMD = First Message Date
  // FMT = First Message Time
  // LMD = Last Message Date
  // LMT = Last Message Time
  // SZ  = file Size
  // SUY = file contains Start-Up (instead of SUY can be SUN - file doesn't contain startup)
  // CF  = Compression Format (currently for RMX V2.0 - zlib)
  // NFV = Name Format Version (currently for RMX V2.0 - 01)
  // RTY/RTN  = Retrieved

  char tmpFileName[CYCLIC_FILE_MAX_NAME_LEN];
  memset(tmpFileName, 0, CYCLIC_FILE_MAX_NAME_LEN);
  char* tmpString = NULL;
  char* paramString = NULL;

  char sequenceNumber[20];
  memset(sequenceNumber, 0, 20);
  char fileSize[20];
  memset(fileSize, 0, 20);
  char firstMessageTime[20];
  memset(firstMessageTime, 0, 20);
  char lastMessageTime[20];
  memset(lastMessageTime, 0, 20);
  char nameFormatVersion[20];
  memset(nameFormatVersion, 0, 20);

  strncpy(tmpFileName, fileNameToParse, sizeof(tmpFileName) - 1);
  tmpFileName[sizeof(tmpFileName) - 1] = '\0';

  tmpString = strstr(tmpFileName, "SN");  // returns a pointer to the name starting with "SN" (without the prefix "LOG_")
  if (NULL == tmpString)
    return FALSE;

  paramString = strtok(tmpString, "_"); // returns a pointer to the first token that is delimited with '_'

  // Flag for each component
  BYTE snFlag = 0;
  BYTE fmdFlag = 0;
  BYTE fmtFlag = 0;
  BYTE lmdFlag = 0;
  BYTE lmtFlag = 0;
  BYTE szFlag = 0;
  BYTE suFlag = 0;
  BYTE logFlag = 0;
  BYTE rtFlag = 0;

  while (NULL != paramString)
  {
    // SN = Sequence Number
    if (0 == strncmp(paramString,
                     fileHeaderNames.hdrSeqNum,
                     strlen(fileHeaderNames.hdrSeqNum)))
    {
      snFlag = 1;
      if (12 != strlen(paramString))      // 12 = 2 chars for "SN" + 10 digits for the seq. num.
        return FALSE;

      strncpy(sequenceNumber, paramString+2, 10); // 10 = 10 digits for the seq. num. (after 2 chars for "SN")
      sequenceNumber[10] = '\0';
      if (false == CObjString::IsNumeric(sequenceNumber))
        return FALSE;

      outFileSequenceNumber = atoi(sequenceNumber);
    }

    // FMD = First Message Date
    if (0 == strncmp(paramString,
                     fileHeaderNames.hdrFirstDate,
                     strlen(fileHeaderNames.hdrFirstDate)))
    {
      fmdFlag = 1;
      if (11 != strlen(paramString))        // 11 = 3 chars for "FMD" + 2 for day + 2 for month + 4 for year
        return FALSE;

      // day
      strncpy(firstMessageTime, paramString+3, 2);  // +3 to get to day's chars
      firstMessageTime[2] = '\0';
      if (FALSE == CObjString::IsNumeric(firstMessageTime))
        return FALSE;

      outFirstMessageTime.m_day = atoi(firstMessageTime);
      // month
      strncpy(firstMessageTime, paramString+5, 2);  // +5 to get to month's chars
      firstMessageTime[2] = '\0';
      if (FALSE == CObjString::IsNumeric(firstMessageTime))
        return FALSE;

      outFirstMessageTime.m_mon = atoi(firstMessageTime);
      // year
      strncpy(firstMessageTime, paramString+7, 4);  // +7 to get to year's chars
      firstMessageTime[4] = '\0';
      if (FALSE == CObjString::IsNumeric(firstMessageTime))
        return FALSE;

      outFirstMessageTime.m_year = atoi(firstMessageTime);
    }

    if (0 == strncmp(paramString,
                     fileHeaderNames.hdrFirstTime,
                     strlen(fileHeaderNames.hdrFirstTime)))
    {
      fmtFlag = 1;
      // 9 = 3 chars for "FMT" + 2 for hour + 2 for minutes + 2 for seconds
      if (9 != strlen(paramString))
        return FALSE;

      // hour
      strncpy(firstMessageTime, paramString+3, 2);  // +3 to get to hour's chars
      firstMessageTime[2] = '\0';
      if (FALSE == CObjString::IsNumeric(firstMessageTime))
        return FALSE;

      outFirstMessageTime.m_hour = atoi(firstMessageTime);
      // minutes
      strncpy(firstMessageTime, paramString+5, 2);  // +5 to get to minutes' chars
      firstMessageTime[2] = '\0';
      if (FALSE == CObjString::IsNumeric(firstMessageTime))
        return FALSE;

      outFirstMessageTime.m_min = atoi(firstMessageTime);
      // seconds
      strncpy(firstMessageTime, paramString+7, 2);  // +7 to get to seconds' chars
      firstMessageTime[2] = '\0';
      if (FALSE == CObjString::IsNumeric(firstMessageTime))
        return FALSE;

      outFirstMessageTime.m_sec = atoi(firstMessageTime);
    }

    // LMD = Last Message Date
    if (0 == strncmp(paramString,
                     fileHeaderNames.hdrLastDate,
                     strlen(fileHeaderNames.hdrLastDate)))
    {
      lmdFlag = 1;
      // 11 = 3 chars for "LMD" + 2 for day + 2 for month + 4 for year
      if (11 != strlen(paramString))
        return FALSE;

      // day
      strncpy(lastMessageTime, paramString+3, 2);   // +3 to get to day's chars
      lastMessageTime[2] = '\0';
      if (FALSE == CObjString::IsNumeric(lastMessageTime))
        return FALSE;

      outLastMessageTime.m_day = atoi(lastMessageTime);
      // month
      strncpy(lastMessageTime, paramString+5, 2);   // +5 to get to month's chars
      lastMessageTime[2] = '\0';
      if (FALSE == CObjString::IsNumeric(lastMessageTime))
        return FALSE;

      outLastMessageTime.m_mon = atoi(lastMessageTime);
      // year
      strncpy(lastMessageTime, paramString+7, 4);   // +7 to get to year's chars
      lastMessageTime[4] = '\0';
      if (FALSE == CObjString::IsNumeric(lastMessageTime))
        return FALSE;

      outLastMessageTime.m_year = atoi(lastMessageTime);
    }

    // LMT = Last Message Time
    if (0 == strncmp(paramString,
                     fileHeaderNames.hdrLastTime,
                     strlen(fileHeaderNames.hdrLastTime)))
    {
      lmtFlag = 1;
      // 9 = 3 chars for "LMT" + 2 for hour + 2 for minutes + 2 for seconds
      if (9 != strlen(paramString))
        return FALSE;

      // hour
      strncpy(lastMessageTime, paramString+3, 2);   // +3 to get to hour's chars
      lastMessageTime[2] = '\0';
      if (FALSE == CObjString::IsNumeric(lastMessageTime))
        return FALSE;

      outLastMessageTime.m_hour = atoi(lastMessageTime);
      // minutes
      strncpy(lastMessageTime, paramString+5, 2);   // +5 to get to minutes' chars
      lastMessageTime[2] = '\0';
      if (FALSE == CObjString::IsNumeric(lastMessageTime))
        return FALSE;

      outLastMessageTime.m_min = atoi(lastMessageTime);
      // seconds
      strncpy(lastMessageTime, paramString+7, 2);   // +7 to get to seconds' chars
      lastMessageTime[2] = '\0';
      if (FALSE == CObjString::IsNumeric(lastMessageTime))
        return FALSE;

      outLastMessageTime.m_sec = atoi(lastMessageTime);
    }

    // SZ = File Size
    if (0 == strncmp(paramString,
                     fileHeaderNames.hdrFileSize,
                     strlen(fileHeaderNames.hdrFileSize)))
    {
      szFlag = 1;
      if (3 > strlen(paramString))      // 3 = 2 chars for "SZ" + at least one digit for size
        return FALSE;

      strncpy(fileSize, paramString+2, sizeof(fileSize)-1);
      fileSize[sizeof(fileSize)-1] = '\0';
      if (FALSE == CObjString::IsNumeric(fileSize))
        return FALSE;

      outFileSize = atoi(fileSize);
    }

    // SU = Start-Up (Yes/NO)
    if (0 == strncmp(paramString,
                     fileHeaderNames.hdrIsContainStartup,
                     strlen(fileHeaderNames.hdrIsContainStartup)))
    {
      suFlag = 1;
      if ('Y' == paramString[2])
        outContainsStartup = 1;
      else if ('N' == paramString[2])
        outContainsStartup = 0;
      else
        return FALSE;
    }

    if (0 ==
        strncmp(paramString, fileHeaderNames.hdrCmprFormat,
                strlen(fileHeaderNames.hdrCmprFormat)))                                                  // CF = Compression Format
    {
      if (strlen(fileHeaderNames.hdrCmprFormat) + 1 > strlen(paramString)) // 3 = 2 chars for "CF" + at least one char for the format name
        return FALSE;

      // if (strcmp(fileHeaderNames.hdrCmprFormat, "zlib")==0)
      if (strcmp(paramString, "CFzlib") == 0)
        outCmprFormat = COMPRESSION_CODE_ZLIB;
      else
        outCmprFormat = COMPRESSION_CODE_NONE;
    }

    // NFV = Name Format Version
    if (0 == strncmp(paramString,
                     fileHeaderNames.hdrNameFormatVersion,
                     strlen(fileHeaderNames.hdrNameFormatVersion)))
    {
      // 5 = 3 chars for "NFV" + 2 digits for the format version of the log file name
      if (5 != strlen(paramString))
        return FALSE;

      strncpy(nameFormatVersion, paramString+3, 2); // +3 to get to the format version of the log file name
      if (FALSE == CObjString::CObjString::IsNumeric(nameFormatVersion))
        return FALSE;

      outNameFormatVersion = atoi(nameFormatVersion);
    }

    if (outNameFormatVersion > 1) // Retrieved yes/no
    {
      // RT = retrieved(Yes/NO)
      if (0 == strncmp(paramString,
                       fileHeaderNames.hdrIsRetrieved,
                       strlen(fileHeaderNames.hdrIsRetrieved)))
      {
        rtFlag = 1;
        if ('Y' == paramString[2])
          outIsRetrieved = 1;
        else if ('N' == paramString[2])
          outIsRetrieved = 0;
        else
          return FALSE;
      }
    }
    else
      outIsRetrieved = 1;       // we assume that old files are already retrieved

    // .log file - every file should end with this extension
    if (0 == strncmp(paramString,
                     fileHeaderNames.hdrExtension,
                     strlen(fileHeaderNames.hdrExtension)))
      logFlag = 1;

    paramString = strtok(NULL, "_.\0"); // returns a pointer to the next token that is delimited with '_' or '.' (the last one)
  }

  // Check that all file's components were found
  if ((snFlag == 0) || (fmdFlag == 0) || (fmtFlag == 0) || (lmdFlag == 0) ||
      (lmtFlag == 0) || (szFlag == 0) || (suFlag == 0) || (logFlag == 0))
    return FALSE;

  return TRUE;
}

void CCyclicFileList::UpdateRetrieved(DWORD seq,
                                      const std::string file_name)
{
  CCyclicFile* file = GetFileBySeqNum(seq);
  if (file)
  {
    file->SetFileName(file_name.c_str());
    file->SetRetrieved();
  }
}
