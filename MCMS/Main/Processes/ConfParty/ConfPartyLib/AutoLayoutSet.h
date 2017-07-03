//+========================================================================+
//                       AutoLayoutSet.H									   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       AutoLayoutSet.H										       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                        |
//-------------------------------------------------------------------------|
// Who  | Date			  | Description                                   |
//-------------------------------------------------------------------------|
//	Talya	9/2005			Add auto layout feature to carmel
//+========================================================================+

#ifndef _AutoLayoutSet_H_
#define _AutoLayoutSet_H_


#include "PObject.h"
#include "VideoDefines.h"

class CAutoLayoutSet : public CPObject
{
CLASS_TYPE_1(CAutoLayoutSet,CPObject)
public:
	CAutoLayoutSet();
	virtual ~CAutoLayoutSet();

	const char*  NameOf() const;

	void SetLayoutType(const WORD num_of_parties, const LayoutType);
	LayoutType GetLayoutType(WORD num_of_parties, BYTE isSameLayout = NO) const;
	LayoutType GetLayoutTypeForRecLink(WORD num_of_parties) const;

private:
	LayoutType TranslateStringToLayoutType(std::string str);
	LayoutType* m_pPredefinedLayoutSet;
	LayoutType* m_pPredefinedLayoutSetForRecLink;
	WORD m_NumOfPredefinedLayouts;
};

#endif //_AutoLayoutSet_H_

