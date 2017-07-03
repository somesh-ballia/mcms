#ifndef MCMSLOG_PARSER_H
#define MCMSLOG_PARSER_H 1

/*-------------------------- Includes ------------------------------- */
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <string>
#include <map>

/*-------------------------- Defines -------------------------------- */
#define TRUE   1
#define FALSE  0
#define ERROR -1

#define LOG_FILE_NAME_LEN  	    1000   //including path
#define MAX_LINE_LEN 		    (8000) //Defines Max length of a line in the log file
#define MAX_NAME_LEN 		    (50) //Defines Max length of a file/func name in the log file
#define MAX_NUMBER_OF_LINES     (500) //Defines Max number of lines/process per ".c" file

#define MAX_NUMBER_OF_MODULES   (75000) //Defines Max number of ".c" files appearance in the log file
//#define MAX_NUMBER_OF_MODULES   (100000) //Defines Max number of ".c" files appearance in the log file

#define MAX_NUMBER_OF_PROCESSES   (90) //Defines Max number of processes

#define MAX_NUMBER_OF_LOG_FILES 80
#define MAX_PROCESS_NAME_LEN    30
#define NUM_OF_LINE_CHARS       6

#define MCMSLINE   					"E:Mcms"
#define ARTLINE                     "E:Art"
#define CMLINE                      "E:CardManager"
#define VIDEOLINE                   "E:Video"

#define PROCESS_ID           		"P:"
#define PROCESS_NAME_END_CHARACTER	" "
#define FILE_NAME_ID         		"Lctn:"
#define FILE_NAME_END_CHARACTER 	"("
#define Line_NUM_END_CHARACTER 	    ")"

/*-------------------------- Structs -------------------------------- */
typedef unsigned char boolean;

// TODO count per line and not per process
typedef struct {
	unsigned int unLineNumber;
	int ProcessNameId;
	unsigned int unTotalLineProcessRptCnt;
} TLineAllProcesses;


typedef struct {
	char ucModuleNameStr[MAX_NAME_LEN];
	TLineAllProcesses taArrayOfProcessesLines[MAX_NUMBER_OF_LINES];
	unsigned int unLinesArrayCellsCnt; //Counter for the number of cells in LineProcess array . 
} TFile;

typedef struct {
	unsigned int unFilesArrayCellsCnt; //Counter for the number of cells in Board array
    TFile taArrayOfModules[MAX_NUMBER_OF_MODULES];
    char  LogFileName[MAX_NUMBER_OF_LOG_FILES][LOG_FILE_NAME_LEN];
    unsigned int unLogFilesCnt;

    // overall counters
    unsigned int unMcmsLinesCnt;
    unsigned int unArtLinesCnt;
    unsigned int unVideoLinesCnt;
    unsigned int unMCLinesCnt;
} TData;

typedef struct {
    // overall counters
    unsigned int unOverallNumLinesDirCnt; 
    unsigned int unOverAllMcmsLinesCnt;
    unsigned int unOverAllArtLinesCnt;
    unsigned int unOverAllVideoLinesCnt;
    unsigned int unOverAllMCLinesCnt;
} TDataSummary;


/*----------------------- Function Declerations----------------------- */
char* Get_Line(char *aucLineStr, int LineLength, FILE *log_file);
boolean GetProcessName(char *MCMSLineStr, char *OutProcessName);
void PrintToFile (void);
int SearchForMCMSLogs(char auLineOfFileStr[MAX_LINE_LEN]);
int parse_file (char* puInputFile);
void	AddToSummary(void);

class StringToIdMap
{
	public:
		StringToIdMap();
		int GetStringId(const std::string str);	
		
		const std::string GetStrById(int id) const;	

	private:
		int	m_lastId;
		std::map<std::string, int>	m_StrToIdMap;
};


#endif
