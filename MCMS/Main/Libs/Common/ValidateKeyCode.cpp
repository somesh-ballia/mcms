#include "ValidateKeyCode.h"
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "ApiStatuses.h"

extern void ExtractOptionStringFromKeyCode(char option_mask[], char const* key_code);
namespace
{
 void substring(char str[], char src[], int start, int end)
 {
	 int size = 0;
	 int pos = 0;
	 int count = 0;
	 int i;

	 if (str == NULL)
		 return;

	size = strlen(src);
	if (size < end || size < start)
		return;


	for ( i = 0; i < size; i++)
	{
		if (i >= start && i <= end)
		{
			str[count] = src[i];
			count++;
		}
	}
	str[count] = '\0';
 }
}
void ExtractOptionStringFromKeyCode(char option_mask[], char const* key_code)
{
	char str[30] = {0};
	char tmp[30] = {0};
	int size = 0;
    int pos = 0;
	int i;

   // Remove format characters to get option mask
   size = (int)strlen(key_code);

   for ( i = 0; i < size; i++)
   {
	   if (key_code[i] != '-')
	   {
			str[pos] = key_code[i];
			pos++;
	   }
   }
   // Ensure that data has at least valid options mask size of 8 to do the parsing
   size = (int)strlen(str);
   if ((int)strlen(str) > 8)
   {
		substring(tmp, str, size-8, size);     
		strcpy(option_mask, tmp);
   }
}

inline int IsHexString(char const * str)
{
	int const len = strlen(str);
	for (int i=0; i<len; ++i)
	{
		if (!isxdigit(str[i]))
		{
			return false;
		}
	}
	return true;
}

int ValidateKeyCode(eProductType productType, char const * keyCode, int& failReason,int mcuVerMajor/* = -1*/ , int mcuVerMinor /*= -1*/)
{
	failReason = 0;

	switch (productType)
	{
		case eProductTypeGesher:
		case eProductTypeNinja:
		case eProductTypeEdgeAxis:
		case eProductTypeRMX2000:
		case eProductTypeRMX4000:
		case eProductTypeRMX1500:
		case eProductTypeCallGeneratorSoftMCU:
			break;
		default:
			return STATUS_OK;
	}

    char szOptions[32];
    bzero(szOptions, sizeof(szOptions));
    ExtractOptionStringFromKeyCode(szOptions, keyCode);
    if ((8!=strlen(szOptions)) || (!IsHexString(szOptions)))
    {
    	failReason = 1;
        return STATUS_CFS_MSG_1;
    }

    unsigned long options = strtoul(szOptions, NULL, 16);
    unsigned long reserved = BitRangeToNumber(options, RESERVED_BIT_BEGIN, RESERVED_BIT_END); (void)reserved;
#ifdef STRICT_CHECK
    if (0!=reserved)
    {
    	failReason = 2;
        return STATUS_CFS_MSG_1;
    }
#endif

    if (0)//!IsBitOn(options, GESHER_BIT))
    {
    	failReason = 3;
        return STATUS_CFS_MSG_1;
    }

#ifdef STRICT_CHECK
    int const isSvcOn = IsBitOn(options, SVC_OPTION_BIT);
    if (!isSvcOn)
    {
    	failReason = 4;
        return STATUS_CFS_MSG_1;
    }
#endif


	int const isUnitBitOn = IsBitOn(options, PORTS_UNIT_BIT);
	if (mcuVerMajor > 7  || mcuVerMajor == -1 ) //BRIDGE-8001
	{
		if (!isUnitBitOn)
		{
			failReason = 12;
			return STATUS_CFS_MSG_1;
		}
	}

	if (eProductTypeRMX1500 == productType || eProductTypeRMX2000 == productType || eProductTypeRMX4000 == productType || 
            eProductTypeNinja == productType || eProductTypeGesher == productType || eProductTypeEdgeAxis == productType )
	{
		int const isMpmxBitOn = IsBitOn(options, keycode::MPMX_CARD_BIT);
		if (!isMpmxBitOn)
		{
			failReason = 5;
			return STATUS_CFS_MSG_1;
		}
	}

    int const copPortCount = BitRangeToNumber(options, SVC_PORT_BIT_BEGIN, SVC_PORT_BIT_END);
    int const avcPortCount = BitRangeToNumber(options, AVC_PORT_BIT_BEGIN, AVC_PORT_BIT_END);

	if ( (eProductTypeNinja == productType) || (eProductTypeGesher == productType))
	{
		//For Ninja and Gesher, COP Portnumber must be 0 and AVC Port number must not be 0
		if (avcPortCount <= 0)
		{
			failReason = 13;
		    return STATUS_CFS_MSG_1;
		}

		if (copPortCount != 0)
		{
			failReason = 14;
		    return STATUS_CFS_MSG_1;
		}
	}

	// valid license when or cop ports num is positive or avc ports num is positive
    if ( avcPortCount>0  &&  copPortCount>0 && eProductTypeCallGeneratorSoftMCU != productType)
    {
    	failReason = 6;
        return STATUS_CFS_MSG_1;
    }
    if ( avcPortCount<=0  &&  copPortCount<=0 )
    {
    	failReason = 7;
        return STATUS_CFS_MSG_1;
    }

    if (eProductTypeGesher==productType)
    {
        if (avcPortCount>GESHER_MAX_CP)
        {
        	failReason = 8;
            return STATUS_CFS_MSG_1;
        }
    }
    else if (eProductTypeNinja==productType)
    {
        if (avcPortCount>NINJA_MAX_CP)
        {
        	failReason = 9;
            return STATUS_CFS_MSG_1;
        }
    }
    else if (eProductTypeEdgeAxis==productType)
    {
		if( !IsBitOn(options,keycode::PORTS_UNIT_BIT) )
		{
	    	failReason = 10;
			return STATUS_CFS_MSG_1;
		}

		if( !IsBitOn(options,keycode::MPMX_CARD_BIT) )
		{
	    	failReason = 11;
			return STATUS_CFS_MSG_1;
		}
    }

    return STATUS_OK;
}

