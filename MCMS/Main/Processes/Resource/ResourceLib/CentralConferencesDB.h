#ifndef CENTRALCONFERENCESDB_H_
#define CENTRALCONFERENCESDB_H_

#include "PObject.h"
#include "StringsMaps.h"
#include <set>

class CRsrvDB;
class CConfRsrcDB;
class CConfRsrvRsrc;
class CSleepingConference;
class CReservator;
class CProfilesDB;

typedef std::set<CConfRsrvRsrc>       ReservedConferences;
typedef std::set<CSleepingConference> SleepingConferences;

////////////////////////////////////////////////////////////////////////////
//                        CCentralConferencesDB
////////////////////////////////////////////////////////////////////////////
class CCentralConferencesDB : public CPObject
{
	CLASS_TYPE_1(CCentralConferencesDB, CPObject)

public:
	                     CCentralConferencesDB();
	virtual             ~CCentralConferencesDB();
	const char*          NameOf() const                 { return "CCentralConferencesDB";}

	CConfRsrcDB*         GetConfRsrcDB() const          { return m_pConfRsrcsDB; }
	CRsrvDB*             GetRsrvDB() const              { return m_pRsrvDB; }
	ReservedConferences* GetConfRsrvRsrcs() const       { return m_pConfRsrvRsrcs; }
	SleepingConferences* GetSleepingConferences() const { return m_pSleepingConferences; }
	CProfilesDB*         GetProfilesDB() const          { return m_pProfilesDB; }

	STATUS               CreateRsrvDB(CReservator* pReservator);

	void                 DumpUsedSize() const;

private:
	// This list includes all the reservations, in a short list, and the connectivity to the hard-disk for retrieving the full details
	// initialized only in case it's with reservator!!!!
	CRsrvDB*             m_pRsrvDB;

	// This list includes all ongoing conferences, with all the list of their resources
	CConfRsrcDB*         m_pConfRsrcsDB;

	// This list includes all reservations + ongoing conferences, with their NIDs, duration, etc
	ReservedConferences* m_pConfRsrvRsrcs;

	// This list includes all Meeting Rooms, Entry Queues and Sip factories
	SleepingConferences* m_pSleepingConferences;

	// This is the list of all profiles, and their maximum weight (for reservation calculation)
	CProfilesDB*         m_pProfilesDB;
};


#endif /*CENTRALCONFERENCESDB_H_*/
