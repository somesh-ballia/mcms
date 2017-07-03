// CfgApi.c
// Kirill Tsym

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "CfgApi.h"
#include <stdlib.h>

#undef	SRC
#define SRC						"CfgApi"

//#define __PrintOn__

// External routines:
// ------------------

// Macros:
// -------

//#define isascii(_c)   ( (unsigned)(_c) < 0x80 )

#define HeaderSign				">"

#define NumOfKeysOffset			4	// Offset in the message header

#if (__MCMS_ENV__) || (__SIM__) || (__HIGHC__)
   #define htobige(addr)     ((((unsigned long)(addr) & 0x000000FF)<<24) | \
                             (((unsigned long)(addr) & 0x0000FF00)<<8)  | \
                             (((unsigned long)(addr) & 0x00FF0000)>>8)  | \
                             (((unsigned long)(addr) & 0xFF000000)>>24))
#else
	#define htobige(addr)
#endif
 

// External variables:
// -------------------

// External routines:
// -------------------
 
// Structures:
// -----------

typedef struct {
	char	size[4];		// overall message size
	char    numOfParams[2]; // overall number of key params
	char	rsrv[2];		// not used
} cfgBufHeader;
							
// Static variables:		
// -----------------

char	cfgFailRtn[128]={0};
char	cfgFailStr[128]={0};
int		cfgFailStatus		= 0;

// -----------------

// Static routines:
// ----------------

static int RetrieveParams(
	cfgBufHndl		*pBufHndl,			
	int				numOfParams,		
	char			*pCfgBuf,			
//	int				*pBufSize,			
	va_list			params);

static int	FillHeaderInfo(
	cfgBufHndl		*pBufHndl,
	int				curBufSize,
	int				numOfParams);

static int	PutString(
	cfgBufHndl		*pBufHndl,	
	char			*pStr);

static int	PutStringN(
	cfgBufHndl		*pBufHndl,	
	char			*pStr,
	int				strSize);

static int	PutIpAddress(
	cfgBufHndl		*pBufHndl,	
	char			*pStr,
	int				strSize);

static int	PutBool(
	cfgBufHndl		*pBufHndl,	
	char			*pBool);

static int	PutInt32(
	cfgBufHndl		*pBufHndl,	
	int				*pInt);

static int  PutUint32(
    cfgBufHndl      *pBufHndl,  
    unsigned long   *pInt);

static int	PutInt16(
	cfgBufHndl		*pBufHndl,	
	short			*pShort);

static int  PutUint16(
    cfgBufHndl      *pBufHndl,  
    unsigned short  *pShort);

static int	PutInt8(
	cfgBufHndl		*pBufHndl,	
	char			*pInt8);

static int	GetString(
	cfgBufHndl		*pBufHndl,
	char			*pBuf,
	int				*pBufSize);

static int	GetIpAddress(
	cfgBufHndl		*pBufHndl,
	int				*pBuf);

static int	GetBool(
	cfgBufHndl		*pBufHndl,
	char			*pBuf);

static int	GetInt32(
	cfgBufHndl		*pBufHndl,
	int				*pBuf);
static int  GetInt16(
    cfgBufHndl      *pBufHndl,
    short           *pBuf);

static int  GetUint16(
    cfgBufHndl      *pBufHndl,
    unsigned short  *pBuf);

static int	GetInt8(
	cfgBufHndl		*pBufHndl,
	char			*pBuf);

static int inetAddrNoSockets(
	char			*cp,
	int				*val);

static int inetAtonNoSockets(
	char			*cp,
	int				*addr);

static char *ipToString(
	int		ipAddr,
	char*	buf);

static void FormatError(
	char			*rtn, 
	char			*pStr, 
	int				status);

// Global routines:
// ----------------

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
	char			*pCfgBuf,			// IN/OUT : Configuration buffer
	int				bufSize,			// IN  : Maximum configuration buffer size
	int				*pAllocatedSize,	// OUT : Allocated size in this buffer
	char			*pSectionName,		// IN  : Section name
