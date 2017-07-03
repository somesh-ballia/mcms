#include "TraceStream.h"
#include "Board.h"
#include "SystemResources.h"
#include "HelperFuncs.h"
#include "MoveManager.h"
#include "CardResourceConfig.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ResRsrcCalculator.h"

extern char* CardTypeToString(APIU32 cardType);

////////////////////////////////////////////////////////////////////////////
//                        CBoard
////////////////////////////////////////////////////////////////////////////
CBoard::CBoard(WORD OneBasedBoardId)
{
	m_OneBasedBoardId = OneBasedBoardId;
	m_DisplayBoardId  = NO_DISPLAY_BOARD_ID;

	m_pMediaUnitslist    = new CMediaUnitsList;
	m_pSpansRTMlist      = new CSpansList;
	m_videoPreviewList   = new CVideoPreviewList;
	m_pChannelIdsList    = NULL;
	m_UtilizablePQUnitId = 0xFFFF;

	for (int i = 0; i < COND_NUM; i++)
		m_CardsStartup[i] = 0;

	m_CardType = eEmpty;

	m_AudioControllerUnitId                      = NO_AC_ID;
	m_ArtReserveRecoveryUnitId                   = NO_AC_ID;
	m_ACType                                     = E_NORMAL; // should be E_AC_MASTER or E_AC_RESERVED if all OK
	m_NumOfVideoParticipantsWithVideoOnThisBoard = 0;
	m_NumOfVideoParticipantsWithArtOnThisBoard   = 0;
	m_NumOfCOPConfOnThisBoard                    = 0;
	m_isHighUsageCPU                             = false;

	for (WORD i = 0; i < ARRAYSIZE(m_PcmMenuId); ++i)
		m_PcmMenuId[i] = DUMMY_PARTY_ID;
}

////////////////////////////////////////////////////////////////////////////
CBoard::~CBoard()
{
	m_pMediaUnitslist->clear();
	PDELETE(m_pMediaUnitslist);
	m_pMediaUnitslist = NULL;

	m_pSpansRTMlist->clear();
	PDELETE(m_pSpansRTMlist);
	m_pSpansRTMlist = NULL;

	m_videoPreviewList->clear();
	PDELETE(m_videoPreviewList);
	m_videoPreviewList = NULL;

	POBJDELETE(m_pChannelIdsList);
}

////////////////////////////////////////////////////////////////////////////
void CBoard::UnregisterStateMachine()
{
	std::set<CUnitMFA>::iterator  itr;
	std::set<CSpanRTM>::iterator itrSpan;

	for (itr = m_pMediaUnitslist->begin(); itr != m_pMediaUnitslist->end(); itr++)
	{
		CUnitMFA* pUnit = (CUnitMFA*)(&(*itr));
		pUnit->UnregisterInTask();
	}
	for (itrSpan = m_pSpansRTMlist->begin(); itrSpan != m_pSpansRTMlist->end(); itrSpan++)
	{
		CSpanRTM* pSpan = (CSpanRTM*)(&(*itrSpan));
		pSpan->UnregisterInTask();
	}
}

////////////////////////////////////////////////////////////////////////////
void CBoard::InitializeChannelsIdsList(WORD subBid)
{
	m_pChannelIdsList = new CRtmChannelIds(1, m_OneBasedBoardId, subBid);
}

////////////////////////////////////////////////////////////////////////////
void CBoard::RemoveChannelsIdsList()
{
	POBJDELETE(m_pChannelIdsList);
	m_pChannelIdsList = NULL;
}

////////////////////////////////////////////////////////////////////////////
const CUnitMFA* CBoard::GetMFA(WORD UnitId)
{
	std::set<CUnitMFA>::iterator itr;
	CUnitMFA* pUnit = NULL;
	for (itr = m_pMediaUnitslist->begin(); itr != m_pMediaUnitslist->end(); itr++)
	{
		if (itr->GetUnitId() == UnitId)
		{
			pUnit = (CUnitMFA*)(&(*itr));
			break;
		}
	}
	return pUnit;
}

////////////////////////////////////////////////////////////////////////////
const CSpanRTM* CBoard::GetRTM(WORD UnitId)
{
	CSpanRTM* pEncExistUnitRTM = new CSpanRTM(m_OneBasedBoardId, UnitId);

	std::set<CSpanRTM>::iterator i;
	i = m_pSpansRTMlist->find(*pEncExistUnitRTM);

	PDELETE( pEncExistUnitRTM );

	if (i != m_pSpansRTMlist->end())
	{
		return (&(*i));
	}
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////
float CBoard::CalculateNeededPromilles(ePortType type, DWORD artCapacity, BOOL isSvcOnly /*= FALSE*/, BOOL isAudioOnly /*= FALSE*/, eVideoPartyType videoPartyType)
{
	if (CHelperFuncs::IsSoftCard(m_CardType)) //OLGA - SoftMCU
	{
		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		eProductType productType = pSystemResources ? pSystemResources->GetProductType() : eProductTypeSoftMCU;

		switch (productType)
		{
			case eProductTypeSoftMCUMfw:
				return CSoftMfwHelper::PortTypeToPromilles(type);
			case eProductTypeCallGeneratorSoftMCU:
				return CSoftCallGeneratorHelper::PortTypeToPromilles(type);
			default:
				return CSoftMpmxHelper::PortTypeToPromilles(type);
		}
	}
	else if (CHelperFuncs::IsBreeze(m_CardType))
		return CBreezeHelper::CalculateNeededPromilles(type, artCapacity, isSvcOnly, isAudioOnly, videoPartyType);
	else if (CHelperFuncs::IsMpmRx(m_CardType))
		return CMpmRxHelper::CalculateNeededPromilles(type, artCapacity, isSvcOnly, isAudioOnly);  //For MPM-Rx, need capacity to support weighted ART
	else if (eMpmRx_Ninja == m_CardType)
		return CSoftNinjaHelper::PortTypeToPromilles(type);
	else
	{
		PASSERTSTREAM(1, "Illegal card type: " << type);
		return 0;
	}
}

////////////////////////////////////////////////////////////////////////////
WORD CBoard::CalculateNumberOfPortsPerUnit(ePortType type, DWORD artCapacity)
{
	float neededPromillesPerPort = CalculateNeededPromilles(type, artCapacity);
	PASSERT_AND_RETURN_VALUE(neededPromillesPerPort == 0, 0);

	WORD unitCapacity = (CHelperFuncs::IsMpmRxOrNinja(m_CardType) && type == PORT_VIDEO) ? 3000 : 1000;
	return (WORD)(unitCapacity / neededPromillesPerPort);
}

////////////////////////////////////////////////////////////////////////////
void CBoard::FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, ePartyRole partyRole, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode)
{
	TRACEINTO << "CardType:" << ::CardTypeToString(m_CardType);

	if (CHelperFuncs::IsSoftCard(m_CardType))
	{
		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		eProductType productType = pSystemResources ? pSystemResources->GetProductType() : eProductTypeSoftMCU;

		if (eProductTypeCallGeneratorSoftMCU == productType)
			CSoftCallGeneratorHelper::FillVideoUnitsList(videoPartyType, videoAllocData, partyRole);
		else
			CSoftMpmxHelper::FillVideoUnitsList(videoPartyType, videoAllocData, partyRole, HdVswTypeInMixAvcSvcMode);
	}
	else if (CHelperFuncs::IsBreeze(m_CardType))
		CBreezeHelper::FillVideoUnitsList(videoPartyType, videoAllocData, partyRole, HdVswTypeInMixAvcSvcMode);
	else if (CHelperFuncs::IsMpmRxOrNinja(m_CardType))
		CMpmRxHelper::FillVideoUnitsList(videoPartyType, videoAllocData, partyRole, HdVswTypeInMixAvcSvcMode);
	else
		PASSERT(1);
}

////////////////////////////////////////////////////////////////////////////
void CBoard::FillVideoUnitsListCOP(eSessionType sessionType, eLogicalResourceTypes encType, AllocData& videoAllocData)
{
	if (CHelperFuncs::IsBreeze(m_CardType))
	{
		if (eCOP_HD1080_session == sessionType || eCOP_HD720_50_session == sessionType)
		{
			CBreezeHelper::FillVideoUnitsListCOP(sessionType, encType, videoAllocData);
		}
		else if (eVSW_28_session == sessionType || eVSW_56_session == sessionType)
		{
			CBreezeHelper::FillVideoUnitsListVSW(sessionType, videoAllocData);
		}
	}
	else
	{
		PASSERT(1);
	}
}

