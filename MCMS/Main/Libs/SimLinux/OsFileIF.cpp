// OsFileIF.cpp

#include "OsFileIF.h"

#include <limits>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "Macros.h"
#include "TaskApi.h"
#include "DataTypes.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "SharedDefines.h"
#include "SystemFunctions.h"
#include "StatusesGeneral.h"

int GetDirFDByPath(const char* path)
{
  DIR* pDirStream;
  if ((pDirStream = opendir(path)) == NULL)
    return 0;

  return dirfd(pDirStream);
}

BOOL GetDirStreamByPath(const char* path, DIR** pDirStream)
{
  DIR* temp;
  if ((temp = opendir(path)) == NULL)
    return FALSE;

  *pDirStream = temp;
  return TRUE;
}

int GetFDByPath(const char* path)
{
  int filedes = 0;
  if ((filedes = open(path, 0)) == -1)
  {
    std::string temp = "Error opening file ";
    temp = temp + path;
    perror(temp.c_str());
    return -1;
  }

  return filedes;
}

int getFDByDirent(struct dirent* DirentParam, const char* DirPath)
{
  string FilePath = DirPath;
  FilePath = FilePath + "/";
  FilePath = FilePath +  DirentParam->d_name;
  return GetFDByPath(FilePath.c_str());
}

BOOL IsDirectoryEmpty(const char* DirPath)
{
  DIR* DirStream = NULL;
  if (TRUE == GetDirStreamByPath(DirPath, &DirStream))
    if (readdir(DirStream) != NULL)
      return FALSE;

  return TRUE;
}

STATUS DeleteDirectory(const char* path)
{
  FPASSERT_AND_RETURN_VALUE(NULL == path, STATUS_FAIL);
  FPASSERTSTREAM_AND_RETURN_VALUE(!IsFileExists(path),
      "DeleteDirectory: " << path << ": No such file or directory",
      STATUS_FILE_NOT_EXISTS);

  std::vector<FDStruct> files;
  BOOL res = GetDirectoryContents(path, files);
  if (!res)
    return STATUS_FILE_READ_ERROR;

  std::vector<FDStruct>::const_iterator file;
  for (file = files.begin(); file != files.end(); ++file)
  {
    // Composes full name.
    std::ostringstream fname;
    fname << path;

    if (path[strlen(path) - 1] != '/')
      fname <<  "/";

    fname << file->name;

    switch (file->type)
    {
    case 'D':
      {
        // Recursive operation.
        STATUS stat = DeleteDirectory(fname.str().c_str());
        if (STATUS_OK != stat)
          return stat;
      }
      break;

    default:
      res = DeleteFile(fname.str());
      if (!res)
        return STATUS_FILE_DELETE_ERROR;
      break;
    }
  }

  int res2 = rmdir(path);
  FPASSERTSTREAM_AND_RETURN_VALUE(0 != res2,
      "rmdir: " << path << ": " << strerror(errno),
      STATUS_FOLDER_REMOVE_ERROR);

  return STATUS_OK;
}

time_t GetLastModified(int filedes)
{
  struct stat64 buf64;
  if (-1 != fstat64(filedes, &buf64))
    return (buf64.st_mtime);

  return -1;
}

static bool add_file_helper(const char* path,
                            struct dirent* dp,
                            std::vector<std::string>& files,
                            std::ostringstream&)
{
  files.push_back(dp->d_name);
  return true;
}

