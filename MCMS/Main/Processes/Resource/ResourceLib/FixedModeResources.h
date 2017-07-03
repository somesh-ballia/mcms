#ifndef _FIXEDMODERESOURCES_H_
#define _FIXEDMODERESOURCES_H_

#include "PObject.h"
#include "AllocateStructs.h"
#include "EnumsAndDefines.h"
#include "SystemResources.h"
#include "EnhancedConfig.h"
#include "BaseModeResources.h"

class CEnhancedConfigResponse;

////////////////////////////////////////////////////////////////////////////
//                        CFixedModeResources
////////////////////////////////////////////////////////////////////////////
class CFixedModeResources : public CPObject, public CBaseModeResources, public ResourcesInterface
{
	CLASS_TYPE_1(CFixedModeResources, CPObject)

public:
	                            CFixedModeResources(eSystemCardsMode cardMode = eSystemCardsMode_breeze);
	virtual                    ~CFixedModeResources();
	const char*                 NameOf() const { return "CFixedModeResources"; }

	// implementation of ResourcesInterface
	virtual void                CalculateResourceReport(CRsrcReport* pReport);
	virtual void                CalculateConfResourceReport(CSharedRsrcConfReport* pReport);
	virtual BOOL                CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult,
                                                              BYTE i_rmxPortGaugeThreshold = FALSE, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE);
	virtual BOOL                CheckIfOneMorePartyCanBeAddedCOP(ALLOC_PARTY_IND_PARAMS_S* pResult) { return FALSE; }
	virtual BOOL                CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix);
	virtual void                AddParty(BasePartyDataStruct& rPartyData);
	virtual void                RemoveParty(BasePartyDataStruct& rPartyData);
	virtual BOOL                IsThereAnyParty();
	virtual DWORD               GetOccupiedNumberOfParties();
	virtual WORD                GetMaxNumberOfParties();
	virtual WORD                GetNumberOfRequiredUDPPortsPerBoard(eIceEnvironmentType isIceEnv); // ICE 4 ports
	virtual STATUS              IsRsrcEnough(CBoardsStatistics* pBoardsStatistics);
	virtual void                InitDongleRestriction(DWORD& num_parties, BOOL bRecalculateReservationPartyResources = FALSE);
	virtual void                FillRequiredPartyResourcesForConference(WORD minNumAudioPartiesInConf, WORD minNumVideoPartiesInConf, eVideoPartyType maxVideoPartyTypeInConf, CPartiesResources& partiesResourcesToFill);
	virtual void                UpdateReservatorWithLogicalConfiguration(BOOL bRecalculateReservationPartyResources = FALSE);
	virtual void                InitReservatorAndStaticCardConfigParams(BOOL bRecalculateReservationPartyResources = FALSE);
	virtual STATUS              ReconfigureUnitsAccordingToProportion(BOOL bForceReconfigure = FALSE);
	virtual STATUS              CanSetConfigurationNow();
	virtual ResourcesInterface* NewCopy() const { return new CFixedModeResources(*this); }

	// additional things
	STATUS                      GetOrCheckEnhancedConfiguration(const CEnhancedConfig* pEnhancedCfg, CEnhancedConfigResponse* pResponse) const;
	STATUS                      SetEnhancedConfiguration(CEnhancedConfig* pEnhancedCfg, BOOL bIsIpService = FALSE, DWORD ip_service_id = (DWORD)-1);
	void                        FineTuneUnitsConfiguration();
	BOOL                        CheckEnhancedConfigurationWithCurUnitsConfig();
	STATUS                      CheckSetEnhancedConfiguration(BOOL bIsIpService = FALSE, DWORD ip_service_id = (DWORD)-1);

	void                        CheckIfNeedGaugeAlarm(class CResourceManager* pResourceManager, eVideoPartyType videoPartyType, DWORD portGauge, bool bAddParty);
	BOOL                        CheckIfOneMorePartyCanBeAddedToIpService(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult,
                                                                         float service_factor, BOOL round_up, BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE);
	BOOL                        CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult, WORD numHD720PortsAccordingToCards,
                                                              CEnhancedConfig& enhancedConfig, BYTE i_rmxPortGaugeThreshold = FALSE, eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE);
	BOOL                        CheckIfOnePartyCanBechangedInIpService(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, float service_factor, BOOL round_up,
                                                                       BOOL* pbAddAudioAsVideo = NULL, eConfModeTypes confModeType = eNonMix);
	BOOL                        CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, CEnhancedConfig& enhancedConfig);
	virtual void                OnLicenseExpired();
protected:
	virtual BOOL                CheckVideoUnitsWithConfig(const DWORD numUnitsPerBoard[BOARDS_NUM]) const;

private:
	float                       GetTotalOccupiedLogicalWeight(BOOL bAlsoAudio);
	float                       GetTotalOccupiedPromilles(BOOL bAlsoAudio);
	void                        CountConfResourceReport(CSharedRsrcConfReport* pReport, CConfRsrc* pConfRsrc);

	STATUS                      UpdateMaximumPartiesPerTypeAccordingToCards(CBoardsStatistics* pBoardsStatistics);

	BOOL                        CheckEnhancedConfigurationAccordingToDongle(const CEnhancedConfig* pEnhancedConfig) const;
	BOOL                        CheckEnhancedConfigurationAccordingToCardsPerLicense(const CEnhancedConfig* pEnhancedConfig) const;
	BOOL                        CheckEnhancedConfigurationAccordingToEnabledUnits(const CEnhancedConfig* pEnhancedConfig) const;
	BOOL                        CheckEnhancedConfigurationAccordingToDongleAndCards(const CEnhancedConfig* pEnhancedConfig) const;
	BOOL                        CheckVideoUnitsWithEnhancedConfig(const DWORD numUnitsPerBoard[BOARDS_NUM], const CEnhancedConfig* pEnhancedConfig = NULL, BOOL removeAudioControllerUnit = FALSE) const;

	void                        CalculateNeededAudioAndVideoPromilles(const CEnhancedConfig* pEnhancedConfig, float& totalAudioPromilles, float& totalVideoPromilles) const;

	void                        SetConfigurationAccordingToDongle();

	WORD                        CalculateMaximumPartiesPerType(const CEnhancedConfig* pEnhancedConfig, ePartyResourceTypes type) const;

private:
	WORD                        m_numOfCurrentPartiesPerType[NUM_OF_PARTY_RESOURCE_TYPES];
	// first index: actual party type, second index "glided" party type: so always first index < second index
	WORD                        m_numOfCurrentGlidesPerType[NUM_OF_PARTY_RESOURCE_TYPES][NUM_OF_PARTY_RESOURCE_TYPES];

	// weights
	float                       m_TotalPromillesOfParty[NUM_OF_PARTY_RESOURCE_TYPES];
	float                       m_VideoPromillesOfParty[NUM_OF_PARTY_RESOURCE_TYPES];

	WORD                        m_MaximumPartiesAccordingToDongle[NUM_OF_PARTY_RESOURCE_TYPES];
	WORD                        m_MaximumPartiesAccordingToCards[NUM_OF_PARTY_RESOURCE_TYPES];

	CEnhancedConfig             m_enhancedConfig;

	friend class CSelfConsistency;
};

#endif /*_FIXEDMODERESOURCES_H_*/

