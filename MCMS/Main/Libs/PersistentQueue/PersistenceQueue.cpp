#include "PersistenceQueue.h"
#include "OsFileIF.h"

#define FILE_BACKUP_TIMER 1008
#define BACKUP_TIMEOUT (5 * SECOND)

PBEGIN_MESSAGE_MAP(CPersistenceQueue)
ONEVENT(FILE_BACKUP_TIMER  ,ANYCASE , CPersistenceQueue::OnTimerPeriodicBackup)
PEND_MESSAGE_MAP(CPersistenceQueue,CStateMachine);

/////////////////////////////////////////////////////////////////////////////
CPersistenceQueue::CPersistenceQueue(char* nameOfFile, unsigned int maxItemsInQueue, int maxNumOfFiles)

{
	m_maxItemsInQueue = maxItemsInQueue;
	m_maxNumOfFiles = maxNumOfFiles;
	m_nameOfFile = nameOfFile;
	m_isBackupFileShouldSaved = false;
	m_isPossibleToAddItem = true;
	SetPrefixDirectory();
	CreateDirectoryForBakcupFiles();
	//case that rmx recovers from reset and it need to read backup files
	char path[255];
	memset(path, '\0', 255);
	//sprintf(path,"%s%s",m_prefix_directory.c_str(),m_nameOfFile);
	snprintf(path,sizeof(path) -1,"%s%s",m_prefix_directory.c_str(),m_nameOfFile);
	TRACEINTO << " CPersistenceQueue "<< path;
//	printf("siiiiiiiiize %d",sizeof(m_prefix_directory) + sizeof(m_nameOfFile));
	m_numOfFilesCreated = GetDirFilesNum(path);
	SetPeriodBackupFile();
	StartTimer(FILE_BACKUP_TIMER,BACKUP_TIMEOUT);
}


/////////////////////////////////////////////////////////////////////////////
CPersistenceQueue::~CPersistenceQueue()
{
	//add following lines for cleaning Backup Directory

	/*printf("CPersistenceQueueCPersistenceQueueCPersistenceQueueCPersistenceQueue");
	char cmd[255];
	sprintf(cmd,"rm -r %s%s",
			m_prefix_directory,
			m_nameOfFile);

	std::string ans;
	SystemPipedCommand(cmd, ans);*/
}
//=====================================================================================================================================//
void CPersistenceQueue::HandleEvent (CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}
//=====================================================================================================================================//
void* CPersistenceQueue::GetMessageMap()
{
	return (void*)m_msgEntries;
}
/////////////////////////////////////////////////////////////////////////////
void  CPersistenceQueue::Dump(std::ostream& msg) const
{
	int x;
	msg << "CPersistenceQueue::Dump\n"
			<< "--------------\n";

	CPObject::Dump(msg);
}

void CPersistenceQueue::CreateDirectoryForBakcupFiles()
{
	char path[255];
	memset(path, '\0', 255);
	snprintf(path, sizeof(path) -1, "%s%s",m_prefix_directory.c_str(),m_nameOfFile);


	//read. write and not execute for root and owner (but not others)
	CreateDirectory(path,0777);
}
////////////////////////////////////////////////////////////////////////////
void CPersistenceQueue::AddItem(ApiBaseObjectPtr& item)
{
	if(!IsPossilbeToAddItem())
	{
		//TODO add and remove positive and negative fault
		return;
	}
	// if q.size is max - save to file
	if(m_maxItemsInQueue > m_itemsList.m_PersistList.size())
	{
		m_itemsList.m_PersistList.push_back(item);
		m_isBackupFileShouldSaved = true;
	}
	else
	{
		AddItemWhenQueueIsFull(item);
	}
}

////////////////////////////////////////////////////////////////////////////
bool CPersistenceQueue::IsPossilbeToAddItem()
{
	return m_isPossibleToAddItem;
}
////////////////////////////////////////////////////////////////////////////
void CPersistenceQueue::EnableAddingItem()
{
	m_isPossibleToAddItem = true;
}
////////////////////////////////////////////////////////////////////////////
void CPersistenceQueue::DisableAddingItem()
{
	m_isPossibleToAddItem = false;
}

