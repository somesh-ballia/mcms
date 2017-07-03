#include "ResRsrcCalculator.h"
#include "H221.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "ConfPartyApiDefines.h"
#include "TraceStream.h"
#include "PrettyTable.h"

extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);

//////////////////////////////////////////////////////////////////////////////////////////////////


// Resource capacity according to a system/card mode
// it multiplied by 100 because of the problems of transfer double numbers to EMA

#define sd_rsrc_units_mpm_x  	150
#define sd_rsrc_units_mpm_x20  	178

#define hd720_rsrc_units_mpm_x  	300
#define hd720_rsrc_units_mpm_x20  	357

#define hd1080_rsrc_units_mpm_x  	600
#define hd1080_rsrc_units_mpm_x20  	833
#define hd1080p60_rsrc_units_mpm_x  	900
#define hd1080p60_rsrc_units_mpm_x20  	1000

//////////////////////////////////////////////////////////////////////////////////////////////////
static const char* eVideoModeTypeNames[] =
{
	"eInvalidModeType",
	"eCIF30",
	"eCIF60",
	"e2CIF30",
	"eWCIF60",
	"eSD15",
	"eSD30",
	"eW4CIF30",
	"eSD60",
	"eHD720Asymmetric",
	"eHD720Symmetric",
	"eHD720At60Asymmetric",
	"eHD720At60Symmetric",
	"eHD1080Asymmetric",
	"eHD1080Symmetric",
	"eHD1080At60Asymmetric",
	"eHD1080At60Symmetric",
	"eLasth264VideoMode"
};

static const char* eResolutionConfigTypeNames[] = {
	"balanced",
	"resource_optimized",
	"user_exp_optimized",
	"high_profile_optimized",
	"manual"
};

/////////////////////////////////////////////////////////////////////////////////
static DWORD ConvertEThresholdBitRate(EThresholdBitRate erate)
{
	switch(erate)
	{
		case e64000: return 64000;
		case e96000: return 96000;
		case e128000: return 128000;
		case e192000: return 192000;
		case e256000: return 256000;
		case e320000: return 320000;
		case e384000: return 384000;
		case e512000: return 512000;
		case e768000: return 768000;
		case e832000: return 832000;
		case e1024000: return 1024000;
		case e1152000: return 1152000;
		case e1280000: return 1280000;
		case e1472000: return 1472000;
		case e1536000: return 1536000;
		case e1728000: return 1728000;
		case e1920000: return 1920000;
		case e2048000: return 2048000;
		case e2560000: return 2560000;
		case e3072000: return 3072000;
		case e3584000: return 3584000;
		case e4096000: return 4096000;
		case e6144000: return 6144000;
		case e8192000: return 8192000;
		default: break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
static EThresholdBitRate ConvertToEThresholdBitRate(DWORD rate)
{
	switch(rate)
	{
		case 64000: return e64000;
		case 96000: return e96000;
		case 128000: return e128000;
		case 192000: return e192000;
		case 256000: return e256000;
		case 320000: return e320000;
		case 384000: return e384000;
		case 512000: return e512000;
		case 768000: return e768000;
		case 832000: return e832000;
		case 1024000: return e1024000;
		case 1152000: return e1152000;
		case 1280000: return e1280000;
		case 1472000: return e1472000;
		case 1536000: return e1536000;
		case 1728000: return e1728000;
		case 1920000: return e1920000;
		case 2048000: return e2048000;
		case 2560000: return e2560000;
		case 3072000: return e3072000;
		case 3584000: return e3584000;
		case 4096000: return e4096000;
		case 6144000: return e6144000;
		case 8192000: return e8192000;
	}
	return eMAX_NUM_THRESHOLD_RATES;
}

/////////////////////////////////////////////////////////////////////////////////
static Eh264VideoModeType ConvertEResolutionTypeToEh264VideoModeType(eSystemCardsMode systemCardsBasedMode, EResolutionType resType)
{
	eProductType productType = CProcessBase::GetProcess()->GetProductType();

	switch (resType)
	{
		case e_cif30:
			return eCIF30;
		case e_cif60:
			return eCIF60;
		case e_wcif:
			return eCIF30; // eWCIF60:
		case e_sd15:
			return eSD15;
		case e_sd30:
			return eSD30; //	eW4CIF30:
		case e_sd60:
			return eSD60;
		case e_hd720p30:
			return eHD720Symmetric;
		case e_hd720p60:
			return eHD720At60Symmetric;
		case e_hd1080p30:
			return eHD1080Symmetric;
		case e_hd1080p60:
			if (systemCardsBasedMode == eSystemCardsMode_breeze && eProductFamilyRMX == ::ProductTypeToProductFamily(productType))
				return eHD1080At60Asymmetric;
			else
				return eHD1080At60Symmetric;

		default:
		{
			DBGFPASSERT(resType);
			FTRACESTR(eLevelError) << " ConvertEResolutionTypeToEh264VideoModeType : unknown resType = " << resType;
		}
	}
	return eHD1080Symmetric;
}

/////////////////////////////////////////////////////////////////////////////////
static EVideoResolutionType ConvertEResolutionTypeToEVideoResolutionType(eSystemCardsMode systemCardsBasedMode, EResolutionType resType)
{
	switch (resType)
	{
		case e_cif30:
		case e_cif60:
		case e_wcif:
			return eCIF_Res;
		case e_sd15:
		case e_sd30:
		case e_sd60:
			return eSD_Res;
		case e_hd720p30:
		case e_hd720p60:
			return eHD720_Res;
		case e_hd1080p30:
			return eHD1080_Res;
		case e_hd1080p60:
			return eHD1080p60_Res;
		default:
			return eAuto_Res;
	}
}

/////////////////////////////////////////////////////////////////////////////////
static DWORD TranslateRsrvTransferRate(BYTE XferRate)
{
	switch(XferRate)
	{
		case Xfer_64:   return   64000;
		case Xfer_96:   return   96000;
		case Xfer_128:
		case Xfer_2x64:
			return  128000;
		case Xfer_192:
		case Xfer_3x64:
			return  192000;
		case Xfer_256:
		case Xfer_4x64:
			  return  256000;
		case Xfer_320:
		case Xfer_5x64:
			 return  320000;
		case Xfer_384:
		case Xfer_6x64:
			return  384000;
		case Xfer_512:  return  512000;
		case Xfer_768:  return  768000;
		case Xfer_832:  return  832000;
		case Xfer_1024: return 1024000;
		case Xfer_1280: return 1280000;
		case Xfer_1152: return 1152000;
		case Xfer_1472: return 1472000;
		case Xfer_1536: return 1536000;
		case Xfer_1728: return 1728000;
		case Xfer_1920: return 1920000;
		case Xfer_2048: return 2048000;
		case Xfer_2560: return 2560000;
		case Xfer_3072: return 3072000;
		case Xfer_3584: return 3584000;
		case Xfer_4096: return 4096000;
		case Xfer_6144: return 6144000;
		case Xfer_8192: return 8192000;

		default:
		{
			DBGFPASSERT(XferRate);
			FTRACEINTO << " TranslateRsrvTransferRate : unknown XferRate = " << XferRate;
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------- Dynamic Decision Matrix--------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////

EResolutionType CResRsrcCalculator::m_CPmaxRes = e_hd1080p60;
EThresholdBitRate  CResRsrcCalculator::m_CPmaxLineRate = e4096000;
EResolutionConfigType CResRsrcCalculator::m_ConfigType = e_balanced;

BOOL CResRsrcCalculator::m_isHD_enabled = TRUE;// Disable HD in 1500Q
BOOL CResRsrcCalculator::m_isHD1080_enabled = TRUE;// Disable HD in SoftMCU

BOOL CResRsrcCalculator::m_isRMX1500Q = FALSE;
BOOL CResRsrcCalculator::m_isRMX1500QRatios = FALSE;
BOOL CResRsrcCalculator::m_CPLicensing_CIF_Plus = FALSE;


///MUST BE SORTED BY thresholdBitrate
H264VideoModeThresholdStruct CResRsrcCalculator::g_dynamicMotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_MOTION] =
{
{ //MPM
	 {  64000,                eCIF30},
	 { 256000,                e2CIF30},
	 { 1024000,               eSD30},
	 { 1920000,               eHD720Asymmetric},
	 { 1920000,               eHD720Asymmetric}
},
{ //MPM+
	 {  64000,                eCIF30},
	 { 384000,                eWCIF60},
	 { 1024000,               eSD60},
	 { 1920000,               eHD720At60Asymmetric},
	 { 1920000,               eHD720At60Asymmetric}
},
{ //MPMx
	 {  64000,                eCIF30},
	 { 384000,                eWCIF60},
	 { 1024000,               eSD60},
	 { 1920000,               eHD720At60Symmetric},
	 { 3584000,               eHD1080At60Asymmetric}
},
{ //MPM-Rx
	 {  64000,                eCIF30},
	 { 384000,                eWCIF60},
	 { 1024000,               eSD60},
	 { 1920000,               eHD720At60Symmetric},
	 { 3584000,               eHD1080At60Symmetric}
}
};

///MUST BE SORTED BY thresholdBitrate
H264VideoModeThresholdStruct CResRsrcCalculator::g_dynamicSharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_SHARPNESS] =
{
{ //MPM
	 { 64000,                 eCIF30},
	 { 256000,                eSD15},
	 { 512000,                eSD30},
	 { 1024000,               eHD720Asymmetric},
	 { 1024000,               eHD720Asymmetric}
},
{ //MPM+
	 { 64000,                 eCIF30},
	 { 256000,                eSD30},
	 { 1024000,               eHD720Symmetric},
	 { 4096000,               eHD1080Asymmetric},
	 { 4096000,               eHD1080Asymmetric}
},
{ //MPMx
	 { 64000,                 eCIF30},
	 { 256000,                eSD30},
	 { 1024000,               eHD720Symmetric},
	 { 2048000,               eHD1080Symmetric},
	 { 2048000,               eHD1080Symmetric}
},
{ //MPM-Rx
	 { 64000,                 eCIF30},
	 { 256000,                eSD30},
	 { 1024000,               eHD720Symmetric},
	 { 2048000,               eHD1080Symmetric},
	 { 4096000,               eHD1080At60Symmetric}
}
};

H264VideoModeThresholdStruct CResRsrcCalculator::g_dynamicHighProfileMotionThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_MOTION] =
{
{ //MPM
	 { 64000,                 eCIF30},
	 { 128000,                e2CIF30},
	 { 512000,                eSD30},
	 { 832000,                eHD720Asymmetric},
	 { 832000,                eHD720Asymmetric}
},
{ //MPM+
	 { 64000,                 eCIF30},
	 { 128000,                eWCIF60},
	 { 512000,                eSD60},
	 { 832000,                eHD720At60Asymmetric},
	 { 832000,                eHD720At60Asymmetric}
},
{ //MPMx
	 { 64000,                 eCIF30},
	 { 256000,                eWCIF60},
	 { 768000,                eSD60},
	 { 1280000,               eHD720At60Symmetric},
	 { 2048000,               eHD1080At60Asymmetric}
},
{//MPM-Rx
	 {  64000,                eCIF30},
	 { 256000,                eWCIF60},
	 { 768000,                eSD60},
	 { 1280000,               eHD720At60Symmetric},
	 { 2048000,               eHD1080At60Symmetric} // Tsahi TODO: for MPMx should be eHD1080At60Asymmetric, check with Ron
}
};

H264VideoModeThresholdStruct CResRsrcCalculator::g_dynamicHighProfileSharpnessThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_SHARPNESS] =
{
{ //MPM
	 { 64000,                 eCIF30},
	 { 128000,                eSD15},
	 { 512000,                eSD30},
	 { 1024000,               eHD720Asymmetric},
	 { 1024000,               eHD720Asymmetric}
},
{ //MPM+
	 { 64000,                 eCIF30},
	 { 128000,                eSD30},
	 { 512000,                eHD720Symmetric},
	 { 1024000,               eHD1080Asymmetric},
	 { 1024000,               eHD1080Asymmetric}
},
{ //MPMx
	 { 64000,                 eCIF30},
	 { 256000,                eSD30},
	 { 832000,                eHD720Symmetric},
	 { 1536000,               eHD1080Symmetric},
	 { 1536000,               eHD1080Symmetric}
},
{//MPM-Rx
	 { 64000,                 eCIF30},
	 { 256000,                eSD30},
	 { 832000,                eHD720Symmetric},
	 { 1536000,               eHD1080Symmetric},
	 { 2048000,               eHD1080At60Symmetric}
}
};
/////////////////////////////////////////////////////////////////////////////////////////////////////
ResolutionThresholdStruct CResRsrcCalculator::g_defaultMotionThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_MOTION] =
{
{ //MPM                balanced_bitrate    resource_optimized   user_optimized  hi-profile
	 {  eCIF30, 			     e64000,    	e64000,  		   e64000,		   e64000  },
	 {  e2CIF30, 			    e256000, 	   e384000,   		  e256000,		  e128000  },
	 {  eSD30,  			   e1024000, 	  e1024000,  		  e768000,		  e512000  },
	 {  eHD720Asymmetric, 	   e1920000, 	  e1920000, 		 e1280000,		  e832000  },
	 {  eHD720Asymmetric, 	   e1920000, 	  e1920000, 		 e1280000,		  e832000  }
},
{ //MPM+
	 { eCIF30, 	    		      e64000,    	e64000,		     	e64000,		   e64000 },
	 { eWCIF60,				     e384000,      e384000,		   	   e256000,	  	  e128000 },
	 { eSD60, 				    e1024000,  	  e1024000, 		   e768000, 	  e512000 },
	 { eHD720At60Asymmetric,    e1920000,  	  e1920000, 		  e1280000,  	  e832000 },
	 { eHD720At60Asymmetric,    e1920000,  	  e1920000, 		  e1280000,  	  e832000 }
},
{ //MPMx
	 { eCIF30, 			          e64000,   	 e64000,     		e64000,   		e64000 },
	 { eWCIF60,				     e384000,  	 	e384000,		   e256000,  	   e128000 },
	 { eSD60, 				    e1024000,  	   e1024000,    	   e768000,  	   e512000 },
	 { eHD720At60Symmetric,     e1920000,  	   e1920000,   		  e1280000,  	   e832000 },
	 { eHD1080At60Asymmetric,   e3584000,  	   e4096000,   		  e3072000,  	  e1728000 }
},
{ //MPM-Rx
	 { eCIF30, 			          e64000,   	 e64000,     		e64000,   		e64000 },
	 { eWCIF60,				     e384000,  	 	e384000,		   e256000,  	   e128000 },
	 { eSD60, 				    e1024000,  	   e1024000,    	   e768000,  	   e512000 },
	 { eHD720At60Symmetric,     e1920000,  	   e1920000,   		  e1280000,  	   e832000 },
	 { eHD1080At60Symmetric,    e3584000,  	   e4096000,   		  e3072000,  	  e1728000 }
}
};

ResolutionThresholdStruct CResRsrcCalculator::g_defaultSharpnessThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_SHARPNESS] =
{
{ //MPM
	 { eCIF30,              e64000,       		e64000,  		   e64000,			e64000 },
	 { eSD15,  			   e256000, 	 	   e384000,			  e256000,		   e128000 },
	 { eSD30,			   e512000,		 	   e768000, 		  e384000, 		   e512000 },
	 { eHD720Asymmetric,  e1024000,			  e1920000,			 e1024000,		  e1024000 },
	 { eHD720Asymmetric,  e1024000,			  e1920000,			 e1024000,		  e1024000 }
},
{ //MPM+
	 { eCIF30,                e64000,		    e64000, 		    e64000, 	    e64000 },
	 { eSD30,				 e256000, 	 	   e384000,			   e256000,		   e128000 },
	 { eHD720Symmetric,		e1024000, 		  e1920000,			   e768000,		   e512000 },
	 { eHD1080Asymmetric,	e4096000,		  e4096000,			  e1728000,		  e1024000 },
	 { eHD1080Asymmetric,	e4096000,		  e4096000,			  e1728000,		  e1024000 }
},
{ //MPMx
	 { eCIF30,               	  e64000,		     e64000,		    e64000,		    e64000 },
	 { eSD30,                	 e256000,		    e384000,		   e256000,		   e128000 },
	 { eHD720Symmetric,      	e1024000,		   e1536000,		   e832000,		   e512000 },
	 { eHD1080Symmetric,     	e2048000,          e2560000,   	       e1728000,      e1728000 },
	 { eHD1080Symmetric,     	e2048000,          e2560000,   	       e1728000,      e1728000 }
},
{ //MPM-Rx
	 { eCIF30,              	  e64000,		     e64000,		    e64000,		    e64000 },
	 { eSD30,               	 e256000,		    e384000,		   e256000,		   e128000 },
	 { eHD720Symmetric,     	e1024000,		   e1536000,		   e832000,		   e512000 },
	 { eHD1080Symmetric,    	e2048000,		   e2560000,		  e1728000,		  e1728000 },
	 { eHD1080At60Symmetric,	e3584000,		   e4096000,		  e3584000,		  e4096000 }
}
};

ResolutionThresholdStruct CResRsrcCalculator::g_defaultHighProfileMotionThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_MOTION] =
{
{ //MPM                balanced_bitrate    resource_optimized   user_optimized  hi-profile
	 {  eCIF30, 			     e64000,    	e64000,  		   e64000,    	e64000 },
	 {  e2CIF30, 			    e128000, 	   e128000,   		  e128000, 	   e128000 },
	 {  eSD30,  			    e512000, 	   e512000,  		  e512000, 	   e512000 },
	 {  eHD720Asymmetric, 	    e832000, 	   e832000, 		  e832000, 	   e832000 },
	 {  eHD720Asymmetric, 	    e832000, 	   e832000, 		  e832000, 	   e832000 }
},
{ //MPM+
	 { eCIF30, 	    		      e64000,    	e64000,		     	e64000,    	e64000 },
	 { eWCIF60,				     e128000,      e128000,		   	   e128000,    e128000 },
	 { eSD60, 				     e512000,  	   e512000, 		   e512000,    e512000 },
	 { eHD720At60Asymmetric,     e832000,  	   e832000, 		   e832000,	   e832000 },
	 { eHD720At60Asymmetric,     e832000,  	   e832000, 		   e832000,	   e832000 }
},
{ //MPMx
	 { eCIF30, 			          e64000,   	 e64000,     		e64000,		 e64000 },
	 { eWCIF60,				     e256000,  	 	e384000,		   e256000,	    e128000 },
	 { eSD60, 				     e768000,  	   e1024000,    	   e512000,     e512000 },
	 { eHD720At60Symmetric,     e1280000,  	   e2560000,   		   e832000,     e832000 },
	 { eHD1080At60Asymmetric,   e2560000,  	   e3584000,   		  e1728000,    e1728000 }
},
{ //MPM-Rx
	 { eCIF30, 			          e64000,   	 e64000,     		e64000,		 e64000 },
	 { eWCIF60,				     e256000,  	 	e384000,		   e256000,	    e128000 },
	 { eSD60, 				     e768000,  	   e1024000,    	   e512000,     e512000 },
	 { eHD720At60Symmetric,     e1280000,  	   e2560000,   		   e832000,     e832000 },
	 { eHD1080At60Symmetric,    e2560000,  	   e3584000,   		  e1728000,    e1728000 }
}
};

ResolutionThresholdStruct CResRsrcCalculator::g_defaultHighProfileSharpnessThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][MAX_NUM_SLIDER_RATE_SHARPNESS] =
{
{ //MPM
	 { eCIF30,              e64000,       		e64000,  		   e64000,		    e64000 },
	 { eSD15,  			   e128000, 	 	   e128000,			  e128000, 	 	   e128000 },
	 { eSD30,			   e512000,		 	   e512000, 		  e512000, 		   e512000 },
	 { eHD720Asymmetric,  e1024000,			  e1024000,			 e1024000,		  e1024000 },
	 { eHD720Asymmetric,  e1024000,			  e1024000,			 e1024000,		  e1024000 }
},
{ //MPM+
	 { eCIF30,                e64000,		    e64000, 		    e64000,		    e64000 },
	 { eSD30,				 e128000, 	 	   e128000,			   e128000,	 	   e128000 },
	 { eHD720Symmetric,		 e512000, 		   e512000,			   e512000,		   e512000 },
	 { eHD1080Asymmetric,	e1024000,		  e1024000,			  e1024000,		  e1024000 },
	 { eHD1080Asymmetric,	e1024000,		  e1024000,			  e1024000,		  e1024000 }
},
{ //MPMx
	 { eCIF30,              	  e64000,		     e64000,		    e64000,		    e64000 },
	 { eSD30,               	 e256000,		    e384000,		   e256000,	 	   e128000 },
	 { eHD720Symmetric,     	 e832000,		   e1280000,		   e512000,		   e512000 },
	 { eHD1080Symmetric,    	e1536000,		   e2560000,		  e1024000,		  e1024000 },
	 { eHD1080Symmetric,      	e1536000,		   e2560000,		  e1024000,		  e1024000 }
},
{ //MPM-Rx
	 { eCIF30,               	  e64000,		     e64000,		    e64000,		    e64000 },
	 { eSD30,                	 e256000,		    e384000,		   e256000,	 	   e128000 },
	 { eHD720Symmetric,     	 e832000,		   e1280000,		   e512000,		   e512000 },
	 { eHD1080Symmetric,      	e1536000,		   e2560000,		  e1024000,		  e1024000 },
	 { eHD1080At60Symmetric,	e2560000,		   e3584000,		  e2048000,		  e2048000 }
}
};


