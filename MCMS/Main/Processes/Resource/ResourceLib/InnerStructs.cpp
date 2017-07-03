#include "InnerStructs.h"
#include "ObjString.h"
#include "HelperFuncs.h" // carmit-fix
#include <math.h>
#include <sstream>
#include "Trace.h"
#include "TraceStream.h"


////////////////////////////////////////////////////////////////////////////
//                        MediaUnit
////////////////////////////////////////////////////////////////////////////
float MediaUnit::GetTotalNeededCapacity()
{
	float total = 0;
	WORD  max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
	FPASSERTSTREAM_AND_RETURN_VALUE(max_media_ports > MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP, "", STATUS_OUT_OF_RANGE);

	for (int i = 0; i < max_media_ports; i++)
		total += m_MediaPortsList[i].m_needed_capacity_promilles;

	return total;
}

//--------------------------------------------------------------------------
float MediaUnit::GetTotalNeededEncoderCapacity()
{
	float total = 0;
	WORD  max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	FPASSERTSTREAM_AND_RETURN_VALUE(max_media_ports > MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP, "", STATUS_OUT_OF_RANGE);
	for (int i = 0; i < max_media_ports; i++)
	{
		if (CHelperFuncs::IsLogicalVideoEncoderType(m_MediaPortsList[i].m_type))
		{
			total += m_MediaPortsList[i].m_needed_capacity_promilles;
		}
	}

	return total;
}

//--------------------------------------------------------------------------
float MediaUnit::GetTotalNeededDecoderCapacity()
{
	float total = 0;
	WORD  max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	FPASSERTSTREAM_AND_RETURN_VALUE(max_media_ports > MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP, "", STATUS_OUT_OF_RANGE);
	for (int i = 0; i < max_media_ports; i++)
	{
		if (CHelperFuncs::IsLogicalVideoDecoderType(m_MediaPortsList[i].m_type))
		{
			total += m_MediaPortsList[i].m_needed_capacity_promilles;
		}
	}

	return total;
}

//--------------------------------------------------------------------------
DWORD MediaUnit::GetTotalNeededBandwidthIn()
{
	DWORD total = 0;
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	FPASSERTSTREAM_AND_RETURN_VALUE(max_media_ports > MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP, "", STATUS_OUT_OF_RANGE);
	for (int i = 0; i < max_media_ports; i++)
		total += m_MediaPortsList[i].m_needed_bandwidth_in;

	return total;
}

//--------------------------------------------------------------------------
DWORD MediaUnit::GetTotalNeededBandwidthOut()
{
	DWORD total = 0;
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	FPASSERTSTREAM_AND_RETURN_VALUE(max_media_ports > MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP, "", STATUS_OUT_OF_RANGE);
	for (int i = 0; i < max_media_ports; i++)
		total += m_MediaPortsList[i].m_needed_bandwidth_out;

	return total;
}

float MediaUnit::GetTotalNeededEncoderWeight()
{
	float total = 0;
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	FPASSERTSTREAM_AND_RETURN_VALUE(max_media_ports > MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP, "", STATUS_OUT_OF_RANGE);
	for(int i=0; i<max_media_ports; i++)
		total += m_MediaPortsList[i].m_needed_encoder_weight;
	return total;
}

WORD MediaUnit::GetTotalNeededVideoPortsPerType(BOOL isEncoderType)
{
	WORD total = 0;
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	FPASSERTSTREAM_AND_RETURN_VALUE(max_media_ports > MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP, "", STATUS_OUT_OF_RANGE);
	for (int i = 0; i < max_media_ports; i++)
	{
		if (isEncoderType) // count encoders
		{
			if (CHelperFuncs::IsLogicalVideoEncoderType(m_MediaPortsList[i].m_type))
				++total;
		}
		else // count decoders
		{
			if (CHelperFuncs::IsLogicalVideoDecoderType(m_MediaPortsList[i].m_type))
				++total;
		}
	}

	return total;
}

void MediaUnit::SetAcceleratorIdForAllPorts(WORD acceleratorId)
{
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	FPASSERTSTREAM(max_media_ports > MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP, "");
	for (int i=0; i<max_media_ports; i++)
		if (m_MediaPortsList[i].m_type != eLogical_res_none)
		{
			m_MediaPortsList[i].m_acceleratorId = acceleratorId;
		}
}

