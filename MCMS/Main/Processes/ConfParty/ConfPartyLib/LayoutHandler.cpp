#include "LayoutHandler.h"
#include "VideoBridgeAutoScanParams.h"
#include "VideoApiDefinitionsStrings.h"
#include "PrettyTable.h"
#include "TelepresenseEPInfo.h"


struct GridRowInfo
{
	GridRowInfo(BYTE row) : rowIndex(row) {}
	BYTE rowIndex;
	std::vector<int> listFreeCells; // list of free cells indexes
	std::vector<int> listSingleEPs; // list of single ep cells indexes
};

bool compareFreeCellsInRow(const GridRowInfo& g, const GridRowInfo& j)
{
	return (g.listFreeCells.size() > j.listFreeCells.size());
}

class ScreenCpLayoutInfo  //the class for internal usage only: in order to store an intermediate layout info per screen
{
public:
	struct FilmstripInfo
	{
		FilmstripInfo() : m_isHaveFilmstrip(0), m_indexFilmstripFirstCell(0), m_numFilmstripCellsToUse(0), m_numFilmstripFilledCells(0) {}

		BYTE m_isHaveFilmstrip;
		BYTE m_indexFilmstripFirstCell;
		BYTE m_numFilmstripCellsToUse;
		BYTE m_numFilmstripFilledCells;
	};

public:

	ScreenCpLayoutInfo(RoomID id, EScreenPosition screenPos, CVideoBridgePartyCntl* pPartyCntl, CLayout* pOldLayout, TELEPRESENCE_SPEAKER_MODE_LAYOUT_S& rLayoutType) :
		m_roomId(id), m_pPartyCntl(pPartyCntl), m_pOldLayout(pOldLayout)
	{
		m_pNewLayout          = new CLayout(rLayoutType.layout, pPartyCntl->GetConfName());
		m_newScreenType       = rLayoutType.screenType;
		m_screenPos           = screenPos;
		m_numFilledCells      = 0;
		m_numCellsInGridRow   = 0;
		m_indexFirstCell      = 0;
		m_numCellsToUse       = 0;
		m_needToDeleteNewLayout = true;
		m_needToSwap          = true;

		memset(&m_telepresenceCellBorders, 0, sizeof(m_telepresenceCellBorders));
		// set all cells to "not specified" to differentiate from cells we intentionally want to be borderless in the layout
		for (BYTE i=0; i < MAX_NUMBER_OF_CELLS_IN_LAYOUT; ++i)
			m_telepresenceCellBorders.tBorderEdges[i] = gTelepresenceCellBorders[eTelepresenceBordersNotSpecified];

		if (eReserved == m_newScreenType)
		{
			switch (rLayoutType.layout)
			{
				case CP_LAYOUT_OVERLAY_ITP_1P4:
					m_numCellsToUse           = 1;
					m_indexFirstCell          = 0;
					m_filmstrip.m_isHaveFilmstrip         = 1;
					m_filmstrip.m_indexFilmstripFirstCell = 1;
					m_filmstrip.m_numFilmstripCellsToUse  = 4;
					break;

				case CP_LAYOUT_1X2_FLEX:        m_numCellsToUse = 2; m_indexFirstCell = 0; break;
				case CP_LAYOUT_3X3:             m_numCellsToUse = 3; m_indexFirstCell = 3; break;
				case CP_LAYOUT_4X4:             m_numCellsToUse = 4; m_indexFirstCell = 4; break;
				default:
					FTRACESTR(eLevelWarn) << "reserved screen - not supported layout type:" << LayoutTypeAsString[rLayoutType.layout];
					break;
			}
		}
		else
		{
			m_indexFirstCell = 0;
			switch (rLayoutType.layout)
			{
				case CP_LAYOUT_1X1:             m_numCellsToUse = 1;  m_numCellsInGridRow = 1; break;
				case CP_LAYOUT_2X2:             m_numCellsToUse = 4;  m_numCellsInGridRow = 2; break;
				case CP_LAYOUT_3X3:             m_numCellsToUse = 9;  m_numCellsInGridRow = 3; break;
				case CP_LAYOUT_4X4:             m_numCellsToUse = 16; m_numCellsInGridRow = 4; break;
				default:
					FTRACESTR(eLevelWarn) << "grid screen - not supported layout type:" << LayoutTypeAsString[rLayoutType.layout];
					break;
			}
		}
	}

	void NoNeedToDeleteNewLayout()
	{
		m_needToDeleteNewLayout = false;
	}

	void CopyNewLayoutParams(const ScreenCpLayoutInfo* pScreenCpLayoutInfo)
	{
		if (!pScreenCpLayoutInfo)
			return;

		m_pNewLayout        = pScreenCpLayoutInfo->m_pNewLayout;
		m_numCellsToUse     = pScreenCpLayoutInfo->m_numCellsToUse;
		m_indexFirstCell    = pScreenCpLayoutInfo->m_indexFirstCell;
		m_numFilledCells    = pScreenCpLayoutInfo->m_numFilledCells;
		m_numCellsInGridRow = pScreenCpLayoutInfo->m_numCellsInGridRow;
		m_filmstrip.m_isHaveFilmstrip         = pScreenCpLayoutInfo->m_filmstrip.m_isHaveFilmstrip;
		m_filmstrip.m_indexFilmstripFirstCell = pScreenCpLayoutInfo->m_filmstrip.m_indexFilmstripFirstCell;
		m_filmstrip.m_numFilmstripCellsToUse  = pScreenCpLayoutInfo->m_filmstrip.m_numFilmstripCellsToUse;
		m_filmstrip.m_numFilmstripFilledCells = pScreenCpLayoutInfo->m_filmstrip.m_numFilmstripFilledCells;
	}

	~ScreenCpLayoutInfo()
	{
		if (m_needToDeleteNewLayout)
		{
			POBJDELETE(m_pNewLayout);
		}
	}

	bool IsFilmstripCell(BYTE indexCell)
	{
		if (m_filmstrip.m_isHaveFilmstrip)
		{
			BYTE indexLastFilmstripCell = m_filmstrip.m_indexFilmstripFirstCell + m_filmstrip.m_numFilmstripCellsToUse;
			bool ret = (indexCell >= m_filmstrip.m_indexFilmstripFirstCell && indexCell < indexLastFilmstripCell);
			return ret;
		}
		return false;
	}

	BYTE GetNumGridRows()const
	{
		FPASSERT_AND_RETURN_VALUE(!m_pNewLayout, 0);

		BYTE num_rows = 0;
		switch (m_pNewLayout->GetLayoutType())
		{
			case CP_LAYOUT_1X1:             num_rows = 1; break;
			case CP_LAYOUT_2X2:             num_rows = 2; break;
			case CP_LAYOUT_3X3:             num_rows = 3; break;
			case CP_LAYOUT_4X4:             num_rows = 4; break;
			default:
				break;
		}
		return num_rows;
	}

	RoomID                 m_roomId;
	CVideoBridgePartyCntl* m_pPartyCntl;
	CLayout*               m_pOldLayout;
	CLayout*               m_pNewLayout;
	BORDERS_PARAM_S        m_telepresenceCellBorders;

	EScreenType            m_newScreenType;
	EScreenPosition        m_screenPos;
	bool                   m_needToDeleteNewLayout;
	bool                   m_needToSwap;

	BYTE                   m_numCellsToUse;
	BYTE                   m_indexFirstCell;
	BYTE                   m_numFilledCells;

	BYTE                   m_numCellsInGridRow;

	FilmstripInfo          m_filmstrip;
};
// screenPosition==0: DEST_SCREEN_POSITION_1, screenPosition==1: DEST_SCREEN_POSITION_2, screenPosition==2: DEST_SCREEN_POSITION_3,  screenPosition==3: DEST_SCREEN_POSITION_4

void DelScreenCpLayoutInfoElem (std::pair<EScreenPosition, ScreenCpLayoutInfo*> elem)
{
	POBJDELETE(elem.second);
}

////////////////////////////////////////////////////////////////////////////
//                        CLayoutHandler
////////////////////////////////////////////////////////////////////////////
CLayoutHandler::CLayoutHandler(const CVideoBridge* pVideoCntl, CVideoBridgePartyCntl* pPartyCntl)
{
	m_pAutoScanParams = new CVideoBridgeAutoScanParams(pVideoCntl);
	m_AutoScanTimeOut = 0;
	if (CPObject::IsValidPObjectPtr((CPObject*) const_cast<CVideoBridge*> (pVideoCntl)) || CPObject::IsValidPObjectPtr((CPObject*) pPartyCntl))
	{
		m_pVideoCntl = pVideoCntl;
		m_pPartyCntl = pPartyCntl;
		*m_pAutoScanParams = *(((CVideoBridge*) pVideoCntl)->GetVideoBridgeAutoScanParams());
	}
	else
	{
		m_pVideoCntl = NULL;
		m_pPartyCntl = NULL;
		PASSERT(1);
	}
}

////////////////////////////////////////////////////////////////////////////
CLayoutHandler::CLayoutHandler(const CLayoutHandler* pLayoutHandler)
{
	PASSERT_AND_RETURN(!pLayoutHandler);

	m_AutoScanTimeOut = 0;
	if (!(pLayoutHandler->m_pVideoCntl) || !(pLayoutHandler->m_pPartyCntl))
	{
		m_pVideoCntl      = NULL;
		m_pPartyCntl      = NULL;
		m_pAutoScanParams = NULL;
		PASSERT(1);
	}
	else
	{
		m_pVideoCntl       = pLayoutHandler->m_pVideoCntl;
		m_pPartyCntl       = pLayoutHandler->m_pPartyCntl;
		m_pAutoScanParams  = new CVideoBridgeAutoScanParams(m_pVideoCntl);
		*m_pAutoScanParams = *(pLayoutHandler->m_pAutoScanParams);
	}
}

////////////////////////////////////////////////////////////////////////////
CLayoutHandler::~CLayoutHandler()
{
	POBJDELETE(m_pAutoScanParams);
}

////////////////////////////////////////////////////////////////////////////
bool CLayoutHandler::BuildLayout()
{
	PASSERT_AND_RETURN_VALUE(!m_pPartyCntl, false);
	PASSERT_AND_RETURN_VALUE(!m_pVideoCntl, false);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN_VALUE(!pCurrLayout, false);

	// Create copy of current layout before changing it
	CLayout* pPreviouseLayout = new CLayout(*pCurrLayout);

	// Get party current layout type
	LayoutType newLayoutType = m_pPartyCntl->GetPartyCurrentLayoutType();

	if (newLayoutType >= CP_NO_LAYOUT)
	{
		PASSERT(newLayoutType);
		return (false);
	}

	TRACEINTO << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << ", OldLayout:" << LayoutTypeAsString[pCurrLayout->GetLayoutType()] << ", NewLayout:" << LayoutTypeAsString[newLayoutType];

	pCurrLayout->SetLayoutType(newLayoutType);

	// Remove old forces and blanks
	RemoveOldPartForcesAndBlankes();
	RemoveOldConfForcesAndBlankes();

	if (m_pPartyCntl->GetPartyLectureModeRole() == eLISTENER || m_pPartyCntl->GetPartyLectureModeRole() == eCOLD_LISTENER)
	{
		SetForcesForListener();
	}
	else
	{
		SetConfForces();
		SetPartyForces();
	}

	FillLayout();

	BuildLayoutIndications(*pCurrLayout);

	bool isLayoutChanged = (*pPreviouseLayout != *pCurrLayout);
	POBJDELETE(pPreviouseLayout);
	return (isLayoutChanged);
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::SetConfForces(void)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	CLayout* pRsrvLayout = m_pVideoCntl->GetReservationLayout();
	PASSERT_AND_RETURN(!pRsrvLayout);

	WORD IsPrivateLayout = m_pPartyCntl->GetIsPrivateLayout();

	std::ostringstream msg;
	msg << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << ", IsPrivateLayout:" << IsPrivateLayout;

	if (IsPrivateLayout)
		msg << " - Not active in Private layout";

	TRACEINTO << msg.str().c_str();

	if (IsPrivateLayout)
		return;

	WORD numSubImg = pCurrLayout->GetNumberOfSubImages();
	PASSERT_AND_RETURN(numSubImg != pRsrvLayout->GetNumberOfSubImages());

	WORD autoScanCell = pRsrvLayout->GetAutoScanCell();
	for (WORD i = 0; i < numSubImg; i++)
	{
		if (NULL == (*pRsrvLayout)[i] || NULL == (*pCurrLayout)[i])
			continue;

		if (((*pRsrvLayout)[i] && (*pRsrvLayout)[i]->isForcedInConfLevel()) || (*pRsrvLayout)[i]->isBlanked())
		{
			// Here we know that conference is forced here and copy the force into layout intended for building.
			if ((*pCurrLayout)[i]->CanBeSetTo((*pRsrvLayout)[i]->GetRequestPriority(), (*pRsrvLayout)[i]->GetVideoActivities()))
			{
				const char* partyName = (*pRsrvLayout)[i]->GetPartyForce();
				// If specific cell was forced to itself, make the cell voice activated
				if (!m_pVideoCntl->IsSameLayout())
					if (partyName && !strcmp(partyName, m_pPartyCntl->GetName()))
						continue;

				if (partyName && IsBlankCelINeedToBeForcedInITPLayout(pCurrLayout, partyName, i, FALSE))
					continue;

				(*pCurrLayout)[i]->SetPartyForceName(partyName);
				(*pCurrLayout)[i]->SetForceAttributes((*pRsrvLayout)[i]->GetRequestPriority(), (*pRsrvLayout)[i]->GetVideoActivities());
			}
		}
		else if (autoScanCell == i)
		{
			if ((*pCurrLayout)[i]->CanBeSetTo(OPERATOR_Prior, AUTO_CONF_Activ))
			{
				(*pCurrLayout)[i]->SetForceAttributes(AUTO_Prior, AUTO_SCAN_Active);
				(*pCurrLayout)[i]->RemovePartyForceName();
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::SetPartyForces(void)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	CLayout* pRsrvLayout = m_pVideoCntl->GetReservationLayout();
	PASSERT_AND_RETURN(!pRsrvLayout);

	CLayout* pPartyReservLayout = m_pPartyCntl->GetReservationLayout();

	WORD IsPrivateLayout = m_pPartyCntl->GetIsPrivateLayout();

	std::ostringstream msg;
	msg << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << ", IsPrivateLayout:" << IsPrivateLayout;

	if (IsPrivateLayout) // private layout
	{
		pPartyReservLayout = m_pPartyCntl->GetPrivateReservationLayout();
		msg << " - Setting Private reservation";
	}

	TRACEINTO << msg.str().c_str();

	PASSERT_AND_RETURN(!pPartyReservLayout);

	WORD numSubImg = pCurrLayout->GetNumberOfSubImages();
	PASSERT_AND_RETURN(numSubImg != pPartyReservLayout->GetNumberOfSubImages());

	for (WORD i = 0; i < numSubImg; i++)
	{
		if (NULL == (*pPartyReservLayout)[i] || NULL == (*pCurrLayout)[i])
			continue;

		if ((*pPartyReservLayout)[i]->isForcedInPartLevel() || (*pPartyReservLayout)[i]->isBlanked())
		{
			if ((*pCurrLayout)[i]->CanBeSetTo((*pPartyReservLayout)[i]->GetRequestPriority(), (*pPartyReservLayout)[i]->GetVideoActivities()))
			{
				const char* partyName = (*pPartyReservLayout)[i]->GetPartyForce();
				CVidSubImage* pSubImageOfForcedParty = NULL;
				if (!m_pPartyCntl->GetIsPrivateLayout() && !((*pPartyReservLayout)[i]->isBlanked())) // not private layout
				{
					// if party is forced in conference layout, it can't be forced in 2 cells (bug #3982)
					WORD confForceIndex = 0;
					pSubImageOfForcedParty = pRsrvLayout->GetSubImageOfForcedParty(partyName, confForceIndex);
				}

				if (!pSubImageOfForcedParty)
				{
					// In ITP layout, if one of the ITP connection is off line, its layout should be set as blank
					if (partyName && IsBlankCelINeedToBeForcedInITPLayout(pCurrLayout, partyName, i, TRUE))
						continue;

					(*pCurrLayout)[i]->SetPartyForceName(partyName);
					(*pCurrLayout)[i]->SetForceAttributes((*pPartyReservLayout)[i]->GetRequestPriority(), (*pPartyReservLayout)[i]->GetVideoActivities());
				}
			}
		}
		// BRIDGE-12457 cells in new private layout set to "auto select" after being marked as "forced not found" will remain blank
		else if ((*pCurrLayout)[i]->GetForcedNotFoundCell())
		{
			TRACEINTO << "clearing m_forcedNotFoundCell for cell index:" << i << ", partyID:" << m_pPartyCntl->GetPartyRsrcID();
			(*pCurrLayout)[i]->SetForcedNotFoundCell(false);
			(*pCurrLayout)[i]->SetForceAttributes((*pPartyReservLayout)[i]->GetRequestPriority(), (*pPartyReservLayout)[i]->GetVideoActivities());
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::SetForcesForListener()
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);
	PASSERT_AND_RETURN(pCurrLayout->GetLayoutType() != CP_LAYOUT_1X1);

	const char* pLectureName = m_pVideoCntl->GetLecturerName();
	PASSERT_AND_RETURN(!pLectureName);

	BYTE rs = strcmp(pLectureName, "");
	DBGPASSERT_AND_RETURN(rs == 0);

	TRACEINTO << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << ", LecturerName:" << pLectureName;

	DBGPASSERT_AND_RETURN(NULL == (*pCurrLayout)[0]);
	(*pCurrLayout)[0]->SetPartyForceName(pLectureName);
	(*pCurrLayout)[0]->SetForceAttributes(OPERATOR_Prior, FORCE_CONF_Activ);
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::RemoveOldConfForcesAndBlankes(void)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	WORD numbSubImg = pCurrLayout->GetNumberOfSubImages();
	for (WORD i = 0; i < numbSubImg; i++)
	{
		if (NULL == (*pCurrLayout)[i])
			continue;

		if ((*pCurrLayout)[i]->isForcedInConfLevel())
		{
			(*pCurrLayout)[i]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
			(*pCurrLayout)[i]->RemovePartyForceName();
		}
		else
		{
			if ((*pCurrLayout)[i]->GetVideoActivities() == BLANK_CONF_Activ || (*pCurrLayout)[i]->GetVideoActivities() == AUTO_SCAN_Active)
				(*pCurrLayout)[i]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::RemoveOldPartForcesAndBlankes(void)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);


	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	WORD numbSubImg = pCurrLayout->GetNumberOfSubImages();
	for (WORD i = 0; i < numbSubImg; i++)
	{
		if (NULL == (*pCurrLayout)[i])
			continue;

		if ((*pCurrLayout)[i]->isForcedInPartLevel())
		{
				(*pCurrLayout)[i]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
				(*pCurrLayout)[i]->RemovePartyForceName();
			}
		else
		{
			if ((*pCurrLayout)[i]->GetVideoActivities() == BLANK_PARTY_Activ || (*pCurrLayout)[i]->GetVideoActivities() == BLANK_PRIVATE_PARTY_Active)
				(*pCurrLayout)[i]->SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::FillLayout()
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);
	bool isAVMCUParty =  m_pPartyCntl->IsAVMCUParty();

	std::ostringstream msg;
	msg << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << ", CascadeLink:" << (int)m_pPartyCntl->GetCascadeLinkMode() << ", IsSameLayout:" << (int)m_pVideoCntl->IsSameLayout();

	if (m_pVideoCntl->IsSameLayout() && m_pPartyCntl->GetCascadeLinkMode() == CASCADE_MODE_NONE)
	{
		TRACEINTO << msg.str().c_str();
		FillSameLayout();
		return;
	}

	WORD numCellsInLayout = pCurrLayout->GetNumberOfSubImages();
	PASSERT_AND_RETURN(numCellsInLayout == 0);

	//stage 1 ~~ Create copy of current layout before changing it
	CLayout previouseLayout(*pCurrLayout);

	//stage 2 ~~ We always Fill something that is clean
	pCurrLayout->ClearAllImageSources();

	WORD numConnectedImages = m_pVideoCntl->GetPartyImageVectorSize();
	if (numConnectedImages == 0) //may be the case if no decoder is connected in conference
		return;

	//stage 3 ~~ Images that are not muted are inserted to Vector imagesReadyForSetting
	CPartyImageVector imagesReadyForSetting;
	CreateVectorOfImagesReadyForSetting(imagesReadyForSetting);
	WORD numImagesReadyForSetting = imagesReadyForSetting.size();

	msg << ", numConnectedImages:" << numConnectedImages << ", numImagesReadyForSetting:" << numImagesReadyForSetting;
	if (numImagesReadyForSetting)
	{
		msg << ", ImagesReadyForSetting[";
		for (int i = 0; i < numImagesReadyForSetting; ++i)
		{
			msg << imagesReadyForSetting[i];
			if (i == numImagesReadyForSetting-1)
				msg << "]";
			else
				msg << ",";
		}
	}
	TRACEINTO << msg.str().c_str();

	//stage 4 ~~ check that there exist Images for setting
	if (numImagesReadyForSetting == 0) //may be the case if no decoder is connected in conf and is unmuted
	{
		SetSelfView();
		return;
	}

	// stage 5 ~~ if there is one valid image and it is own image SetSelfView
	if ((imagesReadyForSetting.size() == 1) && (imagesReadyForSetting[0] == m_pPartyCntl->GetPartyRsrcID()))
	{
		SetSelfView();
		return;
	}

	if(isAVMCUParty && IsVectorImagesReadyForSettingContainsAVMCULinksOnly(imagesReadyForSetting))
	{
		SetSelfView();
		return;
	}

	// stage 6 ~~~ initiate numCellsLeft (= cells in layout that are not blanked)
	WORD numCellsLeft = 0;
	for (DWORD i = 0; i < numCellsInLayout; i++)
		if (NULL != (*pCurrLayout)[i] && !(*pCurrLayout)[i]->isBlanked())
			numCellsLeft++;

	if (numCellsLeft == 0)
	{ //all cells in layout are blanked
		TRACEINTO << "numCellsLeft:0";
		return;
	}

	// stage 7 ~~~ Set forced parties
	SetForcedPartiesInLayout(imagesReadyForSetting, numCellsLeft);

	// stage 8 ~~~ Remove Self Image from imagesReadyForSetting Array
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
		RemoveSelfImageFromImagesReadyForSetting(imagesReadyForSetting); // Func contains speacial support of AV-MCU

	// stage 9 ~~~ AutoScan Remove from imagesReadyForSetting images that will be seen in the auto_scan cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
	{
		TRACEINTO << "numCellsLeft:" << numCellsLeft << ", imagesReadyForSettingSize:" << imagesReadyForSetting.size();
		//mhalfon - VNGR-21927 - 09-08-2011
		//there were a cases that we saw exception in the logs in case a party was delete.
		//the exception was because the scan vector was containing a NULL pointer for CImage object.
		//(see CVideoBridgeAutoScanParam::GetNextImage line 84)
		//m_pAutoScanParams is updated in BuildLayout function, and we do call this function in case we delete party,
		//but it does not updated and we suspect that it because we should not ask about
		if (pCurrLayout->IsAutoScanSet())
		{
			if(CPObject::IsValidPObjectPtr(m_pAutoScanParams))
			{
				BYTE numAutoScanImages = m_pAutoScanParams->FillAutoScanImages(imagesReadyForSetting,numCellsLeft);
				if (numAutoScanImages)
				{
				 FillAutoScanImageInLayout(*pCurrLayout,numAutoScanImages-1);
				}
			}
		}
	}

	// stage 10 ~~~ Remove from imagesReadyForSetting images that can not enter layout because speaker order
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
		if (numCellsLeft < imagesReadyForSetting.size())
		{
			imagesReadyForSetting.erase((imagesReadyForSetting.begin()+numCellsLeft), imagesReadyForSetting.end());
		}

	// stage 11 ~~~ In Layout with Speaker Picture try to set speaker in to first cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
	{
		if (pCurrLayout->GetLayoutType() == CP_LAYOUT_2P8 || pCurrLayout->GetLayoutType() == CP_LAYOUT_2TOP_P8)
			SetSpeakerImageToBigCellInLayout2Plus8IfNeeded(imagesReadyForSetting, &previouseLayout, numCellsLeft);
		else if(IsLayoutWithSpeakerPicture(pCurrLayout->GetLayoutType()))
		{
			 SetSpeakerImageToBigCellInLayout(imagesReadyForSetting, numCellsLeft);
		}
	}
	// stage 12 ~~~ Try to set the remaining images in their previous cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
	{
		SetImagesToPreviouseSeenCellsInLayout(imagesReadyForSetting, &previouseLayout, numCellsLeft);
	}

	// stage 13 ~~~ Put the remaining images in any unused cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
	{
		SetImagesToAnyUnusedCell(imagesReadyForSetting);
	}
}
////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::FillSameLayout()
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	if(!m_pVideoCntl->IsSameLayout())
	{
		PASSERTMSG(1, "The conference not defined as same layout");
	}

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	WORD numCellsInLayout = pCurrLayout->GetNumberOfSubImages();
	PASSERT_AND_RETURN(numCellsInLayout==0);

	// stage 1 ~~ Create copy of current layout before changing it
	CLayout* pPreviouseLayoutToCopy;

	// if new party in conference copy history from different party
	if (pCurrLayout->isLayoutEmpty())
	{
		CLayout* pLayout = m_pVideoCntl->GetAnyPartyLayout();
		if(pLayout)
			pPreviouseLayoutToCopy = new CLayout(*pLayout);
		else
			PASSERT_AND_RETURN(101);
	}
	else
	{
		pPreviouseLayoutToCopy = new CLayout(*pCurrLayout);
	}

	CLayout previouseLayout(*pPreviouseLayoutToCopy);
	POBJDELETE(pPreviouseLayoutToCopy);

	// stage 2 ~~ We always Fill something that is clean
	pCurrLayout->ClearAllImageSources();

	WORD numConnectedImages = m_pVideoCntl->GetPartyImageVectorSize();
	if (numConnectedImages == 0) //may be the case if no decoder is connected in conf
		return;

	// stage 3 ~~ Images that are not muted are inserted to Vector imagesReadyForSetting
	CPartyImageVector imagesReadyForSetting;
	CLayoutHandler::CreateVectorOfImagesReadyForSetting(imagesReadyForSetting);

	// stage 4 ~~ check that there exist Images for setting
	if(imagesReadyForSetting.size()==0)
		return;

	// stage 5 ~~ if there is one valid image and it is own image SetSelfView
	if ((imagesReadyForSetting.size() == 1) && (imagesReadyForSetting[0] == m_pPartyCntl->GetPartyRsrcID()))
	{
		SetSelfView();
		return;
	}

	// stage 6 ~~~ initiate numCellsLeft (= cells in layout that are not blanked)
	WORD numCellsLeft = 0;
	for (DWORD i = 0; i < numCellsInLayout; i++)
		if ( NULL != (*pCurrLayout)[i] && !((*pCurrLayout)[i]->isBlanked()) )
			numCellsLeft++;

	if (numCellsLeft == 0)
	{ //all cells in layout are blanked
		return;
	}

	// stage 7 ~~~ Set forced parties
	SetForcedPartiesInLayout(imagesReadyForSetting, numCellsLeft);

	// stage 8 - NOT DONE IN SAME LAYOUT
	// ~~~ Remove Self Image from imagesReadyForSetting Array in case that "SameLayout" flag is OFF

	// stage 9 ~~~ NOT DONE IN SAME LAYOUT
	// VNGR-23212: Why not done? Copied from FillLayout and a simple test shows it works well
	// stage 9 ~~~ AutoScan Remove from imagesReadyForSetting images that will be seen in the auto_scan cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
	{
		TRACEINTO << "numCellsLeft:" << numCellsLeft << ", imagesReadyForSettingSize:" << imagesReadyForSetting.size();
		//mhalfon - VNGR-21927 - 09-08-2011
		//there were a cases that we saw exception in the logs in case a party was delete.
		//the exception was because the scan vector was containing a NULL pointer for CImage object.
		//(see CVideoBridgeAutoScanParam::GetNextImage line 84)
		//m_pAutoScanParams is updated in BuildLayout function, and we do call this function in case we delete party,
		//but it does not updated and we suspect that it because we should not ask about
		if (pCurrLayout->IsAutoScanSet())
		{
		if(CPObject::IsValidPObjectPtr(m_pAutoScanParams))
		{
			BYTE numAutoScanImages = m_pAutoScanParams->FillAutoScanImages(imagesReadyForSetting,numCellsLeft);
			if (numAutoScanImages)
			FillAutoScanImageInLayout(*pCurrLayout,numAutoScanImages-1);
		}
		}
	}

	// stage 10 ~~~ Remove from imagesReadyForSetting images that can not enter layout because speaker order
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
		if (numCellsLeft < imagesReadyForSetting.size())
			imagesReadyForSetting.erase((imagesReadyForSetting.begin()+numCellsLeft), imagesReadyForSetting.end());

	// stage 11
	// ~~~ In Layout with Speaker Picture try to set speaker in to first cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
	{
		if (pCurrLayout->GetLayoutType() == CP_LAYOUT_2P8 || pCurrLayout->GetLayoutType() == CP_LAYOUT_2TOP_P8)
			SetSpeakerImageToBigCellInLayout2Plus8IfNeeded(imagesReadyForSetting, &previouseLayout, numCellsLeft);
		else if(IsLayoutWithSpeakerPicture(pCurrLayout->GetLayoutType()))
			SetSpeakerImageToBigCellInLayout(imagesReadyForSetting, numCellsLeft);
	}

	// stage 12 ~~~ Try to set the remaining images in their previous cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
		SetImagesToPreviouseSeenCellsInLayout(imagesReadyForSetting, &previouseLayout, numCellsLeft);

	// stage 12 ~~~ Put the remaining images in any unused cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
		SetImagesToAnyUnusedCell(imagesReadyForSetting);
}

//--------------------------------------------------------------------------
void CLayoutHandler::CreateVectorOfImagesReadyForSetting(CPartyImageVector& rImagesReadyForSetting)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	bool isRelayParty = m_pPartyCntl->IsVideoRelayParty();

	// Run over all connected Images to video bridge images and those not muted insert to Vector
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	WORD partyImageVectorSize = m_pVideoCntl->GetPartyImageVectorSize();
	for (WORD i = 0; i < partyImageVectorSize; ++i)
	{
		DWORD partyRsrcId = m_pVideoCntl->GetPartyImageIdByPosition(i);
		CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRsrcId);
		if (!pImage)
		{
            //VNGFE-7650 -PartyRsrcId that not exist in PartyImageLookupTable but wasn't removed from VideoBridge PartyIamgeVector because of some invalid flow.
            PASSERTSTREAM(!pImage, "Failed, The lookup table doesn't have an element, PartyId:" << partyRsrcId); // BRIDGE-10767: just ignore this image and keep on
            continue;
			// PASSERTSTREAM_AND_RETURN(!pImage, "Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);
		}

		if (!pImage->isMuted())
		{
			if (!isRelayParty)
			{
				if (INVALID == pImage->GetConnectionId())
				{
					continue;
				}
			}

			rImagesReadyForSetting.push_back(partyRsrcId);
		}
	}
}
//--------------------------------------------------------------------------
// TELEPRESENCE_LAYOUTS
void CLayoutHandlerLegacy::CreateVectorOfImagesReadyForSetting(CPartyImageVector& rImagesReadyForSetting)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

#define VECTOR_DEBUG_PRINTS

	BOOL bIsConfInActiveContentPresentation = ((CVideoBridgeCP*)m_pVideoCntl)->IsConfInActiveContentPresentation();
	BOOL bTelepresenceOnOff = m_pVideoCntl->GetTelepresenceOnOff();
	BOOL bManageTelepresenceLayoutsInternally = m_pVideoCntl->GetManageTelepresenceLayoutsInternally();

	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = const_cast<CVideoBridge*> (m_pVideoCntl)->GetTelepresenceLayoutMngr();

	if(!bTelepresenceOnOff || !bIsConfInActiveContentPresentation || !bManageTelepresenceLayoutsInternally || NULL == pTelepresenceLayoutMngr){
#ifdef VECTOR_DEBUG_PRINTS
		TRACEINTO << " create full images vector : bTelepresenceOnOff = " << (WORD)bTelepresenceOnOff << ", bIsConfInActiveContentPresentation = " << (WORD)bIsConfInActiveContentPresentation << ", bManageTelepresenceLayoutsInternally = " << (WORD)bManageTelepresenceLayoutsInternally << ", pTelepresenceLayoutMngr = " << (DWORD)pTelepresenceLayoutMngr;
#endif
		CLayoutHandler::CreateVectorOfImagesReadyForSetting(rImagesReadyForSetting);
		return;
	}


	// Get Layout type and indexes list for the current screen (destination)
#ifdef VECTOR_DEBUG_PRINTS
	TRACEINTO << " one image per room";
#endif

	// set contains resource party IDs that should not be used (siade cameras if ITP room)
	std::set<DWORD> imagesInUsedRooms;

	bool isRelayParty = m_pPartyCntl->IsVideoRelayParty();

	// Run over all connected Images to video bridge images and those not muted insert to Vector
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	WORD partyImageVectorSize = m_pVideoCntl->GetPartyImageVectorSize();

#ifdef VECTOR_DEBUG_PRINTS
	std::ostringstream debug_msg;
	std::ostringstream debug_msg_rooms;
#endif


	for (WORD i = 0; i < partyImageVectorSize; ++i)
	{
		DWORD partyRsrcId = m_pVideoCntl->GetPartyImageIdByPosition(i);
		CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRsrcId);
		if (!pImage)
		{
            //VNGFE-7650 -PartyRsrcId that not exist in PartyImageLookupTable but wasn't removed from VideoBridge PartyIamgeVector because of some invalid flow.
            PASSERTSTREAM(!pImage, "Failed, The lookup table doesn't have an element, PartyId:" << partyRsrcId); // BRIDGE-10767: just ignore this image and keep on
#ifdef VECTOR_DEBUG_PRINTS
            debug_msg << "assert:" << partyRsrcId << " ";
#endif
            continue;
		}

		if (!pImage->isMuted())
		{
			if (!isRelayParty && INVALID == pImage->GetConnectionId())
			{
#ifdef VECTOR_DEBUG_PRINTS
				debug_msg << "cont:" << partyRsrcId << " ";
#endif
				continue;
			}

			if(imagesInUsedRooms.end() == imagesInUsedRooms.find(partyRsrcId))
			{
				rImagesReadyForSetting.push_back(partyRsrcId);
#ifdef VECTOR_DEBUG_PRINTS
				debug_msg << "in:" << partyRsrcId << " ";
#endif
				// update imagesInUsedRooms with other cameras of room
				std::set<DWORD> roomPartiesIds;
				pTelepresenceLayoutMngr->GetRoomPartiesIds(partyRsrcId,roomPartiesIds);
				if(roomPartiesIds.size() > 1){
					for(std::set<DWORD>::iterator it = roomPartiesIds.begin();it != roomPartiesIds.end();++it)
					{
						if(*it != partyRsrcId){
#ifdef VECTOR_DEBUG_PRINTS
							debug_msg_rooms << (*it) << "  ";
#endif
							imagesInUsedRooms.insert(*it);
						}
					}
				}
			}else{
#ifdef VECTOR_DEBUG_PRINTS
				debug_msg << "skip:" << partyRsrcId << " ";
#endif
			}
		}
	}

#ifdef VECTOR_DEBUG_PRINTS
	TRACEINTO << "rImagesReadyForSetting: " << debug_msg.str().c_str();
	TRACEINTO << "imagesInUsedRooms:      " << debug_msg_rooms.str().c_str();
#endif

}

//--------------------------------------------------------------------------
void CLayoutHandler::SetForcedPartiesInLayout(CPartyImageVector& rImagesReadyForSetting, WORD& rNumCellsLeftToFill)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	//Read system flag set blank cell when the party disconnects
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = "PRESERVE_PARTY_CELL_ON_FORCE_LAYOUT";
	BOOL isBlankCellEnabledWhenPartyDisconnect;
	sysConfig->GetBOOLDataByKey(key, isBlankCellEnabledWhenPartyDisconnect);

	// Bridge-10153
	// Check if it's telepresense EP, if it's, we enable blank cell for party disconnected or muted
	const CTelepresenseEPInfo& partyITPInfo = m_pPartyCntl->GetTelepresenceInfo();
	BOOL isTelepresenceEP = FALSE;

	if (partyITPInfo.GetLinkRole() == eRegularParty && partyITPInfo.GetEPtype() != eTelePresencePartyNone)
	{
		isTelepresenceEP = TRUE;
	}

	WORD numCellsInLayout = pCurrLayout->GetNumberOfSubImages();

	for (WORD i = 0; i < numCellsInLayout; ++i)
	{
		if ( NULL == (*pCurrLayout)[i] || (*pCurrLayout)[i]->isBlanked())
			continue;

		if ((*pCurrLayout)[i]->isForced())
		{
			bool forceFound = false;
			(*pCurrLayout)[i]->SetForcedNotFoundCell(false);

			const char* pForcedToSee = (*pCurrLayout)[i]->GetPartyForce();
			if (pForcedToSee && strlen(pForcedToSee))
			{
				CBridgePartyCntl* pPartyCntl = const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(pForcedToSee);
				if (pPartyCntl)
				{
					CPartyImageVector::iterator _ii = std::find(rImagesReadyForSetting.begin(), rImagesReadyForSetting.end(), pPartyCntl->GetPartyRsrcID());
					if (_ii != rImagesReadyForSetting.end())
					{
						(*pCurrLayout)[i]->SetImageId(*_ii);
						_ii = rImagesReadyForSetting.erase(_ii);
						rNumCellsLeftToFill--;
						forceFound = true;
						continue;
					}
				}
			}

			//If we didn't find the participants that fits the layout cell
			if((isBlankCellEnabledWhenPartyDisconnect || isTelepresenceEP)
				&& false == forceFound && m_pPartyCntl->GetIsPrivateLayout())
			{
				if (isTelepresenceEP)
				{
					FTRACEINTO << "ITP (MLA?) forced not found";
				}

				(*pCurrLayout)[i]->SetForcedNotFoundCell(true);
				rNumCellsLeftToFill--;
			}
		}
	}
}

