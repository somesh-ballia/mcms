// LogFileManager.cpp

#include "LogFileManager.h"

#include <errno.h>

#include "DataTypes.h"
#include "ZipCompressor.h"
#include "SystemFunctions.h"
#include "StructTm.h"
#include "Macros.h"
#include "ObjString.h"
#include "FileService.h"
#include "ProcessBase.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "TraceStream.h"
#include "Versions.h"
#include "CyclicFileManager.h"
#include "HlogApi.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"
#include "StringsMaps.h"
#include "Log4cxxConfiguration.h"

class CStructTm;
class CLoggerFile;
class CLoggerFileList;

// the size of log buffer will be CFG_PARAM + LogBufferAddition
// it may prevent failures of deflate(Z_FINISH). Not enough space.
// 524288 = 1024 * 1024 / 2 = 1/2 MB
const DWORD LogBufferAddition = 524288;

// file number since Logger Initialized
static DWORD logFileCounter = 1;

// global error indicator
static eLogFileManagerState eGlobalState = eLogFileManagerState_Idle;

#define FATAL_ERROR_BOOL(a, b) \
  { \
    FatalError(a, b); \
    eGlobalState = eLogFileManagerState_Major; \
    return FALSE; \
  }

#define FATAL_ERROR_VOID(a, b) \
    { \
      FatalError(a, b); \
      eGlobalState =  eLogFileManagerState_Major; \
      return; \
    }

#define FATAL_ERROR_NUM(a, b) \
    { \
      FatalError(a, b); \
      eGlobalState = eLogFileManagerState_Major; \
      return 0; \
    }

#define FATAL_ERROR_GLOBAL(a, b) \
    { \
      GlobalFatalError(a, b); \
      eGlobalState =  eLogFileManagerState_Major; \
      return; \
    }

#define STATE_MACHINE(wanted) \
    { \
      if ((eGlobalState) != (wanted)) \
      { \
        StateError(eGlobalState, wanted); \
        eGlobalState = eLogFileManagerState_Major; \
        return FALSE; \
      } \
    }

static void GetDateTime(ULONG& lDate, ULONG& lTime, ULONG& lTicks)
{
  CStructTm localTime;
  SystemGetTime(localTime);

  ULONG tmp;

  // time
  tmp = localTime.m_sec;
  lTime = tmp;

  tmp = localTime.m_min;
  tmp = tmp << 8;
  lTime = lTime | tmp;

  tmp = localTime.m_hour;
  tmp = tmp << 16;
  lTime = lTime | tmp;

  // date
  tmp = localTime.m_day;
  lDate = tmp;

  tmp = localTime.m_mon;
  tmp = tmp << 8;
  lDate = lDate | tmp;

  tmp = localTime.m_year;
  tmp = tmp << 16;
  lDate = lDate | tmp;

  lTicks = SystemGetTickCount().GetIntegerPartForTrace();
}

// attention : first comes recent values, then older ones.
static BOOL GetDateTimeDiffInTicks(ULONG lDate1, ULONG lTime1, ULONG lTicks1,
                                   ULONG lDate2, ULONG lTime2, ULONG lTicks2,
                                   ULONG* lTickDiff, BOOL* bDate = NULL)
{
  if (bDate != NULL)
    *bDate = FALSE;

  *lTickDiff = lDate1-lDate2;

  if (*(signed long*)lTickDiff < 0) // wrong input or system clock deliberately changed
  {
    if (bDate != NULL)
      *bDate = TRUE;

    return FALSE;
  }

  if (*lTickDiff > 0) // year / month / day changed
    *lTickDiff = SECOND * 60 * 60 * 24; // add day in ticks

  *lTickDiff += ((lTime1 >> 16) - (lTime2 >> 16)) * SECOND * 60 * 60;            // add hours in ticks
  *lTickDiff += (((lTime1 >> 8) & 0xFF) - ((lTime2 >> 8) & 0xFF)) * SECOND * 60;       // add minutes in ticks
  *lTickDiff += ((lTime1 & 0xFF) - (lTime2 & 0xFF)) * SECOND;            // add seconds in ticks
  *lTickDiff += lTicks1-lTicks2;                                                        // add ticks

  if (*(signed long*)lTickDiff < 0) // wrong input or system clock deliberately changed
    return FALSE;

  return TRUE;
}

