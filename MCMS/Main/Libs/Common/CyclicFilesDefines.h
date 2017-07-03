#ifndef __CYCLI_FILES_DEFINES_H__
#define __CYCLI_FILES_DEFINES_H__

#include <string.h>

#define CYCLIC_FILE_MAX_NAME_LEN 255
#define COMPRESSION_FORMAT_MAX_NAME_LEN 10

#define FILE_NAME_PATTERN_LEN 32

#define MAX_FILE_SEQUENCE_NUM			99999999// max sequence number in file's name (8 digits)

#define NAME_FORMAT_VERSION 1

// default headers are taken from Logger.
// use default ctor and then overwrite with your values.
struct FileNameHeaders_S
{
    char hdrName[FILE_NAME_PATTERN_LEN];
    char hdrSeqNum[FILE_NAME_PATTERN_LEN];
    char hdrFirstDate[FILE_NAME_PATTERN_LEN];
    char hdrFirstTime[FILE_NAME_PATTERN_LEN];
    char hdrLastDate[FILE_NAME_PATTERN_LEN];
    char hdrLastTime[FILE_NAME_PATTERN_LEN];
    char hdrFileSize[FILE_NAME_PATTERN_LEN];
    char hdrIsContainStartup[FILE_NAME_PATTERN_LEN];
    char hdrCmprFormat[FILE_NAME_PATTERN_LEN];
    char hdrNameFormatVersion[FILE_NAME_PATTERN_LEN];
    char hdrExtension[FILE_NAME_PATTERN_LEN];
    char hdrXmlTag[FILE_NAME_PATTERN_LEN];
    char hdrIsRetrieved[FILE_NAME_PATTERN_LEN];

public:
    FileNameHeaders_S()
        {
            strcpy(hdrName, "Log");
            strcpy(hdrSeqNum, "SN");
            strcpy(hdrFirstDate, "FMD");
            strcpy(hdrFirstTime, "FMT");
            strcpy(hdrLastDate, "LMD");
            strcpy(hdrLastTime, "LMT");
            strcpy(hdrFileSize, "SZ");
            strcpy(hdrIsContainStartup, "SU");
            strcpy(hdrCmprFormat, "CF");
            strcpy(hdrNameFormatVersion, "NFV");
            strcpy(hdrIsRetrieved,"RT");
            strcpy(hdrExtension, "log");
            strcpy(hdrXmlTag, "LOG_FILE_SUMMARY");
        }
};


#endif // __CYCLI_FILES_DEFINES_H__