//  Set parameters number of times in following format
//	char			*pKeyName,			// IN : Key name
//	void			*pData,				// IN : Pointer to the data
//	cfgDataType		dataType,			// IN : Data type
	... )
{
  //	static char		rtn[]			= "CfgSetParams";
	int				i				= numOfParams;
	va_list			params;
	cfgBufHndl		cfgHndl;
	char			*pKeyName		= NULL;
	int				dataName;
	void			*pDataName		= NULL;
	int				dataSize		= 0;
	cfgDataType		cfType			= 0;
	int				allocBufSize	= 0;
	int				status			= 0;
	char			ipAddrBuff[16];

	memset(
		&cfgHndl,
		0,
		sizeof(cfgBufHndl));

	va_start(params, pSectionName);     /* Initialize variable arguments. */

	// Send phase
	status = CfgPackBuf(
			&cfgHndl,
			pSectionName,
			pCfgBuf,
			bufSize);
	if (status) {
		return status;
	}

	for (i = 0; i < numOfParams; i++) {

		pKeyName  = va_arg(params, char*);
		dataName  = (int) va_arg(params, void*);
		cfType	  = va_arg(params, int);

		if (!pKeyName) {
			return E_INVALID_VA_ARG_PRM;
		}

		dataSize  = 0;
		pDataName = &dataName;
		if (cfType == cfString) {
			dataSize  = strlen((char*) dataName);
			pDataName = (char*) dataName;
		}

		if (cfType == cfIpAddress) {
			dataName = (int) ipToString(
				dataName,
				ipAddrBuff);
			dataSize  = strlen((char*) dataName);
			pDataName = (char*) dataName;
		}

#ifdef __PrintOn__
		if (cfType == cfString ||
			cfType == cfIpAddress)
			printf("\nSET:KEY:[%-16s]	DATA:[%-20s] type:[%d] size:[%d] ", 
				pKeyName, 
				(char*) dataName,
				cfType, 
				dataSize);
		else 
			if (cfType == cfINT16)
				printf("\nSET:KEY:[%-16s]	DATA:[%-20d] type:[%d]", 
					pKeyName, 
					(short) dataName,
					cfType);
			else
				if ((cfType == cfINT8) ||
					(cfType == cfBOOL))
					printf("\nSET:KEY:[%-16s]	DATA:[%-20d] type:[%d]", 
						pKeyName, 
						(char) dataName,
						cfType);
				else
						printf("\nSET:KEY:[%-16s]	DATA:[%-20d] type:[%d]", 
							pKeyName, 
							(int) dataName,
							cfType);
		fflush(stdout);
		printf("\n");
#endif

		status = CfgPutData(
			&cfgHndl,
			pKeyName,
			pDataName,
			dataSize,
			cfType,
			&allocBufSize);

		if (status) {
			return status;
		}
	}

	va_end(params);              /* Reset variable arguments.      */

	*pAllocatedSize = allocBufSize;

	return 0;
}

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
	char			*pCfgBuf,			// IN/OUT : Configuration buffer
	int				bufSize,			// IN  : Maximum configuration buffer size
	int				*pAllocatedSize,	// OUT : Allocated size in this buffer
	char			*pSectionName,		// IN  : Section name
//  Set parameters number of times in following format
//	char			*pKeyName,			// IN : Key name
//	void			*pData,				// IN : Pointer to the data
//	int				dataSize,			// IN : Data size
//	cfgDataType		dataType,			// IN : Data type
	... )
{
  //	static char		rtn[]			= "CfgSetParams";
	int				i				= numOfParams;
	va_list			params;
	cfgBufHndl		cfgHndl;
	char			*pKeyName		= NULL;
	int				dataName;
	void			*pDataName		= NULL;
	int				dataSize		= 0;
	cfgDataType		cfType			= 0;
	int				allocBufSize	= 0;
	int				status			= 0;

	memset(
		&cfgHndl,
		0,
		sizeof(cfgBufHndl));

	va_start(params, pSectionName);     /* Initialize variable arguments. */

	// Send phase
	status = CfgPackBuf(
		&cfgHndl,
		pSectionName,
		pCfgBuf,
		bufSize);

	if (status) {
		return status;
	}

	for (i = 0; i < numOfParams; i++) {

		pKeyName  = va_arg(params, char*);
		dataName  = (int) va_arg(params, void*);
		dataSize  = va_arg(params, int);
		cfType	  = va_arg(params, int);

		if (!pKeyName) {
			return E_INVALID_VA_ARG_PRM;
		}

		pDataName = &dataName;
		if (cfType == cfString ||
			cfType == cfIpAddress) {
			pDataName = (char*) dataName;
		}

#ifdef __PrintOn__
		if (cfType == cfString ||
			cfType == cfIpAddress)
			printf("\nSET:KEY:[%-16s]	DATA:[%-20s] type:[%d] size:[%d] ", 
				pKeyName, 
				(char*) dataName,
				cfType, 
				dataSize);
		else 
			if (cfType == cfINT16)
				printf("\nSET:KEY:[%-16s]	DATA:[%-20d] type:[%d]", 
					pKeyName, 
					((short) dataName),
					cfType);
			else
				if ((cfType == cfINT8) ||
					(cfType == cfBOOL))
					printf("\nSET:KEY:[%-16s]	DATA:[%-20d] type:[%d]", 
						pKeyName, 
						((char) dataName),
						cfType);
				else
						printf("\nSET:KEY:[%-16s]	DATA:[%-20d] type:[%d]", 
							pKeyName, 
							((int) dataName),
							cfType);
		fflush(stdout);
		printf("\n");
#endif

		status = CfgPutData(
			&cfgHndl,
			pKeyName,
			pDataName,
			dataSize,
			cfType,
			&allocBufSize);

		if (status) {
			return status;
		}
	}

	va_end(params);              /* Reset variable arguments.      */

	*pAllocatedSize = allocBufSize;

	return 0;
}

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
	... )
{
  //	static char		rtn[]			= "CfgGetParams";
	//	int				i				= 0;
	va_list			params;
	cfgBufHndl		cfgHndl;
	int				status			= 0;
	int				numOfKeys		= 0;
	
	memset(
		&cfgHndl,
		0,
		sizeof(cfgBufHndl));

	va_start(params, pSectionNameSize);     /* Initialize variable arguments. */

	status = CfgUnpackBuf(
		&cfgHndl,
		pCfgBuf,
		pSectionName,
		pSectionNameSize,
		pBufSize,
		&numOfKeys);

	if (status) {
		return status;
	}

	if (numOfKeys != numOfParams)
		return E_INVALID_NUM_OF_PARAMS_AND_KEYS;

	status = RetrieveParams(
			&cfgHndl,
			numOfParams,
			pCfgBuf,
	//		pBufSize,
			params);
	if (status){
		return status;
	}

	va_end(params);              /* Reset variable arguments.      */

	return 0;
}

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
//  Get parameters number of times in following format
//	char			*pKeyName,			// OUT		: Key name
//	int				*pKeyNameSize,		// OUT		: Key name
//	void			*pData,				// OUT		: Pointer to the data
//	int				*pDataSize,			// OUT		: Data size
//	cfgDataType		dataType,			// IN		: Data type
	... )
{
  //	int				i				= 0;
	va_list			params;
	int				status			= 0;
	//	int				numOfKeys		= 0;

	va_start(params, pCfgBuf);     /* Initialize variable arguments. */

	if (pBufHndl->bInit != CfgTrue)
		return E_NOT_INIT;

	status = RetrieveParams(
			pBufHndl,
			numOfParams,
			pCfgBuf,
	//		pBufSize,
			params);
	if (status){
		return status;
	}

	va_end(params);              /* Reset variable arguments.      */

	return 0;
}

