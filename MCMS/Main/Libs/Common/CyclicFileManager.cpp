// CyclicFileManager.cpp

#include "CyclicFileManager.h"

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <sstream>

#include "CyclicFileList.h"
#include "StatusesGeneral.h"
#include "OsFileIF.h"
#include "ObjString.h"

#define ERROR_FILE_NAME ".ErrorCyclicFile.txt"
#define BUFFER_FILE_NAME_PREFIX ".buffer"

#define CYCLIC_FILES_ERROR(message, details) \
  MessagetoErrorFile(this, __FILE__, __LINE__, message, details, -1)

#define CYCLIC_FILES_ERROR_INT(message, details, errnoCode) \
  MessagetoErrorFile(this, __FILE__, __LINE__, message, details, errnoCode)

static void MessagetoErrorFile(const CCyclicFileManager* cyclicMngr,
                               const char* file,
                               int line,
                               const char* message,
                               const char* details,
                               int errnoCode)
{
  CStructTm now;
  SystemGetTime(now);
  char timeBuffer[254];
  now.DumpToBuffer(timeBuffer);

  CLargeString data;
  data << timeBuffer << ", "
       << file << ":" << line << ' ' << message << ", " << details;

  if (-1 != errnoCode)
    data << ", " << strerror(errnoCode) <<" (" << errnoCode << ")";

  data << '\n';

  int dataLen = data.GetStringLength();

  string fullFileName = cyclicMngr->GetCurrentFileDir();
  fullFileName += ERROR_FILE_NAME;

  FILE* pFile = fopen(fullFileName.c_str(), "a");
  if (NULL == pFile)
  {
    cerr << "Failed to open " << fullFileName.c_str() << endl
         << data.GetString() << endl;
    return;
  }

  int    written = fwrite(data.GetString(), sizeof(BYTE), dataLen, pFile);
  STATUS status = written != dataLen ? STATUS_FAIL : STATUS_OK;
  if (STATUS_OK != status)
  {
    cerr << "Failed to write to " << fullFileName.c_str() << endl
         << data.GetString() << endl;
  }

  fclose(pFile);
}

CCyclicFileManager::CCyclicFileManager(const string& dir,
                                       const FileNameHeaders_S& fnh,
                                       AlertFunctionPointer alertFunction) :
  m_BufferFile(NULL),
  m_pFileList(NULL),
  m_FileHeaderNames(fnh),
  m_FileDir(dir),
  m_IsInit(false),
  m_alertFunction(alertFunction)
{}

CCyclicFileManager::~CCyclicFileManager()
{
  delete m_BufferFile;

  if (!m_IsInit)
    CYCLIC_FILES_ERROR("Cyclic file manager is not initiated",
                       __PRETTY_FUNCTION__);
}

void CCyclicFileManager::DumpFileList(std::ostream& answer)
{
  if (!m_IsInit)
  {
    CYCLIC_FILES_ERROR("Cyclic file manager is not initiated",
                       __PRETTY_FUNCTION__);
    return;
  }

  m_pFileList->Dump(answer);
}

