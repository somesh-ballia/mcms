#include "SelfConsistencyPartyRsrcDetails.h"
#include "ConfResources.h"
#include "SystemResources.h"
#include "InternalProcessStatuses.h"
#include "SelfConsistencyWhatToCheck.h"
#include "SelfConsistencyDefines.h"
#include "Board.h"
#include "HelperFuncs.h"

////////////////////////////////////////////////////////////////////////////
//                        CSelfConsistencyPartyRsrcDetails
////////////////////////////////////////////////////////////////////////////
CSelfConsistencyPartyRsrcDetails::CSelfConsistencyPartyRsrcDetails(CPartyRsrc* pPartyRsrc, DWORD rsrcConfId)
{
	m_pPartyRsrc                  = pPartyRsrc;
	m_RsrcConfId                  = rsrcConfId;
	m_num_of_art_descriptors      = 0;
	m_num_of_video_descriptors    = 0;
	m_num_of_net_descriptors      = 0;
	m_art_capacity_in_promilles   = 0;
	m_weighted_art                = 0;
	m_video_capacity_in_promilles = 0;
	m_num_of_art_ports            = 0;
	m_num_of_video_ports          = 0;
	m_pArt_board                  = NULL;
	m_pVideo_board                = NULL;
	m_pUdpRsrcDesc                = NULL;

	if (pPartyRsrc == NULL)
	{
		GetOStreamAndPrintParty() << "pPartyRsrc is NULL in CSelfConsistencyPartyRsrcDetails::CSelfConsistencyPartyRsrcDetails\n";
	}
}

//--------------------------------------------------------------------------
CSelfConsistencyPartyRsrcDetails::~CSelfConsistencyPartyRsrcDetails()
{
}

//--------------------------------------------------------------------------
const char* CSelfConsistencyPartyRsrcDetails::NameOf() const
{
	return " CSelfConsistencyPartyRsrcDetails ";
}