////////////////////////////////////////////////////////////////////////////
void  CPersistenceQueue::AddItemWhenQueueIsFull(ApiBaseObjectPtr& item)
{
	if(m_numOfFilesCreated < m_maxNumOfFiles)
	{
		char fileName[255];
		memset(fileName, '\0', 255);
		CreateFileName(fileName);
		bool saveTofileSucceed = SaveListToFile(fileName);
		//TODO: add assert if saveTofileSucceed - false (use condition assert)
		CleanList();
		if(IsFileExists(m_periodBackupFileCopmressed))
		{
			DeleteFile(m_periodBackupFileCopmressed);
		}
		m_itemsList.m_PersistList.push_back(item);
		m_isBackupFileShouldSaved = true;
		m_numOfFilesCreated++;
	}
	else
	{
		//TODO: assert once instead
		TRACEINTO << " AddItemWhenQueueIsFull can't add item - got to Max backup files for Queue";
	}
}
void CPersistenceQueue::CreateFileName(char* fileName)
{
	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));

	snprintf(fileName, 254,
			"%s%s/%s_%02d-%02d-%04d_%02d-%02d-%02d_%d.txt",
			m_prefix_directory.c_str(),
			m_nameOfFile,
			m_nameOfFile,
			curTime.m_day,
			curTime.m_mon,
			curTime.m_year,
			curTime.m_hour,
			curTime.m_min,
			curTime.m_sec,
			m_numOfFilesCreated);

}
////////////////////////////////////////////////////////////////////////////
bool CPersistenceQueue::SaveListToFile(std::string fileName, bool bOverride)
{

	bool ans= m_itemsList.WriteToXmlFile(fileName, bOverride);
	CompressFile(fileName);
	return ans;
}
////////////////////////////////////////////////////////////////////////////
bool CPersistenceQueue::ReadNextFileToList()
{
	std::string fileToReadFrom;
	GetFirstFileToRead(&fileToReadFrom);

	std::size_t found = fileToReadFrom.find(".tar.gz");
	if (found!=std::string::npos)
	{
		fileToReadFrom =  ExtractFile (fileToReadFrom);
	}
	m_currentFileToSaveName = fileToReadFrom;
	bool isReadSucceed = m_itemsFromFile.ReadFromXmlFile(fileToReadFrom);
	//todo:ReadNextFileToList add assert if isReadSucceed is false
	return isReadSucceed;
}

std::string CPersistenceQueue::ExtractFile(string fileName)
{
	//1.extract file
	string Cmd = "echo -n `dirname ";
	Cmd +=fileName;
	Cmd +="`";
	string DirecotryName;
	SystemPipedCommand(Cmd.c_str(), DirecotryName);
	Cmd ="cd " ;
	Cmd += DirecotryName;
	Cmd += " && echo -n `tar xzvf ";
	Cmd +=fileName;
	Cmd +="`";
	string ans;
	SystemPipedCommand(Cmd.c_str(), ans);
	string extractedFileName = DirecotryName;
	extractedFileName += "/";
	extractedFileName += ans;
	//2. delete compressed file
	Cmd = "rm ";
	Cmd +=fileName;

	SystemPipedCommand(Cmd.c_str(), ans);
	return extractedFileName;
}
void CPersistenceQueue::GetFirstFileToRead(std::string* firstFile)
{
	char tempst[255];
	memset(tempst, '\0', 255);
	snprintf(tempst, sizeof(tempst) - 1,"echo -n `ls %s%s/ -lt | tail -1 | awk '{print $9}'`",
			m_prefix_directory.c_str(),
			m_nameOfFile);

	std::string ans;
	//std::string cmd = "ls " << path <<" -lt | tail -1 | awk '{print $9}'";
	SystemPipedCommand(tempst, ans);

	snprintf(tempst, sizeof(tempst) - 1 ,"%s%s/%s",
			m_prefix_directory.c_str(),
			m_nameOfFile,
			ans.c_str());
	*firstFile = tempst;
}

