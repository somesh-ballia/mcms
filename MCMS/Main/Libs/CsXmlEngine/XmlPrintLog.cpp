// XmlPrintLog.c
// Written by Ami Noy

#undef	SRC
#define	SRC			"XMLLOG"

//	Include files:
//----------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/times.h> 
#include <sys/wait.h>
#include <netinet/in.h>
#include <ctype.h>

#include <XmlFormat.h>

#ifdef __CS__
#ifdef __cplusplus
extern "C" {
#endif  
#include <AcCommonTrace.h>
#ifdef __cplusplus
}
#endif
#endif

#include <XmlDefs.h>
#include <XmlPrintLog.h>

#ifndef __CS__
#include <Trace.h>
#endif

// Macros:
//-------------------
#define ByteValuesInLine		16
#define NumOfAsciiBytes			ByteValuesInLine * 3
#define FullLine				NumOfAsciiBytes + 2 + ByteValuesInLine + 2 // NumOfAsciiBytes + '\t' + '\t' +  ByteValuesInLine + '\n'+ '\0'
#define MaxPrintFormatLength    XmlMessageLen//(1024 * 32)
#define MaxPrintBytesBufferSize XmlMessageLen//MaxPrintFormatLength

#define MaxAsciiBufferSize		1024 * 2

// Type definitions:
//-------------------

// Static variables:
//-------------------
static unsigned long  xmlTraceId;

static short 	shortSize = sizeof(short);
static long		longSize =  sizeof(long);

// Initial parameters:
//--------------------
char buffer[MaxPrintBytesBufferSize];
char tempSrc[MaxPrintBytesBufferSize];
char asciiBuffer[MaxAsciiBufferSize];
char spaceBuffer[NumOfAsciiBytes];

// Global variables:
//-------------------
extern int xmlTraceLevel;

// External variables:
//---------------------

// Forward declarations:
//-----------------------

// Global Routines:
//-----------------
static BOOL IsPrintable(
	unsigned char	value)
{
	if ((value >= 0x20) &&
		(value <= 0x7F))
		return TRUE;
	else
		return FALSE;
}

char *SetMemoryToAsciiMessage(
	unsigned long	binAddress,
	int				size)
{
	unsigned char	*pByteData	= (unsigned char *) (binAddress);
	
	char			*pMessage	= &asciiBuffer[0];

	int				i;
	
	if (size > MaxAsciiBufferSize)
		size = MaxAsciiBufferSize - 1;
		
	for (i = 0; i < size; i++) {
		
		if (isprint((int)pByteData[i]))
			*pMessage = pByteData[i];
		else
			*pMessage = '.';

		pMessage++;
	}

	*pMessage = '\0';
	
	return  &asciiBuffer[0];

} // SetMemoryToAsciiMessage

#ifdef __CS__
int XmlPrintLogInit()
{
	int status;
	
	if (status = AcTraceGetId(
    	"XmlTrace",
    	&xmlTraceId)) {
    		
    	return status;	
    }
	
	return 0;
			
}
#endif // __CS__

char* XmlPrintBytes(
	char 	*valToPrint,
	int 	varType,
	int		numOfBytes)
{
	unsigned char byteValue[2];	

	int 	i;
	int 	index;
	int		nBytes;
	int 	sumOfWrittenBytes;
	int		nSpaces;

#ifndef __XmlTrace__
	return NULL;
#endif

	if (numOfBytes > MaxPrintBytesBufferSize) {
		
		sprintf(&buffer[0], "message size is %d bigger than MaxPrintBytesBufferSize", numOfBytes);
		return &buffer[0];
	}

	memset(
		&tempSrc[0],
		0,
		MaxPrintBytesBufferSize);
		
	memset(
		&buffer[0],
		0,
		MaxPrintBytesBufferSize);
	
	memset(
		&spaceBuffer[0],
		0,
		NumOfAsciiBytes);
		
	memcpy(
		&tempSrc[0],
		valToPrint,
		numOfBytes);

	byteValue[1] = 0;

	for (i = 0, sumOfWrittenBytes = 0; i < numOfBytes; i++) {
	
		byteValue[0] = (char)tempSrc[i];

		sumOfWrittenBytes += sprintf(&buffer[sumOfWrittenBytes], "%02x ", byteValue[0]);			
		
		if (varType == binVarType) {

			index = i + 1;

			if (!(index & 0xF)) {
				sumOfWrittenBytes += sprintf(&buffer[sumOfWrittenBytes], "\t\t%s\n", SetMemoryToAsciiMessage((unsigned long)&tempSrc[index - ByteValuesInLine], ByteValuesInLine));		
			}
		}
			
		// if the we reached to buffer size near or equal to max buffer size we return the buffer,
		// although it isn't all the binary, to prevent overflow writing.
		if ((sumOfWrittenBytes + FullLine) >= MaxPrintBytesBufferSize)
			return buffer;
	}
	
	if (varType == binVarType) {
		
		index = i & 0xF;
		
		if (index) {
			
			nSpaces = NumOfAsciiBytes - (index * 3);

			memset(
				&spaceBuffer[0],
				' ',
				 nSpaces);
			
			// if the we reached to buffer size near or equal to max buffer size we return the buffer,
			// although it isn't all the binary, to prevent overflow writing.
			if ((sumOfWrittenBytes + FullLine) >= MaxPrintBytesBufferSize)
				return buffer;
				
			sumOfWrittenBytes += sprintf(&buffer[sumOfWrittenBytes], "%s\t\t%s\n", spaceBuffer, SetMemoryToAsciiMessage((unsigned long)&tempSrc[i - index], index));		
		}
	}
	
	return buffer;
	
}// XmlPrintBytes


/*****************************************************************************
*
* Function Name : XmlPrintLog
*
* Description   : Print variable number of parameters to Log
*
* Return Code   :
*
*   - 0                     ... OK
*
******************************************************************************/

static char    message[MaxPrintFormatLength];

int XmlPrintLog(
    char *printFormat,  /* IN : printf format parameters */
    int  keyLevel,      /* IN : Unique printing source key */
    ...)
{
 
#ifndef __XmlTrace__
	return 0;
#endif
	
	if (xmlTraceLevel == emXmlTraceNoTrace)
		return 0;
	else if ((xmlTraceLevel == emXmlTraceErrors) && (keyLevel > PERR))
		return 0;
	else if (((xmlTraceLevel == emXmlTraceErrWrn) && (keyLevel > PWRN)))
		return 0;
	
    va_list printList = NULL;
        
    va_start(printList, keyLevel);   

	memset(message, 0, MaxPrintFormatLength);	
    if (printList)    
        vsnprintf(message, MaxPrintFormatLength - 1 ,printFormat, printList);
        
#ifdef __CS__
	AcTraceStringRequest(
		xmlTraceId,
    	1,
		keyLevel,
		"%s",
		message);
#else
	if (xmlTraceLevel == emXmlTraceFull)
		FPTRACE2(eLevelInfoNormal, "XML TRACE", message);
	else
		FPTRACE2(eLevelError, "XML TRACE", message);
#endif // __CS__


    fflush(stdout);
    
    va_end(printList);

    return 0;
    
}// XmlPrintLog