////////////////////////////////////////////////////////////////////////////
int CBoard::ChangeUnits(int numUnitsToChange, eUnitType newUnitType, BOOL is_Enabled)
{
	TRACEINTO << "CBoard::ChangeUnits on board " << m_OneBasedBoardId << "- numUnitsToChange: " << numUnitsToChange << " newUnitType: " << newUnitType;

	PASSERT_AND_RETURN_VALUE(numUnitsToChange == 0, STATUS_FAIL);

	int numOfChangedUnits = 0;
	CMediaUnitsList::iterator unitsIterator;
	eUnitType oldUnitType;

	WORD ACUnitId = GetAudioControllerUnitId();
	DWORD idOfUnitNextToACThatCouldBeReconfigured = 0xFFFF;

	for (unitsIterator = m_pMediaUnitslist->begin(); unitsIterator != m_pMediaUnitslist->end(); unitsIterator++)
	{
		if (unitsIterator->GetUnitId() == ACUnitId)   //don't touch audio controller
			continue;
		if (unitsIterator->GetFreeCapacity() != 1000) //don't touch units with allocations on them
			continue;
		if (TRUE == is_Enabled && FALSE == unitsIterator->GetIsEnabled())
			continue;

		oldUnitType = unitsIterator->GetUnitType();
		WORD unitID = unitsIterator->GetUnitId();

		if (CHelperFuncs::IsBreeze(m_CardType) && eUnitType_Art == oldUnitType && eUnitType_Video == newUnitType) //in breeze we allow to config ART on special units only
		{
			if (CCardResourceConfigBreeze::ShouldBeARTUnit(unitsIterator->GetPhysicalUnitId())) // unitID)
				continue;
		}
		if ((oldUnitType == eUnitType_Art && newUnitType == eUnitType_Video) || (oldUnitType == eUnitType_Video && newUnitType == eUnitType_Art))
		{
			if (CSystemResources::IsTheSamePCI(unitsIterator->GetPhysicalUnitId(), ACUnitId) == TRUE)
			{
				if (newUnitType == eUnitType_Video) //try not to change the unit next to the AC to video
				{
					idOfUnitNextToACThatCouldBeReconfigured = unitsIterator->GetUnitId();
					continue;
				}
			}
			if (CMoveManager::ReconfigureUnit(m_OneBasedBoardId, unitsIterator->GetUnitId(), newUnitType) == STATUS_OK)
				numOfChangedUnits++;
			if (numOfChangedUnits == numUnitsToChange)
			{
				return numOfChangedUnits;
			}
		}
	}

	if (idOfUnitNextToACThatCouldBeReconfigured != 0xFFFF)
	{
		if (CMoveManager::ReconfigureUnit(m_OneBasedBoardId, idOfUnitNextToACThatCouldBeReconfigured, newUnitType) == STATUS_OK)
			numOfChangedUnits++;
		if (numOfChangedUnits == numUnitsToChange)
		{
			return numOfChangedUnits;
		}
	}

	TRACEINTO << "CBoard::ChangeUnits FAILED on board " << m_OneBasedBoardId << " We should have changed " << numUnitsToChange << " and actually changed only " << numOfChangedUnits;
	return numOfChangedUnits;
}

////////////////////////////////////////////////////////////////////////////
void CBoard::CountUnits(DWORD& audio, DWORD& video, BOOL bCountDisabledToo)
{
	audio = 0;
	video = 0;

	CMediaUnitsList::iterator unitsIterator;

	for (unitsIterator = m_pMediaUnitslist->begin(); unitsIterator != m_pMediaUnitslist->end(); unitsIterator++)
	{
		if (bCountDisabledToo == FALSE && unitsIterator->GetIsEnabled() == FALSE)
			continue;

		if (unitsIterator->GetUnitType() == eUnitType_Art)
			audio++;
		else if (unitsIterator->GetUnitType() == eUnitType_Video)
			video++;
	}
}

////////////////////////////////////////////////////////////////////////////
void CBoard::DivideArtAndVideoUnitsAccordingToProportion(DWORD alreadyConfiguredArtUnits, DWORD alreadyConfiguredVideoUnits, DWORD& numArtUnitsPerBoard, DWORD& numVideoUnitsPerBoard)
{
	numArtUnitsPerBoard = 0;
	numVideoUnitsPerBoard = 0;

	CCardResourceConfig* pConfig;
	if (CHelperFuncs::IsSoftCard(m_CardType))
	{
		pConfig = new CCardResourceConfigSoft(m_CardType);
	}
	else if (CHelperFuncs::IsBreeze(m_CardType))
	{
		pConfig = new CCardResourceConfigBreeze(m_CardType);
	}
	else
	{
		PASSERT(1);
		return;
	}

	pConfig->SetNumConfiguredUnits(alreadyConfiguredArtUnits, alreadyConfiguredVideoUnits);
	pConfig->DivideArtAndVideoUnitsAccordingToProportion(m_pMediaUnitslist->size(), numArtUnitsPerBoard, numVideoUnitsPerBoard);

	POBJDELETE(pConfig);
}

////////////////////////////////////////////////////////////////////////////
void CBoard::AddOneVideoParticipantsWithVideoOnThisBoard()
{
	m_NumOfVideoParticipantsWithVideoOnThisBoard++;
}

////////////////////////////////////////////////////////////////////////////
void CBoard::AddOneVideoParticipantsWithArtOnThisBoard()
{
	m_NumOfVideoParticipantsWithArtOnThisBoard++;
}

////////////////////////////////////////////////////////////////////////////
void CBoard::RemoveOneVideoParticipantsWithVideoOnThisBoard()
{
	if (m_NumOfVideoParticipantsWithVideoOnThisBoard >= 1)
		m_NumOfVideoParticipantsWithVideoOnThisBoard--;
	else
		PASSERTMSG(1, "Trying to remove OneVideoParticipantsWithVideoOnThisBoard, with less than one left");
}

////////////////////////////////////////////////////////////////////////////
void CBoard::RemoveOneVideoParticipantsWithArtOnThisBoard()
{
	if (m_NumOfVideoParticipantsWithArtOnThisBoard >= 1)
		m_NumOfVideoParticipantsWithArtOnThisBoard--;
	else
		PASSERTMSG(1, "Trying to remove OneVideoParticipantsWithArtOnThisBoard, with less than one left");
}

////////////////////////////////////////////////////////////////////////////
WORD CBoard::GetNumVideoParticipantsWithVideoOnThisBoard()
{
	return m_NumOfVideoParticipantsWithVideoOnThisBoard;
}

////////////////////////////////////////////////////////////////////////////
WORD CBoard::GetNumVideoParticipantsWithArtOnThisBoard()
{
	return m_NumOfVideoParticipantsWithArtOnThisBoard;
}

////////////////////////////////////////////////////////////////////////////
void CBoard::AddOneCOPConfOnThisBoard()
{
	m_NumOfCOPConfOnThisBoard++;
}

////////////////////////////////////////////////////////////////////////////
void CBoard::RemoveOneCOPConfOnThisBoard()
{
	if (m_NumOfCOPConfOnThisBoard >= 1)
		m_NumOfCOPConfOnThisBoard--;
	else
		PASSERTMSG(1, "Trying to remove OneVideoParticipantsWithVideoOnThisBoard, with less than one left");
}

////////////////////////////////////////////////////////////////////////////
WORD CBoard::GetNumCOPConfOnThisBoard()
{
	return m_NumOfCOPConfOnThisBoard;
}

////////////////////////////////////////////////////////////////////////////
MenuID CBoard::AllocatePcmMenuId(PartyRsrcID partyId)
{
	for (WORD i = 0; i < ARRAYSIZE(m_PcmMenuId); ++i)
	{
		if (DUMMY_PARTY_ID == m_PcmMenuId[i])
		{
			m_PcmMenuId[i] = partyId;
			return i;
		}
	}
	return DUMMY_ROOM_ID;
}

////////////////////////////////////////////////////////////////////////////
void CBoard::DeallocatePcmMenuId(PartyRsrcID partyId)
{
	for (WORD i = 0; i < ARRAYSIZE(m_PcmMenuId); ++i)
	{
		if (partyId == m_PcmMenuId[i])
		{
			m_PcmMenuId[i] = DUMMY_PARTY_ID;
			return;
		}
	}
	PASSERTSTREAM(m_OneBasedBoardId, "PartyId:" << partyId);
}