static bool add_file_helper(const char* path,
                            struct dirent* dp,
                            std::vector<FDStruct>& files,
                            std::ostringstream& err)
{
  // Is it file, directory, link or pipe?
  char type;

// It looks like the feature is not supported by current version of
// glibc. The macro is defined but the type is sometimes unknown.
// Meanwhile cancel the feature.
#undef _DIRENT_HAVE_D_TYPE

#ifdef _DIRENT_HAVE_D_TYPE

  switch (dp->d_type)
  {
  case DT_REG:
    type = 'F';
    break;

  case DT_DIR:
    type = 'D';
    break;

  case DT_LNK:
    type = 'S';
    break;

  case DT_FIFO:
    type = 'P';
    break;

  default:
    err << __FILE__ << ": " << __LINE__
        << ": readdir: " << path << ": " << dp->d_name
        << ": Illegal type " << dp->d_type;
    return false;
  }

#else

  // Composes full name.
  std::ostringstream fname;
  fname << path << "/" << dp->d_name;

  struct stat64 buf;
  int rs = stat64(fname.str().c_str(), &buf);
  if (rs < 0)
  {
    err << __FILE__ << ": " << __LINE__
        << ": stat64: " << fname.str() << ": "
        << strerror(errno) << " (" << errno << ")";
    return false;
  }

  if (S_ISREG(buf.st_mode))
    type = 'F';
  else if (S_ISDIR(buf.st_mode))
    type = 'D';
  else if (S_ISLNK(buf.st_mode))
    type = 'S';
  else if (S_ISFIFO(buf.st_mode))
    type = 'P';
  else
  {
    err << __FILE__ << ": " << __LINE__
        << ": stat64: " << path << ": " << dp->d_name
        << ": Illegal type " << static_cast<unsigned int>(buf.st_mode);
    return false;
  }

#endif

  files.push_back(FDStruct(dp->d_name, type));

  return true;
}

template<typename T>
bool get_directory_contents_helper(const char* path,
                                   std::vector<T>& files,
                                   std::ostringstream& err)
{
  if (NULL == path)
  {
    err << __FILE__ << ": " << __LINE__ << ": Illegal parameter";
    return false;
  }

  if (!IsFileExists(path))
  {
    err << __FILE__ << ": " << __LINE__ << ": IsFileExists: " << path;
    return true;  // It is not exception error.
  }

  // The IO operations take a lot of time, unlocks other tasks.
  CTaskApp::Unlocker unlocker;

  errno = 0;
  DIR* dirp = opendir(path);
  if (NULL == dirp)
  {
    err << __FILE__ << ": " << __LINE__
        << ": opendir: " << path << ": "
        << strerror(errno) << " (" << errno << ")";
    return false;
  }

  struct dirent* dp;
  while ((dp = readdir(dirp)) != NULL)
  {
    if (0 == strcmp(dp->d_name, ".") || 0 == strcmp(dp->d_name, ".."))
      continue;

    bool rs = add_file_helper(path, dp, files, err);
    if (!rs)
    {
      closedir(dirp);
      return false;
    }
  }

  if (0 != errno)
  {
    err << __FILE__ << ": " << __LINE__
        << ": readdir: " << path << ": "
        << strerror(errno) << " (" << errno << ")";

    closedir(dirp);
    return false;
  }

  int res = closedir(dirp);

  // It is not exceptional error.
  if (res < 0)
    err << __FILE__ << ": " << __LINE__
        << ": closedir: " << path << ": "
        << strerror(errno) << " (" << errno << ")";

  return TRUE;
}

BOOL GetDirectoryContents(const char* path,
                          std::vector<std::string>& files,
                          std::ostringstream& err)
{
  bool ret = get_directory_contents_helper(path, files, err);

  FPASSERTMSG(!ret, err.str().c_str());
  if (ret && !err.str().empty()) {
    FTRACEWARN << err.str();
  }

  return ret ? TRUE : FALSE;
}

BOOL GetDirectoryContents(const char* path, std::vector<FDStruct>& files)
{
  std::ostringstream err;
  bool ret = get_directory_contents_helper(path, files, err);

  FPASSERTMSG(!ret, err.str().c_str());
  if (ret && !err.str().empty()) {
    FTRACEWARN << err.str();
  }

  return ret ? TRUE : FALSE;
}

int GetDirFilePrefixNum(const char* path, const char* prefix)
{
  std::string cmd = "ls ";
  cmd += path;
  cmd += prefix;
  cmd += "* 2> /dev/null | wc -l 2>&1";

  std::string ans;
  STATUS stat = SystemPipedCommand(cmd.c_str(), ans);
  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
    "SystemPipedCommand: " << cmd << ": " << ans,
    0);

  return atoi(ans.c_str());
}

