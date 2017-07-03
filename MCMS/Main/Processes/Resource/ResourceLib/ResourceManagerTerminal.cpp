#include "ResourceManager.h"
#include "ResourceManagerHelper.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsInternal.h"
#include "MplMcmsProtocolTracer.h"
#include "TerminalCommand.h"
#include "HelperFuncs.h"
#include "ProfilesDB.h"

BEGIN_TERMINAL_COMMANDS(CResourceManager)
	ONCOMMAND("CheckConsistency",CResourceManager::HandleTerminalCheckConsistency,"Check Consistency")
	ONCOMMAND("DisableUnit",CResourceManager::HandleTerminalDisableUnit,"Disable Unit")
	ONCOMMAND("SimulateUnitFatal",CResourceManager::HandleTerminalUnitFatal,"Simulate UNIT_FATAL_IND")
	ONCOMMAND("SetMaxArtChannels",CResourceManager::HandleTerminalSetMaxArtChannelsPerArt,"Set Max Art Channels Per Art")
	ONCOMMAND("ShowArtChannels",CResourceManager::HandleTerminalShowNumArtChannels,"Show Art Channels")
	ONCOMMAND("DisableBoard",CResourceManager::HandleTerminalDisableBoard,"Disable Board")
	ONCOMMAND("RemoveBoard",CResourceManager::HandleTerminalRemoveBoard,"Remove Board")
	ONCOMMAND("ReconfigureUnit",CResourceManager::HandleTerminalReconfigureUnit,"Reconfigure Unit")
	ONCOMMAND("ResetPortConfiguration",CResourceManager::HandleTerminalResetPortConfiguration,"Reset port configuration")

	ONCOMMAND("ShowMasterAC",CResourceManager::HandleTerminalShowMasterAC,"Show Master Audio Controller Board ID")
	ONCOMMAND("ShowSlaveAC",CResourceManager::HandleTerminalShowSlaveAC,"Show Slave Audio Controller Board ID")
	ONCOMMAND("ShowReservedAC",CResourceManager::HandleTerminalShowResevedAC,"Show Reserved Audio Controllers Board IDs")
	ONCOMMAND("MoveMasterAC",CResourceManager::HandleTerminalMoveMasterAC,"Move Master Audio Controller")
	ONCOMMAND("RestoreMasterAC",CResourceManager::HandleTerminalRestoreMasterAC,"Restore Master Audio Controller")
	ONCOMMAND("ShowIvrCntrlr",CResourceManager::HandleTerminalShowIvrCntrlr,"Show IVR Controller Board ID")
	ONCOMMAND("ShowSharedMemory",CResourceManager::HandleTerminalShowSharedMemory,"Show Shared Memory Content")
	ONCOMMAND("ShowSlotTable",CResourceManager::HandleTerminalShowSlotTable,"Show Slot Number Conversion Table")
	ONCOMMAND("ShowPQ",CResourceManager::HandleTerminalShowPQ,"Show PQ content")
	ONCOMMAND("NidCount",CResourceManager::HandleTerminalNidCount,"Show number of occupied numeric Ids")
	ONCOMMAND("IsNidOcc",CResourceManager::HandleTerminalIsNidOccupied,"Test if the given numeric Id is occupied")
	ONCOMMAND("ShowCap",CResourceManager::HandleTerminalShowCapacity,"Show resources capacity")
	ONCOMMAND("SendNewHWInd",CResourceManager::HandleTerminalSendNewHWInd,"Send new HW indication")
	ONCOMMAND("SetCPUsize",CResourceManager::HandleTerminalSetCPUSize,"Set CPU capacity")
	ONCOMMAND("GetCPUsize",CResourceManager::HandleTerminalGetCPUSize,"Get CPU capacity")

	// ISDN
	ONCOMMAND("AllocTBN",CResourceManager::HandleTerminalAllocTBN,"Allocate Temporary Bonding Number")
	ONCOMMAND("ShowTBN",CResourceManager::HandleTerminalShowcTBN,"Show Temporary Bonding Numbers")

	ONCOMMAND("IsChIdAlloc",CResourceManager::HandleTerminalIsChannelIdAllocateded,"Test if the given channel Id is allocated")
	ONCOMMAND("PrintRes",CResourceManager::HandleTerminalPrintRes,"Print Reservations")
	ONCOMMAND("PrintProfiles",CResourceManager::HandleTerminalPrintProfiles,"Print Profiles")
	ONCOMMAND("StartRecording",CResourceManager::HandleTerminalStartRecording,"Start Recording")
	ONCOMMAND("StopRecording",CResourceManager::HandleTerminalStopAllMediaRecording,"Stop Recording")
	ONCOMMAND("PrintBoardStreamStatus",CResourceManager::HandleTerminalPrintBoardStreamStatus,"Print Board Stream Status")
	ONCOMMAND("SetPortGauge",CResourceManager::HandleSetPortGauge,"SetPortGauge")
	ONCOMMAND("PrintBrdsState",CResourceManager::HandleDumpBrdsState,"PrintBrdsState")
	ONCOMMAND("UnitRecovery",CResourceManager::HandleTerminalRecoveryUnit,"Unit Recovery")
	ONCOMMAND("ExtractPartyInfo",CResourceManager::HandleTerminalExtractPartyInfo, "Extract Party Information")
	ONCOMMAND("ExtractPartyPorts",CResourceManager::HandleTerminalExtractPartyPorts, "Extract Party Ports")
	ONCOMMAND("SetFreeDisable",CResourceManager::HandleTerminalSetFreeDisable,"Set Free Disable")
	ONCOMMAND("ShiftFutureConf",CResourceManager::HandleShiftFutureConferences,"shift time of future conferences")
	ONCOMMAND("SetConfAvcSvcMode",CResourceManager::HandleSetConfAvcSvcMode,"SetConfAvcSvcMode [Conf Type] [Conf ID]")
