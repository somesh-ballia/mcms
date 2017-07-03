
// cfg_api.h
// Kirill Tsym

#ifndef __CFG_API_H__
#define __CFG_API_H__

#ifdef __SIM__
#define __PrintOn__
#endif

// Output buffer format
// 
// ------------------------------------
// | SIZE | 4 bytes overal size string|
// ------------------------------------
// | RSV  | 4 bytes reserved          |
// ------------------------------------
// | LEN  | 1 bytes - section length  |
// ------------------------------------
// | SECTION | variable length string |
// ------------------------------------
// | LEN  | 1 bytes - KEY length      |
// ------------------------------------
// | KEY | variable length string     |
// ------------------------------------
// | LEN  | 1 bytes - DATA length     |
// ------------------------------------
// | DATA | variable length string    |
// ------------------------------------
// | Next message                     |
// |                                  |
// |                                  |

// Include:
//---------

//#include <stdlib.h>

// Macros:
//--------

#define		CfgBool						unsigned int
#define		CfgFalse					0
#define		CfgTrue						1
#define		CfgNo						0
#define		CfgYes						1

#define		CfgMaxBufferSize			9999	// Maximum buffer size

// Error definitions

typedef enum {
	E_INVALID_PRM						= 0x0101,
	E_NOT_INIT							= 0x0102,
	E_FILL_HDR							= 0x0103,
	E_INVALID_BUF_SIZE					= 0x0104,
	E_INVALID_BUF_SIZE_1				= 0x0105,
	E_INVALID_BUF_SIZE_2				= 0x0106,
	E_INVALID_BUF_SIZE_3				= 0x0107,
	E_GET_STRING						= 0x0108,
	E_GET_IP_ADDR						= 0x0109,
	E_GET_BOOL							= 0x010A,
	E_GET_INT32							= 0x010B,
	E_GET_INT16							= 0x010C,
	E_GET_INT8							= 0x010D,
	E_BUF_OVERFLOW						= 0x010E,
	E_PUT_STRING						= 0x010F,
	E_PUT_STRING_N						= 0x0110,
	E_PUT_IP_ADDR						= 0x0111,
	E_PUT_BOOL							= 0x0112,
	E_PUT_INT32							= 0x0113,
	E_PUT_INT16							= 0x0114,
	E_PUT_INT8							= 0x0115,
	E_PUT_UNKNOWN_TYPE					= 0x0116,
	E_GET_UNKNOWN_TYPE					= 0x0117,
	E_INVALID_DATA_SIZE					= 0x0118,
	E_BUF_IS_EMPTY						= 0x0119,
	E_INVALID_NUM_OF_PARAMS_AND_KEYS	= 0x011A,
	E_INVALID_VA_ARG_PRM				= 0x011B,
    E_PUT_UINT16						= 0x011C,
 	E_PUT_UINT32						= 0x011D,

	E_INVALID_DATA_SIZE_2				= 0x0201,
	E_INVALID_BUF_SIZE_4				= 0x0202,
	E_INVALID_BUF_SIZE_5				= 0x0202,
	E_INVALID_STR_TO_IP					= 0x0203,
} cfgErrorDefs;

// Types:
//-------


typedef struct {
	CfgBool			bInit;			// is handle initialized
	void			*pAppHndl;		// application handle
	char			*pBuf;			// pointer to the start of user buffer
	char			*pCurBuf;		// pointer to the current offset in user buffer

	unsigned int	maxBufSize;		// maximum buffer size
	unsigned int	usedBufSize;	// currently filled buffer size
	unsigned int	numberOfKeys;	// current number of keys
} cfgBufHndl;

typedef enum {
	cfString			= 0,		// Put and get as string
	cfIpAddress			= 1,		// Put as string and get as integer
	cfBOOL				= 2,		// Put as string and get as byte
	cfINT32				= 3,		// Put and get as integer
	cfINT16				= 4,		// Put and get as short
	cfINT8				= 5,		// Put and get as byte
    cfUINT32			= 6,
    cfUINT16			= 7,
} cfgDataType;

#define cfDWORD			cfInt32
#define cfWORD			cfINT16
#define cfBYTE			cfINT8

typedef struct {
	int						len;
	char					*buf;
} prmSendStruct;

// Routines:
//----------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------
//
//	Function name:	CfgSetParams
//
//	Description:	Set various number of configuration params with various
//                  types
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------

int CfgSetParams( 
	int				numOfParams, 
	char			*pCfgBuf,			// IN/OUT	: Configuration buffer
	int				bufSize,			// IN		: Maximum configuration buffer size
	int				*pAllocatedSize,	// OUT		: Allocated size in this buffer
	char			*pSectionName,		// IN		: Section name
//  Set parameters number of times in following format
//	char			*pKeyName,			// IN		: Key name
//	void			*pData,				// IN		: Pointer to the data
//	cfgDataType		dataType,			// IN		: Data type
	... );

