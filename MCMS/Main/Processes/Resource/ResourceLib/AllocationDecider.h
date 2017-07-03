#ifndef ALLOCATIONDECIDER_H_
#define ALLOCATIONDECIDER_H_

#include "DataTypes.h"
#include "InnerStructs.h"
#include "PObject.h"

class CNetServicesDB;
class CSpanRTM;

////////////////////////////////////////////////////////////////////////////
//                        CAllocationDecider
////////////////////////////////////////////////////////////////////////////
class CAllocationDecider: public CPObject
{
CLASS_TYPE_1(CAllocationDecider,CPObject)

public:
	CAllocationDecider(CNetServicesDB* pNetServicesDB);
	~CAllocationDecider() {}
	const char * NameOf() const { return "CAllocationDecider"; }

	STATUS DecideAboutBestBoards(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	STATUS DecideAboutBestBoardsCOP(BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	STATUS DecideDiffBoardsForVSW56Session(BestAllocStruct& bestAllocStruct1, BestAllocStruct& bestAllocStruct2, DataPerBoardStruct* pAllocDataPerBoardArray);
	STATUS DecideAboutBestRTMBoards(PartyDataStruct& partyData, ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, DataPerBoardStruct* pAllocDataPerBoardArray, int notOnThisBoard, int preferablyOnThisBoard);
	STATUS DecideAboutBestARTBoardCOP(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray, int videoBoardId);

private:
	STATUS DecideAboutBestBoardsForVoipParty(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	STATUS DecideAboutBestBoardsForIPVideoParty(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	STATUS DecideAboutBestBoardsForISDNVideoPartyDialOut(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	STATUS DecideAboutBestBoardsForISDNVideoPartyDialIn(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	STATUS DecideAboutBestBoardsForPSTNPartyDialOut(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	STATUS DecideAboutBestBoardsForPSTNPartyDialIn(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	STATUS DecideAboutBestBoardsForCOP(BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);

	void   FillBestVideoDataAccordingToBestVideoBoard(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	void   FillBestVideoDataAccordingToBestVideoBoardCOP(BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	void   FillBestAudioDataAccordingToBestAudioBoard(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray);
	void   FillBestRTMDataAccordingToBestRTMBoards(PartyDataStruct& partyData, ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, DataPerBoardStruct* pAllocDataPerBoardArray);
	void   FillVideoDataForVSW56Session(BestAllocStruct& bestAllocStruct1, BestAllocStruct& bestAllocStruct2, DataPerBoardStruct* pAllocDataPerBoardArray);BOOL BoardMeetsRequirements(PartyDataStruct& partyData, DataPerBoardStruct& boardData, AllocRequirements& allocRequirements, bool fullVideoReq = true);BOOL BoardMeetsRequirementsCOP(DataPerBoardStruct& boardData, AllocRequirements& allocRequirements);BOOL CheckVideoAvailabilityOptimalUnits(PartyDataStruct& partyData, DataPerBoardStruct& boardData);BOOL CheckOptimalVideoAvailability(PartyDataStruct& partyData, DataPerBoardStruct& boardData); //VNGR-15461

	int    GetCompareData(eBoardComparer boardComparer, DataPerBoardStruct& boardData);
	int    GetBestBoardThatMeetsRequirements(eBoardComparer boardComparer, PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray, AllocRequirements& allocRequirements, int notOnThisBoard = 0);
	int    GetBestBoardThatMeetsRequirementsCOP(eBoardComparer boardComparer, DataPerBoardStruct* pAllocDataPerBoardArray, AllocRequirements& allocRequirements, int notOnThisBoard = 0);
	int    GetBestBoardThatMeetsRequirements(eBoardComparer boardComparers[MAX_NUMBER_OF_BOARD_COMPARERS], PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray, AllocRequirements& allocRequirements, int notOnThisBoard = 0);
	int    GetBestVideoBoardNoOtherRequirements(BOOL bOptimalVideo, PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray);
	int    GetBestVideoBoardNoOtherRequirementsCOP(DataPerBoardStruct* pAllocDataPerBoardArray);
	int    GetBestARTBoardNoOtherRequirements(PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray);BOOL GetBestRTMBoardsNoOtherRequirements_And_SetInResponseStruct(PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray, ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, int notOnThisBoard = 0);

	void   DumpData(PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray);

	void   PutFirstSpanLast(CSpanRTM* best_spans_per_board[MAX_NUM_SPANS_ORDER]);
	void   SetRTMBoardIdForChannels(ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, int boardId, int startChannelIndex, int numChannelToSet);
	void   SetRTMBoardForAllChannels(PartyDataStruct& partyData, ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, int boardId);

	CNetServicesDB* m_pNetServicesDB;
};

#endif /*ALLOCATIONDECIDER_H_*/
