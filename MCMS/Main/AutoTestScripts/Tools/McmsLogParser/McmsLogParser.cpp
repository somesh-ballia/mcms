# include "McmsLogParser.h"
#include <iostream>
#include <fstream>
#include <iomanip> 


TData LogFiledata;

TDataSummary DataSummary;


std::ofstream fsOutputFile;

StringToIdMap	ProcessNameToIdMap;

static int numberOfRestarting = 0;

/********************************************************************************************
   class StringToIdMap
    Map string to Uniques Id. Will be used to mao file names to uniques ids and processes to unique Id in order to hold it multiple times in memory
***********************************************************************************************/


StringToIdMap::StringToIdMap() : m_lastId(0)
{
}

int StringToIdMap::GetStringId(const std::string str)
{
	std::map<std::string, int>::iterator it = m_StrToIdMap.find(str);
	if (it != m_StrToIdMap.end())
	{		
		return it->second;
	}

	++m_lastId;
	m_StrToIdMap.insert(std::pair<std::string, int > (str, m_lastId));
	return m_lastId;
		
}

const std::string StringToIdMap::GetStrById(int id) const
{
	std::map<std::string, int>::const_iterator it;
	for (it = m_StrToIdMap.begin(); it != m_StrToIdMap.end(); ++it)
	{
		if (it->second == id)
		{
			return it->first;
		}
	}
	std::cout << "Error to string was found for id " << id << "std::end";
	return "";
}


/********************************************************************************************
   The function gets a pointer to array of strings, size of array and a pointer to an input file. 
   The function reads characters from the input file and stores them into aucLineStr
   until (LineLength-1) characters have been read.
   or either a newline or the End-of-File is reached, whichever comes first.
   The function returns a pointer to the beginning of a line in the input file.
***********************************************************************************************/
char* Get_Line(char *aucLineStr, int LineLength, FILE *log_file)
{
	int tav;
	int i = 0;

	tav = fgetc(log_file);//Reads a single character from log_file
	//Reads till (LineLength-1) characters have been read or either a newline or the End-of-File is reached, whichever comes first
	while ((tav != 10) && (tav != 13) && (i < LineLength - 1) && (!feof(log_file)))
	{
		aucLineStr[i] = tav;//Stores the character in the array
		++i;
		tav = fgetc(log_file);//Reads a single character from log_file
	}
	if (tav == 10)//Checks if it's the end of the line (ended with "\n")
	{
		aucLineStr[i] = (char)tav;
		++i;
		aucLineStr[i] = '\0';//Puts "NULL" to mark the end of the line
	}
	else if (tav == 13)//Checks if it's the end of the line, if line ended with "\r\n"
	{
		aucLineStr[i] = (char)tav;
		++i;
		tav = fgetc(log_file);//Reads a single character from log_file
		if (tav == 10) //Checks if after '\r' we got '\n'
		{
			aucLineStr[i] = (char)tav;
			++i;
			aucLineStr[i] = '\0';//Puts "NULL" to mark the end of the line		
		}
	}
	//Line size is bigger then definition(LineLength)
	else if (i == LineLength -1)
	{
		aucLineStr[i] = '\0';
		fseek (log_file , -1 , SEEK_CUR);
		printf("Error!!!Line size %d exceeded the maximum line length definition %d\n", i , LineLength);
	}
	//End of file is reached
	else if (feof(log_file))
	{
		return NULL;
	}
	return aucLineStr;
}

boolean GetProcessName(char *MCMSLineStr, char *OutProcessName)
{
	char *process_name_start = NULL;
	char *process_name_end = NULL;
	int process_name_len = 0;

	process_name_start = strstr(MCMSLineStr, PROCESS_ID);
	if (process_name_start != NULL) {
		process_name_start += strlen(PROCESS_ID);
		process_name_end = strstr(process_name_start, PROCESS_NAME_END_CHARACTER);
		if (process_name_end != NULL) {
			process_name_len = process_name_end - process_name_start;
		} else {
			return FALSE;
		}
	} else {
		return FALSE;
	}

	if ((process_name_len>0) && (process_name_len< MAX_PROCESS_NAME_LEN)) {
		strncpy(OutProcessName, process_name_start, process_name_len);
		return TRUE;
	} else {
		return FALSE;
	}
}