//---------------------------------------------------------------------------
//
//	Function name:	CfgSetParamsSize
//
//	Description:	Set various number of configuration params with various
//                  types and with explicit size
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------

int CfgSetParamsSize( 
	int				numOfParams, 
	char			*pCfgBuf,			// IN/OUT	: Configuration buffer
	int				bufSize,			// IN		: Maximum configuration buffer size
	int				*pAllocatedSize,	// OUT		: Allocated size in this buffer
	char			*pSectionName,		// IN		: Section name
//  Set parameters number of times in following format
//	char			*pKeyName,			// IN		: Key name
//	void			*pData,				// IN		: Pointer to the data
//	int				dataSize,			// IN		: Data size
//	cfgDataType		dataType,			// IN		: Data type
	... );

//---------------------------------------------------------------------------
//
//	Function name:	CfgGetParams
//
//	Description:	Get various number of configuration params with various
//                  types and with explicit size
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------

int CfgGetParams( 
	int				numOfParams, 
	char			*pCfgBuf,			// IN/OUT	: Configuration buffer
	int				*pBufSize,			// IN/OUT	: Maximum configuration buffer size
	char			*pSectionName,		// OUT		: Section name
	int				*pSectionNameSize,	// OUT		: Section name
//  Get parameters number of times in following format
//	char			*pKeyName,			// OUT		: Key name
//	int				*pKeyNameSize,		// OUT		: Key name
//	void			*pData,				// OUT		: Pointer to the data
//	int				*pDataSize,			// OUT		: Data size
//	cfgDataType		dataType,			// IN		: Data type
	... );

//---------------------------------------------------------------------------
//
//	Function name:	CfgGetParamsContinue
//
//	Description:	Get various number of configuration params with various
//                  types and with explicit size by cfgBufHandle
//                  This funtion privides an option to perform several Get
//                  operations from one buffer
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------

int CfgGetParamsContinue(
	cfgBufHndl		*pBufHndl,			// IN		: Buffer handle
	int				numOfParams,		// IN		: Number of parameters
	char			*pCfgBuf,			// IN/OUT	: Configuration buffer
//	int				*pBufSize,			// IN/OUT	: Maximum configuration buffer size
//	char			*pSectionName,		// OUT		: Section name
//	int				*pSectionNameSize,	// OUT		: Section name
//  Get parameters number of times in following format
//	char			*pKeyName,			// OUT		: Key name
//	int				*pKeyNameSize,		// OUT		: Key name
//	void			*pData,				// OUT		: Pointer to the data
//	int				*pDataSize,			// OUT		: Data size
//	cfgDataType		dataType,			// IN		: Data type
	... );

//---------------------------------------------------------------------------
//
//	Function name:	CfgPackBuf
//
//	Description:	Initilize new configuration buffer
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------
int CfgPackBuf(
	cfgBufHndl		*pBufHndl,				// IN/OUT	: Buffer handle
	char			*pSectionName,			// IN		: Section name
	char			*pCfgBuf,				// IN		: Configuration buffer
	int				bufSize					// IN		: Maximum configuration buffer size
	);

//---------------------------------------------------------------------------
//
//	Function name:	CfgUnpackBuf
//
//	Description:	Open existing configuration buffer and return section name
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------
int CfgUnpackBuf(
	cfgBufHndl		*pBufHndl,				// IN	: Buffer handle
	char			*pCfgBuf,				// IN	: Configuration buffer
//	int				bufSize,				// IN	: Maximum configuration buffer size
	char			*pSectionName,			// OUT	: Section name
	int				*pSecSize,				// OUT	: Section name size
	int				*pMaxBufSize,			// OUT	: Buffer size from the data
	int				*pNumOfKeys				// OUT	: Number of keys in buffer
	);

//---------------------------------------------------------------------------
//
//	Function name:	CfgPutData
//
//	Description:	Put configuration data in to te buffer
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------
int CfgPutData(
	cfgBufHndl		*pBufHndl,			// IN		: Buffer handle
	char			*pKeyName,			// IN		: Key name
	void			*pData,				// IN		: Pointer to the data
	int				dataSize,			// IN		: Data size
	cfgDataType		dataType,			// IN		: Data type
	int				*pAllocatedSize		// OUT		: Allocated size in this buffer
	);

//---------------------------------------------------------------------------
//
//	Function name:	CfgGetData
//
//	Description:	Put configuration data in to te buffer
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------
int CfgGetData(
	cfgBufHndl		*pBufHndl,			// IN		: Buffer handle
	char			*pKeyName,			// OUT		: Key name
	int				*pKeySize,			// IN/OUT	: Key size
	void			*pData,				// OUT		: Pointer to the data
	int				*pDataSize,			// OUT		: Data size
	cfgDataType		dataType			// IN		: Data type
	);

char* CfgGetFailRoutine();
char* CfgGetFailStr();
int	CfgGetFailStatus();

#ifdef __cplusplus
}
#endif
#endif // __CFG_API_H__



