//+========================================================================+
//                       AutoLayoutSet.cpp								   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       AutoLayoutSet.cpp										   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                        	   |
//-------------------------------------------------------------------------|
// Who  | Date			  | Description                                    |
//-------------------------------------------------------------------------|
//	Talya	9/2005			Add auto layout feature to carmel
//+========================================================================+


#include "AutoLayoutSet.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "ObjString.h"
#include "ConfPartyGlobals.h"

#define PREDEFINED_AUTO_LAYOUT_CHOICES 13

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAutoLayoutSet::CAutoLayoutSet()
{
	m_NumOfPredefinedLayouts = PREDEFINED_AUTO_LAYOUT_CHOICES;
	m_pPredefinedLayoutSet = new LayoutType[PREDEFINED_AUTO_LAYOUT_CHOICES];

	m_pPredefinedLayoutSet[0] = CP_LAYOUT_1X1;
	m_pPredefinedLayoutSet[1] = CP_LAYOUT_1X1;
	m_pPredefinedLayoutSet[2] = CP_LAYOUT_1X1;
	m_pPredefinedLayoutSet[3] = CP_LAYOUT_1X2;
	m_pPredefinedLayoutSet[4] = CP_LAYOUT_2X2;
	m_pPredefinedLayoutSet[5] = CP_LAYOUT_2X2;
	m_pPredefinedLayoutSet[6] = CP_LAYOUT_1P5;
	m_pPredefinedLayoutSet[7] = CP_LAYOUT_1P5;
	m_pPredefinedLayoutSet[8] = CP_LAYOUT_1P7;
	m_pPredefinedLayoutSet[9] = CP_LAYOUT_1P7;
	m_pPredefinedLayoutSet[10] = CP_LAYOUT_1P7;
	m_pPredefinedLayoutSet[11] = CP_LAYOUT_1P7;
	m_pPredefinedLayoutSet[12] = CP_LAYOUT_1P12;

	m_pPredefinedLayoutSetForRecLink = new LayoutType[PREDEFINED_AUTO_LAYOUT_CHOICES];

	m_pPredefinedLayoutSetForRecLink[0] = CP_LAYOUT_1X1;
	m_pPredefinedLayoutSetForRecLink[1] = CP_LAYOUT_1X1;
	m_pPredefinedLayoutSetForRecLink[2] = CP_LAYOUT_1X2;
	m_pPredefinedLayoutSetForRecLink[3] = CP_LAYOUT_2X2;
	m_pPredefinedLayoutSetForRecLink[4] = CP_LAYOUT_2X2;
	m_pPredefinedLayoutSetForRecLink[5] = CP_LAYOUT_1P5;
	m_pPredefinedLayoutSetForRecLink[6] = CP_LAYOUT_1P5;
	m_pPredefinedLayoutSetForRecLink[7] = CP_LAYOUT_1P7;
	m_pPredefinedLayoutSetForRecLink[8] = CP_LAYOUT_1P7;
	m_pPredefinedLayoutSetForRecLink[9] = CP_LAYOUT_3X3;
	m_pPredefinedLayoutSetForRecLink[10] = CP_LAYOUT_4X4;
	m_pPredefinedLayoutSetForRecLink[11] = CP_LAYOUT_4X4;
	m_pPredefinedLayoutSetForRecLink[12] = CP_LAYOUT_4X4;


	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

	if(pSysConfig)
	{
		LayoutType layoutType = CP_NO_LAYOUT;
		std::string data;
		CSmallString strKey;
		BYTE isFoundInSystemConfig = FALSE;
		for(int i=0;i<PREDEFINED_AUTO_LAYOUT_CHOICES;i++)
		{
			strKey = "PREDEFINED_AUTO_LAYOUT_";
			strKey << i;

			isFoundInSystemConfig = pSysConfig->GetDataByKey(strKey.GetString(), data);

			if(isFoundInSystemConfig)
			{
				layoutType = TranslateStringToLayoutType(data);
				m_pPredefinedLayoutSet[i] = layoutType;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAutoLayoutSet::~CAutoLayoutSet(void)
{
	if(m_pPredefinedLayoutSet)
		delete [] m_pPredefinedLayoutSet;

	PDELETEA(m_pPredefinedLayoutSetForRecLink);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char*   CAutoLayoutSet::NameOf()  const
{
	return "CAutoLayoutSet";
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAutoLayoutSet::SetLayoutType(const WORD num_of_parties, const LayoutType newLayout)
{
	if(!m_pPredefinedLayoutSet)
		return;
	if(num_of_parties>m_NumOfPredefinedLayouts)
		m_pPredefinedLayoutSet[m_NumOfPredefinedLayouts]=newLayout;
	else
		m_pPredefinedLayoutSet[num_of_parties]=newLayout;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LayoutType CAutoLayoutSet::GetLayoutType(WORD num_of_parties, BYTE isSameLayout) const
{
	if(isSameLayout)
		num_of_parties++; //in same layout - there is an extra dummy party - selfview

	if(num_of_parties>=m_NumOfPredefinedLayouts)
		return m_pPredefinedLayoutSet[m_NumOfPredefinedLayouts-1];
	else
		return m_pPredefinedLayoutSet[num_of_parties];
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LayoutType CAutoLayoutSet::GetLayoutTypeForRecLink(WORD num_of_parties) const
{
	if(num_of_parties>=m_NumOfPredefinedLayouts)
		return m_pPredefinedLayoutSetForRecLink[m_NumOfPredefinedLayouts-1];
	else
		return m_pPredefinedLayoutSetForRecLink[num_of_parties];
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LayoutType CAutoLayoutSet::TranslateStringToLayoutType(std::string layoutStr)
{
	LayoutType layoutType= TranslateSysConfigStringToLayoutType(layoutStr);
	if(CP_NO_LAYOUT==layoutType)
	{
		layoutType = CP_LAYOUT_1X1;
	}
	return layoutType;
}

