// AuditFileManager.cpp

#include "AuditFileManager.h"
#include "AuditFileList.h"
#include "CyclicFileManager.h"
#include "OsFileIF.h"
#include "StatusesGeneral.h"
#include "ObjString.h"
#include "psosxml.h"
#include "AuditorApi.h"
#include "AuditorDefines.h"
#include "AuditorProcess.h"
#include "TraceStream.h"
#include "AuditEventContainer.h"
#include "LoggerDefines.h"


CAuditFileManager::CAuditFileManager()
{
  m_pCyclicFileManager = NULL;
  m_IsInit = false;
  m_IsFirstEvent = true;
  m_startUpFlag = false;
  m_SequenceNumber = 0;
}

CAuditFileManager::~CAuditFileManager()
{
  PDELETE(m_pCyclicFileManager);
}

void CAuditFileManager::Shutdown()
{
  STATUS statusMoveNext = m_pCyclicFileManager->MoveToNextFile(m_FirstMessageTime,
                                                               m_LastMessageTime);
}

void CAuditFileManager::DumpFileList(std::ostream& answer)
{
  m_pCyclicFileManager->DumpFileList(answer);
}

bool CAuditFileManager::Init()
{
  BOOL res = CreateDirectory(AUDIT_DIRECTORY);
  if (!res)
  {
    TRACEWARN << "CreateDirectory: " << AUDIT_DIRECTORY;
    return false;
  }

  CAuditorProcess* pProcess = (CAuditorProcess*)CProcessBase::GetProcess();

  m_startUpFlag = (0 == pProcess->GetNumStartup());

  FileNameHeaders_S fnh;   // the default headers are supposed to be logger
  strcpy(fnh.hdrName, AUDIT_FILE_NAME);
  strcpy(fnh.hdrExtension, AUDIT_FILE_EXT);
  strcpy(fnh.hdrXmlTag, AUDIT_EVENT_XML_TAG);

  CAuditFileList* pAuditFileList = pProcess->GetAuditFileList();
  pAuditFileList->SetFileNameHeaders(fnh);

  m_pCyclicFileManager = new CCyclicFileManager(AUDIT_DIRECTORY, fnh);
  m_pCyclicFileManager->Init(pProcess->GetAuditFileList(),
                             m_startUpFlag,
                             COMPRESSION_CODE_NONE);

  InitEventSequenceNumber();

  InsertXmlAuditListTags();

  m_IsInit = true;

  return true;
}

DWORD CAuditFileManager::ExtractBigestSequenceNumberFromFile(const CCyclicFile& currentFile)
{
  std::string fullName = AUDIT_DIRECTORY;
  fullName += currentFile.GetFileName();

  CAuditEventContainer eventContainer(0xFFFFFFFF);
  STATUS stat = eventContainer.ReadXmlFile(fullName.c_str());
  PASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
      "ReadXmlFile: " << fullName,
      0);

  return eventContainer.GetBigestSequenceNumber();
}

void CAuditFileManager::InitEventSequenceNumber()
{
  CCyclicFileList* list = m_pCyclicFileManager->GetFileList();
  if (0 == list->GetNumOfFiles())
    return;

  // last file may be empty.
  // so we should go from end to begining until we get some sequence number
  for (CYCLIC_FILE_LIST::reverse_iterator iter = list->rbegin();
       iter != list->rend();
       iter++)
  {
    size_t counter = std::distance(iter.base(), list->rbegin().base());
    if (counter > 9)
    {
      m_SequenceNumber = 0;
      TRACEWARN << "Quit without Sequence Number on iteration " << counter;
      break;
    }

    m_SequenceNumber = ExtractBigestSequenceNumberFromFile(*iter);
    if (m_SequenceNumber > 0)
    {
      TRACEINTOFUNC << "Found Biggest Sequence Number " << m_SequenceNumber
                    << " on iteration " << counter;
      break;
    }
  }
}

STATUS CAuditFileManager::Flush()
{
  PASSERTMSG_AND_RETURN_VALUE(!m_IsInit,
      "Auditor is not initialized",
      STATUS_FAIL);

  return m_pCyclicFileManager->FlushBufferFile(false,   // move to next file
                                               false,   // does next file contain startup
                                               m_FirstMessageTime,
                                               m_LastMessageTime);
}