int GetDirFilesNum(const char* path)
{
  std::ostringstream dummy;
  std::vector<std::string> files;
  BOOL res = GetDirectoryContents(path, files, dummy);
  if (!res)
    return 0;

  return files.size();
}

BOOL CreateDirectory(const char* path, mode_t mode)
{
  int res = mkdir(path, 0777);

  // Continues if directory exists.
  if (-1 == res && errno != EEXIST)
    return FALSE;

  // Set mask to write for group and other (octal 022).
  mode_t old = umask(S_IWGRP | S_IWOTH);

  res = chmod(path, mode);

  // Returns the mask back.
  umask(old);

  return (res == -1) ? FALSE : TRUE;
}

FDStruct* GetFirstFile(vector<FDStruct>::iterator& Vector_iterator,
                       vector <FDStruct>& m_fstruct)
{
  Vector_iterator = m_fstruct.begin();
  FDStruct* temp = new FDStruct;
  *temp = *Vector_iterator;
  return temp;
}

FDStruct* GetNextFile(vector <FDStruct>::iterator& Vector_iterator,
                      vector <FDStruct>& m_fstruct)
{
  Vector_iterator++;
  if (Vector_iterator != m_fstruct.end())
  {
    FDStruct* temp = new FDStruct;
    *temp = *Vector_iterator;
    return temp;
  }

  return NULL;
}

void erase_file_list(vector <FDStruct>& m_fstruct)
{
  m_fstruct.erase(m_fstruct.begin(), m_fstruct.end());
}

