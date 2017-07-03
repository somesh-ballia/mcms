#ifndef __LOGGER_H
#define __LOGGER_H


#include <list>
#include "PObject.h"
#include "logdefs.h"
#include "zlib.h"
#include "StructTm.h"
#include "LoggerDefines.h"
#include "LoggerProcess.h"

using namespace std;

class dirent;
class CZipCompressor;
class CFileService;
class CLoggerFile;
class CLoggerFileList;
class CCyclicFileManager;




enum eLogFileManagerState
{
	eLogFileManagerState_Idle,
	eLogFileManagerState_Startup,
	eLogFileManagerState_Ready,
	eLogFileManagerState_Major,

	NumOfLogFileManagerStates
};

static const char *LogFileManagerStateNames []= {
	"eIdle", 	// eIdle
	"eStartup", // eStartup
	"eReady",  	// eReady
	"eMajor"    // eMajor
};

static const char *GetLogFileManagerStateName(eLogFileManagerState state)
{
	const char *name = (eLogFileManagerState_Idle <= state && state < NumOfLogFileManagerStates
						?
						LogFileManagerStateNames[state] : "Invalide");
	return name;
}




///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
class CLogFileManager : public CPObject		// This is a singleton class
{
CLASS_TYPE_1(CLogFileManager,CPObject)

public:
	CLogFileManager();
	virtual ~CLogFileManager();
	virtual const char* NameOf() const { return "CLogFileManager";}

	const logger_configuration_struct & GetSystemCfg() const;
	BOOL MessageToFile(const char *msg);		// save message in local HD
	BOOL Flush(WORD isShutDown=0);
	eLogFileManagerState GetState()const;
    void SetState(eLogFileManagerState newState);
    void InformLogFileRetrieved(const std::string name);

    BOOL TestForFileSystemWarning();
    STATUS RemoveFilesOnLogSizeChanged();
//	void RemoveLogFiles();

	void InitLogger();


private:
	void SetDefaults();

	void SetSystemCfg (logger_configuration_struct &loggerCfg);

	BOOL LoggerShutDown();
	void FatalError(const char *ErrorMessage, const ULONG ErrorCode);

	BOOL  SetMaxMessageSize(const ULONG lMaxMessageSize);			// warning : check memory limits.
	BOOL  SetMaxNumOfFiles(const ULONG lMaxNumOfFiles);				// warning : files can be deleted.
	BOOL  SetFileRange(const ULONG lFileRange);						// not used
	BOOL  SetMaxFileSize(const ULONG lMaxFileSize);					// warning : files can be deleted.
	BOOL  SetMaxFileTimeToLive(const ULONG lMaxFileTimeToLive);		// warning : files can be deleted.
	BOOL  SetMaxFlushedMessages(const ULONG lMaxFlushedMessages);
	BOOL  SetFileBufferMultiplier(const ULONG lFileBufferMultiplier); // warning : check memory limits.

//	void FillLoggerFileList();
//	BOOL IsFileNameValid(const char* loggerFileName);
// 	void CreateFileName(char* loggerFileName,
// 						DWORD nextFileSequenceNumber,
// 						DWORD fileSize,
// 						CStructTm firstMessageTime,
// 						CStructTm lastMessageTime,
// 						BOOL containsStartup);

    static void AlertEveryXFiles(int x);

	void WriteVersionNumberToLog();
	void SendLoggerNumber();

	logger_configuration_struct m_sLoggerConfiguration;
	data_file_data_prefix		m_lFirstMessagePrefix;
	data_file_data_prefix		m_lLastMessagePrefix;

	BOOL			m_startUpFlag;

	CZipCompressor 	*m_pZipCompressor;

	BYTE			*m_pFileBuffer;			// will hold data before writing to HD.
    ULONG        	m_lFileBufferSize;     	// size of buffer in bytes

	CFileService 	*m_FileService;

    CCyclicFileManager *m_pCyclicFileManager;
//	eLogFileManagerState m_State;
};



#endif // #ifndef __LOGGER_H
