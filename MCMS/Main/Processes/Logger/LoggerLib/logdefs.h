#ifndef __LOGGER_DEFS_H
#define __LOGGER_DEFS_H

#include "DataTypes.h"


// Definitions for shortcuts in code

#define LOOP_FOREVER for(;;) // eternal loop

// Definitions for the internal use of the logger system

#define MAX_MESSAGES_IN_QUEUE			5000	// depth of logger message queue
#define TIME_DIFF_DELTA                 SECOND  // when comparing time (in ticks) allow for 1 second delta
#define MAX_OBJECT_NAME_SIZE			50		// max size of any string containg an object name
#define MESSAGE_DATE_TIME_STRING_LENGTH	45		// length of the date and time of a message converted to a string
#define FILE_NAME_SIZE					8+1+3	// max length of a logger file name "log00001.txt"
#define FILE_ONLY_NAME_SIZE				8		// the "log00001" part size from "log00001.txt"
#define FILE_SUFFIX_SIZE				4		// the ".txt" part size from "log00001.txt"
#define FILE_ACTUAL_NAME_SIZE			3		// the "log" part size from "log00001.txt"
#define FILE_NUM_SIZE					5		// the "00001" part size from "log00001.txt"
#define LONG_TO_STRING					11		// for DWORD variables written to file as chars
#define MAX_ERROR_FILE_SIZE             64*1024 // 64 KB for internal error file
#define FILE_SIZE_ZERO					500		// indicates a file is in size 0 - corrupted. this value is not used by errno
#define MAX_FILE_SEQUENCE_NUM			99999999// max sequence number in file's name (8 digits)

#define QUEUE_ERROR						"MESSAGE TOO BIG - DISCARDED"
#define OBJ_NAME_ERROR					"OBJECT NAME TOO BIG - DISCARDED"
#define INVALID_OBJECT_ERROR			"INVALID OR GLOBAL OBJECT"
#define QUEUE_FLUSH_ERROR				"QUEUE FLUSHED. NUM OF MESSAGES LOST:"
#define INDEX_FILE_HEADER				"ERROR LOG SYSTEM INDEX FILE"
#define LAST_MESSAGE_PREFIX				"Last Message  :"
#define FIRST_MESSAGE_PREFIX			"First Message :"
#define NUM_OF_RECORDS_PREFIX			"Num of Records: "
#define MAX_FILE_SIZE_PREFIX			"Max data file size (in bytes): "
#define MAX_NUM_OF_FILES_PREFIX			"Max Num of files: "
#define FILE_RANGE_PREFIX				"File range (in hours): "
#define MAX_FILE_TIME_TO_LIVE_PREFIX	"Max file time to live (in ticks): "
#define LAST_RECORD_POS_PREFIX			"Last record position: "
#define DATA_FILE_HEADER				"ERROR LOG SYSTEM DATA FILE"
/* WARNING : psos does not behave with lowercase names...*/
#define LOGGER_DIR						"LogFiles/" // last character should be "/"
#define DATA_FILE_NAME					"Log00001.log"		// file name will include the file number
#define INDEX_FILE_NAME					"LogIndex.txt"	// containes a list of all data files in the system
#define DATA_FILE_TEMP_END				".llg" // used as temp name when renaming files
#define ERROR_FILE_NAME                 "LogError.txt"        // used to report logger errors
#define CORRUPTED_FILE                  "Corrupted / Empty file deleted!"

// Definitions are defaults for the system variables

#define DEFAULT_MAX_MESSAGE_SIZE					8192	 // bytes - max message size - cannot be more than 64K!!
#define DEFAULT_FILE_BUFFER_MULTIPLIER				128		 // multiplier = means buffer will be 128 * 8192 Bytes = 1 MB.
#define DEFAULT_MAX_FILE_SIZE						1024*1024// Bytes = 1 MB
#define DEFAULT_MAX_NUM_OF_FILES					4000	 //
#define DEFAULT_MAX_NUM_OF_FILES_FOR_AMOS			8000	 // Amos
#define DEFAULT_FILE_RANGE							24		 // hours - NOT IMPLEMENTED
#define DEFAULT_MAX_FILE_TIME_TO_LIVE				SECOND*60*60              // time to live is 1 hour. max possible is 24 hours!!
#define DEFAULT_MAX_FLUSHED_MESSAGES				MAX_MESSAGES_IN_QUEUE / 3 // max number of messages that will be deleted if logger queue is full

