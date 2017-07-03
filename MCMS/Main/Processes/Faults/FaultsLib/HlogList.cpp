// HlogList.cpp

#include "HlogList.h"

#include <algorithm>

#include "StringsLen.h"
#include "OsFileIF.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "TraceStream.h"
#include "FaultsDefines.h"
#include "FaultsProcess.h"
#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"
#include "SysConfig.h"
#include <iostream>
#include <fstream>
#include <iomanip>

#define FILE_PREFIX_LEN 32

template <class ClassT>
struct less_ptr : public binary_function<ClassT, ClassT, bool>
{
  bool operator()(const ClassT x, const ClassT y)
  {
    return x->GetFaultId() < y->GetFaultId();
  }
};

CHlogList::CHlogList(const char* pszFileName)
{
  TRACEINTOFUNC << ": Enter";
  CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
  m_maxFaultsInList = 1000;
  if (pSysConfig)
    pSysConfig->GetDWORDDataByKey("MAX_FAULTS_IN_LIST", m_maxFaultsInList);

  WORD nameLen = (strlen(pszFileName) < NEW_FILE_NAME_LEN) ?
      strlen(pszFileName)+1 : NEW_FILE_NAME_LEN+1;

  m_pszFileName = new char[nameLen];
  strncpy(m_pszFileName, pszFileName, nameLen-1);
  m_pszFileName[nameLen-1] = '\0';
  m_isFileOk            = YES;
  m_isFileErrorAlreadyReported  = NO;
  m_FileNum = 0;
  m_pVector = new CLogFaultVector;

  CFaultsProcess* process =
    dynamic_cast<CFaultsProcess*>(CProcessBase::GetProcess());
  m_FaultId = 1;

  if (YES == process->GetIsHardDiskOk())
  {
    WORD numOfFiles = GetNumOfFiles(pszFileName);
    for (WORD i = 1; i <= numOfFiles; i++)
    {
      char FaultNameString[FILE_PREFIX_LEN];
      PrepareFileName(FaultNameString, i);


   /*   char* buffer = ReadFaultFileIntoBuffer(FaultNameString);
      std::string validXML = RemoveInvalidXMLCharacters(buffer);
      if (buffer!=NULL && strlen(buffer) > validXML.length() ) // there are invalid characaters in FAULTS - remove them
      {
    	  //cout << "file damaged: " << FaultNameString <<endl;
    	  WriteFaultFile(FaultNameString,validXML);
      }

      POBJDELETE(buffer);*/

      STATUS stat = CSerializeObject::ReadXml_Files(FaultNameString,
                                                    eNoActiveAlarm,
                                                    eRenameFile,
                                                    i);

      if ((STATUS_OK != stat) && (STATUS_FILE_NOT_EXIST != stat))
      {
        m_isFileOk            = NO;
        m_isFileErrorAlreadyReported  = YES; // the error is reported within ReadXmlFile() method
      }
    }
  }

  WORD FileNum = 0;
  m_FaultId = 0;
  WORD BiggestF_ID;
  m_FaultId = GetBiggestFaultIdAndUpdateFileNum(&FileNum);

  m_FaultId++;
  SortVectorByFaultId();

  TRACEINTOFUNC << ": Leave";
}

CHlogList::~CHlogList()
{
  ClearVector();

  PDELETE(m_pVector);
  PDELETEA(m_pszFileName);
}

void CHlogList::PrepareFileName(char* name, int num)
{
  int len = strlen(m_pszFileName);
  strncpy(name, m_pszFileName, len);
  sprintf(name + len, "%04d.xml", num);
}

int CHlogList:: GetNumOfFiles(const char* pathName)
{
  char filePrefix[FILE_PREFIX_LEN];
  memset(filePrefix, '\0', FILE_PREFIX_LEN);
  GetFilePrefix(pathName, filePrefix);
  WORD numOfFiles = GetDirFilePrefixNum("Faults/", filePrefix);

  return numOfFiles;
}

void CHlogList::GetFilePrefix(const char* dirName, char* baseName)
{
  char* base = (char*)strrchr(dirName, '/');
  ++base;
  strncpy(baseName, base, FILE_PREFIX_LEN - 1);
}

void CHlogList::ClearVector()
{
  while (!m_pVector->empty())
    ClearFirstElemFromVector();

  m_pVector->clear();
}

void CHlogList::ClearFirstElemFromVector()
{
  CLogFaultVector::iterator itr = m_pVector->begin();
  if (itr == m_pVector->end())
    return;

  CHlogElement* pElem = *itr;
  m_pVector->erase(itr);
  POBJDELETE(pElem);
}