////////////////////////////////////////////////////////////////////////////////////////////////////
//                                       CIF
////////////////////////////////////////////////////////////////////////////////////////////////////

H264VideoModeThresholdStruct CResRsrcCalculator::g_CIF_MotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES] =
{
{ //MPM
	{  64000,                eCIF30},
	{  96000,                eCIF30},
	{ 128000,                eCIF30},
	{ 192000,                eCIF30},
	{ 256000,                eCIF30},
	{ 320000,                eCIF30},
	{ 384000,                eCIF30},
	{ 512000,                eCIF30},
	{ 768000,                eCIF30},
	{ 832000,                eCIF30},
	{ 1024000,               eCIF30},
	{ 1152000,               eCIF30},
	{ 1280000,               eCIF30},
	{ 1472000,               eCIF30},
	{ 1536000,               eCIF30},
	{ 1728000,               eCIF30},
	{ 1920000,               eCIF30},
	{ 2048000,               eCIF30},
	{ 2560000,               eCIF30},
	{ 3072000,               eCIF30},
	{ 3584000,               eCIF30},
	{ 4096000,               eCIF30},
	{ 6144000,               eCIF30},
	{ 8192000,               eCIF30}
},
{ //MPM+
	{  64000,                eCIF30},
	{  96000,                eCIF30},
	{ 128000,                eCIF30},
	{ 192000,                eCIF30},
	{ 256000,                eCIF30}, //VNGFE-5723 change values from eCIF60 to eCIF30
	{ 320000,                eCIF30},
	{ 384000,                eCIF30},
	{ 512000,                eCIF30},
	{ 768000,                eCIF30},
	{ 832000,                eCIF30},
	{ 1024000,               eCIF30},
	{ 1152000,               eCIF30},
	{ 1280000,               eCIF30},
	{ 1472000,               eCIF30},
	{ 1536000,               eCIF30},
	{ 1728000,               eCIF30},
	{ 1920000,               eCIF30},
	{ 2048000,               eCIF30},
	{ 2560000,               eCIF30},
	{ 3072000,               eCIF30},
	{ 3584000,               eCIF30},
	{ 4096000,               eCIF30},
	{ 6144000,               eCIF30},
	{ 8192000,               eCIF30}
},
{ //MPMx
	{  64000,                eCIF30},
	{  96000,                eCIF30},
	{ 128000,                eCIF30},
	{ 192000,                eCIF30},
	{ 256000,                eCIF30},//VNGFE-5723 change values from eCIF60 to eCIF30
	{ 320000,                eCIF30},
	{ 384000,                eCIF30},
	{ 512000,                eCIF30},
	{ 768000,                eCIF30},
	{ 832000,                eCIF30},
	{ 1024000,               eCIF30},
	{ 1152000,               eCIF30},
	{ 1280000,               eCIF30},
	{ 1472000,               eCIF30},
	{ 1536000,               eCIF30},
	{ 1728000,               eCIF30},
	{ 1920000,               eCIF30},
	{ 2048000,               eCIF30},
	{ 2560000,               eCIF30},
	{ 3072000,               eCIF30},
	{ 3584000,               eCIF30},
	{ 4096000,               eCIF30},
	{ 6144000,               eCIF30},
	{ 8192000,               eCIF30}
},
{ //MPM-Rx
	{  64000,                eCIF30},
	{  96000,                eCIF30},
	{ 128000,                eCIF30},
	{ 192000,                eCIF30},
	{ 256000,                eCIF30},//VNGFE-5723 change values from eCIF60 to eCIF30
	{ 320000,                eCIF30},
	{ 384000,                eCIF30},
	{ 512000,                eCIF30},
	{ 768000,                eCIF30},
	{ 832000,                eCIF30},
	{ 1024000,               eCIF30},
	{ 1152000,               eCIF30},
	{ 1280000,               eCIF30},
	{ 1472000,               eCIF30},
	{ 1536000,               eCIF30},
	{ 1728000,               eCIF30},
	{ 1920000,               eCIF30},
	{ 2048000,               eCIF30},
	{ 2560000,               eCIF30},
	{ 3072000,               eCIF30},
	{ 3584000,               eCIF30},
	{ 4096000,               eCIF30},
	{ 6144000,               eCIF30},
	{ 8192000,               eCIF30}
}
};
////////////////////////////////////////////////////////////////////////////////////////////////////

H264VideoModeThresholdStruct CResRsrcCalculator::g_CIF_SharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES] =
{
{ //MPM
	{  64000,                eCIF30},
	{  96000,                eCIF30},
	{ 128000,                eCIF30},
	{ 192000,                eCIF30},
	{ 256000,                eCIF30},
	{ 320000,                eCIF30},
	{ 384000,                eCIF30},
	{ 512000,                eCIF30},
	{ 768000,                eCIF30},
	{ 832000,                eCIF30},
	{ 1024000,               eCIF30},
	{ 1152000,               eCIF30},
	{ 1280000,               eCIF30},
	{ 1472000,               eCIF30},
	{ 1536000,               eCIF30},
	{ 1728000,               eCIF30},
	{ 1920000,               eCIF30},
	{ 2048000,               eCIF30},
	{ 2560000,               eCIF30},
	{ 3072000,               eCIF30},
	{ 3584000,               eCIF30},
	{ 4096000,               eCIF30},
	{ 6144000,               eCIF30},
	{ 8192000,               eCIF30}
},
{ //MPM+
	{  64000,                eCIF30},
	{  96000,                eCIF30},
	{ 128000,                eCIF30},
	{ 192000,                eCIF30},
	{ 256000,                eCIF30},
	{ 320000,                eCIF30},
	{ 384000,                eCIF30},
	{ 512000,                eCIF30},
	{ 768000,                eCIF30},
	{ 832000,                eCIF30},
	{ 1024000,               eCIF30},
	{ 1152000,               eCIF30},
	{ 1280000,               eCIF30},
	{ 1472000,               eCIF30},
	{ 1536000,               eCIF30},
	{ 1728000,               eCIF30},
	{ 1920000,               eCIF30},
	{ 2048000,               eCIF30},
	{ 2560000,               eCIF30},
	{ 3072000,               eCIF30},
	{ 3584000,               eCIF30},
	{ 4096000,               eCIF30},
	{ 6144000,               eCIF30},
	{ 8192000,               eCIF30}
},
{ //MPMx
	{  64000,                eCIF30},
	{  96000,                eCIF30},
	{ 128000,                eCIF30},
	{ 192000,                eCIF30},
	{ 256000,                eCIF30},
	{ 320000,                eCIF30},
	{ 384000,                eCIF30},
	{ 512000,                eCIF30},
	{ 768000,                eCIF30},
	{ 832000,                eCIF30},
	{ 1024000,               eCIF30},
	{ 1152000,               eCIF30},
	{ 1280000,               eCIF30},
	{ 1472000,               eCIF30},
	{ 1536000,               eCIF30},
	{ 1728000,               eCIF30},
	{ 1920000,               eCIF30},
	{ 2048000,               eCIF30},
	{ 2560000,               eCIF30},
	{ 3072000,               eCIF30},
	{ 3584000,               eCIF30},
	{ 4096000,               eCIF30},
	{ 6144000,               eCIF30},
	{ 8192000,               eCIF30}
},
{ //MPM-Rx
	{  64000,                eCIF30},
	{  96000,                eCIF30},
	{ 128000,                eCIF30},
	{ 192000,                eCIF30},
	{ 256000,                eCIF30},
	{ 320000,                eCIF30},
	{ 384000,                eCIF30},
	{ 512000,                eCIF30},
	{ 768000,                eCIF30},
	{ 832000,                eCIF30},
	{ 1024000,               eCIF30},
	{ 1152000,               eCIF30},
	{ 1280000,               eCIF30},
	{ 1472000,               eCIF30},
	{ 1536000,               eCIF30},
	{ 1728000,               eCIF30},
	{ 1920000,               eCIF30},
	{ 2048000,               eCIF30},
	{ 2560000,               eCIF30},
	{ 3072000,               eCIF30},
	{ 3584000,               eCIF30},
	{ 4096000,               eCIF30},
	{ 6144000,               eCIF30},
	{ 8192000,               eCIF30}
}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                       SD
////////////////////////////////////////////////////////////////////////////////////////////////////

H264VideoModeThresholdStruct CResRsrcCalculator::g_SD_MotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES] =
{
{ //MPM
	{  64000,                e2CIF30},
	{  96000,                e2CIF30},
	{ 128000,                e2CIF30},
	{ 192000,                e2CIF30},
	{ 256000,                e2CIF30},
	{ 320000,                e2CIF30},
	{ 384000,                e2CIF30},
	{ 512000,                e2CIF30},
	{ 768000,                e2CIF30},
	{ 832000,                e2CIF30},
	{ 1024000,               e2CIF30},
	{ 1152000,               e2CIF30},
	{ 1280000,               e2CIF30},
	{ 1472000,               e2CIF30},
	{ 1536000,               e2CIF30},
	{ 1728000,               e2CIF30},
	{ 1920000,               e2CIF30},
	{ 2048000,               e2CIF30},
	{ 2560000,               e2CIF30},
	{ 3072000,               e2CIF30},
	{ 3584000,               e2CIF30},
	{ 4096000,               e2CIF30},
	{ 6144000,               e2CIF30},
	{ 8192000,               e2CIF30}
},
{ //MPM+
	{  64000,                eWCIF60},
	{  96000,                eWCIF60},
	{ 128000,                eWCIF60},
	{ 192000,                eWCIF60},
	{ 256000,                eWCIF60},
	{ 320000,                eWCIF60},
	{ 384000,                eWCIF60},
	{ 512000,                eWCIF60},
	{ 768000,                eWCIF60},
	{ 832000,                eWCIF60},
	{ 1024000,               eWCIF60}, //TODO: check if the same as WSD60
	{ 1152000,               eWCIF60},
	{ 1280000,               eWCIF60},
	{ 1472000,               eWCIF60},
	{ 1536000,               eWCIF60},
	{ 1728000,               eWCIF60},
	{ 1920000,               eWCIF60},
	{ 2048000,               eWCIF60},
	{ 2560000,               eWCIF60},
	{ 3072000,               eWCIF60},
	{ 3584000,               eWCIF60},
	{ 4096000,               eWCIF60},
	{ 6144000,               eWCIF60},
	{ 8192000,               eWCIF60}
},
{ //MPMx
	{  64000,                eWCIF60},
	{  96000,                eWCIF60},
	{ 128000,                eWCIF60},
	{ 192000,                eWCIF60},
	{ 256000,                eWCIF60},
	{ 320000,                eWCIF60},
	{ 384000,                eWCIF60},
	{ 512000,                eWCIF60},
	{ 768000,                eWCIF60},
	{ 832000,                eWCIF60},
	{ 1024000,               eWCIF60}, //TODO: check if the same as WSD60
	{ 1152000,               eWCIF60},
	{ 1280000,               eWCIF60},
	{ 1472000,               eWCIF60},
	{ 1536000,               eWCIF60},
	{ 1728000,               eWCIF60},
	{ 1920000,               eWCIF60},
	{ 2048000,               eWCIF60},
	{ 2560000,               eWCIF60},
	{ 3072000,               eWCIF60},
	{ 3584000,               eWCIF60},
	{ 4096000,               eWCIF60},
	{ 6144000,               eWCIF60},
	{ 8192000,               eWCIF60}
},
{ //MPM-Rx
	{  64000,                eWCIF60},
	{  96000,                eWCIF60},
	{ 128000,                eWCIF60},
	{ 192000,                eWCIF60},
	{ 256000,                eWCIF60},
	{ 320000,                eWCIF60},
	{ 384000,                eWCIF60},
	{ 512000,                eWCIF60},
	{ 768000,                eWCIF60},
	{ 832000,                eWCIF60},
	{ 1024000,               eWCIF60}, //TODO: check if the same as WSD60
	{ 1152000,               eWCIF60},
	{ 1280000,               eWCIF60},
	{ 1472000,               eWCIF60},
	{ 1536000,               eWCIF60},
	{ 1728000,               eWCIF60},
	{ 1920000,               eWCIF60},
	{ 2048000,               eWCIF60},
	{ 2560000,               eWCIF60},
	{ 3072000,               eWCIF60},
	{ 3584000,               eWCIF60},
	{ 4096000,               eWCIF60},
	{ 6144000,               eWCIF60},
	{ 8192000,               eWCIF60}
}
};
////////////////////////////////////////////////////////////////////////////////////////////////////

H264VideoModeThresholdStruct CResRsrcCalculator::g_SD_SharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES] =
{
{ //MPM
	{  64000,                eSD15},//TODO: check 4CIF15 or WCIF30
	{  96000,                eSD15},
	{ 128000,                eSD15},
	{ 192000,                eSD15},
	{ 256000,                eSD15},
	{ 320000,                eSD15},
	{ 384000,                eSD15},
	{ 512000,                eSD15},
	{ 768000,                eSD15},
	{ 832000,                eSD15},
	{ 1024000,               eSD15},
	{ 1152000,               eSD15},
	{ 1280000,               eSD15},
	{ 1472000,               eSD15},
	{ 1536000,               eSD15},
	{ 1728000,               eSD15},
	{ 1920000,               eSD15},
	{ 2048000,               eSD15},
	{ 2560000,               eSD15},
	{ 3072000,               eSD15},
	{ 3584000,               eSD15},
	{ 4096000,               eSD15},
	{ 6144000,               eSD15},
	{ 8192000,               eSD15}
},
{ //MPM+
	{  64000,                eSD30},//TODO: check if the same as SD15, 4CIF15, WCIF30
	{  96000,                eSD30},
	{ 128000,                eSD30},
	{ 192000,                eSD30},
	{ 256000,                eSD30},
	{ 320000,                eSD30},
	{ 384000,                eW4CIF30},
	{ 512000,                eW4CIF30},
	{ 768000,                eW4CIF30},
	{ 832000,                eW4CIF30},
	{ 1024000,               eW4CIF30},
	{ 1152000,               eW4CIF30},
	{ 1280000,               eW4CIF30},
	{ 1472000,               eW4CIF30},
	{ 1536000,               eW4CIF30},
	{ 1728000,               eW4CIF30},
	{ 1920000,               eW4CIF30},
	{ 2048000,               eW4CIF30},
	{ 2560000,               eW4CIF30},
	{ 3072000,               eW4CIF30},
	{ 3584000,               eW4CIF30},
	{ 4096000,               eW4CIF30},
	{ 6144000,               eW4CIF30},
	{ 8192000,               eW4CIF30}
},
{ //MPMx
	{  64000,                eSD30},//TODO: check if the same as SD15, 4CIF15, WCIF30
	{  96000,                eSD30},
	{ 128000,                eSD30},
	{ 192000,                eSD30},
	{ 256000,                eSD30},
	{ 320000,                eSD30},
	{ 384000,                eW4CIF30},
	{ 512000,                eW4CIF30},
	{ 768000,                eW4CIF30},
	{ 832000,                eW4CIF30},
	{ 1024000,               eW4CIF30},
	{ 1152000,               eW4CIF30},
	{ 1280000,               eW4CIF30},
	{ 1472000,               eW4CIF30},
	{ 1536000,               eW4CIF30},
	{ 1728000,               eW4CIF30},
	{ 1920000,               eW4CIF30},
	{ 2048000,               eW4CIF30},
	{ 2560000,               eW4CIF30},
	{ 3072000,               eW4CIF30},
	{ 3584000,               eW4CIF30},
	{ 4096000,               eW4CIF30},
	{ 6144000,               eW4CIF30},
	{ 8192000,               eW4CIF30}
},
{ //MPM-Rx
	{  64000,                eSD30},//TODO: check if the same as SD15, 4CIF15, WCIF30
	{  96000,                eSD30},
	{ 128000,                eSD30},
	{ 192000,                eSD30},
	{ 256000,                eSD30},
	{ 320000,                eSD30},
	{ 384000,                eSD30},
	{ 512000,                eSD30},
	{ 768000,                eSD30},
	{ 832000,                eSD30},
	{ 1024000,               eSD30},
	{ 1152000,               eSD30},
	{ 1280000,               eSD30},
	{ 1472000,               eSD30},
	{ 1536000,               eSD30},
	{ 1728000,               eSD30},
	{ 1920000,               eSD30},
	{ 2048000,               eSD30},
	{ 2560000,               eSD30},
	{ 3072000,               eSD30},
	{ 3584000,               eSD30},
	{ 4096000,               eSD30},
	{ 6144000,               eSD30},
	{ 8192000,               eSD30}
}
};
////////////////////////////////////////////////////////////////////////////////////////////////////
//                                       HD720
////////////////////////////////////////////////////////////////////////////////////////////////////

H264VideoModeThresholdStruct CResRsrcCalculator::g_HD720_MotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES] =
{
{ //MPM
	{  64000,                eSD30},
	{  96000,                eSD30},
	{ 128000,                eSD30},
	{ 192000,                eSD30},
	{ 256000,                eSD30},
	{ 320000,                eSD30},
	{ 384000,                eSD30},
	{ 512000,                eSD30},
	{ 768000,                eSD30}, //TODO: check if the same as WCIF30
	{ 832000,                eSD30},
	{ 1024000,               eW4CIF30},//TODO: check if the same as WSD30
	{ 1152000,               eW4CIF30},
	{ 1280000,               eW4CIF30},
	{ 1472000,               eW4CIF30},
	{ 1536000,               eW4CIF30},
	{ 1728000,               eW4CIF30},
	{ 1920000,               eW4CIF30},
	{ 2048000,               eW4CIF30},
	{ 2560000,               eW4CIF30},
	{ 3072000,               eW4CIF30},
	{ 3584000,               eW4CIF30},
	{ 4096000,               eW4CIF30},
	{ 6144000,               eW4CIF30},
	{ 8192000,               eW4CIF30}
},
{ //MPM+
	{  64000,                eSD60},
	{  96000,                eSD60},
	{ 128000,                eSD60},
	{ 192000,                eSD60},
	{ 256000,                eSD60},
	{ 320000,                eSD60},
	{ 384000,                eSD60},
	{ 512000,                eSD60},
	{ 768000,                eSD60},
	{ 832000,                eSD60},
	{ 1024000,               eSD60},
	{ 1152000,               eSD60},
	{ 1280000,               eSD60},
	{ 1472000,               eSD60},
	{ 1536000,               eSD60},
	{ 1728000,               eSD60},
	{ 1920000,               eSD60},
	{ 2048000,               eSD60},
	{ 2560000,               eSD60},
	{ 3072000,               eSD60},
	{ 3584000,               eSD60},
	{ 4096000,               eSD60},
	{ 6144000,               eSD60},
	{ 8192000,               eSD60}
},
{ //MPMx
	{  64000,                eSD60},
	{  96000,                eSD60},
	{ 128000,                eSD60},
	{ 192000,                eSD60},
	{ 256000,                eSD60},
	{ 320000,                eSD60},
	{ 384000,                eSD60},
	{ 512000,                eSD60},
	{ 768000,                eSD60},
	{ 832000,                eSD60},
	{ 1024000,               eSD60},
	{ 1152000,               eSD60},
	{ 1280000,               eSD60},
	{ 1472000,               eSD60},
	{ 1536000,               eSD60},
	{ 1728000,               eSD60},
	{ 1920000,               eSD60},
	{ 2048000,               eSD60},
	{ 2560000,               eSD60},
	{ 3072000,               eSD60},
	{ 3584000,               eSD60},
	{ 4096000,               eSD60},
	{ 6144000,               eSD60},
	{ 8192000,               eSD60}
},
{ //MPM-Rx
	{  64000,                eSD60},
	{  96000,                eSD60},
	{ 128000,                eSD60},
	{ 192000,                eSD60},
	{ 256000,                eSD60},
	{ 320000,                eSD60},
	{ 384000,                eSD60},
	{ 512000,                eSD60},
	{ 768000,                eSD60},
	{ 832000,                eSD60},
	{ 1024000,               eSD60},
	{ 1152000,               eSD60},
	{ 1280000,               eSD60},
	{ 1472000,               eSD60},
	{ 1536000,               eSD60},
	{ 1728000,               eSD60},
	{ 1920000,               eSD60},
	{ 2048000,               eSD60},
	{ 2560000,               eSD60},
	{ 3072000,               eSD60},
	{ 3584000,               eSD60},
	{ 4096000,               eSD60},
	{ 6144000,               eSD60},
	{ 8192000,               eSD60}
}
};
////////////////////////////////////////////////////////////////////////////////////////////////////

