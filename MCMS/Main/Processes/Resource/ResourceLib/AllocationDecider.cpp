#include "TraceStream.h"
#include "AllocationDecider.h"
#include "SystemResources.h"
#include "HelperFuncs.h"
#include "Trace.h"
#include "InternalProcessStatuses.h"
#include "NetServicesDB.h"
#include "ObjString.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"

eBoardComparer VIDEO_FREE_THEN_NUM_PARTICIPANTS_COMPARER[MAX_NUMBER_OF_BOARD_COMPARERS] = { eBoardComparer_Video_Free, eBoardComparer_Video_Parties, eBoardComparer_None, eBoardComparer_None };
eBoardComparer NUM_PARTICIPANTS_THEN_VIDEO_FREE_THEN_FREE_PORTS_COMPARER[MAX_NUMBER_OF_BOARD_COMPARERS] = { eBoardComparer_Video_Parties, eBoardComparer_Video_Free, eBoardComparer_Video_Free_Ports, eBoardComparer_Art_Free_Ports };

////////////////////////////////////////////////////////////////////////////
//                        CAllocationDecider
////////////////////////////////////////////////////////////////////////////
CAllocationDecider::CAllocationDecider(CNetServicesDB* pNetServicesDB)
{
	m_pNetServicesDB = pNetServicesDB;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestBoards(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	TRACEINTO;

	STATUS status;
	if (CHelperFuncs::IsIPParty(partyData.m_networkPartyType))
	{
		if (CHelperFuncs::IsAudioParty(partyData.m_videoPartyType) || CHelperFuncs::IsAudioContentParty(partyData.m_videoPartyType))
			status = DecideAboutBestBoardsForVoipParty(partyData, bestAllocStruct, pAllocDataPerBoardArray);
		else
			status = DecideAboutBestBoardsForIPVideoParty(partyData, bestAllocStruct, pAllocDataPerBoardArray);
	}
	else //isdn
	{
		if (bestAllocStruct.m_pIsdn_Params_Response == NULL)
			bestAllocStruct.m_pIsdn_Params_Response = new ISDN_PARTY_IND_PARAMS_S;

		memset(bestAllocStruct.m_pIsdn_Params_Response, 0, sizeof(ISDN_PARTY_IND_PARAMS_S));

		if (CHelperFuncs::IsDialOutParty(partyData.m_pIsdn_Params_Request))
		{
			if (CHelperFuncs::IsAudioParty(partyData.m_videoPartyType))
				status = DecideAboutBestBoardsForPSTNPartyDialOut(partyData, bestAllocStruct, pAllocDataPerBoardArray);
			else
				status = DecideAboutBestBoardsForISDNVideoPartyDialOut(partyData, bestAllocStruct, pAllocDataPerBoardArray);

			if (status == STATUS_OK)
				FillBestRTMDataAccordingToBestRTMBoards(partyData, bestAllocStruct.m_pIsdn_Params_Response, pAllocDataPerBoardArray);
		}
		else
		{
			if (CHelperFuncs::IsAudioParty(partyData.m_videoPartyType))
				status = DecideAboutBestBoardsForPSTNPartyDialIn(partyData, bestAllocStruct, pAllocDataPerBoardArray);
			else
				status = DecideAboutBestBoardsForISDNVideoPartyDialIn(partyData, bestAllocStruct, pAllocDataPerBoardArray);

			if (status == STATUS_OK)
			{
				//in dial-in we are allocating the first channel, according to what we received, but not the others
				bestAllocStruct.m_pIsdn_Params_Response->spans_order.port_spans_list[0].board_id = partyData.m_pIsdn_Params_Request->board_id;
				bestAllocStruct.m_pIsdn_Params_Response->spans_order.port_spans_list[0].spans_list[0] = partyData.m_pIsdn_Params_Request->span_id;
			}
		}
	}

	if (status == STATUS_OK)
	{
		FillBestAudioDataAccordingToBestAudioBoard(partyData, bestAllocStruct, pAllocDataPerBoardArray);
		if (CHelperFuncs::IsVideoParty(partyData.m_videoPartyType) && (!CHelperFuncs::IsAudioContentParty(partyData.m_videoPartyType)))
			FillBestVideoDataAccordingToBestVideoBoard(partyData, bestAllocStruct, pAllocDataPerBoardArray);
	}

	if (status != STATUS_OK)
		DumpData(partyData, pAllocDataPerBoardArray);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestARTBoardCOP(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray, int videoBoardId)
{
	STATUS status = STATUS_FAIL;
	if (CHelperFuncs::IsIPParty(partyData.m_networkPartyType) && CHelperFuncs::IsVideo2CParty(partyData.m_videoPartyType))
	{
		/* ---------------------------------------------------------------------------------------------------
		 * Allocation priorities for COP: * Choose art on card when the conf video are
		 * --------------------------------------------------------------------------------------------------- */
		int zeroBasedBoardId = (videoBoardId > 0) ? (videoBoardId - 1) : videoBoardId;

		if (zeroBasedBoardId >= 0 && pAllocDataPerBoardArray[zeroBasedBoardId].m_ARTData.m_bCanBeAllocated && pAllocDataPerBoardArray[zeroBasedBoardId].m_ARTData.m_numFreeArtPorts > 0) //Olga - check with Sergey!!!
		{
			bestAllocStruct.m_ArtBoardId = videoBoardId;
			status = STATUS_OK;
		}
		else
		{
			int bestBoardId = GetBestARTBoardNoOtherRequirements(partyData, pAllocDataPerBoardArray);
			if (bestBoardId == 0)
			{
				PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestARTBoardCOP - no ART found");
				status = STATUS_INSUFFICIENT_ART_RSRC;
			}
			else
			{
				bestAllocStruct.m_ArtBoardId = bestBoardId;
				status = STATUS_OK;
			}
		}
	}
	else
	{
		TRACEINTO << " DecideAboutBestARTBoardCOP : illegal party type!!! ";
	}

	if (status == STATUS_OK)
	{
		FillBestAudioDataAccordingToBestAudioBoard(partyData, bestAllocStruct, pAllocDataPerBoardArray);
	}

	if (status != STATUS_OK)
		DumpData(partyData, pAllocDataPerBoardArray);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestBoardsCOP(BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	STATUS status;

	status = DecideAboutBestBoardsForCOP(bestAllocStruct, pAllocDataPerBoardArray);

	if (status == STATUS_OK)
	{
		FillBestVideoDataAccordingToBestVideoBoardCOP(bestAllocStruct, pAllocDataPerBoardArray);
	}

	if (status != STATUS_OK)
		PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestBoardsCOP - no sufficient Video resources");
	//DumpData(partyData, pAllocDataPerBoardArray);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideDiffBoardsForVSW56Session(BestAllocStruct& bestAllocStruct1, BestAllocStruct& bestAllocStruct2, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	// The function is called only in case when the session type is eVSW_56_session
	// and we know that allocation should be done on different boards

	STATUS status = STATUS_FAIL;
	AllocRequirements allocRequirements;
	allocRequirements.m_bCheckVideoAvailability = TRUE;
	WORD board_id = 0;

	if (BoardMeetsRequirementsCOP(pAllocDataPerBoardArray[board_id], allocRequirements) == TRUE)
	{
		if (BoardMeetsRequirementsCOP(pAllocDataPerBoardArray[board_id + 1], allocRequirements) == TRUE)
		{
			status = STATUS_OK;
			//TODO: to optimize in case of AMOS
		}
	}
	if (status == STATUS_OK)
	{
		FillVideoDataForVSW56Session(bestAllocStruct1, bestAllocStruct2, pAllocDataPerBoardArray);
	}

	if (status != STATUS_OK)
		PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideDiffBoardsForVSW56Session - no sufficient Video resources");

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestBoardsForPSTNPartyDialOut(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	/* --------------------------------------------------------------------------------------------------
	 * Allocation priorities for PSTN - dial-out:
	 * *******************************************
	 * Preferably RTM and ART should be on the same card
	 * If both cards have RTM and ART free, take card with the most "surplus" ART free, and allocate RTM there too
	 * --------------------------------------------------------------------------------------------------- */

	AllocRequirements allocRequirements;
	int bestBoardId = 0; //1-based!!!

	//Step 1: RTM and ART together
	allocRequirements.AllFalse();
	allocRequirements.m_bCheckARTAvailability = TRUE;
	allocRequirements.m_bCheckRTMFullAvailability = TRUE;
	bestBoardId = GetBestBoardThatMeetsRequirements(eBoardComparer_Rtm_Free_Ports, partyData, pAllocDataPerBoardArray, allocRequirements);

	if (bestBoardId != 0)
	{
		bestAllocStruct.m_ArtBoardId = bestBoardId;
		SetRTMBoardForAllChannels(partyData, bestAllocStruct.m_pIsdn_Params_Response, bestBoardId);
		return STATUS_OK;
	}

	//Step 2: RTM and ART separate
	BOOL bRTMSuccess = GetBestRTMBoardsNoOtherRequirements_And_SetInResponseStruct(partyData, pAllocDataPerBoardArray, bestAllocStruct.m_pIsdn_Params_Response);
	if (bRTMSuccess == FALSE)
	{
		PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestBoardsForPSTNPartyDialOut - no sufficient RTM found");
		return STATUS_INSUFFICIENT_RTM_RSRC;
	}

	bestBoardId = GetBestARTBoardNoOtherRequirements(partyData, pAllocDataPerBoardArray);
	if (bestBoardId == 0) //no ART found at all
	{
		PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestBoardsForPSTNPartyDialOut - no ART found");
		return STATUS_INSUFFICIENT_ART_RSRC;
	}
	bestAllocStruct.m_ArtBoardId = bestBoardId;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestBoardsForPSTNPartyDialIn(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	/* --------------------------------------------------------------------------------------------------
	 * Allocation priorities for PSTN - dial-in:
	 * *******************************************
	 * Try to take ART on same card as RTM "in" channel
	 * --------------------------------------------------------------------------------------------------- */

	int zeroBasedBoardIdRTM = partyData.m_pIsdn_Params_Request->board_id - 1;

	//in case it's dial-in the best option is the same as the RTM, check if it's OK, if there are some surplus ART units in it
	if (pAllocDataPerBoardArray[zeroBasedBoardIdRTM].m_ARTData.m_bCanBeAllocated && pAllocDataPerBoardArray[zeroBasedBoardIdRTM].m_ARTData.m_numFreeArtPorts > 0)
	{
		bestAllocStruct.m_ArtBoardId = partyData.m_pIsdn_Params_Request->board_id;
	}
	else //if there is no ART surplus left on that card, check on the other cards, and take best one
	{
		int bestBoardId = GetBestARTBoardNoOtherRequirements(partyData, pAllocDataPerBoardArray);

		if (bestBoardId == 0)
		{
			PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestBoardsForPSTNParty dial-in - no ART found");
			return STATUS_INSUFFICIENT_ART_RSRC;
		}

		bestAllocStruct.m_ArtBoardId = bestBoardId;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestBoardsForISDNVideoPartyDialOut(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	/* --------------------------------------------------------------------------------------------------
	 * Allocation priorities for ISDN - dial-out:
	 * *******************************************
	 * Preferably no video downgrade
	 * Preferably Video, RTM and ART should be on the same card
	 * If both cards have Video, RTM and ART free, choose the one with the most video free
	 * If there's no card with the three free:
	 * If possible: put RTM and ART together and Video on the other card
	 * --------------------------------------------------------------------------------------------------- */

	AllocRequirements allocRequirements;
	int bestBoardId = 0; //1-based!!!

	//Step 1: RTM, video and ART together, optimal video
	allocRequirements.AllFalse();
	allocRequirements.m_bCheckARTAvailability = TRUE;
	allocRequirements.m_bCheckRTMFullAvailability = TRUE;
	allocRequirements.m_bCheckVideoAvailability = TRUE;
	allocRequirements.m_bCheckVideoAvailabilityOptimalUnits = TRUE;

	bestBoardId = GetBestBoardThatMeetsRequirements(NUM_PARTICIPANTS_THEN_VIDEO_FREE_THEN_FREE_PORTS_COMPARER, partyData, pAllocDataPerBoardArray, allocRequirements);

	if (bestBoardId != 0)
	{
		bestAllocStruct.m_ArtBoardId = bestBoardId;
		bestAllocStruct.m_VideoAlloc.m_boardId = bestBoardId;
		SetRTMBoardForAllChannels(partyData, bestAllocStruct.m_pIsdn_Params_Response, bestBoardId);
		return STATUS_OK;
	}

	//from this point on, video will be disconnected from ART and RTM decision, so find the best board for video
	//first try to find one with optimal video
	bestBoardId = GetBestVideoBoardNoOtherRequirements(TRUE, partyData, pAllocDataPerBoardArray);
	if (bestBoardId == 0)
	{
		//if not, find one with video anyway
		bestBoardId = GetBestVideoBoardNoOtherRequirements(FALSE, partyData, pAllocDataPerBoardArray);
	}

	if (bestBoardId == 0)
	{	//we didn't find anything where video can be allocated!
		PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestBoardsForISDNVideoPartyDialOut - no Video found");
		return STATUS_INSUFFICIENT_VIDEO_RSRC;
	}
	bestAllocStruct.m_VideoAlloc.m_boardId = bestBoardId;

	//at this point we already have a video board decision, so now just find where it is best to put ART and RTM

	//Step 2: RTM and ART together
	allocRequirements.AllFalse();
	allocRequirements.m_bCheckARTAvailability = TRUE;
	allocRequirements.m_bCheckRTMFullAvailability = TRUE;
	bestBoardId = GetBestBoardThatMeetsRequirements(eBoardComparer_Rtm_Free_Ports, partyData, pAllocDataPerBoardArray, allocRequirements);

	if (bestBoardId != 0)
	{
		bestAllocStruct.m_ArtBoardId = bestBoardId;
		SetRTMBoardForAllChannels(partyData, bestAllocStruct.m_pIsdn_Params_Response, bestBoardId);
		return STATUS_OK;
	}

	//Step 3: RTM and ART separate
	BOOL bRTMSuccess = GetBestRTMBoardsNoOtherRequirements_And_SetInResponseStruct(partyData, pAllocDataPerBoardArray, bestAllocStruct.m_pIsdn_Params_Response);
	if (bRTMSuccess == FALSE)
	{
		PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestBoardsForISDNVideoPartyDialOut - no sufficient RTM found");
		return STATUS_INSUFFICIENT_RTM_RSRC;
	}

	bestBoardId = GetBestARTBoardNoOtherRequirements(partyData, pAllocDataPerBoardArray);
	if (bestBoardId == 0) //no ART found at all
	{
		PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestBoardsForISDNVideoPartyDialOut - no ART found");
		return STATUS_INSUFFICIENT_ART_RSRC;
	}
	bestAllocStruct.m_ArtBoardId = bestBoardId;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestBoardsForISDNVideoPartyDialIn(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	/* --------------------------------------------------------------------------------------------------
	 * Allocation priorities for ISDN - dial-in:
	 * *******************************************
	 * Preferably no video downgrade
	 * Try to take Video, ART on same card as RTM "in" channel
	 * If not put them on another card
	 * --------------------------------------------------------------------------------------------------- */

	int zeroBasedBoardIdRTM = partyData.m_pIsdn_Params_Request->board_id - 1;
	int bestBoardId;

	//in case it's dial-in the best option is the same as the RTM, check if it's OK, if there are some ART and enough video units in it (with optimal units)
	if (pAllocDataPerBoardArray[zeroBasedBoardIdRTM].m_ARTData.m_bCanBeAllocated && pAllocDataPerBoardArray[zeroBasedBoardIdRTM].m_ARTData.m_numFreeArtPorts > 0 && pAllocDataPerBoardArray[zeroBasedBoardIdRTM].m_VideoData.m_bCanBeAllocated && pAllocDataPerBoardArray[zeroBasedBoardIdRTM].m_VideoData.m_numFreeVideoCapacity > 0 && CheckVideoAvailabilityOptimalUnits(partyData, pAllocDataPerBoardArray[zeroBasedBoardIdRTM]))
	{
		bestAllocStruct.m_ArtBoardId = partyData.m_pIsdn_Params_Request->board_id;
		bestAllocStruct.m_VideoAlloc.m_boardId = partyData.m_pIsdn_Params_Request->board_id;
		return STATUS_OK;
	}
	else if (pAllocDataPerBoardArray[zeroBasedBoardIdRTM].m_ARTData.m_bCanBeAllocated && pAllocDataPerBoardArray[zeroBasedBoardIdRTM].m_ARTData.m_numFreeArtPorts > 0)
	{ //if at least ART can be allocated there, this is good too. For Video, take the best card
		bestAllocStruct.m_ArtBoardId = partyData.m_pIsdn_Params_Request->board_id;
	}
	else //if there is no ART left on that card, check the best from other cards
	{
		bestBoardId = GetBestARTBoardNoOtherRequirements(partyData, pAllocDataPerBoardArray);
		if (bestBoardId == 0)
		{
			PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestBoardsForISDNVideoPartyDialIn - no ART found");
			return STATUS_INSUFFICIENT_ART_RSRC;
		}
		bestAllocStruct.m_ArtBoardId = bestBoardId;
	}

	//at this point we already have ART allocated, now just find the best video board

	//first try to find one with optimal video
	bestBoardId = GetBestVideoBoardNoOtherRequirements(TRUE, partyData, pAllocDataPerBoardArray);
	if (bestBoardId == 0)
	{
		//if not, find one with video anyway
		bestBoardId = GetBestVideoBoardNoOtherRequirements(FALSE, partyData, pAllocDataPerBoardArray);
	}

	if (bestBoardId == 0)
	{ //we didn't find anything where video can be allocated!
		PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestBoardsForISDNVideoPartyDialIn - no Video found");
		return STATUS_INSUFFICIENT_VIDEO_RSRC;
	}
	bestAllocStruct.m_VideoAlloc.m_boardId = bestBoardId;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestBoardsForVoipParty(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	/* --------------------------------------------------------------------------------------------------
	 * Allocation priorities for VOIP:
	 * *******************************
	 * Choose art on card with most "surplus" ART free
	 * --------------------------------------------------------------------------------------------------- */
	int bestBoardId = GetBestARTBoardNoOtherRequirements(partyData, pAllocDataPerBoardArray);
	if (bestBoardId == 0)
	{
		PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestBoardsForVoipParty - no ART found");
		return STATUS_INSUFFICIENT_ART_RSRC;
	}
	bestAllocStruct.m_ArtBoardId = bestBoardId;

	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestBoardsForIPVideoParty(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	/* --------------------------------------------------------------------------------------------------
	 * Allocation priorities for IP:
	 * *******************************
	 * Preferably no video downgrade
	 * Video and ART on same board
	 * If both cards have Video and ART free, choose the one with the most video free
	 *
	 * --------------------------------------------------------------------------------------------------- */

	AllocRequirements allocRequirements;

	allocRequirements.AllFalse();
	allocRequirements.m_bCheckARTAvailability = TRUE;
	allocRequirements.m_bCheckVideoAvailability = TRUE;
	allocRequirements.m_bCheckVideoAvailabilityOptimalUnits = TRUE;

	//Step 1: video and ART together, optimal video
	int bestBoardId = GetBestBoardThatMeetsRequirements(NUM_PARTICIPANTS_THEN_VIDEO_FREE_THEN_FREE_PORTS_COMPARER, partyData, pAllocDataPerBoardArray, allocRequirements);
	if (bestBoardId != 0)
	{
		bestAllocStruct.m_ArtBoardId = bestBoardId;
		bestAllocStruct.m_VideoAlloc.m_boardId = bestBoardId;
		TRACEINTO << "BoardId:" << bestBoardId << " - Found video and ART together optimal video best board";
		return STATUS_OK;
	}
	else
	{
		TRACEINTO << "Can't find video and ART together, optimal video";
	}

	//Step 2: video and ART separate, optimal video
	bestBoardId = GetBestVideoBoardNoOtherRequirements(TRUE, partyData, pAllocDataPerBoardArray);
	if (bestBoardId != 0)
	{
		bestAllocStruct.m_VideoAlloc.m_boardId = bestBoardId;

		bestBoardId = GetBestARTBoardNoOtherRequirements(partyData, pAllocDataPerBoardArray);
		if (bestBoardId == 0) //no ART found at all
		{
			TRACEINTOLVLERR << "Failed find ART, INSUFFICIENT_ART_RSRC";
			return STATUS_INSUFFICIENT_ART_RSRC;
		}
		bestAllocStruct.m_ArtBoardId = bestBoardId;
		TRACEINTO << "BoardId:" << bestBoardId << " - Found video and ART separate, optimal video";
		return STATUS_OK;
	}
	else
	{
		TRACEINTO << "Can't find find video and ART separate, optimal video";
	}

	//Step 3: video and ART together, not optimal video
	allocRequirements.AllFalse();
	allocRequirements.m_bCheckARTAvailability = TRUE;
	allocRequirements.m_bCheckVideoAvailability = TRUE;
	// when the video is not optimal we should prefer the best video downgrade
	// and only then try to allocate all conf video parties on the same board
	// vngr-15855
	bestBoardId = GetBestBoardThatMeetsRequirements(VIDEO_FREE_THEN_NUM_PARTICIPANTS_COMPARER, partyData, pAllocDataPerBoardArray, allocRequirements);
	if (bestBoardId != 0)
	{
		bestAllocStruct.m_ArtBoardId = bestBoardId;
		bestAllocStruct.m_VideoAlloc.m_boardId = bestBoardId;
		TRACEINTO << "BoardId:" << bestBoardId << " - Found video and ART together, but not optimal video";
		return STATUS_OK;
	}
	else
	{
		TRACEINTO << "Can't find video and ART together, not optimal video";
	}

	//Step 4: video and ART separate, not optimal video
	bestBoardId = GetBestVideoBoardNoOtherRequirements(FALSE, partyData, pAllocDataPerBoardArray);
	if (bestBoardId == 0)
	{
		TRACEINTOLVLERR << "Failed find video and ART separate, not optimal video, INSUFFICIENT_ART_RSRC";
		return STATUS_INSUFFICIENT_VIDEO_RSRC;
	}
	else
	{
		TRACEINTO << "BoardId:" << bestBoardId << " - Found video and ART separate, not optimal video";
	}

	bestAllocStruct.m_VideoAlloc.m_boardId = bestBoardId;

	bestBoardId = GetBestARTBoardNoOtherRequirements(partyData, pAllocDataPerBoardArray);
	if (bestBoardId == 0) //no ART found at all
	{
		TRACEINTOLVLERR << "BoardId:" << bestBoardId << " - Failed find ART unit on this board, INSUFFICIENT_ART_RSRC";
		return STATUS_INSUFFICIENT_ART_RSRC;
	}

	bestAllocStruct.m_ArtBoardId = bestBoardId;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestBoardsForCOP(BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	/* --------------------------------------------------------------------------------------------------
	 * Allocation priorities for COP:
	 * *******************************
	 * --------------------------------------------------------------------------------------------------- */
	int bestBoardId = 0; //1-based!!!
	//Step 1: video , no other requirement
	bestBoardId = GetBestVideoBoardNoOtherRequirementsCOP(pAllocDataPerBoardArray);
	if (bestBoardId != 0)
	{
		bestAllocStruct.m_VideoAlloc.m_boardId = bestBoardId;
		return STATUS_OK;
	}

	return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////
void CAllocationDecider::FillBestVideoDataAccordingToBestVideoBoard(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	int boardIndex;

	if (bestAllocStruct.m_VideoAlloc.m_boardId > 0 && bestAllocStruct.m_VideoAlloc.m_boardId <= BOARDS_NUM)
	{
		boardIndex = bestAllocStruct.m_VideoAlloc.m_boardId - 1; //the boards are 1-based, while the index is zero-based
	}
	else
	{
		PASSERT(bestAllocStruct.m_VideoAlloc.m_boardId + 100);
		boardIndex = 0;
	}

	bestAllocStruct.m_VideoAlloc = pAllocDataPerBoardArray[boardIndex].m_VideoData.m_VideoAlloc;
	bestAllocStruct.m_NumVideoUnitsToReconfigure = pAllocDataPerBoardArray[boardIndex].m_VideoData.m_NumVideoUnitsToReconfigure;

	//update the party type if it was downgraded
	if (pAllocDataPerBoardArray[boardIndex].m_VideoData.m_bWasDownGraded == TRUE)
	{
		TRACEINTO << "\nRESOURCE_CAUSE: CAllocationDecider::FillBestVideoDataAccordingToBestVideoBoard downgrade type is=" << eVideoPartyTypeNames[pAllocDataPerBoardArray[boardIndex].m_VideoData.m_DownGradedPartyType] << "\n";
		partyData.m_videoPartyType = pAllocDataPerBoardArray[boardIndex].m_VideoData.m_DownGradedPartyType;
	}
}

////////////////////////////////////////////////////////////////////////////
void CAllocationDecider::FillBestVideoDataAccordingToBestVideoBoardCOP(BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	int boardIndex = bestAllocStruct.m_VideoAlloc.m_boardId - 1; //the boards are 1-based, while the index is zero-based
	bestAllocStruct.m_VideoAlloc = pAllocDataPerBoardArray[boardIndex].m_VideoData.m_VideoAlloc;
}

////////////////////////////////////////////////////////////////////////////
void CAllocationDecider::FillVideoDataForVSW56Session(BestAllocStruct& bestAllocStruct1, BestAllocStruct& bestAllocStruct2, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	bestAllocStruct1.m_VideoAlloc = pAllocDataPerBoardArray[0].m_VideoData.m_VideoAlloc;
	bestAllocStruct2.m_VideoAlloc = pAllocDataPerBoardArray[1].m_VideoData.m_VideoAlloc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAllocationDecider::FillBestAudioDataAccordingToBestAudioBoard(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	int boardIndex = bestAllocStruct.m_ArtBoardId - 1; //the boards are 1-based, while the index is zero-based
	if (pAllocDataPerBoardArray[boardIndex].m_ARTData.m_bCanAndShouldReconfigureVideoUnitToART == TRUE)
		bestAllocStruct.m_NumARTUnitsToReconfigure = 1;
}

////////////////////////////////////////////////////////////////////////////
void CAllocationDecider::FillBestRTMDataAccordingToBestRTMBoards(PartyDataStruct& partyData, ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	CSpanRTM* best_spans_per_board[MAX_NUM_SPANS_ORDER];

	int currentBoardIndex = -1;
	int boardIndex;
	int zeroBasedBoardId;
	STATUS status;

	int allreadyAllocatedOnCurrentSpan = 0;
	int freePortsThatAreNotReservedOnCurrentSpan = 0;

	for (int port_index = 0; port_index < partyData.m_pIsdn_Params_Request->num_of_isdn_ports; port_index++)
	{
		//for each "new" board, load the best spans list (usually will be done once, unless the party was split between boards)
		boardIndex = pIsdn_Params_Response->spans_order.port_spans_list[port_index].board_id;
		if (boardIndex != currentBoardIndex)
		{
			currentBoardIndex = boardIndex;
			for (int l = 0; l < MAX_NUM_SPANS_ORDER; l++)
				best_spans_per_board[l] = NULL;

			zeroBasedBoardId = currentBoardIndex - 1;

			status = m_pNetServicesDB->GetBestSpansListPerBoard(*(partyData.m_pIsdn_Params_Request), best_spans_per_board, zeroBasedBoardId);
			if (status != STATUS_OK)
			{
				PASSERT(1); //shouldn't happen
				return;
			}

			allreadyAllocatedOnCurrentSpan = 0;
			freePortsThatAreNotReservedOnCurrentSpan = 0;
			if (best_spans_per_board[0] != NULL)
				freePortsThatAreNotReservedOnCurrentSpan = best_spans_per_board[0]->GetNumFreePortsThatAreNotReserved();
		}

		//now that we have the best spans, fill data for this port,
		for (int i = 0; i < MAX_NUM_SPANS_ORDER; i++)
		{
			pIsdn_Params_Response->spans_order.port_spans_list[port_index].spans_list[i] = (best_spans_per_board[i] ? best_spans_per_board[i]->GetUnitId() : 0);
		}
		allreadyAllocatedOnCurrentSpan++;
		if (allreadyAllocatedOnCurrentSpan == freePortsThatAreNotReservedOnCurrentSpan)
		//we alocated all ports on current span, so move to next span: split between spans
		{
			PutFirstSpanLast(best_spans_per_board);
			allreadyAllocatedOnCurrentSpan = 0; //start counting again
			if (best_spans_per_board[0] != NULL)
				freePortsThatAreNotReservedOnCurrentSpan = best_spans_per_board[0]->GetNumFreePortsThatAreNotReserved();
		}
	}
}

////////////////////////////////////////////////////////////////////////////
BOOL CAllocationDecider::BoardMeetsRequirements(PartyDataStruct& partyData, DataPerBoardStruct& boardData, AllocRequirements& allocRequirements, bool fullVideoReq)
{
	if (allocRequirements.m_bCheckRTMFullAvailability)
	{
		if (boardData.m_RTMData.m_bCanBeAllocated == FALSE || boardData.m_RTMData.m_numFreeRTMPorts == 0)
			return FALSE;
	}

	if (allocRequirements.m_bCheckRTMPartialAvailability)
	{
		if (boardData.m_RTMData.m_bCanBePartiallyAllocated == FALSE || boardData.m_RTMData.m_numFreeRTMPorts == 0)
			return FALSE;
	}

	if (allocRequirements.m_bCheckARTAvailability)
	{
		if (allocRequirements.m_bAllowVideoReconfigurationToART == FALSE)
		{
			if (boardData.m_ARTData.m_bCanBeAllocated == FALSE || boardData.m_ARTData.m_numFreeArtPorts == 0)
			{
				// PTRACE(eLevelInfoNormal,"CAllocationDecider::BoardMeetsRequirements , boardData.m_ARTData.m_bCanBeAllocated == FALSE  || boardData.m_ARTData.m_numFreeArtPorts == 0");
				return FALSE;
			}
		}
		else
		{
			if ((boardData.m_ARTData.m_bCanBeAllocated == FALSE || boardData.m_ARTData.m_numFreeArtPorts == 0) && boardData.m_ARTData.m_bCanAndShouldReconfigureVideoUnitToART == FALSE)
				// PTRACE(eLevelInfoNormal,"CAllocationDecider::BoardMeetsRequirements , boardData.m_ARTData");
				return FALSE;
		}
	}

	if (allocRequirements.m_bCheckVideoAvailability)
	{
		if (boardData.m_VideoData.m_bCanBeAllocated == FALSE)
		{
			// PTRACE(eLevelInfoNormal,"CAllocationDecider::BoardMeetsRequirements , boardData.m_VideoData.m_bCanBeAllocated == FALSE");
			return FALSE;
		}
	}

	if (allocRequirements.m_bCheckVideoAvailabilityOptimalUnits)
	{
		if (fullVideoReq == TRUE)
		{
			if (CheckVideoAvailabilityOptimalUnits(partyData, boardData) == FALSE)
				return FALSE;
		}
		else
		{ //VNGR-15461
			if (CheckOptimalVideoAvailability(partyData, boardData) == FALSE)
				return FALSE;
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CAllocationDecider::BoardMeetsRequirementsCOP(DataPerBoardStruct& boardData, AllocRequirements& allocRequirements)
{

	if (allocRequirements.m_bCheckVideoAvailability)
	{
		if (boardData.m_VideoData.m_bCanBeAllocated == FALSE)
			return FALSE;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CAllocationDecider::CheckVideoAvailabilityOptimalUnits(PartyDataStruct& partyData, DataPerBoardStruct& boardData)
{
	//better not to reconfigure units
	if (boardData.m_VideoData.m_NumVideoUnitsToReconfigure > 0)
		return FALSE;

	//means that if it's CIF, then it's on a fragmented unit and if it's any video it will not be downgrade
	if (CHelperFuncs::IsVideoBasicParty(partyData.m_videoPartyType))
	{
		if (boardData.m_VideoData.m_bFragmentedUnit == FALSE)
			return FALSE;
	}

	if (CHelperFuncs::IsVideoParty(partyData.m_videoPartyType))
	{
		if (boardData.m_VideoData.m_bWasDownGraded == TRUE)
			return FALSE;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CAllocationDecider::CheckOptimalVideoAvailability(PartyDataStruct& partyData, DataPerBoardStruct& boardData)
{ //VNGR-15461
	if (CHelperFuncs::IsVideoParty(partyData.m_videoPartyType))
	{
		if (boardData.m_VideoData.m_bWasDownGraded == TRUE)
		{
			PTRACE(eLevelInfoNormal, "CAllocationDecider::CheckOptimalVideoAvailability , boardData.m_VideoData.m_bWasDownGraded == TRUE");
			return FALSE;
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
int CAllocationDecider::GetCompareData(eBoardComparer boardComparer, DataPerBoardStruct& boardData)
{
	switch (boardComparer)
	{
		case eBoardComparer_Rtm_Free_Ports  : return boardData.m_RTMData.m_numFreeRTMPorts;
		case eBoardComparer_Art_Free_Ports  : return boardData.m_ARTData.m_numFreeArtPorts;
		case eBoardComparer_Video_Free_Ports: return boardData.m_VideoData.m_numFreeVideoPortsCapacity;
		case eBoardComparer_Video_Free      : return boardData.m_VideoData.m_numFreeVideoCapacity;
		case eBoardComparer_Video_Parties   : return boardData.m_VideoData.m_NumVideoPartiesSameConf;

		default:
			PASSERT(1);
			return 0;
	}
}

////////////////////////////////////////////////////////////////////////////
int CAllocationDecider::GetBestBoardThatMeetsRequirements(eBoardComparer boardComparer, PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray, AllocRequirements& allocRequirements, int notOnThisBoard /*=0*/)
{
	eBoardComparer boardComparers[MAX_NUMBER_OF_BOARD_COMPARERS];
	boardComparers[0] = boardComparer;
	for (int i = 1; i < MAX_NUMBER_OF_BOARD_COMPARERS; i++)
		boardComparers[i] = eBoardComparer_None;

	return GetBestBoardThatMeetsRequirements(boardComparers, partyData, pAllocDataPerBoardArray, allocRequirements, notOnThisBoard);
}

////////////////////////////////////////////////////////////////////////////
int CAllocationDecider::GetBestBoardThatMeetsRequirementsCOP(eBoardComparer boardComparer, DataPerBoardStruct* pAllocDataPerBoardArray, AllocRequirements& allocRequirements, int notOnThisBoard /*=0*/)
{
	int boardIdWithMaxValue = 0; //1 based!!!
	int maxValue = -1;
	int currentValue = 0;
	for (int board_id = 0; board_id < BOARDS_NUM; board_id++)
	{
		if (notOnThisBoard != 0 && notOnThisBoard == board_id + 1)
			continue;

		if (BoardMeetsRequirementsCOP(pAllocDataPerBoardArray[board_id], allocRequirements) == TRUE)
		{
			currentValue = GetCompareData(boardComparer, pAllocDataPerBoardArray[board_id]);
			if (currentValue > maxValue)
			{
				maxValue = currentValue;
				boardIdWithMaxValue = board_id + 1;
			}
		}
	}

	return boardIdWithMaxValue;
}

////////////////////////////////////////////////////////////////////////////
int CAllocationDecider::GetBestBoardThatMeetsRequirements(eBoardComparer boardComparers[MAX_NUMBER_OF_BOARD_COMPARERS], PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray, AllocRequirements& allocRequirements, int notOnThisBoard /*=0*/)
//this function will return 0 if no board was found that meets the requirements!!!
{
	std::ostringstream msg;

	int boardIdWithMaxValue = 0; //1 based!!!
	int maxValue[MAX_NUMBER_OF_BOARD_COMPARERS];
	for (int i = 0; i < MAX_NUMBER_OF_BOARD_COMPARERS; i++)
		maxValue[i] = -1;

	//VNGR-15461
	bool isNeedAllowReconfigForOptimalVideo = true;
	bool isNotFullOptimalVideoFit = false;
	bool fullVideoReq = true;

	if (allocRequirements.m_bCheckVideoAvailabilityOptimalUnits)
	{
		for (int board_id = 0; board_id < BOARDS_NUM; board_id++)
		{
			if (notOnThisBoard != 0 && notOnThisBoard == board_id + 1)
				continue;

			if (BoardMeetsRequirements(partyData, pAllocDataPerBoardArray[board_id], allocRequirements, true) == TRUE) //full requirements
				isNeedAllowReconfigForOptimalVideo = false;

			if (BoardMeetsRequirements(partyData, pAllocDataPerBoardArray[board_id], allocRequirements, false) == TRUE) //part of requirements fits
				isNotFullOptimalVideoFit = true;
		}

		if ((isNeedAllowReconfigForOptimalVideo == true) //we don't have the best board without reconfig
		&& (isNotFullOptimalVideoFit == true)) //we have part of requirements fits
		{
			TRACEINTO << "Can't find optimal video unless allow reconfigure";
			fullVideoReq = false;
		}
	}

	for (int board_id = 0; board_id < BOARDS_NUM; board_id++)
	{
		msg << "\nBoardId:" << board_id+1;
		if (notOnThisBoard != 0 && notOnThisBoard == board_id + 1)
		{
			msg << " - Board should be skipped";
			continue;
		}

		if (BoardMeetsRequirements(partyData, pAllocDataPerBoardArray[board_id], allocRequirements, fullVideoReq) == TRUE)
		{
			for (int j = 0; j < MAX_NUMBER_OF_BOARD_COMPARERS; j++)
			{
				if (boardComparers[j] == eBoardComparer_None)
				{
					msg << "\n  " << boardComparers[j] << ", Finish";
					break;
				}
				int currentValue = GetCompareData(boardComparers[j], pAllocDataPerBoardArray[board_id]);
				msg << "\n\t" << left << setw(31) << boardComparers[j] << " | CurValue:" << setw(5) << currentValue << " | MaxValue:" << maxValue[j];
				if (currentValue > maxValue[j])
				{
					msg << ", setting maxValue[" << boardComparers[j] << "]";
					maxValue[j] = currentValue;
					boardIdWithMaxValue = board_id + 1;
					for (int k = j + 1; k < MAX_NUMBER_OF_BOARD_COMPARERS; k++)
					{
						if (boardComparers[k] == eBoardComparer_None)
							break;
						maxValue[k] = GetCompareData(boardComparers[k], pAllocDataPerBoardArray[board_id]);
					}
					break;
				}
				else if (currentValue < maxValue[j])
				{
					msg << " , break";
					break;
				}
			}
		}
		else
		{
			msg << " - Board does not meets requirements";
		}
	}
	msg << "\n-----------------";
	msg << "\nChoosen BoardId:" << boardIdWithMaxValue;

	TRACEINTO << msg.str().c_str();

	return boardIdWithMaxValue;
}

////////////////////////////////////////////////////////////////////////////
int CAllocationDecider::GetBestVideoBoardNoOtherRequirements(BOOL bOptimalVideo, PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	AllocRequirements allocRequirements;

	allocRequirements.m_bCheckVideoAvailability = TRUE;
	if (bOptimalVideo == TRUE)
		allocRequirements.m_bCheckVideoAvailabilityOptimalUnits = TRUE;

	return GetBestBoardThatMeetsRequirements(NUM_PARTICIPANTS_THEN_VIDEO_FREE_THEN_FREE_PORTS_COMPARER, partyData, pAllocDataPerBoardArray, allocRequirements);
}

////////////////////////////////////////////////////////////////////////////
int CAllocationDecider::GetBestVideoBoardNoOtherRequirementsCOP(DataPerBoardStruct* pAllocDataPerBoardArray)
{
	AllocRequirements allocRequirements;

	allocRequirements.m_bCheckVideoAvailability = TRUE;
	return GetBestBoardThatMeetsRequirementsCOP(eBoardComparer_Video_Free, pAllocDataPerBoardArray, allocRequirements);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAllocationDecider::GetBestARTBoardNoOtherRequirements(PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	AllocRequirements allocRequirements;
	allocRequirements.m_bCheckARTAvailability = TRUE;

	int boardId = GetBestBoardThatMeetsRequirements(eBoardComparer_Art_Free_Ports, partyData, pAllocDataPerBoardArray, allocRequirements);
	if (boardId != 0)
		return boardId;

	if (partyData.m_allowReconfiguration == TRUE)
	{
		allocRequirements.m_bAllowVideoReconfigurationToART = TRUE;
		return GetBestBoardThatMeetsRequirements(eBoardComparer_Video_Free, partyData, pAllocDataPerBoardArray, allocRequirements);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////
BOOL CAllocationDecider::GetBestRTMBoardsNoOtherRequirements_And_SetInResponseStruct(PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray, ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, int notOnThisBoard /*= 0*/)
{
	//first try to find a board where all RTM channels fit in
	//--------------------------------------------------------
	AllocRequirements allocRequirements;
	allocRequirements.m_bCheckRTMFullAvailability = TRUE;

	int boardId = GetBestBoardThatMeetsRequirements(eBoardComparer_Rtm_Free_Ports, partyData, pAllocDataPerBoardArray, allocRequirements, notOnThisBoard);
	if (boardId != 0)
	{
		SetRTMBoardForAllChannels(partyData, pIsdn_Params_Response, boardId);
		return TRUE;
	}

	//if not found, SPLIT between boards
	//-----------------------------------
	int numberLeftToAllocate = partyData.m_pIsdn_Params_Request->num_of_isdn_ports;
	int channelStartIndex = 0;
	int numToAllocateOnThisBoard = 0;

	allocRequirements.AllFalse();
	allocRequirements.m_bCheckRTMPartialAvailability = TRUE;

	for (int board_id = 0; board_id < BOARDS_NUM && numberLeftToAllocate != 0; board_id++)
	{
		if (notOnThisBoard != 0 && notOnThisBoard == board_id + 1)
			continue;

		if (BoardMeetsRequirements(partyData, pAllocDataPerBoardArray[board_id], allocRequirements))
		{
			numToAllocateOnThisBoard = min(numberLeftToAllocate, pAllocDataPerBoardArray[board_id].m_RTMData.m_numFreeRTMPorts);
			SetRTMBoardIdForChannels(pIsdn_Params_Response, board_id + 1, channelStartIndex, numToAllocateOnThisBoard);
			numberLeftToAllocate -= numToAllocateOnThisBoard;
			channelStartIndex += numToAllocateOnThisBoard;
		}
	}

	if (numberLeftToAllocate != 0)
	{
		//for some reason we didn't found enough RTM channels to allocate. This shouldn't happen (because we checked before starting the whole allocation procedure)
		//but this might happen see vngr 9972
		DBGPASSERT(1);
		return FALSE;
	}
	else
		return TRUE;
}

////////////////////////////////////////////////////////////////////////////
void CAllocationDecider::DumpData(PartyDataStruct& partyData, DataPerBoardStruct* pAllocDataPerBoardArray)
{
	std::ostringstream msg;

	msg
		<< "\nPartyDataStruct:"
		<< "\n  MonitorConfId            :" << partyData.m_monitor_conf_id
		<< "\n  VideoPartyType           :" << eVideoPartyTypeNames[partyData.m_videoPartyType]
		<< "\n  NetworkPartyType         :" << eNetworkPartyTypeNames[partyData.m_networkPartyType]
		<< "\n  AllowReconfiguration     :" << (int)partyData.m_allowReconfiguration;

	if (CHelperFuncs::IsISDNParty(partyData.m_networkPartyType))
	{
		msg
			<< "\n  IsdnRequestNumOfPorts    :" << partyData.m_pIsdn_Params_Request->num_of_isdn_ports
			<< "\n  IsdnRequestBoardId       :" << partyData.m_pIsdn_Params_Request->board_id
			<< "\n  IsdnRequestSpanId        :" << partyData.m_pIsdn_Params_Request->span_id;
	}

	WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
	for (int board_id = 0; board_id < BOARDS_NUM; board_id++)
	{
		msg
			<< "\n"
			<< "\n**BoardIndex               :" << board_id
			<< "\n  BoardId                  :" << pAllocDataPerBoardArray[board_id].m_VideoData.m_VideoAlloc.m_boardId
			<< "\n  FreeVideoCapacity        :" << pAllocDataPerBoardArray[board_id].m_VideoData.m_numFreeVideoCapacity
			<< "\n  DownGradedPartyType      :" << eVideoPartyTypeNames[pAllocDataPerBoardArray[board_id].m_VideoData.m_DownGradedPartyType]
			<< "\n  WasDownGraded            :" << (int)pAllocDataPerBoardArray[board_id].m_VideoData.m_bWasDownGraded
			<< "\n  FragmentedUnit           :" << (int)pAllocDataPerBoardArray[board_id].m_VideoData.m_bFragmentedUnit
			<< "\n  NumVideoUnitsToReconfig  :" << pAllocDataPerBoardArray[board_id].m_VideoData.m_NumVideoUnitsToReconfigure
			<< "\n  CanBeAllocated           :" << (int)pAllocDataPerBoardArray[board_id].m_VideoData.m_bCanBeAllocated;

		for (int i = 0; i < min(max_units_video, MAX_UNITS_NEEDED_FOR_VIDEO_COP); i++)
		{
			msg << "\n  UnitIndexToUnitId        :" << i << "-" << pAllocDataPerBoardArray[board_id].m_VideoData.m_VideoAlloc.m_unitsList[i].m_UnitId;
		}
		for (int i = 0; i < NUM_OF_FPGAS_PER_BOARD; i++)
		{
			msg << "\n  FPGA Index               :" << i << " [in:" << pAllocDataPerBoardArray[board_id].m_VideoData.m_freeBandwidth_in[i] << ", out:" << pAllocDataPerBoardArray[board_id].m_VideoData.m_freeBandwidth_out[i] << "]";
		}
		msg
			<< "\n  RTM data:"
			<< "\n    CanBeAllocated         :" << (int)pAllocDataPerBoardArray[board_id].m_RTMData.m_bCanBeAllocated
			<< "\n    CanBePartiallyAllocated:" << (int)pAllocDataPerBoardArray[board_id].m_RTMData.m_bCanBePartiallyAllocated
			<< "\n    NumFreeRTMPorts        :" << pAllocDataPerBoardArray[board_id].m_RTMData.m_numFreeRTMPorts
			<< "\n  ART data:"
			<< "\n    CanBeAllocated         :" << (int)pAllocDataPerBoardArray[board_id].m_ARTData.m_bCanBeAllocated
			<< "\n    NumFreeArtPorts        :" << pAllocDataPerBoardArray[board_id].m_ARTData.m_numFreeArtPorts;
	}
	TRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
void CAllocationDecider::PutFirstSpanLast(CSpanRTM* best_spans_per_board[MAX_NUM_SPANS_ORDER])
{
	CSpanRTM* pfirstSpan = best_spans_per_board[0];

	int j;
	for (j = 1; j < MAX_NUM_SPANS_ORDER && best_spans_per_board[j] != NULL; j++)
	{
		best_spans_per_board[j - 1] = best_spans_per_board[j];
	}

	best_spans_per_board[j - 1] = pfirstSpan;
}

////////////////////////////////////////////////////////////////////////////
void CAllocationDecider::SetRTMBoardIdForChannels(ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, int boardId, int beginChannelIndex, int numChannelToSet)
{
	for (int i = 0; i < numChannelToSet; i++)
	{
		pIsdn_Params_Response->spans_order.port_spans_list[beginChannelIndex + i].board_id = boardId;
	}
}

////////////////////////////////////////////////////////////////////////////
void CAllocationDecider::SetRTMBoardForAllChannels(PartyDataStruct& partyData, ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, int boardId)
{
	SetRTMBoardIdForChannels(pIsdn_Params_Response, boardId, 0, partyData.m_pIsdn_Params_Request->num_of_isdn_ports);
}

////////////////////////////////////////////////////////////////////////////
STATUS CAllocationDecider::DecideAboutBestRTMBoards(PartyDataStruct& partyData, ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, DataPerBoardStruct* pAllocDataPerBoardArray, int notOnThisBoard, int preferablyOnThisBoard)
{
	TRACEINTO;

	AllocRequirements allocRequirements;

	if (preferablyOnThisBoard != 0)
	{
		allocRequirements.m_bCheckRTMFullAvailability = TRUE;
		int zeroBasedBId = preferablyOnThisBoard - 1;
		if (BoardMeetsRequirements(partyData, pAllocDataPerBoardArray[zeroBasedBId], allocRequirements))
		{
			SetRTMBoardForAllChannels(partyData, pIsdn_Params_Response, preferablyOnThisBoard);
			FillBestRTMDataAccordingToBestRTMBoards(partyData, pIsdn_Params_Response, pAllocDataPerBoardArray);
			return STATUS_OK;
		}
	}

	BOOL bRTMSuccess = GetBestRTMBoardsNoOtherRequirements_And_SetInResponseStruct(partyData, pAllocDataPerBoardArray, pIsdn_Params_Response, notOnThisBoard);
	if (bRTMSuccess == FALSE)
	{
		PTRACE(eLevelInfoNormal, "CAllocationDecider::DecideAboutBestRTMBoards - no sufficient RTM found");
		DumpData(partyData, pAllocDataPerBoardArray);
		return STATUS_INSUFFICIENT_RTM_RSRC;
	}

	FillBestRTMDataAccordingToBestRTMBoards(partyData, pIsdn_Params_Response, pAllocDataPerBoardArray);
	return STATUS_OK;
}