//--------------------------------------------------------------------------
void CLayoutHandler::RemoveSelfImageFromImagesReadyForSetting(CPartyImageVector& rImagesReadyForSetting)
{
	PASSERT_AND_RETURN(!m_pVideoCntl);
	CPartyImageVector::iterator _ii = std::find(rImagesReadyForSetting.begin(), rImagesReadyForSetting.end(), m_pPartyCntl->GetPartyRsrcID());

	if (_ii != rImagesReadyForSetting.end())
		rImagesReadyForSetting.erase(_ii);

	if( m_pPartyCntl->IsAVMCUParty())
	{
		CAVMCUMngr* pAVMCUMngr = m_pVideoCntl->GetAVMCUMngr();
		PASSERT_AND_RETURN(!pAVMCUMngr);
		CAVMCULinksVector& AVMCULinksVector = pAVMCUMngr->GetMCULinksVector();
		CAVMCULinksVector::iterator _jj =  AVMCULinksVector.begin();
        for(_jj; _jj != AVMCULinksVector.end();_jj++)
        {
        	DWORD linkMCURsrcPartyID = *_jj;
            _ii = std::find(rImagesReadyForSetting.begin(), rImagesReadyForSetting.end(), linkMCURsrcPartyID);
            if (_ii != rImagesReadyForSetting.end())
            	rImagesReadyForSetting.erase(_ii);
        }
	}
}
//--------------------------------------------------------------------------
bool CLayoutHandler::IsVectorImagesReadyForSettingContainsAVMCULinksOnly(CPartyImageVector& rImagesReadyForSetting)
{
	bool IsVectorImagesReadyForSettingContainsAVMCULinksOnly = false;
	if(!m_pPartyCntl->IsAVMCUParty())
		return IsVectorImagesReadyForSettingContainsAVMCULinksOnly;

	CAVMCUMngr* pAVMCUMngr = m_pVideoCntl->GetAVMCUMngr();
	if(!pAVMCUMngr)
	{
		PASSERT(1);
		return IsVectorImagesReadyForSettingContainsAVMCULinksOnly;
	}
	CAVMCULinksVector& AVMCULinksVector = pAVMCUMngr->GetMCULinksVector();
	CAVMCULinksVector::iterator _jj =  AVMCULinksVector.begin();
	CPartyImageVector::iterator _ii  = rImagesReadyForSetting.begin();
	for(_ii;_ii != rImagesReadyForSetting.end();_ii++)
	{
		PartyRsrcID partyRsrcID = *_ii;
		_jj = std::find(AVMCULinksVector.begin(), AVMCULinksVector.end(), partyRsrcID );
		if(_jj == AVMCULinksVector.end())
		{
			return IsVectorImagesReadyForSettingContainsAVMCULinksOnly;
		}
	}
    // No RMX EP was found in rImagesReadyForSetting
	ON(IsVectorImagesReadyForSettingContainsAVMCULinksOnly);
	return IsVectorImagesReadyForSettingContainsAVMCULinksOnly;
}


//--------------------------------------------------------------------------
void CLayoutHandler::SetSpeakerImageToBigCellInLayout(CPartyImageVector& rImagesReadyForSetting, WORD& rNumCellsLeftToFill)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);
	PASSERT_AND_RETURN(!(*pCurrLayout)[0]);

	// first place in rImagesReadyForSetting is either the speaker or the last speaker that has a valid image
	if (!(*pCurrLayout)[0]->isBlanked() && (*pCurrLayout)[0]->noImgSet())
	{
		CPartyImageVector::iterator _ii = rImagesReadyForSetting.begin();
		if (_ii != rImagesReadyForSetting.end())
		{
			(*pCurrLayout)[0]->SetImageId(*_ii);
			rImagesReadyForSetting.erase(_ii);
			rNumCellsLeftToFill--;
		}
	}
}

//--------------------------------------------------------------------------
void CLayoutHandler::SetSpeakerImageToBigCellInLayout2Plus8IfNeeded(CPartyImageVector& rImagesReadyForSetting, CLayout* pPreviouselySeenLayout, WORD& rNumCellsLeftToFill)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	std::ostringstream msg;
	msg << "\n  PartyName         :" << m_pPartyCntl->GetName();

	WORD newSpeakerPlaceInCurrLayout = 0;
	WORD oldSpeakerPlaceInCurrLayout = 0;

	WORD oldSpeakerPlaceInPrevLayout = 0;
	WORD newSpeakerPlaceInPrevLayout = 0;

	CPartyImageVector::iterator _iiNewSpeaker = rImagesReadyForSetting.begin();
	if (_iiNewSpeaker != rImagesReadyForSetting.end())
	{
		msg << "\n  new_speaker_id    :" << *_iiNewSpeaker;
		newSpeakerPlaceInPrevLayout = pPreviouselySeenLayout->FindImagePlaceInLayout(*_iiNewSpeaker);
	}
	else
	{
		PASSERT_AND_RETURN(1);
	}

	// Old Speaker place in previous layout can be in the 2 big cells in the middle or not in layout (audio only)
	CPartyImageVector::iterator _iiOldSpeaker = _iiNewSpeaker+1;
	if (_iiOldSpeaker != rImagesReadyForSetting.end())
	{
		msg << "\n  old_speaker_id    :" << *_iiOldSpeaker;
		oldSpeakerPlaceInPrevLayout = pPreviouselySeenLayout->FindImagePlaceInLayout(*_iiOldSpeaker);
	}

	msg << "\n  new_speaker place :" << newSpeakerPlaceInPrevLayout << " (in previous layout)";
	msg << "\n  old_speaker_place :" << oldSpeakerPlaceInPrevLayout << " (in previous layout)";

	//Put the new speaker next to the old speaker (avoid change layout)
	if (oldSpeakerPlaceInPrevLayout == 0)
		newSpeakerPlaceInCurrLayout = 1;

	msg << "\n  new_speaker_place :" << newSpeakerPlaceInCurrLayout;

	if (newSpeakerPlaceInPrevLayout == 0 || newSpeakerPlaceInPrevLayout == 1)
	{
		oldSpeakerPlaceInCurrLayout = newSpeakerPlaceInPrevLayout ^ 1;
		msg << "\n  old_speaker_place :" << oldSpeakerPlaceInCurrLayout;

		CVidSubImage* pVidSubImageOldSpeaker = (*pPreviouselySeenLayout)[oldSpeakerPlaceInCurrLayout];
		if (pVidSubImageOldSpeaker)
		{
			DWORD partyRscId = pVidSubImageOldSpeaker->GetImageId();
			if (partyRscId)
			{
				// check if image that was in big cell in old layout is still available
				CPartyImageVector::iterator _ii = std::find(rImagesReadyForSetting.begin(), rImagesReadyForSetting.end(), partyRscId);

				// image that was in big cell in old layout is not available (disconnected / forced ...)
				// try to set another image at the big cell
				if (_ii == rImagesReadyForSetting.end())
				{
					msg << "\n  new speaker is already in big cell, but old speaker is not available (disconnected or muted)";
					if (_iiOldSpeaker != rImagesReadyForSetting.end())
					{
						if ((*pCurrLayout)[oldSpeakerPlaceInCurrLayout] && !(*pCurrLayout)[oldSpeakerPlaceInCurrLayout]->isBlanked() && (*pCurrLayout)[oldSpeakerPlaceInCurrLayout]->noImgSet())
						{
							msg << "\n  --> Set old_speaker_id:" << *_iiOldSpeaker << " to cell: " << oldSpeakerPlaceInCurrLayout;
							(*pCurrLayout)[oldSpeakerPlaceInCurrLayout]->SetImageId(*_iiOldSpeaker);
							rImagesReadyForSetting.erase(_iiOldSpeaker);
							rNumCellsLeftToFill--;
						}
					}
				}
				else
				{
					msg << "\n  Layout is 2+8 and new speaker is already in big cell, so do nothing";
				}
			}
		}
	}
	else if ((*pCurrLayout)[newSpeakerPlaceInCurrLayout] && !(*pCurrLayout)[newSpeakerPlaceInCurrLayout]->isBlanked() && (*pCurrLayout)[newSpeakerPlaceInCurrLayout]->noImgSet())
	{
		rNumCellsLeftToFill--; //any case we exit the function with --

		if (YES == IsContentImageNeedToBeAdded() && NULL != (*pCurrLayout)[0])
		{
			msg << "\n  --> Set content image in cell: 0";
			CImage* pContentImage = ((CVideoBridgeCPContent*)m_pVideoCntl)->GetContentImage();
			if (pContentImage)
				(*pCurrLayout)[0]->SetImageId(pContentImage->GetArtPartyId());

			//Remove extra(last) image from itrImagesReadyForSetting
			//In Legacy Handler we can be here with extra party (x+1)to fill x cells that left compared to regular Handler
			//To avoid this different that cause the speaker NOT to be seen in the layout if it wasn`t there before since
			//from this stage we are filling the layout from the end of the list
			if (rNumCellsLeftToFill < rImagesReadyForSetting.size())
				rImagesReadyForSetting.erase((rImagesReadyForSetting.begin()+rNumCellsLeftToFill));
		}
		else
		{
			msg << "\n  --> Set new_speaker_id:" << *_iiNewSpeaker << " to cell: " << newSpeakerPlaceInCurrLayout;
			(*pCurrLayout)[newSpeakerPlaceInCurrLayout]->SetImageId(*_iiNewSpeaker);
			rImagesReadyForSetting.erase(_iiNewSpeaker);
		}
	}
	else
	{
		msg << "\n  The cell " << newSpeakerPlaceInCurrLayout << " is blanked or forced, so try to set in cell " << (newSpeakerPlaceInCurrLayout ^ 1);
		newSpeakerPlaceInCurrLayout = newSpeakerPlaceInCurrLayout ^ 1;

		if ((*pCurrLayout)[newSpeakerPlaceInCurrLayout] && !(*pCurrLayout)[newSpeakerPlaceInCurrLayout]->isBlanked() && (*pCurrLayout)[newSpeakerPlaceInCurrLayout]->noImgSet())
		{
			msg << "\n  --> Set new_speaker_id:" << *_iiNewSpeaker << " to cell: " << newSpeakerPlaceInCurrLayout;
			(*pCurrLayout)[newSpeakerPlaceInCurrLayout]->SetImageId(*_iiNewSpeaker);
			rImagesReadyForSetting.erase(_iiNewSpeaker);
			rNumCellsLeftToFill--;
		}
		else
		{
			msg << "\n  The cell " << newSpeakerPlaceInCurrLayout << " is blanked or forced too, so do not set speaker image";
		}
	}
	TRACEINTO << msg.str().c_str();
}

//--------------------------------------------------------------------------
void CLayoutHandler::SetImagesToPreviouseSeenCellsInLayout(CPartyImageVector& rImagesReadyForSetting, CLayout* pPreviouselySeenLayout, WORD& rNumCellsLeftToFill)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	CPartyImageVector::iterator _ii = rImagesReadyForSetting.begin();
	while (_ii != rImagesReadyForSetting.end())
	{
		BYTE whereWasSeen = pPreviouselySeenLayout->FindImagePlaceInLayout(*_ii);
		if (whereWasSeen != AUTO)
		{
			if ((*pCurrLayout)[whereWasSeen] && (*pCurrLayout)[whereWasSeen]->noImgSet())
			{
				(*pCurrLayout)[whereWasSeen]->SetImageId(*_ii);
				_ii = rImagesReadyForSetting.erase(_ii);
				rNumCellsLeftToFill--;
				continue;
			}
		}
		_ii++;
	}
}

