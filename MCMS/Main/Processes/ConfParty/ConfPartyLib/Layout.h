#ifndef _CLayout_H_
#define _CLayout_H_

#include "PObject.h"
//#include "Layout.h"
#include "VidSubImage.h"
#include "Indication.h"

class CConf;
class CVideoLayout;
class CLayoutHandler;

////////////////////////////////////////////////////////////////////////////
//                        CLayout
////////////////////////////////////////////////////////////////////////////
class CLayout : public CPObject
{
  CLASS_TYPE_1(CLayout, CPObject)

  friend class CLayoutHandler;

public:
                      CLayout(LayoutType m_layoutType, const char* confName);
                      CLayout(const CLayout& oldLayout, const LayoutType layoutType);
                      CLayout(const CLayout&);
  virtual            ~CLayout(void);
  virtual const char* NameOf() const                   { return "CLayout";}

  CVidSubImage*       operator[](const WORD size);
  const CVidSubImage* operator[](const WORD size) const;
  CLayout&            operator=(const CLayout& other);

  void                SetStartsForImages(void);
  WORD                GetNumberOfSubImages(void) const {return m_numberOfSubImages;}
  LayoutType          GetLayoutType(void) const        { return m_layoutType;}
  WORD                FindImagePlaceInLayout(DWORD partyRscId) const;
  WORD                FindFirstUnUsedCell(void) const;

  BYTE                operator==(CVideoLayout& operLayout) const;
  friend BYTE         operator==(const CLayout& first, const CLayout& second);
  friend BYTE         operator!=(const CLayout& first, const CLayout& second);

  virtual void        SetLayout(CVideoLayout& operLayout, Force_Level forceLevel);
  void                SetLayoutFromRes(CVideoLayout& operLayout, Force_Level forceLevel);
  void                SetLayoutType(LayoutType layoutType);
  void                ClearAllForces(void);
  void                ClearCurrentView(void);
  void                ClearAllImageSources(void);
  void                CleanUp(void);
  void                RemovePartyForce(const char* ppartyName, RequestPriority who_asked = AUTO_Prior);
  void                Dump(std::ostream& msg) const;
  void                DumpForces(COstrStream& msg) const;
  CVidSubImage*       GetSubImageOfForcedParty(const char* ForcedParty, WORD& SubImageNumber);

  BYTE                NumberOfCellWherePartyForced(const CTaskApp* pParty) const;
  BYTE                Serialize(Force_Level inf_level, CSegment* pSeg) const;
  BYTE                isActive(void) const                { return m_isActive;}
  void                SetCurrActiveLayout(const BYTE flag){ m_isActive = flag;}
  BYTE                isSetInOtherCell(const char* szPartyName) const;
  BYTE                isLayoutEmpty() const;

  CVidSubImage*       GetSubImageNum(WORD pos);
  virtual void        SetLayout(CVideoLayout& operLayout, Force_Level forceLevel, WORD isPrivate);
  void                SetVswLayout(DWORD partyRscId, const char* pConfName);

  BYTE                IsAutoScanSet() const;
  WORD                GetAutoScanCell(void) const;

  const CLayoutIndications& indications() const { return m_oIndications; }
  DWORD               TranslateSysConfigStringToIconLocation(std::string szLocation);
///  BOOL 				  IsThereAnyConfForcesInLayout();
  CLayoutIndications	m_oIndications; 			// indication icons on layout
  CLayoutIndications& indications() { return m_oIndications; }

private:
  //CLayoutIndications& indications() { return m_oIndications; }
  void                initIndications();

protected:
  LayoutType          m_layoutType;
  CVidSubImage*       m_layout[MAX_SUB_IMAGES_IN_LAYOUT];
  WORD                m_numberOfSubImages;
  char                m_confName[H243_NAME_LEN];  // instead of pointer to conf in order to search for party names in DB
  BYTE                m_isAutoScan;               // YES if there is one cell that marked as auto_scan, NO otherwise
  BYTE                m_isActive;                 // if layout is current layout , it is turned on to active
  //CLayoutIndications  m_oIndications;             // indication icons on layout
};

#endif // _CLayout_H_

