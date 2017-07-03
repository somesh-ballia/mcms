#include "TraceStream.h"
#include "MoveManager.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "SystemResources.h"
#include "HelperFuncs.h"
#include "CardsStructs.h"
#include "OpcodesMcmsInternal.h"
#include "InternalProcessStatuses.h"
#include "TaskApi.h"
#include "ManagerApi.h"
#include "CardResourceConfig.h"

///////////////////////////////////////////////////////////////////////
CMoveManager::CMoveManager()
{
}

///////////////////////////////////////////////////////////////////////
CMoveManager::~CMoveManager()
{
}

///////////////////////////////////////////////////////////////////////
STATUS CMoveManager::ReconfigureUnit(int boardId, int unitId, eUnitType unitType)
{
	FTRACEINTO << "BoardId:" << boardId << ", UnitId:" << unitId << ", UnitType:" << unitType;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	FPASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	// check that we are allowing reconfigure
	FPASSERTMSG_AND_RETURN_VALUE(CHelperFuncs::IsEnhancedCardsAllocationType() == FALSE, "Reconfigure unit can only be done with breeze", STATUS_FAIL);

	//check that the board exists
	CBoard* pBoard = pSystemResources->GetBoard(boardId);
	FPASSERTSTREAM_AND_RETURN_VALUE(!pBoard, "BoardId:" << boardId << " - Board not found", STATUS_FAIL);

	//check that the unit exists
	CUnitMFA* pUnit = (CUnitMFA*)(pBoard->GetMFA(unitId));
	FPASSERTSTREAM_AND_RETURN_VALUE(!pUnit, "UnitId:" << unitId << " - Unit not found", STATUS_FAIL);

	//check that it's empty
	FPASSERTSTREAM_AND_RETURN_VALUE(pUnit->GetIsAllocated() == TRUE, "UnitId:" << unitId << " - Unit not empty", STATUS_FAIL);

	//check that it is not already of the new type
	FPASSERTSTREAM_AND_RETURN_VALUE(pUnit->GetUnitType() == unitType, "UnitId:" << unitId << " - Unit already configured that way", STATUS_FAIL);

	//check that it is not audio controller
	FPASSERTMSG_AND_RETURN_VALUE(unitId == pBoard->GetAudioControllerUnitId(), "Can't change audio controller", STATUS_FAIL);

	//send reconfigure
	eCardUnitTypeConfigured newUnitType = (unitType == eUnitType_Art) ?  eArt : eVideo;

	pUnit->SetEnabled(FALSE); // we should return status to enable after receiving indication from embedded.

	UNIT_RECONFIG_S configParamsStruct;
	configParamsStruct.boardId = boardId;
	configParamsStruct.unitId = unitId;
	configParamsStruct.unitType = newUnitType;

	CSegment* pSegment = new CSegment;
	pSegment->Put((BYTE*)&configParamsStruct, sizeof(configParamsStruct));

	CManagerApi api(eProcessCards);
	api.SendMsg(pSegment, RESOURCE_UNIT_RECONFIG_REQ);

	return STATUS_OK;
}

///////////////////////////////////////////////////////////////////////
void CMoveManager::ReceivedReconfigureUnitInd(UNIT_RECONFIG_S* pIndication)
{
	eUnitReconfigStatus status = (eUnitReconfigStatus)pIndication->unitStatus;

	FTRACEINTO << "BoardId:" << pIndication->boardId << ", UnitId:" << pIndication->unitId;

	//status OK,
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	//check that the board exists
	CBoard* pBoard = pSystemResources->GetBoard(pIndication->boardId);
	PASSERTSTREAM_AND_RETURN(!pBoard, "BoardId:" << pIndication->boardId << " - Board not found");

	//check that the unit exists
	CUnitMFA* pUnit = (CUnitMFA*)(pBoard->GetMFA(pIndication->unitId));
	PASSERTSTREAM_AND_RETURN(!pUnit, "UnitId:" << pIndication->unitId << " - Unit not found");

	WORD numOfAudPortsPerUnit = pBoard->CalculateNumberOfPortsPerUnit(PORT_ART);
	WORD numOfVidPortsPerUnit = pBoard->CalculateNumberOfPortsPerUnit(PORT_VIDEO);

	//update our internal db
	eCardUnitTypeConfigured newUnitType = (eCardUnitTypeConfigured)pIndication->unitType;
	eUnitType unitType;
	eUnitType oldUnitType = pUnit->GetUnitType();
	BYTE enbl_st = (status == eUnitReconfigOk ? TRUE : FALSE);
	if (newUnitType == eArt)
	{
		unitType = eUnitType_Art;
		if (oldUnitType != eUnitType_Art)
		{
			//update numbers of configuration
			pSystemResources->SetNumConfiguredArtPorts(pSystemResources->GetNumConfiguredArtPorts(pIndication->boardId) + numOfAudPortsPerUnit, pIndication->boardId);
			pSystemResources->SetNumConfiguredVideoPorts(pSystemResources->GetNumConfiguredVideoPorts(pIndication->boardId) - numOfVidPortsPerUnit, pIndication->boardId);
		}
	}
	else
	{
		unitType = eUnitType_Video;
		if (oldUnitType != eUnitType_Video)
		{
			//update numbers of configuration
			pSystemResources->SetNumConfiguredArtPorts(pSystemResources->GetNumConfiguredArtPorts(pIndication->boardId) - numOfAudPortsPerUnit, pIndication->boardId);
			pSystemResources->SetNumConfiguredVideoPorts(pSystemResources->GetNumConfiguredVideoPorts(pIndication->boardId) + numOfVidPortsPerUnit, pIndication->boardId);
		}
	}
	pUnit->SetUnitType(unitType);
	pUnit->SetEnabled(enbl_st);
}

