#ifndef FailoverDefines_H_
#define FailoverDefines_H_

#include "DefinesGeneral.h"
#define MASTER_KEEP_ALIVE_SEND_TIMEOUT		(2*SECOND)

#define FAILOVER_RECONNECT_SOCKET_TIMEOUT	(10*SECOND)

#define FAILOVER_RELOGIN_TIMEOUT	(10*SECOND)

#define FAILOVER_MASTER_PING_TIMEOUT	(10*SECOND)
#define FAILOVER_MASTER_GET_STATE_FROM_SLAVE_TIMEOUT	(10*SECOND)

/*added by Richer for BRIDGE-14263, 2014.7.16*/
#define FAILOVER_INIT_TIMER_TIMEOUT     (10*SECOND)

#define SYNC_ONGOING_CONFERNCES_TIMEOUT		(3*SECOND)
#define SYNC_IP_SERVICE_TIMEOUT				(3*SECOND)

#define FAILOVER_CFG_FILE "Cfg/Failover.xml"

#define FIRST_MSG_ID						1

// ------------------------------------
// -------- SyncedElement enum --------
// ------------------------------------
enum eSyncedElement
{
	eSyncedElement_IpService = 0,
	eSyncedElement_MngmntService,
	eSyncedElement_IvrServices,
	eSyncedElement_ConfProfiles,
	eSyncedElement_MeetingRooms,
	eSyncedElement_OngoingConferences,
	eSyncedElement_Reservations,
	eSyncedElement_RecordingLinks,

	eNUM_OF_SYNCED_ELEMENTS
};

static const char *syncedElementsStr[] =
{
	"IpService",
	"MngmntService",
	"IvrServices",
	"ConfProfiles",
	"MeetingRooms",
	"OngoingConferences",
	"Reservations",
	"RecordingLinks",
};

static const char *GetSyncedElementStr(eSyncedElement syncedElem)
{
	const char *name = (0 <= syncedElem && syncedElem < eNUM_OF_SYNCED_ELEMENTS
						?
						syncedElementsStr[syncedElem] : "Invalid synced element");
	return name;
}
// ------------------------------------

enum eChangeType
{
	eChange_AddOrUpdate = 0,
	eChange_Delete
};


enum eFailoverStatusType
{
	eFailoverStatusNone,
	eFail,
	eAttempting,
	eSynchOk,
};

enum eFailoverType
{
	eFailoverTypeNone=0,
	eMaster,
	eSlave,
};


#endif /*FailoverDefines_H_*/