STATUS CCyclicFileManager::LoadEMAReadyFiles()
{
  // Get the list of logger files available in the system
  // (under the logger files directory)
  std::ostringstream err;
  std::vector<std::string> dirContent;
  BOOL res = GetDirectoryContents(m_FileDir.c_str(), dirContent, err);
  if (!res)
  {
    CYCLIC_FILES_ERROR("GetDirectoryContents", err.str().c_str());
    return STATUS_FAIL;
  }

  std::vector<std::string>::const_iterator iter;
  for (iter = dirContent.begin(); iter != dirContent.end(); iter++)
  {
    // Does not delete a error file if exists (and don't add it to list)
    if (0 == strncmp(iter->c_str(), ERROR_FILE_NAME, strlen(ERROR_FILE_NAME)))
      continue;

    // buffer file should not be deleted.
    // this might happen at the case of wild reset.
    if (0 == strncmp(iter->c_str(),
                     BUFFER_FILE_NAME_PREFIX,
                     strlen(BUFFER_FILE_NAME_PREFIX)))
      continue;

    // Build cyclic file object to add to list
    DWORD     fileSequenceNumber = 0;
    DWORD     fileSize = 0;
    CStructTm firstMessageTime;
    CStructTm lastMessageTime;
    BOOL      containsStartup = FALSE;
    BOOL      isRetrieved = FALSE;
    BYTE      cmprFormat;
    int       nameFormatVersion = 0;
    BOOL parseRes = CCyclicFileList::ParseFileName(iter->c_str(),
                                                   m_FileHeaderNames,
                                                   fileSequenceNumber,
                                                   fileSize,
                                                   firstMessageTime,
                                                   lastMessageTime,
                                                   containsStartup,
                                                   isRetrieved,
                                                   cmprFormat,
                                                   nameFormatVersion);


    std::ostringstream buf;
    buf << m_FileDir << *iter;

    if (FALSE == parseRes)
    {
      // Delete in case of a wrong name

      DeleteFile(buf.str());
      continue;
    }


	struct stat results;
	stat((const char *) (buf.str().c_str()), &results);
	mode_t requiredPermission =S_IRUSR | S_IWUSR | S_IRGRP;

	if ((results.st_mode & requiredPermission) != requiredPermission )
	{
		if (chmod(buf.str().c_str(), results.st_mode | requiredPermission) != 0)
		{
			DeleteFile(buf.str());
			continue;
		}
	}

    CCyclicFile file;
    file.SetParams(iter->c_str(),
                   fileSequenceNumber,
                   fileSize,
                   firstMessageTime,
                   lastMessageTime,
                   containsStartup,
                   isRetrieved,
                   cmprFormat,
                   nameFormatVersion,
                   m_FileHeaderNames);

    // Add the file object to list
    m_pFileList->AddFile(file);
  }

  // Sort the file list
  if (m_pFileList->GetNumOfFiles() > 0)
    m_pFileList->SortList();

  return STATUS_OK;
}

// CCyclicFileManager::Init 1) loads "ready for EMA" files.
// 2) init buffer object
// 2) converts buffer to "EMA" file
STATUS CCyclicFileManager::Init(CCyclicFileList* fileList,
                                BOOL isStartup,
                                const BYTE& cmprFormat)
{
  m_pFileList = fileList;
  m_pFileList->SetFileHeaders(m_FileHeaderNames);


  // 1) loads "ready for EMA" files.
  STATUS status = LoadEMAReadyFiles();
  if (STATUS_OK != status)
  {
    CYCLIC_FILES_ERROR_INT("LoadEMAReadyFiles", strerror(errno), errno);
    return STATUS_FAIL;
  }

  // 1.1) remove old files
  if (!RemoveOldFiles())
	  return STATUS_FAIL;

  // 2) init buffer object
  InitBufferFilenameHeaders(m_BufferFileHeaderNames);
  m_BufferFile = new CCyclicFile;

  std::string bufferFileName;
  if (IsBufferFileExist(bufferFileName))   // buffer file exist at startup - wild reset
  {
    InitBufferFileExist(bufferFileName);

    CStructTm now;
    SystemGetTime(now);
    status = FlushBufferFile(true,
                             isStartup,
                             m_BufferFile->GetFirstMessageTime(),
                             now);
    if (STATUS_OK != status)
    {
      CYCLIC_FILES_ERROR_INT("FlushBufferFile", strerror(errno), errno);
      return STATUS_FAIL;
    }
  }
  else
  {
    IncSeqNumInitBufferFile(isStartup, cmprFormat);
  }

  m_IsInit = true;

  return STATUS_OK;
}

BOOL CCyclicFileManager::RemoveOldFiles()
{
	BOOL deleteRes = m_pFileList->DeleteFilesIfNeeded();
	if (!deleteRes)
		CYCLIC_FILES_ERROR_INT("DeleteFilesIfNeeded", strerror(errno), errno);

	return deleteRes;


}

CCyclicFile& CCyclicFileManager::GetCurrentFile()
{
  CCyclicFile& currentFile = m_pFileList->Back();
  return currentFile;
}

