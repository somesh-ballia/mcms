// LoggerFileList.cpp

#include "LoggerFileList.h"

#include <iomanip>

#include "psosxml.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "logdefs.h"
#include "OsFileIF.h"
#include "StatusesGeneral.h"
#include "ProcessBase.h"
#include "Log4cxxConfiguration.h"
#include "LoggerProcess.h"

class CStructTm;

CLoggerFileList::CLoggerFileList() :
  CCyclicFileList(LOGGER_DIR)
{}

void CLoggerFileList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
  CXMLDOMElement* pLoggerFileListNode =
      pFatherNode->AddChildNode("LOG_FILE_SUMMARY_LS");

  CCyclicFileList::SerializeXml(pLoggerFileListNode);
}

void CLoggerFileList::SerializeXmlPartialList(CXMLDOMElement*& pFatherNode,int nLastItems) const
{
  CXMLDOMElement* pLoggerFileListNode =
      pFatherNode->AddChildNode("LOG_FILE_SUMMARY_LS");

  CCyclicFileList::SerializeXmlPartialList(pLoggerFileListNode,nLastItems);
}

BOOL CLoggerFileList::DeleteFilesIfNeeded()
{
  BOOL res = TRUE;

  int max_files = ((CLoggerProcess*)CProcessBase::GetProcess())->GetMaxNumberOfFiles();

  while (GetNumOfFiles() > max_files)
    res = res && (DeleteOldestLoggerFile());

  return res;
}


BOOL CLoggerFileList::DeleteOldestLoggerFile()
{
  // Get the oldest file name
  char fullFileName[LOGGER_FILE_MAX_NAME_LEN + strlen(LOGGER_DIR)];
  sprintf(fullFileName, "%s%s", LOGGER_DIR, Front().GetFileName());

  // Remove file entry from list
  PopFront();

  // Delete file from disk
  BOOL res = DeleteFile(fullFileName, FALSE);

  return res;
}

BOOL CLoggerFileList::TestForFileSystemWarning()
{
  int skip = 0;
  CYCLIC_FILE_LIST::iterator iTer = m_fileList.end();

  do
  {
    skip++;
    iTer--;
    if (skip > ((CLoggerProcess*)CProcessBase::GetProcess())->GetMaxNumberOfFiles() / 2)
    // the newest log files (half of max size of the list)
    // will never make such alert
    {

      if (iTer->IsRetrieved() == FALSE)
      {
        return TRUE;
        // one of the log files (in the older half)
        // need to be backuped
      }

    }
  } while (iTer != m_fileList.begin());

  return FALSE;
}
