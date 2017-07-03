/*
 * CGetDynamicContentRateResTable.cpp
 *
 *  Created on: Nov 29, 2011
 *      Author: mhalfon
 */

#include "CGetDynamicContentRateResTable.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "UnifiedComMode.h"

/*
 * GetContentRateTable.cpp
 *
 *  Created on: Nov 29, 2011
 *      Author: mhalfon
 */

// CGetDynamicContentRateResTable.cpp: implementation of the CGetDynamicContentRateResTable class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Get XML IVR List
//========   ==============   =====================================================================

#include "NStream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGetContentRateResTable::CGetContentRateResTable()
	: m_isHighProfile(false)
{
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
int CGetContentRateResTable::DeSerializeXml(CXMLDOMElement* pResNode,char* pszError,const char* strAction)
{
	DeSerializeXml(pResNode,pszError);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CGetContentRateResTable::DeSerializeXml(CXMLDOMElement* pActionsNode, char* pszError)
{
	int nStatus = STATUS_OK;

	char* tagName;
	pActionsNode->get_tagName(&tagName);
	if(!strcmp(tagName, "GET_DYNAMIC_HP_CONTENT_RATE_TABLE") || !strcmp(tagName, "GET_CUSTOMIZED_HP_CONTENT_RATE_TABLE"))
		m_isHighProfile = true;

	GET_VALIDATE_CHILD(pActionsNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);

	return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CGetDynamicContentRateResTable::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	PTRACE(eLevelError,"contentpp CGetDynamicContentRateResTable::SerializeXml");

	RateResolutionMinValuesMap amcH264DynamicContentRateResTableMinValues;

	getAmcH264DynamicContentMinRate(amcH264DynamicContentRateResTableMinValues);

	CXMLDOMElement* pContentRateTableNode;
	pContentRateTableNode = pActionsNode->AddChildNode("DYNAMIC_CONTENT_RATE_TABLE");

	RateResolutionMinValuesMap::iterator map1IndexItr = amcH264DynamicContentRateResTableMinValues.begin();

	for( ; map1IndexItr != amcH264DynamicContentRateResTableMinValues.end(); ++map1IndexItr)
	{
		Resolution2RateMap::iterator map2IndexItr = (*map1IndexItr).second.begin();
		for( ; map2IndexItr != (*map1IndexItr).second.end(); ++map2IndexItr)
		{
			CXMLDOMElement* pContentRateNode = pContentRateTableNode->AddChildNode("CONTENT_RATE");
			pContentRateNode->AddChildNode("ENTERPRISE_MODE", (*map1IndexItr).first, ENTERPRISE_MODE_ENUM);
			pContentRateNode->AddChildNode("CASCADE_OPTIMIZE_RESOLUTION", (*map2IndexItr).first, CASCADE_OPTIMIZE_RESOLUTION_ENUM);
			pContentRateNode->AddChildNode("TRANSFER_RATE", (*map2IndexItr).second, TRANSFER_RATE_ENUM);
		}
	}

	char* buff = NULL;
	pActionsNode->DumpDataAsLongStringEx(&buff);
	PTRACE2(eLevelError,"contentpp CGetDynamicContentRateResTable::SerializeXml", buff);
	delete[] buff;
}

///////////////////////////////////////////////////////////////////////////
void CGetDynamicContentRateResTable::getAmcH264DynamicContentMinRate(
	RateResolutionMinValuesMap& amcH264DynamicContentRateResTableMinValues
	) const
{
	ContentRateMap amcH264DynamicContentRateResTable;
	CUnifiedComMode::getAmcH264DynamicContentRateResTable(amcH264DynamicContentRateResTable, m_isHighProfile);

	ContentRateMap::iterator map1IndexItr = amcH264DynamicContentRateResTable.begin();
	for ( ; map1IndexItr != amcH264DynamicContentRateResTable.end(); ++map1IndexItr)
	{
		RateResolutionMinValuesMap::iterator map2IndexItr = (*map1IndexItr).second.begin();
		for (; map2IndexItr != (*map1IndexItr).second.end(); ++map2IndexItr)
		{
			Resolution2RateMap::iterator map3IndexItr = (*map2IndexItr).second.begin();
			for ( ; map3IndexItr != (*map2IndexItr).second.end(); ++map3IndexItr)
			{
				int minValue = amcH264DynamicContentRateResTableMinValues[(*map2IndexItr).first][(*map3IndexItr).first];

				if (minValue == 0 && (*map3IndexItr).second != 0)
				{
					amcH264DynamicContentRateResTableMinValues[(*map2IndexItr).first][(*map3IndexItr).first] = (*map1IndexItr).first;
					minValue = (*map1IndexItr).first;
				}

				if (CUnifiedComMode::TranslateXferRateToIpRate((*map1IndexItr).first) < CUnifiedComMode::TranslateXferRateToIpRate(minValue))
				{
					amcH264DynamicContentRateResTableMinValues[(*map2IndexItr).first][(*map3IndexItr).first] = (*map1IndexItr).first;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CGetCustomizedContentRateResTable::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	PTRACE(eLevelError,"contentpp CGetDynamicContentRateResTable::SerializeXml");

	Rate2ResolutionMap maxResolutionPerContentRateValues;

	CUnifiedComMode::getMaxResolutionPerContentRateTable(maxResolutionPerContentRateValues, m_isHighProfile);

	CXMLDOMElement* pContentRateTableNode;
	pContentRateTableNode = pActionsNode->AddChildNode("CUSTOMIZED_CONTENT_RATE_TABLE");

	Rate2ResolutionMap::iterator it = maxResolutionPerContentRateValues.begin();

	for (; it != maxResolutionPerContentRateValues.end(); ++it)
	{
		// Improve for BRIDGE-15526: for (BYTE resolution = 1; resolution <= it->second; ++resolution)
		{
			CXMLDOMElement* pContentRateNode = pContentRateTableNode->AddChildNode("CUSTOMIZED_CONTENT_RATE");
			pContentRateNode->AddChildNode("CASCADE_OPTIMIZE_RESOLUTION", it->second/*resolution*/, CASCADE_OPTIMIZE_RESOLUTION_ENUM);
			pContentRateNode->AddChildNode("TRANSFER_RATE", it->first, TRANSFER_RATE_ENUM);
		}
	}

	char* buff = NULL;
	pActionsNode->DumpDataAsLongStringEx(&buff);
	PTRACE2(eLevelError,"contentpp CGetDynamicContentRateResTable::SerializeXml", buff);
	delete[] buff;
}

//////////////////////////////////////////////////////////////////////////////////////