H264VideoModeThresholdStruct CResRsrcCalculator::g_HD720_SharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES] =
{
{ //MPM

	{  64000,                eSD30},
	{  96000,                eSD30},
	{ 128000,                eSD30},
	{ 192000,                eSD30},
	{ 256000,                eSD30},
	{ 320000,                eSD30},
	{ 384000,                eSD30},
	{ 512000,                eSD30},
	{ 768000,                eSD30},
	{ 832000,                eSD30},
	{ 1024000,               eSD30},
	{ 1152000,               eSD30},
	{ 1280000,               eSD30},
	{ 1472000,               eSD30},
	{ 1536000,               eSD30},
	{ 1728000,               eSD30},
	{ 1920000,               eSD30},
	{ 2048000,               eSD30},
	{ 2560000,               eSD30},
	{ 3072000,               eSD30},
	{ 3584000,               eSD30},
	{ 4096000,               eSD30},
	{ 6144000,               eSD30},
	{ 8192000,               eSD30}
},
{ //MPM+
	{  64000,                eHD720Symmetric},
	{  96000,                eHD720Symmetric},
	{ 128000,                eHD720Symmetric},
	{ 192000,                eHD720Symmetric},
	{ 256000,                eHD720Symmetric},
	{ 320000,                eHD720Symmetric},
	{ 384000,                eHD720Symmetric},
	{ 512000,                eHD720Symmetric},
	{ 768000,                eHD720Symmetric},
	{ 832000,                eHD720Symmetric},
	{ 1024000,               eHD720Symmetric},
	{ 1152000,               eHD720Symmetric},
	{ 1280000,               eHD720Symmetric},
	{ 1472000,               eHD720Symmetric},
	{ 1536000,               eHD720Symmetric},
	{ 1728000,               eHD720Symmetric},
	{ 1920000,               eHD720Symmetric},
	{ 2048000,               eHD720Symmetric},
	{ 2560000,               eHD720Symmetric},
	{ 3072000,               eHD720Symmetric},
	{ 3584000,               eHD720Symmetric},
	{ 4096000,               eHD720Symmetric},
	{ 6144000,               eHD720Symmetric},
	{ 8192000,               eHD720Symmetric}
},
{ //MPMx
	{  64000,                eHD720Symmetric},
	{  96000,                eHD720Symmetric},
	{ 128000,                eHD720Symmetric},
	{ 192000,                eHD720Symmetric},
	{ 256000,                eHD720Symmetric},
	{ 320000,                eHD720Symmetric},
	{ 384000,                eHD720Symmetric},
	{ 512000,                eHD720Symmetric},
	{ 768000,                eHD720Symmetric},
	{ 832000,                eHD720Symmetric},
	{ 1024000,               eHD720Symmetric},
	{ 1152000,               eHD720Symmetric},
	{ 1280000,               eHD720Symmetric},
	{ 1472000,               eHD720Symmetric},
	{ 1536000,               eHD720Symmetric},
	{ 1728000,               eHD720Symmetric},
	{ 1920000,               eHD720Symmetric},
	{ 2048000,               eHD720Symmetric},
	{ 2560000,               eHD720Symmetric},
	{ 3072000,               eHD720Symmetric},
	{ 3584000,               eHD720Symmetric},
	{ 4096000,               eHD720Symmetric},
	{ 6144000,               eHD720Symmetric},
	{ 8192000,               eHD720Symmetric}
},
{ //MPM-Rx
	{  64000,                eHD720Symmetric},
	{  96000,                eHD720Symmetric},
	{ 128000,                eHD720Symmetric},
	{ 192000,                eHD720Symmetric},
	{ 256000,                eHD720Symmetric},
	{ 320000,                eHD720Symmetric},
	{ 384000,                eHD720Symmetric},
	{ 512000,                eHD720Symmetric},
	{ 768000,                eHD720Symmetric},
	{ 832000,                eHD720Symmetric},
	{ 1024000,               eHD720Symmetric},
	{ 1152000,               eHD720Symmetric},
	{ 1280000,               eHD720Symmetric},
	{ 1472000,               eHD720Symmetric},
	{ 1536000,               eHD720Symmetric},
	{ 1728000,               eHD720Symmetric},
	{ 1920000,               eHD720Symmetric},
	{ 2048000,               eHD720Symmetric},
	{ 2560000,               eHD720Symmetric},
	{ 3072000,               eHD720Symmetric},
	{ 3584000,               eHD720Symmetric},
	{ 4096000,               eHD720Symmetric},
	{ 6144000,               eHD720Symmetric},
	{ 8192000,               eHD720Symmetric}
}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                       HD1080
////////////////////////////////////////////////////////////////////////////////////////////////////

H264VideoModeThresholdStruct CResRsrcCalculator::g_HD1080_MotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES] =
{
{ //MPM
	{  64000,                eHD720Asymmetric},
	{  96000,                eHD720Asymmetric},
	{ 128000,                eHD720Asymmetric},
	{ 192000,                eHD720Asymmetric},
	{ 256000,                eHD720Asymmetric},
	{ 320000,                eHD720Asymmetric},
	{ 384000,                eHD720Asymmetric},
	{ 512000,                eHD720Asymmetric},
	{ 768000,                eHD720Asymmetric},
	{ 832000,                eHD720Asymmetric},
	{ 1024000,               eHD720Asymmetric},
	{ 1152000,               eHD720Asymmetric},
	{ 1280000,               eHD720Asymmetric},
	{ 1472000,               eHD720Asymmetric},
	{ 1536000,               eHD720Asymmetric},
	{ 1728000,               eHD720Asymmetric},
	{ 1920000,               eHD720Asymmetric},
	{ 2048000,               eHD720Asymmetric},
	{ 2560000,               eHD720Asymmetric},
	{ 3072000,               eHD720Asymmetric},
	{ 3584000,               eHD720Asymmetric},
	{ 4096000,               eHD720Asymmetric},
	{ 6144000,               eHD720Asymmetric},
	{ 8192000,               eHD720Asymmetric}
},
{ //MPM+
	{  64000,                eHD720At60Asymmetric},
	{  96000,                eHD720At60Asymmetric},
	{ 128000,                eHD720At60Asymmetric},
	{ 192000,                eHD720At60Asymmetric},
	{ 256000,                eHD720At60Asymmetric},
	{ 320000,                eHD720At60Asymmetric},
	{ 384000,                eHD720At60Asymmetric},
	{ 512000,                eHD720At60Asymmetric},
	{ 768000,                eHD720At60Asymmetric},
	{ 832000,                eHD720At60Asymmetric},
	{ 1024000,               eHD720At60Asymmetric},
	{ 1152000,               eHD720At60Asymmetric},
	{ 1280000,               eHD720At60Asymmetric},
	{ 1472000,               eHD720At60Asymmetric},
	{ 1536000,               eHD720At60Asymmetric},
	{ 1728000,               eHD720At60Asymmetric},
	{ 1920000,               eHD720At60Asymmetric},
	{ 2048000,               eHD720At60Asymmetric},
	{ 2560000,               eHD720At60Asymmetric},
	{ 3072000,               eHD720At60Asymmetric},
	{ 3584000,               eHD720At60Asymmetric},
	{ 4096000,               eHD720At60Asymmetric},
	{ 6144000,               eHD720At60Asymmetric},
	{ 8192000,               eHD720At60Asymmetric}
},
{ //MPMx
	{  64000,                eHD720At60Symmetric},
	{  96000,                eHD720At60Symmetric},
	{ 128000,                eHD720At60Symmetric},
	{ 192000,                eHD720At60Symmetric},
	{ 256000,                eHD720At60Symmetric},
	{ 320000,                eHD720At60Symmetric},
	{ 384000,                eHD720At60Symmetric},
	{ 512000,                eHD720At60Symmetric},
	{ 768000,                eHD720At60Symmetric},
	{ 832000,                eHD720At60Symmetric},
	{ 1024000,               eHD720At60Symmetric},
	{ 1152000,               eHD720At60Symmetric},
	{ 1280000,               eHD720At60Symmetric},
	{ 1472000,               eHD720At60Symmetric},
	{ 1536000,               eHD720At60Symmetric},
	{ 1728000,               eHD720At60Symmetric},
	{ 1920000,               eHD720At60Symmetric},
	{ 2048000,               eHD720At60Symmetric},
	{ 2560000,               eHD720At60Symmetric},
	{ 3072000,               eHD720At60Symmetric},
	{ 3584000,               eHD720At60Symmetric},
	{ 4096000,               eHD720At60Symmetric},
	{ 6144000,               eHD720At60Symmetric},
	{ 8192000,               eHD720At60Symmetric}
},
{ //MPM-Rx
	{  64000,                eHD720At60Symmetric},
	{  96000,                eHD720At60Symmetric},
	{ 128000,                eHD720At60Symmetric},
	{ 192000,                eHD720At60Symmetric},
	{ 256000,                eHD720At60Symmetric},
	{ 320000,                eHD720At60Symmetric},
	{ 384000,                eHD720At60Symmetric},
	{ 512000,                eHD720At60Symmetric},
	{ 768000,                eHD720At60Symmetric},
	{ 832000,                eHD720At60Symmetric},
	{ 1024000,               eHD720At60Symmetric},
	{ 1152000,               eHD720At60Symmetric},
	{ 1280000,               eHD720At60Symmetric},
	{ 1472000,               eHD720At60Symmetric},
	{ 1536000,               eHD720At60Symmetric},
	{ 1728000,               eHD720At60Symmetric},
	{ 1920000,               eHD720At60Symmetric},
	{ 2048000,               eHD720At60Symmetric},
	{ 2560000,               eHD720At60Symmetric},
	{ 3072000,               eHD720At60Symmetric},
	{ 3584000,               eHD720At60Symmetric},
	{ 4096000,               eHD720At60Symmetric},
	{ 6144000,               eHD720At60Symmetric},
	{ 8192000,               eHD720At60Symmetric}
}
};
////////////////////////////////////////////////////////////////////////////////////////////////////

H264VideoModeThresholdStruct CResRsrcCalculator::g_HD1080_SharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES] =
{
{ //MPM
	{  64000,                eHD720Asymmetric},
	{  96000,                eHD720Asymmetric},
	{ 128000,                eHD720Asymmetric},
	{ 192000,                eHD720Asymmetric},
	{ 256000,                eHD720Asymmetric},
	{ 320000,                eHD720Asymmetric},
	{ 384000,                eHD720Asymmetric},
	{ 512000,                eHD720Asymmetric},
	{ 768000,                eHD720Asymmetric},
	{ 832000,                eHD720Asymmetric},
	{ 1024000,               eHD720Asymmetric},
	{ 1152000,               eHD720Asymmetric},
	{ 1280000,               eHD720Asymmetric},
	{ 1472000,               eHD720Asymmetric},
	{ 1536000,               eHD720Asymmetric},
	{ 1728000,               eHD720Asymmetric},
	{ 1920000,               eHD720Asymmetric},
	{ 2048000,               eHD720Asymmetric},
	{ 2560000,               eHD720Asymmetric},
	{ 3072000,               eHD720Asymmetric},
	{ 3584000,               eHD720Asymmetric},
	{ 4096000,               eHD720Asymmetric},
	{ 6144000,               eHD720Asymmetric},
	{ 8192000,               eHD720Asymmetric}
},
{ //MPM+
	{  64000,                eHD1080Asymmetric},
	{  96000,                eHD1080Asymmetric},
	{ 128000,                eHD1080Asymmetric},
	{ 192000,                eHD1080Asymmetric},
	{ 256000,                eHD1080Asymmetric},
	{ 320000,                eHD1080Asymmetric},
	{ 384000,                eHD1080Asymmetric},
	{ 512000,                eHD1080Asymmetric},
	{ 768000,                eHD1080Asymmetric},
	{ 832000,                eHD1080Asymmetric},
	{ 1024000,               eHD1080Asymmetric},
	{ 1152000,               eHD1080Asymmetric},
	{ 1280000,               eHD1080Asymmetric},
	{ 1472000,               eHD1080Asymmetric},
	{ 1536000,               eHD1080Asymmetric},
	{ 1728000,               eHD1080Asymmetric},
	{ 1920000,               eHD1080Asymmetric},
	{ 2048000,               eHD1080Asymmetric},
	{ 2560000,               eHD1080Asymmetric},
	{ 3072000,               eHD1080Asymmetric},
	{ 3584000,               eHD1080Asymmetric},
	{ 4096000,               eHD1080Asymmetric},
	{ 6144000,               eHD1080Asymmetric},
	{ 8192000,               eHD1080Asymmetric}
},
{ //MPMx
	{  64000,                eHD1080Symmetric},
	{  96000,                eHD1080Symmetric},
	{ 128000,                eHD1080Symmetric},
	{ 192000,                eHD1080Symmetric},
	{ 256000,                eHD1080Symmetric},
	{ 320000,                eHD1080Symmetric},
	{ 384000,                eHD1080Symmetric},
	{ 512000,                eHD1080Symmetric},
	{ 768000,                eHD1080Symmetric},
	{ 832000,                eHD1080Symmetric},
	{ 1024000,               eHD1080Symmetric},
	{ 1152000,               eHD1080Symmetric},
	{ 1280000,               eHD1080Symmetric},
	{ 1472000,               eHD1080Symmetric},
	{ 1536000,               eHD1080Symmetric},
	{ 1728000,               eHD1080Symmetric},
	{ 1920000,               eHD1080Symmetric},
	{ 2048000,               eHD1080Symmetric},
	{ 2560000,               eHD1080Symmetric},
	{ 3072000,               eHD1080Symmetric},
	{ 3584000,               eHD1080Symmetric},
	{ 4096000,               eHD1080Symmetric},
	{ 6144000,               eHD1080Symmetric},
	{ 8192000,               eHD1080Symmetric}
},
{ //MPM-Rx
	{  64000,                eHD1080Symmetric},
	{  96000,                eHD1080Symmetric},
	{ 128000,                eHD1080Symmetric},
	{ 192000,                eHD1080Symmetric},
	{ 256000,                eHD1080Symmetric},
	{ 320000,                eHD1080Symmetric},
	{ 384000,                eHD1080Symmetric},
	{ 512000,                eHD1080Symmetric},
	{ 768000,                eHD1080Symmetric},
	{ 832000,                eHD1080Symmetric},
	{ 1024000,               eHD1080Symmetric},
	{ 1152000,               eHD1080Symmetric},
	{ 1280000,               eHD1080Symmetric},
	{ 1472000,               eHD1080Symmetric},
	{ 1536000,               eHD1080Symmetric},
	{ 1728000,               eHD1080Symmetric},
	{ 1920000,               eHD1080Symmetric},
	{ 2048000,               eHD1080Symmetric},
	{ 2560000,               eHD1080Symmetric},
	{ 3072000,               eHD1080Symmetric},
	{ 3584000,               eHD1080Symmetric},
	{ 4096000,               eHD1080Symmetric},
	{ 6144000,               eHD1080Symmetric},
	{ 8192000,               eHD1080Symmetric}
}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                       HD1080 60
////////////////////////////////////////////////////////////////////////////////////////////////////

H264VideoModeThresholdStruct CResRsrcCalculator::g_HD1080_60_MotionVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES] =
{
{ //MPM
	{  64000,                eHD720Asymmetric},
	{  96000,                eHD720Asymmetric},
	{ 128000,                eHD720Asymmetric},
	{ 192000,                eHD720Asymmetric},
	{ 256000,                eHD720Asymmetric},
	{ 320000,                eHD720Asymmetric},
	{ 384000,                eHD720Asymmetric},
	{ 512000,                eHD720Asymmetric},
	{ 768000,                eHD720Asymmetric},
	{ 832000,                eHD720Asymmetric},
	{ 1024000,               eHD720Asymmetric},
	{ 1152000,               eHD720Asymmetric},
	{ 1280000,               eHD720Asymmetric},
	{ 1472000,               eHD720Asymmetric},
	{ 1536000,               eHD720Asymmetric},
	{ 1728000,               eHD720Asymmetric},
	{ 1920000,               eHD720Asymmetric},
	{ 2048000,               eHD720Asymmetric},
	{ 2560000,               eHD720Asymmetric},
	{ 3072000,               eHD720Asymmetric},
	{ 4096000,               eHD720Asymmetric},
	{ 6144000,               eHD720Asymmetric},
	{ 8192000,               eHD720Asymmetric}
},
{ //MPM+
	{  64000,                eHD720At60Asymmetric},
	{  96000,                eHD720At60Asymmetric},
	{ 128000,                eHD720At60Asymmetric},
	{ 192000,                eHD720At60Asymmetric},
	{ 256000,                eHD720At60Asymmetric},
	{ 320000,                eHD720At60Asymmetric},
	{ 384000,                eHD720At60Asymmetric},
	{ 512000,                eHD720At60Asymmetric},
	{ 768000,                eHD720At60Asymmetric},
	{ 832000,                eHD720At60Asymmetric},
	{ 1024000,               eHD720At60Asymmetric},
	{ 1152000,               eHD720At60Asymmetric},
	{ 1280000,               eHD720At60Asymmetric},
	{ 1472000,               eHD720At60Asymmetric},
	{ 1536000,               eHD720At60Asymmetric},
	{ 1728000,               eHD720At60Asymmetric},
	{ 1920000,               eHD720At60Asymmetric},
	{ 2048000,               eHD720At60Asymmetric},
	{ 2560000,               eHD720At60Asymmetric},
	{ 3072000,               eHD720At60Asymmetric},
	{ 3584000,               eHD720At60Asymmetric},
	{ 4096000,               eHD720At60Asymmetric},
	{ 6144000,               eHD720At60Asymmetric},
	{ 8192000,               eHD720At60Asymmetric}
},
{ //MPMx
	{  64000,                eHD1080At60Asymmetric},
	{  96000,                eHD1080At60Asymmetric},
	{ 128000,                eHD1080At60Asymmetric},
	{ 192000,                eHD1080At60Asymmetric},
	{ 256000,                eHD1080At60Asymmetric},
	{ 320000,                eHD1080At60Asymmetric},
	{ 384000,                eHD1080At60Asymmetric},
	{ 512000,                eHD1080At60Asymmetric},
	{ 768000,                eHD1080At60Asymmetric},
	{ 832000,                eHD1080At60Asymmetric},
	{ 1024000,               eHD1080At60Asymmetric},
	{ 1152000,               eHD1080At60Asymmetric},
	{ 1280000,               eHD1080At60Asymmetric},
	{ 1472000,               eHD1080At60Asymmetric},
	{ 1536000,               eHD1080At60Asymmetric},
	{ 1728000,               eHD1080At60Asymmetric},
	{ 1920000,               eHD1080At60Asymmetric},
	{ 2048000,               eHD1080At60Asymmetric},
	{ 2560000,               eHD1080At60Asymmetric},
	{ 3072000,               eHD1080At60Asymmetric},
	{ 3584000,               eHD1080At60Asymmetric},
	{ 4096000,               eHD1080At60Asymmetric},
	{ 6144000,               eHD1080At60Asymmetric},
	{ 8192000,               eHD1080At60Asymmetric}
},
{ //MPM-Rx
	{  64000,                eHD1080At60Symmetric},
	{  96000,                eHD1080At60Symmetric},
	{ 128000,                eHD1080At60Symmetric},
	{ 192000,                eHD1080At60Symmetric},
	{ 256000,                eHD1080At60Symmetric},
	{ 320000,                eHD1080At60Symmetric},
	{ 384000,                eHD1080At60Symmetric},
	{ 512000,                eHD1080At60Symmetric},
	{ 768000,                eHD1080At60Symmetric},
	{ 832000,                eHD1080At60Symmetric},
	{ 1024000,               eHD1080At60Symmetric},
	{ 1152000,               eHD1080At60Symmetric},
	{ 1280000,               eHD1080At60Symmetric},
	{ 1472000,               eHD1080At60Symmetric},
	{ 1536000,               eHD1080At60Symmetric},
	{ 1728000,               eHD1080At60Symmetric},
	{ 1920000,               eHD1080At60Symmetric},
	{ 2048000,               eHD1080At60Symmetric},
	{ 2560000,               eHD1080At60Symmetric},
	{ 3072000,               eHD1080At60Symmetric},
	{ 3584000,               eHD1080At60Symmetric},
	{ 4096000,               eHD1080At60Symmetric},
	{ 6144000,               eHD1080At60Symmetric},
	{ 8192000,               eHD1080At60Symmetric}
}
};