static void LoggerErrorToFile(const char* chError)
{
  FILE* pFile = NULL;
  ULONG lDate, lTime, lTicks;
  ULONG lIOError;

  if (strlen(chError) > DEFAULT_MAX_MESSAGE_SIZE - 100)
    return;

  std::string fileName = MCU_OUTPUT_TMP_DIR;
  if (!IsTarget())
	  fileName = MCU_TMP_DIR+"/";
  fileName += ERROR_FILE_NAME;

  char chFinalError[DEFAULT_MAX_MESSAGE_SIZE];
  memset(chFinalError, 0, DEFAULT_MAX_MESSAGE_SIZE);
  
  // Opens an existing binary file for read/write
  pFile = fopen(fileName.c_str(), "r+b");

  // Error in open - tries to create new file
  if (pFile == NULL)
  {
    // File does not exist
    if (ENOENT == errno)
    {
      // Creates and opens a new binary file for reading / writing
      pFile = fopen(fileName.c_str(), "w+b");

      // Cannot create file
      if (pFile == NULL)
      {
        eGlobalState = eLogFileManagerState_Major;
        FPASSERTSTREAM(true,
          "fopen(w+b): " << fileName
          << ": " << strerror(errno) << " (" << errno << "): " << chError);
        return;
      }
    }
    else
    {
      eGlobalState = eLogFileManagerState_Major;
      FPASSERTSTREAM(true,
        "fopen(r+b): " << fileName
        << ": " << strerror(errno) << " (" << errno << ")" << chError);
      return;
    }
  }
  else
  {
    // Goes to end of file
    lIOError = fseek(pFile, 0, SEEK_END);
    if (lIOError)
    {
      eGlobalState = eLogFileManagerState_Major;
      FPASSERTSTREAM(true,
        "fseek: " << fileName
        << ": " << strerror(errno) << " (" << errno << ")" << chError);
      fclose(pFile);
      return;
    }

    long lFileSize = ftell(pFile);     // get current position - file length
    if (lFileSize == -1L)
    {
      eGlobalState = eLogFileManagerState_Major;
      FPASSERTSTREAM(true,
        "ftell: " << fileName
        << ": " << strerror(errno) << " (" << errno << ")" << chError);
      fclose(pFile);
      return;
    }

    if (lFileSize + strlen(chError) > MAX_ERROR_FILE_SIZE)
    {
      // Max file size reached - destroy file.
      fclose(pFile);

      // Creates and opens a new binary file for reading / writing
      pFile = fopen(fileName.c_str(), "w+b");
      if (pFile == NULL)
      {
        eGlobalState = eLogFileManagerState_Major;
        FPASSERTSTREAM(true,
          "fopen(w+b): " << fileName
          << ": " << strerror(errno) << " (" << errno << "): " << chError);
        return;
      }
    }
  }

  GetDateTime(lDate, lTime, lTicks);

  sprintf(chFinalError, "%02d/%02d/%04d %02d:%02d:%02d:%03d\r\n%s\r\n\r\n",
          static_cast<int>(lDate & 0xFF),
          static_cast<int>((lDate >> 8) & 0xFF),
          static_cast<int>((lDate >> 16)),
          static_cast<int>(lTime >> 16),
          static_cast<int>((lTime >> 8) & 0xFF),
          static_cast<int>(lTime & 0xFF),
          static_cast<int>(lTicks*10),
          chError);

  ULONG lSize = strlen(chFinalError);
  ULONG lBytesIn = fwrite(chFinalError, sizeof(BYTE), lSize, pFile);
  lIOError = ferror(pFile);

  // Checks that all data was written and there is no error
  if (lBytesIn != lSize || lIOError)
  {
    eGlobalState = eLogFileManagerState_Major;
    FPASSERTSTREAM(true,
      "fwrite: " << fileName
      << ": " << strerror(errno) << " (" << errno << "): " << chError);
    fclose(pFile);
    return;
  }

  lIOError = fclose(pFile);
  if (lIOError)
  {
    eGlobalState = eLogFileManagerState_Major;
    FPASSERTSTREAM(true,
      "fflush: " << fileName
      << ": " << strerror(errno) << " (" << errno << "): " << chError);
  }
}

void StateError(eLogFileManagerState current, eLogFileManagerState wanted)
{
  string buff = "StateError : current state = ";
  buff += GetLogFileManagerStateName(current);
  buff += "; wanted state : ";
  buff += GetLogFileManagerStateName(wanted);
  LoggerErrorToFile(buff.c_str());
}