//--------------------------------------------------------------------------
void CSelfConsistencyPartyRsrcDetails::AddResourceDescriptor(CRsrcDesc* pRsrcdesc)
{
	eLogicalResourceTypes type = pRsrcdesc->GetType();
	switch (type)
	{
		case eLogical_video_encoder:
		case eLogical_video_decoder:
			m_num_of_video_descriptors++;
			break;

		case eLogical_audio_encoder:
		case eLogical_audio_decoder:
		case eLogical_rtp:
			m_num_of_art_descriptors++;
			break;

		case eLogical_net:
			m_num_of_net_descriptors++;
			break;

		default:
			TRACEINTO << "D.K.LogicalResourceType:" << type << " - Skiped";
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
}

//--------------------------------------------------------------------------
void CSelfConsistencyPartyRsrcDetails::AddActivePort(CActivePort* pActivePort, CBoard* pBoard)
{
	eResourceTypes type = pActivePort->GetPortType();
	switch (type)
	{
		case ePhysical_video_encoder:
		case ePhysical_video_decoder:
		if (m_pVideo_board == NULL)  // wasn't set yet
		{
			m_pVideo_board = pBoard;
		}
		else                         // if it was set already, check that it's still the same
		{
			if (m_pVideo_board != pBoard)
			{
				GetOStreamAndPrintParty() << "CSelfConsistencyPartyRsrcDetails::AddActivePort: m_pVideo_board (with board id "<< m_pVideo_board->GetOneBasedBoardId()
				                          << ") is different than pBoard (with board id "  << pBoard->GetOneBasedBoardId() << ")\n";
			}
		}

		m_video_capacity_in_promilles += pActivePort->GetPromilUtilized();
		m_num_of_video_ports++;
		break;

		case ePhysical_art:
		// there should be only one art active port
		if (m_art_capacity_in_promilles != 0)
		{
			GetOStreamAndPrintParty() << "m_art_capacity_in_promilles is NOT 0, but trying to add another art active port\n";
		}

		if (m_pArt_board != NULL)
		{
			GetOStreamAndPrintParty() << "m_pArt_board is not NULL\n";
		}

		m_pArt_board                 = pBoard;
		m_art_capacity_in_promilles += pActivePort->GetPromilUtilized();
		m_weighted_art              += pActivePort->GetCapacity();
		m_num_of_art_ports++;
		break;

		default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
	} // switch
}

//--------------------------------------------------------------------------
void CSelfConsistencyPartyRsrcDetails::AddUDPDescriptor(CUdpRsrcDesc* pUdpRsrcDesc)
{
	// there should be only one UDP descriptor, and of course it should not be null
	if (pUdpRsrcDesc == NULL || m_pUdpRsrcDesc != NULL)
	{
		if (pUdpRsrcDesc == NULL)
			GetOStreamAndPrintParty() << "pUdpRsrcDesc is NULL in CSelfConsistencyPartyRsrcDetails::CSelfConsistencyPartyRsrcDetails\n";
		else if (m_pUdpRsrcDesc != NULL)
			GetOStreamAndPrintParty() << "m_pUdpRsrcDesc is NOT NULL in CSelfConsistencyPartyRsrcDetails::CSelfConsistencyPartyRsrcDetails\n";

		return;
	}

	m_pUdpRsrcDesc = pUdpRsrcDesc;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistencyPartyRsrcDetails::CheckConsistency(CSelfConsistencyWhatToCheck* pSelfConsistencyWhatToCheck)
{
	STATUS finalStatus = STATUS_OK;

	if (pSelfConsistencyWhatToCheck->m_bCheckPartiesUDPDescriptors)
	{
		if (CheckUDPDescriptor() != STATUS_OK)
			finalStatus = STATUS_FAIL;
	}

	if (pSelfConsistencyWhatToCheck->m_bCheckPartiesPorts)
	{
		if (CheckVideoCapacityAndPorts() != STATUS_OK)
			finalStatus = STATUS_FAIL;

		if (CheckArtCapacityAndPorts() != STATUS_OK)
			finalStatus = STATUS_FAIL;
	}

	if (pSelfConsistencyWhatToCheck->m_bCheckPartiesResourceDescriptors)
	{
		if (CheckNumVideoDescriptors() != STATUS_OK)
			finalStatus = STATUS_FAIL;

		if (CheckNumArtDescriptors() != STATUS_OK)
			finalStatus = STATUS_FAIL;

		if (CheckNumNetDescriptors() != STATUS_OK)
			finalStatus = STATUS_FAIL;
	}

	return finalStatus;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistencyPartyRsrcDetails::CheckArtCapacityAndPorts()
{
	// for all types there should be one ART port, with the appropriate capacity
	if (m_pArt_board == NULL)
	{
		GetOStreamAndPrintParty() << "m_pArt_board is NULL\n";
		return STATUS_FAIL;
	}

	if (m_art_capacity_in_promilles != m_pArt_board->CalculateNeededPromilles(PORT_ART, m_weighted_art))
	{
		GetOStreamAndPrintParty() << "m_art_capacity_in_promilles (" << m_art_capacity_in_promilles << ") is not equal to m_pArt_board->CalculateNeededPromilles(PORT_ART,m_weighted_art) ("<< m_pArt_board->CalculateNeededPromilles(PORT_ART, m_weighted_art) <<")\n";
		return STATUS_FAIL;
	}

	if (m_num_of_art_ports != SC_NUM_OF_ART_PORTS_PER_PARTY)
	{
		GetOStreamAndPrintParty() << "m_num_of_art_ports (" << m_num_of_art_ports << ") is not equal to SC_NUM_OF_ART_PORTS_PER_PARTY\n";
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistencyPartyRsrcDetails::CheckNumArtDescriptors()
{
	WORD expectedNumOfArtDescriptors = SC_NUM_OF_ART_DESCRIPTORS_FOR_IP_PARTY;
	if (CHelperFuncs::IsISDNParty(m_pPartyRsrc->GetNetworkPartyType()))
		expectedNumOfArtDescriptors = SC_NUM_OF_ART_DESCRIPTORS_FOR_ISDN_PARTY;

	if (m_num_of_art_descriptors != expectedNumOfArtDescriptors)
	{
		GetOStreamAndPrintParty() << "m_num_of_art_descriptors (" << m_num_of_art_descriptors
		                          << ") is not equal to expectedNumOfArtDescriptors (" << expectedNumOfArtDescriptors << ")\n";
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistencyPartyRsrcDetails::CheckNumNetDescriptors()
{
	if (CHelperFuncs::IsVideoISDNParty(m_pPartyRsrc->GetNetworkPartyType(), m_pPartyRsrc->GetVideoPartyType()))
	{
		// each party should have at least one channel
		if (m_num_of_net_descriptors < SC_MINIMUM_NUM_OF_NET_DESCRIPTORS_FOR_ISDN_PARTY)
		{
			GetOStreamAndPrintParty() << "m_num_of_net_descriptors (" << m_num_of_net_descriptors
			                          << ") is less than SC_MINIMUM_NUM_OF_NET_DESCRIPTORS_FOR_ISDN_PARTY (" << SC_MINIMUM_NUM_OF_NET_DESCRIPTORS_FOR_ISDN_PARTY << ")\n";
			return STATUS_FAIL;
		}
	}
	else
	{
		WORD expectedNumOfNetDescriptors = SC_NUM_OF_NET_DESCRIPTORS_FOR_IP_PARTY;
		if (CHelperFuncs::IsPSTNParty(m_pPartyRsrc->GetNetworkPartyType(), m_pPartyRsrc->GetVideoPartyType()))
			expectedNumOfNetDescriptors = SC_NUM_OF_NET_DESCRIPTORS_FOR_PSTN_PARTY;

		if (m_num_of_net_descriptors != expectedNumOfNetDescriptors)
		{
			GetOStreamAndPrintParty() << "m_num_of_net_descriptors (" << m_num_of_net_descriptors
			                          << ") is not equal to expectedNumOfNetDescriptors (" << expectedNumOfNetDescriptors << ")\n";
			return STATUS_FAIL;
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistencyPartyRsrcDetails::CheckVideoCapacityAndPorts()
{
	WORD  expectedNumOfVideoPorts    = 0;
	float expectedNumOfVideoCapacity = 0;
	if (CHelperFuncs::IsVideoParty(m_pPartyRsrc->GetVideoPartyType()))
	{
		if (m_pVideo_board == NULL)
		{
			GetOStreamAndPrintParty() << "m_pVideo_board is NULL\n";
			return STATUS_FAIL;
		}

		AllocData videoCurrentAllocData;
		m_pVideo_board->FillVideoUnitsList(m_pPartyRsrc->GetVideoPartyType(), videoCurrentAllocData);

		WORD      max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
		WORD      max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
		for (int i = 0; i < max_units_video; i++)
		{
			for (int j = 0; j < max_media_ports; j++)
			{
				if (videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
				{
					expectedNumOfVideoPorts++;
					expectedNumOfVideoCapacity += videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_needed_capacity_promilles;
				}
			}
		}
	}

	if (m_video_capacity_in_promilles != expectedNumOfVideoCapacity)
	{
		GetOStreamAndPrintParty() << "m_video_capacity_in_promilles (" << m_video_capacity_in_promilles
		                          << ") is not equal to expectedNumOfVideoCapacity (" << expectedNumOfVideoCapacity << ")\n";
		return STATUS_FAIL;
	}

	if (m_num_of_video_ports != expectedNumOfVideoPorts)
	{
		GetOStreamAndPrintParty() << "m_num_of_video_ports (" << m_num_of_video_ports
		                          << ") is not equal to expectedNumOfVideoPorts (" << expectedNumOfVideoPorts << ")\n";
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistencyPartyRsrcDetails::CheckNumVideoDescriptors()
{
	WORD expectedNumOfVideoDescriptors = 0;
	if (CHelperFuncs::IsVideoParty(m_pPartyRsrc->GetVideoPartyType()))
	{
		if (m_pVideo_board == NULL)
		{
			GetOStreamAndPrintParty() << "m_pVideo_board is NULL\n";
			return STATUS_FAIL;
		}

		AllocData videoCurrentAllocData;
		m_pVideo_board->FillVideoUnitsList(m_pPartyRsrc->GetVideoPartyType(), videoCurrentAllocData);

		WORD      max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
		WORD      max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
		for (int i = 0; i < max_units_video; i++)
		{
			for (int j = 0; j < max_media_ports; j++)
			{
				if (videoCurrentAllocData.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
				{
					expectedNumOfVideoDescriptors++;
				}
			}
		}
	}

	if (m_num_of_video_descriptors != expectedNumOfVideoDescriptors)
	{
		GetOStreamAndPrintParty() << "m_num_of_video_descriptors (" << m_num_of_video_descriptors
		                          << ") is not equal to expectedNumOfVideoDescriptors (" << expectedNumOfVideoDescriptors << ")\n";
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CSelfConsistencyPartyRsrcDetails::CheckUDPDescriptor()
{
	if (m_pUdpRsrcDesc &&
	    ((m_pUdpRsrcDesc->m_udp.AudioChannelAdditionalPorts != 0 && m_pUdpRsrcDesc->m_udp.AudioChannelPort == 0)   // ICE 4 ports
	     || (m_pUdpRsrcDesc->m_udp.VideoChannelAdditionalPorts != 0 && m_pUdpRsrcDesc->m_udp.VideoChannelPort == 0)
	     || (m_pUdpRsrcDesc->m_udp.FeccChannelAdditionalPorts != 0 && m_pUdpRsrcDesc->m_udp.FeccChannelPort == 0)
	     || (m_pUdpRsrcDesc->m_udp.ContentChannelAdditionalPorts != 0 && m_pUdpRsrcDesc->m_udp.ContentChannelPort == 0)
	     || (m_pUdpRsrcDesc->m_udp.BfcpChannelAdditionalPorts != 0 && m_pUdpRsrcDesc->m_udp.BfcpChannelPort == 0)))
	{
		PrintWrongUDP();
		return STATUS_FAIL;
	}

	if (CHelperFuncs::IsISDNParty(m_pPartyRsrc->GetNetworkPartyType()))
	{
		// no udp ports for ISDN
		if (m_pUdpRsrcDesc != NULL)
		{
			GetOStreamAndPrintParty() << "m_pUdpRsrcDesc is not NULL for ISDN party\n";
			PrintWrongUDP();
			return STATUS_FAIL;
		}
	}
	else if (m_pPartyRsrc->GetVideoPartyType() == eVideo_party_type_none) // voip (because we already checked isdn
	{
		if (m_pUdpRsrcDesc == NULL)
		{
			GetOStreamAndPrintParty() << "m_pUdpRsrcDesc is NULL for VOIP party\n";
			return STATUS_FAIL;
		}

		// for voip parties there should be only an audio channel UDP port
		if (m_pUdpRsrcDesc->m_udp.AudioChannelPort == 0
		    || m_pUdpRsrcDesc->m_udp.VideoChannelPort != 0
		    || m_pUdpRsrcDesc->m_udp.FeccChannelPort != 0
		    || m_pUdpRsrcDesc->m_udp.ContentChannelPort != 0)
		{
			PrintWrongUDP();
			return STATUS_FAIL;
		}
	}
	else
	{
		if (m_pUdpRsrcDesc == NULL)
		{
			GetOStreamAndPrintParty() << "m_pUdpRsrcDesc is NULL for IP video party\n";
			return STATUS_FAIL;
		}

		// for IP video parties all channels should have a UDP port
		if (m_pUdpRsrcDesc->m_udp.AudioChannelPort == 0
		    || m_pUdpRsrcDesc->m_udp.VideoChannelPort == 0
		    || m_pUdpRsrcDesc->m_udp.FeccChannelPort == 0
		    || m_pUdpRsrcDesc->m_udp.ContentChannelPort == 0)
		{
			PrintWrongUDP();
			return STATUS_FAIL;
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CSelfConsistencyPartyRsrcDetails::PrintWrongUDP()
{
	GetOStreamAndPrintParty() << "Wrong UDP port:"
	                          <<" m_pUdpRsrcDesc->m_udp.AudioChannelPort: " << m_pUdpRsrcDesc->m_udp.AudioChannelPort
	                          <<" m_pUdpRsrcDesc->m_udp.AudioChannelAdditionalPorts: " << m_pUdpRsrcDesc->m_udp.AudioChannelAdditionalPorts // ICE 4 ports
	                          <<" m_pUdpRsrcDesc->m_udp.VideoChannelPort: " << m_pUdpRsrcDesc->m_udp.VideoChannelPort
	                          <<" m_pUdpRsrcDesc->m_udp.VideoChannelAdditionalPorts: " << m_pUdpRsrcDesc->m_udp.VideoChannelAdditionalPorts
	                          <<" m_pUdpRsrcDesc->m_udp.FeccChannelPort: " << m_pUdpRsrcDesc->m_udp.FeccChannelPort
	                          <<" m_pUdpRsrcDesc->m_udp.FeccChannelAdditionalPorts: " << m_pUdpRsrcDesc->m_udp.FeccChannelAdditionalPorts
	                          <<" m_pUdpRsrcDesc->m_udp.ContentChannelPort: " << m_pUdpRsrcDesc->m_udp.ContentChannelPort
	                          <<" m_pUdpRsrcDesc->m_udp.ContentChannelAdditionalPorts: " << m_pUdpRsrcDesc->m_udp.ContentChannelAdditionalPorts
	                          <<" m_pUdpRsrcDesc->m_udp.BfcpChannelPort: " << m_pUdpRsrcDesc->m_udp.BfcpChannelPort
	                          <<" m_pUdpRsrcDesc->m_udp.BfcpChannelAdditionalPorts: " << m_pUdpRsrcDesc->m_udp.BfcpChannelAdditionalPorts
	                          <<"\n";
}

//--------------------------------------------------------------------------
eVideoPartyType CSelfConsistencyPartyRsrcDetails::GetPartyType()
{
	return m_pPartyRsrc->GetVideoPartyType();
}

//--------------------------------------------------------------------------
ePartyRole CSelfConsistencyPartyRsrcDetails::     GetPartyRole()
{
	return m_pPartyRsrc->GetPartyRole();
}

//--------------------------------------------------------------------------
COstrStream& CSelfConsistencyPartyRsrcDetails::   GetOStreamAndPrintParty()
{
	COstrStream& stream = CSelfConsistency::GetOStream();
	stream << "Party: m_RsrcConfId - " << m_RsrcConfId;
	if (m_pPartyRsrc != NULL)
		stream << " RsrcPartyId - " << m_pPartyRsrc->GetRsrcPartyId()
		<< " MonitorPartyId - " << m_pPartyRsrc->GetMonitorPartyId()
		<< " VideoPartyType - " << eVideoPartyTypeNames[m_pPartyRsrc->GetVideoPartyType()]
		<< " NetworkPartyType - " << eNetworkPartyTypeNames[m_pPartyRsrc->GetNetworkPartyType()]
		;
	else
		stream << " m_pPartyRsrc is null";

	stream << " : ";
	return stream;
}
