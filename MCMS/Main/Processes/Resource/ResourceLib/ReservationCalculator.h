#ifndef RESERVATIONCALCULATOR_H_
#define RESERVATIONCALCULATOR_H_

#include "PObject.h"
#include "RsrvResources.h"


#define Interval_Id DWORD
typedef std::map<Interval_Id, CInterval*> RSRC_INTERVAL_ARRAY;


////////////////////////////////////////////////////////////////////////////
//                        CIpServiceIntervals
////////////////////////////////////////////////////////////////////////////
class CIpServiceIntervals : public CPObject
{
	CLASS_TYPE_1(CIpServiceIntervals, CPObject)

public:
	                           CIpServiceIntervals(DWORD ipServiceId, float service_factor = 1, BOOL round_up  = 1);
	                           CIpServiceIntervals(const CIpServiceIntervals& other);
	virtual                   ~CIpServiceIntervals();

	const char*                NameOf() const { return "CIpServiceIntervals"; }
	void                       Cleanup();

	const CIpServiceIntervals& operator=(const CIpServiceIntervals& other);
	friend bool                operator==(const CIpServiceIntervals& ser1, const CIpServiceIntervals& ser2);
	friend bool                operator<(const CIpServiceIntervals& ser1, const CIpServiceIntervals& ser2);

	STATUS                     ReserveAmount(DWORD start, DWORD end, CIntervalRsrvAmount& neededAmount, BOOL isFromPassive = FALSE, BOOL isNewRes = FALSE, ePhoneAllocationTypes allocPhone = ePhoneAlloc);
	void                       UpdateServiceFactor(float m_service_factor, BOOL m_round_up);
	void                       SetLogicalResources(float logical_num_parties[NUM_OF_PARTY_RESOURCE_TYPES]);
	void                       DumpTotals() const;
	DWORD                      GetServiceId() const { return m_ipServiceId; }
	void                       MultiplyTotalPartiesResourcesByFactor();

protected:
	CInterval*                 GetIntrvalById(Interval_Id intrvlID);

	DWORD                      m_ipServiceId;
	CPartiesResources          m_totalPartiesResources;
	RSRC_INTERVAL_ARRAY*       m_pRsrcIntervals;
	float                      m_service_factor;
	BOOL                       m_round_up;
};


////////////////////////////////////////////////////////////////////////////
//                        CReservationCalculator
////////////////////////////////////////////////////////////////////////////
class CReservationCalculator : public CPObject
{
	CLASS_TYPE_1(CReservationCalculator, CPObject)

public:
	                           CReservationCalculator();
	virtual                   ~CReservationCalculator();

	const char*                NameOf() const { return "CReservationCalculator"; }

	void                       Cleanup();
	STATUS                     ReserveAmount(DWORD start, DWORD end, CIntervalRsrvAmount& neededAmount, BOOL isFromPassive = FALSE, BOOL isNewRes = FALSE, ePhoneAllocationTypes allocPhone = ePhoneAlloc, DWORD ipServiceId = ID_ALL_IP_SERVICES);
	STATUS                     ReserveAmountOnSystem(DWORD start, DWORD end, CIntervalRsrvAmount& neededAmount, BOOL isFromPassive, BOOL isNewRes, ePhoneAllocationTypes allocPhone);

	void                       SetLogicalResources(WORD logical_num_parties[NUM_OF_PARTY_RESOURCE_TYPES]);

	void                       SetNetServiceName(const char* name);
	void                       SetPhonesList(std::set<CPhone>& pPhoneList);
	STATUS                     ReservePhoneIfNeeded(CServicePhoneStr* pPhoneStr, CConfRsrvRsrc* pConf, const char* pSimilarToThisString = NULL);
	STATUS                     PhoneCheckOrAlloc(CServicePhoneStr* pPhoneStr, ePhoneAllocationTypes allocPhone = ePhoneAlloc);

	void                       AddIpServiceCalc(DWORD m_ipServiceId, float m_service_factor, BOOL m_round_up);
	CIpServiceIntervals*       GetIpServiceIntervalsCalc(DWORD ipServiceId);
	void                       DumpTotals() const;

private:
	STATUS                     PhoneCheckOrAlloc(const char* currPhoneNumber, ePhoneAllocationTypes phoneAllocType);
	STATUS                     PhoneCheckOrAlloc(ULONGLONG currPhoneNumber, ePhoneAllocationTypes phoneAllocType);
	CInterval*                 GetIntrvalById(Interval_Id intrvlID);

	CPartiesResources          m_totalPartiesResources;
	RSRC_INTERVAL_ARRAY*       m_pRsrcIntervals;
	std::set<CPhone>*          m_pNID_list;
	std::set<CPhone>*          m_pPhoneList; // dial-in phones.
	char                       m_netServiceName[NET_SERVICE_PROVIDER_NAME_LEN];
	std::set<CIpServiceIntervals*>* m_pIpServicesCalc;
	std::map<ULONGLONG,std::string> m_pPhonesMap;  //VNGFE-7414
};


#endif /*RESERVATIONCALCULATOR_H_*/
