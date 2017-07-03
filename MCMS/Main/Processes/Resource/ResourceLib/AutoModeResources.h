#ifndef AUTOMODERESOURCES_H_
#define AUTOMODERESOURCES_H_

#include "PObject.h"
#include "BaseModeResources.h"
#include "SystemResources.h"

////////////////////////////////////////////////////////////////////////////
//                        CAutoModeResources
////////////////////////////////////////////////////////////////////////////
class CAutoModeResources : public CPObject, public ResourcesInterface, public CBaseModeResources
{
	CLASS_TYPE_1(CAutoModeResources, CPObject)

public:
	                  CAutoModeResources(eSystemCardsMode cardMode = eSystemCardsMode_mpmrx);
	                  CAutoModeResources(const CAutoModeResources& other);
	virtual          ~CAutoModeResources();
	const char*       NameOf() const { return "CAutoModeResources"; }

	const CAutoModeResources& operator =(const CAutoModeResources& other);

	// Implementation of ResourcesInterface
	ResourcesInterface*
	                  NewCopy() const { return new CAutoModeResources(*this); }

	void              CalculateResourceReport(CRsrcReport* pReport);
	void              CalculateConfResourceReport(CSharedRsrcConfReport* pReport);
	BOOL              CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult,
	                                                BYTE i_rmxPortGaugeThreshold = FALSE, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE);
	BOOL              CheckIfOneMorePartyCanBeAddedCOP(ALLOC_PARTY_IND_PARAMS_S* pResult);
	BOOL              CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix);
	void              AddParty(BasePartyDataStruct& rPartyData);
	void              RemoveParty(BasePartyDataStruct& rPartyData);
	BOOL              IsThereAnyParty();
	WORD              GetMaxNumberOfParties();
	DWORD             GetOccupiedNumberOfParties();
	WORD              GetNumberOfRequiredUDPPortsPerBoard(eIceEnvironmentType isIceEnv); //ICE 4 ports
	STATUS            IsRsrcEnough(CBoardsStatistics* pBoardsStatistics);
	void              InitDongleRestriction(DWORD& num_parties, BOOL bRecalculateReservationPartyResources = FALSE);
	int               SetSvcDongleRestriction(DWORD num_parties);
	void              FillRequiredPartyResourcesForConference(WORD minNumAudioPartiesInConf, WORD minNumVideoPartiesInConf, eVideoPartyType maxVideoPartyTypeInConf, CPartiesResources& partiesResourcesToFill);
	void              UpdateReservatorWithLogicalConfiguration(BOOL bRecalculateReservationPartyResources = FALSE);
	void              InitReservatorAndStaticCardConfigParams(BOOL bRecalculateReservationPartyResources = FALSE);
	STATUS            ReconfigureUnitsAccordingToProportion(BOOL bForceReconfigure = FALSE);
	STATUS            CanSetConfigurationNow();

	// multiple service
	BOOL              CheckIfOneMorePartyCanBeAddedToIpService(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult, float service_factor, BOOL round_up,
	                                                           BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE);
	BOOL              CheckIfOnePartyCanBechangedInIpService(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, float service_factor, BOOL round_up,
	                                                         BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix);
	float             CalculateAdditionalLicenseForMixMode(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, bool isNewParty);
	float             GetTotalAllocatedVideoParties();
	void              OnLicenseExpired();
	// End of Implementation of ResourcesInterface


	// Additional functions
	STATUS            UserChangedPortConfiguration();
	void              UpdateAudioVideoConfigProportion();
	STATUS            ResetPortConfiguration();
	CPortsConfig*     GetPortsConfig() const { return m_pPortsConfig; }
	void              CheckIfNeedGaugeAlarm(class CResourceManager * pResourceManager, eVideoPartyType videoPartyType, DWORD portGauge, bool bAddParty);
	BOOL              CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult, WORD numHD720PortsAccordingToCards,
	                                                WORD numAudPartiesInConfig, WORD numVidPartiesInConfig, BYTE i_rmxPortGaugeThreshold = FALSE, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE);
	BOOL              CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, WORD numHD720PortsAccordingToCards, WORD numAudPartiesInConfig, WORD numVidPartiesInConfig, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix);

	float             GetTotalAllocatedAudioParties(BOOL isSeperatePools = FALSE);
	DWORD             GetTotalAudioPartiesAllocatedAsVideo();

	void              DumpTotalAllocatedParties();

private:
	void              CountReportPorts(eRPRTtypes rprt_type, WORD hd720_ports_according_to_cards, WORD& numVidHD720Parties, WORD& numVidCifParties, WORD& numAudParties, DWORD& ppm);
	void              CountConfResourceReport(CSharedRsrcConfReport* pReport, CConfRsrc* pConfRsrc);
	void              AddPartyToMatrix(eVideoPartyType videoPartyType, ePartyRole partyRole, eConfModeTypes confModeType, WORD reqBoardId);
	void              RemovePartyFromMatrix(eVideoPartyType videoPartyType, ePartyRole partyRole, eConfModeTypes confModeType, WORD reqBoardId);

protected:
	virtual BOOL      CheckVideoUnitsWithConfig(const DWORD numUnitsPerBoard[BOARDS_NUM]) const;
	float             GetLogicalWeightForResourceCalculations(eVideoPartyType videoPartyType, ePartyRole partyRole, eConfModeTypes confModeType);
	float             GetLicenseFactorAccordingToPartyRole(eVideoPartyType videoPartyType, ePartyRole partyRole);
	virtual void      GetLogicalPartyWeight(eVideoPartyType videoPartyType, float& , float&);

	float             m_numCurrentAVCParties[BOARDS_NUM][NUM_OF_CONF_MODE_TYPES][NUM_OF_PARTY_RESOURCE_TYPES];
	float             m_numCurrentSVCParties[BOARDS_NUM][NUM_OF_CONF_MODE_TYPES][NUM_OF_PARTY_RESOURCE_TYPES];

	DWORD             m_numAudioPartiesAllocatedAsVideo[BOARDS_NUM];
	CAudioVideoConfig m_audVidConfig;
	CPortsConfig*     m_pPortsConfig;

	friend class CSelfConsistency;
};

#endif /*AUTOMODERESOURCES_H_*/