boolean GetFileName(char *MCMSLineStr,
					char *TempFileName,
					char **pLineNumber)
{
	char *file_name_start = NULL;
	char *file_name_end = NULL;
	int file_name_len = 0;

	file_name_start = strstr(MCMSLineStr, FILE_NAME_ID);
	if (file_name_start != NULL) {
		file_name_start += strlen(FILE_NAME_ID);
		file_name_end = strstr(file_name_start, FILE_NAME_END_CHARACTER);
		if (file_name_end != NULL) {
			file_name_len = file_name_end - file_name_start;
		} else {
			return FALSE;
		}
	} else {
		return FALSE;
	}

	if (file_name_len < MAX_NAME_LEN)
		strncpy(TempFileName, file_name_start, file_name_len);

	*pLineNumber = file_name_end+1;
	return TRUE;
}


boolean GetLineNum(char *InLinePtr, int *outLineNum)
{
	char *line_num_end = NULL;
	char tempLinestr[NUM_OF_LINE_CHARS];
	int i;

	memset(tempLinestr, '\0', NUM_OF_LINE_CHARS);

	line_num_end = strstr(InLinePtr, Line_NUM_END_CHARACTER);
	if (line_num_end == NULL)
		return FALSE;

	if (line_num_end-InLinePtr < NUM_OF_LINE_CHARS)
		strncpy(tempLinestr, InLinePtr, line_num_end-InLinePtr);
	else
		return FALSE;

	for (i=0; i<(NUM_OF_LINE_CHARS-1) && tempLinestr[i]!='\0'; i++)
	{
		if (! (isdigit(tempLinestr[i])))
			return FALSE;
	}

	*outLineNum = atoi(tempLinestr);
	return TRUE;
}

boolean Look_if_file_in_data(char *inFileName,
							 int  *outModuleIndex)
{
	int i;

	if (LogFiledata.unFilesArrayCellsCnt == 0) {
		*outModuleIndex = 0;
		return FALSE;
	}

	for (i=0; i<LogFiledata.unFilesArrayCellsCnt; i++)
	{
		if (strcmp(inFileName,
				LogFiledata.taArrayOfModules[i].ucModuleNameStr) == 0)
		{
			*outModuleIndex = i;
			return TRUE;
		}
	}

	return FALSE;
}


boolean Look_if_line_in_data(int inLineNum,
							 int inModuleIndex,
							 // char *inProcessName,
							 int inProcessNameId,
							 int *outlineIndex)
{
	int     i;
	boolean found = FALSE;

	if (LogFiledata.taArrayOfModules[inModuleIndex].unLinesArrayCellsCnt == 0) {
		*outlineIndex = 0;
		return FALSE;
	}

	for (i=0; i<LogFiledata.taArrayOfModules[inModuleIndex].unLinesArrayCellsCnt; i++)
	{
		if (inProcessNameId == LogFiledata.taArrayOfModules[inModuleIndex].taArrayOfProcessesLines[i].ProcessNameId &&
			(LogFiledata.taArrayOfModules[inModuleIndex].taArrayOfProcessesLines[i].unLineNumber == inLineNum) )
		{
			found = TRUE;
			*outlineIndex = i;
		}

	}

	if (found)
		return TRUE;
	else {
		*outlineIndex = LogFiledata.taArrayOfModules[inModuleIndex].unLinesArrayCellsCnt;
		return FALSE;
	}
}

