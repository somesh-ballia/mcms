#include "SelfConsistency.h"
#include "ResourceProcess.h"
#include "InternalProcessStatuses.h"
#include "ResourceManager.h"
#include "SelfConsistencyPartyRsrcDetails.h"
#include "SelfConsistencyWhatToCheck.h"
#include "SelfConsistencyDefines.h"
#include "TraceStream.h"
#include "Reservator.h"
#include "HelperFuncs.h"
#include "CardResourceConfig.h"
#include "FixedModeResources.h"
#include "AutoModeResources.h"
#include "HelperFuncs.h"
#include "CRsrvDB.h"
#include "ConfResources.h"

CSelfConsistency* CSelfConsistency::m_pInstance = NULL;

////////////////////////////////////////////////////////////////////////////
//                        CSelfConsistency
////////////////////////////////////////////////////////////////////////////
CSelfConsistency::CSelfConsistency()
{
	m_pSystemResources   = NULL;
	m_pConfRsrcDB        = NULL;
	m_pConnToCardManager = NULL;
	m_pSystemResources   = NULL;
	m_pConfRsrcDB        = NULL;
	m_pReservator        = NULL;
	m_pConnToCardManager = NULL;
	m_pProcess           = (CResourceProcess*)CResourceProcess::GetProcess();

	if (m_pProcess)
	{
		m_pSystemResources = m_pProcess->GetSystemResources();
		m_pConfRsrcDB      = m_pProcess->GetConfRsrcDB();
		m_pReservator      = m_pProcess->GetReservator();

		CTaskApp* pTaskApp = m_pProcess->GetCurrentTask();
		if (pTaskApp && !strcmp((pTaskApp->GetTaskName()), "Manager"))
			m_pConnToCardManager = ((CResourceManager*)pTaskApp)->GetConnToCardManager();
	}
}

//--------------------------------------------------------------------------
CSelfConsistency::~CSelfConsistency()
{
}

