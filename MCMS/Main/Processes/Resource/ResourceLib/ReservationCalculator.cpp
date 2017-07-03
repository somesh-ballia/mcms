#include "TraceStream.h"
#include "ReservationCalculator.h"
#include "HelperFuncs.h"
#include "SystemResources.h"
#include "EnumsAndDefines.h"
#include "Reservator.h"
#include "ProcessBase.h"

#undef min
#include "PrettyTable.h"

////////////////////////////////////////////////////////////////////////////
//                        CReservationCalculator
////////////////////////////////////////////////////////////////////////////
CReservationCalculator::CReservationCalculator()
{
	m_pRsrcIntervals    = new RSRC_INTERVAL_ARRAY;
	m_pNID_list         = new std::set<CPhone>;
	m_pPhoneList        = new std::set<CPhone>;
	m_netServiceName[0] = '\0';
	m_pIpServicesCalc   = new std::set<CIpServiceIntervals*>;
}

//--------------------------------------------------------------------------
CReservationCalculator::~CReservationCalculator()
{
	Cleanup();

	POBJDELETE(m_pRsrcIntervals);
	POBJDELETE(m_pNID_list);
	POBJDELETE(m_pPhoneList);

	for (std::set<CIpServiceIntervals*>::iterator _ii = m_pIpServicesCalc->begin(); _ii != m_pIpServicesCalc->end(); ++_ii)
	{
		CIpServiceIntervals* pCIpServiceIntervals = *(_ii);
		PDELETE(pCIpServiceIntervals);
	}

	m_pIpServicesCalc->clear();
	POBJDELETE(m_pIpServicesCalc);
}

//--------------------------------------------------------------------------
CInterval* CReservationCalculator::GetIntrvalById(Interval_Id intrvlID)
{
	RSRC_INTERVAL_ARRAY::iterator itr = m_pRsrcIntervals->find(intrvlID);

	if (itr != m_pRsrcIntervals->end())
		return ((*itr).second);
	return NULL;
}

//--------------------------------------------------------------------------
void CReservationCalculator::Cleanup()
{
	for (RSRC_INTERVAL_ARRAY::iterator _ii = m_pRsrcIntervals->begin(); _ii != m_pRsrcIntervals->end(); ++_ii)
		POBJDELETE((*_ii).second);

	m_pRsrcIntervals->clear();
	m_pNID_list->clear();
	if (m_pPhoneList)
	m_pPhoneList->clear();

	m_pPhonesMap.clear();//VNGFE-7414
	m_netServiceName[0] = '\0';

	for (std::set<CIpServiceIntervals*>::iterator _ii = m_pIpServicesCalc->begin(); _ii != m_pIpServicesCalc->end(); ++_ii)
		(*(_ii))->Cleanup();
}

//--------------------------------------------------------------------------
void CReservationCalculator::SetNetServiceName(const char* name)
{
	strcpy_safe(m_netServiceName, name);
}

//--------------------------------------------------------------------------
void CReservationCalculator::SetPhonesList(std::set<CPhone>& pPhoneList)
{
	if (m_pPhoneList)
		m_pPhoneList->clear();
	else
		m_pPhoneList = new std::set<CPhone>;

	m_pPhonesMap.clear();//VNGFE-7414
	for (std::set<CPhone>::iterator _ii = pPhoneList.begin(); _ii != pPhoneList.end(); _ii++)
	{
		m_pPhoneList->insert(*_ii);
		ULONGLONG phoneNum = atoll(_ii->GetNumber());
		m_pPhonesMap[phoneNum] = _ii->GetNumber();
	}
}