/******************************************************************************************************************************
   The function gets a pointer to a line in the log file, a pointer to a Board/Switch name and the log's lines counter.
   The function searches for the Board/Switch name in the line, stores it in the struct if necessary and increases it's counter by 1.
*******************************************************************************************************************************/
int SearchForMCMSLogs(char auLineOfFileStr[MAX_LINE_LEN])
{
	char* puMCMSLineStr = NULL;
    char auNewLineStr[MAX_LINE_LEN] = "\0";
    char TempProcessName[MAX_PROCESS_NAME_LEN] = "\0";
    char TempFileName[MAX_NAME_LEN] = "\0";
    int  tempLineNum = 0;
    boolean file_exist = FALSE;
    boolean line_process_exist = FALSE;
    char *pLineNumber = NULL;
    int  moduleIndex = 0;
    int  lineIndex = 0;

    strncpy(auNewLineStr, auLineOfFileStr, MAX_LINE_LEN); //Create a copy of the whole line
    puMCMSLineStr = strstr(auLineOfFileStr, MCMSLINE); //Searches for a MCMS id in the line
    if (puMCMSLineStr != NULL) //Checks it is a MCMS line
    {
    	// count MCMS lines
    	LogFiledata.unMcmsLinesCnt++;

		if (!GetProcessName(puMCMSLineStr, TempProcessName))
		{
			return FALSE;
		}
		if (!GetFileName(puMCMSLineStr, TempFileName, &pLineNumber))
		{
			return FALSE;
		}

		if (!GetLineNum(pLineNumber, &tempLineNum))
		{
			return FALSE;
		}

		int ProcessNameId = ProcessNameToIdMap.GetStringId(TempProcessName);

		file_exist = Look_if_file_in_data(TempFileName, &moduleIndex);
		if (file_exist)
		{
			line_process_exist = Look_if_line_in_data(tempLineNum, moduleIndex, ProcessNameId, &lineIndex);
		}
		else {
			line_process_exist = FALSE;
			lineIndex = 0;
		}

		if (++LogFiledata.unFilesArrayCellsCnt > MAX_NUMBER_OF_MODULES) {			
			printf("Overflow too many *.cpp files for parsing\n");
			//LogFiledata.unFilesArrayCellsCnt--;
			//print the data to output file and start over....
			PrintToFile();
			AddToSummary();
			memset(&LogFiledata, 0, sizeof(TData));
			LogFiledata.unFilesArrayCellsCnt = 1;
			file_exist = FALSE;
			line_process_exist = FALSE;
		}

		if (!file_exist) {
			moduleIndex = LogFiledata.unFilesArrayCellsCnt-1;
			strncpy(LogFiledata.taArrayOfModules[moduleIndex].ucModuleNameStr, TempFileName,
					MAX_NAME_LEN-1);
			LogFiledata.taArrayOfModules[moduleIndex].ucModuleNameStr[MAX_NAME_LEN-1] = '\0';
		}

		if (!line_process_exist) {
			if (++LogFiledata.taArrayOfModules[moduleIndex].unLinesArrayCellsCnt >
				MAX_NUMBER_OF_LINES) {
				printf("Overflow too many lines/process for parsing\n");
				LogFiledata.taArrayOfModules[moduleIndex].unLinesArrayCellsCnt--;
				return ERROR;
			}

			LogFiledata.taArrayOfModules[moduleIndex].taArrayOfProcessesLines[lineIndex].unTotalLineProcessRptCnt++;
			lineIndex = LogFiledata.taArrayOfModules[moduleIndex].unLinesArrayCellsCnt-1;
			LogFiledata.taArrayOfModules[moduleIndex].taArrayOfProcessesLines[lineIndex].unLineNumber = tempLineNum;
			LogFiledata.taArrayOfModules[moduleIndex].taArrayOfProcessesLines[lineIndex].ProcessNameId = ProcessNameId;
			/*strncpy(LogFiledata.taArrayOfModules[moduleIndex].taArrayOfProcessesLines[lineIndex].ProcessName,
					TempProcessName,
					MAX_PROCESS_NAME_LEN-1);*/
			// LogFiledata.taArrayOfModules[moduleIndex].taArrayOfProcessesLines[lineIndex].ProcessName[MAX_PROCESS_NAME_LEN-1] = '\0';
		} else {
			if ((moduleIndex>=0) && (moduleIndex<MAX_NUMBER_OF_MODULES) &&
				(lineIndex>=0) && (lineIndex<MAX_NUMBER_OF_LINES))
			LogFiledata.taArrayOfModules[moduleIndex].taArrayOfProcessesLines[lineIndex].unTotalLineProcessRptCnt++;
		}

	} else if (strstr(auLineOfFileStr, ARTLINE)) {
		LogFiledata.unArtLinesCnt++;
	} else if (strstr(auLineOfFileStr, CMLINE)) {
		LogFiledata.unMCLinesCnt++;
	} else if (strstr(auLineOfFileStr, VIDEOLINE)) {
		LogFiledata.unVideoLinesCnt++;
	}

    return TRUE;
}