//---------------------------------------------------------------------------
//
//	Function name:	CfgInitBuf
//
//	Description:	Initilize new configuration buffer
//
//	Return code:
//			0					- success
//			negative value		- error
//
//---------------------------------------------------------------------------

int CfgPackBuf(
	cfgBufHndl		*pBufHndl,				// IN : Buffer handle
	char			*pSectionName,			// IN : Section name
	char			*pCfgBuf,				// IN : Configuration buffer
	int				bufSize					// IN : Maximum configuration buffer size
	)
{
	static char		rtn[]		= "CfgPackBuf";
	int				status		= 0;

	if (!pBufHndl ||
		!pCfgBuf  ||
		!pSectionName)
		return E_INVALID_PRM;

	if (pBufHndl->bInit == CfgTrue)
		return E_NOT_INIT;

	memset(
		pBufHndl,
		0,
		sizeof(cfgBufHndl));
	memset(
		pCfgBuf,
		0,
		bufSize);

	pBufHndl->pAppHndl		= NULL;
	pBufHndl->pBuf			= pCfgBuf;
	pBufHndl->pCurBuf		= pBufHndl->pBuf;

	if (bufSize > CfgMaxBufferSize)
		return E_INVALID_BUF_SIZE_1;

	status = FillHeaderInfo(
			pBufHndl,
			sizeof(cfgBufHeader),
			0);
	if (status){
		return E_FILL_HDR;
	}
	pBufHndl->usedBufSize	+= sizeof(cfgBufHeader);
	pBufHndl->pCurBuf		+= pBufHndl->usedBufSize;

	pBufHndl->maxBufSize	= bufSize;

	// Fill section name
	status = PutString(
			pBufHndl,
			pSectionName);
	if (status) {
		FormatError(rtn, pSectionName, status);
		return E_PUT_STRING;
	}

	status = FillHeaderInfo(
			pBufHndl,
			pBufHndl->usedBufSize,
			0);
	if (status){
		return E_FILL_HDR;
	}

	pBufHndl->bInit			= CfgTrue;

	return 0;
}

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
	cfgBufHndl		*pBufHndl,			// IN		: Buffer handle
	char			*pCfgBuf,			// IN		: Configuration buffer