END_TERMINAL_COMMANDS

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalCheckConsistency(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalCheckConsistency", command);

	CSelfConsistency::Instantiate()->CheckConsistency(&answer);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalUnitFatal(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalFatalUnit", command);

	DWORD numOfParams = command.GetNumOfParams();
	if (3 != numOfParams)
	{
		answer << "error: action must be specified: FatalUnit\n";
		answer << "usage: Bin/McuCmd SimulateUnitFatal Resource [board id] [unit id] [flag]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);
	const string &unitId = command.GetToken(eCmdParam2);
	const string &e_flag = command.GetToken(eCmdParam3); // true if Fatal false if unfatal

	answer << " Board Id = " << boardId << " Unit Id = " << unitId;

	WORD b_id = atoi(boardId.c_str());
	WORD u_id = atoi(unitId.c_str());
	BYTE isFatal = (e_flag == "YES" ? TRUE : FALSE);

	UNIT_RECOVERY_S params;

	params.unit_recover.box_id = 1;
	params.unit_recover.board_id = b_id;
	params.unit_recover.sub_board_id = MFA_SUBBOARD_ID;
	params.unit_recover.unit_id = u_id;

	ExecUnitFatalOrUnFatalInd(&params, !isFatal);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalSetMaxArtChannelsPerArt(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if (1 != numOfParams)
	{
		answer << "error: action must be specified: SetMaxArtChannels\n";
		answer << "usage: Bin/McuCmd SetMaxArtChannels Resource [max art channels]\n";
		return STATUS_FAIL;
	}

	const string &strMaxArtChannels = command.GetToken(eCmdParam1);

	answer << " max art channels = " << strMaxArtChannels;

	int maxArtChannels = atoi(strMaxArtChannels.c_str());

	CUnitMFA::SetMaxArtChannelsPerArt(maxArtChannels);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalShowNumArtChannels(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if (1 != numOfParams)
	{
		answer << "error: action must be specified: ShowArtChannels\n";
		answer << "usage: Bin/McuCmd ShowArtChannels Resource [board_id]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);
	answer << " Board Id = " << boardId << "\n";
	WORD b_id = atoi(boardId.c_str());

	STATUS status;
	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst)
	{
		status = pSyst->ShowNumArtChannels(b_id, answer);
	}
	else
	{
		answer << "No system resources";
		status = STATUS_FAIL;
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalDisableUnit(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalDisableUnit", command);

	DWORD numOfParams = command.GetNumOfParams();
	if (3 != numOfParams)
	{
		answer << "error: action must be specified: DisableUnit\n";
		answer << "usage: Bin/McuCmd DisableUnit Resource [board id] [unit id] [flag]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);
	const string &unitId = command.GetToken(eCmdParam2);
	const string &e_flag = command.GetToken(eCmdParam3);

	answer << " Board Id = " << boardId << " Unit Id = " << unitId;

	WORD b_id = atoi(boardId.c_str());
	WORD u_id = atoi(unitId.c_str());
	BYTE enbl = (e_flag == "NO" ? TRUE : FALSE);

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst)
	{
		STATUS status = pSyst->SetUnitMfaStatus(b_id, u_id, enbl, TRUE);
		if (STATUS_OK != status)
		{
			answer << "error: unit not found!";
		}
	}

	CheckResourceEnoughAndAddOrRemoveAciveAlarm();
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalSetFreeDisable(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalSetFreeDisable", command);

	DWORD numOfParams = command.GetNumOfParams();
	if (3 != numOfParams)
	{
		answer << "error: action must be specified: SetFreeDisable\n";
		answer << "usage: Bin/McuCmd SetFreeDisable Resource [board id] [unit id] [port_id]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);
	const string &unitId = command.GetToken(eCmdParam2);
	const string &portId = command.GetToken(eCmdParam3);

	answer << " Board Id = " << boardId << " Unit Id = " << unitId << " Port Id = " << portId << " will set as PORT_FREE_DISABLE\n";

	WORD b_id = atoi(boardId.c_str());
	WORD u_id = atoi(unitId.c_str());
	WORD p_id = atoi(portId.c_str());

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst == NULL)
	{
		answer << "error: pSyst is NULL\n";
		return STATUS_FAIL;
	}

	CUnitMFA* pUnitMFA = pSyst->GetUnit(b_id, u_id);
	if (pUnitMFA == NULL)
	{
		answer << "error: unit not found!\n";
		return STATUS_FAIL;
	}

	pUnitMFA->SetFreePortsToFreeDisable(p_id);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalDisableBoard(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalDisableBoard", command);

	DWORD numOfParams = command.GetNumOfParams();
	if (2 != numOfParams)
	{
		answer << "error: action must be specified: DisableBoard\n";
		answer << "usage: Bin/McuCmd DisableBoard Resource [board id] [flag]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);
	const string &e_flag = command.GetToken(eCmdParam2);

	answer << " Board Id = " << boardId;

	WORD b_id = atoi(boardId.c_str());
	BYTE enbl = (e_flag == "NO" ? TRUE : FALSE);

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst)
	{
		STATUS status = pSyst->SetBoardMfaStatus(b_id, enbl, TRUE);
		if (STATUS_OK != status)
		{
			answer << "error: command is failed!";
		}
	}
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalRemoveBoard(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalRemoveBoard", command);

	DWORD numOfParams = command.GetNumOfParams();
	if (2 != numOfParams)
	{
		answer << "error: action must be specified: RemoveBoard\n";
		answer << "usage: Bin/McuCmd RemoveBoard Resource [board id] [sub board id]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);
	const string &subboardid = command.GetToken(eCmdParam2);

	answer << " Board Id = " << boardId;
	answer << " sub Board Id = " << subboardid;

	WORD b_id = atoi(boardId.c_str());
	WORD subb_id = atoi(subboardid.c_str());

	CARD_REMOVED_IND_S card_removed_ind_s;
	card_removed_ind_s.BoardID = b_id;
	card_removed_ind_s.SubBoardID = subb_id;

	CardRemovedRequest(&card_removed_ind_s);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalSendNewHWInd(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalSendNewHWInd", command);

	DWORD numOfParams = command.GetNumOfParams();
	if (1 != numOfParams)
	{
		answer << "error: action must be specified: SendNewHWInd\n";
		answer << "usage: Bin/McuCmd SendNewHWInd Resource [board id] \n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);
	answer << " Board Id = " << boardId;

	WORD b_id = atoi(boardId.c_str());

	CManagerApi confpartyManagerApi(eProcessConfParty);
	BOARD_ID_IND_PARAMS_S* pData = new BOARD_ID_IND_PARAMS_S;
	pData->board_id = b_id;
	CSegment* pRetPar = new CSegment();
	pRetPar->Put((BYTE*)pData, sizeof(BOARD_ID_IND_PARAMS_S));
	confpartyManagerApi.SendMsg(pRetPar, HW_NEW_IND);
	delete pData;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalSetCPUSize(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalSetCPUSize", command);

	DWORD numOfParams = command.GetNumOfParams();
	if (1 != numOfParams)
	{
		answer << "error: action must be specified: \n";
		answer << "usage: Bin/McuCmd SetCPUsize Resource [capacity] \n";
		return STATUS_FAIL;
	}

	const string &capacity = command.GetToken(eCmdParam1);
	answer << " num capacity = " << capacity;

	DWORD num_capacity = atoi(capacity.c_str());

	CSystemResources* pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst)
		pSyst->SetCpuSize(num_capacity); // pSyst->SetCpuSize(eSystemCPUSize_illegal, num_capacity);

	CheckResourceEnoughAndAddOrRemoveAciveAlarm();
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalGetCPUSize(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalGetCPUSize", command);

	DWORD numOfParams = command.GetNumOfParams();
	if (0 != numOfParams)
	{
		answer << "error: action must be specified: \n";
		answer << "usage: Bin/McuCmd GetCPUsize Resource \n";
		return STATUS_FAIL;
	}

	CSystemResources* pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst)
	{
		DWORD num = pSyst->GetMcuCapacity();
		answer << num;
	}
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalShowMasterAC(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalShowMasterAC", command);

	STATUS status = STATUS_OK;

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
		answer << "Can't retrieve Master Audio Controller at this stage";
	else
	{
		if (pSyst)
			answer << "Master Audio Controller is on board : " << pSyst->GetAudioCntrlMasterBid();
		else
		{
			answer << "Error while retreiving the Master Audio Controller";
			status = STATUS_FAIL;
		}
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalShowSlaveAC(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalShowSlaveAC", command);

	STATUS status = STATUS_OK;

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();

	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
		answer << "Can't retrieve Slave Audio Controller at this stage";
	else
	{
		if (pSyst)
		{
			eProductType prodType = pSyst->GetProductType();
			if (prodType != eProductTypeRMX2000 && prodType != eProductTypeCallGenerator)
			{
				answer << "Slave AC is configured only in RMX2000 mode";
				status = STATUS_FAIL;
			}
			else
				answer << "Slave Audio Controller is on board : " << pSyst->GetAudioCntrlSlaveBid();
		}
		else
		{
			answer << "Error while retreiving the Slave Audio Controller";
			status = STATUS_FAIL;
		}
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalShowResevedAC(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalShowResevedAC", command);

	STATUS status = STATUS_OK;

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();

	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
		answer << "Can't retrieve Reserved Audio Controllers at this stage";
	else
	{
		if (pSyst)
		{
			eProductType prodType = pSyst->GetProductType();
			if (prodType != eProductTypeRMX4000)
			{
				answer << "Reserved AC are configured only in RMX4000 mode";
				status = STATUS_FAIL;
			}
			else
			{
				CBoard* pBoard = NULL;
				ECntrlType ac_type = E_NORMAL;
				answer << "Reserved Audio Controllers are configured on boards : ";
				for (WORD i = 0; i < BOARDS_NUM; i++)
				{
					pBoard = pSyst->GetBoard(i + 1);
					if (pBoard)
					{
						ac_type = pBoard->GetACType();
						if (E_AC_RESERVED == ac_type)
							answer << "\nBoard Id = " << i + 1;
					}
				}
			}
		}
		else
		{
			answer << "Error while retreiving the Slave Audio Controller";
			status = STATUS_FAIL;
		}
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalMoveMasterAC(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalMoveMasterAC", command);
	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	WORD masterBId = pSyst ? pSyst->GetAudioCntrlMasterBid() : 0;

	MoveMasterAC();

	WORD new_masterBId = pSyst ? pSyst->GetAudioCntrlMasterBid() : 0;
	answer << "Master Audio Controller was on board : " << masterBId << " ==> moved to the board : " << new_masterBId;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalRestoreMasterAC(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalRestoreMasterAC", command);
	DWORD numOfParams = command.GetNumOfParams();
	if (1 != numOfParams)
	{
		answer << "error: action must be specified: \n";
		answer << "usage: Bin/McuCmd RestoreMasterAC Resource [board id]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);

	answer << " Board Id = " << boardId;

	WORD b_id = atoi(boardId.c_str());

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	CBoard* pBoard = pSyst ? pSyst->GetBoard(b_id) : NULL;
	if (pSyst && pBoard)
	{
		WORD u_id = pBoard->GetAudioControllerUnitId();
		answer << " Unit Id = " << u_id;

		STATUS status = (NO_AC_ID == u_id) ? STATUS_FAIL : pSyst->SetUnitMfaStatus(b_id, u_id, TRUE);
		if (STATUS_OK != status)
			answer << "error: unit not found!";
		else
			MoveMasterAC(b_id, eAC_Back);
	}

	CheckResourceEnoughAndAddOrRemoveAciveAlarm(TRUE, TRUE);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalShowIvrCntrlr(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalShowIvrCntrlr", command);

	STATUS status = STATUS_OK;

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();

	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
		answer << "Can't retrieve IVR Controller at this stage";
	else
	{
		if (pSyst)
			answer << "IVR Controller is on board : " << pSyst->GetIvrCntrlBid();
		else
		{
			answer << "Error while retreiving the Slave Audio Controller";
			status = STATUS_FAIL;
		}
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalShowSharedMemory(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalShowSharedMemory", command);

	STATUS status = STATUS_OK;

	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
		answer << "Can't retrieve Shared Memory Content at this stage";
	else
	{
		if (m_pConnToCardManager)
			answer << *m_pConnToCardManager;
		else
		{
			answer << "Error while retrieving the Shared Memory Content";
			status = STATUS_FAIL;
		}
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalShowSlotTable(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalShowSlotTable", command);

	STATUS status = STATUS_OK;
	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst)
	{
		answer << "BoardId \tSubBoardId \tDisplayBoardId\n" << "------- \t---------- \t--------------\n";

		SLOTS_NUMBERING_CONVERSION_TABLE_S* pSlotNumberingConversionTable = pSyst->GetSlotNumberingConversionTable();
		for (int i = 0; i < (int)(pSlotNumberingConversionTable->numOfBoardsInTable); i++)
		{
			answer << pSlotNumberingConversionTable->conversionTable[i].boardId << "        \t" << pSlotNumberingConversionTable->conversionTable[i].subBoardId << "             \t" << pSlotNumberingConversionTable->conversionTable[i].displayBoardId << '\n';
		}
	}
	else
	{
		answer << "Error while retrieving the Slot table (pSyst is NULL)";
		status = STATUS_FAIL;
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalPrintRes(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoHigh, " CResourceManager::HandleTerminalPrintRes");

	STATUS status = STATUS_OK;

	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
		answer << "Can't retrieve reservation list content at this stage";
	else
	{
		CRsrvDB* pCRsrvDB = CHelperFuncs::GetRsrvDB();
		if (pCRsrvDB)
			answer << *(pCRsrvDB);
		else
		{
			answer << "Error while retrieving the reservation db";
			status = STATUS_FAIL;
		}
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalPrintProfiles(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoHigh, " CResourceManager::HandleTerminalPrintProfiles ");

	STATUS status = STATUS_OK;

	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
		answer << "Can't retrieve profiles list content at this stage";
	else
	{
		CProfilesDB* pProfilesDB = CHelperFuncs::GetProfilesDB();
		if (pProfilesDB)
			answer << *(pProfilesDB);
		else
		{
			answer << "Error while retreiving the profiles db";
			status = STATUS_FAIL;
		}
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalPrintBoardStreamStatus(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoHigh, " CResourceManager::HandleTerminalPrintBoardStatus ");
	DWORD numOfParams = command.GetNumOfParams();
	if (1 != numOfParams)
	{
		answer << "error: action must be specified: \n";
		answer << "usage: Bin/McuCmd PrintBoardStreamStatus Resource [board id]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);

	answer << " Board Id = " << boardId;

	WORD b_id = atoi(boardId.c_str());

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	CBoard* pBoard = pSyst ? pSyst->GetBoard(b_id) : NULL;
	if (pBoard)
	{
		WORD numPartiesWithVideoOnThisBoard = pBoard->GetNumVideoParticipantsWithVideoOnThisBoard();
		answer << " numVideoPartiesWithVideoOnThisBoard = " << numPartiesWithVideoOnThisBoard;
		WORD numPartiesWithArtOnThisBoard = pBoard->GetNumVideoParticipantsWithArtOnThisBoard();
		answer << " numVideoPartiesWithArtOnThisBoard = " << numPartiesWithArtOnThisBoard;
	}
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalShowPQ(CTerminalCommand& command, std::ostream& answer)
{
	STATUS status = STATUS_FAIL;
	const DWORD numOfParams = command.GetNumOfParams();

	struct Param
	{
		const char* name;
		WORD value;
	};

	Param params[] = {
		{ "Service ID", 0 },
		{ "Sub-Service ID", 0 },
		{ "Box ID", 1 },
		{ "Board ID", 0 },
		{ "Sub-Board ID", 1 },
		{ "Unit ID", 0 }, };

	if (sizeof(params) / sizeof(params[0]) != numOfParams)
	{
		answer << "error: action must be specified: ShowPQ\n";
		answer << "usage: Bin/McuCmd ShowPQ Resource";

		for (size_t i = 0; i < sizeof(params) / sizeof(params[0]); ++i)
		{
			answer << " <" << params[i].name << ">";
		}

		answer << "\n";
		return status;
	}

	for (size_t i = 0; i < sizeof(params) / sizeof(params[0]); ++i)
	{
		params[i].value = atoi(command.GetToken(static_cast<eCommandParamsIndex>(eCmdParam1 + i)).c_str());
	}

	switch (CHelperFuncs::GetProcessStatus())
	{
		case eProcessStartup:
		case eProcessIdle:
			answer << "Can't retrieve information at this stage";
			break;

		default:
			const CSystemResources* pSyst = CHelperFuncs::GetSystemResources();
			if (pSyst)
			{
				const CPQperSrvResource* pPQM = pSyst->ARTtoPQM(params[0].value, params[1].value, params[2].value, params[3].value, params[4].value, params[5].value);

				if (pPQM)
				{
					answer << *pPQM;
					status = STATUS_OK;
				}
				else
					answer << "Error while retrieving the PQ information (pPQM)";
			}
			else
				answer << "Error while retrieving the PQ information (pSyst)";
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalNidCount(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalNidCount", command);

	STATUS status = STATUS_OK;

	CReservator* pReservator = CHelperFuncs::GetReservator();

	DWORD numOfParams = command.GetNumOfParams();
	if (0 != numOfParams && 1 != numOfParams)
	{
		answer << "error: action must be specified: NidCount\n";
		answer << "usage: Bin/McuCmd NidCount Resource [length]\n";
		return STATUS_FAIL;
	}

	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
	{
		answer << "Can't retrieve information at this stage";
		status = STATUS_FAIL;
	}
	else
	{
		if (pReservator)
		{
			if (1 == numOfParams)
			{
				const string &lengthStr = command.GetToken(eCmdParam1);
				WORD length = atoi(lengthStr.c_str());

				if (0 != length)
				{
					size_t count = pReservator->NumericIdCount(length);
					answer << "Number of occupied Numeric Ids of length " << length << " = " << count;
				}
				else
					answer << "Illegal parameter: " << length;
			}
			else
			{
				size_t count = pReservator->NumericIdCount();
				answer << "Number of occupied Numeric Ids = " << count;
			}
		}
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalIsNidOccupied(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalIsNidOccupied", command);

	STATUS status = STATUS_OK;

	CReservator* pReservator = CHelperFuncs::GetReservator();

	DWORD numOfParams = command.GetNumOfParams();
	if (1 != numOfParams)
	{
		answer << "error: action must be specified: IsNidOcc\n";
		answer << "usage: Bin/McuCmd IsNidOcc Resource [numeric id]\n";
		return STATUS_FAIL;
	}

	const string &numId = command.GetToken(eCmdParam1);

	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
	{
		answer << "Can't retrieve information at this stage";
		status = STATUS_FAIL;
	}
	else
	{
		if (pReservator)
		{
			BYTE l_bIsNidExist = pReservator->NumericIdExist(numId.c_str());
			if (l_bIsNidExist)
				answer << "Numeric Id : " << numId << " is occupied";
			else
				answer << "Numeric Id : " << numId << " is free";
		}
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalShowCapacity(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalShowCapacity", command);

	STATUS status = STATUS_OK;

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();

	DWORD numOfParams = command.GetNumOfParams();
	if (numOfParams < 2)
	{
		answer << "error: action must be specified: ShowCap\n";
		answer << "usage: Bin/McuCmd ShowCap Resource [board id] [sub board id] [unit id - optional]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);
	const string &subboardid = command.GetToken(eCmdParam2);
	const string &unitId = command.GetToken(eCmdParam3);

	WORD u_id = 0xFFFF;

	if (strcmp(unitId.c_str(), "Invalide Token"))
	{
		u_id = atoi(unitId.c_str());
	}

	WORD b_id = atoi(boardId.c_str());
	WORD subb_id = atoi(subboardid.c_str());

	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
	{
		answer << "Can't retrieve information at this stage";
		status = STATUS_FAIL;
	}
	else
	{
		if (pSyst)
		{
			BYTE l_bIsRtm = (MFA_SUBBOARD_ID == subb_id ? FALSE : TRUE);

			WORD cap = pSyst->GetCapacity(b_id, subb_id, u_id);

			char str[10];
			if (cap < 10 /* str array size */)
			{
				return STATUS_FAIL;
			}

			snprintf(str, sizeof(str), "%d", cap);
			answer << "Total capacity is : " << str;

			if (l_bIsRtm)
				answer << " (Parties)";
			else
				answer << " (Promil)";
		}
		else
		{
			answer << "Internal Error";
			status = STATUS_FAIL;
		}
	}
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalAllocTBN(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalAllocTBN", command);

	STATUS status = STATUS_OK;

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();

	const string &serviceName = command.GetToken(eCmdParam1);
	const string &str_monitor_conf_id = command.GetToken(eCmdParam2);
	DWORD monitor_conf_id = atoi(str_monitor_conf_id.c_str());
	const string &str_monitor_party_id = command.GetToken(eCmdParam3);
	DWORD monitor_party_id = atoi(str_monitor_party_id.c_str());
	const string &str_is_alloc = command.GetToken(eCmdParam4);
	BYTE is_alloc = (((str_is_alloc == "YES") || (str_is_alloc == "Yes") || (str_is_alloc == "Y")) ? TRUE : FALSE);
	const string &str_phone_to_dealloc = command.GetToken(eCmdParam5);

	DWORD numOfParams = command.GetNumOfParams();
	if ((is_alloc && 4 != numOfParams) || (!is_alloc && 5 != numOfParams))
	{
		answer << "error: action must be specified: AllocTBN\n";
		answer << "usage: Bin/McuCmd AllocTBN Resource service_name monitor_conf_id monitor_party_id [phone_to_deallocate] YES (Alloc)/NO (DeAlloc)\n";
		return STATUS_FAIL;
	}

	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
		answer << "Can't retrieve information at this stage";
	else
	{
		CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
		const CConfRsrc* pConfRsrc = (NULL != pConfRsrcDB) ? pConfRsrcDB->GetConfRsrc(monitor_conf_id) : NULL;
		if (NULL != pConfRsrc)
		{
			DWORD rsrc_conf_id = pConfRsrc->GetRsrcConfId();
			const CPartyRsrc* pPartyRsrc = ((CConfRsrc*)pConfRsrc)->GetParty(monitor_party_id);
			if (NULL != pPartyRsrc)
			{
				DWORD rsrc_party_id = pPartyRsrc->GetRsrcPartyId();
				if (is_alloc)
				{
					Phone* phone = new Phone;
					memset(phone, 0, sizeof(Phone));

					status = m_pRsrcAlloc->AllocateBondingTemporaryNumber(serviceName.c_str(), rsrc_conf_id, rsrc_party_id, phone);
					if (STATUS_OK == status)
						answer << "Allocated temporary bonding phone number = " << phone->phone_number << " for Conf ID = " << monitor_conf_id << " Party ID = " << monitor_party_id << "\n";
					else
						answer << "Couldn't allocate temporary bonding phone number for Conf ID = " << monitor_conf_id << " Party ID = " << monitor_party_id << "\n";

					PDELETE(phone);
				}
				else
				{
					status = m_pRsrcAlloc->DeAllocateBondingTemporaryNumber(serviceName.c_str(), monitor_conf_id, monitor_party_id, str_phone_to_dealloc.c_str());
					if (STATUS_OK == status)
						answer << "DeAllocated temporary bonding phone number = " << str_phone_to_dealloc.c_str() << " for Conf ID = " << monitor_conf_id << " Party ID = " << monitor_party_id << "\n";
					else
						answer << "Couldn't deallocate temporary bonding phone number = " << str_phone_to_dealloc.c_str() << " for Conf ID = " << monitor_conf_id << " Party ID = " << monitor_party_id << "\n";
				}
			}
			else
			{
				answer << "Couldn't find participant in DB\n";
				status = STATUS_FAIL;
			}
		}
		else
		{
			answer << "Couldn't find conference in DB\n";
			status = STATUS_FAIL;
		}
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalShowcTBN(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalShowcTBN", command);
	STATUS status = STATUS_OK;

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();

	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
	{
		answer << "\nCan't retrieve information at this stage";
		return STATUS_FAIL;
	}

	DWORD numOfParams = command.GetNumOfParams();
	DWORD monitor_party_id = 0xFFFFFFFF, monitor_conf_id = 0xFFFFFFFF;

	switch (numOfParams)
	{
		case 2:
		{
			const string &str_monitor_party_id = command.GetToken(eCmdParam2);
			monitor_party_id = atoi(str_monitor_party_id.c_str());
		}
		/* no break */
		case 1:
		{
			const string &str_monitor_conf_id = command.GetToken(eCmdParam1);
			monitor_conf_id = atoi(str_monitor_conf_id.c_str());
		}
		/* no break */
		case 0:
		{
			CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
			if (NULL == pConfRsrcDB)
			{
				answer << "\nCouldn't retrieve Conferences Resource List\n";
				return STATUS_FAIL;
			}

			CConfRsrc* pConfRsrc = NULL;
			const std::set<CConfRsrc> *pConfRsrcsList = pConfRsrcDB->GetConfRsrcsList();

			std::set<CConfRsrc>::iterator confItr;

			BYTE wasConfFound = FALSE;

			for (confItr = pConfRsrcsList->begin(); confItr != pConfRsrcsList->end(); confItr++)
			{
				pConfRsrc = (CConfRsrc*)&(*confItr);
				if (NULL != pConfRsrc)
				{
					DWORD current_monitor_conf_id = ((CConfRsrc*)pConfRsrc)->GetMonitorConfId();
					if (monitor_conf_id != 0xFFFFFFFF && current_monitor_conf_id != monitor_conf_id)
						continue;

					wasConfFound = TRUE;

					const CPartyRsrc* pPartyRsrc = NULL;

					BONDING_PHONES* pTempBondingPhoneNumbersMap = pConfRsrc->GetTempBondingPhoneNumbersMap();
					answer << "\nTemporary Bonding Phone Number allocated for conf " << current_monitor_conf_id << " : \n" << "======================================================\n";
					for (BONDING_PHONES::iterator itr = pTempBondingPhoneNumbersMap->begin(); itr != pTempBondingPhoneNumbersMap->end(); itr++)
					{
						pPartyRsrc = ((CConfRsrc*)pConfRsrc)->GetPartyRsrcByRsrcPartyId(itr->first);
						if (NULL != pPartyRsrc)
						{
							DWORD current_monitor_party_id = ((CPartyRsrc*)pPartyRsrc)->GetMonitorPartyId();
							if (0xFFFFFFFF != monitor_party_id) // Show number for specific party
							{
								if (monitor_party_id == current_monitor_party_id)
								{
									answer << "party " << current_monitor_party_id << " : " << itr->second->phone_number << "\n";
									break;
								}
							}
							else
								answer << "party " << current_monitor_party_id << " : " << itr->second->phone_number << "\n";
						}
						else
						{
							answer << "\nError retreiving party rsrc by rsrc party id\n";
							return STATUS_FAIL;
						}
					}
					if (monitor_conf_id != 0xFFFFFFFF)
						break;
				}
				else
				{
					answer << "\nInternal Error 1\n";
					return STATUS_FAIL;
				}
			}
			if (!wasConfFound)
				answer << "\nConference wasn't found\n";
			break;	// break case
		}
		default:
		{
			answer << "error: action must be specified: ShowTBN\n";
			answer << "usage: Bin/McuCmd ShowTBN Resource [monitor_conf_id] [monitor_party_id]\n";
			answer << "If no parameters are given   - prints all numbers for all conferences\n";
			answer << "If monitor_conf_id is given  - prints for specific conference\n";
			answer << "If monitor_party_id is given - prints for specific participant in a specific conference\n";
			return STATUS_FAIL;
		}
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalIsChannelIdAllocateded(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalIsChannelIdAllocateded", command);

	STATUS status = STATUS_OK;

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();

	DWORD numOfParams = command.GetNumOfParams();
	if (2 != numOfParams)
	{
		answer << "error: action must be specified: IsChIdAlloc\n";
		answer << "usage: Bin/McuCmd IsChIdAlloc Resource [channel id] [board id]\n";
		return STATUS_FAIL;
	}

	const string &channelIdStr = command.GetToken(eCmdParam1);
	const string &str_board_id = command.GetToken(eCmdParam2);
	WORD channelId = atoi(channelIdStr.c_str());
	WORD boardId = atoi(str_board_id.c_str());

	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
	{
		answer << "Can't retrieve information at this stage";
		status = STATUS_FAIL;
	}
	else
	{
		if (pSyst)
		{
			BYTE l_bIsChannelidAllocated = pSyst->GetIsChannelIdAllocated(channelId, boardId);
			answer << "Channel Id : " << channelIdStr << " in board " << str_board_id << (l_bIsChannelidAllocated ? " is occupied" : " is free");
		}
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalReconfigureUnit(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoHigh, " CResourceManager::HandleTerminalReconfigureUnit ");

	DWORD numOfParams = command.GetNumOfParams();
	if (3 != numOfParams)
	{
		answer << "error: action must be specified: ReconfigureUnit\n";
		answer << "usage: Bin/McuCmd ReconfigureUnit Resource [board id] [unit id] [flag]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);
	const string &unitId = command.GetToken(eCmdParam2);
	const string &e_flag = command.GetToken(eCmdParam3);

	answer << " Board Id = " << boardId << " Unit Id = " << unitId << " flag = " << e_flag;

	int b_id = atoi(boardId.c_str());
	int u_id = atoi(unitId.c_str());
	eUnitType unitType = (e_flag == "ART" ? eUnitType_Art : eUnitType_Video);

	CMoveManager* pMoveManager = CHelperFuncs::GetMoveManager();
	PASSERT_AND_RETURN_VALUE(pMoveManager == NULL, STATUS_FAIL);

	STATUS status = pMoveManager->ReconfigureUnit(b_id, u_id, unitType);
	if (STATUS_OK != status)
	{
		answer << "\nFailed to reconfigure unit";
	}
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalResetPortConfiguration(CTerminalCommand & command, std::ostream& answer)
{
	OnResetPortConfiguration(NULL);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalStopAllMediaRecording(CTerminalCommand & command, std::ostream& answer)
{
	DumpTerminalCommand("HandleTerminalStopAllMediaRecording", command);
	DWORD numOfParams = command.GetNumOfParams();
	if (0 != numOfParams)
	{
		answer << "error: action must be specified: StopRecording\n";
		answer << "usage: Bin/McuCmd StopRecording Resource\n";
		return STATUS_FAIL;
	}

	STATUS status = SendStopRecordingToCm();
	PASSERT(status);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalStartRecording(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CResourceMonitor::HandleTerminalStartRecording  ");
	DumpTerminalCommand("HandleTerminalStartRecording", command);

	STATUS status = STATUS_OK;
	DWORD length = 0;

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();

	const string &FileName = command.GetToken(eCmdParam1);
	const string &str_cid = command.GetToken(eCmdParam2);
	DWORD conf_id = atoi(str_cid.c_str());
	const string &str_bid = command.GetToken(eCmdParam3);
	DWORD b_id = atoi(str_bid.c_str());
	const string &str_uid = command.GetToken(eCmdParam4);
	DWORD u_id = atoi(str_uid.c_str());
	const string &str_portid = command.GetToken(eCmdParam5);
	DWORD port_id = atoi(str_portid.c_str());
	const string &str_acceleratorid = command.GetToken(eCmdParam6);
	DWORD accelerator_id = atoi(str_acceleratorid.c_str());
	const string &str_juncid = command.GetToken(eCmdParam7);
	DWORD junc_id = atoi(str_juncid.c_str());
	const string &str_rateid = command.GetToken(eCmdParam8);
	DWORD rate_id = atoi(str_rateid.c_str());

	DWORD numOfParams = command.GetNumOfParams();
	if (7 != numOfParams)
	{
		answer << "error: action must be specified: SetJunction\n";
		answer << "usage: Bin/McuCmd StartRecording Resource file_name rsrc_conf_id board_id unit_id port_id junc_id rate_id\n";
		return STATUS_FAIL;
	}

	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
		answer << "Can't retrieve information at this stage";
	else
	{
		TStartDebugRecordingReq* pRecord = new TStartDebugRecordingReq;
		pRecord->tDebugRecordingJunction.unJunction = junc_id;
		pRecord->tDebugRecordingJunction.unRate = rate_id;
		pRecord->tDebugRecordingJunction.unMaxRecFileLen = 0xFFFFFFFF;

		if (FileName[0] != '\0')
		{
			// this is exactly what strncpy will check anyway
			strncpy(pRecord->tDebugRecordingJunction.ucFileName, FileName.c_str(), MAX_RECORDING_FILE_NAME_LENGTH - 1);
			pRecord->tDebugRecordingJunction.ucFileName[MAX_RECORDING_FILE_NAME_LENGTH - 1] = '\0';
		}
		else
		{
			PASSERT(1);
			pRecord->tDebugRecordingJunction.ucFileName[0] = '\0';
		}

		DWORD connId = 0xFFFFFFFF;	//not needed
		eResourceTypes physicalRsrcType = (eResourceTypes)1; //JunctionId2PhysicalType((WORD)junc_id);
		//fills in all needed params
		pRecord->physicalPort.physical_unit_params.box_id = 1;
		pRecord->physicalPort.physical_unit_params.board_id = b_id;
		pRecord->physicalPort.physical_unit_params.sub_board_id = 1;
		pRecord->physicalPort.physical_unit_params.unit_id = u_id;
		pRecord->physicalPort.accelerator_id = accelerator_id;
		pRecord->physicalPort.port_id = port_id;
		pRecord->physicalPort.resource_type = physicalRsrcType;

		AddDSPInfoToFileName(pRecord->tDebugRecordingJunction.ucFileName, pRecord->physicalPort.physical_unit_params.board_id, pRecord->physicalPort.physical_unit_params.unit_id);

		// building MPL protocol
		CMplMcmsProtocol *mplPrtcl = new CMplMcmsProtocol();
		mplPrtcl->AddCommonHeader(START_DEBUG_RECORDING_REQ);
		mplPrtcl->AddMessageDescriptionHeader();
		mplPrtcl->AddPhysicalHeader(1, b_id, 1); //boxId, boardId, subBoardId, unitId = 0 means sent to CM

		mplPrtcl->AddPortDescriptionHeader(DUMMY_PARTY_ID, conf_id, DUMMY_CONNECTION_ID);
		mplPrtcl->AddData(sizeof(TStartDebugRecordingReq), (char*)pRecord);

		CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");

		PTRACE2INT(eLevelInfoHigh, " CResourceManager::HandleTerminalStartRecording , boardId = ", b_id);

		mplPrtcl->SendMsgToMplApiCommandDispatcher();

		POBJDELETE(mplPrtcl);
		POBJDELETE(pRecord);
	}
	return status;
}


STATUS CResourceManager::HandleTerminalExtractPartyPorts(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if( numOfParams < 2 )
	{
		answer << "usage: Bin/McuCmd ExtractPartyPorts Resource [conf id] [party id]\n";
		return STATUS_FAIL;
	}
	const string&  confID = command.GetToken(eCmdParam1);
	const string& partyID = command.GetToken(eCmdParam2);

	int monitor_conf_id = atoi(confID.c_str());
	int monitor_party_id = atoi(partyID.c_str());

	STATUS ret = ExtractPartyInfo(answer, monitor_conf_id, monitor_party_id);

	return ret;
}


STATUS CResourceManager::HandleTerminalExtractPartyInfo(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if( numOfParams < 2 )
	{
		answer << "usage: Bin/McuCmd ExtractPartyInfo Resource [conf name] [party name] [optional string]\n";
		return STATUS_FAIL;
	}
	ostringstream ostr;
	const string&  confName = command.GetToken(eCmdParam1);
	const string& partyName = command.GetToken(eCmdParam2);
	ostr << "CResourceManager::HandleTerminalExtractPartyInfo : conf = " << confName << ", party = " << partyName;
	if( numOfParams > 2 ) {
	    const string& optionalStr = command.GetToken(eCmdParam3);
		ostr << " " << optionalStr;
	}
	PTRACE( eLevelInfoNormal, ostr.str().c_str() );
	//ask ConfParty to find monitor id's corresponding to the conf and party names
	DWORD confNameLen = strlen(confName.c_str());
	DWORD partyNameLen = strlen(partyName.c_str());
    CSegment* conf_id_request = new CSegment;
    CSegment retSeg;
    OPCODE   retOp;

    conf_id_request->Put( confNameLen );
	const char* confNameStr = confName.c_str();
    conf_id_request->Put( ((BYTE*) confNameStr), confNameLen);

	conf_id_request->Put( partyNameLen );
	const char* partyNameStr = partyName.c_str();
    conf_id_request->Put( ((BYTE*) partyNameStr), partyNameLen);

    CManagerApi api(eProcessConfParty);
    STATUS res = api.SendMessageSync(conf_id_request,
									 EXTRACT_PARTY_RSRC_INFO,
									 10*SECOND,
									 retOp,
									 retSeg);

    if (res == STATUS_OK && retOp == STATUS_OK)
	{
	    //look for the Conf object in DB
    	DWORD monitor_conf_id = 0, monitor_party_id = 0;
    	retSeg.Get( (BYTE*)(&monitor_conf_id), sizeof(DWORD) );
    	retSeg.Get( (BYTE*)(&monitor_party_id), sizeof(DWORD) );

    	res = ExtractPartyInfo(answer, monitor_conf_id, monitor_party_id);
	}
    else
    {
		answer << "\nusage: wrong conf name or party name! \n";
		return STATUS_FAIL;
    }
	return res;
}

STATUS CResourceManager::HandleDumpBrdsState(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoHigh, " CResourceManager::HandleDumpBrdsState ");

	STATUS status = STATUS_OK;

	DWORD numOfParams = command.GetNumOfParams();
	if(0 != numOfParams)
	{
		answer << "error: action must be specified: \n";
		answer << "usage: Bin/McuCmd PrintBrdsState Resource \n";
		return STATUS_FAIL;
	}

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst != NULL)
	{
		pSyst->DumpBrdsStateCmd(answer);
	}
	else
	{
		status = STATUS_FAIL;
	}

	return STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleSetPortGauge(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoHigh, " CResourceManager::HandleSetPortGauge ");

	STATUS status = STATUS_OK;

	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: action must be specified: \n";
		answer << "usage: Bin/McuCmd SetPortGauge Resource [portGauge]\n";
		return STATUS_FAIL;
	}

	const string &str_portGauge = command.GetToken(eCmdParam1);

	DWORD portGauge = atoi(str_portGauge.c_str());

	answer <<  " input portGauge = " << portGauge;

	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst != NULL)
	{
		pSyst->SetPortGauge(portGauge);
	}
	else
	{
		status = STATUS_FAIL;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleTerminalRecoveryUnit(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoHigh, " CResourceManager::HandleTerminalRecoveryUnit ");

	DWORD numOfParams = command.GetNumOfParams();
	if(3 != numOfParams)
	{
		answer << "error: action must be specified: ReconfigureUnit\n";
		answer << "usage: Bin/McuCmd ReconfigureUnit Resource [board id] [unit id] [flag]\n";
		return STATUS_FAIL;
	}

	const string &boardId = command.GetToken(eCmdParam1);
	const string &unitId = command.GetToken(eCmdParam2);
	const string &e_flag = command.GetToken(eCmdParam3); //recovery = YES, end_recovery = NO

	answer <<  " Board Id = " << boardId << " Unit Id = " << unitId << " flag = " <<  e_flag;

	int b_id = atoi(boardId.c_str());
	int u_id = atoi(unitId.c_str());

	UNIT_RECOVERY_S* pParams = new UNIT_RECOVERY_S;
	pParams->unit_recover.box_id = 1;
	pParams->unit_recover.board_id = b_id;
	pParams->unit_recover.sub_board_id = 1;
	pParams->unit_recover.unit_id = u_id;

	if( "YES" == e_flag ) {
		RECOVERY_REPLACEMENT_UNIT_S* pResultStruct = new RECOVERY_REPLACEMENT_UNIT_S;
		m_pRsrcAlloc->UnitRecovery(pParams, pResultStruct);

		BYTE box_id = pResultStruct->unit_replacement.box_id;
		BYTE board_id = pResultStruct->unit_replacement.board_id;
		BYTE sub_board_id = pResultStruct->unit_replacement.sub_board_id;
		BYTE unit_id = pResultStruct->unit_replacement.unit_id;
		STATUS unit_replacement_status = pResultStruct->status;

		answer << "\n after : status = " << unit_replacement_status
				<< ", board_id = " << (WORD)board_id << ", sub_board_id = " << (WORD)sub_board_id << ", replacement unit id = " << (WORD)unit_id;

		CSegment*  pReplaceParam = new CSegment;
		pReplaceParam->Put((BYTE*)pResultStruct,sizeof(RECOVERY_REPLACEMENT_UNIT_S));

		CMplMcmsProtocol *mplPrtclReplace= new CMplMcmsProtocol();
		mplPrtclReplace->AddCommonHeader(RECOVERY_REPLACEMENT_UNIT_REQ);
		mplPrtclReplace->AddMessageDescriptionHeader();
		mplPrtclReplace->AddPhysicalHeader(1, b_id, 1);
		mplPrtclReplace->AddPortDescriptionHeader( DUMMY_PARTY_ID, DUMMY_CONF_ID, DUMMY_CONNECTION_ID );

		BYTE*  pMessageResult = new BYTE[pReplaceParam->GetWrtOffset()];
		pReplaceParam->Get(pMessageResult, pReplaceParam->GetWrtOffset());
		mplPrtclReplace->AddData(pReplaceParam->GetWrtOffset(),(const char*)pMessageResult);
		PDELETEA(pMessageResult);


		CMplMcmsProtocolTracer(*mplPrtclReplace).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");
		STATUS status = mplPrtclReplace->SendMsgToMplApiCommandDispatcher();

		POBJDELETE(pReplaceParam);
		POBJDELETE(pResultStruct);
		POBJDELETE(mplPrtclReplace);
	}
	else
		m_pRsrcAlloc->UnitRecoveryEnd(pParams);

	POBJDELETE(pParams);

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////

STATUS CResourceManager::HandleShiftFutureConferences(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if( numOfParams < 2 )
	{
		answer << "usage: Bin/McuCmd ShiftFutureConferences Resource [hour] [min] [sign]\n";
		return STATUS_FAIL;
	}
	ostringstream ostr;
	const string& hour = command.GetToken(eCmdParam1);
	const string& min = command.GetToken(eCmdParam2);
	BYTE sign = TRUE;
	ostr << "CResourceManager::HandleShiftFutureConferences : hour = " << hour << ", min = " << min;
	if( numOfParams > 2 ) {
	    const string& optionalStr = command.GetToken(eCmdParam3);
		ostr << ", sign = " << optionalStr;
		sign = ( optionalStr == "YES" ? TRUE : FALSE );
	}
	PTRACE( eLevelInfoNormal, ostr.str().c_str() );
	DWORD hour_shift = atoi(hour.c_str());
	DWORD min_shift = atoi(min.c_str());

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if( pReservator )
		pReservator->ShiftStartTimeAndRereserve( hour_shift, min_shift, sign );

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CResourceManager::HandleSetConfAvcSvcMode(CTerminalCommand & command, std::ostream& answer)
{

  DWORD numOfParams = command.GetNumOfParams();
  if(2 != numOfParams){
    answer << "error: wrong number of params: SetConfAvcSvcMode\n";
    answer << "usage: Bin/McuCmd SetConfAvcSvcMode [Conf Type] [Monitor Conf ID]\n";
    return STATUS_FAIL;
  }

  const string &confType = command.GetToken(eCmdParam1);
  const string &confId = command.GetToken(eCmdParam2);

  if(0!=strncmp(confType.c_str(),"MIX",strlen("MIX"))){
    answer << " conf type " << confType.c_str() << " not handled yet";
    return STATUS_FAIL;
  }

  CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
  if(pConfRsrcDB == NULL ){
    PASSERT(1);
    answer << "failed to get pConfRsrcDB\n";
    return STATUS_FAIL;
  }

  DWORD monitorConfID = atoi(confId.c_str());
  CConfRsrc* pConfRsrc = (CConfRsrc*)pConfRsrcDB->GetConfRsrc(monitorConfID);
  if (NULL==pConfRsrc){
    answer << "conference with monitor conf id " << monitorConfID << " not found \n";
    return STATUS_FAIL;
  }

  DWORD resourceConfID = pConfRsrc->GetRsrcConfId();
  m_pRsrcAlloc->UpgradePartiesToAvcSvcMix(resourceConfID,TRUE,TRUE);

  answer << "conference with monitor conf id " << monitorConfID << " upgraded to mix AVC-SVC \n";
  return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CResourceManager::DumpTerminalCommand(char* functionname, CTerminalCommand & command)
{
	DWORD numOfParams = command.GetNumOfParams();

	CMedString *pStrReq = new CMedString;
	*pStrReq <<"CResourceManager::" << functionname << ". DumpTerminalCommand: "
		<< "numOfParams	   =   "<< numOfParams <<'\n';
	for(DWORD i=0; i<numOfParams; i++)
	{
		const string &param = command.GetToken((eCommandParamsIndex)(eCmdParam1+i));
		*pStrReq << " param" << i+1  << "="<< param;
	}
	PTRACE(eLevelInfoHigh, pStrReq->GetString());
	POBJDELETE(pStrReq);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::ExtractPartyInfo(std::ostream& answer, DWORD monitor_conf_id, DWORD monitor_party_id)
{
	TRACEINTO << " CResourceManager::ExtractPartyInfo : conf id = " <<  monitor_conf_id << ", party id = " << monitor_party_id;
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if(pConfRsrcDB)
	{
		if(pConfRsrcDB->IsExitingConf(monitor_conf_id) == FALSE ) // while spreading it has to be done, excluded EQ
		{
			TRACEINTO << " CResourceManager::ExtractPartyInfo : conference doesn't exist! ";
			return STATUS_FAIL;
		}
	}
	else
	{
		TRACEINTO << " CResourceManager::ExtractPartyInfo : pConfRsrcDB NULL! ";
		return STATUS_FAIL;
	}
	DWORD rsrcConfId = pConfRsrcDB->MonitorToRsrcConfId(monitor_conf_id);
	CConfRsrc* pConfRsrc = (CConfRsrc*)( pConfRsrcDB ? pConfRsrcDB->GetConfRsrcByRsrcConfId( rsrcConfId ) : NULL );

	//look for the Party object in the Conf
	const CPartyRsrc* pPartyRsrc = pConfRsrc ? pConfRsrc->GetParty(monitor_party_id) : NULL;
	if( !pPartyRsrc )//party doesn't exist, error
	{
		TRACEINTO << " CResourceManager::ExtractPartyInfo : party doesn't exist! ";
		return STATUS_FAIL;
	}
	DWORD rsrcPartyId = pPartyRsrc->GetRsrcPartyId();
	answer << "\n---------------------------------------------------------------------------------";
	answer << "\n monitor conf id = " << monitor_conf_id << ", rsrc conf id = " << rsrcConfId << ", monitor party id = " << monitor_party_id << ", rsrc party id = " << rsrcPartyId;
	answer << "\n---------------------------------------------------------------------------------";
	WORD bId = 0, uId = 0, portId = 0, connId = 0;
	CRsrcDesc** pRsrcDescArray = new CRsrcDesc*[MAX_NUM_ALLOCATED_RSRCS];
	for ( int i = 0; i < MAX_NUM_ALLOCATED_RSRCS; i++ )
		pRsrcDescArray[i] = NULL;

	// Get all resource descriptors per a given party
	pConfRsrc->GetDescArrayPerResourceTypeByRsrcId( rsrcPartyId, eLogical_res_none, pRsrcDescArray, MAX_NUM_ALLOCATED_RSRCS );
	for(int j=0; j<MAX_NUM_ALLOCATED_RSRCS; j++)
	{
		if( pRsrcDescArray[j] == NULL )
			break;
		connId = pRsrcDescArray[j]->GetConnId();
		bId    = pRsrcDescArray[j]->GetBoardId();
		uId    = pRsrcDescArray[j]->GetUnitId();
		portId = pRsrcDescArray[j]->GetFirstPortId();
		answer << "\n" << pRsrcDescArray[j]->GetType()
			   << ":\t connId = " << connId << ", board = " << bId << ", unit = " << uId << ", port = " << portId;
	}
	delete []pRsrcDescArray;
	answer << "\n---------------------------------------------------------------------------------";
	return STATUS_OK;
}