/////////////////////////////////////////////////////////////////////////////////
H264VideoModeThresholdStruct CResRsrcCalculator::g_HD1080_60_SharpnessVideoModeThresholdTbl[NUM_OF_SYSTEM_CARDS_MODES][eMAX_NUM_THRESHOLD_RATES] =
{
{ //MPM
	{  64000,                eHD720Asymmetric},
	{  96000,                eHD720Asymmetric},
	{ 128000,                eHD720Asymmetric},
	{ 192000,                eHD720Asymmetric},
	{ 256000,                eHD720Asymmetric},
	{ 320000,                eHD720Asymmetric},
	{ 384000,                eHD720Asymmetric},
	{ 512000,                eHD720Asymmetric},
	{ 768000,                eHD720Asymmetric},
	{ 832000,                eHD720Asymmetric},
	{ 1024000,               eHD720Asymmetric},
	{ 1152000,               eHD720Asymmetric},
	{ 1280000,               eHD720Asymmetric},
	{ 1472000,               eHD720Asymmetric},
	{ 1536000,               eHD720Asymmetric},
	{ 1728000,               eHD720Asymmetric},
	{ 1920000,               eHD720Asymmetric},
	{ 2048000,               eHD720Asymmetric},
	{ 2560000,               eHD720Asymmetric},
	{ 3072000,               eHD720Asymmetric},
	{ 3584000,               eHD720Asymmetric},
	{ 4096000,               eHD720Asymmetric},
	{ 6144000,               eHD720Asymmetric},
	{ 8192000,               eHD720Asymmetric}
},
{ //MPM+
	{  64000,                eHD1080Asymmetric},
	{  96000,                eHD1080Asymmetric},
	{ 128000,                eHD1080Asymmetric},
	{ 192000,                eHD1080Asymmetric},
	{ 256000,                eHD1080Asymmetric},
	{ 320000,                eHD1080Asymmetric},
	{ 384000,                eHD1080Asymmetric},
	{ 512000,                eHD1080Asymmetric},
	{ 768000,                eHD1080Asymmetric},
	{ 832000,                eHD1080Asymmetric},
	{ 1024000,               eHD1080Asymmetric},
	{ 1152000,               eHD1080Asymmetric},
	{ 1280000,               eHD1080Asymmetric},
	{ 1472000,               eHD1080Asymmetric},
	{ 1536000,               eHD1080Asymmetric},
	{ 1728000,               eHD1080Asymmetric},
	{ 1920000,               eHD1080Asymmetric},
	{ 2048000,               eHD1080Asymmetric},
	{ 2560000,               eHD1080Asymmetric},
	{ 3072000,               eHD1080Asymmetric},
	{ 3584000,               eHD1080Asymmetric},
	{ 4096000,               eHD1080Asymmetric},
	{ 6144000,               eHD1080Asymmetric},
	{ 8192000,               eHD1080Asymmetric}
},
{ //MPMx
	{  64000,                eHD1080At60Asymmetric},
	{  96000,                eHD1080At60Asymmetric},
	{ 128000,                eHD1080At60Asymmetric},
	{ 192000,                eHD1080At60Asymmetric},
	{ 256000,                eHD1080At60Asymmetric},
	{ 320000,                eHD1080At60Asymmetric},
	{ 384000,                eHD1080At60Asymmetric},
	{ 512000,                eHD1080At60Asymmetric},
	{ 768000,                eHD1080At60Asymmetric},
	{ 832000,                eHD1080At60Asymmetric},
	{ 1024000,               eHD1080At60Asymmetric},
	{ 1152000,               eHD1080At60Asymmetric},
	{ 1280000,               eHD1080At60Asymmetric},
	{ 1472000,               eHD1080At60Asymmetric},
	{ 1536000,               eHD1080At60Asymmetric},
	{ 1728000,               eHD1080At60Asymmetric},
	{ 1920000,               eHD1080At60Asymmetric},
	{ 2048000,               eHD1080At60Asymmetric},
	{ 2560000,               eHD1080At60Asymmetric},
	{ 3072000,               eHD1080At60Asymmetric},
	{ 3584000,               eHD1080At60Asymmetric},
	{ 4096000,               eHD1080At60Asymmetric},
	{ 6144000,               eHD1080At60Asymmetric},
	{ 8192000,               eHD1080At60Asymmetric}
},
{ //MPM-Rx
	{  64000,                eHD1080At60Symmetric},
	{  96000,                eHD1080At60Symmetric},
	{ 128000,                eHD1080At60Symmetric},
	{ 192000,                eHD1080At60Symmetric},
	{ 256000,                eHD1080At60Symmetric},
	{ 320000,                eHD1080At60Symmetric},
	{ 384000,                eHD1080At60Symmetric},
	{ 512000,                eHD1080At60Symmetric},
	{ 768000,                eHD1080At60Symmetric},
	{ 832000,                eHD1080At60Symmetric},
	{ 1024000,               eHD1080At60Symmetric},
	{ 1152000,               eHD1080At60Symmetric},
	{ 1280000,               eHD1080At60Symmetric},
	{ 1472000,               eHD1080At60Symmetric},
	{ 1536000,               eHD1080At60Symmetric},
	{ 1728000,               eHD1080At60Symmetric},
	{ 1920000,               eHD1080At60Symmetric},
	{ 2048000,               eHD1080At60Symmetric},
	{ 2560000,               eHD1080At60Symmetric},
	{ 3072000,               eHD1080At60Symmetric},
	{ 3584000,               eHD1080At60Symmetric},
	{ 4096000,               eHD1080At60Symmetric},
	{ 6144000,               eHD1080At60Symmetric},
	{ 8192000,               eHD1080At60Symmetric}
}
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
CResRsrcCalculator::CResRsrcCalculator()
{
}

/////////////////////////////////////////////////////////////////////////////////
void CResRsrcCalculator::DumpDynamicThresholdTblByInd(DWORD i, ostringstream& str)
{
	if (eSystemCardsMode_breeze-1==i)
	{
		str << "\nMPMx mode :\n";
	}
	if (eSystemCardsMode_mpmrx-1==i)
	{
		str << "\nMPM-Rx mode :\n";
	}
	else
	{
		str << "\nUnknown mode(!) :" << i;
		return;
	}

	str << "\t Base Profile Motion table = { "
		         << eVideoModeTypeNames[g_dynamicMotionVideoModeThresholdTbl[i][0].videoModeType] << "=" << g_dynamicMotionVideoModeThresholdTbl[i][0].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicMotionVideoModeThresholdTbl[i][1].videoModeType] << "=" << g_dynamicMotionVideoModeThresholdTbl[i][1].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicMotionVideoModeThresholdTbl[i][2].videoModeType] << "=" << g_dynamicMotionVideoModeThresholdTbl[i][2].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicMotionVideoModeThresholdTbl[i][3].videoModeType] << "=" << g_dynamicMotionVideoModeThresholdTbl[i][3].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicMotionVideoModeThresholdTbl[i][4].videoModeType] << "=" << g_dynamicMotionVideoModeThresholdTbl[i][4].thresholdBitrate << " }\n"
		<< " \t Base Profile Sharpness table = { "
		         << eVideoModeTypeNames[g_dynamicSharpnessVideoModeThresholdTbl[i][0].videoModeType] << "=" << g_dynamicSharpnessVideoModeThresholdTbl[i][0].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicSharpnessVideoModeThresholdTbl[i][1].videoModeType] << "=" << g_dynamicSharpnessVideoModeThresholdTbl[i][1].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicSharpnessVideoModeThresholdTbl[i][2].videoModeType] << "=" << g_dynamicSharpnessVideoModeThresholdTbl[i][2].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicSharpnessVideoModeThresholdTbl[i][3].videoModeType] << "=" << g_dynamicSharpnessVideoModeThresholdTbl[i][3].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicSharpnessVideoModeThresholdTbl[i][4].videoModeType] << "=" << g_dynamicSharpnessVideoModeThresholdTbl[i][4].thresholdBitrate << " }\n"
		<< "\t High Profile Motion table = { "
			       << eVideoModeTypeNames[g_dynamicHighProfileMotionThresholdTbl[i][0].videoModeType] << "=" << g_dynamicHighProfileMotionThresholdTbl[i][0].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicHighProfileMotionThresholdTbl[i][1].videoModeType] << "=" << g_dynamicHighProfileMotionThresholdTbl[i][1].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicHighProfileMotionThresholdTbl[i][2].videoModeType] << "=" << g_dynamicHighProfileMotionThresholdTbl[i][2].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicHighProfileMotionThresholdTbl[i][3].videoModeType] << "=" << g_dynamicHighProfileMotionThresholdTbl[i][3].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicHighProfileMotionThresholdTbl[i][4].videoModeType] << "=" << g_dynamicHighProfileMotionThresholdTbl[i][4].thresholdBitrate << " }\n"
		<< " \t High Profile Sharpness table = { "
		         << eVideoModeTypeNames[g_dynamicHighProfileSharpnessThresholdTbl[i][0].videoModeType] << "=" << g_dynamicHighProfileSharpnessThresholdTbl[i][0].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicHighProfileSharpnessThresholdTbl[i][1].videoModeType] << "=" << g_dynamicHighProfileSharpnessThresholdTbl[i][1].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicHighProfileSharpnessThresholdTbl[i][2].videoModeType] << "=" << g_dynamicHighProfileSharpnessThresholdTbl[i][2].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicHighProfileSharpnessThresholdTbl[i][3].videoModeType] << "=" << g_dynamicHighProfileSharpnessThresholdTbl[i][3].thresholdBitrate
		<< ",  " << eVideoModeTypeNames[g_dynamicHighProfileSharpnessThresholdTbl[i][4].videoModeType] << "=" << g_dynamicHighProfileSharpnessThresholdTbl[i][4].thresholdBitrate << " }";
}

/////////////////////////////////////////////////////////////////////////////////
void CResRsrcCalculator::DumpDynamicThresholdTbl(eSystemCardsMode systemCardsBasedMode)
{
	ostringstream str;
	const char *descrCPmaxRes = NULL;
	BYTE res = CStringsMaps::GetDescription(RESOLUTION_THRESHOLD_TYPE_ENUM, m_CPmaxRes, &descrCPmaxRes);
	str << "CResRsrcCalculator::DumpDynamicResSlider - ConfigType:" << eResolutionConfigTypeNames[m_ConfigType] << ", CPmaxRes:" << descrCPmaxRes;

	if (eSystemCardsMode_illegal == systemCardsBasedMode)
	{
		for (WORD i = 0; i < (NUM_OF_SYSTEM_CARDS_MODES - 1); i++)
			DumpDynamicThresholdTblByInd(i, str);
	}
	else
	{
		WORD card_tbl_index = systemCardsBasedMode - 1;
		DumpDynamicThresholdTblByInd(card_tbl_index, str);
	}
	FPTRACE(eLevelInfoNormal, str.str().c_str()); //DEBUG
}

/////////////////////////////////////////////////////////////////////////////////
/*void CResRsrcCalculator::DumpDefaultThresholdTblByInd(DWORD i, ostringstream& str)
{
	if (eSystemCardsMode_breeze-1==i)
	{
		str << "\nMPMx mode :";
	}
	if (eSystemCardsMode_mpmrx-1==i)
	{
		str << "\nMPM-Rx mode :";
	}
	else
	{
		str << "\nUnknown mode(!) :" << i;
		return;
	}

	CPrettyTable<const char*, DWORD, DWORD, DWORD, DWORD> tbl1("VideoMode", "Balanced", "Optimized", "Quality", "HiProfile");
	for (int j = 1; j < MAX_NUM_SLIDER_RATE_MOTION; ++j)
	{
		tbl1.Add(
			eVideoModeTypeNames[g_defaultMotionThresholdTbl[i][j].videoModeType],
			ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][j].balanced_bitrate),
			ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][j].resource_optimized_bitrate),
			ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][j].user_optimized_bitrate),
			ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][j].hi_profile_optimized_bitrate));
	}

	CPrettyTable<const char*, DWORD, DWORD, DWORD, DWORD> tbl2("VideoMode", "Balanced", "Optimized", "Quality", "HiProfile");
	for (int j = 1; j < MAX_NUM_SLIDER_RATE_MOTION; ++j)
	{
		tbl2.Add(
			eVideoModeTypeNames[g_defaultSharpnessThresholdTbl[i][j].videoModeType],
			ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][j].balanced_bitrate),
			ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][j].resource_optimized_bitrate),
			ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][j].user_optimized_bitrate),
			ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][j].hi_profile_optimized_bitrate));
	}

	CPrettyTable<const char*, DWORD, DWORD, DWORD, DWORD> tbl3("VideoMode", "Balanced", "Optimized", "Quality", "HiProfile");
	for (int j = 1; j < MAX_NUM_SLIDER_RATE_MOTION; ++j)
	{
		tbl3.Add(
			eVideoModeTypeNames[g_defaultHighProfileMotionThresholdTbl[i][j].videoModeType],
			ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][j].balanced_bitrate),
			ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][j].resource_optimized_bitrate),
			ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][j].user_optimized_bitrate),
			ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][j].hi_profile_optimized_bitrate));
	}

	CPrettyTable<const char*, DWORD, DWORD, DWORD, DWORD> tbl4("VideoMode", "Balanced", "Optimized", "Quality", "HiProfile");
	for (int j = 1; j < MAX_NUM_SLIDER_RATE_MOTION; ++j)
	{
		tbl4.Add(
			eVideoModeTypeNames[g_defaultHighProfileSharpnessThresholdTbl[i][j].videoModeType],
			ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][j].balanced_bitrate),
			ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][j].resource_optimized_bitrate),
			ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][j].user_optimized_bitrate),
			ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][j].hi_profile_optimized_bitrate));
	}

	str << "\n\t Base Profile Motion table" << tbl1.Get();
	str << "\n\t Base Profile Sharpness table" << tbl2.Get();
	str << "\n\t High Profile Motion table" << tbl3.Get();
	str << "\n\t High Profile Sharpness table" << tbl4.Get();
}
*/

////////*********//////////

// Temporary fix for BRIDGE-14282 - revert Dima's KW fix for CResRsrcCalculator::DumpDefaultThresholdTblByInd


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CResRsrcCalculator::DumpDefaultThresholdTblByInd(DWORD i, ostringstream& str)
{
	str << ( ((eSystemCardsMode_breeze-1)==i) ? "\nMPMx mode :\n" : ((eSystemCardsMode_mpmrx-1)==i ? "\nMPM-Rx mode :\n" : "\nUnknown mode(!) :\n") )
	    << "\t Base Profile Motion table = { "
		<< "\n\t\t " << eVideoModeTypeNames[g_defaultMotionThresholdTbl[i][1].videoModeType]
		<< ":  balanced=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][1].balanced_bitrate)
	    << "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][1].resource_optimized_bitrate)
	    << "\t video_quality=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][1].user_optimized_bitrate)
	    << "\t hi_profile=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][1].hi_profile_optimized_bitrate)
	    << "\n\t\t " << eVideoModeTypeNames[g_defaultMotionThresholdTbl[i][2].videoModeType]
  		<< ":  balanced=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][2].balanced_bitrate)
  		<< "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][2].resource_optimized_bitrate)
  		<< "\t video_quality=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][2].user_optimized_bitrate)
  		<< "\t hi_profile=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][2].hi_profile_optimized_bitrate)
  		<< "\n\t\t " << eVideoModeTypeNames[g_defaultMotionThresholdTbl[i][3].videoModeType]
  		<< ":  balanced=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][3].balanced_bitrate)
  		<< "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][3].resource_optimized_bitrate)
  		<< "\t video_quality=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][3].user_optimized_bitrate)
  		<< "\t hi_profile=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][3].hi_profile_optimized_bitrate)
   		<< "\n\t\t " << eVideoModeTypeNames[g_defaultMotionThresholdTbl[i][4].videoModeType]
  		<< ":  balanced=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][4].balanced_bitrate)
  		<< "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][4].resource_optimized_bitrate)
  		<< "\t video_quality=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][4].user_optimized_bitrate)
  		<< "\t hi_profile=" << ConvertEThresholdBitRate(g_defaultMotionThresholdTbl[i][4].hi_profile_optimized_bitrate)
 		<< " }\n"

  		<< " \t Base Profile Sharpness table = { "
	    << "\n\t\t " << eVideoModeTypeNames[g_defaultSharpnessThresholdTbl[i][1].videoModeType]
        << ":  balanced=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][1].balanced_bitrate)
	    << "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][1].resource_optimized_bitrate)
	    << "\t video_quality=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][1].user_optimized_bitrate)
	    << "\t hi_profile_optimized=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][1].hi_profile_optimized_bitrate)
	    << "\n\t\t " << eVideoModeTypeNames[g_defaultSharpnessThresholdTbl[i][2].videoModeType]
	    << ":  balanced=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][2].balanced_bitrate)
	    << "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][2].resource_optimized_bitrate)
	    << "\t video_quality=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][2].user_optimized_bitrate)
	    << "\t hi_profile_optimized=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][2].hi_profile_optimized_bitrate)
	    << "\n\t\t " << eVideoModeTypeNames[g_defaultSharpnessThresholdTbl[i][3].videoModeType]
        << ":  balanced=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][3].balanced_bitrate)
        << "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][3].resource_optimized_bitrate)
        << "\t video_quality=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][3].user_optimized_bitrate)
        << "\t hi_profile_optimized=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][3].hi_profile_optimized_bitrate)
	    << "\n\t\t " << eVideoModeTypeNames[g_defaultSharpnessThresholdTbl[i][4].videoModeType]
        << ":  balanced=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][4].balanced_bitrate)
        << "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][4].resource_optimized_bitrate)
        << "\t video_quality=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][4].user_optimized_bitrate)
        << "\t hi_profile_optimized=" << ConvertEThresholdBitRate(g_defaultSharpnessThresholdTbl[i][4].hi_profile_optimized_bitrate)
        << " }";

	str << "\n\t High Profile Motion table = { "
		<< "\n\t\t " << eVideoModeTypeNames[g_defaultHighProfileMotionThresholdTbl[i][1].videoModeType]
		<< ":  balanced=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][1].balanced_bitrate)
	    << "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][1].resource_optimized_bitrate)
	    << "\t video_quality=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][1].user_optimized_bitrate)
	    << "\n\t\t " << eVideoModeTypeNames[g_defaultHighProfileMotionThresholdTbl[i][2].videoModeType]
  		<< ":  balanced=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][2].balanced_bitrate)
  		<< "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][2].resource_optimized_bitrate)
  		<< "\t video_quality=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][2].user_optimized_bitrate)
  		<< "\n\t\t " << eVideoModeTypeNames[g_defaultHighProfileMotionThresholdTbl[i][3].videoModeType]
  		<< ":  balanced=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][3].balanced_bitrate)
  		<< "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][3].resource_optimized_bitrate)
  		<< "\t video_quality=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][3].user_optimized_bitrate)
  		<< "\n\t\t " << eVideoModeTypeNames[g_defaultHighProfileMotionThresholdTbl[i][4].videoModeType]
  		<< ":  balanced=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][4].balanced_bitrate)
  		<< "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][4].resource_optimized_bitrate)
  		<< "\t video_quality=" << ConvertEThresholdBitRate(g_defaultHighProfileMotionThresholdTbl[i][4].user_optimized_bitrate)
 		<< " }\n"

  		<< " \t High Profile Sharpness table = { "
	    << "\n\t\t " << eVideoModeTypeNames[g_defaultHighProfileSharpnessThresholdTbl[i][1].videoModeType]
        << ":  balanced=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][1].balanced_bitrate)
	    << "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][1].resource_optimized_bitrate)
	    << "\t video_quality=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][1].user_optimized_bitrate)
	    << "\n\t\t " << eVideoModeTypeNames[g_defaultHighProfileSharpnessThresholdTbl[i][2].videoModeType]
	    << ":  balanced=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][2].balanced_bitrate)
	    << "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][2].resource_optimized_bitrate)
	    << "\t video_quality=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][2].user_optimized_bitrate)
	    << "\n\t\t " << eVideoModeTypeNames[g_defaultHighProfileSharpnessThresholdTbl[i][3].videoModeType]
        << ":  balanced=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][3].balanced_bitrate)
        << "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][3].resource_optimized_bitrate)
        << "\t video_quality=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][3].user_optimized_bitrate)
	    << "\n\t\t " << eVideoModeTypeNames[g_defaultHighProfileSharpnessThresholdTbl[i][4].videoModeType]
        << ":  balanced=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][4].balanced_bitrate)
        << "\t resource_optimized=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][4].resource_optimized_bitrate)
        << "\t video_quality=" << ConvertEThresholdBitRate(g_defaultHighProfileSharpnessThresholdTbl[i][4].user_optimized_bitrate)
        << " }";
}
////////*********//////////


/////////////////////////////////////////////////////////////////////////////////
void CResRsrcCalculator::DumpDefaultThresholdTbl(eSystemCardsMode systemCardsBasedMode)
{
	ostringstream str;
	const char *descrCPmaxRes = NULL;
	BYTE res = CStringsMaps::GetDescription(RESOLUTION_THRESHOLD_TYPE_ENUM, m_CPmaxRes, &descrCPmaxRes);
	str << "CResRsrcCalculator::DumpDefaultThresholdTbl - ConfigType:" << eResolutionConfigTypeNames[m_ConfigType] << ", CPmaxRes:" << descrCPmaxRes;
	if (eSystemCardsMode_illegal == systemCardsBasedMode)
	{
		for (WORD i = 0; i < (NUM_OF_SYSTEM_CARDS_MODES - 1); i++)
			DumpDefaultThresholdTblByInd(i, str);
	}
	else
	{
		WORD card_tbl_index = systemCardsBasedMode - 1;
		if (card_tbl_index < NUM_OF_SYSTEM_CARDS_MODES)
		   DumpDefaultThresholdTblByInd(card_tbl_index, str);
	}
	FPTRACE(eLevelInfoNormal, str.str().c_str()); //DEBUG
}

