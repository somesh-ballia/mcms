#ifndef _CVisualEffectsParams_H_
  #define _CVisualEffectsParams_H_
//+========================================================================+
//                       VisualEffectsParams.H                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:   VisualEffectsParams.H                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                   |
//-------------------------------------------------------------------------|
//+========================================================================+


//#include "VideoStructs.h"
#include "AllocateStructs.h"
#include "PObject.h"
#include "../../../McmIncld/MPL/Card/PhysicalPortVideo/VideoStructs.h"

class CSegment;
class COstrStream;
class CIstrStream;
class CXMLDOMElement;
//extern class BORDERS_PARAM_S;


enum eLayoutBorderWidth
{
  eLayoutBorderNone = 0,
  eLayoutBorderThin,
  eLayoutBorderNormal,
  eLayoutBorderThick,
  eLayoutBorderTypeUnknown // MUST be last
};


#define NO_COLOR    0 // Color is invalid
#define BLACK_COLOR 0x00108080

class CVisualEffectsParams : public CPObject
{
  CLASS_TYPE_1(CVisualEffectsParams, CPObject)

public:
                      CVisualEffectsParams();
                      CVisualEffectsParams(const CVisualEffectsParams &other);
                      CVisualEffectsParams(CVisualEffectsParams* other);
                     ~CVisualEffectsParams();
  virtual const char* NameOf() const { return "CVisualEffectsParams"; }

  CVisualEffectsParams& operator =(const CVisualEffectsParams& other);
  friend BYTE operator==(const CVisualEffectsParams& first, const CVisualEffectsParams& second);

  BYTE                VisualEffectsDifferOnlyInSpeakerIndicationParams(CVisualEffectsParams* pVisualEffects);

  void                Serialize(WORD format, CSegment& seg);
  void                DeSerialize(WORD format, CSegment& seg);
  void                Serialize(WORD format, std::ostream &ostr);
  void                DeSerialize(WORD format, std::istream &istr);
  void                SerializeXml(CXMLDOMElement *pFatherNode);
  int                 DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError);

  static void         TranslateRGBColorToYUV(DWORD dwColorRGB, DWORD& dwColorYUV);

  DWORD               GetBackgroundColorRGB() const                               { return m_backgroundColorRGB; }
  DWORD               GetBackgroundColorYUV() const                               { return m_backgroundColorYUV; }
  BYTE                IslayoutBorderEnable() const                                { return m_islayoutBorderEnable; }
  DWORD               GetlayoutBorderColorRGB() const                             { return m_layoutBorderColorRGB; }
  DWORD               GetlayoutBorderColorYUV() const                             { return m_layoutBorderColorYUV; }
  eLayoutBorderWidth  GetlayoutBorderWidth() const                                { return m_layoutBorderWidth; }
  BYTE                IsSpeakerNotationEnable() const                             { return m_isSpeakerNotationEnable; }
  DWORD               GetSpeakerNotationColorRGB() const                          { return m_speakerNotationColorRGB; }
  DWORD               GetSpeakerNotationColorYUV() const                          { return m_speakerNotationColorYUV; }
  DWORD               GetBackgroundImageID() const                                { return m_backgroundImageID; }
  BYTE                IsSiteNamesEnable() const                                   { return m_isSiteNamesEnable; }
  BOOL                UseYUVcolor()                                               { return m_useYUVcolor; }
  BORDERS_PARAM_S*    GetSpecifiedBorders() const                                 { return m_pSpecificBorders; }

  void                SetBackgroundColorRGB(DWORD backgroundColorRGB)             { m_backgroundColorRGB      = backgroundColorRGB;       }
  void                SetBackgroundColorYUV(DWORD backgroundColorYUV)             { m_backgroundColorYUV      = backgroundColorYUV;       }
  void                SetlayoutBorderEnable(BYTE islayoutBorderEnable)            { m_islayoutBorderEnable    = islayoutBorderEnable;     }
  void                SetlayoutBorderColorRGB(DWORD layoutBorderColorRGB)         { m_layoutBorderColorRGB    = layoutBorderColorRGB;     }
  void                SetlayoutBorderColorYUV(DWORD layoutBorderColorYUV)         { m_layoutBorderColorYUV    = layoutBorderColorYUV;     }
  void                SetlayoutBorderWidth(eLayoutBorderWidth layoutBorderWidth)  { m_layoutBorderWidth       = layoutBorderWidth;        }
  void                SetSpeakerNotationEnable(BYTE isSpeakerNotationEnable)      { m_isSpeakerNotationEnable = isSpeakerNotationEnable;  }
  void                SetSpeakerNotationColorRGB(DWORD speakerNotationColorRGB)   { m_speakerNotationColorRGB = speakerNotationColorRGB;  }
  void                SetSpeakerNotationColorYUV(DWORD speakerNotationColorYUV)   { m_speakerNotationColorYUV = speakerNotationColorYUV;  }
  void                SetBackgroundImageID(DWORD backgroundImageID)               { m_backgroundImageID       = backgroundImageID;        }
  void                SetUseYUVcolor(BOOL useYUVYesNo)                            { m_useYUVcolor             = useYUVYesNo;              }
  void                SetSiteNamesEnable(BYTE isSiteNamesEnable)                  { m_isSiteNamesEnable       = isSiteNamesEnable;        }
  void			      SetSpecifiedBorders(BORDERS_PARAM_S* srcBorders);
  DWORD               CalcYUVinDWORD(BYTE Y, BYTE U, BYTE V);

protected:
  int                 DeSerializeColorXml(CXMLDOMElement *pColorNode, DWORD& dwColorYUV, DWORD& dwColorRGB, char *pszError);
  void                SerializeColorXml(CXMLDOMElement *pColorNode, DWORD dwColorRGB, DWORD dwColorYUV);

  DWORD               m_backgroundColorRGB;
  DWORD               m_backgroundColorYUV;
  BYTE                m_islayoutBorderEnable;
  DWORD               m_layoutBorderColorRGB;
  DWORD               m_layoutBorderColorYUV;
  eLayoutBorderWidth  m_layoutBorderWidth;
  BYTE                m_isSpeakerNotationEnable;
  DWORD               m_speakerNotationColorRGB;
  DWORD               m_speakerNotationColorYUV;
  DWORD               m_backgroundImageID;
  BOOL                m_useYUVcolor; // 0-Use RGB , 1-Use YUV
  BYTE                m_isSiteNamesEnable;
  BORDERS_PARAM_S*    m_pSpecificBorders;
};

#endif //CVisualEffectsParams