CLogFileManager::CLogFileManager()
	: m_pZipCompressor(NULL), m_pFileBuffer(NULL), m_FileService(NULL), m_pCyclicFileManager(NULL)
{
  SetDefaults();
}

CLogFileManager::~CLogFileManager()
{
  LoggerShutDown();
  PDELETEA(m_pFileBuffer);
  PDELETE(m_pZipCompressor);
  PDELETE(m_FileService);
  PDELETE(m_pCyclicFileManager);
}

void CLogFileManager::SetDefaults()
{
  m_sLoggerConfiguration.lMaxMessageSize = DEFAULT_MAX_MESSAGE_SIZE;
  m_sLoggerConfiguration.lMaxNumOfFiles = DEFAULT_MAX_NUM_OF_FILES;
  m_sLoggerConfiguration.lFileRange = DEFAULT_FILE_RANGE;
  m_sLoggerConfiguration.lMaxFileSize = DEFAULT_MAX_FILE_SIZE;
  m_sLoggerConfiguration.lMaxFileTimeToLive = DEFAULT_MAX_FILE_TIME_TO_LIVE;
  m_sLoggerConfiguration.lMaxFlushedMessages = DEFAULT_MAX_FLUSHED_MESSAGES;
  m_sLoggerConfiguration.lFileBufferMultiplier = DEFAULT_FILE_BUFFER_MULTIPLIER;
  m_sLoggerConfiguration.lMaxNumOfFiles = ((CLoggerProcess*)CProcessBase::GetProcess())->GetMaxNumberOfFiles();
  PDELETEA(m_pFileBuffer);
  m_lFileBufferSize = 0;
  delete m_pZipCompressor;
  m_pZipCompressor = new CZipCompressor;
  delete m_FileService;
  m_FileService = new CFileService;
}



void CLogFileManager::InitLogger()
{
  eGlobalState = eLogFileManagerState_Startup;

  CLoggerProcess* pProcess = (CLoggerProcess*)CProcessBase::GetProcess();

  // Allocations
  m_lFileBufferSize = (DWORD)(m_sLoggerConfiguration.lMaxFileSize + LogBufferAddition);
  PDELETEA(m_pFileBuffer);
  m_pFileBuffer = new BYTE[m_lFileBufferSize];
  if (NULL == m_pFileBuffer)
    FATAL_ERROR_VOID("CLogFileManager::InitLogger : cannot allocate memory for file buffer",
                     errno);

  // Writes the MCU version number to logger file
  WriteVersionNumberToLog();

  // Creates folder if needed
  if (!IsFileExists(LOGGER_DIR))
  {
    bool res = m_FileService->CreateFolder(LOGGER_DIR);
    if (false == res)
    {
      std::stringstream buf;
      buf << "make_dir: " << LOGGER_DIR << ": "
          << strerror(errno) << " (" << errno << ")";
      FATAL_ERROR_VOID(buf.str().c_str(), errno);
    }
  }

  FileNameHeaders_S fnh;   // the default headers are supposed to be logger
  delete m_pCyclicFileManager;
  m_pCyclicFileManager = new CCyclicFileManager(LOGGER_DIR,
                                                fnh,
                                                CLogFileManager::AlertEveryXFiles);

  m_startUpFlag = (0 == pProcess->GetNumStartup());
  STATUS status = m_pCyclicFileManager->Init(pProcess->GetLoggerFileList(),
                                             m_startUpFlag,
                                             COMPRESSION_CODE_ZLIB);
  m_startUpFlag = FALSE;
  if (STATUS_OK != status)
    FATAL_ERROR_VOID("CCyclicFileManager::Init", errno);

  // Initialize m_lFirstMessagePrefix & m_lLastMessagePrefix data members
  memset(&m_lFirstMessagePrefix, 0, sizeof(data_file_data_prefix));
  memset(&m_lLastMessagePrefix, 0, sizeof(data_file_data_prefix));

  // Init Compressor
  int lErr = m_pZipCompressor->Init(m_pFileBuffer, m_lFileBufferSize);
  if (lErr != Z_OK)
    FATAL_ERROR_VOID("CLogFileManager::InitLogger : Cannot init zip deflate",
                     lErr);

  eGlobalState = eLogFileManagerState_Ready;
}

