/*
 * PersistenceQueue.h
 *
 *  Created on: Jun 23, 2013
 *      Author: Ofir Nissel
 */

#ifndef __PersistenceQueue_H__
#define __PersistenceQueue_H__

#include "PObject.h"
#include "DataTypes.h"
#include "Trace.h"
#include "TraceStream.h"
#include <queue>
#include <string>
#include "StructTm.h"
#include "SystemFunctions.h"
#include "StateMachine.h"
#include "PersistenceList.h"

const char* PERIODIC_BACKUP_FILENAME = "periodic_backup.txt";
const char* PERIODIC_BACKUP_FILENAME_COMPRESSED = "periodic_backup.tar.gz";

enum ePersistenceQueueReadFromSource
{
	eEmptyQueue,
	eReadFromMemoryQueue,
	eReadFromFileToQueue,
	eReadFromTempQueue,
};

class CPersistenceQueue : public CStateMachine
{
	CLASS_TYPE_1(CPersistenceQueue, CStateMachine)

public:
	// Constructors
	CPersistenceQueue(char*  nameOfFile, unsigned int maxItemsInQueue = 1000, int maxNumOfFiles = 100000);
	virtual            ~CPersistenceQueue();
	virtual const char* NameOf() const { return "CPersistenceQueue"; }
	virtual void*  GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	void  Dump(std::ostream& msg) const;
	void AddItem(ApiBaseObjectPtr& item);
	void AddItemWhenQueueIsFull(ApiBaseObjectPtr& item);
	bool SaveListToFile(std::string fileName, bool bOverride = false);
	bool ReadNextFileToList();
	void CleanList();
	void DumpList();
	void DumpFiles();
	bool IsFileToLoadExists();
	bool IsMemoryListEmpty() {return m_itemsList.m_PersistList.empty();}
	bool IsQueueFromFileEmpty() {return m_itemsFromFile.m_PersistList.empty();}
	ApiBaseObjectPtr* HandleCaseReadFormFileToQueue();
	void handleQueueFromFileisEmpty();
	void PopFirstItemFromFilesQueue();
	std::string ExtractFile(string fileName);
	bool IsEmpty();
	ApiBaseObjectPtr* Peek();
	void PopAfterPeek();
	void ReadFromMemoryQueue();
	ApiBaseObjectPtr* Pop();
	ApiBaseObjectPtr* GetFirstItemFromFilesQueue();
	//const char* CreateFileName();
	void CreateFileName(char* fileName);
	void GetFirstFileToRead(std::string* firstFile);
	void CreateDirectoryForBakcupFiles();
	PersistenceList GetInternalQueue(){return m_itemsList;}
	void SetPeriodBackupFile();
	void SetPrefixDirectory();
	bool CompressFile(string fileName);
	bool ExtracTarFile(string fileName);
	bool IsPossilbeToAddItem();
	void DisableAddingItem();
	void EnableAddingItem();

private:
	unsigned int m_maxItemsInQueue;
	int m_maxFileSize;
	int m_maxNumOfFiles;
	int m_numOfFilesCreated;
	bool m_isBackupFileShouldSaved;
	PersistenceList m_itemsList;
	PersistenceList m_itemsFromFile;
	std::string m_currentFileToSaveName;
	char*  m_nameOfFile;
	std::string m_prefix_directory;
	std::string  m_periodBackupFile;
	std::string  m_periodBackupFileCopmressed;
	bool m_isPossibleToAddItem;
	ePersistenceQueueReadFromSource m_sourceToReadFrom;
	ePersistenceQueueReadFromSource GetSourceToReadFrom();


protected:
	void OnTimerPeriodicBackup();
	PDECLAR_MESSAGE_MAP

};



#endif /* __PersistenceQueue_H__ */




