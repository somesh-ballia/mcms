// // CyclicFileList.h

#ifndef CYCLIC_FILE_LIST_H_
#define CYCLIC_FILE_LIST_H_

#include <list>
#include <vector>

#include "SerializeObject.h"
#include "CyclicFilesDefines.h"
#include "DataTypes.h"
#include "StructTm.h"
#include "Macros.h"

class CCyclicFile;

struct DeleteFileReport_S
{
  std::string fileName;
  BOOL deleteResult;
};

typedef std::vector<DeleteFileReport_S> DELETE_FILE_REPORT_VECTOR;
typedef std::list<CCyclicFile> CYCLIC_FILE_LIST;

class CCyclicFile : public CSerializeObject
{
  CLASS_TYPE_1(CCyclicFile, CSerializeObject)

public:
  CCyclicFile();
  friend bool               operator<(const CCyclicFile& lhs,
                                      const CCyclicFile& rhs);

  virtual void              SerializeXml(CXMLDOMElement*& pFatherNode) const;
  virtual int               DeSerializeXml(CXMLDOMElement* pActionNode,
                                           char* pszError,
                                           const char* action);
  virtual CSerializeObject* Clone();
  virtual const char*       NameOf() const     { return "CCyclicFile"; }
  const char*               GetFileName() const;
  void                      SetFileName(const char* fileName);
  void                      SetSize(DWORD size){m_fileSize = size;}
  DWORD                     GetSize() const    {return m_fileSize;}
  DWORD                     GetSequenceNumber() const;
  void                      SetSequenceNumber(DWORD seqNum);
  BOOL                      IsContainStartup() const;
  BOOL                      IsRetrieved() const;
  void                      SetRetrieved();
  BYTE                GetCmprFormat() const;
  BYTE                      GetNameFormatVersion() const {return m_nameFormatVersion;}
  const CStructTm&          GetFirstMessageTime() const {return m_firstMessageTime;}
  void                      SetFirstMessageTime(const CStructTm& time) {m_firstMessageTime = time;}
  const CStructTm&          GetLastMessageTime() const {return m_lastMessageTime;}
  void                      SetLastMessageTime(const CStructTm& time) {m_lastMessageTime = time;}
  void                      SetParams(const string& fileName,
                                      DWORD fileSequenceNumber,
                                      DWORD fileSize,
                                      const CStructTm& firstMessageTime,
                                      const CStructTm& lastMessageTime,
                                      BOOL containsStartup,
                                      BOOL isRetrieved,
                                      const BYTE cmprFormat,
                                      int nameFormatVersion,
                                      const FileNameHeaders_S& fnh);
  void                        Dump(std::ostream& msg) const;

private:
  char      m_fileName[CYCLIC_FILE_MAX_NAME_LEN];
  DWORD     m_sequenceNumber;
  DWORD     m_fileSize;
  CStructTm m_firstMessageTime;
  CStructTm m_lastMessageTime;
  BOOL      m_containsStartup;
  BYTE      m_compressionFormat;
  BYTE      m_nameFormatVersion;
  BOOL      m_IsRetrieved;

  FileNameHeaders_S m_FileHeaders;
};

class CCyclicFileList : public CSerializeObject
{
public:
  CCyclicFileList(const std::string& currentDir);

  virtual void                       SerializeXml(CXMLDOMElement*& pFatherNode) const;
  virtual void                       SerializeXmlPartialList(CXMLDOMElement*& pFatherNode,int nLastItems) const;
  virtual int                        DeSerializeXml(CXMLDOMElement* pActionNode,
                                                    char* pszError,
                                                    const char* action);
  virtual CSerializeObject*          Clone();
  virtual const char*                NameOf() const { return "CCyclicFileList"; }
  virtual void                       Dump(std::ostream& msg) const;
  virtual BOOL                       TestForFileSystemWarning() = 0;
  virtual BOOL                       DeleteFilesIfNeeded() = 0; // will be called after moving to next file
  void                               UpdateRetrieved(DWORD seq,
                                                     const std::string file_name);
  BOOL                               DeleteOldFilesCyclic(int maxNumOfFiles,
                                                          DELETE_FILE_REPORT_VECTOR& outDeletedFileVector);
  BOOL                               DeleteOldestFile(std::string& outDeletedFileName);
  DWORD                              GetLastFileSequenceNumber() const {return m_lastFileSequenceNumber;}
  void                               SetLastFileSequenceNumber(DWORD seqNumber) {m_lastFileSequenceNumber = seqNumber;}
  WORD                               GetNumOfFiles() const {return m_numOfFiles;}
  WORD                               GetNumOfUnretrievedFiles() const {return m_numOfUnretrievedFiles;}
  const char*                        GetCurrentDirrectory() const {return m_CurrentDir.c_str();}
  void                               SetFileHeaders(const FileNameHeaders_S& fnh);
  void                               AddFile(const CCyclicFile& loggerFile);
  const CCyclicFile&                 GetOldestFile() const;
  CCyclicFile&                       Back();
  CCyclicFile&                       Front();
  const CCyclicFile&                 FrontConst() const;
  void                               PopFront();
  CYCLIC_FILE_LIST::reverse_iterator rbegin(){return m_fileList.rbegin();}
  CYCLIC_FILE_LIST::reverse_iterator rend()  {return m_fileList.rend();}
  void                               SortList();
  CCyclicFile*                       GetFileBySeqNum(DWORD wantedSeqNum);

  static void CreateFileName(char* outFileName,
                             const FileNameHeaders_S&
                             fileHeaderNames,
                             DWORD fileSequenceNumber,
                             DWORD fileSize,
                             CStructTm firstMessageTime,
                             CStructTm lastMessageTime,
                             BOOL containsStartup,
                             BOOL isRetrieved,
                             const BYTE& cmprFormat);

  static BOOL ParseFileName(const char* fileNameToParse,
                            const FileNameHeaders_S&
                            fileHeaderNames,
                            DWORD& outFileSequenceNumber,
                            DWORD& outFileSize,
                            CStructTm&
                            outFirstMessageTime,
                            CStructTm&
                            outLastMessageTime,
                            BOOL& outContainsStartup,
                            BOOL& outIsRetrieved,
                            BYTE& outCmprFormat,
                            int& outNameFormatVersion);

protected:
  WORD              m_numOfFiles;
  WORD              m_numOfUnretrievedFiles;
  DWORD             m_lastFileSequenceNumber;
  const string      m_CurrentDir;
  CYCLIC_FILE_LIST  m_fileList;
  FileNameHeaders_S m_FileHeaders;

  DISALLOW_COPY_AND_ASSIGN(CCyclicFileList);
};

#endif  // CYCLIC_FILE_LIST_H_