void CLogFileManager::WriteVersionNumberToLog()
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  CVersions ver;
  std::string versionFilePath = VERSIONS_FILE_PATH;
  STATUS status = ver.ReadXmlFile(versionFilePath.c_str());
  PASSERTSTREAM_AND_RETURN(STATUS_OK != status,
    "ReadXmlFile: " << VERSIONS_FILE_PATH << ": Failed to parse: "
    << proc->GetStatusAsString(status));

  VERSION_S mcuv = ver.GetMcuVersion();
  VERSION_S mcmv = ver.GetMcmsVersion();
  eProductType productType = proc->GetProductType();

  CLoggerProcess* pProcess = (CLoggerProcess*)CProcessBase::GetProcess();
  const LOGGER_LICENSING_S* pLicensingData = pProcess->GetLicensingData();
  ostringstream str;
  if (pLicensingData)
  {
	  const char *pszMediaCard = NULL;
	  BYTE res = CStringsMaps::GetDescription(SYSTEM_CARDS_MODE_ENUM, pLicensingData->sys_card_mode, &pszMediaCard);
	  str << "\nMedia Card Type : " << pszMediaCard
		  << "\nNum of CP ports : " << pLicensingData->num_cp_parties
		  << "\nNum of COP ports: " << pLicensingData->num_cop_parties;
  }

  TRACESTR(eLevelError)
      << "MCU Version   : "
      << mcuv.ver_major << "."
      << mcuv.ver_minor << "."
      << mcuv.ver_release << "."
      << mcuv.ver_internal
      << "\nMCU Baseline  : " << ver.GetMcuBaseline()
      << "\nMCMS Version  : "
      << mcmv.ver_major << "."
      << mcmv.ver_minor << "."
      << mcmv.ver_release << "."
      << mcmv.ver_internal
      << "\nMCMS Baseline : " << ver.GetMcmsBaseline()
  	  << "\nMCU Product Type: " << ProductTypeToString(productType)
  	  << str.str().c_str();
}

BOOL CLogFileManager::LoggerShutDown()
{
  STATE_MACHINE(eLogFileManagerState_Ready);

  int lErr;

  WORD isShutDown = 1;
  Flush(isShutDown);

  if (m_pZipCompressor->IsInit()) // if compression stream initialized
  {
    // release compression stream memory
    lErr = m_pZipCompressor->DeflateEnd();
    if (lErr != Z_OK)
      FATAL_ERROR_BOOL("CLogFileManager::LoggerShutDown : error closing compression stream",
                       lErr);
  }

  DEALLOCBUFFER(m_pFileBuffer);   // release memory buffer
  return TRUE;
}

eLogFileManagerState CLogFileManager::GetState() const
{
  return eGlobalState;
}

void CLogFileManager::SetState(eLogFileManagerState newState)
{
  eGlobalState = newState;
}

const logger_configuration_struct& CLogFileManager::GetSystemCfg() const
{
  return m_sLoggerConfiguration;
}

BOOL CLogFileManager::MessageToFile(const char* msg)
{
  STATE_MACHINE(eLogFileManagerState_Ready);

  const int msgLen = strlen(msg);
  if (0 == msgLen)
    return FALSE;

  ULONG lZippedBlockSize  = m_pZipCompressor->GetTotalOut();

  bool bLimitReached = (lZippedBlockSize + MAX_INTERNAL_COMPRESSION_BUFFER
                        >
                        m_sLoggerConfiguration.lMaxFileSize);

  if (0 == m_lFirstMessagePrefix.lDate)
  {
    GetDateTime(m_lFirstMessagePrefix.lDate,
                m_lFirstMessagePrefix.lTime,
                m_lFirstMessagePrefix.lTicks);
  }

  if (true == bLimitReached)
  {
    BOOL res = Flush();
    if (FALSE == res)
      FATAL_ERROR_BOOL("CLogFileManager::MessageToFile : flush failed", 0);

    GetDateTime(m_lFirstMessagePrefix.lDate,
                m_lFirstMessagePrefix.lTime,
                m_lFirstMessagePrefix.lTicks);
  }

  m_pZipCompressor->SetNextIn((BYTE*)msg, msgLen);

  int lErr = m_pZipCompressor->Deflate(Z_NO_FLUSH);
  if (lErr != Z_OK)
    FATAL_ERROR_BOOL("CLogFileManager::MessageToFile: deflate failed", lErr);

  GetDateTime(m_lLastMessagePrefix.lDate,
              m_lLastMessagePrefix.lTime,
              m_lLastMessagePrefix.lTicks);

  return TRUE;
}

