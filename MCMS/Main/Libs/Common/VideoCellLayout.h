// VideoCellLayout.h: interface for the CVideoCellLayout class.
//
////////////////////////////////////////////////////////////////////////////
//Revisions and Updates: 
//
//Date      Updated By    Description
//
//10/7/05		Yoella				Porting to Carmel
////////////////////////////////////////////////////////////////////////////


#if !defined(_VIDEO_CELL_LAYOUT_)
#define _VIDEO_CELL_LAYOUT_

#include "ConfPartyApiDefines.h"
#include "StringsMaps.h"
#include "Segment.h"
#include "InitCommonStrings.h"
#include "SharedDefines.h"

class CXMLDOMElement;

////////////////////////////////////////////////////////////////////////////
//                        CVideoCellLayout
////////////////////////////////////////////////////////////////////////////
class CVideoCellLayout : public CPObject
{
CLASS_TYPE_1(CVideoCellLayout, CPObject)
public:
                      CVideoCellLayout();
                      CVideoCellLayout(const CVideoCellLayout &other);
  virtual            ~CVideoCellLayout();
  virtual const char* NameOf() const { return "CVideoCellLayout"; }
  CVideoCellLayout&   operator =(const CVideoCellLayout& other);

  // Implementation
  void                Serialize(WORD format, std::ostream& m_ostr);
  void                DeSerialize(WORD format, std::istream& m_istr);
  void                Serialize(WORD format, CSegment& seg);
  void                DeSerialize(WORD format, CSegment& seg);
  int                 DeSerializeXml(CXMLDOMElement* pCellNode, char* pszError, BYTE bIsPrivate);
  void                SerializeXml(CXMLDOMElement* pForceNode);

  BYTE                GetCellId() const                 { return m_cellId;         }
  void                SetCellId(const BYTE cellId)      { m_cellId = cellId;       }

  BYTE                GetCellStatus() const             { return m_cellStatus;     }
  void                SetCellStatus(const BYTE status)  { m_cellStatus = status;   }

  DWORD               GetCurrentPartyId() const         { return m_currentPartyId; }
  void                SetCurrentPartyId(const DWORD currentPartyId, const BYTE status = BY_OPERATOR_ALL_CONF);

  DWORD               GetForcedPartyId() const          { return m_forcedPartyId;  }
  void                SetForcedPartyId(const DWORD forcedPartyId, const BYTE status = BY_OPERATOR_ALL_CONF);

  void                SetAudioActivated();
  WORD                IsAudioActivated() const;

  void                SetBlank(const BYTE status = EMPTY_BY_OPERATOR_ALL_CONF);
  WORD                IsBlank() const;

  void                SetAutoScan();
  WORD                IsAutoScan() const;

  void                SetName(const char* name);
  const char*         GetName() const                   { return m_H243_partyName; }

public:
  BYTE                m_cellId;
  BYTE                m_cellStatus;
  DWORD               m_forcedPartyId;
  DWORD               m_currentPartyId;
  char                m_H243_partyName[H243_NAME_LEN];
};

#endif // !defined(_VIDEO_CELL_LAYOUT_)