BOOL CreateFile(const std::string& fname)
{
  int res = open(fname.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
  if (-1 == res && errno != EEXIST)
    return FALSE;

  return TRUE;
}

BOOL DeleteFile(const std::string& fname, BOOL bSync)
{
  FTRACECOND_AND_RETURN_VALUE(!IsFileExists(fname),
    "IsFileExists: " << fname << ": No such file or directory",
     TRUE);

  int res = unlink(fname.c_str());
  FPASSERTSTREAM_AND_RETURN_VALUE(-1 == res,
    "unlink: " << fname << ": " << strerror(errno),
    FALSE);

  // Commits buffer cache to disk.
  if (bSync)
  {
	  // sync(); BRIDGE-14554 installerManager got stuckon the sync and demon killed it.
	  std::string cmd;
	  std::string answer;
	  cmd = "sync";
	  SystemPipedCommand(cmd.c_str(), answer, TRUE, TRUE, FALSE);
  }

  return TRUE;
}

time_t GetLastModified(const std::string& fname)
{
  struct stat64 buff64;
  int res = stat64(fname.c_str(), &buff64);
  if (-1 == res)
    return -1;

  return buff64.st_mtime;
}

BOOL IsFileExists(const std::string& fname)
{
  struct stat64 buff64;

  int res = stat64(fname.c_str(), &buff64);
  if (-1 != res)
    return TRUE;

  // No assertion if the path path does not exist.
  if (ENOENT != errno) {
    FPASSERTSTREAM(true, "stat: " << fname << ": " << strerror(errno));
  }

  return FALSE;
}

BOOL RenameFile(const std::string& OldFileName, const std::string& NewFileName)
{
  int res = 0;
  res = rename(OldFileName.c_str(), NewFileName.c_str());
  if (-1 == res)
    return FALSE;

  return TRUE;
}

BOOL CopyFile(const std::string& src, const std::string& dst)
{
  std::ifstream in(src.c_str());
  std::ofstream out(dst.c_str());

  char buf[4096];
  int  read_len = 0;
  do
  {
    read_len = in.readsome(buf, ARRAYSIZE(buf));
    if (read_len > 0)
      out.write(buf, read_len);
  }
  while (read_len != 0);

  in.close();
  out.close();

  return TRUE;
}

int GetFileSize(const std::string& fname)
{
  struct stat64 buf;
  int res = stat64(fname.c_str(), &buf);
  if (-1 == res)
  {
    // No assertion if the path path does not exist.
    if (ENOENT != errno) {
      FPASSERTSTREAM(true,
          "stat64: " << fname << ": " << strerror(errno) << " (" << errno << ")");
    }

    return -1;
  }

  return buf.st_size;
}

BOOL CreateSymbolicLink(const std::string& path,
                        const std::string& link)
{
  int res = symlink(path.c_str(), link.c_str());
  FPASSERTSTREAM_AND_RETURN_VALUE(-1 == res,
      "symlink: " << path << " to " << link
      << ": " << strerror(errno) << " (" << errno << ")",
      FALSE);

  return TRUE;
}

BOOL ReadSymbolicLink(const std::string& fname, std::string& content, BOOL suppressAssert)
{
	char buf[PATH_MAX];

	ssize_t res = readlink(fname.c_str(), buf, ARRAYSIZE(buf) - 1);
	if (static_cast<ssize_t>(-1) != res)
	{
		buf[res] = '\0';
		content = buf;
	}
	else
	{
		if (suppressAssert)
		{
			FTRACEINTOFUNC << "failed with error " << strerror(errno) << " (" << errno << ")";
			return FALSE;
		}
		else
		{
			FPASSERTSTREAM_AND_RETURN_VALUE(static_cast<ssize_t>(-1) == res,
					"readlink: " << fname << ": " << strerror(errno) << " (" << errno << ")",
					FALSE);
		}
	}



	return TRUE;
}

// Reads limited number of symbols from a file to a string.
// max_len = numeric_limits<unsigned int>::max() means read all file.
STATUS ReadFileToString(const char*   fname,
                        unsigned long max_len,
                        std::string&  out)
{
  FPASSERT_AND_RETURN_VALUE(NULL == fname, STATUS_FAIL);

  struct stat64 st;
  int res = stat64(fname, &st);
  FPASSERTSTREAM_AND_RETURN_VALUE(res < 0,
    "stat64: " << fname << ": " << strerror(errno) << " (" << errno << ")",
    STATUS_FILE_NOT_EXISTS);

  FPASSERTSTREAM_AND_RETURN_VALUE(!S_ISREG(st.st_mode),
    "stat64: " << fname << ": Is not a regular file",
    STATUS_FILE_NOT_EXISTS);

  std::ifstream in(fname);
  FPASSERTSTREAM_AND_RETURN_VALUE(!in,
      "ifstream::open: " << fname << ": " << strerror(errno) << " (" << errno << ")",
      STATUS_FILE_NOT_EXISTS);

  // Gets the size of the file.
  in.seekg(0, std::ios::end);
  std::streampos len = in.tellg();
  in.seekg(0, std::ios::beg);

  // Zeroes errno before read.
  errno = 0;

  // For /proc "files" len is -1. Reads these files by character which is slower.
  if (len < 0)
  {
    // Extra parentheses around the first argument are essential. They prevent
    // the problem known as the "most vexing parse".
    out.assign((std::istreambuf_iterator<char>(in)),
               std::istreambuf_iterator<char>());
  }
  else if (len < max_len)
  {
    // Reads a whole file to a string.
    out.resize(static_cast<size_t>(len)+1);
    in.read(&out[0], len);
    out[len] = '\0';
  }
  else
  {
    char tail[] = "...";

    // Cuts symbols by max_len.
    if (max_len > ARRAYSIZE(tail))
    {
    	out.resize(static_cast<size_t>(max_len));

      // Keeps some symbols for the tail.
      in.read(&out[0], max_len - ARRAYSIZE(tail));

      // Adds tail to the output string.
      std::copy(tail, ARRAYEND(tail), &out[max_len - ARRAYSIZE(tail)]);
      out[max_len-1] = '\0';
    }
    else
    {
      // There is place only for the tail.
      out.assign(tail);
    }
  }

  FPASSERTSTREAM_AND_RETURN_VALUE(errno,
      "ifstream::read: " << fname << ": " << strerror(errno) << " (" << errno << ")",
      STATUS_FILE_READ_ERROR);

  FPASSERTSTREAM_AND_RETURN_VALUE(in.bad(),
      "ifstream::read: " << fname << ": Unknown error",
      STATUS_FILE_READ_ERROR);

  return STATUS_OK;
}

STATUS IsFileASCII(const char* fname, bool& result)
{
  FPASSERT_AND_RETURN_VALUE(NULL == fname, STATUS_FAIL);
  FPASSERTSTREAM_AND_RETURN_VALUE(!IsFileExists(fname),
      "IsFileExists: " << fname,
      STATUS_FAIL);

  // Counts ASCII symbols in the file.
  std::string        ans1;
  std::ostringstream cmd1;
  cmd1 << "echo -n `strings " << fname << " | wc -c 2>&1`";

  STATUS stat = SystemPipedCommand(cmd1.str().c_str(), ans1, FALSE, FALSE);
  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
      "SystemPipedCommand: " << cmd1.str() << ": " << ans1,
      STATUS_FAIL);

  // Counts all symbols in the file.
  std::string        ans2;
  std::ostringstream cmd2;
  cmd2 << "echo -n `cat " << fname << " | wc -c 2>&1`";

  stat = SystemPipedCommand(cmd2.str().c_str(), ans2, FALSE, FALSE);
  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
      "SystemPipedCommand: " << cmd2.str() << ": " << ans2,
      STATUS_FAIL);

  int ascii = atoi(ans1.c_str());
  int all = atoi(ans2.c_str());

  // Command strings in ASCII check adds last \n.
  result = (ascii - 1 == all);

  if (!result)
    FTRACEWARN << "File " << fname << " has " << ascii - 1
               << " ASCII symbols of " << all << " all symbols";

  return STATUS_OK;
}