BOOL CLogFileManager::Flush(WORD isShutDown)
{
  STATE_MACHINE(eLogFileManagerState_Ready);

  if (m_pZipCompressor->GetTotalIn() == 0)
	  return TRUE;

  m_pZipCompressor->SetNextIn(NULL, 0);

  // Compressor
  int lErr = m_pZipCompressor->Deflate(Z_FINISH);
  if (lErr != Z_STREAM_END)
    FATAL_ERROR_BOOL("CLogFileManager::Flush : compressed data buffer not flushed at limit",
                     lErr);

  if (m_pZipCompressor->GetTotalOut() > m_lFileBufferSize)
    FATAL_ERROR_BOOL("CLogFileManager::Flush : File limit crossed. MAX_INTERNAL_COMPRESSION_BUFFER is too small.",
                     0);

  // Write buffer to data file
  STATUS status = m_pCyclicFileManager->AppendToCurrentFile(
    m_pFileBuffer, m_pZipCompressor->GetTotalOut());
  if (STATUS_OK != status)
    FATAL_ERROR_BOOL("CLogFileManager::Flush : write data failed", 0);

  // date & time of first & last messages
  CStructTm firstMessageTime(m_lFirstMessagePrefix.lDate & 0xFF,    // day
                             m_lFirstMessagePrefix.lDate >> 8 & 0xFF, // month
                             m_lFirstMessagePrefix.lDate >> 16, // year
                             m_lFirstMessagePrefix.lTime >> 16, // hour
                             m_lFirstMessagePrefix.lTime >> 8 & 0xFF, // minutes
                             m_lFirstMessagePrefix.lTime & 0xFF); // seconds

  CStructTm lastMessageTime(m_lLastMessagePrefix.lDate & 0xFF,    // day
                            m_lLastMessagePrefix.lDate >> 8 & 0xFF, // month
                            m_lLastMessagePrefix.lDate >> 16, // year
                            m_lLastMessagePrefix.lTime >> 16, // hour
                            m_lLastMessagePrefix.lTime >> 8 & 0xFF, // minutes
                            m_lLastMessagePrefix.lTime & 0xFF); // seconds

  // move to next file
  status = m_pCyclicFileManager->MoveToNextFile(firstMessageTime,
                                                lastMessageTime);
  m_startUpFlag = FALSE;
  if (STATUS_OK != status)
    FATAL_ERROR_BOOL("CLogFileManager::Flush : failed to move to next file", 0);

  // Reset buffer
  lErr = m_pZipCompressor->DeflateReset(m_pFileBuffer, m_lFileBufferSize);
  if (lErr != Z_OK)
    FATAL_ERROR_BOOL("CLogFileManager::Flush : deflate reset failed", lErr);

  m_lFirstMessagePrefix.lDate = 0;

  // Write the MCU version number to logger file
  if (!isShutDown)
  {
    WriteVersionNumberToLog();
	logFileCounter++;
    SendLoggerNumber();
  }

  return TRUE;
}

void CLogFileManager::SendLoggerNumber()
{
	CSegment*  pParam = new CSegment;
    DWORD seqNumber = m_pCyclicFileManager->GetCurrentSeqNumber();
    // write the log number since last logger startup
     TRACESTR(eLevelError) << "LogFile# since Logger Startup: " << logFileCounter << ", seqNumber: " << seqNumber;
    *pParam << (DWORD)seqNumber;
    CManagerApi api(eProcessConfParty);
	STATUS status = api.SendMsg(pParam, LOGGER_CURRENT_FILE_NUMBER_REPORT);
}

STATUS CLogFileManager::RemoveFilesOnLogSizeChanged()
{
	 if (!m_pCyclicFileManager->RemoveOldFiles())
		  return STATUS_FAIL;

	 return STATUS_OK;
}

void CLogFileManager::FatalError(const char* ErrorMessage,
                                 const ULONG ErrorCode)
{
  CMedString errorStr;

  errorStr << "Logger System Critical Error: " << ErrorMessage
           << "; Error No. : " << (DWORD)ErrorCode;

  eGlobalState = eLogFileManagerState_Major;
  LoggerErrorToFile(errorStr.GetString());   // write error to file
}

void CLogFileManager::AlertEveryXFiles(int)
{}

void CLogFileManager::InformLogFileRetrieved(const std::string name)
{
  if (m_pCyclicFileManager)
    m_pCyclicFileManager->InformRetrived(name);
}

BOOL CLogFileManager::TestForFileSystemWarning()
{
  if (!m_pCyclicFileManager)
    return FALSE;

  return m_pCyclicFileManager->TestForFileSystemWarning();
}