//--------------------------------------------------------------------------
STATUS CReservationCalculator::ReserveAmount(DWORD start, DWORD end, CIntervalRsrvAmount& neededAmount,
                                             BOOL isFromPassive, BOOL isNewRes, ePhoneAllocationTypes allocPhone, DWORD ipServiceId)
{
	bool isMultipleService = CHelperFuncs::IsMultipleService();

	std::ostringstream msg;
	msg.precision(0);
	msg << "NumericConfId:"       << neededAmount.GetNID()
	    << ", start:"             << start
	    << ", end:"               << end
	    << ", isFromPassive:"     << (int)isFromPassive
	    << ", isNewRes:"          << (int)isNewRes
	    << ", isAllocPhone:"      << (int)allocPhone
	    << ", isMultipleService:" << (int)isMultipleService
	    << ", ipServiceId:"       << ipServiceId;

	if (start != 0 || end != 0)
	{
		msg << "\n --------+----------";
		msg << "\n parties | needed   ";
		msg << "\n type    | parties  ";
		msg << "\n --------+----------";
		for (int i = e_Audio; i < NUM_OF_PARTY_RESOURCE_TYPES; ++i)
		{
			ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
			msg << "\n " << setw(8) << left << partyResourceType << "|"
			    << " " << setw(5) << left << neededAmount.m_PartiesResources.m_logical_num_parties[i];
		}
		msg << "\n --------+----------";
	}
	TRACEINTO << msg.str().c_str();

	STATUS status = ReserveAmountOnSystem(start, end, neededAmount, isFromPassive, isNewRes, allocPhone);
	if (status != STATUS_OK)
	{
		TRACEINTO << "status:" << status << " - Failed, Cannot reserve on System";
		return status;
	}

	if (isMultipleService && ipServiceId != ID_ALL_IP_SERVICES)
	{
		CIpServiceIntervals* ipServiceIntervals = GetIpServiceIntervalsCalc(ipServiceId);
		if (ipServiceIntervals == NULL)
		{
			TRACEINTO << "ipServiceId:" << ipServiceId << " - Failed, IP Service not found";
			return STATUS_FAIL;
		}
		status = ipServiceIntervals->ReserveAmount(start, end, neededAmount, isFromPassive, isNewRes, allocPhone);
		if (status != STATUS_OK)
		{
			TRACEINTO << "status:" << status << " - Failed, Cannot reserve on IP Service";
			return status;
		}
	}
	return status;
}

//--------------------------------------------------------------------------
STATUS CReservationCalculator::ReserveAmountOnSystem(DWORD start, DWORD end, CIntervalRsrvAmount& neededAmount,
                                                     BOOL isFromPassive, BOOL isNewRes, ePhoneAllocationTypes allocPhone)
{
	eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
	if (eMcuState_Normal == systemState)
	{
		std::ostringstream msg;
		msg << "NumericConfId:" << neededAmount.GetNID();

		if (m_pNID_list->size())
		{
			msg << ", AlreadyReserved:";
			for (std::set<CPhone>::iterator _ii = m_pNID_list->begin(); _ii != m_pNID_list->end(); ++_ii)
			{
				if (_ii != m_pNID_list->begin())
					msg << ",";
				msg << _ii->GetNumber();
			}
		}
		TRACEINTO << msg.str().c_str();
	}

	if (!isFromPassive && strlen(neededAmount.GetNID()) != 0)
	{
		CPhone* pStrNID = new CPhone(neededAmount.GetNID());
		if (m_pNID_list->find(*pStrNID) != m_pNID_list->end())
		{
			TRACEINTO << "NumericConfId:" << neededAmount.GetNID() << ", Number:" << pStrNID->GetNumber() << " - Reservation exists";
			if (isNewRes)
			{
				if (!strcmp(pStrNID->GetNumber(), "7001"))
				{
					POBJDELETE(pStrNID);
					return STATUS_NUMERIC_CONF_ID_OCCUPIED_BY_SIP_FACTORY;
				}
				else
				{
					POBJDELETE(pStrNID);
					return STATUS_NUMERIC_CONFERENCE_ID_OCCUPIED;
				}
			}
		}
		else
			m_pNID_list->insert(*pStrNID);

		POBJDELETE(pStrNID);
	}

	STATUS status = STATUS_OK;

	// inserting phone's occupation
	if (!isFromPassive && m_netServiceName[0] != '\0')
	{
		CServicePhoneStr* pPhoneStr = (CServicePhoneStr*)neededAmount.GetServicePhone();  // olga
		status = PhoneCheckOrAlloc(pPhoneStr, allocPhone);
		if (STATUS_OK != status)
			return status;
	}

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	WORD max_num_of_conf = 0;
	if (pSystemResources->IsEventMode())
	{
		max_num_of_conf = pSystemResources->GetMaxNumOngoingConfPerCardsInEventMode();
		if (max_num_of_conf < 8)
		{
			// ICBC patch: because when loading new version we start the Reservations DB before receiving  CM_CARD_MNGR_LOADED_IND
			// we get max_num_of_conf = 0 , since ICBC uses RMX2000 with 2 MPM+ , and we need version today,  (yes, yes) we will set max_num_of_conf to 8
			max_num_of_conf = 8;
		}
	}
	else
		max_num_of_conf = pSystemResources->GetMaxNumberOfOngoingConferences();

	WORD max_num_vsw_conf = pSystemResources->GetMaxNumberVSWConferencesEventMode();

	for (DWORD i = start; i <= end; i++)
	{
		CInterval* pInterval = GetIntrvalById(i);

		if (!pInterval) // create intervals for constant range in functions above???
		{
			pInterval = new CInterval(i, m_totalPartiesResources, max_num_of_conf, max_num_vsw_conf);
			(*m_pRsrcIntervals)[i] = pInterval;
		}

		status = pInterval->Reserve(neededAmount);

		// roll-back , all intervals behind have to exist.
		if (STATUS_OK != status)
		{
			if (i != start)
			{
				for (DWORD j = i-1; j >= start; j--)
				{
					pInterval = GetIntrvalById(j);
					if (pInterval)
						pInterval->UnReserve(neededAmount);

					if (j == 0) // don't go lower than 0, DWORD is unsigned and if we do j-- we will get a very big number
					{
						break;
					}
				}

				if (!isFromPassive && strlen(neededAmount.GetNID()) != 0)
				{
					CPhone strNID(neededAmount.GetNID());
					if (m_pNID_list->find(strNID) != m_pNID_list->end())
						m_pNID_list->erase(strNID);
					else
						PASSERT(1);
				}
			}
			break;
		}
	}
	return status;
}

