#include "RsrcParams.h"
#include "Trace.h"
#include "ConfPartyDefines.h"
#include "ObjString.h"


////////////////////////////////////////////////////////////////////////////
//                        CRsrcParams
////////////////////////////////////////////////////////////////////////////
CRsrcParams::CRsrcParams(ConnectionID connectionId, PartyRsrcID partyId, ConfRsrcID confId, eLogicalResourceTypes LRT, WORD roomId)
{
  m_PartyRsrcID = partyId;
  m_ConfRsrcID  = confId;
  m_RoomId      = roomId;
  m_pRsrcDesc   = new CRsrcDesc(connectionId, LRT);
}

////////////////////////////////////////////////////////////////////////////
CRsrcParams::CRsrcParams(const CRsrcParams& other) : CPObject(other)
{
  m_pRsrcDesc = NULL;

  *this = other;
}

////////////////////////////////////////////////////////////////////////////
CRsrcParams::~CRsrcParams()
{
  POBJDELETE(m_pRsrcDesc);
}

////////////////////////////////////////////////////////////////////////////
const CRsrcParams& CRsrcParams::operator=(const CRsrcParams& other)
{
  if (&other == this)
    return *this;

  m_ConfRsrcID  = other.m_ConfRsrcID;
  m_PartyRsrcID = other.m_PartyRsrcID;
  m_RoomId      = other.m_RoomId;

  POBJDELETE(m_pRsrcDesc);
  if (other.GetRsrcDesc())
    m_pRsrcDesc = new CRsrcDesc(*(other.GetRsrcDesc()));

  return *this;
}

////////////////////////////////////////////////////////////////////////////
DWORD CRsrcParams::GetConnectionId() const
{
  PASSERT_AND_RETURN_VALUE(!m_pRsrcDesc, 0);

  return m_pRsrcDesc->GetConnectionId();
}

////////////////////////////////////////////////////////////////////////////
eLogicalResourceTypes CRsrcParams::GetLogicalRsrcType() const
{
  PASSERT_AND_RETURN_VALUE(!m_pRsrcDesc, eLogical_res_none);

  return m_pRsrcDesc->GetLogicalRsrcType();
}

////////////////////////////////////////////////////////////////////////////
void CRsrcParams::SetConnectionId(ConnectionID conID)
{
  PASSERT_AND_RETURN(!m_pRsrcDesc);

  m_pRsrcDesc->SetConnectionId(conID);
}

////////////////////////////////////////////////////////////////////////////
void CRsrcParams::SetLogicalRsrcType(eLogicalResourceTypes LRT)
{
  PASSERT_AND_RETURN(!m_pRsrcDesc);

  m_pRsrcDesc->SetLogicalRsrcType(LRT);
}

////////////////////////////////////////////////////////////////////////////
void CRsrcParams::SetRsrcDesc(CRsrcDesc rsrcDesc)
{
  PASSERT_AND_RETURN(!m_pRsrcDesc);

  *m_pRsrcDesc = rsrcDesc;
}

////////////////////////////////////////////////////////////////////////////
void CRsrcParams::Serialize(WORD format, CSegment& seg)
{
  if (NATIVE == format)
  {
    seg << m_ConfRsrcID << m_PartyRsrcID << m_RoomId;
    m_pRsrcDesc->Serialize(format, seg);
  }
}

////////////////////////////////////////////////////////////////////////////
void CRsrcParams::DeSerialize(WORD format, CSegment& seg)
{
  if (NATIVE == format)
  {
    seg >> m_ConfRsrcID >> m_PartyRsrcID >> m_RoomId;
    m_pRsrcDesc->DeSerialize(format, seg);
  }
}