/////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CResRsrcCalculator::GetRsrcVideoType(eSystemCardsMode systemCardsBasedMode, CCommResApi* pRsrv)
{
	eVideoPartyType rsrcVideoType = eVideo_party_type_dummy;
	BYTE isHDVsw = pRsrv->GetIsHDVSW();
	if (isHDVsw)
	{
		rsrcVideoType = eVSW_video_party_type;
	}
	else //CP
	{
		eVideoQuality rsrvVidQuality = pRsrv->GetVideoQuality();
		BYTE rsrvTransferRate = pRsrv->GetConfTransferRate();
		DWORD transferRate = TranslateRsrvTransferRate(rsrvTransferRate);

		Eh264VideoModeType maxConfResolution = GetMaxConfResolution(systemCardsBasedMode, pRsrv);
		Eh264VideoModeType maxCPconfigType = ConvertEResolutionTypeToEh264VideoModeType(systemCardsBasedMode, m_CPmaxRes);
		if (maxConfResolution > maxCPconfigType)
		{
			FTRACEINTO << " CResRsrcCalculator::GetRsrcVideoType : maxConfResolution = " << eVideoModeTypeNames[maxConfResolution] << ", maxCPconfigType = " << eVideoModeTypeNames[maxCPconfigType];
			maxConfResolution = maxCPconfigType;
		}
		rsrcVideoType = GetRsrcVideoType(systemCardsBasedMode, transferRate, rsrvVidQuality, maxConfResolution); //olga

		eVideoPartyType maxRsrcTypeAccordingToSysFlag = GetMaxRsrcTypeAccordingToSysFlag(systemCardsBasedMode);
		if (maxRsrcTypeAccordingToSysFlag < rsrcVideoType)
		{
			PTRACE2(eLevelInfoNormal, "CResRsrcCalculator::GetRsrcVideoType maxRsrcTypeAccordingToSysFlag smaller than the Res rsrc type. System type = ", eVideoPartyTypeNames[maxRsrcTypeAccordingToSysFlag]);
			rsrcVideoType = maxRsrcTypeAccordingToSysFlag;
		}

	}
	FTRACEINTO << "RsrcVideoType:" << eVideoPartyTypeNames[rsrcVideoType];
	return rsrcVideoType;
}

/////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CResRsrcCalculator::GetRsrcVideoType(eSystemCardsMode systemCardsBasedMode, DWORD callRate, eVideoQuality videoQuality, Eh264VideoModeType maxVideoMode)
{
	Eh264VideoModeType h264VideoModeType = GetVideoMode(systemCardsBasedMode, callRate, videoQuality, maxVideoMode, TRUE);
	eVideoPartyType videoPartyType = TranslateVideoTypeToResourceType(systemCardsBasedMode, h264VideoModeType);
	return videoPartyType;
}

/////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CResRsrcCalculator::GetMaxRsrcTypeAccordingToSysFlag(eSystemCardsMode systemCardsBasedMode)
{
	eVideoPartyType maxVideoType = eVideo_party_type_dummy;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	eProductType productType = CProcessBase::GetProcess()->GetProductType();
	string maxVidModeString = "";
	const string key = "MAX_CP_RESOLUTION";
	BOOL isMaxResolution = sysConfig->GetDataByKey(key, maxVidModeString);
	if (isMaxResolution)
	{
		int maxVidModeStringLen = maxVidModeString.length();
		if ((maxVidModeStringLen >= 3) && (maxVidModeString.compare("CIF") == 0))
		{
			maxVideoType = eCP_H264_upto_CIF_video_party_type;
		}
		else if ((maxVidModeStringLen >= 4) && (maxVidModeString.compare("SD30") == 0))
		{
			maxVideoType = eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type;
		}
		else if (((maxVidModeStringLen >= 2) && (maxVidModeString.compare("HD") == 0)) || ((maxVidModeStringLen >= 5) && (maxVidModeString.compare("HD720") == 0)))
		{
			maxVideoType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
		}
		else if ((maxVidModeStringLen >= 6) && (maxVidModeString.compare("HD1080") == 0))
		{
			if (systemCardsBasedMode == eSystemCardsMode_breeze && productType != eProductTypeCallGeneratorSoftMCU)
			{
				maxVideoType = eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type;
			}
			else // MPM-Rx
			{
				maxVideoType = eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type;
			}
		}

	}
	return maxVideoType;
}

/////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CResRsrcCalculator::TranslateVideoTypeToResourceType(eSystemCardsMode systemCardsBasedMode, Eh264VideoModeType vidType)
{
	eVideoPartyType partyVideoType = eVideo_party_type_none;

	switch (vidType)
	{
		case eCIF30:
		case eCIF60:
		{
			partyVideoType = eCP_H264_upto_CIF_video_party_type;
			break;
		}
		case e2CIF30:
		{
			partyVideoType = eCP_H264_upto_CIF_video_party_type;
			break;
		}
		case eWCIF60:
		case eSD30:
		case eW4CIF30:
		{
			partyVideoType = eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type;
			break;
		}
		case eSD60:
		case eHD720Symmetric:
		{
			partyVideoType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
			break;
		}
		case eHD720At60Symmetric:
		{
			partyVideoType = eCP_H264_upto_HD720_60FS_Symmetric_video_party_type;
			break;
		}
		case eHD1080Symmetric:
		{
			partyVideoType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
			break;
		}
		case eHD1080At60Asymmetric:
		{
			partyVideoType = eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type;
			break;
		}
		case eHD1080At60Symmetric:
		{
			partyVideoType = eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type;
			break;
		}
		default:
		{
			FPTRACE2INT(eLevelInfoNormal, "TranslateVideoTypeToResourceType : unknown vidType = ", (DWORD )vidType);
		}
			//eCOP_party_type  ???
	}
	return partyVideoType;
}

/////////////////////////////////////////////////////////////////////////////////
Eh264VideoModeType CResRsrcCalculator::GetMaxConfResolution(eSystemCardsMode systemCardsBasedMode, CCommResApi* pRsrv)
{
	// Tsahi TODO: consider remove systemCardsBasedMode param
	eProductType productType = CProcessBase::GetProcess()->GetProductType();
	Eh264VideoModeType maxConfResolution = eLasth264VideoMode; //olga - what a default value should be?
	EVideoResolutionType rsrvVideoResolutionType = (EVideoResolutionType)pRsrv->GetConfMaxResolution();
	eVideoQuality rsrvVidQuality = pRsrv->GetVideoQuality();
	FTRACEINTO << "RsrvVideoResolutionType:" << eVideoResolutionTypeNames[rsrvVideoResolutionType] << ", RsrvVidQuality:" << rsrvVidQuality;

	switch (rsrvVideoResolutionType)
	{
		case eCIF_Res:
		{
			maxConfResolution = eCIF30;
			break;
		}

		case eSD_Res:
		{
			if (eVideoQualityMotion == rsrvVidQuality)
			{
				maxConfResolution = eSD60;
			}
			else
			{
				maxConfResolution = eSD30;
			}
			break;
		}

		case eHD720_Res:
		{
			maxConfResolution = eHD720At60Symmetric;
			break;
		}

		case eHD1080_Res:
		{
			if (eVideoQualityMotion == rsrvVidQuality)
				maxConfResolution = eHD720At60Symmetric;
			else
				maxConfResolution = eHD1080Symmetric;
			break;
		}

		case eHD1080p60_Res:
		case eAuto_Res: //to check ?
		{
			if (eVideoQualityMotion == rsrvVidQuality)
			{
				if (systemCardsBasedMode == eSystemCardsMode_mpmrx)
					maxConfResolution = eHD1080At60Symmetric;
				else if (systemCardsBasedMode == eSystemCardsMode_breeze && eProductFamilyRMX == ::ProductTypeToProductFamily(productType))
					maxConfResolution = eHD1080At60Asymmetric;
				else
					maxConfResolution = eHD720At60Symmetric;
			}
			else
			{
				maxConfResolution = eHD1080Symmetric;
			}
			break;
		}
	}

	FPTRACE2(eLevelInfoNormal, "CResRsrcCalculator::GetMaxConfResolution - MaxConfResolution:", eVideoModeTypeNames[maxConfResolution]);
	return maxConfResolution;
}

/////////////////////////////////////////////////////////////////////////////////
Eh264VideoModeType CResRsrcCalculator::GetMaxCPResolution( eSystemCardsMode systemCardsBasedMode )
{
	Eh264VideoModeType maxCPconfigType = ConvertEResolutionTypeToEh264VideoModeType( systemCardsBasedMode, m_CPmaxRes );

	return maxCPconfigType;
}

/////////////////////////////////////////////////////////////////////////////////
EVideoResolutionType CResRsrcCalculator::GetMaxCPResolutionType( eSystemCardsMode systemCardsBasedMode )
{
	EVideoResolutionType maxCPconfigType = ConvertEResolutionTypeToEVideoResolutionType( systemCardsBasedMode, m_CPmaxRes );

	return maxCPconfigType;
}

/////////////////////////////////////////////////////////////////////////////////
DWORD CResRsrcCalculator::GetRateAccordingToVideoModeType(eSystemCardsMode systemCardsBasedMode, eVideoQuality videoQuality, Eh264VideoModeType H264VideoMode)
{
	bool isMotion = (eVideoQualityMotion == videoQuality);
	WORD card_tbl_index = systemCardsBasedMode - 1;
	int j = 0;
	DWORD curThresholdRate = 0;

	ostringstream ostr;
	ostr << "CResRsrcCalculator::GetRateAccordingToVideoModeType : systemCards mode = " << ::GetSystemCardsModeStr(systemCardsBasedMode) << ", quality = " << (isMotion ? "Motion" : "Sharpness") << ", H264VideoMode = " << eVideoModeTypeNames[H264VideoMode];

	FPTRACE(eLevelInfoNormal, ostr.str().c_str());

	if (e_manual == m_ConfigType)
	{
		H264VideoModeThresholdStruct* pTblTemp = NULL;

		if (isMotion)
			pTblTemp = g_dynamicMotionVideoModeThresholdTbl[card_tbl_index];
		else
			pTblTemp = g_dynamicSharpnessVideoModeThresholdTbl[card_tbl_index];

		for (j = 0; j < GetMaxNumSliderRates(isMotion); j++)
		{	// Look for a resolution that configured for the current predefined type
			if (pTblTemp[j].videoModeType == H264VideoMode)
			{
				curThresholdRate = pTblTemp[j].thresholdBitrate;
			}
		}
	}
	else
	{
		ResolutionThresholdStruct* pThresholdTblTemp = NULL;
		EThresholdBitRate curBitRate = e64000;

		if (isMotion)
			pThresholdTblTemp = g_defaultMotionThresholdTbl[card_tbl_index];
		else
			pThresholdTblTemp = g_defaultSharpnessThresholdTbl[card_tbl_index];

		for (int j = 0; j < GetMaxNumSliderRates(isMotion); j++)
		{	// Look for a resolution that configured for the current predefined type
			if (pThresholdTblTemp[j].videoModeType == H264VideoMode)
			{
				switch (m_ConfigType)
				{
					case e_balanced:
						curBitRate = pThresholdTblTemp[j].balanced_bitrate;
						break;
					case e_resource_optimized:
						curBitRate = pThresholdTblTemp[j].resource_optimized_bitrate;
						break;
					case e_user_exp_optimized:
						curBitRate = pThresholdTblTemp[j].user_optimized_bitrate;
						break;
					case e_hi_profile_optimized:
						curBitRate = pThresholdTblTemp[j].hi_profile_optimized_bitrate;
						break;
					default:
						// Note: some enumeration value are not handled in switch. Add default to suppress warning.
						break;
				}
			}
		}

		curThresholdRate = ConvertEThresholdBitRate(curBitRate);

	}

	return curThresholdRate;
}

/////////////////////////////////////////////////////////////////////////////////
// The function looks for Eh264VideoModeType according to the dynamic resolution slider
Eh264VideoModeType CResRsrcCalculator::GetVideoMode(eSystemCardsMode systemCardsBasedMode, DWORD callRate, eVideoQuality videoQuality, Eh264VideoModeType maxVidMode, BOOL isHighProfile)
{
	eProductType productType = CProcessBase::GetProcess()->GetProductType();

	if (maxVidMode >= eLasth264VideoMode)
	{
		maxVidMode = (eSystemCardsMode_breeze == systemCardsBasedMode && eProductFamilyRMX == ::ProductTypeToProductFamily(productType)) ? eHD1080At60Asymmetric : eHD1080At60Symmetric;
		FTRACEINTO << "Received illegal maxVideoMode is changed to default, maxVideoMode:" << eVideoModeTypeNames[maxVidMode];
	}

	bool isMotion = (eVideoQualityMotion == videoQuality);
	bool isSharpn = (eVideoQualitySharpness == videoQuality);

	FTRACEINTO
		<< " 1 "
		<< "- systemCardsBasedMode:" << ::GetSystemCardsModeStr(systemCardsBasedMode)
		<< ", callRate:"             << callRate
		<< ", quality:"              << (isMotion ? "Motion" : (isSharpn ? "Sharpness" : "Auto"))
		<< ", maxVideoMode:"         << eVideoModeTypeNames[maxVidMode]
		<< ", isHighProfile:"        << (isHighProfile ? "True" : "False")
		<< ", isHDenabled:"          << (m_isHD_enabled ? "True" : "False")
		<< ", isHD1080enabled in SoftMCU:" << (m_isHD1080_enabled ? "True" : "False");

	Eh264VideoModeType maxVideoMode = maxVidMode;
	if (!m_isHD_enabled && maxVideoMode > eW4CIF30)
	{
		maxVideoMode = eW4CIF30;
		FTRACEINTO << " HD resolution is disabled, so set maxVideoMode:" << eVideoModeTypeNames[maxVideoMode];
	}

	if (!m_isHD1080_enabled && maxVideoMode > eHD720Symmetric)
	{
		maxVideoMode = eHD720Symmetric;
		FTRACEINTO << " HD1080 resolution in SoftMCU is disabled, so set maxVideoMode:" << eVideoModeTypeNames[maxVideoMode];
	}

	Eh264VideoModeType h264VidMode = eLasth264VideoMode;
	Eh264VideoModeType videoModeAccordingToResSlider = eLasth264VideoMode;

	WORD card_tbl_index = systemCardsBasedMode - 1;

	H264VideoModeThresholdStruct* pSpecificDecisionMatrix = NULL;

	const char* isHighProfileStr = (isHighProfile) ? " High Profile " : " Base Profile ";

	DWORD curThresholdRate = 0;
	DWORD minThresholdRate = 0;

	if (e_manual == m_ConfigType)
	{
		H264VideoModeThresholdStruct* pTblTemp = NULL;

		if (isHighProfile)
			pTblTemp = (isMotion ? g_dynamicHighProfileMotionThresholdTbl[card_tbl_index] : g_dynamicHighProfileSharpnessThresholdTbl[card_tbl_index]);
		else
			pTblTemp = (isMotion ? g_dynamicMotionVideoModeThresholdTbl[card_tbl_index] : g_dynamicSharpnessVideoModeThresholdTbl[card_tbl_index]);

		minThresholdRate = pTblTemp[0].thresholdBitrate;

		if (minThresholdRate > callRate) // The rate is smaller then the min threshold
			FTRACEINTO << " 1 - The callRate:" << callRate << " is smaller then the minThresholdRate:" << minThresholdRate;

		int j = 0;
		for (int i = 0; i < GetMaxNumSliderRates(isMotion); i++)
		{ // Look for a resolution type that configured by resolution slider for the callRate and videoQuality
			j = i + 1;
			curThresholdRate = pTblTemp[j].thresholdBitrate;

			if ((j == GetMaxNumSliderRates(isMotion)) || (curThresholdRate > callRate) || (pTblTemp[j].videoModeType > maxVideoMode))
			{
				videoModeAccordingToResSlider = pTblTemp[i].videoModeType;
				FTRACEINTO << " 2 - According to" << isHighProfileStr << "manual mode <==> chosen " << eVideoModeTypeNames[videoModeAccordingToResSlider] << " video mode";
				break;
			}
			j++;
		}
	}
	else
	{
		ResolutionThresholdStruct* pThresholdTblTemp = NULL;
		if (isHighProfile)
			pThresholdTblTemp = (isMotion ? g_defaultHighProfileMotionThresholdTbl[card_tbl_index] : g_defaultHighProfileSharpnessThresholdTbl[card_tbl_index]);
		else
			pThresholdTblTemp = (isMotion ? g_defaultMotionThresholdTbl[card_tbl_index] : g_defaultSharpnessThresholdTbl[card_tbl_index]);

		EThresholdBitRate curBitRate = e64000;

		int j = 0;
		for (int i = 0; i < GetMaxNumSliderRates(isMotion); i++)
		{ // Look for a resolution that configured for the current predefined type
			j = i + 1;

			switch (m_ConfigType)
			{
				case e_balanced:
					curBitRate = pThresholdTblTemp[j].balanced_bitrate;
					break;
				case e_resource_optimized:
					curBitRate = pThresholdTblTemp[j].resource_optimized_bitrate;
					break;
				case e_user_exp_optimized:
					curBitRate = pThresholdTblTemp[j].user_optimized_bitrate;
					break;
				case e_hi_profile_optimized:
					curBitRate = pThresholdTblTemp[j].hi_profile_optimized_bitrate;
					break;
				default:
					// Note: some enumeration value are not handled in switch. Add default to suppress warning.
					break;
			}

			if ((j == GetMaxNumSliderRates(isMotion)) || (ConvertEThresholdBitRate(curBitRate) > callRate) || (pThresholdTblTemp[j].videoModeType > maxVideoMode))
			{
				videoModeAccordingToResSlider = pThresholdTblTemp[i].videoModeType;
				FTRACEINTO << " 2 - According to" << isHighProfileStr << eResolutionConfigTypeNames[m_ConfigType] << " mode <==> chosen " << eVideoModeTypeNames[videoModeAccordingToResSlider] << " video mode";
				break;
			}
		}
	}

	// Look for a specific resolution
	switch (videoQuality)
	{
		case eVideoQualityMotion:
		{
			switch (videoModeAccordingToResSlider)
			{
				case eCIF30:
				case eCIF60:
				{
					pSpecificDecisionMatrix = g_CIF_MotionVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				case eWCIF60:
				case eW4CIF30:
				case e2CIF30:
				case eSD15:
				{
					pSpecificDecisionMatrix = g_SD_MotionVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				case eSD30:
				{
					pSpecificDecisionMatrix = g_SD_MotionVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				case eSD60:
				case eHD720Asymmetric:
				case eHD720Symmetric:
				{
					pSpecificDecisionMatrix = g_HD720_MotionVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				case eHD720At60Asymmetric:
				case eHD720At60Symmetric:
				case eHD1080Asymmetric:
				case eHD1080Symmetric:
				{
					pSpecificDecisionMatrix = g_HD1080_MotionVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				case eHD1080At60Asymmetric:
				case eHD1080At60Symmetric:
				{
					pSpecificDecisionMatrix = g_HD1080_60_MotionVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				default:
					// Note: some enumeration value are not handled in switch. Add default to suppress warning.
					break;
			}
		}
			break;

		case eVideoQualitySharpness:
		{
			switch (videoModeAccordingToResSlider)
			{
				case eCIF30:
				case eCIF60:
				{
					pSpecificDecisionMatrix = g_CIF_SharpnessVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				case eWCIF60:
				case eW4CIF30:
				case e2CIF30:
				case eSD15:
				{
					pSpecificDecisionMatrix = g_SD_SharpnessVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				case eSD30:
				{
					pSpecificDecisionMatrix = g_SD_SharpnessVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				case eSD60:
				case eHD720Asymmetric:
				case eHD720Symmetric:
				{
					pSpecificDecisionMatrix = g_HD720_SharpnessVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				case eHD720At60Asymmetric:
				case eHD720At60Symmetric:
				case eHD1080Asymmetric:
				case eHD1080Symmetric:
				{
					pSpecificDecisionMatrix = g_HD1080_SharpnessVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				case eHD1080At60Asymmetric:
				case eHD1080At60Symmetric:
				{
					pSpecificDecisionMatrix = g_HD1080_60_SharpnessVideoModeThresholdTbl[card_tbl_index];
					break;
				}
				default:
					// Note: some enumeration value are not handled in switch. Add default to suppress warning.
					break;
			}
		}
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}

	if (pSpecificDecisionMatrix)
	{
		int j = 0;
		minThresholdRate = pSpecificDecisionMatrix[0].thresholdBitrate;
		if (minThresholdRate > callRate) // The rate is smaller then the min threshold
		{
			FTRACEINTO << " 3 - The callRate:" << callRate << " is smaller then the minThresholdRate:" << minThresholdRate;
			return eLasth264VideoMode;
		}
		for (int i = 0; i < eMAX_NUM_THRESHOLD_RATES; i++)
		{
			j = i + 1;
			curThresholdRate = pSpecificDecisionMatrix[j].thresholdBitrate;
			if ((j == eMAX_NUM_THRESHOLD_RATES) || (curThresholdRate > callRate) || (pSpecificDecisionMatrix[j].videoModeType > maxVideoMode))
			{
				FTRACEINTO << " 4 - callRate:" << callRate << ", videoModeType:" << eVideoModeTypeNames[pSpecificDecisionMatrix[i].videoModeType];
				return pSpecificDecisionMatrix[i].videoModeType;
			}
			j++;
		}
	}
	else
		FPTRACE(eLevelError, " 5 - Invalid H264 video mode");

	return h264VidMode;
}

/////////////////////////////////////////////////////////////////////////////////
BOOL CResRsrcCalculator::UpdateResolutionConfiguration(eSystemCardsMode systemCardsBasedMode, CResolutionSliderDetails* pSliderDetails)
{
	BOOL wasChanged = FALSE, configTypeCPmaxResChanged = FALSE;
	WORD card_tbl_index = systemCardsBasedMode - 1;
	if (pSliderDetails)
	{
		//update max cp resolution value
		//SRS flexera licensing:RPCS_CIF_PLUS	When this license is disabled the MCU maximum CP resolution should be set to CIF
		EResolutionType newCPmaxRes = (EResolutionType)pSliderDetails->m_CPmaxRes;
		if (m_CPmaxRes != newCPmaxRes && m_CPLicensing_CIF_Plus)
		{
			const char *descrCPmaxRes = NULL, *descrCPmaxResNew = NULL;
			BYTE res = CStringsMaps::GetDescription(RESOLUTION_THRESHOLD_TYPE_ENUM, m_CPmaxRes, &descrCPmaxRes);
			res = CStringsMaps::GetDescription(RESOLUTION_THRESHOLD_TYPE_ENUM, newCPmaxRes, &descrCPmaxResNew);

			FTRACEINTO << "CPmaxRes has been changed from \"" << descrCPmaxRes << "\" to be \"" << descrCPmaxResNew << "\"";
			m_CPmaxRes = newCPmaxRes;
			configTypeCPmaxResChanged = TRUE;
			wasChanged = TRUE;
		}
		else if (FALSE == m_CPLicensing_CIF_Plus)
		{
			m_CPmaxRes = e_cif30;
		}

		// update a configuration type value (manual or predefined)
		EResolutionConfigType newConfigType = (EResolutionConfigType)pSliderDetails->m_ConfigType;
		if (m_ConfigType != newConfigType)
		{
			FTRACEINTO << "ConfigType has been changed from \"" << eResolutionConfigTypeNames[m_ConfigType] << "\" to be \"" << eResolutionConfigTypeNames[newConfigType] << "\"";
			m_ConfigType = newConfigType;
			configTypeCPmaxResChanged = TRUE;
			wasChanged = TRUE;
		}

		if (e_manual == m_ConfigType)
		{
			H264VideoModeThresholdStruct* pTblMotion = g_dynamicMotionVideoModeThresholdTbl[card_tbl_index];
			H264VideoModeThresholdStruct* pTblSharpness = g_dynamicSharpnessVideoModeThresholdTbl[card_tbl_index];

			DWORD sd_motion_rate = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_SD_MotionMinRate);
			if (pTblMotion[1].thresholdBitrate != sd_motion_rate)
			{
				pTblMotion[1].thresholdBitrate = sd_motion_rate;
				wasChanged = TRUE;
			}
			DWORD hd720_motion_rate = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD720_MotionMinRate);
			if (pTblMotion[2].thresholdBitrate != hd720_motion_rate)
			{
				pTblMotion[2].thresholdBitrate = hd720_motion_rate;
				wasChanged = TRUE;
			}
			DWORD hd1080_motion_rate = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD1080_MotionMinRate);
			if (pTblMotion[3].thresholdBitrate != hd1080_motion_rate)
			{
				pTblMotion[3].thresholdBitrate = hd1080_motion_rate;
				wasChanged = TRUE;
			}
			DWORD hd1080_60_motion_rate = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD1080_60_MotionMinRate);
			if (pTblMotion[4].thresholdBitrate != hd1080_60_motion_rate)
			{
				pTblMotion[4].thresholdBitrate = hd1080_60_motion_rate;
				wasChanged = TRUE;
			}

			DWORD sd_sharp_rate = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_SD_SharpnessMinRate);
			if (pTblSharpness[1].thresholdBitrate != sd_sharp_rate)
			{
				pTblSharpness[1].thresholdBitrate = sd_sharp_rate;
				wasChanged = TRUE;
			}
			DWORD hd720_sharp_rate = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD720_SharpnessMinRate);
			if (pTblSharpness[2].thresholdBitrate != hd720_sharp_rate)
			{
				pTblSharpness[2].thresholdBitrate = hd720_sharp_rate;
				wasChanged = TRUE;
			}
			DWORD hd1080_sharp_rate = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD1080_SharpnessMinRate);
			if (pTblSharpness[3].thresholdBitrate != hd1080_sharp_rate)
			{
				pTblSharpness[3].thresholdBitrate = hd1080_sharp_rate;
				wasChanged = TRUE;
			}
			DWORD hd1080_60_sharp_rate = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD1080_60_SharpnessMinRate);
			if (pTblSharpness[4].thresholdBitrate != hd1080_60_sharp_rate)
			{
				pTblSharpness[4].thresholdBitrate = hd1080_60_sharp_rate;
				wasChanged = TRUE;
			}

			// HighProfile
			H264VideoModeThresholdStruct* pTblMotionHighProfile = g_dynamicHighProfileMotionThresholdTbl[card_tbl_index];
			H264VideoModeThresholdStruct* pTblSharpnessHighProfile = g_dynamicHighProfileSharpnessThresholdTbl[card_tbl_index];

			DWORD sd_motion_rate_hp = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_SD_MotionMinRateHighProfile);
			if (pTblMotionHighProfile[1].thresholdBitrate != sd_motion_rate_hp)
			{
				pTblMotionHighProfile[1].thresholdBitrate = sd_motion_rate_hp;
				wasChanged = TRUE;
			}
			DWORD hd720_motion_rate_hp = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD720_MotionMinRateHighProfile);
			if (pTblMotionHighProfile[2].thresholdBitrate != hd720_motion_rate_hp)
			{
				pTblMotionHighProfile[2].thresholdBitrate = hd720_motion_rate_hp;
				wasChanged = TRUE;
			}
			DWORD hd1080_motion_rate_hp = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD1080_MotionMinRateHighProfile);
			if (pTblMotionHighProfile[3].thresholdBitrate != hd1080_motion_rate_hp)
			{
				pTblMotionHighProfile[3].thresholdBitrate = hd1080_motion_rate_hp;
				wasChanged = TRUE;
			}
			DWORD hd1080_60_motion_rate_hp = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD1080_60_MotionMinRateHighProfile);
			if (pTblMotionHighProfile[4].thresholdBitrate != hd1080_60_motion_rate_hp)
			{
				pTblMotionHighProfile[4].thresholdBitrate = hd1080_60_motion_rate_hp;
				wasChanged = TRUE;
			}

			DWORD sd_sharp_rate_hp = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_SD_SharpnessMinRateHighProfile);
			if (pTblSharpnessHighProfile[1].thresholdBitrate != sd_sharp_rate_hp)
			{
				pTblSharpnessHighProfile[1].thresholdBitrate = sd_sharp_rate_hp;
				wasChanged = TRUE;
			}
			DWORD hd720_sharp_rate_hp = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD720_SharpnessMinRateHighProfile);
			if (pTblSharpnessHighProfile[2].thresholdBitrate != hd720_sharp_rate_hp)
			{
				pTblSharpnessHighProfile[2].thresholdBitrate = hd720_sharp_rate_hp;
				wasChanged = TRUE;
			}
			DWORD hd1080_sharp_rate_hp = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD1080_SharpnessMinRateHighProfile);
			if (pTblSharpnessHighProfile[3].thresholdBitrate != hd1080_sharp_rate_hp)
			{
				pTblSharpnessHighProfile[3].thresholdBitrate = hd1080_sharp_rate_hp;
				wasChanged = TRUE;
			}
			DWORD hd1080_60_sharp_rate_hp = ConvertEThresholdBitRate((EThresholdBitRate)pSliderDetails->m_HD1080_60_SharpnessMinRateHighProfile);
			if (pTblSharpnessHighProfile[4].thresholdBitrate != hd1080_60_sharp_rate_hp)
			{
				pTblSharpnessHighProfile[4].thresholdBitrate = hd1080_60_sharp_rate_hp;
				wasChanged = TRUE;
			}
		}
	}
	else
		FPTRACE(eLevelError, "CResRsrcCalculator:UpdateResolutionConfiguration : pSliderDetails is NULL pointer");

	if (configTypeCPmaxResChanged && m_ConfigType != e_manual)
		DumpDefaultThresholdTbl(systemCardsBasedMode);
	else if (wasChanged)
		DumpDynamicThresholdTbl(systemCardsBasedMode);

	FTRACEINTO << "mode is " << eResolutionConfigTypeNames[m_ConfigType] << ", slider configuration " << (wasChanged ? "was changed " : "was not changed");
	return wasChanged;
}

