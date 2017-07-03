#include "TraceStream.h"
#include "CardResourceConfig.h"
#include "HelperFuncs.h"
#include "InternalProcessStatuses.h"
#include "Trace.h"
#include "SysConfig.h"

extern char* CardTypeToString(APIU32 cardType);

WORD CCardResourceConfig::m_num_aud_ports_left_to_config = 0;
WORD CCardResourceConfig::m_num_vid_hd720_ports_left_to_config = 0;

float CCardResourceConfigBreeze::m_configProportion = 0;
float CCardResourceConfigMpmRx::m_configProportion = 0;
float CCardResourceConfigSoft::m_configProportion = 0;
float CCardResourceConfigNinja::m_configProportion = 0;

////////////////////////////////////////////////////////////////////////////
//                        CCardResourceConfig
////////////////////////////////////////////////////////////////////////////
CCardResourceConfig::CCardResourceConfig(eCardType cardType)
{
	m_pSystemResources = CHelperFuncs::GetSystemResources();

	m_num_Needed_ART_prml = 0;
	m_num_Needed_VID_prml = 0;

	m_num_configured_ARTs = 0;
	m_num_configured_VIDEOs = 0;

	m_num_configured_ARTs_Other_Cards = 0;
	m_num_configured_VIDEOs_Other_Cards = 0;

	m_cardType = cardType;
}

////////////////////////////////////////////////////////////////////////////
void CCardResourceConfig::SetNumConfiguredUnits(DWORD num_configured_ARTs, DWORD num_configured_VIDEOs)
{
	m_num_configured_ARTs_Other_Cards = num_configured_ARTs;
	m_num_configured_VIDEOs_Other_Cards = num_configured_VIDEOs;
}

////////////////////////////////////////////////////////////////////////////
eUnitType CCardResourceConfig::DecideWhatToConfigureNext()
{
	if (m_cardType == eMpmRx_Half || m_cardType == eMpmRx_Full)
	{
		if (m_num_configured_ARTs < 15)
			return eUnitType_Art;
		else
			return eUnitType_Video;
	}

	if (m_cardType == eMpmRx_Ninja)
	{
		// There can be at most VIDEO_UNIT_START_NUMBER_NINJA - 1 ARTs
		if (m_num_configured_ARTs < (VIDEO_UNIT_START_NUMBER_NINJA - 1))
			return eUnitType_Art;
		else
			return eUnitType_Video;
	}

	//if no ART yet, start with an ART
	if (m_num_configured_ARTs == 0 && !CHelperFuncs::IsSoftMCU(CProcessBase::GetProcess()->GetProductType()))
		return eUnitType_Art;

	if ((m_num_configured_ARTs < 10 && m_cardType == eMpmx_80) || (m_num_configured_ARTs < 5 && m_cardType == eMpmx_40) || (m_num_configured_ARTs < 2 && m_cardType == eMpmx_20))
	{
		//full Breeze cards should always have 8 art units + 2
		return eUnitType_Art;
	}
	if (m_cardType == eMpmx_Soft_Full || m_cardType == eMpmx_Soft_Half)  //OLGA - SoftMCU
	{
		if (m_num_configured_VIDEOs < 1)
			return eUnitType_Video;
		return eUnitType_Art;
	}

	float proportionIfAddingArt = ((float)(m_num_configured_VIDEOs + m_num_configured_VIDEOs_Other_Cards)) / (m_num_configured_ARTs + m_num_configured_ARTs_Other_Cards + 1);
	float proportionIfAddingVideo = ((float)(m_num_configured_VIDEOs + m_num_configured_VIDEOs_Other_Cards + 1)) / (m_num_configured_ARTs + m_num_configured_ARTs_Other_Cards);

	//check what is best to add, one audio or one video, what brings us closer to the configProportion
	if (abs(proportionIfAddingArt - GetConfigProportion()) <= abs(proportionIfAddingVideo - GetConfigProportion()))
	{
		return eUnitType_Art;
	}
	else
	{
		return eUnitType_Video;
	}
}

