// VideoCellLayout.cpp: implementation of the CVideoCellLayout class.
//
////////////////////////////////////////////////////////////////////////////

#include "NStream.h"
#include "VideoCellLayout.h"
#include "psosxml.h"
#include "StatusesGeneral.h"

////////////////////////////////////////////////////////////////////////////
//                        CVideoCellLayout
////////////////////////////////////////////////////////////////////////////
CVideoCellLayout::CVideoCellLayout()
                 :CPObject()
{
  m_cellId          = 0;
  m_cellStatus      = 0;
  m_currentPartyId  = 0xFFFFFFFF;
  m_forcedPartyId   = 0xFFFFFFFF;
  memset(m_H243_partyName, 0, sizeof(m_H243_partyName));
}

////////////////////////////////////////////////////////////////////////////
CVideoCellLayout::CVideoCellLayout(const CVideoCellLayout& other)
                 :CPObject(other)
{
  m_cellId          = other.m_cellId;
  m_cellStatus      = other.m_cellStatus;
  m_currentPartyId  = other.m_currentPartyId;
  m_forcedPartyId   = other.m_forcedPartyId;
  SetName(other.m_H243_partyName);
}

////////////////////////////////////////////////////////////////////////////
CVideoCellLayout::~CVideoCellLayout()
{
}

////////////////////////////////////////////////////////////////////////////
CVideoCellLayout& CVideoCellLayout::operator =(const CVideoCellLayout& other)
{
  if (&other == this)
    return *this;

  m_cellId          = other.m_cellId;
  m_cellStatus      = other.m_cellStatus;
  m_currentPartyId  = other.m_currentPartyId;
  m_forcedPartyId   = other.m_forcedPartyId;
  SetName(other.m_H243_partyName);
  return *this;
}
////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::Serialize(WORD format, std::ostream& m_ostr)
{
  // assuming format = OPERATOR_MCMS
  m_ostr << (WORD)m_cellId << "\n";
  m_ostr << (WORD)m_cellStatus << "\n";
  m_ostr << m_forcedPartyId << "\n";
  m_ostr << m_currentPartyId << "\n";
  m_ostr << m_H243_partyName << "\n";
}

////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::DeSerialize(WORD format, std::istream& m_istr)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  m_istr >> tmp;
  m_cellId = (BYTE)tmp;
  m_istr >> tmp;
  m_cellStatus = (BYTE)tmp;
  m_istr >> m_forcedPartyId;
  m_istr >> m_currentPartyId;
  m_istr.ignore(1);
  m_istr.getline(m_H243_partyName, H243_NAME_LEN, '\n');
}

////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::Serialize(WORD format, CSegment& seg)
{
  // assuming format = OPERATOR_MCMS
  seg << (WORD)m_cellId;
  seg << (WORD)m_cellStatus;
  seg << m_forcedPartyId;
  seg << m_currentPartyId;
  seg << m_H243_partyName;
}

////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::DeSerialize(WORD format, CSegment& seg)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  seg >> tmp;
  m_cellId = (BYTE)tmp;
  seg >> tmp;
  m_cellStatus = (BYTE)tmp;
  seg >> m_forcedPartyId;
  seg >> m_currentPartyId;
  seg >> m_H243_partyName;
}

////////////////////////////////////////////////////////////////////////////
int CVideoCellLayout::DeSerializeXml(CXMLDOMElement* pCellNode, char* pszError, BYTE bIsPrivate)
{
  int nStatus = STATUS_OK;
  int nForceState = 0;

  GET_VALIDATE_MANDATORY_CHILD(pCellNode,"ID",&m_cellId,_0_TO_DWORD);

  GET_VALIDATE_CHILD(pCellNode, "FORCE_NAME", m_H243_partyName, _0_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pCellNode, "FORCE_ID", &m_forcedPartyId, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pCellNode, "SOURCE_ID", &m_currentPartyId, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pCellNode, "FORCE_STATE", &nForceState, FORCE_STATE_ENUM);

  if (nStatus == STATUS_OK)
  {
    switch (nForceState)
    {
      case EMPTY_BY_OPERATOR_THIS_PARTY:
      {
        SetName("[Black Screen]");
        SetBlank(bIsPrivate ? EMPTY_BY_OPERATOR_THIS_PARTY : EMPTY_BY_OPERATOR_ALL_CONF);
        break;
      }
      case AUDIO_ACTIVATED:
      {
        SetName("[Auto Select]");
        SetAudioActivated();
        break;
      }
      case BY_OPERATOR_ALL_CONF:
      {
        m_cellStatus = bIsPrivate ? BY_OPERATOR_THIS_PARTY : BY_OPERATOR_ALL_CONF;
        break;
      }
      case AUTO_SCAN:
      {
        SetName("[Auto Scan]");
        SetAutoScan();
        break;
      }
    }
  }
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::SerializeXml(CXMLDOMElement* pForceNode)
{
  CXMLDOMElement* pTempNode = pForceNode->AddChildNode("CELL");

  pTempNode->AddChildNode("ID", GetCellId());

  if (IsBlank())
    pTempNode->AddChildNode("FORCE_STATE", "blank");
  else if (IsAudioActivated())
    pTempNode->AddChildNode("FORCE_STATE", "auto");
  else if (IsAutoScan())
    pTempNode->AddChildNode("FORCE_STATE", "auto_scan");
  else
  {
    pTempNode->AddChildNode("FORCE_STATE", "forced");
    pTempNode->AddChildNode("FORCE_ID", GetForcedPartyId());
    pTempNode->AddChildNode("FORCE_NAME", GetName());
  }
  pTempNode->AddChildNode("SOURCE_ID", GetCurrentPartyId());
}

////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::SetCurrentPartyId(const DWORD currentPartyId, const BYTE status)
{
  m_currentPartyId = currentPartyId;
  m_cellStatus = status;
}

////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::SetForcedPartyId(const DWORD forcedPartyId, const BYTE status)
{
  m_forcedPartyId = forcedPartyId;
  m_cellStatus = status;
}

////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::SetAudioActivated()
{
  m_cellStatus = AUDIO_ACTIVATED;
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoCellLayout::IsAudioActivated() const
{
  return (m_cellStatus == AUDIO_ACTIVATED) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::SetBlank(const BYTE status)
{
  if (status == EMPTY_BY_OPERATOR_THIS_PARTY || status == EMPTY_BY_OPERATOR_ALL_CONF)
    m_cellStatus = status;
  else
    m_cellStatus = AUTO;
  m_currentPartyId = 0xFFFFFFFF;
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoCellLayout::IsBlank() const
{
  if (m_cellStatus == AUTO || m_cellStatus == EMPTY_BY_OPERATOR_ALL_CONF || m_cellStatus == EMPTY_BY_OPERATOR_THIS_PARTY)
    return TRUE;
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::SetAutoScan()
{
  m_cellStatus = AUTO_SCAN;
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoCellLayout::IsAutoScan() const
{
  return (m_cellStatus == AUTO_SCAN) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CVideoCellLayout::SetName(const char* name)
{
  strncpy(m_H243_partyName, name, H243_NAME_LEN-1);
  m_H243_partyName[H243_NAME_LEN-1] = '\0';
}
