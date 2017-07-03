// VideoLayout.h: interface for the CVideoLayout class.
//
////////////////////////////////////////////////////////////////////////////
//Revisions and Updates:
//
//Date      Updated By    Description
//
//10/7/05		Yoella				Porting to Carmel
////////////////////////////////////////////////////////////////////////////

#if !defined(_VIDEO_LAYOUT_)
#define _VIDEO_LAYOUT_

#include "ConfPartyApiDefines.h"
#include "PObject.h"

class CVideoCellLayout;
class CXMLDOMElement;
class CSegment;

////////////////////////////////////////////////////////////////////////////
//                        CVideoLayout
////////////////////////////////////////////////////////////////////////////
class CVideoLayout : public CPObject
{
CLASS_TYPE_1(CVideoLayout, CPObject)
public:
  //Constructors
                      CVideoLayout();
                      CVideoLayout(const CVideoLayout &other);
                      CVideoLayout(BYTE screenLayout, BYTE isActive);
  virtual            ~CVideoLayout();
  virtual const char* NameOf() const { return "CVideoLayout"; }
  CVideoLayout&       operator=(CVideoLayout& other);

  // Implementation
  void                Serialize(WORD format, std::ostream& m_ostr);
  void                DeSerialize(WORD format, std::istream& m_istr);
  void                Serialize(WORD format, CSegment& seg);
  void                DeSerialize(WORD format, CSegment& seg);
  int                 DeSerializeXml(CXMLDOMElement* pForceNode, char* pszError, BYTE bIsPrivate);
  void                SerializeXml(CXMLDOMElement* pForceNode);

  BYTE                GetScreenLayout() const;
  BYTE 			GetScreenLayoutITP();
  void                SetScreenLayout(const BYTE screenLayout);

  int                 AddCell(const CVideoCellLayout &other);
  int                 FindCell(const CVideoCellLayout &other);
  int                 FindCell(const BYTE cellID);
  int                 FindCell(const char* partyName);
  int                 UpdateCell(const CVideoCellLayout &other);
  int                 CancelCell(const BYTE cellID);
  CVideoCellLayout*   GetCurrentCell(const BYTE cellID);
  WORD                GetNumCells(const DWORD partyID);
  void                DeletePartyFromCells(const char* name);

  void                SetActive(const BYTE state);
  BYTE                IsActive() const;
  WORD                GetMaxNumberOfCells(DWORD apiNum);
  static int          Layout2SrcNum(int nLayout);
  void                SetCellStatusToAllArr(BYTE newStatus, bool isOverrideProfileLayout = false);
  bool                IsGatheringLayout();
  void                ToString(std::string& sLayout);
  static std::string  LayoutTypeToString(int layoutType);
  static CVideoLayout*CreateGatheringLayout();
  static CVideoLayout*CreateNonGatheringLayout();
  void  SetLayoutForRecording(enSrsVideoLayoutType   layout);

public:
  BYTE                m_screenLayout;
  WORD                m_numb_of_cell;
  CVideoCellLayout*   m_pCellLayout[MAX_CELL_NUMBER];
  BYTE                m_active; // YES/NO
};

#endif // !defined(_VIDEO_LAYOUT_)