/********************************************************************************************
   The function scans the LogFiledata and prints to the output file
   the total appearances of each File, process and Line in the log file.
*********************************************************************************************/
void PrintToFile (void)
{
	int file_i, line_i;

	++numberOfRestarting;
	fsOutputFile << "========================================================================\n";

	fsOutputFile << "Total MCMS lines in files = " << LogFiledata.unMcmsLinesCnt << "\n";
	fsOutputFile  << std::left << std::setw(32) << std::setfill(' ') << "Process Name";
	fsOutputFile  << std::left << std::setw(32) << std::setfill(' ') <<  "Module name ";
	fsOutputFile << std::left << std::setw(14) << std::setfill(' ') << "line number ";
	fsOutputFile << "repeated prints \n";

	for (file_i=0; file_i<LogFiledata.unFilesArrayCellsCnt; file_i++)
	{
		for (line_i=0; line_i<LogFiledata.taArrayOfModules[file_i].unLinesArrayCellsCnt; line_i++)
		{
			if ((LogFiledata.taArrayOfModules[file_i].taArrayOfProcessesLines[line_i].unLineNumber != 0) &&
				(LogFiledata.taArrayOfModules[file_i].taArrayOfProcessesLines[line_i].ProcessNameId != 0))
			{
				// print only lines that repeated 10% than overall number
				if (LogFiledata.unMcmsLinesCnt < 50)
				{
					return;
				}
				if (LogFiledata.taArrayOfModules[file_i].taArrayOfProcessesLines[line_i].unTotalLineProcessRptCnt >
					(unsigned int) LogFiledata.unMcmsLinesCnt/50) {
//			2000) { 
//					(unsigned int) LogFiledata.unMcmsLinesCnt/40) {

					std::string currentProcessName = ProcessNameToIdMap.GetStrById(LogFiledata.taArrayOfModules[file_i].taArrayOfProcessesLines[line_i].ProcessNameId);				
					
  					fsOutputFile  << std::left << std::setw(32) << std::setfill(' ') << currentProcessName;
					fsOutputFile  << std::left << std::setw(32) << std::setfill(' ') << LogFiledata.taArrayOfModules[file_i].ucModuleNameStr;
					fsOutputFile << std::left << std::setw(14) << std::setfill(' ') << LogFiledata.taArrayOfModules[file_i].taArrayOfProcessesLines[line_i].unLineNumber;					
					fsOutputFile << LogFiledata.taArrayOfModules[file_i].taArrayOfProcessesLines[line_i].unTotalLineProcessRptCnt << " ("
									<< (int)((double)LogFiledata.taArrayOfModules[file_i].taArrayOfProcessesLines[line_i].unTotalLineProcessRptCnt/(double)LogFiledata.unMcmsLinesCnt*100)
									<< "%)" << std::endl;

              
					}		
			}
		}
	}

	fsOutputFile << "========================================================================\n\n";
}


void	AddToSummary(void)
{	
	DataSummary.unOverAllMcmsLinesCnt += LogFiledata.unMcmsLinesCnt;
	DataSummary.unOverAllArtLinesCnt += LogFiledata.unArtLinesCnt;
	DataSummary.unOverAllVideoLinesCnt += LogFiledata.unVideoLinesCnt;
	DataSummary.unOverAllMCLinesCnt += LogFiledata.unMCLinesCnt;

	DataSummary.unOverallNumLinesDirCnt = DataSummary.unOverAllMcmsLinesCnt + DataSummary.unOverAllArtLinesCnt + DataSummary.unOverAllVideoLinesCnt + DataSummary.unOverAllMCLinesCnt;

}


