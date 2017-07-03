#include <string.h>
#include "ProductType.h"
#include "DataTypes.h"

#define RMX2000        "RMX2000"
#define RMX4000        "RMX4000"
#define RMX1500        "RMX1500"
#define NPG2000        "NPG2000"
#define CALL_GENERATOR "CALL_GENERATOR"
#define SOFT_MCU       "SOFT_MCU"
#define GESHER         "GESHER"
#define NINJA          "NINJA"
#define SOFT_MCU_MFW   "SOFT_MCU_MFW"
#define SOFT_MCU_EDGE  "SOFT_MCU_EDGE"
#define SOFT_MCU_CG    "SOFT_MCU_CG"
#define UNKNWON        "UNKNOWN"


#define MGC_FAMILY "MGC"
#define RMX_FAMILY "RMX"
#define NPG_FAMILY "NPG"
#define CALL_GEN_FAMILY "CG"
#define SOFT_MCU_FAMILY "S-MCU"

// ========= eProductType =========
const char* ProductTypeToString(eProductType productType)
{
    switch (productType)
    {
        case eProductTypeRMX2000:
            return RMX2000;

      
        case eProductTypeRMX4000:
            return RMX4000;

        case eProductTypeRMX1500:
             return RMX1500;
      
        case eProductTypeNPG2000:
            return NPG2000;

        case eProductTypeCallGenerator:
            return CALL_GENERATOR;
            
        case eProductTypeSoftMCU:
            return SOFT_MCU;

        case eProductTypeGesher:
            return GESHER;
			
        case eProductTypeNinja:
            return NINJA;
			
        case eProductTypeSoftMCUMfw:
        	return SOFT_MCU_MFW;

        case eProductTypeEdgeAxis:
        	return SOFT_MCU_EDGE;

        case eProductTypeCallGeneratorSoftMCU:
        	return SOFT_MCU_CG;

        default:
            return UNKNWON;

    }
    
    return NULL;
    
}

//--------------------------------------------------------------------------
const char* ProductTypeToRPCSString(eProductType type)
{
	switch(type)
	{
	case eProductTypeRMX2000:
		return "RPCS 2000";

	case eProductTypeRMX4000:
		return "RPCS 4000";

	case eProductTypeRMX1500:
		return "RPCS 1500";

	case eProductTypeNinja:
		return "RPCS 1800";


	case eProductTypeEdgeAxis:
	case eProductTypeGesher:
	case eProductTypeSoftMCU:
		return "RPCS 800s VE";

	case eProductTypeSoftMCUMfw:
		return "RPCS MFW";



	default :
		return "";
	}
}

const char* ProductFamilyToString(eProductFamily productFamily)
{
    switch(productFamily)
    {
        case eProductFamilyMGC:
            return MGC_FAMILY;

        case eProductFamilyRMX:
            return RMX_FAMILY;

        case eProductFamilyNPG:
            return NPG_FAMILY;

        case eProductFamilyCallGenerator:
            return CALL_GEN_FAMILY;

        case eProductFamilySoftMcu:
            return SOFT_MCU_FAMILY;


        default:
            return UNKNWON;
            
    };

    return NULL;
}


// ========= eProductType =========
eProductType StringToProductType(const char * pn_string)
{
    if (pn_string == NULL)
        return eProductTypeUnknown;
    
    if (strcmp(pn_string,RMX2000) == 0)
        return eProductTypeRMX2000;

    if (strcmp(pn_string,RMX4000) == 0)
        return eProductTypeRMX4000;

    if (strcmp(pn_string,RMX1500) == 0)
        return eProductTypeRMX1500;

    if (strcmp(pn_string,NPG2000) == 0)
        return eProductTypeNPG2000;

    if (strcmp(pn_string,CALL_GENERATOR) == 0)
        return eProductTypeCallGenerator;

    if (strcmp(pn_string,SOFT_MCU) == 0)
        return eProductTypeSoftMCU;

    if (strcmp(pn_string,GESHER) == 0)
        return eProductTypeGesher;

    if (strcmp(pn_string,NINJA) == 0)
        return eProductTypeNinja;

    if (strcmp(pn_string,SOFT_MCU_MFW) == 0)
        return eProductTypeSoftMCUMfw;

    if (strcmp(pn_string,SOFT_MCU_EDGE) == 0)
        return eProductTypeEdgeAxis;

    if (strcmp(pn_string,SOFT_MCU_CG) == 0)
            return eProductTypeCallGeneratorSoftMCU;

    return eProductTypeUnknown;
    
}


// ========= eProductType =========
eProductFamily ProductTypeToProductFamily(eProductType productType)
{
    switch (productType)
    {
        case eProductTypeRMX2000:
        case eProductTypeRMX4000:
        case eProductTypeRMX1500:
            return eProductFamilyRMX;
            
        case eProductTypeNPG2000:
            return eProductFamilyNPG;
            
        case eProductTypeCallGenerator:
            return eProductFamilyCallGenerator;

        case eProductTypeSoftMCU:
        case eProductTypeSoftMCUMfw:
        case eProductTypeGesher:
        case eProductTypeNinja:
        case eProductTypeEdgeAxis:
        case eProductTypeCallGeneratorSoftMCU:
            return eProductFamilySoftMcu;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
    }
    
    return eProductFamilyUnknown;
  
    
}