////////////////////////////////////////////////////////////////////////////
void CCardResourceConfig::DivideArtAndVideoUnitsAccordingToProportion(DWORD numUnits, DWORD& numArtUnitsPerBoard, DWORD& numVideoUnitsPerBoard)
{
	numArtUnitsPerBoard = 0;
	numVideoUnitsPerBoard = 0;

	eUnitType unitType;
	for (DWORD i = 0; i < numUnits; i++)
	{
		if (i == (numUnits - 1) && GetConfigProportion() != 0 &&   //VNGR-15563
		    (m_num_configured_VIDEOs + m_num_configured_VIDEOs_Other_Cards) == 0)
		{
			PTRACE(eLevelInfoNormal, "CCardResourceConfig::DivideArtAndVideoUnitsAccordingToProportion - Need to config one video unit at least");
			m_num_configured_VIDEOs++;
			continue;
		}
		unitType = DecideWhatToConfigureNext();
		if (unitType == eUnitType_Art)
		{
			m_num_configured_ARTs++;
		}
		else
		{
			m_num_configured_VIDEOs++;
		}
	}
	numArtUnitsPerBoard = m_num_configured_ARTs;
	numVideoUnitsPerBoard = m_num_configured_VIDEOs;
	PASSERT(numArtUnitsPerBoard + numVideoUnitsPerBoard != numUnits);
}