////////////////////////////////////////////////////////////////////////////
bool CBoard::IsPcmMenuIdExist(PartyRsrcID partyId) const
{
	for (WORD i = 0; i < ARRAYSIZE(m_PcmMenuId); ++i)
	{
		if (partyId == m_PcmMenuId[i])
			return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////
STATUS CBoard::AddVideoPreviewOnThisBoard(DWORD confID, DWORD partyID, WORD direction)
{
	CVideoPreview videoPreviewObj(confID, partyID, direction);
	if (MAX_NUM_VIDEO_PREVIEW_PER_BOARD == m_videoPreviewList->size() || m_videoPreviewList->find(videoPreviewObj) != m_videoPreviewList->end())
	{
		std::ostringstream msg;
		msg << "CBoard::AddVideoPreviewOnThisBoard - Failed, due to existing video previews:";
		for (std::set<CVideoPreview>::iterator itr = m_videoPreviewList->begin(); itr != m_videoPreviewList->end(); ++itr)
			msg << "\n  ConfId:" << itr->m_confID << ", PartyId:" << itr->m_partyID << ", Direction:" << itr->m_direction;

		TRACEINTO << msg.str().c_str();
		return STATUS_FAIL;
	}

	m_videoPreviewList->insert(videoPreviewObj);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CBoard::RemoveVideoPreviewOnThisBoard(DWORD confID, DWORD partyID, WORD direction)
{
	if (m_videoPreviewList->empty())
		return;

	if (direction != CVideoPreview::ePREVIEW_NONE)
	{
		CVideoPreview videoPreviewObj(confID, partyID, direction);
		m_videoPreviewList->erase(videoPreviewObj);
	}
	else
	{
		CVideoPreview videoPreviewInObj(confID, partyID, CVideoPreview::ePREVIEW_IN);
		m_videoPreviewList->erase(videoPreviewInObj);

		CVideoPreview videoPreviewOutObj(confID, partyID, CVideoPreview::ePREVIEW_OUT);
		m_videoPreviewList->erase(videoPreviewOutObj);
	}
}

////////////////////////////////////////////////////////////////////////////
void CBreezeHelper::FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, ePartyRole partyRole, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode)
{
	FTRACEINTO;
	// //////////////////////////////////////////////////////////////////////////////////////////////
	// NOTE: the decoder should always be the first unit, so the connection id of it stays the same, in realloc
	// //////////////////////////////////////////////////////////////////////////////////////////////
	memset(&videoAllocData.m_unitsList, 0, sizeof(videoAllocData.m_unitsList));

	BOOL bLimit3CifPortsOnDsp        = FALSE;
	BOOL bIsNotRMX1500QOrFlagIsOn    = FALSE;
	BOOL bLimitCifSdPortsPerMpmxCard = FALSE;
	int  port                        = 0;
	int  unit                        = 0;
	int  mix_rsrc_unit_id            = -1, mix_rsrc_port_id = -1;

	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_LIMIT_CIF_SD_PORTS_PER_MPMX_CARD, bLimitCifSdPortsPerMpmxCard);

	bool isAllocDecoder = (partyRole == eParty_Role_regular_party || partyRole == eParty_Role_content_decoder || partyRole == eParty_Role_AvMcuLink_SlaveIn);
	bool isAllocEncoder = (partyRole == eParty_Role_regular_party || partyRole == eParty_Role_content_encoder || partyRole == eParty_Role_AvMcuLink_SlaveOut);
	eLogicalResourceTypes encoderType = (partyRole == eParty_Role_content_encoder) ? eLogical_video_encoder_content : eLogical_video_encoder;

	switch (videoPartyType)
	{
		case eVSW_video_party_type:
		case eCP_H264_upto_CIF_video_party_type:
		case eCP_VP8_upto_CIF_video_party_type:
		{
			// VNGR-21581
			if (CResRsrcCalculator::IsRMX1500Q())
			{
				CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_3CIF_PORTS_ON_DSP, bLimit3CifPortsOnDsp);
			}

			// bIsNotRMX1500QOrFlagIsOn is FALSE for RMX1500Q system with 3CIF_PORTS_ON_DSP=NO (default for RMX1500Q)
			bIsNotRMX1500QOrFlagIsOn = ((videoPartyType == eCP_H264_upto_CIF_video_party_type || videoPartyType == eCP_VP8_upto_CIF_video_party_type) && (bLimit3CifPortsOnDsp || !CResRsrcCalculator::IsRMX1500Q()));

			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_type = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_encoder_weight = 0;
				if (bLimitCifSdPortsPerMpmxCard && eCP_H264_upto_CIF_video_party_type == videoPartyType)
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_CIF_SD30_PROMILLES_MODE3_BREEZE;
				else
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_CIF_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_in  = VID_DEC_CIF_BW_IN_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_out = VID_DEC_CIF_BW_OUT_BREEZE;

				if (bLimitCifSdPortsPerMpmxCard && eCP_H264_upto_CIF_video_party_type == videoPartyType)
					unit++;
				else
					port++;

			}
			if (isAllocEncoder)
			{
				if (bLimitCifSdPortsPerMpmxCard && eCP_H264_upto_CIF_video_party_type == videoPartyType)
				{
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_CIF_SD30_PROMILLES_MODE3_BREEZE;
				}
				else
				{
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_CIF_PROMILLES_BREEZE;
				}
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_type = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_encoder_weight = bIsNotRMX1500QOrFlagIsOn ? VID_ENC_CIF_WEIGHT_MODE2_MPMX : VID_ENC_CIF_WEIGHT_MPMX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_in = VID_ENC_CIF_BW_IN_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_out = VID_ENC_CIF_BW_OUT_BREEZE;
			}
			mix_rsrc_unit_id = unit + 1;
			mix_rsrc_port_id = 0;
			break;
		}

		case eCP_H261_H263_upto_CIF_video_party_type:
		{
			if (CResRsrcCalculator::IsRMX1500Q())
			{
				CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_3CIF_PORTS_ON_DSP, bLimit3CifPortsOnDsp);
			}

			// bIsNotRMX1500QOrFlagIsOn is FALSE for RMX1500Q system with 3CIF_PORTS_ON_DSP=NO (default for RMX1500Q)
			bIsNotRMX1500QOrFlagIsOn = (bLimit3CifPortsOnDsp || !CResRsrcCalculator::IsRMX1500Q());

			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_type = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_encoder_weight = 0;
				if (bLimitCifSdPortsPerMpmxCard)
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_CIF_SD30_PROMILLES_MODE3_BREEZE;
				else
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_SD30_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_in = VID_DEC_SD30_BW_IN_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_out = VID_DEC_SD30_BW_OUT_BREEZE;

				if (bLimitCifSdPortsPerMpmxCard)
					unit++;
				else
					port++;

			}
			if (isAllocEncoder)
			{
				if (bLimitCifSdPortsPerMpmxCard)
				{
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_CIF_SD30_PROMILLES_MODE3_BREEZE;
				}
				else
				{
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_SD30_PROMILLES_BREEZE;
				}
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_type = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_encoder_weight = bIsNotRMX1500QOrFlagIsOn ? VID_ENC_SD30_WEIGHT_MODE2_MPMX : VID_ENC_SD30_WEIGHT_MPMX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_in = VID_ENC_SD30_BW_IN_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_out = VID_ENC_SD30_BW_OUT_BREEZE;
			}
			mix_rsrc_unit_id = unit + 1;
			mix_rsrc_port_id = 0;
			break;
		}

		case eCP_H264_upto_SD30_video_party_type:
		case eCP_VP8_upto_SD30_video_party_type:
		{
			// VNGR-21581
			if (CResRsrcCalculator::IsRMX1500Q())
			{
				CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_3CIF_PORTS_ON_DSP, bLimit3CifPortsOnDsp);
			}

			// bIsNotRMX1500QOrFlagIsOn is FALSE for RMX1500Q system with 3CIF_PORTS_ON_DSP=NO (default for RMX1500Q)
			bIsNotRMX1500QOrFlagIsOn = (bLimit3CifPortsOnDsp || !CResRsrcCalculator::IsRMX1500Q());

			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_type = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_encoder_weight = 0;
				if (bLimitCifSdPortsPerMpmxCard)
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_CIF_SD30_PROMILLES_MODE3_BREEZE;
				else
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_SD30_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_in = VID_DEC_SD30_BW_IN_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_out = VID_DEC_SD30_BW_OUT_BREEZE;

				if (bLimitCifSdPortsPerMpmxCard)
					unit++;
				else
					port++;

			}
			if (isAllocEncoder)
			{
				if (bLimitCifSdPortsPerMpmxCard)
				{
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_CIF_SD30_PROMILLES_MODE3_BREEZE;
				}
				else
				{
					videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_SD30_PROMILLES_BREEZE;
				}
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_type = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_encoder_weight = bIsNotRMX1500QOrFlagIsOn ? VID_ENC_SD30_WEIGHT_MODE2_MPMX : VID_ENC_SD30_WEIGHT_MPMX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_in = VID_ENC_SD30_BW_IN_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[port].m_needed_bandwidth_out = VID_ENC_SD30_BW_OUT_BREEZE;
			}
			mix_rsrc_unit_id = unit + 1;
			mix_rsrc_port_id = 0;
			break;
		}

		case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_H263_4CIF_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = VID_DEC_SD30_BW_IN_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = VID_DEC_SD30_BW_OUT_BREEZE;
				port++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = encoderType;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = VID_ENC_H263_4CIF_WEIGHT_MPMX;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_H263_4CIF_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = VID_ENC_SD30_BW_IN_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = VID_ENC_SD30_BW_OUT_BREEZE;
			}
			mix_rsrc_unit_id = 1, mix_rsrc_port_id = 0;
			break;
		}

		// H263 legacy content decoder should be alone in both MPMx and MPMP, since in both of them we have real time problems
		// and no optimizations in XGA resolutions in H263.
		case eCP_Content_for_Legacy_Decoder_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_cntrl_type                = E_VIDEO_CONTENT_DECODER;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_HD1080_30FS_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = VID_DEC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = VID_DEC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE;
			}
			break;
		}

		case eCP_Content_for_Legacy_Decoder_HD1080_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_cntrl_type                = E_VIDEO_CONTENT_DECODER;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_HD1080_30FS_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = VID_DEC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = VID_DEC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE;
			}
			break;
		}

		case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = VID_DEC_HD720_30FS_SYMMETRIC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = VID_DEC_HD720_30FS_SYMMETRIC_BW_OUT_BREEZE;
				port++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = encoderType;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = VID_ENC_HD720_30FS_WEIGHT_MPMX;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = VID_ENC_HD720_30FS_SYMMETRIC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = VID_ENC_HD720_30FS_SYMMETRIC_BW_OUT_BREEZE;
			}
			mix_rsrc_unit_id = 1, mix_rsrc_port_id = 0;
			break;
		}

		case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD720_60FS_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_DEC_HD720_60FS_SYMMETRIC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_DEC_HD720_60FS_SYMMETRIC_BW_OUT_BREEZE;
				unit++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = VID_ENC_HD720_60FS_WEIGHT_MPMX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD720_60FS_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_ENC_HD720_60FS_SYMMETRIC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_ENC_HD720_60FS_SYMMETRIC_BW_OUT_BREEZE;
			}
			mix_rsrc_unit_id = 2, mix_rsrc_port_id = 0;
			break;
		}

		case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
		case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD1080_30FS_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_DEC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_DEC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE;
				unit++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = VID_ENC_HD1080_30FS_WEIGHT_MPMX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD1080_30FS_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_ENC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_ENC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE;
			}
			mix_rsrc_unit_id = 2, mix_rsrc_port_id = 0;
			break;
		}

		case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
		{
			// decoder must be first because in reallocate last connection id drops
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD720_60FS_PROMILLES_BREEZE;                // 1  DSP
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_DEC_HD720_60FS_SYMMETRIC_BW_IN_BREEZE;          // 0  IN
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_DEC_HD720_60FS_SYMMETRIC_BW_OUT_BREEZE;         // 214  OUT

			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_type                      = eLogical_video_encoder;
			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_cntrl_type                = E_VIDEO_MASTER_SPLIT_ENCODER;
			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_needed_encoder_weight     = VID_ENC_HD1080_60FS_WEIGHT_MPMX;
			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD1080_30FS_PROMILLES_BREEZE;               // 1  DSP
			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_ENC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE;         // 207  IN
			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_ENC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE;        // 0  OUT

			videoAllocData.m_unitsList[2].m_MediaPortsList[0].m_type                      = eLogical_video_encoder;
			videoAllocData.m_unitsList[2].m_MediaPortsList[0].m_cntrl_type                = E_VIDEO_SLAVE_SPLIT_ENCODER;
			videoAllocData.m_unitsList[2].m_MediaPortsList[0].m_needed_encoder_weight     = VID_ENC_HD1080_60FS_WEIGHT_MPMX;
			videoAllocData.m_unitsList[2].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD1080_30FS_PROMILLES_BREEZE;               // 1  DSP
			videoAllocData.m_unitsList[2].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_ENC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE;         // 207  IN
			videoAllocData.m_unitsList[2].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_ENC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE;        // 0  OUT
			mix_rsrc_unit_id                                                              = 3, mix_rsrc_port_id = 0;
			break;
		}

		case eVideo_relay_CIF_party_type:
		case eVideo_relay_SD_party_type:
		case eVideo_relay_HD720_party_type:
		case eVideo_relay_HD1080_party_type:
		{
			if (videoAllocData.m_confModeType == eMix)
			{
				FillAdditionalAvcSvcMixVideoUnitsList(videoPartyType, videoAllocData, 0, 0, HdVswTypeInMixAvcSvcMode);
			}
		}
		break;

		case eVSW_Content_for_CCS_Plugin_party_type:
			break;

		case eVSW_relay_CIF_party_type:
		case eVSW_relay_SD_party_type:
		case eVSW_relay_HD720_party_type:
		case eVSW_relay_HD1080_party_type:
			break;


		default:
			FPASSERT(1);
			break;
	}

	// MIX on RMX
	if ((videoAllocData.m_confModeType == eMix) && !CHelperFuncs::IsVideoRelayParty(videoPartyType) && (mix_rsrc_unit_id > -1) && (mix_rsrc_port_id > -1)
	    && (eParty_Role_content_decoder != partyRole) && (eParty_Role_content_encoder != partyRole))
	{
		FillAdditionalAvcSvcMixVideoUnitsList(videoPartyType, videoAllocData, mix_rsrc_unit_id, mix_rsrc_port_id, HdVswTypeInMixAvcSvcMode);
	}
}

