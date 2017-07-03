//+========================================================================+
//                       TextOnScreenMsg.H                                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TextOnScreenMsg.H                                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date  December - 2007  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef TEXTONSCREENMSG_H_
#define TEXTONSCREENMSG_H_

#include "PObject.h"
#include "Segment.h"

#include "VideoDefines.h"
#include "ConfPartyDefines.h"

class COstrStream;
class CTaskApp;

class CTextOnScreenMsg : public CPObject 
{
CLASS_TYPE_1(CTextOnScreenMsg, CPObject)
public:
	CTextOnScreenMsg();
	CTextOnScreenMsg(const char* TextToDisplay,DWORD TextColor,DWORD BackgroundColor,DWORD Transparency,DWORD ShadowWidth,DWORD m_Alignment, DWORD FontType );
    virtual ~CTextOnScreenMsg(void);
	
	CTextOnScreenMsg&	operator= (const CTextOnScreenMsg& rOther);
		
	virtual const char*  NameOf() const{ return "CTextOnScreenMsg";}	
	const char* GetTextLine()const{ return m_TextLine; }
	void	SetTextLine(const char* pTextLine);
    void	SetFontType(DWORD FontType);
    void    SetAlignment(DWORD Alignment);
	DWORD GetTextColor()const {return m_TextColor;};
	DWORD GetBackgroundColor()const {return m_BackgroundColor;};
	DWORD GetTransparency()const {return m_Transparency;};
	DWORD GetShadowWidth()const {return m_ShadowWidth;};
	DWORD GetAlignment()const {return m_Alignment;};
	DWORD GetFontType()const {return m_FontType;};
	
private:
	char   m_TextLine[MAX_SITE_NAME_ARR_SIZE];
	DWORD  m_TextColor;
	DWORD  m_BackgroundColor;
	DWORD  m_Transparency;
	DWORD  m_ShadowWidth;
	DWORD  m_Alignment;
	DWORD  m_FontType;
	
	
	
};

#endif /*TEXTONSCREENMSG_H_*/

