// LoggerFileList.h

#ifndef LOGGER_FILE_LIST_H_
#define LOGGER_FILE_LIST_H_

#include "StructTm.h"
#include "SerializeObject.h"
#include "LoggerDefines.h"
#include "CyclicFileList.h"

class CStructTm;
class CXMLDOMElement;
class CLoggerFile;

class CLoggerFileList: public CCyclicFileList
{
CLASS_TYPE_1(CLoggerFileList,CSerializeObject)
public:
  CLoggerFileList();
  virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
  virtual void SerializeXmlPartialList(CXMLDOMElement*& pFatherNode,int nLastItems) const;
  virtual const char* NameOf() const
  {
    return "CLoggerFileList";
  }

  virtual BOOL DeleteFilesIfNeeded();
  virtual BOOL TestForFileSystemWarning();
  BOOL DeleteOldestLoggerFile();
};

#endif
