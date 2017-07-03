#ifndef _ONGOING_CONF_STORE_H_
#define _ONGOING_CONF_STORE_H_

#include "SerializeObject.h"
#include "FileManager.h"
#include "StatusesGeneral.h"
#include <vector>
#include "FaultsDefines.h"
//#include "ConfPartyDefines.h"
#include <map>
#include "AllocateStructs.h"
#include "Trace.h"
#include "PObject.h"

#define ONGOING_CONF_STORE_DB_DIR "Cfg/OngoingConfStore/"
#define ONGOING_CONF_STORE (COngoingConfStore::GetInstance())

class CCommRes;
class CCommResApi;

typedef std::vector<CCommResApi*> STORED_CONFS_ARRAY;
typedef std::map<DWORD, std::string> STORED_CONFIDS_FILES;
typedef std::map<std::string, std::string> STORED_CONFNAMES_FILES;

class COngoingConfStore : public CPObject
{
	CLASS_TYPE_1(CCommConfStoreDB,CPObject)

	static COngoingConfStore* m_pInstance;
	COngoingConfStore();
	virtual ~COngoingConfStore();

public:
	static COngoingConfStore* GetInstance();
	const char*  NameOf() const;

	friend std::ostream& operator<< (std::ostream& os, COngoingConfStore& obj );
	void Dump(std::ostream& os);

	STATUS InitOngoingConfStore();
	STATUS Add(CCommResApi*  pConfRes);
	STATUS Remove(const DWORD  dwConfMonitorID);

private:
	void ClearCommResVector(std::vector< CCommResApi *> & vect);
	void ClearVector();

	CFileManager<CCommResApi> * m_pFileManager;
	STORED_CONFS_ARRAY m_arrStoredConfs;
	STORED_CONFIDS_FILES m_mapConfIdFileName;
	STORED_CONFNAMES_FILES m_mapConfNameFileName;

	friend class CSelfConsistency;
};


#endif