void CPersistenceQueue::CleanList()
{
	m_itemsList.m_PersistList.clear();
}

bool CPersistenceQueue::IsEmpty()
{
	bool isFileToLoadExists = IsFileToLoadExists();
	bool isMemoryListEmpty = IsMemoryListEmpty();
	bool isQueueFromFileEmpty = IsQueueFromFileEmpty();
	if(isFileToLoadExists == false && isMemoryListEmpty == true && isQueueFromFileEmpty == true)
		return true;
	else
		return false;
}

void CPersistenceQueue::DumpList()
{
	std::list<ApiBaseObjectPtr>::iterator itr = m_itemsList.m_PersistList.begin();
	while (itr != m_itemsList.m_PersistList.end())
	{
		itr++;
	}
}
void CPersistenceQueue::DumpFiles()
{
	PersistenceList listOfItemsToPrint;
	bool isReadSucceed = listOfItemsToPrint.ReadFromXmlFile(m_currentFileToSaveName);

	if(isReadSucceed == false)
		return;

	std::list<ApiBaseObjectPtr>::iterator itr = listOfItemsToPrint.m_PersistList.begin();

	while (itr != listOfItemsToPrint.m_PersistList.end())
	{
		itr++;
	}

}
bool CPersistenceQueue::IsFileToLoadExists()
{
	if(m_numOfFilesCreated > 0)
		return true;
	else
		return false;
}
ePersistenceQueueReadFromSource CPersistenceQueue::GetSourceToReadFrom()
{
	bool isFileToLoadExists = IsFileToLoadExists();
	bool isMemoryListEmpty = IsMemoryListEmpty();
	bool isQueueFromFileEmpty = IsQueueFromFileEmpty();
	if(isFileToLoadExists == false && isMemoryListEmpty == true && isQueueFromFileEmpty == true)
		return eEmptyQueue;
	else if(isFileToLoadExists == false && isMemoryListEmpty == false && isQueueFromFileEmpty == true)
		return eReadFromMemoryQueue;
	else if(isFileToLoadExists == true  && isQueueFromFileEmpty == true)
		return eReadFromFileToQueue;
	else if(isFileToLoadExists == true && isQueueFromFileEmpty == false)
		return eReadFromTempQueue;
	else
		return eEmptyQueue;
}

ApiBaseObjectPtr* CPersistenceQueue::Peek()
{
	m_sourceToReadFrom = GetSourceToReadFrom();

	switch(m_sourceToReadFrom)
	{
		case eEmptyQueue:
			return NULL;
			break;
		case eReadFromMemoryQueue:
			return  &(m_itemsList.m_PersistList.front());
			break;
		case eReadFromFileToQueue:
			return HandleCaseReadFormFileToQueue();
			break;
		case eReadFromTempQueue:
			return GetFirstItemFromFilesQueue();
			break;

		default:
		{
			return NULL;
		}
	}
}

//after Peek done and get an Ack, need to pop the read item
void CPersistenceQueue::PopAfterPeek()
{
	switch(m_sourceToReadFrom)
	{
	case eEmptyQueue:
		break;
	case eReadFromMemoryQueue:
		ReadFromMemoryQueue();
		break;
	case eReadFromFileToQueue:
	case eReadFromTempQueue:
		PopFirstItemFromFilesQueue();
		break;
	}

}

//use this function in order to get next item & pop it
void  CPersistenceQueue::ReadFromMemoryQueue()
{
	m_itemsList.m_PersistList.pop_front();
	if(m_itemsList.m_PersistList.empty())
	{
		//remove periodic flie if there the q is empty
		if(IsFileExists (m_periodBackupFileCopmressed))
		{
			DeleteFile(m_periodBackupFileCopmressed);
		}
		//no need to save backup file when q is empty
		m_isBackupFileShouldSaved = false;
	}
	else
	{
		//queue changed - need to backup file
		m_isBackupFileShouldSaved = true;
	}
}
//use this function in order to get next item & pop it
ApiBaseObjectPtr* CPersistenceQueue::Pop()
{
	ApiBaseObjectPtr* peekedItem = Peek();
	PopAfterPeek();
	return peekedItem;
}
ApiBaseObjectPtr* CPersistenceQueue::GetFirstItemFromFilesQueue()
{
	return  &(m_itemsFromFile.m_PersistList.front());
}

