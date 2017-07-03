#include "CRsrvDB.h"
#include "CommResRsrvShort.h"
#include "CommResApi.h"
#include "psosxml.h"
#include <utility>
#include "TraceStream.h"
#include "Reservator.h"
#include "HlogApi.h"
#include "HelperFuncs.h"

struct SAscendingMonitorIdSort
{
	bool operator()(CCommResApi* const& rpStart, CCommResApi* const& rpEnd)
	{
		return rpStart->GetMonitorConfId() < rpEnd->GetMonitorConfId();
	}
};

////////////////////////////////////////////////////////////////////////////
//                        CRsrvDB
////////////////////////////////////////////////////////////////////////////
CRsrvDB::CRsrvDB(WORD DBtype)
	: CSerializeObject()
    , m_DBtype(DBtype)
    , m_DBCounter(0)
	, m_dwSummaryUpdateCounter(0)
	, m_LastDeletedIndex(0)
	, m_pFileManager(0)

{
	m_prefixMap[PROFILES_DATABASE]     = "profile_";
	m_prefixMap[MEETING_ROOM_DATABASE] = "meeting_room_";
	m_prefixMap[RESERVATION_DATABASE]  = "reservation_";

	memset(m_DeletedCounterHistory, 0, sizeof(m_DeletedCounterHistory));
	memset(m_DeletedIdHistory, 0, sizeof(m_DeletedIdHistory));
}

//--------------------------------------------------------------------------
CRsrvDB::~CRsrvDB()
{
	ClearVector();

	POBJDELETE(m_pFileManager);
}

//--------------------------------------------------------------------------
int CRsrvDB::DeSerializeXml(CXMLDOMElement* pListNode, char* pszError, const char* action)
{
	PASSERTMSG_AND_RETURN_VALUE(1, "Failed, This code should not be invoked", STATUS_FAIL);
}

//--------------------------------------------------------------------------
void CRsrvDB::SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken)
{
	bool            bChanged = false;
	DWORD           ResSummeryCounter = 0xFFFFFFFF;
	CXMLDOMElement* pSumListNode = NULL;

	if (m_DBtype == MEETING_ROOM_DATABASE)
		pSumListNode = pActionNode->AddChildNode("MEETING_ROOM_SUMMARY_LS");
	else if (m_DBtype == PROFILES_DATABASE)
		pSumListNode = pActionNode->AddChildNode("PROFILE_SUMMARY_LS");
	else if (m_DBtype == RESERVATION_DATABASE)
		pSumListNode = pActionNode->AddChildNode("RES_SUMMARY_LS");
	else
	{
		PASSERT(1);
		return;
	}

	pSumListNode->AddChildNode("OBJ_TOKEN", m_dwSummaryUpdateCounter);

	if (ObjToken == 0xFFFFFFFF)
		bChanged = true;
	else
	{
		// in case the string can not be converted ResSummeryCounter=0
		// the conferences will be added as if the user sent -1 in the object token .
		ResSummeryCounter = ObjToken;
		if (m_dwSummaryUpdateCounter > ResSummeryCounter)
			bChanged = true;
	}

	if (!bChanged)
	{
		pSumListNode->AddChildNode("CHANGED", FALSE, _BOOL);
		return;
	}

	CXMLDOMElement* pChangedNode = pSumListNode->AddChildNode("CHANGED", TRUE, _BOOL);
	CXMLDOMElement* pDeletedNode = pSumListNode->AddChildNode("DELETED_RES_LIST");
	for (int i = 0; i < HISTORY_SIZE; i++)
	{
		if (m_DeletedCounterHistory[i] > ResSummeryCounter && ResSummeryCounter != 0xFFFFFFFF)
		{
			pDeletedNode->AddChildNode("ID", m_DeletedIdHistory[i]);
		}
	}

	for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
	{
		PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(*it));

		int opcode = CONF_NOT_CHANGED;
		if ((int)(*it)->GetSummeryCreationUpdateCounter() > (int)ResSummeryCounter)
			opcode = CONF_COMPLETE_INFO;
		else if ((int)(*it)->GetSummeryUpdateCounter() > (int)ResSummeryCounter)
			opcode = CONF_FAST_PLUS_SLOW_INFO;

		(*it)->SerializeXml(pSumListNode, opcode, m_DBtype);
	}
}

