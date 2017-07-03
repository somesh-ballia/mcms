// Parser.c
// Ami Noy

// Includes files
//----------------
#include <stdlib.h>
#include <string.h>
#include <XmlDefs.h>
#include <XmlErrors.h>
#include <Parser.h>
#include <XmlPrintLog.h>

// Macros
//--------

#undef	SRC
#define SRC	"Parser"

typedef struct{
    char chr;
    int value;
} hexMapSt ;

typedef struct {
	char lowChar;
	char highChar;
} letterCaseSt;

	
#define hexMapLen  16

//	Static variables
//-------------------
static hexMapSt hexMap[hexMapLen] = {
    {'0', 0}, {'1', 1},
    {'2', 2}, {'3', 3},
    {'4', 4}, {'5', 5},
    {'6', 6}, {'7', 7},
    {'8', 8}, {'9', 9},
    {'a', 10}, {'b', 11},
    {'c', 12}, {'d', 13},
    {'e', 14}, {'f', 15}
};

static letterCaseSt letterCaseTable[] = {
	{'a', 'A'},
	{'b', 'B'},
	{'c', 'C'},
	{'d', 'D'},
	{'e', 'E'},
	{'f', 'F'}	
};

// Global variables
//------------------



// External variables
//------------------

//	External Routines:
//--------------------

// Forward Declarations:
//----------------------

// Routines:
//----------
//---------------------------------------------------------------------------
//
//	Function name:	GetLetterLowerCaseFromTable
//
//	Description:	
//					
//	Return code:
//			value form table	- success
//			negative value		- error
//			
//---------------------------------------------------------------------------
static int GetLetterLowerCaseFromTable(char *highStr, char *lowStr)
{
	letterCaseSt *pStrTbl = &letterCaseTable[0];
	
	while (pStrTbl->highChar != *highStr)
		pStrTbl++;
		
	*lowStr = pStrTbl->lowChar;
	
	return 0;	
	
}// GetLetterLowerCaseFromTable

//---------------------------------------------------------------------------
//
//	Function name:	GetHexValFromTable
//
//	Description:	Convert char to hex value
//					
//	Return code:
//			value form table	- success
//			negative value		- error
//			
//---------------------------------------------------------------------------
static int GetHexValFromTable(char *str)
{
	int i;

	for (i = 10; i < hexMapLen; i++) {

		if (*str == hexMap[i].chr)
			return hexMap[i].value;
	}

	return -1;
}

//---------------------------------------------------------------------------
//
//	Function name:	ConvertStringToHex1
//
//	Description:	Convert a string to binary value
//					(this work on string that start with 0x)			
//	Return code:
//			result				- success
//			negative value		- error
//			
//---------------------------------------------------------------------------
static int ConvertStringToHex(char *str)
{
	int				val = 0;
	unsigned long	result = 0;
	
	char tempStr;
	
	while (*str != '<') {

		if ((*str >= '0') && (*str <= '9'))
			val = (*str - '0');
		else {
		
			if ((*str >= 'A') && (*str <= 'F'))
				 GetLetterLowerCaseFromTable(str, &tempStr);
   		    else if ((*str >= 'a') && (*str <= 'f'))
   		    	tempStr = *str;
   		    else 
   		    	tempStr = '@';
			
			val = GetHexValFromTable(&tempStr);
		}

		result <<= 4; 
		result |= val;

		str++;
	}
	
	return result;

}// ConvertStringToHex

int ConvertStringToDec(char *str)
{
	int		val = 0;

	str = strstr(str, ">");
	str++;

	val = atoi(str);

	return val;
}

//---------------------------------------------------------------------------
//
//	Function name:	GetXmlVal
//
//	Description:	Get XML string and parse it until the beginning of the binary variable
//					
//	Return code:
//			result				- success
//			negative value		- error
//			
//---------------------------------------------------------------------------
int GetXmlVal(char *line, char *string)
{
	char *pStr;
	unsigned long val;

	pStr = strstr(line, string); 
	
	if (!pStr)
		return E_XML_NAME_NOT_FOUND;

	pStr += (strlen(string) + 1); // string + ">"

	pStr += 2;// skip on '0x'

	val = ConvertStringToHex(pStr);

	return val;
}

//---------------------------------------------------------------------------
//
//	Function name:	GetXmlString
//
//	Description:	Get XML string and parse it until the beginning of the binary string
//					
//	Return code:
//			0					- success
//			negative value		- error
//			
//---------------------------------------------------------------------------
int GetXmlString(
	char    *xmlLine,
    char    *elementName,
    char    *msgOut)
{
	char *pStartStr;
	char *pEndStr;

	int len;

	pStartStr = strstr(xmlLine, elementName); 
	
	if (!pStartStr)
		return E_XML_NAME_NOT_FOUND;
	
	pStartStr += strlen(elementName); // string + ">"

	pStartStr += 1;

	pEndStr = strstr(pStartStr, "<");

	len = pEndStr - pStartStr;

    if (len < 0)
        return E_XML_INVALID_STR_LEN;
        
	memcpy(msgOut, pStartStr, len);

    *((char *) msgOut + len) = '\0';
    
	return 0;
}

//---------------------------------------------------------------------------
//
//  Function name:  GetXmlDynamicPart
//
//  Description:    Get XML string and parse it until the beginning of the binary string
//                  
//  Return code:
//          0                   - success
//          negative value      - error
//          
//---------------------------------------------------------------------------
int GetXmlDynamicPart(
    char    *xmlLine,
    char    *tagName,
    char    *msgOut,
    int     len)
{
	int     i;
	
	unsigned long	byteValue;
    char 	*pStartStr;
    char	*pEndStrToScan;

    pStartStr = strstr(xmlLine, tagName); 
    
    if (!pStartStr) {
    	XmlPrintLog("Start tag name not found: tag name = %s, len = %d", PERR, tagName, len);
        return E_XML_NAME_NOT_FOUND;
    }
    
    pStartStr += strlen(tagName); // skip on tag name 

    pStartStr += 1; // skip on '>'
    pEndStrToScan = strstr(pStartStr, "</");// find end of string to scan
		
	if (!pEndStrToScan) {
		XmlPrintLog("End tag name not found: tag name = %s, len = %d", PERR, tagName, len);
		return E_XML_INVALID_PARAMETER;
	}
	
	for (i = 0; i < len; i++) {
		
		byteValue =  strtoul(pStartStr, &pEndStrToScan, 16);
	
		msgOut[i] = byteValue;
	
		pStartStr += 3;		
	}
    
    return 0;

}// GetXmlDynamicPart
