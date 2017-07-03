//+========================================================================+
//                       TextOnScreenMsg.CPP                                          |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TextOnScreenMsg.CPP                                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date  December - 2007  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+


#include "TextOnScreenMsg.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTextOnScreenMsg::CTextOnScreenMsg()
{
	m_TextColor = 0x00DC6E82; // yellow  (old color (like in site names)- 0X00EB8080);
	m_BackgroundColor = 0X00309F79;
	m_Transparency = 0;
	m_ShadowWidth = 1;
	m_Alignment = E_TEXT_ALIGNMENT_LEFT;
	m_FontType = TEXT_FONT_TYPE_NULL;
	memset(m_TextLine, 0, MAX_SITE_NAME_ARR_SIZE);
	
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTextOnScreenMsg::CTextOnScreenMsg(const char* TextToDisplay,DWORD TextColor,DWORD BackgroundColor,DWORD Transparency,DWORD ShadowWidth,DWORD Alignment, DWORD FontType )
{
	m_TextColor = TextColor;
	m_BackgroundColor = BackgroundColor;
	m_Transparency = Transparency ;
	m_ShadowWidth = ShadowWidth;
	m_Alignment = Alignment;
    m_FontType = FontType;
    
	memset(m_TextLine, 0, MAX_SITE_NAME_ARR_SIZE);
	strncpy(m_TextLine, TextToDisplay, sizeof(m_TextLine) - 1);
	m_TextLine[sizeof(m_TextLine) - 1] = 0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTextOnScreenMsg::~CTextOnScreenMsg(void)
{
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTextOnScreenMsg& CTextOnScreenMsg::operator= (const CTextOnScreenMsg& rOtherTextLine)
{
	m_TextColor = rOtherTextLine.m_TextColor;
	m_BackgroundColor = rOtherTextLine.m_BackgroundColor;
	m_Transparency = rOtherTextLine.m_Transparency ;
	m_ShadowWidth = rOtherTextLine.m_ShadowWidth;
	m_Alignment = rOtherTextLine.m_Alignment;
    m_FontType = rOtherTextLine.m_FontType;
     
	memset(m_TextLine, 0, MAX_SITE_NAME_ARR_SIZE);
	strncpy(m_TextLine, rOtherTextLine.m_TextLine, MAX_SITE_NAME_ARR_SIZE);
	return *this;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CTextOnScreenMsg::SetTextLine(const char* pTextToDisplay)
{
	strncpy(m_TextLine, pTextToDisplay, sizeof(m_TextLine) - 1);
	m_TextLine[sizeof(m_TextLine) - 1] = 0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CTextOnScreenMsg::SetAlignment(DWORD Alignment)
{
	m_Alignment = Alignment;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CTextOnScreenMsg::SetFontType(DWORD FontType)
{
	m_FontType = m_FontType+FontType;
}


	