// needed as an estimate to know when buffer / file limit is reached
#define MAX_INTERNAL_COMPRESSION_BUFFER 32*1024  // 32 KB

struct index_file_record
{
	char chNum[FILE_NUM_SIZE];
	char chSpace1[1];
	char chDataFileName[FILE_NAME_SIZE];
	char chSpace2[1];
	char chFileSize[LONG_TO_STRING];
	char chSpace3[1];
	char chFirstMessageDateTime[MESSAGE_DATE_TIME_STRING_LENGTH];
	char chSpace4[1];
	char chLastMessageDateTime [MESSAGE_DATE_TIME_STRING_LENGTH];
	char chEOL[2];
};

struct index_file_first_record
{
	char chIndexFileHeader[sizeof(INDEX_FILE_HEADER)-1];
	char chEOL1[2];
	char chNumOfRecordsPrefix[sizeof(NUM_OF_RECORDS_PREFIX)-1];
	char chNumOfRecords[FILE_NUM_SIZE];
	char chEOL2[2];
	char chMaxFileSizePrefix[sizeof(MAX_FILE_SIZE_PREFIX)-1];
	char chMaxFileSize[LONG_TO_STRING];
	char chEOL3[2];
	char chMaxNumOfFilesPrefix[sizeof(MAX_NUM_OF_FILES_PREFIX)-1];
	char chMaxNumOfFiles[LONG_TO_STRING];
	char chEOL4[2];
	char chFileRangePrefix[sizeof(FILE_RANGE_PREFIX)-1];
	char chFileRange[LONG_TO_STRING];
	char chEOL5[2];
	char chMaxFileTimeToLivePrefix[sizeof(MAX_FILE_TIME_TO_LIVE_PREFIX)-1];
	char chMaxFileTimeToLive[LONG_TO_STRING];
	char chEOL6[2];
	char chLastRecordPosPrefix[sizeof(LAST_RECORD_POS_PREFIX)-1];
	char chLastRecordPos[LONG_TO_STRING];
	char chEOL7[2];
};

struct data_file_first_record
{
	char chDataFileHeader		[sizeof(DATA_FILE_HEADER)-1];
	char chEOL1[2];
	char chFirstMessageDateTime [MESSAGE_DATE_TIME_STRING_LENGTH];
	char chEOL2[2];
	char chLastMessageDateTime  [MESSAGE_DATE_TIME_STRING_LENGTH];
	char chEOL3[2];
};

struct data_file_data_prefix
{
	ULONG lSerialNumber;
	ULONG lTextSize;
	ULONG lDate;
	ULONG lTime;
	ULONG lTicks;
	ULONG lTaskID;
	ULONG lSourceID;
	ULONG lTraceLevel;
	ULONG lObjectNameSize;
};

struct data_file_data_shell
{
	DWORD lJunkData;
	WORD  iOpcode;
	data_file_data_prefix sErrMsg;
};

struct logger_configuration_struct
{
	DWORD lMaxMessageSize;		// in bytes
	DWORD lFileBufferMultiplier;// in bytes - file buffer size is in multiples of lMaxMessageSize  used to pop messages from queue.
	DWORD lMaxFileSize;			// in bytes - a single file cannot exceed this size.
	DWORD lMaxNumOfFiles;		// maximum number of files allowed.
	DWORD lFileRange;			// in hours - files older than this value will be deleted.
	DWORD lMaxFileTimeToLive;	// in ticks - a file will be permanently closed when this value is reached.
	DWORD lMaxFlushedMessages;	// if queue is full try to pop out this number of messages before resending messages
};

/*
struct dirent {
        unsigned long d_filno;
        char    d_name[MAX_NAME_LEN + 1];
};
*/
#endif // ##ifndef __LOGGER_DEFS_H
