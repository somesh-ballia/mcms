/*
 * CommConfStoreDB.cpp
 *
 *  Created on: May 8, 2011
 *      Author: bguelfand
 */

#include "OngoingConfStore.h"
#include "CommResShort.h"
#include "CommRes.h"
#include "psosxml.h"
#include <utility>
#include <fcntl.h>
#include "TraceStream.h"
#include "HlogApi.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"
#include "RsrvManagerApi.h"

const char* pszFileNamePrefix = "ongoing_conf";

struct SAscendingMonitorIdSort
{
     bool operator()(CCommResApi* const& rpStart, CCommResApi* const& rpEnd)
     {
          return rpStart->GetMonitorConfId() < rpEnd->GetMonitorConfId();
     }
};

COngoingConfStore* COngoingConfStore::m_pInstance = NULL;

COngoingConfStore* COngoingConfStore::GetInstance()
{
	if (m_pInstance == NULL)
		m_pInstance = new COngoingConfStore;
	return m_pInstance;
}
/////////////////////////////////////////////////////////////////////////////
COngoingConfStore::COngoingConfStore()
	: m_pFileManager(0)
{
	if( open(ONGOING_CONF_STORE_DB_DIR, O_DIRECTORY) == -1) //Create the Folder only if needed
	{
		if( CreateDirectory(ONGOING_CONF_STORE_DB_DIR) == FALSE)
		{
			PTRACE(eLevelError,"CCentralConferencesDB::CreateRsrvDB Can not create Reservations directory");
			PASSERT(1);
			return;
		}
	}
	m_pFileManager = new CFileManager<CCommResApi>(ONGOING_CONF_STORE_DB_DIR);
}
///////////////////////////////////////////////////////////////////////////////
COngoingConfStore::~COngoingConfStore()
{
 	ClearVector();

 	POBJDELETE(m_pFileManager);
}

/////////////////////////////////////////////////////////////////////////////
STATUS COngoingConfStore::InitOngoingConfStore()
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::InitOngoingConfStore - begin");

	CProcessBase* pProcess = CProcessBase::GetProcess();
	if (pProcess && pProcess->GetIsFailoverFeatureEnabled())//if (HotBackUp)
	{
		m_pFileManager->RemoveAllFiles();
		PTRACE(eLevelInfoNormal,"COngoingConfStore::InitOngoingConfStore - HotBack feature is enabled - return");
		return STATUS_OK;
	}

	ClearCommResVector(m_arrStoredConfs);
	std::vector<CCommResApi*> vect;
	if((m_pFileManager->LoadDataToVect(m_arrStoredConfs)) == STATUS_FAIL)
	{
		PTRACE(eLevelError,"COngoingConfStore::InitOngoingConfStore Failed on loading all the profiles from Filemanager");
		PASSERT(1);
	}

