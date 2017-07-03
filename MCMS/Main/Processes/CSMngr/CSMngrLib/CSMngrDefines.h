// CSMngrDefines.h:
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		definitions for CSMngr
//========   ==============   =====================================================================

#ifndef      __CSMngrDefines__
#define      __CSMngrDefines__


#define CS_KEEP_ALIVE_RETRIES		60

#define MFW_UDP_FIRST_PORT_MIN		40000
#define MFW_UDP_FIRST_PORT_MAX		50000

enum eCsFailureType
{
	eFailureTypeCsUnitFailure = 0,
	eFailureTypeCsNoConnection
};

enum eCsMultipleServiceMode
{
	eMSNotConfigure   = -1,
	eFALSE            =  0,
	eTRUE             =  1
};



#endif  // __CSMngrDefines__