////////////////////////////////////////////////////////////////////////////
void CBreezeHelper::FillAdditionalAvcSvcMixVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, int mix_rsrc_unit_id, int mix_rsrc_port_id, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode)
{
	BOOL isNeeded = FALSE, isSvc = FALSE, isHD = FALSE;
	CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(videoPartyType, isNeeded, isSvc, isHD);
	FTRACEINTO << "VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << ", IsNeeded:" << (WORD)isNeeded << ", IsSvc:" << (WORD)isSvc << ", IsHD:" << (WORD)isHD << ", HdVswTypeInMixAvcSvcMode:" << (WORD)HdVswTypeInMixAvcSvcMode;
	if (!isNeeded)
		return;

	if (isSvc)
	{
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_type = eLogical_video_decoder; //needed only if the party connects to MIXED conf
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_encoder_weight = 0;
		switch (HdVswTypeInMixAvcSvcMode)
		{
			case e_HdVsw720:
			{
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in       = VID_DEC_HD720_30FS_SYMMETRIC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = VID_DEC_HD720_30FS_SYMMETRIC_BW_OUT_BREEZE;
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_DEC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE;
			}
			break;

			case e_HdVsw1080:
			{
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in       = VID_DEC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = VID_DEC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE;
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_DEC_HD1080_30FS_PROMILLES_BREEZE;
			}
			break;

			case eHdVswNon:
			{
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in       = VID_DEC_SD30_BW_IN_BREEZE;
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = VID_DEC_360P_BW_OUT_BREEZE;
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_DEC_SD30_PROMILLES_BREEZE;
			}
			break;

			default:
			{
				FPASSERT(1);
			}
			break;
		}
	}
	else
	{
		BOOL bLimit3CifPortsOnDsp = FALSE;
		if (CResRsrcCalculator::IsRMX1500Q())
			CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_3CIF_PORTS_ON_DSP, bLimit3CifPortsOnDsp);

		// bIsNotRMX1500QOrFlagIsOn is FALSE for RMX1500Q system with 3CIF_PORTS_ON_DSP=NO (default for RMX1500Q)
		BOOL bIsNotRMX1500QOrFlagIsOn = (bLimit3CifPortsOnDsp || !CResRsrcCalculator::IsRMX1500Q());

		// AVC party
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_type                      = eLogical_relay_avc_to_svc_video_encoder_1;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_encoder_weight     = bIsNotRMX1500QOrFlagIsOn ? VID_ENC_CIF_WEIGHT_MODE2_MPMX : VID_ENC_CIF_WEIGHT_MPMX;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in       = VID_ENC_CIF_BW_IN_BREEZE;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = VID_ENC_CIF_BW_IN_BREEZE;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_ENC_CIF_PROMILLES_BREEZE;

		if (isHD)
		{
			++mix_rsrc_unit_id;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_type                      = eLogical_relay_avc_to_svc_video_encoder_2;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_encoder_weight     = bIsNotRMX1500QOrFlagIsOn ? VID_ENC_SD30_WEIGHT_MODE2_MPMX : VID_ENC_SD30_WEIGHT_MPMX;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in       = VID_ENC_SD30_BW_IN_BREEZE;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = VID_ENC_SD30_BW_OUT_BREEZE;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_ENC_SD30_PROMILLES_BREEZE;
		}
	}

	FTRACEINTO
	<< "LogicalResourceType:" << videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_type
	<< ", EncoderWeight:" << videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_encoder_weight
	<< ", BandwidthIn:" << videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in
	<< ", BandwidthOut:" << videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out
	<< ", PromillesCapacity:" << videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles;
}

