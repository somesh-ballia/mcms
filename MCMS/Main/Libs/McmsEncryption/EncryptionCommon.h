#ifndef __ENCYPTION_COMMON_H__
#define __ENCYPTION_COMMON_H__


#include <map>
#include <sstream>
#include <string>

#include "PObject.h"
#include "DataTypes.h"
#include "SharedDefines.h"
#include "DefinesGeneral.h"
#include "Trace.h"
#include "TraceStream.h"
#include "EncryptionKeyServerAPI.h"
#include "SharedMemoryQueue.h"
#include "SysConfig.h"



#define POLYCOM_DH_GENERATOR 2
#define TANBERG_H320_DH_GENERATOR 5
#define TANBERG_H323_DH_GENERATOR 3

#define ENCRYPTED_KEYS_TABLE_BASE_NAME "EncryptionKeyShm_"

/////////////////////////// class EncyptedSharedMemoryTables;///////////////////////////

class EncryptedKey
{	
public:
	
	void DeSerialize(CSegment &seg);
	

	const BYTE*	GetHalfKey() const {return m_halfKey; } 
	const BYTE*	GetRandomNumber() const {return m_randomNumber;}
	
private:
	BYTE		m_halfKey [HALF_KEY_SIZE];
	BYTE		m_randomNumber [HALF_KEY_SIZE];
	
	friend std::ostream &operator<<( std::ostream &out, const EncryptedKey& encryptedKey );
} ;

/////////////////////////// typedef CSharedMemoryQueue///////////////////////////
typedef CSharedMemoryQueue<EncryptedKey> SharedMemoryEncryptedKeyQueue ; 

/////////////////////////// class EncyptedSharedMemoryTables ///////////////////////////

// yaela TODEL
//  enum PIZZA_TABLE_SIZES{PIZZA_G2=10,PIZZA_G3=10,PIZZA_G5=10};	

enum PIZZA_TABLE_SIZES{PIZZA_G2=200,PIZZA_G3=200,PIZZA_G5=200};

class EncyptedSharedMemoryTables : public CPObject
{
	CLASS_TYPE_1(EncyptedSharedMemoryTables, CPObject)
	
	
	
public:
	
	EncyptedSharedMemoryTables();
	~EncyptedSharedMemoryTables();

	virtual const char* NameOf() const {return "EncyptedSharedMemoryTables";}
	
	STATUS DequeuEncryptedKey(DWORD generator, EncryptedKey & encryptedKey) ;

	STATUS QueueEncryptedKey(DWORD generator, const EncryptedKey & encryptedKey);

    const SharedMemoryEncryptedKeyQueue* GetSharedMemory(DWORD generator) const;
    
    const std::map< DWORD , SharedMemoryEncryptedKeyQueue* >& GetSharedMemoryEncryptedKeyQueueMap() const
    {
    	return m_sharedMemoryEncryptedKeyQueueMap;
    }
    	
private:
	
	std::map< DWORD , SharedMemoryEncryptedKeyQueue* > 	m_sharedMemoryEncryptedKeyQueueMap ;
		
	
	void CreateAllTables();
	void CreateSharedMemoryTable(CSysConfig *pSysConfig, BOOL isSystemTarget, DWORD generatorId, PIZZA_TABLE_SIZES pizzaTableSize, const std::string tableKey);
	void DeleteAllTables();
};


#endif //__ENCYPTION_COMMON_H__