////////////////////////////////////////////////////////////////////////////
//                        CCardResourceConfigBreeze
////////////////////////////////////////////////////////////////////////////
STATUS CCardResourceConfigBreeze::FindConfigureVideoUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type)
{
	STATUS status = STATUS_INSUFFICIENT_VIDEO_RSRC;
	bool isController = false;
	BoardID boardId = -1;
	UnitID unitId = -1;

	#define PARAMS "BoardId:" << boardId << ", UnitId:" << unitId << ", IsController:" << (int)isController

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		if ((eDsp != pConfigParams[i].unitType) || (eNotExist == pConfigParams[i].status))
			continue;

		if (pRetConfigParams->unitsParamsList[i].type != eNotConfigured) //already configured
			continue;

		boardId = pConfigParams[i].physicalHeader.board_id;
		unitId  = pConfigParams[i].physicalHeader.unit_id;

		if (ShouldBeARTUnit(unitId)) //arts units
			continue;

		status = m_pSystemResources->EnableUnit(boardId, unitId, eUnitType_Video, isController);

		if (status == STATUS_OK)
		{
			pRetConfigParams->unitsParamsList[i].type = eVideo;
			pRetConfigParams->unitsParamsList[i].pqNumber = pConfigParams->pqNumber;

			m_num_Needed_VID_prml = (m_num_Needed_VID_prml < 1000) ? 0 : m_num_Needed_VID_prml - 1000;
			break;
		}
	}

	// update num VIDEOs configured in system
	if (status == STATUS_OK)
	{
		m_num_configured_VIDEOs++;
		TRACEINTO << PARAMS << " - Found";
	}
	else
	{
		if (STATUS_INSUFFICIENT_VIDEO_RSRC == status)
			TRACEINTOLVLERR << PARAMS << " - Failed, Status:STATUS_INSUFFICIENT_VIDEO_RSRC";
		else
			TRACEINTOLVLERR << PARAMS << " - Failed, Status:" << status;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCardResourceConfigBreeze::FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type)
{
	STATUS status = STATUS_INSUFFICIENT_ART_RSRC;
	bool isController = false;
	BoardID boardId = -1;
	UnitID unitId = -1;

	#define PARAMS "BoardId:" << boardId << ", UnitId:" << unitId << ", IsController:" << (int)isController

	//try to set the audio controller in unit 20 or 15
	// 1. look for preferred AC #1
	bool foundPreferredAC = FindConfigureArtUnitForACPreferred(pConfigParams, pRetConfigParams, AUD_CNTLER_UNIT_ID_BREEZE_PRIORITY_1, boardId, unitId, isController, status);
	// 2. look for preferred AC #2
	if (!foundPreferredAC)
	{
		foundPreferredAC = FindConfigureArtUnitForACPreferred(pConfigParams, pRetConfigParams, AUD_CNTLER_UNIT_ID_BREEZE_PRIORITY_2, boardId, unitId, isController, status);
	}
	// 3. look for ShouldBeARTUnit DSP
	if (!foundPreferredAC)
	{
		for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
		{

			if ((eDsp != pConfigParams[i].unitType) || (eNotExist == pConfigParams[i].status))
				continue;

			if (pRetConfigParams->unitsParamsList[i].type != eNotConfigured) //already configured
				continue;

			boardId = pConfigParams[i].physicalHeader.board_id;
			unitId  = pConfigParams[i].physicalHeader.unit_id;

			if (!ShouldBeARTUnit(unitId) || IsTurboVideoUnit(unitId))
				continue;

			status = m_pSystemResources->EnableUnit(boardId, unitId, eUnitType_Art, isController);
			if (status == STATUS_OK)
			{
				pRetConfigParams->unitsParamsList[i].type = (isController == TRUE) ? eArtCntlr : eArt;
				pRetConfigParams->unitsParamsList[i].pqNumber = pConfigParams->pqNumber;

				m_num_Needed_ART_prml = (m_num_Needed_ART_prml < 1000) ? 0 : m_num_Needed_ART_prml - 1000;
				break;
			}
			else
			{
				TRACEINTOLVLERR << PARAMS << " - Failed, Status:" << status;
			}
		}
	}

	if (STATUS_OK != status)
	{
		// 3. look for not turbo DSP
		status = FindConfigureArtUnit(pConfigParams, pRetConfigParams, card_type, FALSE, boardId, unitId, isController);
		if (STATUS_OK != status)
		{
			// 4. look for turbo DSP
			TRACEINTO << PARAMS << " - Find ART and use as turbo dsp";
			status = FindConfigureArtUnit(pConfigParams, pRetConfigParams, card_type, TRUE, boardId, unitId, isController);
		}
	}

	// update num ARTs configured in system
	if (status == STATUS_OK)
	{
		m_num_configured_ARTs++;
		TRACEINTO << PARAMS << " - Found";
	}
	else
	{
		if (STATUS_INSUFFICIENT_VIDEO_RSRC == status)
			TRACEINTOLVLERR << PARAMS << " - Failed, Status:STATUS_INSUFFICIENT_VIDEO_RSRC";
		else
			TRACEINTOLVLERR << PARAMS << " - Failed, Status:" << status;
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCardResourceConfigBreeze::FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type, bool useTurboUnit, BoardID& boardId, UnitID& unitId, bool& isController)
{
	STATUS status = STATUS_INSUFFICIENT_ART_RSRC;
	//	  BYTE isCntler;
	//	  WORD bId, uId;
	for (int i = 0; i < MAX_NUM_OF_UNITS; i++) // second: configure unit among not-mandatory arts (other than 13/14)
	{
		if ((eDsp != pConfigParams[i].unitType) || (eNotExist == pConfigParams[i].status))
			continue;
		if (pRetConfigParams->unitsParamsList[i].type != eNotConfigured) //already configured
			continue;

		boardId = pConfigParams[i].physicalHeader.board_id;
		unitId = pConfigParams[i].physicalHeader.unit_id;
		if (IsTurboVideoUnit(unitId) && !useTurboUnit)
		{
			TRACEINTO << " CCardResourceConfigBreeze::FindConfigureArtUnit : skip Turbo DSP , unit = " << unitId;
			continue;
		}
		else if (IsTurboVideoUnit(unitId))
			TRACEINTO << " CCardResourceConfigBreeze::FindConfigureArtUnit : use Turbo DSP , unit = " << unitId;

		// mandatory art units already checked
		status = m_pSystemResources->EnableUnit(boardId, unitId, eUnitType_Art, (bool&)isController);

		if (status == STATUS_OK)
		{
			pRetConfigParams->unitsParamsList[i].type = (isController == TRUE) ? eArtCntlr : eArt;
			pRetConfigParams->unitsParamsList[i].pqNumber = pConfigParams->pqNumber;

			m_num_Needed_ART_prml = (m_num_Needed_ART_prml < 1000) ? 0 : m_num_Needed_ART_prml - 1000;
			break;
		}
		else
		{
			TRACEINTO << " CCardResourceConfigBreeze::FindConfigureArtUnit 2 - failed to configure unit on board " << boardId << " unit " << unitId;
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCardResourceConfig::Config(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type)
{
	TRACEINTO << "CardType:" << CardTypeToString(m_cardType) << ", Card_Type:" << CardTypeToString(card_type);

	STATUS status = STATUS_OK;
	if (CHelperFuncs::IsMode2C()) //Breeze-COP
	{
		CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
		if (!pSystemResources)
			return FALSE;

		if (eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode())
		{
			//Breeze COP
			if (m_cardType == eMpmx_20)
			{
				TRACEINTO << "COP mode configuring 2 arts + 8 video";
				m_num_Needed_ART_prml = 2 * 1000;
				m_num_Needed_VID_prml = 8 * 1000; // in case of MPMX has 20 units
			}
			else if (m_cardType == eMpmx_40)
			{
				TRACEINTO << "COP mode configuring 5 arts + 15 video";
				m_num_Needed_ART_prml = 5 * 1000;
				m_num_Needed_VID_prml = 15 * 1000; // in case of MPMX has 20 units
			}
			else
			{
				TRACEINTO << "COP mode configuring 10 arts + 30 video";
				m_num_Needed_ART_prml = 10 * 1000;
				m_num_Needed_VID_prml = 30 * 1000; // in case of MPMX has 40 units
			}
		}
	}
	else if (m_cardType == eMpmRx_Full)
	{
		TRACEINTO << "Configuring 15 arts + 17 video";
		m_num_Needed_ART_prml = 15 * 1000;
		m_num_Needed_VID_prml = 17 * 3000;
	}
	else if (m_cardType == eMpmRx_Half)
	{
		TRACEINTO << "Configuring 15 arts + 5 video";
		m_num_Needed_ART_prml = 15 * 1000;
		m_num_Needed_VID_prml = 5 * 3000;
	}
	else
	{
		m_num_Needed_ART_prml = m_num_aud_ports_left_to_config * GetAudBasePrml();
		m_num_Needed_VID_prml = m_num_vid_hd720_ports_left_to_config * GetVidBasePrml();
	}

	///////////////// start - configuring according to what is needed

	eUnitType unitTypeToConfigure;

	STATUS status_vid = STATUS_OK, status_art = STATUS_OK;

	WORD is_config_cont = (m_num_Needed_ART_prml == 0 && m_num_Needed_VID_prml == 0) ? FALSE : TRUE;

	while (is_config_cont)
	{
		if (status_art != STATUS_OK || m_num_Needed_ART_prml == 0)
		{ //if we couldn't find an art unit, or no more ART needed, try a video unit
			unitTypeToConfigure = eUnitType_Video;
		}
		else if (status_vid != STATUS_OK || m_num_Needed_VID_prml == 0)
		{ //if we couldn't find a video unit, or no more VIDEO needed, try an art unit
			unitTypeToConfigure = eUnitType_Art;
		}
		else
		{ //in normal case, decide what is better
			unitTypeToConfigure = DecideWhatToConfigureNext();
		}

		CMedString mstr;
		mstr << "CCardResourceConfig::Config: \n";
		mstr << "m_num_Needed_ART_prml = " << m_num_Needed_ART_prml << "\n";
		mstr << "m_num_Needed_VID_prml = " << m_num_Needed_VID_prml << "\n";
		mstr << "m_num_configured_ARTs = " << m_num_configured_ARTs << "\n";
		mstr << "m_num_configured_VIDEOs = " << m_num_configured_VIDEOs << "\n";
		mstr << "unitTypeToConfigure = ";
		if (eUnitType_Art == unitTypeToConfigure)
		{
			mstr << "eUnitType_Art \n\n";
		}
		else if (eUnitType_Video == unitTypeToConfigure)
		{
			mstr << "eUnitType_Video \n\n";
		}
		else
		{
			mstr << unitTypeToConfigure << "\n\n";
		}
		PTRACE(eLevelInfoNormal, mstr.GetString());

		if (unitTypeToConfigure == eUnitType_Art)
		{
			status = FindConfigureArtUnit(pConfigParams, pRetConfigParams, card_type);

			if (STATUS_OK != status)
				status_art = status;
		}
		else if (unitTypeToConfigure == eUnitType_Video)
		{
			status = FindConfigureVideoUnit(pConfigParams, pRetConfigParams, card_type);

			if (STATUS_OK != status)
				status_vid = status;
		}
		else
		{
			is_config_cont = FALSE;
			PASSERT(1);
		}

		// either all configured, or type/types which still need(s) to be configured - already bad status
		if ((m_num_Needed_ART_prml == 0 || status_art != STATUS_OK) && (m_num_Needed_VID_prml == 0 || status_vid != STATUS_OK))
			is_config_cont = FALSE;
	}

	///////////////// end - configuring according to what is needed:

	if (!CHelperFuncs::IsMpmRx(m_cardType))
	{
		// START - configuring excessive units if any
		status_art = STATUS_OK;
		status_vid = STATUS_OK;
		is_config_cont = TRUE;

		while (is_config_cont)
		{
			if (status_art != STATUS_OK)
			{ //if we couldn't find an art unit, try a video unit
				unitTypeToConfigure = eUnitType_Video;
			}
			else if (status_vid != STATUS_OK)
			{ //if we couldn't find a video unit, try an art unit
				unitTypeToConfigure = eUnitType_Art;
			}
			else
			{ //in normal case, decide what is better
				unitTypeToConfigure = DecideWhatToConfigureNext();
			}

			CMedString mstr1;
			mstr1 << "CCardResourceConfig::Config - excessive units: : \n";
			mstr1 << "m_num_Needed_ART_prml = " << m_num_Needed_ART_prml << "\n";
			mstr1 << "m_num_Needed_VID_prml = " << m_num_Needed_VID_prml << "\n";
			mstr1 << "m_num_configured_ARTs = " << m_num_configured_ARTs << "\n";
			mstr1 << "m_num_configured_VIDEOs = " << m_num_configured_VIDEOs << "\n";
			mstr1 << "unitTypeToConfigure = ";
			if (eUnitType_Art == unitTypeToConfigure)
			{
				mstr1 << "eUnitType_Art \n\n";
			}
			else if (eUnitType_Video == unitTypeToConfigure)
			{
				mstr1 << "eUnitType_Video \n\n";
			}
			else
			{
				mstr1 << unitTypeToConfigure << "\n\n";
			}
			PTRACE(eLevelInfoNormal, mstr1.GetString());

			// 		  TRACEINTO << " CCardResourceConfig::Config - excessive units: " <<
			// 	                "\n m_num_Needed_ART_prml = " << m_num_Needed_ART_prml <<
			// 	                "\n m_num_Needed_VID_prml = " << m_num_Needed_VID_prml <<
			// 	                "\n m_num_configured_ARTs = " << m_num_configured_ARTs <<
			// 	                "\n m_num_configured_VIDEOs = " << m_num_configured_VIDEOs <<
			// 	                "\n unitTypeToConfigure = " << unitTypeToConfigure << "\n\n";

			if (unitTypeToConfigure == eUnitType_Art)
			{
				status = FindConfigureArtUnit(pConfigParams, pRetConfigParams, card_type);

				if (STATUS_OK != status)
					status_art = status;
			}
			else if (unitTypeToConfigure == eUnitType_Video)
			{
				status = FindConfigureVideoUnit(pConfigParams, pRetConfigParams, card_type);

				if (STATUS_OK != status)
					status_vid = status;
			}
			else
			{
				is_config_cont = FALSE;
				PASSERT(1);
			}

			// either all configured, or type/types which still need(s) to be configured - already bad status
			if (status_art != STATUS_OK && status_vid != STATUS_OK)
				is_config_cont = FALSE;
		}
		// END - configuring excessive units if any
	}

	// update num of parties left to configure
	m_num_vid_hd720_ports_left_to_config = (WORD)(m_num_Needed_VID_prml / GetVidBasePrml());
	m_num_aud_ports_left_to_config = (WORD)(m_num_Needed_ART_prml / GetAudBasePrml());

	return status;
}

////////////////////////////////////////////////////////////////////////////
BOOL CCardResourceConfigBreeze::ShouldBeARTUnit(int unitID)
{
//	if(unitID == 5  || unitID == 10 || unitID == 14 || unitID == 15 ||
//	   unitID == 25 || unitID == 30 || unitID == 34 || unitID == 35)
	if (unitID == 9 || unitID == 10 || unitID == 15 || unitID == 20 || unitID == 25 || unitID == 30 || unitID == 35 || unitID == 40)
		return TRUE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CCardResourceConfigBreeze::CanBeUsedForARTRecovery(int unitID)
{
	if (!ShouldBeARTUnit(unitID) || !CanUnitBeArtSwapRecovery(unitID))
	{	              // DSP 9 and 10 shouldn't be used as replacement units for
		return FALSE; // swap recovery because they are on the same bus (by Yuval T.)
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CCardResourceConfigBreeze::CanUnitBeArtSwapRecovery(int unitID)
{
	// DSP 9 and 10 shouldn't be used as replacement units for
	// swap recovery because they are on the same bus (by Yuval T.)
	// DSP 9 and 10 also shouldn't use swap recovery if they crashes (vngr-22438)
	if (9 == unitID || 10 == unitID)
	{
		return FALSE;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CCardResourceConfigBreeze::IsTurboVideoUnit(int unitID)
{
	if (unitID == 1 || unitID == 2 || unitID == 11 || unitID == 12 || unitID == 23 || unitID == 24 || unitID == 33 || unitID == 34)
		return TRUE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
bool CCardResourceConfigBreeze::FindConfigureArtUnitForACPreferred(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, UnitID preferredUnitId, BoardID& boardId, UnitID& unitId, bool& isController, STATUS& status)
{
	bool foundPreferredAC = false;

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++) //first: trying to configure mandatory ART units (13 & 14)
	{
		if ((eDsp != pConfigParams[i].unitType) || (eNotExist == pConfigParams[i].status))
			continue;

		if (pRetConfigParams->unitsParamsList[i].type != eNotConfigured) //already configured
			continue;

		boardId = pConfigParams[i].physicalHeader.board_id;
		unitId  = pConfigParams[i].physicalHeader.unit_id;

		if (!ShouldBeARTUnit(unitId) || IsTurboVideoUnit(unitId))
			continue;

		if (unitId != preferredUnitId)
			continue;

		status = m_pSystemResources->EnableUnit(boardId, unitId, eUnitType_Art, (bool&)isController);
		if (status == STATUS_OK)
		{
			foundPreferredAC = true;

			pRetConfigParams->unitsParamsList[i].type = (isController == TRUE) ? eArtCntlr : eArt;
			pRetConfigParams->unitsParamsList[i].pqNumber = pConfigParams->pqNumber;

			m_num_Needed_ART_prml = (m_num_Needed_ART_prml < 1000) ? 0 : m_num_Needed_ART_prml - 1000;

			TRACEINTO << "BoardId:" << boardId << ", UnitId:" << unitId << ", IsController:" << (int)isController << " - Found";
			break;
		}
		else
		{
			TRACEINTOLVLERR << "BoardId:" << boardId << ", UnitId:" << unitId << ", IsController:" << (int)isController << " - Failed to configure";
		}
	}

	return foundPreferredAC;
}

////////////////////////////////////////////////////////////////////////////
float CCardResourceConfigBreeze::GetAudBasePrml() const
{
	float artPromilsBreeze = CHelperFuncs::GetArtPromilsBreeze();
	return artPromilsBreeze;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardResourceConfigBreeze::GetNumConfiguredARTPorts() const
{
	return m_num_configured_ARTs * ((WORD)(1000 / GetAudBasePrml()));
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardResourceConfigBreeze::GetNumConfiguredVideoPorts() const
{
	return m_num_configured_VIDEOs * ((WORD)(1000 / GetVidBasePrml()));
}

////////////////////////////////////////////////////////////////////////////
STATUS CCardResourceConfigMpmRx::FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type)
{
	STATUS status = STATUS_INSUFFICIENT_ART_RSRC;
	BYTE isCntler;
	WORD bId, uId;

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		if ((eDsp != pConfigParams[i].unitType) || (eNotExist == pConfigParams[i].status))
			continue;

		if (pRetConfigParams->unitsParamsList[i].type != eNotConfigured) //already configured
			continue;

		bId = pConfigParams[i].physicalHeader.board_id;
		uId = pConfigParams[i].physicalHeader.unit_id;

		status = m_pSystemResources->EnableUnit(bId, uId, eUnitType_Art, (bool&)isCntler);

		if (status == STATUS_OK)
		{
			pRetConfigParams->unitsParamsList[i].type = (isCntler == TRUE) ? eArtCntlr : eArt;
			pRetConfigParams->unitsParamsList[i].pqNumber = pConfigParams->pqNumber;

			m_num_Needed_ART_prml = (m_num_Needed_ART_prml < 1000) ? 0 : m_num_Needed_ART_prml - 1000;
			break;
		}
		else
		{
			TRACEINTO << "CCardResourceConfigMpmRx::FindConfigureArtUnit - failed to configure unit on board " << bId << " unit " << uId;
		}
	}

	// update num ARTs configured in system
	if (status == STATUS_OK)
		m_num_configured_ARTs++;
	else
		PTRACE2INT(eLevelInfoNormal, "CCardResourceConfigMpmRx::FindConfigureArtUnit: FAILED! status: ", status);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCardResourceConfigMpmRx::FindConfigureVideoUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type)
{
	STATUS status = STATUS_INSUFFICIENT_VIDEO_RSRC;
	bool isController;
	BoardID boardId = -1;
	UnitID unitId = -1;

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		if ((eDsp != pConfigParams[i].unitType) || (eNotExist == pConfigParams[i].status))
			continue;
		if (pRetConfigParams->unitsParamsList[i].type != eNotConfigured) //already configured
			continue;

		boardId = pConfigParams[i].physicalHeader.board_id;
		unitId = pConfigParams[i].physicalHeader.unit_id;

		if (unitId <= 15) //arts units
			continue;

		status = m_pSystemResources->EnableUnit(boardId, unitId, eUnitType_Video, (bool&)isController);

		if (status == STATUS_OK)
		{
			pRetConfigParams->unitsParamsList[i].type = eVideo;
			pRetConfigParams->unitsParamsList[i].pqNumber = pConfigParams->pqNumber;

			m_num_Needed_VID_prml = (m_num_Needed_VID_prml < 3000) ? 0 : m_num_Needed_VID_prml - 3000;
			break;
		}
	}

	// update num VIDEOs configured in system
	if (status == STATUS_OK)
	{
		m_num_configured_VIDEOs++;
		TRACEINTO << "BoardId:" << boardId << ", UnitId:" << unitId << ", IsController:" << isController << " - Found";
	}
	else
	{
		if (STATUS_INSUFFICIENT_VIDEO_RSRC == status)
			TRACEINTOLVLERR << "BoardId:" << boardId << ", UnitId:" << unitId << ", IsController:" << isController << " - Failed, Status:STATUS_INSUFFICIENT_VIDEO_RSRC";
		else
			TRACEINTOLVLERR << "BoardId:" << boardId << ", UnitId:" << unitId << ", IsController:" << isController << " - Failed, Status:" << status;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardResourceConfigMpmRx::GetNumConfiguredARTPorts() const
{
	return m_num_configured_ARTs * ((WORD)(1000 / GetAudBasePrml()));
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardResourceConfigMpmRx::GetNumConfiguredVideoPorts() const
{
	return m_num_configured_VIDEOs * ((WORD)(3000 / GetVidBasePrml()));
}

////////////////////////////////////////////////////////////////////////////
float CCardResourceConfigSoft::GetAudBasePrml() const //OLGA - SoftMCU
{
	eProductType prodType = m_pSystemResources->GetProductType();
	switch (prodType)
	{
		case eProductTypeNinja:
		case eProductTypeSoftMCU:
		case eProductTypeGesher:
		case eProductTypeEdgeAxis:
			return ART_PROMILLES_SOFT_MCU;
		case eProductTypeCallGeneratorSoftMCU:
			return ART_PROMILLES_SOFT_CG;
		case eProductTypeSoftMCUMfw:
			return ART_PROMILLES_SOFT_MFW;
		default:
			PASSERT(prodType);
	}

	return ART_PROMILLES_SOFT_MCU;
}

////////////////////////////////////////////////////////////////////////////
float CCardResourceConfigSoft::GetVidBasePrml() const //OLGA - SoftMCU
{
	eProductType prodType = m_pSystemResources->GetProductType();
	switch (prodType)
	{
		case eProductTypeSoftMCU:
		case eProductTypeGesher:
		case eProductTypeEdgeAxis:
			return VID_TOTAL_HD720_PROMILLES_SOFT_MPMX;
		case eProductTypeCallGeneratorSoftMCU:
			return VID_TOTAL_HD720_30FS_PROMILLES_SOFT_CG;
		case eProductTypeSoftMCUMfw:
			return VIDEO_PROMILLES_SOFT_MFW;
		default:
			PASSERT(prodType);
	}
	return VID_TOTAL_HD720_PROMILLES_SOFT_MPMX;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCardResourceConfigSoft::FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type)
{
	STATUS status = STATUS_INSUFFICIENT_ART_RSRC;
	BYTE isCntler;
	WORD bId, uId;

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		if ((eDsp != pConfigParams[i].unitType) || (eNotExist == pConfigParams[i].status) || (pRetConfigParams->unitsParamsList[i].type != eNotConfigured)) //already configured
			continue;

		bId = pConfigParams[i].physicalHeader.board_id;
		uId = pConfigParams[i].physicalHeader.unit_id;

		status = m_pSystemResources->EnableUnit(bId, uId, eUnitType_Art, (bool&)isCntler);

		if (status == STATUS_OK)
		{
			pRetConfigParams->unitsParamsList[i].type = (isCntler == TRUE) ? eArtCntlr : eArt;
			pRetConfigParams->unitsParamsList[i].pqNumber = pConfigParams->pqNumber;

			m_num_Needed_ART_prml = (m_num_Needed_ART_prml < 1000) ? 0 : m_num_Needed_ART_prml - 1000;
			break;
		}
		else
			TRACEINTO << "CCardResourceConfigSoft::FindConfigureArtUnit - failed to configure unit on board " << bId << " unit " << uId;
	}

	// update num ARTs configured in system
	if (status == STATUS_OK)
		m_num_configured_ARTs++;
	else
		PTRACE2INT(eLevelInfoNormal, "CCardResourceConfigSoft::FindConfigureArtUnit: status: ", status);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCardResourceConfigSoft::FindConfigureVideoUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type)
{
	STATUS status = STATUS_INSUFFICIENT_VIDEO_RSRC;
	bool isController;
	BoardID boardId = -1;
	UnitID unitId = -1;

	//for(int i=0; i<MAX_NUM_OF_UNITS; i++)
	// VSGNINJA-983: For all SoftMCU family except Ninja, there will only be one video unit with number 1.
	for (int i = 1; i <= 1; i++)
	{
		if ((eDsp != pConfigParams[i].unitType) || (eNotExist == pConfigParams[i].status) || (pRetConfigParams->unitsParamsList[i].type != eNotConfigured)) //already configured
			continue;

		boardId = pConfigParams[i].physicalHeader.board_id;
		unitId = pConfigParams[i].physicalHeader.unit_id;

		status = m_pSystemResources->EnableUnit(boardId, unitId, eUnitType_Video, isController);

		if (status == STATUS_OK)
		{
			pRetConfigParams->unitsParamsList[i].type = eVideo;
			pRetConfigParams->unitsParamsList[i].pqNumber = pConfigParams->pqNumber;

			m_num_Needed_VID_prml = (m_num_Needed_VID_prml < 1000) ? 0 : m_num_Needed_VID_prml - 1000;
			break;
		}
		else
			TRACEINTO << "CCardResourceConfigSoft::FindConfigureVideoUnit - failed to configure unit on board " << boardId << " unit " << unitId;
	}
	// update num VIDEOs configured in system
	if (status == STATUS_OK)
		m_num_configured_VIDEOs++;
	else
		PTRACE2INT(eLevelInfoNormal, "CCardResourceConfigSoft::FindConfigureVideoUnit: status: ", status);

	return status;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardResourceConfigSoft::GetNumConfiguredARTPorts() const
{
	return m_num_configured_ARTs * ((WORD)(1000 / GetAudBasePrml()));
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardResourceConfigSoft::GetNumConfiguredVideoPorts() const
{
	return m_num_configured_VIDEOs * ((WORD)(1000 / GetVidBasePrml()));
}

////////////////////////////////////////////////////////////////////////////
float CCardResourceConfigNinja::GetAudBasePrml() const //OLGA - SoftMCU
{
	return ART_PROMILLES_SOFT_MCU;
}

////////////////////////////////////////////////////////////////////////////
float CCardResourceConfigNinja::GetVidBasePrml() const //OLGA - SoftMCU
{
	return VID_TOTAL_HD720_30FS_PROMILLES_MPMRX;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCardResourceConfigNinja::FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type)
{
	STATUS status = STATUS_INSUFFICIENT_ART_RSRC;
	BYTE isCntler;
	WORD bId, uId;

	for (int i = 0; i < VIDEO_UNIT_START_NUMBER_NINJA; i++)
	{
		if ((eDsp != pConfigParams[i].unitType) || (eNotExist == pConfigParams[i].status) || (pRetConfigParams->unitsParamsList[i].type != eNotConfigured)) //already configured
			continue;

		bId = pConfigParams[i].physicalHeader.board_id;
		uId = pConfigParams[i].physicalHeader.unit_id;

		status = m_pSystemResources->EnableUnit(bId, uId, eUnitType_Art, (bool&)isCntler);

		if (status == STATUS_OK)
		{
			pRetConfigParams->unitsParamsList[i].type = (isCntler == TRUE) ? eArtCntlr : eArt;
			pRetConfigParams->unitsParamsList[i].pqNumber = pConfigParams->pqNumber;

			m_num_Needed_ART_prml = (m_num_Needed_ART_prml < 1000) ? 0 : m_num_Needed_ART_prml - 1000;
			break;
		}
		else
			TRACEINTO << "failed to configure unit on board " << bId << " unit " << uId;
	}

	// update num ARTs configured in system
	if (status == STATUS_OK)
		m_num_configured_ARTs++;
	else
		TRACEINTO << "status: " << status;

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCardResourceConfigNinja::FindConfigureVideoUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type)
{
	STATUS status = STATUS_INSUFFICIENT_VIDEO_RSRC;
	BYTE isCntler;
	WORD bId, uId;

	for (int i = VIDEO_UNIT_START_NUMBER_NINJA; i < MAX_NUM_OF_UNITS; i++)
	{
		if ((eDsp != pConfigParams[i].unitType) || (eNotExist == pConfigParams[i].status) || (pRetConfigParams->unitsParamsList[i].type != eNotConfigured)) //already configured
			continue;

		bId = pConfigParams[i].physicalHeader.board_id;
		uId = pConfigParams[i].physicalHeader.unit_id;

		status = m_pSystemResources->EnableUnit(bId, uId, eUnitType_Video, (bool&)isCntler);

		if (status == STATUS_OK)
		{
			pRetConfigParams->unitsParamsList[i].type = eVideo;
			pRetConfigParams->unitsParamsList[i].pqNumber = pConfigParams->pqNumber;

			m_num_Needed_VID_prml = (m_num_Needed_VID_prml < 3000) ? 0 : m_num_Needed_VID_prml - 3000;
			break;
		}
		else
			TRACEINTO << "CCardResourceConfigSoft::FindConfigureVideoUnit - failed to configure unit on board " << bId << " unit " << uId;
	}
	// update num VIDEOs configured in system
	if (status == STATUS_OK)
		m_num_configured_VIDEOs++;
	else
		PTRACE2INT(eLevelInfoNormal, "CCardResourceConfigSoft::FindConfigureVideoUnit: status: ", status);

	return status;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardResourceConfigNinja::GetNumConfiguredARTPorts() const
{
	return m_num_configured_ARTs * ((WORD)(1000 / GetAudBasePrml()));
}

////////////////////////////////////////////////////////////////////////////
DWORD CCardResourceConfigNinja::GetNumConfiguredVideoPorts() const
{
	return m_num_configured_VIDEOs * ((WORD)(3000 / GetVidBasePrml()));
}