////////////////////////////////////////////////////////////////////////////
void CBreezeHelper::FillVideoUnitsListCOP( eSessionType sessionType, eLogicalResourceTypes encoderType, AllocData& videoAllocData)
{       //Breeze-COP
	BOOL isSessionCOP = (eCOP_HD1080_session == sessionType || eCOP_HD720_50_session == sessionType);
	FPASSERT_AND_RETURN(!isSessionCOP);

	WORD i = 0, max_num_cif_dec = 0;

	memset(&videoAllocData.m_unitsList, 0, sizeof(videoAllocData.m_unitsList));
	WORD index = 0;

	if (eCOP_HD1080_session == sessionType) //1080p25
	{
		if (eLogical_COP_HD1080_encoder == encoderType)
		{
			//HD1080 encoder master
			videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_HD1080_encoder;
			videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD1080_30FS_PROMILLES_BREEZE;
			videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_ENC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE;
			videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_ENC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE;
			index++;
		}
		else
		{
			//HD720 encoder
			videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_HD720_encoder;
			videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_COP_ENC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE;
			videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_ENC_HD720_30FS_SYMMETRIC_BW_IN_BREEZE;
			videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_ENC_HD720_30FS_SYMMETRIC_BW_OUT_BREEZE;
			index++;
		}


		//4xCIF decoder x 3DSP
		WORD dsp_counter = index + 3;
		max_num_cif_dec = 4;
		for (WORD un = index; un < dsp_counter; un++)
		{
			for (i = 0; i < max_num_cif_dec; i++)
			{
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_type                      = eLogical_COP_Dynamic_decoder;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_capacity_promilles = VID_COP_CIF_DEC_PROMILLES_COP_BREEZE;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_bandwidth_in       = VID_DEC_CIF_BW_IN_BREEZE;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_bandwidth_out      = VID_DEC_CIF_BW_OUT_BREEZE;
			}
			index++;
		}


		//HD720 encoder + 2xCIF decoder
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_HD720_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_ENC_HD720_30FS_SYMMETRIC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_ENC_HD720_30FS_SYMMETRIC_BW_OUT_BREEZE;

		max_num_cif_dec = 2;
		for (i = 0; i < max_num_cif_dec; i++)
		{
			videoAllocData.m_unitsList[index].m_MediaPortsList[i + 1].m_type                      = eLogical_COP_Dynamic_decoder;
			videoAllocData.m_unitsList[index].m_MediaPortsList[i + 1].m_needed_capacity_promilles = VID_COP_CIF_DEC_PROMILLES_COP_BREEZE;
			videoAllocData.m_unitsList[index].m_MediaPortsList[i + 1].m_needed_bandwidth_in       = VID_DEC_CIF_BW_IN_BREEZE;
			videoAllocData.m_unitsList[index].m_MediaPortsList[i + 1].m_needed_bandwidth_out      = VID_DEC_CIF_BW_OUT_BREEZE;
		}
		index++;


		//CIF enc + PCM enc + 2CIF dec

		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_CIF_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_COP_CIF_ENC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_COP_CIF_ENC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_COP_CIF_ENC_BW_OUT_BREEZE;

		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_type                      = eLogical_COP_PCM_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_capacity_promilles = VID_COP_PCM_ENC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_bandwidth_in       = VID_COP_PCM_ENC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_bandwidth_out      = VID_COP_PCM_ENC_BW_OUT_BREEZE;

		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_type                      = eLogical_COP_Dynamic_decoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_capacity_promilles = VID_COP_CIF_DEC_PROMILLES_COP_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_bandwidth_in       = VID_DEC_CIF_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_bandwidth_out      = VID_DEC_CIF_BW_OUT_BREEZE;

		videoAllocData.m_unitsList[index].m_MediaPortsList[3].m_type                      = eLogical_COP_Dynamic_decoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[3].m_needed_capacity_promilles = VID_COP_CIF_DEC_PROMILLES_COP_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[3].m_needed_bandwidth_in       = VID_DEC_CIF_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[3].m_needed_bandwidth_out      = VID_DEC_CIF_BW_OUT_BREEZE;

		index++;


		//1080p decoder
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_LM_decoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_COP_HD1080_DEC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_COP_HD1080_DEC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_COP_HD1080_DEC_BW_OUT_BREEZE;
		index++;

		//SD enc+ VSW(dec+enc)
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_4CIF_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_SD30_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_COP_4CIF_ENC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_COP_4CIF_ENC_BW_OUT_BREEZE;

		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_type                      = eLogical_COP_VSW_decoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_capacity_promilles = VID_COP_VSW_DEC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_bandwidth_in       = VID_COP_VSW_DEC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_bandwidth_out      = VID_COP_VSW_DEC_BW_OUT_BREEZE;

		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_type                      = eLogical_COP_VSW_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_capacity_promilles = VID_COP_VSW_ENC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_bandwidth_in       = VID_COP_VSW_ENC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_bandwidth_out      = VID_COP_VSW_ENC_BW_OUT_BREEZE;

	}
	else    //720p50
	{       //720p 50fps enc(=HD1080 encoder)

		//HD1080 encoder
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_HD1080_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD1080_30FS_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_ENC_HD1080_30FS_SYMMETRIC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_ENC_HD1080_30FS_SYMMETRIC_BW_OUT_BREEZE;
		index++;

		//4xCIF decoder x 3DSP
		WORD dsp_counter = index + 3;
		max_num_cif_dec = 4;
		for (WORD un = index; un < dsp_counter; un++)
		{
			for (i = 0; i < max_num_cif_dec; i++)
			{
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_type                      = eLogical_COP_Dynamic_decoder;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_capacity_promilles = VID_COP_CIF_DEC_PROMILLES_COP_BREEZE;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_bandwidth_in       = VID_DEC_CIF_BW_IN_BREEZE;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_bandwidth_out      = VID_DEC_CIF_BW_OUT_BREEZE;
			}
			index++;
		}


		//720p 25fps enc(=HD720 encoder)  +   2xCIF decoder
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_HD720_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_ENC_HD720_30FS_SYMMETRIC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_ENC_HD720_30FS_SYMMETRIC_BW_OUT_BREEZE;

		max_num_cif_dec = 2;
		for (i = 0; i < max_num_cif_dec; i++)
		{
			videoAllocData.m_unitsList[index].m_MediaPortsList[i + 1].m_type                      = eLogical_COP_Dynamic_decoder;
			videoAllocData.m_unitsList[index].m_MediaPortsList[i + 1].m_needed_capacity_promilles = VID_COP_CIF_DEC_PROMILLES_COP_BREEZE;
			videoAllocData.m_unitsList[index].m_MediaPortsList[i + 1].m_needed_bandwidth_in       = VID_DEC_CIF_BW_IN_BREEZE;
			videoAllocData.m_unitsList[index].m_MediaPortsList[i + 1].m_needed_bandwidth_out      = VID_DEC_CIF_BW_OUT_BREEZE;
		}
		index++;


		//CIF enc + PCM enc + 2CIF decoder
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_CIF_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_COP_CIF_ENC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_COP_CIF_ENC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_COP_CIF_ENC_BW_IN_BREEZE;

		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_type                      = eLogical_COP_PCM_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_capacity_promilles = VID_COP_PCM_ENC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_bandwidth_in       = VID_COP_PCM_ENC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_bandwidth_out      = VID_COP_PCM_ENC_BW_OUT_BREEZE;

		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_type                      = eLogical_COP_Dynamic_decoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_capacity_promilles = VID_COP_CIF_DEC_PROMILLES_COP_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_bandwidth_in       = VID_DEC_CIF_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_bandwidth_out      = VID_DEC_CIF_BW_OUT_BREEZE;

		videoAllocData.m_unitsList[index].m_MediaPortsList[3].m_type                      = eLogical_COP_Dynamic_decoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[3].m_needed_capacity_promilles = VID_COP_CIF_DEC_PROMILLES_COP_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[3].m_needed_bandwidth_in       = VID_DEC_CIF_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[3].m_needed_bandwidth_out      = VID_DEC_CIF_BW_OUT_BREEZE;

		index++;


		//720p 50fps dec(=HD1080 decoder)
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_LM_decoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_COP_HD1080_DEC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_COP_HD1080_DEC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_COP_HD1080_DEC_BW_OUT_BREEZE;

		index++;

		//SE enc + VSW(enc+dec)
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_type                      = eLogical_COP_4CIF_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_SD30_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_COP_4CIF_ENC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_COP_4CIF_ENC_BW_OUT_BREEZE;

		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_type                      = eLogical_COP_VSW_decoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_capacity_promilles = VID_COP_VSW_DEC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_bandwidth_in       = VID_COP_VSW_DEC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[1].m_needed_bandwidth_out      = VID_COP_VSW_DEC_BW_OUT_BREEZE;

		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_type                      = eLogical_COP_VSW_encoder;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_capacity_promilles = VID_COP_VSW_ENC_PROMILLES_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_bandwidth_in       = VID_COP_VSW_ENC_BW_IN_BREEZE;
		videoAllocData.m_unitsList[index].m_MediaPortsList[2].m_needed_bandwidth_out      = VID_COP_VSW_ENC_BW_OUT_BREEZE;
	}
}

