#ifndef RESOURCESINTERFACEARRAY_H_
#define RESOURCESINTERFACEARRAY_H_

#include "InnerStructs.h"
#include "PObject.h"
#include "CardsStructs.h"
#include "SystemFunctions.h"
#include <set>

class CEnhancedConfig;

#define MAX_NUMBER_OF_RESOURCES_TYPE 2

////////////////////////////////////////////////////////////////////////////
//                        CResourcesInterfaceArray
////////////////////////////////////////////////////////////////////////////
class CResourcesInterfaceArray : public CPObject, ResourcesInterface
{
	CLASS_TYPE_1(CResourcesInterfaceArray, CPObject)

public:
	                    CResourcesInterfaceArray();
	                    CResourcesInterfaceArray(const CResourcesInterfaceArray& other);
	const CResourcesInterfaceArray& operator =(const CResourcesInterfaceArray& other);
	virtual            ~CResourcesInterfaceArray();
	const char*         NameOf() const {return "CResourcesInterfaceArray"; };

	virtual void        CalculateResourceReport(CRsrcReport* pReport);
	virtual void        CalculateConfResourceReport(CSharedRsrcConfReport* pReport);
	virtual BOOL        CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult,
	                                                  BYTE i_rmxPortGaugeThreshold = FALSE, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE);
	virtual BOOL        CheckIfOneMorePartyCanBeAddedCOP( ALLOC_PARTY_IND_PARAMS_S* pResult);
	virtual BOOL        CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix);
	virtual void        AddParty(BasePartyDataStruct& rPartyData);
	virtual void        RemoveParty(BasePartyDataStruct& rPartyData);
	virtual BOOL        IsThereAnyParty();
	virtual WORD        GetMaxNumberOfParties();
	virtual DWORD       GetOccupiedNumberOfParties();
	virtual WORD        GetNumberOfRequiredUDPPortsPerBoard(eIceEnvironmentType isIceEnv); //ICE 4 ports
	virtual STATUS      IsRsrcEnough(CBoardsStatistics* pBoardsStatistics);
	virtual void        InitDongleRestriction(DWORD& num_parties, BOOL bRecalculateReservationPartyResources = FALSE);
	virtual int         SetSvcDongleRestriction(DWORD num_parties);
	virtual void        FillRequiredPartyResourcesForConference(WORD minNumAudioPartiesInConf, WORD minNumVideoPartiesInConf, eVideoPartyType maxVideoPartyTypeInConf, CPartiesResources& partiesResourcesToFill);
	virtual void        UpdateReservatorWithLogicalConfiguration(BOOL bRecalculateReservationPartyResources = FALSE);
	virtual void        InitReservatorAndStaticCardConfigParams(BOOL bRecalculateReservationPartyResources = FALSE);
	virtual STATUS      ReconfigureUnitsAccordingToProportion(BOOL bForceReconfigure = FALSE);
	virtual STATUS      CanSetConfigurationNow();
	ResourcesInterface* NewCopy() const {return new CResourcesInterfaceArray(*this); }
	ResourcesInterface* GetCurrentResourcesInterface() const;
	void                InitResourceAllocationMode(eResourceAllocationTypes resourceAllocationType);
	STATUS              ChangeResourceAllocationMode(eResourceAllocationTypes resourceAllocationType);
	virtual STATUS      SetEnhancedConfiguration(CEnhancedConfig* pEnhancedCfg);
	virtual STATUS      CheckSetEnhancedConfiguration();
	virtual BOOL        CheckIfOneMorePartyCanBeAddedToIpService(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult,
	                                                             float service_factor, BOOL round_up, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE);
	virtual BOOL        CheckIfOnePartyCanBechangedInIpService(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, float service_factor, BOOL round_up, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix);
	virtual void        GetLogicalPartyWeight(eVideoPartyType videoPartyType, float& logicalPartyWeightInNonMix, float& logicalPartyWeightInMix);

	DWORD               GetDongleNumOfParties() const { return m_dongleNumParties; }
	void                SetDongleNumOfParties(DWORD dongleNumParties) { m_dongleNumParties = dongleNumParties; }

	STATUS              UpdatePortWeightsTo1500Q();
	void                UpdateLogicalWeightsAndValues(eSystemCardsMode cardsMode);

	BOOL                IsInterfaceArrayInitiated() const { return (m_CurrentResourcesInterfaceIndex == -1) ? FALSE : TRUE; }
	void                OnLicenseExpired();

