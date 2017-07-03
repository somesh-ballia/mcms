// FileList.cpp

#include "FileList.h"

#include <stdio.h>
#include "Macros.h"
#include "StatusesGeneral.h"
#include "StringsLen.h"
#include "InitCommonStrings.h"
#include "OsFileIF.h"
#include "DefinesGeneral.h"
#include "ApiStatuses.h"

typedef struct dirent DIRENT;

CFileList::CFileList()
{
  for (unsigned int i = 0; i < ARRAYSIZE(m_fileName); i++)
    m_fileName[i] = NULL;

  m_numb_of_files = 0;
  m_ind_file = 0;
}

CFileList::CFileList(const CFileList& other) :
    CSerializeObject(other)
{
  for (unsigned int i = 0; i < ARRAYSIZE(m_fileName); i++)
  {
    if (other.m_fileName[i] == NULL)
      m_fileName[i] = NULL;
    else
    {
      m_fileName[i] = new char[NEW_FILE_NAME_LEN];
      strncpy(m_fileName[i], other.m_fileName[i], NEW_FILE_NAME_LEN);
    }
  }

  m_numb_of_files = other.m_numb_of_files;
  m_ind_file = other.m_ind_file;
}

CFileList::~CFileList()
{
  for (WORD i = 0; i < m_numb_of_files; i++)
    PDELETEA(m_fileName[i]);
}

void CFileList::SerializeXml(CXMLDOMElement*& pParentNode) const
{
  CXMLDOMElement* pFilesListNode = pParentNode->AddChildNode("DIRECTORY");

  for (WORD i = 0; i < m_numb_of_files; i++)
    pFilesListNode->AddChildNode("FILE_NAME", m_fileName[i]);
}

int CFileList::DeSerializeXml(CXMLDOMElement* pActionNode,
                              char* pszError,
                              const char* action)
{
  if (!pActionNode)
    return STATUS_FAIL;

  return DeSerializeXml(pActionNode, pszError);
}

int CFileList::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;
  CXMLDOMElement* pDirectoryNode, *pFileNode;
  char*           pFileName;

  GET_CHILD_NODE(pActionNode, "DIRECTORY", pDirectoryNode);

  if (pDirectoryNode)
  {
    m_numb_of_files = 0;

    GET_FIRST_CHILD_NODE(pDirectoryNode, "FILE_NAME", pFileNode);

    while (pFileNode && m_numb_of_files < ARRAYSIZE(m_fileName))
    {
      GET_VALIDATE(pFileNode, &pFileName, _0_TO_NEW_FILE_NAME_LENGTH);

      if (nStatus == STATUS_OK)
      {
        m_fileName[m_numb_of_files] = new char[NEW_FILE_NAME_LEN];
        GET_VALIDATE(pFileNode, m_fileName[m_numb_of_files], _0_TO_NEW_FILE_NAME_LENGTH);
        m_numb_of_files++;
      }

      GET_NEXT_CHILD_NODE(pDirectoryNode, "FILE_NAME", pFileNode);
    }
  }

  return STATUS_OK;
}

int CFileList::Add(const char* other)
{
  if (m_numb_of_files >= ARRAYSIZE(m_fileName))
    return STATUS_MAX_FILES_EXCEEDED;

  m_fileName[m_numb_of_files] = new char[NEW_FILE_NAME_LEN];
  strncpy(m_fileName[m_numb_of_files], other, NEW_FILE_NAME_LEN);

  m_numb_of_files++;

  return STATUS_OK;
}

int CFileList::Update(const char* other)
{
  int ind;
  ind = FindFile(other);
  if (ind == NOT_FIND || ind >= MAX_FILES_IN_LIST)
    return STATUS_FILE_NOT_EXISTS;

  PDELETEA(m_fileName[ind]);
  m_fileName[ind] = new char[NEW_FILE_NAME_LEN];
  strncpy(m_fileName[ind], other, NEW_FILE_NAME_LEN);

  return STATUS_OK;
}

int CFileList::Cancel(const char* name)
{
  int ind;
  ind = FindFile(name);
  if (ind == NOT_FIND || ind >= MAX_FILES_IN_LIST) return STATUS_FILE_NOT_EXISTS;

  PDELETEA(m_fileName[ind]);

  int i = 0;
  for (int i = 0; i < (int)m_numb_of_files; i++)
  {
    if (m_fileName[i] == NULL)
      break;
  }

  int j = 0;
  for (int j = i; j < (int)m_numb_of_files-1; j++)
  {
    m_fileName[j] = m_fileName[j+1];
  }

  m_fileName[m_numb_of_files-1] = NULL;
  m_numb_of_files--;

  return STATUS_OK;
}

int CFileList::FindFile(const char* name)
{
  for (int i = 0; i < (int)m_numb_of_files; i++)
  {
    if (m_fileName[i] != NULL)
      if (!strcmp(m_fileName[i], name))
        return i;
  }

  return NOT_FIND;
}

WORD CFileList::GetFilesNumber() const
{
  return m_numb_of_files;
}

void CFileList::SetFilesNumber(const WORD num)
{
  m_numb_of_files = num;
}

