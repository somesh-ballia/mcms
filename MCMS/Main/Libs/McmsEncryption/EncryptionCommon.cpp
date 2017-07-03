#include "EncryptionCommon.h"

#include <stdio.h>
#include "StatusesGeneral.h"
#include "StructTm.h"
#include "Macros.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"


std::ostream &operator<<( std::ostream &out, const EncryptedKey& encryptedKey )
{	
	char        temp[16];
	out << "m_halfKey: {";
	for(DWORD i=0;i < SHARED_SECRET_LENGTH;++i)
	{
		sprintf(temp," 0x%02x",(unsigned char)(encryptedKey.m_halfKey[i]));
		out << (char *) temp;
		
	}
	out << "}\n";

	out << "m_randomNumber: {";
	for(DWORD i=0;i < SHARED_SECRET_LENGTH;++i)
	{
		sprintf(temp," 0x%02x",(unsigned char)(encryptedKey.m_randomNumber[i]));
		out << (char *) temp;		
	}
	out << "}\n";
	
	return out;
}

void EncryptedKey::DeSerialize(CSegment &seg)
{
	for(DWORD i=0; i<SHARED_SECRET_LENGTH; ++i)
	{	
		seg>> m_halfKey[i];
	}	
	for(DWORD i=0; i<SHARED_SECRET_LENGTH; ++i)
	{	
		seg>> m_randomNumber[i];
	}
}


EncyptedSharedMemoryTables::EncyptedSharedMemoryTables()
{
	CreateAllTables();
}
	
EncyptedSharedMemoryTables::~EncyptedSharedMemoryTables()
{
	DeleteAllTables();
}
const SharedMemoryEncryptedKeyQueue* EncyptedSharedMemoryTables::GetSharedMemory(DWORD generator) const
{
	std::map< DWORD , SharedMemoryEncryptedKeyQueue* >::const_iterator it = m_sharedMemoryEncryptedKeyQueueMap.find(generator);		
	if (it == m_sharedMemoryEncryptedKeyQueueMap.end())
	{
		//TRACESTR(eLevelInfoNormal) <<  "GetSharedMempry no shared memepry for generator  " <<  generator;
		return (const SharedMemoryEncryptedKeyQueue*)NULL;
	}
	return (const SharedMemoryEncryptedKeyQueue*)it->second;
	
}

STATUS EncyptedSharedMemoryTables::DequeuEncryptedKey(DWORD generator, EncryptedKey & encryptedKey) 
{	
	std::map< DWORD , SharedMemoryEncryptedKeyQueue* >::iterator it = m_sharedMemoryEncryptedKeyQueueMap.find(generator);		
	if (it == m_sharedMemoryEncryptedKeyQueueMap.end())
	{		 
       //FTRACEINTOFUNC <<  "GetEncryptedKey no table for this generator ID " <<  generator;
		TRACESTR(eLevelInfoNormal) <<  "GetEncryptedKey no table for this generator ID " <<  generator;
       return STATUS_FAIL;
	}	
	return it->second->Dequeue(encryptedKey);
}

STATUS EncyptedSharedMemoryTables::QueueEncryptedKey(DWORD generator, const EncryptedKey & encryptedKey) 
{
	std::map< DWORD , SharedMemoryEncryptedKeyQueue* >::iterator it = m_sharedMemoryEncryptedKeyQueueMap.find(generator);		
	if (it == m_sharedMemoryEncryptedKeyQueueMap.end())
	{		
		TRACESTR(eLevelInfoNormal) <<  "GetEncryptedKey no table for this generator ID " <<  generator;		
		return STATUS_FAIL;
	}
	return it->second->Queue(encryptedKey);	
}	

void EncyptedSharedMemoryTables::CreateAllTables()
{
	CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();	
	BOOL isSystemTarget = IsTarget();		
		
	TRACESTR(eLevelInfoNormal) << "Creating / attaching POLYCOM_DH_GENERATOR shared memory. Pizza size: " <<  PIZZA_G2 << " System key: " << "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_POLYCOM" << " isSystemTarget: " << (WORD)isSystemTarget << "\n";
	CreateSharedMemoryTable(pSysConfig, isSystemTarget, POLYCOM_DH_GENERATOR, PIZZA_G2, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_POLYCOM");
	
	TRACESTR(eLevelInfoNormal) << "Creating / attaching TANBERG_H320_DH_GENERATOR shared memory. Pizza size: " <<  PIZZA_G5 << " System key: " << "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_ISDN" << " isSystemTarget: " << (WORD)isSystemTarget << "\n";
	CreateSharedMemoryTable(pSysConfig, isSystemTarget, TANBERG_H320_DH_GENERATOR, PIZZA_G5, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_ISDN");
	
	TRACESTR(eLevelInfoNormal) << "Creating / attaching TANBERG_H323_DH_GENERATOR shared memory. Pizza size: " <<  PIZZA_G3 << " System key: " << "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_IP" << " isSystemTarget: " << (WORD)isSystemTarget << "\n";
	CreateSharedMemoryTable(pSysConfig, isSystemTarget, TANBERG_H323_DH_GENERATOR, PIZZA_G3, "SIZE_OF_ENCRYPTION_KEY_DATABASE_FOR_TANDBERG_IP");		
}


void EncyptedSharedMemoryTables::CreateSharedMemoryTable(CSysConfig *pSysConfig, BOOL isSystemTarget, DWORD generatorId, PIZZA_TABLE_SIZES pizzaTableSize, const std::string tableKey)
{	
		
	DWORD tableSize = (DWORD)pizzaTableSize;;	
		
	if (isSystemTarget != NO)
	{
		if (pSysConfig->GetDWORDDataByKey(tableKey, tableSize) == FALSE)
		{
			TRACESTR(eLevelError) << "Failed to get system key " << tableKey;			
		}				 		
	}
	
	std::ostringstream  tableSharedMemName;
	tableSharedMemName <<  ENCRYPTED_KEYS_TABLE_BASE_NAME << generatorId;
			
	SharedMemoryEncryptedKeyQueue*  pSharedMemoryQueue = new SharedMemoryEncryptedKeyQueue (tableSharedMemName.str().c_str(),
	    		               					1,
	    		               					tableSize /*+ 2*/);
	
	TRACESTR(eLevelInfoNormal) << "Created SharedMemoryEncryptedKeyQueue name: " <<  tableSharedMemName.str() << " size " << tableSize  << " IsCreator " << (int)pSharedMemoryQueue->IsCreator();
	
	m_sharedMemoryEncryptedKeyQueueMap[generatorId] = pSharedMemoryQueue;
	
}
void EncyptedSharedMemoryTables::DeleteAllTables()
{
	SharedMemoryEncryptedKeyQueue*  pSharedMemoryQueue = NULL;
	pSharedMemoryQueue = m_sharedMemoryEncryptedKeyQueueMap[POLYCOM_DH_GENERATOR];
	POBJDELETE(pSharedMemoryQueue);
	pSharedMemoryQueue = m_sharedMemoryEncryptedKeyQueueMap[TANBERG_H320_DH_GENERATOR];
	POBJDELETE(pSharedMemoryQueue);
	pSharedMemoryQueue = m_sharedMemoryEncryptedKeyQueueMap[TANBERG_H323_DH_GENERATOR];
	POBJDELETE(pSharedMemoryQueue);
}


