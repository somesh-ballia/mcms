// FileService.h

#ifndef FILE_SERVICE_H_
#define FILE_SERVICE_H_

#include <string>
#include <vector>

#include "PObject.h"
#include "OsFileIF.h"
#include "Macros.h"

typedef std::vector<std::string> VFile;

class CFileService : public CPObject
{
  CLASS_TYPE_1(CFileService, CPObject)

public:
  CFileService();

  virtual const char* NameOf() const { return "CFileService";}
  bool                CreateFolder(const std::string& dirName);
  bool                SetCurrentDir(const std::string& dirName);
  DWORD               GetNumOfFiles();
  std::string         GetFirstFileName();
  std::string         GetNextFileName();

  FILE*               GetFirstFile();
  FILE*               GetNextFile();

  std::string         GetCurrentFileName();

private:
  std::string     m_CurrentDirectory;
  VFile           m_CurrentDirContent;
  VFile::iterator m_IterCurrentFile;
  FILE*           m_FHandle;

  DISALLOW_COPY_AND_ASSIGN(CFileService);
};

#endif