char* CFileList::GetCurrentFile(const char* name)
{
  for (WORD i = 0; i < m_numb_of_files; i++)
  {
    if (m_fileName[i] != NULL)
      if (!strcmp(m_fileName[i], name))
        return m_fileName[i];
  }

  return NULL;
}

char* CFileList::GetFirstFile()
{
  m_ind_file = 1;
  return m_fileName[0];
}

char* CFileList::GetNextFile()
{
  if (m_ind_file >= m_numb_of_files)
    return NULL;

  return m_fileName[m_ind_file++];
}

const char* CFileList::NameOf() const
{
  return "CFileList";
}

int CFileList::FillFileList(const char* Path,
                            WORD bNested,
                            BYTE bXmlFileList /*=FALSE*/,
                            std::string strDirName /*=""*/)
{
  std::string strToAdd;
  int         status = STATUS_OK;

  WORD lenPath, maxLenNewPath;

  std::vector<FDStruct>           dirFiles;
  std::vector<FDStruct>::iterator dirIterator;
  BOOL                            bRes;
  FDStruct*                       file = NULL;

  if (!IsFileExists(Path))
		  return STATUS_NOT_FOUND;
  bRes = GetDirectoryContents(Path, dirFiles);   // the path should end with '/'

  if (!bRes)
  {
    FPTRACE2(eLevelError,
             "CFileList::FillFileList: failed getting directory contents: ",
             Path);
    return STATUS_NOT_FOUND;
  }

  if (0 == dirFiles.size())
    return STATUS_OK;

  lenPath = strlen(Path);

  int File_length;
  if (bXmlFileList)
    File_length = NEW_FILE_NAME_LEN;
  else
    File_length = IVR_FILE_NAME_LEN;

  maxLenNewPath = lenPath + File_length + 4 /*spare bytes*/;

  ALLOCBUFFER(newPath, maxLenNewPath);
  memset(newPath, 0, maxLenNewPath);

  strncpy(newPath, Path, maxLenNewPath);
  newPath[maxLenNewPath - 1] = '\0';

  file = ::GetFirstFile(dirIterator, dirFiles);     // allocate struct inside, need to free
  while (NULL != file)
  {
    if ((0 != strcmp(file->name.c_str(), "")) &&
        (0 != strcmp(file->name.c_str(), ".")) &&
        (0 != strcmp(file->name.c_str(), "..")) &&
        (0 != strcmp(file->name.c_str(), ".copyarea.db")))
      // checking if it is a directory
      // (0 == bNested): only when we take the languages name

      if (!bNested || (file->type != 'D'))
      {
        status = Add(file->name.c_str());

        if (STATUS_OK != status)
        {
          PTRACE(eLevelError,
                 "CFileList::FillFileList - Add to m_FileName array failed");
          PDELETE(file);    // free and zeroing
          DEALLOCBUFFER(newPath);

          return STATUS_FAIL;   // /??
        }
      }

     // Clear the struct file before the next setting
    file->name.clear();   // string must be cleared before setting
    file->type = '0';
    PDELETE(file);    // free and zeroing

    file = ::GetNextFile(dirIterator, dirFiles);    // allocate inside, need to free
  }

  if (bNested == TRUE)
  {
    file = ::GetFirstFile(dirIterator, dirFiles);

    while (NULL != file)
    {
      if ((file->type == 'D') &&
          (0 != strcmp(file->name.c_str(), "")) &&
          (0 != strcmp(file->name.c_str(), ".")) &&
          (0 != strcmp(file->name.c_str(), "..")))
      {
        // checking if it is a directory
        // (0 == bNested): only when we take the languages name

        strToAdd = file->name.c_str();

        if (bXmlFileList)
        {
          strToAdd = strDirName + "/" + strToAdd;

          status = Add(strToAdd.c_str());

          if (STATUS_OK != status)
          {
            PTRACE(eLevelError,
                   "CFileList::FillFileList - Add to m_FileName array failed");
            PDELETE(file);    // free and zeroing
            DEALLOCBUFFER(newPath);
			
            return STATUS_FAIL;   // /??
          }
        }

        sprintf(newPath, "%s%s/", Path, file->name.c_str());
        int newPathLen = strlen(newPath);
        newPath[newPathLen] = '\0';

        FillFileList(newPath, TRUE, bXmlFileList, strToAdd);

        if (bXmlFileList == FALSE)
        {
          status = Add(strToAdd.c_str());

          if (STATUS_OK != status)
          {
            PTRACE(eLevelError,
                   "CFileList::FillFileList - Add to m_FileName array failed");
            PDELETE(file);    // free and zeroing
            DEALLOCBUFFER(newPath);
            return STATUS_FAIL;   // /??
          }
        }
      }

      // Clear the struct file before the next setting
      file->name.clear();   // string must be cleared before setting
      file->type = '0';
      PDELETE(file);    // free and zeroing

      file = ::GetNextFile(dirIterator, dirFiles);    // allocate inside, need to free
    }
  }

  DEALLOCBUFFER(newPath)
  return status;
}