void print_summary(void) 
{

	fsOutputFile <<  "\n***** Summary: ****\n ";

	if (DataSummary.unOverallNumLinesDirCnt > 0) {
		fsOutputFile << "Total MCMS log lines in directory \t " << DataSummary.unOverAllMcmsLinesCnt << " (" 
		<<(int)((double)DataSummary.unOverAllMcmsLinesCnt/(double)DataSummary.unOverallNumLinesDirCnt*100) << ")\n"

		<< "Total Art log lines in directory \t " << DataSummary.unOverAllArtLinesCnt << " (" 
		<<(int)((double)DataSummary.unOverAllArtLinesCnt/(double)DataSummary.unOverallNumLinesDirCnt*100) << ")\n"

		<< "Total Video log lines in directory \t " << DataSummary.unOverAllVideoLinesCnt << " (" 
		<<(int)((double)DataSummary.unOverAllVideoLinesCnt/(double)DataSummary.unOverallNumLinesDirCnt*100) << ")\n"

		<< "Total Media-Card log lines in directory \t " << DataSummary.unOverAllMCLinesCnt << " (" 
		<<(int)((double)DataSummary.unOverAllMCLinesCnt/(double)DataSummary.unOverallNumLinesDirCnt*100) << ")\n"

		<< "Total log line in directory \t " << DataSummary.unOverallNumLinesDirCnt << "\n";
	 
	} else {
		fsOutputFile << "Empty data!\n ";
	}
	fsOutputFile << "========================================================================\n\n";
}


/*********************************************************************************
   The function gets number of arguments and array of file's names.
   The function parses each file name in the array(starting from cell #1)
   and produce one output file which includes all files statistics.
**********************************************************************************/
int parse_file (char* puInputFile)
{
	FILE         *log_file;
	char         aucLineStr[MAX_LINE_LEN] = "\0";
    char*        puLineValid              = NULL;
    int          result                   = TRUE;
	int          auInputFileIdx           = 1;//Index for file's names array

	strncpy(LogFiledata.LogFileName[LogFiledata.unLogFilesCnt], puInputFile, LOG_FILE_NAME_LEN-1);
	LogFiledata.LogFileName[LogFiledata.unLogFilesCnt][LOG_FILE_NAME_LEN-1] = '\0';
	LogFiledata.unLogFilesCnt++;
	
	log_file = fopen(puInputFile, "r");//Opening log file #i for reading
	if (log_file == NULL) //Checks if succeeded in opening the the file
	{
		printf("Error opening input file! File name: %s\n", puInputFile);
		exit(1);
	}

	printf("\n*********************** Processing file: %s ******\n", puInputFile);

	do
	{
		memset (aucLineStr, '\0', sizeof(aucLineStr));

		puLineValid = Get_Line(aucLineStr, MAX_LINE_LEN, log_file); //Pulls out the first line from the file

		if ( (puLineValid != NULL) && (aucLineStr[0] != '\0'))
		{
			result = SearchForMCMSLogs(aucLineStr);
			if (result == ERROR) {
				break;
			} else if (!result) {
				printf("Error parsing file %s", puInputFile);
				continue;
			} // else SUCCESS

		}
	} while(!feof(log_file));  //Scans each line in the file till the end of the file
	fclose(log_file);
	++auInputFileIdx; //Increase Input file's array index

	return TRUE;
}

int main (int argc, char **argv)
{
	DIR *dp;
	struct dirent *ep;
	char   log_file_name[LOG_FILE_NAME_LEN] = "\0";

	if (argc != 2) {
		printf("usage McmsLogParser <dir_path>");
		exit(0);
	}

	fsOutputFile.open("Output.txt");

	dp = opendir (argv[1]);

	memset(&DataSummary, 0, sizeof(TDataSummary));

	memset(&LogFiledata, 0, sizeof(TData));

	if (dp != NULL)
		{
		while (ep = readdir (dp)) {

			if ( (strncmp(ep->d_name, "Log", strlen("Log")) == 0) &&
				(strstr(ep->d_name, ".log")) ) {
				strncpy(log_file_name, argv[1], sizeof(log_file_name));
				strcat(log_file_name, ep->d_name);
				parse_file(log_file_name);
			}
		}

		(void) closedir (dp);
		AddToSummary();
		PrintToFile(); //Writing the struct to the output file		
		print_summary();
		}
	else {
		 printf ("Couldn't open the directory %s errno: %s\n", argv[1], strerror( errno ) );
		fsOutputFile << "Couldn't open the directory " << argv[1] << " errno: " << strerror( errno ) << "\n";
	}


	std::cout << "numberOfRestarting " << numberOfRestarting << " MAX_NUMBER_OF_MODULES " << MAX_NUMBER_OF_MODULES << std::endl ;


	fsOutputFile.close();
	return 0;
}