////////////////////////////////////////////////////////////////////////////
void CBreezeHelper::FillVideoUnitsListVSW( eSessionType sessionType, AllocData& videoAllocData)
{
	BOOL isSessionVSW = (eVSW_28_session == sessionType || eVSW_56_session == sessionType);
	FPASSERT_AND_RETURN(!isSessionVSW);

	memset(&videoAllocData.m_unitsList, 0, sizeof(videoAllocData.m_unitsList));

	WORD max_num_cif_ports = 1000 / VID_COP_VSW_ENC_PROMILLES_BREEZE;
	WORD max_num_units = (eVSW_28_session == sessionType) ? VSW_28_UNITS_NUM : VSW_56_UNITS_NUM;

	for (int un = 0; un < max_num_units; un++)
	{
		for (int i = 0; i < max_num_cif_ports; i++)
		{
			if (i % 2 == 0)
			{
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_capacity_promilles = VID_COP_VSW_DEC_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_bandwidth_in       = VID_COP_VSW_DEC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_bandwidth_out      = VID_COP_VSW_DEC_BW_OUT_BREEZE;
			}
			else
			{
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_type                      = eLogical_video_encoder;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_capacity_promilles = VID_COP_VSW_ENC_PROMILLES_BREEZE;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_bandwidth_in       = VID_COP_VSW_ENC_BW_IN_BREEZE;
				videoAllocData.m_unitsList[un].m_MediaPortsList[i].m_needed_bandwidth_out      = VID_COP_VSW_ENC_BW_OUT_BREEZE;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
float CBreezeHelper::CalculateNeededPromilles(ePortType type, DWORD artCapacity, BOOL isSvcOnly /*= FALSE*/, BOOL isAudioOnly /*= FALSE*/, eVideoPartyType videoPartyType /*=eVideo_party_type_dummy*/)
{
	switch (type)
	{
		case PORT_GENERIC:   { return 1001; }
		case PORT_ART:       { return artCapacityToPromilles(artCapacity, isSvcOnly, isAudioOnly, videoPartyType); }
		case PORT_ART_LIGHT: { return artCapacityToPromilles(artCapacity, isSvcOnly, isAudioOnly, videoPartyType); }
		case PORT_VIDEO:     { return VID_TOTAL_HD720_30FS_PROMILLES_BREEZE; }
		default:             { FPASSERT(1); return 1001; }
	}
}

////////////////////////////////////////////////////////////////////////////
float CMpmRxHelper::CalculateNeededPromilles(ePortType type, DWORD artCapacity, BOOL isSvcOnly /*= FALSE*/, BOOL isAudioOnly /*= FALSE*/)
{
	switch (type)
	{
		case PORT_GENERIC:   { return 1001; }
		case PORT_ART:       { return artCapacityToPromilles(artCapacity, isSvcOnly, isAudioOnly); }
		case PORT_ART_LIGHT: { return artCapacityToPromilles(artCapacity, isSvcOnly, isAudioOnly); }
		case PORT_VIDEO:     { return VID_TOTAL_HD720_30FS_PROMILLES_MPMRX; }
		default:             { FPASSERT(1); return 1001; }
	}
}

////////////////////////////////////////////////////////////////////////////
float CMpmRxHelper::artCapacityToPromilles(DWORD artCapacity, BOOL isSvcOnly /*= FALSE*/, BOOL isAudioOnly /*= FALSE*/)
{
	BOOL isTrafficShapingEnabled = NO;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING, isTrafficShapingEnabled);
	BOOL isTrafficShapingNeeded = !isSvcOnly && !isAudioOnly && isTrafficShapingEnabled && (artCapacity > 0);

	float artPromilles = 0;

	if (isSvcOnly || isAudioOnly)
	{
		artPromilles = ART_PROMILLES_AUDIO_OR_SVC_ONLY_MPMRX;
	}
	else if (artCapacity <= 1024) // up to 1MB connection
	{
		artPromilles = isTrafficShapingNeeded ? ART_PROMILLES_UP_TO_1MB_MPMRX_TRAFFIC_SHAPING : ART_PROMILLES_UP_TO_1MB_MPMRX;
	}
	else if (artCapacity <= 4096) // above 1MB connection, and up to 4MB
	{
		artPromilles = isTrafficShapingNeeded ? ART_PROMILLES_ABOVE_1MB_UP_TO_4MB_MPMRX_TRAFFIC_SHAPING : ART_PROMILLES_ABOVE_1MB_UP_TO_4MB_MPMRX;
	}
	else // for 6MB connection (above 4MB)
	{
		artPromilles = ART_PROMILLES_ABOVE_4MB_MPMRX;
	}

	return artPromilles;
}

////////////////////////////////////////////////////////////////////////////
float CBreezeHelper::artCapacityToPromilles(DWORD artCapacity, BOOL isSvcOnly /*= FALSE*/, BOOL isAudioOnly /*= FALSE*/, eVideoPartyType videoPartyType)
{
	BOOL isTrafficShapingEnabled = NO;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING, isTrafficShapingEnabled);
	BOOL isTrafficShapingNeeded = !isSvcOnly && !isAudioOnly && isTrafficShapingEnabled && (artCapacity > 0);

	float artPromilles = 0;
	if (isSvcOnly || isAudioOnly || artCapacity == 0) // for Audio Only, SVC and translators
	{
		artPromilles = ART_PROMILLES_BREEZE_TRAFFIC_SHAPING;
	}
	else if (videoPartyType == eCP_H261_H263_upto_CIF_video_party_type || videoPartyType == eCP_H264_upto_CIF_video_party_type) // CIF resolution
	{
		artPromilles = isTrafficShapingNeeded ? ART_PROMILLES_CIF_MPMX_TRAFFIC_SHAPING : CHelperFuncs::GetArtPromilsBreeze();
	}
	else
	{
		artPromilles = isTrafficShapingNeeded ? ART_PROMILLES_ABOVE_512MB_MPMX_TRAFFIC_SHAPING : CHelperFuncs::GetArtPromilsBreeze();
	}

	return artPromilles;
}

////////////////////////////////////////////////////////////////////////////
void  CMpmRxHelper::FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, ePartyRole partyRole, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode)
{
	FTRACEINTO;
	////////////////////////////////////////////////////////////////////////////////////////////////
	//NOTE: the decoder should always be the first unit, so the connection id of it stays the same, in realloc
	////////////////////////////////////////////////////////////////////////////////////////////////
	memset(&videoAllocData.m_unitsList, 0, sizeof(videoAllocData.m_unitsList));

	int port = 0;
	int unit = 0;
	int mix_rsrc_unit_id = -1, mix_rsrc_port_id = -1;

	bool isAllocDecoder = (partyRole == eParty_Role_regular_party || partyRole == eParty_Role_content_decoder || partyRole == eParty_Role_AvMcuLink_SlaveIn);
	bool isAllocEncoder = (partyRole == eParty_Role_regular_party || partyRole == eParty_Role_content_encoder || partyRole == eParty_Role_AvMcuLink_SlaveOut);
	eLogicalResourceTypes encoderType = (partyRole == eParty_Role_content_encoder) ? eLogical_video_encoder_content : eLogical_video_encoder;

	switch (videoPartyType)
	{
		case eVSW_video_party_type:
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_VSW_PROMILLES_MPMRX;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_HD_720_30_BW_MPMRX_NINJA_ENC_DEC;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_type                      = eLogical_video_encoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_capacity_promilles = VID_ENC_VSW_PROMILLES_MPMRX;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_bandwidth_in       = VID_HD_720_30_BW_MPMRX_NINJA_ENC_DEC;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_bandwidth_out      = 0;
			mix_rsrc_unit_id                                                              = 1, mix_rsrc_port_id = 0;
			break;

		case eCP_H264_upto_CIF_video_party_type:
		case eCP_VP8_upto_CIF_video_party_type:
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_CIF_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_CIF_BW_MPMRX_NINJA_ENC_DEC;
				unit++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_CIF_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_CIF_BW_MPMRX_NINJA_ENC_DEC;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
				unit++;
			}
			mix_rsrc_unit_id = unit, mix_rsrc_port_id = 0;
			break;

		case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_H261_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_CIF_BW_MPMRX_NINJA_ENC_DEC;
				unit++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_H261_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_CIF_BW_MPMRX_NINJA_ENC_DEC;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
				unit++;
			}
			mix_rsrc_unit_id = unit, mix_rsrc_port_id = 0;
			break;

		case eCP_H264_upto_SD30_video_party_type:
		case eCP_VP8_upto_SD30_video_party_type:
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_SD30_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC;
				unit++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_SD30_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
				unit++;
			}
			mix_rsrc_unit_id = unit, mix_rsrc_port_id = 0;
			break;

		case eCP_Content_for_Legacy_Decoder_video_party_type:
		case eCP_Content_for_Legacy_Decoder_HD1080_video_party_type:
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_cntrl_type                = E_VIDEO_CONTENT_DECODER;
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD1080_30FS_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_HD_1080_30_BW_MPMRX_NINJA_ENC_DEC;
			}
			break;

		case eCP_H261_H263_upto_CIF_video_party_type:         // In MPM-Rx this type is H263 CIF only
		case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD720_30FS_SYMMETRIC_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = videoPartyType == eCP_H261_H263_upto_CIF_video_party_type ? VID_CIF_BW_MPMRX_NINJA_ENC_DEC : VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC;
				unit++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = videoPartyType == eCP_H261_H263_upto_CIF_video_party_type ? VID_CIF_BW_MPMRX_NINJA_ENC_DEC : VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
				unit++;
			}
			mix_rsrc_unit_id = unit, mix_rsrc_port_id = 0;
			break;

		case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD720_30FS_SYMMETRIC_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_HD_720_30_BW_MPMRX_NINJA_ENC_DEC;
				unit++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_HD_720_30_BW_MPMRX_NINJA_ENC_DEC;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
				unit++;
			}
			mix_rsrc_unit_id = unit, mix_rsrc_port_id = 0;
			break;

		case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD720_60FS_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_HD_720_60_BW_MPMRX_NINJA_ENC_DEC;
				unit++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD720_60FS_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_HD_720_60_BW_MPMRX_NINJA_ENC_DEC;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
				unit++;
			}
			mix_rsrc_unit_id = unit, mix_rsrc_port_id = 0;
			break;

		case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD1080_30FS_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_HD_1080_30_BW_MPMRX_NINJA_ENC_DEC;
				unit++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_type                      = encoderType;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD1080_30FS_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_HD_1080_30_BW_MPMRX_NINJA_ENC_DEC;
				videoAllocData.m_unitsList[unit].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
				unit++;
			}
			mix_rsrc_unit_id = unit, mix_rsrc_port_id = 0;
			break;

		case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD1080_60FS_SYMMETRIC_PROMILLES_MPMRX;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_HD_1080_60_BW_MPMRX_NINJA_ENC_DEC;
			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_type                      = eLogical_video_encoder;
			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_needed_capacity_promilles = VID_ENC_HD1080_60FS_SYMMETRIC_PROMILLES_MPMRX;
			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_needed_bandwidth_in       = VID_HD_1080_60_BW_MPMRX_NINJA_ENC_DEC;
			videoAllocData.m_unitsList[1].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
			mix_rsrc_unit_id                                                              = 2, mix_rsrc_port_id = 0;
			break;

		// by Han: Ninja support HD1080 Content to non-H.239 endpoints
		case eCP_Content_for_Legacy_Decoder_HD1080_60FS_video_party_type:
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_cntrl_type                = E_VIDEO_CONTENT_DECODER;
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD1080_60FS_SYMMETRIC_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_out      = VID_HD_1080_60_BW_MPMRX_NINJA_ENC_DEC;
			}
			break;

		case eVideo_relay_CIF_party_type:
		case eVideo_relay_SD_party_type:
		case eVideo_relay_HD720_party_type:
		case eVideo_relay_HD1080_party_type:
			if (videoAllocData.m_confModeType == eMix)
			{
				FillAdditionalAvcSvcMixVideoUnitsList(videoPartyType, videoAllocData, 0, 0, HdVswTypeInMixAvcSvcMode);
			}
			break;

		case eVSW_Content_for_CCS_Plugin_party_type:
			break;

		default:
			FPASSERT(1);
			break;
	}

	// MIX on RMX
	if ((videoAllocData.m_confModeType == eMix) && !CHelperFuncs::IsVideoRelayParty(videoPartyType) && (mix_rsrc_unit_id > -1) && (mix_rsrc_port_id > -1)
	    && (eParty_Role_content_decoder != partyRole) && (eParty_Role_content_encoder != partyRole))
	{
		FillAdditionalAvcSvcMixVideoUnitsList(videoPartyType, videoAllocData, mix_rsrc_unit_id, mix_rsrc_port_id, HdVswTypeInMixAvcSvcMode);
	}
}

