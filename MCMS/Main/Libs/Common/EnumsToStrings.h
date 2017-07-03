#if !defined(_ENUMSTOSTRINGS_H__)
#define _ENUMSTOSTRINGS_H__

#include "ConfPartyApiDefines.h"
#include "IpCsContentRoleToken.h"
#include "IpChannelParams.h"
#include "RvCommonDefs.h"

char* IpTypeToString(APIU32 ipType, bool caps = false);
const char* ConfMediaTypeToString(eConfMediaType confMediaType);
const char* RoleLabelToString(ERoleLabel roleLabel);
const char* CapEnumToString(CapEnum capEnumNum);
const char* ChanneltypeToString(kChanneltype channelType);
const char* CapDataTypeToString(cmCapDataType dataType);
const char* CapDirectionToString(cmCapDirection direction);
const char* AllocationPolicyToString(APIU32 allocationPolicy);
const char* TelePresenceLayoutModeToString(ETelePresenceLayoutMode telePresenceLayoutMode);

#endif