//	int				bufSize,			// IN		: Maximum configuration buffer size
	char			*pSectionName,		// OUT		: Section name
	int				*pSecSize,			// IN/OUT	: Section name size
	int				*pMaxBufSize,		// OUT		: Buffer size from the data
	int				*pNumOfKeys			// OUT		: Number of keys in buffer
	)
{
	static char		rtn[]		= "CfgUnpackBuf";
	int				status		= 0;
	int				inBufSize	= 0;
	//	int				secNameLen	= 0;
	int				numOfKeys	= 0;
	char			tmpLen[4];
	if (!pBufHndl ||
		!pCfgBuf  ||
		!pSectionName)
		return E_INVALID_PRM;

	if (pBufHndl->bInit == CfgTrue)
		return E_NOT_INIT;

	memset(
		pBufHndl,
		0,
		sizeof(cfgBufHndl));

	pBufHndl->pBuf	  = pCfgBuf;
	pBufHndl->pCurBuf = pBufHndl->pBuf;

	tmpLen[0] = pBufHndl->pCurBuf[0];
	tmpLen[1] = pBufHndl->pCurBuf[1];
	tmpLen[2] = pBufHndl->pCurBuf[2];
	tmpLen[3] = pBufHndl->pCurBuf[3];

	// Read overall size from the header
	inBufSize  = atoi(tmpLen);
	// Received buffer size validation
	if ((inBufSize < 0) ||
		(inBufSize > CfgMaxBufferSize)) 
		return E_INVALID_BUF_SIZE_2;

//	if (inBufSize > bufSize)
//		return E_INVALID_BUF_SIZE_3;

	pBufHndl->pCurBuf = pBufHndl->pCurBuf + NumOfKeysOffset;

	tmpLen[0] = pBufHndl->pCurBuf[0];
	tmpLen[1] = pBufHndl->pCurBuf[1];
	tmpLen[2] = 0;
	tmpLen[3] = 0;

	numOfKeys  = atoi(tmpLen);

	// Pass the header
	pBufHndl->pCurBuf = pBufHndl->pBuf + sizeof(cfgBufHeader);
	pBufHndl->usedBufSize += sizeof(cfgBufHeader);

	// Read the section name length and section
	status = GetString(
			pBufHndl,
			pSectionName,
			pSecSize);
	if (status) {
		FormatError(rtn, pSectionName, status);
		return E_GET_STRING;
	}

	*pMaxBufSize = inBufSize;
	*pNumOfKeys  = numOfKeys;

	pBufHndl->maxBufSize = inBufSize;
	pBufHndl->bInit = CfgTrue;

	return 0;
}

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
	cfgBufHndl		*pBufHndl,			// IN : Buffer handle
	char			*pKeyName,			// IN : Key name
	void			*pData,				// IN : Pointer to the data
	int				dataSize,			// IN : Data size
	cfgDataType		dataType,			// IN : Data type
	int				*pAllocatedSize		// OUT : Allocated size in this buffer
	)
{
	static char		rtn[]		= "CfgPutData";
	int				status		= 0;

	if (!pBufHndl ||
		!pKeyName ||
		!pData	  || 
		!pAllocatedSize)
		return E_INVALID_PRM;

	if (pBufHndl->bInit != CfgTrue)
		return E_NOT_INIT;

	if ((strlen(pKeyName) + dataSize) > pBufHndl->maxBufSize)
		return E_BUF_OVERFLOW;

	// Fill key name
	status = PutString(
			pBufHndl,
			pKeyName);
	if (status) {
		return -20;
	}

	// Fill data place
	switch (dataType) {
		case cfString: {
			status = PutStringN(
							pBufHndl,
							(char*) pData,
							dataSize);
			if (status) {
				FormatError(rtn, pKeyName, status);
				return E_PUT_STRING_N;
			}
		}
		break;
		case cfIpAddress: {

			status = PutStringN(
							pBufHndl,
							(char*) pData,
							dataSize);
			if (status) {
				FormatError(rtn, pKeyName, status);
				return E_PUT_STRING_N;
			}
		}
		break;
		case cfBOOL: {

			status = PutBool(
							pBufHndl,
							(char*) pData);
			if (status) {
				FormatError(rtn, pKeyName, status);
				return E_PUT_BOOL;
			}
		}
		break;
		case cfINT32: {
			status = PutInt32(
							pBufHndl,
							(int*) pData);
			if (status) {
				FormatError(rtn, pKeyName, status);
				return E_PUT_INT32;
            }           
        }
        break;
        case cfUINT32: {
        	status = PutUint32(
        	                pBufHndl,
        	                (unsigned long*) pData);
            if (status) {
 //               FormatError(pKeyName, status);
                return E_PUT_UINT32;
            }           
		}
		break;
		case cfINT16: {
			status = PutInt16(
							pBufHndl,
							(short*) pData);
			if (status) {
				FormatError(rtn, pKeyName, status);
				return E_PUT_INT16;
            }           
        }
        break;
        case cfUINT16: {
        	status = PutUint16(
        	                pBufHndl,
        	                (unsigned short*) pData);
            if (status) {
//                FormatError(pKeyName, status);
                return E_PUT_INT16;
            }           
		}
		break;
		case cfINT8: {
			status = PutInt8(
							pBufHndl,
							(char*) pData);
			if (status) {
				FormatError(rtn, pKeyName, status);
				return E_PUT_INT8;
			}			
		}
		break;

		default:
			return E_PUT_UNKNOWN_TYPE;
	}

	pBufHndl->numberOfKeys++;

	status = FillHeaderInfo(
			pBufHndl,
			pBufHndl->usedBufSize,
			pBufHndl->numberOfKeys);
	if (status){
		return E_FILL_HDR;
	}

	*pAllocatedSize = pBufHndl->usedBufSize;

	return 0;
}

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
	int				*pDataSize,			// IN/OUT	: Data size
	cfgDataType		dataType			// IN		: Data type
	)
{
	static char		rtn[]		= "CfgGetData";
	int				status		= 0;

	if (!pBufHndl ||
		!pKeyName ||
		!pData)
		return E_INVALID_PRM;

	if (pBufHndl->bInit != CfgTrue)
		return E_NOT_INIT;

	if (pBufHndl->usedBufSize >= pBufHndl->maxBufSize)
		return E_BUF_IS_EMPTY;

	if ((dataType == cfString) &&
		(!pDataSize))
		return E_INVALID_DATA_SIZE;

	// Get a key
	status = GetString(
			pBufHndl,
			(char*) pKeyName,
			pKeySize);
	if (status) {
		FormatError(rtn, pKeyName, status);
		return E_GET_STRING;
	}

	// Get data from their place
	switch (dataType) {
		case cfString: {
			status = GetString(
							pBufHndl,
							(char*) pData,
							pDataSize);
			if (status) {
				FormatError(rtn, (char*) pData, status);
				return E_GET_STRING;
			}
		}
		break;
		case cfIpAddress: {
			*((int*) pData) = (int) 0;
			status = GetIpAddress(
							pBufHndl,
							(int*) pData);
			if (status) {
				FormatError(rtn, (char*) pData, status);
				return E_GET_IP_ADDR;
			}
		}
		break;
		case cfBOOL: {
			status = GetBool(
							pBufHndl,
							(char*) pData);
			if (status) {
				FormatError(rtn, (char*) pData, status);
				return E_GET_BOOL;
			}
		}
		break;
		case cfINT32: {
			*((int*) pData) = (int) 0;
			status = GetInt32(
							pBufHndl,
							(int*) pData);
			if (status) {
				FormatError(rtn, (char*) pData, status);
				return E_GET_INT32;
			}
		}
		break;
		case cfINT16: {
			*((short*) pData) = (short) 0;
			status = GetInt16(
							pBufHndl,
							(short*) pData);
			if (status) {
				FormatError(rtn, (char*) pData, status);
				return E_GET_INT16;
            }
        }
        break;
        case cfUINT16: {
            *((unsigned short*) pData) = (unsigned short) 0;
            status = GetUint16(
                            pBufHndl,
                            (unsigned short*) pData);
            if (status) {
 //               FormatError((char*) pData, status);
                return E_GET_INT16;
            }
        }
		break;
		case cfINT8: {
			status = GetInt8(
							pBufHndl,
							(char*) pData);
			if (status) {
				FormatError(rtn, (char*) pData, status);
				return E_GET_INT8;
			}
		}
		break;

		default:
			return E_GET_UNKNOWN_TYPE;
	}

	return 0;
}

