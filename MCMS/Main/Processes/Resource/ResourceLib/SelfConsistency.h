#ifndef SELFCONSISTENCY_H_
#define SELFCONSISTENCY_H_

#include "NStream.h"
#include <map>
#include "DataTypes.h"
#include "PObject.h"
#include "SystemResources.h"

class CConfRsrcDB;
class CConnToCardManager;
class CRsrcDesc;
class CConfRsrc;
class CResourceProcess;
class CReservator;
class CSelfConsistencyPartyRsrcDetails;
class CSelfConsistencyWhatToCheck;
class CConfRsrvRsrc;

typedef std::map<std::string, CSelfConsistencyPartyRsrcDetails*> PartyDetailsList;

////////////////////////////////////////////////////////////////////////////
//                        CSelfConsistency
////////////////////////////////////////////////////////////////////////////
class CSelfConsistency : public CPObject
{
	CLASS_TYPE_1(CSelfConsistency, CPObject)

public:
	static CSelfConsistency*          Instantiate();
	static void                       TearDown();

	static COstrStream&               GetOStream();

	virtual const char*               NameOf(void) const;

	STATUS                            CheckConsistency(CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck, std::ostream* pAdditionalStreamToPrintAnswerTo = NULL);
	STATUS                            CheckConsistency(std::ostream* pAdditionalStreamToPrintAnswerTo = NULL);

private:
	                                  CSelfConsistency();
	virtual                          ~CSelfConsistency();

	static CSelfConsistency*          m_pInstance;
	COstrStream                       m_OStream;

	CSystemResources*                 m_pSystemResources;
	CConfRsrcDB*                      m_pConfRsrcDB;
	CConnToCardManager*               m_pConnToCardManager;
	CResourceProcess*                 m_pProcess;
	CReservator*                      m_pReservator;

	STATUS                            CheckAudioVideoConfiguration();
	STATUS                            CheckNumberOfConferences();
	STATUS                            CheckAllResourcesDeletedIfNoMoreConferences();
	STATUS                            CheckNIDs();
	STATUS                            CheckPhones();
	STATUS                            CheckParties(CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck);
	STATUS                            CheckDescriptorInSharedMemory(CRsrcDesc* pRsrcdesc);
	STATUS                            CheckInternalPointers();
	STATUS                            CheckTiming();

	STATUS                            AddResourceDescriptorsToPartyList(PartyDetailsList* pPartyDetailsList, CConfRsrc* pConfRsrc, BOOL bCheckPartiesResourceDescriptorsCheckSharedMemory);
	STATUS                            AddUDPRsrcDescriptorsToPartyList(PartyDetailsList* pPartyDetailsList, CConfRsrc* pConfRsrc);
	STATUS                            AddActivePortsToPartyList(PartyDetailsList* pPartyDetailsList);

	STATUS                            CheckPartyDetailsListConsistency(PartyDetailsList* pPartyDetailsList, CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck);
	STATUS                            BuildPartyDetailsList(PartyDetailsList* pPartyDetailsList, CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck);
	void                              CleanUpPartyDetailsList(PartyDetailsList* pPartyDetailsList);

	CSelfConsistencyPartyRsrcDetails* GetPartyFromList(PartyDetailsList* pPartyDetailsList, DWORD rsrcConfId, DWORD rsrcPartyId);
	std::string                       GetPartyKey(DWORD rsrcConfId, DWORD rsrcPartyId);

	STATUS                            CheckPhoneNumberIsBusyInPhonesList(char* strPhone, std::set<CPhone>* pPhonesListInService);

	void                              PrintStatus(STATUS status);
	void                              ClearStream();

	void                              FinishCheckConsistency(std::ostream* pAdditionalStreamToPrintAnswerTo, STATUS finalStatus);
};

#endif /*SELFCONSISTENCY_H_*/