/////////////////////////////////////////////////////////////////////////////////
STATUS CResRsrcCalculator::ReadResolutionConfigurationFromFile(eSystemCardsMode systemCardsBasedMode, BOOL* isWasChanged)
{
	FTRACEINTO << "CardsMode:" << GetSystemCardsModeStr(systemCardsBasedMode);

	CSetResolutionSliderDetails pResolutionSlider(systemCardsBasedMode);
	STATUS ret_stat = pResolutionSlider.ReadXmlFile(RESOLUTION_SLIDER_CONFIG_FILE_NAME);
	if (STATUS_OK == ret_stat)
	{
		eProductType productType = CProcessBase::GetProcess()->GetProductType();

		switch (productType)
		{
			case eProductTypeNinja:
				m_CPmaxLineRate = e6144000; //eFeatureLineRate_6M
				break;

			default:
				m_CPmaxLineRate = e4096000;
				break;
		}

		BOOL wasChanged = UpdateResolutionConfiguration(systemCardsBasedMode, &pResolutionSlider);
		if (isWasChanged)
			*isWasChanged = wasChanged;
	}
	return ret_stat;
}

/////////////////////////////////////////////////////////////////////////////////
STATUS CResRsrcCalculator::SaveResolutionConfigurationToFile(eSystemCardsMode systemCardsMode, BOOL useSysFlag)
{
	FTRACEINTO << "SystemCardsMode:" << systemCardsMode << ", UseSysFlag:" << (int)useSysFlag;

	CSetResolutionSliderDetails pResolutionSlider(systemCardsMode);
	GetResolutionSliderDetails(systemCardsMode, &pResolutionSlider);
	if (useSysFlag)
	{
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		string maxVidModeString = "";
		const string key = "MAX_CP_RESOLUTION";
		BOOL isMaxResolution = sysConfig->GetDataByKey(key, maxVidModeString);
		if (isMaxResolution)
		{
			int maxVidModeStringLen = maxVidModeString.length();
			if ((maxVidModeStringLen >= 3) && (maxVidModeString.compare("CIF") == 0))
			{
				pResolutionSlider.m_CPmaxRes = e_cif30;
			}
			else if ((maxVidModeStringLen >= 4) && (maxVidModeString.compare("SD15") == 0))
			{
				pResolutionSlider.m_CPmaxRes = e_sd30;
			}
			else if ((maxVidModeStringLen >= 4) && (maxVidModeString.compare("SD30") == 0))
			{
				pResolutionSlider.m_CPmaxRes = e_sd30;
			}
			else if (((maxVidModeStringLen >= 2) && (maxVidModeString.compare("HD") == 0)) || ((maxVidModeStringLen >= 5) && (maxVidModeString.compare("HD720") == 0)))
			{
				pResolutionSlider.m_CPmaxRes = e_hd720p60;
			}
			else if ((maxVidModeStringLen >= 6) && (maxVidModeString.compare("HD1080") == 0))
			{
				pResolutionSlider.m_CPmaxRes = pResolutionSlider.GetMaxResForCardType();
			}
		}
	}

	STATUS ret_stat = pResolutionSlider.WriteXmlFile(RESOLUTION_SLIDER_CONFIG_FILE_NAME, "SET_RESOLUTIONS_SET"); //to check root node
	return ret_stat;
}

/////////////////////////////////////////////////////////////////////////////////
STATUS CResRsrcCalculator::SetResolutionAccordingToLicensing(BOOL bCPLicensing_CIF_Plus, eSystemCardsMode systemCardMode)
{
	STATUS ret_stat = STATUS_OK;
	SetLicensingCifPlus(bCPLicensing_CIF_Plus);
	TRACEINTO << "CPLicensing_CIF_Plus:" << (int)m_CPLicensing_CIF_Plus;
	if (FALSE == m_CPLicensing_CIF_Plus)
	{
		PTRACE(eLevelInfoNormal, "CResRsrcCalculator::SetResolutionAccordingToLicensing m_CPmaxRes should be set to e_cif30 recording to Licensing_CIF_Plus");
		m_CPmaxRes = e_cif30;
		CSetResolutionSliderDetails pResolutionSlider(systemCardMode);
		GetResolutionSliderDetails(systemCardMode, &pResolutionSlider);
		pResolutionSlider.m_CPmaxRes = m_CPmaxRes;
		ret_stat = pResolutionSlider.WriteXmlFile(RESOLUTION_SLIDER_CONFIG_FILE_NAME, "SET_RESOLUTIONS_SET");
	}
	return ret_stat;
}