STATUS CCyclicFileManager::AppendToCurrentFile(const BYTE* data,
                                               DWORD dataLen,
                                               DWORD offset)
{
  if (!m_IsInit)
  {
    CYCLIC_FILES_ERROR("Cyclic file manager is not initiated",
                       __PRETTY_FUNCTION__);
    return STATUS_FAIL;
  }

  int errnoCode = -1;

  string fullFileName = m_FileDir;
  fullFileName += m_BufferFile->GetFileName();

  // first try open with r+, otherwise when open with 'a' or 'w' the fseek doesn't work
  FILE* pFile = fopen(fullFileName.c_str(), "r+");
  if (NULL == pFile)
  {
    errnoCode = errno;
    if (ENOENT == errnoCode)    // means file doesn't exist
    {
      // going to create file
      // VNGR-17275: PSO - GEN002700 - System audit logs are more permissive than 640
      // set mask to write for group and read/write for other (octal 026)
      mode_t old = umask(S_IWGRP | S_IROTH | S_IWOTH);

      pFile = fopen(fullFileName.c_str(), "w");

      // return the mask back
      umask(old);

      if (NULL == pFile)
      {
        errnoCode = errno;
        CYCLIC_FILES_ERROR_INT("open file (w)", fullFileName.c_str(), errnoCode);
        return STATUS_FAIL;
      }
    }
    else
    {
      CYCLIC_FILES_ERROR_INT("open file (r+)", fullFileName.c_str(), errnoCode);
      return STATUS_FAIL;
    }
  }

  long fseekRet = fseek(pFile, -offset, SEEK_END);
  if (-1L == fseekRet)
  {
    errnoCode = errno;
    CYCLIC_FILES_ERROR_INT("fseek", fullFileName.c_str(), errnoCode);
    return STATUS_FAIL;
  }

  int written = fwrite(data, sizeof(BYTE), dataLen, pFile);

  STATUS status = STATUS_OK;
  if ((DWORD)written != dataLen)
  {
    errnoCode = errno;
    status = STATUS_FAIL;
    CYCLIC_FILES_ERROR_INT("fwrite", fullFileName.c_str(), errnoCode);
  }

  fclose(pFile);
  return status;
}

// FlushBufferFile 1) rm EMA ready file with the same sequence number
// 2) create file name and update/add to EMA file list
// 3) copy file: buffer -> EMA ready file
STATUS CCyclicFileManager::FlushBufferFile(bool isMoveNextFile,
                                           bool isNextFileStartup,
                                           const CStructTm& firstMessageTime,
                                           const CStructTm& lastMessageTime)
{
  // 1) rm EMA ready file with the same sequence number
  //
  // after this block we have new file object in list and no file
  // the file object in the list has to be updated.

  DWORD        seqNum = m_BufferFile->GetSequenceNumber();
  CCyclicFile* pCurrentFile = m_pFileList->GetFileBySeqNum(seqNum);
  if (NULL != pCurrentFile)
  {
    // remove file
    string oldFullFileName = m_FileDir;
    oldFullFileName += pCurrentFile->GetFileName();

    DeleteFile(oldFullFileName);
  }
  else
  {
    m_pFileList->AddFile(*m_BufferFile);
    pCurrentFile = &m_pFileList->Back();
    m_pFileList->SetLastFileSequenceNumber(seqNum);
  }

  // 2) generate the file name with updated values
  string bufferFullFileName = m_FileDir;
  bufferFullFileName += m_BufferFile->GetFileName();

  DWORD fileSize = GetFileSize(bufferFullFileName);
  BOOL  isContainStartup = m_BufferFile->IsContainStartup();
  BOOL  isRetrieved = m_BufferFile->IsRetrieved();

  const BYTE cmprFormat = m_BufferFile->GetCmprFormat();
  BYTE       nameFormatVersion = m_BufferFile->GetNameFormatVersion();

  char newFileName[CYCLIC_FILE_MAX_NAME_LEN];
  CCyclicFileList::CreateFileName(newFileName,
                                  m_FileHeaderNames,
                                  seqNum,
                                  fileSize,
                                  firstMessageTime,
                                  lastMessageTime,
                                  isContainStartup,
                                  isRetrieved,
                                  cmprFormat);

  // 4) update changed fields in file list
  pCurrentFile->SetParams(newFileName,
                          seqNum,
                          fileSize,
                          firstMessageTime,
                          lastMessageTime,
                          isContainStartup,
                          isRetrieved,
                          cmprFormat,
                          nameFormatVersion,
                          m_FileHeaderNames);

  string newFullFileName = m_FileDir;
  newFullFileName += pCurrentFile->GetFileName();

  // 5) copy/rename buffer -> EMA ready file
  if (isMoveNextFile)  // a new buffer file should be initiated
  {
    BOOL renameRes = RenameFile(bufferFullFileName, newFullFileName);
    if (FALSE == renameRes)
    {
      string message = bufferFullFileName;
      message += " -> ";
      message += newFullFileName;
      CYCLIC_FILES_ERROR_INT("Rename file: ", message.c_str(), errno);
      return STATUS_FAIL;
    }

    IncSeqNumInitBufferFile(isNextFileStartup, cmprFormat);
  }
  else   // the same buffer will be used for incoming events
  {
    BOOL copyRes = CopyFile(bufferFullFileName, newFullFileName);
    if (FALSE == copyRes)
    {
      string message = bufferFullFileName;
      message += " -> ";
      message += newFullFileName;
      CYCLIC_FILES_ERROR("Copy file: ", message.c_str());
      return STATUS_FAIL;
    }
  }

  return STATUS_OK;
}

