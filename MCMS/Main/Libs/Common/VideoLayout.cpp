// VideoLayout.cpp: implementation of the CVideoLayout class.
//
////////////////////////////////////////////////////////////////////////////

#include "NStream.h"
#include "VideoLayout.h"
#include "VideoCellLayout.h"
#include "psosxml.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "ObjString.h"
#include <iostream>
#include <iomanip>
using namespace std;


////////////////////////////////////////////////////////////////////////////
//                        CVideoLayout
////////////////////////////////////////////////////////////////////////////
CVideoLayout::CVideoLayout()
             :CPObject()
{
  m_screenLayout      = ONE_ONE;
  m_active            = YES;
  m_numb_of_cell      = 0;
  memset(m_pCellLayout, 0, sizeof(m_pCellLayout));
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout::CVideoLayout(BYTE screenLayout, BYTE isActive)
{
  m_screenLayout     = screenLayout;
  m_active           = isActive;
  m_numb_of_cell     = Layout2SrcNum(m_screenLayout);
  memset(m_pCellLayout, 0, sizeof(m_pCellLayout));
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout::CVideoLayout(const CVideoLayout &other)
             :CPObject(other)
{
  m_screenLayout      = other.m_screenLayout;
  m_active            = other.m_active;
  m_numb_of_cell      = other.m_numb_of_cell;

  for (int i = 0; i < MAX_CELL_NUMBER; i++)
  {
    if (other.m_pCellLayout[i] == NULL)
      m_pCellLayout[i] = NULL;
    else
      m_pCellLayout[i] = new CVideoCellLayout(*other.m_pCellLayout[i]);
  }
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout::~CVideoLayout()
{
  for (int i = 0; i < MAX_CELL_NUMBER; i++)
    POBJDELETE(m_pCellLayout[i]);
}

////////////////////////////////////////////////////////////////////////////
void CVideoLayout::Serialize(WORD format, CSegment& seg)
{
  // assuming format = OPERATOR_MCMS
  seg << (WORD)m_screenLayout;
  seg << (WORD)m_numb_of_cell;

  for (WORD i = 0; i < m_numb_of_cell; i++)
    if (m_pCellLayout[i] != NULL)
      m_pCellLayout[i]->Serialize(format, seg);

  seg << (WORD)m_active;
}

////////////////////////////////////////////////////////////////////////////
void CVideoLayout::Serialize(WORD format, std::ostream& m_ostr)
{
  // assuming format = OPERATOR_MCMS
  m_ostr << (WORD)m_screenLayout << "\n";
  m_ostr << (WORD)m_numb_of_cell << "\n";

  for (WORD i = 0; i < m_numb_of_cell; i++)
    if (m_pCellLayout[i] != NULL)
      m_pCellLayout[i]->Serialize(format, m_ostr);

  m_ostr << (WORD)m_active << "\n";
}

////////////////////////////////////////////////////////////////////////////
void CVideoLayout::DeSerialize(WORD format, CSegment& seg)
{
  WORD tmp;
  seg >> tmp; m_screenLayout = (BYTE)tmp;
  seg >> m_numb_of_cell;

  for (int i = 0; i < (int)m_numb_of_cell; i++)
  {
    m_pCellLayout[i] = new CVideoCellLayout();
    m_pCellLayout[i]->DeSerialize(format, seg);
  }

  seg >> tmp; m_active = (BYTE)tmp;
}

////////////////////////////////////////////////////////////////////////////
void CVideoLayout::DeSerialize(WORD format, std::istream& m_istr)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  m_istr >> tmp; m_screenLayout = (BYTE)tmp;
  m_istr >> m_numb_of_cell;

  for (int i = 0; i < (int)m_numb_of_cell; i++)
  {
    m_pCellLayout[i] = new CVideoCellLayout();
    m_pCellLayout[i]->DeSerialize(format, m_istr);
  }

  m_istr >> tmp; m_active = (BYTE)tmp;
}

////////////////////////////////////////////////////////////////////////////
int CVideoLayout::DeSerializeXml(CXMLDOMElement* pForceNode, char* pszError, BYTE bIsPrivate)
{
  CXMLDOMElement *pCellNode;
  int nStatus;
  BYTE nID;

  GET_VALIDATE_MANDATORY_CHILD(pForceNode, "LAYOUT", &m_screenLayout, LAYOUT_ENUM);

  GET_FIRST_CHILD_NODE(pForceNode, "CELL", pCellNode);

  for (int i = 0; i < MAX_CELL_NUMBER; i++)
    POBJDELETE(m_pCellLayout[i]);
  m_numb_of_cell = 0 ;
  
  while (pCellNode) //no need to check ranges (done with AddCell)
  {
    CVideoCellLayout CellLayout;

    BOOL bCellExist = FALSE;
    GET_VALIDATE_MANDATORY_CHILD(pCellNode, "ID", &nID, _0_TO_DWORD);

    //printf("====CVideoLayout::DeSerializeXml cellId %d\n",nID);
    CVideoCellLayout* pCellLayout = GetCurrentCell(nID);
    if (pCellLayout)
      bCellExist = TRUE;

    nStatus = CellLayout.DeSerializeXml(pCellNode, pszError, bIsPrivate);
    if (nStatus != STATUS_OK)
      return nStatus;

    if (bCellExist)
    {
    	//printf ("====CVideoLayout::FindCell nID %d \n",nID);
    	int ind = FindCell(nID);
    	/// Romem klocwork
    	if((ind >= 0) && (ind < MAX_CELL_NUMBER))
    	{
    		POBJDELETE(m_pCellLayout[ind]);
    		m_pCellLayout[ind] = new CVideoCellLayout(CellLayout);
    	}
    	else
    	{
    		PTRACE2INT(eLevelInfoNormal, "CCVideoLayout::DeSerializeXml = cell index is wrong, indes is: ",ind);
    		return STATUS_CELL_ID_NOT_EXISTS;
    	}
    }
    else
      AddCell(CellLayout);
    GET_NEXT_CHILD_NODE(pForceNode, "CELL", pCellNode);
  }

  //printf ("====CVideoLayout::DeSerializeXml nunmber of cell %d\n",m_numb_of_cell);
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CVideoLayout::SerializeXml(CXMLDOMElement *pForceNode)
{
  pForceNode->AddChildNode("LAYOUT", GetScreenLayoutITP(), LAYOUT_ENUM);

	WORD num_of_cells = m_numb_of_cell;
	if (GetScreenLayoutITP() == ONE_PLUS_TWO_OVERLAY_ITP)
		num_of_cells = 3;
	
  for (int i = 0; i < num_of_cells; i++)
  {
    CVideoCellLayout* pCell = GetCurrentCell(i+1);
    if (pCell)
      pCell->SerializeXml(pForceNode);
  }
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoLayout::GetScreenLayoutITP()
{
	BYTE layout = m_screenLayout;
	if (layout == ONE_PLUS_THREE_OVERLAY_ITP || layout == ONE_PLUS_FOUR_OVERLAY_ITP)
		layout = ONE_PLUS_TWO_OVERLAY_ITP;
  return layout;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoLayout::GetScreenLayout() const
{
  return m_screenLayout;
}

////////////////////////////////////////////////////////////////////////////
void CVideoLayout::SetScreenLayout(const BYTE screenLayout)
{
  m_screenLayout = screenLayout;
}

////////////////////////////////////////////////////////////////////////////
void CVideoLayout::SetActive(const BYTE state)
{
  m_active = state;
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoLayout::IsActive() const
{
  return m_active;
}

////////////////////////////////////////////////////////////////////////////
int CVideoLayout::AddCell(const CVideoCellLayout& other)
{
  if (m_numb_of_cell >= MAX_CELL_NUMBER)
    return STATUS_ILLEGAL;

  if (FindCell(other) != NOT_FIND)
    return STATUS_CELL_ID_EXISTS;

  //Search for the same party in another cell
  const char* party_name = other.GetName();
  if (strcmp(party_name, "[Black Screen]") && strcmp(party_name, "[Auto Select]"))
  {
    int ind = FindCell(party_name);
    if (ind != NOT_FIND)
      return STATUS_PARTY_IS_PRESENT_IN_SEVERAL_CELLS;
  }

  POBJDELETE(m_pCellLayout[m_numb_of_cell]);
  m_pCellLayout[m_numb_of_cell] = new CVideoCellLayout(other);
  m_numb_of_cell++;
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CVideoLayout::UpdateCell(const CVideoCellLayout &other)
{
  int ind = FindCell(other);
  if (ind == NOT_FIND)
    return STATUS_CELL_ID_NOT_EXISTS;

  //Search for the same party in another cell
  const char* party_name = other.GetName();
  if (strcmp(party_name, "[Black Screen]") && strcmp(party_name, "[Auto Select]"))
  {
    int ind = FindCell(party_name);
    if (ind != NOT_FIND)
      return STATUS_PARTY_IS_PRESENT_IN_SEVERAL_CELLS;
  }
  // Romem klock =work
  if(ind >=  MAX_CELL_NUMBER)
  {
	  PTRACE2INT(eLevelInfoNormal, "CCVideoLayout::DeSerializeXml = cell index is wrong, indes is: ",ind);
	  return STATUS_CELL_ID_NOT_EXISTS;
  }
  POBJDELETE(m_pCellLayout[ind]);
  m_pCellLayout[ind] = new CVideoCellLayout(other);
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CVideoLayout::FindCell(const CVideoCellLayout &other)
{
  for (int i = 0; i < (int)m_numb_of_cell; i++)
    if (m_pCellLayout[i]->GetCellId() == other.GetCellId())
      return i;
  return NOT_FIND;
}

////////////////////////////////////////////////////////////////////////////
int CVideoLayout::FindCell(const BYTE cellId)
{
  for (int i = 0; i < (int)m_numb_of_cell; i++)
    if (m_pCellLayout[i]->GetCellId() == cellId)
      return i;
  return NOT_FIND;
}

////////////////////////////////////////////////////////////////////////////
int CVideoLayout::FindCell(const char* partyName)
{
  if (partyName && strlen(partyName))
  {
    for (int i = 0; i < (int)m_numb_of_cell; i++)
      if (!strncmp(m_pCellLayout[i]->GetName(), partyName, H243_NAME_LEN))
        return i;
  }
  return NOT_FIND;
}

////////////////////////////////////////////////////////////////////////////
int CVideoLayout::CancelCell(const BYTE cellID)
{
  int ind = FindCell(cellID);
  if (ind == NOT_FIND)
    return STATUS_CELL_ID_NOT_EXISTS;
  // Romem klocwork
  if(ind >= MAX_CELL_NUMBER)
	return STATUS_CELL_ID_NOT_EXISTS;

  POBJDELETE(m_pCellLayout[ind]);

  PASSERTSTREAM_AND_RETURN_VALUE(m_numb_of_cell > MAX_CELL_NUMBER,
    "m_numb_of_cell has invalid value " << m_numb_of_cell, STATUS_FAIL);
  
  WORD emptyIndex;
  for (emptyIndex = 0; emptyIndex < m_numb_of_cell; emptyIndex++)
    if (m_pCellLayout[emptyIndex] == NULL)
      break;

  for (WORD i = emptyIndex; i < m_numb_of_cell-1; i++)
    m_pCellLayout[i] = m_pCellLayout[i+1];

  m_pCellLayout[m_numb_of_cell-1] = NULL;
  m_numb_of_cell--;
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CVideoLayout::Layout2SrcNum(int nLayout)
{
  switch (nLayout)
  {
    case ONE_ONE:
    case ONE_ONE_QCIF:
      return 1;

    case ONE_TWO:
    case TWO_ONE:
    case ONE_TWO_VER:
    case ONE_TWO_HOR:
      return 2;

    case ONE_PLUS_TWO_HOR:
    case ONE_PLUS_TWO_VER:
    case ONE_PLUS_TWO_HOR_UPPER:
      return 3;

    case TWO_TWO:
    case ONE_PLUS_THREE_HOR:
    case ONE_PLUS_THREE_VER:
    case ONE_PLUS_THREE_HOR_UPPER:
      return 4;

    case ONE_PLUS_FOUR_VER:
    case ONE_PLUS_FOUR_HOR:
    case ONE_PLUS_FOUR_HOR_UPPER:
      return 5;

    case ONE_AND_FIVE:
      return 6;

    case ONE_SEVEN:
      return 8;

    case THREE_THREE:
    case ONE_PLUS_EIGHT_CENTRAL:
    case ONE_PLUS_EIGHT_UPPER:
    case ONE_TOP_LEFT_PLUS_EIGHT:
    case ONE_PLUS_EIGTH:
      return 9;

    case TWO_PLUS_EIGHT:
    case TWO_TOP_PLUS_EIGHT:
      return 10;

    case ONE_PLUS_TWELVE:
      return 13;

    case FOUR_FOUR:
      return 16;

 	case ONE_PLUS_ONE_OVERLAY:
		return 2;

	case ONE_PLUS_TWO_OVERLAY:
	case ONE_PLUS_TWO_OVERLAY_ITP:
		return 3;

	case ONE_PLUS_THREE_OVERLAY:
	case ONE_PLUS_THREE_OVERLAY_ITP:
		return 4;

	case ONE_PLUS_FOUR_OVERLAY_ITP:
		return 5;
  }
  return 1;
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout& CVideoLayout::operator =(CVideoLayout& other)
{
  if (&other == this)
    return *this;

  m_screenLayout      = other.m_screenLayout;
  m_active            = other.m_active;
  m_numb_of_cell      = other.m_numb_of_cell;

  for (int i = 0; i < MAX_CELL_NUMBER; i++)
  {
    POBJDELETE(m_pCellLayout[i]);
    if (other.m_pCellLayout[i] != NULL)
      m_pCellLayout[i] = new CVideoCellLayout(*other.m_pCellLayout[i]);
  }
  return *this;
}

////////////////////////////////////////////////////////////////////////////
CVideoCellLayout* CVideoLayout::GetCurrentCell(const BYTE cellID)
{
  for (int i = 0; i < (int)m_numb_of_cell; i++)
  {
    if (m_pCellLayout[i] != NULL)
    {
      if (m_pCellLayout[i]->GetCellId() == cellID)
        return m_pCellLayout[i];
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoLayout::GetNumCells(const DWORD partyID)
{
  WORD num = 0;

  for (int i = 0; i < (int)m_numb_of_cell; i++)
  {
    if (m_pCellLayout[i] != NULL)
    {
      switch (m_pCellLayout[i]->GetCellStatus())
      {
        case EMPTY_BY_OPERATOR_THIS_PARTY:
        case EMPTY_BY_OPERATOR_ALL_CONF:
        case AUDIO_ACTIVATED:
        case AUTO:
          continue;
      }
      if (m_pCellLayout[i]->GetForcedPartyId() == partyID)
        num++;
    }
  }
  return num;
}

////////////////////////////////////////////////////////////////////////////
void CVideoLayout::DeletePartyFromCells(const char* name)
{
  for (WORD i = 0; i < m_numb_of_cell; i++)
  {
    if (m_pCellLayout[i] != NULL)
      if (!strcmp(m_pCellLayout[i]->GetName(), name))
        m_pCellLayout[i]->SetCellStatus(AUDIO_ACTIVATED);
  }
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoLayout::GetMaxNumberOfCells(DWORD apiNum)
{
  return Layout2SrcNum(FOUR_FOUR);
}

////////////////////////////////////////////////////////////////////////////
void CVideoLayout::SetCellStatusToAllArr(BYTE newStatus, bool isOverrideProfileLayout)
{
	//BRIDGE-11956
	if (isOverrideProfileLayout)
	{
		for (int i = 0; i < m_numb_of_cell; ++i)
		{
			if(m_pCellLayout[i]->GetCellStatus()!=EMPTY_BY_OPERATOR_ALL_CONF && //Blank
				m_pCellLayout[i]->GetCellStatus()!= BY_OPERATOR_ALL_CONF && //Party force name
				m_pCellLayout[i]->GetCellStatus() != AUTO_SCAN)//BRIDGE-4014

					m_pCellLayout[i]->SetCellStatus(newStatus);
		}
	}
	else
	{
		for (int i = 0; i < m_numb_of_cell; ++i)
		{
			if (m_pCellLayout[i]->GetCellStatus() != AUTO_SCAN)//BRIDGE-4014
					m_pCellLayout[i]->SetCellStatus(newStatus);
		}
	}
}
////////////////////////////////////////////////////////////////////////////
CVideoLayout* CVideoLayout::CreateGatheringLayout()
{
  CVideoLayout* pLayout = new CVideoLayout();
  pLayout->m_active       = 1;
  pLayout->m_numb_of_cell = 5;
  pLayout->m_screenLayout = ONE_PLUS_FOUR_VER;

  char* arrStr[2] = {"[Black Screen]", "[Auto Select]"};
  for (int i = 0; i < (int)(pLayout->m_numb_of_cell); i++)
  {
    pLayout->m_pCellLayout[i] = new CVideoCellLayout;
    pLayout->m_pCellLayout[i]->m_cellId = i + 1;
    pLayout->m_pCellLayout[i]->m_cellStatus = (i ? 0 : 1);
    pLayout->m_pCellLayout[i]->m_forcedPartyId = 0xFFFFFFFF;
    pLayout->m_pCellLayout[i]->m_currentPartyId = 0xFFFFFFFF;
    pLayout->m_pCellLayout[i]->SetName(arrStr[i ? 1 : 0]);
  }
  return pLayout;
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout* CVideoLayout::CreateNonGatheringLayout()
{
  CVideoLayout* pLayout = new CVideoLayout();
  pLayout->m_active       = 0;
  pLayout->m_numb_of_cell = 5;
  pLayout->m_screenLayout = ONE_PLUS_FOUR_VER;

  for (int i = 0; i < (int)(pLayout->m_numb_of_cell); i++)
  {
    pLayout->m_pCellLayout[i] = new CVideoCellLayout;
    pLayout->m_pCellLayout[i]->m_cellId = i + 1;
    pLayout->m_pCellLayout[i]->m_cellStatus = AUDIO_ACTIVATED;
    pLayout->m_pCellLayout[i]->m_forcedPartyId = 0xFFFFFFFF;
    pLayout->m_pCellLayout[i]->m_currentPartyId = 0xFFFFFFFF;
    pLayout->m_pCellLayout[i]->SetName("[Auto Select]");
  }
  return pLayout;
}

////////////////////////////////////////////////////////////////////////////
bool CVideoLayout::IsGatheringLayout()
{
  if (m_active != 1)
  {
    PTRACE(eLevelInfoNormal, "CVideoLayout::IsGatheringLayout = false: m_active != 1");
    return false;
  }
  if (m_numb_of_cell != 5)
  {
    PTRACE(eLevelInfoNormal,"CVideoLayout::IsGatheringLayout = false: m_numb_of_cell != 5");
    return false;
  }
  if (m_screenLayout != ONE_PLUS_FOUR_VER)
  {
    PTRACE(eLevelInfoNormal,"CVideoLayout::IsGatheringLayout = false: m_screenLayout != ONE_PLUS_FOUR_VER");
    return false;
  }

  CSmallString sstr;
  for (int i = 0; i < (int)m_numb_of_cell; i++)
  {
    if (m_pCellLayout[i]->m_cellId != i + 1)
    {
      sstr << "m_cellId != i + 1  , i = " << i;
      PTRACE2(eLevelInfoNormal,"CVideoLayout::IsGatheringLayout = false: ",sstr.GetString());
      return false;
    }
    if (i == 0 && !m_pCellLayout[i]->IsBlank() && !m_pCellLayout[i]->IsAudioActivated())
    {
      PTRACE(eLevelInfoNormal,"CVideoLayout::IsGatheringLayout = false: cell 0 is not set to Blank and not set to Audio Activated");
      return false;
    }
    if (i > 0 && !m_pCellLayout[i]->IsAudioActivated())
    {
      sstr << "cell " << i << " is not set to audio activate";
      PTRACE2(eLevelInfoNormal,"CVideoLayout::IsGatheringLayout = false: ",sstr.GetString());
      return false;
    }
    if (m_pCellLayout[i]->m_forcedPartyId != 0xFFFFFFFF)
    {
      sstr << "cell " << i << " m_forcedPartyId != 0xFFFFFFFF";
      PTRACE2(eLevelInfoNormal,"CVideoLayout::IsGatheringLayout = false: ",sstr.GetString());
      return false;
    }
  }
  PTRACE(eLevelInfoNormal,"CVideoLayout::IsGatheringLayout = true");
  return true;
}

////////////////////////////////////////////////////////////////////////////
void CVideoLayout::ToString(std::string& sLayout)
{
  std::ostringstream str;
  str << "  isActive            :" << (m_active ? "true" : "false") << endl;
  str << "  numberOfCells       :" << (int)m_numb_of_cell << endl;
  str << "  layoutType          :" << LayoutTypeToString(m_screenLayout) << endl;

  str << "VIDEO_CELL_LAYOUT:\n  ";
  str << right << setw( 9) << "CellIndex |"
               << setw( 9) << " CellId |"
               << setw(13) << " CellStatus |"
               << setw(15) << " ForcedPartyId |"
               << setw(16) << " CurrentPartyId |"
               << setw(10) << " PartyName"<< endl;

  for (int i = 0; i < (int)m_numb_of_cell; i++)
  {
    str << right << setw(11) << i << " |"
                 << setw( 7) << (int)(m_pCellLayout[i]->m_cellId) << " |"
                 << setw(11) << (int)(m_pCellLayout[i]->m_cellStatus) << " |"
                 << setw(14) << (long)(m_pCellLayout[i]->m_forcedPartyId) << " |"
                 << setw(15) << (long)(m_pCellLayout[i]->m_currentPartyId) << " |"
                 << " " << left << m_pCellLayout[i]->GetName() << endl;
  }
  sLayout = str.str();
}

////////////////////////////////////////////////////////////////////////////
std::string CVideoLayout::LayoutTypeToString(int layoutType)
{
  std::ostringstream str;
  str << layoutType;

  const char *layoutTypeAsString = NULL;
  CStringsMaps::GetDescription(LAYOUT_ENUM, layoutType, &layoutTypeAsString);
  if (layoutTypeAsString)
    str << " (" << layoutTypeAsString << ")";

  return str.str();;
}
/////////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin   -- input: 1: 1X1, 2: 1X2
void CVideoLayout::SetLayoutForRecording(enSrsVideoLayoutType  layout)
{
	m_active		= 1;
	m_numb_of_cell = 0;
	
	switch(layout)
	{
		case eSrsVideoLayout1X1:
		{
			char* arrStr= "[Auto Select]";
			m_pCellLayout[0] = new CVideoCellLayout;
			m_pCellLayout[0]->m_cellId = 1;
			m_pCellLayout[0]->m_cellStatus = AUDIO_ACTIVATED;
			m_pCellLayout[0]->m_forcedPartyId = 0xFFFFFFFF;
			m_pCellLayout[0]->m_currentPartyId = 0xFFFFFFFF;
			m_pCellLayout[0]->SetName(arrStr);
			m_numb_of_cell  =1;
			m_screenLayout  = ONE_ONE;
			break;
		}
		case eSrsVideoLayout1X2:
		{
			
			m_pCellLayout[0] = new CVideoCellLayout;
			m_pCellLayout[0]->m_cellId = 1;
			m_pCellLayout[0]->m_cellStatus = AUDIO_ACTIVATED;
			m_pCellLayout[0]->m_forcedPartyId = 0xFFFFFFFF;
			m_pCellLayout[0]->m_currentPartyId = 0xFFFFFFFF;
			m_pCellLayout[0]->SetName("[Auto Select]");

			m_pCellLayout[1] = new CVideoCellLayout;
			m_pCellLayout[1]->m_cellId = 2;
			m_pCellLayout[1]->m_cellStatus = AUDIO_ACTIVATED;
			m_pCellLayout[1]->m_forcedPartyId = 0xFFFFFFFF;
			m_pCellLayout[1]->m_currentPartyId = 0xFFFFFFFF;
			m_pCellLayout[1]->SetName("[Auto Select]");
			m_numb_of_cell =2;
			m_screenLayout  = ONE_TWO;
			break;
		}
		default:
		break;
	}
	return;
}