char* CfgGetFailRoutine()
{
	return cfgFailRtn;
}

char* CfgGetFailStr()
{
	return cfgFailStr;
}

int CfgGetFailStatus()
{
	return cfgFailStatus;
}

// ----------------------------------------------------------------------------------------------
// -------------------------------- Static routines ---------------------------------------------
// ----------------------------------------------------------------------------------------------

static int RetrieveParams(
	cfgBufHndl		*pBufHndl,			
	int				numOfParams,		
	char			*pCfgBuf,			
//	int				*pBufSize,			
	va_list			params)
{
	char			*pKeyName		= NULL;
	int				*pKeyNameSize	= NULL;
	void			*pData			= NULL;
	int				*pDataSize		= NULL;
	cfgDataType		dataType;
	int				status			= 0;
	int				i;

	for (i = 0; i < numOfParams; i++) {

		pKeyName		= va_arg(params, char*);
		pKeyNameSize	= va_arg(params, int*);
		pData			= va_arg(params, void*);
		pDataSize		= va_arg(params, int*);
		dataType		= (cfgDataType) va_arg(params, int*/*cfgDataType*/);

		if (!pKeyName ||
			!pKeyNameSize ||
			!pData) {
			return E_INVALID_VA_ARG_PRM;
		}

		if ((dataType == cfString) &&
			(!pDataSize)) {
			return E_INVALID_VA_ARG_PRM;
		}

		status = CfgGetData(
					pBufHndl,
					pKeyName,
					pKeyNameSize,
					pData,
					pDataSize,
					dataType);
		if (status) {
			return status;
		}

#ifdef __PrintOn__
		if (dataType == cfString)
			printf("\nRCV:KEY:[%-16s]	DATA:[%-20s] type:[%d] size:[%d] ", 
				pKeyName, 
				(char*) pData,
				dataType, 
				*pDataSize);
		else 
			if (dataType == cfINT16)
				printf("\nRCV:KEY:[%-16s]	DATA:[%-20d] type:[%d]", 
					pKeyName, 
					*((short*) pData),
					dataType);
			else
				if ((dataType == cfINT8) ||
					(dataType == cfBOOL)) {
					printf("\nRCV:KEY:[%-16s]	DATA:[%-20d] type:[%d]", 
						pKeyName, 
						*((char*) pData),
						dataType);
				}
				else
					if (dataType == cfIpAddress)
						printf("\nRCV:KEY:[%-16s]	DATA:[0x%-18x] type:[%d]", 
							pKeyName, 
							*((int*) pData),
							dataType);
					else
						printf("\nRCV:KEY:[%-16s]	DATA:[%-20d] type:[%d]", 
							pKeyName, 
							*((int*) pData),
							dataType);
		fflush(stdout);
		printf("\n");
#endif
	}

	return 0;
}

