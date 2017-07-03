///////////////////////////////////////////////////
//
// Project dependent variable.  Need to change to match 
//		project requirement.
//
///////////////////////////////////////////////////

#define	STR_PRODUCT_HIDDEN_REG_PATH			_T("SOFTWARE\\PLCM\\PN")
#define STR_KEY_CODE						_T("Code")
#define STR_REDUNDANCY_KEY_CODE				_T("RedundancyCode")


///////////////////////////////////////////////////
//
// Constant.  Should not change.
//
///////////////////////////////////////////////////


typedef enum
{
	gkAuthCodeUndefined,
	gkAuthCodeDemo,
	gkAuthCodeLicense,
	gkAuthCodeRedundancy
} KeyCodeType_e;

typedef enum
{
	LM_INVALID_AUTH_CODE		=	-10,
	LM_NOT_ENOUGH_WARRANTY		=	-9,
	LM_WARRANTY_KEY_NOT_FOUND	=	-8,
	LM_LICENSE_KEY_NOT_FOUND	=	-7,
	LM_NOT_UNDER_WARRANTY		=	-6,
	LM_CANT_PARSE_AUTH_CODE		=	-5,
	LM_NO_AUTH_CODE				=	-4,
	LM_NO_LICENSES				=	-3,
	LM_INVALID_SERIAL			=	-2,
	LM_OK						=	-1
} LM_RC;


#define OPTION_LEN	8

#define KEYCODE_TYPE	'X'			//Defined by CFS (Charge For Software)

//Bitmask information
//DDDDMSSS
#define SEAT_DIGIT			3
#define MODULE_DIGIT		1
#define EXPIRATION_DIGIT	4


