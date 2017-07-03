// OsFileIF.h

#ifndef OSFILEIF_H_
#define OSFILEIF_H_

#include <dirent.h>
#include <iostream>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>

#include "DataTypes.h"

struct FDStruct
{
  FDStruct() :
    type('\0')
  {}

  FDStruct(const char* pname, char ptype) :
    name(pname),
    type(ptype)
  {}

  std::string name;     // The file name
  char type;            // The file Descriptor type 'D' = Directory
                        // 'F' = File
                        // 'S' = symbolic link
                        // 'P' = pipe
};

BOOL      GetDirectoryContents(const char* path,
                               std::vector<std::string>& files,
                               std::ostringstream& err);
BOOL      GetDirectoryContents(const char* path, std::vector<FDStruct>& files);
int       GetDirFilePrefixNum(const char* path, const char* prefix);
int       GetDirFilesNum(const char* path);
int       GetDirFDByPath(const char* path);
int       getFDByDirent(struct dirent* DirentParam, const char* DirPath);
BOOL      GetDirStreamByPath(const char* path, DIR** DirStream);
STATUS    DeleteDirectory(const char* path);

int       GetFDByPath(const char* path);
time_t    GetLastModified(int filedes);
time_t    GetLastModified(const std::string& FileName);
BOOL      CreateDirectory(const char* path, mode_t mode = 0775);
BOOL      CreateFile(const std::string& FileName);
FDStruct* GetFirstFile(std::vector<FDStruct>::iterator& Vector_iterator,
                       std::vector<FDStruct>& m_fstruct);
FDStruct* GetNextFile(std::vector<FDStruct>::iterator& Vector_iterator,
                      std::vector<FDStruct>& m_fstruct);
void      erase_file_list(std::vector<FDStruct>& m_fstruct);
BOOL      DeleteFile(const std::string& FileName, BOOL bSync = true);
BOOL      IsDirectoryEmpty(const char* DirPath);
BOOL      IsFileExists(const std::string& FileName);
BOOL      RenameFile(const std::string& OldFileName,
                     const std::string& NewFileName);
BOOL      CopyFile(const std::string& src, const std::string& target);
BOOL      CreateSymbolicLink(const std::string& PathToBeLinkedTo,
                             const std::string& LinkFullPath);
int       GetFileSize(const std::string& FileName);
BOOL      ReadSymbolicLink(const std::string& Link, std::string& LinkContent, BOOL suppressAssert = 0);

// Reads limited number of symbols from a file to a string
STATUS    ReadFileToString(const char* fname,
                           unsigned long max_len,
                           std::string& out);

// Checks whole file consists of ASCII symbols
STATUS    IsFileASCII(const char* fname, bool& result);

// Writes the string to MCU_TMP_DIR/common-log file
void      WriteLocalLog(const char* str, bool log_per_proc = false);

// Looks for a process name in /proc directory
STATUS    IsProcAlive(const char* pname, bool& is_alive);

#endif  // OSFILEIF_H_

