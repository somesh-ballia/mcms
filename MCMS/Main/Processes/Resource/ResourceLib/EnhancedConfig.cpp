#include "EnhancedConfig.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include "TraceStream.h"
#include "ProcessSettings.h"
#include "HelperFuncs.h"
#include "ObjString.h"
#include "PrettyTable.h"

static const char* PureModeConfigurationTypesNames[] =
{
	"audioEnhanced", // audioEnhanced - so not to have collision with "audio" of mixed mode
	"cif",
	"sd30",
	"hd720",
	"hd1080"
};

static const char* PureModeXMLTypesNames[] =
{
	"AUDIO_NUM_PORTS_CONFIG",
	"CIF_NUM_PORTS_CONFIG",
	"SD_NUM_PORTS_CONFIG",
	"HD720_NUM_PORTS_CONFIG",
	"HD1080_NUM_PORTS_CONFIG"
};

////////////////////////////////////////////////////////////////////////////
//                        CEnhancedConfig
////////////////////////////////////////////////////////////////////////////
CEnhancedConfig::CEnhancedConfig()
{
	memset(m_Configuration, 0, sizeof(m_Configuration));
}

////////////////////////////////////////////////////////////////////////////
CEnhancedConfig::~CEnhancedConfig()
{
}

////////////////////////////////////////////////////////////////////////////
CEnhancedConfig& CEnhancedConfig::operator=(const CEnhancedConfig& other)
{
	memcpy(m_Configuration, other.m_Configuration, sizeof(m_Configuration));

	return *this;
}

////////////////////////////////////////////////////////////////////////////
CEnhancedConfig& CEnhancedConfig::SetIpServicePartConfig(float service_factor, BOOL round_up)
{
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		m_Configuration[i] = (WORD)CHelperFuncs::CalcIpServicePart((DWORD)m_Configuration[i], service_factor, round_up);

	return *this;
}

////////////////////////////////////////////////////////////////////////////
int CEnhancedConfig::GetConfiguration(ePartyResourceTypes type) const
{
	PASSERT_AND_RETURN_VALUE(type >= NUM_OF_PARTY_RESOURCE_TYPES, 0);

	return m_Configuration[type];
}

////////////////////////////////////////////////////////////////////////////
void CEnhancedConfig::SetConfiguration(ePartyResourceTypes type, int configuration)
{
	PASSERT_AND_RETURN(type >= NUM_OF_PARTY_RESOURCE_TYPES);

	m_Configuration[type] = configuration;
}

////////////////////////////////////////////////////////////////////////////
int CEnhancedConfig::GetConfiguration(int type) const
{
	if (type < 0 || type > NUM_OF_PARTY_RESOURCE_TYPES) {
		PASSERTMSG_AND_RETURN_VALUE(0, "invalid type in CEnhancedConfig::GetConfiguration", 0);
	}

	return GetConfiguration((ePartyResourceTypes)type);
}

////////////////////////////////////////////////////////////////////////////
void CEnhancedConfig::SetConfiguration(int type, int configuration)
{
	if (type < 0 || type > NUM_OF_PARTY_RESOURCE_TYPES) {
		PASSERTMSG_AND_RETURN(0, "invalid type in CEnhancedConfig::SetConfiguration");
	}

	SetConfiguration((ePartyResourceTypes)type, configuration);
}

////////////////////////////////////////////////////////////////////////////
void CEnhancedConfig::SerializeXml(CXMLDOMElement*& thisNode) const
{
	PASSERTMSG(TRUE, "CEnhancedConfig::SerializeXml - Should not be called"); // only for receiving from API
}

////////////////////////////////////////////////////////////////////////////
int CEnhancedConfig::DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* action)
{
	TRACEINTO;

	int nStatus = STATUS_OK;

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		GET_VALIDATE_CHILD(pNode, (char*)PureModeXMLTypesNames[i], &m_Configuration[i], _0_TO_WORD);
		if (nStatus)
			return nStatus;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CEnhancedConfig::DumpToTrace()
{
	CPrettyTable<const char*, WORD> tbl("Type", "Ports");

	for (int i = e_Audio; i < NUM_OF_PARTY_RESOURCE_TYPES; ++i)
	{
		ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
		tbl.Add(to_string(partyResourceType), m_Configuration[i]);
	}

	TRACEINTO << "Ports per resource type" << tbl.Get();
}

////////////////////////////////////////////////////////////////////////////
void CEnhancedConfig::ReadFromProcessSetting()
{
	TRACEINTO;

	CProcessSettings* pProcessSettings = CHelperFuncs::GetProcessSettings();
	PASSERT_AND_RETURN(pProcessSettings == NULL);

	DWORD temp;

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		if (pProcessSettings->GetSettingDWORD(PureModeConfigurationTypesNames[i], temp))
		{
			m_Configuration[i] = temp;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CEnhancedConfig::WriteToProcessSetting()
{
	TRACEINTO;

	CProcessSettings* pProcessSettings = CHelperFuncs::GetProcessSettings();
	PASSERT_AND_RETURN(pProcessSettings == NULL);

	ALLOCBUFFER(temp, ONE_LINE_BUFFER_LEN);

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		sprintf(temp, "%d", m_Configuration[i]);
		pProcessSettings->SetSetting(PureModeConfigurationTypesNames[i], temp);
	}

	DEALLOCBUFFER(temp);
}



