#include "TraceStream.h"
#include "EnhancedConfigResponse.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include "ObjString.h"
#include "PrettyTable.h"

////////////////////////////////////////////////////////////////////////////
//                        CEnhancedConfigResponse
////////////////////////////////////////////////////////////////////////////
CEnhancedConfigResponse::CEnhancedConfigResponse()
{
}

//--------------------------------------------------------------------------
CEnhancedConfigResponse::~CEnhancedConfigResponse()
{
}

//--------------------------------------------------------------------------
void CEnhancedConfigResponse::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pEnhancedNode = pFatherNode->AddChildNode("ENHANCED_PORT_CONFIGURATION");
	m_ConfigResponse[e_Audio].SerializeXml(pEnhancedNode, "AUDIO_CONFIG");
	m_ConfigResponse[e_Cif].SerializeXml(pEnhancedNode, "CIF_CONFIG");
	m_ConfigResponse[e_SD30].SerializeXml(pEnhancedNode, "SD_CONFIG");
	m_ConfigResponse[e_HD720].SerializeXml(pEnhancedNode, "HD720_CONFIG");
	m_ConfigResponse[e_HD1080p30].SerializeXml(pEnhancedNode, "HD1080_CONFIG");
}

//--------------------------------------------------------------------------
int CEnhancedConfigResponse::DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* action)
{
	// only for sending response to API
	PASSERTMSG(TRUE, "CEnhancedConfigResponse::DeSerializeXml - Should not be called");
	return STATUS_OK;
}

//--------------------------------------------------------------------------
CEnhancedConfigResponseItem* CEnhancedConfigResponse::GetConfigItem(ePartyResourceTypes type)
{
	PASSERT_AND_RETURN_VALUE(type >= NUM_OF_PARTY_RESOURCE_TYPES, 0);

	return &(m_ConfigResponse[type]);
}

//--------------------------------------------------------------------------
void CEnhancedConfigResponse::DumpToTrace()
{
	CPrettyTable<const char*, WORD, WORD, WORD, WORD> tbl("Type", "System max", "Optional max", "Current", "Step");

	for (int i = e_Audio; i < NUM_OF_PARTY_RESOURCE_TYPES; ++i)
	{
		ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
		tbl.Add(to_string(partyResourceType),
		        m_ConfigResponse[i].GetSystemMaximum(),
		        m_ConfigResponse[i].GetOptionalMaximum(),
		        m_ConfigResponse[i].GetCurrent(),
		        m_ConfigResponse[i].GetStep());
	}

	TRACEINTO << "Ports per resource type" << tbl.Get();
}


////////////////////////////////////////////////////////////////////////////
//                        CEnhancedConfigResponseItem
////////////////////////////////////////////////////////////////////////////
CEnhancedConfigResponseItem::CEnhancedConfigResponseItem()
{
	m_system_maximum   = 0;
	m_current          = 0;
	m_optional_maximum = 0;
	m_step             = 1;
}

//--------------------------------------------------------------------------
CEnhancedConfigResponseItem::~CEnhancedConfigResponseItem()
{
}

//--------------------------------------------------------------------------
void CEnhancedConfigResponseItem::SerializeXml(CXMLDOMElement*& pFatherNode, char* NodeName) const
{
	CXMLDOMElement* pItemNode = pFatherNode->AddChildNode(NodeName);
	pItemNode->AddChildNode("CONFIG_SYSTEM_MAXIMUM", m_system_maximum);
	pItemNode->AddChildNode("CONFIG_CURRENT", m_current);
	pItemNode->AddChildNode("CONFIG_OPTIONAL_MAXIMUM", m_optional_maximum);
	pItemNode->AddChildNode("CONFIG_STEP", m_step);
}