void CPersistenceQueue::PopFirstItemFromFilesQueue()
{
	m_itemsFromFile.m_PersistList.pop_front();
	if(IsQueueFromFileEmpty())
		handleQueueFromFileisEmpty();
}

ApiBaseObjectPtr* CPersistenceQueue::HandleCaseReadFormFileToQueue()
{
	bool isReadSucceed = ReadNextFileToList();
	if (isReadSucceed == false)
	{
		return NULL;
	}
	return GetFirstItemFromFilesQueue();
}



void CPersistenceQueue::handleQueueFromFileisEmpty()
{
	//Delete file
	DeleteFile(m_currentFileToSaveName, true);
	m_numOfFilesCreated--;
	if(IsFileToLoadExists())
		ReadNextFileToList();

}
void CPersistenceQueue::SetPeriodBackupFile()
{
	m_periodBackupFile = m_prefix_directory;
	m_periodBackupFile += m_nameOfFile;
	m_periodBackupFile += '/';
	m_periodBackupFile += PERIODIC_BACKUP_FILENAME;

	m_periodBackupFileCopmressed = m_prefix_directory;
	m_periodBackupFileCopmressed += m_nameOfFile;
	m_periodBackupFileCopmressed += '/';
	m_periodBackupFileCopmressed += PERIODIC_BACKUP_FILENAME_COMPRESSED;
}
void CPersistenceQueue::OnTimerPeriodicBackup()
{
	if(m_isBackupFileShouldSaved == true)
	{
		if(IsFileExists (m_periodBackupFileCopmressed))
		{
			DeleteFile(m_periodBackupFileCopmressed);
		}
		SaveListToFile(m_periodBackupFile, true);
		m_isBackupFileShouldSaved = false;

	}
	StartTimer(FILE_BACKUP_TIMER,BACKUP_TIMEOUT);

}
void CPersistenceQueue::SetPrefixDirectory()
{
	if(TRUE == IsRmxSimulation())
	{
		m_prefix_directory = MCU_TMP_DIR;
	}
	else
	{
		m_prefix_directory = MCU_OUTPUT_TMP_DIR;
	}
}
bool CPersistenceQueue::CompressFile(string fileName)
{
	//1. DirecotryName get directory (example: MCU_TMP_DIR/cdr)
	string Cmd = "echo -n `dirname ";
	Cmd +=fileName;
	Cmd +="`";
	string DirecotryName;
	SystemPipedCommand(Cmd.c_str(), DirecotryName);

    //2. get basename (exmaple: periodic_backup.txt)
	Cmd = "echo -n `basename ";
	Cmd +=fileName;
	Cmd +="`";
	string basename;
	SystemPipedCommand(Cmd.c_str(), basename);

	//3. get new newfilename (example: MCU_TMP_DIR/cdr/periodic_backup.tar.gz)
	string newfilename = fileName;
	newfilename.erase(newfilename.end()-4 , newfilename.end());
	newfilename += ".tar.gz";
	//4. execute  exmaple: cd MCU_TMP_DIR/cdr/ && tar cz periodic_backup.txt > MCU_TMP_DIR/cdr/periodic_backup.tar.gz
	Cmd = "cd ";
	Cmd += DirecotryName;
	Cmd += " && tar cz ";
	Cmd += basename;
	Cmd += " > ";
	Cmd += newfilename;
	string ans;
	SystemPipedCommand(Cmd.c_str(), ans);

	//5. delete origin file
	Cmd = "rm  " + fileName;
	SystemPipedCommand(Cmd.c_str(), ans);
	return true;
}
