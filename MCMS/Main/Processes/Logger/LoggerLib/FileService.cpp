// FileService.cpp

#include "FileService.h"

#include <errno.h>
#include <sstream>

#include "Trace.h"

static void FClose(FILE*& handle)
{
  if (NULL != handle)
    fclose(handle);

  handle = NULL;
}

CFileService::CFileService() :
    m_FHandle(NULL)
{
  m_IterCurrentFile = m_CurrentDirContent.end();
}

std::string CFileService::GetCurrentFileName()
{
  PASSERT_AND_RETURN_VALUE(m_IterCurrentFile == m_CurrentDirContent.end(), "");

  return *m_IterCurrentFile;
}

bool CFileService::CreateFolder(const std::string& dirName)
{
  BOOL res = CreateDirectory(dirName.c_str());

  return TRUE == res;
}

bool CFileService::SetCurrentDir(const std::string& dirName)
{
  std::ostringstream dummy;
  m_CurrentDirectory = dirName;
  BOOL res = GetDirectoryContents(m_CurrentDirectory.c_str(),
                                  m_CurrentDirContent,
                                  dummy);

  return TRUE == res;
}

DWORD CFileService::GetNumOfFiles()
{
  return m_CurrentDirContent.size();
}

std::string CFileService::GetFirstFileName()
{
  m_IterCurrentFile = m_CurrentDirContent.begin();
  if (m_IterCurrentFile != m_CurrentDirContent.end())
    return m_CurrentDirectory + *m_IterCurrentFile;

  return "";
}

std::string CFileService::GetNextFileName()
{
  m_IterCurrentFile++;

  if (m_IterCurrentFile != m_CurrentDirContent.end())
    return m_CurrentDirectory + *m_IterCurrentFile;

  return "";
}

FILE* CFileService::GetFirstFile()
{
  FClose(m_FHandle);

  m_IterCurrentFile = m_CurrentDirContent.begin();
  if (m_IterCurrentFile != m_CurrentDirContent.end())
  {
    std::string fullName = m_CurrentDirectory + *m_IterCurrentFile;
    m_FHandle = fopen(fullName.c_str(), "r+b");
  }

  return m_FHandle;
}

FILE* CFileService::GetNextFile()
{
  FClose(m_FHandle);

  m_IterCurrentFile++;

  if (m_IterCurrentFile != m_CurrentDirContent.end())
  {
    std::string fullName = m_CurrentDirectory + *m_IterCurrentFile;
    m_FHandle = fopen(fullName.c_str(), "r+b");
  }

  return m_FHandle;
}