////////////////////////////////////////////////////////////////////////////
void CMpmRxHelper::FillAdditionalAvcSvcMixVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, int mix_rsrc_unit_id, int mix_rsrc_port_id, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode)
{
	BOOL isNeeded = FALSE, isSvc = FALSE, isHD = FALSE;
	CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(videoPartyType, isNeeded, isSvc, isHD);
	FTRACEINTO << "videoPartyType:" << eVideoPartyTypeNames[videoPartyType] << ", isNeeded:" << (WORD)isNeeded << ", isSvc:" << (WORD)isSvc << ", isHD:" << (WORD)isHD << ", mix_rsrc_unit_id:" << mix_rsrc_unit_id << ", mix_rsrc_port_id:" << mix_rsrc_port_id << ", HdVswTypeInMixAvcSvcMode:" << (WORD)HdVswTypeInMixAvcSvcMode;

	if (FALSE == isNeeded)
	{
		FTRACEINTO << " videoPartyType = " << eVideoPartyTypeNames[videoPartyType] << " isNeeded == FALSE";
		return;
	}

	if (isSvc)
	{
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_type                  = eLogical_video_decoder; //needed only if the party connects to MIXED conf
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_encoder_weight = 0;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in   = 0;

		switch (HdVswTypeInMixAvcSvcMode)
		{
			case e_HdVsw720:
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_DEC_HD720_30FS_SYMMETRIC_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = VID_HD_720_30_BW_MPMRX_NINJA_ENC_DEC;
				break;
			case  e_HdVsw1080:
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_DEC_HD1080_30FS_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = VID_HD_1080_30_BW_MPMRX_NINJA_ENC_DEC;
				break;
			case eHdVswNon:
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_DEC_SD30_PROMILLES_MPMRX;
				videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC;
				break;
			default:
				FPASSERT(1);
				break;
		}
	}
	else
	{
		// AVC party
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_type                      = eLogical_relay_avc_to_svc_video_encoder_1;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_encoder_weight     = 0;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in       = VID_CIF_BW_MPMRX_NINJA_ENC_DEC;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = 0;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_ENC_CIF_PROMILLES_MPMRX;

		if (isHD)
		{
			++mix_rsrc_unit_id;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_type                      = eLogical_relay_avc_to_svc_video_encoder_2;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in       = VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_ENC_SD30_PROMILLES_MPMRX;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CSoftMpmxHelper::FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, ePartyRole partyRole, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode)
{
	FTRACEINTO;
	memset(&videoAllocData.m_unitsList, 0, sizeof(videoAllocData.m_unitsList));

	int port = 0;

	bool isAllocDecoder = (partyRole == eParty_Role_regular_party || partyRole == eParty_Role_content_decoder || partyRole == eParty_Role_AvMcuLink_SlaveIn);
	bool isAllocEncoder = (partyRole == eParty_Role_regular_party || partyRole == eParty_Role_content_encoder || partyRole == eParty_Role_AvMcuLink_SlaveOut);
	eLogicalResourceTypes encoderType = (partyRole == eParty_Role_content_encoder) ? eLogical_video_encoder_content : eLogical_video_encoder;

	switch (videoPartyType)
	{
		case eCP_H264_upto_CIF_video_party_type:
		case eCP_VP8_upto_CIF_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_CIF_PROMILLES_SOFT_MPMX;
				port++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = encoderType;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_CIF_PROMILLES_SOFT_MPMX;
			}
		}
		break;

		case eCP_H261_H263_upto_CIF_video_party_type:
		case eCP_H264_upto_SD30_video_party_type:
		case eCP_VP8_upto_SD30_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_SD_PROMILLES_SOFT_MPMX;
				port++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = encoderType;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_SD_PROMILLES_SOFT_MPMX;
			}
		}
		break;

		case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
		case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_HD_PROMILLES_SOFT_MPMX;
				port++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = encoderType;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_encoder_weight     = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_HD_PROMILLES_SOFT_MPMX;
			}
		}
		break;

		case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_HD1080_30FS_PROMILLES_SOFT_MPMX;
				port++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = encoderType;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_HD1080_30FS_PROMILLES_SOFT_MPMX;
			}
		}
		break;

		case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
		{
			if (isAllocDecoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_HD1080_60FS_PROMILLES_SOFT_MPMX;
				port++;
			}
			if (isAllocEncoder)
			{
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = encoderType;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
				videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_ENC_HD1080_30FS_PROMILLES_SOFT_MPMX;
			}
		}
		break;

		// by Han: SoftMCU support Content to non-H.239 endpoints
		case eCP_Content_for_Legacy_Decoder_video_party_type:
		{
			videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_cntrl_type                = E_VIDEO_CONTENT_DECODER;
			videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_HD_PROMILLES_SOFT_MPMX;
		} break;

		// by Han: SoftMCU support HD1080 Content to non-H.239 endpoints
		case eCP_Content_for_Legacy_Decoder_HD1080_video_party_type:
		{
			videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_cntrl_type                = E_VIDEO_CONTENT_DECODER;
			videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_type                      = eLogical_video_decoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[port].m_needed_capacity_promilles = VID_DEC_HD1080_30FS_PROMILLES_SOFT_MPMX;
		}
		break;

		case eVideo_relay_CIF_party_type:
		case eVideo_relay_SD_party_type:
		case eVideo_relay_HD720_party_type:
		case eVideo_relay_HD1080_party_type:
		{
			if (videoAllocData.m_confModeType == eMix)        //isSVC_TO_AVC_Enable )// &&
			{
				FillAdditionalAvcSvcMixVideoUnitsList(videoPartyType, videoAllocData, 0, 0, HdVswTypeInMixAvcSvcMode);
			}
		}
		break;

		case eVSW_relay_CIF_party_type:
		case eVSW_relay_SD_party_type:
		case eVSW_relay_HD720_party_type:
		case eVSW_relay_HD1080_party_type:
			break;

		default:
			FPASSERT(1);
			break;
	}

	if ((videoAllocData.m_confModeType == eMix) && !CHelperFuncs::IsVideoRelayParty(videoPartyType)
	    && (eParty_Role_content_decoder != partyRole) && (eParty_Role_content_encoder != partyRole))     // isAVC_TO_SVC_Enable &&
	{
		FillAdditionalAvcSvcMixVideoUnitsList(videoPartyType, videoAllocData, 0, 2, HdVswTypeInMixAvcSvcMode);
	}
}