//--------------------------------------------------------------------------
STATUS CRsrvDB::Add(CCommResApi& otherResConf, bool isResFromProfilesFolder)
{
	PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(&otherResConf), STATUS_FAIL);
	PASSERT_AND_RETURN_VALUE(!m_pFileManager, STATUS_FAIL);

	CCommResRsrvShort* shortRes = new CCommResRsrvShort;
	UpdateSchortResWithLongResData(shortRes, otherResConf);

	// Every new reservation enters with operator mask = TRUE
	COPERMASKONINRSRVSHORT(shortRes);

	if (FindName(otherResConf.GetName()) != m_reservArray.end())
	{
		POBJDELETE(shortRes);
		if (m_DBtype == PROFILES_DATABASE)
			return STATUS_PROFILE_NAME_ALREADY_EXISTS;
		else if (m_DBtype == RESERVATION_DATABASE)
			return STATUS_RESERVATION_NAME_EXISTS;
		else
			return STATUS_RESERVATION_NAME_EXISTS_IN_MEETING_ROOM_DB;
	}

	/* Write to file only if the file comes from the operator
	* (a file can come in the init stage when reading all files from folder,
	* in this case no need to write to the disk again */

	std::string uniqueFileName = otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype]);
	if (!isResFromProfilesFolder && m_pFileManager->AddFileData(otherResConf, uniqueFileName) != STATUS_OK)
	{
		POBJDELETE(shortRes);
		PASSERTSTREAM_AND_RETURN_VALUE(1, "Cannot add the CCommResApi file:" << uniqueFileName, STATUS_FAIL);
	}

	m_reservArray.push_back(shortRes);
	IncreaseSummaryUpdateCounter();
	shortRes->SetSummeryCreationUpdateCounter(GetSummaryUpdateCounter());

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CRsrvDB::UpdateSchortResWithLongResData(CCommResRsrvShort* shortRes, CCommResApi& otherResConf)
{
	DWORD bIsCordinateModes = STATUS_FAIL;

	shortRes->SetFileUniqueName(otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype]));
	shortRes->SetName(otherResConf.GetName());
	shortRes->SetDisplayName(otherResConf.GetDisplayName());
	shortRes->SetPassw(otherResConf.GetH243Password());
	shortRes->SetConferenceId(otherResConf.GetMonitorConfId());
	shortRes->SetRepSchedulingId(otherResConf.GetRepSchedulingId());
	shortRes->SetStartTime(*otherResConf.GetStartTime());
	shortRes->SetDurationTime(*otherResConf.GetDuration());
	shortRes->SetEntryPassword(otherResConf.GetEntryPassword());
	shortRes->SetNumericConfId(otherResConf.GetNumericConfId());
	shortRes->SetNumUndefParties(otherResConf.GetNumUndefParties());
	shortRes->SetAdHocProfileId(otherResConf.GetAdHocProfileId());
	shortRes->SetMeetingRoomState(otherResConf.GetMeetingRoomState());

	CServicePhoneStr* pServicePhoneStr = otherResConf.GetFirstServicePhone();
	while (pServicePhoneStr)
	{
		shortRes->AddServicePhone(*pServicePhoneStr);
		pServicePhoneStr = otherResConf.GetNextServicePhone();
	}

	shortRes->SetRsrvFlags(otherResConf.CalcRsrvFlags());
  shortRes->SetRsrvFlags2(otherResConf.CalcRsrvFlags2());
	shortRes->SetEncryptionType(otherResConf.GetEncryptionType());
	shortRes->SetOperatorConf(otherResConf.GetOperatorConf());

	CReservator* pReservator = CHelperFuncs::GetReservator(); // Status Field
	PASSERT(!pReservator);
	if (pReservator)
		bIsCordinateModes = pReservator->CheckReservationSysMode(otherResConf);

	if (bIsCordinateModes != STATUS_OK)
		shortRes->SetStatus(eWrongSysMode);                     // Status Field
	else
	{
		if (shortRes->GetStatus() != eStsSUSPEND)
			shortRes->SetStatus(eStsOK);
	}
}

