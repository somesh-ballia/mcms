#ifndef _RESOURCE_STARTUP_INFO_H_
#define _RESOURCE_STARTUP_INFO_H_

#include "PObject.h"
#include "CardsStructs.h"
#include "ProductType.h"
#include "EnumsAndDefines.h"
#include "AllocateStructs.h"

struct sIpServiceInfo
{
	WORD m_service_id;
	char m_service_name[NET_SERVICE_PROVIDER_NAME_LEN+1];
};

struct sMfaCardInfo
{
	BoardID m_board_id;
	DWORD   m_card_type;
};

////////////////////////////////////////////////////////////////////////////
//                        CResourceStartupInfo
////////////////////////////////////////////////////////////////////////////
class CResourceStartupInfo
{
public:
	                 CResourceStartupInfo();

	// set actions
	void             SystemCardsModeInd(eSystemCardsMode system_cards_mode);
	void             LicensingInd(DWORD license_num_parties, eProductType license_product_type, bool is_Federal, bool isHD_enable, bool isRPPLicense);
	void             MultipleServicesInd(bool is_multiple_ip_services);
	void             IPServiceConfigReqInd(WORD service_id, char* service_name);
	void             IPServicesParamsEndInd();
	void             MfaCardConfigReq(BoardID boardId, DWORD cardType);
	void             MfaCardStartupComplete(BoardID boardId);

	//get actions
	BOOL             Is1500qEnableHD()   { return m_license_1500q_isHD_enable; }
	BOOL             IsLicenseReceived() { return m_received_licensing; }
	bool             isRPPLicense()      { return m_isRPP_license; }

	DWORD            GetCardType(BoardID boardId);

	bool             CanUpdateIpServicesDongleRestrictions() const;

protected:
	bool             m_received_cards_mode;
	bool             m_received_licensing;
	bool             m_received_multiple_ip_services;
	bool             m_received_ip_services_params_end;
	bool             m_license_is_Federal;
	bool             m_license_1500q_isHD_enable;
	bool             m_is_multiple_ip_services;
	bool             m_isRPP_license;
	WORD             m_received_ip_service_params;
	WORD             m_received_card_config_req;
	WORD             m_received_card_startup_complete;
	DWORD            m_license_num_parties;
	eSystemCardsMode m_system_cards_mode;
	eProductType     m_license_product_type;
	sIpServiceInfo   ipServicesInfo[MAX_NUM_OF_IP_SERVICES];
	sMfaCardInfo     mfaCardsInfo[BOARDS_NUM];

	friend std::ostream& operator <<(std::ostream& os, CResourceStartupInfo& params);
};

#endif