////////////////////////////////////////////////////////////////////////////
//                        MediaPort
////////////////////////////////////////////////////////////////////////////
eResourceTypes MediaPort::GetPhysicalResourceType()
{
	switch (m_type)
	{
		case eLogical_audio_encoder:
			return ePhysical_art;
		case eLogical_audio_decoder:
			return ePhysical_art;

		case eLogical_video_encoder:
		case eLogical_video_encoder_content:
		case eLogical_COP_CIF_encoder:
		case eLogical_COP_VSW_encoder:
		case eLogical_COP_PCM_encoder:
		case eLogical_COP_HD720_encoder:
		case eLogical_COP_HD1080_encoder:
		case eLogical_COP_4CIF_encoder:
			return ePhysical_video_encoder;

		case eLogical_video_decoder:
		case eLogical_COP_Dynamic_decoder:
		case eLogical_COP_VSW_decoder:
		case eLogical_COP_LM_decoder:
			return ePhysical_video_decoder;

        //OLGA - SoftMCU
        case eLogical_relay_audio_encoder:
        case eLogical_relay_audio_decoder:
        case eLogical_legacy_to_SAC_audio_encoder:
        case eLogical_relay_avc_to_svc_rtp:
        case eLogical_relay_avc_to_svc_rtp_with_audio_encoder:
            return ePhysical_art;

		case eLogical_relay_rtp:
		case eLogical_relay_svc_to_avc_rtp:
		case eLogical_relay_video_encoder:
		    return ePhysical_mrmp;

		case eLogical_relay_avc_to_svc_video_encoder_1:
		case eLogical_relay_avc_to_svc_video_encoder_2:
			return ePhysical_video_encoder;
	    //OLGA - SoftMCU

		default:
		    FPTRACE2INT(eLevelInfoNormal, "MediaPort::GetPhysicalResourceType : type=", (WORD)m_type);
			return ePhysical_res_none;
	} // switch
}


////////////////////////////////////////////////////////////////////////////
//                        AllocData
////////////////////////////////////////////////////////////////////////////

AllocData::AllocData()
{
	memset(this, 0, sizeof(*this));
	m_confModeType = eNonMix;
	m_boardId      = 0xFFFF;
}