/////////////////////////////////////////////////////////////////////////////////
void CResRsrcCalculator::GetResolutionSliderDetails(eSystemCardsMode systemCardsBasedMode, CResolutionSliderDetails* pResponse) const
{
	if (!pResponse)
	{
		PTRACE(eLevelInfoNormal, "ResRsrcCalculator::GetResolutionSliderDetails - No Response object!!! ");
		return;
	}
	WORD card_tbl_index = systemCardsBasedMode - 1;
	H264VideoModeThresholdStruct* pTblMotion = g_dynamicMotionVideoModeThresholdTbl[card_tbl_index];
	H264VideoModeThresholdStruct* pTblSharpness = g_dynamicSharpnessVideoModeThresholdTbl[card_tbl_index];

	pResponse->m_CPmaxRes = m_CPmaxRes;
	pResponse->m_CPmaxLineRate = (DWORD)m_CPmaxLineRate;
	pResponse->m_ConfigType = m_ConfigType;

	pResponse->m_SD_MotionMinRate = ConvertToEThresholdBitRate(pTblMotion[eSD_Res - 1].thresholdBitrate);
	pResponse->m_HD720_MotionMinRate = ConvertToEThresholdBitRate(pTblMotion[eHD720_Res - 1].thresholdBitrate);
	pResponse->m_HD1080_MotionMinRate = ConvertToEThresholdBitRate(pTblMotion[eHD1080_Res - 1].thresholdBitrate);
	pResponse->m_HD1080_60_MotionMinRate = ConvertToEThresholdBitRate(pTblMotion[eHD1080p60_Res - 1].thresholdBitrate);

	pResponse->m_SD_SharpnessMinRate = ConvertToEThresholdBitRate(pTblSharpness[eSD_Res - 1].thresholdBitrate);
	pResponse->m_HD720_SharpnessMinRate = ConvertToEThresholdBitRate(pTblSharpness[eHD720_Res - 1].thresholdBitrate);
	pResponse->m_HD1080_SharpnessMinRate = ConvertToEThresholdBitRate(pTblSharpness[eHD1080_Res - 1].thresholdBitrate);
	pResponse->m_HD1080_60_SharpnessMinRate = ConvertToEThresholdBitRate(pTblSharpness[eHD1080p60_Res - 1].thresholdBitrate);

	// HighProfile
	H264VideoModeThresholdStruct* pTblMotionHighProfile = g_dynamicHighProfileMotionThresholdTbl[card_tbl_index];
	H264VideoModeThresholdStruct* pTblSharpnessHighProfile = g_dynamicHighProfileSharpnessThresholdTbl[card_tbl_index];

	pResponse->m_SD_MotionMinRateHighProfile = ConvertToEThresholdBitRate(pTblMotionHighProfile[eSD_Res - 1].thresholdBitrate);
	pResponse->m_HD720_MotionMinRateHighProfile = ConvertToEThresholdBitRate(pTblMotionHighProfile[eHD720_Res - 1].thresholdBitrate);
	pResponse->m_HD1080_MotionMinRateHighProfile = ConvertToEThresholdBitRate(pTblMotionHighProfile[eHD1080_Res - 1].thresholdBitrate);
	pResponse->m_HD1080_60_MotionMinRateHighProfile = ConvertToEThresholdBitRate(pTblMotionHighProfile[eHD1080p60_Res - 1].thresholdBitrate);

	pResponse->m_SD_SharpnessMinRateHighProfile = ConvertToEThresholdBitRate(pTblSharpnessHighProfile[eSD_Res - 1].thresholdBitrate);
	pResponse->m_HD720_SharpnessMinRateHighProfile = ConvertToEThresholdBitRate(pTblSharpnessHighProfile[eHD720_Res - 1].thresholdBitrate);
	pResponse->m_HD1080_SharpnessMinRateHighProfile = ConvertToEThresholdBitRate(pTblSharpnessHighProfile[eHD1080_Res - 1].thresholdBitrate);
	pResponse->m_HD1080_60_SharpnessMinRateHighProfile = ConvertToEThresholdBitRate(pTblSharpnessHighProfile[eHD1080p60_Res - 1].thresholdBitrate);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
CResolutionSliderDetails::CResolutionSliderDetails()
{
	Init( eSystemCardsMode_illegal );
}

/////////////////////////////////////////////////////////////////////////////////
CResolutionSliderDetails::CResolutionSliderDetails(eSystemCardsMode systemCardsBasedMode)
{
	Init( systemCardsBasedMode );
}

/////////////////////////////////////////////////////////////////////////////////
void CResolutionSliderDetails::Init( eSystemCardsMode systemCardsBasedMode)
{
	m_SD_MotionMinRate = e256000;
	m_HD720_MotionMinRate = e512000;
	m_HD1080_MotionMinRate = e1024000;
	m_HD1080_60_MotionMinRate = e2048000;

	m_SD_SharpnessMinRate = e256000;
	m_HD720_SharpnessMinRate = e512000;
	m_HD1080_SharpnessMinRate = e1024000;
	m_HD1080_60_SharpnessMinRate = e2048000;

	m_SD_MotionMinRateHighProfile = e128000;
	m_HD720_MotionMinRateHighProfile = e512000;
	m_HD1080_MotionMinRateHighProfile = e832000;
	m_HD1080_60_MotionMinRateHighProfile = e1024000;

	m_SD_SharpnessMinRateHighProfile = e128000;
	m_HD720_SharpnessMinRateHighProfile = e512000;
	m_HD1080_SharpnessMinRateHighProfile = e1024000;
	m_HD1080_60_SharpnessMinRateHighProfile = e2048000;

	m_CPmaxRes = e_hd1080p60;
	m_ConfigType = e_balanced;
	m_CPmaxLineRate = e4096000;

	m_cardMode =  systemCardsBasedMode;
}

/////////////////////////////////////////////////////////////////////////////////
CResolutionSliderDetails::~CResolutionSliderDetails()
{}

/////////////////////////////////////////////////////////////////////////////////
void CResolutionSliderDetails::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	FTRACEINTO << " CResolutionSliderDetails::SerializeXml ";
	PASSERT_AND_RETURN(m_cardMode > NUM_OF_SYSTEM_CARDS_MODES);

	WORD card_tbl_index = m_cardMode - 1;

	ResolutionThresholdStruct* defaultMotionThreshold = CResRsrcCalculator::g_defaultMotionThresholdTbl[card_tbl_index];
	ResolutionThresholdStruct* defaultSharpThreshold  = CResRsrcCalculator::g_defaultSharpnessThresholdTbl[card_tbl_index];

	// Tsahi TBD: need to add values for MPM-Rx
	DWORD rsrc_units1 = (CResRsrcCalculator::IsRMX1500QRatios() ? sd_rsrc_units_mpm_x20 : sd_rsrc_units_mpm_x);
	DWORD rsrc_units2 = (CResRsrcCalculator::IsRMX1500QRatios() ? hd720_rsrc_units_mpm_x20 : hd720_rsrc_units_mpm_x);
	DWORD rsrc_units3 = (CResRsrcCalculator::IsRMX1500QRatios() ? hd1080_rsrc_units_mpm_x20 : hd1080_rsrc_units_mpm_x);
	DWORD rsrc_units4 = (CResRsrcCalculator::IsRMX1500QRatios() ? hd1080p60_rsrc_units_mpm_x20 : hd1080p60_rsrc_units_mpm_x);

	CXMLDOMElement* pSet = pFatherNode->AddChildNode("RESOLUTIONS_PARAMS");
	pSet->AddChildNode("CP_MAX_LINE_RATE", m_CPmaxLineRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSet->AddChildNode("CP_MAX_RESOLUTION", m_CPmaxRes, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSet->AddChildNode("CONFIGURATION_TYPE", m_ConfigType, RESOLUTION_CONFIG_TYPE_ENUM);

	// SHARPNESS - BASE PROFILE
	CXMLDOMElement* pSharpnessNode = pSet->AddChildNode("SHARPNESS_RESOLUTIONS");
	CXMLDOMElement*  pSharpSliderParamsList = pSharpnessNode->AddChildNode("RESOLUTION_SLIDER_PARAMS_LIST");
	// SD
	CXMLDOMElement*  pSharpSliderParamsSD = pSharpSliderParamsList->AddChildNode("RESOLUTION_SLIDER_PARAMS");
	pSharpSliderParamsSD->AddChildNode("RESOLUTION_TYPE", e_sd30, RESOLUTION_THRESHOLD_TYPE_ENUM );
	pSharpSliderParamsSD->AddChildNode("RESOURCE_UNITS", rsrc_units1, _0_TO_DWORD );
	pSharpSliderParamsSD->AddChildNode("MINIMAL_RESOLUTION_RATE", m_SD_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsSD->AddChildNode("BALANCED_MODE_RATE", defaultSharpThreshold[eFirst].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsSD->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eFirst].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsSD->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eFirst].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsSD->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eFirst].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	// HD720
	CXMLDOMElement*  pSharpSliderParamsHD720 = pSharpSliderParamsList->AddChildNode("RESOLUTION_SLIDER_PARAMS");
	pSharpSliderParamsHD720->AddChildNode("RESOLUTION_TYPE", e_hd720p30, RESOLUTION_THRESHOLD_TYPE_ENUM );
	pSharpSliderParamsHD720->AddChildNode("RESOURCE_UNITS", rsrc_units2, _0_TO_DWORD );
	pSharpSliderParamsHD720->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD720_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD720->AddChildNode("BALANCED_MODE_RATE", defaultSharpThreshold[eSecond].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD720->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eSecond].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD720->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eSecond].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD720->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eSecond].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	// HD1080
	CXMLDOMElement*  pSharpSliderParamsHD1080 = pSharpSliderParamsList->AddChildNode("RESOLUTION_SLIDER_PARAMS");
	pSharpSliderParamsHD1080->AddChildNode("RESOLUTION_TYPE", e_hd1080p30, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSharpSliderParamsHD1080->AddChildNode("RESOURCE_UNITS", rsrc_units3, _0_TO_DWORD );
	pSharpSliderParamsHD1080->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD1080->AddChildNode("BALANCED_MODE_RATE", defaultSharpThreshold[eThird].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD1080->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eThird].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD1080->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eThird].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD1080->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eThird].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	// HD1080 60
	CXMLDOMElement*  pSharpSliderParamsHD1080p60 = pSharpSliderParamsList->AddChildNode("RESOLUTION_SLIDER_PARAMS");
	pSharpSliderParamsHD1080p60->AddChildNode("RESOLUTION_TYPE", e_hd1080p60, RESOLUTION_THRESHOLD_TYPE_ENUM );
	pSharpSliderParamsHD1080p60->AddChildNode("RESOURCE_UNITS", rsrc_units4, _0_TO_DWORD );
	pSharpSliderParamsHD1080p60->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_60_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM );
	pSharpSliderParamsHD1080p60->AddChildNode("BALANCED_MODE_RATE", defaultSharpThreshold[eFourth].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD1080p60->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eFourth].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD1080p60->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eFourth].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSharpSliderParamsHD1080p60->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultSharpThreshold[eFourth].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);

	// MOTION - BASE PROFILE
	CXMLDOMElement* pMotionNode = pSet->AddChildNode("MOTION_RESOLUTIONS");
	CXMLDOMElement*  pSliderParamsList = pMotionNode->AddChildNode("RESOLUTION_SLIDER_PARAMS_LIST");

	// SD
	CXMLDOMElement*  pSliderParamsSD = pSliderParamsList->AddChildNode("RESOLUTION_SLIDER_PARAMS");
	pSliderParamsSD->AddChildNode("RESOLUTION_TYPE",  e_cif60,  RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSliderParamsSD->AddChildNode("RESOURCE_UNITS", rsrc_units1, _0_TO_DWORD );
	pSliderParamsSD->AddChildNode("MINIMAL_RESOLUTION_RATE", m_SD_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsSD->AddChildNode("BALANCED_MODE_RATE", defaultMotionThreshold[eFirst].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsSD->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eFirst].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsSD->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eFirst].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsSD->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eFirst].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);

	// HD720
	CXMLDOMElement*  pSliderParamsHD720 = pSliderParamsList->AddChildNode("RESOLUTION_SLIDER_PARAMS");
	pSliderParamsHD720->AddChildNode("RESOLUTION_TYPE", e_sd60,  RESOLUTION_THRESHOLD_TYPE_ENUM );
	pSliderParamsHD720->AddChildNode("RESOURCE_UNITS", rsrc_units2, _0_TO_DWORD );
	pSliderParamsHD720->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD720_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM );
	pSliderParamsHD720->AddChildNode("BALANCED_MODE_RATE", defaultMotionThreshold[eSecond].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsHD720->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eSecond].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsHD720->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eSecond].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsHD720->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eSecond].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);

	// HD1080
	CXMLDOMElement*  pSliderParamsHD1080 = pSliderParamsList->AddChildNode("RESOLUTION_SLIDER_PARAMS");
	pSliderParamsHD1080->AddChildNode("RESOLUTION_TYPE", e_hd720p60, RESOLUTION_THRESHOLD_TYPE_ENUM );
	pSliderParamsHD1080->AddChildNode("RESOURCE_UNITS", rsrc_units3, _0_TO_DWORD );
	pSliderParamsHD1080->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM );
	pSliderParamsHD1080->AddChildNode("BALANCED_MODE_RATE", defaultMotionThreshold[eThird].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsHD1080->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eThird].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsHD1080->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eThird].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsHD1080->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eThird].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);

	// HD1080 60
	CXMLDOMElement*  pSliderParamsHD1080p60 = pSliderParamsList->AddChildNode("RESOLUTION_SLIDER_PARAMS");
	pSliderParamsHD1080p60->AddChildNode("RESOLUTION_TYPE", e_hd1080p60, RESOLUTION_THRESHOLD_TYPE_ENUM );
	pSliderParamsHD1080p60->AddChildNode("RESOURCE_UNITS", rsrc_units4, _0_TO_DWORD );
	pSliderParamsHD1080p60->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_60_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM );
	pSliderParamsHD1080p60->AddChildNode("BALANCED_MODE_RATE", defaultMotionThreshold[eFourth].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsHD1080p60->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eFourth].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsHD1080p60->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eFourth].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	pSliderParamsHD1080p60->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultMotionThreshold[eFourth].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);

	// HighProfile
	{
		ResolutionThresholdStruct* defaultMotionThresholdHP = CResRsrcCalculator::g_defaultHighProfileMotionThresholdTbl[card_tbl_index];
		ResolutionThresholdStruct* defaultSharpThresholdHP  = CResRsrcCalculator::g_defaultHighProfileSharpnessThresholdTbl[card_tbl_index];

		CXMLDOMElement* pSharpnessNodeHP = pSet->AddChildNode("SHARPNESS_HIGH_PROFILE_RESOLUTIONS");
		CXMLDOMElement*  pSharpSliderParamsListHP = pSharpnessNodeHP->AddChildNode("RESOLUTION_SLIDER_PARAMS_LIST");

		// SD
		CXMLDOMElement*  pHPSharpSliderParamsSD = pSharpSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_PARAMS");
		pHPSharpSliderParamsSD->AddChildNode("RESOLUTION_TYPE", e_sd30, RESOLUTION_THRESHOLD_TYPE_ENUM );
		pHPSharpSliderParamsSD->AddChildNode("RESOURCE_UNITS", rsrc_units1, _0_TO_DWORD );
		pHPSharpSliderParamsSD->AddChildNode("MINIMAL_RESOLUTION_RATE", m_SD_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsSD->AddChildNode("BALANCED_MODE_RATE", defaultSharpThresholdHP[eFirst].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsSD->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eFirst].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsSD->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eFirst].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsSD->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eFirst].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		// HD720
		CXMLDOMElement*  pHPSharpSliderParamsHD720 = pSharpSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_PARAMS");
		pHPSharpSliderParamsHD720->AddChildNode("RESOLUTION_TYPE", e_hd720p30, RESOLUTION_THRESHOLD_TYPE_ENUM );
		pHPSharpSliderParamsHD720->AddChildNode("RESOURCE_UNITS", rsrc_units2, _0_TO_DWORD );
		pHPSharpSliderParamsHD720->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD720_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD720->AddChildNode("BALANCED_MODE_RATE", defaultSharpThresholdHP[eSecond].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD720->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eSecond].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD720->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eSecond].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD720->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eSecond].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		// HD1080
		CXMLDOMElement*  pHPSharpSliderParamsHD1080 = pSharpSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_PARAMS");
		pHPSharpSliderParamsHD1080->AddChildNode("RESOLUTION_TYPE", e_hd1080p30, RESOLUTION_THRESHOLD_TYPE_ENUM);
		pHPSharpSliderParamsHD1080->AddChildNode("RESOURCE_UNITS", rsrc_units3, _0_TO_DWORD );
		pHPSharpSliderParamsHD1080->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD1080->AddChildNode("BALANCED_MODE_RATE", defaultSharpThresholdHP[eThird].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD1080->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eThird].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD1080->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eThird].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD1080->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eThird].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);

		// HD1080 60
		CXMLDOMElement*  pHPSharpSliderParamsHD1080hp60 = pSharpSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_PARAMS");
		pHPSharpSliderParamsHD1080hp60->AddChildNode("RESOLUTION_TYPE", e_hd1080p60, RESOLUTION_THRESHOLD_TYPE_ENUM );
		pHPSharpSliderParamsHD1080hp60->AddChildNode("RESOURCE_UNITS", rsrc_units4, _0_TO_DWORD );
		pHPSharpSliderParamsHD1080hp60->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_60_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM );
		pHPSharpSliderParamsHD1080hp60->AddChildNode("BALANCED_MODE_RATE", defaultSharpThresholdHP[eFourth].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD1080hp60->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eFourth].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD1080hp60->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eFourth].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSharpSliderParamsHD1080hp60->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultSharpThresholdHP[eFourth].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);

		// MOTION - HIGH PROFILE
		CXMLDOMElement* pMotionNodeHP = pSet->AddChildNode("MOTION_HIGH_PROFILE_RESOLUTIONS");
		CXMLDOMElement*  pSliderParamsListHP = pMotionNodeHP->AddChildNode("RESOLUTION_SLIDER_PARAMS_LIST");

		// SD
		CXMLDOMElement*  pHPSliderParamsSD = pSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_PARAMS");
		pHPSliderParamsSD->AddChildNode("RESOLUTION_TYPE",  e_cif60,  RESOLUTION_THRESHOLD_TYPE_ENUM);
		pHPSliderParamsSD->AddChildNode("RESOURCE_UNITS", rsrc_units1, _0_TO_DWORD );
		pHPSliderParamsSD->AddChildNode("MINIMAL_RESOLUTION_RATE", m_SD_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsSD->AddChildNode("BALANCED_MODE_RATE", defaultMotionThresholdHP[eFirst].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsSD->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eFirst].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsSD->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eFirst].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsSD->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eFirst].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);

		// HD720
		CXMLDOMElement*  pHPSliderParamsHD720 = pSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_PARAMS");
		pHPSliderParamsHD720->AddChildNode("RESOLUTION_TYPE", e_sd60,  RESOLUTION_THRESHOLD_TYPE_ENUM );
		pHPSliderParamsHD720->AddChildNode("RESOURCE_UNITS", rsrc_units2, _0_TO_DWORD );
		pHPSliderParamsHD720->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD720_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM );
		pHPSliderParamsHD720->AddChildNode("BALANCED_MODE_RATE", defaultMotionThresholdHP[eSecond].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsHD720->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eSecond].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsHD720->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eSecond].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsHD720->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eSecond].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);

		// HD1080
		CXMLDOMElement*  pHPSliderParamsHD1080 = pSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_PARAMS");
		pHPSliderParamsHD1080->AddChildNode("RESOLUTION_TYPE", e_hd720p60, RESOLUTION_THRESHOLD_TYPE_ENUM );
		pHPSliderParamsHD1080->AddChildNode("RESOURCE_UNITS", rsrc_units3, _0_TO_DWORD );
		pHPSliderParamsHD1080->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM );
		pHPSliderParamsHD1080->AddChildNode("BALANCED_MODE_RATE", defaultMotionThresholdHP[eThird].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsHD1080->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eThird].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsHD1080->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eThird].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsHD1080->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eThird].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);

		// HD1080 60
		CXMLDOMElement*  pHPSliderParamsHD1080hp60 = pSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_PARAMS");
		pHPSliderParamsHD1080hp60->AddChildNode("RESOLUTION_TYPE", e_hd1080p60, RESOLUTION_THRESHOLD_TYPE_ENUM );
		pHPSliderParamsHD1080hp60->AddChildNode("RESOURCE_UNITS", rsrc_units4, _0_TO_DWORD );
		pHPSliderParamsHD1080hp60->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_60_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM );
		pHPSliderParamsHD1080hp60->AddChildNode("BALANCED_MODE_RATE", defaultMotionThresholdHP[eFourth].balanced_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsHD1080hp60->AddChildNode("RESOURCE_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eFourth].resource_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsHD1080hp60->AddChildNode("USER_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eFourth].user_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
		pHPSliderParamsHD1080hp60->AddChildNode("HIGH_PROFILE_OPTIMIZED_MODE_RATE", defaultMotionThresholdHP[eFourth].hi_profile_optimized_bitrate, RESOLUTION_THRESHOLD_RATE_ENUM);
	}
}