//--------------------------------------------------------------------------
void CReservationCalculator::SetLogicalResources(WORD logical_num_parties[NUM_OF_PARTY_RESOURCE_TYPES])
{
	if (CHelperFuncs::IsMode2C())
	{
		m_totalPartiesResources.m_logical_COP_num_parties = logical_num_parties[0];
	}
	else
	{
		for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
			m_totalPartiesResources.m_logical_num_parties[i] = logical_num_parties[i];

		// VNGR-25735 D.K.
		// Update all multiple services with new logical number of parties
		for (std::set<CIpServiceIntervals*>::iterator _ii = m_pIpServicesCalc->begin(); _ii != m_pIpServicesCalc->end(); ++_ii)
		{
			(*_ii)->SetLogicalResources(m_totalPartiesResources.m_logical_num_parties);
			(*_ii)->MultiplyTotalPartiesResourcesByFactor();
		}
	}
}

//--------------------------------------------------------------------------
STATUS CReservationCalculator::ReservePhoneIfNeeded(CServicePhoneStr* pPhoneStr,
                                                    CConfRsrvRsrc* pConf,
                                                    const char* pSimilarToThisString)
{
	if ((pConf && pConf->GetFromPassive()) || m_netServiceName[0] == '\0')
		return STATUS_OK;

	STATUS status = STATUS_PHONE_NUMBER_OCCUPIED;
	if (pPhoneStr != NULL && pPhoneStr->m_netServiceName[0] != '\0' && !strcmp(pPhoneStr->GetNetServiceName(), m_netServiceName))
	{
		Phone* phone = pPhoneStr->GetFirstPhoneNumber();
		if (phone != NULL)
			return STATUS_OK;   // no need to allocate phone number

		char* number = NULL;
		std::set<CPhone>::iterator iter;
		for (iter = m_pPhoneList->begin(); iter != m_pPhoneList->end(); iter++)
		{
			if (iter->IsBusy() == TRUE)
				continue;
			else
			{
				char* phoneNum = (char*)(iter->GetNumber());
				if (phoneNum && (!pSimilarToThisString || iter->IsSimilarTo(pSimilarToThisString)))
				{
					WORD length = strlen(phoneNum);
					number = new char[length+1];
					strncpy(number, phoneNum, length);
					number[length] = '\0';

					status = STATUS_OK;
					break;
				}
			}
		}

		if (NULL != number)
		{
			pPhoneStr->AddPhoneNumber(number);
			if (pConf)
				pConf->AddServicePhoneNumber(m_netServiceName, number);

			delete [] number;
		}
	}
	return status;
}

