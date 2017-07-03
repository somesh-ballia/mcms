#ifndef _ROUTINGTBLKEY
  #define _ROUTINGTBLKEY

#include "PObject.h"
#include "Segment.h"
#include "RsrcDesc.h"
#include "HostCommonDefinitions.h"

class CSegment;

////////////////////////////////////////////////////////////////////////////
//                        CRsrcParams
////////////////////////////////////////////////////////////////////////////
class CRsrcParams : public CPObject
{
  CLASS_TYPE_1(CRsrcParams, CPObject)

public:
                             CRsrcParams(ConnectionID connectionId = DUMMY_CONNECTION_ID,
                                         PartyRsrcID partyId = DUMMY_PARTY_ID,
                                         ConfRsrcID confId = DUMMY_CONF_ID,
                                         eLogicalResourceTypes LRT = eLogical_res_none,
                                         WORD roomId = DUMMY_ROOM_ID);
                             CRsrcParams(const CRsrcParams&);
                             virtual ~CRsrcParams();
                             virtual const CRsrcParams& operator=(const CRsrcParams&);
  virtual const char*        NameOf() const { return "CRsrcParams";}

  void                       SetConfRsrcId(ConfRsrcID confId)     { m_ConfRsrcID = confId;    }
  ConfRsrcID                 GetConfRsrcId() const                { return m_ConfRsrcID;      }

  void                       SetPartyRsrcId(PartyRsrcID partyId)  { m_PartyRsrcID = partyId;  }
  PartyRsrcID                GetPartyRsrcId() const               { return m_PartyRsrcID;     }

  void                       SetRoomId(WORD roomId)               { m_RoomId = roomId;        }
  WORD                       GetRoomId() const                    { return m_RoomId;          }

  void                       SetConnectionId(ConnectionID);
  ConnectionID               GetConnectionId() const;

  void                       SetLogicalRsrcType(eLogicalResourceTypes);
  eLogicalResourceTypes      GetLogicalRsrcType() const;

  void                       SetRsrcDesc(CRsrcDesc rsrcDesc);
  CRsrcDesc*                 GetRsrcDesc() const                  { return m_pRsrcDesc;       }

  virtual void               Serialize(WORD format, CSegment& seg);
  virtual void               DeSerialize(WORD format, CSegment& seg);

protected:
  ConfRsrcID                 m_ConfRsrcID;
  PartyRsrcID                m_PartyRsrcID;
  WORD                       m_RoomId;
  CRsrcDesc*                 m_pRsrcDesc; // Created as pointer to enable inheritance by specific resource types in future
};

#endif /* _ROUTINGTBLKEY */