static int	FillHeaderInfo(
	cfgBufHndl		*pBufHndl,
	int				curBufSize,
	int				numOfParams)
{
	// Fill header information
	sprintf(pBufHndl->pBuf, "%4d%2d%1s", curBufSize, numOfParams, HeaderSign);
	pBufHndl->pBuf[7] = '@';

	return 0;
}

static int	PutString(
	cfgBufHndl		*pBufHndl,	
	char			*pStr)
{
  //	static char		rtn[]		= "PutString";
	int				len			= strlen(pStr);

	pBufHndl->pCurBuf[0] = (char) len;
	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;
	sprintf(pBufHndl->pCurBuf, "%s", pStr);
	pBufHndl->usedBufSize += len;
	pBufHndl->pCurBuf	  += len;	

	return 0;
}

static int	PutStringN(
	cfgBufHndl		*pBufHndl,	
	char			*pStr,
	int				strSize)
{
  //	static char		rtn[]		= "PutStringN";

	pBufHndl->pCurBuf[0] = (char) strSize;
	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	memcpy(
		pBufHndl->pCurBuf,
		pStr,
		strSize);
	pBufHndl->usedBufSize += strSize;
	pBufHndl->pCurBuf	  += strSize;	

	return 0;
}

static int	PutBool(
	cfgBufHndl		*pBufHndl,	
	char			*pBool)
{
  //	static char		rtn[]		= "PutBool";
	int				len			= 0;
	int				cursor		= 0;
	char			*pCursor	= 0;

	cursor = pBufHndl->usedBufSize;
	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	len = 1;
	// TRUE/FALSE passed as 'T' or 'F' character
	pBufHndl->pCurBuf[0] = (*pBool)?'T':'F';

	pCursor	= &pBufHndl->pBuf[cursor];
	pCursor[0] = (char) 1;
	pBufHndl->usedBufSize++;
	pBufHndl->pCurBuf++;

	return 0;
}


static int PutIpAddress(cfgBufHndl		*pBufHndl,	
			char		*pStr,
			int		strSize)
{

  //	static char		rtn[]		= "PutIpAddress";
	int				len			= 0;
	int				cursor		= 0;
	int				ipAddress	= 0;
	char			*pCursor	= 0;
	char			intBuf[16]  = {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0};
	int				status		= 0;

	status = inetAddrNoSockets(pStr, &ipAddress);
	if (status){
		return status;
	}

	cursor = pBufHndl->usedBufSize;
	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	sprintf(intBuf, "0x%x", ipAddress);
	len = strlen(intBuf) + 1;
	memcpy(
		pBufHndl->pCurBuf,
		intBuf,
		len);

	pCursor	= &pBufHndl->pBuf[cursor];
	pCursor[0] = (char) len;
	pBufHndl->usedBufSize += len;
	pBufHndl->pCurBuf	  += len;

	return 0;
	}


static int	PutInt32(
	cfgBufHndl		*pBufHndl,	
	int				*pInt)
{
  //	static char		rtn[]		= "PutInt32";
	int		len					= 0;
	int		cursor				= 0;
	char	*pCursor			= 0;
	char	intBuf[16]			= {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0};

	cursor = pBufHndl->usedBufSize;
	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	sprintf(intBuf, "%d", *pInt);
	len = strlen(intBuf);
	memcpy(
		pBufHndl->pCurBuf,
		intBuf,
		len);

	pCursor	= &pBufHndl->pBuf[cursor];
	pCursor[0] = (char) len;
	pBufHndl->usedBufSize += len;
	pBufHndl->pCurBuf	  += len;

	return 0;
}