//--------------------------------------------------------------------------
void CLayoutHandler::SetImagesToAnyUnusedCell(CPartyImageVector& rImagesReadyForSetting)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	CPartyImageVector::iterator _ii = rImagesReadyForSetting.begin();

	BOOL isManagedByMLA =  !(m_pVideoCntl->GetManageTelepresenceLayoutsInternally()) && m_pVideoCntl->GetTelepresenceOnOff();

	while (_ii != rImagesReadyForSetting.end())
	{
		//Bridge - 8015 MLA sent forced image that isn't in the images ready for settings vector and of the same room
		if (isManagedByMLA && IsImageInSameRoom(*_ii))
		{
			TRACEINTO << "Skipping image #" << (DWORD)*_ii << " in vector - same room id (" << m_pPartyCntl->GetTelepresenceInfo().GetRoomID() << ")";
			++_ii;
			continue;
		}

		WORD unUsedCell = pCurrLayout->FindFirstUnUsedCell();
		if (unUsedCell != AUTO)
		{
			if ((*pCurrLayout)[unUsedCell])
			{
				(*pCurrLayout)[unUsedCell]->SetImageId(*_ii);
				_ii = rImagesReadyForSetting.erase(_ii);
			}
			else
			{
				_ii++;
			}
		}
		else
		{
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::SetSelfView(void)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	bool isAVMCUParty =  m_pPartyCntl->IsAVMCUParty();
	CAVMCUMngr* pAVMCUMngr = NULL;
	PartyRsrcID partyRsrcID = m_pPartyCntl->GetPartyRsrcID();


	WORD numbSubImages = pCurrLayout->GetNumberOfSubImages();
	TRACEINTO << "PartyId:" << partyRsrcID  << ", numbSubImages:"<< numbSubImages;

	if(isAVMCUParty)
	{
		CAVMCUMngr* pAVMCUMngr = m_pVideoCntl->GetAVMCUMngr();
		PASSERT_AND_RETURN(!pAVMCUMngr);
		partyRsrcID = pAVMCUMngr->GetMasterPartyRsrcID();
		TRACEINTO << "AV MCU Party, PartyId: " << m_pPartyCntl->GetPartyRsrcID()  << ", numbSubImages:"<< numbSubImages;
		if (m_pPartyCntl->GetIsPrivateLayout())
		{
			CVidSubImage* pVidSubImage_0 = (*pCurrLayout)[0];
			if (pVidSubImage_0)
			{
				pVidSubImage_0->SetForceAttributes(OPERATOR_Prior, BLANK_PRIVATE_PARTY_Active);
				TRACEINTO << "AV MCU Party, Set Image 0 in private layout to blank,  party Name: " << m_pPartyCntl->GetFullName();
			}
		}

	}

	//In case of private layout or same layout:
	//If the party is forced in one of its cells -> we set its image
	//to the forced cell and not to cell number 0.
	WORD SubImageNumber = 0;
	CVidSubImage* pAudActivSubImage = pCurrLayout->GetSubImageOfForcedParty(m_pPartyCntl->GetName(), SubImageNumber);
	if (CPObject::IsValidPObjectPtr(pAudActivSubImage) && (m_pPartyCntl->GetIsPrivateLayout() || m_pVideoCntl->IsSameLayout()))
	{
		TRACEINTO << "Party is forced, we set the self view to cell:" << SubImageNumber;
		pAudActivSubImage->SetImageId(partyRsrcID);
		return;
	}

	//The party is not forced.
	for(WORD i=0;i<numbSubImages;i++)
	{
		pAudActivSubImage = (*pCurrLayout)[i];
		if (!pAudActivSubImage) {
			PASSERT_AND_RETURN(i+1);
		}

		if (!m_pPartyCntl->GetIsPrivateLayout()) // private layout
		{
			TRACEINTO << "Conference force active in Private layout, index:" << i;
			if (!(pAudActivSubImage->isBlanked()))
			{
				if (!(pAudActivSubImage->isForcedInConfLevel()) || !(pAudActivSubImage->isForcedInPartLevel()))
				{
					pAudActivSubImage->SetImageId(partyRsrcID);
					break;
				}
			}
		}
		else
		{
			TRACEINTO << "Conference force not active in Private layout, index:" << i;
			if (!(pAudActivSubImage->isBlanked()))
			{
				if( !(pAudActivSubImage->isForcedInPartLevel()) )
				{
					pAudActivSubImage->SetImageId(partyRsrcID);
					break;
				}
				else if ( pAudActivSubImage->isForcedInPartLevel() && (strcmp(pAudActivSubImage->GetPartyForce(),m_pPartyCntl->GetName()) != 0))
				{
					// This is in case there is a force but it's not this.
					break;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
// This function receives the next image to scan from m_pAutoScanParams and add it to the layout
void CLayoutHandler::FillAutoScanImageInLayout(CLayout& rResultLayout, BYTE needToStartTimer)
{
	m_AutoScanTimeOut = 0;

	std::ostringstream msg;
	msg << "PartyName:" << m_pPartyCntl->GetName() << ", PartyId:" << m_pPartyCntl->GetPartyRsrcID();
	WORD auto_scan_cell = rResultLayout.GetAutoScanCell();
	if (auto_scan_cell != AUTO)
	{
		// VNGR-25729 fix - Oleg
		// Find next synchronized image in auto-scan vector
		DWORD partyRscId = 0;
		DWORD imagesVectorAutoScanSize = GetImagesVectorAutoScanSize();
		CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
		for (DWORD i = 0; i < imagesVectorAutoScanSize; ++i)
		{
			partyRscId = m_pAutoScanParams->GetNextPartyImageId();
			if (partyRscId)
			{
				CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
				if (pImage && !pImage->GetOutOfSync()) // EP image synced
					break;
			}
			partyRscId = 0;
		}

		if (!partyRscId) // EP image synced
		{
			TRACEINTO << msg.str().c_str() << ", Failed - all EP's are out of sync";
			return;
		}
		// VNGR-25729 - end fix

		msg << " will see image of PartyId:" << partyRscId << " at cell index:" << auto_scan_cell;
		if (NULL != rResultLayout[auto_scan_cell])
			rResultLayout[auto_scan_cell]->SetImageId(partyRscId);
		else
			DBGPASSERT(1);

		if (needToStartTimer)
		{
			// turn on a flag, the timer interval is taken from the bridge (timer is being set at conf level)
			CVideoBridgeAutoScanParams* pBridgeAutoScanParams = m_pVideoCntl->GetVideoBridgeAutoScanParams();
			if (IsValidPObjectPtr(pBridgeAutoScanParams))
			{
				m_AutoScanTimeOut = pBridgeAutoScanParams->GetTimerInterval();
				msg << "\nStart AUTO_SCAN_TIMER timer with interval " << m_AutoScanTimeOut << " seconds";
			}
			else
			{
				msg << "\nFunction Failed - invalid pointer, pBridgeAutoScanParams=NULL";
			}
		}
		else
		{
			msg << "\nNo need to start timer - only one image to scan";
		}
	}
	else
	{
		msg << "\nAuto scan cell is not in layout - do nothing";
	}
	PTRACE(eLevelInfoNormal, msg.str().c_str());
}

////////////////////////////////////////////////////////////////////////////
// This func receives the new scan order (from EMA / API) and fill m_pAutoScanParams accordingly
void CLayoutHandler::SetAutoScanOrder(CAutoScanOrder* pAutoScanOrder)
{
	if (m_pAutoScanParams)
		m_pAutoScanParams->InitScanOrder(pAutoScanOrder);
}

////////////////////////////////////////////////////////////////////////////
BYTE CLayoutHandler::IsLayoutWithSpeakerPicture(const LayoutType layoutType)
{
	return isLayoutWithSpeakerPicture(layoutType);
}

////////////////////////////////////////////////////////////////////////////
BYTE CLayoutHandler::IsContentImageNeedToBeAdded()
{
	return NO;
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::BuildLayoutCOP(BYTE &rIsLayoutChanged)
{
	rIsLayoutChanged = FALSE;

	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	//~~ Create copy of current layout before changing it
	CLayout* pPreviouseLayout = new CLayout(*pCurrLayout);

	//~~ Get party current layout type
	LayoutType newLayoutType = m_pPartyCntl->GetPartyCurrentLayoutType();
	PASSERT_AND_RETURN(newLayoutType >= CP_NO_LAYOUT);

	pCurrLayout->SetLayoutType(newLayoutType);

	FillLayoutCOP();

	if(*pPreviouseLayout != *pCurrLayout)
		rIsLayoutChanged = TRUE;

	POBJDELETE(pPreviouseLayout);
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::FillLayoutCOP()
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	WORD numCellsInLayout = pCurrLayout->GetNumberOfSubImages();
	PASSERT_AND_RETURN(numCellsInLayout == 0);

	// stage 1 ~~ Create copy of current layout before changing it
	CLayout previouseLayout(*pCurrLayout);

	// stage 2 ~~ We always Fill something that is clean
	pCurrLayout->ClearAllImageSources();

	WORD numConnectedImages = m_pVideoCntl->GetPartyImageVectorSize();
	if (numConnectedImages == 0) //may be the case if no decoder is connected in conf
		return;

	// stage 3 ~~ Images that are not muted are inserted to Vector imagesReadyForSetting
	CPartyImageVector imagesReadyForSetting;
	CreateVectorOfImagesReadyForSetting(imagesReadyForSetting);

	// stage 4 ~~ check that there exist Images for setting - skipped

	// stage 5 ~~ if there is one valid image and it is own image SetSelfView
	if ((imagesReadyForSetting.size() == 1) && (imagesReadyForSetting[0] == m_pPartyCntl->GetPartyRsrcID()))
	{
		SetSelfView();
		return;
	}

	// stage 6 ~~~ initiate numCellsLeft (= cells in layout that are not blanked)
	WORD numCellsLeft = 0;
	for (DWORD i = 0; i < numCellsInLayout; i++)
		if (NULL != (*pCurrLayout)[i] && !((*pCurrLayout)[i]->isBlanked()))
			numCellsLeft++;

	if (numCellsLeft == 0)
	{ //all cells in layout are blanked
		return;
	}

	// stage 7 ~~~ Set forced parties
	SetForcedPartiesInLayout(imagesReadyForSetting, numCellsLeft);

	// stage 8 ~~~ Remove Self Image from imagesReadyForSetting Array
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
		RemoveSelfImageFromImagesReadyForSetting(imagesReadyForSetting);

	// stage 9 ~~~ Remove from imagesReadyForSetting images that can not enter layout because speaker order
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
		if (numCellsLeft < imagesReadyForSetting.size())
			imagesReadyForSetting.erase((imagesReadyForSetting.begin()+numCellsLeft), imagesReadyForSetting.end());

	// stage 10 ~~~ In Layout with Speaker Picture try to set speaker in to first cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
		if (IsLayoutWithSpeakerPicture(pCurrLayout->GetLayoutType()))
			SetSpeakerImageToBigCellInLayout(imagesReadyForSetting, numCellsLeft);

	// stage 11 ~~~ Try to set the remaining images in their previous cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
		SetImagesToPreviouseSeenCellsInLayout(imagesReadyForSetting, &previouseLayout, numCellsLeft);

	// stage 12 ~~~ Put the remaining images in any unused cell
	if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
		SetImagesToAnyUnusedCell(imagesReadyForSetting);
}

////////////////////////////////////////////////////////////////////////////
bool CLayoutHandler::BuildLayoutIndications(CLayout& oLayout)
{
	EIconType recordingType = m_pPartyCntl->GetRecordingType();
	WORD isAudioParticipantsEnabled = m_pPartyCntl->GetIsAudioParticipantsIconActive();


	if(FALSE == m_pPartyCntl->IsPartyValidForLayoutIndication())
	{
		isAudioParticipantsEnabled = FALSE;
		recordingType = E_ICON_REC_OFF;
		TRACEINTO<<"PartyId: "<< m_pPartyCntl->GetPartyRsrcID()<< ",layout indication(audio + recording) is deactived.";
	}

	if(m_pPartyCntl->isInGatheringMode())
	{
		isAudioParticipantsEnabled = FALSE;
		TRACEINTO<<"PartyId: "<< m_pPartyCntl->GetPartyRsrcID()<< ",audio icon is deactived for gathering mode.";
	}


	const size_t nSubImages = oLayout.GetNumberOfSubImages();
	CLayoutIndications& oIndications = oLayout.indications();

	const CImage* pImage = m_pPartyCntl->GetPartyImage();

	bool bChanged = false;

	//1.Recording Icon
	//EIconType recordingType = m_pPartyCntl->GetRecordingType();
	bChanged |= oIndications.indication(eIconRecording).on(recordingType);
	if(bChanged)
		TRACEINTO << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << ", RecordingType Changed to:" << EIconRecordingTypeNames[recordingType];
	if(E_ICON_REC_OFF == recordingType)
		oIndications.indication(eIconRecording).off();


	//2. audio participants
	//WORD isAudioParticipantsEnabled = m_pPartyCntl->GetIsAudioParticipantsIconActive();
	if(isAudioParticipantsEnabled)
	{
		WORD  numAudioParticipantsInConf = m_pPartyCntl->GetNumAudioParticipantsInConf();
		if(0 == numAudioParticipantsInConf)
		{
			bChanged |= (!(oIndications.indication(eIconAudioPartiesCount).isActive()));
			bChanged |= oIndications.indication(eIconAudioPartiesCount).on(numAudioParticipantsInConf);
			//The audio participants number is 0, on()function won't active the icon.
			oIndications.indication(eIconAudioPartiesCount).active();
		}
		else
		{
			bChanged |= oIndications.indication(eIconAudioPartiesCount).on(numAudioParticipantsInConf);
		}
		TRACEINTO << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() <<
					" - audio participants icon is enabled, data:" << numAudioParticipantsInConf;
	}
	else
	{
		bChanged |= oIndications.indication(eIconAudioPartiesCount).isActive();
		oIndications.indication(eIconAudioPartiesCount).off();
		TRACEINTO << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << " - audio participants icon is disabled";
	}



	//3. Network quality
	if (pImage)
		bChanged |= oIndications.indication(eIconNetworkQuality).on(pImage->networkQualityPerLayout().queryForDisplay());
	else
		TRACEINTO << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << " - Invalid party image";

	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();

	for (size_t i = 0; i < nSubImages; ++i)
	{
		pImage = NULL;
		CVidSubImage* pVidSubImage = oLayout[i];

		if (!pVidSubImage)
		{
			TRACEINTO << "Warning, Invalid pVidSubImage, index:" << i;
			continue;
		}

		DWORD partyRscId = pVidSubImage->GetImageId();
		if (partyRscId)
		{
			pImage = pPartyImageLookupTable->GetPartyImage(partyRscId);
			PASSERTSTREAM(!pImage, "Failed, The lookup table doesn't have an element, PartyId:" << partyRscId);
		}

		if (!pImage || pVidSubImage->isBlanked())
			continue;

		// we do not want here short-circuit evaluation, so the '|=' operator is OK.
		bChanged |= oIndications.indication(eIconNetworkQuality, i).on(pImage->networkQualityPerCell().queryForDisplay());
	}

	return bChanged;
}


////////////////////////////////////////////////////////////////////////////
void CLayoutHandler::DumpImagesVectorAutoScan(std::ostringstream& msg)
{
	if (m_pAutoScanParams != NULL)
		m_pAutoScanParams->DumpImagesVector(msg);
	else
		msg << "\n  AutoScanParams :NULL";
}

/////////////////////////////////////////////////////////////////////////////
DWORD CLayoutHandler::GetImagesVectorAutoScanSize()
{
	return m_pAutoScanParams->GetAutoScanVectorSize();
}
////////////////////////////////////////////////////////////////////////////
BOOL CLayoutHandler::IsBridgePartyCntlValid(CBridgePartyCntl*pPartyCntl)
{
	if (!IsValidPObjectPtr(pPartyCntl))
		return FALSE;

	if (pPartyCntl->GetState() != CVideoBridgePartyCntl::CONNECTED)
		return FALSE;

	if (!const_cast<CVideoBridge*>(m_pVideoCntl)->IsPartyImageExistInImageVector(pPartyCntl->GetPartyRsrcID()))
		return FALSE;

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////
BOOL CLayoutHandler::IsBlankCelINeedToBeForcedInITPLayout(CLayout* pCurrLayout, const char* partyName, WORD cellId, BOOL bIsPrivate)
{
	if (pCurrLayout->GetLayoutType() == CP_LAYOUT_OVERLAY_ITP_1P2 ||
	    pCurrLayout->GetLayoutType() == CP_LAYOUT_OVERLAY_ITP_1P3 ||
	    pCurrLayout->GetLayoutType() == CP_LAYOUT_OVERLAY_ITP_1P4)
	{
		string str_PartyName = partyName;
		vector<string> key_list;
		key_list.push_back("_1");
		key_list.push_back("_2");
		key_list.push_back("_3");
		key_list.push_back("_4");
		size_t found = string::npos;
		for (unsigned int t = 0; t < key_list.size(); t++)
		{
			found = str_PartyName.find(key_list[t]);
			if (found != string::npos)
				break;
		}

		if (found != string::npos && cellId > 0)
		{
			CBridgePartyCntl* pPartyCntl = const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(partyName);

			// printf("partyName %s pPartyCntl->GetState() %d \n",partyName,pPartyCntl->GetState() );
			if (!IsBridgePartyCntlValid(pPartyCntl))
			{
				// printf(" cell %d ,  %s has been forced to blank..\n",cellId,partyName);
				TRACEINTO << "PartyName:" << partyName << ", cellId:" << cellId<< " - Has been forced to blank";
				if ((*pCurrLayout)[cellId])
				{
					if (bIsPrivate)
						(*pCurrLayout)[cellId]->SetForceAttributes(OPERATOR_Prior, BLANK_PRIVATE_PARTY_Active);
					else
						(*pCurrLayout)[cellId]->SetForceAttributes(OPERATOR_Prior, BLANK_PARTY_Activ);
				}
				else
					PASSERT(1);

				return TRUE;
			}
		}
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////////
//                        CLayoutHandlerVSW
////////////////////////////////////////////////////////////////////////////
CLayoutHandlerVSW::CLayoutHandlerVSW(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl)
                  :CLayoutHandler(videoCntl, partyCntl)
{
}

////////////////////////////////////////////////////////////////////////////
CLayoutHandlerVSW::CLayoutHandlerVSW(const CLayoutHandlerVSW* layout)
                  :CLayoutHandler(layout)
{
}
////////////////////////////////////////////////////////////////////////////
bool CLayoutHandlerVSW::BuildLayout()
{
	PASSERT_AND_RETURN_VALUE(!m_pPartyCntl, false);
	PASSERT_AND_RETURN_VALUE(!m_pVideoCntl, false);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN_VALUE(!pCurrLayout, false);

	bool IsLayoutChanged = false;

	// get new layout from bridge
	CLayout* pConfLayout = NULL;
	if (m_pPartyCntl == ((CVideoBridgeVSW*)m_pVideoCntl)->GetConfSource())
	{
		TRACEINTO << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << " - Party is conference video source";
		pConfLayout = ((CVideoBridgeVSW*)m_pVideoCntl)->GetConfSourceLayout();
	}
	else
	{
		TRACEINTO << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << " - Party is not conference video source";
		pConfLayout = ((CVideoBridgeVSW*)m_pVideoCntl)->GetConfLayout();
	}

	if (pConfLayout == NULL)
	{
		TRACEWARN << "Conference layout is NULL";
	}
	else
	{
		CVidSubImage* pConfSubImage0 = (*pConfLayout)[0];
		CVidSubImage* pCurrSubImage0 = (*pCurrLayout)[0];

		DWORD confPartyRscIdInSubImage0 = pConfSubImage0 ? pConfSubImage0->GetImageId() : 0;
		DWORD currRartyRscIdInSubImage0 = pCurrSubImage0 ? pCurrSubImage0->GetImageId() : 0;

		if (confPartyRscIdInSubImage0)
		{
			if (*pCurrLayout != *pConfLayout)
			{
				TRACEINTO << "Current layout changed, update layout";
				IsLayoutChanged = true;
				POBJDELETE(pCurrLayout);
				pCurrLayout = new CLayout(*pConfLayout);
			}
			else
			{
				TRACEINTO << "Current layout did not changed";
			}
		}
		else if (currRartyRscIdInSubImage0 == 0 && pCurrSubImage0 != NULL)
		{
			// set self view if image is NULL
			IsLayoutChanged = true;
			TRACEINTO << "Image is NULL, so set the self view";
			pCurrSubImage0->SetImageId(m_pPartyCntl->GetPartyRsrcID());
		}
		else
		{
			TRACEINTO << "Conference image is NULL, so do nothing";
		}
	}

	return (IsLayoutChanged);
}


////////////////////////////////////////////////////////////////////////////
//                        CLayoutHandlerLegacy
////////////////////////////////////////////////////////////////////////////
CLayoutHandlerLegacy::CLayoutHandlerLegacy(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl)
                     :CLayoutHandler(videoCntl, partyCntl)
{
}

////////////////////////////////////////////////////////////////////////////
CLayoutHandlerLegacy::CLayoutHandlerLegacy(const CLayoutHandlerLegacy* layout)
                     :CLayoutHandler(layout)
{
}

////////////////////////////////////////////////////////////////////////////
BYTE CLayoutHandlerLegacy::IsContentImageNeedToBeAdded()
{
	return ((CVideoBridgeCPContent*)m_pVideoCntl)->IsContentImageNeedToBeAdded();
}

////////////////////////////////////////////////////////////////////////////
BYTE CLayoutHandlerLegacy::IsLayoutWithSpeakerPicture(const LayoutType layoutType)
{
	BYTE isWithSpeaker = NO;

	if (YES == IsContentImageNeedToBeAdded())
		isWithSpeaker = YES; //in Legacy Mode if content needed to be shown it should appears in square0 for any layout
	else
		isWithSpeaker = CLayoutHandler::IsLayoutWithSpeakerPicture(layoutType);

	return isWithSpeaker;
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerLegacy::SetForcesForListener()
{
	if (YES == IsContentImageNeedToBeAdded())
	{
		PASSERT_AND_RETURN(!m_pPartyCntl);
		PASSERT_AND_RETURN(!m_pVideoCntl);

		CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
		PASSERT_AND_RETURN(!pCurrLayout);

		LayoutType layoutType = pCurrLayout->GetLayoutType();
		PASSERT_AND_RETURN( layoutType >= CP_NO_LAYOUT);

		const char* pLectureName = m_pVideoCntl->GetLecturerName();
		PASSERT_AND_RETURN(!pLectureName);

		BYTE rs = strcmp(pLectureName,"");
		DBGPASSERT_AND_RETURN(rs == 0);

		TRACEINTO << "PartyId:" << m_pPartyCntl->GetPartyRsrcID() << ", LecturerName:" << pLectureName;

		PASSERT_AND_RETURN(!(*pCurrLayout)[1]);
		if (layoutType != CP_LAYOUT_1X1)
		{
			(*pCurrLayout)[1]->SetPartyForceName(pLectureName);
			(*pCurrLayout)[1]->SetForceAttributes(OPERATOR_Prior, FORCE_CONF_Activ);
			const WORD numCellsInLayout = pCurrLayout->GetNumberOfSubImages();
			for(DWORD i = 2; i < numCellsInLayout; i++) // the first two cells are for content and for lecturer
			{
			if((*pCurrLayout)[i])
			{
					(*pCurrLayout)[i]->SetForceAttributes(OPERATOR_Prior, BLANK_PARTY_Activ);
					(*pCurrLayout)[i]->RemovePartyForceName();
			}
			}
		}
		return;
	}
	CLayoutHandler::SetForcesForListener();
}

//--------------------------------------------------------------------------
void CLayoutHandlerLegacy::SetSpeakerImageToBigCellInLayout(CPartyImageVector& rImagesReadyForSetting, WORD& rNumCellsLeftToFill)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	// first place in rImagesReadyForSetting is either the speaker or the last speaker that has a valid image
	if ((*pCurrLayout)[0] && !(*pCurrLayout)[0]->isBlanked() && (*pCurrLayout)[0]->noImgSet())
	{
		rNumCellsLeftToFill--; // any case we exit the function with --

		if (YES == IsContentImageNeedToBeAdded())
		{
			CImage* pContentImage = (CImage*)(((CVideoBridgeCPContent*)m_pVideoCntl)->GetContentImage());
			if (pContentImage)
				(*pCurrLayout)[0]->SetImageId(pContentImage->GetArtPartyId());

			// Remove extra(last) image from itrImagesReadyForSetting
			// In Legacy Handler we can be here with extra party (x+1)to fill x cells that left compared to regular Handler
			// To avoid this different that cause the speaker NOT to be seen in the layout if it wasn`t there before since
			// from this stage we are filling the layout from the end of the list
			if (rNumCellsLeftToFill < rImagesReadyForSetting.size())
				rImagesReadyForSetting.erase((rImagesReadyForSetting.begin()+rNumCellsLeftToFill));
		}
		else
		{
			CPartyImageVector::iterator _ii = rImagesReadyForSetting.begin();
			if (_ii != rImagesReadyForSetting.end())
			{
				(*pCurrLayout)[0]->SetImageId(*_ii);
				rImagesReadyForSetting.erase(_ii);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerLegacy::FillLayout()
{
	CLayoutHandler::FillLayout();

	//In some cases (for example when only 2 parties in conf and LectureMode and Legacy are on ,we have to set the Lecturer in the 2nd square ==> no parties are left to set any more and in
	//FillLayout we are not entering to SetSpeakerImageToBigCellInLayout since no actual parties left to be set in layout(only the content decoder which is NOT a party in the list.
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	TRACEINTO << "PartyId:" << m_pPartyCntl->GetPartyRsrcID();

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	if (NULL != (*pCurrLayout)[0] && (!(*pCurrLayout)[0]->isBlanked()) && (*pCurrLayout)[0]->noImgSet())
	{
		if (YES == IsContentImageNeedToBeAdded())
		{
			CImage* pContentImage = (CImage*)(((CVideoBridgeCPContent*)m_pVideoCntl)->GetContentImage());
			if(pContentImage)
				(*pCurrLayout)[0]->SetImageId(pContentImage->GetArtPartyId());
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//                        CLayoutHandlerTelepresence
////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerTelepresence::SetBlackScreen(CVideoBridgePartyCntl* pPartyCntl)
{
	PASSERT_AND_RETURN(!pPartyCntl);
	CLayout* pCurrLayout = pPartyCntl->GetCurrentLayout();

	SetBlackScreen(pCurrLayout);
}

void CLayoutHandlerTelepresence::SetBlackScreen(CLayout* pCurrLayout)
{
	PASSERT_AND_RETURN(!pCurrLayout);

	WORD numbSubImages = pCurrLayout->GetNumberOfSubImages();
	for(WORD i=0;i<numbSubImages;i++)
	{
		if (NULL != (*pCurrLayout)[i])
			(*pCurrLayout)[i]->SetForceAttributes(OPERATOR_Prior, BLANK_PARTY_Activ);
	}
}

////////////////////////////////////////////////////////////////////////////
//                        CLayoutHandlerTelepresenceRoomSwitch
////////////////////////////////////////////////////////////////////////////
CLayoutHandlerTelepresenceRoomSwitch::CLayoutHandlerTelepresenceRoomSwitch(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl) :
		CLayoutHandlerTelepresence(videoCntl, partyCntl)
{}

////////////////////////////////////////////////////////////////////////////
CLayoutHandlerTelepresenceRoomSwitch::CLayoutHandlerTelepresenceRoomSwitch(const CLayoutHandlerTelepresenceRoomSwitch* layout) : CLayoutHandlerTelepresence(layout)
{}
////////////////////////////////////////////////////////////////////////////
bool CLayoutHandlerTelepresenceRoomSwitch::BuildLayout()
{
	PASSERT_AND_RETURN_VALUE(!m_pPartyCntl, false);
	PASSERT_AND_RETURN_VALUE(!m_pVideoCntl, false);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN_VALUE(!pCurrLayout, false);

	PTRACE(eLevelInfoNormal, "CLayoutHandlerTelepresenceRoomSwitch::BuildLayout");

	//Create copy of current layout before changing it
	CLayout* pPreviouseLayout = new CLayout(*pCurrLayout);

	///****CHECK IF NEEDED HERE
/*	//Get party current layout type
	LayoutType newLayoutType = m_pPartyCntl->GetPartyCurrentLayoutType();
	if (newLayoutType >= CP_NO_LAYOUT)
	{
	  PASSERT(newLayoutType);
	  return (false);
	}

	pCurrLayout->SetLayoutType(newLayoutType);
*/
	  //Remove old forces and blanks
	  RemoveOldPartForcesAndBlankes();
	  RemoveOldConfForcesAndBlankes();


	FillLayout();

	BuildLayoutIndications(*pCurrLayout);	// ****check if needed????

	bool isLayoutChanged = (*pPreviouseLayout != *pCurrLayout);
	POBJDELETE(pPreviouseLayout);
	return (isLayoutChanged);

}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerTelepresenceRoomSwitch::FillLayout()
{
  PTRACE(eLevelInfoNormal, "CLayoutHandlerTelepresenceRoomSwitch::FillLayout - begin");

  PASSERT_AND_RETURN(!m_pPartyCntl);
  PASSERT_AND_RETURN(!m_pVideoCntl);

  CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
  PASSERT_AND_RETURN(!pCurrLayout);

  WORD numCellsInLayout = pCurrLayout->GetNumberOfSubImages();
  PASSERT_AND_RETURN(numCellsInLayout == 0);

  //stage 2 ~~ We always Fill something that is clean
  pCurrLayout->ClearAllImageSources();

  WORD numConnectedImages = m_pVideoCntl->GetPartyImageVectorSize();
  if (numConnectedImages == 0) //may be the case if no decoder is connected in conference
  {
    PTRACE(eLevelInfoNormal, "CLayoutHandlerTelepresenceRoomSwitch::FillLayout - return");
    return;
  }

  //stage 3 ~~ Images that are not muted are inserted to Vector imagesReadyForSetting
  CPartyImageVector imagesReadyForSetting;
  CreateVectorOfImagesReadyForSetting(imagesReadyForSetting);

  //stage 4 ~~ check that there exist Images for setting
  if (imagesReadyForSetting.size() == 0) //may be the case if no decoder is connected in conf and is unmuted
  {
    SetBlackScreen(m_pPartyCntl);	// in Telepresence we display black screen instead of self view
    PTRACE(eLevelInfoNormal, "CLayoutHandlerTelepresenceRoomSwitch::FillLayout - return");
    return;
  }

  // stage 5 ~~ if there is one valid image and it is own image SetSelfView
  if (((imagesReadyForSetting.size() == 1) && (imagesReadyForSetting[0] == m_pPartyCntl->GetPartyRsrcID())) ||
	  (CheckIfAllImagesReadyForSettingAreInTheSameRoom(imagesReadyForSetting)))
  {
	SetBlackScreen(m_pPartyCntl);	// in Telepresence we display black screen instead of self view
    PTRACE(eLevelInfoNormal, "CLayoutHandler::FillLayout - return");
    return;
  }

  // stage 8 ~~~ Remove Self Image from imagesReadyForSetting Array
  if (imagesReadyForSetting.size() != 0)
    RemoveSelfImageFromImagesReadyForSetting(imagesReadyForSetting);

  //Get required layout type, blanks and Telepresence room layout indexes
  TELEPRESENCE_LAYOUT_INDEXES_S* pTelepresenceLayoutIndexesStruct = GetTelepresenceLayoutIndexesStruct(imagesReadyForSetting);
  PASSERT_AND_RETURN(!pTelepresenceLayoutIndexesStruct);
  //set layoutType
  pCurrLayout->SetLayoutType(pTelepresenceLayoutIndexesStruct->layoutType);


  /* **** Redundant for Telepresence room switch - only one speaker is displayed
  // stage 10 ~~~ Remove from imagesReadyForSetting images that can not enter layout because speaker order
  if (numCellsLeft != 0 && imagesReadyForSetting.size() != 0)
    if (numCellsLeft < imagesReadyForSetting.size())
    {
      imagesReadyForSetting.erase((imagesReadyForSetting.begin()+numCellsLeft), imagesReadyForSetting.end());
    }
  */


  SetImagesByIndexesInLayout(pTelepresenceLayoutIndexesStruct, imagesReadyForSetting);

}

////////////////////////////////////////////////////////////////////////////
TELEPRESENCE_LAYOUT_INDEXES_S* CLayoutHandlerTelepresenceRoomSwitch::GetTelepresenceLayoutIndexesStruct(CPartyImageVector& rImagesReadyForSetting)
{
	PASSERT_AND_RETURN_VALUE(!m_pPartyCntl, NULL);
	PASSERT_AND_RETURN_VALUE(!m_pVideoCntl, NULL);

	// Get destination (current) party's # of screens & position (# screen out of # of screens)
	const CTelepresenseEPInfo& tpInfo = m_pPartyCntl->GetTelepresenceInfo();
	DWORD roomNumOfScreens = tpInfo.GetNumOfLinks();
	DWORD screenPosition = tpInfo.GetLinkNum();
/*
	std::ostringstream msg;
	tpInfo.Dump(msg);
	TRACEINTO << "viewer: " << msg.str().c_str();
*/

	// Get speaker's # of screens
	DWORD speakerImageId = *(rImagesReadyForSetting.begin());
	CVideoBridgePartyCntl* pSpeakerPartyCntl = (CVideoBridgePartyCntl*)(const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(speakerImageId));
	PASSERT_AND_RETURN_VALUE(!IsValidPObjectPtr(pSpeakerPartyCntl), NULL);
	const CTelepresenseEPInfo& speakerTpInfo = pSpeakerPartyCntl->GetTelepresenceInfo();
	DWORD speakerRoomNumOfScreens = speakerTpInfo.GetNumOfLinks();

/*
	std::ostringstream msg1;
	speakerTpInfo.Dump(msg1);
	TRACEINTO << "speaker: " << msg1.str().c_str();
*/

//	TRACEINTO << "***_e_m_DEBUG***  for party - " << m_pPartyCntl->GetPartyRsrcID() << " - : "
//				<< "\nspeakerRoomNumOfScreens: " << speakerRoomNumOfScreens
//				<< "\nroomNumOfScreens: " << roomNumOfScreens
//				<< "\nscreenPosition: " << screenPosition;

	// Get Layout type and indexes list for the current screen (destination)
	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = const_cast<CVideoBridge*> (m_pVideoCntl)->GetTelepresenceLayoutMngr();
	PASSERT_AND_RETURN_VALUE(!pTelepresenceLayoutMngr, NULL);
	CTelepresenceLayoutLogic* pTelepresenceLayoutLogic = pTelepresenceLayoutMngr->GetTelepresenceLayoutLogic();
	PASSERT_AND_RETURN_VALUE(!pTelepresenceLayoutLogic, NULL);

	WORD screenNumKey = pTelepresenceLayoutLogic->CreateScreenNumbersKey(roomNumOfScreens, screenPosition, speakerRoomNumOfScreens);
//	TRACEINTO << "***_e_m_DEBUG*** screenNumKey: " << screenNumKey;
	TELEPRESENCE_LAYOUT_INDEXES_S *pTelepresenceLayoutIndexesStruct = pTelepresenceLayoutLogic->GetScreenLayoutEntity(screenNumKey);
	return pTelepresenceLayoutIndexesStruct;

}

////////////////////////////////////////////////////////////////////////////
BOOL CLayoutHandlerTelepresenceRoomSwitch::CheckIfAllImagesReadyForSettingAreInTheSameRoom(CPartyImageVector& rImagesReadyForSetting)
{
	if (rImagesReadyForSetting.size() > MAX_CASCADED_LINKS_NUMBER)
		return FALSE;

	DWORD currentImageId = (DWORD)(-1);
	DWORD currentImageRoomId = (DWORD)(-1);

	const CTelepresenseEPInfo& tpInfo = m_pPartyCntl->GetTelepresenceInfo();
	DWORD selfRoomId = tpInfo.GetRoomID();

	for (DWORD i = 0; i < rImagesReadyForSetting.size(); i++)
	{
		currentImageId = rImagesReadyForSetting[i];

		CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)(const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(currentImageId));
		PASSERT_AND_RETURN_VALUE(!IsValidPObjectPtr(pPartyCntl), FALSE);

		const CTelepresenseEPInfo& tpInfo = pPartyCntl->GetTelepresenceInfo();
		currentImageRoomId = tpInfo.GetRoomID();

		if (currentImageRoomId != selfRoomId)
			return FALSE;
	}

	return TRUE; // all images are from the same room
}


////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerTelepresenceRoomSwitch::RemoveSelfImageFromImagesReadyForSetting(CPartyImageVector& rImagesReadyForSetting)
{
	// ****Check this function - check that erasing the iterator doesn't influence the search... ****
	TRACEINTO << "***DEBUG***";

	// Remove all self room images
	const CTelepresenseEPInfo& tpInfo = m_pPartyCntl->GetTelepresenceInfo();
	DWORD selfRoomId = tpInfo.GetRoomID();
	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = const_cast<CVideoBridge*> (m_pVideoCntl)->GetTelepresenceLayoutMngr();
	PASSERT_AND_RETURN(!pTelepresenceLayoutMngr);
	CRoomInfo pRoomInfo = pTelepresenceLayoutMngr->GetRoomInfo(selfRoomId);

	DWORD secondaryScreenPartyRsrcId = (DWORD)(-1);

	for (WORD i = 0; i < MAX_CASCADED_LINKS_NUMBER; ++i)
	{
		///if (roomLinksPartyIDs[i] != (DWORD)(-1)) // ****TBD
		secondaryScreenPartyRsrcId = pRoomInfo.GetLinkPartyId(i);
		if (secondaryScreenPartyRsrcId != (DWORD)(-1))
		{
			CPartyImageVector::iterator _ii = rImagesReadyForSetting.begin();
			do{
				_ii = std::find(_ii, rImagesReadyForSetting.end(), secondaryScreenPartyRsrcId);
				if (_ii != rImagesReadyForSetting.end())
					rImagesReadyForSetting.erase(_ii);
			}
			while (_ii != rImagesReadyForSetting.end());
		}
	}
}


////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerTelepresenceRoomSwitch::SetImagesByIndexesInLayout(TELEPRESENCE_LAYOUT_INDEXES_S* pTelepresenceLayoutIndexesStruct, CPartyImageVector& rImagesReadyForSetting)
{
	PASSERT_AND_RETURN(!m_pPartyCntl);
	PASSERT_AND_RETURN(!m_pVideoCntl);

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN(!pCurrLayout);

	WORD numCellsInLayout = pCurrLayout->GetNumberOfSubImages();
	//PASSERT_AND_RETURN(numCellsInLayout > MAX_SUB_IMAGES_IN_LAYOUT);
	if (numCellsInLayout > MAX_SUB_IMAGES_IN_LAYOUT)	//anatg-KW
	{
		DBGPASSERT(numCellsInLayout);
		numCellsInLayout = MAX_SUB_IMAGES_IN_LAYOUT;
	}

	PartyRsrcID speakerMainScreenPartyRsrcId = *(rImagesReadyForSetting.begin());	// main link of speaker EP - the first image in ImagesReadyForSetting list
	// Get speaker's main screen party cntl in order to get to its Telepresence info
	CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)(const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(speakerMainScreenPartyRsrcId));
	PASSERT_AND_RETURN(!IsValidPObjectPtr(pPartyCntl));
	// Get speaker's RoomId
	const CTelepresenseEPInfo& speakerMainScreenTpInfo = pPartyCntl->GetTelepresenceInfo();
	DWORD speakerRoomId = speakerMainScreenTpInfo.GetRoomID();
	// Get speaker's RoomInfo in order to get other screens' partyRsrcIds
	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = const_cast<CVideoBridge*> (m_pVideoCntl)->GetTelepresenceLayoutMngr();
	PASSERT_AND_RETURN(!pTelepresenceLayoutMngr);
	CRoomInfo pSpeakerRoomInfo = pTelepresenceLayoutMngr->GetRoomInfo(speakerRoomId);

	PartyRsrcID screenPartyRsrcId = (DWORD)(-1);
	CPartyImageVector::iterator _ii = rImagesReadyForSetting.end();
	BYTE roomScreenIndex = BLANK_CELL;
	// Go over all cells in layout and set the relevant image according to indexes struct
	for (WORD i = 0; (i < numCellsInLayout && i < MAX_SUB_IMAGES_IN_LAYOUT); ++i)
	{
		// Get the required index (the number of the screen in the speaker room) to set in the current cell
		roomScreenIndex = pTelepresenceLayoutIndexesStruct->IndexArr[i];
//		TRACEINTO << "***_e_m_DEBUG*** i = " << i << " roomScreenIndex: " << (int)roomScreenIndex;
		if ( NULL == (*pCurrLayout)[i] || (*pCurrLayout)[i]->isBlanked() || (roomScreenIndex == ((BYTE)BLANK_CELL)))
		{
			continue;
		}

		// Set relevant image in cell according to room screen index
		screenPartyRsrcId = pSpeakerRoomInfo.GetLinkPartyId(roomScreenIndex - 1);	// get partyId of each screen
		_ii = std::find(rImagesReadyForSetting.begin(), rImagesReadyForSetting.end(), screenPartyRsrcId);	//find the relevant image
		if (_ii != rImagesReadyForSetting.end())
		{
			(*pCurrLayout)[i]->SetImageId(*_ii);
			_ii = rImagesReadyForSetting.erase(_ii);
			continue;
		}
		else // ****TBD
		{
			//TRACE + decide what we want to do here
		}
	}
}

////////////////////////////////////////////////////////////////////////////
BOOL CLayoutHandler::IsImageInSameRoom(DWORD partyID)
{
	CVideoBridgePartyCntl* pCellPartyCntl = NULL;
	DWORD partyRoomId, imagePartyRoomId;
	partyRoomId = imagePartyRoomId = (DWORD)-1;

	partyRoomId = m_pPartyCntl->GetTelepresenceInfo().GetRoomID();

	pCellPartyCntl = (CVideoBridgePartyCntl*)const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(partyID);
	if (!IsValidPObjectPtr(pCellPartyCntl))
	{
		TRACEINTO << "Image isn't valid" << partyID;
		return FALSE;
	}

	imagePartyRoomId = pCellPartyCntl->GetTelepresenceInfo().GetRoomID();

	return (partyRoomId == imagePartyRoomId);
}
////////////////////////////////////////////////////////////////////
CLayoutHandlerTelepresenceCP::CLayoutHandlerTelepresenceCP(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl) :
		CLayoutHandlerTelepresence(videoCntl, partyCntl)
{}

////////////////////////////////////////////////////////////////////////////
CLayoutHandlerTelepresenceCP::~CLayoutHandlerTelepresenceCP()
{}

////////////////////////////////////////////////////////////////////////////
void  CLayoutHandlerTelepresenceCP::CollectRoomScreenLayoutsInfo(CRoomInfo& rLocalRoomInfo,
                                                                 TelepresenceCpModeLayout& rLayoutMapPerRoom,
                                                                 CRoomScreensLayoutMap& retRoomLayoutDB,
                                                                 ostringstream& ostr)
{
	RoomID id = rLocalRoomInfo.GetRoomId();
	BYTE localScreensNumber = (id != (RoomID)-1) ? rLocalRoomInfo.GetNumberOfActiveLinks() : 1;

	ostr << "\n\nCollectRoomScreenLayoutsInfo - roomId:" << id << " numLinks:" << (WORD)localScreensNumber;

	TelepresenceCpModeLayout::iterator iRoomLayout    = rLayoutMapPerRoom.begin();
	TelepresenceCpModeLayout::iterator iRoomLayoutEnd = rLayoutMapPerRoom.end();

	CVideoBridgePartyCntl* pPartyCtrl = NULL;

	for (BYTE linkNum=0; linkNum<localScreensNumber; ++linkNum)
	{
		PartyRsrcID otherScreenPartyId = DUMMY_PARTY_ID;
		if (id != (RoomID)-1)
		{
			otherScreenPartyId = rLocalRoomInfo.GetLinkPartyId(linkNum);
			pPartyCtrl = (CVideoBridgePartyCntl*)const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(otherScreenPartyId);
		}
		else
			pPartyCtrl = m_pPartyCntl;

		CLayout* pCurrLayout = pPartyCtrl ? pPartyCtrl->GetCurrentLayout() : NULL;
		if (!pPartyCtrl || !pCurrLayout)
		{
			ostr << "\n partyId:" << otherScreenPartyId << " screenPosition:" << (WORD)linkNum << " not found!";
			continue;
		}
		EScreenPosition screenPos = (EScreenPosition)linkNum;
		iRoomLayout = rLayoutMapPerRoom.find(screenPos);

		if (iRoomLayout != iRoomLayoutEnd && iRoomLayout->second.layout != CP_NO_LAYOUT && retRoomLayoutDB.find(screenPos) == retRoomLayoutDB.end())
		{
			ScreenCpLayoutInfo* pPartyLI = new ScreenCpLayoutInfo(id, screenPos, pPartyCtrl, pCurrLayout, iRoomLayout->second);
			ostr << "\n partyId:" << otherScreenPartyId << " screenPosition:" << screenPos
					<< " screenType:" << CTelepresenceSpeakerModeLayoutLogic::ScreenTypeAsString(iRoomLayout->second.screenType)
					<< " layoutType:" << LayoutTypeAsString[iRoomLayout->second.layout];
			retRoomLayoutDB.insert(make_pair(screenPos, pPartyLI));
		}
	}
}

////////////////////////////////////////////////////////////////////////////
bool CLayoutHandlerTelepresenceCP::BuildLayout()
{
	PASSERT_AND_RETURN_VALUE(!m_pPartyCntl, false);
	PASSERT_AND_RETURN_VALUE(!m_pVideoCntl, false);
	TRACEINTO << "partyID:" << m_pPartyCntl->GetPartyRsrcID();

	CLayout* pCurrLayout = m_pPartyCntl->GetCurrentLayout();
	PASSERT_AND_RETURN_VALUE(!pCurrLayout, false);

	// Create copy of current layout before changing it
	CLayout* pPreviouseLayout = new CLayout(*pCurrLayout);

	RemoveOldPartForcesAndBlankes();
	RemoveOldConfForcesAndBlankes();

	FillLayout();

//	BuildLayoutIndications(*pCurrLayout);// currently  ITP isn't supporting

	bool isLayoutChanged = (*pPreviouseLayout != *pCurrLayout);

	POBJDELETE(pPreviouseLayout);

	return isLayoutChanged;
}


void CLayoutHandlerTelepresenceCP::OccupyCellByPartyImage(WORD cellToBeImageSeen, ScreenCpLayoutInfo* pScreenLayoutInfo,
														  std::pair<PartyRsrcID, bool>& rCurrSpeakerImage,
														  bool isFilmstripCell, ostringstream& ostr)
{
	CLayout* pScreenLayout = pScreenLayoutInfo ? pScreenLayoutInfo->m_pNewLayout : NULL;
	TRACECOND_AND_RETURN(!pScreenLayoutInfo || !pScreenLayout, "illegal screen parameters");

	CVidSubImage* pCurrScreenCell = pScreenLayout ? (*pScreenLayout)[cellToBeImageSeen] : NULL;
	TRACECOND_AND_RETURN(!pCurrScreenCell, "illegal cell");

	PartyRsrcID imgPartyId = rCurrSpeakerImage.first;
	pCurrScreenCell->SetImageId(imgPartyId);

	rCurrSpeakerImage.second = 1; //mark the current image as used

	if (!isFilmstripCell)
		pScreenLayoutInfo->m_numFilledCells += 1;
	else
		pScreenLayoutInfo->m_filmstrip.m_numFilmstripFilledCells += 1;

	ostr << "\n\nOccupyCellByPartyImage - roomId:" << pScreenLayoutInfo->m_roomId << " screen:" << pScreenLayoutInfo->m_screenPos
         << " cellToBeImageSeen:" << cellToBeImageSeen << " imgPartyId:" << imgPartyId << " isFilmstripCell:" << (WORD)isFilmstripCell;
}

////////////////////////////////////////////////////////////////////////////
BYTE CLayoutHandlerTelepresenceCP::FindSpaceInGridLayout(const ScreenCpLayoutInfo& pGridLayoutInfo, WORD numSpeakerImages, ostringstream& ostr)
{
	BYTE currCellInGrid  = 0xFF;
	CLayout* pGridLayout = pGridLayoutInfo.m_pNewLayout;
	PASSERTMSG_AND_RETURN_VALUE(!pGridLayout, "no grid layout", currCellInGrid);

	BYTE maxCellsInGrid  = pGridLayoutInfo.m_numCellsToUse;
	BYTE numCellsInRow   = pGridLayoutInfo.m_numCellsInGridRow;

	for (BYTE indCell=0; indCell<maxCellsInGrid; ++indCell)
	{
		if (0 == (indCell % numCellsInRow))
		{
			BYTE numFreeCellsInRow = 0, indexCellToStart = 0;
			for (BYTE i=indCell, prevFreeCellIndex=0xFF; (i < indCell+numCellsInRow) && (numFreeCellsInRow < numSpeakerImages); ++i)
			{
//				if (2==numSpeakerImages)
//					ostr << " Olga_1 : i:" << (WORD)i << " numCellsInRow:" << (WORD)numCellsInRow << " numFreeCellsInRow:" << (WORD)numFreeCellsInRow << " numSpeakerImages:" << numSpeakerImages << endl;
				if ((*pGridLayout)[i] && (*pGridLayout)[i]->noImgSet()) //if this is empty cell
				{
					if (0xFF==prevFreeCellIndex)   //important to check the sequential cells
					{
						indexCellToStart = i;
//						if (2==numSpeakerImages)
//							ostr << " Olga_2: indexCellToStart:" << (WORD)indexCellToStart << endl;
					}
					if (i==(prevFreeCellIndex+1) || 0xFF==prevFreeCellIndex)
					{
						++numFreeCellsInRow;
						prevFreeCellIndex = i;
//						if (2==numSpeakerImages)
//							ostr << " Olga_3: numFreeCellsInRow:" << (WORD)numFreeCellsInRow << " prevFreeCellIndex:" << (WORD)prevFreeCellIndex << endl;
					}
//					if (2==numSpeakerImages)
//						ostr << " Olga_4: indexCellToStart:" << (WORD)indexCellToStart << " prevFreeCellIndex:" << (WORD)prevFreeCellIndex << endl;
				}
				else if (0xFF != prevFreeCellIndex)
				{
					numFreeCellsInRow = 0;
					prevFreeCellIndex = 0xFF;
//					if (2==numSpeakerImages)
//						ostr << " Olga_5: numFreeCellsInRow:0" << endl ;
				}
			}
			if (numFreeCellsInRow >= numSpeakerImages)
			{
				currCellInGrid = indexCellToStart;
//				ostr << " Olga_6: currCellInGrid:" << (WORD)currCellInGrid << " numFilledCells:" << (WORD)pGridLayoutInfo.m_numFilledCells << endl;
				break;
			}
		}
	}
	ostr << "\n\nFindSpaceInGridLayout - room:" << pGridLayoutInfo.m_roomId << " screen:" << pGridLayoutInfo.m_screenPos
		 << " maxCellsInGrid:"<< (WORD)maxCellsInGrid << " numFilledCells:" << (WORD)pGridLayoutInfo.m_numFilledCells << " chosen cell:" << (WORD)currCellInGrid << endl;

	return currCellInGrid;
}
////////////////////////////////////////////////////////////////////////////
bool CLayoutHandlerTelepresenceCP::FindSpeakerInGridLayout(std::pair<RoomID, CPartyImageInfoVector>& rSpeakerImages,
														   CRoomScreensLayoutMap& currRoomLayoutDB, CVidSubImage* pCurrScreenNewCell, ostringstream& ostr)
{
	TRACECOND_AND_RETURN_VALUE(!pCurrScreenNewCell, "illegal cell parameters", false);

	size_t numSpeakerImages = rSpeakerImages.second.size();
	ostr << "\n\nFindSpeakerInGridLayout - Speaker RoomId:" << rSpeakerImages.first << "  numSpeakerImages:" << numSpeakerImages;

	CRoomScreensLayoutMap::iterator iRoomLayoutInfoEnd = currRoomLayoutDB.end();

	BYTE numImagesFound = 0;
	std::map<PartyRsrcID, WORD> indexCellToOccupy;

	EScreenPosition screenPosition = ePosNone;

	for (BYTE indexSpeakerImages = 0; indexSpeakerImages < numSpeakerImages; ++indexSpeakerImages)
	{
		std::pair<PartyRsrcID, bool>& partySpeakerImage = rSpeakerImages.second.at(indexSpeakerImages);

		PartyRsrcID imgPartyId = partySpeakerImage.first;
		ostr << "\nPartyId:" << imgPartyId;
		if (!imgPartyId) // it can happen if RoomInfo already updated about some link party, but its PartyCntrl doesn't exist yet
		{
			pCurrScreenNewCell->SetImageId(0);
			pCurrScreenNewCell->SetForceAttributes(OPERATOR_Prior, BLANK_PARTY_Activ);
			continue;
		}

		if (ePosNone == screenPosition)
		{
			for (CRoomScreensLayoutMap::iterator iRoomLayoutInfo = currRoomLayoutDB.begin(); iRoomLayoutInfo != iRoomLayoutInfoEnd; ++iRoomLayoutInfo)
			{
				ScreenCpLayoutInfo* pPartyLI = iRoomLayoutInfo->second;

				CLayout* pOldLayout = pPartyLI->m_pOldLayout;
				CLayout* pNewLayout = pPartyLI->m_pNewLayout;
				if (!pOldLayout || !pNewLayout)
				{
					ostr << "\n\tscreen:" << iRoomLayoutInfo->first << " - no layout found";
					continue;
				}
				if (pOldLayout->GetLayoutType() != pNewLayout->GetLayoutType() || pPartyLI->m_newScreenType != eGrid)
				{
					ostr << "\n\tscreen:" << iRoomLayoutInfo->first << " - layout type is changing";
					continue;
				}

				WORD whereWasSeen = pOldLayout->FindImagePlaceInLayout(imgPartyId);
				if (whereWasSeen != AUTO)
				{
					ostr << "\n\tscreen:" << iRoomLayoutInfo->first << "  whereWasSeen:" << whereWasSeen;
					const CVidSubImage* pOldCellOnThisScreen = (*pOldLayout)[whereWasSeen];
					CVidSubImage*       pNewCellOnThisScreen = (*pNewLayout)[whereWasSeen];
					if (!pNewCellOnThisScreen || !pOldCellOnThisScreen)
					{
						ostr << "\n\tcell:" << whereWasSeen << "illegal cell number";
						continue;
					}
					if (!pNewCellOnThisScreen->noImgSet() || !pOldCellOnThisScreen->IsEqualOrLargeSize(*pCurrScreenNewCell))
					{
						ostr << " we can't use the old screen cell";
						return false;
					}
					screenPosition = iRoomLayoutInfo->first;
					numImagesFound++;
					indexCellToOccupy.insert(make_pair(imgPartyId, whereWasSeen));
					break;
				}
			}
		}
		else
		{
			CRoomScreensLayoutMap::iterator iRoomLayoutInfo = currRoomLayoutDB.find(screenPosition);
			if (iRoomLayoutInfo == iRoomLayoutInfoEnd)
			{
				ostr << "\n\tscreenPosition:" << screenPosition << " not found screen";
			}
			else
			{
				ScreenCpLayoutInfo* pPartyLI = iRoomLayoutInfo->second;
				CLayout* pOldLayout = pPartyLI->m_pOldLayout;
				WORD whereWasSeen   = pOldLayout ? pOldLayout->FindImagePlaceInLayout(imgPartyId) : AUTO;
				if (whereWasSeen != AUTO)
				{
					numImagesFound++;
					indexCellToOccupy.insert(make_pair(imgPartyId, whereWasSeen));
				}
			}
		}
	}

	if (numImagesFound == numSpeakerImages)
	{
		CRoomScreensLayoutMap::iterator iRoomLayoutInfo = currRoomLayoutDB.find(screenPosition);
		if (iRoomLayoutInfo != iRoomLayoutInfoEnd)
		{
			ostr << "\nscreen:" << screenPosition << " speaker room:" << rSpeakerImages.first << " use old screen";
			ScreenCpLayoutInfo* pPartyLI = iRoomLayoutInfo->second;
			CLayout* pNewLayout = pPartyLI->m_pNewLayout;
			if (pNewLayout)
			{
				std::map<PartyRsrcID, WORD>::iterator iiEnd = indexCellToOccupy.end();

				for (BYTE indexSpeakerImages = 0; indexSpeakerImages < numSpeakerImages; ++indexSpeakerImages)
				{
					std::pair<PartyRsrcID, bool>& partySpeakerImage = rSpeakerImages.second.at(indexSpeakerImages);

					PartyRsrcID imgPartyId = partySpeakerImage.first;

					std::map<PartyRsrcID, WORD>::iterator ii = indexCellToOccupy.find(imgPartyId);
					if (ii != iiEnd)
					{
						WORD whereWasSeen = ii->second;
						ostr << "\nPartyId:" << imgPartyId << " cell:" << whereWasSeen;
						OccupyCellByPartyImage(whereWasSeen, pPartyLI, partySpeakerImage, false, ostr);
					}
				}
			}
		}
	}
	else
		ostr << "\nscreen:" << screenPosition << " speaker room:" << rSpeakerImages.first << " ==> not all speaker images found!";

	return (numImagesFound == numSpeakerImages);
}
////////////////////////////////////////////////////////////////////////////

void CLayoutHandlerTelepresenceCP::SetBlackScreen(RoomID id, bool send_change_layout_to_sublinks)
{
	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = const_cast<CVideoBridge*> (m_pVideoCntl)->GetTelepresenceLayoutMngr();
	PASSERT_AND_RETURN(!pTelepresenceLayoutMngr);

	CRoomInfo localRoomInfo = pTelepresenceLayoutMngr->GetRoomInfo(id);
	BYTE localScreenNum = localRoomInfo.GetNumberOfActiveLinks();
	for (BYTE linkNum=0; linkNum<localScreenNum; ++linkNum)
	{
		PartyRsrcID otherScreenId = localRoomInfo.GetLinkPartyId(linkNum);
		CVideoBridgePartyCntl* pPartyCtrl = (CVideoBridgePartyCntl*)const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(otherScreenId);
		if (!pPartyCtrl)
			continue;

		CLayout* pCurrLayout = pPartyCtrl->GetCurrentLayout();
		if (pCurrLayout)
			pCurrLayout->ClearAllImageSources();

		CLayoutHandlerTelepresence::SetBlackScreen(pPartyCtrl);

		if (send_change_layout_to_sublinks && pPartyCtrl != m_pPartyCntl)
			pPartyCtrl->ChangeLayoutOfTPRoomSublink(pPartyCtrl->GetCurrentLayout(), NULL);
	}
}

////////////////////////////////////////////////////////////////////////////

WORD CLayoutHandlerTelepresenceCP::CreateVectorOfRoomImagesReadyForSetting(CRoomSpeakerVector& vectorOfImagesReadyForSetting, ostringstream& ostr)
{
	PASSERT_AND_RETURN_VALUE(!m_pPartyCntl, 0);
	PASSERT_AND_RETURN_VALUE(!m_pVideoCntl, 0);
	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = const_cast<CVideoBridge*> (m_pVideoCntl)->GetTelepresenceLayoutMngr();

	WORD numImages = 0;

	// Run over all connected to video bridge images and insert to Vector parties per RoomID those which are not muted
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	WORD partyImageVectorSize = m_pVideoCntl->GetPartyImageVectorSize();
	for (WORD i = 0; i < partyImageVectorSize; ++i)
	{
		PartyRsrcID partyId = m_pVideoCntl->GetPartyImageIdByPosition(i);
		CImage* pImage = pPartyImageLookupTable->GetPartyImage(partyId);
		if (pImage)
		{
			CVideoBridgePartyCntl* pPartyCtrl = (CVideoBridgePartyCntl*)const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(partyId);
			if (!pPartyCtrl)
			{
				ostr << "\n\nCreateVectorOfRoomImagesReadyForSetting - WARNING - partyId:" << partyId << " no PartyCtrl found";
				continue;
			}
			const CTelepresenseEPInfo& tpInfo = pPartyCtrl->GetTelepresenceInfo();
			RoomID roomId = tpInfo.GetRoomID();
			DWORD linkNum = tpInfo.GetLinkNum();

			CRoomPartyVectorIter ii;
			CRoomPartyVectorIter iend = vectorOfImagesReadyForSetting.end();
			for (ii = vectorOfImagesReadyForSetting.begin(); ii != iend; ++ii)
			{
				if (ii->first == roomId) //vector already have an images from this room
					break;
			}
			if (ii == iend) // add new room to this vector
			{
				BYTE localScreensNumber = MAX_CASCADED_LINKS_NUMBER;
				if (pTelepresenceLayoutMngr)
				{
					CRoomInfo localRoomInfo = pTelepresenceLayoutMngr->GetRoomInfo(roomId);
					localScreensNumber = (0xFFFF == localRoomInfo.GetRoomId()) ? 1 : localRoomInfo.GetNumberOfActiveLinks();
				}
				CPartyImageInfoVector partyIdVector(localScreensNumber, make_pair(0, 0));
				ii = vectorOfImagesReadyForSetting.insert(iend, make_pair(roomId, partyIdVector));
			}

			if (ii != vectorOfImagesReadyForSetting.end())
			{
				if (linkNum < ii->second.size()) //add this current party to the room vector
				{
					ii->second[ tpInfo.GetLinkNum() ] = make_pair(partyId, 0);
					++numImages;
				}
				else
					ostr << "\n\nCreateVectorOfRoomImagesReadyForSetting - WARNING - partyId:" << partyId << ", linkNum:" << linkNum << ", vector size:" << ii->second.size() ;
			}
			else
				ostr << "\n\nCreateVectorOfRoomImagesReadyForSetting - WARNING - partyId:" << partyId << ", linkNum:" << linkNum << " => not found roomId:" << roomId;

			if (pImage->isMuted())
				ostr << "\n\nCreateVectorOfRoomImagesReadyForSetting - WARNING - partyId:" << partyId << " pImage is muted!";
 		}
		else
			ostr << "\n\nCreateVectorOfRoomImagesReadyForSetting - WARNING - partyId:" << partyId << " pImage doesn't exist!";

	}

	//in case all images of same room are muted need to remove this room from the vector
	IsRoomMuted isRoomMutedExceptLocal(m_pPartyCntl->GetTelepresenceInfo().GetRoomID());
	vectorOfImagesReadyForSetting.erase(std::remove_if(vectorOfImagesReadyForSetting.begin(), vectorOfImagesReadyForSetting.end(), isRoomMutedExceptLocal), vectorOfImagesReadyForSetting.end());

	//Dump the vector
	CPrettyTable<RoomID, const char*> tbl("RoomID", "PartyID's");
	CRoomPartyVectorIter   iend = vectorOfImagesReadyForSetting.end();
	for(CRoomPartyVectorIter ii = vectorOfImagesReadyForSetting.begin(); ii != iend; ++ii)
	{
		ostringstream omsg;
		for(CPartyImageInfoVector::iterator it = ii->second.begin(); it != ii->second.end(); ++it)
			omsg << it->first << ", ";

		tbl.Add(ii->first, omsg.str().c_str());
	}

	ostr << "\n\nCreateVectorOfRoomImagesReadyForSetting" << tbl.Get();

	return numImages;
}

////////////////////////////////////////////////////////////////////////////
WORD CLayoutHandlerTelepresenceCP::RemoveSelfImagesFromRoomImagesReadyForSetting(RoomID id, CRoomSpeakerVector& rImagesReadyForSetting)
{
	CRoomPartyVectorIter iend = rImagesReadyForSetting.end();
	for (CRoomPartyVectorIter ii = rImagesReadyForSetting.begin(); ii != iend; ++ii)
	{
		if (ii->first == id)
		{
			rImagesReadyForSetting.erase(ii);
			break;
		}
	}
	WORD numImages = 0;
	iend = rImagesReadyForSetting.end();
	for(CRoomPartyVectorIter ii = rImagesReadyForSetting.begin(); ii != iend; ++ii)
	{
		numImages += ii->second.size();
	}
	return numImages;
}
////////////////////////////////////////////////////////////////////////////

DWORD CLayoutHandlerTelepresenceCP::CountRemainingImage(CRoomSpeakerVector& rImagesReadyForSetting)
{
	DWORD numImagesToSet = 0;

	CRoomPartyVectorIter iend = rImagesReadyForSetting.end();
	for (CRoomPartyVectorIter ii = rImagesReadyForSetting.begin(); ii != iend; ++ii)
	{
		numImagesToSet += std::count_if (ii->second.begin(), ii->second.end(), IsNotUsedImage());
	}
	return numImagesToSet;
}
////////////////////////////////////////////////////////////////////
void CLayoutHandlerTelepresenceCP::SendRoomChangeLayout(CRoomScreensLayoutMap& currRoomLayoutDB, ostringstream& ostr)
{
	CRoomScreensLayoutMap::iterator iRoomLayoutInfoEnd = currRoomLayoutDB.end();

	for (CRoomScreensLayoutMap::iterator ii = currRoomLayoutDB.begin(); ii != iRoomLayoutInfoEnd; ++ii)
	{
		ScreenCpLayoutInfo* pScreenCpLayout = ii->second;
		if (!pScreenCpLayout)
		{
			ostr << "\n\nFillLayout - screen:" << ii->first << "pScreenCpLayout is NULL skipping";
			continue;
		}

		CVideoBridgePartyCntl* pPartyCtrl   = pScreenCpLayout->m_pPartyCntl;
		if (!pPartyCtrl)
		{
			ostr << "\n\nFillLayout - screen:" << ii->first << "with room id: " << pScreenCpLayout->m_roomId <<" pPartyCtrl is NULL skipping";
			continue;
		}

		if (0 == pScreenCpLayout->m_numFilledCells)
		{
			ostr << "\n\nFillLayout - screen:" << ii->first << " partyId:" << pPartyCtrl->GetPartyRsrcID() << " force blank layout";
			CLayoutHandlerTelepresence::SetBlackScreen(pScreenCpLayout->m_pNewLayout);
		}

		if (pPartyCtrl == m_pPartyCntl) //main party - update current layout
		{
			ostr << "\n\nFillLayout - screen:" << ii->first << " partyId:" << pPartyCtrl->GetPartyRsrcID() << " MAIN PARTY change layout";
			*m_pPartyCntl->GetCurrentLayout() = *pScreenCpLayout->m_pNewLayout;
			m_pPartyCntl->GetPartyVisualEffects()->SetSpecifiedBorders(&pScreenCpLayout->m_telepresenceCellBorders);
		}
		else if (*pScreenCpLayout->m_pOldLayout != *pScreenCpLayout->m_pNewLayout)
		{
			//send change layout to all sublinks
			ostr << "\nFillLayout - screen:" << ii->first << " partyId:" << pPartyCtrl->GetPartyRsrcID() << " send change layout";
			pPartyCtrl->ChangeLayoutOfTPRoomSublink(pScreenCpLayout->m_pNewLayout, &pScreenCpLayout->m_telepresenceCellBorders);
		}
	}
}
////////////////////////////////////////////////////////////////////

void CLayoutHandlerTelepresenceCP::FillGridLayout(TelepresenceCpModeLayout& rLayoutMapPerRoom,
                                                  CRoomSpeakerVector& vectorOfImagesReadyForSetting,
                                                  CRoomScreensLayoutMap& rCurrRoomLayoutDB,
                                                  ostringstream& ostr, eFillGridPolicy fillPolicy)
{
	WORD numGridCellsOnAllScreens = 0;

	// calculate number of available cells on all grid screens
	TelepresenceCpModeLayout::iterator   iRoomLayoutEnd = rLayoutMapPerRoom.end();
	for (TelepresenceCpModeLayout::iterator iRoomLayout = rLayoutMapPerRoom.begin(); iRoomLayout != iRoomLayoutEnd; ++iRoomLayout)
	{
		if (eGrid != iRoomLayout->second.screenType)
			continue;
		if ((eFillOnly4x4 == fillPolicy && CP_LAYOUT_4X4 != iRoomLayout->second.layout) || //The logic of filling the cells is: Use grid cells (up to 3x3) before filling the filmstrip cells.
			(eFillUpTo3x3 == fillPolicy && CP_LAYOUT_4X4 == iRoomLayout->second.layout) )
			continue;
		switch (iRoomLayout->second.layout)
		{
			case CP_LAYOUT_1X1:  numGridCellsOnAllScreens += 1;  break;
			case CP_LAYOUT_2X2:  numGridCellsOnAllScreens += 4;  break;
			case CP_LAYOUT_3X3:  numGridCellsOnAllScreens += 9;  break;
			case CP_LAYOUT_4X4:  numGridCellsOnAllScreens += 16; break;
			default:
				break;
		}
	}
	size_t numSpeakerRoomsForSetting = vectorOfImagesReadyForSetting.size();

	BYTE currSpeakerRoomIndex = 0xFF, lastSpeakerRoomIndex = numSpeakerRoomsForSetting;
	WORD numOfRemainImages    = 0;

	//look for a first and last room which should be displayed in a grid screens (number of remaining images <= number of available grid cells)
	for(BYTE indexRoom = 0; indexRoom < numSpeakerRoomsForSetting; ++indexRoom)
	{
		CPartyImageInfoVector& rPartyImages = vectorOfImagesReadyForSetting.at(indexRoom).second;
		size_t numCurRoomImages = rPartyImages.size();
		if (numCurRoomImages && (0 != rPartyImages.front().second)) //check that a first room image isn't used still
			continue;
		if (numCurRoomImages && 0xFF == currSpeakerRoomIndex)
		{
			currSpeakerRoomIndex = indexRoom;
		}
		if (numOfRemainImages+numCurRoomImages > numGridCellsOnAllScreens)
		{
			lastSpeakerRoomIndex = indexRoom;
			break;
		}
		numOfRemainImages += numCurRoomImages;
	}
	ostr << "\n\nFillGridLayout - numGridCellsOnAllScreens: " << numGridCellsOnAllScreens  << " numSpeakerRoomsForSetting:" << numSpeakerRoomsForSetting
		 << " currSpeakerRoomIndex:" << (WORD)currSpeakerRoomIndex << " lastSpeakerRoomIndex:" << (WORD)lastSpeakerRoomIndex << " fillPolicy:" << (WORD)fillPolicy;

	while (!FillGridLayoutByDefinedSpeakers(rLayoutMapPerRoom, vectorOfImagesReadyForSetting,
											currSpeakerRoomIndex, lastSpeakerRoomIndex,
											rCurrRoomLayoutDB, ostr, fillPolicy)
					&& lastSpeakerRoomIndex > currSpeakerRoomIndex)
	{
		CRoomScreensLayoutMap::iterator  iRoomLayoutInfoEnd = rCurrRoomLayoutDB.end();
		for (CRoomScreensLayoutMap::iterator iRoomInfoLayout = rCurrRoomLayoutDB.begin(); iRoomInfoLayout != iRoomLayoutInfoEnd; ++iRoomInfoLayout)
		{
			if (eGrid == iRoomInfoLayout->second->m_newScreenType && iRoomInfoLayout->second->m_pNewLayout)
			{
				iRoomInfoLayout->second->m_numFilledCells = 0;
				iRoomInfoLayout->second->m_pNewLayout->CleanUp();
			}
		}
		for(BYTE i=currSpeakerRoomIndex ; i < lastSpeakerRoomIndex; ++i)
		{
			std::pair<RoomID, CPartyImageInfoVector>& iSpeakerImages = vectorOfImagesReadyForSetting.at(i);
			size_t numSpeakerImages = iSpeakerImages.second.size();
			if (numSpeakerImages && (0 != iSpeakerImages.second.front().second)) //check that a first room image isn't used still
			{
				for (BYTE indexSpeakerImages = 0; indexSpeakerImages < numSpeakerImages; ++indexSpeakerImages)
				{
					std::pair<PartyRsrcID, bool>& rCurrSpeakerImage = iSpeakerImages.second.at(indexSpeakerImages);
					rCurrSpeakerImage.second = 0;
				}
			}
		}
		lastSpeakerRoomIndex -= 1;
		ostr << "\nNot all speakers mapped - exclude the last speaker room - currSpeakerRoomIndex:" << (WORD)currSpeakerRoomIndex << " new lastSpeakerRoomIndex:" << (WORD)lastSpeakerRoomIndex;
	}
}
////////////////////////////////////////////////////////////////////////////
bool CLayoutHandlerTelepresenceCP::FillGridLayoutByDefinedSpeakers(TelepresenceCpModeLayout& rLayoutMapPerRoom,
																CRoomSpeakerVector& vectorOfImagesReadyForSetting,
																WORD currSpeakerRoomIndex, WORD lastSpeakerRoomIndex,
																CRoomScreensLayoutMap& rCurrRoomLayoutDB,
																ostringstream& ostr, eFillGridPolicy fillPolicy)
{
	CRoomScreensLayoutMap::iterator     iRoomLayoutInfoEnd = rCurrRoomLayoutDB.end();

	// look for a right cell in the grid layout for the remaining images , i.e. we'd like to avoid moving participants
	// when not necessary and  try to keep original cells location when changing grid
	std::vector<std::pair<WORD, EScreenPosition> > listNewSpeakersRoomIdToMapInLayout;

	for( ; currSpeakerRoomIndex < lastSpeakerRoomIndex; ++currSpeakerRoomIndex)
	{
		std::pair<RoomID, CPartyImageInfoVector>& iSpeakerImages = vectorOfImagesReadyForSetting.at(currSpeakerRoomIndex);
		size_t numSpeakerImages = iSpeakerImages.second.size();
		if (numSpeakerImages && (0 != iSpeakerImages.second.front().second)) //check that a first room image isn't used still
			continue;

		BYTE numCellsInBestGridRow = 0xFF, bestGridCellPosition = 0xFF;
		EScreenPosition bestGridScreenPos = ePosNone;

		bestGridScreenPos = LookForBestGridScreen(rLayoutMapPerRoom, rCurrRoomLayoutDB, iSpeakerImages, bestGridCellPosition, ostr, fillPolicy);

		if (ePosNone == bestGridScreenPos)
		{
			ostr << "\nFillGridLayoutByDefinedSpeakers - currSpeakerRoomIndex:" << (WORD)currSpeakerRoomIndex << ", numSpeakerImages:" << numSpeakerImages << " - can't find grid to populate this speaker room";

			continue;
		}

		//found a best grid; now we need to spread the speaker images in this grid

		CRoomScreensLayoutMap::iterator iRoomInfo = rCurrRoomLayoutDB.find(bestGridScreenPos);
		if (iRoomInfo == iRoomLayoutInfoEnd)
		{
			ostr << "\nFillGridLayoutByDefinedSpeakers - linkNum:" << bestGridScreenPos << " no found layout info";
			continue;
		}
		ScreenCpLayoutInfo* pGridLayoutInfo = iRoomInfo->second;
		CLayout*  pGridLayout = pGridLayoutInfo ? pGridLayoutInfo->m_pNewLayout : NULL;
		if (!pGridLayout)
		{
			ostr << "\nFillGridLayoutByDefinedSpeakers - linkNum:" << bestGridScreenPos << " - no new layout to set";
			continue;
		}

		CVidSubImage* pCurrGridCell = (*pGridLayout)[bestGridCellPosition];

		//Try to keep original cells location when changing grid. To do this it's enough to look for a single image of this room in old layouts
		bool foundImage = FindSpeakerInGridLayout(iSpeakerImages, rCurrRoomLayoutDB, pCurrGridCell, ostr);
		if (!foundImage)
		{
			// first of all, we need to set in the layout all images previously shown in this grid and only after that to populate new images
			listNewSpeakersRoomIdToMapInLayout.push_back(make_pair(currSpeakerRoomIndex, bestGridScreenPos));
		}
	}

	std::vector<std::pair<WORD, EScreenPosition> > speakersRoomIdNoFreePlaceInLayout;
	// populate new speakers/images
	std::vector<std::pair<WORD, EScreenPosition> >::iterator itr = listNewSpeakersRoomIdToMapInLayout.begin();
	for( ; itr != listNewSpeakersRoomIdToMapInLayout.end(); ++itr)
	{
		std::pair<RoomID, CPartyImageInfoVector>& iSpeakerImages = vectorOfImagesReadyForSetting.at(itr->first);
		BYTE currCellInGrid = 0xFF;
		EScreenPosition bestGridScreenPos = LookForBestGridScreen(rLayoutMapPerRoom, rCurrRoomLayoutDB, iSpeakerImages, currCellInGrid, ostr, fillPolicy);
		if (ePosNone == bestGridScreenPos)
			bestGridScreenPos = itr->second;
		CRoomScreensLayoutMap::iterator          iRoomLayoutInfo = rCurrRoomLayoutDB.find(bestGridScreenPos);
		ScreenCpLayoutInfo*                      pGridLayoutInfo = iRoomLayoutInfo->second;
		CLayout*                                     pGridLayout = pGridLayoutInfo ? pGridLayoutInfo->m_pNewLayout : NULL;
		if (!pGridLayout)
			continue;

		if (0xFF == currCellInGrid)
		{
			ostr << "\nFillGridLayoutByDefinedSpeakers - wrong cell id:" << (WORD)currCellInGrid << " screen:" << bestGridScreenPos;
			speakersRoomIdNoFreePlaceInLayout.push_back(make_pair(itr->first, bestGridScreenPos));
			continue;
		}

		size_t numSpeakerImages = iSpeakerImages.second.size();

		for (BYTE indexSpeakerImages = 0; indexSpeakerImages < numSpeakerImages; ++indexSpeakerImages, ++currCellInGrid)
		{
			WORD indexInSpeakerImageVector = GetSuitableCellIndexOfImageVector(numSpeakerImages, indexSpeakerImages);

			std::pair<PartyRsrcID, bool>& rCurrSpeakerImage = iSpeakerImages.second.at(indexInSpeakerImageVector);

			ostr << "\n\nParty resource id:" << rCurrSpeakerImage.first << " bestGridCellPosition:" << (WORD)currCellInGrid;
			OccupyCellByPartyImage(currCellInGrid, pGridLayoutInfo, rCurrSpeakerImage, false, ostr);
		}
	}

	// BRIDGE-15773
	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = const_cast<CVideoBridge*> (m_pVideoCntl)->GetTelepresenceLayoutMngr();
	PASSERT_AND_RETURN_VALUE(!pTelepresenceLayoutMngr, true);

	std::vector<std::pair<WORD, EScreenPosition> >::iterator itt = speakersRoomIdNoFreePlaceInLayout.begin();
	for( ; itt != speakersRoomIdNoFreePlaceInLayout.end(); ++itt)
	{
		bool foundPlaceInGrid = false;
		std::pair<RoomID, CPartyImageInfoVector>& iSpeakerImages = vectorOfImagesReadyForSetting.at(itt->first);
		ostr << "\n\nATTENTION: not mapped speaker room:" << iSpeakerImages.first;
		TelepresenceCpModeLayout::iterator   iRoomLayoutEnd = rLayoutMapPerRoom.end();
		for (TelepresenceCpModeLayout::iterator iRoomLayout = rLayoutMapPerRoom.begin(); iRoomLayout != iRoomLayoutEnd && !foundPlaceInGrid; ++iRoomLayout)
		{
			if (eGrid != iRoomLayout->second.screenType)
				continue;
			EScreenPosition linkNum = iRoomLayout->first;
			CRoomScreensLayoutMap::iterator iRoomInfo = rCurrRoomLayoutDB.find(linkNum);
			if (iRoomInfo == iRoomLayoutInfoEnd)
			{
				ostr << "\n linkNum:" << linkNum << " no found grid layout info";
				continue;
			}
			ScreenCpLayoutInfo* pGridLayoutInfo = iRoomInfo->second;
			CLayout* pGridLayout = pGridLayoutInfo ? pGridLayoutInfo->m_pNewLayout : NULL;
			if (!pGridLayout)
				continue;
			LayoutType layoutType = pGridLayout->GetLayoutType();
			if ((eFillOnly4x4 == fillPolicy && CP_LAYOUT_4X4 != layoutType) || //The logic of filling the cells is: Use grid cells (up to 3x3) before filling the filmstrip cells.
				(eFillUpTo3x3 == fillPolicy && CP_LAYOUT_4X4 == layoutType))
				continue;
			// Look for a best row (there are more free cells) to display this speaker. BTW we count single EP's in order to move them (if needed) to another rows

			std::vector<GridRowInfo> gridRowList;
			BYTE num_rows = pGridLayoutInfo->GetNumGridRows();
			ostr << " \ntry to free space in grid screen:" << linkNum << " num rows:" << (WORD)num_rows;

			for (BYTE i=0; i<num_rows; ++i)
			{
				GridRowInfo row_i(i);
				gridRowList.push_back(row_i);
			}
			size_t numSpeakerImages = iSpeakerImages.second.size();
			BYTE freeCellsInGrid = pGridLayoutInfo->m_numCellsToUse - pGridLayoutInfo->m_numFilledCells;
			if (pGridLayoutInfo->m_numCellsInGridRow >= numSpeakerImages &&	freeCellsInGrid >= numSpeakerImages)
			{
				// go through grid matrix in order to count free cells and single EP's which can be moved
				for (BYTE i=0; i<pGridLayoutInfo->m_numCellsToUse; ++i)
				{
					if (!(*pGridLayout)[i])
						continue;
					BYTE rowIndex = i / pGridLayoutInfo->m_numCellsInGridRow;
					if (rowIndex >= gridRowList.size())
						continue;
					if ((*pGridLayout)[i]->noImgSet()) // free cell
						gridRowList[rowIndex].listFreeCells.push_back(i);
					else
					{	// check if this cell is occupied by single EP
						PartyRsrcID currentImageId = (*pGridLayout)[i]->GetImageId();
						CVideoBridgePartyCntl* pPartyCntl = (CVideoBridgePartyCntl*)(const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(currentImageId));
						if (pPartyCntl)
						{
							const CTelepresenseEPInfo& tpInfo = pPartyCntl->GetTelepresenceInfo();
							CRoomInfo localRoomInfo = pTelepresenceLayoutMngr->GetRoomInfo(tpInfo.GetRoomID());
							BYTE isSingleEP = (ONE_SCREEN == localRoomInfo.GetNumberOfActiveLinks());
							if (isSingleEP)
								gridRowList[rowIndex].listSingleEPs.push_back(i);
						}
					}
				}

				// Sort the rows by free cells number
				std::sort(gridRowList.begin(), gridRowList.end(), compareFreeCellsInRow);

				ostr << "\nAfter sorting:";
				BYTE sumSingleEpsNum = 0;
				for (BYTE i=0; i<num_rows; ++i)
				{
					ostr << "\n rowIndex:" << (WORD)gridRowList[i].rowIndex << " numFreeCells:" << gridRowList[i].listFreeCells.size() << " numSingleEPs:" << gridRowList[i].listSingleEPs.size();
					sumSingleEpsNum += gridRowList[i].listSingleEPs.size();
				}
				if (!sumSingleEpsNum)
				{
					ostr << "\nNo single EP's to move!!!!!!!!!!!!!!!!!";
					return false;	// need to repeat the fill grid flow but with less Speaker Rooms

				}
				// Move  single EP's to another row(s) in order to free space in chosen row
				BYTE rowIndex = 0;
				for (BYTE rowIndex = 0; rowIndex < gridRowList.size() && !foundPlaceInGrid; ++rowIndex)
				{
					size_t numFreeCellsInRow = gridRowList[rowIndex].listFreeCells.size();
					size_t numSingleEPsInRow = gridRowList[rowIndex].listSingleEPs.size();
					if (numFreeCellsInRow + numSingleEPsInRow >= numSpeakerImages)// we can use this row by moving single ep's to another row(s)
					{
						BYTE numCellsToFree = numSpeakerImages - numFreeCellsInRow;
						BYTE numMovedEPs = 0;
						for (std::vector<int>::iterator singleEpItr = gridRowList[rowIndex].listSingleEPs.begin(); singleEpItr < gridRowList[rowIndex].listSingleEPs.end(); ++singleEpItr)
						{
							if (!(*pGridLayout)[*singleEpItr])
								continue;
							DWORD partyRscIdToMove = (*pGridLayout)[*singleEpItr]->GetImageId();
							bool isPartyMoved = false;

							for (BYTE rowIndexToMove = 0; rowIndexToMove < gridRowList.size() && !isPartyMoved; ++rowIndexToMove)
							{
								if (rowIndex == rowIndexToMove)
									continue;
								size_t numFreeCellsInRowToMove = gridRowList[rowIndexToMove].listFreeCells.size();
								if (!numFreeCellsInRowToMove)
									continue;
								for (std::vector<int>::iterator emptyCellItr = gridRowList[rowIndexToMove].listFreeCells.begin();
																emptyCellItr < gridRowList[rowIndexToMove].listFreeCells.end(); ++emptyCellItr)
								{
									if (!(*pGridLayout)[*emptyCellItr])
										continue;
									ostr << "\n Move partyId:" << partyRscIdToMove << " from cell:" <<  *singleEpItr << " to cell:" << *emptyCellItr;

									(*pGridLayout)[*emptyCellItr]->SetImageId(partyRscIdToMove);

									(*pGridLayout)[*singleEpItr]->CleanUp(); // to free the cell

									gridRowList[rowIndexToMove].listFreeCells.erase(emptyCellItr);

									numMovedEPs++;
									isPartyMoved = true;

									break;//emptyCellItr = gridRowList[rowIndexToMove].listFreeCells.begin(); //initialize the iterator after erasing
								}
							}
							if (numMovedEPs == numCellsToFree)
								break;
						}
						if (numMovedEPs == numCellsToFree) // There are enough free cells in the chosen row.
						{
							BYTE currCellInGrid = FindSpaceInGridLayout(*pGridLayoutInfo, numSpeakerImages, ostr);

							if (0xFF == currCellInGrid) // need to move occupied cells to the right side in order to free sequential cells for this speaker
							{
								BYTE actualRowIndex = gridRowList[rowIndex].rowIndex;
								ostr << " move occupied cells to the right side of row:" << (WORD)actualRowIndex << " in order to free sequential cells for this speaker";
								for (int i=pGridLayoutInfo->m_numCellsInGridRow-1; i>=0; --i)
								{
									BYTE index_i = actualRowIndex * pGridLayoutInfo->m_numCellsInGridRow + i;

									if ((*pGridLayout)[index_i] && !(*pGridLayout)[index_i]->noImgSet())
									{
										DWORD partyRscIdToMove = (*pGridLayout)[index_i]->GetImageId();
										for (BYTE j=pGridLayoutInfo->m_numCellsInGridRow-1; j>i; --j)
										{
											BYTE index_j = actualRowIndex * pGridLayoutInfo->m_numCellsInGridRow + j;

											if ((*pGridLayout)[index_j] && (*pGridLayout)[index_j]->noImgSet())
											{
												(*pGridLayout)[index_j]->SetImageId(partyRscIdToMove);
												(*pGridLayout)[index_i]->CleanUp();
												break;
											}
										}
									}
								}
								currCellInGrid = FindSpaceInGridLayout(*pGridLayoutInfo, numSpeakerImages, ostr);
							}
							if (0xFF == currCellInGrid)
								continue;

							foundPlaceInGrid = true;

							for (BYTE indexSpeakerImages = 0; indexSpeakerImages < numSpeakerImages; ++indexSpeakerImages, ++currCellInGrid)
							{
								WORD indexInSpeakerImageVector = GetSuitableCellIndexOfImageVector(numSpeakerImages, indexSpeakerImages);

								std::pair<PartyRsrcID, bool>& rCurrSpeakerImage = iSpeakerImages.second.at(indexInSpeakerImageVector);

								ostr << "\n\nParty resource id:" << rCurrSpeakerImage.first << " bestGridCellPosition:" << (WORD)currCellInGrid;
								OccupyCellByPartyImage(currCellInGrid, pGridLayoutInfo, rCurrSpeakerImage, false, ostr);
							}
						}
					}
				}
			}
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////
EScreenPosition CLayoutHandlerTelepresenceCP::LookForBestGridScreen(TelepresenceCpModeLayout& rLayoutMapPerRoom,
				CRoomScreensLayoutMap& rCurrRoomLayoutDB,
				std::pair<RoomID, CPartyImageInfoVector>& iSpeakerImages,
				BYTE& bestGridCellPosition,
				ostringstream& ostr,
				eFillGridPolicy fillPolicy)
{
	size_t numSpeakerImages = iSpeakerImages.second.size();
	BYTE numCellsInBestGridRow = 0xFF;
	EScreenPosition bestGridScreenPos = ePosNone;

	CRoomScreensLayoutMap::iterator  iRoomLayoutInfoEnd = rCurrRoomLayoutDB.end();
	TelepresenceCpModeLayout::iterator   iRoomLayoutEnd = rLayoutMapPerRoom.end();
	for (TelepresenceCpModeLayout::iterator iRoomLayout = rLayoutMapPerRoom.begin(); iRoomLayout != iRoomLayoutEnd; ++iRoomLayout)
	{
		if (eGrid != iRoomLayout->second.screenType)
			continue;
		EScreenPosition linkNum = iRoomLayout->first;
		CRoomScreensLayoutMap::iterator iRoomInfo = rCurrRoomLayoutDB.find(linkNum);
		if (iRoomInfo == iRoomLayoutInfoEnd)
		{
			ostr << "\n linkNum:" << linkNum << " no found grid layout info";
			continue;
		}
		ScreenCpLayoutInfo* pGridLayoutInfo = iRoomInfo->second;
		CLayout* pGridLayout = pGridLayoutInfo ? pGridLayoutInfo->m_pNewLayout : NULL;
		if (!pGridLayout)
			continue;
		LayoutType layoutType = pGridLayout->GetLayoutType();
		if ((eFillOnly4x4 == fillPolicy && CP_LAYOUT_4X4 != layoutType) || //The logic of filling the cells is: Use grid cells (up to 3x3) before filling the filmstrip cells.
			(eFillUpTo3x3 == fillPolicy && CP_LAYOUT_4X4 == layoutType))
			continue;
		BYTE freeCellsInGrid = pGridLayoutInfo->m_numCellsToUse - pGridLayoutInfo->m_numFilledCells;
		if (pGridLayoutInfo->m_numCellsInGridRow >= numSpeakerImages &&	freeCellsInGrid >= numSpeakerImages)
		{
//			ostr << "\nOlga_00: PartyId:" << iSpeakerImages.second.at(0).first << " numSpeakerImages:" << numSpeakerImages;
			//We need to populate all speaker images in same row, therefore we look for a free sequence cells in some row of this grid
			BYTE currCellInGrid = FindSpaceInGridLayout(*pGridLayoutInfo, numSpeakerImages, ostr);

			if (currCellInGrid != 0xFF && pGridLayoutInfo->m_numCellsInGridRow < numCellsInBestGridRow)
			{
				numCellsInBestGridRow    = pGridLayoutInfo->m_numCellsInGridRow;
				bestGridScreenPos        = linkNum;
				bestGridCellPosition     = currCellInGrid;
			}
		}
	}
	return bestGridScreenPos;
}
////////////////////////////////////////////////////////////////////////////
BYTE CLayoutHandlerTelepresenceCP::GetSuitableCellIndexOfImageVector(size_t numSpeakerImages,BYTE indexSpeakerImages)
{
	WORD indexCurrCell = 0;
	switch(numSpeakerImages)
	{
		case ONE_SCREEN:	indexCurrCell = 0; break;
		case TWO_SCREEN:	indexCurrCell = (0==indexSpeakerImages ? 1 : 0); break;
		case THREE_SCREEN:	indexCurrCell = (0==indexSpeakerImages ? 1 : (1==indexSpeakerImages ? 0 : 2)); break;
		case FOUR_SCREEN:	indexCurrCell = (0==indexSpeakerImages ? 3 :
				(1==indexSpeakerImages ? 1 : (2==indexSpeakerImages ? 0 : 2))); break;
		default:
			DBGPASSERT(numSpeakerImages+1);
			break;
	}
	return indexCurrCell;

}
////////////////////////////////////////////////////////////////////////////
EScreenPosition CLayoutHandlerTelepresenceCP::GetScreenPositionForActiveSpeaker(CRoomInfo& rLocalRoomInfo)const
{
	return (FOUR_SCREEN == rLocalRoomInfo.GetNumberOfActiveLinks()) ? ePos2 : ePos1;
}
////////////////////////////////////////////////////////////////////////////
WORD CLayoutHandlerTelepresenceCP::GetNumGridCellsOnAllScreens(TelepresenceCpModeLayout& rLayoutMapPerRoom)
{
	WORD numGridCellsOnAllScreens = 0;
	TelepresenceCpModeLayout::iterator   iRoomLayoutEnd = rLayoutMapPerRoom.end();
	for (TelepresenceCpModeLayout::iterator iRoomLayout = rLayoutMapPerRoom.begin(); iRoomLayout != iRoomLayoutEnd; ++iRoomLayout)
	{
		if (eGrid != iRoomLayout->second.screenType)
			continue;
		switch (iRoomLayout->second.layout)
		{
			case CP_LAYOUT_1X1:  numGridCellsOnAllScreens += 1;  break;
			case CP_LAYOUT_2X2:  numGridCellsOnAllScreens += 4;  break;
			case CP_LAYOUT_3X3:  numGridCellsOnAllScreens += 9;  break;
			case CP_LAYOUT_4X4:  numGridCellsOnAllScreens += 16; break;
			default:
				break;
		}
	}
	return numGridCellsOnAllScreens;
}

////////////////////////////////////////////////////////////////////////////
RoomID CLayoutHandlerTelepresenceCP::GetRoomId(PartyRsrcID partyId)
{
	CVideoBridgePartyCntl* pPartyCtrl = (CVideoBridgePartyCntl*)const_cast<CVideoBridge*>(m_pVideoCntl)->GetPartyCntl(partyId);
	if (!pPartyCtrl)
	{
		TRACEINTO << "CLayoutHandlerTelepresenceCP::GetRoomId - WARNING - partyId:" << partyId << " no PartyCtrl found";
		PASSERT_AND_RETURN_VALUE(1,-1);
	}

	return pPartyCtrl->GetTelepresenceInfo().GetRoomID();
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerTelepresenceCP::ResetAllScreensForSwapping(CRoomScreensLayoutMap& rCurrRoomLayoutDB)
{
	CRoomScreensLayoutMap::iterator iRoomLayoutInfoEnd = rCurrRoomLayoutDB.end();

	for (CRoomScreensLayoutMap::iterator iRoomLayoutInfo = rCurrRoomLayoutDB.begin(); iRoomLayoutInfo != iRoomLayoutInfoEnd; ++iRoomLayoutInfo)
	{
		ScreenCpLayoutInfo* pScreenCpLayoutInfo = iRoomLayoutInfo->second;
		if (!pScreenCpLayoutInfo)
			continue;

		pScreenCpLayoutInfo->m_needToSwap = true;
	}
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerTelepresenceCP::MarkScreensNotAllowedToSwap(CRoomScreensLayoutMap& rCurrRoomLayoutDB)
{
	ResetAllScreensForSwapping(rCurrRoomLayoutDB);

	CRoomScreensLayoutMap::iterator iRoomLayoutInfoEnd = rCurrRoomLayoutDB.end();
	for (CRoomScreensLayoutMap::iterator iRoomLayoutInfo = rCurrRoomLayoutDB.begin(); iRoomLayoutInfo != iRoomLayoutInfoEnd; ++iRoomLayoutInfo)
	{
		ScreenCpLayoutInfo* pScreenCpLayoutInfo = iRoomLayoutInfo->second;
		if (!pScreenCpLayoutInfo)
			continue;

		CLayout* pNewLayout = pScreenCpLayoutInfo->m_pNewLayout;
		BYTE indexFirstCell = pScreenCpLayoutInfo->m_indexFirstCell;

		if (!pNewLayout || !pNewLayout->GetSubImageNum(indexFirstCell) || !pNewLayout->GetSubImageNum(indexFirstCell)->GetImageId())
			continue;

		for (CRoomScreensLayoutMap::iterator iRoomLayoutInfoNext = rCurrRoomLayoutDB.begin(); iRoomLayoutInfoNext != iRoomLayoutInfoEnd; ++iRoomLayoutInfoNext)
		{
			if (iRoomLayoutInfo == iRoomLayoutInfoNext)
				continue;

			ScreenCpLayoutInfo* pScreenCpLayoutInfoNext = iRoomLayoutInfoNext->second;
			if (!pScreenCpLayoutInfoNext)
				continue;

			CLayout* pNewLayoutNext = pScreenCpLayoutInfoNext->m_pNewLayout;
			BYTE indexFirstCell2 = pScreenCpLayoutInfoNext->m_indexFirstCell;

			if (!pNewLayoutNext || !pNewLayout->GetSubImageNum(indexFirstCell2) || !pNewLayoutNext->GetSubImageNum(indexFirstCell2)->GetImageId())
				continue;

			if (GetRoomId(pNewLayoutNext->GetSubImageNum(indexFirstCell2)->GetImageId()) == GetRoomId(pNewLayout->GetSubImageNum(indexFirstCell)->GetImageId()))
			{
				pScreenCpLayoutInfo->m_needToSwap = false;
				pScreenCpLayoutInfoNext->m_needToSwap = false;
				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerTelepresenceCP::SwapLayoutReservedAndGridScreensIfNeeded(CRoomScreensLayoutMap& rCurrRoomLayoutDB, ostringstream& ostr)
{
	MarkScreensNotAllowedToSwap(rCurrRoomLayoutDB);

	CRoomScreensLayoutMap::iterator iRoomLayoutInfoEnd = rCurrRoomLayoutDB.end();

	for (CRoomScreensLayoutMap::iterator iRoomLayoutInfo = rCurrRoomLayoutDB.begin(); iRoomLayoutInfo != iRoomLayoutInfoEnd; ++iRoomLayoutInfo)
	{
		ScreenCpLayoutInfo* pScreenCpLayoutInfo = iRoomLayoutInfo->second;
		if (!pScreenCpLayoutInfo || !pScreenCpLayoutInfo->m_needToSwap || (pScreenCpLayoutInfo->m_newScreenType == eGrid && pScreenCpLayoutInfo->m_pNewLayout->GetLayoutType() != CP_LAYOUT_1X1 && pScreenCpLayoutInfo->m_pNewLayout->GetLayoutType() != CP_LAYOUT_2X2))
			continue;

		CLayout* pOldLayout = pScreenCpLayoutInfo->m_pOldLayout;
		CLayout* pNewLayout = pScreenCpLayoutInfo->m_pNewLayout;
		BYTE indexFirstCell = pScreenCpLayoutInfo->m_indexFirstCell;
		if (!pOldLayout || !pNewLayout || !pNewLayout->GetSubImageNum(indexFirstCell) || !pOldLayout->GetSubImageNum(indexFirstCell))
			continue;
		
		for (CRoomScreensLayoutMap::iterator iRoomLayoutInfoNext = rCurrRoomLayoutDB.begin(); iRoomLayoutInfoNext != iRoomLayoutInfoEnd; ++iRoomLayoutInfoNext)		
		{
			if (iRoomLayoutInfoNext == iRoomLayoutInfo)
				continue;

			ScreenCpLayoutInfo* pScreenCpLayoutInfoNext = iRoomLayoutInfoNext->second;
			if (!pScreenCpLayoutInfoNext || !pScreenCpLayoutInfoNext->m_needToSwap || (pScreenCpLayoutInfoNext->m_newScreenType == eGrid && pScreenCpLayoutInfoNext->m_pNewLayout->GetLayoutType() != CP_LAYOUT_1X1 && pScreenCpLayoutInfoNext->m_pNewLayout->GetLayoutType() != CP_LAYOUT_2X2))
				continue;

			CLayout* pOldLayoutNext = pScreenCpLayoutInfoNext->m_pOldLayout;
			CLayout* pNewLayoutNext = pScreenCpLayoutInfoNext->m_pNewLayout;

			//Check validity of cells
			if (!pOldLayoutNext || !pNewLayoutNext || !pOldLayoutNext->GetSubImageNum(indexFirstCell) || !pNewLayoutNext->GetSubImageNum(indexFirstCell))
				continue;

			//Special case when the there is Overlay layout and 1x1 grid and vice verse.
			//Need to check the old next layout type to prevent not needed swaps for example local screen: 4 max: 3 old speaker: 3 screens new speaker: 1 screen
			if (((pNewLayout->GetLayoutType() == CP_LAYOUT_OVERLAY_ITP_1P4 && pNewLayoutNext->GetLayoutType() == CP_LAYOUT_1X1) ||
				(pNewLayout->GetLayoutType() == CP_LAYOUT_1X1 && pNewLayoutNext->GetLayoutType() ==  CP_LAYOUT_OVERLAY_ITP_1P4)) &&
				((pNewLayout->GetLayoutType() == CP_LAYOUT_1X1 && pOldLayoutNext->GetLayoutType() ==  CP_LAYOUT_OVERLAY_ITP_1P4) ||
				(pNewLayout->GetLayoutType() == CP_LAYOUT_OVERLAY_ITP_1P4 && pOldLayoutNext->GetLayoutType() ==  CP_LAYOUT_1X1)))
			{
				if (pNewLayout->GetSubImageNum(indexFirstCell)->GetImageId() == pOldLayoutNext->GetSubImageNum(indexFirstCell)->GetImageId())
				{
					DWORD tmpId = 0;
					tmpId = pNewLayout->GetSubImageNum(indexFirstCell)->GetImageId();

					pNewLayout->GetSubImageNum(indexFirstCell)->SetImageId(pNewLayoutNext->GetSubImageNum(indexFirstCell)->GetImageId());
					pNewLayoutNext->GetSubImageNum(indexFirstCell)->SetImageId(tmpId);
					ostr << "\n\nSwapLayoutReservedScreensIfNeeded - Swap cells in index 0 of curScreenPos:" << iRoomLayoutInfo->first << " nextScreenPos:" << iRoomLayoutInfoNext->first;
					break;
				}
				continue;
			}

			//Special case when there is 2x2 grid with RPX 200 2 screens only vs 1x2 flex and vice verse
			if ((pNewLayout->GetLayoutType() == CP_LAYOUT_2X2 && pOldLayoutNext->GetLayoutType() == CP_LAYOUT_1X2_FLEX) ||
				(pNewLayout->GetLayoutType() == CP_LAYOUT_1X2_FLEX && pOldLayoutNext->GetLayoutType() == CP_LAYOUT_2X2))
			{
				//Check if same cells exists
				if (pNewLayout->GetSubImageNum(indexFirstCell)->GetImageId() == pOldLayoutNext->GetSubImageNum(indexFirstCell)->GetImageId() &&
					pNewLayout->GetSubImageNum(indexFirstCell+1)->GetImageId() == pOldLayoutNext->GetSubImageNum(indexFirstCell+1)->GetImageId())
				{
					bool isNeedToSwap = false;

					//Check if the other cells in layout 2x2 is empty therefore 3 and 4
					if (pNewLayout->GetLayoutType() == CP_LAYOUT_2X2)
						isNeedToSwap = pNewLayout->GetSubImageNum(indexFirstCell+2)->GetImageId() == 0 && pNewLayout->GetSubImageNum(indexFirstCell+3)->GetImageId() == 0;
					else
						isNeedToSwap = pOldLayoutNext->GetSubImageNum(indexFirstCell+2)->GetImageId()== 0 && pOldLayoutNext->GetSubImageNum(indexFirstCell+3)->GetImageId() == 0;

					if (isNeedToSwap)
					{
						ScreenCpLayoutInfo tempLayoutInfoNext(*pScreenCpLayoutInfoNext);
						tempLayoutInfoNext.NoNeedToDeleteNewLayout();

						pScreenCpLayoutInfoNext->CopyNewLayoutParams(pScreenCpLayoutInfo);

						pScreenCpLayoutInfo->CopyNewLayoutParams(&tempLayoutInfoNext);

						ostr << "\n\nSwapLayoutReservedScreensIfNeeded - Swap between 1x2 Flex and 2x2 curScreenPos:" << iRoomLayoutInfo->first << " nextScreenPos:" << iRoomLayoutInfoNext->first;
						break;
					}
					continue;
				}
			}

			if (pNewLayout->GetLayoutType() != pOldLayoutNext->GetLayoutType())
				continue;


			if (pNewLayout->GetSubImageNum(indexFirstCell)->GetImageId() == pOldLayoutNext->GetSubImageNum(indexFirstCell)->GetImageId())
			{
				ScreenCpLayoutInfo tempLayoutInfoNext(*pScreenCpLayoutInfoNext);
				tempLayoutInfoNext.NoNeedToDeleteNewLayout();

				pScreenCpLayoutInfoNext->CopyNewLayoutParams(pScreenCpLayoutInfo);

				pScreenCpLayoutInfo->CopyNewLayoutParams(&tempLayoutInfoNext);
				ostr << "\n\nSwapLayoutReservedScreensIfNeeded - Swap between reserved screens curScreenPos:" << iRoomLayoutInfo->first << " nextScreenPos:" << iRoomLayoutInfoNext->first;
				break;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
struct lookForNotShownImage : public std::unary_function<std::pair<PartyRsrcID, bool>, bool>
{
	bool operator()(const std::pair<PartyRsrcID, bool>& rPartyImage) const
	{
		return !rPartyImage.second;
	}
};

bool CheckIfThereAreNotShownImageParty(CPartyImageInfoVector* pImagesListVect)
{
	CPartyImageInfoVector::iterator it = std::find_if(pImagesListVect->begin(),
													  pImagesListVect->end(),
													  lookForNotShownImage());
	return (it != pImagesListVect->end());
}

CLayoutHandlerTelepresenceSpeakerModeCP::CLayoutHandlerTelepresenceSpeakerModeCP(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl) :
				CLayoutHandlerTelepresenceCP(videoCntl, partyCntl)
{}

////////////////////////////////////////////////////////////////////////////
CLayoutHandlerTelepresenceSpeakerModeCP::~CLayoutHandlerTelepresenceSpeakerModeCP()
{}
////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerTelepresenceSpeakerModeCP::FillLayout()
{
	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = const_cast<CVideoBridge*> (m_pVideoCntl)->GetTelepresenceLayoutMngr();
	PASSERT_AND_RETURN(!pTelepresenceLayoutMngr);
	CTelepresenceSpeakerModeLayoutLogic* pTelepresenceLayoutLogic = pTelepresenceLayoutMngr->GetTelepresenceSpeakerModeLayoutLogic();
	PASSERT_AND_RETURN(!pTelepresenceLayoutLogic);
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	PASSERT_AND_RETURN(!pPartyImageLookupTable);

	const CTelepresenseEPInfo& tpInfo = m_pPartyCntl->GetTelepresenceInfo();
	RoomID id = tpInfo.GetRoomID();

	ostringstream ostr;
	ostr << "EMB_MLA_OLGA - room:" << id << ", partyId:" << m_pPartyCntl->GetPartyRsrcID();

	// 2. Get the room (all screens together) layouts according to Speaker Mode logic and current speakers list
	CRoomInfo localRoomInfo = pTelepresenceLayoutMngr->GetRoomInfo(id);

	BYTE localScreensNumber = (localRoomInfo.GetRoomId() != (RoomID)-1) ? localRoomInfo.GetNumberOfActiveLinks() : 1;

	// 1. Create an image vector sorted by speaker order but organized by room id
	CRoomSpeakerVector vectorOfImagesReadyForSetting;
	WORD numImagesToSet = CreateVectorOfRoomImagesReadyForSetting(vectorOfImagesReadyForSetting, ostr);
	// look for the best grid (bigger cell resolution) screen that can cover all speaker cameras in one row
	size_t numSpeakerRoomsForSetting = vectorOfImagesReadyForSetting.size();

	if (0 == numSpeakerRoomsForSetting) //may be the case if no decoder is connected in conf and is unmuted
	{
		SetBlackScreen(id, true);

		TRACECOND_AND_RETURN(1, "empty vectorOfImagesReadyForSetting - return");
	}

	if ((numSpeakerRoomsForSetting == 1) && (vectorOfImagesReadyForSetting[0].first == id))
	{
		SetBlackScreen(id, true);

		TRACECOND_AND_RETURN(1, " vectorOfImagesReadyForSetting has self image(s) only - return");
	}

	TelepresenceCpModeLayout rLayoutMapPerRoom;

	pTelepresenceLayoutLogic->GetRoomLayout(id, localScreensNumber, vectorOfImagesReadyForSetting, rLayoutMapPerRoom);

	size_t sizeLayouts = rLayoutMapPerRoom.size();
	TRACECOND_AND_RETURN( (sizeLayouts != localScreensNumber), "wrong size of the vector layouts - return");

	// remove the room self images
	if (numSpeakerRoomsForSetting != 0)
		numImagesToSet = RemoveSelfImagesFromRoomImagesReadyForSetting(id, vectorOfImagesReadyForSetting);

	numSpeakerRoomsForSetting = vectorOfImagesReadyForSetting.size();

	// 3. Create a local DB to store all needed layout information for whole room in order to accelerate data extraction about each party in this room

	CRoomScreensLayoutMap currRoomLayoutDB;
	CollectRoomScreenLayoutsInfo(localRoomInfo, rLayoutMapPerRoom, currRoomLayoutDB, ostr);

	CRoomScreensLayoutMap::iterator iRoomLayoutInfoEnd = currRoomLayoutDB.end();

	// 4. Fill the layout of reserved screens and removed used images from the vector before looking for grids layout

	BYTE gridCount = 0, currSpeakerRoomIndex = 0;

	TelepresenceCpModeLayout::iterator iRoomLayout    = rLayoutMapPerRoom.begin();
	TelepresenceCpModeLayout::iterator iRoomLayoutEnd = rLayoutMapPerRoom.end();

	size_t numLocalRoomScreens = rLayoutMapPerRoom.size();
	for (WORD index=0; index<numLocalRoomScreens; ++index)  // go through all layouts and fill only reserved screens
	{
		size_t numSpeakerImagesToShow = (currSpeakerRoomIndex < numSpeakerRoomsForSetting) ? vectorOfImagesReadyForSetting.at(currSpeakerRoomIndex).second.size() : 1;

		EScreenPosition screenPos = (EScreenPosition)index;
		iRoomLayout = GetScreenLayoutByPosition(rLayoutMapPerRoom, screenPos, localRoomInfo, numSpeakerImagesToShow);

		EScreenPosition linkNum = iRoomLayout->first;
		if (linkNum != screenPos)
			ostr << "\n\nFillLayout - screen index:" << screenPos << " was replaced to:" << linkNum;

		if (eReserved == iRoomLayout->second.screenType)
		{
			CRoomScreensLayoutMap::iterator iRoomInfo = currRoomLayoutDB.find(linkNum);
			if (iRoomInfo == iRoomLayoutInfoEnd)
			{
				ostr << "\n\nFillLayout - linkNum:" << linkNum << ", room:" << id << " no found layout info";
				continue;
			}
			ScreenCpLayoutInfo* pScreenLayoutInfo = iRoomInfo->second;
			CLayout*  pScreenLayout = pScreenLayoutInfo ? pScreenLayoutInfo->m_pNewLayout : NULL;
			if (!pScreenLayout)
			{
				TRACESTR(eLevelWarn) << "linkNum:" << linkNum << ", room:" << id << " - no layout to set";
				continue;
			}
			LayoutType layoutType = pScreenLayout->GetLayoutType();

			while (pScreenLayoutInfo->m_numFilledCells < pScreenLayoutInfo->m_numCellsToUse && (currSpeakerRoomIndex < numSpeakerRoomsForSetting))
			{
				BYTE indexRoom = 0, found = 0;
				for( ; indexRoom < numSpeakerRoomsForSetting; ++indexRoom) /* look for an speaker room where images number is equal to this layout type (cells number) */
				{                                                          /* or the local is 3 screens ep and the current speaker is 3 screens as well */
					std::pair<RoomID, CPartyImageInfoVector>& iSpeakerImages = vectorOfImagesReadyForSetting.at(indexRoom);

					bool foundNotShownImage = CheckIfThereAreNotShownImageParty(&iSpeakerImages.second);

					size_t numSpeakerImages = iSpeakerImages.second.size();

					if (foundNotShownImage && (numSpeakerImages == pScreenLayoutInfo->m_numCellsToUse ||
									(CP_LAYOUT_1X2_FLEX == layoutType && FOUR_SCREEN == numSpeakerImages) ||
									(CP_LAYOUT_OVERLAY_ITP_1P4 == layoutType && THREE_SCREEN == numSpeakerImages)) )
					{
						found = 1;
						break;
					}
				}
				std::pair<RoomID, CPartyImageInfoVector>& iSpeakerImages = found ? vectorOfImagesReadyForSetting.at(indexRoom) : vectorOfImagesReadyForSetting.at(currSpeakerRoomIndex);

				size_t numSpeakerImages      = iSpeakerImages.second.size();
				BYTE indexSpeakerImages      = 0;
				bool isAllSpeakerImagesFound = true,  isFirstSpeakerImage = true;
				for (; indexSpeakerImages < numSpeakerImages; ++indexSpeakerImages)
				{
					BYTE indexSpeakerImageOnCurrentScreen = GetReservedScreenSpeakerImageIndex(linkNum, layoutType, indexSpeakerImages, numSpeakerImages);
					if (indexSpeakerImageOnCurrentScreen != indexSpeakerImages)
						ostr << "\n\nFillLayout - indexSpeakerImages:" << (WORD)indexSpeakerImages << " was replaced to:" << (WORD)indexSpeakerImageOnCurrentScreen;

					std::pair<PartyRsrcID, bool>& partySpeakerImage = iSpeakerImages.second.at(indexSpeakerImageOnCurrentScreen);

					if (partySpeakerImage.second) //skip already used image
						continue;

					WORD indexCurrCell = GetReservedScreenCellIndex(layoutType, pScreenLayoutInfo->m_indexFirstCell, indexSpeakerImageOnCurrentScreen, numSpeakerImages);

					CVidSubImage* pCurrScreenNewCell = (*pScreenLayout)[indexCurrCell];

					ostr << "\n\nFillLayout - try to fill screen:" << linkNum << " cell:" << indexCurrCell << " layout:" << LayoutTypeAsString[pScreenLayout->GetLayoutType()];

					if (numSpeakerImages > 1 && isFirstSpeakerImage)//need to check if all images of this room were displayed before in best or same resolution
					{
						isAllSpeakerImagesFound = IfSpeakerRoomDisplayedInBestResolutionBefore(iSpeakerImages, currRoomLayoutDB, pScreenLayoutInfo, indexCurrCell, ostr);
						isFirstSpeakerImage = false; // no need to check twice
					}
					PartyRsrcID imgPartyId = partySpeakerImage.first;
					if (!imgPartyId) // it can happen if RoomInfo already updated about all sublinks, but PartyCntrl doesn't exist yet
					{
						ostr << "\n\nimgPartyId:" << imgPartyId << " skip image";
						if(!pCurrScreenNewCell)
							PASSERTMSG(indexSpeakerImages, "Speaker Image is NULL");
						else
							pCurrScreenNewCell->SetForceAttributes(OPERATOR_Prior, BLANK_PARTY_Activ);

						continue;
					}
					// the layout shouldn't be changed if the new speaker(s) already in layout and displayed in the largest size
					bool foundImage = isAllSpeakerImagesFound ? FindImagePlaceInRoomLayout(partySpeakerImage, currRoomLayoutDB, pScreenLayoutInfo, indexCurrCell, ostr) : false;
					if (!foundImage)
					{
						OccupyCellByPartyImage(indexCurrCell, pScreenLayoutInfo, partySpeakerImage, false, ostr);
						indexCurrCell += 1;
					}
					--numImagesToSet;

					if (pScreenLayoutInfo->m_numFilledCells == pScreenLayoutInfo->m_numCellsToUse)
						break;
				}
				if (indexSpeakerImages == numSpeakerImages) //it's possible that a speaker cameras will be populated on different local screens (e.g. local=3 and speaker=3)
					currSpeakerRoomIndex += 1;              //so we shouldn't go to a next room up till distribute current speaker cameras
			}
		}
		else if (eGrid == iRoomLayout->second.screenType)
			++gridCount;

	}


	ostr << "\n\nFillLayout - gridCount:" << (WORD)gridCount << " numImagesToSet:" << numImagesToSet;

	// 5. Fill the grid(s) screen layout :
	//    - The grid screens are populated after the reserved screens are populated with the speakers.
	//    - Don't use the filmstrips of reserved screens until the grid screens are fill up
	if (gridCount)
	{
		ostr << "\n start to fill grid(s) layout";
		//request the grid screens layouts according to the speaker logic and remaining images
		pTelepresenceLayoutLogic->GetGridLayout(id, vectorOfImagesReadyForSetting, rLayoutMapPerRoom);

		//add grids layouts info to the internal DB
		CollectRoomScreenLayoutsInfo(localRoomInfo, rLayoutMapPerRoom, currRoomLayoutDB, ostr);

		// print the debugging info
		ostr << "\n\nSummary: Room screen layout";
		CRoomScreensLayoutMap::iterator iRoomLayoutInfoEnd = currRoomLayoutDB.end();
		for (CRoomScreensLayoutMap::iterator iRoomLayoutInfo = currRoomLayoutDB.begin(); iRoomLayoutInfo != iRoomLayoutInfoEnd; ++iRoomLayoutInfo)
		{
			if (iRoomLayoutInfo->second && iRoomLayoutInfo->second->m_pNewLayout)
			{
				ostr << "\n screenPosition:" << iRoomLayoutInfo->first
					 << "  screenType:"      << CTelepresenceSpeakerModeLayoutLogic::ScreenTypeAsString(iRoomLayoutInfo->second->m_newScreenType)
					 << "  layout:"          << LayoutTypeAsString[iRoomLayoutInfo->second->m_pNewLayout->GetLayoutType()];
			}
		}

		if (numImagesToSet) // if there is no images, we still need to send change layout (e.g. 1x1 blank)
		{
			WORD numGridCellsOnAllScreens = GetNumGridCellsOnAllScreens(rLayoutMapPerRoom);

			eFillGridPolicy isFillAllGrids = (numImagesToSet <= numGridCellsOnAllScreens) ? eFillAllGrids : eFillUpTo3x3;

			// The logic of filling the cells is: Use grid cells (up to 3x3) before filling the filmstrip cells.
			// Use 4x4 grid if 3x3 grid and filmstrip are not enough to display all participants.
			// Fill the filmstrip before filling the 4x4 grid, but if number of cells to display is smaller than cells in grid, the grid will be filled 1'st

			ostr << "\n\nFillLayout - before fill grid layouts numImagesToSet:" << numImagesToSet << " numGridCellsOnAllScreens: " << numGridCellsOnAllScreens << " fillGridPolicy:" << (WORD)isFillAllGrids;
			FillGridLayout(rLayoutMapPerRoom, vectorOfImagesReadyForSetting, currRoomLayoutDB, ostr, isFillAllGrids);

			numImagesToSet = CountRemainingImage(vectorOfImagesReadyForSetting);
			ostr << "\n\nFillLayout - after fill grid layouts  new numImagesToSet:" << numImagesToSet;
		}
	}
		
	// 6. swap layouts of Reserved/Grid screens if the endpoints continue to be displayed in the largest size - BRIDGE-15720
	SwapLayoutReservedAndGridScreensIfNeeded(currRoomLayoutDB, ostr);

	// 7. Fill the filmstrip of reserved screens
	if (numImagesToSet)
	{
		ostr << "\n\nFillLayout - before filling filmstrips of reserved screens numImagesToSet:" << numImagesToSet;

		FillLayoutFilmstrip(vectorOfImagesReadyForSetting, currRoomLayoutDB, ostr);

		numImagesToSet = CountRemainingImage(vectorOfImagesReadyForSetting);
		ostr << "\n\nFillLayout - after filling filmstrips of reserved screens new numImagesToSet:" << numImagesToSet;
	}

	// 8. Use 4x4 grid if 3x3 grid and filmstrip are not enough to display all participants. Fill the filmstrips before filling the 4x4 grid.
	if (numImagesToSet)
		FillGridLayout(rLayoutMapPerRoom, vectorOfImagesReadyForSetting, currRoomLayoutDB, ostr, eFillOnly4x4);

	// 9. send change layout request to all changed screens
	SendRoomChangeLayout(currRoomLayoutDB, ostr);


#ifdef EMB_MLA_PRINTOUTS
	TRACEINTO << ostr.str().c_str() << std::endl;
#endif

	// 7. Free the local DB objects
	for_each (currRoomLayoutDB.begin(), currRoomLayoutDB.end(), DelScreenCpLayoutInfoElem);
}


////////////////////////////////////////////////////////////////////////////
bool CLayoutHandlerTelepresenceSpeakerModeCP::IfSpeakerRoomDisplayedInBestResolutionBefore(std::pair<RoomID, CPartyImageInfoVector>& rSpeakerImages,
                                                                                           CRoomScreensLayoutMap&                    rCurrRoomLayoutDB,
                                                                                           ScreenCpLayoutInfo*                       pCurrScreenInfo,
                                                                                           WORD                                      indexCurrCell,
                                                                                           ostringstream&                            ostr)
{
	CRoomScreensLayoutMap::iterator iRoomLayoutInfoEnd = rCurrRoomLayoutDB.end();
	size_t numSpeakerImages                            = rSpeakerImages.second.size();
	bool   allImagesFound                              = true;

	for (BYTE indexSpeakerImages = 0; indexSpeakerImages < numSpeakerImages; ++indexSpeakerImages)
	{
		std::pair<PartyRsrcID, bool>& partySpeakerImage = rSpeakerImages.second.at(indexSpeakerImages);

		PartyRsrcID imgPartyId = partySpeakerImage.first;
		if (imgPartyId) // it can happen if RoomInfo already updated about all sublinks, but PartyCntrl doesn't exist yet
		{
			bool imagePartyFound = false;
			for (CRoomScreensLayoutMap::iterator iRoomLayoutInfo = rCurrRoomLayoutDB.begin(); iRoomLayoutInfo != iRoomLayoutInfoEnd; ++iRoomLayoutInfo)
			{
				ScreenCpLayoutInfo* pPartyLI = iRoomLayoutInfo->second;
				if (!pPartyLI)
					continue;
				PartyRsrcID screenPartyId = (pPartyLI->m_pPartyCntl) ? pPartyLI->m_pPartyCntl->GetPartyRsrcID() : DUMMY_PARTY_ID;

				WORD whereWasSeen = GetImagePlaceInRoomLayout(imgPartyId, pPartyLI, pCurrScreenInfo, indexCurrCell, ostr);

				if (whereWasSeen != AUTO)
				{
					ostr << "\n\nIfSpeakerRoomDisplayedInBestResolutionBefore - checking the roomId:" << rSpeakerImages.first << " partyId:" << imgPartyId << ", ScreenPartyId: " << screenPartyId << " - old resolution is same or best ==> so this screen is preferable";
					imagePartyFound = true;
				}
				else
					ostr << "\n\nIfSpeakerRoomDisplayedInBestResolutionBefore - checking the roomId:" << rSpeakerImages.first << " partyId:" << imgPartyId << ", ScreenPartyId: " << screenPartyId << " - old resolution is worse";
			}
			if (!imagePartyFound)
			{
				ostr << "\n\nIfSpeakerRoomDisplayedInBestResolutionBefore - checking the roomId:" << rSpeakerImages.first << " - not all room images found ==> we don't allow to use a previous layout";
				allImagesFound = false;
				break;
			}
		}
	}
	return allImagesFound;
}

////////////////////////////////////////////////////////////////////////////
bool CLayoutHandlerTelepresenceSpeakerModeCP::FindImagePlaceInRoomLayout(std::pair<PartyRsrcID, bool>& rCurrSpeakerImage,
                                                                         CRoomScreensLayoutMap&        rCurrRoomLayoutDB,
                                                                         ScreenCpLayoutInfo*           pCurrScreenInfo,
                                                                         WORD                          indexCurrCell,
                                                                         ostringstream&                ostr)
{
	CLayout*  pScreenLayout = pCurrScreenInfo ? pCurrScreenInfo->m_pNewLayout : NULL;
	TRACECOND_AND_RETURN_VALUE(!pCurrScreenInfo || !pScreenLayout, "illegal screen info", false);

	CVidSubImage* pCurrScreenNewCell = (*pScreenLayout)[indexCurrCell];
	TRACECOND_AND_RETURN_VALUE(!pCurrScreenNewCell, "illegal screen cell", false);

	WORD positionWhereWasSeen = AUTO;
	PartyRsrcID imgPartyId = rCurrSpeakerImage.first;

	ostr << "\n\nFindImagePlaceInRoomLayout - PartyId:" << imgPartyId;

	CRoomScreensLayoutMap::iterator iRoomLayoutInfoEnd = rCurrRoomLayoutDB.end();
	for (CRoomScreensLayoutMap::iterator iRoomLayoutInfo = rCurrRoomLayoutDB.begin(); iRoomLayoutInfo != iRoomLayoutInfoEnd; ++iRoomLayoutInfo)
	{
		ScreenCpLayoutInfo* pPartyLI = iRoomLayoutInfo->second;

		WORD whereWasSeen = GetImagePlaceInRoomLayout(imgPartyId, pPartyLI, pCurrScreenInfo, indexCurrCell, ostr);

		if (whereWasSeen != AUTO)
		{
			ostr << " ==> so this screen is preferable, because the party was seen in same/best resolution here";
			OccupyCellByPartyImage(whereWasSeen, pPartyLI, rCurrSpeakerImage, false, ostr);
			positionWhereWasSeen = whereWasSeen;
			break;
		}
	}
	return (AUTO != positionWhereWasSeen);
}

////////////////////////////////////////////////////////////////////////////
WORD CLayoutHandlerTelepresenceSpeakerModeCP::GetImagePlaceInRoomLayout(PartyRsrcID         imgPartyId,
                                                                        ScreenCpLayoutInfo* pPrevScreenInfo,
                                                                        ScreenCpLayoutInfo* pCurrScreenInfo,
                                                                        WORD                indexCurrCell,
                                                                        ostringstream&      ostr)
{
	CLayout*  pScreenLayout = pCurrScreenInfo ? pCurrScreenInfo->m_pNewLayout : NULL;
	TRACECOND_AND_RETURN_VALUE(!pCurrScreenInfo || !pScreenLayout, "illegal screen info", false);

	CVidSubImage* pCurrScreenNewCell = (*pScreenLayout)[indexCurrCell];
	TRACECOND_AND_RETURN_VALUE(!pCurrScreenNewCell, "illegal screen cell", false);

	CLayout* pOldLayoutPrevScreen = pPrevScreenInfo->m_pOldLayout;
	CLayout* pNewLayoutPrevScreen = pPrevScreenInfo->m_pNewLayout;
	if (!pOldLayoutPrevScreen || !pNewLayoutPrevScreen)
	{
		ostr << "\n\nGetImagePlaceInRoomLayout - screen:" << pPrevScreenInfo->m_screenPos << " - no layout found";
		return AUTO;
	}
	if (pOldLayoutPrevScreen->GetLayoutType() != pNewLayoutPrevScreen->GetLayoutType())
	{
		ostr << "\n\nGetImagePlaceInRoomLayout - screen:" << pPrevScreenInfo->m_screenPos << " - layout type is changing";
		return AUTO;
	}
	if (pPrevScreenInfo->m_screenPos == pCurrScreenInfo->m_screenPos && pPrevScreenInfo->m_newScreenType != pCurrScreenInfo->m_newScreenType)
	{
		ostr << "\n\nGetImagePlaceInRoomLayout - screen:" << pPrevScreenInfo->m_screenPos << " - screen type is changing from:"
			 << CTelepresenceSpeakerModeLayoutLogic::ScreenTypeAsString(pPrevScreenInfo->m_newScreenType) << " to:"
			 << CTelepresenceSpeakerModeLayoutLogic::ScreenTypeAsString(pCurrScreenInfo->m_newScreenType);
		return AUTO;
	}
	WORD whereWasSeen = pOldLayoutPrevScreen->FindImagePlaceInLayout(imgPartyId);
	if (whereWasSeen != AUTO)
	{
		bool isFilmstripCellInOldLayout = pPrevScreenInfo->IsFilmstripCell(whereWasSeen);
		bool isFilmstripCellInNewLayout = pCurrScreenInfo->IsFilmstripCell(indexCurrCell);

		bool needMoveFromFilmstripToOtherReservedScreen = (eReserved == pCurrScreenInfo->m_newScreenType && !isFilmstripCellInNewLayout && isFilmstripCellInOldLayout);

		ostr << "\n\nGetImagePlaceInRoomLayout - screen:"      << pPrevScreenInfo->m_screenPos
			 << " whereWasSeen:"                               << whereWasSeen
			 << " indexCurrCell:"                              << indexCurrCell
			 << " isFilmstripCellInOldLayout:"                 << (WORD)isFilmstripCellInOldLayout
			 << " isFilmstripCellInNewLayout:"                 << (WORD)isFilmstripCellInNewLayout
			 << " needMoveFromFilmstripToOtherReservedScreen:" << (WORD)needMoveFromFilmstripToOtherReservedScreen;

		const CVidSubImage* pOldCellOnThisScreen = (*pOldLayoutPrevScreen)[whereWasSeen];
		CVidSubImage*       pNewCellOnThisScreen = (*pNewLayoutPrevScreen)[whereWasSeen];
		if (pNewCellOnThisScreen && pOldCellOnThisScreen)
		{
			if (pNewCellOnThisScreen->noImgSet() && pOldCellOnThisScreen->IsEqualOrLargeSize(*pCurrScreenNewCell) && !needMoveFromFilmstripToOtherReservedScreen)
				return whereWasSeen;
			else
				ostr << "\n\nnGetImagePlaceInRoomLayout - Failed - screen:" << pPrevScreenInfo->m_screenPos << " noImgSet: " << (DWORD)pNewCellOnThisScreen->noImgSet() << ", IsEqualOrLargeSize: " << (DWORD)pOldCellOnThisScreen->IsEqualOrLargeSize(*pCurrScreenNewCell);
		}
	}
	else
		ostr << "\n\nGetImagePlaceInRoomLayout - Image was not seen in previous layout imgPartyId: " << imgPartyId;

	return AUTO;
}

////////////////////////////////////////////////////////////////////////////
void CLayoutHandlerTelepresenceSpeakerModeCP::FillLayoutFilmstrip(CRoomSpeakerVector& vectorOfImagesReadyForSetting,
                                                                  CRoomScreensLayoutMap& currRoomLayoutDB,
                                                                  ostringstream& ostr)
{
	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = const_cast<CVideoBridge*> (m_pVideoCntl)->GetTelepresenceLayoutMngr();
	PASSERT_AND_RETURN(!pTelepresenceLayoutMngr || !m_pPartyCntl);

//	When using filmstrip,
//	a.	It is populated with the priority speakers (after the ones in the reserved screens).
//	b.	MCU should try populating 2 camera rooms on the same filmstrip.
//	c.	If there are 3 screens with filmstrip, the center filmstrip should be fully populated and the remaining cells should be spread between the other filmstrip-screens.

	CRoomInfo localRoomInfo = pTelepresenceLayoutMngr->GetRoomInfo(m_pPartyCntl->GetRoomID());

	size_t numSpeakerRoomsForSetting = vectorOfImagesReadyForSetting.size();

	CRoomScreensLayoutMap::iterator iRoomLayoutInfoEnd = currRoomLayoutDB.end();

	for(BYTE indexRoom = 0; indexRoom < numSpeakerRoomsForSetting; ++indexRoom)
	{
		std::pair<RoomID, CPartyImageInfoVector>& iSpeakerImages = vectorOfImagesReadyForSetting.at(indexRoom);

		CPartyImageInfoVector& rPartyImages = iSpeakerImages.second;

		size_t numSpeakerImages = rPartyImages.size();

		if (numSpeakerImages && (0 == rPartyImages.front().second)) //check that a first room image isn't used still
		{
			std::vector<BYTE> indexFreeCellInFilmstripToStart(MAX_CASCADED_LINKS_NUMBER, 0xFF); //max 4 screens
			EScreenPosition        bestFilmstripScreen = ePosNone;
			BYTE numFilledCellsInFilmstripOnBestScreen = 0;

			for (CRoomScreensLayoutMap::iterator ii = currRoomLayoutDB.begin(); ii != iRoomLayoutInfoEnd; ++ii)
			{
				EScreenPosition screenPos               = ii->first;
				ScreenCpLayoutInfo* pScreenCpLayoutInfo = ii->second;
				if (!pScreenCpLayoutInfo || pScreenCpLayoutInfo->m_newScreenType != eReserved || !pScreenCpLayoutInfo->m_filmstrip.m_isHaveFilmstrip)
					continue;

				BYTE firstFilmCell   = pScreenCpLayoutInfo->m_filmstrip.m_indexFilmstripFirstCell;
				BYTE numMaxFilmCells = pScreenCpLayoutInfo->m_filmstrip.m_numFilmstripCellsToUse;
				BYTE numFilledCells  = pScreenCpLayoutInfo->m_filmstrip.m_numFilmstripFilledCells;
				ostr << "\n\nFillLayoutFilmstrip - room:"  << pScreenCpLayoutInfo->m_roomId << " screen:" << ii->first << " firstFilmCell:" << (WORD)firstFilmCell << " numMaxFilmCells:" << (WORD)numMaxFilmCells << " numFilledCells:" << (WORD)numFilledCells;

				CLayout* pNewLayout = pScreenCpLayoutInfo->m_pNewLayout;

				if (pNewLayout && numFilledCells < numMaxFilmCells)
				{
					BYTE indexCellToStart = 0xFF, numFreeCellsInFilm=0;

					for (BYTE icell=firstFilmCell, prevFreeCellIndex = 0xFF; icell<(firstFilmCell+numMaxFilmCells); ++icell)
					{
						if ((*pNewLayout)[icell] && (*pNewLayout)[icell]->noImgSet())
						{
							if (0xFF==prevFreeCellIndex)   //important to check the sequential cells
								indexCellToStart = icell;

							if (icell==(prevFreeCellIndex+1) || 0xFF==prevFreeCellIndex)
							{
								++numFreeCellsInFilm;
								prevFreeCellIndex = icell;
							}
						}
						else if (0xFF != prevFreeCellIndex)
						{
							numFreeCellsInFilm = 0;
							prevFreeCellIndex = 0xFF;
						}
					}
					if (numFreeCellsInFilm >= numSpeakerImages && indexCellToStart != 0xFF)
					{
						indexFreeCellInFilmstripToStart[screenPos] = indexCellToStart;

						EScreenPosition activeSpeakerScreenPos = GetScreenPositionForActiveSpeaker(localRoomInfo);
						//the center filmstrip should be fully populated and the remaining cells should be spread between the other filmstrip-screens
						if (activeSpeakerScreenPos == screenPos || ePosNone == bestFilmstripScreen ||
								(activeSpeakerScreenPos != bestFilmstripScreen && numFilledCells < numFilledCellsInFilmstripOnBestScreen))
						{
							bestFilmstripScreen = screenPos;
							numFilledCellsInFilmstripOnBestScreen = numFilledCells;
						}
					}

				}
			}
			if (bestFilmstripScreen != ePosNone)
			{
				BYTE   indexCellToStart                 = indexFreeCellInFilmstripToStart[bestFilmstripScreen];
				CRoomScreensLayoutMap::iterator ii      = currRoomLayoutDB.find(bestFilmstripScreen);
				ScreenCpLayoutInfo* pScreenCpLayoutInfo = (ii != currRoomLayoutDB.end()) ? ii->second : NULL;
				if (pScreenCpLayoutInfo)
				{
					ostr << "\nFillLayoutFilmstrip - room:" << pScreenCpLayoutInfo->m_roomId << " best screen:" << bestFilmstripScreen << " indexCellToStart:" << (WORD)indexCellToStart;

					BORDER_PARAM_S cell_borders;
					eTelepresenceCellBorders eCellBorderType;
					bool bIsCellFilmstrip = false;
					bool bIsPreviousCellFilmstrip= false;
					for (BYTE indexSpeakerImages=0; indexSpeakerImages < numSpeakerImages; ++indexSpeakerImages)
					{
						std::pair<PartyRsrcID, bool>& rCurrSpeakerImage = rPartyImages.at(indexSpeakerImages);
						if (rCurrSpeakerImage.second) // skip already used image
							continue;
						WORD currIndexCell = GetReservedScreenFilmstripCellIndex(numSpeakerImages, indexCellToStart, indexSpeakerImages);
						////// fill borders for filmstrip //////
						memset(&cell_borders, 0, sizeof(BORDER_PARAM_S));
						GetTelepresenceBordersByTPTypeAndScreenNum(localRoomInfo.GetRoomEPType(), (eRoomScreensNumber)numSpeakerImages, indexSpeakerImages, eCellBorderType, cell_borders);
						if(currIndexCell  < MAX_NUMBER_OF_CELLS_IN_LAYOUT)
						{
							pScreenCpLayoutInfo->m_telepresenceCellBorders.tBorderEdges[currIndexCell] = cell_borders;
							// BRIDGE-15718 - prevent "double" borders - when adjacent cell borders are both drawn (appearing as double thickness) unset right side border
							bIsCellFilmstrip = pScreenCpLayoutInfo->IsFilmstripCell(currIndexCell);
							bIsPreviousCellFilmstrip = (currIndexCell > 0) && pScreenCpLayoutInfo->IsFilmstripCell(currIndexCell-1);
							if (bIsCellFilmstrip && bIsPreviousCellFilmstrip)
							{
								if (pScreenCpLayoutInfo->m_telepresenceCellBorders.tBorderEdges[currIndexCell-1].ucRight == 1)
									pScreenCpLayoutInfo->m_telepresenceCellBorders.tBorderEdges[currIndexCell].ucLeft = 0;
							}
							TRACEINTO << " updating telepresence border of party resource ID " << rCurrSpeakerImage.first << " cell index " << currIndexCell << " with border type " << CellBorderTypeStrings[eCellBorderType];

							OccupyCellByPartyImage(currIndexCell, pScreenCpLayoutInfo, rCurrSpeakerImage, true, ostr);
						}
						else
						{
							PASSERTMSG(currIndexCell, "Index Layout cell is out range");
						}
						//bIsPreviousCellFilmstrip = bIsCellFilmstrip;
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CLayoutHandlerTelepresenceSpeakerModeCP::GetReservedScreenSpeakerImageIndex(EScreenPosition linkNum,
                                                                                 LayoutType layoutType,
                                                                                 BYTE indexSpeakerImages,
                                                                                 WORD numSpeakerImages)
{
	if (CP_LAYOUT_1X2_FLEX == layoutType && FOUR_SCREEN == numSpeakerImages)
	{
		switch (linkNum)
		{
			case ePos1:
				if (0==indexSpeakerImages) //main link
					return 0;
				else if (1==indexSpeakerImages)
					return 2;
				break;
			case ePos2:
				if (0==indexSpeakerImages)
					return 1;
				else if (1==indexSpeakerImages)
					return 3;
				break;
			default:
				return indexSpeakerImages;
		}
	}
	return indexSpeakerImages;
}
////////////////////////////////////////////////////////////////////////////

WORD CLayoutHandlerTelepresenceSpeakerModeCP::GetReservedScreenCellIndex(LayoutType layoutType, BYTE indexFirstCell, BYTE indexSpeakerImages, WORD numSpeakerImages)
{
	WORD indexCurrCell = indexFirstCell;
	switch(layoutType)
	{
		case CP_LAYOUT_OVERLAY_ITP_1P4:
			indexCurrCell = indexFirstCell;
			break;
		case CP_LAYOUT_1X2_FLEX:
			if (FOUR_SCREEN == numSpeakerImages)
			{
				if (0==indexSpeakerImages || 3==indexSpeakerImages) //main link
					indexCurrCell = indexFirstCell;
				else
					indexCurrCell = indexFirstCell + 1;
			}
			else
			{
				if (0==indexSpeakerImages) //main link
					indexCurrCell = indexFirstCell + 1;
				else
					indexCurrCell = indexFirstCell;
			}
			break;
		case CP_LAYOUT_3X3:
			if (0==indexSpeakerImages) //main link
				indexCurrCell = indexFirstCell + 1;
			else if (1==indexSpeakerImages)
				indexCurrCell = indexFirstCell;
			else
				indexCurrCell = indexFirstCell + 2;
			break;
		case CP_LAYOUT_4X4:
			if (0==indexSpeakerImages) //main link
				indexCurrCell = indexFirstCell + 2;
			else if (1==indexSpeakerImages)
				indexCurrCell = indexFirstCell + 1;
			else if (2==indexSpeakerImages)
				indexCurrCell = indexFirstCell + 3;
			else
				indexCurrCell = indexFirstCell;
			break;
		default:
			DBGPASSERT(layoutType+1);
			break;
	}
	return indexCurrCell;
}
////////////////////////////////////////////////////////////////////////////

WORD CLayoutHandlerTelepresenceSpeakerModeCP::GetReservedScreenFilmstripCellIndex(WORD numSpeakerImages, BYTE indexFirstCell, BYTE indexSpeakerImages)
{
	WORD indexCurrCell = indexFirstCell;
	switch (numSpeakerImages)
	{
		case ONE_SCREEN:	indexCurrCell = indexFirstCell; break;
		case TWO_SCREEN:	indexCurrCell = (0==indexSpeakerImages ? indexFirstCell+1 : indexFirstCell); break;
		case THREE_SCREEN:	indexCurrCell = (0==indexSpeakerImages ? indexFirstCell+1 : (1==indexSpeakerImages ? indexFirstCell : indexFirstCell+2)); break;
		case FOUR_SCREEN:	indexCurrCell = (0==indexSpeakerImages ? indexFirstCell+2 :
				(1==indexSpeakerImages ? indexFirstCell+1 : (2==indexSpeakerImages ? indexFirstCell+3 : indexFirstCell))); break;
		default:
			DBGPASSERT(numSpeakerImages+1);
			break;
	}
	return indexCurrCell;
}
////////////////////////////////////////////////////////////////////////////

TelepresenceCpModeLayout::iterator CLayoutHandlerTelepresenceSpeakerModeCP::GetScreenLayoutByPosition(TelepresenceCpModeLayout& rLayoutMap,
                                                                                                      EScreenPosition           index,
                                                                                                      CRoomInfo&                rLocalRoomInfo,
                                                                                                      WORD                      numSpeakerImages)
{
	if (FOUR_SCREEN == rLocalRoomInfo.GetNumberOfActiveLinks())
	{
		// Location of the screen numbers when building the layout for RPX 400/ATX 400 :  Left:4(prev speaker)  Center-Left:2(speaker)  Center-Right:1(third speaker) Right:3
		switch(index)
		{
			case ePos1:
				return (numSpeakerImages != FOUR_SCREEN ? rLayoutMap.find(ePos2) : rLayoutMap.find(ePos1));
			case ePos2:
				return (numSpeakerImages != FOUR_SCREEN ? rLayoutMap.find(ePos4) : rLayoutMap.find(ePos2));
			case ePos3:
				return (numSpeakerImages != FOUR_SCREEN ? rLayoutMap.find(ePos1) : rLayoutMap.find(ePos4));
			case ePos4:
				return (numSpeakerImages != FOUR_SCREEN ? rLayoutMap.find(ePos3) : rLayoutMap.find(ePos3));
			default:
				return rLayoutMap.find(index);
		}
	}
	return rLayoutMap.find(index);
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CLayoutHandlerTelepresencePartyModeCP::CLayoutHandlerTelepresencePartyModeCP(const CVideoBridge* videoCntl, CVideoBridgePartyCntl* partyCntl) :
				CLayoutHandlerTelepresenceCP(videoCntl, partyCntl)
{}

////////////////////////////////////////////////////////////////////////////
CLayoutHandlerTelepresencePartyModeCP::~CLayoutHandlerTelepresencePartyModeCP()
{}

////////////////////////////////////////////////////////////////////////////

void CLayoutHandlerTelepresencePartyModeCP::FillLayout()
{
	CTelepresenceLayoutMngr* pTelepresenceLayoutMngr = const_cast<CVideoBridge*> (m_pVideoCntl)->GetTelepresenceLayoutMngr();
	PASSERT_AND_RETURN(!pTelepresenceLayoutMngr);
	CTelepresencePartyModeLayoutLogic* pTelepresenceLayoutLogic = pTelepresenceLayoutMngr->GetTelepresencePartyModeLayoutLogic();
	PASSERT_AND_RETURN(!pTelepresenceLayoutLogic);
	CPartyImageLookupTable* pPartyImageLookupTable = GetPartyImageLookupTable();
	PASSERT_AND_RETURN(!pPartyImageLookupTable);

	const CTelepresenseEPInfo& tpInfo = m_pPartyCntl->GetTelepresenceInfo();
	RoomID id = tpInfo.GetRoomID();

	ostringstream ostr;
	ostr << "EMB_MLA_OLGA - room:" << id << ", partyId:" << m_pPartyCntl->GetPartyRsrcID();

	// 1. Create an image vector sorted by speaker order but organized by room id
	CRoomSpeakerVector vectorOfImagesReadyForSetting;
	WORD numImagesToSet = CreateVectorOfRoomImagesReadyForSetting(vectorOfImagesReadyForSetting, ostr);

	size_t numSpeakerRoomsForSetting = vectorOfImagesReadyForSetting.size();

	if (0 == numSpeakerRoomsForSetting) //may be the case if no decoder is connected in conf and is unmuted
	{
		SetBlackScreen(id, true);
		TRACECOND_AND_RETURN(1, "empty vectorOfImagesReadyForSetting - return");
	}

	if ((numSpeakerRoomsForSetting == 1) && (vectorOfImagesReadyForSetting[0].first == id))
	{
		SetBlackScreen(id, true);
		TRACECOND_AND_RETURN(1, " vectorOfImagesReadyForSetting has self image(s) only - return");
	}

	// 2. Get the room (all screens together) layouts according to Participants priority logic and current speakers list
	TelepresenceCpModeLayout rLayoutMapPerRoom;
	pTelepresenceLayoutLogic->GetRoomLayout(id, vectorOfImagesReadyForSetting, rLayoutMapPerRoom);

	pTelepresenceLayoutLogic->GetGridLayout(id, vectorOfImagesReadyForSetting, rLayoutMapPerRoom);

	size_t sizeLayouts = rLayoutMapPerRoom.size();

	// remove the room self images
	if (numSpeakerRoomsForSetting != 0)
		numImagesToSet = RemoveSelfImagesFromRoomImagesReadyForSetting(id, vectorOfImagesReadyForSetting);

	CRoomInfo localRoomInfo = pTelepresenceLayoutMngr->GetRoomInfo(id);

	BYTE localScreensNumber = localRoomInfo.GetNumberOfActiveLinks();
	TRACECOND_AND_RETURN( (sizeLayouts != localScreensNumber), "wrong size of the vector layouts - return");

	// 3. Create a local DB to store all needed layout information for whole room in order to accelerate data extraction about each party in this room

	CRoomScreensLayoutMap currRoomLayoutDB;
	CollectRoomScreenLayoutsInfo(localRoomInfo, rLayoutMapPerRoom, currRoomLayoutDB, ostr);

/* 4. Fill the layout of screens according Participants priority logic
	Using this mode means we want to show as many participants as possible using the biggest cell size.
	All screens are grid screens, applying the same grid screens logic as in speaker priority mode.
	1. At least one screen needs a grid size to accommodate max # cameras in conference (w/o local).
	2. When there are multiple grid-screens, the grid gradually changes from 1x1, to 2x2 to 3x3.
       Only when all grids are the same (like 2x2) we start with next grid (like 3x3) 1'st on one screen, than on the other(s).
	3. Central screen should display the smallest grid (larger cells).
	4. Speaker should be located in the smallest grid available which can contain the speaker.
	5. Fill the lower grid (2x2) before filling higher grid (3x3). */

	if (numImagesToSet) // if there is no images, we still need to send change layout (e.g. 1x1 blank)
	{
		FillGridLayout(rLayoutMapPerRoom, vectorOfImagesReadyForSetting, currRoomLayoutDB, ostr);

		DWORD numRemainingImages = CountRemainingImage(vectorOfImagesReadyForSetting);
		ostr << "\n\nFillLayout - before fill grid layouts numImagesToSet:" << numImagesToSet << ", numRemainingImages:" << numRemainingImages;
	}

	//send change layout request to all changed screens
	SendRoomChangeLayout(currRoomLayoutDB, ostr);

#ifdef EMB_MLA_PRINTOUTS
	TRACEINTO << ostr.str().c_str() << std::endl;
#endif

	// 7. Free the local DB objects
	for_each (currRoomLayoutDB.begin(), currRoomLayoutDB.end(), DelScreenCpLayoutInfoElem);

}