void WriteLocalLog(const char* str, bool log_per_proc /* = false */)
{
  const char* pname;
  CProcessBase* proc = CProcessBase::GetProcess();
  if (NULL != proc)
  {
    pname = CProcessBase::GetProcessName(proc->GetProcessType());
    if (NULL == pname)
      pname = "unproc";
  }
  else
  {
    pname = "noproc";
  }

  const char* aname;
  if (log_per_proc)
    aname = pname;
  else
    aname = "common";

  std::ostringstream fname;
  fname << MCU_TMP_DIR+"/log-" << aname;

  std::ofstream out(fname.str().c_str(), ios_base::app);
  for (int i = 1; !out; i++)
  {
    std::ostringstream fname2;
    fname2 << fname << i;
    out.open(fname2.str().c_str(), ios_base::app);

    if (i > 20)
      return;
  }

  time_t tt = time(NULL);
  struct tm* now = localtime(&tt);
  if (now)
    out << std::setfill('0')
        << std::setw(4) << now->tm_year + 1900
        << std::setw(2) << now->tm_mon + 1
        << std::setw(2) << now->tm_mday
        << std::setw(2) << now->tm_hour
        << std::setw(2) << now->tm_min
        << std::setw(2) << now->tm_sec;
  else
    out << "undefined time";

  out << ": " << pname << ": " << str << "\n";
}

STATUS IsProcAlive(const char* pname, bool& is_alive)
{
  FPASSERT_AND_RETURN_VALUE(NULL == pname, STATUS_FAIL);

  std::string        ans;
  std::ostringstream cmd;
  cmd << "ps -ef | grep " << pname
      << " | grep -v defunct | grep -v \" Z \" | grep -v \"grep " << pname << "\"";

  STATUS status = SystemPipedCommand(cmd.str().c_str(), ans);

  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != status,
      "SystemPipedCommand: " << cmd.str() << ": " << ans,
      status);

  is_alive = std::string::npos != ans.find(pname);
  return STATUS_OK;
}
