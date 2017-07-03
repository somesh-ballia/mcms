//+========================================================================+
//                         VisualEffectsParams.CPP                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       VisualEffectsParams.H                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Talya                                                       |
//-------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                   |
//-------------------------------------------------------------------------|
//+========================================================================+

#include <ostream>
#include <istream>
#include "VisualEffectsParams.h"
#include "Segment.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "Trace.h"
#include "TraceStream.h"

///////////////////////////////////////////////////////////////////////////////////////
CVisualEffectsParams::CVisualEffectsParams()
{
	m_backgroundColorRGB      = NO_COLOR;
  m_backgroundColorYUV      = NO_COLOR;//Has to be -1-1-1
  m_islayoutBorderEnable    = YES;
  m_layoutBorderColorRGB    = NO_COLOR;
  m_layoutBorderColorYUV    = NO_COLOR;
  m_layoutBorderWidth       = eLayoutBorderNormal;
  m_isSpeakerNotationEnable = YES;
  m_speakerNotationColorRGB = NO_COLOR;
  m_speakerNotationColorYUV = NO_COLOR;
  m_backgroundImageID       = 0;
  m_useYUVcolor             = 0; //0-Use RGB , 1-Use YUV  Default is 0 to support old API
  m_isSiteNamesEnable       = YES;
  m_pSpecificBorders        = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////
CVisualEffectsParams::CVisualEffectsParams(const CVisualEffectsParams &other) : CPObject(other)
{
	m_pSpecificBorders        = NULL;
	*this = other;
}
///////////////////////////////////////////////////////////////////////////////////////
CVisualEffectsParams& CVisualEffectsParams::operator= (const CVisualEffectsParams &other)
{
  if (&other == this)
    return *this;
  m_backgroundColorRGB      = other.m_backgroundColorRGB;
  m_backgroundColorYUV      = other.m_backgroundColorYUV;
  m_islayoutBorderEnable    = other.m_islayoutBorderEnable;
  m_layoutBorderColorRGB    = other.m_layoutBorderColorRGB;
  m_layoutBorderColorYUV    = other.m_layoutBorderColorYUV;
  m_layoutBorderWidth       = other.m_layoutBorderWidth;
  m_isSpeakerNotationEnable = other.m_isSpeakerNotationEnable;
  m_speakerNotationColorRGB = other.m_speakerNotationColorRGB;
  m_speakerNotationColorYUV = other.m_speakerNotationColorYUV;
  m_backgroundImageID       = other.m_backgroundImageID;
  m_useYUVcolor             = other.m_useYUVcolor;
  m_isSiteNamesEnable       = other.m_isSiteNamesEnable;
  SetSpecifiedBorders(other.m_pSpecificBorders);
  return *this;
}
///////////////////////////////////////////////////////////////////////////////////////
CVisualEffectsParams::~CVisualEffectsParams()
{
	if (m_pSpecificBorders != NULL)
			delete(m_pSpecificBorders);
}
///////////////////////////////////////////////////////////////////////////////////////
BYTE operator==(const CVisualEffectsParams& first,const CVisualEffectsParams& second)
{
  if (first.m_backgroundColorRGB != second.m_backgroundColorRGB)
    return FALSE;
  if (first.m_backgroundColorYUV != second.m_backgroundColorYUV)
    return FALSE;
  if (first.m_islayoutBorderEnable != second.m_islayoutBorderEnable)
    return FALSE;
  if (first.m_islayoutBorderEnable)
  {
    //border color and width only relevant if enabled
    if (first.m_layoutBorderColorRGB != second.m_layoutBorderColorRGB)
      return FALSE;
    if (first.m_layoutBorderColorYUV != second.m_layoutBorderColorYUV)
      return FALSE;
    if (first.m_layoutBorderWidth != second.m_layoutBorderWidth)
      return FALSE;
  }
  if (first.m_isSpeakerNotationEnable != second.m_isSpeakerNotationEnable)
    return FALSE;
  if (first.m_isSpeakerNotationEnable)
  {
    //speaker notation color only relevant if enabled
    if (first.m_speakerNotationColorRGB != second.m_speakerNotationColorRGB)
      return FALSE;
    if (first.m_speakerNotationColorYUV != second.m_speakerNotationColorYUV)
      return FALSE;
  }
  if (first.m_backgroundImageID != second.m_backgroundImageID)
    return FALSE;
  if (first.m_useYUVcolor != second.m_useYUVcolor)
    return FALSE;
  if (first.m_isSiteNamesEnable != second.m_isSiteNamesEnable)
    return FALSE;

  return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////
BYTE CVisualEffectsParams::VisualEffectsDifferOnlyInSpeakerIndicationParams(CVisualEffectsParams* pOther)
{
  if (m_backgroundColorRGB != pOther->m_backgroundColorRGB)
    return FALSE;
  if (m_backgroundColorYUV != pOther->m_backgroundColorYUV)
    return FALSE;
  if (m_islayoutBorderEnable != pOther->m_islayoutBorderEnable)
    return FALSE;
  if (m_islayoutBorderEnable)
  { //border color and width only relevant if enabled
    if (m_layoutBorderColorRGB != pOther->m_layoutBorderColorRGB)
      return FALSE;
    if (m_layoutBorderColorYUV != pOther->m_layoutBorderColorYUV)
      return FALSE;
    if (m_layoutBorderWidth != pOther->m_layoutBorderWidth)
      return FALSE;
  }
  if (m_backgroundImageID != pOther->m_backgroundImageID)
    return FALSE;
  if (m_useYUVcolor != pOther->m_useYUVcolor)
    return FALSE;

  return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////
void CVisualEffectsParams::DeSerialize(WORD format, CSegment& seg)
{
  WORD tmp;

  seg >> m_backgroundColorRGB >> m_backgroundColorYUV >> tmp;
  m_islayoutBorderEnable = (BYTE)tmp;

  seg >> m_layoutBorderColorRGB >> m_layoutBorderColorYUV;

  seg >> tmp;
  m_layoutBorderWidth = (eLayoutBorderWidth)tmp;

  seg >> tmp;
  m_isSpeakerNotationEnable = (BYTE)tmp;

  seg >> m_speakerNotationColorRGB >> m_speakerNotationColorYUV;

  seg >> m_backgroundImageID;

  seg >> tmp;
  m_useYUVcolor = (BYTE)tmp;

  seg >> tmp;
  m_isSiteNamesEnable = (BYTE)tmp;
  DWORD tmp1;
  seg >> tmp1;
  SetSpecifiedBorders((BORDERS_PARAM_S*)tmp1);
}
///////////////////////////////////////////////////////////////////////////////////////
void CVisualEffectsParams::Serialize(WORD format, CSegment& seg)
{
  seg << m_backgroundColorRGB
      << m_backgroundColorYUV
      << (WORD)m_islayoutBorderEnable
      << m_layoutBorderColorRGB
      << m_layoutBorderColorYUV;

  seg << (WORD)m_layoutBorderWidth
      << (WORD)m_isSpeakerNotationEnable
      << m_speakerNotationColorRGB
      << m_speakerNotationColorYUV
      << m_backgroundImageID
      << (WORD)m_useYUVcolor
      << (WORD)m_isSiteNamesEnable
  	  << (DWORD)m_pSpecificBorders;
}
///////////////////////////////////////////////////////////////////////////////////////
CVisualEffectsParams::CVisualEffectsParams(CVisualEffectsParams* other)
{
  m_backgroundColorRGB      = other->m_backgroundColorRGB;
  m_backgroundColorYUV      = other->m_backgroundColorYUV;
  m_islayoutBorderEnable    = other->m_islayoutBorderEnable;
  m_layoutBorderColorRGB    = other->m_layoutBorderColorRGB;
  m_layoutBorderColorYUV    = other->m_layoutBorderColorYUV;
  m_layoutBorderWidth       = other->m_layoutBorderWidth;
  m_isSpeakerNotationEnable = other->m_isSpeakerNotationEnable;
  m_speakerNotationColorRGB = other->m_speakerNotationColorRGB;
  m_speakerNotationColorYUV = other->m_speakerNotationColorYUV;
  m_backgroundImageID       = other->m_backgroundImageID;
  m_useYUVcolor             = other->m_useYUVcolor;
  m_isSiteNamesEnable       = other->m_isSiteNamesEnable;
  m_pSpecificBorders        = NULL;
  SetSpecifiedBorders(other->m_pSpecificBorders);
}
///////////////////////////////////////////////////////////////////////////////////////
void CVisualEffectsParams::Serialize(WORD format, std::ostream &ostr/*COstrStream &m_ostr*/)
{
  // assuming format = OPERATOR_MCMS
  ostr << m_backgroundColorRGB << "\n";
  ostr << m_backgroundColorYUV << "\n";
  ostr << (WORD)m_islayoutBorderEnable << "\n";
  ostr << m_layoutBorderColorRGB << "\n";
  ostr << m_layoutBorderColorYUV << "\n";
  ostr << (WORD)m_layoutBorderWidth << "\n";
  ostr << (WORD)m_isSpeakerNotationEnable << "\n";
  ostr << m_speakerNotationColorRGB << "\n";
  ostr << m_speakerNotationColorYUV << "\n";
  ostr << m_backgroundImageID << "\n";
  ostr << (WORD)m_useYUVcolor << "\n";
  ostr << (WORD)m_isSiteNamesEnable << "\n";
  ostr << (DWORD)m_pSpecificBorders << "\n";
}
///////////////////////////////////////////////////////////////////////////////////////
void CVisualEffectsParams::DeSerialize(WORD format, std::istream &istr/*CIstrStream &m_istr*/)
{
  WORD tmp;

  istr >> m_backgroundColorRGB;
  istr >> m_backgroundColorYUV;

  istr >> tmp;
  m_islayoutBorderEnable = (BYTE)tmp;

  istr >> m_layoutBorderColorRGB;
  istr >> m_layoutBorderColorYUV;

  istr >> tmp;
  m_layoutBorderWidth = (eLayoutBorderWidth)tmp;

  istr >> tmp;
  m_isSpeakerNotationEnable = (BYTE)tmp;

  istr >> m_speakerNotationColorRGB;
  istr >> m_speakerNotationColorYUV;

  istr >> m_backgroundImageID;

  istr >> tmp;
  m_useYUVcolor = (BYTE)tmp;

  istr >> tmp;
  m_isSiteNamesEnable = (BYTE)tmp;

  DWORD tmp1 = 0;
  istr >> tmp1;
  SetSpecifiedBorders((BORDERS_PARAM_S*)tmp1);
}
///////////////////////////////////////////////////////////////////////////////////////
int CVisualEffectsParams::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError)
{
  CXMLDOMElement *pColorNode;
  int nStatus;

  GET_VALIDATE_CHILD(pActionNode, "USE_YUV", &m_useYUVcolor, _BOOL);
  GET_CHILD_NODE(pActionNode, "BACKGROUND_COLOR", pColorNode);

  if (pColorNode)
  {
    nStatus = DeSerializeColorXml(pColorNode, m_backgroundColorRGB, m_backgroundColorYUV, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }

  GET_VALIDATE_CHILD(pActionNode, "LAYOUT_BORDER", &m_islayoutBorderEnable, _BOOL);

  GET_CHILD_NODE(pActionNode, "LAYOUT_BORDER_COLOR", pColorNode);

  if (pColorNode)
  {
    nStatus = DeSerializeColorXml(pColorNode, m_layoutBorderColorRGB, m_layoutBorderColorYUV, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }

  GET_VALIDATE_CHILD(pActionNode, "SPEAKER_NOTATION", &m_isSpeakerNotationEnable, _BOOL);

  GET_CHILD_NODE(pActionNode, "SPEAKER_NOTATION_COLOR", pColorNode);

  if (pColorNode)
  {
    nStatus = DeSerializeColorXml(pColorNode, m_speakerNotationColorRGB, m_speakerNotationColorYUV, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }

  GET_VALIDATE_CHILD(pActionNode, "IMAGE_ID", & m_backgroundImageID, _0_TO_DWORD);

  return STATUS_OK;
}
///////////////////////////////////////////////////////////////////////////////////////
void CVisualEffectsParams::SerializeXml(CXMLDOMElement *pFatherNode)
{
  CXMLDOMElement *pVisualEffectsNode, *pColorNode;

  pVisualEffectsNode = pFatherNode->AddChildNode("VISUAL_EFFECTS");
  pColorNode = pVisualEffectsNode->AddChildNode("BACKGROUND_COLOR");
  SerializeColorXml(pColorNode, m_backgroundColorRGB, m_backgroundColorYUV);
  pVisualEffectsNode->AddChildNode("LAYOUT_BORDER", m_islayoutBorderEnable, _BOOL);
  pColorNode = pVisualEffectsNode->AddChildNode("LAYOUT_BORDER_COLOR");
  SerializeColorXml(pColorNode, m_layoutBorderColorRGB, m_layoutBorderColorYUV);
  pVisualEffectsNode->AddChildNode("SPEAKER_NOTATION", m_isSpeakerNotationEnable, _BOOL);
  pColorNode = pVisualEffectsNode->AddChildNode("SPEAKER_NOTATION_COLOR");
  SerializeColorXml(pColorNode, m_speakerNotationColorRGB, m_speakerNotationColorYUV);
  pVisualEffectsNode->AddChildNode("IMAGE_ID", m_backgroundImageID, _0_TO_DWORD);
  pVisualEffectsNode->AddChildNode("USE_YUV", m_useYUVcolor, _BOOL);
}
///////////////////////////////////////////////////////////////////////////////////////
int CVisualEffectsParams::DeSerializeColorXml(CXMLDOMElement *pColorNode,DWORD& dwColorRGB,DWORD& dwColorYUV,char *pszError)
{
  BYTE defaultColor = 0;
  int nStatus;

  BYTE nRed = defaultColor, nGreen = defaultColor, nBlue = defaultColor;
  BYTE nY = defaultColor, nU = defaultColor, nV = defaultColor; //255 is the max value ,default value -1

  DWORD dwTempColorRGB = 0x000000FF; // Valid color - if its first byte is FF (FFxxxxxx)
  DWORD dwTempColorYUV = 0x000000FF; // Valid color - if its first byte is FF (FFxxxxxx)

  GET_VALIDATE_CHILD(pColorNode,"RED",&nRed,_0_TO_BYTE); //was GET_VALIDATE_MANDATORY_CHILD
  GET_VALIDATE_CHILD(pColorNode,"GREEN",&nGreen,_0_TO_BYTE);
  GET_VALIDATE_CHILD(pColorNode,"BLUE",&nBlue,_0_TO_BYTE);

  dwTempColorRGB = ((dwTempColorRGB << 8) | nRed);
  dwTempColorRGB = ((dwTempColorRGB << 8) | nGreen);
  dwTempColorRGB = ((dwTempColorRGB << 8) | nBlue);

  GET_VALIDATE_CHILD(pColorNode,"Y",&nY,_0_TO_BYTE);
  GET_VALIDATE_CHILD(pColorNode,"U",&nU,_0_TO_BYTE);
  GET_VALIDATE_CHILD(pColorNode,"V",&nV,_0_TO_BYTE);

  if (UseYUVcolor())//
  {
    GET_VALIDATE_CHILD(pColorNode,"Y",&nY,_0_TO_BYTE); //***check for 0-255 is done here
    GET_VALIDATE_CHILD(pColorNode,"U",&nU,_0_TO_BYTE);
    GET_VALIDATE_CHILD(pColorNode,"V",&nV,_0_TO_BYTE);

    dwTempColorYUV = ((dwTempColorYUV << 8) | nY);
    dwTempColorYUV = ((dwTempColorYUV << 8) | nU);
    dwTempColorYUV = ((dwTempColorYUV << 8) | nV);
  }
  else
    TranslateRGBColorToYUV(dwTempColorRGB, dwTempColorYUV);

  dwColorRGB = dwTempColorRGB;
  dwColorYUV = dwTempColorYUV;

  return STATUS_OK;
}
///////////////////////////////////////////////////////////////////////////////////////
void CVisualEffectsParams::TranslateRGBColorToYUV(DWORD dwColorRGB,DWORD& dwColorYUV)
{
  BYTE defaultColor = 0;

  BYTE nRed   = (dwColorRGB & 0x00FF0000) >> 16;
  BYTE nGreen = (dwColorRGB & 0x0000FF00) >> 8;
  BYTE nBlue  = dwColorRGB & 0x000000FF;

  BYTE nY = ((257 * nRed + 504 * nGreen + 98 * nBlue) / 1000) + 16;
  BYTE nU = ((-148 * nRed - 291 * nGreen + 439 * nBlue) / 1000) + 128;//Cb
  BYTE nV = ((439 * nRed - 368 * nGreen - 71 * nBlue) / 1000) + 128;//Cr

  dwColorYUV = 0x000000FF; // Valid color - if its first byte is FF (FFxxxxxx)

  dwColorYUV = ((dwColorYUV << 8) | nY);
  dwColorYUV = ((dwColorYUV << 8) | nU);
  dwColorYUV = ((dwColorYUV << 8) | nV);
}
///////////////////////////////////////////////////////////////////////////////////////
void CVisualEffectsParams::SerializeColorXml(CXMLDOMElement *pColorNode, DWORD dwColorRGB,DWORD dwColorYUV)
{
  BYTE Red = 0, Green = 0, Blue = 0;

  if (((BYTE)(dwColorRGB >> 24)) == 0xFF) // Valid color - if its first byte is FF (FFxxxxxx)
  {
    Red   = (BYTE)(dwColorRGB >> 16);
    Green = (BYTE)(dwColorRGB >> 8);
    Blue  = (BYTE)dwColorRGB;
  }

  BYTE Y = 0, U = 0, V = 0;

  if (((BYTE)(dwColorYUV >> 24)) == 0xFF) // Valid color - if its first byte is FF (FFxxxxxx)
  {
    Y = (BYTE)(dwColorYUV >> 16);
    U = (BYTE)(dwColorYUV >> 8);
    V = (BYTE)dwColorYUV;
  }

  pColorNode->AddChildNode("RED", Red);
  pColorNode->AddChildNode("GREEN", Green);
  pColorNode->AddChildNode("BLUE", Blue);

  pColorNode->AddChildNode("Y", Y);
  pColorNode->AddChildNode("U", U);
  pColorNode->AddChildNode("V", V);
}
///////////////////////////////////////////////////////////////////////////////////////
DWORD CVisualEffectsParams::CalcYUVinDWORD(BYTE Y,BYTE U,BYTE V)
{
  DWORD dwTempColorYUV = 0x000000FF; // Valid color - if its first byte is FF (FFxxxxxx)
  dwTempColorYUV = ((dwTempColorYUV << 8) | Y);
  dwTempColorYUV = ((dwTempColorYUV << 8) | U);
  dwTempColorYUV = ((dwTempColorYUV << 8) | V);
  
  return dwTempColorYUV;
}

void CVisualEffectsParams::SetSpecifiedBorders(BORDERS_PARAM_S* srcBorders)
{
	if (srcBorders == NULL)
	{
		if (m_pSpecificBorders != NULL)
		{
			delete(m_pSpecificBorders);
			m_pSpecificBorders = NULL;
		}
		return;
	}
	else if (m_pSpecificBorders == NULL)
		m_pSpecificBorders = new BORDERS_PARAM_S;
	*m_pSpecificBorders = *srcBorders;
}