protected:
	ResourcesInterface*      m_pResourcesInterfaceArray[MAX_NUMBER_OF_RESOURCES_TYPE];
	int                      m_CurrentResourcesInterfaceIndex;
	eResourceAllocationTypes m_ResourceAllocationType;
};


////////////////////////////////////////////////////////////////////////////
//                        CIpServiceResourcesInterfaceArray
////////////////////////////////////////////////////////////////////////////
class CIpServiceResourcesInterfaceArray : public CResourcesInterfaceArray
{
public:
	                    CIpServiceResourcesInterfaceArray(WORD m_serviceId, const char* m_serviceName, float service_factor = 1, BOOL round_up = FALSE);
	virtual            ~CIpServiceResourcesInterfaceArray();
	const char*         NameOf() const {return "CIpServiceResourcesInterfaceArray"; };

	CIpServiceResourcesInterfaceArray(const CIpServiceResourcesInterfaceArray& other);
	const CIpServiceResourcesInterfaceArray& operator =(const CIpServiceResourcesInterfaceArray& other);

	// std::set support
	friend bool         operator ==(const CIpServiceResourcesInterfaceArray& lhs, const CIpServiceResourcesInterfaceArray& rhs);
	friend bool         operator <(const CIpServiceResourcesInterfaceArray& lhs, const CIpServiceResourcesInterfaceArray& rhs);

	const char*         GetServiceName() const { return m_serviceName; }
	WORD                GetServiceId() const { return m_serviceId; }

	void                UpdateServiceWeight(float service_factor, BOOL round_up);

	float               GetServiceWeight() const { return m_service_factor; }
	BOOL                GetRoundup() const { return m_round_up; }

	//implementation of ResourcesInterface
	virtual void        CalculateResourceReportPerService(CRsrcReport* pReport);
	virtual BOOL        CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult,
	                                                  BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE);
	virtual BOOL        CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix);
	virtual WORD        GetMaxNumberOfParties();
	virtual void        UpdateReservatorWithLogicalConfiguration(BOOL bRecalculateReservationPartyResources = FALSE) { };
	virtual void        InitReservatorAndStaticCardConfigParams(BOOL bRecalculateReservationPartyResources = FALSE)  { };

	virtual STATUS      SetEnhancedConfiguration(CEnhancedConfig* pEnhancedCfg);
	virtual STATUS      CheckSetEnhancedConfiguration();

	STATUS              SetPortConfiguration(WORD index);

	void                AddParty(BasePartyDataStruct& rPartyData);    // use base class CResourcesInterfaceArray
	void                RemoveParty(BasePartyDataStruct& rPartyData); // use base class CResourcesInterfaceArray

	STATUS              AddPartyId(PartyRsrcID partyId);
	STATUS              RemovePartyId(PartyRsrcID partyId);
	BOOL                FindPartyId(PartyRsrcID partyId) const;

	friend std::ostream& operator<<(std::ostream& os, const CIpServiceResourcesInterfaceArray& val);

private:
	void                SetServiceName(const char* serviceName) { strcpy_safe(m_serviceName, serviceName); }

	WORD                m_serviceId;
	char                m_serviceName[NET_SERVICE_PROVIDER_NAME_LEN];
	float               m_service_factor;
	BOOL                m_round_up;

	typedef std::set<PartyRsrcID> PartyIdList;
	PartyIdList         m_partyIdList;
};

#endif /*RESOURCESINTERFACEARRAY_H_*/
