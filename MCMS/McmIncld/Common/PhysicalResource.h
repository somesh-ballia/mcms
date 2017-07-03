#ifndef PHYSICALRESOURCE_H_
#define PHYSICALRESOURCE_H_

#include "DataTypes.h"



/*************************************************************************/
typedef struct
{
	APIU8   box_id;
	APIU8   board_id;
	APIU8   sub_board_id;
	APIU8   unit_id;

} PHYSICAL_UNIT_PARAMS_S;
/*************************************************************************/

typedef struct
{
	PHYSICAL_UNIT_PARAMS_S physical_unit_params;
	APIU8   accelerator_id;
	APIU16  port_id;
	APIU8   resource_type; // look at eResourceTypes
	APIU8   future_use1; // in use for ART channel id in ISDN
	APIU8   future_use2; // in use for ART channel id in ISDN
	APIU8   reserved[2];

} PHYSICAL_RESOURCE_INFO_S;
/*************************************************************************/

typedef struct
{
	APIU32						connection_id;
	APIU32						party_id;
	PHYSICAL_RESOURCE_INFO_S    physical_id;
	
} MCMS_MPL_PHYSICAL_RESOURCE_INFO_S;
/*************************************************************************/



#endif /*PHYSICALRESOURCE_H_*/