////////////////////////////////////////////////////////////////////////////
void CSoftMpmxHelper::FillAdditionalAvcSvcMixVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, int mix_rsrc_unit_id, int mix_rsrc_port_id, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode)
{

	BOOL isNeeded = FALSE, isSvc = FALSE, isHD = FALSE;
	CHelperFuncs::GetRequiredAvcSvcTranslatingVideoResorces(videoPartyType, isNeeded, isSvc, isHD);
	FTRACEINTO << " videoPartyType = " << eVideoPartyTypeNames[videoPartyType] << " ,isNeeded = " << (WORD)isNeeded << " ,isSvc = " << (WORD)isSvc << " ,isHD = " << (WORD)isHD;

	if (FALSE == isNeeded)
	{
		FTRACEINTO << " videoPartyType = " << eVideoPartyTypeNames[videoPartyType] << " isNeeded == FALSE";
		return;
	}

	if (isSvc)
	{
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_type                  = eLogical_video_decoder; //needed only if the party connects to MIXED conf
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_encoder_weight = 0;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in   = 0;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out  = 0;

		if (e_HdVsw720 == HdVswTypeInMixAvcSvcMode)
		{
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_DEC_HD_PROMILLES_SOFT_MPMX;
		}
		else
		{
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_DEC_SD_PROMILLES_SOFT_MPMX;
		}
	}
	else
	{
		// AVC party
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_type                      = eLogical_relay_avc_to_svc_video_encoder_1;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_encoder_weight     = 0;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_in       = 0;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_bandwidth_out      = 0;
		videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id].m_needed_capacity_promilles = VID_ENC_SD_PROMILLES_SOFT_MPMX;

		if (isHD)
		{
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id + 1].m_type                      = eLogical_relay_avc_to_svc_video_encoder_2;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id + 1].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id + 1].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id + 1].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[mix_rsrc_unit_id].m_MediaPortsList[mix_rsrc_port_id + 1].m_needed_capacity_promilles = VID_ENC_SD_PROMILLES_SOFT_MPMX;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
float CSoftMpmxHelper::PortTypeToPromilles(ePortType type)
{
	//FTRACEINTO << "PortType:" << type;
	switch (type)
	{
		case PORT_GENERIC:
		{
			return 1001;
		}
		case PORT_ART:
		case PORT_ART_LIGHT:
			return ART_PROMILLES_SOFT_MCU;
		case PORT_VIDEO:
			return VID_TOTAL_HD720_PROMILLES_SOFT_MPMX;

		default:
		{
			FPASSERT(1);
			return 1001;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CSoftMfwHelper::FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData)
{
	FTRACEINTO;
	memset(&videoAllocData.m_unitsList, 0, sizeof(videoAllocData.m_unitsList));

	switch (videoPartyType)
	{
		case eVSW_relay_CIF_party_type:
		case eVSW_relay_SD_party_type:
		case eVSW_relay_HD720_party_type:
		case eVSW_relay_HD1080_party_type:
		case eVideo_relay_CIF_party_type:
		case eVideo_relay_SD_party_type:
		case eVideo_relay_HD720_party_type:
		case eVideo_relay_HD1080_party_type:
		{
			//TODO?
		}
		break;

		default:
			FPASSERT(1);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////
float CSoftMfwHelper::PortTypeToPromilles(ePortType type)
{
	FTRACEINTO << "CSoftMfwHelper::PortTypeToPromilles ";
	switch (type)
	{
		case PORT_GENERIC:   { return 1001; }
		case PORT_ART:
		case PORT_ART_LIGHT:
			return ART_PROMILLES_SOFT_MFW;   //2000 Audio-only ports
		case PORT_VIDEO:
			return VIDEO_PROMILLES_SOFT_MFW; //1000 ports of CIF

		default: { FPASSERT(1); return 1001; }
	}
}

////////////////////////////////////////////////////////////////////////////
void CBoard::FillAdditionalAvcSvcMixVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, int mix_rsrc_unit_id, int mix_rsrc_port_id, eHdVswTypesInMixAvcSvc HdVswTypeInMixAvcSvcMode)
{
	TRACEINTO << "CardType:" << ::CardTypeToString(m_CardType);

	if (CHelperFuncs::IsSoftCard(m_CardType))
	{
		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		CSoftMpmxHelper::FillAdditionalAvcSvcMixVideoUnitsList(videoPartyType, videoAllocData, mix_rsrc_unit_id, mix_rsrc_port_id, HdVswTypeInMixAvcSvcMode);
	}
	else if (CHelperFuncs::IsBreeze(m_CardType))
		CBreezeHelper::FillAdditionalAvcSvcMixVideoUnitsList(videoPartyType, videoAllocData, mix_rsrc_unit_id, mix_rsrc_port_id, HdVswTypeInMixAvcSvcMode);
	else if (CHelperFuncs::IsMpmRxOrNinja(m_CardType))
		CMpmRxHelper::FillAdditionalAvcSvcMixVideoUnitsList(videoPartyType, videoAllocData, mix_rsrc_unit_id, mix_rsrc_port_id, HdVswTypeInMixAvcSvcMode);
	else
		PASSERT(1);
}

//N.A. To fix  !!!!!!!!!!
//		case eCP_VP8_upto_CIF_video_party_type:
//		case eCP_VP8_upto_SD30_video_party_type:
//		case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
////////////////////////////////////////////////////////////////////////////
float CSoftNinjaHelper::PortTypeToPromilles(ePortType type)
{
	FTRACEINTO << "CSoftNinjaHelper::PortTypeToPromilles ";
	switch (type)
	{
		case PORT_GENERIC:   { return 1001; }
		case PORT_ART:
		case PORT_ART_LIGHT:
			return ART_PROMILLES_SOFT_NINJA;
		case PORT_VIDEO:
			return VID_TOTAL_HD720_30FS_PROMILLES_MPMRX;

		default: { FPASSERT(1); return 1001; }
	}
}

////////////////////////////////////////////////////////////////////////////
void CSoftCallGeneratorHelper::FillVideoUnitsList(eVideoPartyType videoPartyType, AllocData& videoAllocData, ePartyRole partyRole)
{
	FTRACEINTO;
	memset(&videoAllocData.m_unitsList, 0, sizeof(videoAllocData.m_unitsList));

	switch (videoPartyType)
	{
		case eVSW_video_party_type:
		case eCP_H264_upto_CIF_video_party_type:
		case eCP_VP8_upto_CIF_video_party_type:
		case eCP_H261_H263_upto_CIF_video_party_type:          // In MPM-Rx this type is H263 CIF only (CG act as MPM-Rx)
		case eCP_H261_CIF_equals_H264_HD1080_video_party_type: // In CG it should be equal to CIF resource since it's a recorded video file.
		case eCP_H264_upto_SD30_video_party_type:
		case eCP_VP8_upto_SD30_video_party_type:
		case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
		{
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_CIF_SD_PROMILLES_SOFT_CG;

			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_type                      = eLogical_video_encoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_capacity_promilles = VID_ENC_CIF_SD_PROMILLES_SOFT_CG;
		}
		break;

		case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
		{
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD720_30FS_PROMILLES_SOFT_CG;

			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_type                      = eLogical_video_encoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_capacity_promilles = VID_ENC_HD720_30FS_PROMILLES_SOFT_CG;
		}
		break;

		case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
		{
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD1080_30FS_PROMILLES_SOFT_CG;

			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_type                      = eLogical_video_encoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_capacity_promilles = VID_ENC_HD1080_30FS_PROMILLES_SOFT_CG;
		}
		break;

		case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
		{
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_type                      = eLogical_video_decoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[0].m_needed_capacity_promilles = VID_DEC_HD1080_60FS_PROMILLES_SOFT_CG;

			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_type                      = eLogical_video_encoder;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_encoder_weight     = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_bandwidth_in       = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_bandwidth_out      = 0;
			videoAllocData.m_unitsList[0].m_MediaPortsList[1].m_needed_capacity_promilles = VID_ENC_HD1080_60FS_PROMILLES_SOFT_CG;
		}
		break;

		case eCP_Content_for_Legacy_Decoder_video_party_type: // Legacy Content is not supported in SoftMCU Call Generator
		case eCP_Content_for_Legacy_Decoder_HD1080_video_party_type:
		case eVideo_relay_CIF_party_type:                     // SVC is not supported in SoftMCU Call Generator
		case eVideo_relay_SD_party_type:
		case eVideo_relay_HD720_party_type:
		case eVideo_relay_HD1080_party_type:
		case eVSW_relay_CIF_party_type:
		case eVSW_relay_SD_party_type:
		case eVSW_relay_HD720_party_type:
		case eVSW_relay_HD1080_party_type:
		default:
			FPASSERT(1);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////
float CSoftCallGeneratorHelper::PortTypeToPromilles(ePortType type)
{
	FTRACEINTO << "CSoftCallGeneratorHelper::PortTypeToPromilles";
	switch (type)
	{
		case PORT_GENERIC:   { return 1001; }
		case PORT_ART:
		case PORT_ART_LIGHT:
			return ART_PROMILLES_SOFT_CG;
		case PORT_VIDEO:
			return VID_TOTAL_HD720_30FS_PROMILLES_SOFT_CG;

		default: { FPASSERT(1); return 1001; }
	}
}
