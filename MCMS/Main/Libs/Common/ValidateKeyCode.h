#ifndef __VALIDATE_KEYCODE_H__
#define __VALIDATE_KEYCODE_H__

#include "StatusesGeneral.h"
#include "ProductType.h"
#include <assert.h>

namespace keycode
{
	enum bits
	{
		MULTIPLE_SERVICES_BIT = 19,
		MPMX_CARD_BIT         = 23,
		SVC_OPTION_BIT        = 24,
		PORTS_UNIT_BIT        = 25,
	};
}


enum
{
	PORTS_UNIT_BIT = 25
	, RESERVED_BIT_BEGIN = 25
	, RESERVED_BIT_END = 32
	, SVC_OPTION_BIT = 24
	, SVC_PORT_BIT_BEGIN = 8
	, SVC_PORT_BIT_END = 16
	, AVC_PORT_BIT_BEGIN = 0
	, AVC_PORT_BIT_END = 8
	, PARTNERS_BIT_BEGIN = 20
	, PARTNERS_BIT_END = 24
	, FEATURES_BIT_BEGIN = 16
	, FEATURES_BIT_END = 20

	, GESHER_MAX_CP = 6 //30 ports
	, NINJA_MAX_CP = 20
};

inline int IsBitOn(unsigned mask, unsigned bit)
{
	return 0!=(mask & (1<<bit));
}

inline int BitRangeToNumber(unsigned mask, unsigned startBit, unsigned endBit)
{
	assert(endBit>startBit);
	return ((mask>>startBit) & ((1<<(endBit-startBit)) - 1));
}

int ValidateKeyCode(eProductType productType, char const * keyCode, int& failReason,int mcuVerMajor = -1,  int mcuVerMinor = -1);

#endif