//--------------------------------------------------------------------------
const char* CSelfConsistency::NameOf() const
{
	return " CSelfConsistency ";
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckInternalPointers()
{
	if (!m_pSystemResources || !m_pConfRsrcDB || !m_pConnToCardManager || !m_pReservator)
	{
		m_OStream << "One of m_pSystemResources, m_pConfRsrcDB, m_pConnToCardManager, m_pReservator is NULL!!!\n";
		DBGPASSERT(1);
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckTiming()
{
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
	{
		m_OStream << "Can't check internal consistency yet: curStatus: " << GetProcessStatusName(curStatus) << "\n";
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckConsistency(std::ostream* pAdditionalStreamToPrintAnswerTo /*= NULL*/)
{
	CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck = new CSelfConsistencyWhatToCheck();
	pSelfConsistencyWhatToCheck->CheckEverything();

	STATUS                       finalStatus = CheckConsistency(pSelfConsistencyWhatToCheck, pAdditionalStreamToPrintAnswerTo);

	delete pSelfConsistencyWhatToCheck;

	return finalStatus;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckConsistency(CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck, std::ostream* pAdditionalStreamToPrintAnswerTo /*= NULL*/)
{
	m_OStream << "\n\nChecking Resource Consistency\n------------------------------\n";

	if (pSelfConsistencyWhatToCheck == NULL)
	{
		TRACEINTO << "pSelfConsistencyWhatToCheck is NULL\n";
		DBGPASSERT(1);
		return STATUS_FAIL;
	}

	STATUS tempStatus;
	STATUS finalStatus = STATUS_OK;

	tempStatus = CheckInternalPointers();
	if (tempStatus != STATUS_OK)
	{
		FinishCheckConsistency(pAdditionalStreamToPrintAnswerTo, tempStatus);
		return tempStatus;
	}

	tempStatus = CheckTiming();
	if (tempStatus != STATUS_OK)
	{
		FinishCheckConsistency(pAdditionalStreamToPrintAnswerTo, tempStatus);
		return tempStatus;
	}

	if (pSelfConsistencyWhatToCheck->m_bCheckNumOfConferences)
	{
		tempStatus = CheckNumberOfConferences();
		PrintStatus(tempStatus);
		if (tempStatus != STATUS_OK)
			finalStatus = tempStatus;
	}

	if (pSelfConsistencyWhatToCheck->m_bCheckNoMoreConferences)
	{
		tempStatus = CheckAllResourcesDeletedIfNoMoreConferences();
		PrintStatus(tempStatus);
		if (tempStatus != STATUS_OK)
			finalStatus = tempStatus;
	}

	if (pSelfConsistencyWhatToCheck->m_bCheckAudioVideoConfiguration)
	{
		tempStatus = CheckAudioVideoConfiguration();
		PrintStatus(tempStatus);
		if (tempStatus != STATUS_OK)
			finalStatus = tempStatus;
	}

	if (pSelfConsistencyWhatToCheck->m_bCheckParties)
	{
		tempStatus = CheckParties(pSelfConsistencyWhatToCheck);
		PrintStatus(tempStatus);
		if (tempStatus != STATUS_OK)
			finalStatus = tempStatus;
	}

	if (pSelfConsistencyWhatToCheck->m_bCheckNIDs)
	{
		tempStatus = CheckNIDs();
		PrintStatus(tempStatus);
		if (tempStatus != STATUS_OK)
			finalStatus = tempStatus;
	}

	if (pSelfConsistencyWhatToCheck->m_bCheckPhones)
	{
		tempStatus = CheckPhones();
		PrintStatus(tempStatus);
		if (tempStatus != STATUS_OK)
			finalStatus = tempStatus;
	}

	FinishCheckConsistency(pAdditionalStreamToPrintAnswerTo, finalStatus);

	return finalStatus;
}

//--------------------------------------------------------------------------
#define MAX_LENGTH_OF_INCONSISTENCY_STRING 100000
void CSelfConsistency::FinishCheckConsistency(std::ostream* pAdditionalStreamToPrintAnswerTo, STATUS finalStatus)
{
	// if the stream is too long it will cause a crash
	long pos = m_OStream.str().length();
	if (pos > MAX_LENGTH_OF_INCONSISTENCY_STRING)
	{
		ClearStream();
		m_OStream << "\nString was too long and has been cleared!\n";
	}

	m_OStream << "*******Finished Checking Resource Consistency\n";
	PrintStatus(finalStatus);

	TRACEINTO << m_OStream.str();

	if (pAdditionalStreamToPrintAnswerTo != NULL)
		*pAdditionalStreamToPrintAnswerTo << m_OStream.str();

	ClearStream();
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckNumberOfConferences()
{
	STATUS status = STATUS_OK;
	m_OStream << "**Checking the number of conferences\n";

	// check if the number of conferences in ConfRsrcDB is the size of the conferences in the list
	if (m_pConfRsrcDB->m_numConfRsrcs != m_pConfRsrcDB->m_confList.size())
	{
		m_OStream << "Inconsistency between m_pConfRsrcDB->m_numConfRsrcs ("
		          << m_pConfRsrcDB->m_numConfRsrcs << ") and m_pConfRsrcDB->m_pConfRsrcsList->size() ("
		          << m_pConfRsrcDB->m_confList.size() << ")\n";

		// set the right number ???
		// m_pConfRsrcDB->m_numConfRsrcs = m_pConfRsrcDB->m_pConfRsrcsList->size();

		status = STATUS_FAIL;
	}

	WORD                                    numMR         = 0;
	WORD                                    numEQ         = 0;
	WORD                                    numSipFact    = 0;
	WORD                                    numConf       = 0;
	WORD                                    numRes        = 0;
	WORD                                    numResAndConf = 0;

	SleepingConferences::iterator i;
	for (i = m_pProcess->GetSleepingConferences()->begin(); i != m_pProcess->GetSleepingConferences()->end(); i++)
	{
		switch (i->GetConfType())
		{
			case eConf_type_none:
			m_OStream << "There's a normal conference in the list of sleeping conferences. It's monitor id is " << i->GetMonitorConfId();
			status = STATUS_FAIL;
			break;

			case eMR_type:
			numMR++; break;

			case eEQ_type:
			numEQ++; break;

			case eSipFact_type:
			numSipFact++; break;

			default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
		} // switch
	}

	numConf = m_pProcess->GetConfRsrcDB()->m_numConfRsrcs;

	CRsrvDB* pRsrvDB = m_pProcess->GetRsrvDB();
	if (pRsrvDB != NULL)  // with reservator
	{
		numRes = pRsrvDB->m_reservArray.size();
	}

	numResAndConf = numConf + numRes;

	/*
	 * When deleting conferences: The number of conferences in m_pConfRsrcDB is updated first
	 * And only later (in another task), the number in the m_pConfRsrvRsrcs is updated
	 * This means that both numbers might not be the same!
	 * But this means that always the number in m_pConfRsrvRsrcs should be bigger than
	 * the number in both dbs.
	 * We are also interested in knowing when there's a big difference between both...
	 */
	size_t size = m_pProcess->GetConfRsrvRsrcs()->size();
	if (numResAndConf < size)
	{
		m_OStream << "m_pConfRsrvRsrc->size() ("
		          << size << ") is bigger than numResAndConf (" << numResAndConf << ") "
		          << "numRes: " << numRes << " numConf: " << numConf << "\n";

		status = STATUS_FAIL;
	}

	if (numResAndConf - size > 10)
	{
		m_OStream << "Big difference between both numbers. m_pConfRsrvRsrc->size() is "
		          << size << "and numResAndConf is " << numResAndConf
		          << " numRes: " << numRes << " numConf: " << numConf << "\n"<< "\n";

		status = STATUS_FAIL;
	}

	if (numMR != m_pReservator->m_numMR || numEQ != m_pReservator->m_numEQ || numSipFact != m_pReservator->m_numSipFact)
	{
		m_OStream << "Non equal numbers:\nnumMR: " << numMR  << " m_pReservator->m_numMR: " << m_pReservator->m_numMR
		          << "\nnumEQ: " << numEQ  << " m_pReservator->m_numEQ: " << m_pReservator->m_numEQ
		          << "\nnumSipFact: " << numSipFact  << " m_pReservator->m_numEQ: " << m_pReservator->m_numSipFact
		          << "\n";

		status = STATUS_FAIL;
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckAllResourcesDeletedIfNoMoreConferences()
{
	m_OStream << "**Checking consistency when no more conferences\n";
	CBoard* pBoard;
	// if there are no more conferences ledt
	if (m_pConfRsrcDB->m_numConfRsrcs == 0)
	{
		// check that there are no more descriptors left in shared memory, except the general ones

		WORD expectedNumOfEntries = 1;   // 1 for the IVRController

		// for each board that has an AC unit (master or slave, there's an additional entry)
		for (int i = 1; i <= BOARDS_NUM; i++)
		{
			if (!(m_pSystemResources->IsBoardIdExists(i)))
				continue;

			pBoard = m_pSystemResources->GetBoard(i);
			if (pBoard == NULL)
				continue;

			if (pBoard->GetMFA(AUD_CNTLER_UNIT_ID) != NULL)
				expectedNumOfEntries++;
		}

		if (m_pConnToCardManager->GetNumOfEntries() != expectedNumOfEntries)
		{
			m_OStream << "Inconsistency between m_pConnToCardManager->GetNumOfEntries() ("
			          << m_pConnToCardManager->GetNumOfEntries() << ") and expectedNumOfEntries ("
			          << expectedNumOfEntries << ")\n";
			return STATUS_FAIL;
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckAudioVideoConfiguration()
{
	m_OStream << "**Checking audio and video configuration\n";

	// This function will check that the numbers of audio and video units are the same as what is written in the SystemResources
	// It will also check that this number is according to the audiovideo configuration
	// this code is based somewhat on IsRsrcEnough of each of the allocationtypes

	std::set<CUnitMFA>::iterator unitMFAItr;
	CUnitMFA*                    pUnitMFA;

	WORD                         num_configured_Video_Ports = 0;
	WORD                         num_configured_ART_Ports   = 0;

	STATUS                       finalStatus                = STATUS_OK;
	CMediaUnitsList*             pMediaUnitslist;
	WORD                         numOfAudPartiesPerUnit     = 0;
	WORD                         numOfVidPartiesPerUnit     = 0;

	for (WORD boardId = 0; boardId < BOARDS_NUM; boardId++)
	{
		if (m_pSystemResources->IsBoardReady(boardId+1) != TRUE)
			continue;

		pMediaUnitslist        = m_pSystemResources->m_pBoards[boardId]->GetMediaUnitsList();
		numOfAudPartiesPerUnit = m_pSystemResources->m_pBoards[boardId]->CalculateNumberOfPortsPerUnit(PORT_ART);
		numOfVidPartiesPerUnit = m_pSystemResources->m_pBoards[boardId]->CalculateNumberOfPortsPerUnit(PORT_VIDEO);

		for (unitMFAItr = pMediaUnitslist->begin(); unitMFAItr != pMediaUnitslist->end(); unitMFAItr++)
		{
			pUnitMFA = ((CUnitMFA*)(&(*unitMFAItr)));
			if (pUnitMFA->GetIsEnabled() != TRUE)
				continue;

			switch (pUnitMFA->GetUnitType())
			{
				case eUnitType_Art:
				case eUnitType_Art_Control:
				num_configured_ART_Ports += numOfAudPartiesPerUnit;
				break;

				case eUnitType_Video:
				num_configured_Video_Ports += numOfVidPartiesPerUnit;
				break;

				default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
			} // switch
		}
	}

	if (m_pSystemResources->GetNumConfiguredArtPorts(ALL_BOARD_IDS) != num_configured_ART_Ports)
	{
		m_OStream << "Inconsistency between m_pSystemResources->GetNumConfiguredArtPorts(ALL_BOARD_IDS) ("
		          << m_pSystemResources->GetNumConfiguredArtPorts(ALL_BOARD_IDS) << ") and num_configured_ART_Ports ("
		          << num_configured_ART_Ports << ")\n";
		finalStatus = STATUS_FAIL;
	}

	if (m_pSystemResources->GetNumConfiguredVideoPorts(ALL_BOARD_IDS) != num_configured_Video_Ports)
	{
		m_OStream << "Inconsistency between m_pSystemResources->GetNumConfiguredVideoPorts(ALL_BOARD_IDS) ("
		          << m_pSystemResources->GetNumConfiguredVideoPorts(ALL_BOARD_IDS) << ") and num_configured_Video_Ports ("
		          << num_configured_Video_Ports << ")\n";
		finalStatus = STATUS_FAIL;
	}

	switch (m_pSystemResources->GetResourceAllocationType())
	{
		case eFixedBreezeMode:
		case eFixedMpmRxMode:
		{
			CFixedModeResources* pFixedModeResources = dynamic_cast<CFixedModeResources*>(m_pSystemResources->GetCurrentResourcesInterface());
			if (pFixedModeResources == NULL)
			{
				PASSERT(1);
				finalStatus = STATUS_FAIL;
			}
			else
			{
				float totalVideoPromilles = 0;
				float totalAudioPromilles = 0;
				pFixedModeResources->CalculateNeededAudioAndVideoPromilles(&(pFixedModeResources->m_enhancedConfig), totalAudioPromilles, totalVideoPromilles);
				int   neededArtUnits      = (int)ceil(totalAudioPromilles/1000);
				int   neededVideoUnits    = (int)ceil(totalVideoPromilles/1000);

				// it's Ok to use numOfAudPartiesPerUnit/numOfVidPartiesPerUnit because those are only BARAK cards...
				DWORD numOfRequiredArtPorts   = neededArtUnits * numOfAudPartiesPerUnit;
				DWORD numOfRequiredVideoPorts = neededVideoUnits * numOfVidPartiesPerUnit;
				DWORD actualNumOfArtPorts     = m_pSystemResources->GetNumConfiguredArtPorts(ALL_BOARD_IDS) + CCardResourceConfig::GetNumAudPortsLeftToConfig();
				DWORD actualNumOfVideoPorts   = m_pSystemResources->GetNumConfiguredVideoPorts(ALL_BOARD_IDS) + CCardResourceConfig::GetNumVidHD720PortsLeftToConfig();

				if (actualNumOfArtPorts < numOfRequiredArtPorts)
				{
					m_OStream << "Inconsistency: actualNumOfArtPorts ("
					          << actualNumOfArtPorts << ") is less than numOfRequiredArtPorts ("
					          << numOfRequiredArtPorts << ")\n";
					finalStatus = STATUS_FAIL;
				}

				if (actualNumOfVideoPorts < numOfRequiredVideoPorts)
				{
					m_OStream << "Inconsistency: actualNumOfVideoPorts ("
					          << actualNumOfVideoPorts << ") is less than numOfRequiredVideoPorts ("
					          << numOfRequiredVideoPorts << ")\n";
					finalStatus = STATUS_FAIL;
				}
			}
		}
		break;

		case eAutoBreezeMode:
		case eAutoMpmRxMode: // Tsahi TODO: need to calculate HD720 ports here!
		case eAutoMixedMode:
		{
			CAutoModeResources* pAutoModeResources = dynamic_cast<CAutoModeResources*>(m_pSystemResources->GetCurrentResourcesInterface());
			if (pAutoModeResources == NULL)
			{
				PASSERT(1);
				finalStatus = STATUS_FAIL;
			}
			else
			{
				// we will check that there are enough resources when all video parties are CIF
				DWORD numAudioOnlyPartiesInConfiguration = pAutoModeResources->m_audVidConfig.GetAudio();
				DWORD numVideoPartiesInConfiguration     = pAutoModeResources->m_audVidConfig.GetVideo();
				float totalAudioPromilles                = (numAudioOnlyPartiesInConfiguration + numVideoPartiesInConfiguration) * ART_PROMILLES_BARAK;
				float totalVideoPromilles                = numVideoPartiesInConfiguration * VID_TOTAL_CIF_PROMILLES_BARAK;
				DWORD totalNeededUnits                   = (DWORD)ceil((float)(totalAudioPromilles/1000)) + (DWORD)ceil((float)(totalVideoPromilles/1000));

				DWORD actualNumOfArtPorts                = m_pSystemResources->GetNumConfiguredArtPorts(ALL_BOARD_IDS) + CCardResourceConfig::GetNumAudPortsLeftToConfig();
				DWORD actualNumOfVideoPorts              = m_pSystemResources->GetNumConfiguredVideoPorts(ALL_BOARD_IDS) + CCardResourceConfig::GetNumVidHD720PortsLeftToConfig();
				float totalActualAudioPromilles          = actualNumOfArtPorts * ART_PROMILLES_BARAK;
				float totalActualVideoPromilles          = actualNumOfVideoPorts * VID_TOTAL_CIF_PROMILLES_BARAK;
				DWORD actualNumUnits                     = (DWORD)ceil((float)(totalActualAudioPromilles/1000)) + (DWORD)ceil((float)(totalActualVideoPromilles/1000));

				if (actualNumUnits < totalNeededUnits)
				{
					m_OStream << "Inconsistency: actualNumUnits ("
					          << actualNumUnits << ") is less than totalNeededUnits ("
					          << totalNeededUnits << ")\n";
					finalStatus = STATUS_FAIL;
				}
			}
		}
		break;

		default:
		PASSERT(1);
		finalStatus = STATUS_FAIL;
		break;
	} // switch

	return finalStatus;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckNIDs()
{
	m_OStream << "**Checking NIDs\n";
	// we will check in this function that all NIDs are unique
	// we will do this by entering the NIDs in a map, and checking if it already was there
	STATUS                                          finalStatus = STATUS_OK;

	std::map<char*, CSleepingConference*>*          pMapOfNIDs  = new std::map<char*, CSleepingConference*>;
	std::map<char*, CSleepingConference*>::iterator pairsIterator;

	// std::set<CConfIds>::iterator i;
	SleepingConferences::iterator         i;
	char*                                           strNID = NULL;

	for (i = m_pProcess->GetSleepingConferences()->begin(); i != m_pProcess->GetSleepingConferences()->end(); i++)
	{
		strNID = i->GetNumConfId();
		if (strNID == NULL)
		{
			// each conference should have a NID
			m_OStream << "Found a conference with no NID. MonitorConfId is " << i->GetMonitorConfId()<< "\n";
			finalStatus = STATUS_FAIL;
			continue;
		}

		pairsIterator = pMapOfNIDs->find(strNID);
		if (pairsIterator != pMapOfNIDs->end())
		{
			// we found it, this is not good, because it should be unique
			m_OStream << "Found a conference with duplicate NID (" << strNID << "). MonitorConfId is " << i->GetMonitorConfId()
			          << " The other conference with this NID has MonitorConfID: " << pairsIterator->second->GetMonitorConfId() << "\n";
			finalStatus = STATUS_FAIL;
			continue;
		}

		// (*pMapOfNIDs)[strNID] = ((CConfIds*)(&(*i)));
		(*pMapOfNIDs)[strNID] = ((CSleepingConference*)(&(*i)));
	}

	// cleanup
	pMapOfNIDs->clear();
	delete pMapOfNIDs;

	return finalStatus;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckPhones()
{
	m_OStream << "**Checking Phones\n";
	// we will check in this function that all phones are unique and that they are in the appropriate service
	// we will do this by entering the phones in a map, and checking if it already was there
	STATUS                                          finalStatus  = STATUS_OK, tempStatus;

	std::map<char*, CSleepingConference*>*          pMapOfPhones = new std::map<char*, CSleepingConference*>;
	std::map<char*, CSleepingConference*>::iterator pairsIterator;

	// std::set<CConfIds>::iterator confIterator;
	SleepingConferences::iterator         confIterator;
	CSleepingConference*                            pConf;
	char*                                           strPhone;
	CServicePhoneStr*                               pPhoneStr;
	Phone*                                          phone;

	char*                                           service_name;
	CNetServiceRsrcs*                               pService;
	std::set<CPhone>*                               pPhonesListInService;

	for (confIterator = m_pProcess->GetSleepingConferences()->begin(); confIterator != m_pProcess->GetSleepingConferences()->end(); confIterator++)
	{
		pConf     = ((CSleepingConference*)(&(*confIterator)));
		pPhoneStr = pConf->GetFirstServicePhone();
		while (pPhoneStr != NULL)
		{
			service_name = (char*)(pPhoneStr->GetNetServiceName());
			pService     = (CNetServiceRsrcs*)(m_pSystemResources->findServiceByName(service_name));
			if (pService == NULL)
			{
				m_OStream << "Found a conference with invalid service name. MonitorConfId is " << confIterator->GetMonitorConfId()
				          << " service_name is: " << service_name << "\n";

				pPhonesListInService = NULL;
				finalStatus          = STATUS_FAIL;
			}
			else
			{
				pPhonesListInService = pService->GetPhonesList();
			}

			phone = pPhoneStr->GetFirstPhoneNumber();
			while (phone != NULL)
			{
				strPhone = phone->phone_number;

				// check that the phone number is in the right service, and that it's written there as busy
				tempStatus = CheckPhoneNumberIsBusyInPhonesList(strPhone, pPhonesListInService);
				if (tempStatus != STATUS_OK)
				{
					finalStatus = tempStatus;
				}

				pairsIterator = pMapOfPhones->find(strPhone);
				if (pairsIterator != pMapOfPhones->end())
				{
					// we found it, this is not good, because it should be unique
					m_OStream << "Found a conference with duplicate phone (" << strPhone << "). MonitorConfId is " << pConf->GetMonitorConfId()
					          << " The other conference with this phone has MonitorConfID: " << pairsIterator->second->GetMonitorConfId() << "\n";
					finalStatus = STATUS_FAIL;
				}
				else
				{
					(*pMapOfPhones)[strPhone] = ((CSleepingConference*)(&(*confIterator)));
				}

				phone = pPhoneStr->GetNextPhoneNumber();
			}

			pPhoneStr = pConf->GetNextServicePhone();
		}
	}

	// cleanup
	pMapOfPhones->clear();
	delete pMapOfPhones;

	return finalStatus;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckPhoneNumberIsBusyInPhonesList(char* strPhone, std::set<CPhone>* pPhonesListInService)
{
	if (strPhone == NULL || pPhonesListInService == NULL)
	{
		m_OStream << "Couldn't check PhoneNumberIsBusyInPhonesList, one of the params is NULL\n";
		return STATUS_FAIL;
	}

	CPhone                     existPhone = CPhone(strPhone);
	std::set<CPhone>::iterator i          = pPhonesListInService->find(existPhone);

	if (i == pPhonesListInService->end())
	{
		m_OStream << "Couldn't find strPhone (" <<strPhone << ") in pPhonesListInService\n";
		return STATUS_FAIL;
	}
	else
	{
		if (i->IsBusy() == FALSE)
		{
			m_OStream << "Phone (" <<strPhone << ") is not marked as busy in pPhonesListInService\n";
			return STATUS_FAIL;
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckParties(CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck)
{
	m_OStream << "**Checking Parties\n";

	// //////////////////////////////////////////////////
	// this function will check the consistency between the parties resources list and the descriptors list
	// it will also check that the total number of video and audio parties is correct
	// And it will check that everything is written in the shared memory
	// //////////////////////////////////////////////////
	STATUS finalStatus  = STATUS_OK;

	PartyDetailsList* pPartyDetailsList = new PartyDetailsList;

	// build the total list
	if (BuildPartyDetailsList(pPartyDetailsList, pSelfConsistencyWhatToCheck) != STATUS_OK)
		finalStatus = STATUS_FAIL;

	// check that the total list is consistent
	if (CheckPartyDetailsListConsistency(pPartyDetailsList, pSelfConsistencyWhatToCheck) != STATUS_OK)
		finalStatus = STATUS_FAIL;

	// cleanup list
	CleanUpPartyDetailsList(pPartyDetailsList);
	delete pPartyDetailsList;

	return finalStatus;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::BuildPartyDetailsList(PartyDetailsList* pPartyDetailsList, CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck)
{
	// //////////////////////////////////////////////////
	// go over all conferences, and get the data from the parties list
	// then integrate it with the data of the resources descriptors, the udp ports list, and the MFA units
	// //////////////////////////////////////////////////
	STATUS finalStatus = STATUS_OK;

	for (std::set<CConfRsrc>::iterator _confRsrc = m_pConfRsrcDB->m_confList.begin(); _confRsrc != m_pConfRsrcDB->m_confList.end(); ++_confRsrc)
	{
		WORD numOfPartiesInConf = 0;
		CConfRsrc* pConfRsrc = ((CConfRsrc*)(&(*_confRsrc)));

		const PARTIES* pParties = _confRsrc->GetPartiesList();

		// go over the parties list in the conference and create an CSelfConsistencyPartyRsrcDetails with all the details
		PARTIES::iterator _partyEnd = pParties->end();
		for (PARTIES::iterator _partyRsrc = pParties->begin(); _partyRsrc != _partyEnd; ++_partyRsrc)
		{
			CPartyRsrc* pPartyRsrc = ((CPartyRsrc*)(&(*_partyRsrc)));
			std::string key = GetPartyKey(pConfRsrc->GetRsrcConfId(), pPartyRsrc->GetRsrcPartyId());
			PartyDetailsList::iterator pairsIterator = pPartyDetailsList->find(key);
			if (pairsIterator != pPartyDetailsList->end())
			{
				CSelfConsistencyPartyRsrcDetails* pPartyRsrcDetails = pairsIterator->second;
				std::string existingKey = pairsIterator->first;
				m_OStream << "Conference with MonitorConfId " << _confRsrc->GetMonitorConfId() << " has got same party twice!"
				          << " pConfRsrc->GetRsrcConfId() is " << _confRsrc->GetRsrcConfId()
				          <<  "\n Party to be inserted: "
				          << " pPartyRsrc->GetRsrcPartyId() is " << pPartyRsrc->GetRsrcPartyId()
				          << " key.data() is " << key.data()
				          <<  "\n Existing party: "
				          << " existingKey.data() is " << existingKey.data()
				          << "\n";

				pPartyRsrcDetails->GetOStreamAndPrintParty();

				finalStatus = STATUS_FAIL;
			}
			else
			{
				CSelfConsistencyPartyRsrcDetails* pPartyRsrcDetails = new CSelfConsistencyPartyRsrcDetails(pPartyRsrc, _confRsrc->GetRsrcConfId());
				(*pPartyDetailsList)[key] = pPartyRsrcDetails;
			}

			numOfPartiesInConf++;
		}

		if (pConfRsrc->m_num_Parties != numOfPartiesInConf)
		{
			m_OStream << "Conference with MonitorConfId " << pConfRsrc->GetMonitorConfId() << " has not the right number of parties! "
			          << "pConfRsrc->m_num_Paries is " << pConfRsrc->m_num_Parties << " numOfPartiesInConf is " << numOfPartiesInConf << "\n";
			finalStatus = STATUS_FAIL;
		}

		if (pSelfConsistencyWhatToCheck->m_bCheckPartiesResourceDescriptors)
		{
			// go over all descriptors in the conference, and update the correct party with the resource descriptor
			if (AddResourceDescriptorsToPartyList(pPartyDetailsList, pConfRsrc, pSelfConsistencyWhatToCheck->m_bCheckPartiesResourceDescriptorsCheckSharedMemory) != STATUS_OK)
				finalStatus = STATUS_FAIL;
		}

		if (pSelfConsistencyWhatToCheck->m_bCheckPartiesUDPDescriptors)
		{
			// go over all UDP descriptors in the conference, and update the correct party with the UDP descriptor
			if (AddUDPRsrcDescriptorsToPartyList(pPartyDetailsList, pConfRsrc) != STATUS_OK)
				finalStatus = STATUS_FAIL;
		}
	}

	if (pSelfConsistencyWhatToCheck->m_bCheckPartiesPorts)
	{
		// go over all MFA units and see which one is occupied by what party
		if (AddActivePortsToPartyList(pPartyDetailsList) != STATUS_OK)
			finalStatus = STATUS_FAIL;
	}

	return finalStatus;
}

//--------------------------------------------------------------------------
void CSelfConsistency::CleanUpPartyDetailsList(PartyDetailsList* pPartyDetailsList)
{
	PartyDetailsList::iterator        pairsIterator;
	CSelfConsistencyPartyRsrcDetails* pPartyRsrcDetails;

	for (pairsIterator = pPartyDetailsList->begin(); pairsIterator != pPartyDetailsList->end(); pairsIterator++)
	{
		pPartyRsrcDetails = pairsIterator->second;
		delete pPartyRsrcDetails;
	}

	pPartyDetailsList->clear();
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckDescriptorInSharedMemory(CRsrcDesc* pRsrcdesc)
{
	// /////////////////////////////////////
	// This function checks that all descritors are written in the shared memory
	// /////////////////////////////////////

	TRACEINTO << "D.K. ConnId:" << pRsrcdesc->GetConnId() << ", PartyId:" << pRsrcdesc->GetRsrcPartyId() << ", ConfId:" << pRsrcdesc->GetRsrcConfId() << ", Type:" << pRsrcdesc->GetType();

	ConnToCardTableEntry entry;
	if (m_pConnToCardManager->Get(pRsrcdesc->GetConnId(), entry) != STATUS_OK)
	{
		m_OStream << "Couldn't find resource descriptor in shared memory. pRsrcdesc: " << pRsrcdesc << "\n";
		return STATUS_FAIL;
	}

	if (pRsrcdesc->GetRsrcConfId() != entry.rsrc_conf_id
	    || pRsrcdesc->GetRsrcPartyId() != entry.rsrc_party_id
	    || pRsrcdesc->GetBoxId() != entry.boxId
	    || pRsrcdesc->GetBoardId() != entry.boardId
	    || pRsrcdesc->GetSubBoardId() != entry.subBoardId
	    || pRsrcdesc->GetUnitId() != entry.unitId
	    || pRsrcdesc->GetAcceleratorId() != entry.acceleratorId
	    || pRsrcdesc->GetFirstPortId() != entry.portId
	    || pRsrcdesc->GetPhysicalType() != entry.physicalRsrcType
	    || pRsrcdesc->GetType() != entry.rsrcType
	    )
	{
		// TBD, dump all the details??? Or maybe use a ready dump function
		m_OStream << "Inconsistency between resource descriptor and entry in shared memory. pRsrcdesc->GetRsrcConfId():  "
		          << pRsrcdesc->GetRsrcConfId() << " entry.rsrc_conf_id: " << entry.rsrc_conf_id
		          << " pRsrcdesc->GetPhysicalType(): " << pRsrcdesc->GetPhysicalType()
		          << " entry.physicalRsrcType: " << entry.physicalRsrcType << "\n";
		return STATUS_FAIL;
	}

	// We write master only in shared memory, not in descriptors. But slave or none, we write in both.
	// so check it only when it's not master
	if (!CHelperFuncs::IsMasterType(entry.rsrcCntlType))
	{
		if (pRsrcdesc->GetCntrlType() != entry.rsrcCntlType)
		{
			m_OStream << "Inconsistency between resource descriptor and entry in shared memory, on control type. pRsrcdesc->GetRsrcConfId():  "
			          << pRsrcdesc->GetRsrcConfId() << " entry.rsrc_conf_id: " << entry.rsrc_conf_id
			          << " pRsrcdesc->GetCntrlType(): " << pRsrcdesc->GetCntrlType()
			          << " entry.rsrcCntlType: " << entry.rsrcCntlType << "\n";
			return STATUS_FAIL;
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::AddUDPRsrcDescriptorsToPartyList(PartyDetailsList* pPartyDetailsList, CConfRsrc* pConfRsrc)
{
	std::set<CUdpRsrcDesc>::iterator  UdpRsrcDescItr;
	CUdpRsrcDesc*                     pUDPRsrcdesc;
	CSelfConsistencyPartyRsrcDetails* pPartyRsrcDetails;

	STATUS                            finalStatus = STATUS_OK;

	for (UdpRsrcDescItr = pConfRsrc->m_pUdpRsrcDescList->begin(); UdpRsrcDescItr != pConfRsrc->m_pUdpRsrcDescList->end(); UdpRsrcDescItr++)
	{
		pUDPRsrcdesc      = ((CUdpRsrcDesc*)(&(*UdpRsrcDescItr)));
		pPartyRsrcDetails = GetPartyFromList(pPartyDetailsList, pConfRsrc->GetRsrcConfId(), pUDPRsrcdesc->GetRsrcPartyId());
		if (pPartyRsrcDetails == NULL)
		{
			// all parties should be in the parties list, so it should already be there
			m_OStream << "Didn't find party rsrc details with RsrcConfId " << pConfRsrc->GetRsrcConfId() <<
			" and RsrcPartyId " << pUDPRsrcdesc->GetRsrcPartyId() <<  "\n";
			finalStatus = STATUS_FAIL;
			continue;
		}

		pPartyRsrcDetails->AddUDPDescriptor(pUDPRsrcdesc);
	}

	return finalStatus;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::AddResourceDescriptorsToPartyList(PartyDetailsList* pPartyDetailsList, CConfRsrc* pConfRsrc, BOOL bCheckPartiesResourceDescriptorsCheckSharedMemory)
{
	STATUS finalStatus = STATUS_OK;

	for (std::set<CRsrcDesc>::iterator _rsrcDesc = pConfRsrc->m_pRsrcDescList.begin(); _rsrcDesc != pConfRsrc->m_pRsrcDescList.end(); ++_rsrcDesc)
	{
		CRsrcDesc* pRsrcdesc = ((CRsrcDesc*)(&(*_rsrcDesc)));
		CSelfConsistencyPartyRsrcDetails* pPartyRsrcDetails = GetPartyFromList(pPartyDetailsList, pConfRsrc->GetRsrcConfId(), pRsrcdesc->GetRsrcPartyId());
		if (pPartyRsrcDetails == NULL)
		{
			// all parties should be in the parties list, so it should already be there
			m_OStream << "Didn't find party rsrc details with RsrcConfId " << pConfRsrc->GetRsrcConfId() <<
			" and RsrcPartyId " << pRsrcdesc->GetRsrcPartyId() <<  "\n";
			finalStatus = STATUS_FAIL;
			continue;
		}

		pPartyRsrcDetails->AddResourceDescriptor(pRsrcdesc);
		if (bCheckPartiesResourceDescriptorsCheckSharedMemory)
		{
			if (CheckDescriptorInSharedMemory(pRsrcdesc) != STATUS_OK)
				finalStatus = STATUS_FAIL;
		}
	}

	return finalStatus;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::AddActivePortsToPartyList(PartyDetailsList* pPartyDetailsList)
{
	std::set<CUnitMFA>::iterator      unitMFAItr;
	CActivePortsList::iterator        activePortItr;
	CUnitMFA*                         pUnitMFA;
	CActivePort*                      pActivePort;
	const CActivePortsList*           pListOfActivePorts;
	CSelfConsistencyPartyRsrcDetails* pPartyRsrcDetails;

	STATUS                            finalStatus = STATUS_OK;

	CMediaUnitsList*                  pMediaUnitslist;

	for (WORD boardId = 0; boardId < BOARDS_NUM; boardId++)
	{
		pMediaUnitslist = m_pSystemResources->m_pBoards[boardId]->GetMediaUnitsList();

		for (unitMFAItr = pMediaUnitslist->begin(); unitMFAItr != pMediaUnitslist->end(); unitMFAItr++)
		{
			pUnitMFA           = ((CUnitMFA*)(&(*unitMFAItr)));
			pListOfActivePorts = pUnitMFA->GetActivePorts();
			if (pListOfActivePorts == NULL)
			{
				m_OStream << "List of active ports is NULL on unit mfa on board: " << pUnitMFA->GetBoardId()
				          << " , on unit: " << pUnitMFA->GetUnitId() <<  "\n";

				finalStatus = STATUS_FAIL;
				continue;
			}

			int size     = pListOfActivePorts->size();
			int numAdded = 0;
			for (activePortItr = pListOfActivePorts->begin(); activePortItr != pListOfActivePorts->end(); activePortItr++)
			{
				pActivePort       = ((CActivePort*)(&(*activePortItr)));
				pPartyRsrcDetails = GetPartyFromList(pPartyDetailsList, pActivePort->GetConfId(), pActivePort->GetPartyId());
				if (pPartyRsrcDetails == NULL)
				{
					// all parties should be in the parties list, so it should already be there
					m_OStream << "Didn't find party rsrc details with ConfId " << pActivePort->GetConfId() <<
					" and PartyId " << pActivePort->GetPartyId() <<  "\n";
					finalStatus = STATUS_FAIL;
					continue;
				}

				pPartyRsrcDetails->AddActivePort(pActivePort, m_pSystemResources->m_pBoards[boardId]);
				numAdded++;
			}

			if (size != numAdded)
			{
				m_OStream << "Inconsistency between size of list of active ports (" << size << ") and numAdded ("
				          << numAdded << ") on unit mfa on board: " << pUnitMFA->GetBoardId()
				          << " , on unit: " << pUnitMFA->GetUnitId() << "\n";
				finalStatus = STATUS_FAIL;
			}
		}
	}

	return finalStatus;
}

//--------------------------------------------------------------------------
CSelfConsistencyPartyRsrcDetails* CSelfConsistency::GetPartyFromList(PartyDetailsList* pPartyDetailsList, DWORD rsrcConfId, DWORD rsrcPartyId)
{
	PartyDetailsList::iterator pairsIterator = pPartyDetailsList->find(GetPartyKey(rsrcConfId, rsrcPartyId));
	if (pairsIterator == pPartyDetailsList->end())
		return NULL;
	else
		return pairsIterator->second;
}

//--------------------------------------------------------------------------
std::string CSelfConsistency::GetPartyKey(DWORD rsrcConfId, DWORD rsrcPartyId)
{
	std::ostringstream o;
	o << rsrcConfId << "_" << rsrcPartyId;
	return o.str();
}

//--------------------------------------------------------------------------
STATUS CSelfConsistency::CheckPartyDetailsListConsistency(PartyDetailsList* pPartyDetailsList, CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck)
{
	CFixedModeResources* pFixedModeResources = NULL;
	CAutoModeResources*  pAutoModeResources  = NULL;
	if (m_pSystemResources->GetResourceAllocationType() == eFixedBreezeMode)
	{
		pFixedModeResources = dynamic_cast<CFixedModeResources*>(m_pSystemResources->GetCurrentResourcesInterface());
		if (pFixedModeResources == NULL)
		{
			m_OStream << "pFixedModeResources == NULL\n";
			PASSERT(1);
			return STATUS_FAIL;
		}
	}
	else
	{
		pAutoModeResources = dynamic_cast<CAutoModeResources*>(m_pSystemResources->GetCurrentResourcesInterface());
		if (pAutoModeResources == NULL)
		{
			m_OStream << "pAutoModeResources == NULL\n";
			PASSERT(1);
			return STATUS_FAIL;
		}
	}

	std::map<std::string, CSelfConsistencyPartyRsrcDetails*>::iterator pairsIterator;
	CSelfConsistencyPartyRsrcDetails* pPartyRsrcDetails;

	// for mixed mode
	float totalNumVideoHD720Units  = 0;
	float totalNumAudioOnlyParties = 0;
	// for pure mode
	DWORD numOfCurrentPartiesPerType[NUM_OF_PARTY_RESOURCE_TYPES];
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		numOfCurrentPartiesPerType[i] = 0;
	}

	ePartyResourceTypes partyResourceTypes;
	eVideoPartyType videoPartyType;

	STATUS finalStatus = STATUS_OK;

	BOOL isMPMX = (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_breeze == m_pSystemResources->GetSystemCardsMode());

	// //////////////////////////////////////////////////
	// go over the total list of parties in all conferences, count the parties according to type
	// and check that each party on itself is consistent (has the right number of ports, descriptors, etc)
	// //////////////////////////////////////////////////
	for (pairsIterator = pPartyDetailsList->begin(); pairsIterator != pPartyDetailsList->end(); pairsIterator++)
	{
		pPartyRsrcDetails = pairsIterator->second;
		if (pPartyRsrcDetails->CheckConsistency(pSelfConsistencyWhatToCheck) != STATUS_OK)
			finalStatus = STATUS_FAIL;

		videoPartyType = pPartyRsrcDetails->GetPartyType();
		if (m_pSystemResources->GetResourceAllocationType() == eFixedBreezeMode)
		{
			partyResourceTypes = CFixedModeResources::VideoPartyTypeToPartyResourceType(videoPartyType);
			numOfCurrentPartiesPerType[partyResourceTypes]++;
		}
		else
		{
			if (isMPMX && (CHelperFuncs::IsAudioParty(videoPartyType) || CHelperFuncs::IsAudioContentParty(videoPartyType)))
				totalNumAudioOnlyParties += 1;
			else
				totalNumVideoHD720Units += pAutoModeResources->GetLogicalWeightForResourceCalculations(videoPartyType, pPartyRsrcDetails->GetPartyRole(), eNonMix);
		}
	}

	if (m_pSystemResources->GetResourceAllocationType() == eFixedBreezeMode)
	{
		// update the number with the glides
		for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		{
			for (int j = 0; j < NUM_OF_PARTY_RESOURCE_TYPES; j++)
			{
				if (j < i)
				{
					// add glide to
					numOfCurrentPartiesPerType[i] += pFixedModeResources->m_numOfCurrentGlidesPerType[j][i];
				}
				else if (j > i)
				{
					// remove glide from
					numOfCurrentPartiesPerType[i] -= pFixedModeResources->m_numOfCurrentGlidesPerType[i][j];
				}
			}
		}

		// now check if this is what we have
		for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		{
			if (numOfCurrentPartiesPerType[i] != pFixedModeResources->m_numOfCurrentPartiesPerType[i])
			{
				m_OStream << "Inconsistency: CheckPartyDetailsListConsistency for party type " << i << ": numOfCurrentPartiesPerType[i] ("
				<< numOfCurrentPartiesPerType[i] << ") is not equal to pFixedModeResources->m_numOfCurrentPartiesPerType[i] ("
				<< pFixedModeResources->m_numOfCurrentPartiesPerType[i] << ")\n";

				finalStatus = STATUS_FAIL;
			}
		}
	}
	else
	{
		if (totalNumVideoHD720Units != pAutoModeResources->GetTotalAllocatedVideoParties())
		{
			m_OStream << "Inconsistency: CheckPartyDetailsListConsistency totalNumVideoHD720Units ("
			          << totalNumVideoHD720Units << ") is not equal to pAutoModeResources->GetTotalAllocatedVideoParties() ("
			          << pAutoModeResources->GetTotalAllocatedVideoParties() << ")\n";
			finalStatus = STATUS_FAIL;
		}

		if (totalNumAudioOnlyParties != pAutoModeResources->GetTotalAllocatedAudioParties(isMPMX))
		{
			m_OStream << "Inconsistency: CheckPartyDetailsListConsistency totalNumAudioOnlyParties ("
			          << totalNumAudioOnlyParties << ") is not equal to pAutoModeResources->GetTotalAllocatedAudioParties() ("
			          << pAutoModeResources->GetTotalAllocatedAudioParties(isMPMX) << ")\n";
			finalStatus = STATUS_FAIL;
		}
	}

	return finalStatus;
}

//--------------------------------------------------------------------------
CSelfConsistency* CSelfConsistency::Instantiate()
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new CSelfConsistency();
	}

	return m_pInstance;
}

//--------------------------------------------------------------------------
void CSelfConsistency::TearDown()
{
	if (m_pInstance != NULL && (CPObject::IsValidPObjectPtr(m_pInstance)))
	{
		delete m_pInstance;
	}
}

//--------------------------------------------------------------------------
COstrStream& CSelfConsistency::GetOStream()
{
	return Instantiate()->m_OStream;
}

//--------------------------------------------------------------------------
void CSelfConsistency::ClearStream()
{
	// the following line will clean the buffer, see: http://www.velocityreviews.com/forums/t275945-re-how-to-clear-ostringstream-buffer.html
	m_OStream.str("");
}

//--------------------------------------------------------------------------
void CSelfConsistency::PrintStatus(STATUS status)
{
	if (status == STATUS_OK)
		m_OStream << "Status is STATUS_OK\n";
	else if (status == STATUS_FAIL)
		m_OStream << "Status is STATUS_FAIL\n";
	else
		m_OStream << "Status number is " << status << "\n";
}
//--------------------------------------------------------------------------