STATUS CAuditFileManager::HandleEvent(CAuditHdrWrapper& event)
{
  if (!m_IsInit)
  {
    std::string buf;
    FormatAuditEvent(buf, event);
    PASSERTSTREAM(true, "Auditor is not initialized, event dropped: " << buf);

    return STATUS_FAIL;
  }

  if (m_IsFirstEvent)
  {
    m_IsFirstEvent = false;
    SystemGetTime(m_FirstMessageTime);
  }

  // It is current time
  SystemGetTime(m_LastMessageTime);
  event.SetEventTime(m_LastMessageTime);

  m_SequenceNumber++;
  event.SetSequenceNum(m_SequenceNumber);

  std::string buf;
  FormatAuditEvent(buf, event);

  buf += "\n\n";
  STATUS stat = AppendToCurrentFile((BYTE*)buf.c_str(), buf.length());

  COstrStream ostr;
  bool isFileComplete = IsFileComplete(m_FirstMessageTime,
                                       m_LastMessageTime,
                                       ostr);
  if (isFileComplete)
  {
    TRACEINTOFUNC << "Move to next file, " << ostr.str().c_str();
    MoveToNextFile();
  }

  return stat;
}

STATUS CAuditFileManager::MoveToNextFile()
{
  STATUS statusMoveNext = m_pCyclicFileManager->MoveToNextFile(m_FirstMessageTime,
                                                               m_LastMessageTime);

  // set current time for empty buffer file
  SystemGetTime(m_LastMessageTime);
  m_FirstMessageTime = m_LastMessageTime;

  m_startUpFlag = false;
  m_IsFirstEvent = true;

  InsertXmlAuditListTags();   // to the new and empty file

  return statusMoveNext;
}

STATUS CAuditFileManager::AppendToCurrentFile(const BYTE* data, DWORD dataLen)
{
  STATUS statusAppend = m_pCyclicFileManager->AppendToCurrentFile(data,
                                                                  dataLen,
                                                                  strlen(AUDIT_LIST_XML_TAG_END));
  InsertXmlRootEndElement();

  return statusAppend;
}

void CAuditFileManager::InsertXmlAuditListTags()
{
  InsertXmlRootBeginElement();
  InsertXmlRootEndElement();
}

bool CAuditFileManager::IsFileComplete(const CStructTm& firstEventTime,
                                       const CStructTm& lastEventTime,
                                       std::ostream& outDesc)
{
  DWORD fileSize = m_pCyclicFileManager->GetCurrentFileSize();
  if (MAX_AUDIT_FILE_SIZE <= fileSize)
  {
    outDesc << "File size : " << MAX_AUDIT_FILE_SIZE << " <= " << fileSize;
    return true;
  }

  return false;
}

void CAuditFileManager::FormatAuditEvent(string& outBuffer,
                                         const CAuditHdrWrapper& auditWrapper)
{
  CXMLDOMElement* pfatherNode = NULL;
  auditWrapper.SerializeXml(pfatherNode);

  char* szResultString = NULL;
  pfatherNode->DumpDataAsLongStringEx(&szResultString, TRUE);

  outBuffer = szResultString;

  delete [] szResultString;
  delete pfatherNode;
}

STATUS CAuditFileManager::InsertXmlRootBeginElement()
{
  STATUS statusAppend =
      m_pCyclicFileManager->AppendToCurrentFile((BYTE*)AUDIT_LIST_XML_TAG_BEGIN,
                                                strlen(AUDIT_LIST_XML_TAG_BEGIN));
  return statusAppend;
}

STATUS CAuditFileManager::InsertXmlRootEndElement()
{
  STATUS statusAppend =
      m_pCyclicFileManager->AppendToCurrentFile((BYTE*)AUDIT_LIST_XML_TAG_END,
                                                strlen(AUDIT_LIST_XML_TAG_END));
  return statusAppend;
}

void CAuditFileManager::InformAuditFileRetrieved(const std::string name)
{
  if (m_pCyclicFileManager)
    m_pCyclicFileManager->InformRetrived(name);
}

BOOL CAuditFileManager::TestForFileSystemWarning()
{
  if (!m_pCyclicFileManager)
    return FALSE;

  return m_pCyclicFileManager->TestForFileSystemWarning();
}
