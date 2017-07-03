#ifndef RESOURCEMANAGERHELPER_H_
#define RESOURCEMANAGERHELPER_H_

#include "DataTypes.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "AllocateStructs.h"
#include "SharedMcmsCardsStructs.h"
#include "TBStructs.h"

std::ostream& operator<<(std::ostream& os, ALLOC_PARTY_IND_PARAMS_S& params);
std::ostream& operator<<(std::ostream& os, DEALLOC_PARTY_IND_PARAMS_S& params);
std::ostream& operator<<(std::ostream& os, BOARD_FULL_REQ_PARAMS_S& params);
std::ostream& operator<<(std::ostream& os, RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S& params);
std::ostream& operator<<(std::ostream& os, CM_UNITS_CONFIG_S& params);
std::ostream& operator<<(std::ostream& os, PARTY_MOVE_RSRC_REQ_PARAMS_S& params);
std::ostream& operator<<(std::ostream& os, PARTY_MOVE_RSRC_IND_PARAMS_S& params);
std::ostream& operator<<(std::ostream& os, IP_SERVICE_UDP_RESOURCES_S& params);
std::ostream& operator<<(std::ostream& os, ACK_IND_S& params);
std::ostream& operator<<(std::ostream& os, SPAN_ENABLED_S& params);
std::ostream& operator<<(std::ostream& os, SPAN_DISABLE_S& params);
std::ostream& operator<<(std::ostream& os, MR_MONITOR_NUMERIC_ID_LIST_S& params);
std::ostream& operator<<(std::ostream& os, PROFILE_IND_S& params);
std::ostream& operator<<(std::ostream& os, MR_MONITOR_NUMERIC_ID_LIST_S& params);
std::ostream& operator<<(std::ostream& os, MR_IND_LIST_S& params);
std::ostream& operator<<(std::ostream& os, PROFILE_IND_S& params);
std::ostream& operator<<(std::ostream& os, PROFILE_IND_LIST_S& params);
std::ostream& operator<<(std::ostream& os, RTM_ISDN_PARAMS_MCMS_S& params);
std::ostream& operator<<(std::ostream& os, RTM_ISDN_SERVICE_CANCEL_S& params);
std::ostream& operator<<(std::ostream& os, RTM_ISDN_PHONE_RANGE_UPDATE_S& params);
std::ostream& operator<<(std::ostream& os, DEALLOCATE_BONDING_TEMP_PHONE_S& params);
std::ostream& operator<<(std::ostream& os, UPDATE_ISDN_PORT_S& params);
std::ostream& operator<<(std::ostream& os, ACK_UPDATE_ISDN_PORT_S& params);
std::ostream& operator<<(std::ostream& os, CONF_RSRC_REQ_PARAMS_S& params);
std::ostream& operator<<(std::ostream& os, CONF_RSRC_IND_PARAMS_S& params);
std::ostream& operator<<(std::ostream& os, UNIT_RECONFIG_S& params);
std::ostream& operator<<(std::ostream& os, SLOTS_NUMBERING_CONVERSION_TABLE_S& params);
std::ostream& operator<<(std::ostream& os, CARD_REMOVED_IND_S& params);
std::ostream& operator<<(std::ostream& os, HW_REMOVED_PARTY_LIST_S& params);
std::ostream& operator<<(std::ostream& os, CONF_PARTY_LIST_S& params);
std::ostream& operator<<(std::ostream& os, UNIT_RECOVERY_S& param);
std::ostream& operator<<(std::ostream& os, RECOVERY_REPLACEMENT_UNIT_S& param);
std::ostream& operator<<(std::ostream& os, PREFERRED_NUMERIC_ID_S& param);

#endif /* RESOURCEMANAGERHELPER_H_ */