void CCyclicFileManager::InitBufferFilenameHeaders(FileNameHeaders_S& bufferHdr)
{
  strcpy(bufferHdr.hdrName, BUFFER_FILE_NAME_PREFIX);
  bufferHdr.hdrName[FILE_NAME_PATTERN_LEN - 1] = '\0';

  strcpy(bufferHdr.hdrExtension, "tmp");
  bufferHdr.hdrExtension[FILE_NAME_PATTERN_LEN - 1] = '\0';

  strcpy(bufferHdr.hdrXmlTag, "CUCU_LULU");   // should not be used.
}

void CCyclicFileManager::IncSeqNumInitBufferFile(BOOL isContainStartup,
                                                 const BYTE& cmprFormat)
{
  BOOL isRetrieved = FALSE;
  // generate sequence number
  DWORD lastFileSequenceNumber = m_pFileList->GetLastFileSequenceNumber();
  DWORD nextFileSequenceNumber = lastFileSequenceNumber + 1;
  if (MAX_FILE_SEQUENCE_NUM < nextFileSequenceNumber)
    nextFileSequenceNumber = 1;

  if (m_alertFunction != NULL)
    m_alertFunction(nextFileSequenceNumber);

  m_pFileList->SetLastFileSequenceNumber(nextFileSequenceNumber);
  m_BufferFile->SetSequenceNumber(nextFileSequenceNumber);

  CStructTm now;
  SystemGetTime(now);

  char fileName[CYCLIC_FILE_MAX_NAME_LEN];
  CCyclicFileList::CreateFileName(fileName,
                                  m_BufferFileHeaderNames,
                                  nextFileSequenceNumber,
                                  0,
                                  now,
                                  now,
                                  isContainStartup,
                                  isRetrieved,
                                  cmprFormat);

  m_BufferFile->SetParams(fileName,
                          nextFileSequenceNumber,
                          0,
                          now,
                          now,
                          isContainStartup,
                          isRetrieved,
                          cmprFormat,
                          NAME_FORMAT_VERSION,
                          m_BufferFileHeaderNames);
}

void CCyclicFileManager::InitBufferFileExist(const string& bufferFileName)
{
  // Build cyclic file object to add to list
  DWORD     fileSequenceNumber = 0;
  DWORD     fileSize = 0;
  CStructTm firstMessageTime;
  CStructTm lastMessageTime;
  BOOL      containsStartup = FALSE;
  BOOL      isRetrieved = FALSE;

  BYTE cmprFormat;
  int  nameFormatVersion = 0;

  BOOL parseRes = CCyclicFileList::ParseFileName(bufferFileName.c_str(),
                                                 m_BufferFileHeaderNames,
                                                 fileSequenceNumber,
                                                 fileSize,
                                                 firstMessageTime,
                                                 lastMessageTime,
                                                 containsStartup,
                                                 isRetrieved,
                                                 cmprFormat,
                                                 nameFormatVersion);
  if (FALSE == parseRes)
  {
    // Delete in case of a wrong name
    string fullName = m_FileDir;
    fullName += bufferFileName.c_str();
    DeleteFile(fullName);
    CYCLIC_FILES_ERROR("Failed to parse file, so it was deleted",
                       bufferFileName.c_str());

    return;
  }

  m_BufferFile->SetParams(bufferFileName.c_str(),
                          fileSequenceNumber,
                          0,
                          firstMessageTime,
                          lastMessageTime,
                          containsStartup,
                          isRetrieved,
                          cmprFormat,
                          nameFormatVersion,
                          m_BufferFileHeaderNames);
}

