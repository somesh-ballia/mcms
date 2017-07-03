#ifndef CBASEMODERESOURCES_H_
#define CBASEMODERESOURCES_H_

#include "AllocateStructs.h"
#include "EnumsAndDefines.h"
#include "CardsStructs.h"
#include "InnerStructs.h"

////////////////////////////////////////////////////////////////////////////
//                        CBaseModeResources
////////////////////////////////////////////////////////////////////////////
class CBaseModeResources
{
public:
	                           CBaseModeResources(eSystemCardsMode cardsMode);
	                           CBaseModeResources(const CBaseModeResources& other);
	const CBaseModeResources&  operator=(const CBaseModeResources& other);
	virtual                   ~CBaseModeResources();

	void                       OnReconfigureUnitsTimer();
	void                       UpdatePortWeightsTo1500Q();
	void                       UpdateLogicalWeightsAndValues(eSystemCardsMode cardsMode);

private:
	void                       PrintDataAboutUnitReconfiguration(DWORD currentNumArtUnitsPerBoard[BOARDS_NUM], DWORD currentNumVideoUnitsPerBoard[BOARDS_NUM],
	                                                             DWORD numArtUnitsPerBoard[BOARDS_NUM], DWORD numVideoUnitsPerBoard[BOARDS_NUM], char* name);

protected:
	void                       PrintToTrace(float number[NUM_OF_PARTY_RESOURCE_TYPES], char* name);
	void                       PrintToTrace(WORD number[NUM_OF_PARTY_RESOURCE_TYPES], char* name);
	void                       PrintToTracePartyLogicalWeight(char* name);

	static ePartyResourceTypes VideoPartyTypeToPartyResourceType(eVideoPartyType videoPartyType);
	static eVideoPartyType     PartyResourceTypeToVideoPartyType(ePartyResourceTypes partyResourceType, eSystemCardsMode cardsMode);

	void                       PureModeReconfigureUnitsAccordingToProportion();
	void                       ReconfigureUnits(int numOfUnitsToReconfigure[BOARDS_NUM], eUnitType newUnitType[BOARDS_NUM]);

	STATUS                     BaseCanSetConfigurationNow();
	void                       BaseInitDongleRestriction(DWORD& num_parties);

	virtual BOOL               CheckVideoUnitsWithConfig(const DWORD numUnitsPerBoard[BOARDS_NUM]) const = 0;

	void                       UpdateInternalDataFromIsRsrcEnough(CBoardsStatistics* pBoardsStatistics);

	void                       InitBreezeWeightsAndValues();
	void                       InitMpmRxWeightsAndValues();
	void                       InitSoftMPMXWeightsAndValues();
	void                       InitSoftMFW_WeightsAndValues();
	void                       InitSoftNinjaWeightsAndValues();
	void                       InitPortWeightsTo1500Q();
	void                       InitSoftCGWeightsAndValues();

	void                       InitHighCapacityAndWeightsSoftMFW();
	void                       InitLowCapacityAndWeightsSoftMFW();
	void                       InitDemoCapacityAndWeightsSoftMFW();
	void                       InitWeightsSoftMFW(float num_parties);

	void                       UpdateMpmRxWeightsAndValues();
	void                       UpdateSoftNinjaWeightsAndValues();
	void                       UpdateSoftMPMXWeightsAndValues(bool isRPPMode);

	CBoardsStatistics          m_BoardsStatistics;

	float                      m_LogicalHD720WeightAvcParty[NUM_OF_CONF_MODE_TYPES][NUM_OF_PARTY_RESOURCE_TYPES];
	float                      m_logicalHD720WeightSvcParty[NUM_OF_CONF_MODE_TYPES][NUM_OF_PARTY_RESOURCE_TYPES];
	float                      m_logicalHD720WeightCopParty; // carmit-fix

	WORD                       m_numDonglePartiesSVC[NUM_OF_CONF_MODE_TYPES][NUM_OF_PARTY_RESOURCE_TYPES];

	BOOL                       m_BusyInReconfigureUnits;

	// data that comes from isRsrcEnoughData
	float                      m_totalPromillesAccordingToCards;
	float                      m_PromillesPerCard[BOARDS_NUM];

	float                      m_ART_PROMILLES;
	float                      m_VID_TOTAL_HD720_PROMILLES;
};

#endif /*CBASEMODERESOURCES_H_*/