static int  PutUint32(
    cfgBufHndl      *pBufHndl,  
    unsigned long   *pInt)
{
    int     len                 = 0;
    int     cursor              = 0;
    char    *pCursor            = 0;
    char    intBuf[16]          = {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0};

    cursor = pBufHndl->usedBufSize;
    pBufHndl->pCurBuf++;
    pBufHndl->usedBufSize++;

    sprintf(intBuf, "%d", (int) *pInt);
    len = strlen(intBuf);
    memcpy(
        pBufHndl->pCurBuf,
        intBuf,
        len);

    pCursor = &pBufHndl->pBuf[cursor];
    pCursor[0] = (char) len;
    pBufHndl->usedBufSize += len;
    pBufHndl->pCurBuf     += len;

    return 0;
}

static int	PutInt16(
	cfgBufHndl		*pBufHndl,	
	short			*pShort)
{
  //	static char		rtn[]		= "PutInt16";
	int				len			= 0;
	int				cursor		= 0;
	char			*pCursor	= 0;
	char			shortBuf[6]	= {0,0, 0,0, 0,0};

	cursor = pBufHndl->usedBufSize;
	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	snprintf(shortBuf, sizeof(shortBuf), "%d", *pShort);
	len = strlen(shortBuf);
	memcpy(
		pBufHndl->pCurBuf,
		shortBuf,
		len);

	pCursor		= &pBufHndl->pBuf[cursor];
	pCursor[0]	= (char) len;
	pBufHndl->usedBufSize += len;
	pBufHndl->pCurBuf	  += len;

	return 0;
}

static int  PutUint16(
    cfgBufHndl      *pBufHndl,  
    unsigned short  *pShort)
{
    int             len         = 0;
    int             cursor      = 0;
    char            *pCursor    = 0;
    char            shortBuf[6] = {0,0, 0,0, 0,0};

    cursor = pBufHndl->usedBufSize;
    pBufHndl->pCurBuf++;
    pBufHndl->usedBufSize++;

    snprintf(shortBuf, sizeof(shortBuf), "%d", (unsigned short) *pShort);

    len = strlen(shortBuf);
    memcpy(
        pBufHndl->pCurBuf,
        shortBuf,
        len);

    pCursor     = &pBufHndl->pBuf[cursor];
    pCursor[0]  = (char) len;
    pBufHndl->usedBufSize += len;
    pBufHndl->pCurBuf     += len;

    return 0;
}

static int	PutInt8(
	cfgBufHndl		*pBufHndl,	
	char			*pInt8)
{
  //	static char			rtn[]	= "PutInt8";
	int		len					= 0;
	int		cursor				= 0;
	char	*pCursor			= 0;

	cursor = pBufHndl->usedBufSize;
	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	len = 1;
	pBufHndl->pCurBuf[0] = *pInt8;

	pCursor	= &pBufHndl->pBuf[cursor];
	pCursor[0] = (char) 1;
	pBufHndl->usedBufSize++;
	pBufHndl->pCurBuf++;

	return 0;
}

static int	GetString(
	cfgBufHndl	*pBufHndl,
	char		*pBuf,
	int			*pBufSize)
{
	static	char			rtn[]	= "GetString";
	int						strSize	= 0;

	// Read the section name length and section
	strSize = pBufHndl->pCurBuf[0];

	if (strSize > *pBufSize) {
		pBuf[0] = '\0';
		FormatError(rtn, "Invalid buffer size", E_INVALID_BUF_SIZE_4);
		return E_INVALID_BUF_SIZE_4;
	}

	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	memcpy(
		pBuf,
		pBufHndl->pCurBuf,
		strSize);
	pBuf[strSize] = '\0';
	pBufHndl->pCurBuf	+= strSize;
	pBufHndl->usedBufSize += strSize;

	*pBufSize = strSize;

	return 0;
}

static int	GetBool(
	cfgBufHndl	*pBufHndl,
	char		*pBuf)
{
  //	static char			rtn[]		= "GetBool";
	int					dataSize	= 0;

	// Read the section name length and section
	dataSize = pBufHndl->pCurBuf[0];
	if (dataSize > 1)
		return E_INVALID_DATA_SIZE;

	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	*pBuf = pBufHndl->pCurBuf[0];

	// TRUE/FALSE passed as 'T' or 'F' character
	*pBuf = (*pBuf)=='T'?1:0;

	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	return 0;
}

static int	GetInt32(
	cfgBufHndl	*pBufHndl,
	int			*pBuf)
{
  //	static char		rtn[]		= "GetInt32";
	int				dataSize	= 0;
	char			intBuf[16]  = {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0};

	// Read the section name length and section
	dataSize = pBufHndl->pCurBuf[0];
	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	memcpy(
		intBuf,
		pBufHndl->pCurBuf,
		dataSize);
	*pBuf = atoi(intBuf);

	pBufHndl->pCurBuf	+= dataSize;
	pBufHndl->usedBufSize += dataSize;

	return 0;
}

