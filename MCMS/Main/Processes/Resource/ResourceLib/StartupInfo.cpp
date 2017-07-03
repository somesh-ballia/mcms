#include "StartupInfo.h"
#include "ObjString.h"
#include "ResourceManager.h"

extern char* CardTypeToString(APIU32 cardType);

////////////////////////////////////////////////////////////////////////////
//                        CResourceStartupInfo
////////////////////////////////////////////////////////////////////////////
CResourceStartupInfo::CResourceStartupInfo()
{
	memset(this, 0, sizeof(*this));

	m_license_product_type = eProductTypeUnknown;
}

////////////////////////////////////////////////////////////////////////////
void CResourceStartupInfo::SystemCardsModeInd(eSystemCardsMode system_cards_mode)
{
	m_received_cards_mode = true;
	m_system_cards_mode = system_cards_mode;
}

////////////////////////////////////////////////////////////////////////////
void CResourceStartupInfo::LicensingInd(DWORD license_num_parties, eProductType license_product_type, bool is_Federal, bool isHD_enable, bool isRPPLicense)
{
	m_received_licensing = true;
	m_license_num_parties = license_num_parties;
	m_license_product_type = license_product_type;
	m_license_is_Federal = is_Federal;
	m_license_1500q_isHD_enable = isHD_enable;
	m_isRPP_license = isRPPLicense;
}

////////////////////////////////////////////////////////////////////////////
void CResourceStartupInfo::MultipleServicesInd(bool is_multiple_ip_services)
{
	m_received_multiple_ip_services = true;
	m_is_multiple_ip_services = is_multiple_ip_services;
}

////////////////////////////////////////////////////////////////////////////
void CResourceStartupInfo::IPServiceConfigReqInd(WORD service_id, char* service_name)
{
	m_received_ip_service_params++;

	for (WORD i = 0; i < MAX_NUM_OF_IP_SERVICES; i++)
	{
		if (ipServicesInfo[i].m_service_id == ((WORD)(-1)))
		{
			ipServicesInfo[i].m_service_id = service_id;
			if (service_name != NULL)
			{
				const int maxLenForKlocWork = sizeof(ipServicesInfo[i].m_service_name) - 1;
				strncpy(ipServicesInfo[i].m_service_name, service_name, maxLenForKlocWork);
				ipServicesInfo[i].m_service_name[maxLenForKlocWork] = '\0';
			}
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceStartupInfo::IPServicesParamsEndInd()
{
	m_received_ip_services_params_end = true;
}

////////////////////////////////////////////////////////////////////////////
void CResourceStartupInfo::MfaCardConfigReq(BoardID boardId, DWORD cardType)
{
	m_received_card_config_req++;
	for (WORD i = 0; i < BOARDS_NUM; i++)
	{
		if (mfaCardsInfo[i].m_board_id == ((WORD)(-1)))
		{
			mfaCardsInfo[i].m_board_id  = boardId;
			mfaCardsInfo[i].m_card_type = cardType;
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
DWORD CResourceStartupInfo::GetCardType(BoardID boardId)
{
	for (WORD i = 0; i < BOARDS_NUM; i++)
	{
		if (mfaCardsInfo[i].m_board_id == boardId)
			return mfaCardsInfo[i].m_card_type;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
void CResourceStartupInfo::MfaCardStartupComplete(BoardID boardId)
{
	m_received_card_startup_complete++;
}

////////////////////////////////////////////////////////////////////////////
bool CResourceStartupInfo::CanUpdateIpServicesDongleRestrictions() const
{
	if (m_received_cards_mode && m_received_licensing && m_received_ip_services_params_end && m_received_ip_service_params > 0 && m_received_card_startup_complete > 0)
		return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, CResourceStartupInfo& params)
{
	os
	<< "\nFlags:"
	<< "\n  received_cards_mode              :" << (int)params.m_received_cards_mode
	<< "\n  received_licensing               :" << (int)params.m_received_licensing
	<< "\n  received_multiple_ip_services    :" << (int)params.m_received_multiple_ip_services
	<< "\n  received_ip_service_params       :" << (int)params.m_received_ip_service_params
	<< "\n  received_ip_services_params_end  :" << (int)params.m_received_ip_services_params_end
	<< "\n  received_card_config_req         :" << (int)params.m_received_card_config_req
	<< "\n  received_card_startup_complete   :" << (int)params.m_received_card_startup_complete
	<< "\nData:"
	<< "\n  system_cards_mode                :" << ::GetSystemCardsModeStr(params.m_system_cards_mode)
	<< "\n  license_num_parties              :" << (int)params.m_license_num_parties
	<< "\n  license_product_type             :" << ::ProductTypeToString(params.m_license_product_type)
	<< "\n  license_1500q_hd_enable          :" << (int)params.m_license_1500q_isHD_enable
	<< "\n  license_rpp                      :" << (int)params.m_isRPP_license
	<< "\n  license_is_federal               :" << (int)params.m_license_is_Federal
	<< "\n  is_multiple_ip_services          :" << (int)params.m_is_multiple_ip_services
	<< "\nIpServicesInfo:";

	for (WORD i = 0; i < MAX_NUM_OF_IP_SERVICES; i++)
		if (params.ipServicesInfo[i].m_service_id != ((WORD)(-1)))
			os << "\n  service_id                       :" << params.ipServicesInfo[i].m_service_id << " (" << params.ipServicesInfo[i].m_service_name << ")";

	os << "\nCardsInfo:";
	for (WORD j = 0; j < BOARDS_NUM; j++)
		if (params.mfaCardsInfo[j].m_board_id != ((WORD)(-1)))
			os << "\n  board_id                         :" << params.mfaCardsInfo[j].m_board_id << " (" << ::CardTypeToString(params.mfaCardsInfo[j].m_card_type) << ")";

	return os;
}