//--------------------------------------------------------------------------
MediaUnit* AllocData::GetUnitThatIncludesThisPort(eLogicalResourceTypes type, ECntrlType cntrl_type)
{
	WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
	FPASSERTSTREAM(max_units_video > MAX_UNITS_NEEDED_FOR_VIDEO_COP, "");

	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
	FPASSERTSTREAM(max_media_ports > MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP, "");

	for (int i = 0; i < max_units_video && i < MAX_UNITS_NEEDED_FOR_VIDEO_COP; i++)
	{
		for (int j = 0; j < max_media_ports && j < MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP; j++)
		{
			if (m_unitsList[i].m_MediaPortsList[j].m_type == type &&
			    m_unitsList[i].m_MediaPortsList[j].m_cntrl_type == cntrl_type)
			{
				return &(m_unitsList[i]);
			}
		}
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////////
//                        CPartiesResources
////////////////////////////////////////////////////////////////////////////
CPartiesResources::CPartiesResources()
{
	Init();
}

//--------------------------------------------------------------------------
void CPartiesResources::Init()
{
	memset(this, 0, sizeof(*this));
}

//--------------------------------------------------------------------------
BOOL CPartiesResources::HasParties()
{
	return m_bHasParties;
}

//--------------------------------------------------------------------------
void CPartiesResources::SetIpServiceFactor(float factor, bool round_up)
{
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		// mul by factor
		m_logical_num_parties[i] *= factor;
		// round
		if (round_up)
		{
			if (m_logical_num_parties[i] != floor(m_logical_num_parties[i]))
			{
				m_logical_num_parties[i] = floor(m_logical_num_parties[i]) + 1;
			}
		}
		else
		{
			m_logical_num_parties[i] = floor(m_logical_num_parties[i]);
		}
	}

	m_logical_COP_num_parties *= factor;

	float physical_audio_ports = m_physical_audio_ports * factor;
	float physical_hd720_ports = m_physical_hd720_ports * factor;

	if (round_up)
	{
		if (m_logical_COP_num_parties != floor(m_logical_COP_num_parties))
		{
			m_logical_COP_num_parties = floor(m_logical_COP_num_parties) + 1;
		}

		if (physical_audio_ports != floor(physical_audio_ports))
		{
			m_physical_audio_ports = (WORD)physical_audio_ports + 1;
		}

		if (physical_hd720_ports != floor(physical_hd720_ports))
		{
			m_physical_hd720_ports = (WORD)physical_hd720_ports + 1;
		}
	}
	else
	{
		m_logical_COP_num_parties = floor(m_logical_COP_num_parties);
		m_physical_audio_ports = (WORD)physical_audio_ports;
		m_physical_hd720_ports = (WORD)physical_hd720_ports;
	}
}

//--------------------------------------------------------------------------
void CPartiesResources::DumpToTrace(char* calledFrom) const
{
	std::ostringstream msg;

	msg << calledFrom
	    << "\n  PhysicalAudioPorts  :" << m_physical_audio_ports
	    << "\n  PhysicalVideoPorts  :" << m_physical_hd720_ports
	    << "\n  HasParties          :" << (int)m_bHasParties;

	if (CHelperFuncs::IsMode2C())
	{
		//msg << "\n  m_logical_COP_num_parties  :" << m_logical_COP_num_parties;
	}
	else
	{
		for (int i = e_Audio; i < NUM_OF_PARTY_RESOURCE_TYPES; ++i)
		{
			ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
			msg << "\n  " << std::left << std::setw(20) << partyResourceType << ":" << m_logical_num_parties[i];
		}
	}
	FPTRACE(eLevelInfoHigh, msg.str().c_str());
}

//--------------------------------------------------------------------------
void CPartiesResources::CalculateHasParties()
{
	m_bHasParties = FALSE;

	if (CHelperFuncs::IsMode2C())
	{
		if (m_logical_COP_num_parties > 0)
		{
			m_bHasParties = TRUE;
			return;
		}
	}
	else
	{
		for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		{
			if (m_logical_num_parties[i] > 0)
			{
				m_bHasParties = TRUE;
				return;
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////
//                        CVideoPreview
////////////////////////////////////////////////////////////////////////////
CVideoPreview::CVideoPreview()
{
	m_confID    = 0xFFFFFFFF;
	m_partyID   = 0xFFFFFFFF;
	m_direction = ePREVIEW_NONE;
}

//--------------------------------------------------------------------------
CVideoPreview::CVideoPreview(DWORD confID, DWORD partyID, WORD direction)
{
	m_confID    = confID;
	m_partyID   = partyID;
	m_direction = direction;
}

//--------------------------------------------------------------------------
WORD operator==(const CVideoPreview& lvp, const CVideoPreview& rvp)
{
	if (lvp.m_confID == rvp.m_confID &&
	    lvp.m_partyID == rvp.m_partyID &&
	    lvp.m_direction == rvp.m_direction)
		return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------------
bool operator<(const CVideoPreview& lvp, const CVideoPreview& rvp)
{
	if (lvp.m_confID != rvp.m_confID)
		return lvp.m_confID < rvp.m_confID;

	if (lvp.m_partyID != rvp.m_partyID)
		return lvp.m_partyID < rvp.m_partyID;

	return lvp.m_direction < rvp.m_direction;
}

////////////////////////////////////////////////////////////////////////////
//                        CBoardsStatistics
////////////////////////////////////////////////////////////////////////////
CBoardsStatistics::CBoardsStatistics()
{
	for (DWORD i=0; i < BOARDS_NUM; i++)
	{
		m_NumConfiguredAudioPorts[i] = 0;
		m_NumConfiguredVideoPorts[i] = 0;
		m_NumOfEnabledArtUnits[i] = 0;
		m_NumOfEnabledVideoUnits[i] = 0;

		m_NumOfEnabledUnits[i] = 0;
		m_NumOfUnits[i] = 0;
	}
}

CBoardsStatistics& CBoardsStatistics::operator=(const CBoardsStatistics &other)
{
	for (DWORD i=0; i < BOARDS_NUM; i++)
	{
		m_NumConfiguredAudioPorts[i] = other.m_NumConfiguredAudioPorts[i];
		m_NumConfiguredVideoPorts[i] = other.m_NumConfiguredVideoPorts[i];
		m_NumOfEnabledArtUnits[i] = other.m_NumOfEnabledArtUnits[i];
		m_NumOfEnabledVideoUnits[i] = other.m_NumOfEnabledVideoUnits[i];

		m_NumOfEnabledUnits[i] = other.m_NumOfEnabledUnits[i];
		m_NumOfUnits[i] = other.m_NumOfUnits[i];
	}

	return *this;
}

DWORD CBoardsStatistics::GetTotalCunfiguredAudioPortsOnCards() const
{
	DWORD totalAudioPorts = 0;

	for (DWORD i=0; i < BOARDS_NUM; i++)
	{
		totalAudioPorts += m_NumConfiguredAudioPorts[i];
	}

	return totalAudioPorts;
}

DWORD CBoardsStatistics::GetTotalCunfiguredVideoPortsOnCards() const
{
	DWORD totalVideoPorts = 0;

	for (DWORD i=0; i < BOARDS_NUM; i++)
	{
		totalVideoPorts += m_NumConfiguredVideoPorts[i];
	}

	return totalVideoPorts;
}

DWORD CBoardsStatistics::GetTotalNumOfEnabledArtUnits() const
{
	DWORD totalArtUnits = 0;

	for (DWORD i=0; i < BOARDS_NUM; i++)
	{
		totalArtUnits += m_NumOfEnabledArtUnits[i];
	}

	return totalArtUnits;
}

DWORD CBoardsStatistics::GetTotalNumOfEnabledVideoUnits() const
{
	DWORD totalVideoUnits = 0;

	for (DWORD i=0; i < BOARDS_NUM; i++)
	{
		totalVideoUnits += m_NumOfEnabledVideoUnits[i];
	}

	return totalVideoUnits;
}