STATUS CHlogList::AddElement(CHlogElement* pHlogElement)
{
  STATUS retStat = STATUS_OK;

  if (pHlogElement == NULL)
    return STATUS_FAIL;

  if (!CPObject::IsValidPObjectPtr(pHlogElement))
    return STATUS_FAIL;

  CHlogElement* pNewElement = dynamic_cast<CHlogElement*>(pHlogElement->Clone());
  if (m_pVector->size() > m_maxFaultsInList-1)
    ClearFirstElemFromVector();

  DWORD faultId = GetIncFaultId();
  pNewElement->SetFaultId(faultId);

  m_pVector->push_back(pNewElement);
  pNewElement->IncreaseReferenceCounter();
  IncreaseUpdateCounter();

  char FaultNameString[28];
  int  nameNumber = faultId % m_maxFaultsInList;
  if (0 == nameNumber)
    nameNumber = m_maxFaultsInList;

  PrepareFileName(FaultNameString, nameNumber);

  CFaultsProcess* process =
    dynamic_cast<CFaultsProcess*>(CProcessBase::GetProcess());

  if ((YES == process->GetIsHardDiskOk()) && (YES == m_isFileOk))
  {
    CSerializeObject::WriteXml_File(FaultNameString, "HLOG");
    if (STATUS_OK != retStat)
      TreatWriteXmlFileError();
  }

  return retStat;
}

void CHlogList::TreatWriteXmlFileError()
{
  m_isFileOk = NO;

  if (NO == m_isFileErrorAlreadyReported)
  {
    PTRACE2(eLevelError,
            "CHlogList::TreatWriteXmlFileError: failed to write data, file = ",
            m_pszFileName);
    m_isFileErrorAlreadyReported = YES;
  }
}

void CHlogList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
  SerializeXml(pFatherNode, 0, 0);
}

void CHlogList::Serialize_Xml(CXMLDOMElement*& pFatherNode) const
{
  SerializeXml(pFatherNode, 0, 0);
}

void CHlogList::SerializeXml(CXMLDOMElement*& pFatherNode,
                             DWORD objToken,
                             const DWORD idStart) const
{
  CXMLDOMElement* pListNode = pFatherNode;

  DWORD size = m_pVector->size();
  pListNode->AddChildNode("ID", size);
  WORD bChanged = InsertUpdateCntChanged(pListNode, objToken);

  if (TRUE == bChanged)
  {
    DWORD startIndex = (size > idStart ? idStart : 0);
    DWORD i = size - 1;
    m_pVector->at(i)->SerializeXml(pListNode);
  }
}

void CHlogList::SerializeXmlAllVector(CXMLDOMElement*& pFatherNode,
                                      DWORD objToken,
                                      const DWORD idStart) const
{
  CXMLDOMElement* pListNode = pFatherNode->AddChildNode("FAULTS_LIST");

  DWORD size = m_pVector->size();
  pListNode->AddChildNode("ID", size);
  WORD bChanged = InsertUpdateCntChanged(pListNode, objToken);
  if (TRUE == bChanged)
  {
    DWORD startIndex = 0;
    DWORD temp_t = 0;
    objToken = 0;
    for (DWORD i = startIndex; (i < size); i++)
    {
      if ((m_pVector->at(i)->GetFaultId()) > objToken)
        m_pVector->at(i)->SerializeXml(pListNode);
    }
  }
}

int CHlogList::DeSerializeXml(CXMLDOMElement* pActionNode,
                              char* pszError,
                              const char* action)
{
  int             nStatus = STATUS_OK;
  CXMLDOMElement* pHlogNode = NULL;
  GET_CHILD_NODE(pActionNode, "LOG_FAULT_ELEMENT", pHlogNode);
  if (pHlogNode)
  {
    DWORD         dummy;
    char*         pszChildName = NULL;
    CHlogElement* pHlogElement = NULL;
    pHlogElement = new CLogFltElement;
    nStatus = pHlogElement->DeSerializeXml(pHlogNode, pszError, action);
    if (nStatus)
    {
      POBJDELETE(pHlogElement);
    }
    else
    {
      m_pVector->push_back(pHlogElement);
      pHlogElement->IncreaseReferenceCounter();
      if (m_pVector->size() > m_maxFaultsInList)
        ClearFirstElemFromVector();
    }
  }

  return STATUS_OK;
}

int CHlogList::DeSerialize_Xml(CXMLDOMElement* pActionNode,
                               char* pszError,
                               const char* action,
                               int FileNum)
{
  int             nStatus = STATUS_OK;
  CXMLDOMElement* pHlogNode = NULL;
  GET_CHILD_NODE(pActionNode, "LOG_FAULT_ELEMENT", pHlogNode);
  if (pHlogNode)
  {
    DWORD         dummy;
    char*         pszChildName = NULL;
    CHlogElement* pHlogElement = NULL;
    pHlogElement = new CLogFltElement;
    nStatus = pHlogElement->DeSerializeXml(pHlogNode, pszError, action);

    pHlogElement->SetFileNum(FileNum);
    DWORD currentFaultId = pHlogElement->GetFaultId();
    int   ModeFileId = ((int) currentFaultId % m_maxFaultsInList);

    if (nStatus)
    {
      POBJDELETE(pHlogElement);
    }
    else
    {
      m_pVector->push_back(pHlogElement);
      pHlogElement->IncreaseReferenceCounter();
      if (m_pVector->size() > m_maxFaultsInList)
        ClearFirstElemFromVector();
    }
  }

  return nStatus;
}


