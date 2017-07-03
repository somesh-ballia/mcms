// FileList.h

#ifndef FILE_LIST_H_
#define FILE_LIST_H_

#include "SerializeObject.h"
#include "NStream.h"
#include "psosxml.h"

#define MAX_FILES_IN_LIST 10000

class CXMLDOMElement;

class CFileList: public CSerializeObject
{
  CLASS_TYPE_1(CFileList, CSerializeObject)
public:
  CFileList();
  CFileList(const CFileList& other);
  virtual ~CFileList();

  void              SerializeXml(CXMLDOMElement*& pParentNode) const;
  int               DeSerializeXml(CXMLDOMElement* pActionNode,
                                   char* pszError,
                                   const char* action);
  int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
  CSerializeObject* Clone() {return new CFileList;}
  const char*       NameOf() const;
  WORD              GetFilesNumber() const;
  void              SetFilesNumber(const WORD num);
  int               Add(const char* other);
  int               Update(const char* other);
  int               Cancel(const char* name);
  int               FindFile(const char* name);
  char*             GetCurrentFile(const char* name);
  char*             GetFirstFile();
  char*             GetNextFile();
  int               FillFileList(const char* Path,
                                 WORD bNested = FALSE,
                                 BYTE bXmlFileList = FALSE,
                                 std::string strDirName = "");
  WORD  m_numb_of_files;
  char* m_fileName[MAX_FILES_IN_LIST];

private:
  CFileList& operator=(const CFileList&);
  WORD m_ind_file;
};

#endif