//--------------------------------------------------------------------------
STATUS CRsrvDB::Update(CCommResApi& otherResConf)
{
	PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(&otherResConf), STATUS_FAIL);

	CCommResRsrvShort* shortRes = new CCommResRsrvShort;
	UpdateSchortResWithLongResData(shortRes, otherResConf);
	shortRes->SetStatus(STATUS_OK); // if we got until here, it means that the reservation is OK now

	// Every new reservation enters with operator mask = TRUE
	COPERMASKONINRSRVSHORT(shortRes);

	ReservArray::iterator it = FindName(otherResConf.GetName());
	if (it == m_reservArray.end())
	{
		POBJDELETE(shortRes);
		return (m_DBtype == PROFILES_DATABASE) ? STATUS_PROFILE_NOT_FOUND : STATUS_RESERVATION_NOT_EXISTS;
	}

	// First delete the old file from the folder
	std::string uniqueFileName = (*it)->GetFileUniqueName();
	if (m_pFileManager->DeleteFileData(uniqueFileName) != STATUS_OK)
	{
		POBJDELETE(shortRes);
		PASSERTSTREAM_AND_RETURN_VALUE(!m_pFileManager, "Cannot delete the CCommResApi file:" << uniqueFileName, STATUS_FAIL);
	}

	// write the new one to the disk
	uniqueFileName = otherResConf.GetFileUniqueName(m_prefixMap[m_DBtype]);
	if (m_pFileManager->AddFileData(otherResConf, uniqueFileName) != STATUS_OK)
	{
		POBJDELETE(shortRes);
		PASSERTSTREAM_AND_RETURN_VALUE(!m_pFileManager, "Cannot write the CCommResApi file:" << uniqueFileName, STATUS_FAIL);
	}

	*(*it) = (*shortRes);
	POBJDELETE(shortRes);

	IncreaseSummaryUpdateCounter();

	(*it)->SetSummeryUpdateCounter(GetSummaryUpdateCounter());

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CRsrvDB::Cancel(const DWORD confId, BOOL bForUpdate)
{
	ReservArray::iterator it = FindId(confId);

	if (it == m_reservArray.end())
		return STATUS_RESERVATION_NOT_EXISTS;

	PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(*it), STATUS_FAIL);

	// remove the Res from Dir
	std::string uniqueFileName = (*it)->GetFileUniqueName();
	if (m_pFileManager->DeleteFileData(uniqueFileName) != STATUS_OK)
	{
		PASSERTSTREAM_AND_RETURN_VALUE(!m_pFileManager, "Cannot delete the CCommResApi file:" << uniqueFileName, STATUS_FAIL);
	}

	TRACEINTO << "MonitorConfId:" << confId << ", FileName:" << uniqueFileName << ", ForUpdate:" << (int)bForUpdate << " - Removing conference from DB";

	POBJDELETE(*it);         // Erase the element
	m_reservArray.erase(it); // remove the element from the vector

	IncreaseSummaryUpdateCounter();

	if (bForUpdate == FALSE)
	{
		if (m_LastDeletedIndex >= HISTORY_SIZE)
			m_LastDeletedIndex = 0;

		m_DeletedIdHistory[m_LastDeletedIndex]      = confId;
		m_DeletedCounterHistory[m_LastDeletedIndex] = m_dwSummaryUpdateCounter;
		m_LastDeletedIndex++;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
BOOL CRsrvDB::NameExists(const char* name)
{
	if (FindName(name) != m_reservArray.end())
		return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------------
BOOL CRsrvDB::DisplayNameExists(const char* name)
{
	if (FindDisplayName(name) != m_reservArray.end())
		return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------------
ReservArray::iterator CRsrvDB::FindName(const char* name)
{
	PASSERT_AND_RETURN_VALUE(!name, m_reservArray.end());

	for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
	{
		if (!CPObject::IsValidPObjectPtr(*it))
		{
			TRACEINTO << "Name:" << name << " - Invalid object in vector";
			continue;
		}
		if (!strncmp((*it)->GetName(), name, H243_NAME_LEN))
			return it;
	}
	return m_reservArray.end();
}

//--------------------------------------------------------------------------
ReservArray::iterator CRsrvDB::FindDisplayName(const char* name)
{
	PASSERT_AND_RETURN_VALUE(!name, m_reservArray.end());

	for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
	{
		if (!CPObject::IsValidPObjectPtr(*it))
		{
			TRACEINTO << "Name:" << name << " - Invalid object in vector";
			continue;
		}
		if (!strncmp((*it)->GetDisplayName(), name, H243_NAME_LEN))
			return it;
	}
	return m_reservArray.end();
}

//--------------------------------------------------------------------------
ReservArray::iterator CRsrvDB::FindId(const DWORD confId)
{
	for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
	{
		if (!CPObject::IsValidPObjectPtr(*it))
		{
			TRACEINTO << "MonitorConfId:" << confId << " - Invalid object in vector";
			continue;
		}
		if ((*it)->GetConferenceId() == confId)
				return it;
	}
	return m_reservArray.end();
}

//--------------------------------------------------------------------------
CCommResApi* CRsrvDB::GetRsrv(const DWORD confId)
{
	// Search in file manager the file with the match id
	ReservArray::iterator it = FindId(confId);

	if (it == m_reservArray.end())
	{
		TRACEINTO << "MonitorConfId:" << confId << " - Failed, Can not find the conference the CommResDB";
		return NULL;
	}

	return m_pFileManager->GetFileData((*it)->GetFileUniqueName());
}

//--------------------------------------------------------------------------
CCommResRsrvShort* CRsrvDB::GetShortRsrv(const DWORD confId)
{
	// Search in file manager the file with the match id
	ReservArray::iterator it = FindId(confId);

	if (it == m_reservArray.end())
	{
		// Zoe: I removed this trace, because many times we search for conferences, and it's Ok that we don't find them.
		// PTRACE(eLevelError,"CRsrvDB::GetShortRsrv Can not find the id in the CommResDB");
		return NULL;
	}

	return (*it);
}

//--------------------------------------------------------------------------
void CRsrvDB::IncreaseSummaryUpdateCounter()
{
	m_dwSummaryUpdateCounter++;

	if (m_dwSummaryUpdateCounter == 0xFFFFFFFF)
		m_dwSummaryUpdateCounter = 0;
}

//--------------------------------------------------------------------------
void CRsrvDB::ClearVector()
{
	for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
		POBJDELETE(*it);

	m_reservArray.clear();
}

//--------------------------------------------------------------------------
void CRsrvDB::ClearCommResVector(std::vector< CCommResApi*>& vect)
{
	for (std::vector< CCommResApi*>::iterator it = vect.begin(); it != vect.end(); ++it)
		POBJDELETE(*it);

	vect.clear();
}

//--------------------------------------------------------------------------
STATUS CRsrvDB::InitRsrvDB(std::string dirName, CReservator* pReservator)
{
	TRACEINTO << "DirName:" << dirName;

	POBJDELETE(m_pFileManager);
	m_pFileManager = new CFileRsrvManager<CCommResApi>(dirName);

	std::vector< CCommResApi*> vect;
	// Update the ShortRes Array with the loaded array
	// VNGR-9280 if there is problem loading some of the files we won't clear all the vector and just upload the once we can
	if ((m_pFileManager->LoadDataToVect(vect)) == STATUS_FAIL)
	{
		PASSERTSTREAM(1, "Failed on loading all the profiles from File Manager, DirName:" << dirName);
	}

	// Sort the vector according to monitor id (so that the suspended conferences will be the last one reserved)
	std::sort(vect.begin(), vect.end(), SAscendingMonitorIdSort());

	// first set the Items Counter to the max counter
	int maxItemID = 0;
	CStructTm now;
	SystemGetTime(now);

	int count = 0;
	for (std::vector< CCommResApi*>::iterator it = vect.begin(); it != vect.end(); ++it)   // run over all items
	{
		if (!CPObject::IsValidPObjectPtr(*it))
		{
			ClearCommResVector(vect);
			PASSERTMSG_AND_RETURN_VALUE(1, "Failed in adding the Item manager", STATUS_FAIL);
		}

		// if start time has passed, don't add to list
		if (*((*it)->GetStartTime()) <= now)
		{
			CLargeString description;
			description << "Reservation \"" << (*it)->GetDisplayName() << "\" is past due and has been deleted from the Reservation list.";

			TRACEINTO << "MonitorConfId:" << (*it)->GetMonitorConfId() << " - " << description;

			std::string uniqueFileName = (*it)->GetFileUniqueName(m_prefixMap[m_DBtype]);
			if (m_pFileManager->DeleteFileData(uniqueFileName) != STATUS_OK)
				TRACEINTO << "FileName:" << uniqueFileName << " - Failed, Can not delete the CCommResApi file";

			CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, START_TIME_OF_RESERVED_CONFER_IS_OVER, SYSTEM_MESSAGE, description.GetString(), FALSE);
			continue;
		}

		if ((Add(*(*it), true)) == STATUS_OK)                                                            // Add each one to the DB
		{
			maxItemID = std::max(maxItemID, int((*it)->GetMonitorConfId()));                               // Calculate max id
			InitDBSpecifiedFields(*it);                                                                    // Set relevant fields to default
			STATUS tempstatus = pReservator->CreateReservationFromStartupOfResDB((*it));
			if (tempstatus == STATUS_RESERVATION_SUSPENDED)                                                // collision in reservations, set appropriate flag in short
			{
				ReservArray::iterator resIterator = FindId((*it)->GetMonitorConfId());
				if (resIterator != m_reservArray.end())
				{
					TRACEINTO << "MonitorConfId:" << (*it)->GetMonitorConfId() << " - Failed with problem 1";
					(*resIterator)->SetStatus(eStsSUSPEND);
				}
				else
				{
					PASSERT(1);                                                                                // we've jut added i, so where is it????
				}
			}
			else if (tempstatus != STATUS_OK)
			{
				TRACEINTO << "MonitorConfId:" << (*it)->GetMonitorConfId() << " - Failed with problem 2";    // INIT_RESERVATIONS_DEBUG
				Cancel((*it)->GetMonitorConfId(), TRUE);
			}

			// 2 modes cop/cp specific reservation's check.
			DWORD bIsCordinateModes = pReservator->CheckReservationSysMode(*(*it));
			TRACEINTO << "MonitorConfId:" << (*it)->GetMonitorConfId() << ", bIsCordinateModes:" << (EResStatusList)bIsCordinateModes; // carmit

			ReservArray::iterator resIterator = FindId((*it)->GetMonitorConfId());
			if (resIterator != m_reservArray.end())
			{
				if (bIsCordinateModes != STATUS_OK)
				{
					(*resIterator)->SetStatus(eWrongSysMode);                                                  // Status Field
				}
				else
				{
					if ((*resIterator)->GetStatus() != eStsSUSPEND)
					{
						(*resIterator)->SetStatus(eStsOK);                                                       // Status Field
					}
				}
			}
		}
		else
		{
			ClearCommResVector(vect);
			PASSERTMSG_AND_RETURN_VALUE(1, "Failed in adding the Item manager", STATUS_FAIL);
		}

		count++;
		if (count%50 == 0)
		{
			// We go to sleep for a little while every 100 reservations.
			// To allow other tasks to be performed in the Resource Process (vngr-9656) - especially answering the watchdog
			TRACEINTO << "Number of created reservations is " << count;
			SystemSleep(10, TRUE);
		}
	}

	SetDBCounter(maxItemID);                           // Set the profiles Counter
	ClearCommResVector(vect);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CRsrvDB::CheckAllReservationsSysMode()
{
	TRACEINTO;

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (!pReservator)
		return STATUS_FAIL;

	CCommResApi* pCommResApi = NULL;
	ReservArray::iterator _itr, _end = m_reservArray.end();
	for (_itr = m_reservArray.begin(); _itr != _end; ++_itr)
	{
		if (CPObject::IsValidPObjectPtr(*_itr))
		{
			ConfMonitorID confId = (*_itr)->GetConferenceId();
			CCommResApi* pCommResApi = GetRsrv(confId);
			if (pCommResApi)
			{
				STATUS isCordinateModes = pReservator->CheckReservationSysMode(*pCommResApi);
				TRACEINTO << "MonitorConfId:" << confId << ", IsCordinateModes:" << isCordinateModes;

				ReservArray::iterator _itrReserv = FindId(pCommResApi->GetMonitorConfId());
				if (_itrReserv != m_reservArray.end())
				{
					if (isCordinateModes != STATUS_OK)
						(*_itrReserv)->SetStatus(eWrongSysMode); // Status Field
					else if ((*_itrReserv)->GetStatus() != eStsSUSPEND)
						(*_itrReserv)->SetStatus(eStsOK);        // Status Field
				}
				delete pCommResApi;
			}
			else
			{
				TRACEINTO << "MonitorConfId:" << confId << " - Failed, Cannot find reservation";
			}
		}
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CRsrvDB::InitDBSpecifiedFields(CCommResApi* pCommRes)
{
	// Update only MR DB
	if (m_DBtype != MEETING_ROOM_DATABASE)
		return;

	// first read of MR from the file, set it to passive mode
	if (pCommRes->GetMeetingRoomState() != MEETING_ROOM_PASSIVE_STATE)
	{
		pCommRes->SetMeetingRoomState(MEETING_ROOM_PASSIVE_STATE);
		if (Update(*pCommRes) != STATUS_OK)
			TRACEINTO << "Failed, Cannot update the fields on disk";
	}
}

//--------------------------------------------------------------------------
CCommResRsrvShort* CRsrvDB::GetNextReservation()
{
	CCommResRsrvShort* pNextResShort = NULL;
	CStructTm          nearestTime;
	nearestTime.m_year = 3000; // far away

	for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
	{
		if (!CPObject::IsValidPObjectPtr(*it))
		{
			TRACEINTO << "Invalid object in vector";
			continue;
		}
		if (*((*it)->GetStartTime()) <= nearestTime)
		{
			pNextResShort = (*it);
			nearestTime   = *((*it)->GetStartTime());
		}
	}
	return pNextResShort;
}

//--------------------------------------------------------------------------
void CRsrvDB::Dump(std::ostream& os)
{
	os.setf(std::ios::left, std::ios::adjustfield);
	os.setf(std::ios::showbase);

	os << "CRsrvDB::Dump\n"
		 << "----------------\n";

	CCommResRsrvShort* pCommResRsrvShort;
	for (ReservArray::iterator it = m_reservArray.begin(); it != m_reservArray.end(); ++it)
	{
		pCommResRsrvShort = (CCommResRsrvShort*)(*it);
		os << "\n  Reservation:" << (pCommResRsrvShort->GetName())
			 << "\n  Start time :" << *(pCommResRsrvShort->GetStartTime())
			 << "\n  End time   :" << *(pCommResRsrvShort->GetEndTime())
			 << "\n  Duration   :" << *(pCommResRsrvShort->GetDurationTime())
			 << "\n  ConfID     :" << (pCommResRsrvShort->GetConferenceId())
			 << "\n  NID        :" << (pCommResRsrvShort->GetNumericConfId())
			 << "\n  Status     :" << (int)(pCommResRsrvShort->GetStatus())
			 << "\n";
	}
}

//--------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, CRsrvDB& obj)
{
	obj.Dump(os);
	return os;
}

//--------------------------------------------------------------------------
WORD CRsrvDB::GetResNumber() const
{
	return m_reservArray.size();
}

//--------------------------------------------------------------------------
bool CRsrvDB::IsRsrvUsingProfile(DWORD profileID) const
{
	bool rc = false;
	for (ReservArray::const_iterator cIt =  m_reservArray.begin(); cIt != m_reservArray.end(); ++cIt)
	{
		if ((*cIt)->GetAdHocProfileId() == profileID)
		{
			rc = true;
			break;
		}
	}
	TRACEINTO << "ProfileId:" << profileID << ", IsUsing:" << (int)rc;
	return rc;
}