//	m_pFileManager->RemoveAllFiles();

	// Sort the vector according to monitorid (so that the suspended conferences will be the last one reserved)
	std::sort(m_arrStoredConfs.begin(), m_arrStoredConfs.end(), SAscendingMonitorIdSort());

	//first set the Items Counter to the max counter
	std::vector<CCommResApi*>::iterator it= m_arrStoredConfs.begin();
	int maxItemID=0;
	CStructTm stTimeNow;
	SystemGetTime(stTimeNow);
	char buff[30] = "";
	stTimeNow.DumpToBuffer(buff);
	PTRACE2(eLevelInfoNormal,"COngoingConfStore::InitOngoingConfStore Time Now = ", buff);

 	int count =0;
	for (; it != m_arrStoredConfs.end() ; ++it)  //run over all items
	{
		if(!CPObject::IsValidPObjectPtr(*it))
		{
			PTRACE(eLevelError,"COngoingConfStore::InitOngoingConfStore failed in Adding the Item manager");
//			ClearCommResVector(vect);
//			PASSERT(1);
//			PTRACE(eLevelInfoNormal,"CConfPartyManager::InitOngoingConfStoreDB - return");
//			return STATUS_FAIL;
			continue;
		}

		std::string sFileName = (*it)->GetFileUniqueName(pszFileNamePrefix);
		std::string sConfName = (*it)->GetName();
		m_mapConfNameFileName[sConfName] = sFileName;

		//if end time has passed, don't add to list
		bool bPermanent = (*it)->IsPermanent() != FALSE;
		const CStructTm* pEndTime = (*it)->GetEndTime();
		pEndTime->DumpToBuffer(buff);
		PTRACE2(eLevelInfoNormal,"COngoingConfStore::InitOngoingConfStore Time End = ", buff);
		DWORD dwDiff = (*pEndTime) - stTimeNow; // Seconds
		PTRACE2INT(eLevelInfoNormal,"COngoingConfStore::InitOngoingConfStore Time Diff = ", dwDiff);
		if(!bPermanent && dwDiff < 300) // diff < 5 min.
		{
			CLargeString description;
			description << "Stored Ongoing Conference \"" << (*it)->GetDisplayName()
					<< "\" is past due and has been deleted from the OngoingConfStore.";
			TRACEINTO << "COngoingConfStore::InitOngoingConfStoreDB:  monitor id " << (*it)->GetMonitorConfId() << ":" << description;
			if ( m_pFileManager->DeleteFileData(sFileName) != STATUS_OK)
				PTRACE(eLevelError,"COngoingConfStore::InitOngoingConfStore Can not delete the CCommResApi file");

			CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,START_TIME_OF_RESERVED_CONFER_IS_OVER,SYSTEM_MESSAGE,description.GetString(), FALSE);
			continue;
		}

		(*it)->SetStartTime(stTimeNow);
		(static_cast<CCommRes*>(*it))->SetAdHocProfileId(-1);
		(static_cast<CCommRes*>(*it))->SetStandBy(YES);
		if (bPermanent)
			PTRACE(eLevelInfoNormal,"COngoingConfStore::InitOngoingConfStore Time Duration = PERMANENT");
		else
		{
			int hour = dwDiff/3600;
			int min = (dwDiff%3600)/60;
			int sec = (dwDiff%3600)%60;
			CStructTm timeTmp(0,0,0,hour,min,sec);
			timeTmp.DumpToBuffer(buff);
			PTRACE2(eLevelInfoNormal,"COngoingConfStore::InitOngoingConfStore Time Duration = ", buff);
			(static_cast<CCommRes*>(*it))->SetDurationTime(timeTmp);
		}

