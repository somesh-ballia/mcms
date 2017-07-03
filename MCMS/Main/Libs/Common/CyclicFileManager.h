// CyclicFileManager.h

#ifndef CYCLIC_FILE_MANAGER_H_
#define CYCLIC_FILE_MANAGER_H_

#include <string>

#include "PObject.h"
#include "CyclicFilesDefines.h"
#include "Macros.h"

class CCyclicFile;
class CCyclicFileList;
class CStructTm;
class CCyclicFileManager;

typedef void (*AlertFunctionPointer)(int);

class CCyclicFileManager : public CPObject
{
CLASS_TYPE_1(CCyclicFileManager, CPObject)    
public:
    CCyclicFileManager(const std::string& dir,
                       const FileNameHeaders_S& fnh,
                       AlertFunctionPointer alertFunction = NULL);
    virtual ~CCyclicFileManager(void);
    virtual const char* NameOf() const{return "CCyclicFileManager";}
    BOOL TestForFileSystemWarning();

    // main interface methods
    STATUS Init(CCyclicFileList *fileList,
                BOOL isStartup,
                const BYTE & cmprFormat);// compress format for the next file 

    // appends to current file (in list)
    STATUS AppendToCurrentFile(const BYTE *data,     // data to file
                               DWORD dataLen,        // quantity of data
                               DWORD offset = 0);    // offset from the end of the current file

    STATUS MoveToNextFile(const CStructTm & firstMessageTime,    // time of first message
                          const CStructTm & lastMessageTime);     // time of last message

    CCyclicFileList* GetFileList(){return m_pFileList;}
    const std::string& GetCurrentFileDir()const{return m_FileDir;}
    DWORD GetCurrentFileSize()const;
    DWORD GetCurrentSeqNumber() const;

    void DumpFileList(std::ostream& answer);

    // converts buffer into "ready for EMA" file
    STATUS FlushBufferFile(bool isMoveNextFile,
                           bool isNextFileStartup,
                           const CStructTm & firstMessageTime,
                           const CStructTm & lastMessageTime);
    
    CCyclicFile & GetCurrentFile();

    void InformRetrived(const std::string &file_name);
    BOOL RemoveOldFiles();
    
private:
    STATUS LoadEMAReadyFiles();
    STATUS InitBufferFile();
    void RemoveEMAFileBySeqNumber(DWORD sequenceNumber);
    
    void IncSeqNumInitBufferFile(BOOL isContainStartup,
                                 const BYTE & cmprFormat);
    
    void InitBufferFileExist(const std::string& bufferFileName);
    bool IsBufferFileExist(std::string& outBufferFileName);
    void InitBufferFilenameHeaders(FileNameHeaders_S & bufferHdr);
    
    CCyclicFile *m_BufferFile;
    CCyclicFileList *m_pFileList;
    const FileNameHeaders_S m_FileHeaderNames;
    FileNameHeaders_S m_BufferFileHeaderNames;
    const std::string m_FileDir;
    bool m_IsInit;
    AlertFunctionPointer m_alertFunction;

    DISALLOW_COPY_AND_ASSIGN(CCyclicFileManager);
};

#endif  // CYCLIC_FILE_MANAGER_H_
