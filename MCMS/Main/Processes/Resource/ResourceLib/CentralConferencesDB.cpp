#include "TraceStream.h"
#include "CentralConferencesDB.h"
#include <fcntl.h>
#include "CRsrvDB.h"
#include "ConfResources.h"
#include "RsrvResources.h"
#include "Reservator.h"
#include "ProfilesDB.h"

#undef min
#include "PrettyTable.h"

////////////////////////////////////////////////////////////////////////////
//                        CCentralConferencesDB
////////////////////////////////////////////////////////////////////////////
CCentralConferencesDB::CCentralConferencesDB()
{
	m_pConfRsrcsDB         = new CConfRsrcDB;
	m_pConfRsrvRsrcs       = new ReservedConferences;
	m_pSleepingConferences = new SleepingConferences;
	m_pProfilesDB          = new CProfilesDB;
	m_pRsrvDB              = NULL; // will be created only if necessary
}

//--------------------------------------------------------------------------
CCentralConferencesDB::~CCentralConferencesDB()
{
	POBJDELETE(m_pConfRsrcsDB);
	POBJDELETE(m_pRsrvDB);
	POBJDELETE(m_pProfilesDB);

	if (m_pConfRsrvRsrcs)
	{
		m_pConfRsrvRsrcs->clear();
		POBJDELETE(m_pConfRsrvRsrcs);
	}

	if (m_pSleepingConferences)
	{
		m_pSleepingConferences->clear();
		PDELETE(m_pSleepingConferences);
	}
}

//--------------------------------------------------------------------------
STATUS CCentralConferencesDB::CreateRsrvDB(CReservator* pReservator)
{
	STATUS status = STATUS_OK;

	m_pRsrvDB = new CRsrvDB(RESERVATION_DATABASE);
	string rsrvFolder = RESERVATION_DB_DIR; // Reservations folder path

	if (open(rsrvFolder.c_str(), O_DIRECTORY) == -1) // Create the Folder only if needed
	{
		if (CreateDirectory(rsrvFolder.c_str()) == FALSE)
		{
			PASSERTMSG(1, "Can not create Reservations directory");
			return STATUS_FAIL;
		}
	}

	status = m_pRsrvDB->InitRsrvDB(rsrvFolder, pReservator);

	return status;
}

//--------------------------------------------------------------------------
void CCentralConferencesDB::DumpUsedSize() const
{
	DWORD numRsrv               = (m_pRsrvDB) ? m_pRsrvDB->GetResNumber() : 0;
	DWORD numConfRsrvRsrcs      = (m_pConfRsrvRsrcs) ? m_pConfRsrvRsrcs->size() : 0;
	DWORD numSleepingConference = (m_pSleepingConferences) ? m_pSleepingConferences->size() : 0;
	DWORD numProfilesDB         = (m_pProfilesDB) ? m_pProfilesDB->GetNumOfProfiles() : 0;

	std::ostringstream msg;
	msg << "NumRsrvDB:" << numRsrv << ", NumConfRsrvRsrcs:" << numConfRsrvRsrcs << ", NumSleepingConferences:" << numSleepingConference << ", numProfilesDB:" << numProfilesDB;

	TRACEINTO << msg.str().c_str();
}