/////////////////////////////////////////////////////////////////////////////////
int	CResolutionSliderDetails::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action)
{
	FTRACEINTO << " CResolutionSliderDetails::DeSerializeXml - shouldn't be called";
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////
CSetResolutionSliderDetails::CSetResolutionSliderDetails() : CResolutionSliderDetails()
{}

/////////////////////////////////////////////////////////////////////////////////
CSetResolutionSliderDetails::CSetResolutionSliderDetails(eSystemCardsMode systemCardsBasedMode) :
		CResolutionSliderDetails(systemCardsBasedMode)
{}

/////////////////////////////////////////////////////////////////////////////////
void CSetResolutionSliderDetails::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	FTRACEINTO << " CSetResolutionSliderDetails::SerializeXml ";

	CXMLDOMElement* pSet = pFatherNode->AddChildNode("SET_RESOLUTIONS_PARAMS");

	pSet->AddChildNode("CP_MAX_RESOLUTION", m_CPmaxRes, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSet->AddChildNode("CONFIGURATION_TYPE", m_ConfigType, RESOLUTION_CONFIG_TYPE_ENUM);

	// SHARPNESS - BASE PROFILE
	CXMLDOMElement* pSharpnessNode = pSet->AddChildNode("SET_SHARPNESS_RESOLUTIONS");
	CXMLDOMElement* pSharpSliderParamsList = pSharpnessNode->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS_LIST");
	// SD
	CXMLDOMElement* pSharpSliderParamsSD = pSharpSliderParamsList->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
	EResolutionType resType1 = (eSystemCardsMode_mpm == m_cardMode) ? e_sd15 : e_sd30;
	pSharpSliderParamsSD->AddChildNode("RESOLUTION_TYPE", resType1, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSharpSliderParamsSD->AddChildNode("MINIMAL_RESOLUTION_RATE", m_SD_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	// HD720
	CXMLDOMElement* pSharpSliderParamsHD720 = pSharpSliderParamsList->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
	EResolutionType resType2 = (eSystemCardsMode_mpm == m_cardMode) ? e_sd30 : e_hd720p30;
	pSharpSliderParamsHD720->AddChildNode("RESOLUTION_TYPE", resType2, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSharpSliderParamsHD720->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD720_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	// HD1080
	CXMLDOMElement* pSharpSliderParamsHD1080 = pSharpSliderParamsList->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
	EResolutionType resType3 = e_hd1080p30;
	pSharpSliderParamsHD1080->AddChildNode("RESOLUTION_TYPE", resType3, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSharpSliderParamsHD1080->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	// HD1080 60
	CXMLDOMElement* pSharpSliderParamsHD1080p60 = pSharpSliderParamsList->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
	EResolutionType resType4 = e_hd1080p60;
	pSharpSliderParamsHD1080p60->AddChildNode("RESOLUTION_TYPE", resType4, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSharpSliderParamsHD1080p60->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_60_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);

	// MOTION - BASE PROFILE
	CXMLDOMElement* pMotionNode = pSet->AddChildNode("SET_MOTION_RESOLUTIONS");
	CXMLDOMElement* pSliderParamsList = pMotionNode->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS_LIST");
	// SD
	CXMLDOMElement* pSliderParamsSD = pSliderParamsList->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
	resType1 = (eSystemCardsMode_mpm == m_cardMode) ? e_wcif : e_cif60;
	pSliderParamsSD->AddChildNode("RESOLUTION_TYPE", resType1, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSliderParamsSD->AddChildNode("MINIMAL_RESOLUTION_RATE", m_SD_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	// HD720
	CXMLDOMElement* pSliderParamsHD720 = pSliderParamsList->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
	resType2 = (eSystemCardsMode_mpm == m_cardMode) ? e_sd30 : e_sd60;
	pSliderParamsHD720->AddChildNode("RESOLUTION_TYPE", resType2, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSliderParamsHD720->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD720_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	// HD1080
	CXMLDOMElement* pSliderParamsHD1080 = pSliderParamsList->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
	resType3 = (eSystemCardsMode_mpm == m_cardMode) ? e_hd720p30 : e_hd720p60;
	pSliderParamsHD1080->AddChildNode("RESOLUTION_TYPE", resType3, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSliderParamsHD1080->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
	// HD1080 60
	CXMLDOMElement* pSliderParamsHD1080p60 = pSliderParamsList->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
	resType4 = e_hd1080p60;
	pSliderParamsHD1080p60->AddChildNode("RESOLUTION_TYPE", resType4, RESOLUTION_THRESHOLD_TYPE_ENUM);
	pSliderParamsHD1080p60->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_60_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);

	// HighProfile
	{
		// SHARPNESS - HIGH PROFILE
		CXMLDOMElement* pSharpnessNodeHP = pSet->AddChildNode("SET_SHARPNESS_HIGH_PROFILE_RESOLUTIONS");
		CXMLDOMElement* pSharpSliderParamsListHP = pSharpnessNodeHP->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS_LIST");
		// SD
		CXMLDOMElement* pSharpSliderParamsSDHP = pSharpSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
		pSharpSliderParamsSDHP->AddChildNode("RESOLUTION_TYPE", e_sd30, RESOLUTION_THRESHOLD_TYPE_ENUM);
		pSharpSliderParamsSDHP->AddChildNode("MINIMAL_RESOLUTION_RATE", m_SD_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
		// HD720
		CXMLDOMElement* pSharpSliderParamsHD720HP = pSharpSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
		pSharpSliderParamsHD720HP->AddChildNode("RESOLUTION_TYPE", e_hd720p30, RESOLUTION_THRESHOLD_TYPE_ENUM);
		pSharpSliderParamsHD720HP->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD720_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
		// HD1080
		CXMLDOMElement* pSharpSliderParamsHD1080HP = pSharpSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
		pSharpSliderParamsHD1080HP->AddChildNode("RESOLUTION_TYPE", e_hd1080p30, RESOLUTION_THRESHOLD_TYPE_ENUM);
		pSharpSliderParamsHD1080HP->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
		// HD1080 60
		CXMLDOMElement* pSharpSliderParamsHD1080Hp60 = pSharpSliderParamsListHP->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
		pSharpSliderParamsHD1080Hp60->AddChildNode("RESOLUTION_TYPE", e_hd1080p60, RESOLUTION_THRESHOLD_TYPE_ENUM);
		pSharpSliderParamsHD1080Hp60->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_60_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);

		// MOTION - HIGH PROFILE
		CXMLDOMElement* pMotionNodeHp = pSet->AddChildNode("SET_MOTION_HIGH_PROFILE_RESOLUTIONS");
		CXMLDOMElement* pSliderParamsListHp = pMotionNodeHp->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS_LIST");
		// SD
		CXMLDOMElement* pSliderParamsSDHp = pSliderParamsListHp->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
		pSliderParamsSDHp->AddChildNode("RESOLUTION_TYPE", e_cif60, RESOLUTION_THRESHOLD_TYPE_ENUM);
		pSliderParamsSDHp->AddChildNode("MINIMAL_RESOLUTION_RATE", m_SD_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
		// HD720
		CXMLDOMElement* pSliderParamsHD720Hp = pSliderParamsListHp->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
		pSliderParamsHD720Hp->AddChildNode("RESOLUTION_TYPE", e_sd60, RESOLUTION_THRESHOLD_TYPE_ENUM);
		pSliderParamsHD720Hp->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD720_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
		// HD1080
		CXMLDOMElement* pSliderParamsHD1080Hp = pSliderParamsListHp->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
		pSliderParamsHD1080Hp->AddChildNode("RESOLUTION_TYPE", e_hd720p60, RESOLUTION_THRESHOLD_TYPE_ENUM);
		pSliderParamsHD1080Hp->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
		// HD1080 60
		CXMLDOMElement* pSliderParamsHD1080Hp60 = pSliderParamsListHp->AddChildNode("RESOLUTION_SLIDER_SHORT_PARAMS");
		pSliderParamsHD1080Hp60->AddChildNode("RESOLUTION_TYPE", e_hd1080p60, RESOLUTION_THRESHOLD_TYPE_ENUM);
		pSliderParamsHD1080Hp60->AddChildNode("MINIMAL_RESOLUTION_RATE", m_HD1080_60_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
	}
}

/////////////////////////////////////////////////////////////////////////////////
int CSetResolutionSliderDetails::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	FTRACEINTO << " CSetResolutionSliderDetails::DeSerializeXml ";
	STATUS nStatus = STATUS_OK;
	CXMLDOMElement* pMotionNode, *pSharpnessNode, *pSet, *pListNode, *pNode;

	GET_CHILD_NODE(pActionNode, "SET_RESOLUTIONS_PARAMS", pSet);
	if (pSet)
	{
		DWORD resType;

		GET_VALIDATE_CHILD(pSet, "CP_MAX_RESOLUTION", &m_CPmaxRes, RESOLUTION_THRESHOLD_TYPE_ENUM);
//	    BOOL res = CProcessBase::GetProcess()->GetSysConfig()->OverWriteParam("MAX_CP_RESOLUTION", data);

		GET_VALIDATE_CHILD(pSet, "CONFIGURATION_TYPE", &m_ConfigType, RESOLUTION_CONFIG_TYPE_ENUM);

		// SHARPNESS - BASE PROFILE
		GET_CHILD_NODE(pSet, "SET_SHARPNESS_RESOLUTIONS", pSharpnessNode);
		if (pSharpnessNode)
		{
			GET_CHILD_NODE(pSharpnessNode, "RESOLUTION_SLIDER_SHORT_PARAMS_LIST", pListNode);

			GET_FIRST_CHILD_NODE(pListNode, "RESOLUTION_SLIDER_SHORT_PARAMS", pNode);
			while (pNode)
			{
				GET_VALIDATE_CHILD(pNode, "RESOLUTION_TYPE", &resType, RESOLUTION_THRESHOLD_TYPE_ENUM);

				if (e_sd15 == resType || (e_sd30 == resType && (m_cardMode != eSystemCardsMode_mpm)))
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_SD_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else if (e_sd30 == resType || (e_hd720p30 == resType && (m_cardMode != eSystemCardsMode_mpm)))
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD720_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else if (e_hd720p30 == resType || e_hd1080p30 == resType)
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD1080_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else
				{ // e_hd1080p60,
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD1080_60_SharpnessMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				GET_NEXT_CHILD_NODE(pListNode, "RESOLUTION_SLIDER_SHORT_PARAMS", pNode);
			}
		}
		// MOTION - BASE PROFILE
		GET_CHILD_NODE(pSet, "SET_MOTION_RESOLUTIONS", pMotionNode);
		if (pMotionNode)
		{
			GET_CHILD_NODE(pMotionNode, "RESOLUTION_SLIDER_SHORT_PARAMS_LIST", pListNode);

			GET_FIRST_CHILD_NODE(pListNode, "RESOLUTION_SLIDER_SHORT_PARAMS", pNode);
			while (pNode)
			{
				GET_VALIDATE_CHILD(pNode, "RESOLUTION_TYPE", &resType, RESOLUTION_THRESHOLD_TYPE_ENUM);
				if (e_wcif == resType || e_cif60 == resType)
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_SD_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else if (e_sd30 == resType || e_sd60 == resType)
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD720_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else if (e_hd720p60 == resType || e_hd720p30 == resType || e_hd1080p30 == resType)
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD1080_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else
				{ // e_hd1080p60,
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD1080_60_MotionMinRate, RESOLUTION_THRESHOLD_RATE_ENUM);
				}

				GET_NEXT_CHILD_NODE(pListNode, "RESOLUTION_SLIDER_SHORT_PARAMS", pNode);
			}
		}

		// SHARPNESS - HIGH PROFILE
		GET_CHILD_NODE(pSet, "SET_SHARPNESS_HIGH_PROFILE_RESOLUTIONS", pSharpnessNode);
		if (pSharpnessNode)
		{
			GET_CHILD_NODE(pSharpnessNode, "RESOLUTION_SLIDER_SHORT_PARAMS_LIST", pListNode);

			GET_FIRST_CHILD_NODE(pListNode, "RESOLUTION_SLIDER_SHORT_PARAMS", pNode);
			while (pNode)
			{
				GET_VALIDATE_CHILD(pNode, "RESOLUTION_TYPE", &resType, RESOLUTION_THRESHOLD_TYPE_ENUM);

				if (e_sd30 == resType)
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_SD_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else if (e_hd720p30 == resType)
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD720_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else if (e_hd1080p30 == resType)
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD1080_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD1080_60_SharpnessMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				GET_NEXT_CHILD_NODE(pListNode, "RESOLUTION_SLIDER_SHORT_PARAMS", pNode);
			}
		}
		// MOTION - HIGH PROFILE
		GET_CHILD_NODE(pSet, "SET_MOTION_HIGH_PROFILE_RESOLUTIONS", pMotionNode);
		if (pMotionNode)
		{
			GET_CHILD_NODE(pMotionNode, "RESOLUTION_SLIDER_SHORT_PARAMS_LIST", pListNode);

			GET_FIRST_CHILD_NODE(pListNode, "RESOLUTION_SLIDER_SHORT_PARAMS", pNode);
			while (pNode)
			{
				GET_VALIDATE_CHILD(pNode, "RESOLUTION_TYPE", &resType, RESOLUTION_THRESHOLD_TYPE_ENUM);
				if (e_cif60 == resType)
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_SD_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else if (e_sd60 == resType)
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD720_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else if (e_hd720p60 == resType || e_hd720p30 == resType || e_hd1080p30 == resType)
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD1080_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				else
				{
					GET_VALIDATE_CHILD(pNode, "MINIMAL_RESOLUTION_RATE", &m_HD1080_60_MotionMinRateHighProfile, RESOLUTION_THRESHOLD_RATE_ENUM);
				}
				GET_NEXT_CHILD_NODE(pListNode, "RESOLUTION_SLIDER_SHORT_PARAMS", pNode);
			}
		}
	}
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////
EResolutionType CResolutionSliderDetails::GetMaxResForCardType() const
{
	EResolutionType res;

	switch (m_cardMode)
	{
		case eSystemCardsMode_mpmrx:
		case eSystemCardsMode_breeze:
			res = e_hd1080p60;
			break;

		default:
			res = e_hd1080p30;
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////
WORD CResRsrcCalculator::GetMaxNumSliderRates(bool isMotion)
{
	return (isMotion) ? MAX_NUM_SLIDER_RATE_MOTION : MAX_NUM_SLIDER_RATE_SHARPNESS;
}

/////////////////////////////////////////////////////////////////////////////////
DWORD CResRsrcCalculator::GetHDBitRateThrshld(eSystemCardsMode systemCardsBasedMode)
{
	WORD card_tbl_index = systemCardsBasedMode - 1;
	DWORD bitRate = 0;
	if (e_manual == m_ConfigType)
	{
		H264VideoModeThresholdStruct* pTblSharpness = g_dynamicSharpnessVideoModeThresholdTbl[card_tbl_index];
		bitRate = pTblSharpness[2].thresholdBitrate;

		H264VideoModeThresholdStruct* pTblSharpnessHighProfile = g_dynamicHighProfileSharpnessThresholdTbl[card_tbl_index];
		bitRate = min(pTblSharpnessHighProfile[2].thresholdBitrate, bitRate);
	}
	else
	{
		ResolutionThresholdStruct* pThresholdTblSharpness = NULL;
		ResolutionThresholdStruct* pThresholdTblSharpnessHighProfile = NULL;

		pThresholdTblSharpness = g_defaultSharpnessThresholdTbl[card_tbl_index];
		bitRate = ConvertEThresholdBitRate(pThresholdTblSharpness[2].user_optimized_bitrate);

		pThresholdTblSharpnessHighProfile = g_defaultHighProfileSharpnessThresholdTbl[card_tbl_index];
		DWORD temp = ConvertEThresholdBitRate(pThresholdTblSharpnessHighProfile[2].user_optimized_bitrate);
		bitRate = min(temp, bitRate);

		bitRate = bitRate / 1000;
	}
	return bitRate;
}

//FSN-613: Dynamic Content for SVC/Mix Conf
DWORD CResRsrcCalculator::GetRateThrshldBasedOnVideoModeType(eSystemCardsMode systemCardsBasedMode,  eVideoQuality videoQuality, BOOL isHighProfile, Eh264VideoModeType specificVideoModeType)
{
	DWORD bitRate = 0;
	bool isMotion = (eVideoQualityMotion == videoQuality);
  	bool isSharpn = (eVideoQualitySharpness == videoQuality);

  	FTRACEINTO << " ***** "
             << "- systemCardsBasedMode:" << ::GetSystemCardsModeStr(systemCardsBasedMode)
             << ", quality:"              << (isMotion ? "Motion" : (isSharpn ? "Sharpness" : "Auto"))
             << ", specificVideoMode:"         << eVideoModeTypeNames[specificVideoModeType]
             << ", isHighProfile:"        << (isHighProfile ? "True" : "False");

	WORD card_tbl_index = systemCardsBasedMode - 1;

	if( e_manual == m_ConfigType )
	{
		H264VideoModeThresholdStruct* pTblTemp = NULL;

		if (isHighProfile)
	      		pTblTemp = (isMotion ? g_dynamicHighProfileMotionThresholdTbl[card_tbl_index] : g_dynamicHighProfileSharpnessThresholdTbl[card_tbl_index]);
	    	else
	      		pTblTemp = (isMotion ? g_dynamicMotionVideoModeThresholdTbl[card_tbl_index] : g_dynamicSharpnessVideoModeThresholdTbl[card_tbl_index]);

	    	for (int i = 0; i < GetMaxNumSliderRates(isMotion); i++)
	   	{
	   		if (specificVideoModeType == pTblTemp[i].videoModeType)
	   		{
				bitRate = pTblTemp[i].thresholdBitrate;
				FTRACEINTO << " Get threshold rate based on manual: " << bitRate;
				break;
	   		}
	    	}
  	}
	else
	{
    		ResolutionThresholdStruct* pThresholdTblTemp = NULL;
		EThresholdBitRate curBitRate = e64000;

		if (isHighProfile)
			pThresholdTblTemp = (isMotion ? g_defaultHighProfileMotionThresholdTbl[card_tbl_index] : g_defaultHighProfileSharpnessThresholdTbl[card_tbl_index]);
		else
		      pThresholdTblTemp = (isMotion ? g_defaultMotionThresholdTbl[card_tbl_index] : g_defaultSharpnessThresholdTbl[card_tbl_index]);

		for (int i = 0; i < GetMaxNumSliderRates(isMotion); i++)
	   	{
	   		if (specificVideoModeType == pThresholdTblTemp[i].videoModeType)
	   		{
	   			switch (m_ConfigType)
			      {
			      		case e_balanced:
			          	curBitRate = pThresholdTblTemp[i].balanced_bitrate;
			          	break;
			        	case e_resource_optimized:
			          	curBitRate = pThresholdTblTemp[i].resource_optimized_bitrate;
			          	break;
				       case e_user_exp_optimized:
				       curBitRate = pThresholdTblTemp[i].user_optimized_bitrate;
				       break;
				       case e_hi_profile_optimized:
				       curBitRate = pThresholdTblTemp[i].hi_profile_optimized_bitrate;
				       break;
					default:
					// Note: some enumeration value are not handled in switch. Add default to suppress warning.
					break;
			      }

				bitRate = ConvertEThresholdBitRate(curBitRate);
				FTRACEINTO << " Get threshold rate based on other type: " << bitRate;
				break;
	   		}
	    	}
	}

	bitRate = bitRate/100;
	return bitRate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////MPM Table//////////////////////////////////////////
//ResRateRcrsVidTypeStruct CResRsrcCalculator::g_MpmBasedReservationVideoTypeTbl[MAX_RESERVATION_RATES] =
//{
////      |Reservation Transfer Rate   | Sharpness Video Type                                  |Motion Video Type |
//		{Xfer_64,                      eCP_H261_H263_H264_upto_CIF_video_party_type,         eCP_H261_H263_H264_upto_CIF_video_party_type        },
//		{Xfer_96,                      eCP_H261_H263_H264_upto_CIF_video_party_type,         eCP_H261_H263_H264_upto_CIF_video_party_type        },
//
//		{Xfer_2x64,                    eCP_H261_H263_H264_upto_CIF_video_party_type,         eCP_H261_H263_H264_upto_CIF_video_party_type        },
//	    {Xfer_128,                     eCP_H261_H263_H264_upto_CIF_video_party_type,         eCP_H261_H263_H264_upto_CIF_video_party_type        },
//
//	    {Xfer_3x64,                    eCP_H261_H263_H264_upto_CIF_video_party_type,         eCP_H261_H263_H264_upto_CIF_video_party_type        },
//	    {Xfer_192,                     eCP_H261_H263_H264_upto_CIF_video_party_type,         eCP_H261_H263_H264_upto_CIF_video_party_type        },
//
//	    {Xfer_4x64,                    eCP_H263_H264_upto_SD15_video_party_type,             eCP_H263_H264_upto_SD15_video_party_type            },
//	    {Xfer_256,                     eCP_H263_H264_upto_SD15_video_party_type,             eCP_H263_H264_upto_SD15_video_party_type            },
//
//	    {Xfer_5x64,                    eCP_H263_H264_upto_SD15_video_party_type,             eCP_H263_H264_upto_SD15_video_party_type                },
//	    {Xfer_320,                     eCP_H263_H264_upto_SD15_video_party_type,             eCP_H263_H264_upto_SD15_video_party_type                 },
//
//	    {Xfer_6x64,                    eCP_H263_H264_upto_SD15_video_party_type,             eCP_H263_H264_upto_SD15_video_party_type                 },
//		{Xfer_384,                     eCP_H263_H264_upto_SD15_video_party_type,             eCP_H263_H264_upto_SD15_video_party_type                 },
//
//		{Xfer_512,                     eCP_H264_upto_SD30_video_party_type,                  eCP_H263_H264_upto_SD15_video_party_type                 },
//		{Xfer_768,                     eCP_H264_upto_SD30_video_party_type,                  eCP_H263_H264_upto_SD15_video_party_type                 },
//		{Xfer_1024,                    eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type, eCP_H264_upto_SD30_video_party_type                 },
//		{Xfer_1152,                    eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type, eCP_H264_upto_SD30_video_party_type                 },
//		{Xfer_1472,                    eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type, eCP_H264_upto_SD30_video_party_type                 },
//		{Xfer_1536,                    eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type, eCP_H264_upto_SD30_video_party_type                 },
//		{Xfer_1920,                    eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type, eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type},
//		{Xfer_4096,                    eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type, eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type},
//		{Xfer_6144,                    eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type, eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type},
//};

//////////////////////////////MPM+ Table//////////////////////////////////////////
//ResRateRcrsVidTypeStruct CResRsrcCalculator::g_MpmPlusBasedReservationVideoTypeTbl[MAX_RESERVATION_RATES] =
//{
////      |Reservation Transfer Rate   | Sharpness Video Type                                |Motion Video Type |
//		{Xfer_64,                      eCP_H261_H263_H264_upto_CIF_video_party_type,          eCP_H261_H263_H264_upto_CIF_video_party_type        },
//		{Xfer_96,                      eCP_H261_H263_H264_upto_CIF_video_party_type,          eCP_H261_H263_H264_upto_CIF_video_party_type        },
//
//		{Xfer_2x64,                    eCP_H261_H263_H264_upto_CIF_video_party_type,          eCP_H261_H263_H264_upto_CIF_video_party_type        },
//	    {Xfer_128,                     eCP_H261_H263_H264_upto_CIF_video_party_type,          eCP_H261_H263_H264_upto_CIF_video_party_type        },
//
//	    {Xfer_3x64,                    eCP_H261_H263_H264_upto_CIF_video_party_type,          eCP_H261_H263_H264_upto_CIF_video_party_type        },
//	    {Xfer_192,                     eCP_H261_H263_H264_upto_CIF_video_party_type,          eCP_H261_H263_H264_upto_CIF_video_party_type        },
//
//	    {Xfer_4x64,                    eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//	    {Xfer_256,                     eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//
//	    {Xfer_5x64,                    eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//	    {Xfer_320,                     eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//
//	    {Xfer_6x64,                    eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//		{Xfer_384,                     eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//
//		{Xfer_512,                     eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//		{Xfer_768,                     eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//		{Xfer_1024,                    eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,   eCP_H264_upto_HD720_30FS_Symmetric_video_party_type },
//		{Xfer_1152,                    eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,   eCP_H264_upto_HD720_30FS_Symmetric_video_party_type },
//		{Xfer_1472,                    eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,   eCP_H264_upto_HD720_30FS_Symmetric_video_party_type },
//		{Xfer_1536,                    eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,   eCP_H264_upto_HD720_30FS_Symmetric_video_party_type },
//		{Xfer_1920,                    eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,   eCP_H264_upto_HD1080_30FS_Asymmetric_video_party_type},
//		{Xfer_4096,                    eCP_H264_upto_HD1080_30FS_Asymmetric_video_party_type, eCP_H264_upto_HD1080_30FS_Asymmetric_video_party_type},
//		{Xfer_6144,                    eCP_H264_upto_HD1080_30FS_Asymmetric_video_party_type, eCP_H264_upto_HD1080_30FS_Asymmetric_video_party_type},
//};
//////////////////////////////MPMX Table//////////////////////////////////////////
//ResRateRcrsVidTypeStruct CResRsrcCalculator::g_BreezeBasedReservationVideoTypeTbl[MAX_RESERVATION_RATES] =
//{
////      |Reservation Transfer Rate   | Sharpness Video Type                                |Motion Video Type |
//		{Xfer_64,                      eCP_H264_upto_CIF_video_party_type,          eCP_H264_upto_CIF_video_party_type        },
//		{Xfer_96,                      eCP_H264_upto_CIF_video_party_type,          eCP_H264_upto_CIF_video_party_type        },
//
//		{Xfer_2x64,                    eCP_H264_upto_CIF_video_party_type,          eCP_H264_upto_CIF_video_party_type        },
//	    {Xfer_128,                     eCP_H264_upto_CIF_video_party_type,          eCP_H264_upto_CIF_video_party_type       },
//
//	    {Xfer_3x64,                    eCP_H264_upto_CIF_video_party_type,          eCP_H264_upto_CIF_video_party_type        },
//	    {Xfer_192,                     eCP_H264_upto_CIF_video_party_type,          eCP_H264_upto_CIF_video_party_type        },
//
//	    {Xfer_4x64,                    eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//	    {Xfer_256,                     eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//
//	    {Xfer_5x64,                    eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//	    {Xfer_320,                     eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//
//	    {Xfer_6x64,                    eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//		{Xfer_384,                     eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//
//		{Xfer_512,                     eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//		{Xfer_768,                     eCP_H264_upto_SD30_video_party_type,                   eCP_H264_upto_SD30_video_party_type                 },
//		{Xfer_1024,                    eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,   eCP_H264_upto_HD720_30FS_Symmetric_video_party_type },
//		{Xfer_1152,                    eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,   eCP_H264_upto_HD720_30FS_Symmetric_video_party_type },
//		{Xfer_1472,                    eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,   eCP_H264_upto_HD720_30FS_Symmetric_video_party_type },
//		{Xfer_1536,                    eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,   eCP_H264_upto_HD720_30FS_Symmetric_video_party_type },
//		{Xfer_1920,                    eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,   eCP_H264_upto_HD720_60FS_Symmetric_video_party_type},
//		{Xfer_4096,                    eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type, eCP_H264_upto_HD720_60FS_Symmetric_video_party_type},
//		{Xfer_6144,                    eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type, eCP_H264_upto_HD720_60FS_Symmetric_video_party_type},
//};

////////////////////////////////////////////////////////////////////////////////////////////
//eVideoPartyType CResRsrcCalculator::GetMPMBasedCPResRsrcVideoType(eVideoQuality rsrvVidQuality, BYTE rsrvTransferRate)
//{
//	eVideoPartyType videoRsrcType =  eVideo_party_type_dummy;
//	for(int i=0;i<MAX_RESERVATION_RATES;i++)
//	{
//		if (g_MpmBasedReservationVideoTypeTbl[i].resTransferRate == rsrvTransferRate)
//		{
//			if(rsrvVidQuality==eVideoQualitySharpness)
//			{
//				videoRsrcType = g_MpmBasedReservationVideoTypeTbl[i].sharpnessVideoPartyType;
//			}
//			else
//			{
//				videoRsrcType = g_MpmBasedReservationVideoTypeTbl[i].motionVideoPartyType;
//			}
//			return videoRsrcType;
//		}
//	}
//	PASSERT(rsrvTransferRate);
//	return videoRsrcType;
//}
////////////////////////////////////////////////////////////////////////////////////////////
//eVideoPartyType CResRsrcCalculator::GetMPMPlusBasedCPResRsrcVideoType(eVideoQuality rsrvVidQuality, BYTE rsrvTransferRate)
//{
//	eVideoPartyType videoRsrcType =  eVideo_party_type_dummy;
//	for(int i=0;i<MAX_RESERVATION_RATES;i++)
//	{
//		if (g_MpmPlusBasedReservationVideoTypeTbl[i].resTransferRate == rsrvTransferRate)
//		{
//			if(rsrvVidQuality==eVideoQualitySharpness)
//			{
//				videoRsrcType = g_MpmPlusBasedReservationVideoTypeTbl[i].sharpnessVideoPartyType;
//			}
//			else
//			{
//				videoRsrcType = g_MpmPlusBasedReservationVideoTypeTbl[i].motionVideoPartyType;
//			}
//			return videoRsrcType;
//		}
//	}
//	PASSERT(rsrvTransferRate);
//	return videoRsrcType;
//}
////////////////////////////////////////////////////////////////////////////////////////////
//eVideoPartyType CResRsrcCalculator::GetBreezeBasedCPResRsrcVideoType(eVideoQuality rsrvVidQuality, BYTE rsrvTransferRate)
//{
//	eVideoPartyType videoRsrcType =  eVideo_party_type_dummy;
//	for(int i=0;i<MAX_RESERVATION_RATES;i++)
//	{
//		if (g_BreezeBasedReservationVideoTypeTbl[i].resTransferRate == rsrvTransferRate)
//		{
//			if(rsrvVidQuality==eVideoQualitySharpness)
//			{
//				videoRsrcType = g_BreezeBasedReservationVideoTypeTbl[i].sharpnessVideoPartyType;
//			}
//			else
//			{
//				videoRsrcType = g_BreezeBasedReservationVideoTypeTbl[i].motionVideoPartyType;
//			}
//			return videoRsrcType;
//		}
//	}
//	PASSERT(rsrvTransferRate);
//	return videoRsrcType;
//}