//Remove Undefined participants
		bool bBreak = false;
		while ( ! bBreak )
		{
			bBreak = true;
			CRsrvParty* pParty = (*it)->GetFirstParty();
			while (pParty)
			{
				if (pParty->IsUndefinedParty())
				{
					(static_cast<CCommRes*>(*it))->Cancel(pParty->GetName());
					bBreak = false;
					break;
				}
				pParty = (*it)->GetNextParty();
			}
		}


		OPCODE opcode = START_AD_HOC_CONF_REQ;
		CSegment *pToRsrcSeg = new CSegment;

		COstrStream ostrToRsrc;
		(*it)->Serialize(NATIVE, ostrToRsrc);

		// VNGR-22659: set the sender position identifier in order to identify ongoing 
		// conference restore after reboot
		ostrToRsrc << (WORD)3 << "\n";

		ostrToRsrc.Serialize(*pToRsrcSeg);

		CRsrvManagerApi rsrvManagerApi;
		rsrvManagerApi.SendMsg(pToRsrcSeg,opcode);

	}
	PTRACE(eLevelInfoNormal,"CConfPartyManager::InitOngoingConfStore - end");
	return STATUS_OK;
}
///////////////////////////////////////////////////////////////////////////////
STATUS COngoingConfStore::Add(CCommResApi*  pConfRes)
{
	PTRACE(eLevelInfoNormal,"COngoingConfStore::Add - begin");

	CProcessBase* pProcess = CProcessBase::GetProcess();
	if (pProcess && pProcess->GetIsFailoverFeatureEnabled())//if (HotBackUp)
	{
		PTRACE(eLevelInfoNormal,"COngoingConfStore::Add - HotBack feature is enabled - return");
		return STATUS_OK;
	}

	PTRACE2(eLevelInfoNormal,"COngoingConfStore::Add - Class of pConfRes - ", pConfRes->NameOf());
	if (!CPObject::IsValidPObjectPtr(pConfRes) )
	{
		PTRACE(eLevelError,"COngoingConfStore::Add CCommResApi not valid - return");
		PASSERT(1);
		return STATUS_FAIL;
	}

	if (pConfRes->IsMeetingRoom() || pConfRes->GetEntryQ() || pConfRes->IsConfTemplate() || pConfRes->GetIsGateway())
	{
		PTRACE(eLevelInfoNormal,"COngoingConfStore::Add - MR or EQ or CT or GW");
		return STATUS_OK;
	}

	//make sure the file manager is exists
	if (!m_pFileManager)
	{
		PTRACE(eLevelError,"COngoingConfStore::Add Can not write to file File Manager is NULL - return");
 		return STATUS_FAIL;
 	}

	string sFileName = pConfRes->GetFileUniqueName(pszFileNamePrefix);
	string sConfName = pConfRes->GetName();
	DWORD dwConfMonitorID = pConfRes->GetMonitorConfId();
	if (m_pFileManager->ReplaceFileData(pConfRes, sFileName) != STATUS_OK)
	{
		PTRACE(eLevelError,"COngoingConfStore::Add Can not Add the CCommResApi file - return");
		PASSERT(1);
		return STATUS_FAIL;
	}

	if (m_mapConfNameFileName.find(sConfName) != m_mapConfNameFileName.end())
	{
		if (m_mapConfNameFileName[sConfName] != sFileName)
		{
			m_pFileManager->RemoveFile(m_mapConfNameFileName[sConfName]);
			m_mapConfNameFileName.erase(sConfName);
		}
	}
	m_mapConfIdFileName[dwConfMonitorID] = sFileName;

	PTRACE(eLevelInfoNormal,"COngoingConfStore::Add - end");
 	return  STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS COngoingConfStore::Remove(const DWORD dwConfMonitorID)
{
	STORED_CONFIDS_FILES::iterator it = m_mapConfIdFileName.find(dwConfMonitorID);
	if (it == m_mapConfIdFileName.end() )
 		return STATUS_RESERVATION_NOT_EXISTS;

	 //remove the Res from Dir
	if ( m_pFileManager->DeleteFileData(it->second) != STATUS_OK)
	{
		PTRACE(eLevelError,"COngoingConfStore::Remove - Can not delete the CCommResApi file");
		PASSERT(1);
		return STATUS_FAIL;
	}

	PTRACE2INT(eLevelInfoNormal, "COngoingConfStore::Remove - Removing from DB conf with id ", dwConfMonitorID);

	m_mapConfIdFileName.erase(it);//remove the elemnet from the map

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
 const char*  COngoingConfStore::NameOf() const
 {
 	return "COngoingConfStore";
 }
 /////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void COngoingConfStore::ClearVector()
{
	PTRACE(eLevelInfoNormal,"COngoingConfStore::ClearVector Clearing vector");
	for (STORED_CONFS_ARRAY::iterator it= m_arrStoredConfs.begin(); it != m_arrStoredConfs.end();++it)
 		POBJDELETE(*it);

 	m_arrStoredConfs.clear();
}
/////////////////////////////////////////////////////////////////////////////
void COngoingConfStore::ClearCommResVector(std::vector< CCommResApi *> & vect)
{
	PTRACE(eLevelInfoNormal,"COngoingConfStore::ClearCommResVector Clearing vector");

	for (std::vector< CCommResApi *>::iterator it= vect.begin(); it != vect.end();++it)
 		POBJDELETE(*it);

 	vect.clear();
}

void COngoingConfStore::Dump(std::ostream& os)
{
	os.setf(std::ios::left,std::ios::adjustfield);
	os.setf(std::ios::showbase);

	os << "\n"
		<< "COngoingConfStore::Dump\n"
		<< "----------------\n";

	CCommResApi* pCommResApi;
    for (STORED_CONFS_ARRAY::iterator it=m_arrStoredConfs.begin(); it != m_arrStoredConfs.end();++it)
	{
	    	pCommResApi = (CCommResApi*)(*it);
			os << "\nReservation Name: " << (pCommResApi->GetName())
			<< "\nStart time: " << *(pCommResApi->GetStartTime())
			<< "\nEnd time: " << *(pCommResApi->GetEndTime())
			<< "\nDuration: " << *(pCommResApi->GetDuration())
			<< "\nConfMonitorID: " << (pCommResApi->GetMonitorConfId())
			<< "\nNID: " << (pCommResApi->GetNumericConfId())
			<< "\n";
	}
}

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator<< (std::ostream& os, COngoingConfStore& obj )
{
	obj.Dump(os);
	return os;
}