bool CCyclicFileManager::IsBufferFileExist(std::string& out)
{
  std::ostringstream err;
  std::vector<std::string> files;
  BOOL res = GetDirectoryContents(m_FileDir.c_str(), files, err);
  if (!res)
  {
    CYCLIC_FILES_ERROR("GetDirectoryContents", err.str().c_str());
    return false;
  }

  std::vector<std::string>::const_iterator iter;
  for (iter = files.begin(); iter != files.end(); iter++)
  {
    size_t len = strlen(BUFFER_FILE_NAME_PREFIX);
    if (0 == strncmp(iter->c_str(), BUFFER_FILE_NAME_PREFIX, len))
    {
      out = *iter;
      return true;
    }
  }

  return false;
}

STATUS CCyclicFileManager::MoveToNextFile(const CStructTm& firstMessageTime,
                                          const CStructTm& lastMessageTime)
{
  if (!m_IsInit)
  {
    CYCLIC_FILES_ERROR("Cyclic file manager is not initiated",
                       __PRETTY_FUNCTION__);
    return STATUS_FAIL;
  }

  STATUS status = FlushBufferFile(true, false, firstMessageTime,
                                  lastMessageTime);
  if (STATUS_OK != status)
  {
    CYCLIC_FILES_ERROR("Flush buffer failed", "");
    return STATUS_FAIL;
  }

  BOOL deleteRes = m_pFileList->DeleteFilesIfNeeded();
  if (!deleteRes)
  {
    CYCLIC_FILES_ERROR("Delete files failed", "");
    return STATUS_FAIL;
  }

  return STATUS_OK;
}

DWORD CCyclicFileManager::GetCurrentFileSize() const
{
  if (!m_IsInit)
  {
    CYCLIC_FILES_ERROR("Cyclic file manager is not initiated",
                       __PRETTY_FUNCTION__);
    return 0;
  }

  string fullName = m_FileDir;
  fullName += m_BufferFile->GetFileName();
  int size = GetFileSize(fullName);
  if (-1 == size)
  {
    CYCLIC_FILES_ERROR("Failed to get file size ", fullName.c_str());
    return 0;
  }

  return size;
}

DWORD CCyclicFileManager::GetCurrentSeqNumber() const
{
	if (m_BufferFile)
		return m_BufferFile->GetSequenceNumber();
	return 0;
}

void CCyclicFileManager::InformRetrived(const std::string& name)
{
  char temp_oldFileName[CYCLIC_FILE_MAX_NAME_LEN+101];
  strncpy(temp_oldFileName, name.c_str(), CYCLIC_FILE_MAX_NAME_LEN+100);
  const char* oldFileName = basename(temp_oldFileName);

  char folder[CYCLIC_FILE_MAX_NAME_LEN+101];
  strncpy(folder, name.c_str(), CYCLIC_FILE_MAX_NAME_LEN+100);
  folder[CYCLIC_FILE_MAX_NAME_LEN + 100] = '\0';
  dirname(folder);


  DWORD     fileSequenceNumber = 0;
  DWORD     fileSize = 0;
  CStructTm firstMessageTime;
  CStructTm lastMessageTime;
  BOOL      containsStartup = FALSE;
  BOOL      isRetrieved = FALSE;
  BYTE      cmprFormat;
  int       nameFormatVersion = 0;

  BOOL parseRes = CCyclicFileList::ParseFileName(name.c_str(),
                                                 m_FileHeaderNames,
                                                 fileSequenceNumber,
                                                 fileSize,
                                                 firstMessageTime,
                                                 lastMessageTime,
                                                 containsStartup,
                                                 isRetrieved,
                                                 cmprFormat,
                                                 nameFormatVersion);

  if (parseRes && isRetrieved == FALSE)
  {
    char newFileName[CYCLIC_FILE_MAX_NAME_LEN];
    CCyclicFileList::CreateFileName(newFileName,
                                    m_FileHeaderNames,
                                    fileSequenceNumber,
                                    fileSize,
                                    firstMessageTime,
                                    lastMessageTime,
                                    containsStartup,
                                    TRUE,
                                    cmprFormat);

    std::string oldname = folder;
    oldname += "/";
    oldname += oldFileName;
    std::string newname = folder;
    newname += "/";
    newname += newFileName;

    PASSERT(rename(oldname.c_str(), newname.c_str()));

    if (m_pFileList)
      m_pFileList->UpdateRetrieved(fileSequenceNumber, newFileName);
  }
}

BOOL CCyclicFileManager::TestForFileSystemWarning()
{
  if (m_pFileList)
    return m_pFileList->TestForFileSystemWarning();

  return FALSE;
}