//--------------------------------------------------------------------------
STATUS CReservationCalculator::PhoneCheckOrAlloc(CServicePhoneStr* pPhoneStr, ePhoneAllocationTypes phoneAllocType)
{
	STATUS ret_status = STATUS_OK;
	if (pPhoneStr != NULL && pPhoneStr->m_netServiceName[0] != '\0' &&
	    !strcmp(pPhoneStr->GetNetServiceName(), m_netServiceName))
	{
		BYTE isUseAsRange = pPhoneStr->IsUseServicePhonesAsRange();
		if (isUseAsRange)
		{
			Phone* phone1 = pPhoneStr->GetFirstPhoneNumber();
			Phone* phone2 = pPhoneStr->GetNextPhoneNumber();
			if (phone1)
			{
				ALLOCBUFFER(currPhoneNumber, PHONE_NUMBER_DIGITS_LEN);
				DWORD phone1Num = atoll(phone1->phone_number);
				ULONGLONG numPhones = phone2 ? (atoll(phone2->phone_number) - phone1Num) : 0;
				TRACEINTO << " Use Service Phones as range"
				          << ", phone1:"    << phone1->phone_number
				          << ", phone2:"    << (phone2 ? phone2->phone_number : "-")
				          << ", numPhones:" << numPhones;

				for (DWORD i = 0; i < numPhones+1; i++)
				{
					ULONGLONG currPhone = phone1Num + i;
//					memset(currPhoneNumber, 0, PHONE_NUMBER_DIGITS_LEN);
//					sprintf(currPhoneNumber, "%d", currPhone);

					ret_status = PhoneCheckOrAlloc(currPhone, phoneAllocType);
					if (ret_status != STATUS_OK)
						break;
				}

				DEALLOCBUFFER(currPhoneNumber);
			}

			return ret_status;
		}
		else
		{
			Phone* existPhone = pPhoneStr->GetFirstPhoneNumber();
			while (existPhone)   // do it for 2 phones
			{
				ret_status = PhoneCheckOrAlloc(existPhone->phone_number, phoneAllocType);
				if (ret_status != STATUS_OK)
					return ret_status;

				existPhone = pPhoneStr->GetNextPhoneNumber();
			}
		}
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CReservationCalculator::PhoneCheckOrAlloc(ULONGLONG currPhoneNumber, ePhoneAllocationTypes phoneAllocType)// const char* currPhoneNumber,
{
	std::map<ULONGLONG,std::string>::iterator itrMap = m_pPhonesMap.find(currPhoneNumber);
	if (itrMap != m_pPhonesMap.end())
	{
		return PhoneCheckOrAlloc(itrMap->second.c_str(), phoneAllocType);
	}
	TRACESTRFUNC(eLevelError) << "CReservationCalculator::PhoneCheckOrAlloc - Failed, Phone number not exist in service " << currPhoneNumber;
	return STATUS_PHONE_NUMBER_NOT_EXISTS;
}
STATUS CReservationCalculator::PhoneCheckOrAlloc(const char* currPhoneNumber, ePhoneAllocationTypes phoneAllocType)
{
	if (currPhoneNumber)
	{
		CPhone exist_phone(currPhoneNumber);
		std::set<CPhone>::iterator itr = m_pPhoneList->find(exist_phone);
		if (itr == m_pPhoneList->end())
		{
			TRACEINTO << "Failed, Phone number not exist in service " << exist_phone.GetNumber();
			return STATUS_PHONE_NUMBER_NOT_EXISTS;
		}

		if (itr->IsBusy() == TRUE && (ePhoneAlloc == phoneAllocType || ePhoneCheck == phoneAllocType))
		{
			TRACEINTO << "Failed, Phone already occupied " << exist_phone.GetNumber();
			return STATUS_PHONE_NUMBER_OCCUPIED;
		}

		if (ePhoneCheck != phoneAllocType)
			((CPhone*)(&(*itr)))->SetIsBusy(TRUE);
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CReservationCalculator::AddIpServiceCalc(DWORD ipServiceId, float service_factor, BOOL round_up)
{
	CSmallString sstr;
	sstr << "ipServiceId:" << ipServiceId << ", service_factor:" << service_factor << ", round_up:" << round_up;

	CIpServiceIntervals* pCIpServiceIntervals = new CIpServiceIntervals(ipServiceId, service_factor, round_up);
	pCIpServiceIntervals->SetLogicalResources(m_totalPartiesResources.m_logical_num_parties);
	pCIpServiceIntervals->MultiplyTotalPartiesResourcesByFactor();

	for (std::set<CIpServiceIntervals*>::iterator ser_itr = m_pIpServicesCalc->begin(); ser_itr != m_pIpServicesCalc->end(); ++ser_itr)
	{
		if ((*(*(ser_itr))) == (*(pCIpServiceIntervals)))
		{
			(*(ser_itr))->UpdateServiceFactor(service_factor, round_up);
			(*(ser_itr))->SetLogicalResources(m_totalPartiesResources.m_logical_num_parties);
			(*(ser_itr))->MultiplyTotalPartiesResourcesByFactor();

			TRACEINTO << sstr.GetString() << " - Updating the Service";
			POBJDELETE(pCIpServiceIntervals);
			return;
		}
	}
	m_pIpServicesCalc->insert(pCIpServiceIntervals);
	TRACEINTO << sstr.GetString() << " - Adding the Service";
}

//--------------------------------------------------------------------------
void CReservationCalculator::DumpTotals() const
{
	m_totalPartiesResources.DumpToTrace("CReservationCalculator::DumpTotals for System");

	for (std::set<CIpServiceIntervals*>::iterator _ii = m_pIpServicesCalc->begin(); _ii != m_pIpServicesCalc->end(); ++_ii)
		(*(_ii))->DumpTotals();
}

//--------------------------------------------------------------------------
CIpServiceIntervals* CReservationCalculator::GetIpServiceIntervalsCalc(DWORD ipServiceId)
{
	for (std::set<CIpServiceIntervals*>::iterator _ii = m_pIpServicesCalc->begin(); _ii != m_pIpServicesCalc->end(); ++_ii)
		if ((*(_ii))->GetServiceId() == ipServiceId)
			return (CIpServiceIntervals*)(*(_ii));

	return NULL;
}


////////////////////////////////////////////////////////////////////////////
//                        CIpServiceIntervals
////////////////////////////////////////////////////////////////////////////
CIpServiceIntervals::CIpServiceIntervals(DWORD ipServiceId, float service_factor, BOOL round_up) : m_ipServiceId(ipServiceId), m_service_factor(service_factor), m_round_up(round_up)
{
	m_pRsrcIntervals = new RSRC_INTERVAL_ARRAY;
}

//--------------------------------------------------------------------------
CIpServiceIntervals::~CIpServiceIntervals()
{
	Cleanup();
	POBJDELETE(m_pRsrcIntervals);
}

//--------------------------------------------------------------------------
void CIpServiceIntervals::Cleanup()
{
	for (RSRC_INTERVAL_ARRAY::iterator intrv = m_pRsrcIntervals->begin(); intrv != m_pRsrcIntervals->end(); ++intrv)
		POBJDELETE((*intrv).second);

	m_pRsrcIntervals->clear();
}

//--------------------------------------------------------------------------
bool operator==(const CIpServiceIntervals& ser1, const CIpServiceIntervals& ser2)
{
	return (ser1.m_ipServiceId == ser2.m_ipServiceId);
}

//--------------------------------------------------------------------------
bool operator<(const CIpServiceIntervals& ser1, const CIpServiceIntervals& ser2)
{
	return (ser1.m_ipServiceId < ser2.m_ipServiceId);
}

//--------------------------------------------------------------------------
CIpServiceIntervals::CIpServiceIntervals(const CIpServiceIntervals& other) : CPObject(other)
{
	m_ipServiceId    = other.m_ipServiceId;
	m_round_up       = other.m_round_up;
	m_service_factor = other.m_service_factor;

	m_pRsrcIntervals = new RSRC_INTERVAL_ARRAY;
	for (RSRC_INTERVAL_ARRAY::iterator _ii = other.m_pRsrcIntervals->begin(); _ii != other.m_pRsrcIntervals->end(); ++_ii)
	{
		Interval_Id IntervalId = (*_ii).first;
		CInterval*  pInterval  = new CInterval(*((*_ii).second));
		(*m_pRsrcIntervals)[IntervalId] = pInterval;
	}
}

//--------------------------------------------------------------------------
const CIpServiceIntervals& CIpServiceIntervals::operator=(const CIpServiceIntervals& other)
{
	if (*this == other)
		return *this;

	m_ipServiceId = other.m_ipServiceId;
	Cleanup();
	for (RSRC_INTERVAL_ARRAY::iterator _ii = other.m_pRsrcIntervals->begin(); _ii != other.m_pRsrcIntervals->end(); ++_ii)
	{
		Interval_Id IntervalId = ((*_ii).first);
		CInterval*  pInterval  = new CInterval(*((*_ii).second));
		(*m_pRsrcIntervals)[IntervalId] = pInterval;
	}

	return *this;
}

//--------------------------------------------------------------------------
STATUS CIpServiceIntervals::ReserveAmount(DWORD start, DWORD end, CIntervalRsrvAmount& neededAmount,
                                          BOOL isFromPassive, BOOL isNewRes, ePhoneAllocationTypes allocPhone)
{
	STATUS status = STATUS_OK;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	WORD max_num_of_conf = 0;
	if (pSystemResources->IsEventMode())
	{
		max_num_of_conf = pSystemResources->GetMaxNumOngoingConfPerCardsInEventMode();
		if (max_num_of_conf < 8)
		{
			// ICBC patch: because when loading new version we start the Reservations DB before receiving  CM_CARD_MNGR_LOADED_IND
			// we get max_num_of_conf = 0 , since ICBC uses RMX2000 with 2 MPM+ , and we need version today,  (yes, yes) we will set max_num_of_conf to 8
			max_num_of_conf = 8;
		}
	}
	else
		max_num_of_conf = pSystemResources->GetMaxNumberOfOngoingConferences();

	WORD max_num_vsw_conf = pSystemResources->GetMaxNumberVSWConferencesEventMode();

	for (DWORD i = start; i <= end; i++)
	{
		CInterval* pInterval = GetIntrvalById(i);

		if (!pInterval)  // create intervals for constant range in functions above???
		{
			pInterval = new CInterval(i, m_totalPartiesResources, max_num_of_conf, max_num_vsw_conf);
			(*m_pRsrcIntervals)[i] = pInterval;
		}

		status = pInterval->Reserve(neededAmount);

		// roll-back , all intervals behind have to exist.
		if (STATUS_OK != status)
		{
			if (i != start)
			{
				for (DWORD j = i-1; j >= start; j--)
				{
					pInterval = GetIntrvalById(j);
					if (pInterval)
						pInterval->UnReserve(neededAmount);

					if (j == 0) // don't go lower than 0, DWORD is unsigned and if we do j-- we will get a very big number
					{
						break;
					}
				}
			}
			break;
		}
	}
	return status;
}

//--------------------------------------------------------------------------
CInterval* CIpServiceIntervals::GetIntrvalById(Interval_Id intrvlID)
{
	RSRC_INTERVAL_ARRAY::iterator itr = m_pRsrcIntervals->find(intrvlID);

	return (itr != m_pRsrcIntervals->end()) ? ((*itr).second) : NULL;
}

//--------------------------------------------------------------------------
void CIpServiceIntervals::SetLogicalResources(float logical_num_parties[NUM_OF_PARTY_RESOURCE_TYPES])
{
	if (CHelperFuncs::IsMode2C())
		m_totalPartiesResources.m_logical_COP_num_parties = logical_num_parties[0];
	else
		memcpy(m_totalPartiesResources.m_logical_num_parties, logical_num_parties, sizeof(m_totalPartiesResources.m_logical_num_parties));
}

//--------------------------------------------------------------------------
void CIpServiceIntervals::UpdateServiceFactor(float service_factor, BOOL round_up)
{
	m_service_factor = service_factor;
	m_round_up       = round_up;
}

//--------------------------------------------------------------------------
void CIpServiceIntervals::DumpTotals() const
{
	CSmallString sstr;
	sstr << "CReservationCalculator::DumpTotals for Service, ipServiceId:" << m_ipServiceId << ", service_factor:" << m_service_factor << ", round_up:" << m_round_up;
	m_totalPartiesResources.DumpToTrace((char*)sstr.GetString());
}

//--------------------------------------------------------------------------
void CIpServiceIntervals::MultiplyTotalPartiesResourcesByFactor()
{
	if (CHelperFuncs::IsMode2C())
	{
		float total_parties      = m_totalPartiesResources.m_logical_COP_num_parties;
		float ip_service_parties = CHelperFuncs::CalcIpServicePart(total_parties, m_service_factor, m_round_up);
		m_totalPartiesResources.m_logical_COP_num_parties = ip_service_parties;
	}
	else
	{
		for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		{
			float total_parties      = m_totalPartiesResources.m_logical_num_parties[i];
			float ip_service_parties = CHelperFuncs::CalcIpServicePart(total_parties, m_service_factor, m_round_up);
			m_totalPartiesResources.m_logical_num_parties[i] = ip_service_parties;
		}
	}
}