void CHlogList::DumpAllVectorAsString(std::ostream &ostr ) const
{

	ostr  << std::left << std::setw(6) << std::setfill(' ') << "MCIID";
	ostr  << std::left << std::setw(22) << std::setfill(' ') << "GMT Time";
	ostr  << std::left << std::setw(16) << std::setfill(' ') << "Category";
	ostr  << std::left << std::setw(16) << std::setfill(' ') << "Level";
	ostr  << std::left << std::setw(16) << std::setfill(' ') << "Code";
	ostr  << std::left << std::setw(22) << std::setfill(' ') << "Process Name";
	ostr  << std::left << std::setw(32) << std::setfill(' ') << "Description";
	ostr  << "\r\n";


	for (DWORD i = 0; i < m_pVector->size(); ++i)
	{
		if (m_pVector->at(i) != NULL)
		{
			m_pVector->at(i)->DumpAsString(ostr);
		}
		else
		{
			TRACEINTOLVLERR << "Fault " << i << " is NULL \n";
		}
	}
}

DWORD CHlogList::GetIncFaultId()
{
  return m_FaultId++;
}

DWORD CHlogList::GetBiggestFaultId() const
{
  DWORD maxId = 0;

  CLogFaultVector::iterator iTer =  m_pVector->begin();
  CLogFaultVector::iterator iEnd =  m_pVector->end();
  while (iTer != iEnd)
  {
    const CHlogElement* logElement = *iTer;
    DWORD               currentFaultId = logElement->GetFaultId();
    DWORD               currentFileNum = logElement->GetFileNum();
    if (maxId < currentFaultId)
      maxId = currentFaultId;

    iTer++;
  }

  return maxId;
}

DWORD CHlogList::GetBiggestFaultIdAndUpdateFileNum(WORD* FileNum)
{
  DWORD                     maxId = 0;
  WORD                      Temp_file_num = 0;
  CLogFaultVector::iterator iTer =  m_pVector->begin();
  CLogFaultVector::iterator iEnd =  m_pVector->end();
  while (iTer != iEnd)
  {
    const CHlogElement* logElement = *iTer;
    DWORD               currentFaultId = logElement->GetFaultId();
    DWORD               currentFileNum = logElement->GetFileNum();
    if (maxId < currentFaultId)
    {
      maxId = currentFaultId;
      Temp_file_num = currentFileNum;
    }

    iTer++;
  }

  *FileNum = Temp_file_num;
  return maxId;
}

void CHlogList::SortVectorByFaultId()
{
  CLogFaultVector::iterator iTer =  m_pVector->begin();
  CLogFaultVector::iterator iEnd =  m_pVector->end();
  less_ptr<CHlogElement*>   sorter;
  sort(m_pVector->begin(), m_pVector->end(), sorter);
}

bool CHlogList::greater(CHlogElement elem1, CHlogElement elem2)
{
  return (elem1.m_hlog.faultId > elem2.m_hlog.faultId);
}

void CHlogList::SetIsFileOk(BOOL isOk)
{
  m_isFileOk = isOk;
}

BOOL CHlogList::GetIsFileOk()
{
  return m_isFileOk;
}

std::string CHlogList::RemoveInvalidXMLCharacters(char* buffer)
{
	std::string strXMLValid;
	int codePoint;
	BYTE ch;
	unsigned int  i = 0;
	while (buffer!=NULL && i < strlen(buffer))
	{
		ch = buffer[i];
	    // remove any characters outside the valid UTF-8 range as well as all control characters         // except tabs and new lines
        if ( !( (ch > 31 && ch < 253 ) || ch == '\t' || ch == '\n' || ch == '\r') )
		{   //continoue
		}
        else
        {
        	strXMLValid += ch;
        	//cout << ch ;
        }
		i = i+1;
	}
	return strXMLValid;
}

char* CHlogList::ReadFaultFileIntoBuffer(char* FaultNameString)
{
    std::ifstream t;
    std::ofstream out;
    int length;
    t.open(FaultNameString);  // open input file
    t.seekg(0, std::ios::end); 	// go to the end
    length = t.tellg();        // report location (this is the length)
    t.seekg(0, std::ios::beg);    // go back to the beginning
    char* buffer = new char[length+1];//allocate memory for a buffer of appropriate dimension
    t.read(buffer, length);       // read the whole file into the buffer t.close();                    // close file handle
    t.close();
    buffer[length]='\0';
    return buffer;
}

void CHlogList::WriteFaultFile(char* FaultNameString,std::string buffer)
{
    std::ofstream out;
    int length;
    out.open(FaultNameString);  // open input file
    length = buffer.length();        // report location (this is the length)
    out.write((const char*)buffer.c_str(), length);       // read the whole file into the buffer t.close();                    // close file handle
    out.close();
}