static int	GetInt16(
					 cfgBufHndl	*pBufHndl,
					 short		*pBuf)
{
  //	static char		rtn[]		= "GetInt16";
	int				dataSize	= 0;
	char			intBuf[16]  = {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0};
	
	// Read the section name length and section
	dataSize = pBufHndl->pCurBuf[0];
	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;
	
	memcpy(
		intBuf,
		pBufHndl->pCurBuf,
		dataSize);
	*pBuf = atoi(intBuf);
	
	pBufHndl->pCurBuf	+= dataSize;
	pBufHndl->usedBufSize += dataSize;
	
	return 0;
}

static int  GetUint16(
    cfgBufHndl  *pBufHndl,
    unsigned short       *pBuf)
{
    int             dataSize    = 0;
    char            intBuf[6]  = {0,0, 0,0, 0,0};

    // Read the section name length and section
    dataSize = pBufHndl->pCurBuf[0];
    pBufHndl->pCurBuf++;
    pBufHndl->usedBufSize++;

    memcpy(
        intBuf,
        pBufHndl->pCurBuf,
        dataSize);
    *pBuf = atoi(intBuf);

//  {
//      int i;
//
//      printf("\n\nGET_INT16: pCur:0x%x DataSize:%d\nIntBuf:",
//          pBufHndl->pCurBuf,
//          dataSize);
//
//      for (i = 0; i < 6; i++)
//          printf("0x%x.", intBuf[i]);
//
//      printf("\n");
//      printf("\n*pBuf=%d 0x%x", *pBuf, pBuf);
//  }

    pBufHndl->pCurBuf   += dataSize;
    pBufHndl->usedBufSize += dataSize;

    return 0;
}

static int	GetInt8(
	cfgBufHndl		*pBufHndl,
	char			*pBuf)
{
  //	static char		rtn[]		= "GetInt8";
	int				dataSize	= 0;

	// Read the section name length and section
	dataSize = pBufHndl->pCurBuf[0];
	if (dataSize > 1)
		return E_INVALID_DATA_SIZE;

	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	*pBuf = pBufHndl->pCurBuf[0];

	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	return 0;
}

static int	GetIpAddress(
	cfgBufHndl	*pBufHndl,
	int			*pBuf)
{
  //	static char		rtn[]		= "GetIpAddress";
	char			tmpBuf[16]	= {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0};
	int				strSize		= 0;
	int				status		= 0;

	// Read the section name length and section
	strSize = pBufHndl->pCurBuf[0];

	if (strSize > 16) {
		return E_INVALID_BUF_SIZE_5;
	}

	pBufHndl->pCurBuf++;
	pBufHndl->usedBufSize++;

	memcpy(
		tmpBuf,
		pBufHndl->pCurBuf,
		strSize);

	pBufHndl->pCurBuf	+= strSize;
	pBufHndl->usedBufSize += strSize;

	status = inetAddrNoSockets(tmpBuf, pBuf);
	if (status){
		return status;
	}

	return 0;
}

/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
/*  */
/* inet_addr */
static int inetAddrNoSockets(
	char		*cp,
	int			*val)
{
	if (inetAtonNoSockets(cp, val))
		return E_INVALID_STR_TO_IP;

	return 0;
}

/* 
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
/*  */
/* inet_aton */
static int inetAtonNoSockets(
	char	*cp,
	int		*addr)
{
	int val;
	int base, n;
	char c;
	int i;
	unsigned int parts[4];
	register unsigned int *pp = parts;

	for(i = 0; i < 4; i++)
		parts[i] = 0;	

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return (0);
		val = 0; base = 10;
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		for (;;) {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
			break;
		}
		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */

			if (pp >= parts + 3)
				return (0);
			*pp++ = (unsigned int) val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 0:
		return (0);		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (val > 0xffffff)
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}

	if (addr)
		*addr = /*htonl(*/val/*)*/;

	return 0;
}

static char *ipToString(
	int		ipAddr,
	char*	buf)
{
	unsigned char* ip = (unsigned char*) &ipAddr;

	ipAddr = htobige(ipAddr)//;
	sprintf(buf,"%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

	return buf;
}

static void FormatError(
	char			*rtn, 
	char			*pStr, 
	int				status)
{
	strncpy(cfgFailRtn,	rtn, sizeof(cfgFailRtn)-1);
	strncpy(cfgFailStr, pStr, sizeof(cfgFailStr)-1);
	cfgFailStatus = status;
#ifdef __PrintOn__
	printf("\nCFG: [%s]:[%s] returned status=0x%x\n", rtn, pStr, status);
	fflush(stdout);
	printf("\n\n");
#endif
}
